/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */


#include "OmmConsumerActiveConfig.h"
#include "OmmInvalidConfigurationException.h"
#include "ElementList.h"
#include "Access/Include/Common.h"
#include "StaticDecoder.h"

using namespace thomsonreuters::ema::access;


ProgrammaticConfigure::ProgrammaticConfigure( const Map& map, EmaConfigErrorList& emaConfigErrList ) :
_emaConfigErrList( emaConfigErrList ),
_configList(),
_nameflags( 0 ),
_loadnames( false ),
_overrideConsName( false ),
_consumerName(),
_channelName(),
_channelSet(),
_loggerName(),
_dictionaryName()
{
	addConfigure( map );
}

void ProgrammaticConfigure::clear()
{
	_nameflags = 0;
	_loadnames = false;
	_channelName.clear();
	_loggerName.clear();
	_dictionaryName.clear();
}

void ProgrammaticConfigure::addConfigure( const Map& map )
{
	for( UInt32 i = 0; i < _configList.size() ; i ++ )
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
			found = ProgrammaticConfigure::retrieveDefaultConsumer( *_configList[i], defaultConsumer );
	}

	return found;
}

bool ProgrammaticConfigure::specifyConsumerName( const EmaString& consumerName )
{
	bool found = false;

	for ( UInt32 i = 0 ; i < _configList.size() ; i++ )
	{
		found = ProgrammaticConfigure::validateConsumerName( *_configList[i], consumerName );

		if ( found )
		{
			_overrideConsName = true;
			_consumerName = consumerName;
			break;
		}
	}

	return found;
}

void ProgrammaticConfigure::retrieveDependencyNames( const Map& map, const EmaString& consumerName, UInt8& flags, EmaString& channelName, 
													EmaString& loggerName, EmaString& dictionaryName, EmaString& channelSet )
{
	unsigned int position = 0;
	unsigned int channelPos = 0, channelSetPos = 0;

	map.reset();
	while ( map.forth() )
	{
		const MapEntry & mapEntry = map.getEntry();

		if ( mapEntry.getKey().getDataType() == DataType::AsciiEnum && mapEntry.getKey().getAscii() == "ConsumerGroup" )
		{
			if ( mapEntry.getLoadType() == DataType::ElementListEnum )
			{
				const ElementList & elementList = mapEntry.getElementList();

				while ( elementList.forth() )
				{
					const ElementEntry & elementEntry = elementList.getEntry();

					if ( elementEntry.getLoadType() == DataType::MapEnum )
					{
						if ( elementEntry.getName() == "ConsumerList" && ( elementEntry.getLoad().getDataType() == DataType::MapEnum ) )
						{
							const Map& map = elementEntry.getMap();

							while ( map.forth() )
							{
								const MapEntry & mapEntry = map.getEntry();

								if ( ( mapEntry.getKey().getDataType() == DataType::AsciiEnum ) && ( mapEntry.getKey().getAscii() == consumerName ) )
								{
									if ( mapEntry.getLoadType() == DataType::ElementListEnum )
									{
										const ElementList & elementListConsumer = mapEntry.getElementList();
										position = 0;
										while ( elementListConsumer.forth() )
										{
											const ElementEntry & consumerEntry = elementListConsumer.getEntry();
											position++;
											switch ( consumerEntry.getLoadType() )
											{
												case DataType::AsciiEnum:
													if ( consumerEntry.getName() == "Channel" )
													{
														channelName = consumerEntry.getAscii();
														flags |= 0x01;
														channelPos = position;
													}
													else if ( consumerEntry.getName() == "Logger" )
													{
														loggerName = consumerEntry.getAscii();
														flags |= 0x02;
													}
													else if ( consumerEntry.getName() == "Dictionary" )
													{
														dictionaryName = consumerEntry.getAscii();
														flags |= 0x04;
													}
													else if ( consumerEntry.getName() == "ChannelSet" )
													{
														channelSet = consumerEntry.getAscii();
														flags |= 0x08;
														channelSetPos = position;
													}
													break;
											}
										}
										if((flags & 0x01) && (flags & 0x08))
										{
											if(channelSetPos > channelPos)
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

bool ProgrammaticConfigure::getActiveChannelName( const EmaString& consumerName, EmaString& channelName )
{
	if ( !_loadnames )
	{
		for ( UInt32 i = 0 ; i < _configList.size() ; i++ )
			retrieveDependencyNames( *_configList[i], consumerName, _nameflags,_channelName, _loggerName, _dictionaryName, _channelSet );

		_loadnames = true;
	}

	if ( _nameflags & 0x01 )
	{
		channelName = _channelName;
		return true;
	}
	else
		return false;
}

bool ProgrammaticConfigure::getActiveChannelSet( const EmaString& consumerName, EmaString& channelSet)
{
	if ( !_loadnames )
	{
		for ( UInt32 i = 0 ; i < _configList.size() ; i++ )
			retrieveDependencyNames( *_configList[i], consumerName, _nameflags,_channelName, _loggerName, _dictionaryName, _channelSet );

		_loadnames = true;
	}

	if ( _nameflags & 0x08 )
	{
		channelSet = _channelSet;
		return true;
	}
	else
		return false;
}


bool ProgrammaticConfigure::getActiveLoggerName( const EmaString& consumerName, EmaString& loggerName )
{
	if ( !_loadnames )
	{
		for ( UInt32 i = 0 ; i < _configList.size() ; i++ )
			retrieveDependencyNames( *_configList[i], consumerName, _nameflags,_channelName, _loggerName, _dictionaryName, _channelSet);

		_loadnames = true;
	}

	if ( _nameflags & 0x02 )
	{
		loggerName = _loggerName;
		return true;
	}
	else
		return false;
}


bool ProgrammaticConfigure::getActiveDictionaryName( const EmaString& consumerName, EmaString& dictionaryName )
{
	if ( !_loadnames )
	{
		for ( UInt32 i = 0 ; i < _configList.size() ; i++ )
			retrieveDependencyNames( *_configList[i], consumerName, _nameflags,_channelName, _loggerName, _dictionaryName, _channelSet );

		_loadnames = true;
	}

	if ( _nameflags & 0x04 )
	{
		dictionaryName = _dictionaryName;
		return true;
	}
	else
		return false;
}

bool ProgrammaticConfigure::retrieveDefaultConsumer( const Map& map, EmaString& defaultConsumer )
{
	bool foundDefaultConsumer = false;

	StaticDecoder::setData( &const_cast<Map &>(map), 0);

	while ( map.forth() )
	{
		const MapEntry & mapEntry = map.getEntry();

		if (  ( mapEntry.getKey().getDataType() == DataType::AsciiEnum ) && ( mapEntry.getKey().getAscii() == "ConsumerGroup" ) 
			&& ( mapEntry.getLoad().getDataType() == DataType::ElementListEnum ) )
		{
			const ElementList & elementList = mapEntry.getElementList();

			while ( elementList.forth() )
			{
				const ElementEntry & elementEntry = elementList.getEntry();

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

bool ProgrammaticConfigure::validateConsumerName( const Map & map, const EmaString& consumerName )
{
	StaticDecoder::setData( &const_cast<Map &>(map), 0);

	while ( map.forth() )
	{
		const MapEntry & mapEntry = map.getEntry();

		if ( ( mapEntry.getKey().getDataType() == DataType::AsciiEnum ) && ( mapEntry.getKey().getAscii() == "ConsumerGroup" ) 
			&& ( mapEntry.getLoad().getDataType() == DataType::ElementListEnum ) )
		{
			const ElementList & elementList = mapEntry.getElementList();

			while ( elementList.forth() )
			{
				const ElementEntry & elementEntry = elementList.getEntry();

				if ( ( elementEntry.getName() == "ConsumerList" ) && ( elementEntry.getLoad().getDataType() == DataType::MapEnum ) )
				{
					const Map& consumerMap = elementEntry.getMap();

					while( consumerMap.forth() )
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

void  ProgrammaticConfigure::retrieveConsumerConfig( const EmaString& consumerName, OmmConsumerActiveConfig& activeConfig )
{
	for ( UInt32 i = 0 ; i < _configList.size() ; i++ )
		ProgrammaticConfigure::retrieveConsumer( *_configList[i], consumerName, _emaConfigErrList, activeConfig );
}

void  ProgrammaticConfigure::retrieveChannelConfig( const EmaString& channelName,  OmmConsumerActiveConfig& activeConfig, bool hostFnCalled, ChannelConfig *fileCfg )
{
	for ( UInt32 i = 0 ; i < _configList.size() ; i++ )
		ProgrammaticConfigure::retrieveChannel( *_configList[i], channelName, _emaConfigErrList, activeConfig, hostFnCalled, fileCfg );
}

void  ProgrammaticConfigure::retrieveLoggerConfig( const EmaString& loggerName, OmmConsumerActiveConfig& activeConfig )
{
	for ( UInt32 i = 0 ; i < _configList.size() ; i++ )
		ProgrammaticConfigure::retrieveLogger( *_configList[i], loggerName, _emaConfigErrList, activeConfig );
}

void  ProgrammaticConfigure::retrieveDictionaryConfig( const EmaString& dictionaryName, OmmConsumerActiveConfig& activeConfig )
{
	for ( UInt32 i = 0 ; i < _configList.size() ; i++ )
		ProgrammaticConfigure::retrieveDictionary( *_configList[i], dictionaryName, _emaConfigErrList, activeConfig );
}

void ProgrammaticConfigure::retrieveConsumer( const Map& map, const EmaString& consumerName, EmaConfigErrorList& emaConfigErrList,
												  OmmConsumerActiveConfig& ommConsumerActiveConfig )
{
	map.reset();
	while ( map.forth() )
	{
		const MapEntry & mapEntry = map.getEntry();

		if ( mapEntry.getKey().getDataType() == DataType::AsciiEnum && mapEntry.getKey().getAscii() == "ConsumerGroup" )
		{
			if ( mapEntry.getLoadType() == DataType::ElementListEnum )
			{
				const ElementList & elementList = mapEntry.getElementList();

				while ( elementList.forth() )
				{
					const ElementEntry & elementEntry = elementList.getEntry();

					if ( elementEntry.getLoadType() == DataType::MapEnum )
					{
						if ( elementEntry.getName() == "ConsumerList" )
						{
							const Map& map = elementEntry.getMap();

							while ( map.forth() )
							{
								const MapEntry & mapEntry = map.getEntry();

								if ( ( mapEntry.getKey().getDataType() == DataType::AsciiEnum ) && ( mapEntry.getKey().getAscii() == consumerName ) )
								{
									if ( mapEntry.getLoadType() == DataType::ElementListEnum )
									{
										const ElementList & elementListConsumer = mapEntry.getElementList();

										while ( elementListConsumer.forth() )
										{
											const ElementEntry & consumerEntry = elementListConsumer.getEntry();

											switch ( consumerEntry.getLoadType() )
											{
												case DataType::AsciiEnum:
													break;

												case DataType::UIntEnum:
													if ( consumerEntry.getName() == "ItemCountHint" )
													{
														ommConsumerActiveConfig.setItemCountHint(consumerEntry.getUInt());
													}
													else if ( consumerEntry.getName() == "ServiceCountHint" )
													{
														ommConsumerActiveConfig.setServiceCountHint(consumerEntry.getUInt());
													}
													else if ( consumerEntry.getName() == "ObeyOpenWindow" )
													{
														ommConsumerActiveConfig.setObeyOpenWindow(consumerEntry.getUInt());
													}
													else if ( consumerEntry.getName() == "PostAckTimeout" )
													{
														ommConsumerActiveConfig.setPostAckTimeout(consumerEntry.getUInt());
													}
													else if ( consumerEntry.getName() == "RequestTimeout" )
													{
														ommConsumerActiveConfig.setRequestTimeout(consumerEntry.getUInt());
													}
													else if ( consumerEntry.getName() == "MaxOutstandingPosts" )
													{
														ommConsumerActiveConfig.setMaxOutstandingPosts(consumerEntry.getUInt());
													}
													else if ( consumerEntry.getName() == "CatchUnhandledException" )
													{
														ommConsumerActiveConfig.setCatchUnhandledException(consumerEntry.getUInt());
													}
													else if ( consumerEntry.getName() == "MaxDispatchCountApiThread" )
													{
														ommConsumerActiveConfig.setMaxDispatchCountApiThread(consumerEntry.getUInt());
													}
													else if ( consumerEntry.getName() == "MaxDispatchCountUserThread" )
													{
														ommConsumerActiveConfig.setMaxDispatchCountUserThread(consumerEntry.getUInt());
													}
													else if ( consumerEntry.getName() == "LoginRequestTimeout" )
													{
														ommConsumerActiveConfig.setLoginRequestTimeOut(consumerEntry.getUInt());
													}
													else if ( consumerEntry.getName() == "DirectoryRequestTimeout" )
													{
														ommConsumerActiveConfig.setDirectoryRequestTimeOut(consumerEntry.getUInt());
													}
													break;
									
												case DataType::IntEnum:

													if ( consumerEntry.getName() == "DispatchTimeoutApiThread" )
													{
														ommConsumerActiveConfig.dispatchTimeoutApiThread = consumerEntry.getInt();
													}
													else if ( consumerEntry.getName() == "PipePort" )
													{
														ommConsumerActiveConfig.pipePort = consumerEntry.getInt();
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
											OmmConsumerActiveConfig& ommConsumerActiveConfig, bool hostFnCalled, ChannelConfig *fileCfg )
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

				while ( elementList.forth())
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
										retrieveChannelInfo(mapEntry, channelName, emaConfigErrList, ommConsumerActiveConfig, hostFnCalled, fileCfg);									
								}
							}
						}
					}
				}
			}
		}
	}

}

void ProgrammaticConfigure::retrieveChannelInfo(const MapEntry& mapEntry, const EmaString& channelName, EmaConfigErrorList& emaConfigErrList,
											OmmConsumerActiveConfig& ommConsumerActiveConfig, bool hostFnCalled, ChannelConfig *fileCfg )
{
	const ElementList& elementListChannel = mapEntry.getElementList();

	EmaString name, interfaceName, host, port, xmlTraceFileName, objectName;
	thomsonreuters::ema::access::UInt16 channelType, compressionType;
	thomsonreuters::ema::access::Int64 reconnectAttemptLimit, reconnectMinDelay, reconnectMaxDelay, xmlTraceMaxFileSize;
	thomsonreuters::ema::access::UInt64 guaranteedOutputBuffers, compressionThreshold, connectionPingTimeout, numInputBuffers, sysSendBufSize, sysRecvBufSize,
	  tcpNodelay, xmlTraceToFile, xmlTraceToStdout, xmlTraceToMultipleFiles, xmlTraceWrite, xmlTraceRead, xmlTracePing, xmlTraceHex, msgKeyInUpdates;

	thomsonreuters::ema::access::UInt64 flags = 0;
	thomsonreuters::ema::access::UInt32 maxUInt32 = 0xFFFFFFFF;
	thomsonreuters::ema::access::UInt64 mcastFlags = 0;

	ReliableMcastChannelConfig tempRelMcastCfg;
	while ( elementListChannel.forth() )
	{
		const ElementEntry & channelEntry = elementListChannel.getEntry();

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
			else if ( channelEntry.getName() == "XmlTraceFileName" )
			{
				xmlTraceFileName = channelEntry.getAscii();
				flags |= 0x08;
			}
			else if ( channelEntry.getName() == "InterfaceName" )
			{
				interfaceName = channelEntry.getAscii();
				flags |= 0x80000;
			}
			else if ( channelEntry.getName() == "ObjectName" )
			{
				objectName = channelEntry.getAscii();
				flags |= 0x100000;
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
					EmaString text( "Invalid ChannelType [");
					text.append( channelType );
					text.append("] in Programmatic Configuration. Use default ChannelType [");
					text.append( DEFAULT_CONNECTION_TYPE );
					text.append( "]");
					EmaConfigError * mce( new EmaConfigError( text, OmmLoggerClient::ErrorEnum ) );
					emaConfigErrList.add( mce );
					break;
				}
			}
			else if ( channelEntry.getName() == "CompressionType" )
			{
				compressionType = channelEntry.getEnum();

				if ( compressionType > 2 )
				{
					EmaString text( "Invalid CompressionType [");
					text.append( compressionType );
					text.append("] in Programmatic Configuration. Use default CompressionType [");
					text.append( DEFAULT_COMPRESSION_TYPE );
					text.append( "] ");
					EmaConfigError * mce( new EmaConfigError( text, OmmLoggerClient::ErrorEnum ) );
					emaConfigErrList.add( mce );
				}
				else
				{
					flags |= 0x20;
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
			else if ( channelEntry.getName() == "TcpNodelay" )
			{
				tcpNodelay = channelEntry.getUInt();
				flags |= 0x80;
			}
			else if ( channelEntry.getName() == "XmlTraceToFile" )
			{
				xmlTraceToFile = channelEntry.getUInt();
				flags |= 0x200;
			}
			else if ( channelEntry.getName() == "XmlTraceToStdout" )
			{
				xmlTraceToStdout = channelEntry.getUInt();
				flags |= 0x400;
			}
			else if ( channelEntry.getName() == "XmlTraceToMultipleFiles" )
			{
				xmlTraceToMultipleFiles = channelEntry.getUInt();
				flags |= 0x800;
			}
			else if ( channelEntry.getName() == "XmlTraceWrite" )
			{
				xmlTraceWrite = channelEntry.getUInt();
				flags |= 0x1000;
			}
			else if ( channelEntry.getName() == "XmlTraceRead" )
			{
				xmlTraceRead = channelEntry.getUInt();
				flags |= 0x2000;
			}
			else if ( channelEntry.getName() == "XmlTracePing" )
			{
			    xmlTracePing = channelEntry.getUInt();
			    flags |= 0x2000000;
			}
			else if ( channelEntry.getName() == "XmlTraceHex" )
			{
			    xmlTraceHex = channelEntry.getUInt();
			    flags |= 0x4000000;
			}
			else if ( channelEntry.getName() == "MsgKeyInUpdates" )
			{
				msgKeyInUpdates = channelEntry.getUInt();
				flags |= 0x4000;
			}
			else if ( channelEntry.getName() == "ConnectionPingTimeout" )
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
				tempRelMcastCfg.setPacketTTL(channelEntry.getUInt());
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
				tempRelMcastCfg.setNdata(channelEntry.getUInt());
				mcastFlags |= 0x800;
			}
			else if ( channelEntry.getName() == "nmissing" )
			{
				tempRelMcastCfg.setNmissing(channelEntry.getUInt());
				mcastFlags |= 0x1000;
			}
			else if ( channelEntry.getName() == "nrreq" )
			{
				tempRelMcastCfg.setNrreq(channelEntry.getUInt());
				mcastFlags |= 0x2000;
			}
			else if ( channelEntry.getName() == "tdata" )
			{
				tempRelMcastCfg.setTdata(channelEntry.getUInt());
				mcastFlags |= 0x4000;
			}
			else if ( channelEntry.getName() == "trreq" )
			{
				tempRelMcastCfg.setTrreq(channelEntry.getUInt());
				mcastFlags |= 0x8000;
			}
			else if ( channelEntry.getName() == "pktPoolLimitHigh" )
			{
				tempRelMcastCfg.setPktPoolLimitHigh(channelEntry.getUInt());
				mcastFlags |= 0x10000;
			}
			else if ( channelEntry.getName() == "pktPoolLimitLow" )
			{
				tempRelMcastCfg.setPktPoolLimitLow(channelEntry.getUInt());
				mcastFlags |= 0x20000;
			}
			else if ( channelEntry.getName() == "twait" )
			{
				tempRelMcastCfg.setTwait(channelEntry.getUInt());
				mcastFlags |= 0x40000;
			}
			else if ( channelEntry.getName() == "tbchold" )
			{
				tempRelMcastCfg.setTbchold(channelEntry.getUInt());
				mcastFlags |= 0x80000;
			}
			else if ( channelEntry.getName() == "tpphold" )
			{
				tempRelMcastCfg.setTpphold(channelEntry.getUInt());
				mcastFlags |= 0x100000;
			}
			else if ( channelEntry.getName() == "userQLimit" )
			{
				tempRelMcastCfg.setUserQLimit(channelEntry.getUInt());
				mcastFlags |= 0x200000;
			}
			break;

		case DataType::IntEnum:
			if ( channelEntry.getName() == "XmlTraceMaxFileSize" )
			{
				xmlTraceMaxFileSize = channelEntry.getInt();
				flags |= 0x100;
			}
			else if ( channelEntry.getName() == "ReconnectAttemptLimit" )
			{
				reconnectAttemptLimit = channelEntry.getInt();
				flags |= 0x10000;
			}
			else if ( channelEntry.getName() == "ReconnectMinDelay" )
			{
				reconnectMinDelay = channelEntry.getInt();
				flags |= 0x20000;
			}
			else if ( channelEntry.getName() == "ReconnectMaxDelay" )
			{
				reconnectMaxDelay = channelEntry.getInt();
				flags |= 0x40000;
			}
			break;
		}
	}
										
	if ( flags & 0x10 )
	{
		if ( hostFnCalled )
		{
			channelType = RSSL_CONN_TYPE_SOCKET;
			ommConsumerActiveConfig.clearChannelSet();
		}
		unsigned int positionFound = 0;
		ChannelConfig *pCurrentChannelConfig = 0;

		try
		{
			if ( channelType == RSSL_CONN_TYPE_SOCKET )
			{
				SocketChannelConfig * socketChannelConfig = new SocketChannelConfig();
				pCurrentChannelConfig = socketChannelConfig;
				ommConsumerActiveConfig.configChannelSet.push_back(pCurrentChannelConfig);

				SocketChannelConfig *fileCfgSocket = NULL;
				if(fileCfg && fileCfg->connectionType == RSSL_CONN_TYPE_SOCKET)
					fileCfgSocket = static_cast<SocketChannelConfig*> (fileCfg);
				if ( flags & 0x80 )
					socketChannelConfig->tcpNodelay = tcpNodelay ? true : false;
				else if (fileCfgSocket)
					socketChannelConfig->tcpNodelay = fileCfgSocket->tcpNodelay;

				if ( flags & 0x02 && ! hostFnCalled )
					socketChannelConfig->hostName = host;
				else if (fileCfgSocket)
					socketChannelConfig->hostName = fileCfgSocket->hostName;

				if ( flags & 0x04 && ! hostFnCalled )
					socketChannelConfig->serviceName = port;
				else if (fileCfgSocket)
					socketChannelConfig->serviceName = fileCfgSocket->serviceName;			
				
			}
			else if ( channelType == RSSL_CONN_TYPE_RELIABLE_MCAST )
			{
				ReliableMcastChannelConfig *reliableMcastChannelCfg = new ReliableMcastChannelConfig();		
				pCurrentChannelConfig = reliableMcastChannelCfg;
				EmaString errorMsg;
				if(setReliableMcastChannelInfo(reliableMcastChannelCfg, mcastFlags, tempRelMcastCfg, errorMsg, fileCfg) )
					ommConsumerActiveConfig.configChannelSet.push_back( pCurrentChannelConfig );
				else 
				{
					throwIceException(errorMsg);
					return;
				}
			}
			else if ( channelType == RSSL_CONN_TYPE_HTTP )
			{
				HttpChannelConfig *httpChanelConfig = new HttpChannelConfig();
				pCurrentChannelConfig = httpChanelConfig;
				ommConsumerActiveConfig.configChannelSet.push_back(pCurrentChannelConfig);

				HttpChannelConfig *fileCfgHttp = NULL;
				if(fileCfg && fileCfg->connectionType == RSSL_CONN_TYPE_HTTP)
					fileCfgHttp = static_cast<HttpChannelConfig*> (fileCfg);

				if ( flags & 0x80 )
					httpChanelConfig->tcpNodelay = tcpNodelay ? true : false;
				else if (fileCfgHttp)
					httpChanelConfig->tcpNodelay = fileCfgHttp->tcpNodelay;

				if ( flags & 0x02 && ! hostFnCalled )
					httpChanelConfig->hostName = host;
				else if (fileCfgHttp)
					httpChanelConfig->hostName = fileCfgHttp->hostName;

				if ( flags & 0x04 && ! hostFnCalled )
					httpChanelConfig->serviceName = port;
				else if (fileCfgHttp)
					httpChanelConfig->serviceName = fileCfgHttp->serviceName;			

				if( flags & 0x100000)
					httpChanelConfig->objectName = objectName;			
				else if (fileCfgHttp)
					httpChanelConfig->objectName = fileCfgHttp->objectName;			
				
			}
			else if ( channelType == RSSL_CONN_TYPE_ENCRYPTED )
			{
				EncryptedChannelConfig* encryptChanelConfig = new EncryptedChannelConfig();										
				pCurrentChannelConfig = encryptChanelConfig;
				ommConsumerActiveConfig.configChannelSet.push_back(pCurrentChannelConfig);

				EncryptedChannelConfig *fileCfgEncrypt = NULL;
				if(fileCfg && fileCfg->connectionType == RSSL_CONN_TYPE_ENCRYPTED )
					fileCfgEncrypt = static_cast<EncryptedChannelConfig*> (fileCfg);

				if ( flags & 0x80 )
					encryptChanelConfig->tcpNodelay = tcpNodelay ? true : false;
				else if (fileCfgEncrypt)
					encryptChanelConfig->tcpNodelay = fileCfgEncrypt->tcpNodelay;

				if ( flags & 0x02 && ! hostFnCalled )
					encryptChanelConfig->hostName = host;
				else if (fileCfgEncrypt)
					encryptChanelConfig->hostName = fileCfgEncrypt->hostName;

				if ( flags & 0x04 && ! hostFnCalled )
					encryptChanelConfig->serviceName = port;
				else if (fileCfgEncrypt)
					encryptChanelConfig->serviceName = fileCfgEncrypt->serviceName;			

				if( flags & 0x100000)
					encryptChanelConfig->objectName = objectName;			
				else if (fileCfgEncrypt)
					encryptChanelConfig->objectName = fileCfgEncrypt->objectName;		
			}
		} 
		catch ( std::bad_alloc ) 
		{
			const char* temp = "Failed to allocate memory for ChannelConfig. Out of memory!";
			throwMeeException( temp );
		}

		pCurrentChannelConfig->name = channelName;

		bool useFileCfg = ( fileCfg && fileCfg->connectionType == pCurrentChannelConfig->connectionType )? true : false;

		if ( flags & 0x80000 )
			pCurrentChannelConfig->interfaceName = interfaceName;
		else if( useFileCfg )
			pCurrentChannelConfig->interfaceName = fileCfg->interfaceName;

		if(channelType != RSSL_CONN_TYPE_RELIABLE_MCAST)
		{
			if ( flags & 0x20 )
				pCurrentChannelConfig->compressionType = (RsslCompTypes)compressionType;
			else if( useFileCfg )
				pCurrentChannelConfig->compressionType = fileCfg->compressionType;

			if ( flags & 0x1000000 )
				pCurrentChannelConfig->compressionThreshold = compressionThreshold > maxUInt32 ? maxUInt32 : (UInt32)compressionThreshold;
			else if( useFileCfg )
				pCurrentChannelConfig->compressionThreshold = fileCfg->compressionThreshold;
		}

		if ( flags & 0x40 )
			pCurrentChannelConfig->setGuaranteedOutputBuffers(guaranteedOutputBuffers);
		else if( useFileCfg )
			pCurrentChannelConfig->guaranteedOutputBuffers = fileCfg->guaranteedOutputBuffers;

		if ( flags & 0x800000 )
			pCurrentChannelConfig->setNumInputBuffers(numInputBuffers);
		else if( useFileCfg )
			pCurrentChannelConfig->numInputBuffers = fileCfg->numInputBuffers;

		if ( flags & 0x200000 )
			pCurrentChannelConfig->sysRecvBufSize = sysRecvBufSize > maxUInt32 ? maxUInt32 : (UInt32)sysRecvBufSize;
		else if( useFileCfg )
			pCurrentChannelConfig->sysRecvBufSize = fileCfg->sysRecvBufSize;

		if ( flags & 0x400000 )
			pCurrentChannelConfig->sysSendBufSize = sysSendBufSize > maxUInt32 ? maxUInt32 : (UInt32)sysSendBufSize;
		else if( useFileCfg )
			pCurrentChannelConfig->sysSendBufSize = fileCfg->sysSendBufSize;

		if ( flags & 0x8000 )
			pCurrentChannelConfig->connectionPingTimeout = connectionPingTimeout > maxUInt32  ? maxUInt32 : (UInt32)connectionPingTimeout;
		else if( useFileCfg )
			pCurrentChannelConfig->connectionPingTimeout = fileCfg->connectionPingTimeout;

		if ( flags & 0x10000 )
			pCurrentChannelConfig->setReconnectAttemptLimit( reconnectAttemptLimit );
		else if( useFileCfg )
			pCurrentChannelConfig->reconnectAttemptLimit = fileCfg->reconnectAttemptLimit;

		if ( flags & 0x20000 )
			pCurrentChannelConfig->setReconnectMinDelay( reconnectMinDelay );
		else if( useFileCfg )
			pCurrentChannelConfig->reconnectMinDelay = fileCfg->reconnectMinDelay;

		if ( flags & 0x40000 )
			pCurrentChannelConfig->setReconnectMaxDelay( reconnectMaxDelay );
		else if( useFileCfg )
			pCurrentChannelConfig->reconnectMaxDelay = fileCfg->reconnectMaxDelay;

		if ( flags & 0x08 )
			pCurrentChannelConfig->xmlTraceFileName = xmlTraceFileName;
		else if( useFileCfg )
			pCurrentChannelConfig->xmlTraceFileName = fileCfg->xmlTraceFileName;

		if ( flags & 0x100 )
			pCurrentChannelConfig->xmlTraceMaxFileSize = xmlTraceMaxFileSize;
		else if( useFileCfg )
			pCurrentChannelConfig->xmlTraceMaxFileSize = fileCfg->xmlTraceMaxFileSize;

		if ( flags & 0x200 )
			pCurrentChannelConfig->xmlTraceToFile = xmlTraceToFile ? true : false;
		else if( useFileCfg )
			pCurrentChannelConfig->xmlTraceToFile = fileCfg->xmlTraceToFile;

		if ( flags & 0x400 )
			pCurrentChannelConfig->xmlTraceToStdout = xmlTraceToStdout ? true : false;
		else if( useFileCfg )
			pCurrentChannelConfig->xmlTraceToStdout = fileCfg->xmlTraceToStdout;

		if ( flags & 0x800 )
			pCurrentChannelConfig->xmlTraceToMultipleFiles = xmlTraceToMultipleFiles ? true : false;
		else if( useFileCfg )
			pCurrentChannelConfig->xmlTraceToMultipleFiles = fileCfg->xmlTraceToMultipleFiles;

		if ( flags & 0x1000 )
			pCurrentChannelConfig->xmlTraceWrite = xmlTraceWrite ? true : false;
		else if( useFileCfg )
			pCurrentChannelConfig->xmlTraceWrite = fileCfg->xmlTraceWrite;

		if ( flags & 0x2000 )
			pCurrentChannelConfig->xmlTraceRead = xmlTraceRead ? true : false;
		else if( useFileCfg )
			pCurrentChannelConfig->xmlTraceRead = fileCfg->xmlTraceRead;

		if ( flags & 0x2000000 )
			pCurrentChannelConfig->xmlTracePing = xmlTracePing ? true : false;
		else if( useFileCfg )
			pCurrentChannelConfig->xmlTracePing = fileCfg->xmlTracePing;

		if ( flags & 0x4000000 )
			pCurrentChannelConfig->xmlTraceHex = xmlTraceHex ? true : false;
		else if( useFileCfg )
			pCurrentChannelConfig->xmlTraceHex = fileCfg->xmlTraceHex;

		if ( flags & 0x4000 )
			pCurrentChannelConfig->msgKeyInUpdates = msgKeyInUpdates ? true : false;
		else if( useFileCfg )
			pCurrentChannelConfig->msgKeyInUpdates = fileCfg->msgKeyInUpdates;
	}
}

bool ProgrammaticConfigure::setReliableMcastChannelInfo( ReliableMcastChannelConfig *pChannelCfg, UInt64& mcastFlags, ReliableMcastChannelConfig &relMcastcfg, EmaString& errorText, ChannelConfig *fileCfg)
{
	bool bValid = true;

	ReliableMcastChannelConfig *pFileMcastCfg =  NULL;
	bool useFileCfg = ( fileCfg && fileCfg->connectionType == relMcastcfg.connectionType )? true : false;
	if( useFileCfg )
		pFileMcastCfg =  static_cast<ReliableMcastChannelConfig *> (fileCfg);

	if( mcastFlags & 0x01 )
		pChannelCfg->recvAddress = relMcastcfg.recvAddress;
	else if( useFileCfg )
		pChannelCfg->recvAddress = pFileMcastCfg->recvAddress;		

	if( mcastFlags & 0x02 )
		pChannelCfg->recvServiceName = relMcastcfg.recvServiceName;
	else if( useFileCfg )
		pChannelCfg->recvServiceName = pFileMcastCfg->recvServiceName;

	if( mcastFlags & 0x04 )
		pChannelCfg->sendAddress = relMcastcfg.sendAddress;
	else if( useFileCfg )
		pChannelCfg->sendAddress = pFileMcastCfg->sendAddress;

	if( mcastFlags & 0x08 )
		pChannelCfg->sendServiceName = relMcastcfg.sendServiceName;
	else if( useFileCfg )
		pChannelCfg->sendServiceName = pFileMcastCfg->sendServiceName;

	if( mcastFlags & 0x10 )
		pChannelCfg->unicastServiceName = relMcastcfg.unicastServiceName;
	else if( useFileCfg )
		pChannelCfg->unicastServiceName = pFileMcastCfg->unicastServiceName;

	if( mcastFlags & 0x20 )
		pChannelCfg->hsmInterface = relMcastcfg.hsmInterface;
	else if( useFileCfg )
		pChannelCfg->hsmInterface = pFileMcastCfg->hsmInterface;
	if( mcastFlags & 0x40 )
		pChannelCfg->hsmMultAddress = relMcastcfg.hsmMultAddress;
	else if( useFileCfg )
		pChannelCfg->hsmMultAddress = pFileMcastCfg->hsmMultAddress;
	if( mcastFlags & 0x80 )
		pChannelCfg->hsmPort = relMcastcfg.hsmPort;
	else if( useFileCfg )
		pChannelCfg->hsmPort = pFileMcastCfg->hsmPort;
	if( mcastFlags & 0x100 )
		pChannelCfg->packetTTL = relMcastcfg.packetTTL;
	else if( useFileCfg )
		pChannelCfg->packetTTL = pFileMcastCfg->packetTTL;
	if( mcastFlags & 0x200 )
		pChannelCfg->disconnectOnGap = relMcastcfg.disconnectOnGap;
	else if( useFileCfg )
		pChannelCfg->disconnectOnGap = pFileMcastCfg->disconnectOnGap;
	if( mcastFlags & 0x400 )
		pChannelCfg->hsmInterval = relMcastcfg.hsmInterval;
	else if( useFileCfg )
		pChannelCfg->hsmInterval = pFileMcastCfg->hsmInterval;
	if( mcastFlags & 0x800 )
		pChannelCfg->ndata = relMcastcfg.ndata;
	else if( useFileCfg )
		pChannelCfg->ndata = pFileMcastCfg->ndata;
	if( mcastFlags & 0x1000 )
		pChannelCfg->nmissing = relMcastcfg.nmissing;
	else if( useFileCfg )
		pChannelCfg->nmissing = pFileMcastCfg->nmissing;
	if( mcastFlags & 0x2000 )
		pChannelCfg->nrreq = relMcastcfg.nrreq;
	else if( useFileCfg )
		pChannelCfg->nrreq = pFileMcastCfg->nrreq;
	if( mcastFlags & 0x4000 )
		pChannelCfg->tdata = relMcastcfg.tdata;
	else if( useFileCfg )
		pChannelCfg->tdata = pFileMcastCfg->tdata;
	if( mcastFlags & 0x8000 )
		pChannelCfg->trreq = relMcastcfg.trreq;
	else if( useFileCfg )
		pChannelCfg->trreq = pFileMcastCfg->trreq;
	if( mcastFlags & 0x10000 )
		pChannelCfg->pktPoolLimitHigh = relMcastcfg.pktPoolLimitHigh;
	else if( useFileCfg )
		pChannelCfg->pktPoolLimitHigh = pFileMcastCfg->pktPoolLimitHigh;
	if( mcastFlags & 0x20000 )
		pChannelCfg->pktPoolLimitLow = relMcastcfg.pktPoolLimitLow;
	else if( useFileCfg )
		pChannelCfg->pktPoolLimitLow = pFileMcastCfg->pktPoolLimitLow;
	if( mcastFlags & 0x40000 )
		pChannelCfg->twait = relMcastcfg.twait;
	else if( useFileCfg )
		pChannelCfg->twait = pFileMcastCfg->twait;
	if( mcastFlags & 0x80000 )
		pChannelCfg->tbchold = relMcastcfg.tbchold;
	else if( useFileCfg )
		pChannelCfg->tbchold = pFileMcastCfg->tbchold;
	if( mcastFlags & 0x100000 )
		pChannelCfg->tpphold = relMcastcfg.tpphold;
	else if( useFileCfg )
		pChannelCfg->tpphold = pFileMcastCfg->tpphold;
	if( mcastFlags & 0x200000 )
		pChannelCfg->userQLimit = relMcastcfg.userQLimit;
	else if( useFileCfg )
		pChannelCfg->userQLimit = pFileMcastCfg->userQLimit;
	if( mcastFlags & 0x400000 )
		pChannelCfg->tcpControlPort = relMcastcfg.tcpControlPort;
	else if( useFileCfg )
		pChannelCfg->tcpControlPort = pFileMcastCfg->tcpControlPort;

	return bValid;
}

void ProgrammaticConfigure::retrieveLogger( const Map& map, const EmaString& loggerName, EmaConfigErrorList& emaConfigErrList,
										  OmmConsumerActiveConfig& ommConsumerActiveConfig )
{
	map.reset();
	while ( map.forth() )
	{
		const MapEntry & mapEntry = map.getEntry();

		if ( mapEntry.getKey().getDataType() == DataType::AsciiEnum && mapEntry.getKey().getAscii() == "LoggerGroup" )
		{
			if ( mapEntry.getLoadType() == DataType::ElementListEnum )
			{
				const ElementList & elementList = mapEntry.getElementList();

				while ( elementList.forth() )
				{
					const ElementEntry & elementEntry = elementList.getEntry();

					if ( elementEntry.getLoadType() == DataType::MapEnum )
					{
						if ( elementEntry.getName() == "LoggerList" )
						{
							const Map & map = elementEntry.getMap();

							while ( map.forth() )
							{
								const MapEntry & mapEntry = map.getEntry();

								if ( ( mapEntry.getKey().getDataType() == DataType::AsciiEnum ) && ( mapEntry.getKey().getAscii() == loggerName ) )
								{
									if ( mapEntry.getLoadType() == DataType::ElementListEnum )
									{
										const ElementList & elementListLogger = mapEntry.getElementList();

										while ( elementListLogger.forth() )
										{
											const ElementEntry & loggerEntry = elementListLogger.getEntry();

											switch ( loggerEntry.getLoadType() )
											{
											case DataType::AsciiEnum:
												if ( loggerEntry.getName() == "FileName" )
												{
													ommConsumerActiveConfig.loggerConfig.loggerFileName = loggerEntry.getAscii();
												}
												break;

											case DataType::EnumEnum:
												if ( loggerEntry.getName() == "LoggerType" )
												{
													UInt16 loggerType = loggerEntry.getEnum();

													if ( loggerType > 1 )
													{
														EmaString text( "Invalid LoggerType [");
														text.append( loggerType );
														text.append("] in Programmatic Configuration. Use default LoggerType [");
														text.append( DEFAULT_LOGGER_TYPE );
														text.append( "]" );
														EmaConfigError * mce( new EmaConfigError( text, OmmLoggerClient::ErrorEnum ) );
														emaConfigErrList.add( mce );
													}
													else
													{
														ommConsumerActiveConfig.loggerConfig.loggerType = (OmmLoggerClient::LoggerType)loggerType;
													}
												}
												else if ( loggerEntry.getName() == "LoggerSeverity" )
												{
													UInt16 severityType = loggerEntry.getEnum();

													if ( severityType > 4 )
													{
														EmaString text( "Invalid LoggerSeverity [");
														text.append( severityType );
														text.append("] in Programmatic Configuration. Use default LoggerSeverity [");
														text.append( DEFAULT_LOGGER_SEVERITY );
														text.append( "]" );
														EmaConfigError * mce( new EmaConfigError( text, OmmLoggerClient::ErrorEnum ) );
														emaConfigErrList.add( mce );
													}
													else
													{
														ommConsumerActiveConfig.loggerConfig.minLoggerSeverity = (OmmLoggerClient::Severity)severityType;
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
											  OmmConsumerActiveConfig& ommConsumerActiveConfig )
{
	map.reset();
	while ( map.forth() )
	{
		const MapEntry & mapEntry = map.getEntry();

		if ( mapEntry.getKey().getDataType() == DataType::AsciiEnum && mapEntry.getKey().getAscii() == "DictionaryGroup" )
		{
			if ( mapEntry.getLoadType() == DataType::ElementListEnum )
			{
				const ElementList & elementList = mapEntry.getElementList();

				while ( elementList.forth() )
				{
					const ElementEntry & elementEntry = elementList.getEntry();

					if ( elementEntry.getLoadType() == DataType::MapEnum )
					{
						if ( elementEntry.getName() == "DictionaryList" )
						{
							const Map & map = elementEntry.getMap();

							while ( map.forth() )
							{
								const MapEntry & mapEntry = map.getEntry();

								if ( ( mapEntry.getKey().getDataType() == DataType::AsciiEnum ) && ( mapEntry.getKey().getAscii() == dictionaryName ) )
								{
									if ( mapEntry.getLoadType() == DataType::ElementListEnum )
									{
										const ElementList & elementListDictionary = mapEntry.getElementList();

										while ( elementListDictionary.forth() )
										{
											const ElementEntry & loggerEntry = elementListDictionary.getEntry();

											switch ( loggerEntry.getLoadType() )
											{
											case DataType::AsciiEnum:

												if ( loggerEntry.getName() == "RdmFieldDictionaryFileName" )
												{
													ommConsumerActiveConfig.dictionaryConfig.rdmfieldDictionaryFileName = loggerEntry.getAscii();
												}
												else if ( loggerEntry.getName() == "EnumTypeDefFileName" )
												{
													ommConsumerActiveConfig.dictionaryConfig.enumtypeDefFileName = loggerEntry.getAscii();
												}
												break;

											case DataType::EnumEnum:

												if ( loggerEntry.getName() == "DictionaryType" )
												{
													UInt16 dictionaryType = loggerEntry.getEnum();

													if ( dictionaryType > 1 )
													{
														EmaString text( "Invalid DictionaryType [");
														text.append( dictionaryType );
														text.append("] in Programmatic Configuration. Use default DictionaryType [");
														text.append( DEFAULT_DICTIONARY_TYPE );
														text.append( "]" );
														EmaConfigError * mce( new EmaConfigError( text, OmmLoggerClient::ErrorEnum ) );
														emaConfigErrList.add( mce );
													}
													else
													{
														ommConsumerActiveConfig.dictionaryConfig.dictionaryType = (Dictionary::DictionaryType)dictionaryType;
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
