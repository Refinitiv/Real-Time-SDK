/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "rtr/GlobalFidDb.h"

void GlobalSSLFidDb::setFidDatabase(SSLFidDatabase &fidDatabase)
{
	_globalSSLFidDatabase = &fidDatabase;
}

//void GlobalSSLFidDb::setTrwfSetDb(trwfSetDefinitionDB *setDb)
//{
	//_globalTrwfSetDb = setDb;
//}

void GlobalSSLFidDb::setRsslDataDictionary(RsslDataDictionary *dataDictionary)
{
	_globalRsslDataDictionary = dataDictionary;
}

SSLFidDatabase *GlobalSSLFidDb::_globalSSLFidDatabase = 0;
//trwfSetDefinitionDB *GlobalSSLFidDb::_globalTrwfSetDb = 0;
RsslDataDictionary *GlobalSSLFidDb::_globalRsslDataDictionary = 0;

