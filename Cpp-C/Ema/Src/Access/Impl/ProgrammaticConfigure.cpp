/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "ActiveConfig.h"
#include "EmaConfigImpl.h"
#include "Common.h"
#include "StaticDecoder.h"
#include "DirectoryServiceStore.h"
#include "ExceptionTranslator.h"

using namespace thomsonreuters::ema::access;

ProgrammaticConfigure::ProgrammaticConfigure( const Map& map, EmaConfigErrorList& emaConfigErrList ) :
	_consumerName(),
	_niProviderName(),
	_iProviderName(),
	_channelName(),
	_loggerName(),
	_dictionaryName(),
	_directoryName(),
	_channelSet(),
	_overrideConsName( false ),
	_overrideNiProvName( false ),
	_overrideIProvName( false ),
	_dependencyNamesLoaded( false ),
	_nameflags( 0 ),
	_emaConfigErrList( emaConfigErrList ),
	_configList()
{
	addConfigure( map );
}

void ProgrammaticConfigure::clear()
{
	_consumerName.clear();
	_niProviderName.clear();
	_iProviderName.clear();
	_channelName.clear();
	_loggerName.clear();
	_dictionaryName.clear();
	_directoryName.clear();
	_overrideConsName = false;
	_overrideNiProvName = false;
	_overrideIProvName = false;
	_dependencyNamesLoaded = false;
	_nameflags = 0;
}

void ProgrammaticConfigure::addConfigure( const Map& map )
{
	for ( UInt32 i = 0; i < _configList.size() ; ++i )
	{
		if ( _configList[i] == &map )
			return;
	}

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
		clear();

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
		clear();

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
		clear();

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

void ProgrammaticConfigure::retrieveDependencyNames( const Map& map, const EmaString& userName,
    UInt8& flags, EmaString& channelName,
    EmaString& loggerName, EmaString& dictionaryName,
    EmaString& channelSet, EmaString& directoryName )
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
													channelName = instanceEntry.getAscii();
													flags |= 0x01;
													channelPos = position;
												}
												else if ( instanceEntry.getName() == "Logger" )
												{
													loggerName = instanceEntry.getAscii();
													flags |= 0x02;
												}
												else if ( instanceEntry.getName() == "Dictionary" )
												{
													dictionaryName = instanceEntry.getAscii();
													flags |= 0x04;
												}
												else if ( instanceEntry.getName() == "ChannelSet" )
												{
													channelSet = instanceEntry.getAscii();
													flags |= 0x08;
													channelSetPos = position;
												}
												else if ( instanceEntry.getName() == "Directory" )
												{
													directoryName = instanceEntry.getAscii();
													flags |= 0x10;
												}
												break;
											}
										}
										if ( ( flags & 0x01 ) && ( flags & 0x08 ) )
										{
											if ( channelSetPos > channelPos )
											{
												flags &= ~0x01;
												channelName.clear();
											}
											else
											{
												flags &= ~0x08;
												channelSet.clear();
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
			retrieveDependencyNames( *_configList[i], instanceName, _nameflags, _channelName, _loggerName, _dictionaryName, _channelSet, _directoryName );

		_dependencyNamesLoaded = true;
	}

	if ( _nameflags & 0x01 )
	{
		channelName = _channelName;
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
			retrieveDependencyNames( *_configList[i], instanceName, _nameflags, _channelName, _loggerName, _dictionaryName, _channelSet, _directoryName );

		_dependencyNamesLoaded = true;
	}

	if ( _nameflags & 0x08 )
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
			retrieveDependencyNames( *_configList[i], instanceName, _nameflags, _channelName, _loggerName, _dictionaryName, _channelSet, _directoryName );

		_dependencyNamesLoaded = true;
	}

	if ( _nameflags & 0x02 )
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
			retrieveDependencyNames( *_configList[i], instanceName, _nameflags, _channelName, _loggerName, _dictionaryName, _channelSet, _directoryName );

		_dependencyNamesLoaded = true;
	}

	if ( _nameflags & 0x04 )
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
			retrieveDependencyNames( *_configList[i], instanceName, _nameflags, _channelName, _loggerName, _dictionaryName, _channelSet, _directoryName );

		_dependencyNamesLoaded = true;
	}

	if ( _nameflags & 0x10 )
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

	StaticDecoder::setData( &const_cast<Map&>( map ), 0 );

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

	StaticDecoder::setData( &const_cast<Map&>( map ), 0 );

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

	StaticDecoder::setData( &const_cast<Map&>( map ), 0 );

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
	StaticDecoder::setData( &const_cast<Map&>( map ), 0 );

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
	StaticDecoder::setData( &const_cast<Map&>( map ), 0 );

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
	StaticDecoder::setData( &const_cast<Map&>( map ), 0 );

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

void  ProgrammaticConfigure::retrieveCustomConfig( const EmaString& instanceName, ActiveConfig& activeConfig )
{
	for ( UInt32 i = 0; i < _configList.size(); i++ )
		retrieveInstanceCustomConfig( *_configList[i], instanceName, _emaConfigErrList, activeConfig );
}

void  ProgrammaticConfigure::retrieveChannelConfig( const EmaString& channelName,  ActiveConfig& activeConfig, int hostFnCalled, bool isReadingLastChannel, ChannelConfig* fileCfg)
{
	for ( UInt32 i = 0 ; i < _configList.size() ; i++ )
		retrieveChannel( *_configList[i], channelName, _emaConfigErrList, activeConfig, hostFnCalled, fileCfg, isReadingLastChannel);
}

void  ProgrammaticConfigure::retrieveLoggerConfig( const EmaString& loggerName, ActiveConfig& activeConfig )
{
	for ( UInt32 i = 0 ; i < _configList.size() ; i++ )
		retrieveLogger( *_configList[i], loggerName, _emaConfigErrList, activeConfig );
}

void  ProgrammaticConfigure::retrieveDictionaryConfig( const EmaString& dictionaryName, ActiveConfig& activeConfig )
{
	for ( UInt32 i = 0 ; i < _configList.size() ; i++ )
		retrieveDictionary( *_configList[i], dictionaryName, _emaConfigErrList, activeConfig );
}

void  ProgrammaticConfigure::retrieveDirectoryConfig( const EmaString& directoryName, DirectoryCache& directoryCahce )
{
	for ( UInt32 i = 0 ; i < _configList.size() ; i++ )
		retrieveDirectory( *_configList[i], directoryName, _emaConfigErrList, directoryCahce );
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
													activeConfig.parameterConfigGroup |= PARAMETER_SET_BY_PROGRAMMATIC;
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
													activeConfig.parameterConfigGroup |= PARAMETER_SET_BY_PROGRAMMATIC;
												}
												else if (eentry.getName() == "XmlTraceToStdout")
												{
													activeConfig.xmlTraceToStdout = eentry.getUInt() ? true : false;
													activeConfig.parameterConfigGroup |= PARAMETER_SET_BY_PROGRAMMATIC;
												}
												else if (eentry.getName() == "XmlTraceToMultipleFiles")
												{
													activeConfig.xmlTraceToMultipleFiles = eentry.getUInt() ? true : false;
													activeConfig.parameterConfigGroup |= PARAMETER_SET_BY_PROGRAMMATIC;
												}
												else if (eentry.getName() == "XmlTraceWrite")
												{
													activeConfig.xmlTraceWrite = eentry.getUInt() ? true : false;
													activeConfig.parameterConfigGroup |= PARAMETER_SET_BY_PROGRAMMATIC;
												}
												else if (eentry.getName() == "XmlTraceRead")
												{
													activeConfig.xmlTraceRead = eentry.getUInt() ? true : false;
													activeConfig.parameterConfigGroup |= PARAMETER_SET_BY_PROGRAMMATIC;
												}
												else if (eentry.getName() == "XmlTracePing")
												{
													activeConfig.xmlTracePing = eentry.getUInt() ? true : false;
													activeConfig.parameterConfigGroup |= PARAMETER_SET_BY_PROGRAMMATIC;
												}
												else if (eentry.getName() == "XmlTraceHex")
												{
													activeConfig.xmlTraceHex = eentry.getUInt() ? true : false;
													activeConfig.parameterConfigGroup |= PARAMETER_SET_BY_PROGRAMMATIC;
												}
												else if (eentry.getName() == "MsgKeyInUpdates")
												{
													activeConfig.msgKeyInUpdates = eentry.getUInt() ? true : false;
													activeConfig.parameterConfigGroup |= PARAMETER_SET_BY_PROGRAMMATIC;
												}
												else if ( eentry.getName() == "LoginRequestTimeOut" )
												{
													activeConfig.setLoginRequestTimeOut( eentry.getUInt() );
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
													activeConfig.parameterConfigGroup |= PARAMETER_SET_BY_PROGRAMMATIC;
												}
												else if (eentry.getName() == "ReconnectAttemptLimit")
												{
													activeConfig.setReconnectAttemptLimit( eentry.getInt() );
													activeConfig.parameterConfigGroup |= PARAMETER_SET_BY_PROGRAMMATIC;
												}
												else if (eentry.getName() == "ReconnectMinDelay")
												{
													activeConfig.setReconnectMinDelay( eentry.getInt() );
													activeConfig.parameterConfigGroup |= PARAMETER_SET_BY_PROGRAMMATIC;
												}
												else if (eentry.getName() == "ReconnectMaxDelay")
												{
													activeConfig.setReconnectMaxDelay( eentry.getInt() );
													activeConfig.parameterConfigGroup |= PARAMETER_SET_BY_PROGRAMMATIC;
												}
												else if ( eentry.getName() == "PipePort" )
												{
													activeConfig.pipePort = eentry.getInt();
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

void ProgrammaticConfigure::retrieveInstanceCustomConfig( const Map& map, const EmaString& instanceName, EmaConfigErrorList& emaConfigErrList, ActiveConfig& activeConfig )
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
													activeConfig.setObeyOpenWindow( eentry.getUInt() );
												}
												else if ( eentry.getName() == "PostAckTimeout" )
												{
													activeConfig.setPostAckTimeout( eentry.getUInt() );
												}
												else if ( eentry.getName() == "MaxOutstandingPosts" )
												{
													activeConfig.setMaxOutstandingPosts( eentry.getUInt() );
												}
												else if ( eentry.getName() == "DirectoryRequestTimeOut" )
												{
													activeConfig.setDirectoryRequestTimeOut( eentry.getUInt() );
												}
												else if ( eentry.getName() == "DictionaryRequestTimeOut" )
												{
													activeConfig.setDictionaryRequestTimeOut( eentry.getUInt() );
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
											case DataType::AsciiEnum:
												break;

											case DataType::UIntEnum:
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
	}
}

void ProgrammaticConfigure::retrieveChannel( const Map& map, const EmaString& channelName, EmaConfigErrorList& emaConfigErrList,
    ActiveConfig& activeConfig, int hostFnCalled, ChannelConfig* fileCfg, bool isReadingLastChannel)
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
										retrieveChannelInfo( mapEntry, channelName, emaConfigErrList, activeConfig, hostFnCalled, fileCfg, isReadingLastChannel);
								}
							}
						}
					}
				}
			}
		}
	}
}

void ProgrammaticConfigure::retrieveChannelInfo( const MapEntry& mapEntry, const EmaString& channelName, EmaConfigErrorList& emaConfigErrList,
    ActiveConfig& activeConfig, int setByFnCalled, ChannelConfig* fileCfg, bool isReadingLastChannel)
{
	const ElementList& elementListChannel = mapEntry.getElementList();

	EmaString name, interfaceName, host, port, objectName, tunnelingProxyHost, tunnelingProxyPort;
	UInt16 channelType, compressionType, tunnelingSecurityProtocol;
	UInt64 guaranteedOutputBuffers, compressionThreshold, connectionPingTimeout, numInputBuffers, sysSendBufSize, sysRecvBufSize, highWaterMark,
	       tcpNodelay;

	UInt64 flags = 0;
	UInt32 maxUInt32 = 0xFFFFFFFF;
	UInt64 mcastFlags = 0;
	UInt64 tunnelingFlags = 0;
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
			/* @deprecated DEPRECATED:
			*ReconnectAttemptLimit,ReconnectMinDelay,ReconnectMaxDelay,MsgKeyInUpdates,XmlTrace is per consumer/niprov/iprov instance based now.
			The following varable will be removed in the future.
			*/
			else if ( isReadingLastChannel && channelEntry.getName() == "XmlTraceFileName" )
				activeConfig.xmlTraceFileName = channelEntry.getAscii();
			else if ( channelEntry.getName() == "InterfaceName" )
			{
				interfaceName = channelEntry.getAscii();
				flags |= 0x80000;
			}
			else if ( channelEntry.getName() == "ObjectName" )
			{
				objectName = channelEntry.getAscii();
				tunnelingFlags |= 0x02;
			}
			else if (channelEntry.getName() == "ProxyPort")
			{
				tunnelingProxyPort = channelEntry.getAscii();
				tunnelingFlags |= 0x04;
			}
			else if (channelEntry.getName() == "ProxyHost")
			{
				tunnelingProxyHost = channelEntry.getAscii();
				tunnelingFlags |= 0x08;
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

				if ( compressionType > 2 )
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
			}else if ( channelEntry.getName() == "SecurityProtocol" )
			{
				tunnelingSecurityProtocol = channelEntry.getEnum();

				if ( tunnelingSecurityProtocol > 7 )
				{
					EmaString text( "Invalid SecurityProtocol [" );
					text.append(tunnelingSecurityProtocol);
					text.append( "] in Programmatic Configuration. Use default SecurityProtocol [" );
					text.append(RSSL_ENC_TLSV1_2);
					text.append( "] " );
					EmaConfigError* mce( new EmaConfigError( text, OmmLoggerClient::ErrorEnum ) );
					emaConfigErrList.add( mce );
				}
				else
				{
					tunnelingFlags |= 0x10;
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
			else if (isReadingLastChannel && channelEntry.getName() == "XmlTraceToFile")
				activeConfig.xmlTraceToFile = channelEntry.getUInt() ? true : false;
			else if (isReadingLastChannel && channelEntry.getName() == "XmlTraceToStdout")
				activeConfig.xmlTraceToStdout = channelEntry.getUInt() ? true : false;
			else if (isReadingLastChannel && channelEntry.getName() == "XmlTraceToMultipleFiles")
				activeConfig.xmlTraceToMultipleFiles = channelEntry.getUInt() ? true : false;
			else if (isReadingLastChannel && channelEntry.getName() == "XmlTraceWrite")
				activeConfig.xmlTraceWrite = channelEntry.getUInt() ? true : false;
			else if (isReadingLastChannel && channelEntry.getName() == "XmlTraceRead")
				activeConfig.xmlTraceRead = channelEntry.getUInt() ? true : false;
			else if (isReadingLastChannel && channelEntry.getName() == "XmlTracePing")
				activeConfig.xmlTracePing = channelEntry.getUInt() ? true : false;
			else if (isReadingLastChannel && channelEntry.getName() == "XmlTraceHex")
				activeConfig.xmlTraceHex = channelEntry.getUInt() ? true : false;
			else if (isReadingLastChannel && channelEntry.getName() == "MsgKeyInUpdates")
				activeConfig.msgKeyInUpdates = channelEntry.getUInt() ? true : false;
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
			break;

		case DataType::IntEnum:
			if (isReadingLastChannel && channelEntry.getName() == "XmlTraceMaxFileSize")
			{
				activeConfig.xmlTraceMaxFileSize = channelEntry.getInt();
			}
			else if (isReadingLastChannel && channelEntry.getName() == "ReconnectAttemptLimit")
			{
				activeConfig.setReconnectAttemptLimit(channelEntry.getInt());
			}
			else if (isReadingLastChannel && channelEntry.getName() == "ReconnectMinDelay")
			{
				activeConfig.setReconnectMinDelay(channelEntry.getInt());
			}
			else if (isReadingLastChannel && channelEntry.getName() == "ReconnectMaxDelay")
			{
				activeConfig.setReconnectMaxDelay(channelEntry.getInt());
			}
			break;
		}
	}

	if ( flags & 0x10 )
	{
		if ( setByFnCalled == SOCKET_CONN_HOST_CONFIG_BY_FUNCTION_CALL )
		{
			channelType = RSSL_CONN_TYPE_SOCKET;
			activeConfig.clearChannelSet();
		}
		else if ( setByFnCalled == TUNNELING_CONN_CONFIG_BY_FUNCTION_CALL )
		{
			if (channelType == RSSL_CONN_TYPE_SOCKET)
				channelType = RSSL_CONN_TYPE_ENCRYPTED;
			activeConfig.clearChannelSet();
		}

		unsigned int positionFound = 0;
		ChannelConfig* pCurrentChannelConfig = 0;

		try
		{
			if ( channelType == RSSL_CONN_TYPE_SOCKET )
			{
				SocketChannelConfig* socketChannelConfig = new SocketChannelConfig( activeConfig.defaultServiceName() );
				pCurrentChannelConfig = socketChannelConfig;
				activeConfig.configChannelSet.push_back( pCurrentChannelConfig );

				SocketChannelConfig* fileCfgSocket = NULL;
				if ( fileCfg && fileCfg->connectionType == RSSL_CONN_TYPE_SOCKET )
					fileCfgSocket = static_cast<SocketChannelConfig*>( fileCfg );

				if ( flags & 0x80 )
					socketChannelConfig->tcpNodelay = tcpNodelay ? true : false;
				else if ( fileCfgSocket )
					socketChannelConfig->tcpNodelay = fileCfgSocket->tcpNodelay;

				if ( flags & 0x02 && setByFnCalled == 0 )
					socketChannelConfig->hostName = host;
				else if ( fileCfgSocket )
					socketChannelConfig->hostName = fileCfgSocket->hostName;

				if ( flags & 0x04 && setByFnCalled == 0 )
					socketChannelConfig->serviceName = port;
				else if ( fileCfgSocket )
					socketChannelConfig->serviceName = fileCfgSocket->serviceName;

			}
			else if ( channelType == RSSL_CONN_TYPE_RELIABLE_MCAST )
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
			else if ( channelType == RSSL_CONN_TYPE_ENCRYPTED || channelType == RSSL_CONN_TYPE_HTTP )
			{
				EncryptedChannelConfig* encryptChanelConfig = new EncryptedChannelConfig();
				encryptChanelConfig->connectionType = (RsslConnectionTypes) channelType;
				pCurrentChannelConfig = encryptChanelConfig;
				activeConfig.configChannelSet.push_back( pCurrentChannelConfig );

				EncryptedChannelConfig* fileCfgEncrypt = NULL;
				if ( fileCfg && (fileCfg->connectionType == RSSL_CONN_TYPE_ENCRYPTED || fileCfg->connectionType == RSSL_CONN_TYPE_HTTP) )
					fileCfgEncrypt = static_cast<EncryptedChannelConfig*>( fileCfg );

				if ( flags & 0x80 )
					encryptChanelConfig->tcpNodelay = tcpNodelay ? true : false;
				else if ( fileCfgEncrypt )
					encryptChanelConfig->tcpNodelay = fileCfgEncrypt->tcpNodelay;

				if ( flags & 0x02 )
					encryptChanelConfig->hostName = host;
				else if ( fileCfgEncrypt )
					encryptChanelConfig->hostName = fileCfgEncrypt->hostName;

				if ( flags & 0x04 )
					encryptChanelConfig->serviceName = port;
				else if ( fileCfgEncrypt )
					encryptChanelConfig->serviceName = fileCfgEncrypt->serviceName;

				if (tunnelingFlags & 0x02 && setByFnCalled == 0 )
					encryptChanelConfig->objectName = objectName;
				else if ( fileCfgEncrypt )
					encryptChanelConfig->objectName = fileCfgEncrypt->objectName;

				if (tunnelingFlags & 0x04 && setByFnCalled == 0 )
					encryptChanelConfig->proxyPort = tunnelingProxyPort;
				else if (fileCfgEncrypt)
					encryptChanelConfig->proxyPort = fileCfgEncrypt->proxyPort;

				if (tunnelingFlags & 0x08 && setByFnCalled == 0 )
					encryptChanelConfig->proxyHostName = tunnelingProxyHost;
				else if (fileCfgEncrypt)
					encryptChanelConfig->proxyHostName = fileCfgEncrypt->proxyHostName;

				if (tunnelingFlags & 0x10 && setByFnCalled == 0 )
					encryptChanelConfig->securityProtocol = tunnelingSecurityProtocol;
				else if (fileCfgEncrypt)
					encryptChanelConfig->securityProtocol = fileCfgEncrypt->securityProtocol;
			}
		}
		catch ( std::bad_alloc )
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
				pCurrentChannelConfig->compressionThreshold = compressionThreshold > maxUInt32 ? maxUInt32 : ( UInt32 )compressionThreshold;
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
			pCurrentChannelConfig->sysRecvBufSize = sysRecvBufSize > maxUInt32 ? maxUInt32 : ( UInt32 )sysRecvBufSize;
		else if ( useFileCfg )
			pCurrentChannelConfig->sysRecvBufSize = fileCfg->sysRecvBufSize;

		if ( flags & 0x400000 )
			pCurrentChannelConfig->sysSendBufSize = sysSendBufSize > maxUInt32 ? maxUInt32 : ( UInt32 )sysSendBufSize;
		else if ( useFileCfg )
			pCurrentChannelConfig->sysSendBufSize = fileCfg->sysSendBufSize;

		if ( flags & 0x8000000 )
			pCurrentChannelConfig->highWaterMark = highWaterMark > maxUInt32 ? maxUInt32 : (UInt32) highWaterMark;
		else if ( useFileCfg )
			pCurrentChannelConfig->highWaterMark = fileCfg->highWaterMark;

		if ( flags & 0x8000 )
			pCurrentChannelConfig->connectionPingTimeout = connectionPingTimeout > maxUInt32  ? maxUInt32 : ( UInt32 )connectionPingTimeout;
		else if ( useFileCfg )
			pCurrentChannelConfig->connectionPingTimeout = fileCfg->connectionPingTimeout;
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
    ActiveConfig& activeConfig )
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
    ActiveConfig& activeConfig )
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
													activeConfig.dictionaryConfig.rdmfieldDictionaryFileName = entry.getAscii();
												}
												else if ( entry.getName() == "EnumTypeDefFileName" )
												{
													activeConfig.dictionaryConfig.enumtypeDefFileName = entry.getAscii();
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
														text.append( "] in Programmatic Configuration. Use default DictionaryType [" );
														text.append( DEFAULT_DICTIONARY_TYPE );
														text.append( "]" );
														EmaConfigError* mce( new EmaConfigError( text, OmmLoggerClient::ErrorEnum ) );
														emaConfigErrList.add( mce );
													}
													else
													{
														activeConfig.dictionaryConfig.dictionaryType = ( Dictionary::DictionaryType )dictionaryType;
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

void ProgrammaticConfigure::retrieveDirectory( const Map& map, const EmaString& directoryName, EmaConfigErrorList& emaConfigErrList,
    DirectoryCache& directoryCache )
{
	map.reset();
	while ( map.forth() )
	{
		const MapEntry& mapEntry = map.getEntry();

		if ( mapEntry.getKey().getDataType() == DataType::AsciiEnum && mapEntry.getKey().getAscii() == "DirectoryGroup" )
		{
			if ( mapEntry.getLoadType() == DataType::ElementListEnum )
			{
				const ElementList& elementList = mapEntry.getElementList();

				while ( elementList.forth() )
				{
					const ElementEntry& elementEntry = elementList.getEntry();

					if ( elementEntry.getLoadType() == DataType::MapEnum )
					{
						if ( elementEntry.getName() == "DirectoryList" )
						{
							const Map& map = elementEntry.getMap();

							while ( map.forth() )
							{
								const MapEntry& mapEntry = map.getEntry();

								if ( ( mapEntry.getKey().getDataType() == DataType::AsciiEnum ) && ( mapEntry.getKey().getAscii() == directoryName ) )
								{
									if ( mapEntry.getLoadType() == DataType::ElementListEnum )
									{
										const ElementList& elementListDirectory = mapEntry.getElementList();

										while ( elementListDirectory.forth() )
										{
											const ElementEntry& entry = elementListDirectory.getEntry();

											switch ( entry.getLoadType() )
											{
											case DataType::AsciiEnum:

												// todo ... provide missing impl
												break;

											case DataType::EnumEnum:

												// todo .... provide missing impl
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

void ProgrammaticConfigure::retrieveGroupAndListName( const Map& map, EmaString& groupName, EmaString& listName )
{
	static EmaString group;
	static EmaString list;
	static bool set( false );

	if ( set )
	{
		groupName = group;
		listName = list;
		return;
	}

	map.reset();
	while ( map.forth() )
	{
		const MapEntry& mapEntry = map.getEntry();
		if ( mapEntry.getKey().getDataType() == DataType::AsciiEnum )
			if ( mapEntry.getKey().getAscii() == "ConsumerGroup" )
			{
				group = "ConsumerGroup";
				list = "ConsumerList";
				break;
			}
			else if ( mapEntry.getKey().getAscii() == "NiProviderGroup" )
			{
				group = "NiProviderGroup";
				list = "NiProviderList";
				break;
			}
	}

	set = true;
	groupName = group;
	listName = list;
}
