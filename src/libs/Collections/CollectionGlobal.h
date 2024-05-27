#ifndef COLLECTION_GLOBAL_H
#define COLLECTION_GLOBAL_H

#if defined(COLLECTION_LIBRARY)
#define COLLECTION_EXPORT __declspec(dllexport)
#else
#define COLLECTION_EXPORT __declspec(dllimport)
#endif

#endif