/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|          Copyright (C) 2019-2020 Refinitiv. All rights reserved.          --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_ProgrammaticConfigure_h
#define __refinitiv_ema_access_ProgrammaticConfigure_h

#include "Map.h"
#include "EmaVector.h"
#include "EmaList.h"
#include "ConfigErrorHandling.h"

namespace refinitiv {

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
class WarmStandbyChannelConfig;
class WarmStandbyServerInfoConfig;
class EmaConfigImpl;

class ProgrammaticConfigure
{
public:
	/** @enum InstanceEntryFlagEnum
	An enumeration representing entry level config variables.
	*/
	enum InstanceEntryFlagEnum
	{
		ChannelFlagEnum			=		0x001,
		LoggerFlagEnum			=		0x002,
		DictionaryFlagEnum		=		0x004,
		ChannelSetFlagEnum		=		0x008,
		DirectoryFlagEnum		=		0x010,
		ServerFlagEnum			=		0x020,
		WarmStandbyChannelSetFlagEnum = 0x040
	};

	/** @enum ServerEntryFlagEnum
	An enumeration representing entry level config variables.
	*/
	enum ServerEntryFlagEnum
	{
		ServerTypeFlagEnum =			0x0000001,
		PortFlagEnum =					0x0000002,
		InterfaceNameFlagEnum =			0x0000004,
		CompTypeFlagEnum =				0x0000008,
		GuarantOutputBufFlagEnum =		0x0000010,
		NumInputBufFlagEnum =			0x0000020,
		SysRecvBufSizeFlagEnum =		0x0000040,
		SysSendBufSizeFlagEnum =		0x0000080,
		HighWaterMarkFlagEnum =			0x0000100,
		TcpNodelayFlagEnum =			0x0000200,
		ConnMinPingTimeoutFlagEnum =	0x0000400,
		ConnPingTimeoutFlagEnum =		0x0000800,
		CompressThresHoldFlagEnum =		0x0001000,
		InitializationTimeoutFlagEnum = 0x0002000,
		ServerCertEnum =				0x0004000,
		ServerPrivateKeyEnum =			0x0008000,
		DHParamEnum =					0x0010000,
		CipherSuiteEnum =				0x0020000,
		LibSslNameEnum =				0x0040000,
		LibCryptoNameEnum =				0x0080000,
		LibCurlNameEnum =				0x0100000,
		MaxFragmentSizeFlagEnum =		0x0200000,
		ServerSharedSocketEnum =		0x0400000,
		WebsocketProtocolEnum =			0x0800000,
		DirectWriteFlagEnum =			0x1000000

	};

	enum ClientEntryFlagEnum
	{
		HostEnum						= 0x0000001,
		PortEnum						= 0x0000002,
		InterfaceNameEnum				= 0x0000004,
		LocationEnum					= 0x0000008,
		ObjectNameEnum					= 0x0000010,
		ProxyPortEnum					= 0x0000020, 
		ProxyHostEnum					= 0x0000040,
		OpenSSLCAStoreEnum				= 0x0000080,
		WsProtocolsEnum					= 0x0000100,
		ChannelTypeEnum					= 0x0000200,
		CompressionTypeEnum				= 0x0000400,
		EncryptedProtocolTypeEnum		= 0x0000800,
		GuaranteedOutputBuffersEnum		= 0x0001000,
		NumInputBuffersEnum				= 0x0002000,
		SysRecvBufSizeEnum				= 0x0004000,
		SysSendBufSizeEnum				= 0x0008000,
		HighWaterMarkEnum				= 0x0010000,
		TcpNodelayEnum					= 0x0020000,
		ConnectionPingTimeoutEnum		= 0x0040000,
		CompressionThresholdEnum		= 0x0080000,
		InitializationTimeoutEnum		= 0x0100000,
		EnableSessionManagementEnum		= 0x0200000,
		SecurityProtocolEnum			= 0x0400000,
		ServiceDiscoveryRetryCountEnum	= 0x0800000,
		WsMaxMsgSizeEnum				= 0x1000000,
		DirectWriteEnum					= 0x2000000,
		ProxyConnectionTimeoutEnum		= 0x4000000
	};

	enum MultcastEntryFlagEnum
	{
		RecvAddressEnum					= 0x000001,
		RecvPortEnum					= 0x000002,
		SendPortEnum					= 0x000004,
		SendAddressEnum					= 0x000008,
		UnicastPortEnum					= 0x000010,
		HsmInterfaceEnum				= 0x000020,
		HsmMultAddressEnum				= 0x000040,
		HsmPortEnum						= 0x000080,
		TcpControlPortEnum				= 0x000100,
		PacketTTLEnum					= 0x000200,
		DisconnectOnGapEnum				= 0x000400,
		HsmIntervalEnum					= 0x000800,
		ndataEnum						= 0x001000,
		nmissingEnum					= 0x002000,
		nrreqEnum						= 0x004000,
		tdataEnum						= 0x008000,
		trreqEnum						= 0x010000,
		pktPoolLimitHighEnum			= 0x020000,
		pktPoolLimitLowEnum				= 0x040000,
		twaitEnum						= 0x080000,
		tbcholdEnum						= 0x100000,
		tppholdEnum						= 0x200000,
		userQLimitEnum					= 0x400000
	};

	enum WSBServerInfoEnum
	{
		WSBChannelNameEnum   = 0x01,
		WSBPerServiceNameSet = 0x02
	};

	enum WSBChannelEnum
	{
		WSBActiveServerNameEnum		= 0x01,
		WSBStandbyServerSetEnum		= 0x02,
		WSBModeEnum					= 0x04,
		WSBDownloadConnectionConfig = 0x08
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

	bool getActiveWSBChannelSetName(const EmaString&, EmaString&);

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

	void retrieveWSBChannelConfig(const EmaString&, ActiveConfig&, WarmStandbyChannelConfig* fileCfg = 0);

	void retrieveWSBServerInfoConfig(const EmaString&, ActiveConfig&, WarmStandbyServerInfoConfig* currentCfg, WarmStandbyServerInfoConfig* fileCfg = 0);

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
	
	void retrieveWSBChannel(const Map&, const EmaString&, EmaConfigErrorList&, ActiveConfig&, WarmStandbyChannelConfig*);

	void retrieveWSBServer(const Map&, const EmaString&, EmaConfigErrorList&, ActiveConfig&, WarmStandbyServerInfoConfig*, WarmStandbyServerInfoConfig*);

	void retrieveServer(const Map&, const EmaString&, EmaConfigErrorList&, ActiveServerConfig&, int, ServerConfig*);
	
	void retrieveChannelInfo( const MapEntry&, const EmaString&, EmaConfigErrorList&, ActiveConfig&, int, ChannelConfig*);

	void retrieveWSBChannelInfo(const MapEntry&, const EmaString&, EmaConfigErrorList&, ActiveConfig&, WarmStandbyChannelConfig*);

	void retrieveWSBServerInfo(const MapEntry&, const EmaString&, EmaConfigErrorList&, ActiveConfig&, WarmStandbyServerInfoConfig*, WarmStandbyServerInfoConfig*);

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
	EmaString	_warmStandbyChannelSetName;

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

#endif // __refinitiv_ema_access_ProgrammaticConfigure_h
