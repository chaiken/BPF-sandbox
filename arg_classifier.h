// The goal of the program is to discover what type_class GCC returns for
// various C++ objects such as std::string in order to understand why FOLLY_SDT
// fails to produce JIT-compilable code when complex objects are passed as
// arguments.
#ifndef ARG_CLASSIFIER_H
#define ARG_CLASSIFIER_H

#include "gcc/typeclass.h"

#include <cxxabi.h>
#include <iostream>
#include <memory>
#include <type_traits>
#include <typeinfo>

namespace arg_classify {

const std::string type_names[]{
    "void_type_class",      "integer_type_class",  "char_type_class",
    "enumeral_type_class",  "boolean_type_class",  "pointer_type_class",
    "reference_type_class", "offset_type_class",   "real_type_class",
    "complex_type_class",   "function_type_class", "method_type_class",
    "record_type_class",    "union_type_class",    "array_type_class",
    "string_type_class",    "lang_type_class",     "opaque_type_class"};

template <typename T> void pretty_print_info(T x, int type) {
  // If @a __output_buffer is not long enough, it is expanded using realloc.
  char *unmangled = abi::__cxa_demangle(typeid(x).name(), 0, 0, 0);

  std::string enum_name =
      (-1 == type) ? "no_type_class" : type_names[__builtin_classify_type(x)];

  std::cout << "Type " << unmangled << " of size " << sizeof(x) << " is in "
            << enum_name << std::endl
            << std::endl;
  free(unmangled);
}

//  Following
//  https://gcc.gnu.org/onlinedocs/libstdc++/manual/ext_demangling.html
template <typename T>
constexpr int _get_builtin_classification(T x, bool verbose = false) {
  int type_class = __builtin_classify_type(x);
  if (verbose) {
    pretty_print_info(x, type_class);
  }
  return type_class;
}

// https://www.cplusplus.com/articles/oz18T05o/
class Parameter {
public:
  template <typename T>
  Parameter(const T &par) : outer_parameter(new ParameterModel<T>(par)) {}
  int get_classification() const {
    return outer_parameter->get_classification();
  }
  // clang-format off
  // error: use of ‘auto arg_classify::Parameter::ParameterConcept::get_inner_parameter() const’
  // before deduction of ‘auto’
  // 62 |     return  outer_parameter->get_inner_parameter();
  // auto get_outer_parameter() const {
  // clang-format on
  //
  // Compiles, but how to make template type deduction work from actual code?
  template <typename T> T get_outer_parameter() const {
    return outer_parameter->get_inner_parameter();
  }

private:
  // ParameterConcept and ParameterModel are private members of Parameter.
  struct ParameterConcept {
    virtual ~ParameterConcept() {}
    virtual int get_classification() const = 0;
    auto get_inner_parameter() const;
  };

  template <typename T> struct ParameterModel : ParameterConcept {
    ParameterModel(const T &t) : inner_parameter(t) {}
    virtual ~ParameterModel() {}
    // There is no reason for this function to be virtual.
    int get_classification() const {
      return _get_builtin_classification<T>(inner_parameter);
    }
    T get_inner_parameter() const { return inner_parameter; }

  private:
    T inner_parameter;
  };
  // Not boost::shared_ptr as at cplusplus.com.
  std::shared_ptr<ParameterConcept> outer_parameter;
};

// Determine if the BPF JIT will accept a Folly userspace static tracepoint with
// the provided data type.  Formerly relied on GCC's
// __builtin_classify_type() since Facebook's library does, at least for x86_64.
// However, in C++, std::is_pod() produces the same result.
template <typename T> constexpr bool folly_sdt_parameter_is_invalid(T /*x*/) {
  return ((!std::is_pod<T>::value) && (!std::is_array<T>::value));
}

// https://eli.thegreenplace.net/2014/variadic-templates-in-c/
template <typename T, typename... Pars>
constexpr bool folly_sdt_parameters_are_all_valid(const char *, const char *,
                                                  T first, Pars... pars) {
  return (!folly_sdt_parameter_is_invalid(first)) &&
         (!folly_sdt_parameter_is_invalid(pars...));
}

// StaticTracepoint-ELFx86.h:
// Structure of note section for the probe.
// #define FOLLY_SDT_NOTE_CONTENT(provider, name, has_semaphore, arg_template)
//  FOLLY_SDT_ASM_STRING(provider)
//  FOLLY_SDT_ASM_STRING(name)
// First two arguments to FOLLY_SDT() macro therefore must be strings.

} // namespace arg_classify

#endif
