/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmNiProviderActiveConfig.h"
#include "ExceptionTranslator.h"
#include <new>

using namespace thomsonreuters::ema::access;

#define DEFAULT_USER_DISPATCH							OmmNiProviderConfig::ApiDispatchEnum
#define DEFAULT_REFRESH_FIRST_REQUIRED					true
#define DEFAULT_MERGE_SOURCE_DIRECTORY_STREAMS			true
#define DEFAULT_DIRECTORY_ADMIN_CONTROL					OmmNiProviderConfig::ApiControlEnum
#define DEFAULT_RECOVER_USER_SUBMIT_SOURCEDIRECTORY		true
#define DEFAULT_REMOVE_ITEMS_ON_DISCONNECT				false
static const EmaString DEFAULT_NIPROVIDER_SERVICE_NAME( "14003" );


OmmNiProviderActiveConfig::OmmNiProviderActiveConfig() :
	ActiveConfig( DEFAULT_NIPROVIDER_SERVICE_NAME ),
	operationModel( DEFAULT_USER_DISPATCH ),
	directoryAdminControl( DEFAULT_DIRECTORY_ADMIN_CONTROL ),
	refreshFirstRequired( DEFAULT_REFRESH_FIRST_REQUIRED ),
	mergeSourceDirectoryStreams( DEFAULT_MERGE_SOURCE_DIRECTORY_STREAMS ),
	recoverUserSubmitSourceDirectory( DEFAULT_RECOVER_USER_SUBMIT_SOURCEDIRECTORY ),
	removeItemsOnDisconnect( DEFAULT_REMOVE_ITEMS_ON_DISCONNECT )
{
}

OmmNiProviderActiveConfig::~OmmNiProviderActiveConfig()
{
}

void OmmNiProviderActiveConfig::clear()
{
	ActiveConfig::clear();
	operationModel = DEFAULT_USER_DISPATCH;
	directoryAdminControl = DEFAULT_DIRECTORY_ADMIN_CONTROL;
	refreshFirstRequired = DEFAULT_REFRESH_FIRST_REQUIRED;
	mergeSourceDirectoryStreams = DEFAULT_MERGE_SOURCE_DIRECTORY_STREAMS;
	recoverUserSubmitSourceDirectory = DEFAULT_RECOVER_USER_SUBMIT_SOURCEDIRECTORY;
	removeItemsOnDisconnect = DEFAULT_REMOVE_ITEMS_ON_DISCONNECT;
}

EmaString OmmNiProviderActiveConfig::configTrace()
{
	ActiveConfig::configTrace();
	traceStr.append("\n\t operationModel: ").append(operationModel)
		.append("\n\t directoryAdminControl: ").append(directoryAdminControl)
		.append("\n\t refreshFirstRequired : ").append(refreshFirstRequired)
		.append("\n\t mergeSourceDirectoryStreams : ").append(mergeSourceDirectoryStreams)
		.append("\n\t recoverUserSubmitSourceDirectory : ").append(recoverUserSubmitSourceDirectory)
		.append("\n\t removeItemsOnDisconnect : ").append(removeItemsOnDisconnect);
	return traceStr;
}

OmmNiProviderConfig::AdminControl OmmNiProviderActiveConfig::getDirectoryAdminControl()
{
	return directoryAdminControl;
}

bool OmmNiProviderActiveConfig::getMergeSourceDirectoryStreams()
{
	return mergeSourceDirectoryStreams;
}

bool OmmNiProviderActiveConfig::getRecoverUserSubmitSourceDirectory()
{
	return recoverUserSubmitSourceDirectory;
}

bool OmmNiProviderActiveConfig::getRemoveItemsOnDisconnect()
{
	return removeItemsOnDisconnect;
}

void OmmNiProviderActiveConfig::setMergeSourceDirectoryStreams(UInt64 value)
{
	if (value <= 0)
		mergeSourceDirectoryStreams = 0;
	else
		mergeSourceDirectoryStreams = 1;
}

void OmmNiProviderActiveConfig::setRecoverUserSubmitSourceDirectory(UInt64 value)
{
	if (value <= 0)
		recoverUserSubmitSourceDirectory = 0;
	else
		recoverUserSubmitSourceDirectory = 1;
}

void OmmNiProviderActiveConfig::setRemoveItemsOnDisconnect(UInt64 value)
{
	if (value <= 0)
		removeItemsOnDisconnect = 0;
	else
		removeItemsOnDisconnect = 1;
}

bool OmmNiProviderActiveConfig::getRefreshFirstRequired()
{
	return refreshFirstRequired;
}

void OmmNiProviderActiveConfig::setRefreshFirstRequired(UInt64 value)
{
	if (value <= 0)
		refreshFirstRequired = 0;
	else
		refreshFirstRequired = 1;
}