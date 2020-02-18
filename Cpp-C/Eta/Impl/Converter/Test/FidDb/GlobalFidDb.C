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

