namespace plc {

#pragma warning(push)
#pragma warning(disable: 4244)

/** @file plcBaseTemplate.h
	Header which includes templated methods for abstract record class and 
	the DataValue class.
 ************************************************************************/

/** DataValue::data_enum
 ************************************************************************/
template <typename T> 
	const typename DataValueTraits<T>::data_type_enum 
	DataValueTraits<T>::data_enum = data_type_enum::dtInvalid;

/** Will read the value and reset the dirty flag.
   @brief Reset and read
 ************************************************************************/
template<typename T, typename U>
bool reset_and_read (DataValueTypeDef::atomic_bool& dirty, 
					 T& dest, U source)
{
	// must be before read
	dirty.store (false, DataValueTypeDef::memory_order); 
	dest = (T) (source->load (DataValueTypeDef::memory_order));
	return true;
}

/** Will read the value and reset the dirt flag (same type).
   @brief Reset and read
 ************************************************************************/
template<typename T>
bool reset_and_read (DataValueTypeDef::atomic_bool& dirty, T& dest, 
					 typename DataValueTraits<T>::traits_atomic* source)
{
	// must be before read
	dirty.store (false, DataValueTypeDef::memory_order); 
	dest = source->load (DataValueTypeDef::memory_order);
	return true;
}

/** Will set the dirty bit, when the newly written value is different 
   from the old one.
   @brief Write and test
 ************************************************************************/
template<typename T, typename U>
bool write_and_test (DataValueTypeDef::atomic_bool& dirty, 
					 const DataValueTypeDef::atomic_bool& read_pending,
					 DataValueTypeDef::atomic_bool& valid, 
					 U dest, const T& source)
{
	if (read_pending.load(DataValueTypeDef::memory_order)) return false;
	auto old = dest->exchange (source, DataValueTypeDef::memory_order);
	bool oldvalid = valid.exchange (true, DataValueTypeDef::memory_order);
	//if (old != (decltype(old)) source || !oldvalid) {
		// must be after modifying the value
		dirty.store (true, DataValueTypeDef::memory_order); 
	//}
	return true;
}

/** Will set the dirty bit, when the newly written value is different 
   from the old one (same type).
   @brief Write and test
 ************************************************************************/
template<typename T>
bool write_and_test (DataValueTypeDef::atomic_bool& dirty,
					 const DataValueTypeDef::atomic_bool& read_pending,
					 DataValueTypeDef::atomic_bool& valid, 
					 typename DataValueTraits<T>::traits_atomic* dest, 
					 const T& source)
{
	if (read_pending.load(DataValueTypeDef::memory_order)) return false;
	T old = dest->exchange (source, DataValueTypeDef::memory_order);
	bool oldvalid = valid.exchange (true, DataValueTypeDef::memory_order);
	//if (old != source || !oldvalid) {
		// must be after modifying the value
		dirty.store (true, DataValueTypeDef::memory_order); 
	//}
	return true;
}

/** DataValue::Read (bool, Inegral and floating point types)
 ************************************************************************/
template <typename T> 
bool DataValue::Read (atomic_bool& dirty, T& data) const
{
	switch (mytype) {
	case data_type_enum::dtBool:
		return reset_and_read (dirty, data, (atomic_bool*)mydata);
	case data_type_enum::dtInt8:
		return reset_and_read (dirty, data, (atomic_int8*)mydata);
	case data_type_enum::dtUInt8:
		return reset_and_read (dirty, data, (atomic_uint8*)mydata);
	case data_type_enum::dtInt16:
		return reset_and_read (dirty, data, (atomic_int16*)mydata);
	case data_type_enum::dtUInt16:
		return reset_and_read (dirty, data, (atomic_uint16*)mydata);
	case data_type_enum::dtInt32:
		return reset_and_read (dirty, data, (atomic_int32*)mydata);
	case data_type_enum::dtUInt32:
		return reset_and_read (dirty, data, (atomic_uint32*)mydata);
	case data_type_enum::dtInt64:
		return reset_and_read (dirty, data, (atomic_int64*)mydata);
	case data_type_enum::dtUInt64:
		return reset_and_read (dirty, data, (atomic_uint64*)mydata);
	case data_type_enum::dtFloat:
		return reset_and_read (dirty, data, (atomic_float*)mydata);
	case data_type_enum::dtDouble:
		return reset_and_read (dirty, data, (atomic_double*)mydata);
	default:
		return false;
	}
}

/** DataValue::UserWrite (bool, Integral and floating point types)
 ************************************************************************/
template<typename T> 
bool DataValue::Write (atomic_bool& dirty, const atomic_bool& pend, const T& data)
{
	switch (mytype) {
	case data_type_enum::dtBool:
		return write_and_test (dirty, pend, myvalid, (atomic_bool*)mydata, data);
	case data_type_enum::dtInt8:
		return write_and_test (dirty, pend, myvalid, (atomic_int8*)mydata, data);
	case data_type_enum::dtUInt8:
		return write_and_test (dirty, pend, myvalid, (atomic_uint8*)mydata, data);
	case data_type_enum::dtInt16:
		return write_and_test (dirty, pend, myvalid, (atomic_int16*)mydata, data);
	case data_type_enum::dtUInt16:
		return write_and_test (dirty, pend, myvalid, (atomic_uint16*)mydata, data);
	case data_type_enum::dtInt32:
		return write_and_test (dirty, pend, myvalid, (atomic_int32*)mydata, data);
	case data_type_enum::dtUInt32:
		return write_and_test (dirty, pend, myvalid, (atomic_uint32*)mydata, data);
	case data_type_enum::dtInt64:
		return write_and_test (dirty, pend, myvalid, (atomic_int64*)mydata, data);
	case data_type_enum::dtUInt64:
		return write_and_test (dirty, pend, myvalid, (atomic_uint64*)mydata, data);
	case data_type_enum::dtFloat:
		return write_and_test (dirty, pend, myvalid, (atomic_float*)mydata, data);
	case data_type_enum::dtDouble:
		return write_and_test (dirty, pend, myvalid, (atomic_double*)mydata, data);
	default:
		return false;
	}
}

/* BaseRecord::UserPush
 ************************************************************************/
inline
bool BaseRecord::UserPush (bool force) 
{ 
	if (user.get() && (force || UserIsDirty())) {
		return user->push(); 
	} 
	else {
		return false; 
	} 
}

/* BaseRecord::UserPull
 ************************************************************************/
inline
bool BaseRecord::UserPull() 
{ 
	if (user.get()) {
		return user->pull(); 
	} 
	else {
		return false; 
	} 
}

/* BaseRecord::PlcPush
 ************************************************************************/
inline
bool BaseRecord::PlcPush (bool force) 
{ 
	if (plc.get() && (force || PlcIsDirty())) {
		return plc->push(); 
	} 
	else {
		return false; 
	} 
}

/* BaseRecord::PlcPull
 ************************************************************************/
inline
bool BaseRecord::PlcPull() 
{ 
	if (plc.get()) {
		return plc->pull(); 
	} 
	else {
		return false; 
	} 
}

/* BasePLC::for_each
 ************************************************************************/
template <typename func> 
void BasePLC::for_each (func& f) 
{
	guard lock (mux);
	for (auto& i: records) {
		f (i.second.get()); 
	}
}

/* BasePLC::for_each
 ************************************************************************/
template <typename func>
void BasePLC::for_each(const func& f)
{
	guard lock(mux);
	for (auto& i : records) {
		f(i.second.get());
	}
}

/* System::for_each
 ************************************************************************/
template <typename func> 
void System::for_each (func& f) 
{
	guard lock (mux);
	for (auto& i: PLCs) {
		f (i.second.get()); 
	}
}

/* System::for_each
 ************************************************************************/
template <typename func>
void System::for_each(const func& f)
{
	guard lock(mux);
	for (auto& i : PLCs) {
		f(i.second.get());
	}
}

#pragma warning (pop)
}
