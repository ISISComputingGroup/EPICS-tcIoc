/** @file stringcase_hash.h
	Specialization for case sensitive strings.
 ************************************************************************/
namespace std {


/** This is a function specialization for case sensitive strings.
	Perform a 32/64 bit Fowler/Noll/Vo hash on a string.
    @brief hash for case insensitive string.
 ************************************************************************/
template<>
struct std::hash<std::stringcase>
{
public:
	/// Perform a 32 or 64 bit Fowler/Noll/Vo hash on a string
	/// Adapted from fnv_32_str and fnv_64_str
	/// http://www.isthe.com/chongo/tech/comp/fnv/index.html#FNV-source
	/// @param str string to hash
	/// @return 32/64 bit hash
	std::size_t operator()(const stringcase& str) const noexcept;
};

/** This is a function specialization for case sensitive unicode strings.
	Perform a 32/64 bit Fowler/Noll/Vo hash on a unicode string.
    @brief hash for case insensitive unicode string.
 ************************************************************************/
template<>
struct std::hash<std::wstringcase>
{
public:
	/// Perform a 32 or 64 bit Fowler/Noll/Vo hash on a string
	/// Adapted from fnv_32_str
	/// http://www.isthe.com/chongo/tech/comp/fnv/index.html#FNV-source
	/// @param str string to hash
	/// @return 32/64 bit hash
	std::size_t operator()(const wstringcase& str) const noexcept;
};


}
