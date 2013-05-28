//
//    meta.h
//    schwa::job01::core
//
//    Copyright (c) 2013 Schwaftwarez
//    Licence: Apache v2.0
//
//    Utilities for template metaprogramming.
//
//////////////////////////////////////////////////////////////////////////////


#ifndef __schwa__job01__core__meta__
#define __schwa__job01__core__meta__


// Add this as the last argument to a function that you want to
// be enabled for types whose size is within the specified range.
#define SCHWA_ONLY_IF_SIZE_BETWEEN(TYPE, LO, HI)   \
typename std::enable_if<(sizeof(TYPE) > LO) &&     \
                        (sizeof(TYPE) <= HI)>::type* dummy = nullptr


// Add this as the last argument to a function that you want to
// be enabled for only for types that are subclasses of the given base type.
#define SCHWA_USE_IF_SUBCLASS(PARAM_T, BASE_T) \
typename std::enable_if<std::is_base_of<BASE_T, PARAM_T>::value>::type* dummy = nullptr


#endif  // #ifndef __schwa__job01__core__meta__
