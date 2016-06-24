/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_OmmNiProviderActiveConfig_h
#define __thomsonreuters_ema_access_OmmNiProviderActiveConfig_h

#include "ActiveConfig.h"
#include "OmmProviderConfig.h"
#include "rtr/rsslState.h"
#include "rtr/rsslQos.h"

namespace thomsonreuters {

namespace ema {

namespace access {

class InfoFilterConfig
{
public :

	InfoFilterConfig();
	InfoFilterConfig( const InfoFilterConfig& );
	InfoFilterConfig& operator=( const InfoFilterConfig& );

	virtual ~InfoFilterConfig();

	void clear();

	UInt16					serviceId;
	EmaString				vendorName;
	UInt64					isSource;
	EmaVector< UInt16 >		capabilities;
	EmaVector< EmaString >	dictionariesProvided;
	EmaVector< EmaString >	dictionariesUsed;
	EmaVector< RsslQos >	qos;
	UInt64					supportsQosRange;
	EmaString				itemList;
	UInt64					supportsOutOfBandSnapshots;
	UInt64					acceptingConsumerStatus;
};

class StateFilterConfig
{
public :

	StateFilterConfig();
	StateFilterConfig( const StateFilterConfig& );
	StateFilterConfig& operator=( const StateFilterConfig& );

	virtual ~StateFilterConfig();

	void clear();

	RsslUInt		serviceState;
	RsslUInt		acceptingRequests;
	RsslState		status;
	bool			isStatusConfigured;
	EmaString		statusText;
};

class ServiceConfig
{
public :

	ServiceConfig();
	ServiceConfig( const ServiceConfig& );
	ServiceConfig& operator=( const ServiceConfig& );

	virtual ~ServiceConfig();

	EmaString			serviceName;
	InfoFilterConfig	infoFilter;
	StateFilterConfig	stateFilter;
};

class DirectoryConfig
{
public :

	DirectoryConfig();
	virtual ~DirectoryConfig();

	void clear();

	void addService( const ServiceConfig& );

	void removeService( UInt16 );

	ServiceConfig* getService( UInt16 );

	const EmaVector< ServiceConfig* >& getServiceList() const;

	EmaString		directoryName;

private :

	EmaVector< ServiceConfig* >		serviceList;
};

class OmmNiProviderActiveConfig : public ActiveConfig
{
public:

	OmmNiProviderActiveConfig();

	virtual ~OmmNiProviderActiveConfig();

	void clear();

	OmmNiProviderConfig::OperationModel		operationModel;
	OmmNiProviderConfig::AdminControl		directoryAdminControl;
	bool									refreshFirstRequired;
	bool									mergeSourceDirectoryStreams;
	bool									recoverUserSubmitSourceDirectory;
	bool									removeItemsOnDisconnect;
	DirectoryConfig							directoryConfig;
	DirectoryConfig							userSubmittedDirectoryConfig;
};

}

}

}

#endif // __thomsonreuters_ema_access_OmmNiProviderActiveConfig_h
