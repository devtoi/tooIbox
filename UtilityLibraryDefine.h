#ifdef _WIN32
	#ifdef UTILITY_DLL_EXPORT
		#define UTILITY_API __declspec(dllexport)
	#else
		#define UTILITY_API __declspec(dllimport)
	#endif
#else
	#define UTILITY_API
#endif

#ifndef UTILITY_ALLOCATION_HEADER
#include "Alloc.h"
#else
#include UTILITY_ALLOCATION_HEADER
#endif
