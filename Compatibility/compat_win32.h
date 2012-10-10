
#pragma warning(disable:4244) 
#pragma warning(disable:4305) 
#pragma warning(disable:4996) 
#pragma warning(disable:4018) 

#ifndef snprintf
#if defined(Q_WS_WIN) || defined(WIN32)
#define snprintf	_snprintf
#else
#define snprintf	snprintf
#endif
#endif

#ifndef INT64_C
#define INT64_C(c) (c ## LL)
#define UINT64_C(c) (c ## ULL)
#endif


#ifndef PRId64
#define PRId64 "I64d"
#endif /* PRId64 */

#ifndef PRIu64
#define PRIu64 "I64u"
#endif /* PRIu64 */

#ifndef PRIx64
#define PRIx64 "I64x"
#endif /* PRIx64 */

#ifndef PRIX64
#define PRIX64 "I64X"
#endif /* PRIX64 */

static inline long lrint(double f) { 
	return (long)f; 
} 

static inline long int lrintf(float x) {
	return (int)x;
}

static inline double log2(double x) {
	return x; // TODO
}
