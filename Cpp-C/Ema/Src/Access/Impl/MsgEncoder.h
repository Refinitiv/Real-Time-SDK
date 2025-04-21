/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019, 2024-2025 LSEG. All rights reserved.             --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_MsgEncoder_h
#define __refinitiv_ema_access_MsgEncoder_h

#include "Encoder.h"
#include "EmaPool.h"
#include "EmaBuffer.h"
#include "rtr/rsslMsg.h"

namespace refinitiv {

namespace ema {

namespace access {

class ComplexType;

class MsgEncoder : public Encoder
{
public :

	MsgEncoder();

	virtual ~MsgEncoder();

	void clear();

	void release();

	bool ownsIterator() const;

	virtual void domainType( UInt8 ) = 0;

	virtual void streamId( Int32 ) = 0;

	virtual void serviceId( UInt16 ) = 0;

	virtual void name( const EmaString& ) = 0;

	virtual void nameType( UInt8 ) = 0;

	virtual void filter( UInt32 ) = 0;

	virtual void addFilter( UInt32 ) = 0;

	virtual void identifier( Int32 ) = 0;

	virtual void payload( const ComplexType& ) = 0;

	virtual void attrib( const ComplexType& ) = 0;

	virtual void serviceName( const EmaString& );

	virtual void serviceListName(const EmaString&);

	virtual bool hasServiceId() const = 0;

	virtual const EmaString& getServiceName() const;

	virtual const EmaString& getServiceListName() const;

	virtual bool hasServiceName() const;

	virtual bool hasServiceListName() const;

	virtual bool hasName() const;

	bool isComplete() const;

	RsslBuffer& getRsslBuffer() const;

protected :

	virtual RsslMsg* getRsslMsg() const = 0;

#ifdef __EMA_COPY_ON_SET__
	EmaString			_name;
	EmaString			_serviceName;
	EmaString			_serviceListName;

	EmaBuffer			_attrib;
	EmaBuffer			_payload;
	EmaBuffer			_extendedHeader;

	bool				_nameSet;
	bool				_serviceNameSet;
	bool				_serviceListNameSet;
	bool				_extendedHeaderSet;
#else
	const EmaString*	_pName;
	const EmaString*	_pServiceName;
	const EmaString*	_pServiceListName;
	RsslBuffer*			_pAttrib;
	RsslBuffer*			_pPayload;
	const EmaBuffer*	_pExtendedHeader;
#endif

	RsslContainerType	_attribDataType;
	RsslContainerType	_payloadDataType;

private :

	void endEncodingEntry() const;

	friend class PackedMsgImpl;
};

}

}

}

#endif // __refinitiv_ema_access_MsgEncoder_h
