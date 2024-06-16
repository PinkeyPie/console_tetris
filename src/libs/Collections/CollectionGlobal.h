#ifndef COLLECTION_GLOBAL_H
#define COLLECTION_GLOBAL_H

#if defined(COLLECTION_LIBRARY)
#ifdef _MSC_VER
#define COLLECTION_EXPORT __declspec(dllexport)
#else
#define COLLECTION_EXPORT
#endif
#else
#ifdef _MSC_VER
#define COLLECTION_EXPORT __declspec(dllimport)
#else
#define COLLECTION_EXPORT
#endif
#endif

#endif