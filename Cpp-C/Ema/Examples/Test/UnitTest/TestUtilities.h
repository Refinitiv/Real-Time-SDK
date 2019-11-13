/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|        Copyright (C) 2019 Refinitiv. All rights reserved.          --
 *|-----------------------------------------------------------------------------
 */

#ifndef __TestUtilities_h
#define __TestUtilities_h

#include "gtest/gtest.h"
#include "rtr/rsslDataDictionary.h"
#include "Access/Impl/StaticDecoder.h"
#include "rtr/rsslMsgDecoders.h"
#include "Ema.h"

/* These are user defined FIDs to be used in the example so that we can show types */
/* that are not in the standard dictionary. User defined FIDs are always negative  */
#define FID_INT			-1
#define FID_DATETIME	-4
#define FID_QOS			-5
#define FID_STATE		-6
#define FID_BUFFER		-7
#define FID_ENUM		-8
#define FID_FLOAT		-9
#define FID_DOUBLE		-10

namespace thomsonreuters {

namespace ema {

namespace access {

class EmaString;
class ElementList;
class FieldList;
class Map;
class EmaBuffer;

}

}

}

extern thomsonreuters::ema::access::EmaString g_userName;
extern thomsonreuters::ema::access::EmaString g_password;

void encodeFieldList( RsslBuffer& rsslBuf, thomsonreuters::ema::access::EmaString& inText );
void encodeNonRWFData( RsslBuffer* rsslBuf, RsslBuffer* value );
bool loadDictionaryFromFile( RsslDataDictionary* pDictionary );

void perfDecode( const thomsonreuters::ema::access::ElementList& el );
void perfDecode( const thomsonreuters::ema::access::FieldList& fl );

void RsslEncodeElementListAll( RsslBuffer& rsslBuf );
void RsslEncodeFieldListAll( RsslBuffer& rsslBuf );
void RsslEncodeMapAll( RsslBuffer& mapBuffer );

void EmaEncodeFieldListAll( thomsonreuters::ema::access::FieldList& fl );
void EmaEncodeElementListAll( thomsonreuters::ema::access::ElementList& el );
void EmaEncodeMapAll( thomsonreuters::ema::access::Map& map );

void EmaDecodeFieldListAll( const thomsonreuters::ema::access::FieldList& fl );
void EmaDecodeElementListAll( const thomsonreuters::ema::access::ElementList& el );
void EmaDecodeMapAll( const thomsonreuters::ema::access::Map& map );

bool comparingData(RsslBuffer& rsslBuffer, const thomsonreuters::ema::access::EmaString& emaString);

#endif // __TestUtilities_h
