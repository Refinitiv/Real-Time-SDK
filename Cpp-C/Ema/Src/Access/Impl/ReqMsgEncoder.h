/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019, 2024-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_ReqMsgEncoder_h
#define __refinitiv_ema_access_ReqMsgEncoder_h

#include "MsgEncoder.h"

namespace refinitiv {

namespace ema {

namespace access {

class ReqMsgEncoder : public MsgEncoder
{
public :

	ReqMsgEncoder();

	virtual ~ReqMsgEncoder();

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

	// Fills the EmaVector with the batch item names, if found.
	// Returns the number of items in the batch request
	UInt32 getBatchItemList(EmaVector<EmaString>*) const;

	//Fills the EmaBuffer with the View elements.
	// This will only get the view out of the payload, not any batch item names.
	bool getViewPayload(EmaBuffer&) const;


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

#endif // __refinitiv_ema_access_ReqMsgEncoder_h
