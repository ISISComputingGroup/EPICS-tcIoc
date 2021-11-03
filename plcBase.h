#pragma once
#include "stdafx.h"
#include <thread>
#include "atomic_string.h"

/** @file plcBase.h
	Header which includes abstract base classes for defining an internal 
	record entry for the IOC.
 ************************************************************************/

struct dbCommon;

/** @namespace plc
	PLC namespace 
	@brief Namespace for abstract plc functionality
 ************************************************************************/
namespace plc {

class BaseRecord;
class BasePLC;


/** This is a smart pointer to a PLC 
    @brief Smart pointer to PLC
************************************************************************/
typedef std::shared_ptr<BasePLC> BasePLCPtr;


/** This is a base class for an abstract interface to access the PLC 
    (slave) or the user side (master).
    @brief Abstract interface
************************************************************************/
class Interface
{
public:
	/// Constructor
	/// @param dval Reference to a tag/channel record
	explicit Interface (BaseRecord& dval) : record (dval) {}
	/// Desctructor
	virtual ~Interface() {};

	/// Return a pointer to the tag/channel record
	BaseRecord& get_record() { return record; };
	/// Return a pointer to the tag/channel record
	const BaseRecord& get_record() const { return record; };

	/// Pure virtual method indicating that the value needs to be pushed
	virtual bool push() = 0;
	/// Pure virtual method indicating that the value needs to be pulled
	virtual bool pull() = 0;

	/// Get parent PLC that owns this record
	BasePLC* get_parent();
	/// Get parent PLC that owns this record
	const BasePLC* get_parent() const;
	/// Print values to file
	/// @param fp File pointer
	virtual void printVal (FILE* fp) {}
	/// Get symbol name
	virtual const char* get_symbol_name() const { return nullptr; }
protected:
	/// Pointer to tag/channel record associated with this interface
	BaseRecord&			record;
};

/** This is a smart pointer for Interface
    @brief Smart pointer to interface
 ************************************************************************/
typedef std::unique_ptr<Interface> InterfacePtr;

/** This is an enumerated type listing all the available data types
    @brief Data type enumeration
 ************************************************************************/
enum class data_type_enum 
{
	/// Invalid data type
	dtInvalid,
	/// Boolean
	dtBool,
	/// 1-byte integer
	dtInt8,
	/// 1-byte unsigned integer
	dtUInt8,
	/// 2-byte integer
	dtInt16,
	/// 2-byte unsigned integer
	dtUInt16,
	/// 4-byte integer
	dtInt32,
	/// 4-byte unsigned integer
	dtUInt32,
	/// 8-byte integer
	dtInt64,
	/// 8-byte unsigned integer
	dtUInt64,
	/// 4-byte single precision floating point
	dtFloat,
	/// 8-byte double precision floating point
	dtDouble,
	/// string class
	dtString,
	/// wstring class
	dtWString,
	/// binary object
	dtBinary
};

/** Traits class for data value
    @brief Data value traits
 ************************************************************************/
template <typename T>
struct DataValueTraits
{
	/// size type
	typedef size_t size_type;
	/// enumerated type for data type
	typedef plc::data_type_enum data_type_enum;
	/// traits type
	typedef typename T traits_type;
	/// atomic variable type
	typedef typename std::atomic<T> traits_atomic;
	/// data type enumertaion value
	static const data_type_enum data_enum;
};

/** Type definitions for data value
    @brief Collection of type definitions
 ************************************************************************/
struct DataValueTypeDef
{
	/// size type
	typedef size_t size_type;
	/// enumerated type for data type
	typedef plc::data_type_enum data_type_enum;

	/// bool type
	typedef bool type_bool;
	/// 1-byte integer type
	typedef signed char type_int8;
	/// 1-byte unsigned integer type
	typedef unsigned char type_uint8;
	/// 3-byte integer type
	typedef short type_int16;
	/// 3-byte unsigned integer type
	typedef unsigned short type_uint16;
	/// 4-byte integer type
	typedef int type_int32;
	/// 4-byte unsigned integer type
	typedef unsigned int type_uint32;
	/// 8-byte integer type
	typedef long long type_int64;
	/// 8-byte unsigned integer type
	typedef unsigned long long type_uint64;
	/// 4-byte single precision floating point type
	typedef float type_float;
	/// 4-byte double precision floating point type
	typedef double type_double;
	/// string type
	typedef std::string type_string;
	/// wstring type
	typedef std::wstring type_wstring;
	/// binary type
	typedef void* type_binary;
	/// string character type
	typedef std::string::value_type type_string_value;
	/// wstring character type
	typedef std::wstring::value_type type_wstring_value;

	/// memory order used for atomic access
	static const std::memory_order memory_order = std::memory_order_seq_cst;
	/// atomic bool type
	typedef DataValueTraits<type_bool>::traits_atomic atomic_bool;
	/// atomic 1-byte integer type
	typedef DataValueTraits<type_int8>::traits_atomic atomic_int8;
	/// atomic 1-byte unsigned integer type
	typedef DataValueTraits<type_uint8>::traits_atomic atomic_uint8;
	/// atomic 2-byte integer type
	typedef DataValueTraits<type_int16>::traits_atomic atomic_int16;
	/// atomic 2-byte unsigned integer type
	typedef DataValueTraits<type_uint16>::traits_atomic atomic_uint16;
	/// atomic 4-byte integer type
	typedef DataValueTraits<type_int32>::traits_atomic atomic_int32;
	/// atomic 4-byte unsigned integer type
	typedef DataValueTraits<type_uint32>::traits_atomic atomic_uint32;
	/// atomic 8-byte integer type
	typedef DataValueTraits<type_int64>::traits_atomic atomic_int64;
	/// atomic 8-byte unsigned integer type
	typedef DataValueTraits<type_uint64>::traits_atomic atomic_uint64;
	/// atomic 4-byte single precision floating point type
	typedef DataValueTraits<type_float>::traits_atomic atomic_float;
	/// atomic 8-byte double precision floating point type
	typedef DataValueTraits<type_double>::traits_atomic atomic_double;
	/// atomic string type
	typedef DataValueTraits<type_string>::traits_atomic atomic_string;
	/// atomic wstring type
	typedef DataValueTraits<type_wstring>::traits_atomic atomic_wstring;
	/// atomic binary type
	typedef DataValueTraits<type_binary>::traits_atomic atomic_binary;

	/// Define timestamp type
	typedef DataValueTypeDef::type_uint64 time_type;
};

/** Class for data value
    This class stores a data value and provides synchronization 
	between the user (slave) and the plc (master) interfaces. 
	When the plc writes a value, it is marked dirty on the user side.
	Then, when the user reads this data, it resets the dirty flag.
	The same logic applies for writes by the user and reads by the plc.

	Data access is guaranteed to be atomic and MT safe. Construction,
	initialization and destruction is not MT safe and all data access
	has to be stopped during these operations. 

	Type conversion is provided between all simple data types. However,
	the loss of information is not checked upon a down cast. Strings have
	to be read as strings. Binary data needs to be acessed with the 
	binary read/write operations. However, all data can be accessed 
	through binary access.

    @brief Data value
 ************************************************************************/
class DataValue : public DataValueTypeDef
{
public:
	/// Internally used storage pointer type
	typedef void* data_type;

	/// Default constructor
	DataValue() : mydata (nullptr), mytype (data_type_enum::dtInvalid), mysize (0),
		myvalid (false), myuserdirty (false), myplcdirty (false) {}
	/// Constructor
	/// @param rt Data type enumeration value
	/// @param len Length of data
	explicit DataValue (data_type_enum rt, size_type len = 0) 
		: mydata (nullptr), mytype (data_type_enum::dtInvalid), mysize (0),
		myvalid (false), myuserdirty (false), myplcdirty (false) { 
		Init(rt, len); }
	/// Desctructor
	~DataValue();
	/// Copy constructor
	DataValue (const DataValue&);
	/// Assignment operator
	DataValue& operator= (const DataValue&);

	/// Initializes data value
	/// @param rt Data type enumeration value
	/// @param len Length of data (use only for binary)
	void Init (data_type_enum rt, size_type len = 0);
	/// is valid
	bool IsValid () const { 
		return (mydata && (mytype != data_type_enum::dtInvalid) && (mysize > 0) && myvalid); };
	/// get type
	data_type_enum get_data_type() const { return mytype; }
	/// get size
	size_type get_size() const { return mysize; }

	/// Read data by the user
	/// @param data Data value reference (return)
	template <typename T> bool UserRead (T& data) const {
		return Read (myuserdirty, data); }
	/// Read fixed length character array data by the user
	/// @param data Data value reference for a fixed length character array (return)
	template <size_type N> bool UserRead (type_string_value (& data)[N]) const {
		return ReadBinary (myuserdirty, &data, N) > 0; }
	/// Read character array (pchar) by the user
	/// @param data Destination buffer
	/// @param max Maximum length
	bool UserRead (type_string_value* data, size_type max) const {
		return Read (myuserdirty, data, max); }
	/// Read character array (pwchar) by the user
	/// @param data Destination buffer
	/// @param max Maximum length
	bool UserRead (type_wstring_value* data, size_type max) const {
		return Read (myuserdirty, data, max); }
	/// Write data by the user
	/// @param data Data value reference
	template <typename T> bool UserWrite (const T& data) {
		return Write (myplcdirty, myuserdirty, data); }
	/// Write fixed length character array data by the user
	/// @param data Data value reference
	template <size_type N> bool UserWrite (const type_string_value (& data)[N]) {
		return WriteBinary (myplcdirty, myuserdirty, &data, N) > 0; }

	/// Write character array (pchar) by the user
	/// @param data Source buffer
	/// @param max Maximum length
	bool UserWrite (const type_string_value* data, size_type max) {
		return Write (myplcdirty, myuserdirty, data, max); }
	/// Write character array (wpchar) by the user
	/// @param data Source buffer
	/// @param max Maximum length
	bool UserWrite (const type_wstring_value* data, size_type max) {
		return Write (myplcdirty, myuserdirty, data, max); }

	/// Read data as binary by the user
	/// @param p value pointer (destination buffer)
	/// @param len Length in bytes
	size_type UserReadBinary (type_binary p, size_type len) const {
		return ReadBinary (myuserdirty, p, len); }
	/// Write data as binary by the user
	/// @param p value pointer (source buffer)
	/// @param len Length in bytes
	size_type UserWriteBinary (const type_binary p, size_type len) {
		return WriteBinary (myplcdirty, myuserdirty, p, len); }
	/// New data for user
	bool UserIsDirty() const { return myuserdirty; }
	/// Set dirty flag for user
	void UserSetDirty() { myuserdirty.store (true); }

	/// Set the valid flag and set the dirty flag when flag changes
	/// @param valid True for valid data, False for invalid
	void UserSetValid (bool valid) { SetValid (myuserdirty, valid); }
	/// Get the valid flag and reset the dirty flag
	/// @return valid True for valid data, False for invalid
	bool UserGetValid() const { return GetValid (myplcdirty); }

	/// Read data by the plc
	/// @param data Data value reference (return)
	template <typename T> bool PlcRead (T& data) const {
		return Read (myplcdirty, data); }
	/// Read fixed length character array data by the plc
	/// @param data Data value reference for a fixed length character array (return)
	template <size_type N> bool PlcRead (type_string_value (& data)[N]) const {
		return ReadBinary (myplcdirty, &data, N) > 0; }
	/// Read character array (pchar) by the plc
	/// @param data Destination buffer
	/// @param max Maximum length
	bool PlcRead (type_string_value* data, size_type max) const {
		return Read (myplcdirty, data, max); }
	/// Read character array (pwchar) by the plc
	/// @param data Destination buffer
	/// @param max Maximum length
	bool PlcRead (type_wstring_value* data, size_type max) const {
		return Read (myplcdirty, data, max); }
	/// Write data by the plc
	/// @param data Data value reference
	template <typename T> bool PlcWrite (const T& data) {
		return Write (myuserdirty, myplcdirty, data); }
	/// Write fixed length character array data by the plc
	/// @param data Data value reference
	template <size_type N> bool PlcWrite (const type_string_value (& data)[N]) {
		return WriteBinary (myuserdirty, myplcdirty, &data, N) > 0; }
	/// Write character array (pchar) by the plc
	/// @param data Source buffer
	/// @param max Maximum length
	bool PlcWrite (const type_string_value* data, size_type max) {
		return Write (myuserdirty, myplcdirty, data, max); }
	/// Write character array (wpchar) by the plc
	/// @param data Source buffer
	/// @param max Maximum length
	bool PlcWrite (const type_wstring_value* data, size_type max) {
		return Write (myuserdirty, myplcdirty, data, max); }

	/// Read data as binary by the plc
	/// @param p value pointer (destination buffer)
	/// @param len Length in bytes
	size_type PlcReadBinary (type_binary p, size_type len) const {
		return ReadBinary (myplcdirty, p, len); }
	/// Write data as binary by the plc
	/// @param p value pointer (source buffer)
	/// @param len Length in bytes
	size_type PlcWriteBinary (const type_binary p, size_type len) {
		return WriteBinary (myuserdirty, myplcdirty, p, len); }
	/// New data for plc
	bool PlcIsDirty() const {return myplcdirty; }
	/// Set dirty flag for plc
	void PlcSetDirty() { myplcdirty.store (true); }

	/// Set the valid flag and set the dirty flag when flag changes
	/// @param valid True for valid data, False for invalid
	void PlcSetValid (bool valid) { SetValid (myplcdirty, valid); }
	/// Get the valid flag and reset the dirty flag
	/// @return valid True for valid data, False for invalid
	bool PlcGetValid() const { return GetValid (myuserdirty); }

protected:
	/// Constructor (hidden)
	DataValue (const DataValue&&);
	/// Assignment operator
	DataValue& operator= (const DataValue&&);

	/// Read data
	/// @param dirty Reference to dirty flag (user or plc)
	/// @param data Data value reference (return)
	template <typename T> bool Read (atomic_bool& dirty, T& data) const;
	/// Read string (template specialization)
	/// @param dirty Reference to dirty flag (user or plc)
	/// @param data Data value reference (return)
	bool Read (atomic_bool& dirty, type_string& data) const;
	/// Read wstring (template specialization)
	/// @param dirty Reference to dirty flag (user or plc)
	/// @param data Data value reference (return)
	bool Read (atomic_bool& dirty, type_wstring& data) const;
	/// Read character array (pchar)
	/// @param dirty Reference to dirty flag (user or plc)
	/// @param data Destination buffer
	/// @param max Maximum length
	bool Read (atomic_bool& dirty, type_string_value* data, size_type max) const;
	/// Read character array (pwchar)
	/// @param dirty Reference to dirty flag (user or plc)
	/// @param data Destination buffer
	/// @param max Maximum length
	bool Read (atomic_bool& dirty, type_wstring_value* data, size_type max) const;

	/// Write data
	/// @param dirty Reference to dirty flag (user or plc)
	/// @param pend Reference to pending read flag (plc or user)
	/// @param data Data value reference (return)
	template <typename T> bool Write (atomic_bool& dirty, 
		const atomic_bool& pend, const T& data);
	/// Write string (template specialization)
	/// @param dirty Reference to dirty flag (user or plc)
	/// @param pend Reference to pending read flag (plc or user)
	/// @param data Data value reference
	bool Write (atomic_bool& dirty, const atomic_bool& pend, 
		const type_string& data);
	/// Write wstring (template specialization)
	/// @param dirty Reference to dirty flag (user or plc)
	/// @param pend Reference to pending read flag (plc or user)
	/// @param data Data value reference
	bool Write (atomic_bool& dirty, const atomic_bool& pend, 
		const type_wstring& data);
	/// Write character array (pchar)
	/// @param dirty Reference to dirty flag (user or plc)
	/// @param pend Reference to pending read flag (plc or user)
	/// @param data Source buffer
	/// @param max Maximum length
	bool Write (atomic_bool& dirty, const atomic_bool& pend, 
		const type_string_value* data, size_type max);
	/// Write character array (pwchar)
	/// @param dirty Reference to dirty flag (user or plc)
	/// @param pend Reference to pending read flag (plc or user)
	/// @param data Source buffer
	/// @param max Maximum length
	bool Write (atomic_bool& dirty, const atomic_bool& pend, 
		const type_wstring_value* data, size_type max);

	/// Read data as binary
	/// @param dirty Reference to dirty flag (user or plc)
	/// @param p value pointer (destination buffer)
	/// @param len Length in bytes
	size_type ReadBinary (atomic_bool& dirty, type_binary p, size_type len) const;
	/// Write data as binary
	/// @param dirty Reference to dirty flag (user or plc)
	/// @param pend Reference to pending read flag (plc or user)
	/// @param p value pointer (source buffer)
	/// @param len Length in bytes
	size_type WriteBinary (atomic_bool& dirty, const atomic_bool& pend, 
		const type_binary p, size_type len);

	/// Set the valid flag and set the dirty flag when flag changes
	/// @param dirty Reference to dirty flag (user or plc)
	/// @param valid True for valid data, False for invalid
	void SetValid (atomic_bool& dirty, bool valid);
	/// Get the valid flag and reset the dirty flag
	/// @param dirty Reference to dirty flag (user or plc)
	/// @return valid True for valid data, False for invalid
	bool GetValid (atomic_bool& dirty) const;

	/// Data pointer
	data_type				mydata;
	/// Size of allocated memory for simple types; size of string class
	/// for strings and size of data for binary
	size_type				mysize;
	/// Data type
	data_type_enum			mytype;
	/// Valid flag
	atomic_bool				myvalid;
	/// Dirty flag indicating user needs to update
	mutable atomic_bool		myuserdirty;
	/// Dirty flag indicating plc needs to update
	mutable atomic_bool		myplcdirty;
};

/** Enum for access rights of a record
	@brief Access righths enum
************************************************************************/
enum class access_rights_enum 
{
	/// Read only
	read_only, 
	/// Write only
	write_only, 
	/// Read/write 
	read_write
};

/** This is the base class for a tag/channel. It contains a data value
	and owns two pointers to a user and plc interface, respectively.
	This is the basic work horse for exchanging data between a user
	(slave) and a plc (master).
    @brief Class for managing a tag/channel
************************************************************************/
class BaseRecord : public DataValueTypeDef
{
public:
	/// Default constructor
	BaseRecord() : access (access_rights_enum::read_write), process (false), parent (nullptr) {}
	/// Constructor
	/// @param tag Name of tag/channel
	explicit BaseRecord (const std::stringcase& tag)
		: name (tag), access (access_rights_enum::read_write), process (true), parent (nullptr) {}
	/// Constructor
	/// @param recordName Name of tag/channel
	/// @param rt Data type
	/// @param puser Pointer to user interface object (will be adopted!)
	/// @param pplc Pointer to plc interface object (will be adopted!)
	BaseRecord (const std::stringcase& recordName, 
		data_type_enum rt, Interface* puser = nullptr, Interface* pplc = nullptr)
		: name (recordName), access (access_rights_enum::read_write), process (true), value (rt), 
		user (puser), plc (pplc), parent (nullptr) {}
	/// Desctructor
	virtual ~BaseRecord() {};

	/// Get name
	const std::stringcase& get_name() const { return name; };
	/// Set name
	void set_name (const std::stringcase& recordName) { name = recordName; };
	/// Get process flag: false = disabled, true = enabled
	bool get_process() const { return process.load(); };
	/// Set process flag
	void set_process (bool isEnabled) { process = isEnabled; };
	/// Get access rights
	access_rights_enum get_access_rights() { return access; };
	/// Set access rights
	void set_access_rights(access_rights_enum rights) { access = rights; };
	/// Get time stamp
	time_type get_timestamp() const;

	/// Get pointer to user interface (no ownership transfer)
	virtual Interface* get_userInterface() const { return user.get(); };
	/// Get pointer to plc interface (no ownership transfer)
	virtual Interface* get_plcInterface() const { return plc.get(); };
	/// Set user interface (object will be adopted!)
	void set_userInterface (Interface* puser) { user.reset (puser); };
	/// Set plc interface (object will be adopted!)
	void set_plcInterface (Interface* pplc) { plc.reset (pplc); };
	/// Get parent plc
	virtual BasePLC* get_parent() const { return parent; };
	/// Set parent plc
	virtual void set_parent(BasePLC* pPLC) { parent = pPLC; }

	/// Get a const reference to the data object
	const DataValue& get_data() const { return value; }
	/// Get a reference to the data object
	DataValue& get_data() { return value; }
	/// Returns true, if the data is valid
	bool DataIsValid() {
		return process && value.IsValid(); }

	/// Execute a user read, but pull plc first
	/// @param data Reference to data (return)
	/// @return true if successfull
	template <typename T> bool UserRead (T& data) {
		PlcPull(); return value.UserRead (data); }
	/// Execute a user read, but pull plc first
	/// @param data character pointer, pchar (return)
	/// @param max Maximum number of characters
	/// @return true if successfull
	bool UserRead (type_string_value* data, size_type max) {
		PlcPull(); return value.UserRead (data, max); }
	/// Execute a user read, but pull plc first
	/// @param data character pointer, pwchar (return)
	/// @param max Maximum number of characters
	/// @return true if successfull
	bool UserRead (DataValue::type_wstring_value* data, size_type max) {
		PlcPull(); return value.UserRead (data, max); }
	/// Execute a user write and push plc
	/// @param data Reference to data
	/// @return true if successfull
	template <typename T> bool UserWrite (const T& data) {
		bool ret = value.UserWrite (data); if (ret) PlcPush(); return ret; }
	/// Execute a user write and push plc
	/// @param data character pointer, pchar
	/// @param max Maximum number of characters
	/// @return true if successfull
	bool UserWrite (const type_string_value* data, size_type max) {
		bool ret = value.UserWrite (data, max); if (ret) PlcPush(); return ret; }
	/// Execute a user write and push plc
	/// @param data character pointer, pwchar
	/// @param max Maximum number of characters
	/// @return true if successfull
	bool UserWrite (const type_wstring_value* data, size_type max) {
		bool ret = value.UserWrite (data, max); if (ret) PlcPush(); return ret; }

	/// Execute a user read, but pull plc first
	/// @param p Pointer to data (destination buffer)
	/// @param len Length in bytes (must be the same as data length)
	/// @return Number of bytes read (0 on error)
	size_type UserReadBinary (type_binary p, size_type len) {
		PlcPull(); return value.UserReadBinary (p, len); }
	/// Execute a user write and push plc
	/// @param p Pointer to data (source buffer)
	/// @param len Length in bytes (must be the same as data length)
	/// @return Number of bytes written (0 on error)
	size_type UserWriteBinary (const type_binary p, size_type len) {
		size_type ret = value.UserWriteBinary (p, len); 
		if (ret > 0) PlcPush(); return ret; }
	/// Ckecks if the user needs to read an updated value
	bool UserIsDirty() const {return value.UserIsDirty(); }
	/// Set dirty flag for user
	void UserSetDirty() { value.UserSetDirty(); UserPush(); }
	/// Initiated a user pull, if the data needs an update
	/// @param force Forces a user update even when not needed
	inline bool UserPush (bool force = false);
	/// Pulls the user for new data
	inline bool UserPull();

	/// Set the user valid flag and set the dirty flag when flag changes
	/// @param valid True for valid data, False for invalid
	void UserSetValid (bool valid) { value.UserSetValid (valid); UserPush(); }
	/// Get the user valid flag and reset the dirty flag
	/// @return valid True for valid data, False for invalid
	bool UserGetValid() { UserPull(); 
		return value.UserGetValid() && process.load(); }

	/// Execute a plc read, but pull user first
	/// @param data Reference to data (return)
	/// @return true if successfull
	template <typename T> bool PlcRead (T& data) {
		UserPull(); return value.PlcRead (data); }
	/// Execute a plc read, but pull user first
	/// @param data character pointer, pchar (return)
	/// @param max Maximum number of characters
	/// @return true if successfull
	bool PlcRead (type_string_value* data, size_type max) {
		UserPull(); return value.PlcRead (data, max); }
	/// Execute a plc read, but pull user first
	/// @param data character pointer, pwchar (return)
	/// @param max Maximum number of characters
	/// @return true if successfull
	bool PlcRead (type_wstring_value* data, size_type max) {
		UserPull(); return value.PlcRead (data, max); }

	/// Execute a plc write and push user
	/// @param data Reference to data
	/// @return true if successfull
	template <typename T> bool PlcWrite (const T& data) {
		bool ret = value.PlcWrite (data); if (ret) UserPush(); return ret; }
	/// Execute a plc write and push user
	/// @param data character pointer, pchar
	/// @param max Maximum number of characters
	/// @return true if successfull
	bool PlcWrite (const type_string_value* data, size_type max) {
		bool ret = value.PlcWrite (data, max); if (ret) UserPush(); return ret; }
	/// Execute a plc write and push user
	/// @param data character pointer, pwchar
	/// @param max Maximum number of characters
	/// @return true if successfull
	bool PlcWrite (const type_wstring_value* data, size_type max) {
		bool ret = value.PlcWrite (data, max); if (ret) UserPush(); return ret; }

	/// Execute a plc read, but pull user first
	/// @param p Pointer to data (destination buffer)
	/// @param len Length in bytes (must be the same as data length)
	/// @return Number of bytes read (0 on error)
	size_type PlcReadBinary (type_binary p, size_type len) {
		UserPull(); return value.PlcReadBinary (p, len); }
	/// Execute a plc write and push user
	/// @param p Pointer to data (source buffer)
	/// @param len Length in bytes (must be the same as data length)
	/// @return Number of bytes written (0 on error)
	size_type PlcWriteBinary (const type_binary p, size_type len) {
		size_type ret = value.PlcWriteBinary (p, len); 
		if (ret > 0) UserPush(); return ret; }
	/// Checks if the plc needs to read an updated value
	bool PlcIsDirty() const {return value.PlcIsDirty(); }
	/// Set dirty flag for plc
	void PlcSetDirty() { value.PlcSetDirty(); PlcPush(); }
	/// Initiated a plc pull, if the data needs an update
	/// @param force Forces a plc update even when not needed
	inline bool PlcPush (bool force = false);
	/// Pulls the plc for new data
	inline bool PlcPull();

	/// Set the plc valid flag and set the dirty flag when flag changes
	/// @param valid True for valid data, False for invalid
	void PlcSetValid (bool valid) { value.PlcSetValid (valid); PlcPush(); }
	/// Get the plc valid flag and reset the dirty flag
	/// @return valid True for valid data, False for invalid
	bool PlcGetValid() { PlcPull(); 
		return value.PlcGetValid() && process.load(); }

protected:
	/// Name 
	std::stringcase			name;
	/// Enum for access rights
	access_rights_enum      access;
	/// Process flag: false = disabled, true = enabled
	atomic_bool				process;
	/// Data value
	DataValue				value;
	/// PLC interface (master)
	InterfacePtr			plc;
	/// User interface (slave)
	InterfacePtr			user;
	/// PLC that this record belongs to
	BasePLC*				parent;
};

/** This is a smart pointer to a tag/channel record 
    @brief smart pointer to record
************************************************************************/
typedef std::shared_ptr<BaseRecord> BaseRecordPtr;

/** This is a list of tag/channel records organized as a hash map
    @brief list of record
************************************************************************/
typedef std::unordered_map<std::stringcase, BaseRecordPtr> BaseRecordList;

/** This is a base class for interfacing a programmable logic controller.
    It contains and manages a list of tag/channel records. This is a base
	class which needs to be used a derived class by a real implementation.

	This class is MT safe and uses a mutex to synchronize access.

    @brief Base PLC
 ************************************************************************/
class BasePLC
{
public:
	/// Defines the mutex type
	typedef std::recursive_mutex mutex_type;
	/// Defined the mutex guard type
	typedef std::lock_guard<mutex_type> guard; 
	/// Define timestamp type
	typedef DataValueTypeDef::type_uint64 time_type;
	/// Function pointer to scanner
	typedef void (BasePLC::*scanner_func) ();

	/// Default constructor
	BasePLC();
	/// Destructor
	virtual ~BasePLC();

	/// Get read scannner period in ms
	int get_read_scanner_period () const { 
		return read_scanner_period; }
	/// Set read scannner period in ms
	void set_read_scanner_period (int period) {
		read_scanner_period = period; }
	/// Start read scannner
	bool start_read_scanner();
	/// Terminate read scannner
	bool terminate_read_scanner();

	/// Get write scannner period in ms
	int get_write_scanner_period () const { 
		return write_scanner_period; }
	/// Set write scannner period in ms
	void set_write_scanner_period (int period) {
		write_scanner_period = period; }
	/// Start write scannner
	bool start_write_scanner();
	/// Terminate write scannner
	bool terminate_write_scanner();

	/// Get update scannner period in ms
	int get_update_scanner_period () const { 
		return update_scanner_period; }
	/// Set update scannner period in ms
	void set_update_scanner_period (int period) {
		update_scanner_period = period; }
	/// Start update scannner
	bool start_update_scanner();
	/// Terminate update scannner
	bool terminate_update_scanner();

	/// is scanner active?
	bool is_scanner_active() const { return scanners_active; }
	/// set scanner active state
	void set_scanners_active (bool active) { scanners_active = active; }

	/// Reserves the given number of elements in the tag/channel list.
	/// Use this function when you know many elements are added beforehand
	/// to avoid unnecessary rehashing.
	/// @param n Number of expected tag/channel records
	void reserve (BaseRecordList::size_type n) {
		records.reserve (n); }
	/// Add a new tag/channel record. Adding a duplicate is not possible.
	/// @param precord Pointer to record. Will be adopted
	/// @return true, if it could be added
	bool add (BaseRecord* precord) {
		return precord ? add (BaseRecordPtr (precord)) : false; }
	/// Add a new tag/channel record. Adding a duplicate is not possible.
	/// @param precord Smart pointer to record. 
	/// @return true, if it could be added
	bool add (BaseRecordPtr precord);
	/// Find a new tag/channel record. 
	/// @param name Name of record
	/// @return Smart pointer to record (contains nullptr when not found)
	BaseRecordPtr find (const std::stringcase& name);
	/// Erase a tag/channel record. 
	/// @param name Name of record
	/// @return true if erased
	bool erase (const std::stringcase& name);

	/// Get next record in list. Resets to the beginning, if the list 
	/// changes between subsequest access. This is an MT safe access
	/// method to cycle through the list. However, it is not high 
	/// performance and is meant for slow external acecss.
	/// @param next Next tag/channel record (return)
	/// @param prev tag/channel record 
	/// @return true if successful
	bool get_next (BaseRecordPtr& next, const BaseRecordPtr& prev) const;
	/// Iterate over all list elements
	/// This will yield good performance, but will lock the PLC 
	/// for the entire processing time
	/// @param f Function which takes BaseRecord* as the argument
	template <typename func> void for_each (func& f);
	/// Iterate over all list elements
	/// This will yield good performance, but will lock the PLC 
	/// for the entire processing time
	/// @param f Function which takes BaseRecord* as the argument
	template <typename func> void for_each(const func& f);
	/// Count the number of records
	int count() const;

	/// Print all records and vals to stdout. (override for action)
	virtual void printAllRecords() {};
	/// Print a record values to stdout. (override for action)
	/// @param var variable name (accepts wildcards)
	virtual void printRecord (const std::string& var) {};

	/// Get time stamp
	virtual time_type get_timestamp() const { return timestamp; }
	/// Get time stamp as unix time (seconds since 1970-01-01 00:00:00)
	/// Does not include leap seconds
	virtual time_t get_timestamp_unix() const;
	/// Set time stamp
	virtual void set_timestamp (time_type tstamp) { timestamp = tstamp; }
	/// Set time stamp to current time
	virtual void update_timestamp();

	/// Get name
	const std::stringcase& get_name() const { return name; }
	/// Get nick name/alias
	const std::stringcase& get_alias () const { return alias; }
	/// Set nick name/alias
	void set_alias (const std::stringcase& nickname) { alias = nickname; }

	/** This function is called by the driver support in drvtc.cpp, after 
	the PLC has been initialized. This function is overridden by the 
	derived PLCs, and generally will start all the scanner threads.
	@return true if successful
	*/
	virtual bool start() { return true; };

	/// Set the valid flag for all data values by the user
	/// @param valid Valid flag, true for valid, false for invalid
	virtual void user_data_set_valid (bool valid);
	/// Set the valid flag for all data values by the plc
	/// @param valid Valid flag, true for valid, false for invalid
	virtual void plc_data_set_valid (bool valid);

protected:
	/// Set name (careful! This is used for indexing in the PLCList of System)
	void set_name (const std::stringcase& n) { name = n; }

	/// Mutex to synchronize access to this class
	mutable mutex_type	mux;
	/// Name
	std::stringcase		name;
	/// Nick name or alias (used to generate info record names)
	std::stringcase		alias;
	/// List of tags/channels. 
	/// The load factor is initialized to 0.5.
	BaseRecordList		records;
	/// Time stamp
	time_type			timestamp;
	/// read scanner period in ms
	int					read_scanner_period;
	/// write scanner period in ms
	int					write_scanner_period;
	/// update scanner period in ms
	int					update_scanner_period;
	/// scanners are active
	std::atomic<bool>	scanners_active;
	/// read thread 
	std::thread			read_thread;
	/// write thread 
	std::thread			write_thread;
	/// update thread 
	std::thread			update_thread;

	/// read scanner (override for action)
	virtual void read_scanner () {};
	/// write scanner (override for action)
	virtual void write_scanner () {};
	/// update scanner (override for action)
	virtual void update_scanner () {};
};

/** This is list of BasePLC, ordered by their name.
    @brief BasePLC map
************************************************************************/
typedef std::map<std::stringcase, BasePLCPtr> BasePLCList;


/** This is a class for managing multiple PLCs.
    @brief System to keep track of PLCs
 ************************************************************************/
class System
{
public:
	/// Defines the mutex type
	typedef std::recursive_mutex mutex_type;
	/// Defined the mutex guard type
	typedef std::lock_guard<mutex_type> guard; 

	/// Return a reference to the gloabl System variable
	static System& get() { return tCat; }
	/// Add a new PLC, PLC will be adopted
	/// @param plc Pointer to plc
	/// @return true if successful
	bool add (BasePLC* plc) {
		return plc ? add (BasePLCPtr (plc)) : false; }
	/// Add a new PLC
	/// @param plc Smart pointer to plc
	/// @return true if successful
	bool add (BasePLCPtr plc);
	/// Finds a PLC by its name
	BasePLCPtr find (std::stringcase id);
	/// Iterate over all list elements
	/// This will yield good performance, but will lock the PLC 
	/// for the entire processing time
	/// @param f Function which takes BaseRecord* as the argument
	template <typename func> void for_each (func& f);
	/// Iterate over all list elements
	/// This will yield good performance, but will lock the PLC 
	/// for the entire processing time
	/// @param f Function which takes BaseRecord* as the argument
	template <typename func> void for_each(const func& f);

	/// Print all PLC record values to the console
	void printVals();
	/// Print PLC record value to the console
	/// @param var Variable name or wildcard
	void printVal (const std::string& var);

	/// Start scanning after ioc is running
	void start();
	/// Stop scanning when ioc is paused
	void stop();

	/// get Ioc run state
	bool is_ioc_running () const { return IocRun; }
	/// set Ioc run state
	void set_ioc_state (bool run) { 
		IocRun = run; run ? start() : stop(); }
protected:
	/// Mutex to synchronize access to this class
	mutable mutex_type	mux;
	/// Master list of all PLCs
	BasePLCList			PLCs;
	/// IOC is running
	bool				IocRun;
private:
	/// Constructor
	System();
	/// Copy operator
	System(const System& tc);
	/// Assignment operator
	System& operator= (const System& tc);
	/// Deconstructor
	~System();
	/// A single instance of System
	static System	tCat;
};



}

// Stop TwinCAT
//extern "C" {
//__declspec(dllexport) void __cdecl stopTc(void);
//}

#include "plcBaseTemplate.h"