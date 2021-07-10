/*
  libgcc recognizes C++ PODs, raw pointers and unions, but std::strings,
  std::functions, smart pointers and more complex objects like structs are all
  bucketed into record_type_class, which may have any arbitrary size.  Any
  "record" with size larger than 8 bytes appears to cause the BPF JIT to choke.
  libgcc's reference_type_class, complex_type_class, function_type_class,
  method_type_class, array_type_class and string_type_class all appear to be
  unreachable from C++.   What enumeral_type_class, offset_type_class, and
  lang_type_class are is hard to guess.  Nothing appears to be classified as
  opaque_type_class.
 */
#include "arg_classifier.h"

#include "gcc/typeclass.h"
#include "gtest/gtest.h"

#include <chrono>
#include <complex>
#include <memory>

namespace arg_classify {
namespace local_testing {

union aunion_type {
  double val;
  int first;
  int second;
};

struct astruct_type {
  int val;
  char first;
  char second;
  char third;
  char fourth;
};

struct bstruct_type {
  bstruct_type() : val(3), first('a') {}
  bstruct_type(int a, char b) : val(a), first(b) {}
  // clang-format off
  //  bstruct_type(const struct bstruct_type& b) : val(b.val),  first(b.first) {}
  // clang-format on
  int val;
  char first;
};

TEST(TypeErasureTest, Basic) {
  std::vector<Parameter> param_list;
  param_list.push_back(Parameter(42));
  // Without the explicit string constructor, the function receives the
  // parameter as a constr char* and emits an array about pushing an array.
  param_list.push_back(Parameter(std::string("hello world")));

  union aunion_type u {};
  param_list.push_back(Parameter(u));
  struct astruct_type a {};
  param_list.push_back(Parameter(a));
  struct bstruct_type b {
    11, 'b'
  };
  param_list.push_back(Parameter(b));
  //  param_list.push_back(Parameter(union aunion_type a()));

  std::vector<Parameter>::iterator it = std::begin(param_list);
  EXPECT_EQ(integer_type_class, it->get_classification());
  // error: no matching function for call to
  // ‘arg_classify::Parameter::get_outer_parameter()’ couldn’t deduce template
  // parameter ‘T'
  //  EXPECT_EQ(42, *(it->get_outer_parameter()));
  it++;
  // std::string
  EXPECT_EQ(record_type_class, it->get_classification());
  it++;
  // union
  EXPECT_EQ(union_type_class, it->get_classification());
  it++;
  // simple struct
  EXPECT_EQ(record_type_class, it->get_classification());
  it++;
  // struct with ctor
  EXPECT_EQ(record_type_class, it->get_classification());
}

TEST(ClassifierTest, Ints) {
  int val{1};
  std::cout << "Int: ";
  EXPECT_EQ(integer_type_class, _get_builtin_classification<int>(val, true));
  EXPECT_FALSE(folly_sdt_parameter_is_invalid(val));
  std::cout << "Pointer to int: ";
  EXPECT_EQ(pointer_type_class, _get_builtin_classification<int *>(&val, true));
  EXPECT_FALSE(folly_sdt_parameter_is_invalid(&val));
  std::cout << "Reference to int: ";

  // References to ints are classified as ints by libgcc.
  // Apparently libgcc's reference_type_class is unreachable from C++.
  int &ref{val};
  EXPECT_EQ(integer_type_class, _get_builtin_classification<int &>(ref, true));
  EXPECT_FALSE(folly_sdt_parameter_is_invalid(ref));
  std::cout << "Const reference to int: ";
  const int &ref2{val};
  EXPECT_EQ(integer_type_class,
            _get_builtin_classification<const int &>(ref2, true));
  EXPECT_FALSE(folly_sdt_parameter_is_invalid(ref2));

  const bool flag{false};
  EXPECT_EQ(boolean_type_class,
            _get_builtin_classification<const bool>(flag, true));
  EXPECT_FALSE(folly_sdt_parameter_is_invalid(flag));

  // Apparently there is no C++ type that libgcc will classify as
  // char_type_class.
  const char justone{'y'};
  EXPECT_EQ(integer_type_class,
            _get_builtin_classification<const char>(justone, true));
  EXPECT_FALSE(folly_sdt_parameter_is_invalid(justone));
  const wchar_t foreign{L'ß'};
  EXPECT_EQ(integer_type_class,
            _get_builtin_classification<const wchar_t>(foreign, true));
  EXPECT_FALSE(folly_sdt_parameter_is_invalid(foreign));
}

TEST(ClassifierTest, Numbers) {
  double val{3.14159};
  EXPECT_EQ(real_type_class, _get_builtin_classification<double>(val, true));
  EXPECT_FALSE(folly_sdt_parameter_is_invalid(val));
  std::complex<double> val2{val, val};
  EXPECT_EQ(record_type_class,
            _get_builtin_classification<std::complex<double>>(val2, true));
  EXPECT_TRUE(folly_sdt_parameter_is_invalid(val2));
}

// std::strings are record_type_class!
TEST(ClassifierTest, Strings) {
  // libgcc classifies std::string as record_type_class.
  // The 32 bytes for a std::string includes the 8-byte c_str() raw pointer plus
  // the allocator that is also a std::basic_string template parameter.
  const std::string obvious{"Hello, world!"};
  std::cout << "String: ";
  EXPECT_EQ(record_type_class,
            _get_builtin_classification<const std::string>(obvious, true));
  EXPECT_TRUE(folly_sdt_parameter_is_invalid(obvious));
  const std::string concat{obvious + obvious + obvious + obvious};
  std::cout << "Longer string: ";
  EXPECT_EQ(record_type_class,
            _get_builtin_classification<const std::string>(concat, true));
  EXPECT_TRUE(folly_sdt_parameter_is_invalid(concat));
  // c-strings are pointers to char: no surprise.
  std::cout << "C-string: ";
  EXPECT_EQ(pointer_type_class,
            _get_builtin_classification<const char *>(obvious.c_str(), true));
  EXPECT_FALSE(folly_sdt_parameter_is_invalid(obvious.c_str()));
  std::cout << "Longer C-string: ";
  EXPECT_EQ(pointer_type_class,
            _get_builtin_classification<const char *>(concat.c_str(), true));
  EXPECT_FALSE(folly_sdt_parameter_is_invalid(concat.c_str()));
  std::cout << "Const reference to string: ";
  const std::string &ref{obvious};
  EXPECT_EQ(record_type_class,
            _get_builtin_classification<const std::string &>(ref, true));
  EXPECT_TRUE(folly_sdt_parameter_is_invalid(ref));
}

TEST(ClassifierTest, Pointers) {
  // Size 16 includes the raw pointer and the pointer to the control block.
  // Obviously "records" can be any arbitrary number of 8-byte chunks.
  const std::shared_ptr<int> intp{new int(4096)};
  EXPECT_EQ(
      record_type_class,
      _get_builtin_classification<const std::shared_ptr<int>>(intp, true));
  EXPECT_TRUE(folly_sdt_parameter_is_invalid(intp));

  EXPECT_EQ(pointer_type_class,
            _get_builtin_classification<int *>(intp.get(), true));
  EXPECT_FALSE(folly_sdt_parameter_is_invalid(intp.get()));
}

/*
 * These examples don't compile, as multiple values are passed on the stack and
   must therefore be independently evaluated.  Presumably there is no way to
   libgcc's array_type_class.
TEST(ClassifierTest, Arrays) {
  const std::array<double,2> anarray {-5.0, -4.0, -3.0, -2.0};
  EXPECT_EQ(pointer_type_class,
            _get_builtin_classification<const std::array<double,2>&>(anarray,
true)); EXPECT_FALSE(folly_sdt_parameter_is_invalid(anarray)); const
std::pair<double,double> apair {-5.0, -4.0}; EXPECT_EQ(pointer_type_class,
            _get_builtin_classification<const std::pair<double,double>&>(apair,
true)); EXPECT_FALSE(folly_sdt_parameter_is_invalid(apair));
}
*/

// Unsurprisingly function_type_class and method_type_class appear to be
// unreachable from C++.
TEST(ClassifierTest, Functions) {
  auto adder = [](int a, int b) { return a + b; };
  EXPECT_EQ(
      record_type_class,
      _get_builtin_classification<std::function<int(int, int)>>(adder, true));
  EXPECT_TRUE(folly_sdt_parameter_is_invalid(adder));
}

// Unions are recognized by libgcc but other more complex objects are not.
TEST(ClassifierTest, Objects) {
  const std::chrono::duration<int> dur(5);
  EXPECT_EQ(record_type_class,
            _get_builtin_classification<std::chrono::duration<int>>(dur, true));
  // Apparently duration decays to a POD.
  EXPECT_FALSE(folly_sdt_parameter_is_invalid(dur));

  const union aunion_type aunion {};
  EXPECT_EQ(union_type_class,
            _get_builtin_classification<const union aunion_type>(aunion, true));
  EXPECT_FALSE(folly_sdt_parameter_is_invalid(aunion));

  const struct astruct_type astruct {};
  EXPECT_EQ(
      record_type_class,
      _get_builtin_classification<const struct astruct_type>(astruct, true));
  // A struct is a POD as long as it does not have a custom default constructor.
  EXPECT_FALSE(folly_sdt_parameter_is_invalid(astruct));

  const struct bstruct_type bstruct;
  EXPECT_TRUE(folly_sdt_parameter_is_invalid(bstruct));
}

TEST(ClassifierTest, TwoParameters) {
  // Ints are valid.
  int a{1}, b{2};
  EXPECT_TRUE(
      folly_sdt_parameters_are_all_valid(__FILE__, "operation_start", a, b));

  // Strings are invalid.
  const std::string obvious{"Hello, world!"},
      lessobvious{"regardless of inclination"};
  EXPECT_FALSE(folly_sdt_parameters_are_all_valid(__FILE__, "operation_start",
                                                  obvious, lessobvious));

  // Cannot provide a string parameter no matter what.
  EXPECT_FALSE(folly_sdt_parameters_are_all_valid(__FILE__, "operation_start",
                                                  b, obvious));

  // C-strings are okay.
  EXPECT_TRUE(folly_sdt_parameters_are_all_valid(__FILE__, "operation_start", b,
                                                 obvious.c_str()));
}

} // namespace local_testing
} // namespace arg_classify
