/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_GenericMsgEncoder_h
#define __thomsonreuters_ema_access_GenericMsgEncoder_h

#include "MsgEncoder.h"

namespace thomsonreuters {

namespace ema {

namespace access {

class GenericMsgEncoder : public MsgEncoder
{
public :

	GenericMsgEncoder();

	virtual ~GenericMsgEncoder();

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

	void secondarySeqNum( UInt32 );

	void partNum( UInt16 );

	void extendedHeader( const EmaBuffer& );

	void permissionData( const EmaBuffer& );

	void complete( bool );

	bool hasServiceId() const;

	RsslGenericMsg* getRsslGenericMsg() const;

private :

	RsslMsg* getRsslMsg() const;

#ifdef __EMA_COPY_ON_SET__
	EmaBuffer				_permissionData;
#endif
	RsslGenericMsg			_rsslGenericMsg;
};

class GenericMsgEncoderPool : public EncoderPool< GenericMsgEncoder >
{
public :

	GenericMsgEncoderPool( unsigned int size = 5 ) : EncoderPool< GenericMsgEncoder >( size ) {};

	virtual ~GenericMsgEncoderPool() {}

private :

	GenericMsgEncoderPool( const GenericMsgEncoderPool& );
	GenericMsgEncoderPool& operator=( const GenericMsgEncoderPool& );
};

}

}

}

#endif // __thomsonreuters_ema_access_GenericMsgEncoder_h
