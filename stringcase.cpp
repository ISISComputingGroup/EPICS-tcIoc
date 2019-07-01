#include "stringcase.h"

/** @file stringcase.cpp
	Methods for case insensitve strings.
 ************************************************************************/

namespace std {

/* This string comparison function is not case sensitive.
 ************************************************************************/
int _strncasecmp (const char *s1, const char *s2, int n) 
{
	int d, i = 0;
	if (n <= 0) {
		return 0;
	}
	while (s1[i] || s2[i]) {
		if ((d = tolower (s1[i]) - tolower (s2[i])))
			return d;
		if (++i == n)
			break;
	}
	return 0;
}

/* This unicode string comparison function is not case sensitive.
 ************************************************************************/
int _wcsncasewcmp (const wchar_t *s1, const wchar_t *s2, int n) 
{
	int d, i = 0;
	if (n <= 0) {
		return 0;
	}
	while (s1[i] || s2[i]) {
		if ((d = towlower (s1[i]) - towlower (s2[i])))
			return d;
		if (++i == n)
			break;
	}
	return 0;
}

/* trim space on both ends.
 ************************************************************************/
void trim_space (stringcase& s)
{
	while (!s.empty() && isspace ((unsigned char)*s.begin())) s.erase (0, 1);
	while (!s.empty() && isspace ((unsigned char)*s.rbegin())) s.erase (s.size()-1, 1);
}

/* trim space on both ends.
 ************************************************************************/
void trim_space (wstringcase& s)
{
	while (!s.empty() && iswspace (*s.begin())) s.erase (0, 1);
	while (!s.empty() && iswspace (*s.rbegin())) s.erase (s.size()-1, 1);
}


#ifdef _WIN64
// FNV prime for 64 bits
static const size_t FNV_64_PRIME = (size_t)0x100000001B3;
// FNV start value for 64 bits
static const size_t FNV_64_INIT = (size_t)14695981039346656037;
#else 
// FNV prime for 32 bits
static const size_t FNV_32_PRIME = (size_t)0x01000193;
// FNV start value for 32 bits
static const size_t FNV1_32_INIT = (size_t)1099511628211;
#endif

// Perform a 32 or 64 bit Fowler/Noll/Vo hash on a case insensitive string
std::size_t std::hash<stringcase>::operator()(const stringcase& str) const 
{
	const unsigned char* s = (unsigned char*)str.c_str();
#ifdef _WIN64
	size_t hval = FNV1_64_INIT;
	while (*s) {
		hval *= FNV_64_PRIME;
		hval ^= (size_t)tolower(*s++);
	}
#else 
	size_t hval = FNV1_32_INIT;
	while (*s) {
		hval *= FNV_32_PRIME;
		hval ^= (size_t)tolower(*s++);
	}
#endif;
	return hval;
}

// Perform a 32/64 bit Fowler/Noll/Vo hash on a case insensitive 
// unicode string
std::size_t std::hash<wstringcase>::operator()(const wstringcase& str) const
{
	const wchar_t* s = str.c_str();
	wchar_t c;
#ifdef _WIN64
	size_t hval = FNV1_64_INIT;
	while (*s) {
		c = towlower (*s++);
		hval *= FNV_64_PRIME;
		hval ^= (size_t)(c & 0cFF);
		hval *= FNV_64_PRIME;
		hval ^= (size_t)(c >> 8);
	}
#else 
	size_t hval = FNV1_32_INIT;
	while (*s) {
		c = towlower (*s++);
		hval *= FNV_32_PRIME;
		hval ^= (size_t)(c & 0xFF);
		hval *= FNV_32_PRIME;
		hval ^= (size_t)(c >> 8);
	}
#endif;
	return hval;
}

}