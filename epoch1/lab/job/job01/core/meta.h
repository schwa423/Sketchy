//
//    meta.h
//    schwa::job01::core
//
//    Copyright (c) 2013 Schwaftwarez
//    Licence: Apache v2.0
//
//    Utilities for template metaprogramming.
//
//
//////////////////////////////////////////////////////////////////////////////


#ifndef __schwa__job01__core__meta__
#define __schwa__job01__core__meta__


// The first argument is used as a template parameter only if it is
// a subclass of the second.  Otherwise, the template is not matched.
#define SCHWA_USE_IF_SUBCLASS(PARAM_T, BASE_T) \
typename PARAM_T,                              \
class = typename std::enable_if<std::is_base_of<BASE_T, PARAM_T>::value>::type


#endif  // #ifndef __schwa__job01__core__meta__
