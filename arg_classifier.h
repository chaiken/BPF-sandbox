// The goal of the program is to discover what type_class GCC returns for
// various C++ objects such as std::string in order to understand why FOLLY_SDT
// fails to produce JIT-compilable code when complex objects are passed as
// arguments.
#ifndef ARG_CLASSIFIER_H
#define ARG_CLASSIFIER_H

#include "gcc/typeclass.h"

#include <cstdarg>
#include <iostream>

#include <cxxabi.h>
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

  std::cout << x << " of type " << unmangled << " of size " << sizeof(x)
            << " is in " << enum_name << std::endl
            << std::endl;
  free(unmangled);
}

//  Following
//  https://gcc.gnu.org/onlinedocs/libstdc++/manual/ext_demangling.html
template <typename T> int get_builtin_classification(T x) {
  int type_class = __builtin_classify_type(x);
#ifdef DEBUG
  pretty_print_info(x, type_class);
#endif
  return type_class;
}

} // namespace arg_classify

#endif
