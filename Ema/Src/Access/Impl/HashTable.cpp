/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "HashTable.h"

using namespace thomsonreuters::ema::access;

template<>
size_t
Hasher<int>::operator()( const int & val ) const
{
  return val;
}

template<>
size_t
Hasher<EmaString>::operator()( const EmaString & val ) const
{
  size_t retVal = val[0ULL] % sizeRandom;
  const char *p(val.c_str());
  while(*p)
    retVal = retVal * 101 + *p++;
  return retVal;
}

template<>
bool
Equal_To<int>::operator()( const int & x, const int & y ) const
{
  return x == y;
}

template<>
bool
Equal_To<EmaString>::operator()( const EmaString& x, const EmaString& y ) const
{
  return x == y;
}
