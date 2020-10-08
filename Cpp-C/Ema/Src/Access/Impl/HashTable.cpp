/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "HashTable.h"

using namespace refinitiv::ema::access;

template<>
size_t Hasher<int>::operator()( const int& val ) const
{
	return val;
}

template<>
size_t Hasher<EmaString>::operator()( const EmaString& val ) const
{
	size_t result = 0;
	size_t magic = 8388593; // the greatest prime lower than 2^23

	const char* s = val.c_str();
	UInt32 n = val.length();
	while (n--)
		result = ((result % magic) << 8) + (size_t)* s++;
	return result;
}

template<>
bool Equal_To<int>::operator()( const int& x, const int& y ) const
{
	return x == y ? true : false;
}

template<>
bool Equal_To<EmaString>::operator()( const EmaString& x, const EmaString& y ) const
{
	return x == y ? true : false;
}
