/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_UpdateMsgEncoder_h
#define __refinitiv_ema_access_UpdateMsgEncoder_h

#include "MsgEncoder.h"
#include "UpdateMsg.h"

namespace refinitiv {

namespace ema {

namespace access {

class UpdateMsgEncoder : public MsgEncoder
{
public :

	UpdateMsgEncoder();

	virtual ~UpdateMsgEncoder();

	void clear();

	void domainType( UInt8 );

	void streamId( Int32 );

	void serviceId( UInt16 );

	void name( const EmaString& );

	void nameType( UInt8 );

	void filter( UInt32 );

	void addFilter( UInt32 );

	void identifier( Int32 );

	void updateTypeNum( UInt8 );

	void seqNum( UInt32 );

	void publisherId( UInt32 , UInt32 );

	void conflated( UInt16 count, UInt16 time );

	void permissionData( const EmaBuffer& );

	void payload( const ComplexType& );

	void attrib( const ComplexType& );

	void extendedHeader( const EmaBuffer& );

	void doNotCache( bool );

	void doNotConflate( bool );

	void doNotRipple( bool );

	bool hasServiceId() const;

	RsslUpdateMsg* getRsslUpdateMsg() const;

private :

	void clearRsslUpdateMsg();

	RsslMsg* getRsslMsg() const;

	mutable RsslUpdateMsg	_rsslUpdateMsg;

#ifdef __EMA_COPY_ON_SET__
	EmaBuffer				_permissionData;
	bool					_permissionDataSet;
#else
	const EmaBuffer*		_pPermissionData;
#endif

	UInt8					_updateTypeNum;
	UInt8					_nameType;
	UInt8					_domainType;
	UInt16					_serviceId;
	UInt32					_filter;
	Int32					_identifier;
	Int32					_streamId;
	UInt32					_sequenceNum;
	UInt16					_conflatedCount;
	UInt16					_conflatedTime;
	UInt32					_userId;
	UInt32					_userAddress;

	bool					_identifierSet;
	bool					_filterSet;
	bool					_nameTypeSet;
	bool					_serviceIdSet;
	bool					_sequenceNumSet;
	bool					_conflatedSet;
	bool					_publisherIdSet;
	bool					_doNotCache;
	bool					_doNotConflate;
	bool					_doNotRipple;
};

class UpdateMsgEncoderPool : public EncoderPool< UpdateMsgEncoder >
{
public :

	UpdateMsgEncoderPool( unsigned int size = 5 ) : EncoderPool< UpdateMsgEncoder >( size ) {};

	virtual ~UpdateMsgEncoderPool() {}

private :

	UpdateMsgEncoderPool( const UpdateMsgEncoderPool& );
	UpdateMsgEncoderPool& operator=( const UpdateMsgEncoderPool& );
};

}

}

}

#endif // __refinitiv_ema_access_UpdateMsgEncoder_h
