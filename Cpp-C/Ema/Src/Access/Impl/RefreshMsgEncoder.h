/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_RefreshMsgEncoder_h
#define __thomsonreuters_ema_access_RefreshMsgEncoder_h

#include "MsgEncoder.h"
#include "RefreshMsg.h"

namespace thomsonreuters {

namespace ema {

namespace access {

class RefreshMsgEncoder : public MsgEncoder
{
public :

	RefreshMsgEncoder();

	virtual ~RefreshMsgEncoder();

	void clear();

	void domainType( UInt8 );

	void streamId( Int32 );

	void serviceId( UInt16 );

	void name( const EmaString& );

	void nameType( UInt8 );

	void filter( UInt32 );

	void addFilter( UInt32 );

	void identifier( Int32 );

	void qos( UInt32 timeliness , UInt32 rate );

	void state( OmmState::StreamState , OmmState::DataState , UInt8 , const EmaString& );

	void seqNum( UInt32 );

	void partNum( UInt16 );

	void publisherId( UInt32 , UInt32 );

	void conflated( UInt16 count, UInt16 time );

	void itemGroup( const EmaBuffer& );

	void permissionData( const EmaBuffer& );

	void payload( const ComplexType& );

	void attrib( const ComplexType& );

	void extendedHeader( const EmaBuffer& );

	void solicited( bool );

	void doNotCache( bool );

	void clearCache( bool );

	void complete( bool );

	void doNotRipple( bool );

	void privateStream( bool );

	bool hasServiceId() const;

	RsslRefreshMsg* getRsslRefreshMsg() const;

private :

	void clearRsslRefreshMsg();

	RsslMsg* getRsslMsg() const;

	mutable RsslRefreshMsg	_rsslRefreshMsg;

#ifdef __EMA_COPY_ON_SET__
	EmaBuffer				_permissionData;
	mutable EmaBuffer			_itemGroup;
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
	UInt32					_sequenceNum;
	UInt16					_partNum;
	UInt16					_conflatedCount;
	UInt16					_conflatedTime;
	UInt32					_userId;
	UInt32					_userAddress;
	UInt32					_timeliness;
	UInt32					_rate;
	UInt8					_streamState;
	UInt8					_dataState;
	UInt8					_stateCode;

	bool					_identifierSet;
	bool					_filterSet;
	bool					_nameTypeSet;
	bool					_serviceIdSet;
	bool					_sequenceNumSet;
	bool					_partNumSet;
	bool					_conflatedSet;
	bool					_publisherIdSet;
	bool					_doNotCache;
	bool					_solicited;
	bool					_clearCache;
	bool					_complete;
	bool					_doNotRipple;
	bool					_privateStream;
	bool					_qosSet;
	bool					_stateSet;
};

class RefreshMsgEncoderPool : public EncoderPool< RefreshMsgEncoder >
{
public :

	RefreshMsgEncoderPool( unsigned int size = 5 ) : EncoderPool< RefreshMsgEncoder >( size ) {};

	virtual ~RefreshMsgEncoderPool() {}

private :

	RefreshMsgEncoderPool( const RefreshMsgEncoderPool& );
	RefreshMsgEncoderPool& operator=( const RefreshMsgEncoderPool& );
};

}

}

}

#endif // __thomsonreuters_ema_access_RefreshMsgEncoder_h
