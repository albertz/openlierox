/*
	On Ubuntu, the macro _FORTIFY_SOURCE is defined which tries to gives some
	usefull warnings about obvious mistakes.
 
	Ubuntu has bad libc headers though, which makes things messed up:
	fread/fwrite are marked with warn_unused_result.

	According to GCC docs, "the warn_unused_result attribute causes a warning to be
	emitted if a caller of the function with this attribute does not use its return
	value. This is useful for functions where not checking the result is either a
	security problem or always a bug". This is not the case for fread/fwrite;
	we have many usage of fread/fwrite where checking the return value is not needed.

	This header has a work around for this problem.
*/

#include <cstdio>

inline size_t fread_WUR_HACK(void *ptr, size_t size, size_t nmemb, FILE *stream) {
	return fread(ptr, size, nmemb, stream);
}

inline size_t fwrite_WUR_HACK(const void *ptr, size_t size, size_t nmemb, FILE *stream) {
	return fwrite(ptr, size, nmemb, stream);
}

#define fread fread_WUR_HACK
#define fwrite fwrite_WUR_HACK

