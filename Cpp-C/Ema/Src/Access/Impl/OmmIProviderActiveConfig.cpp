#include "OmmIProviderActiveConfig.h"

using namespace thomsonreuters::ema::access;

#define DEFAULT_USER_DISPATCH							OmmNiProviderConfig::ApiDispatchEnum
#define DEFAULT_REFRESH_FIRST_REQUIRED					true
#define DEFAULT_DIRECTORY_ADMIN_CONTROL					OmmNiProviderConfig::ApiControlEnum
#define DEFAULT_FIELD_DICT_FRAGMENT_SIZE				8192
#define	DEFAULT_ENUM_TYPE_FRAGMENT_SIZE					12288
#define DEFAULT_REQUEST_TIMEOUT							15000
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

EmaString OmmIProviderActiveConfig::configTrace()
{
	ActiveServerConfig::configTrace();
	traceStr.append("\n\t operationModel: ").append(operationModel)
	.append("\n\t dictionaryAdminControl: ").append(dictionaryAdminControl)
	.append("\n\t directoryAdminControl : ").append(directoryAdminControl)
	.append("\n\t refreshFirstRequired : ").append(refreshFirstRequired)
	.append("\n\t maxFieldDictFragmentSize : ").append(maxFieldDictFragmentSize)
	.append("\n\t maxEnumTypeFragmentSize : ").append(maxEnumTypeFragmentSize);
	return traceStr;
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

void OmmIProviderActiveConfig::setMaxFieldDictFragmentSize(UInt64 value)
{
	if (value <= 0) {}
	else if (value > 0xFFFFFFFF)
		maxFieldDictFragmentSize = 0xFFFFFFFF;
	else
		maxFieldDictFragmentSize = (UInt32)value;
}

void OmmIProviderActiveConfig::setMaxEnumTypeFragmentSize(UInt64 value)
{
	if (value <= 0) {}
	else if (value > 0xFFFFFFFF)
		maxEnumTypeFragmentSize = 0xFFFFFFFF;
	else
		maxEnumTypeFragmentSize = (UInt32)value;
}

bool OmmIProviderActiveConfig::getRefreshFirstRequired()
{
	return refreshFirstRequired;
}

void OmmIProviderActiveConfig::setRefreshFirstRequired(UInt64 value)
{
	if (value <= 0)
		refreshFirstRequired = false;
	else
		refreshFirstRequired = true;
}
