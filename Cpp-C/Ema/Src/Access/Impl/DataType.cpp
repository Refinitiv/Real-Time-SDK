/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
 *|-----------------------------------------------------------------------------
 */

#include "DataType.h"

using namespace refinitiv::ema::access;

extern const EmaString& getDTypeAsString( DataType::DataTypeEnum dType );

DataType::DataType( DataTypeEnum dType ) :
 _dataType( dType )
{
}

DataType::~DataType()
{
}

const EmaString& DataType::toString() const
{
	return getDTypeAsString( _dataType );
}

DataType::operator const char* () const
{
	return toString().c_str();
}
