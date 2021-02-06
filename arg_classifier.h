// The goal of the program is to discover what type_class GCC returns for
// various C++ objects such as std::string in order to understand why FOLLY_SDT
// fails to produce JIT-compilable code when complex objects are passed as
// arguments.
#ifndef ARG_CLASSIFIER_H
#define ARG_CLASSIFIER_H

#include "folly/tracing/StaticTracepoint.h"
#include "gcc/typeclass.h"

#include <cxxabi.h>
#include <iostream>
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

// Determine if the BPF JIT will accept a Folly userspace static tracepoint with
// the provided data type.  Rely on the compiler's __builtin_classify_type()
// since Facebook's library does, at least for x86_64.
template <typename T> constexpr bool folly_sdt_parameter_is_invalid(T x) {
  return ((no_type_class == _get_builtin_classification(x)) ||
          (record_type_class == _get_builtin_classification(x)) ||
          (opaque_type_class == _get_builtin_classification(x)));
}

// https://eli.thegreenplace.net/2014/variadic-templates-in-c/
template <typename T, typename... Pars>
constexpr bool folly_sdt_parameters_are_all_valid(const char *, const char *,
                                                  T first, Pars... pars) {
  return (!folly_sdt_parameter_is_invalid(first)) &&
         (!folly_sdt_parameter_is_invalid(pars...));
}

template <typename T, typename... Pars>
bool maybe_insert_folly_sdt_probe(const char *progname, const char *probename,
                                  T first, Pars... pars) {
  if (arg_classify::folly_sdt_parameters_are_all_valid(progname, probename,
                                                       first, pars...)) {
    FOLLY_SDT(progname, probename, first, pars);
    return true;
  } else {
    std::cerr << "Probe is not JIT-compilable.";
    return false;
  }
}

// StaticTracepoint-ELFx86.h:
// Structure of note section for the probe.
// #define FOLLY_SDT_NOTE_CONTENT(provider, name, has_semaphore, arg_template)
//  FOLLY_SDT_ASM_STRING(provider)
//  FOLLY_SDT_ASM_STRING(name)
// First two arguments to FOLLY_SDT() macro therefore must be strings.

} // namespace arg_classify

#endif
