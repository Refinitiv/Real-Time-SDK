/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019, 2024 LSEG. All rights reserved.             --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_Encoder_h
#define __refinitiv_ema_access_Encoder_h

#include "Data.h"
#include "rtr/rsslDataTypeEnums.h"
#include "EncodeIterator.h"

namespace refinitiv {

namespace ema {

namespace access {

class RefreshMsg;
class AckMsg;
class UpdateMsg;
class PostMsg;
class StatusMsg;
class ReqMsg;
class RefreshMsg;
class GenericMsg;
}

namespace rdm  {

class DataDictionaryImpl;
}

namespace access {

class EmaString;

class Encoder
{
public :

	virtual void clear();

	virtual void release() = 0;

	bool ownsIterator() const;

	void passEncIterator( Encoder& );

	virtual RsslBuffer& getRsslBuffer() const;

	RsslUInt8 getMajVer() const;

	RsslUInt8 getMinVer() const;

	virtual void endEncodingEntry() const = 0;

	UInt8 convertDataType( DataType::DataTypeEnum ) const;

	virtual bool isComplete() const;

protected :

	friend class refinitiv::ema::rdm::DataDictionaryImpl;
	friend class refinitiv::ema::access::AckMsg;
	friend class refinitiv::ema::access::UpdateMsg;
	friend class refinitiv::ema::access::PostMsg;
	friend class refinitiv::ema::access::StatusMsg;
	friend class refinitiv::ema::access::ReqMsg;
	friend class refinitiv::ema::access::RefreshMsg;
	friend class refinitiv::ema::access::GenericMsg;

	Encoder();

	virtual ~Encoder();

	void acquireEncIterator(UInt32 allocatedSize = 4096);

	void releaseEncIterator();

	void clearEncIterator();

	bool hasEncIterator() const;

	EncodeIterator*		_pEncodeIter;
	EncodeIterator*		_pEncodeIterCached;

	const Encoder*		_iteratorOwner;

	bool				_containerComplete;

	/**
	* @brief Internal iterator states. Copied from ./Impl/Codec/rtr/rsslIteratorUtilsInt.h
	*/

	typedef enum
	{
		RSSL_EIS_NONE = 0,
		RSSL_EIS_SET_DEFINITIONS = 1,
		RSSL_EIS_SUMMARY_DATA = 2,
		RSSL_EIS_SET_DATA = 3,
		RSSL_EIS_SET_ENTRY_INIT = 4,
		RSSL_EIS_PRIMITIVE = 5,
		RSSL_EIS_PRIMITIVE_U15 = 6,
		RSSL_EIS_ENTRIES = 7,
		RSSL_EIS_ENTRY_INIT = 8,
		RSSL_EIS_ENTRY_WAIT_COMPLETE = 9,
		RSSL_EIS_SET_ENTRY_WAIT_COMPLETE = 10,
		RSSL_EIS_EXTENDED_HEADER = 11,
		RSSL_EIS_OPAQUE = 12,
		RSSL_EIS_OPAQUE_AND_EXTENDED_HEADER = 13,
		RSSL_EIS_WAIT_COMPLETE = 14,
		RSSL_EIS_COMPLETE = 15,
		RSSL_EIS_NON_RWF_DATA = 16,
		RSSL_EIS_REQATTRIB = 17,
		RSSL_EIS_OPAQUE_REQATTRIB = 18,
		RSSL_EIS_EXTENDED_HEADER_REQATTRIB = 19,
		RSSL_EIS_OPAQUE_EXTENDED_HEADER_REQATTRIB = 20
	}EncodeIteratorStates;
};

}

}

}

#endif // __refinitiv_ema_access_Encoder_h
