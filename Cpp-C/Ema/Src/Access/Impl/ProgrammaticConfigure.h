/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_ProgrammaticConfigure_h
#define __thomsonreuters_ema_access_ProgrammaticConfigure_h

#include "Map.h"
#include "EmaVector.h"
#include "EmaList.h"
#include "ConfigErrorHandling.h"

namespace thomsonreuters {

namespace ema {

namespace access {

class ActiveConfig;
class ReliableMcastChannelConfig;
class ChannelConfig;
class DirectoryCache;
class DirectoryServiceStore;
class ActiveServerConfig;
class ServerConfig;
class ServiceDictionaryConfig;
class Service;
class BaseConfig;
class DictionaryConfig;


class ProgrammaticConfigure
{
public:
	/** @enum InstanceEntryFlagEnum
	An enumeration representing entry level config variables.
	*/
	enum InstanceEntryFlagEnum
	{
		ChannelFlagEnum =		0x001,
		LoggerFlagEnum =		0x002,
		DictionaryFlagEnum =	0x004,
		ChannelSetFlagEnum =	0x008,
		DirectoryFlagEnum =		0x010,
		ServerFlagEnum =		0x020
	};

	/** @enum ServerEntryFlagEnum
	An enumeration representing entry level config variables.
	*/
	enum ServerEntryFlagEnum
	{
		ServerTypeFlagEnum =			0x000001,
		PortFlagEnum =					0x000002,
		InterfaceNameFlagEnum =			0x000004,
		CompTypeFlagEnum =				0x000008,
		GuarantOutputBufFlagEnum =		0x000010,
		NumInputBufFlagEnum =			0x000020,
		SysRecvBufSizeFlagEnum =		0x000040,
		SysSendBufSizeFlagEnum =		0x000080,
		HighWaterMarkFlagEnum =			0x000100,
		TcpNodelayFlagEnum =			0x000200,
		ConnMinPingTimeoutFlagEnum =	0x000400,
		ConnPingTimeoutFlagEnum =		0x000800,
		CompressThresHoldFlagEnum =		0x001000,
		InitializationTimeoutFlagEnum = 0x002000,
		ServerCertEnum =				0x004000,
		ServerPrivateKeyEnum =			0x008000,
		DHParamEnum =					0x010000,
		CipherSuiteEnum =				0x020000,
		LibSslNameEnum =				0x040000,
		LibCryptoNameEnum =				0x080000,
		LibCurlNameEnum =				0x100000,

	};


	ProgrammaticConfigure( const Map&, EmaConfigErrorList& );
	~ProgrammaticConfigure();

	void addConfigure( const Map& );

	bool getDefaultConsumer( EmaString& );

	bool getDefaultNiProvider( EmaString& );

	bool getDefaultIProvider( EmaString& );

	bool specifyConsumerName( const EmaString& consumerName );

	bool specifyNiProviderName( const EmaString& );

	bool specifyIProviderName( const EmaString& );

	bool getActiveChannelName( const EmaString&, EmaString& );

	bool getActiveServerName(const EmaString&, EmaString&);

	bool getActiveChannelSet( const EmaString&, EmaString& );

	bool getActiveLoggerName( const EmaString&, EmaString& );

	bool getActiveDictionaryName( const EmaString&, EmaString& );

	bool getActiveDirectoryName( const EmaString&, EmaString& );

	void retrieveCommonConfig( const EmaString&, ActiveConfig& );

	void retrieveCommonConfig(const EmaString&, ActiveServerConfig&);

	void retrieveCustomConfig( const EmaString&, BaseConfig& );

	int retrieveChannelTypeConfig(const EmaString&);

	void retrieveChannelConfig( const EmaString&, ActiveConfig&, int, ChannelConfig* fileCfg = 0 );

	void retrieveServerConfig( const EmaString&, ActiveServerConfig&, int, ServerConfig* fileCfg );

	void retrieveLoggerConfig( const EmaString&, BaseConfig& );

	void retrieveDictionaryConfig( const EmaString&, ActiveConfig& );

	void retrieveDictionaryConfig(const EmaString&, DictionaryConfig&);

	void retrieveDirectoryConfig( const EmaString&, DirectoryServiceStore&, DirectoryCache&, EmaList<ServiceDictionaryConfig*>* );

	void clear();

private:

	void retrieveGroupAndListName( const Map&, EmaString& groupName, EmaString& listName );

	bool retrieveDefaultConsumer( const Map&, EmaString& );

	bool retrieveDefaultNiProvider( const Map&, EmaString& );

	bool retrieveDefaultIProvider( const Map&, EmaString& );

	void retrieveDependencyNames( const Map&, const EmaString& );

	void retrieveInstanceCommonConfig( const Map&, const EmaString&, EmaConfigErrorList&, ActiveConfig& );

	void retrieveInstanceCommonConfig(const Map&, const EmaString&, EmaConfigErrorList&, ActiveServerConfig&);

	void retrieveInstanceCustomConfig( const Map&, const EmaString&, EmaConfigErrorList&, BaseConfig& );

	void retrieveChannel( const Map&, const EmaString&, EmaConfigErrorList&, ActiveConfig&, int, ChannelConfig*);
	
	void retrieveServer(const Map&, const EmaString&, EmaConfigErrorList&, ActiveServerConfig&, int, ServerConfig*);
	
	void retrieveChannelInfo( const MapEntry&, const EmaString&, EmaConfigErrorList&, ActiveConfig&, int, ChannelConfig*);

	void retrieveServerInfo(const MapEntry&, const EmaString&, EmaConfigErrorList&, ActiveServerConfig&, int, ServerConfig*);

	bool setReliableMcastChannelInfo( ReliableMcastChannelConfig*, UInt64& flags, ReliableMcastChannelConfig&, EmaString&, ChannelConfig* );

	void retrieveLogger( const Map&, const EmaString&, EmaConfigErrorList&, BaseConfig& );

	void retrieveDictionary( const Map&, const EmaString&, EmaConfigErrorList&, DictionaryConfig& );
	
	void retrieveServerAllDictionaryConfig(const Map& map);

	void retrieveServerDictionaryConfig(Service&, EmaList<ServiceDictionaryConfig*>*);

	void retrieveDirectory(const Map&, const EmaString&, DirectoryServiceStore&, DirectoryCache&, EmaList<ServiceDictionaryConfig*>*);

	bool retrieveServiceInfo(Service&, const ElementList&, DirectoryCache&);

	bool validateConsumerName( const Map&, const EmaString& );

	bool validateNiProviderName( const Map&, const EmaString& );

	bool validateIProviderName( const Map&, const EmaString& );

	ProgrammaticConfigure( const ProgrammaticConfigure& );

	bool findString(const EmaString& inputValue, EmaString& outputString);

	ServiceDictionaryConfig* findServiceDictConfig(EmaList<ServiceDictionaryConfig*>* serviceDictionaryConfigList, int serviceId);

	void removeConfigFileService(DirectoryServiceStore&, DirectoryCache& directoryCache);

	void internalClear();

	EmaString	_consumerName;
	EmaString	_niProviderName;
	EmaString	_iProviderName;
	EmaString	_channelName;
	EmaString	_serverName;
	EmaString	_loggerName;
	EmaString	_dictionaryName;
	EmaString	_directoryName;
	EmaString	_channelSet;

	bool		_overrideConsName;
	bool		_overrideNiProvName;
	bool		_overrideIProvName;
	bool		_dependencyNamesLoaded;
	UInt8		_nameflags;
	EmaList<DictionaryConfig*> _serverDictList;

	EmaVector<const Map*>	_configList;
	EmaConfigErrorList&		_emaConfigErrList;
	EmaVector<EmaString> _dictProvided;
	EmaVector<EmaString> _dictUsed;
	EmaVector<EmaString> _serviceNameList;
	EmaString _group;
	EmaString _list;
	bool _setGroup;
};

}

}

}

#endif // __thomsonreuters_ema_access_ProgrammaticConfigure_h
