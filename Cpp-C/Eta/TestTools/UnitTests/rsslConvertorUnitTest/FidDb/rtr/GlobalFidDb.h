/*|-----------------------------------------------------------------------------
*| This source code is provided under the Apache 2.0 license –
*| and is provided AS IS with no warranty or guarantee of fit for purpose. –
*| See the project's LICENSE.md for details. –
*| Copyright (C) 2020 LSEG. All rights reserved.      –
*|-----------------------------------------------------------------------------
*/

#ifndef _GLOBAL_SSL_FID_DATABASE_
#define _GLOBAL_SSL_FID_DATABASE_

//#include "trwf/trwfSetData.h"
#include "rtr/rsslDataDictionary.h"

class SSLFidDatabase;

class GlobalSSLFidDb
{
public:
	static SSLFidDatabase& fidDatabase() { return *_globalSSLFidDatabase; }
	static void setFidDatabase(SSLFidDatabase &fidDatabase);

	//static trwfSetDefinitionDB *trwfSetDb() { return _globalTrwfSetDb; }
	//static void setTrwfSetDb(trwfSetDefinitionDB *setDb);

	static RsslDataDictionary *dataDictionary() { return _globalRsslDataDictionary; }
	static RsslDataDictionary **dataDictionaryPtr() { return &_globalRsslDataDictionary; }
	static void setRsslDataDictionary(RsslDataDictionary *dataDictionary);

protected:

	static SSLFidDatabase *_globalSSLFidDatabase;
	//static trwfSetDefinitionDB *_globalTrwfSetDb;
	static RsslDataDictionary *_globalRsslDataDictionary;
};

#endif
