/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __rtsdk_ema_access_PostMsgEncoder_h
#define __rtsdk_ema_access_PostMsgEncoder_h

#include "MsgEncoder.h"

namespace rtsdk {

namespace ema {

namespace access {

class PostMsgEncoder : public MsgEncoder
{
public :

	PostMsgEncoder();

	virtual ~PostMsgEncoder();

	void clear();

	void domainType( UInt8 );

	void streamId( Int32 );

	void serviceId( UInt16 );

	void name( const EmaString& );

	void nameType( UInt8 );

	void filter( UInt32 );

	void addFilter( UInt32 );

	void identifier( Int32 );

	void payload( const ComplexType& );

	void attrib( const ComplexType& );

	void seqNum( UInt32 );

	void partNum( UInt16 );

	void postUserRights( UInt16 );

	void postId( UInt32 );

	void publisherId( UInt32, UInt32 );

	void solicitAck( bool );

	void extendedHeader( const EmaBuffer& );

	void permissionData( const EmaBuffer& );

	void complete( bool );

	bool hasServiceId() const;

	RsslPostMsg* getRsslPostMsg() const;

private :

	RsslMsg* getRsslMsg() const;

#ifdef __EMA_COPY_ON_SET__
	EmaBuffer			_permissionData;
	bool				_permissionDataSet;
#else
	const EmaBuffer*	_pPermissionData;
#endif
	RsslPostMsg			_rsslPostMsg;
};

class PostMsgEncoderPool : public EncoderPool< PostMsgEncoder >
{
public :

	PostMsgEncoderPool( unsigned int size = 5 ) : EncoderPool< PostMsgEncoder >( size ) {};

	virtual ~PostMsgEncoderPool() {}

private :

	PostMsgEncoderPool( const PostMsgEncoderPool& );
	PostMsgEncoderPool& operator=( const PostMsgEncoderPool& );
};

}

}

}

#endif // __rtsdk_ema_access_PostMsgEncoder_h
