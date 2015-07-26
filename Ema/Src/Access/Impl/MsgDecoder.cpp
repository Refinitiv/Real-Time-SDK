/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "MsgDecoder.h"
#include "StaticDecoder.h"

using namespace thomsonreuters::ema::access;

MsgDecoder::MsgDecoder() :
 _pRsslDictionary( 0 ),
 _attrib(),
 _payload()
{
}

MsgDecoder::~MsgDecoder()
{
	StaticDecoder::morph( &_payload, DataType::NoDataEnum );
	StaticDecoder::morph( &_attrib, DataType::NoDataEnum );
}

void MsgDecoder::setAtExit()
{
}

const Data& MsgDecoder::getAttribData() const
{
	return _attrib;
}

const Data& MsgDecoder::getPayloadData() const
{
	return _payload;
}
