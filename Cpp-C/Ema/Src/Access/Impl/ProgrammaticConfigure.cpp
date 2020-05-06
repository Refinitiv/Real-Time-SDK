/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|          Copyright (C) 2019-2020 Refinitiv. All rights reserved.          --
 *|-----------------------------------------------------------------------------
 */

#include "ActiveConfig.h"
#include "OmmIProviderActiveConfig.h"
#include "OmmNiProviderActiveConfig.h"
#include "EmaConfigImpl.h"
#include "Common.h"
#include "StaticDecoder.h"
#include "DirectoryServiceStore.h"
#include "ExceptionTranslator.h"
#include "OmmQosDecoder.h"
#include "OmmArray.h"
#include "Series.h"

#include <ctype.h>

using namespace thomsonreuters::ema::access;

#define MAX_UNSIGNED_INT16		0xFFFF
#define MAX_UNSIGNED_INT32		0xFFFFFFFF

ProgrammaticConfigure::ProgrammaticConfigure( const Map& map, EmaConfigErrorList& emaConfigErrList ) :
	_consumerName(),
	_niProviderName(),
	_iProviderName(),
	_channelName(),
	_serverName(),
	_loggerName(),
	_dictionaryName(),
	_directoryName(),
	_channelSet(),
	_overrideConsName( false ),
	_overrideNiProvName( false ),
	_overrideIProvName( false ),
	_dependencyNamesLoaded( false ),
	_setGroup( false ),
	_nameflags( 0 ),
	_emaConfigErrList( emaConfigErrList ),
	_configList()
{
	addConfigure( map );
}

ProgrammaticConfigure::~ProgrammaticConfigure()
{
	clear();
}

void ProgrammaticConfigure::clear()
{
	internalClear();
	_configList.clear();
}

void ProgrammaticConfigure::internalClear()
{
	_consumerName.clear();
	_niProviderName.clear();
	_iProviderName.clear();
	_channelName.clear();
	_serverName.clear();
	_loggerName.clear();
	_dictionaryName.clear();
	_directoryName.clear();
	_overrideConsName = false;
	_overrideNiProvName = false;
	_overrideIProvName = false;
	_dependencyNamesLoaded = false;
	_setGroup = false;
	_nameflags = 0;
	_dictProvided.clear();
	_dictUsed.clear();
	_serviceNameList.clear();
	_group.clear();

	if (!_serverDictList.empty())
	{
		DictionaryConfig* dictConfig = _serverDictList.pop_front();
		while (dictConfig)
		{
			delete dictConfig;
			dictConfig = _serverDictList.pop_front();
		}
	}
}

void ProgrammaticConfigure::addConfigure( const Map& map )
{
	for ( UInt32 i = 0; i < _configList.size() ; ++i )
	{
		if ( _configList[i] == &map )
			return;
	}

	StaticDecoder::setData(const_cast<Map*>(&map), 0);
	_configList.push_back( &map );
}

bool ProgrammaticConfigure::getDefaultConsumer( EmaString& defaultConsumer )
{
	bool found = false;

	if ( _overrideConsName )
	{
		defaultConsumer = _consumerName;
		found = true;
	}
	else
	{
		internalClear();

		for ( UInt32 i = 0 ; i < _configList.size() ; i++ )
			found = retrieveDefaultConsumer( *_configList[i], defaultConsumer );
	}

	return found;
}

bool ProgrammaticConfigure::getDefaultNiProvider( EmaString& defaultNiProvider )
{
	bool found = false;

	if ( _overrideNiProvName )
	{
		defaultNiProvider = _niProviderName;
		found = true;
	}
	else
	{
		internalClear();

		for ( UInt32 i = 0 ; i < _configList.size() ; i++ )
			found = retrieveDefaultNiProvider( *_configList[i], defaultNiProvider );
	}

	return found;
}

bool ProgrammaticConfigure::getDefaultIProvider( EmaString& defaultIProvider )
{
	bool found = false;

	if ( _overrideIProvName )
	{
		defaultIProvider = _iProviderName;
		found = true;
	}
	else
	{
		internalClear();

		for ( UInt32 i = 0; i < _configList.size(); i++ )
			found = retrieveDefaultIProvider( *_configList[i], defaultIProvider );
	}

	return found;
}

bool ProgrammaticConfigure::specifyConsumerName( const EmaString& consumerName )
{
	for ( UInt32 i = 0 ; i < _configList.size() ; i++ )
	{
		if ( validateConsumerName( *_configList[i], consumerName ) )
		{
			_overrideConsName = true;
			_consumerName = consumerName;
			return true;
		}
	}

	return false;
}

bool ProgrammaticConfigure::specifyNiProviderName( const EmaString& niProviderName )
{
	for ( UInt32 i = 0 ; i < _configList.size() ; i++ )
	{
		if ( validateNiProviderName( *_configList[i], niProviderName ) )
		{
			_overrideNiProvName = true;
			_niProviderName = niProviderName;
			return true;
		}
	}

	return false;
}

bool ProgrammaticConfigure::specifyIProviderName( const EmaString& iProviderName )
{
	for ( UInt32 i = 0; i < _configList.size(); i++ )
	{
		if ( validateIProviderName( *_configList[i], iProviderName ) )
		{
			_overrideIProvName = true;
			_iProviderName = iProviderName;
			return true;
		}
	}

	return false;
}

void ProgrammaticConfigure::retrieveDependencyNames( const Map& map, const EmaString& userName )
{
	unsigned int position = 0;
	unsigned int channelPos = 0, channelSetPos = 0;

	EmaString groupName;
	EmaString listName;
	retrieveGroupAndListName( map, groupName, listName );

	if ( groupName.empty() )
		return;

	map.reset();
	while ( map.forth() )
	{
		const MapEntry& mapEntry = map.getEntry();

		if ( mapEntry.getKey().getDataType() == DataType::AsciiEnum && mapEntry.getKey().getAscii() == groupName )
		{
			if ( mapEntry.getLoadType() == DataType::ElementListEnum )
			{
				const ElementList& elementList = mapEntry.getElementList();

				while ( elementList.forth() )
				{
					const ElementEntry& elementEntry = elementList.getEntry();

					if ( elementEntry.getLoadType() == DataType::MapEnum )
					{
						if ( elementEntry.getName() == listName && ( elementEntry.getLoad().getDataType() == DataType::MapEnum ) )
						{
							const Map& map = elementEntry.getMap();

							while ( map.forth() )
							{
								const MapEntry& mapEntry = map.getEntry();

								if ( ( mapEntry.getKey().getDataType() == DataType::AsciiEnum ) && ( mapEntry.getKey().getAscii() == userName ) )
								{
									if ( mapEntry.getLoadType() == DataType::ElementListEnum )
									{
										const ElementList& elementList = mapEntry.getElementList();
										position = 0;
										while ( elementList.forth() )
										{
											const ElementEntry& instanceEntry = elementList.getEntry();
											position++;
											switch ( instanceEntry.getLoadType() )
											{
											case DataType::AsciiEnum:
												if ( instanceEntry.getName() == "Channel" )
												{
													_channelName = instanceEntry.getAscii();
													_nameflags |= ChannelFlagEnum;
													channelPos = position;
												}
												if (instanceEntry.getName() == "Server")
												{
													_serverName = instanceEntry.getAscii();
													_nameflags |= ServerFlagEnum;
												}
												else if ( instanceEntry.getName() == "Logger" )
												{
													_loggerName = instanceEntry.getAscii();
													_nameflags |= LoggerFlagEnum;
												}
												else if ( instanceEntry.getName() == "Dictionary" )
												{
													_dictionaryName = instanceEntry.getAscii();
													_nameflags |= DictionaryFlagEnum;
												}
												else if ( instanceEntry.getName() == "ChannelSet" )
												{
													_channelSet = instanceEntry.getAscii();
													_nameflags |= ChannelSetFlagEnum;
													channelSetPos = position;
												}
												else if ( instanceEntry.getName() == "Directory" )
												{
													_directoryName = instanceEntry.getAscii();
													_nameflags |= DirectoryFlagEnum;
												}
												break;
											}
										}
										if ( (_nameflags & ChannelFlagEnum) && (_nameflags & ChannelSetFlagEnum) )
										{
											if ( channelSetPos > channelPos )
											{
												_nameflags &= ~ChannelFlagEnum;
												_channelName.clear();
											}
											else
											{
												_nameflags &= ~ChannelSetFlagEnum;
												_channelSet.clear();
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
}

bool ProgrammaticConfigure::getActiveChannelName( const EmaString& instanceName, EmaString& channelName )
{
	if ( !_dependencyNamesLoaded )
	{
		for ( UInt32 i = 0 ; i < _configList.size() ; i++ )
			retrieveDependencyNames( *_configList[i], instanceName );

		_dependencyNamesLoaded = true;
	}

	if ( _nameflags & ChannelFlagEnum )
	{
		channelName = _channelName;
		return true;
	}
	else
		return false;
}

bool ProgrammaticConfigure::getActiveServerName(const EmaString& instanceName, EmaString& serverName)
{
	if (!_dependencyNamesLoaded)
	{
		for (UInt32 i = 0; i < _configList.size(); i++)
			retrieveDependencyNames(*_configList[i], instanceName );

		_dependencyNamesLoaded = true;
	}

	if (_nameflags & ServerFlagEnum)
	{
		serverName = _serverName;
		return true;
	}
	else
		return false;
}

bool ProgrammaticConfigure::getActiveChannelSet( const EmaString& instanceName, EmaString& channelSet )
{
	if ( !_dependencyNamesLoaded )
	{
		for ( UInt32 i = 0 ; i < _configList.size() ; i++ )
			retrieveDependencyNames( *_configList[i], instanceName );

		_dependencyNamesLoaded = true;
	}

	if ( _nameflags & ChannelSetFlagEnum )
	{
		channelSet = _channelSet;
		return true;
	}
	else
		return false;
}

bool ProgrammaticConfigure::getActiveLoggerName( const EmaString& instanceName, EmaString& loggerName )
{
	if ( !_dependencyNamesLoaded )
	{
		for ( UInt32 i = 0 ; i < _configList.size() ; i++ )
			retrieveDependencyNames( *_configList[i], instanceName );

		_dependencyNamesLoaded = true;
	}

	if ( _nameflags & LoggerFlagEnum )
	{
		loggerName = _loggerName;
		return true;
	}
	else
		return false;
}

bool ProgrammaticConfigure::getActiveDictionaryName( const EmaString& instanceName, EmaString& dictionaryName )
{
	if ( !_dependencyNamesLoaded )
	{
		for ( UInt32 i = 0 ; i < _configList.size() ; i++ )
			retrieveDependencyNames( *_configList[i], instanceName );

		_dependencyNamesLoaded = true;
	}

	if ( _nameflags & DictionaryFlagEnum)
	{
		dictionaryName = _dictionaryName;
		return true;
	}
	else
		return false;
}

bool ProgrammaticConfigure::getActiveDirectoryName( const EmaString& instanceName, EmaString& directoryName )
{
	if ( !_dependencyNamesLoaded )
	{
		for ( UInt32 i = 0 ; i < _configList.size() ; i++ )
			retrieveDependencyNames( *_configList[i], instanceName );

		_dependencyNamesLoaded = true;
	}

	if ( _nameflags & DirectoryFlagEnum )
	{
		directoryName = _directoryName;
		return true;
	}
	else
		return false;
}

bool ProgrammaticConfigure::retrieveDefaultConsumer( const Map& map, EmaString& defaultConsumer )
{
	bool foundDefaultConsumer = false;

	map.reset();
	while ( map.forth() )
	{
		const MapEntry& mapEntry = map.getEntry();

		if ( ( mapEntry.getKey().getDataType() == DataType::AsciiEnum ) && ( mapEntry.getKey().getAscii() == "ConsumerGroup" )
		     && ( mapEntry.getLoad().getDataType() == DataType::ElementListEnum ) )
		{
			const ElementList& elementList = mapEntry.getElementList();

			while ( elementList.forth() )
			{
				const ElementEntry& elementEntry = elementList.getEntry();

				if ( elementEntry.getLoadType() == DataType::AsciiEnum )
				{
					if ( elementEntry.getName() == "DefaultConsumer" )
					{
						defaultConsumer = elementEntry.getAscii();
						foundDefaultConsumer = true;
					}
				}
			}
		}
	}

	return foundDefaultConsumer;
}

bool ProgrammaticConfigure::retrieveDefaultNiProvider( const Map& map, EmaString& defaultNiProvider )
{
	bool foundDefaultNiProvider = false;

	map.reset();
	while ( map.forth() )
	{
		const MapEntry& mapEntry = map.getEntry();

		if ( ( mapEntry.getKey().getDataType() == DataType::AsciiEnum ) && ( mapEntry.getKey().getAscii() == "NiProviderGroup" )
		     && ( mapEntry.getLoad().getDataType() == DataType::ElementListEnum ) )
		{
			const ElementList& elementList = mapEntry.getElementList();

			while ( elementList.forth() )
			{
				const ElementEntry& elementEntry = elementList.getEntry();

				if ( elementEntry.getLoadType() == DataType::AsciiEnum )
				{
					if ( elementEntry.getName() == "DefaultNiProvider" )
					{
						defaultNiProvider = elementEntry.getAscii();
						foundDefaultNiProvider = true;
					}
				}
			}
		}
	}

	return foundDefaultNiProvider;
}

bool ProgrammaticConfigure::retrieveDefaultIProvider( const Map& map, EmaString& defaultIProvider )
{
	bool foundDefaultIProvider = false;

	map.reset();
	while ( map.forth() )
	{
		const MapEntry& mapEntry = map.getEntry();

		if ( ( mapEntry.getKey().getDataType() == DataType::AsciiEnum ) && ( mapEntry.getKey().getAscii() == "IProviderGroup" )
			&& ( mapEntry.getLoad().getDataType() == DataType::ElementListEnum ) )
		{
			const ElementList& elementList = mapEntry.getElementList();

			while ( elementList.forth() )
			{
				const ElementEntry& elementEntry = elementList.getEntry();

				if ( elementEntry.getLoadType() == DataType::AsciiEnum )
				{
					if ( elementEntry.getName() == "DefaultIProvider" )
					{
						defaultIProvider = elementEntry.getAscii();
						foundDefaultIProvider = true;
					}
				}
			}
		}
	}

	return foundDefaultIProvider;
}

bool ProgrammaticConfigure::validateConsumerName( const Map& map, const EmaString& consumerName )
{
	map.reset();
	while ( map.forth() )
	{
		const MapEntry& mapEntry = map.getEntry();

		if ( ( mapEntry.getKey().getDataType() == DataType::AsciiEnum ) && ( mapEntry.getKey().getAscii() == "ConsumerGroup" )
		     && ( mapEntry.getLoad().getDataType() == DataType::ElementListEnum ) )
		{
			const ElementList& elementList = mapEntry.getElementList();

			while ( elementList.forth() )
			{
				const ElementEntry& elementEntry = elementList.getEntry();

				if ( ( elementEntry.getName() == "ConsumerList" ) && ( elementEntry.getLoad().getDataType() == DataType::MapEnum ) )
				{
					const Map& consumerMap = elementEntry.getMap();

					while ( consumerMap.forth() )
					{
						const MapEntry& consumerMapEntry = consumerMap.getEntry();

						if ( ( consumerMapEntry.getKey().getDataType() == DataType::AsciiEnum ) && ( consumerMapEntry.getKey().getAscii() == consumerName ) )
							return true;
					}
				}
			}
		}
	}

	return false;
}

bool ProgrammaticConfigure::validateNiProviderName( const Map& map, const EmaString& niProviderName )
{
	map.reset();
	while ( map.forth() )
	{
		const MapEntry& mapEntry = map.getEntry();

		if ( ( mapEntry.getKey().getDataType() == DataType::AsciiEnum ) && ( mapEntry.getKey().getAscii() == "NiProviderGroup" )
		     && ( mapEntry.getLoad().getDataType() == DataType::ElementListEnum ) )
		{
			const ElementList& elementList = mapEntry.getElementList();

			while ( elementList.forth() )
			{
				const ElementEntry& elementEntry = elementList.getEntry();

				if ( ( elementEntry.getName() == "NiProviderList" ) && ( elementEntry.getLoad().getDataType() == DataType::MapEnum ) )
				{
					const Map& niProviderMap = elementEntry.getMap();

					while ( niProviderMap.forth() )
					{
						const MapEntry& niProviderMapEntry = niProviderMap.getEntry();

						if ( ( niProviderMapEntry.getKey().getDataType() == DataType::AsciiEnum ) && ( niProviderMapEntry.getKey().getAscii() == niProviderName ) )
							return true;
					}
				}
			}
		}
	}

	return false;
}

bool ProgrammaticConfigure::validateIProviderName( const Map& map, const EmaString& iProviderName )
{
	map.reset();
	while ( map.forth() )
	{
		const MapEntry& mapEntry = map.getEntry();

		if ( ( mapEntry.getKey().getDataType() == DataType::AsciiEnum ) && ( mapEntry.getKey().getAscii() == "IProviderGroup" )
			&& ( mapEntry.getLoad().getDataType() == DataType::ElementListEnum ) )
		{
			const ElementList& elementList = mapEntry.getElementList();

			while ( elementList.forth() )
			{
				const ElementEntry& elementEntry = elementList.getEntry();

				if ( ( elementEntry.getName() == "IProviderList" ) && ( elementEntry.getLoad().getDataType() == DataType::MapEnum ) )
				{
					const Map& niProviderMap = elementEntry.getMap();

					while ( niProviderMap.forth() )
					{
						const MapEntry& niProviderMapEntry = niProviderMap.getEntry();

						if ( ( niProviderMapEntry.getKey().getDataType() == DataType::AsciiEnum ) && ( niProviderMapEntry.getKey().getAscii() == iProviderName ) )
							return true;
					}
				}
			}
		}
	}

	return false;
}

void  ProgrammaticConfigure::retrieveCommonConfig( const EmaString& instanceName, ActiveConfig& activeConfig )
{
	for ( UInt32 i = 0 ; i < _configList.size() ; i++ )
		retrieveInstanceCommonConfig( *_configList[i], instanceName, _emaConfigErrList, activeConfig );
}

void  ProgrammaticConfigure::retrieveCommonConfig(const EmaString& instanceName, ActiveServerConfig& activeConfig)
{
	for (UInt32 i = 0; i < _configList.size(); i++)
		retrieveInstanceCommonConfig(*_configList[i], instanceName, _emaConfigErrList, activeConfig);
}

void  ProgrammaticConfigure::retrieveCustomConfig( const EmaString& instanceName, BaseConfig& activeConfig )
{
	for ( UInt32 i = 0; i < _configList.size(); i++ )
		retrieveInstanceCustomConfig( *_configList[i], instanceName, _emaConfigErrList, activeConfig );
}

int  ProgrammaticConfigure::retrieveChannelTypeConfig(const EmaString& channelName)
{
	for (UInt32 i = 0; i < _configList.size(); i++)
	{
		const Map& map = *_configList[i];

		map.reset();
		while (map.forth())
		{
			const MapEntry& mapEntry = map.getEntry();

			if (mapEntry.getKey().getDataType() == DataType::AsciiEnum &&
				mapEntry.getKey().getAscii() == "ChannelGroup" &&
				mapEntry.getLoadType() == DataType::ElementListEnum)
			{
				const ElementList& elementList = mapEntry.getElementList();
				while (elementList.forth())
				{
					const ElementEntry& elementEntry = elementList.getEntry();
					if (elementEntry.getLoadType() == DataType::MapEnum && elementEntry.getName() == "ChannelList")
					{
						const Map& map = elementEntry.getMap();
						while (map.forth())
						{
							const MapEntry& mapEntry = map.getEntry();
							if ((mapEntry.getKey().getDataType() == DataType::AsciiEnum) && (mapEntry.getKey().getAscii() == channelName)
								&& (mapEntry.getLoadType() == DataType::ElementListEnum))
							{
								const ElementList& elementListChannel = mapEntry.getElementList();
								while (elementListChannel.forth())
								{
									const ElementEntry& channelEntry = elementListChannel.getEntry();
									if (channelEntry.getLoadType() == DataType::EnumEnum && channelEntry.getName() == "ChannelType")
									{
										int channType = (int)channelEntry.getEnum();
										switch (channType)
										{
										case RSSL_CONN_TYPE_SOCKET:
										case RSSL_CONN_TYPE_RELIABLE_MCAST:
										case RSSL_CONN_TYPE_HTTP:
										case RSSL_CONN_TYPE_ENCRYPTED:
										case RSSL_CONN_TYPE_WEBSOCKET:
											return channType;
										default:
											return -1;
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	return -1;
}

void  ProgrammaticConfigure::retrieveChannelConfig( const EmaString& channelName,  ActiveConfig& activeConfig, int hostFnCalled, ChannelConfig* fileCfg)
{
	for ( UInt32 i = 0 ; i < _configList.size() ; i++ )
		retrieveChannel( *_configList[i], channelName, _emaConfigErrList, activeConfig, hostFnCalled, fileCfg);
}

void  ProgrammaticConfigure::retrieveServerConfig(const EmaString& serverName, ActiveServerConfig& activeServerConfig, int portFnCalled, ServerConfig* fileCfg)
{
	for (UInt32 i = 0; i < _configList.size(); i++)
		retrieveServer(*_configList[i], serverName, _emaConfigErrList, activeServerConfig, portFnCalled, fileCfg);
}

void  ProgrammaticConfigure::retrieveLoggerConfig( const EmaString& loggerName, BaseConfig& activeConfig )
{
	for ( UInt32 i = 0 ; i < _configList.size() ; i++ )
		retrieveLogger( *_configList[i], loggerName, _emaConfigErrList, activeConfig );
}

void  ProgrammaticConfigure::retrieveDictionaryConfig( const EmaString& dictionaryName, ActiveConfig& activeConfig )
{
	for ( UInt32 i = 0 ; i < _configList.size() ; i++ )
		retrieveDictionary( *_configList[i], dictionaryName, _emaConfigErrList, activeConfig.dictionaryConfig );
}

void  ProgrammaticConfigure::retrieveDictionaryConfig(const EmaString& dictionaryName, DictionaryConfig& dictConfig)
{
	for (UInt32 i = 0; i < _configList.size(); i++)
		retrieveDictionary(*_configList[i], dictionaryName, _emaConfigErrList, dictConfig);
}

void  ProgrammaticConfigure::retrieveServerAllDictionaryConfig(const Map& map)
{
	EmaString rdmFieldDictionaryItemName, enumTypeItemName, rdmfieldDictionaryFileName, enumtypeDefFileName;
	bool hasDictInfo = false;

	map.reset();
	while (map.forth())
	{
		const MapEntry& mapEntry = map.getEntry();

		if (mapEntry.getKey().getDataType() == DataType::AsciiEnum && mapEntry.getKey().getAscii() == "DictionaryGroup")
		{
			if (mapEntry.getLoadType() == DataType::ElementListEnum)
			{
				const ElementList& elementList = mapEntry.getElementList();

				while (elementList.forth())
				{
					const ElementEntry& elementEntry = elementList.getEntry();

					if (elementEntry.getLoadType() == DataType::MapEnum)
					{
						if (elementEntry.getName() == "DictionaryList")
						{
							const Map& map = elementEntry.getMap();

							while (map.forth())
							{
								const MapEntry& mapEntry = map.getEntry();

								if (mapEntry.getKey().getDataType() == DataType::AsciiEnum &&
									mapEntry.getLoadType() == DataType::ElementListEnum)
								{
									const ElementList& elementListDictionary = mapEntry.getElementList();
									hasDictInfo = false;
									while (elementListDictionary.forth())
									{
										const ElementEntry& entry = elementListDictionary.getEntry();
										switch (entry.getLoadType())
										{
											case DataType::AsciiEnum:
												if (entry.getName() == "RdmFieldDictionaryFileName")
												{
													hasDictInfo = true;
													rdmfieldDictionaryFileName = entry.getAscii();
												}
												else if (entry.getName() == "EnumTypeDefFileName")
												{
													hasDictInfo = true;
													enumtypeDefFileName = entry.getAscii();
												}
												if (entry.getName() == "RdmFieldDictionaryItemName")
												{
													hasDictInfo = true;
													rdmFieldDictionaryItemName = entry.getAscii();
												}
												else if (entry.getName() == "EnumTypeDefItemName")
												{
													hasDictInfo = true;
													enumTypeItemName = entry.getAscii();
												}
												break;
											default:
												break;
										}
									}

									if (hasDictInfo)
									{
										DictionaryConfig* dictConfig = new DictionaryConfig();
										_serverDictList.push_back(dictConfig);
										dictConfig->dictionaryName = mapEntry.getKey().getAscii();
										dictConfig->dictionaryType = DEFAULT_DICTIONARY_TYPE;

										if (!rdmFieldDictionaryItemName.empty())
											dictConfig->rdmFieldDictionaryItemName = rdmFieldDictionaryItemName;
										else
											dictConfig->rdmFieldDictionaryItemName = "RWFFld";
										if (!enumTypeItemName.empty())
											dictConfig->enumTypeDefItemName = enumTypeItemName;
										else
											dictConfig->enumTypeDefItemName = "RWFEnum";
										if (!rdmfieldDictionaryFileName.empty())
											dictConfig->rdmfieldDictionaryFileName = rdmfieldDictionaryFileName;
										else
											dictConfig->rdmfieldDictionaryFileName = "./RDMFieldDictionary";
										if (!enumtypeDefFileName.empty())
											dictConfig->enumtypeDefFileName = enumtypeDefFileName;
										else
											dictConfig->enumtypeDefFileName = "./enumtype.def";
									}
								}
							}
						}
					}
				}
				break;
			}
		}
	}
}


void  ProgrammaticConfigure::retrieveServerDictionaryConfig(Service& service, EmaList<ServiceDictionaryConfig*>* serviceDictionaryConfigList)
{
	if (_dictProvided.empty() && _dictUsed.empty())
		return;

	ServiceDictionaryConfig* fileServiceDictConfig = 0;
	ServiceDictionaryConfig* currentServiceDicConfig = 0; 

	if (serviceDictionaryConfigList)
	{
		fileServiceDictConfig = findServiceDictConfig(serviceDictionaryConfigList, service.serviceId);
		currentServiceDicConfig = new ServiceDictionaryConfig();
		currentServiceDicConfig->serviceId = service.serviceId;
	}
		
	for (UInt32 i = 0; i < _dictProvided.size(); i++)
	{
		DictionaryConfig* findDictConfig = 0;
		if (_serverDictList.size())
		{
			DictionaryConfig* dictConfig = _serverDictList.front();
			while (dictConfig)
			{
				if (dictConfig->dictionaryName == _dictProvided[i])
				{
					findDictConfig = dictConfig;
					break;
				}
				dictConfig = dictConfig->next();
			}
		}

		if (!findDictConfig && fileServiceDictConfig) //will use dict config from file
			findDictConfig = fileServiceDictConfig->findDictionary(_dictProvided[i], true);

		DictionaryConfig* newDictConfig = new DictionaryConfig();
		newDictConfig->dictionaryName = _dictProvided[i];
		if (!findDictConfig) //use default
		{
			newDictConfig->dictionaryType = DEFAULT_DICTIONARY_TYPE;
			newDictConfig->rdmFieldDictionaryItemName = "RWFFld";
			newDictConfig->enumTypeDefItemName = "RWFEnum";
			newDictConfig->rdmfieldDictionaryFileName = "./RDMFieldDictionary";
			newDictConfig->enumtypeDefFileName = "./enumtype.def";
		}
		else
		{
			newDictConfig->dictionaryType = findDictConfig->dictionaryType;
			newDictConfig->rdmFieldDictionaryItemName = findDictConfig->rdmFieldDictionaryItemName;
			newDictConfig->enumTypeDefItemName = findDictConfig->enumTypeDefItemName;
			newDictConfig->rdmfieldDictionaryFileName = findDictConfig->rdmfieldDictionaryFileName;
			newDictConfig->enumtypeDefFileName = findDictConfig->enumtypeDefFileName;
		}

		service.infoFilter.dictionariesProvided.push_back(newDictConfig->rdmFieldDictionaryItemName);
		service.infoFilter.dictionariesProvided.push_back(newDictConfig->enumTypeDefItemName);

		if (currentServiceDicConfig)
			currentServiceDicConfig->addDictionaryProvided(newDictConfig);
		else
			delete newDictConfig;
	}

	for (UInt32 i = 0; i < _dictUsed.size(); i++)
	{
		DictionaryConfig* findDictConfig = 0;
		if (_serverDictList.size())
		{
			DictionaryConfig* dictConfig = _serverDictList.front();
			while (dictConfig)
			{
				if (dictConfig->dictionaryName == _dictUsed[i])
				{
					findDictConfig = dictConfig;
					break;
				}
				dictConfig = dictConfig->next();
			}
		}

		if (!findDictConfig && fileServiceDictConfig) //will use dict config from file
			findDictConfig = fileServiceDictConfig->findDictionary(_dictUsed[i], false);

		DictionaryConfig* newDictConfig = new DictionaryConfig();
		newDictConfig->dictionaryName = _dictUsed[i];
		if (!findDictConfig) //use default
		{
			newDictConfig->dictionaryType = DEFAULT_DICTIONARY_TYPE;
			newDictConfig->rdmFieldDictionaryItemName = "RWFFld";
			newDictConfig->enumTypeDefItemName = "RWFEnum";
			newDictConfig->rdmfieldDictionaryFileName = "./RDMFieldDictionary";
			newDictConfig->enumtypeDefFileName = "./enumtype.def";
		}
		else
		{
			newDictConfig->dictionaryType = findDictConfig->dictionaryType;
			newDictConfig->rdmFieldDictionaryItemName = findDictConfig->rdmFieldDictionaryItemName;
			newDictConfig->enumTypeDefItemName = findDictConfig->enumTypeDefItemName;
			newDictConfig->rdmfieldDictionaryFileName = findDictConfig->rdmfieldDictionaryFileName;
			newDictConfig->enumtypeDefFileName = findDictConfig->enumtypeDefFileName;
		}

		service.infoFilter.dictionariesUsed.push_back(newDictConfig->rdmFieldDictionaryItemName);
		service.infoFilter.dictionariesUsed.push_back(newDictConfig->enumTypeDefItemName);

		if (currentServiceDicConfig)
			currentServiceDicConfig->addDictionaryUsed(newDictConfig);
		else
			delete newDictConfig;
	}

	if (currentServiceDicConfig)
	{
		if (fileServiceDictConfig)
		{
			serviceDictionaryConfigList->remove(fileServiceDictConfig);
			delete fileServiceDictConfig;
		}
		serviceDictionaryConfigList->push_back(currentServiceDicConfig);
	}
}

void  ProgrammaticConfigure::retrieveDirectoryConfig(const EmaString& dictionaryName, DirectoryServiceStore& dirServiceStrore, DirectoryCache& directoryCache, EmaList<ServiceDictionaryConfig*>* serviceDictionaryConfigList)
{
	if (!_serverDictList.size())
	{
		for (UInt32 i = 0; i < _configList.size(); i++)
			retrieveServerAllDictionaryConfig(*_configList[i]);
	}

	for (UInt32 i = 0; i < _configList.size(); i++)
		retrieveDirectory(*_configList[i], dictionaryName, dirServiceStrore, directoryCache, serviceDictionaryConfigList);
}

void ProgrammaticConfigure::retrieveInstanceCommonConfig( const Map& map, const EmaString& instanceName, EmaConfigErrorList& emaConfigErrList, ActiveConfig& activeConfig )
{
	EmaString groupName;
	EmaString listName;
	retrieveGroupAndListName( map, groupName, listName );

	if ( groupName.empty() )
		return;

	map.reset();
	while ( map.forth() )
	{
		const MapEntry& mapEntry = map.getEntry();

		if ( mapEntry.getKey().getDataType() == DataType::AsciiEnum && 
			( ( mapEntry.getKey().getAscii() == "ConsumerGroup" ) ||
			( mapEntry.getKey().getAscii() == "NiProviderGroup" ) ) )
		{
			if ( mapEntry.getLoadType() == DataType::ElementListEnum )
			{
				const ElementList& elementList = mapEntry.getElementList();

				while ( elementList.forth() )
				{
					const ElementEntry& elementEntry = elementList.getEntry();

					if ( elementEntry.getLoadType() == DataType::MapEnum )
					{
						if ( ( elementEntry.getName() == "ConsumerList" ) ||
							( elementEntry.getName() == "NiProviderList" ) )
						{
							const Map& instanceMap = elementEntry.getMap();

							while ( instanceMap.forth() )
							{
								const MapEntry& mapEntry = instanceMap.getEntry();

								if ( ( mapEntry.getKey().getDataType() == DataType::AsciiEnum ) && ( mapEntry.getKey().getAscii() == instanceName ) )
								{
									if ( mapEntry.getLoadType() == DataType::ElementListEnum )
									{
										const ElementList& elementListInstance = mapEntry.getElementList();

										while ( elementListInstance.forth() )
										{
											const ElementEntry& eentry = elementListInstance.getEntry();

											switch ( eentry.getLoadType() )
											{
											case DataType::AsciiEnum:
												if (eentry.getName() == "XmlTraceFileName")
												{
													activeConfig.xmlTraceFileName = eentry.getAscii();
												}
												else if (eentry.getName() == "LibsslName")
												{
													if (activeConfig.libSslName.length() == 0)
														activeConfig.libSslName = eentry.getAscii();
												}
												else if (eentry.getName() == "LibcryptoName")
												{
													if (activeConfig.libCryptoName.length() == 0)
														activeConfig.libCryptoName = eentry.getAscii();
												}
												else if (eentry.getName() == "LibcurlName")
												{
													if (activeConfig.libcurlName.length() == 0)
														activeConfig.libcurlName = eentry.getAscii();
												}
												break;

											case DataType::UIntEnum:
												if ( eentry.getName() == "ItemCountHint" )
												{
													activeConfig.setItemCountHint( eentry.getUInt() );
												}
												else if ( eentry.getName() == "ServiceCountHint" )
												{
													activeConfig.setServiceCountHint( eentry.getUInt() );
												}
												else if ( eentry.getName() == "RequestTimeout" )
												{
													activeConfig.setRequestTimeout( eentry.getUInt() );
												}
												else if ( eentry.getName() == "CatchUnhandledException" )
												{
													activeConfig.setCatchUnhandledException( eentry.getUInt() );
												}
												else if ( eentry.getName() == "MaxDispatchCountApiThread" )
												{
													activeConfig.setMaxDispatchCountApiThread( eentry.getUInt() );
												}
												else if ( eentry.getName() == "MaxDispatchCountUserThread" )
												{
													activeConfig.setMaxDispatchCountUserThread( eentry.getUInt() );
												}
												else if (eentry.getName() == "XmlTraceToFile")
												{
													activeConfig.xmlTraceToFile = eentry.getUInt() ? true : false;
												}
												else if (eentry.getName() == "XmlTraceToStdout")
												{
													activeConfig.xmlTraceToStdout = eentry.getUInt() ? true : false;
												}
												else if (eentry.getName() == "XmlTraceToMultipleFiles")
												{
													activeConfig.xmlTraceToMultipleFiles = eentry.getUInt() ? true : false;
												}
												else if (eentry.getName() == "XmlTraceWrite")
												{
													activeConfig.xmlTraceWrite = eentry.getUInt() ? true : false;
												}
												else if (eentry.getName() == "XmlTraceRead")
												{
													activeConfig.xmlTraceRead = eentry.getUInt() ? true : false;
												}
												else if (eentry.getName() == "XmlTracePing")
												{
													activeConfig.xmlTracePing = eentry.getUInt() ? true : false;
												}
												else if (eentry.getName() == "XmlTraceHex")
												{
													activeConfig.xmlTraceHex = eentry.getUInt() ? true : false;
												}
												else if (eentry.getName() == "XmlTraceDump")
												{
													activeConfig.xmlTraceDump = eentry.getUInt() ? true : false;
												}
												else if (eentry.getName() == "MsgKeyInUpdates")
												{
													activeConfig.msgKeyInUpdates = eentry.getUInt() ? true : false;
												}
												else if ( eentry.getName() == "LoginRequestTimeOut" )
												{
													activeConfig.setLoginRequestTimeOut( eentry.getUInt() );
												}
												else if (eentry.getName() == "RestRequestTimeOut")
												{
													activeConfig.setRestRequestTimeOut( eentry.getUInt() );
												}
												else if (eentry.getName() == "DefaultServiceID")
												{
													activeConfig.defaultServiceIDForConverter = eentry.getUInt() <= 0xFFFF ? (RsslUInt16)eentry.getUInt() : 0xFFFF;
												}
												else if (eentry.getName() == "JsonExpandedEnumFields")
												{
													activeConfig.jsonExpandedEnumFields = eentry.getUInt() ? true : false;
												}
												else if (eentry.getName() == "CatchUnknownJsonFids")
												{
													activeConfig.catchUnknownJsonFids = eentry.getUInt() ? true : false;
												}
												else if (eentry.getName() == "CatchUnknownJsonKeys")
												{
													activeConfig.catchUnknownJsonKeys = eentry.getUInt() ? true : false;
												}
												else if (eentry.getName() == "CloseChannelFromConverterFailure")
												{
													activeConfig.closeChannelFromFailure = eentry.getUInt() ? true : false;
												}
												else if (eentry.getName() == "OutputBufferSize")
												{
													activeConfig.outputBufferSize = eentry.getUInt() <= 0xFFFFFFFF ? (RsslUInt32)eentry.getUInt() : 0xFFFFFFFF;
												}
											
												break;

											case DataType::IntEnum:

												if ( eentry.getName() == "DispatchTimeoutApiThread" )
												{
													activeConfig.dispatchTimeoutApiThread = eentry.getInt();
												}
												else if (eentry.getName() == "XmlTraceMaxFileSize")
												{
													activeConfig.xmlTraceMaxFileSize = eentry.getInt();
												}
												else if (eentry.getName() == "ReconnectAttemptLimit")
												{
													if (eentry.getInt() >= -1)
														activeConfig.reconnectAttemptLimit = eentry.getInt() > 0x7FFFFFFF ? 0x7FFFFFFF : (Int32)eentry.getInt();
												}
												else if (eentry.getName() == "ReconnectMinDelay")
												{
													activeConfig.setReconnectMinDelay( eentry.getInt() );
												}
												else if (eentry.getName() == "ReconnectMaxDelay")
												{
													activeConfig.setReconnectMaxDelay( eentry.getInt() );
												}
												else if ( eentry.getName() == "PipePort" )
												{
													activeConfig.pipePort = eentry.getInt();
												}
												else if (eentry.getName() == "ReissueTokenAttemptLimit")
												{
													activeConfig.reissueTokenAttemptLimit = eentry.getInt();
												}
												else if (eentry.getName() == "ReissueTokenAttemptInterval")
												{
													activeConfig.reissueTokenAttemptInterval = eentry.getInt();
												}
												else if (eentry.getName() == "MaxEventsInPool")
												{
													activeConfig.setMaxEventsInPool(eentry.getInt());
												}
												break;

											case DataType::DoubleEnum:

												if (eentry.getName() == "TokenReissueRatio")
												{
													activeConfig.tokenReissueRatio = eentry.getDouble();
												}

												break;
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
}

void ProgrammaticConfigure::retrieveInstanceCommonConfig(const Map& map, const EmaString& instanceName, EmaConfigErrorList& emaConfigErrList, ActiveServerConfig& activeConfig)
{
	EmaString groupName;
	EmaString listName;
	retrieveGroupAndListName(map, groupName, listName);

	if (groupName.empty())
		return;

	map.reset();
	while (map.forth())
	{
		const MapEntry& mapEntry = map.getEntry();

		if (mapEntry.getKey().getDataType() == DataType::AsciiEnum &&
			mapEntry.getKey().getAscii() == "IProviderGroup" && 
			mapEntry.getLoadType() == DataType::ElementListEnum)
		{
			const ElementList& elementList = mapEntry.getElementList();

			while (elementList.forth())
			{
				const ElementEntry& elementEntry = elementList.getEntry();

				if (elementEntry.getLoadType() == DataType::MapEnum && elementEntry.getName() == "IProviderList")
				{
					const Map& instanceMap = elementEntry.getMap();

					while (instanceMap.forth())
					{
						const MapEntry& mapEntry = instanceMap.getEntry();

						if ((mapEntry.getKey().getDataType() == DataType::AsciiEnum) &&
							(mapEntry.getKey().getAscii() == instanceName &&
							mapEntry.getLoadType() == DataType::ElementListEnum))
						{
							const ElementList& elementListInstance = mapEntry.getElementList();

							while (elementListInstance.forth())
							{
								const ElementEntry& eentry = elementListInstance.getEntry();

								switch (eentry.getLoadType())
								{
								case DataType::AsciiEnum:
									if (eentry.getName() == "XmlTraceFileName")
									{
										activeConfig.xmlTraceFileName = eentry.getAscii();
									}
									break;

								case DataType::UIntEnum:
									if (eentry.getName() == "ItemCountHint")
									{
										activeConfig.setItemCountHint(eentry.getUInt());
									}
									else if (eentry.getName() == "ServiceCountHint")
									{
										activeConfig.setServiceCountHint(eentry.getUInt());
									}
									else if (eentry.getName() == "RequestTimeout")
									{
										activeConfig.setRequestTimeout(eentry.getUInt());
									}
									else if (eentry.getName() == "CatchUnhandledException")
									{
										activeConfig.setCatchUnhandledException(eentry.getUInt());
									}
									else if (eentry.getName() == "MaxDispatchCountApiThread")
									{
										activeConfig.setMaxDispatchCountApiThread(eentry.getUInt());
									}
									else if (eentry.getName() == "MaxDispatchCountUserThread")
									{
										activeConfig.setMaxDispatchCountUserThread(eentry.getUInt());
									}
									else if (eentry.getName() == "XmlTraceToFile")
									{
										activeConfig.xmlTraceToFile = eentry.getUInt() ? true : false;
									}
									else if (eentry.getName() == "XmlTraceToStdout")
									{
										activeConfig.xmlTraceToStdout = eentry.getUInt() ? true : false;
									}
									else if (eentry.getName() == "XmlTraceToMultipleFiles")
									{
										activeConfig.xmlTraceToMultipleFiles = eentry.getUInt() ? true : false;
									}
									else if (eentry.getName() == "XmlTraceWrite")
									{
										activeConfig.xmlTraceWrite = eentry.getUInt() ? true : false;
									}
									else if (eentry.getName() == "XmlTraceRead")
									{
										activeConfig.xmlTraceRead = eentry.getUInt() ? true : false;
									}
									else if (eentry.getName() == "XmlTracePing")
									{
										activeConfig.xmlTracePing = eentry.getUInt() ? true : false;
									}
									else if (eentry.getName() == "XmlTraceHex")
									{
										activeConfig.xmlTraceHex = eentry.getUInt() ? true : false;
									}
									else if (eentry.getName() == "XmlTraceDump")
									{
										activeConfig.xmlTraceDump = eentry.getUInt() ? true : false;
									}
									else if (eentry.getName() == "DefaultServiceID")
									{
										activeConfig.defaultServiceIDForConverter = eentry.getUInt() <= 0xFFFF ? (RsslUInt16)eentry.getUInt() : 0xFFFF;
									}
									else if (eentry.getName() == "JsonExpandedEnumFields")
									{
										activeConfig.jsonExpandedEnumFields = eentry.getUInt() ? true : false;
									}
									else if (eentry.getName() == "CatchUnknownJsonFids")
									{
										activeConfig.catchUnknownJsonFids = eentry.getUInt() ? true : false;
									}
									else if (eentry.getName() == "CatchUnknownJsonKeys")
									{
										activeConfig.catchUnknownJsonKeys = eentry.getUInt() ? true : false;
									}
									else if (eentry.getName() == "CloseChannelFromConverterFailure")
									{
										activeConfig.closeChannelFromFailure = eentry.getUInt() ? true : false;
									}
									else if (eentry.getName() == "OutputBufferSize")
									{
										activeConfig.outputBufferSize = eentry.getUInt() <= 0xFFFFFFFF ? (RsslUInt32)eentry.getUInt() : 0xFFFFFFFF;
									}

									break;

								case DataType::IntEnum:

									if (eentry.getName() == "DispatchTimeoutApiThread")
									{
										activeConfig.dispatchTimeoutApiThread = eentry.getInt();
									}
									else if (eentry.getName() == "XmlTraceMaxFileSize")
									{
										activeConfig.xmlTraceMaxFileSize = eentry.getInt();
									}
									else if (eentry.getName() == "PipePort")
									{
										activeConfig.pipePort = eentry.getInt();
									}
									else if (eentry.getName() == "MaxEventsInPool")
									{
										activeConfig.setMaxEventsInPool(eentry.getInt());
									}
									break;
								}
							}
						}
					}
				}
			}
		}
	}
}

void ProgrammaticConfigure::retrieveInstanceCustomConfig( const Map& map, const EmaString& instanceName, EmaConfigErrorList& emaConfigErrList, BaseConfig& activeConfig )
{
	EmaString groupName;
	EmaString listName;
	retrieveGroupAndListName( map, groupName, listName );

	if ( groupName.empty() )
		return;

	map.reset();
	while ( map.forth() )
	{
		const MapEntry& mapEntry = map.getEntry();

		if ( mapEntry.getKey().getDataType() == DataType::AsciiEnum && mapEntry.getKey().getAscii() == "ConsumerGroup" )
		{
			if ( mapEntry.getLoadType() == DataType::ElementListEnum )
			{
				const ElementList& elementList = mapEntry.getElementList();

				while ( elementList.forth() )
				{
					const ElementEntry& elementEntry = elementList.getEntry();

					if ( elementEntry.getLoadType() == DataType::MapEnum )
					{
						if ( elementEntry.getName() == "ConsumerList" )
						{
							const Map& mapInstance = elementEntry.getMap();

							while ( mapInstance.forth() )
							{
								const MapEntry& mapEntry = mapInstance.getEntry();

								if ( ( mapEntry.getKey().getDataType() == DataType::AsciiEnum ) && ( mapEntry.getKey().getAscii() == instanceName ) )
								{
									if ( mapEntry.getLoadType() == DataType::ElementListEnum )
									{
										const ElementList& elementListInstance = mapEntry.getElementList();

										while ( elementListInstance.forth() )
										{
											const ElementEntry& eentry = elementListInstance.getEntry();

											switch ( eentry.getLoadType() )
											{
											case DataType::AsciiEnum:
												break;

											case DataType::UIntEnum:
												if ( eentry.getName() == "ObeyOpenWindow" )
												{
													static_cast<ActiveConfig&>(activeConfig).setObeyOpenWindow( eentry.getUInt() );
												}
												else if ( eentry.getName() == "PostAckTimeout" )
												{
													static_cast<ActiveConfig&>(activeConfig).setPostAckTimeout( eentry.getUInt() );
												}
												else if ( eentry.getName() == "MaxOutstandingPosts" )
												{
													static_cast<ActiveConfig&>(activeConfig).setMaxOutstandingPosts( eentry.getUInt() );
												}
												else if ( eentry.getName() == "DirectoryRequestTimeOut" )
												{
													static_cast<ActiveConfig&>(activeConfig).setDirectoryRequestTimeOut( eentry.getUInt() );
												}
												else if ( eentry.getName() == "DictionaryRequestTimeOut" )
												{
													static_cast<ActiveConfig&>(activeConfig).setDictionaryRequestTimeOut( eentry.getUInt() );
												}
												break;

											case DataType::IntEnum:
												break;
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
		else if ( mapEntry.getKey().getDataType() == DataType::AsciiEnum && mapEntry.getKey().getAscii() == "NiProviderGroup" )
		{
			if ( mapEntry.getLoadType() == DataType::ElementListEnum )
			{
				const ElementList& elementList = mapEntry.getElementList();

				while ( elementList.forth() )
				{
					const ElementEntry& elementEntry = elementList.getEntry();

					if ( elementEntry.getLoadType() == DataType::MapEnum )
					{
						if ( elementEntry.getName() == "NiProviderList" )
						{
							const Map& map = elementEntry.getMap();

							while ( map.forth() )
							{
								const MapEntry& mapEntry = map.getEntry();

								if ( ( mapEntry.getKey().getDataType() == DataType::AsciiEnum ) && ( mapEntry.getKey().getAscii() == instanceName ) )
								{
									if ( mapEntry.getLoadType() == DataType::ElementListEnum )
									{
										const ElementList& elementListInstance = mapEntry.getElementList();

										while ( elementListInstance.forth() )
										{
											const ElementEntry& eentry = elementListInstance.getEntry();

											switch ( eentry.getLoadType() )
											{
											case DataType::UIntEnum:
												if (eentry.getName() == "RefreshFirstRequired")
												{
													static_cast<OmmNiProviderActiveConfig&>(activeConfig).setRefreshFirstRequired(eentry.getUInt());
												}
												else if (eentry.getName() == "MergeSourceDirectoryStreams")
												{
													static_cast<OmmNiProviderActiveConfig&>(activeConfig).setMergeSourceDirectoryStreams(eentry.getUInt());
												}
												else if (eentry.getName() == "RecoverUserSubmitSourceDirectory")
												{
													static_cast<OmmNiProviderActiveConfig&>(activeConfig).setRecoverUserSubmitSourceDirectory(eentry.getUInt());
												}
												else if (eentry.getName() == "RemoveItemsOnDisconnect")
												{
													static_cast<OmmNiProviderActiveConfig&>(activeConfig).setRemoveItemsOnDisconnect(eentry.getUInt());
												}

												break;
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
		else if (mapEntry.getKey().getDataType() == DataType::AsciiEnum && mapEntry.getKey().getAscii() == "IProviderGroup")
		{
			if (mapEntry.getLoadType() == DataType::ElementListEnum)
			{
				const ElementList& elementList = mapEntry.getElementList();

				while (elementList.forth())
				{
					const ElementEntry& elementEntry = elementList.getEntry();

					if (elementEntry.getLoadType() == DataType::MapEnum)
					{
						if (elementEntry.getName() == "IProviderList")
						{
							const Map& map = elementEntry.getMap();

							while (map.forth())
							{
								const MapEntry& mapEntry = map.getEntry();

								if ((mapEntry.getKey().getDataType() == DataType::AsciiEnum) && (mapEntry.getKey().getAscii() == instanceName))
								{
									if (mapEntry.getLoadType() == DataType::ElementListEnum)
									{
										const ElementList& elementListInstance = mapEntry.getElementList();

										while (elementListInstance.forth())
										{
											const ElementEntry& eentry = elementListInstance.getEntry();

											switch (eentry.getLoadType())
											{
											case DataType::UIntEnum:
												if (eentry.getName() == "AcceptDirMessageWithoutMinFilters")
												{
													static_cast<ActiveServerConfig&>(activeConfig).acceptDirMessageWithoutMinFilters = (eentry.getUInt() > 0 ? true : false);
												}
												else if (eentry.getName() == "AcceptMessageSameKeyButDiffStream")
												{
													static_cast<ActiveServerConfig&>(activeConfig).acceptMessageSameKeyButDiffStream = (eentry.getUInt() > 0 ? true : false);
												}
												else if (eentry.getName() == "AcceptMessageThatChangesService")
												{
													static_cast<ActiveServerConfig&>(activeConfig).acceptMessageThatChangesService = (eentry.getUInt() > 0 ? true : false);
												}
												else if (eentry.getName() == "AcceptMessageWithoutAcceptingRequests")
												{
													static_cast<ActiveServerConfig&>(activeConfig).acceptMessageWithoutAcceptingRequests = (eentry.getUInt() > 0 ? true : false);
												}
												else if (eentry.getName() == "AcceptMessageWithoutBeingLogin")
												{
													static_cast<ActiveServerConfig&>(activeConfig).acceptMessageWithoutBeingLogin = (eentry.getUInt() > 0 ? true : false);
												}
												else if (eentry.getName() == "AcceptMessageWithoutQosInRange")
												{
													static_cast<ActiveServerConfig&>(activeConfig).acceptMessageWithoutQosInRange = (eentry.getUInt() > 0 ? true : false);
												}
												else if (eentry.getName() == "RefreshFirstRequired")
												{
													static_cast<OmmIProviderActiveConfig&>(activeConfig).setRefreshFirstRequired(eentry.getUInt());
												}
												else if (eentry.getName() == "EnumTypeFragmentSize")
												{
													static_cast<OmmIProviderActiveConfig&>(activeConfig).setMaxEnumTypeFragmentSize(eentry.getUInt());
												}
												else if (eentry.getName() == "FieldDictionaryFragmentSize")
												{
													static_cast<OmmIProviderActiveConfig&>(activeConfig).setMaxFieldDictFragmentSize(eentry.getUInt());
												}
												break;
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
}

void ProgrammaticConfigure::retrieveChannel( const Map& map, const EmaString& channelName, EmaConfigErrorList& emaConfigErrList,
    ActiveConfig& activeConfig, int hostFnCalled, ChannelConfig* fileCfg)
{
	map.reset();
	while ( map.forth() )
	{
		const MapEntry& mapEntry = map.getEntry();

		if ( mapEntry.getKey().getDataType() == DataType::AsciiEnum && mapEntry.getKey().getAscii() == "ChannelGroup" )
		{
			if ( mapEntry.getLoadType() == DataType::ElementListEnum )
			{
				const ElementList& elementList = mapEntry.getElementList();

				while ( elementList.forth() )
				{
					const ElementEntry& elementEntry = elementList.getEntry();

					if ( elementEntry.getLoadType() == DataType::MapEnum )
					{
						if ( elementEntry.getName() == "ChannelList" )
						{
							const Map& map = elementEntry.getMap();

							while ( map.forth() )
							{
								const MapEntry& mapEntry = map.getEntry();

								if ( ( mapEntry.getKey().getDataType() == DataType::AsciiEnum ) && ( mapEntry.getKey().getAscii() == channelName ) )
								{
									if ( mapEntry.getLoadType() == DataType::ElementListEnum )
										retrieveChannelInfo( mapEntry, channelName, emaConfigErrList, activeConfig, hostFnCalled, fileCfg);
								}
							}
						}
					}
				}
			}
		}
	}
}

void ProgrammaticConfigure::retrieveServer(const Map& map, const EmaString& serverName, EmaConfigErrorList& emaConfigErrList,
	ActiveServerConfig& activeServerConfig, int portFnCalled, ServerConfig* fileCfg)
{
	map.reset();
	while (map.forth())
	{
		const MapEntry& mapEntry = map.getEntry();

		if (mapEntry.getKey().getDataType() == DataType::AsciiEnum &&
			mapEntry.getKey().getAscii() == "ServerGroup" &&
			mapEntry.getLoadType() == DataType::ElementListEnum)
		{
			const ElementList& elementList = mapEntry.getElementList();

			while (elementList.forth())
			{
				const ElementEntry& elementEntry = elementList.getEntry();

				if (elementEntry.getLoadType() == DataType::MapEnum && elementEntry.getName() == "ServerList")
				{
					const Map& map = elementEntry.getMap();

					while (map.forth())
					{
						const MapEntry& mapEntry = map.getEntry();

						if ((mapEntry.getKey().getDataType() == DataType::AsciiEnum) && (mapEntry.getKey().getAscii() == serverName))
						{
							if (mapEntry.getLoadType() == DataType::ElementListEnum)
								retrieveServerInfo(mapEntry, serverName, emaConfigErrList, activeServerConfig, portFnCalled, fileCfg);
						}
					}
				}
			}
		}
	}
}

void ProgrammaticConfigure::retrieveChannelInfo( const MapEntry& mapEntry, const EmaString& channelName, EmaConfigErrorList& emaConfigErrList,
    ActiveConfig& activeConfig, int setByFnCalled, ChannelConfig* fileCfg)
{
	const ElementList& elementListChannel = mapEntry.getElementList();

	EmaString name, interfaceName, host, port, objectName, tunnelingProxyHost, tunnelingProxyPort, location, sslCAStore, wsProtocols;
	UInt16 channelType, compressionType, encryptedProtocolType;
	UInt64 guaranteedOutputBuffers, compressionThreshold, connectionPingTimeout, numInputBuffers, sysSendBufSize, sysRecvBufSize, highWaterMark,
	       tcpNodelay, enableSessionMgnt, encryptedSslProtocolVer, initializationTimeout, wsMaxMsgSize;

	UInt64 flags = 0;
	UInt64 mcastFlags = 0;
	UInt64 encryptionFlags = 0;
	UInt64 websocketFlags = 0;
	ReliableMcastChannelConfig tempRelMcastCfg;

	while ( elementListChannel.forth() )
	{
		const ElementEntry& channelEntry = elementListChannel.getEntry();

		switch ( channelEntry.getLoadType() )
		{
		case DataType::AsciiEnum:
			if ( channelEntry.getName() == "Host" )
			{
				host = channelEntry.getAscii();
				flags |= 0x02;
			}
			else if ( channelEntry.getName() == "Port" )
			{
				port = channelEntry.getAscii();
				flags |= 0x04;
			}
			else if ( channelEntry.getName() == "InterfaceName" )
			{
				interfaceName = channelEntry.getAscii();
				flags |= 0x80000;
			}
			else if (channelEntry.getName() == "Location")
			{
				location = channelEntry.getAscii();
				flags |= 0x2000000;
			}
			else if ( channelEntry.getName() == "ObjectName" )
			{
				objectName = channelEntry.getAscii();
				encryptionFlags |= 0x02;
			}
			else if (channelEntry.getName() == "ProxyPort")
			{
				tunnelingProxyPort = channelEntry.getAscii();
				encryptionFlags |= 0x04;
			}
			else if (channelEntry.getName() == "ProxyHost")
			{
				tunnelingProxyHost = channelEntry.getAscii();
				encryptionFlags |= 0x08;
			}
			else if (channelEntry.getName() == "OpenSSLCAStore")
			{
				sslCAStore = channelEntry.getAscii();
				encryptionFlags |= 0x40;
			}
			else if ( channelEntry.getName() == "RecvAddress" )
			{
				tempRelMcastCfg.recvAddress = channelEntry.getAscii();
				mcastFlags |= 0x01;
			}
			else if ( channelEntry.getName() == "RecvPort" )
			{
				tempRelMcastCfg.recvServiceName = channelEntry.getAscii();
				mcastFlags |= 0x02;
			}
			else if ( channelEntry.getName() == "SendAddress" )
			{
				tempRelMcastCfg.sendAddress = channelEntry.getAscii();
				mcastFlags |= 0x04;
			}
			else if ( channelEntry.getName() == "SendPort" )
			{
				tempRelMcastCfg.sendServiceName = channelEntry.getAscii();
				mcastFlags |= 0x08;
			}
			else if ( channelEntry.getName() == "UnicastPort" )
			{
				tempRelMcastCfg.unicastServiceName = channelEntry.getAscii();
				mcastFlags |= 0x10;
			}
			else if ( channelEntry.getName() == "HsmInterface" )
			{
				tempRelMcastCfg.hsmInterface = channelEntry.getAscii();
				mcastFlags |= 0x20;
			}
			else if ( channelEntry.getName() == "HsmMultAddress" )
			{
				tempRelMcastCfg.hsmMultAddress = channelEntry.getAscii();
				mcastFlags |= 0x40;
			}
			else if ( channelEntry.getName() == "HsmPort" )
			{
				tempRelMcastCfg.hsmPort = channelEntry.getAscii();
				mcastFlags |= 0x80;
			}
			else if ( channelEntry.getName() == "tcpControlPort" )
			{
				tempRelMcastCfg.tcpControlPort = channelEntry.getAscii();
				mcastFlags |= 0x400000;
			}
			else if (channelEntry.getName() == "WsProtocols")
			{
				wsProtocols = channelEntry.getAscii();
				websocketFlags |= 0x01;
			}
			break;

		case DataType::EnumEnum:
			if ( channelEntry.getName() == "ChannelType" )
			{
				channelType = channelEntry.getEnum();

				switch ( channelType )
				{
				case RSSL_CONN_TYPE_SOCKET:
				case RSSL_CONN_TYPE_RELIABLE_MCAST:
				case RSSL_CONN_TYPE_HTTP:
				case RSSL_CONN_TYPE_ENCRYPTED:
				case RSSL_CONN_TYPE_WEBSOCKET:
					flags |= 0x10;
					break;
				default:
					EmaString text( "Invalid ChannelType [" );
					text.append( channelType );
					text.append( "] in Programmatic Configuration. Use default ChannelType [" );
					text.append( DEFAULT_CONNECTION_TYPE );
					text.append( "]" );
					EmaConfigError* mce( new EmaConfigError( text, OmmLoggerClient::ErrorEnum ) );
					emaConfigErrList.add( mce );
					break;
				}
			}
			else if ( channelEntry.getName() == "CompressionType" )
			{
				compressionType = channelEntry.getEnum();

				if ( compressionType > RSSL_COMP_LZ4)
				{
					EmaString text( "Invalid CompressionType [" );
					text.append( compressionType );
					text.append( "] in Programmatic Configuration. Use default CompressionType [" );
					text.append( DEFAULT_COMPRESSION_TYPE );
					text.append( "] " );
					EmaConfigError* mce( new EmaConfigError( text, OmmLoggerClient::ErrorEnum ) );
					emaConfigErrList.add( mce );
				}
				else
				{
					flags |= 0x20;
				}
			}
			else if (channelEntry.getName() == "EncryptedProtocolType")
			{
				encryptedProtocolType = channelEntry.getEnum();

				switch (encryptedProtocolType)
				{
				case RSSL_CONN_TYPE_SOCKET:
				case RSSL_CONN_TYPE_HTTP:
				case RSSL_CONN_TYPE_WEBSOCKET:
					encryptionFlags |= 0x10;
					break;
				default:
					EmaString text("Invalid Encrypted Channel Type [");
					text.append(channelType);
					text.append("] ");
					EmaConfigError* mce(new EmaConfigError(text, OmmLoggerClient::ErrorEnum));
					emaConfigErrList.add(mce);
					break;
				}
			}
			break;

		case DataType::UIntEnum:
			if ( channelEntry.getName() == "GuaranteedOutputBuffers" )
			{
				guaranteedOutputBuffers = channelEntry.getUInt();
				flags |= 0x40;
			}
			if ( channelEntry.getName() == "NumInputBuffers" )
			{
				numInputBuffers = channelEntry.getUInt();
				flags |= 0x800000;
			}
			if ( channelEntry.getName() == "SysRecvBufSize" )
			{
				sysRecvBufSize = channelEntry.getUInt();
				flags |= 0x200000;
			}
			if ( channelEntry.getName() == "SysSendBufSize" )
			{
				sysSendBufSize = channelEntry.getUInt();
				flags |= 0x400000;
			}
			if ( channelEntry.getName() == "HighWaterMark" )
			{
				highWaterMark = channelEntry.getUInt();
				flags |= 0x8000000;
			}
			else if ( channelEntry.getName() == "TcpNodelay" )
			{
				tcpNodelay = channelEntry.getUInt();
				flags |= 0x80;
			}
			else if (channelEntry.getName() == "ConnectionPingTimeout" )
			{
				connectionPingTimeout = channelEntry.getUInt();
				flags |= 0x8000;
			}
			else if ( channelEntry.getName() == "CompressionThreshold" )
			{
				compressionThreshold = channelEntry.getUInt();
				flags |= 0x1000000;
			}
			else if (channelEntry.getName() == "InitializationTimeout")
			{
				initializationTimeout = channelEntry.getUInt();
				flags |= 0x2000000;
			}
			else if (channelEntry.getName() == "EnableSessionManagement")
			{
				enableSessionMgnt = channelEntry.getUInt();
				flags |= 0x4000000;
			}
			else if (channelEntry.getName() == "SecurityProtocol")
			{
				encryptedSslProtocolVer = channelEntry.getUInt();
				encryptionFlags |= 0x20;
			}
			else if ( channelEntry.getName() == "PacketTTL" )
			{
				tempRelMcastCfg.setPacketTTL( channelEntry.getUInt() );
				mcastFlags |= 0x100;
			}
			else if ( channelEntry.getName() == "DisconnectOnGap" )
			{
				tempRelMcastCfg.disconnectOnGap = channelEntry.getUInt()  ? true : false;
				mcastFlags |= 0x200;
			}
			else if ( channelEntry.getName() == "HsmInterval" )
			{
				tempRelMcastCfg.setHsmInterval( channelEntry.getUInt() );
				mcastFlags |= 0x400;
			}
			else if ( channelEntry.getName() == "ndata" )
			{
				tempRelMcastCfg.setNdata( channelEntry.getUInt() );
				mcastFlags |= 0x800;
			}
			else if ( channelEntry.getName() == "nmissing" )
			{
				tempRelMcastCfg.setNmissing( channelEntry.getUInt() );
				mcastFlags |= 0x1000;
			}
			else if ( channelEntry.getName() == "nrreq" )
			{
				tempRelMcastCfg.setNrreq( channelEntry.getUInt() );
				mcastFlags |= 0x2000;
			}
			else if ( channelEntry.getName() == "tdata" )
			{
				tempRelMcastCfg.setTdata( channelEntry.getUInt() );
				mcastFlags |= 0x4000;
			}
			else if ( channelEntry.getName() == "trreq" )
			{
				tempRelMcastCfg.setTrreq( channelEntry.getUInt() );
				mcastFlags |= 0x8000;
			}
			else if ( channelEntry.getName() == "pktPoolLimitHigh" )
			{
				tempRelMcastCfg.setPktPoolLimitHigh( channelEntry.getUInt() );
				mcastFlags |= 0x10000;
			}
			else if ( channelEntry.getName() == "pktPoolLimitLow" )
			{
				tempRelMcastCfg.setPktPoolLimitLow( channelEntry.getUInt() );
				mcastFlags |= 0x20000;
			}
			else if ( channelEntry.getName() == "twait" )
			{
				tempRelMcastCfg.setTwait( channelEntry.getUInt() );
				mcastFlags |= 0x40000;
			}
			else if ( channelEntry.getName() == "tbchold" )
			{
				tempRelMcastCfg.setTbchold( channelEntry.getUInt() );
				mcastFlags |= 0x80000;
			}
			else if ( channelEntry.getName() == "tpphold" )
			{
				tempRelMcastCfg.setTpphold( channelEntry.getUInt() );
				mcastFlags |= 0x100000;
			}
			else if ( channelEntry.getName() == "userQLimit" )
			{
				tempRelMcastCfg.setUserQLimit( channelEntry.getUInt() );
				mcastFlags |= 0x200000;
			}
			else if (channelEntry.getName() == "WsMaxMsgSize")
			{
				wsMaxMsgSize = channelEntry.getUInt();
				websocketFlags |= 0x02;
			}
			break;
		}
	}

	if ( flags & 0x10 )
	{
		if ( setByFnCalled & (SOCKET_CONN_HOST_CONFIG_BY_FUNCTION_CALL | SOCKET_SERVER_PORT_CONFIG_BY_FUNCTION_CALL) )
		{
			channelType = RSSL_CONN_TYPE_SOCKET;
			activeConfig.clearChannelSet();
		}

		unsigned int positionFound = 0;
		ChannelConfig* pCurrentChannelConfig = 0;

		try
		{
			if ( channelType == RSSL_CONN_TYPE_RELIABLE_MCAST )
			{
				ReliableMcastChannelConfig* reliableMcastChannelCfg = new ReliableMcastChannelConfig();
				pCurrentChannelConfig = reliableMcastChannelCfg;
				EmaString errorMsg;

				if ( setReliableMcastChannelInfo( reliableMcastChannelCfg, mcastFlags, tempRelMcastCfg, errorMsg, fileCfg ) )
					activeConfig.configChannelSet.push_back( pCurrentChannelConfig );
				else
				{
					throwIceException( errorMsg );
					return;
				}
			}
			else if (channelType == RSSL_CONN_TYPE_SOCKET || channelType == RSSL_CONN_TYPE_ENCRYPTED || channelType == RSSL_CONN_TYPE_HTTP || channelType == RSSL_CONN_TYPE_WEBSOCKET)
			{
				SocketChannelConfig* socketChannelConfig;

				if (channelType == RSSL_CONN_TYPE_ENCRYPTED)
				{
					/*	Both host and port is set as empty string by default to support the Reactor's session management
						to query them from EDP-RT service discovery when the SocketChannelConfig.enableSessionMgnt is set to true.
					*/
					socketChannelConfig = new SocketChannelConfig("", "", (RsslConnectionTypes)channelType);
					socketChannelConfig->initializationTimeout = DEFAULT_INITIALIZATION_TIMEOUT_ENCRYPTED_CON;
				}
				else
				{
					socketChannelConfig = new SocketChannelConfig(DEFAULT_HOST_NAME, activeConfig.defaultServiceName(), (RsslConnectionTypes)channelType);
				}

				pCurrentChannelConfig = socketChannelConfig;
				activeConfig.configChannelSet.push_back(pCurrentChannelConfig);

				SocketChannelConfig* fileCfgSocket = NULL;
				if (fileCfg && ((fileCfg->connectionType == RSSL_CONN_TYPE_SOCKET) || (fileCfg->connectionType == RSSL_CONN_TYPE_ENCRYPTED) || (fileCfg->connectionType == RSSL_CONN_TYPE_HTTP) || (fileCfg->connectionType == RSSL_CONN_TYPE_WEBSOCKET)))
					fileCfgSocket = static_cast<SocketChannelConfig*>(fileCfg);

				if (flags & 0x80)
					socketChannelConfig->tcpNodelay = tcpNodelay ? true : false;
				else if (fileCfgSocket)
					socketChannelConfig->tcpNodelay = fileCfgSocket->tcpNodelay;

				if (flags & 0x02 && !(setByFnCalled & SOCKET_CONN_HOST_CONFIG_BY_FUNCTION_CALL))
					socketChannelConfig->hostName = host;
				else if (fileCfgSocket)
					socketChannelConfig->hostName = fileCfgSocket->hostName;

				if (flags & 0x04 && !(setByFnCalled & SOCKET_SERVER_PORT_CONFIG_BY_FUNCTION_CALL))
					socketChannelConfig->serviceName = port;
				else if (fileCfgSocket)
					socketChannelConfig->serviceName = fileCfgSocket->serviceName;

				if (encryptionFlags & 0x02 && !(setByFnCalled & TUNNELING_OBJNAME_CONFIG_BY_FUNCTION_CALL))
					socketChannelConfig->objectName = objectName;
				else if (fileCfgSocket)
					socketChannelConfig->objectName = fileCfgSocket->objectName;

				if (encryptionFlags & 0x04 && !(setByFnCalled & PROXY_PORT_CONFIG_BY_FUNCTION_CALL))
					socketChannelConfig->proxyPort = tunnelingProxyPort;
				else if (fileCfgSocket)
					socketChannelConfig->proxyPort = fileCfgSocket->proxyPort;

				if (encryptionFlags & 0x08 && !(setByFnCalled & PROXY_HOST_CONFIG_BY_FUNCTION_CALL))
					socketChannelConfig->proxyHostName = tunnelingProxyHost;
				else if (fileCfgSocket)
					socketChannelConfig->proxyHostName = fileCfgSocket->proxyHostName;

				if (encryptionFlags & 0x10)
					socketChannelConfig->encryptedConnectionType = (RsslConnectionTypes)encryptedProtocolType;
				else if (fileCfgSocket)
					socketChannelConfig->encryptedConnectionType = fileCfgSocket->encryptedConnectionType;

				if (encryptionFlags & 0x20)
					socketChannelConfig->securityProtocol = (int)encryptedSslProtocolVer;
				else if (fileCfgSocket)
					socketChannelConfig->securityProtocol = fileCfgSocket->securityProtocol;

				if (encryptionFlags & 0x40)
					socketChannelConfig->sslCAStore = sslCAStore;
				else if (fileCfgSocket)
					socketChannelConfig->sslCAStore = fileCfgSocket->sslCAStore;

				if (channelType == RSSL_CONN_TYPE_WEBSOCKET)
				{
					if (websocketFlags & 0x01)
						socketChannelConfig->wsProtocols = wsProtocols;
					else if (fileCfgSocket)
						socketChannelConfig->wsProtocols = fileCfgSocket->wsProtocols;

					if (websocketFlags & 0x02)
						socketChannelConfig->wsMaxMsgSize = wsMaxMsgSize;
					else if (fileCfgSocket)
						socketChannelConfig->wsMaxMsgSize = fileCfgSocket->wsMaxMsgSize;
				}

				if ((setByFnCalled & PROXY_USERNAME_CONFIG_BY_FUNCTION_CALL) && fileCfgSocket)
					socketChannelConfig->proxyUserName = fileCfgSocket->proxyUserName;

				if ((setByFnCalled & PROXY_PASSWD_CONFIG_BY_FUNCTION_CALL) && fileCfgSocket)
					socketChannelConfig->proxyPasswd = fileCfgSocket->proxyPasswd;

				if ((setByFnCalled & PROXY_DOMAIN_CONFIG_BY_FUNCTION_CALL) && fileCfgSocket)
					socketChannelConfig->proxyDomain = fileCfgSocket->proxyDomain;

				if (channelType == RSSL_CONN_TYPE_ENCRYPTED)
				{
					if (flags & 0x2000000)
						socketChannelConfig->location = location;
					else if (fileCfgSocket && fileCfgSocket->connectionType == RSSL_CONN_TYPE_ENCRYPTED)
						socketChannelConfig->location = fileCfgSocket->location;

					if (flags & 0x4000000)
						socketChannelConfig->enableSessionMgnt = (RsslBool)enableSessionMgnt;
					else if (fileCfgSocket && fileCfgSocket->connectionType == RSSL_CONN_TYPE_ENCRYPTED)
						socketChannelConfig->enableSessionMgnt = fileCfgSocket->enableSessionMgnt;

					//need to copy other tunneling setting from function calls.
					if (fileCfgSocket && fileCfgSocket->connectionType == RSSL_CONN_TYPE_ENCRYPTED)
					{
						socketChannelConfig->securityProtocol = fileCfgSocket->securityProtocol;
					}
				}
			}
		}
		catch ( std::bad_alloc& )
		{
			const char* temp = "Failed to allocate memory for ChannelConfig. Out of memory!";
			throwMeeException( temp );
		}

		pCurrentChannelConfig->name = channelName;

		bool useFileCfg = ( fileCfg && fileCfg->connectionType == pCurrentChannelConfig->connectionType ) ? true : false;

		if ( flags & 0x80000 )
			pCurrentChannelConfig->interfaceName = interfaceName;
		else if ( useFileCfg )
			pCurrentChannelConfig->interfaceName = fileCfg->interfaceName;

		if ( channelType != RSSL_CONN_TYPE_RELIABLE_MCAST )
		{
			if ( flags & 0x20 )
				pCurrentChannelConfig->compressionType = ( RsslCompTypes )compressionType;
			else if ( useFileCfg )
				pCurrentChannelConfig->compressionType = fileCfg->compressionType;

			if ( flags & 0x1000000 )
				pCurrentChannelConfig->compressionThreshold = compressionThreshold > MAX_UNSIGNED_INT32 ? MAX_UNSIGNED_INT32 : ( UInt32 )compressionThreshold;
			else if ( useFileCfg ) {
				pCurrentChannelConfig->compressionThreshold = fileCfg->compressionThreshold;

				/* unfortunately fileCfg->compressionThreshold is set regardless of whether or
				 * not "compressionThreshold" appears in the configuration file. Thus, if the
				 * user sets compression to LZ4 programmatically and the compressionThreshold
				 * equals the ZLib default, we'll set the threshold to the LZ4 default. In
				 * theory, this may override a user setting in the configuration file, but only
				 * in the case where the user set it to 30 which is invalid for LZ4 anyway.
				 */
				if ( flags & 0x20 && compressionType == RSSL_COMP_LZ4 &&
					 pCurrentChannelConfig->compressionThreshold == DEFAULT_COMPRESSION_THRESHOLD)
				  pCurrentChannelConfig->compressionThreshold = DEFAULT_COMPRESSION_THRESHOLD_LZ4;
			}
		}

		if ( flags & 0x40 )
			pCurrentChannelConfig->setGuaranteedOutputBuffers( guaranteedOutputBuffers );
		else if ( useFileCfg )
			pCurrentChannelConfig->guaranteedOutputBuffers = fileCfg->guaranteedOutputBuffers;

		if ( flags & 0x800000 )
			pCurrentChannelConfig->setNumInputBuffers( numInputBuffers );
		else if ( useFileCfg )
			pCurrentChannelConfig->numInputBuffers = fileCfg->numInputBuffers;

		if ( flags & 0x200000 )
			pCurrentChannelConfig->sysRecvBufSize = sysRecvBufSize > MAX_UNSIGNED_INT32 ? MAX_UNSIGNED_INT32 : ( UInt32 )sysRecvBufSize;
		else if ( useFileCfg )
			pCurrentChannelConfig->sysRecvBufSize = fileCfg->sysRecvBufSize;

		if ( flags & 0x400000 )
			pCurrentChannelConfig->sysSendBufSize = sysSendBufSize > MAX_UNSIGNED_INT32 ? MAX_UNSIGNED_INT32 : ( UInt32 )sysSendBufSize;
		else if ( useFileCfg )
			pCurrentChannelConfig->sysSendBufSize = fileCfg->sysSendBufSize;

		if ( flags & 0x8000000 )
			pCurrentChannelConfig->highWaterMark = highWaterMark > MAX_UNSIGNED_INT32 ? MAX_UNSIGNED_INT32 : (UInt32) highWaterMark;
		else if ( useFileCfg )
			pCurrentChannelConfig->highWaterMark = fileCfg->highWaterMark;

		if ( flags & 0x8000 )
			pCurrentChannelConfig->connectionPingTimeout = connectionPingTimeout > MAX_UNSIGNED_INT32  ? MAX_UNSIGNED_INT32 : ( UInt32 )connectionPingTimeout;
		else if ( useFileCfg )
			pCurrentChannelConfig->connectionPingTimeout = fileCfg->connectionPingTimeout;

		if (flags & 0x2000000)
			pCurrentChannelConfig->initializationTimeout = initializationTimeout > MAX_UNSIGNED_INT32 ? MAX_UNSIGNED_INT32 : (UInt32)initializationTimeout;
		else if (useFileCfg)
			pCurrentChannelConfig->initializationTimeout = fileCfg->initializationTimeout;
	}
}


void ProgrammaticConfigure::retrieveServerInfo(const MapEntry& mapEntry, const EmaString& serverName, EmaConfigErrorList& emaConfigErrList,
	ActiveServerConfig& activeServerConfig, int setByFnCalled, ServerConfig* fileCfg)
{
	const ElementList& elementListServer = mapEntry.getElementList();

	EmaString name, interfaceName, port, serverCert, serverPrivateKey, dhParams, cipherSuite, libSslName, libCryptoName, libCurlName, wsProtocols;
	UInt16 serverType, compressionType;
	UInt64 guaranteedOutputBuffers, compressionThreshold, connectionMinPingTimeout, connectionPingTimeout, numInputBuffers, sysSendBufSize, sysRecvBufSize, highWaterMark,
		tcpNodelay, initializationTimeout, maxFragmentSize;

	UInt64 flags = 0;
	UInt64 mcastFlags = 0;
	UInt64 tunnelingFlags = 0;
	UInt64 websocketFlags = 0;
	ReliableMcastChannelConfig tempRelMcastCfg;

	while (elementListServer.forth())
	{
		const ElementEntry& serverEntry = elementListServer.getEntry();

		switch (serverEntry.getLoadType())
		{
		case DataType::AsciiEnum:
			if (serverEntry.getName() == "Port")
			{
				port = serverEntry.getAscii();
				flags |= PortFlagEnum;
			}
			else if (serverEntry.getName() == "InterfaceName")
			{
				interfaceName = serverEntry.getAscii();
				flags |= InterfaceNameFlagEnum;
			}
			else if (serverEntry.getName() == "ServerCert")
			{
				serverCert = serverEntry.getAscii();
				flags |= ServerCertEnum;
			}
			else if (serverEntry.getName() == "ServerPrivateKey")
			{
				serverPrivateKey = serverEntry.getAscii();
				flags |= ServerPrivateKeyEnum;
			}
			else if (serverEntry.getName() == "DHParams")
			{
				dhParams = serverEntry.getAscii();
				flags |= DHParamEnum;
			}
			else if (serverEntry.getName() == "CipherSuite")
			{
				cipherSuite = serverEntry.getAscii();
				flags |= CipherSuiteEnum;
			}
			else if (serverEntry.getName() == "LibSslName")
			{
				libSslName = serverEntry.getAscii();
				flags |= LibSslNameEnum;
			}
			else if (serverEntry.getName() == "LibCryptoName")
			{
				libCryptoName = serverEntry.getAscii();
				flags |= LibCryptoNameEnum;
			}
			else if (serverEntry.getName() == "LibCurlName")
			{
				libCryptoName = serverEntry.getAscii();
				flags |= LibCurlNameEnum;
			}
			else if (serverEntry.getName() == "WsProtocols")
			{
				wsProtocols = serverEntry.getAscii();
				websocketFlags |= 0x01;
			}
			break;

		case DataType::EnumEnum:
			if (serverEntry.getName() == "ServerType")
			{
				serverType = serverEntry.getEnum();

				switch (serverType)
				{
				case RSSL_CONN_TYPE_SOCKET:
					flags |= ServerTypeFlagEnum;
					break;

				case RSSL_CONN_TYPE_WEBSOCKET:
					flags |= ServerTypeFlagEnum;
					break;

				case RSSL_CONN_TYPE_ENCRYPTED:
					flags |= ServerTypeFlagEnum;
					break;
				default:
					EmaString text("Invalid ServerType [");
					text.append(serverType);
					text.append("] in Programmatic Configuration. Use either ServerType [");
					text.append("RSSL_SOCKET | RSSL_ENCRYPTED");
					text.append("]");
					EmaConfigError* mce(new EmaConfigError(text, OmmLoggerClient::ErrorEnum));
					emaConfigErrList.add(mce);
					break;
				}
			}
			else if (serverEntry.getName() == "CompressionType")
			{
				compressionType = serverEntry.getEnum();

				if (compressionType > RSSL_COMP_LZ4)
				{
					EmaString text("Invalid CompressionType [");
					text.append(compressionType);
					text.append("] in Programmatic Configuration. Use default CompressionType [");
					text.append(DEFAULT_COMPRESSION_TYPE);
					text.append("] ");
					EmaConfigError* mce(new EmaConfigError(text, OmmLoggerClient::ErrorEnum));
					emaConfigErrList.add(mce);
				}
				else
				{
					flags |= CompTypeFlagEnum;
				}
			}
			
			break;

		case DataType::UIntEnum:
			if (serverEntry.getName() == "GuaranteedOutputBuffers")
			{
				guaranteedOutputBuffers = serverEntry.getUInt();
				flags |= GuarantOutputBufFlagEnum;
			}
			if (serverEntry.getName() == "NumInputBuffers")
			{
				numInputBuffers = serverEntry.getUInt();
				flags |= NumInputBufFlagEnum;
			}
			if (serverEntry.getName() == "SysRecvBufSize")
			{
				sysRecvBufSize = serverEntry.getUInt();
				flags |= SysRecvBufSizeFlagEnum;
			}
			if (serverEntry.getName() == "SysSendBufSize")
			{
				sysSendBufSize = serverEntry.getUInt();
				flags |= SysSendBufSizeFlagEnum;
			}
			if (serverEntry.getName() == "HighWaterMark")
			{
				highWaterMark = serverEntry.getUInt();
				flags |= HighWaterMarkFlagEnum;
			}
			else if (serverEntry.getName() == "TcpNodelay")
			{
				tcpNodelay = serverEntry.getUInt();
				flags |= TcpNodelayFlagEnum;
			}
			if (serverEntry.getName() == "ConnectionMinPingTimeout")
			{
				connectionMinPingTimeout = serverEntry.getUInt();
				flags |= ConnMinPingTimeoutFlagEnum;
			}
			else if (serverEntry.getName() == "ConnectionPingTimeout")
			{
				connectionPingTimeout = serverEntry.getUInt();
				flags |= ConnPingTimeoutFlagEnum;
			}
			else if (serverEntry.getName() == "CompressionThreshold")
			{
				compressionThreshold = serverEntry.getUInt();
				flags |= CompressThresHoldFlagEnum;
			}
			else if (serverEntry.getName() == "InitializationTimeout")
			{
				initializationTimeout = serverEntry.getUInt();
				flags |= InitializationTimeoutFlagEnum;
			}
			else if (serverEntry.getName() == "MaxFragmentSize")
			{
				maxFragmentSize = serverEntry.getUInt();
				flags |= MaxFragmentSizeFlagEnum;
			}
			break;
		}
	}

	if (flags & ServerTypeFlagEnum)
	{
		try
		{
			activeServerConfig.pServerConfig = new SocketServerConfig(activeServerConfig.defaultServiceName());
			SocketServerConfig* pCurrentServerConfig = static_cast<SocketServerConfig*>(activeServerConfig.pServerConfig);
			SocketServerConfig* fileCfgSocket = static_cast<SocketServerConfig*>(fileCfg);

			pCurrentServerConfig->connectionType = (RsslConnectionTypes)serverType;

			if (flags & LibSslNameEnum)
				pCurrentServerConfig->libSslName = libSslName;
			else if (fileCfgSocket)
				pCurrentServerConfig->libSslName = fileCfgSocket->libSslName;

			if (flags & LibCryptoNameEnum)
				pCurrentServerConfig->libCryptoName = libCryptoName;
			else if (fileCfgSocket)
				pCurrentServerConfig->libCryptoName = fileCfgSocket->libCryptoName;

			if (flags & LibCurlNameEnum)
				pCurrentServerConfig->libCurlName = libCurlName;
			else if (fileCfgSocket)
				pCurrentServerConfig->libCurlName = fileCfgSocket->libCurlName;

			if (flags & TcpNodelayFlagEnum)
				pCurrentServerConfig->tcpNodelay = tcpNodelay ? true : false;
			else if (fileCfgSocket)
				pCurrentServerConfig->tcpNodelay = fileCfgSocket->tcpNodelay;

			if (flags & PortFlagEnum && setByFnCalled == 0)
				pCurrentServerConfig->serviceName = port;
			else if (fileCfgSocket)
				pCurrentServerConfig->serviceName = fileCfgSocket->serviceName;

			pCurrentServerConfig->name = serverName;

			if (flags & InterfaceNameFlagEnum)
				pCurrentServerConfig->interfaceName = interfaceName;
			else if (fileCfgSocket)
				pCurrentServerConfig->interfaceName = fileCfg->interfaceName;

			if (flags & ConnMinPingTimeoutFlagEnum)
				pCurrentServerConfig->connectionMinPingTimeout = compressionThreshold > MAX_UNSIGNED_INT32 ? MAX_UNSIGNED_INT32 : (UInt32)connectionMinPingTimeout;
			else if(fileCfgSocket)
				pCurrentServerConfig->connectionMinPingTimeout = fileCfg->connectionMinPingTimeout;

			if (flags & CompTypeFlagEnum)
				pCurrentServerConfig->compressionType = (RsslCompTypes)compressionType;
			else if (fileCfgSocket)
				pCurrentServerConfig->compressionType = fileCfg->compressionType;

			if (flags & CompressThresHoldFlagEnum)
				pCurrentServerConfig->compressionThreshold = compressionThreshold > MAX_UNSIGNED_INT32 ? MAX_UNSIGNED_INT32 : (UInt32)compressionThreshold;
			else if (fileCfgSocket)
				pCurrentServerConfig->compressionThreshold = fileCfg->compressionThreshold;

			if (flags & GuarantOutputBufFlagEnum)
				pCurrentServerConfig->setGuaranteedOutputBuffers(guaranteedOutputBuffers);
			else if (fileCfgSocket)
				pCurrentServerConfig->guaranteedOutputBuffers = fileCfg->guaranteedOutputBuffers;

			if (flags & NumInputBufFlagEnum)
				pCurrentServerConfig->setNumInputBuffers(numInputBuffers);
			else if (fileCfgSocket)
				pCurrentServerConfig->numInputBuffers = fileCfg->numInputBuffers;

			if (flags & SysRecvBufSizeFlagEnum)
				pCurrentServerConfig->sysRecvBufSize = sysRecvBufSize > MAX_UNSIGNED_INT32 ? MAX_UNSIGNED_INT32 : (UInt32)sysRecvBufSize;
			else if (fileCfgSocket)
				pCurrentServerConfig->sysRecvBufSize = fileCfg->sysRecvBufSize;

			if (flags & SysSendBufSizeFlagEnum)
				pCurrentServerConfig->sysSendBufSize = sysSendBufSize > MAX_UNSIGNED_INT32 ? MAX_UNSIGNED_INT32 : (UInt32)sysSendBufSize;
			else if (fileCfgSocket)
				pCurrentServerConfig->sysSendBufSize = fileCfg->sysSendBufSize;

			if (flags & HighWaterMarkFlagEnum)
				pCurrentServerConfig->highWaterMark = highWaterMark > MAX_UNSIGNED_INT32 ? MAX_UNSIGNED_INT32 : (UInt32)highWaterMark;
			else if (fileCfgSocket)
				pCurrentServerConfig->highWaterMark = fileCfg->highWaterMark;

			if (flags & ConnPingTimeoutFlagEnum)
				pCurrentServerConfig->connectionPingTimeout = connectionPingTimeout > MAX_UNSIGNED_INT32 ? MAX_UNSIGNED_INT32 : (UInt32)connectionPingTimeout;
			else if (fileCfgSocket)
				pCurrentServerConfig->connectionPingTimeout = fileCfg->connectionPingTimeout;

			if (flags & InitializationTimeoutFlagEnum)
				pCurrentServerConfig->initializationTimeout = initializationTimeout > MAX_UNSIGNED_INT32 ? MAX_UNSIGNED_INT32 : (UInt32)initializationTimeout;
			else if (fileCfgSocket)
				pCurrentServerConfig->initializationTimeout = fileCfg->initializationTimeout;

			if (flags & MaxFragmentSizeFlagEnum)
				pCurrentServerConfig->maxFragmentSize = maxFragmentSize;
			else if (fileCfgSocket)
				pCurrentServerConfig->maxFragmentSize = fileCfgSocket->maxFragmentSize;


			if (serverType == RSSL_CONN_TYPE_ENCRYPTED)
			{
				if (flags & ServerCertEnum)
					pCurrentServerConfig->serverCert = serverCert;
				else if (fileCfgSocket)
					pCurrentServerConfig->serverCert = fileCfgSocket->serverCert;

				if (flags & ServerPrivateKeyEnum)
					pCurrentServerConfig->serverPrivateKey = serverPrivateKey;
				else if (fileCfgSocket)
					pCurrentServerConfig->serverPrivateKey = fileCfgSocket->serverPrivateKey;

				if (flags & DHParamEnum)
					pCurrentServerConfig->dhParams = dhParams;
				else if (fileCfgSocket)
					pCurrentServerConfig->dhParams = fileCfgSocket->dhParams;

				if (flags & CipherSuiteEnum)
					pCurrentServerConfig->cipherSuite = cipherSuite;
				else if (fileCfgSocket)
					pCurrentServerConfig->cipherSuite = fileCfgSocket->cipherSuite;
			}

			if (serverType == RSSL_CONN_TYPE_WEBSOCKET)
			{
				if (websocketFlags & 0x01)
					pCurrentServerConfig->wsProtocols = wsProtocols;
				else if (fileCfgSocket)
					pCurrentServerConfig->wsProtocols = fileCfgSocket->wsProtocols;
			}
		}
		catch (std::bad_alloc&)
		{
			const char* temp = "Failed to allocate memory for ServerConfig. Out of memory!";
			throwMeeException(temp);
		}
	}
}

bool ProgrammaticConfigure::setReliableMcastChannelInfo( ReliableMcastChannelConfig* pChannelCfg, UInt64& mcastFlags, ReliableMcastChannelConfig& relMcastcfg, EmaString& errorText, ChannelConfig* fileCfg )
{
	bool bValid = true;

	ReliableMcastChannelConfig* pFileMcastCfg =  NULL;
	bool useFileCfg = ( fileCfg && fileCfg->connectionType == relMcastcfg.connectionType ) ? true : false;
	if ( useFileCfg )
		pFileMcastCfg =  static_cast<ReliableMcastChannelConfig*>( fileCfg );

	if ( mcastFlags & 0x01 )
		pChannelCfg->recvAddress = relMcastcfg.recvAddress;
	else if ( useFileCfg )
		pChannelCfg->recvAddress = pFileMcastCfg->recvAddress;

	if ( mcastFlags & 0x02 )
		pChannelCfg->recvServiceName = relMcastcfg.recvServiceName;
	else if ( useFileCfg )
		pChannelCfg->recvServiceName = pFileMcastCfg->recvServiceName;

	if ( mcastFlags & 0x04 )
		pChannelCfg->sendAddress = relMcastcfg.sendAddress;
	else if ( useFileCfg )
		pChannelCfg->sendAddress = pFileMcastCfg->sendAddress;

	if ( mcastFlags & 0x08 )
		pChannelCfg->sendServiceName = relMcastcfg.sendServiceName;
	else if ( useFileCfg )
		pChannelCfg->sendServiceName = pFileMcastCfg->sendServiceName;

	if ( mcastFlags & 0x10 )
		pChannelCfg->unicastServiceName = relMcastcfg.unicastServiceName;
	else if ( useFileCfg )
		pChannelCfg->unicastServiceName = pFileMcastCfg->unicastServiceName;

	if ( mcastFlags & 0x20 )
		pChannelCfg->hsmInterface = relMcastcfg.hsmInterface;
	else if ( useFileCfg )
		pChannelCfg->hsmInterface = pFileMcastCfg->hsmInterface;

	if ( mcastFlags & 0x40 )
		pChannelCfg->hsmMultAddress = relMcastcfg.hsmMultAddress;
	else if ( useFileCfg )
		pChannelCfg->hsmMultAddress = pFileMcastCfg->hsmMultAddress;

	if ( mcastFlags & 0x80 )
		pChannelCfg->hsmPort = relMcastcfg.hsmPort;
	else if ( useFileCfg )
		pChannelCfg->hsmPort = pFileMcastCfg->hsmPort;

	if ( mcastFlags & 0x100 )
		pChannelCfg->packetTTL = relMcastcfg.packetTTL;
	else if ( useFileCfg )
		pChannelCfg->packetTTL = pFileMcastCfg->packetTTL;

	if ( mcastFlags & 0x200 )
		pChannelCfg->disconnectOnGap = relMcastcfg.disconnectOnGap;
	else if ( useFileCfg )
		pChannelCfg->disconnectOnGap = pFileMcastCfg->disconnectOnGap;

	if ( mcastFlags & 0x400 )
		pChannelCfg->hsmInterval = relMcastcfg.hsmInterval;
	else if ( useFileCfg )
		pChannelCfg->hsmInterval = pFileMcastCfg->hsmInterval;

	if ( mcastFlags & 0x800 )
		pChannelCfg->ndata = relMcastcfg.ndata;
	else if ( useFileCfg )
		pChannelCfg->ndata = pFileMcastCfg->ndata;

	if ( mcastFlags & 0x1000 )
		pChannelCfg->nmissing = relMcastcfg.nmissing;
	else if ( useFileCfg )
		pChannelCfg->nmissing = pFileMcastCfg->nmissing;

	if ( mcastFlags & 0x2000 )
		pChannelCfg->nrreq = relMcastcfg.nrreq;
	else if ( useFileCfg )
		pChannelCfg->nrreq = pFileMcastCfg->nrreq;

	if ( mcastFlags & 0x4000 )
		pChannelCfg->tdata = relMcastcfg.tdata;
	else if ( useFileCfg )
		pChannelCfg->tdata = pFileMcastCfg->tdata;

	if ( mcastFlags & 0x8000 )
		pChannelCfg->trreq = relMcastcfg.trreq;
	else if ( useFileCfg )
		pChannelCfg->trreq = pFileMcastCfg->trreq;

	if ( mcastFlags & 0x10000 )
		pChannelCfg->pktPoolLimitHigh = relMcastcfg.pktPoolLimitHigh;
	else if ( useFileCfg )
		pChannelCfg->pktPoolLimitHigh = pFileMcastCfg->pktPoolLimitHigh;

	if ( mcastFlags & 0x20000 )
		pChannelCfg->pktPoolLimitLow = relMcastcfg.pktPoolLimitLow;
	else if ( useFileCfg )
		pChannelCfg->pktPoolLimitLow = pFileMcastCfg->pktPoolLimitLow;

	if ( mcastFlags & 0x40000 )
		pChannelCfg->twait = relMcastcfg.twait;
	else if ( useFileCfg )
		pChannelCfg->twait = pFileMcastCfg->twait;

	if ( mcastFlags & 0x80000 )
		pChannelCfg->tbchold = relMcastcfg.tbchold;
	else if ( useFileCfg )
		pChannelCfg->tbchold = pFileMcastCfg->tbchold;

	if ( mcastFlags & 0x100000 )
		pChannelCfg->tpphold = relMcastcfg.tpphold;
	else if ( useFileCfg )
		pChannelCfg->tpphold = pFileMcastCfg->tpphold;

	if ( mcastFlags & 0x200000 )
		pChannelCfg->userQLimit = relMcastcfg.userQLimit;
	else if ( useFileCfg )
		pChannelCfg->userQLimit = pFileMcastCfg->userQLimit;

	if ( mcastFlags & 0x400000 )
		pChannelCfg->tcpControlPort = relMcastcfg.tcpControlPort;
	else if ( useFileCfg )
		pChannelCfg->tcpControlPort = pFileMcastCfg->tcpControlPort;

	return bValid;
}

void ProgrammaticConfigure::retrieveLogger( const Map& map, const EmaString& loggerName, EmaConfigErrorList& emaConfigErrList,
	BaseConfig& activeConfig )
{
	map.reset();
	while ( map.forth() )
	{
		const MapEntry& mapEntry = map.getEntry();

		if ( mapEntry.getKey().getDataType() == DataType::AsciiEnum && mapEntry.getKey().getAscii() == "LoggerGroup" )
		{
			if ( mapEntry.getLoadType() == DataType::ElementListEnum )
			{
				const ElementList& elementList = mapEntry.getElementList();

				while ( elementList.forth() )
				{
					const ElementEntry& elementEntry = elementList.getEntry();

					if ( elementEntry.getLoadType() == DataType::MapEnum )
					{
						if ( elementEntry.getName() == "LoggerList" )
						{
							const Map& map = elementEntry.getMap();

							while ( map.forth() )
							{
								const MapEntry& mapEntry = map.getEntry();

								if ( ( mapEntry.getKey().getDataType() == DataType::AsciiEnum ) && ( mapEntry.getKey().getAscii() == loggerName ) )
								{
									if ( mapEntry.getLoadType() == DataType::ElementListEnum )
									{
										const ElementList& elementListLogger = mapEntry.getElementList();

										while ( elementListLogger.forth() )
										{
											const ElementEntry& entry = elementListLogger.getEntry();

											switch ( entry.getLoadType() )
											{
											case DataType::AsciiEnum:
												if ( entry.getName() == "FileName" )
												{
													activeConfig.loggerConfig.loggerFileName = entry.getAscii();
												}
												break;

											case DataType::EnumEnum:
												if ( entry.getName() == "LoggerType" )
												{
													UInt16 loggerType = entry.getEnum();

													if ( loggerType > 1 )
													{
														EmaString text( "Invalid LoggerType [" );
														text.append( loggerType );
														text.append( "] in Programmatic Configuration. Use default LoggerType [" );
														text.append( DEFAULT_LOGGER_TYPE );
														text.append( "]" );
														EmaConfigError* mce( new EmaConfigError( text, OmmLoggerClient::ErrorEnum ) );
														emaConfigErrList.add( mce );
													}
													else
													{
														activeConfig.loggerConfig.loggerType = ( OmmLoggerClient::LoggerType )loggerType;
													}
												}
												else if ( entry.getName() == "LoggerSeverity" )
												{
													UInt16 severityType = entry.getEnum();

													if ( severityType > 4 )
													{
														EmaString text( "Invalid LoggerSeverity [" );
														text.append( severityType );
														text.append( "] in Programmatic Configuration. Use default LoggerSeverity [" );
														text.append( DEFAULT_LOGGER_SEVERITY );
														text.append( "]" );
														EmaConfigError* mce( new EmaConfigError( text, OmmLoggerClient::ErrorEnum ) );
														emaConfigErrList.add( mce );
													}
													else
													{
														activeConfig.loggerConfig.minLoggerSeverity = ( OmmLoggerClient::Severity )severityType;
													}
												}
												break;
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
}

void ProgrammaticConfigure::retrieveDictionary( const Map& map, const EmaString& dictionaryName, EmaConfigErrorList& emaConfigErrList,
    DictionaryConfig& dictionaryConfig )
{
	map.reset();
	while ( map.forth() )
	{
		const MapEntry& mapEntry = map.getEntry();

		if ( mapEntry.getKey().getDataType() == DataType::AsciiEnum && mapEntry.getKey().getAscii() == "DictionaryGroup" )
		{
			if ( mapEntry.getLoadType() == DataType::ElementListEnum )
			{
				const ElementList& elementList = mapEntry.getElementList();

				while ( elementList.forth() )
				{
					const ElementEntry& elementEntry = elementList.getEntry();

					if ( elementEntry.getLoadType() == DataType::MapEnum )
					{
						if ( elementEntry.getName() == "DictionaryList" )
						{
							const Map& map = elementEntry.getMap();

							while ( map.forth() )
							{
								const MapEntry& mapEntry = map.getEntry();

								if ( ( mapEntry.getKey().getDataType() == DataType::AsciiEnum ) && ( mapEntry.getKey().getAscii() == dictionaryName ) )
								{
									if ( mapEntry.getLoadType() == DataType::ElementListEnum )
									{
										const ElementList& elementListDictionary = mapEntry.getElementList();

										while ( elementListDictionary.forth() )
										{
											const ElementEntry& entry = elementListDictionary.getEntry();

											switch ( entry.getLoadType() )
											{
											case DataType::AsciiEnum:

												if ( entry.getName() == "RdmFieldDictionaryFileName" )
												{
													dictionaryConfig.rdmfieldDictionaryFileName = entry.getAscii();
												}
												else if ( entry.getName() == "EnumTypeDefFileName" )
												{
													dictionaryConfig.enumtypeDefFileName = entry.getAscii();
												}
												if (entry.getName() == "RdmFieldDictionaryItemName")
												{
													dictionaryConfig.rdmFieldDictionaryItemName = entry.getAscii();
												}
												else if (entry.getName() == "EnumTypeDefItemName")
												{
													dictionaryConfig.enumTypeDefItemName = entry.getAscii();
												}
												break;

											case DataType::EnumEnum:

												if ( entry.getName() == "DictionaryType" )
												{
													UInt16 dictionaryType = entry.getEnum();

													if ( dictionaryType > 1 )
													{
														EmaString text( "Invalid DictionaryType [" );
														text.append( dictionaryType );
														text.append("] in Programmatic Configuration. Use default DictionaryType.");
														EmaConfigError* mce( new EmaConfigError( text, OmmLoggerClient::ErrorEnum ) );
														emaConfigErrList.add( mce );
													}
													else
													{
														dictionaryConfig.dictionaryType = ( Dictionary::DictionaryType )dictionaryType;
													}
												}
												break;
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
}

void ProgrammaticConfigure::retrieveDirectory( const Map& map, const EmaString& directoryName, DirectoryServiceStore& dirServiceStrore, DirectoryCache& directoryCache, EmaList<ServiceDictionaryConfig*>* serviceDictionaryConfigList)
{
	map.reset();
	while (map.forth())
	{
		const MapEntry& mapEntry = map.getEntry();
		if (mapEntry.getKey().getDataType() == DataType::AsciiEnum && 
			mapEntry.getKey().getAscii() == "DirectoryGroup" &&
			mapEntry.getLoadType() == DataType::ElementListEnum)
		{
			const ElementList& dirGroupElementList = mapEntry.getElementList();
			while (dirGroupElementList.forth() )
			{
				const ElementEntry& defaultDirAndDirListElementEntry = dirGroupElementList.getEntry();
				if (defaultDirAndDirListElementEntry.getLoadType() == DataType::MapEnum && defaultDirAndDirListElementEntry.getName() == "DirectoryList")
				{
					const Map& dirListMap = defaultDirAndDirListElementEntry.getMap();
					while (dirListMap.forth())  
					{
						const MapEntry& dirListMapEntry = dirListMap.getEntry();

						if (dirListMapEntry.getKey().getDataType() == DataType::AsciiEnum &&
							dirListMapEntry.getKey().getAscii() == directoryName &&
							dirListMapEntry.getLoadType() == DataType::MapEnum)  
						{
							_serviceNameList.clear();
							Service newService;
							int origServiceId = 0;

							const Map& serviceListMap = dirListMapEntry.getMap();  
							while (serviceListMap.forth())
							{
								const MapEntry& eachServiceEntry = serviceListMap.getEntry(); 
								if (eachServiceEntry.getKey().getDataType() == DataType::AsciiEnum &&
									eachServiceEntry.getLoadType() == DataType::ElementListEnum)
								{
									_dictProvided.clear();
									_dictUsed.clear();
									EmaString serviceName = eachServiceEntry.getKey().getAscii();
									Service* service = directoryCache.getService(serviceName);
									newService.clear();
									bool addNewService = false;
									origServiceId = 0;
									if (!service)
									{
										addNewService = true;
										newService.infoFilter.serviceName = serviceName;
										service = &newService;

										//allocate default value for qos first.
										RsslQos rsslQos;
										rsslClearQos(&rsslQos);
										rsslQos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
										rsslQos.timeliness = RSSL_QOS_TIME_REALTIME;
										service->infoFilter.qos.push_back(rsslQos);
										service->infoFilter.flags |= RDM_SVC_IFF_HAS_QOS;
									}
									else
										origServiceId = service->serviceId;

									const ElementList& serviceFilterList = eachServiceEntry.getElementList();
									while (serviceFilterList.forth())  // two directory filters
									{
										const ElementEntry& eachFilterEntry = serviceFilterList.getEntry();

										if (eachFilterEntry.getLoadType() == DataType::ElementListEnum &&
											(eachFilterEntry.getName() == "InfoFilter" || eachFilterEntry.getName() == "StateFilter"))
										{
											const ElementList& eachServiceInfo = eachFilterEntry.getElementList();
											if (!retrieveServiceInfo(*service, eachServiceInfo, directoryCache))
											{
												service = 0;
												break;
											}
											else
											{
												if (eachFilterEntry.getName() == "InfoFilter")
												{
													service->infoFilter.action = RSSL_FTEA_SET_ENTRY;
													service->flags |= RDM_SVCF_HAS_INFO;
												}
												else if (eachFilterEntry.getName() == "StateFilter")
												{
													service->stateFilter.action = RSSL_FTEA_SET_ENTRY;
													service->flags |= RDM_SVCF_HAS_STATE;
												}
											}
										}
									}
									
									if (service)
									{
										if (addNewService)
										{
											service = directoryCache.addService(newService);
											dirServiceStrore.addServiceIdAndNamePair(service->serviceId, new EmaString(service->infoFilter.serviceName), 0);
										}
										else if (origServiceId != service->serviceId)
										{
											dirServiceStrore.removeServiceNamePair(origServiceId);
											dirServiceStrore.addServiceIdAndNamePair(service->serviceId, new EmaString(service->infoFilter.serviceName), 0);
											service = directoryCache.addService(*service);
											directoryCache.removeService(origServiceId);
										}

										retrieveServerDictionaryConfig(*service, serviceDictionaryConfigList);
										_serviceNameList.push_back(service->infoFilter.serviceName);
									}
								}
							}
						
							//remove old ones from config file which is not configured by programmatic
							removeConfigFileService(dirServiceStrore, directoryCache);
							break;
						}
					}
				}
			}
		}
	}
}

bool ProgrammaticConfigure::retrieveServiceInfo(Service& service, const ElementList& serviceInfo, DirectoryCache& dirCache)
{
	UInt64 uintValue;
	UInt64 rate = 0, timeliness = 0;
	EmaString infoString;
	static bool addQos = false;
				
	while (serviceInfo.forth())
	{
		const ElementEntry& entry = serviceInfo.getEntry();

		switch (entry.getLoadType())
		{
		case DataType::AsciiEnum:
			if (entry.getName() == "StreamState")
			{
				infoString.clear();
				if (!findString(entry.getAscii(), infoString))
					continue;

				bool found = false;
				service.stateFilter.status.streamState = OmmState::OpenEnum;
				service.stateFilter.flags |= RDM_SVC_STF_HAS_STATUS;
				for (int i = 0; i < sizeof streamStateConverter / sizeof streamStateConverter[0]; ++i)
				{
					if (!strcmp(streamStateConverter[i].configInput, infoString))
					{
						found = true;
						service.stateFilter.status.streamState = streamStateConverter[i].convertedValue;
						break;
					}
				}
				if (!found)
				{
					EmaString text("failed to convert a StreamState from the programmatically configured service [");
					text.append(service.infoFilter.serviceName).append("]. Will use default StreamState. Suspect value is ")
						.append(entry.getAscii());
					EmaConfigError* mce(new EmaConfigError(text, OmmLoggerClient::ErrorEnum));
					_emaConfigErrList.add(mce);
				}
			}
			else if (entry.getName() == "DataState")
			{
				infoString.clear();
				if (!findString(entry.getAscii(), infoString))
					continue;

				bool found = false;
				service.stateFilter.status.dataState = OmmState::OkEnum;
				service.stateFilter.flags |= RDM_SVC_STF_HAS_STATUS;
				for (int i = 0; i < sizeof dataStateConverter / sizeof dataStateConverter[0]; ++i)
				{
					if (!strcmp(dataStateConverter[i].configInput, infoString))
					{
						found = true;
						service.stateFilter.status.dataState = dataStateConverter[i].convertedValue;
						break;
					}
				}
				if (!found)
				{
					EmaString text("failed to convert a DataState from the programmatically configured service [");
					text.append(service.infoFilter.serviceName).append("]. Will use default DataState. Suspect value is ")
						.append(entry.getAscii());
					EmaConfigError* mce(new EmaConfigError(text, OmmLoggerClient::ErrorEnum));
					_emaConfigErrList.add(mce);
				}
			}
			else if (entry.getName() == "StatusCode")
			{
				infoString.clear();
				if (!findString(entry.getAscii(), infoString))
					continue;

				bool found = false;
				service.stateFilter.status.code = OmmState::NoneEnum;
				service.stateFilter.flags |= RDM_SVC_STF_HAS_STATUS;
				for (int i = 0; i < sizeof statusCodeConverter / sizeof statusCodeConverter[0]; ++i)
				{
					if (!strcmp(statusCodeConverter[i].configInput, infoString))
					{
						found = true;
						service.stateFilter.status.code = statusCodeConverter[i].convertedValue;
						break;
					}
				}
				if (!found)
				{
					EmaString text("failed to convert a StatusCode from the programmatically configured service [");
					text.append(service.infoFilter.serviceName).append("]. Will use default StatusCode. Suspect value is ")
						.append(entry.getAscii());
					EmaConfigError* mce(new EmaConfigError(text, OmmLoggerClient::ErrorEnum));
					_emaConfigErrList.add(mce);
				}
			}
			else if (entry.getName() == "StatusText")
			{
				if (!entry.getAscii().empty())
				{
					service.stateFilter.statusText.clear();
					service.stateFilter.statusText.append(entry.getAscii().c_str());
					service.stateFilter.status.text.data = (char*)service.stateFilter.statusText.c_str();
					service.stateFilter.status.text.length = service.stateFilter.statusText.length();
					service.stateFilter.flags |= RDM_SVC_STF_HAS_STATUS;
				}
				else
					rsslClearBuffer(&service.stateFilter.status.text);
			}
			else if (entry.getName() == "Vendor")
			{
				service.infoFilter.vendorName = entry.getAscii();
				service.infoFilter.flags |= RDM_SVC_IFF_HAS_VENDOR;
			}
			else if (entry.getName() == "ItemList")
			{
				service.infoFilter.itemList = entry.getAscii();
				service.infoFilter.flags |= RDM_SVC_IFF_HAS_ITEM_LIST;
			}
			else if (entry.getName() == "Timeliness")
			{
				bool found = false;
				for (int i = 0; i < sizeof qosTimelinessConverter / sizeof qosTimelinessConverter[0]; ++i)
				{
					if (!strcmp(qosTimelinessConverter[i].configInput, entry.getAscii()))
					{
						found = true;
						timeliness = qosTimelinessConverter[i].convertedValue;
						service.infoFilter.flags |= RDM_SVC_IFF_HAS_QOS;
						addQos = true;
						break;
					}
				}

				if (!found)
				{
					EmaString text("failed to convert a QoS Timeliness from the programmatically configured service [");
					text.append(service.infoFilter.serviceName).append("]. Will use default Timeliness. Suspect value is = ")
						.append(entry.getAscii());
					EmaConfigError* mce(new EmaConfigError(text, OmmLoggerClient::ErrorEnum));
					_emaConfigErrList.add(mce);
				}
			}
			else if (entry.getName() == "Rate")
			{
				bool found = false;
				for (int i = 0; i < sizeof qosRateConverter / sizeof qosRateConverter[0]; ++i)
				{
					if (!strcmp(qosRateConverter[i].configInput, entry.getAscii()))
					{
						found = true;
						rate = qosRateConverter[i].convertedValue;
						service.infoFilter.flags |= RDM_SVC_IFF_HAS_QOS;
						addQos = true;
						break;
					}
				}

				if (!found)
				{
					EmaString text("failed to convert a QoS Rate from the programmatically configured service [");
					text.append(service.infoFilter.serviceName).append("]. Will use default Rate. Suspect value is = ")
						.append(entry.getAscii());
					EmaConfigError* mce(new EmaConfigError(text, OmmLoggerClient::ErrorEnum));
					_emaConfigErrList.add(mce);
				}
			}
			break;
		case DataType::UIntEnum:
			if (entry.getName() == "ServiceState")
			{
				uintValue = entry.getUInt();
				service.stateFilter.serviceState = uintValue > 0 ? 1 : 0;
			}
			else if (entry.getName() == "AcceptingRequests")
			{
				uintValue = entry.getUInt();
				service.stateFilter.acceptingRequests = uintValue > 0 ? 1 : 0;
				service.stateFilter.flags |= RDM_SVC_STF_HAS_ACCEPTING_REQS;
			}
			else if (entry.getName() == "ServiceId")
			{
				uintValue = entry.getUInt();
				if (uintValue > MAX_UNSIGNED_INT16)
				{
					EmaString text("service [");
					text.append(service.infoFilter.serviceName)
						.append("] from the programmatically configure specifies out of range ServiceId [");
					text.append(uintValue).append("]. Will drop this service.");
					EmaConfigError* mce(new EmaConfigError(text, OmmLoggerClient::ErrorEnum));
					_emaConfigErrList.add(mce);
					return false;
				}

				Service* existingService = dirCache.getService(uintValue);
				if (existingService && existingService != &service)
				{
					EmaString text("service [");
					text.append(service.infoFilter.serviceName)
						.append("] from the programmatically configure specifies the same ServiceId [");
					text.append(uintValue).append("] as already specified by another service. Will drop this service.");
					EmaConfigError* mce(new EmaConfigError(text, OmmLoggerClient::ErrorEnum));
					_emaConfigErrList.add(mce);

					return false;
				}

				service.serviceId = (UInt16)uintValue;
			}
			else if (entry.getName() == "IsSource")
			{
				uintValue = entry.getUInt();
				service.infoFilter.isSource = uintValue > 0 ? 1 : 0;
				service.infoFilter.flags |= RDM_SVC_IFF_HAS_IS_SOURCE;
			}
			else if (entry.getName() == "SupportsQoSRange")
			{
				uintValue = entry.getUInt();
				service.infoFilter.supportsQosRange = uintValue > 0 ? 1 : 0;
				service.infoFilter.flags |= RDM_SVC_IFF_HAS_SUPPORT_QOS_RANGE;
			}
			else if (entry.getName() == "SupportsOutOfBandSnapshots")
			{
				uintValue = entry.getUInt();
				service.infoFilter.supportsOutOfBandSnapshots = uintValue > 0 ? 1 : 0;
				service.infoFilter.flags |= RDM_SVC_IFF_HAS_SUPPORT_OOB_SNAPSHOTS;
			}
			else if (entry.getName() == "AcceptingConsumerStatus")
			{
				uintValue = entry.getUInt();
				service.infoFilter.acceptingConsumerStatus = uintValue > 0 ? 1 : 0;
				service.infoFilter.flags |= RDM_SVC_IFF_HAS_ACCEPTING_CONS_STATUS;
			}
			else if (entry.getName() == "Timeliness")
			{
				timeliness = entry.getUInt();
				if (timeliness > MAX_UNSIGNED_INT32)
				{
					EmaString text("service [");
					text.append(service.infoFilter.serviceName)
						.append("] from the programmatically configure specifies service QoS::Timeliness is greater than allowed maximum. Will use maximum Timeliness.")
						.append(" Suspect value is ").append(entry.getUInt());
					EmaConfigError* mce(new EmaConfigError(text, OmmLoggerClient::ErrorEnum));
					_emaConfigErrList.add(mce);

					timeliness = MAX_UNSIGNED_INT32;
				}

				service.infoFilter.flags |= RDM_SVC_IFF_HAS_QOS;
				addQos = true;
			}
			else if (entry.getName() == "Rate")
			{
				rate = entry.getUInt();
				if (rate > MAX_UNSIGNED_INT32)
				{
					EmaString text("service [");
					text.append(service.infoFilter.serviceName)
						.append("] from the programmatically configure specifies service QoS::Rate is greater than allowed maximum. Will use maximum Rate. Suspect value is ")
						.append(entry.getUInt());
					EmaConfigError* mce(new EmaConfigError(text, OmmLoggerClient::ErrorEnum));
					_emaConfigErrList.add(mce);

					rate = MAX_UNSIGNED_INT32;
				}

				service.infoFilter.flags |= RDM_SVC_IFF_HAS_QOS;
				addQos = true;
			}

			break;
		case DataType::ArrayEnum:
			if (entry.getName() == "DictionariesProvided")
			{
				service.infoFilter.dictionariesProvided.clear();
				const OmmArray& dArray = entry.getArray();
				while (dArray.forth())
				{
					const OmmArrayEntry& dEntry = dArray.getEntry();
					if (dEntry.getLoadType() == DataType::AsciiEnum)
						_dictProvided.push_back(dEntry.getAscii());
				}
				service.infoFilter.flags |= RDM_SVC_IFF_HAS_DICTS_PROVIDED;
			}
			else if (entry.getName() == "DictionariesUsed")
			{
				service.infoFilter.dictionariesUsed.clear();
				const OmmArray& dArray = entry.getArray();
				while (dArray.forth())
				{
					const OmmArrayEntry& dEntry = dArray.getEntry();
					if (dEntry.getLoadType() == DataType::AsciiEnum)
						_dictUsed.push_back(dEntry.getAscii());
				}
				service.infoFilter.flags |= RDM_SVC_IFF_HAS_DICTS_USED;
			}
			else if (entry.getName() == "Capabilities")
			{
				service.infoFilter.capabilities.clear();

				const OmmArray& cArray = entry.getArray();
				while (cArray.forth())
				{
					const OmmArrayEntry& arrayEntry = cArray.getEntry();
					if (arrayEntry.getLoadType() == DataType::AsciiEnum)
					{
						EmaString domainTypeString = arrayEntry.getAscii();
						if (isdigit(domainTypeString.c_str()[0]))
						{
							UInt64 domainType;
							if (sscanf(domainTypeString.c_str(), "%llu", &domainType) == 1)
							{
								if (domainType > MAX_UNSIGNED_INT16)
								{
									EmaString text("service [");
									text.append(service.infoFilter.serviceName)
										.append("] from the programmatically configure specifies the service which contains out of range capability. Will drop this capability. Suspect value is = ")
										.append(domainType);
									EmaConfigError* mce(new EmaConfigError(text, OmmLoggerClient::ErrorEnum));
									_emaConfigErrList.add(mce);
									continue;
								}
								else
								{
									if (service.infoFilter.capabilities.getPositionOf((UInt16)domainType) == -1)
										service.infoFilter.capabilities.push_back((UInt16)domainType);
								}
							}
							else
							{
								EmaString text("failed to convert a capability from the programmatically configured service [");
								text.append(service.infoFilter.serviceName).append("]. Will drop this capability. Suspect value is = ")
									.append(domainType);
								EmaConfigError* mce(new EmaConfigError(text, OmmLoggerClient::ErrorEnum));
								_emaConfigErrList.add(mce);
								continue;
							}
						}
						else
						{
							bool found = false;
							for (int i = 0; i < sizeof msgTypeConverter / sizeof msgTypeConverter[0]; ++i)
							{
								if (!strcmp(msgTypeConverter[i].configInput, arrayEntry.getAscii()))
								{
									found = true;
									if (service.infoFilter.capabilities.getPositionOf(msgTypeConverter[i].convertedValue) == -1)
										service.infoFilter.capabilities.push_back(msgTypeConverter[i].convertedValue);
									break;
								}
							}

							if (!found)
							{
								EmaString text("failed to convert a capability from the programmatically configured service [");
								text.append(service.infoFilter.serviceName).append("]. Will drop this capability. Suspect value is = ")
									.append(entry.getAscii());
								EmaConfigError* mce(new EmaConfigError(text, OmmLoggerClient::ErrorEnum));
								_emaConfigErrList.add(mce);
								continue;
							}
						}
					}
					else if (arrayEntry.getLoadType() == DataType::UIntEnum)
					{
						UInt64 domainType = arrayEntry.getUInt();
						if (domainType > MAX_UNSIGNED_INT16)
						{
							EmaString text("service [");
							text.append(service.infoFilter.serviceName)
								.append("] from the programmatically configure specifies the service which contains out of range capability. Will drop this capability. Suspect value is = ");
							text.append(domainType);
							EmaConfigError* mce(new EmaConfigError(text, OmmLoggerClient::ErrorEnum));
							_emaConfigErrList.add(mce);
							continue;
						}
						else
						{
							if (service.infoFilter.capabilities.getPositionOf((UInt16)domainType) == -1)
								service.infoFilter.capabilities.push_back((UInt16)domainType);
						}
					}
				}

				if (service.infoFilter.capabilities.empty())
				{
					EmaString text("service [");
					text.append(service.infoFilter.serviceName)
						.append("] from the programmatically configure specifies the service which contains no capabilities. Will drop this service.");
					EmaConfigError* mce(new EmaConfigError(text, OmmLoggerClient::ErrorEnum));
					_emaConfigErrList.add(mce);
					return false;
				}

				int capSize = service.infoFilter.capabilities.size();
				for (int i = 0; i <= capSize; i++)
				{
					for (int j = 0; j < (capSize - 1); j++)
					{
						if (service.infoFilter.capabilities[j] > service.infoFilter.capabilities[j + 1])
						{
							UInt64 temp = service.infoFilter.capabilities[j];
							service.infoFilter.capabilities[j] = service.infoFilter.capabilities[j + 1];
							service.infoFilter.capabilities[j + 1] = temp;
						}
					}
				}
			}

			break;
		case DataType::ElementListEnum: //"StateFilter|Status| or "InfoFilter|DictionariesProvided| or InfoFilter|DictionariesUsed or InfoFilter|Qos
			if (entry.getName() == "Status")
			{
				if (entry.getLoadType() == DataType::ElementListEnum)
				{
					const ElementList& statusEntryInfo = entry.getElementList();
					retrieveServiceInfo(service, statusEntryInfo, dirCache);
				}
				else
				{
					EmaString text("service [");
					text.append(service.infoFilter.serviceName)
						.append("] from the programmatically configure specifies the service status which contains invalid data type. Suspect value is = ");
					text.append(entry.getLoadType());
					EmaConfigError* mce(new EmaConfigError(text, OmmLoggerClient::ErrorEnum));
					_emaConfigErrList.add(mce);
				}
			}

			break;
		case DataType::SeriesEnum: 
			if (entry.getName() == "QoS")
			{
				service.infoFilter.qos.clear();
				const Series& qosSeries = entry.getSeries();
				while (qosSeries.forth())
				{
					const SeriesEntry& qosEntry = qosSeries.getEntry();
					if (qosEntry.getLoadType() == DataType::ElementListEnum)
					{
						retrieveServiceInfo(service, qosEntry.getElementList(), dirCache);
					}
				}
			}
			break;
		};
	}

	RsslQos rsslQos;
	if (service.infoFilter.flags & RDM_SVC_IFF_HAS_QOS && addQos)
	{
		rsslClearQos(&rsslQos);
		OmmQosDecoder::convertToRssl(&rsslQos, (UInt32)timeliness, (UInt32)rate);
		service.infoFilter.qos.push_back(rsslQos);
		addQos = false;
	}

	return true;
}

void ProgrammaticConfigure::retrieveGroupAndListName( const Map& map, EmaString& groupName, EmaString& listName )
{
	if ( _setGroup )
	{
		groupName = _group;
		listName = _list;
		return;
	}

	map.reset();
	while ( map.forth() )
	{
		const MapEntry& mapEntry = map.getEntry();
		if ( mapEntry.getKey().getDataType() == DataType::AsciiEnum )
			if ( mapEntry.getKey().getAscii() == "ConsumerGroup" )
			{
				_group = "ConsumerGroup";
				_list = "ConsumerList";
				break;
			}
			else if ( mapEntry.getKey().getAscii() == "NiProviderGroup" )
			{
				_group = "NiProviderGroup";
				_list = "NiProviderList";
				break;
			}
			else if (mapEntry.getKey().getAscii() == "IProviderGroup")
			{
				_group = "IProviderGroup";
				_list = "IProviderList";
				break;
			}
	}

	_setGroup = true;
	groupName = _group;
	listName = _list;
}

bool ProgrammaticConfigure::findString(const EmaString& inputValue, EmaString& outputString)
{
	int colonPosition(inputValue.find("::"));
	if (colonPosition == -1)
	{
		EmaString text("failed to convert from the programmatically specified configuration attribute. Its value is = ");
		text.append(inputValue).append(" expected typename::value (e.g., StreamState::Open)");
		EmaConfigError* mce(new EmaConfigError(text, OmmLoggerClient::ErrorEnum));
		_emaConfigErrList.add(mce);

		return false;
	}
	else
	{

		outputString = inputValue.substr(colonPosition + 2, inputValue.length() - colonPosition - 2);
		return true;
	}
}

ServiceDictionaryConfig* ProgrammaticConfigure::findServiceDictConfig(EmaList<ServiceDictionaryConfig*>* serviceDictionaryConfigList, int serviceId)
{
	if (serviceDictionaryConfigList && !serviceDictionaryConfigList->empty())
	{
		ServiceDictionaryConfig* serviceDictionaryConfig = serviceDictionaryConfigList->front();
		while (serviceDictionaryConfig)
		{
			if (serviceDictionaryConfig->serviceId == serviceId)
				return serviceDictionaryConfig;

			serviceDictionaryConfig = serviceDictionaryConfig->next();
		}
	}

	return NULL;
}

void ProgrammaticConfigure::removeConfigFileService(DirectoryServiceStore& dirServiceStore, DirectoryCache& directoryCache)
{
	if (_serviceNameList.size() == 0)
		return;

	EmaList< Service* >& serviceList = const_cast<EmaList< Service* >&>(directoryCache.getServiceList());
	Service* service = serviceList.front();
	while (service)
	{
		bool found = false;
		for (UInt32 j = 0; j < _serviceNameList.size(); j++)
		{
			if (_serviceNameList[j] == service->infoFilter.serviceName)
			{
				found = true;
				break;
			}
		}
		
		if (!found)
		{
			Service* temp = service;
			service = service->next();
			int serviceId = temp->serviceId;
			directoryCache.removeService(serviceId);
			dirServiceStore.removeServiceNamePair(serviceId);
		}
		else
			service = service->next();
	}
}
