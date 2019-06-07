#pragma once
#include <string>
#include <atomic>

/** @file atomic_string.h
	Header which includes a class for implementing an atomic 
	specialization for string.
 ************************************************************************/

namespace std {

/** This is a class implements an atomic specialization for strings
    @brief atomic strings
************************************************************************/
template<typename stringT>
class atomic_string
{
public:
	/// Default constructor
	atomic_string() { flag._My_flag = 0; }
	/// Constructor from data 
	atomic_string (const stringT& s) : data (s) { flag._My_flag = 0; }

	/// Assignment operator on basic type
	stringT operator= (const stringT& right);
	/// Not lock free
	bool is_lock_free() const {	return false; }
	/// Store
	void store (stringT value,
		memory_order order = memory_order_seq_cst);
	/// Load
	stringT load (memory_order order = memory_order_seq_cst) const;
	/// Convert to string
	operator stringT() const { return load(); }
	/// Exchange
	stringT exchange(const stringT& value, 
		memory_order order = memory_order_seq_cst); 
	/// Compare exchange 
	bool compare_exchange_weak (stringT& exp, const stringT& value,
		memory_order order1, memory_order order2);
	/// Compare exchange 
	bool compare_exchange_weak(stringT& exp, const stringT& value,
		memory_order order = memory_order_seq_cst) {
			return compare_exchange_weak (exp, value, order, order); }
	/// Compare exchange 
	bool compare_exchange_strong(stringT& exp, const stringT& value,
		memory_order order1, memory_order order2) {
			return compare_exchange_weak (exp, value, order1, order2); }
	/// Compare exchange 
	bool compare_exchange_strong(stringT& exp, const stringT& value,
		memory_order order = memory_order_seq_cst) {
			return compare_exchange_weak (exp, value, order, order); }

protected:
	/// flag for spin lock
	mutable atomic_flag	flag;
	/// data string
	stringT					data;

private:
	/// Copy constructor not defined
	atomic_string (const atomic_string&);
	/// Assignment operator not defined
	atomic_string& operator= (const atomic_string&);
};

/** This is a class implements an atomic specialization for std::string
    @brief atomic<string>
************************************************************************/
template<>
class atomic<string> : public atomic_string<string>
{
public:
	/// Default constructor
	atomic() {}
	/// Constructor from string
	atomic (const string& s) : atomic_string (s) {}
};

/** This is a class implements an atomic specialization for std::wstring
    @brief atomic<wstring>
************************************************************************/
template<>
class atomic<wstring> : public atomic_string<wstring>
{
public:
	/// Default constructor
	atomic() {}
	/// Constructor from string
	atomic (const wstring& s) : atomic_string (s) {}
};


/// Assignment operator
template <typename stringT>
stringT atomic_string<stringT>::operator= (const stringT& right) 
{
	while (flag.test_and_set()) {}
	stringT ret = data;
	data = right;
	flag.clear();
	return data;
}

// Store
template <typename stringT>
void atomic_string<stringT>::store (stringT value, memory_order order) 
{
	while (flag.test_and_set()) {}
	data = value;
	flag.clear();
}
/// Load
template <typename stringT>
stringT atomic_string<stringT>::load (memory_order order) const
{
	while (flag.test_and_set()) {}
	stringT ret = data;
	flag.clear();
	return ret;
}

// Exchange
template <typename stringT>
stringT atomic_string<stringT>::exchange(const stringT& value, memory_order order) 
{
	while (flag.test_and_set()) {}
	stringT ret = data;
	data = value;
	flag.clear();
	return ret;
}

// Compare exchange 
template <typename stringT>
bool atomic_string<stringT>::compare_exchange_weak (stringT& exp, 
	const stringT& value, memory_order order1, memory_order order2) 
{
	while (flag.test_and_set()) {}
	if (data == exp) {
		data = value;
		flag.clear();
		return true;
	}
	else {
		exp = data;
		flag.clear();
		return false;
	}
}

}
