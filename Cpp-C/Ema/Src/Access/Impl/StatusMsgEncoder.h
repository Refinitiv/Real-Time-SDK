/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015,2019-2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_StatusMsgEncoder_h
#define __refinitiv_ema_access_StatusMsgEncoder_h

#include "MsgEncoder.h"
#include "StatusMsg.h"

namespace refinitiv {

namespace ema {

namespace access {

class StatusMsgEncoder : public MsgEncoder
{
public :

	StatusMsgEncoder();

	virtual ~StatusMsgEncoder();

	void clear();

	void release();

	void domainType( UInt8 );

	void streamId( Int32 );

	void serviceId( UInt16 );

	void name( const EmaString& );

	void nameType( UInt8 );

	void filter( UInt32 );

	void addFilter( UInt32 );

	void identifier( Int32 );

	void state( OmmState::StreamState , OmmState::DataState , UInt8 , const EmaString& );

	void publisherId( UInt32 , UInt32 );

	void conflated( UInt16 count, UInt16 time );

	void itemGroup( const EmaBuffer& );

	void permissionData( const EmaBuffer& );

	void payload( const ComplexType& );

	void attrib( const ComplexType& );

	void extendedHeader( const EmaBuffer& );

	void clearCache( bool );

	void privateStream( bool );

	bool hasServiceId() const;

	RsslStatusMsg* getRsslStatusMsg() const;

private :

	void clearRsslStatusMsg();

	RsslMsg* getRsslMsg() const;

	mutable RsslStatusMsg	_rsslStatusMsg;

#ifdef __EMA_COPY_ON_SET__
	EmaBuffer				_permissionData;
	EmaBuffer				_itemGroup;
	EmaString				_statusText;
	bool					_permissionDataSet;
	bool					_itemGroupSet;
#else
	const EmaBuffer*		_pPermissionData;
	const EmaBuffer*		_pItemGroup;
	const EmaString*		_pStatusText;
#endif

	UInt8					_nameType;
	UInt8					_domainType;
	UInt16					_serviceId;
	UInt32					_filter;
	Int32					_identifier;
	Int32					_streamId;
	UInt32					_userId;
	UInt32					_userAddress;
	UInt8					_streamState;
	UInt8					_dataState;
	UInt8					_stateCode;

	bool					_identifierSet;
	bool					_filterSet;
	bool					_nameTypeSet;
	bool					_serviceIdSet;
	bool					_publisherIdSet;
	bool					_clearCache;
	bool					_privateStream;
	bool					_stateSet;
};

class StatusMsgEncoderPool : public EncoderPool< StatusMsgEncoder >
{
public :

	StatusMsgEncoderPool( unsigned int size = 5 ) : EncoderPool< StatusMsgEncoder >( size ) {};

	virtual ~StatusMsgEncoderPool() {}

private :

	StatusMsgEncoderPool( const StatusMsgEncoderPool& );
	StatusMsgEncoderPool& operator=( const StatusMsgEncoderPool& );
};

}

}

}

#endif // __refinitiv_ema_access_StatusMsgEncoder_h
