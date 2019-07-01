#pragma once
#include <string>
#include <ctype.h>

/** @file stringcase.h
	Functions and classes for a case-insensitive string.
 ************************************************************************/

namespace std {

/** @defgroup stringcase Utility functions and classes
 ************************************************************************/
/** @{ */

/** Case insensitive compare.
	@param s1 first string
	@param s1 second string
	@return <0 smaller, 0 equal, >0 greater
    @brief case insensitive compare with maximum length
 ************************************************************************/
inline
int strncasecmp (const char *s1, const char *s2, int n) {
	return _strnicmp (s1, s2, n); }

/** Case insensitive compare for unicode string.
	@param s1 first string
	@param s1 second string
	@return <0 smaller, 0 equal, >0 greater
    @brief case insensitive unicode compare with maximum length
 ************************************************************************/
inline
int wcsncasewcmp (const wchar_t *s1, const wchar_t *s2, int n) {
	return _wcsnicmp (s1, s2, n); }

/** This traits class is not case sensitive.
    @brief case insensitive traits.
 ************************************************************************/
struct case_char_traits : public std::char_traits<char> 
{
	static bool eq (const char_type& c1, const char_type& c2) { 
		return tolower (c1) == tolower (c2); 
	}
	static bool ne (const char_type& c1, const char_type& c2) { 
		return !(tolower (c1) == tolower (c2));
	}
	static bool lt (const char_type& c1, const char_type& c2) { 
		return tolower (c1) < tolower (c2);
	}
	static int compare (const char_type* s1, const char_type* s2, size_t n) { 
		return strncasecmp (s1, s2, n); 
	}
};

/** This unicode traits class is not case sensitive.
    @brief case insensitive unicode traits.
 ************************************************************************/
struct case_wchar_traits : public std::char_traits<wchar_t> 
{
	static bool eq (const char_type& c1, const char_type& c2) { 
		return towlower (c1) == towlower (c2); 
	}
	static bool ne (const char_type& c1, const char_type& c2) { 
		return !(towlower (c1) == towlower (c2));
	}
	static bool lt (const char_type& c1, const char_type& c2) { 
		return towlower (c1) < towlower (c2);
	}
	static int compare (const char_type* s1, const char_type* s2, size_t n) { 
		return wcsncasewcmp (s1, s2, n); 
	}
};

/** This string class is not case sensitive.
    @brief case insensitive string.
 ************************************************************************/
typedef std::basic_string <char, case_char_traits> stringcase;

/** This string class is not case sensitive.
    @brief case insensitive string.
 ************************************************************************/
typedef std::basic_string <wchar_t, case_wchar_traits> wstringcase;

/** trim space on both ends.
	@param s string to trim
 ************************************************************************/
void trim_space (stringcase& s);

/** trim space on both ends.
	@param s string to trim
 ************************************************************************/
void trim_space (wstringcase& s);

/** Splits a string into its tokens and adds them to a container.
	The delimiter can be easily specified with a lambda expression.
	Example:
		stringcase arg ("This is a test!");
		vector<stringcase> list;
		split_string (list, arg, [] (char c)->bool { 
			return isspace (c) != 0; }, true);
	@param output Output container
	@param input Input string
	@param pred Function which returns true when character is a separator
	@param trimEmpty Trims empty strings when true
	@brief Splits a strings
 ************************************************************************/
template <class Container, class String, class Predicate>
void split_string (Container& output, const String& input,
				   const Predicate& pred, bool trimEmpty = true)
{
    auto i = begin (input);
    auto iLast = i;
    while (i = find_if (i, end (input), pred), i != end (input)) {
        if (!(trimEmpty && i == iLast)) {
            output.emplace_back (iLast, i);
        }
        ++i;
        iLast = i;
    }
	if (i != iLast) {
		output.emplace_back (iLast, i);
    }
}

}

#include "stringcase_hash.h"
