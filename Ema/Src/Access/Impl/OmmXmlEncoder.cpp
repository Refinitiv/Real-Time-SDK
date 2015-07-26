/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmXmlEncoder.h"

using namespace thomsonreuters::ema::access;

OmmXmlEncoder::OmmXmlEncoder()
{
}

OmmXmlEncoder::~OmmXmlEncoder()
{
}

void OmmXmlEncoder::set( const EmaBuffer& value )
{
	encodeBuffer( value.c_buf(), value.length() );
}

void OmmXmlEncoder::set( const EmaString& value )
{
	encodeBuffer( value.c_str(), value.length() );
}
