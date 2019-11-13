/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_AckMsgEncoder_h
#define __thomsonreuters_ema_access_AckMsgEncoder_h

#include "MsgEncoder.h"
#include "AckMsg.h"

namespace thomsonreuters {

namespace ema {

namespace access {

class AckMsgEncoder : public MsgEncoder
{
public :

	AckMsgEncoder();

	virtual ~AckMsgEncoder();

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

	void ackId( UInt32 );

	void nackCode( UInt8 );

	void text( const EmaString& );

	void extendedHeader( const EmaBuffer& );

	void privateStream( bool );

	bool hasServiceId() const;

	RsslAckMsg* getRsslAckMsg() const;

private :

	RsslMsg* getRsslMsg() const;

#ifdef __EMA_COPY_ON_SET__
	EmaString			_text;
#endif
	RsslAckMsg			_rsslAckMsg;
};

class AckMsgEncoderPool : public EncoderPool< AckMsgEncoder >
{
public :

	AckMsgEncoderPool( unsigned int size = 5 ) : EncoderPool< AckMsgEncoder >( size ) {};

	virtual ~AckMsgEncoderPool() {}

private :

	AckMsgEncoderPool( const AckMsgEncoderPool& );
	AckMsgEncoderPool& operator=( const AckMsgEncoderPool& );
};

}

}

}

#endif // __thomsonreuters_ema_access_AckMsgEncoder_h
