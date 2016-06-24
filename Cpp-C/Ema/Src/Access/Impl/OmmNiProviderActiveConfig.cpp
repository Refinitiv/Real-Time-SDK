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
#define DEFAULT_SERVICE_STATE							1
#define DEFAULT_ACCEPTING_REQUESTS						1
#define DEFAULT_IS_STATUS_CONFIGURED					false
#define DEFAULT_SERVICE_ID								0
#define DEFAULT_SERVICE_IS_SOURCE						0
#define DEFAULT_SERVICE_SUPPORTS_QOS_RANGE				0
#define DEFAULT_SERVICE_SUPPORTS_OUT_OF_BAND_SNAPSHATS	1
#define DEFAULT_SERVICE_ACCEPTING_CONSUMER_SERVICE		1
#define DEFAULT_REFRESH_FIRST_REQUIRED					true
#define DEFAULT_MERGE_SOURCE_DIRECTORY_STREAMS			true
#define DEFAULT_DIRECTORY_ADMIN_CONTROL					OmmNiProviderConfig::ApiControlEnum
#define DEFAULT_RECOVER_USER_SUBMIT_SOURCEDIRECTORY		true
#define DEFAULT_REMOVE_ITEMS_ON_DISCONNECT				false
static const EmaString DEFAULT_NIPROVIDER_SERVICE_NAME( "14003" );

InfoFilterConfig::InfoFilterConfig() :
	serviceId( DEFAULT_SERVICE_ID ),
	vendorName(),
	isSource( DEFAULT_SERVICE_IS_SOURCE ),
	capabilities(),
	dictionariesProvided(),
	dictionariesUsed(),
	qos(),
	supportsQosRange( DEFAULT_SERVICE_SUPPORTS_QOS_RANGE ),
	itemList(),
	supportsOutOfBandSnapshots( DEFAULT_SERVICE_SUPPORTS_OUT_OF_BAND_SNAPSHATS ),
	acceptingConsumerStatus( DEFAULT_SERVICE_ACCEPTING_CONSUMER_SERVICE )
{
}

InfoFilterConfig::~InfoFilterConfig()
{
}

InfoFilterConfig::InfoFilterConfig( const InfoFilterConfig& other ) :
	serviceId( other.serviceId ),
	vendorName( other.vendorName ),
	isSource( other.isSource ),
	capabilities( other.capabilities ),
	dictionariesProvided( other.dictionariesProvided ),
	dictionariesUsed( other.dictionariesUsed ),
	qos( other.qos ),
	supportsQosRange( other.supportsQosRange ),
	itemList( other.itemList ),
	supportsOutOfBandSnapshots( other.supportsOutOfBandSnapshots ),
	acceptingConsumerStatus( other.acceptingConsumerStatus )
{
}

InfoFilterConfig& InfoFilterConfig::operator=( const InfoFilterConfig& other )
{
	if ( &other == this ) return *this;

	serviceId = other.serviceId;
	vendorName = other.vendorName;
	isSource = other.isSource;
	capabilities = other.capabilities;
	dictionariesProvided = other.dictionariesProvided;
	dictionariesUsed = other.dictionariesUsed;
	qos = other.qos;
	supportsQosRange = other.supportsQosRange;
	itemList = other.itemList;
	supportsOutOfBandSnapshots = other.supportsOutOfBandSnapshots;
	acceptingConsumerStatus = other.acceptingConsumerStatus;

	return *this;
}

void InfoFilterConfig::clear()
{
	serviceId = DEFAULT_SERVICE_ID;
	vendorName.clear();
	isSource = DEFAULT_SERVICE_IS_SOURCE;
	capabilities.clear();
	dictionariesProvided.clear();
	dictionariesUsed.clear();
	qos.clear();
	supportsQosRange = DEFAULT_SERVICE_SUPPORTS_QOS_RANGE;
	itemList.clear();
	supportsOutOfBandSnapshots = DEFAULT_SERVICE_SUPPORTS_OUT_OF_BAND_SNAPSHATS;
	acceptingConsumerStatus = DEFAULT_SERVICE_ACCEPTING_CONSUMER_SERVICE;
}

StateFilterConfig::StateFilterConfig() :
	serviceState( DEFAULT_SERVICE_STATE ),
	acceptingRequests( DEFAULT_ACCEPTING_REQUESTS ),
	status(),
	isStatusConfigured( DEFAULT_IS_STATUS_CONFIGURED ),
	statusText()
{
	rsslClearState( &status );
}

StateFilterConfig::~StateFilterConfig()
{
}

StateFilterConfig::StateFilterConfig( const StateFilterConfig& other ) :
	serviceState( other.serviceState ),
	acceptingRequests( other.acceptingRequests ),
	status( other.status ),
	isStatusConfigured( other.isStatusConfigured ),
	statusText( other.statusText )
{
	status.text.data = ( char* )statusText.c_str();
	status.text.length = statusText.length();
}

StateFilterConfig& StateFilterConfig::operator=( const StateFilterConfig& other )
{
	if ( &other == this ) return *this;

	serviceState = other.serviceState;
	acceptingRequests = other.acceptingRequests;
	status = other.status;
	isStatusConfigured = other.isStatusConfigured;
	statusText = other.statusText;

	return *this;
}

void StateFilterConfig::clear()
{
	serviceState = DEFAULT_SERVICE_STATE;
	acceptingRequests = DEFAULT_ACCEPTING_REQUESTS;
	rsslClearState( &status );
	isStatusConfigured = DEFAULT_IS_STATUS_CONFIGURED;
	statusText.clear();
}

ServiceConfig::ServiceConfig() :
	serviceName(),
	infoFilter(),
	stateFilter()
{
}

ServiceConfig::~ServiceConfig()
{
}

ServiceConfig::ServiceConfig( const ServiceConfig& other ) :
	serviceName( other.serviceName ),
	infoFilter( other.infoFilter ),
	stateFilter( other.stateFilter )
{
}

ServiceConfig& ServiceConfig::operator=( const ServiceConfig& other )
{
	if ( &other == this ) return *this;

	serviceName = other.serviceName;
	infoFilter = other.infoFilter;
	stateFilter = other.stateFilter;

	return *this;
}

DirectoryConfig::DirectoryConfig() :
	directoryName(),
	serviceList()
{
}

DirectoryConfig::~DirectoryConfig()
{
	clear();
}

void DirectoryConfig::clear()
{
	directoryName.clear();

	for ( UInt32 idx = 0; idx < serviceList.size(); ++idx )
		if ( serviceList[idx] )
			delete serviceList[idx];

	serviceList.clear();
}

const EmaVector< ServiceConfig* >& DirectoryConfig::getServiceList() const
{
	return serviceList;
}

void DirectoryConfig::addService( const ServiceConfig& service )
{
	try 
	{
		ServiceConfig* pService = new ServiceConfig( service );

		serviceList.push_back( pService );
	}
	catch ( std::bad_alloc )
	{
		throwMeeException( "Failed to allocate memory in DirectoryConfig::addService()" );
	}
}

void DirectoryConfig::removeService( UInt16 serviceId )
{
	for ( UInt16 idx = 0; idx < serviceList.size(); ++idx )
	{
		if ( serviceList[idx]->infoFilter.serviceId == serviceId )
		{
			ServiceConfig* pTemp = serviceList[idx];
			serviceList.removePosition( idx );
			delete pTemp;
		}
	}
}

ServiceConfig* DirectoryConfig::getService( UInt16 serviceId )
{
	for ( UInt16 idx = 0; idx < serviceList.size(); ++idx )
	{
		if ( serviceList[idx]->infoFilter.serviceId == serviceId )
			return serviceList[idx];
	}

	return 0;
}

OmmNiProviderActiveConfig::OmmNiProviderActiveConfig() :
	ActiveConfig( DEFAULT_NIPROVIDER_SERVICE_NAME ),
	operationModel( DEFAULT_USER_DISPATCH ),
	directoryAdminControl( DEFAULT_DIRECTORY_ADMIN_CONTROL ),
	refreshFirstRequired( DEFAULT_REFRESH_FIRST_REQUIRED ),
	mergeSourceDirectoryStreams( DEFAULT_MERGE_SOURCE_DIRECTORY_STREAMS ),
	recoverUserSubmitSourceDirectory( DEFAULT_RECOVER_USER_SUBMIT_SOURCEDIRECTORY ),
	removeItemsOnDisconnect( DEFAULT_REMOVE_ITEMS_ON_DISCONNECT ),
	directoryConfig(),
	userSubmittedDirectoryConfig()
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
	directoryConfig.clear();
	userSubmittedDirectoryConfig.clear();
}
