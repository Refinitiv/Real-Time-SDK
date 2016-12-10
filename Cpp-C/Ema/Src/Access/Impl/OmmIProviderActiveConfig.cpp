#include "OmmIProviderActiveConfig.h"

using namespace thomsonreuters::ema::access;

#define DEFAULT_USER_DISPATCH							OmmNiProviderConfig::ApiDispatchEnum
#define DEFAULT_REFRESH_FIRST_REQUIRED					true
#define DEFAULT_DIRECTORY_ADMIN_CONTROL					OmmNiProviderConfig::ApiControlEnum
#define DEFAULT_FIELD_DICT_FRAGMENT_SIZE				8192
#define	DEFAULT_ENUM_TYPE_FRAGMENT_SIZE					128000
static const EmaString DEFAULT_IPROVIDER_SERVICE_NAME("14002");

OmmIProviderActiveConfig::OmmIProviderActiveConfig() :
	ActiveServerConfig(DEFAULT_IPROVIDER_SERVICE_NAME),
	refreshFirstRequired(DEFAULT_REFRESH_FIRST_REQUIRED)
{
	maxFieldDictFragmentSize = DEFAULT_FIELD_DICT_FRAGMENT_SIZE;
	maxEnumTypeFragmentSize = DEFAULT_ENUM_TYPE_FRAGMENT_SIZE;
}

OmmIProviderActiveConfig::~OmmIProviderActiveConfig()
{
}

OmmIProviderConfig::AdminControl OmmIProviderActiveConfig::getDictionaryAdminControl()
{
	return dictionaryAdminControl;
}

OmmIProviderConfig::AdminControl OmmIProviderActiveConfig::getDirectoryAdminControl()
{
	return directoryAdminControl;
}

UInt32 OmmIProviderActiveConfig::getMaxFieldDictFragmentSize()
{
	return maxFieldDictFragmentSize;
}

UInt32 OmmIProviderActiveConfig::getMaxEnumTypeFragmentSize()
{
	return maxEnumTypeFragmentSize;
}