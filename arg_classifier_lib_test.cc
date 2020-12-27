#include "arg_classifier.h"

#include "gcc/typeclass.h"
#include "gtest/gtest.h"

using namespace std;

namespace arg_classify {
namespace local_testing {

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
  int &ref{val};
  EXPECT_EQ(integer_type_class, _get_builtin_classification<int &>(ref, true));
  EXPECT_FALSE(folly_sdt_parameter_is_invalid(ref));
  std::cout << "Const reference to int: ";
  const int &ref2{val};
  EXPECT_EQ(integer_type_class,
            _get_builtin_classification<const int &>(ref2, true));
  EXPECT_FALSE(folly_sdt_parameter_is_invalid(ref2));
}

// std::strings are record_type_class!
TEST(ClassifierTest, Strings) {
  // libgcc fails to classify standard strings.
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

} // namespace local_testing
} // namespace arg_classify
