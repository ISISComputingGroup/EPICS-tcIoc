#include "tcComms.h"
#include "aiRecord.h"
#include "aaiRecord.h"
#include "aoRecord.h"
#include "aaoRecord.h"
#include "biRecord.h"
#include "boRecord.h"
#include "longinRecord.h"
#include "longoutRecord.h"
#if EPICS_VERSION >= 7
#include "int64inRecord.h"
#include "int64outRecord.h"
#endif
#include "stringinRecord.h"
#include "stringoutRecord.h"
#include "lsiRecord.h"
#include "lsoRecord.h"
#include "mbbiRecord.h"
#include "mbboRecord.h"
#include "mbbiDirectRecord.h"
#include "mbboDirectRecord.h"
#include "waveformRecord.h"
#include "eventRecord.h"
#include "histogramRecord.h"
#include "alarm.h"
#include "recGbl.h"

/** @file devTcTemplate.h
	Header which includes templates for the device support functions for 
	different EPICS record types.
 ************************************************************************/

namespace DevTc {

/// Epics traits class specialization for aai record
template<>
struct epics_record_traits<epics_record_enum::aaival>
{
	using traits_type = aaiRecord;
	using value_type = void*;
	static const char* const name () noexcept { return "aaival"; };
    static const int value_conversion = 0;
	static const bool input_record = true;
	static const bool raw_record = false;
	static value_type* val (traits_type* prec) noexcept { return (value_type*) &prec->val; }
	static bool read (traits_type* epicsrec, plc::BaseRecord* baserec) noexcept {
		const auto size = baserec->get_data().get_size();
		return baserec->UserReadBinary(*val (epicsrec), size) == size; }
	static bool write (plc::BaseRecord* baserec, traits_type* epicsrec) noexcept {
		const auto size = baserec->get_data().get_size();
		return baserec->UserWriteBinary(*val (epicsrec), size) == size; }
};

/// Epics traits class specialization for aao record
template<>
struct epics_record_traits<epics_record_enum::aaoval>
{
	using traits_type = aaoRecord;
	using value_type = void*;
	static const char* const name () noexcept { return "aaoval"; };
    static const int value_conversion = 0;
	static const bool input_record = false;
	static const bool raw_record = false;
	static value_type* val (traits_type* prec) noexcept { return (value_type*) &prec->val; }
	static bool read (traits_type* epicsrec, plc::BaseRecord* baserec) noexcept {
		const auto size = baserec->get_data().get_size();
		return baserec->UserReadBinary(*val (epicsrec), size) == size; }
	static bool write (plc::BaseRecord* baserec, traits_type* epicsrec) noexcept {
		const auto size = baserec->get_data().get_size();
		return baserec->UserWriteBinary(*val (epicsrec), size) == size; }
};

/// Epics traits class specialization for ai record
template<>
struct epics_record_traits<epics_record_enum::aival>
{
	using traits_type = aiRecord;
	using value_type = epicsFloat64;
	static const char* const name () noexcept { return "aival"; };
    static const int value_conversion = 2;
	static const bool input_record = true;
	static const bool raw_record = false;
	static value_type* val (traits_type* prec) noexcept { return (value_type*) &prec->val; }
	static bool read (traits_type* epicsrec, plc::BaseRecord* baserec) noexcept {
		return baserec->UserRead (*val (epicsrec)); }
	static bool write (plc::BaseRecord* baserec, traits_type* epicsrec) noexcept {
		return baserec->UserWrite (*val (epicsrec)); }
};

/// Epics traits class specialization for ai raw record
template<>
struct epics_record_traits<epics_record_enum::airval>
{
	using traits_type = aiRecord;
	using value_type = epicsInt32;
	static const char* const name () noexcept { return "airval"; };
    static const int value_conversion = 0;
	static const bool input_record = true;
	static const bool raw_record = true;
	static value_type* val (traits_type* prec) noexcept { return (value_type*) &prec->rval; }
	static bool read (traits_type* epicsrec, plc::BaseRecord* baserec) noexcept {
		return baserec->UserRead (*val (epicsrec)); }
	static bool write (plc::BaseRecord* baserec, traits_type* epicsrec) noexcept {
		return baserec->UserWrite (*val (epicsrec)); }
};

/// Epics traits class specialization for ao record
template<>
struct epics_record_traits<epics_record_enum::aoval>
{
	using traits_type = aoRecord;
	using value_type = epicsFloat64;
	static const char* const name () noexcept { return "aoval"; };
    static const int value_conversion = 0;
	static const bool input_record = false;
	static const bool raw_record = false;
	static value_type* val (traits_type* prec) noexcept { return (value_type*) &prec->val; }
	static bool read (traits_type* epicsrec, plc::BaseRecord* baserec) noexcept {
		return baserec->UserRead (*val (epicsrec)); }
	static bool write (plc::BaseRecord* baserec, traits_type* epicsrec) noexcept {
		return baserec->UserWrite (*val (epicsrec)); }
};

/// Epics traits class specialization for ao raw record
template<>
struct epics_record_traits<epics_record_enum::aorval>
{
	using traits_type = aoRecord;
	using value_type = epicsInt32;
	static const char* const name () noexcept { return "aorval"; };
    static const int value_conversion = 0;
	static const bool input_record = false;
	static const bool raw_record = true;
	static value_type* val (traits_type* prec) noexcept { return (value_type*) &prec->rval; }
	static bool read (traits_type* epicsrec, plc::BaseRecord* baserec) noexcept {
		return baserec->UserRead (*val (epicsrec)); }
	static bool write (plc::BaseRecord* baserec, traits_type* epicsrec) noexcept {
		return baserec->UserWrite (*val (epicsrec)); }
};

/// Epics traits class specialization for bi record
template<>
struct epics_record_traits<epics_record_enum::bival>
{
	using traits_type = biRecord;
	using value_type = epicsEnum16;
	static const char* const name () noexcept { return "bival"; };
    static const int value_conversion = 2;
	static const bool input_record = true;
	static const bool raw_record = false;
	static value_type* val (traits_type* prec) noexcept { return (value_type*) &prec->val; }
	static bool read (traits_type* epicsrec, plc::BaseRecord* baserec) noexcept {
		return baserec->UserRead (*val (epicsrec)); }
	static bool write (plc::BaseRecord* baserec, traits_type* epicsrec) noexcept {
		return baserec->UserWrite (*val (epicsrec)); }
};

/// Epics traits class specialization for bi raw record
template<>
struct epics_record_traits<epics_record_enum::birval>
{
	using traits_type = biRecord;
	using value_type = epicsUInt32;
	static const char* const name () noexcept { return "birval"; };
    static const int value_conversion = 0;
	static const bool input_record = true;
	static const bool raw_record = true;
	static value_type* val (traits_type* prec) noexcept { return (value_type*) &prec->rval; }
	static bool read (traits_type* epicsrec, plc::BaseRecord* baserec) noexcept {
		return baserec->UserRead (*val (epicsrec)); }
	static bool write (plc::BaseRecord* baserec, traits_type* epicsrec) noexcept {
		return baserec->UserWrite (*val (epicsrec)); }
};

/// Epics traits class specialization for bo record
template<>
struct epics_record_traits<epics_record_enum::boval>
{
	using traits_type = boRecord;
	using value_type = epicsEnum16;
	static const char* const name () noexcept { return "boval"; };
    static const int value_conversion = 2;
	static const bool input_record = false;
	static const bool raw_record = false;
	static value_type* val (traits_type* prec) noexcept { return (value_type*) &prec->val; }
	static bool read (traits_type* epicsrec, plc::BaseRecord* baserec) noexcept {
		return baserec->UserRead (*val (epicsrec)); }
	static bool write (plc::BaseRecord* baserec, traits_type* epicsrec) noexcept {
		return baserec->UserWrite (*val (epicsrec)); }
};

/// Epics traits class specialization for bo raw record
template<>
struct epics_record_traits<epics_record_enum::borval>
{
	using traits_type = boRecord;
	using value_type = epicsUInt32;
	static const char* const name () noexcept { return "borval"; };
    static const int value_conversion = 0;
	static const bool input_record = false;
	static const bool raw_record = true;
	static value_type* val (traits_type* prec) noexcept { return (value_type*) &prec->rval; }
	static bool read (traits_type* epicsrec, plc::BaseRecord* baserec) noexcept {
		return baserec->UserRead (*val (epicsrec)); }
	static bool write (plc::BaseRecord* baserec, traits_type* epicsrec) noexcept {
		return baserec->UserWrite (*val (epicsrec)); }
};

/// Epics traits class specialization for longin record
template<>
struct epics_record_traits<epics_record_enum::longinval>
{
	using traits_type = longinRecord;
	using value_type = epicsInt32;
	static const char* const name () noexcept { return "longinval"; };
    static const int value_conversion = 2;
	static const bool input_record = true;
	static const bool raw_record = false;
	static value_type* val (traits_type* prec) noexcept { return (value_type*) &prec->val; }
	static bool read (traits_type* epicsrec, plc::BaseRecord* baserec) noexcept {
		return baserec->UserRead (*val (epicsrec)); }
	static bool write (plc::BaseRecord* baserec, traits_type* epicsrec) noexcept {
		return baserec->UserWrite (*val (epicsrec)); }
};

/// Epics traits class specialization for longout record
template<>
struct epics_record_traits<epics_record_enum::longoutval>
{
	using traits_type = longoutRecord;
	using value_type = epicsInt32;
	static const char* const name () noexcept { return "longoutval"; };
    static const int value_conversion = 0;
	static const bool input_record = false;
	static const bool raw_record = false;
	static value_type* val (traits_type* prec) noexcept { return (value_type*) &prec->val; }
	static bool read (traits_type* epicsrec, plc::BaseRecord* baserec) noexcept {
		return baserec->UserRead (*val (epicsrec)); }
	static bool write (plc::BaseRecord* baserec, traits_type* epicsrec) noexcept {
		return baserec->UserWrite (*val (epicsrec)); }
};

#if EPICS_VERSION >= 7
/// Epics traits class specialization for int64in record
template<>
struct epics_record_traits<epics_record_enum::int64inval>
{
	using traits_type = int64inRecord;
	using value_type = epicsInt64;
	static const char* const name() noexcept { return "int64inval"; };
	static const int value_conversion = 2;
	static const bool input_record = true;
	static const bool raw_record = false;
	static value_type* val(traits_type* prec) noexcept { return (value_type*)&prec->val; }
	static bool read(traits_type* epicsrec, plc::BaseRecord* baserec) noexcept {
		return baserec->UserRead(*val(epicsrec));
	}
	static bool write(plc::BaseRecord* baserec, traits_type* epicsrec) noexcept {
		return baserec->UserWrite(*val(epicsrec));
	}
};

/// Epics traits class specialization for int64out record
template<>
struct epics_record_traits<epics_record_enum::int64outval>
{
	using traits_type = int64outRecord;
	using value_type = epicsInt64;
	static const char* const name() noexcept { return "int64outval"; };
	static const int value_conversion = 0;
	static const bool input_record = false;
	static const bool raw_record = false;
	static value_type* val(traits_type* prec) noexcept { return (value_type*)&prec->val; }
	static bool read(traits_type* epicsrec, plc::BaseRecord* baserec) noexcept {
		return baserec->UserRead(*val(epicsrec));
	}
	static bool write(plc::BaseRecord* baserec, traits_type* epicsrec) noexcept {
		return baserec->UserWrite(*val(epicsrec));
	}
};
#endif

/// Epics traits class specialization for mbbi record
template<>
struct epics_record_traits<epics_record_enum::mbbival>
{
	using traits_type = mbbiRecord;
	using value_type = epicsEnum16;
	static const char* const name () noexcept { return "mbbival"; };
    static const int value_conversion = 2;
	static const bool input_record = true;
	static const bool raw_record = false;
	static value_type* val (traits_type* prec) noexcept { return (value_type*) &prec->val; }
	static bool read (traits_type* epicsrec, plc::BaseRecord* baserec) noexcept {
		return baserec->UserRead (*val (epicsrec)); }
	static bool write (plc::BaseRecord* baserec, traits_type* epicsrec) noexcept {
		return baserec->UserWrite (*val (epicsrec)); }
};

/// Epics traits class specialization for mbbi raw record
template<>
struct epics_record_traits<epics_record_enum::mbbirval>
{
	using traits_type = mbbiRecord;
	using value_type = epicsUInt32;
	static const char* const name () noexcept { return "mbbirval"; };
    static const int value_conversion = 0;
	static const bool input_record = true;
	static const bool raw_record = true;
	static value_type* val (traits_type* prec) noexcept { return (value_type*) &prec->rval; }
	static bool read (traits_type* epicsrec, plc::BaseRecord* baserec) noexcept {
		return baserec->UserRead (*val (epicsrec)); }
	static bool write (plc::BaseRecord* baserec, traits_type* epicsrec) noexcept {
		return baserec->UserWrite (*val (epicsrec)); }
};

/// Epics traits class specialization for mbbo record
template<>
struct epics_record_traits<epics_record_enum::mbboval>
{
	using traits_type = mbboRecord;
	using value_type = epicsEnum16;
	static const char* const name () noexcept { return "mbboval"; };
    static const int value_conversion = 0;
	static const bool input_record = false;
	static const bool raw_record = false;
	static value_type* val (traits_type* prec) noexcept { return (value_type*) &prec->val; }
	static bool read (traits_type* epicsrec, plc::BaseRecord* baserec) noexcept {
		return baserec->UserRead (*val (epicsrec)); }
	static bool write (plc::BaseRecord* baserec, traits_type* epicsrec) noexcept {
		return baserec->UserWrite (*val (epicsrec)); }
};

/// Epics traits class specialization for mbbo raw record
template<>
struct epics_record_traits<epics_record_enum::mbborval>
{
	using traits_type = mbboRecord;
	using value_type = epicsUInt32;
	static const char* const name () noexcept { return "mbborval"; };
    static const int value_conversion = 0;
	static const bool input_record = false;
	static const bool raw_record = true;
	static value_type* val (traits_type* prec) noexcept { return (value_type*) &prec->rval; }
	static bool read (traits_type* epicsrec, plc::BaseRecord* baserec) noexcept {
		return baserec->UserRead (*val (epicsrec)); }
	static bool write (plc::BaseRecord* baserec, traits_type* epicsrec) noexcept {
		return baserec->UserWrite (*val (epicsrec)); }
};

/// Epics traits class specialization for mbbiDirect record
template<>
struct epics_record_traits<epics_record_enum::mbbiDirectval>
{
	using traits_type = mbbiDirectRecord;
	using value_type = epicsEnum16;
	static const char* const name () noexcept { return "mbbiDirectval"; };
    static const int value_conversion = 2;
	static const bool input_record = true;
	static const bool raw_record = false;
	static value_type* val (traits_type* prec) noexcept { return (value_type*) &prec->val; }
	static bool read (traits_type* epicsrec, plc::BaseRecord* baserec) noexcept {
		return baserec->UserRead (*val (epicsrec)); }
	static bool write (plc::BaseRecord* baserec, traits_type* epicsrec) noexcept {
		return baserec->UserWrite (*val (epicsrec)); }
};

/// Epics traits class specialization for mbbiDirect raw record
template<>
struct epics_record_traits<epics_record_enum::mbbiDirectrval>
{
	using traits_type = mbbiDirectRecord;
	using value_type = epicsUInt32;
	static const char* const name () noexcept { return "mbbiDirectrval"; };
    static const int value_conversion = 0;
	static const bool input_record = true;
	static const bool raw_record = true;
	static value_type* val (traits_type* prec) noexcept { return (value_type*) &prec->rval; }
	static bool read (traits_type* epicsrec, plc::BaseRecord* baserec) noexcept {
		return baserec->UserRead (*val (epicsrec)); }
	static bool write (plc::BaseRecord* baserec, traits_type* epicsrec) noexcept {
		return baserec->UserWrite (*val (epicsrec)); }
};

/// Epics traits class specialization for mbboDirect record
template<>
struct epics_record_traits<epics_record_enum::mbboDirectval>
{
	using traits_type = mbboDirectRecord;
	using value_type = epicsEnum16;
	static const char* const name () noexcept { return "mbboDirectval"; };
    static const int value_conversion = 0;
	static const bool input_record = false;
	static const bool raw_record = false;
	static value_type* val (traits_type* prec) noexcept { return (value_type*) &prec->val; }
	static bool read (traits_type* epicsrec, plc::BaseRecord* baserec) noexcept {
		return baserec->UserRead (*val (epicsrec)); }
	static bool write (plc::BaseRecord* baserec, traits_type* epicsrec) noexcept {
		return baserec->UserWrite (*val (epicsrec)); }
};

/// Epics traits class specialization for mbboDirect raw record
template<>
struct epics_record_traits<epics_record_enum::mbboDirectrval>
{
	using traits_type = mbboDirectRecord;
	using value_type = epicsUInt32;
	static const char* const name () noexcept { return "mbboDirectrval"; };
    static const int value_conversion = 0;
	static const bool input_record = false;
	static const bool raw_record = true;
	static value_type* val (traits_type* prec) noexcept { return (value_type*) &prec->rval; }
	static bool read (traits_type* epicsrec, plc::BaseRecord* baserec) noexcept {
		return baserec->UserRead (*val (epicsrec)); }
	static bool write (plc::BaseRecord* baserec, traits_type* epicsrec) noexcept {
		return baserec->UserWrite (*val (epicsrec)); }
};

/// Epics traits class specialization for stringin record
template<>
struct epics_record_traits<epics_record_enum::stringinval>
{
	using traits_type = stringinRecord;
	using value_type = char[40];
	static const char* const name () noexcept { return "stringinval"; };
    static const int value_conversion = 0;
	static const bool input_record = true;
	static const bool raw_record = false;
	static char* val (traits_type* prec) noexcept { return prec->val; }
	static bool read (traits_type* epicsrec, plc::BaseRecord* baserec) noexcept {
		return baserec->UserRead(val(epicsrec), sizeof(value_type)); }
	static bool write (plc::BaseRecord* baserec, traits_type* epicsrec) noexcept {
		return baserec->UserWrite(val(epicsrec), sizeof (value_type)); }
};

/// Epics traits class specialization for stringout record
template<>
struct epics_record_traits<epics_record_enum::stringoutval>
{
	using traits_type = stringoutRecord;
	using value_type = char[40];
	static const char* const name () noexcept { return "stringoutval"; };
    static const int value_conversion = 0;
	static const bool input_record = false;
	static const bool raw_record = false;
	static char* val (traits_type* prec) noexcept { return prec->val; }
	static bool read (traits_type* epicsrec, plc::BaseRecord* baserec) noexcept {
		return baserec->UserRead(val(epicsrec), sizeof(value_type)); }
	static bool write (plc::BaseRecord* baserec, traits_type* epicsrec) noexcept {
		return baserec->UserWrite(val(epicsrec), sizeof(value_type)); }
};

/// Epics traits class specialization for long stringin record
template<>
struct epics_record_traits<epics_record_enum::lsival>
{
	using traits_type = lsiRecord;
	using value_type = char*;
	static const char* const name() noexcept { return "lsival"; };
	static const int value_conversion = 0;
	static const bool input_record = true;
	static const bool raw_record = false;
	static char* val(traits_type* prec) noexcept { return prec->val; }
	static bool read(traits_type* epicsrec, plc::BaseRecord* baserec) noexcept {
		const bool succ = baserec->UserRead(val(epicsrec), epicsrec->sizv);
		if (succ) epicsrec->len = (epicsUInt32)strnlen(val(epicsrec), epicsrec->sizv) + 1;
		return succ;
	}
	static bool write(plc::BaseRecord* baserec, traits_type* epicsrec) noexcept {
		return baserec->UserWrite(val(epicsrec), epicsrec->sizv);
	}
};

/// Epics traits class specialization for long stringout record
template<>
struct epics_record_traits<epics_record_enum::lsoval>
{
	using traits_type = lsoRecord;
	using value_type = char*;
	static const char* const name() noexcept { return "lsoval"; };
	static const int value_conversion = 0;
	static const bool input_record = false;
	static const bool raw_record = false;
	static char* val(traits_type* prec) noexcept { return prec->val; }
	static bool read(traits_type* epicsrec, plc::BaseRecord* baserec) noexcept {
		const bool succ = baserec->UserRead(val(epicsrec), epicsrec->sizv);
		if (succ) epicsrec->len = (epicsUInt32)strnlen(val(epicsrec), epicsrec->sizv) + 1;
		return succ;
	}
	static bool write(plc::BaseRecord* baserec, traits_type* epicsrec) noexcept {
		return baserec->UserWrite(val(epicsrec), epicsrec->sizv);
	}
};

/// Epics traits class specialization for waveform record
template<>
struct epics_record_traits<epics_record_enum::waveformval>
{
	using traits_type = waveformRecord;
	using value_type = void*;
	static const char* const name () noexcept { return "waveformval"; };
    static const int value_conversion = 0;
	static const bool input_record = true;
	static const bool raw_record = false;
	static value_type* val (traits_type* prec) noexcept { return (value_type*) &prec->val; }
};

/// Epics traits class specialization for event record
template<>
struct epics_record_traits<epics_record_enum::eventval>
{
	using traits_type = eventRecord;
	using value_type = epicsUInt16;
	static const char* const name () noexcept { return "eventval"; };
    static const int value_conversion = 0;
	static const bool input_record = true;
	static const bool raw_record = false;
	static value_type* val (traits_type* prec) noexcept { return (value_type*) &prec->val; }
};

/// Epics traits class specialization for histogram record
template<>
struct epics_record_traits<epics_record_enum::histogramval>
{
	using traits_type = histogramRecord;
	using value_type = void*;
	static const char* const name () noexcept { return "histogramval"; };
    static const int value_conversion = 0;
	static const bool input_record = true;
	static const bool raw_record = false;
	static value_type* val (traits_type* prec) noexcept { return (value_type*) &prec->val; }
};

/* Initialization for I/O interrupts
	devTcDefIo<>::get_ioint_info
 ************************************************************************/
template <epics_record_enum RecType>
long devTcDefIo<RecType>::
	get_ioint_info (int cmd, rec_type_ptr prec, IOSCANPVT* ppvt) noexcept
{
    if(!prec || !prec->dpvt)
        return 1;

	const plc::BaseRecord* const pRecord = (const plc::BaseRecord*)(prec->dpvt);
	EpicsInterface* const epics = pRecord ? dynamic_cast<EpicsInterface*>(pRecord->get_userInterface()) : NULL;

	if (!epics) return 1;

    if(!cmd) {
		*ppvt = epics->ioscan();
	}
    else {
		epics->set_isCallback(FALSE);
	}
    return 0;
}

/* devTcDefIn<>::init_read_record
 ************************************************************************/
template <epics_record_enum RecType>
long devTcDefIn<RecType>::init_read_record (rec_type_ptr prec) noexcept
{
	// Make pointer to TCat record
    plc::BaseRecordPtr pRecord;
	// Check for valid EPICS record pointer
    if(!prec) {
        recGblRecordError(S_db_notFound, prec,
            "Fatal error: init_record record has NULL-pointer");
		std::ignore = getchar();
        exit(S_db_notFound);
    }
	// Check for inp.type
    if(!prec->inp.type) {
        recGblRecordError(S_db_badField, prec,
            "Fatal error: init_record INP field not initialized (It has value 0!!!)");
		std::ignore = getchar();
        exit(S_db_badField);
    }
	// Check for inp.type = INST_IO
    if(prec->inp.type != INST_IO) {
        recGblRecordError(S_db_badField, prec,
            "init_record Illegal INP field (INST_IO expected: "
            "check following in your db-file: field(DTYP,\"tcat\");field(IN,\"@tc://...\")");
        return S_db_badField;
    }
	// Check for scan field
    if(prec->scan <  SCAN_IO_EVENT) {
        recGblRecordError(S_db_badField, prec, "init_record Illegal SCAN field");
        return S_db_badField;
    }
    // Copy item name
    std::stringcase itemName = prec->inp.value.instio.string;
	// Disable records if link to TCat record fails
    if(!register_devsup::linkRecord (itemName, (dbCommon*)prec, pRecord)) {
        prec->pact = TRUE;     /* disable this record */
        return S_db_badField;
    }
	// Point EPICS record to internal record entry
    prec->dpvt = (void*) pRecord.get();
	// Check for EPICS interface
	EpicsInterface* epics = dynamic_cast<EpicsInterface*>(pRecord->get_userInterface());
	if (!epics) {
		prec->pact = TRUE;
		recGblRecordError(S_db_badField, prec,
            "Fatal error: Internal record does not have an EPICS interface!");
		std::ignore = getchar();
        exit(S_db_badField);
	}
	// Set scan properties
	pRecord->set_access_rights(plc::access_rights_enum::read_only);
    if(prec->scan == SCAN_IO_EVENT) {
		// Set properties for a read record with SCAN = I/O Intr
		scanIoInit(&(epics->ioscan()));
		scanIoSetComplete(epics->get_ioscan(), (io_scan_complete)complete_io_scan, (void*)epics);
		epics->set_isCallback(true); // need to generate interrupt
		epics->set_isPassive(false);
	}
	else {
		// Set properties for other read records
		epics->set_isCallback(false); // do not need to generate interrupt
		epics->set_ioscan(nullptr);
	}
    
	return 0;
}

/* devTcDefOut<>::init_write_record
 ************************************************************************/
template <epics_record_enum RecType>
long devTcDefOut<RecType>::init_write_record (rec_type_ptr prec) noexcept
{
    // Make pointer to TCat record
    plc::BaseRecordPtr pRecord;
	// Check for valid EPICS record pointer
    if(!prec) {
        recGblRecordError(S_db_notFound, prec,
            "Fatal error: init_record record has NULL-pointer");
		std::ignore = getchar();
        exit(S_db_notFound);
    }
	// Check for out.type
    if( prec->out.type == 0 ) {
        recGblRecordError(S_db_badField, prec,
            "Fatal error: init_record OUT field not initialized (It has value 0!!!)");
		std::ignore = getchar();
        exit(S_db_badField);
    }
	// Check for out.type = INST_IO
    if( prec->out.type != INST_IO ) {
        recGblRecordError(S_db_badField, prec,
            "init_record Illegal OUT field (INST_IO expected: "
            "check following in your db-file: field(DTYP,\"tcat\");field(OUT,\"@tc://...\")");
        return S_db_badField;
    }
	// Copy item name
    std::stringcase itemName = prec->out.value.instio.string; 
	// Disable records if link to TCat record fails
    if(!register_devsup::linkRecord(itemName, (dbCommon*)prec, pRecord)) {
        prec->pact = TRUE;     /* disable this record */
        return S_db_badField;
    }
	// Point EPICS record to internal record entry
    prec->dpvt = (void*) pRecord.get();
	// Check for EPICS interface
	EpicsInterface* epics = dynamic_cast<EpicsInterface*>(pRecord->get_userInterface());
	if (!epics) {
		recGblRecordError(S_db_badField, prec,
            "Fatal error: Internal record does not have an EPICS interface!");
		std::ignore = getchar();
        exit(S_db_badField);
	}
	// Set scan properties
	pRecord->set_access_rights(plc::access_rights_enum::read_write);
	epics->set_isCallback(true); // readwrite record: need to generate callback to do a read
	epics->set_isPassive(true);
	// Set parameters for generating callbacks
	callbackSetProcess(&(epics->callback()), priorityHigh, prec);

	return 0;
}

/* devTcDefWaveformIn<>::init_read_waveform_record
 ************************************************************************/
template <epics_record_enum RecType>
long devTcDefWaveformIn<RecType>::
	init_read_waveform_record (rec_type_ptr precord) noexcept
{
    constexpr int iInit = 0;
    if(!precord) {
        recGblRecordError(S_db_notFound, precord,
            "Fatal error: init_record record has NULL-pointer");
		std::ignore = getchar();
        exit(S_db_notFound);
    }
    if( precord->inp.type == 0 ) {
        recGblRecordError(S_db_badField, (void*)precord,
            "Fatal error: init_record INP field not initialized (It has value 0!!!)");
        precord->pact = TRUE;     // disable this record
		std::ignore = getchar();
        exit(S_db_badField);
    }
    if( precord->inp.type != INST_IO ) {
        recGblRecordError(S_db_badField, (void*)precord,
            "init_record Illegal INP field (INST_IO expected: "
            "check following in your db-file: field(DTYP,\"tcat\");field(IN,\"@tc://...\")");
        precord->pact = TRUE;     // disable this record
        return S_db_badField;
    }
    if( precord->scan <  SCAN_IO_EVENT  ) {
        recGblRecordError(S_db_badField, (void*)precord, "init_record Illegal SCAN field");
        precord->pact = TRUE;     // disable this record
        return S_db_badField;
    }
    //opcItemName = precord->inp.value.instio.string;
    //iInit = opcInitItem(opcItemName,(dbCommon*)precord,&pOpc2Epics);
    //if(!pOpc2Epics) {
    //    recGblRecordError(S_db_noMemory,precord,"OPC-item interface has not been not initialized!");
    //    return S_db_noMemory;
    //}
    //pOpc2Epics->pRecVal=precord->bptr;
    //pOpc2Epics->recType=waveformval;
    //pOpc2Epics->nelm = precord->nelm;
    //pOpc2Epics->nord = & precord->nord;
    //precord->dpvt = (void*)pOpc2Epics;
    if(precord->scan == SCAN_IO_EVENT) {
        //scanIoInit(&(pOpc2Epics->ioscanpvt));
        //pOpc2Epics->isCallback = 1;
    } else {
        //pOpc2Epics->isCallback = 0;
        //pOpc2Epics->ioscanpvt  = 0;
    }
    if(!iInit) {
        precord->pact = TRUE;     // disable this record
        return S_dev_Conflict;
    }
    return 0;
}



/* devTcDefIn<>::read
 ************************************************************************/
template <epics_record_enum RecType>
long devTcDefIn<RecType>::read(rec_type_ptr precord) noexcept
{
	// Get the conversion setting for this record
	constexpr long ret = epics_record_traits<RecType>::value_conversion;
	// Get the IOC internal record entry and EPICS user interface
	plc::BaseRecord* pBaseRecord = (plc::BaseRecord*)precord->dpvt;
	const EpicsInterface* epics = pBaseRecord ? dynamic_cast<EpicsInterface*>(pBaseRecord->get_userInterface()) : NULL;

	if (!pBaseRecord || !epics) {
		recGblRecordError(S_dev_noDeviceFound, precord, "unable to get device interface");
		precord->pact = TRUE;     // disable this record
		return 1;
	}
	// Check the "processing active" field
#ifdef _MSC_VER 
	if (_InterlockedCompareExchange8((char*)&precord->pact, TRUE, FALSE)) {
		return ret;
	}
#else
	if (precord->pact) {
		return ret;
	}
	precord->pact = TRUE;
#endif
	// check validity
	bool udf = false;
	// Check data valid 
	if (pBaseRecord->DataIsValid ()) {
		precord->nsev = NO_ALARM;
		precord->nsta = NO_ALARM;
	}
	else {
		recGblSetSevr (precord, READ_ALARM, INVALID_ALARM);
		udf = true;
	}
	// Grab data value into EPICS 
	epics_record_traits<RecType>::read (precord, pBaseRecord);
	// set time stamp
	const plc::BaseRecord::time_type timestamp = pBaseRecord->get_timestamp();
	try {
		precord->time = epicsTime(*((_FILETIME*)&timestamp));
	}
	catch(...) {
		precord->time = epicsTime();
	};

	precord->udf = udf;
	precord->pact = FALSE;
    return ret;
}

/* devTcDefIn<aaival>::read
 ************************************************************************/
//template <>
//long devTcDefIn<aaival>::read (rec_type_ptr precord);

/* devTcDefOut<>::write
 ************************************************************************/
template <epics_record_enum RecType>
long devTcDefOut<RecType>::write (rec_type_ptr precord) noexcept
{
	// Get the IOC internal record entry and EPICS user interface
	plc::BaseRecord* pBaseRecord = (plc::BaseRecord*) precord->dpvt;
	const EpicsInterface* epics = pBaseRecord ? dynamic_cast<EpicsInterface*>( pBaseRecord->get_userInterface() ) : NULL;

    if(!pBaseRecord || !epics) {
        recGblRecordError(S_dev_noDeviceFound, precord, "unable to get device interface");
        precord->pact = TRUE;     // disable this record
        return 1;
    }

	// Check the "processing active" field
#ifdef _MSC_VER 
	if (_InterlockedCompareExchange8((char*)&precord->pact, TRUE, FALSE)) {
		return 0;
	}
#else
	if (precord->pact) {
		return 0;
	}
	precord->pact = TRUE;
#endif

	// For in/out records, check if read is pending
	bool udf = false;
	if (epics->get_callbackRequestPending()) {
		// Check data valid 
		if (pBaseRecord->DataIsValid ()) {
			//recGblSetSevr (precord, NO_ALARM, NO_ALARM);
			precord->nsev = NO_ALARM;
			precord->nsta = NO_ALARM;
		}
		else {
			recGblSetSevr (precord, WRITE_ALARM, INVALID_ALARM);
			udf = true;
		}
		// Read data value
		epics_record_traits<RecType>::read (precord, pBaseRecord);
		// set time stamp
		const plc::BaseRecord::time_type timestamp = pBaseRecord->get_timestamp();
		try {
			precord->time = epicsTime(*((_FILETIME*)&timestamp));
		}
		catch (...) {
			precord->time = epicsTime();
		};
	}
	else {
		// Write data value
		epics_record_traits<RecType>::write (pBaseRecord, precord);
		// set time stamp
		try {
			precord->time = epicsTime::getCurrent();
		}
		catch (...) {
			precord->time = epicsTime();
		};
	}

	precord->udf = udf;
	precord->pact = FALSE;
    return 0;
}

/* devTcDefIn<>::read_waveform
 ************************************************************************/
template <epics_record_enum RecType>
long devTcDefWaveformIn<RecType>::read_waveform (rec_type_ptr precord) noexcept
{
	constexpr long ret = epics_record_traits<RecType>::value_conversion;
    //long ret = recPropStruct::prop[record_type].ret;
    //OpcToEpics* pOpc2Epics = (OpcToEpics*)precord->dpvt;
    //long ret = recProp[pOpc2Epics->recType].ret;
    //if(!pOpc2Epics) {
    //    recGblRecordError(S_dev_noDeviceFound, (void *)precord, "unable to get device");
    //    precord->pact = TRUE;     // disable this record
    //    return ret;
    //}
    if(precord->pact) {
        return ret;
	}
    //if(opcGetScalar(pOpc2Epics)) {
    //    return ret;
	//}
    precord->pact = FALSE;
    precord->udf  = FALSE;
    return ret;
}
/** @} */

}
