/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "Access/Include/ItemStreamToken.h"

using namespace thomsonreuters::ema::access;

ItemStreamToken::ItemStreamToken() :
 _toString()
{
}

ItemStreamToken::~ItemStreamToken()
{

}

Int64 ItemStreamToken::getStreamId() const
{
	return _streamId;
}

const EmaString& ItemStreamToken::toString() const
{
	return toString( 0 );
}

const EmaString& ItemStreamToken::toString(  UInt64 indent ) const
{
	return _toString;
}

bool ItemStreamToken::hasServiceName() const
{
	return true;
}

const EmaString& ItemStreamToken::getServiceName() const
{
	return _toString;
}

bool ItemStreamToken::getPrivateStream() const
{
	return false;
}

