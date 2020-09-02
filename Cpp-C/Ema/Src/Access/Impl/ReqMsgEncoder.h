/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_ReqMsgEncoder_h
#define __thomsonreuters_ema_access_ReqMsgEncoder_h

#include "MsgEncoder.h"

namespace rtsdk {

namespace ema {

namespace access {

class ReqMsgEncoder : public MsgEncoder
{
public :

	ReqMsgEncoder();

	virtual ~ReqMsgEncoder();

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

	void priority( UInt8 , UInt16 );

	void extendedHeader( const EmaBuffer& );

	void qos( UInt32 timeliness, UInt32 rate );

	void initialImage( bool initialImage );

	void interestAfterRefresh( bool interestAfterRefresh );

	void pause( bool pause );

	void conflatedInUpdates( bool conflatedInUpdates );

	void privateStream( bool privateStream );

	bool hasServiceId() const;

	RsslRequestMsg* getRsslRequestMsg() const;

	UInt32 getBatchItemListSize() const;

	bool getPrivateStream() const;

	bool isDomainTypeSet() const;

private :

	void checkBatchView( RsslBuffer* );

	RsslMsg* getRsslMsg() const;

	RsslRequestMsg			_rsslRequestMsg;

	bool					_domainTypeSet;
};

class ReqMsgEncoderPool : public EncoderPool< ReqMsgEncoder >
{
public :

	ReqMsgEncoderPool( unsigned int size = 5 ) : EncoderPool< ReqMsgEncoder >( size ) {};

	virtual ~ReqMsgEncoderPool() {}

private :

	ReqMsgEncoderPool( const ReqMsgEncoderPool& );
	ReqMsgEncoderPool& operator=( const ReqMsgEncoderPool& );
};

}

}

}

#endif // __thomsonreuters_ema_access_ReqMsgEncoder_h
