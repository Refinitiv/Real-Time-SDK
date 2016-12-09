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

OmmNiProviderConfig::AdminControl OmmNiProviderActiveConfig::getDirectoryAdminControl()
{
	return directoryAdminControl;
}