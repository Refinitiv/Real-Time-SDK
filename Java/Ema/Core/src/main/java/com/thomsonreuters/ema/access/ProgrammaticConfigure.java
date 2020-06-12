///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import com.thomsonreuters.ema.access.DataType.DataTypes;
import com.thomsonreuters.ema.access.DirectoryServiceStore.ServiceIdInteger;
import com.thomsonreuters.ema.access.OmmLoggerClient.Severity;
import com.thomsonreuters.ema.access.OmmState.StreamState;
import com.thomsonreuters.upa.codec.Codec;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.Qos;
import com.thomsonreuters.upa.codec.QosRates;
import com.thomsonreuters.upa.codec.QosTimeliness;
import com.thomsonreuters.upa.transport.CompressionTypes;
import com.thomsonreuters.upa.transport.ConnectionTypes;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryMsgFactory;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.Service;

class ProgrammaticConfigure
{
	/** @class InstanceEntryFlag
	An enumeration representing consumer or provider entry level config variables.
	*/
	class InstanceEntryFlag
	{
		final static int CHANNEL_FLAG =			0x001;
		final static int LOGGER_FLAG =			0x002;
		final static int DICTIONARY_FLAG =		0x004;
		final static int CHANNELSET_FLAG =		0x008;
		final static int DIRECTORY_FLAG =		0x010;
		final static int SERVER_FLAG =			0x020;
	}
	
	/** @class ChannelEntryFlag
	An enumeration representing channel entry level config variables.
	*/
	class ChannelEntryFlag
	{
		final static int CHANNELTYPE_FLAG =						0x001;
		final static int HOST_FLAG =							0x002;
		final static int PORT_FLAG =							0x004;
		final static int INTERFACENAME_FLAG =					0x008;
		final static int GUARANTEED_OUTPUTBUFFERS_FLAG =		0x010;
		final static int NUM_INPUTBUFFERS_FLAG =				0x020;
		final static int SYS_RECV_BUFSIZE_FLAG =				0x040;
		final static int SYS_SEND_BUFSIZE_FLAG =				0x080;
		final static int HIGH_WATERMARK_FLAG =					0x100;
		final static int TCP_NODELAY_FLAG =						0x200;
		final static int CONN_PING_TIMEOUT_FLAG =				0x400;
		final static int COMPRESSION_THRESHOLD_FLAG =			0x800;
		final static int COMPRESSION_TYPE_FLAG =				0x1000;
		final static int DIRECTWRITE_FLAG =						0x2000;
		final static int INIT_TIMEOUT_FLAG =					0x4000;
		final static int ENABLE_SESSION_MGNT_FLAG =				0x8000; // Enable the reactor to refresh the token and reissue login request.
		final static int LOCATION_FLAG = 						0x10000; // Specify a location to get an endpoint for establishing a connection.
	}
	
	/** @class ServerEntryFlag
	An enumeration representing server entry level config variables.
	*/
	class ServerEntryFlag
	{
		final static int SERVERTYPE_FLAG =					0X0001;
		final static int PORT_FLAG =						0X0002;
		final static int INTERFACENAME_FLAG =				0X0004;
		final static int GUARANTEED_OUTPUTBUFFERS_FLAG =	0X0008;
		final static int NUMINPUTBUF_FLAG =					0X0010;
		final static int SYS_RECV_BUFSIZE_FLAG =			0X0020;
		final static int SYS_SEND_BUFSIZE_FLAG =			0X0040;
		final static int HIGH_WATERMARK_FLAG =				0X0080;
		final static int TCP_NODELAY_FLAG =					0X0100;
		final static int CONN_MIN_PING_TIMEOUT_FLAG =		0X0200;
		final static int CONN_PING_TIMEOUT_FLAG =			0X0400;
		final static int COMPRESSION_THRESHOLD_FLAG =		0X0800;
		final static int COMPRESSION_TYPE_FLAG =			0x1000;
		final static int DIRECTWRITE_FLAG =					0x2000;
		final static int INIT_TIMEOUT_FLAG =				0x4000;
	}

	/** @class TunnelingEntryFlag
	An enumeration representing tunneling entry level config variables.
	*/
	class TunnelingEntryFlag
	{
		final static int OBJECTNAME_FLAG =			0x001;
		final static int PROXYPORT_FLAG =			0x002;
		final static int PROXYHOST_FLAG =			0x004;
		final static int SECURITY_PROTOCOL_FLAG =	0x008;
	}
	
	final static int MAX_UNSIGNED_INT16	= 0xFFFF;
	
	String _group;
	String _list;
	boolean _setGroup; 
	private String	_consumerName;
	private String	_niProviderName;
	private String	_iProviderName;
	private String	_channelName;
	private String	_serverName;
	private String	_dictionaryName;
	private String	_directoryName;
	private String	_channelSet;
	
	private	boolean		_overrideConsName;
	private	boolean		_overrideNiProvName;
	private	boolean		_overrideIProvName;
	private	boolean		_dependencyNamesLoaded;
	private	int			_nameflags;
	private	List<DictionaryConfig> _serverDictList;
	private	List<Map> _configList;
	private	ConfigErrorTracker	_emaConfigErrList;
	private	List<String> _dictProvided;
	private	List<String> _dictUsed;
	private	List<String> _serviceNameList;
	boolean addQos = false;
	private EmaObjectManager _objManager;
	private int INVALID_RETVAL = -2;

	ProgrammaticConfigure(  Map map, ConfigErrorTracker emaConfigErrList )
	{
		_nameflags = 0 ;
		_emaConfigErrList = emaConfigErrList;
		_configList = new ArrayList<Map>();
		_dictProvided = new ArrayList<String>();
		_dictUsed = new ArrayList<String>();
		_serviceNameList = new ArrayList<String>();
		_serverDictList = new ArrayList<DictionaryConfig>();
		_objManager = new EmaObjectManager();
		
		Map mapDecoded = new MapImpl(_objManager);
		JUnitTestConnect.setRsslData(mapDecoded, map, Codec.majorVersion(), Codec.minorVersion(), null, null);
		_configList.add(mapDecoded);
	}
	
	void addConfigure(  Map map )
	{
		Map mapDecoded = new MapImpl(_objManager);
		JUnitTestConnect.setRsslData(mapDecoded, map, Codec.majorVersion(), Codec.minorVersion(), null, null);
		_configList.add(mapDecoded);
	}
	
	void clear()
	{
		internalClear();
		_configList.clear();
	}

	void internalClear()
	{
		_consumerName = null;
		_niProviderName = null;
		_iProviderName = null;
		_channelName = null;
		_serverName = null;
		_dictionaryName = null;
		_directoryName = null;
		_channelSet = null;
		
		_overrideConsName = false;
		_overrideNiProvName = false;
		_overrideIProvName = false;
		_dependencyNamesLoaded = false;
		_nameflags = 0;

		_group = null;
		_list = null;
		_setGroup = false; 
		addQos = false;
		_serverDictList.clear();
		_dictProvided.clear();
		_dictUsed.clear();
		_serviceNameList.clear();
	}
	
	String defaultConsumer()
	{
		String retValue;

		if ( _overrideConsName )
			return _consumerName;
		else
		{
			internalClear();

			 for (Map map : _configList)
			 {
				 if ( (retValue = retrieveDefaultConsProvName( map, "ConsumerGroup", "DefaultConsumer" )) != null )
					 return retValue;
			 }
		}

		return null;
	}

	String defaultNiProvider()
	{
		String retValue;
		
		if ( _overrideNiProvName )
			return _niProviderName;
		else
		{
			internalClear();

			 for (Map map : _configList)
			 {
				 if ( (retValue = retrieveDefaultConsProvName( map, "NiProviderGroup", "DefaultNiProvider" )) != null )
					 return retValue;
			 }
		}

		return null;
	}

	String defaultIProvider()
	{
		String retValue;
		
		if ( _overrideIProvName )
			return _iProviderName;
		else
		{
			internalClear();

			 for (Map map : _configList)
			 {
				 if ( (retValue = retrieveDefaultConsProvName( map, "IProviderGroup", "DefaultIProvider" )) != null )
					 return retValue;
			 }
		}

		return null;
	}

	boolean specifyConsumerName(  String consumerName )
	{
		for (Map map : _configList)
		{
			if ( validateConsumerProviderName( map, "ConsumerGroup", "ConsumerList", consumerName ) )
			{
				_overrideConsName = true;
				_consumerName = consumerName;
				return true;
			}
		}

		return false;
	}

	boolean specifyNiProviderName(  String niProviderName )
	{ 
		for (Map map : _configList)
		{
			if ( validateConsumerProviderName( map, "NiProviderGroup", "NiProviderList", niProviderName ) )
			{
				_overrideNiProvName = true;
				_niProviderName = niProviderName;
				return true;
			}
		}

		return false;
	}

	boolean specifyIProviderName(  String iProviderName )
	{
		for (Map map : _configList)
		{
			if ( validateConsumerProviderName( map, "IProviderGroup", "IProviderList", iProviderName ) )
			{
				_overrideIProvName = true;
				_iProviderName = iProviderName;
				return true;
			}
		}

		return false;
	}

	String activeEntryNames( String instanceName, int flag )
	{
		if ( !_dependencyNamesLoaded )
		{
			 for (Map map : _configList)
				retrieveDependencyNames(map, instanceName );

			_dependencyNamesLoaded = true;
		}

		if ((_nameflags & flag) != 0) 
		{
			if ( (InstanceEntryFlag.CHANNELSET_FLAG & flag) != 0 )
			{
				return _channelSet;
			}
			else if ( (InstanceEntryFlag.SERVER_FLAG & flag) != 0 )
			{
				return _serverName;
			}
			else if ( (InstanceEntryFlag.CHANNEL_FLAG & flag) != 0 )
			{
				return _channelName;
			}
			else if ( (InstanceEntryFlag.DICTIONARY_FLAG & flag) != 0 )
			{
				return _dictionaryName;
			}
			else if ( (InstanceEntryFlag.DIRECTORY_FLAG & flag) != 0 )
			{
				return _directoryName;
			}
		}
		
		return null;
	}

	String retrieveDefaultConsProvName( Map map, String group, String defaulName )
	{
		for (MapEntry mapEntry : map)
		{
			if ( ( mapEntry.key().dataType() == DataTypes.ASCII ) && ( mapEntry.key().ascii().ascii().equals(group) ) &&
			      ( mapEntry.load().dataType() == DataTypes.ELEMENT_LIST ) )
			{
				ElementList elementList = mapEntry.elementList();
	
				for (ElementEntry elementEntry : elementList)
				{
					if ( elementEntry.loadType() == DataTypes.ASCII )
					{
						if ( elementEntry.name().equals( defaulName) )
						{
							return elementEntry.ascii().ascii();
						}
					}
				}
			}
		}
	
		return null;
	}
	
	boolean validateConsumerProviderName( Map map, String group, String listName, String conProvName )
	{
		for (MapEntry mapEntry : map)
		{
			if ( ( mapEntry.key().dataType() == DataTypes.ASCII ) && ( mapEntry.key().ascii().ascii().equals(group) ) &&
			      ( mapEntry.load().dataType() == DataTypes.ELEMENT_LIST ) )
			{
				ElementList elementList = mapEntry.elementList();
	
				for (ElementEntry elementEntry : elementList)
				{
					if ( ( elementEntry.name().equals( listName) ) && ( elementEntry.load().dataType() == DataTypes.MAP ) )
					{
						Map consumerMap = elementEntry.map();
						for (MapEntry consumerMapEntry : consumerMap)
						{
							if ( ( consumerMapEntry.key().dataType() == DataTypes.ASCII ) && ( consumerMapEntry.key().ascii().ascii().equals(conProvName)) )
								return true;
						}
					}
				}
			}
		}
	
		return false;
	}
	
	void retrieveDependencyNames(  Map map,  String userName )
	{
		int position = 0;
		int channelPos = 0, channelSetPos = 0;

		retrieveGroupAndListName( map );

		if ( _group != null && _group.isEmpty() )
			return;

		for (MapEntry mapEntry : map)
		{
			if ( mapEntry.key().dataType() == DataTypes.ASCII && mapEntry.key().ascii().ascii().equals(_group) )
			{
				if ( mapEntry.loadType() == DataTypes.ELEMENT_LIST )
				{
					ElementList elementList = mapEntry.elementList();

					for (ElementEntry elementEntry : elementList)
					{
						if ( elementEntry.loadType() == DataTypes.MAP )
						{
							if ( elementEntry.name().equals(_list) && ( elementEntry.load().dataType() == DataTypes.MAP ) )
							{
								 Map mapList = elementEntry.map();

								for (MapEntry mapListEntry : mapList)
								{
									if ( mapListEntry.key().dataType() == DataTypes.ASCII &&
										 mapListEntry.key().ascii().ascii().equals(userName) &&
										 mapEntry.loadType() == DataTypes.ELEMENT_LIST )
									{
										position = 0;
										for (ElementEntry instanceEntry : mapListEntry.elementList())
										{
											position++;
											switch ( instanceEntry.loadType() )
											{
												case DataTypes.ASCII:
													if ( instanceEntry.name().equals("Channel") )
													{
														_channelName = instanceEntry.ascii().ascii();
														_nameflags |= InstanceEntryFlag.CHANNEL_FLAG;
														channelPos = position;
													}
													if (instanceEntry.name().equals("Server"))
													{
														_serverName = instanceEntry.ascii().ascii();
														_nameflags |= InstanceEntryFlag.SERVER_FLAG;
													}
													else if ( instanceEntry.name().equals("Dictionary") )
													{
														_dictionaryName = instanceEntry.ascii().ascii();
														_nameflags |= InstanceEntryFlag.DICTIONARY_FLAG;
													}
													else if ( instanceEntry.name().equals("ChannelSet") )
													{
														_channelSet = instanceEntry.ascii().ascii();
														_nameflags |= InstanceEntryFlag.CHANNELSET_FLAG;
														channelSetPos = position;
													}
													else if ( instanceEntry.name().equals("Directory") )
													{
														_directoryName = instanceEntry.ascii().ascii();
														_nameflags |= InstanceEntryFlag.DIRECTORY_FLAG;
													}
													break;
												default:
													break;
											}
										}
										
										if ( ((_nameflags & InstanceEntryFlag.CHANNEL_FLAG ) != 0) &&
											((_nameflags & InstanceEntryFlag.CHANNELSET_FLAG) != 0))
										{
											if ( channelSetPos > channelPos )
											{
												_nameflags &= ~InstanceEntryFlag.CHANNEL_FLAG;
												_channelName = null;
											}
											else
											{
												_nameflags &= ~InstanceEntryFlag.CHANNELSET_FLAG;
												_channelSet = null;
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
	
	void  retrieveCommonConfig( String instanceName, BaseConfig activeConfig )
	{
		 for (Map map : _configList)
			retrieveInstanceCommonConfig(map, instanceName, activeConfig );
	}
	
	void  retrieveCustomConfig( String instanceName, BaseConfig activeConfig )
	{
		 for (Map map : _configList)
			retrieveInstanceCustomConfig(map, instanceName, activeConfig );
	}
	
	int  retrieveChannelTypeConfig(String channelName)
	{
		for (Map map : _configList)
		{
			for (MapEntry mapEntry : map)
			{
				if ( mapEntry.key().dataType() == DataTypes.ASCII &&
					 mapEntry.key().ascii().ascii().equals("ChannelGroup") &&
					 mapEntry.loadType() == DataTypes.ELEMENT_LIST )
				{
					for (ElementEntry elementEntry : mapEntry.elementList())
					{
						if ( elementEntry.loadType() == DataTypes.MAP && elementEntry.name().equals("ChannelList"))
						{
							for (MapEntry mapListEntry : elementEntry.map())
							{
								if ( mapListEntry.key().dataType() == DataTypes.ASCII  &&
									mapListEntry.key().ascii().ascii().equals(channelName) &&
									mapListEntry.loadType() == DataTypes.ELEMENT_LIST )
								{
									for (ElementEntry channelEntry : mapListEntry.elementList())
									{
										if ( channelEntry.loadType() == DataTypes.ASCII && channelEntry.name().equals("ChannelType"))
											return convertToEnum(channelEntry.ascii().ascii());
									}
								}
							}
						}
					}
				}
			}
		}
		return INVALID_RETVAL;
	}
	
	void  retrieveChannelConfig( String channelName,  ActiveConfig activeConfig, int hostFnCalled, ChannelConfig fileCfg)
	{
		 for (Map map : _configList)
			retrieveChannel(map, channelName, activeConfig, hostFnCalled, fileCfg);
	}
	
	void  retrieveServerConfig(String serverName, ActiveServerConfig activeServerConfig, int portFnCalled, ServerConfig fileCfg)
	{
		 for (Map map : _configList)
			retrieveServer(map, serverName, activeServerConfig, portFnCalled, fileCfg);
	}

	GlobalConfig retrieveGlobalConfig() {
		MapEntry globalConfigEntry = getElementListByNameFromConfigList(_configList, "GlobalConfig");
		if (globalConfigEntry == null) {
			return null;
		}
		GlobalConfig config = new GlobalConfig();
		ElementEntry reactorMsgEventPoolLimit = getIntElementEntry(globalConfigEntry, "ReactorMsgEventPoolLimit");
		ElementEntry reactorChannelEventPoolLimit = getIntElementEntry(globalConfigEntry, "ReactorChannelEventPoolLimit");
		ElementEntry workerEventPoolLimit = getIntElementEntry(globalConfigEntry, "WorkerEventPoolLimit");
		ElementEntry tunnelStreamMsgEventPoolLimit = getIntElementEntry(globalConfigEntry, "TunnelStreamMsgEventPoolLimit");
		ElementEntry tunnelStreamStatusEventPoolLimit = getIntElementEntry(globalConfigEntry, "TunnelStreamStatusEventPoolLimit");

		if (reactorMsgEventPoolLimit != null) {
			config.reactorMsgEventPoolLimit = convertToInt(reactorMsgEventPoolLimit.intValue());
		}
		if (reactorChannelEventPoolLimit != null) {
			config.reactorChannelEventPoolLimit = convertToInt(reactorChannelEventPoolLimit.intValue());
		}
		if (workerEventPoolLimit != null) {
			config.workerEventPoolLimit = convertToInt(workerEventPoolLimit.intValue());
		}
		if (tunnelStreamMsgEventPoolLimit != null) {
			config.tunnelStreamMsgEventPoolLimit = convertToInt(tunnelStreamMsgEventPoolLimit.intValue());
		}
		if (tunnelStreamStatusEventPoolLimit != null) {
			config.tunnelStreamStatusEventPoolLimit = convertToInt(tunnelStreamStatusEventPoolLimit.intValue());
		}
		return config;
	}
	void  retrieveDictionaryConfig( String dictionaryName, ActiveConfig activeConfig )
	{
		 for (Map map : _configList)
			retrieveDictionary(map, dictionaryName, activeConfig.dictionaryConfig );
	}
	
	void  retrieveDictionaryConfig( String dictionaryName, DictionaryConfig dictConfig )
	{
		 for (Map map : _configList)
			retrieveDictionary(map, dictionaryName, dictConfig );
	}
	
	void  retrieveServerAllDictionaryConfig(Map map)
	{
		String rdmFieldDictionaryItemName = null, enumTypeItemName = null, rdmfieldDictionaryFileName = null, enumtypeDefFileName = null;
		boolean hasDictInfo = false;
		
		for (MapEntry mapEntry : map)
		{
			if (mapEntry.key().dataType() == DataTypes.ASCII &&
					mapEntry.key().ascii().ascii().equals("DictionaryGroup") &&
					mapEntry.loadType() == DataTypes.ELEMENT_LIST)
			{
				ElementList elementList = mapEntry.elementList();

				for (ElementEntry elementEntry : elementList)
				{
					if (elementEntry.loadType() == DataTypes.MAP && elementEntry.name().equals("DictionaryList"))
					{
						for (MapEntry dictMapEntry :  elementEntry.map())
						{
							if (dictMapEntry.key().dataType() == DataTypes.ASCII &&
									dictMapEntry.loadType() == DataTypes.ELEMENT_LIST)
							{
								hasDictInfo = false;
								for (ElementEntry entry : dictMapEntry.elementList())
								{
									switch (entry.loadType())
									{
									case DataTypes.ASCII:

										if (entry.name().equals("RdmFieldDictionaryFileName"))
										{
											hasDictInfo = true;
											rdmfieldDictionaryFileName = entry.ascii().ascii();
										}
										else if (entry.name().equals("EnumTypeDefFileName"))
										{
											hasDictInfo = true;
											enumtypeDefFileName = entry.ascii().ascii();
										}
										if (entry.name().equals("RdmFieldDictionaryItemName"))
										{
											hasDictInfo = true;
											rdmFieldDictionaryItemName = entry.ascii().ascii();
										}
										else if (entry.name().equals("EnumTypeDefItemName"))
										{
											hasDictInfo = true;
											enumTypeItemName = entry.ascii().ascii();
										}
										break;
									default:
										break;
									}
								}

								if (hasDictInfo)
								{
									DictionaryConfig dictConfig = new DictionaryConfig(true);
									_serverDictList.add(dictConfig);
									dictConfig.dictionaryName = dictMapEntry.key().ascii().ascii();

									if (rdmFieldDictionaryItemName != null && !rdmFieldDictionaryItemName.isEmpty())
										dictConfig.rdmFieldDictionaryItemName = rdmFieldDictionaryItemName;
									else
										dictConfig.rdmFieldDictionaryItemName = "RWFFld";
										
									if (enumTypeItemName != null && !enumTypeItemName.isEmpty())
										dictConfig.enumTypeDefItemName = enumTypeItemName;
									else
										dictConfig.enumTypeDefItemName = "RWFEnum";
										
									if (rdmfieldDictionaryFileName != null && !rdmfieldDictionaryFileName.isEmpty())
										dictConfig.rdmfieldDictionaryFileName = rdmfieldDictionaryFileName;
									else
										dictConfig.rdmfieldDictionaryFileName = "./RDMFieldDictionary";
										
									if (enumtypeDefFileName != null && !enumtypeDefFileName.isEmpty())
										dictConfig.enumtypeDefFileName = enumtypeDefFileName;
									else
										dictConfig.enumtypeDefFileName = "./enumtype.def";
								}
							}
						}
					}
					break;
				}
			}
		}
	}
	
	
	void  retrieveServerDictionaryConfig(Service service, List<ServiceDictionaryConfig> serviceDictionaryConfigList)
	{
		if (_dictProvided.isEmpty() && _dictUsed.isEmpty())
			return;

		ServiceDictionaryConfig fileServiceDictConfig = null;
		ServiceDictionaryConfig currentServiceDicConfig = null; 

		if (serviceDictionaryConfigList != null)
		{
			fileServiceDictConfig = findServiceDictConfig(serviceDictionaryConfigList, service.serviceId());
			currentServiceDicConfig = new ServiceDictionaryConfig();
			currentServiceDicConfig.serviceId = service.serviceId();
		}
		
		for (String dictName : _dictProvided)
		{
			DictionaryConfig findDictConfig = null;
			for (DictionaryConfig dictConfig : _serverDictList)
			{
				if (dictConfig.dictionaryName.equals(dictName))
				{
					findDictConfig = dictConfig;
					break;
				}
			}

			if (findDictConfig == null && fileServiceDictConfig != null) //will use dict config from file
				findDictConfig = fileServiceDictConfig.findDictionary(dictName, true);

			DictionaryConfig newDictConfig = new DictionaryConfig(true);
			newDictConfig.dictionaryName = dictName;
			if (findDictConfig == null) //use default
			{
				newDictConfig.rdmFieldDictionaryItemName = "RWFFld";
				newDictConfig.enumTypeDefItemName = "RWFEnum";
				newDictConfig.rdmfieldDictionaryFileName = "./RDMFieldDictionary";
				newDictConfig.enumtypeDefFileName = "./enumtype.def";
			}
			else
			{
				newDictConfig.isLocalDictionary = findDictConfig.isLocalDictionary;
				newDictConfig.rdmFieldDictionaryItemName = findDictConfig.rdmFieldDictionaryItemName;
				newDictConfig.enumTypeDefItemName = findDictConfig.enumTypeDefItemName;
				newDictConfig.rdmfieldDictionaryFileName = findDictConfig.rdmfieldDictionaryFileName;
				newDictConfig.enumtypeDefFileName = findDictConfig.enumtypeDefFileName;
			}

			service.info().dictionariesProvidedList().add(newDictConfig.rdmFieldDictionaryItemName);
			service.info().dictionariesProvidedList().add(newDictConfig.enumTypeDefItemName);

			if (currentServiceDicConfig != null)
				currentServiceDicConfig.dictionaryProvidedList.add(newDictConfig);
			else
				newDictConfig = null;
		}

		for (String dictName : _dictUsed)
		{
			DictionaryConfig findDictConfig = null;
			for (DictionaryConfig dictConfig : _serverDictList)
			{
				if (dictConfig.dictionaryName.equals(dictName))
				{
					findDictConfig = dictConfig;
					break;
				}
			}

			if (findDictConfig == null && fileServiceDictConfig != null) //will use dict config from file
				findDictConfig = fileServiceDictConfig.findDictionary(dictName, false);

			DictionaryConfig newDictConfig = new DictionaryConfig(true);
			newDictConfig.dictionaryName = dictName;
			if (findDictConfig == null) //use default
			{
				newDictConfig.rdmFieldDictionaryItemName = "RWFFld";
				newDictConfig.enumTypeDefItemName = "RWFEnum";
				newDictConfig.rdmfieldDictionaryFileName = "./RDMFieldDictionary";
				newDictConfig.enumtypeDefFileName = "./enumtype.def";
			}
			else
			{
				newDictConfig.isLocalDictionary = findDictConfig.isLocalDictionary;
				newDictConfig.rdmFieldDictionaryItemName = findDictConfig.rdmFieldDictionaryItemName;
				newDictConfig.enumTypeDefItemName = findDictConfig.enumTypeDefItemName;
				newDictConfig.rdmfieldDictionaryFileName = findDictConfig.rdmfieldDictionaryFileName;
				newDictConfig.enumtypeDefFileName = findDictConfig.enumtypeDefFileName;
			}

			service.info().dictionariesUsedList().add(newDictConfig.rdmFieldDictionaryItemName);
			service.info().dictionariesUsedList().add(newDictConfig.enumTypeDefItemName);

			if (currentServiceDicConfig != null)
				currentServiceDicConfig.dictionaryUsedList.add(newDictConfig);
			else
				newDictConfig = null;
		}

		if (currentServiceDicConfig != null)
		{
			if (fileServiceDictConfig != null)
			{
				serviceDictionaryConfigList.remove(fileServiceDictConfig);
				fileServiceDictConfig = null;
			}
			serviceDictionaryConfigList.add(currentServiceDicConfig);
		}
	}
	
	void  retrieveDirectoryConfig(String dictionaryName, DirectoryServiceStore dirServiceStore, DirectoryCache directoryCache, List<ServiceDictionaryConfig> serviceDictionaryConfigList)
	{
		if (_serverDictList.size() == 0)
		{
			 for (Map map : _configList)
				retrieveServerAllDictionaryConfig(map);
		}
	
		 for (Map map : _configList)
			retrieveDirectory(map, dictionaryName, dirServiceStore, directoryCache, serviceDictionaryConfigList);
	}
	
	void retrieveInstanceCommonConfig( Map map, String instanceName, BaseConfig activeConfig )
	{
		retrieveGroupAndListName( map );
	
		if ( _group != null && _group.isEmpty() )
			return;
		
		for (MapEntry mapEntry : map)
		{
			if ( mapEntry.key().dataType() == DataTypes.ASCII &&
				( mapEntry.key().ascii().ascii().equals("ConsumerGroup") || mapEntry.key().ascii().ascii().equals("NiProviderGroup")) &&
				mapEntry.loadType() == DataTypes.ELEMENT_LIST )
			{
				for (ElementEntry elementEntry : mapEntry.elementList())
				{
					if ( elementEntry.loadType() == DataTypes.MAP &&
							(elementEntry.name().equals("ConsumerList") ) || ( elementEntry.name().equals("NiProviderList")))
					{
						for (MapEntry instanceMapEntry : elementEntry.map())
						{
							if ( instanceMapEntry.key().dataType() == DataTypes.ASCII &&
									instanceMapEntry.key().ascii().ascii().equals(instanceName ) && 
									instanceMapEntry.loadType() == DataTypes.ELEMENT_LIST )
							{
								for (ElementEntry eentry : instanceMapEntry.elementList())
								{	
									switch ( eentry.loadType() )
									{
									case DataTypes.INT:
										if (eentry.name().equals("XmlTraceToStdout"))
										{
											activeConfig.xmlTraceEnable = eentry.intValue() > 0 ? true : false;
										}
										else if ( eentry.name().equals("ItemCountHint") )
										{
											if (eentry.intValue() >= 0)
												activeConfig.itemCountHint = convertToInt(eentry.intValue());
										}
										else if ( eentry.name().equals("ServiceCountHint") )
										{
											if (eentry.intValue() >= 0)
												activeConfig.serviceCountHint = convertToInt(eentry.intValue());
										}
										else if ( eentry.name().equals("RequestTimeout") )
										{
											if (eentry.intValue() >= 0)
												activeConfig.requestTimeout = convertToInt(eentry.intValue());
										}
										else if ( eentry.name().equals("MaxDispatchCountApiThread") )
										{
											if (eentry.intValue() >= 0)
												activeConfig.maxDispatchCountApiThread = convertToInt(eentry.intValue());
										}
										else if ( eentry.name().equals("MaxDispatchCountUserThread") )
										{
											if (eentry.intValue() >= 0)
												activeConfig.maxDispatchCountUserThread = convertToInt(eentry.intValue());
										}
										else if ( eentry.name().equals("DispatchTimeoutApiThread") )
										{
											if (eentry.intValue() >= 0)
												activeConfig.dispatchTimeoutApiThread = convertToInt(eentry.intValue());
										}
										else if (eentry.name().equals("MsgKeyInUpdates"))
										{
											((ActiveConfig)activeConfig).msgKeyInUpdates = eentry.intValue() > 0 ? true : false;
										}
										else if ( eentry.name().equals("LoginRequestTimeOut") )
										{
											if (eentry.intValue() >= 0)
												((ActiveConfig)activeConfig).loginRequestTimeOut = convertToInt(eentry.intValue());
										}
										else if (eentry.name().equals("ReconnectAttemptLimit"))
										{
											if (eentry.intValue() >= -1)
												((ActiveConfig)activeConfig).reconnectAttemptLimit = convertToInt(eentry.intValue());
										}
										else if (eentry.name().equals("ReconnectMinDelay"))
										{
											if (eentry.intValue() >= 0)
												((ActiveConfig)activeConfig).reconnectMinDelay = convertToInt(eentry.intValue());
										}
										else if (eentry.name().equals("ReconnectMaxDelay"))
										{
											if (eentry.intValue() >= 0)
												((ActiveConfig)activeConfig).reconnectMaxDelay = convertToInt(eentry.intValue());
										}
										break;
									default:
										break;
									}
								}
							}
						}
					}
				}
			}
			else if ( mapEntry.key().dataType() == DataTypes.ASCII &&
					mapEntry.key().ascii().ascii().equals("IProviderGroup") &&  mapEntry.loadType() == DataTypes.ELEMENT_LIST)
			{
				for (ElementEntry elementEntry : mapEntry.elementList())
				{
					if ( elementEntry.loadType() == DataTypes.MAP && elementEntry.name().equals("IProviderList"))
					{
						for (MapEntry instanceMapEntry : elementEntry.map())
						{
							if ( instanceMapEntry.key().dataType() == DataTypes.ASCII &&
									instanceMapEntry.key().ascii().ascii().equals(instanceName ) && 
									instanceMapEntry.loadType() == DataTypes.ELEMENT_LIST )
							{
								for (ElementEntry eentry : instanceMapEntry.elementList())
								{	
									switch ( eentry.loadType() )
									{
									case DataTypes.INT:
										if (eentry.name().equals("XmlTraceToStdout"))
										{
											activeConfig.xmlTraceEnable = eentry.intValue() > 0 ? true : false;
										}
										else if ( eentry.name().equals("ItemCountHint") )
										{
											if (eentry.intValue() >= 0)
												activeConfig.itemCountHint = convertToInt(eentry.intValue());
										}
										else if ( eentry.name().equals("ServiceCountHint") )
										{
											if (eentry.intValue() >= 0)
												activeConfig.serviceCountHint = convertToInt(eentry.intValue());
										}
										else if ( eentry.name().equals("RequestTimeout") )
										{
											if (eentry.intValue() >= 0)
												activeConfig.requestTimeout = convertToInt(eentry.intValue());
										}
										else if ( eentry.name().equals("MaxDispatchCountApiThread") )
										{
											if (eentry.intValue() >= 0)
												activeConfig.maxDispatchCountApiThread = convertToInt(eentry.intValue());
										}
										else if ( eentry.name().equals("MaxDispatchCountUserThread") )
										{
											if (eentry.intValue() >= 0)
												activeConfig.maxDispatchCountUserThread = convertToInt(eentry.intValue());
										}
										else if ( eentry.name().equals("DispatchTimeoutApiThread") )
										{
											if (eentry.intValue() >= 0)
												activeConfig.dispatchTimeoutApiThread = convertToInt(eentry.intValue());
										}
										
										break;
									default:
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
	
	void retrieveInstanceCustomConfig( Map map, String instanceName, BaseConfig activeConfig )
	{
		retrieveGroupAndListName( map );
	
		if ( _group != null && _group.isEmpty() )
			return;
		
		for (MapEntry mapEntry : map)
		{
			if ( mapEntry.key().dataType() == DataTypes.ASCII && mapEntry.loadType() == DataTypes.ELEMENT_LIST )
			{
				if (mapEntry.key().ascii().ascii().equals("ConsumerGroup") )
				{
					for (ElementEntry elementEntry : mapEntry.elementList())
					{
						if ( elementEntry.loadType() == DataTypes.MAP && elementEntry.name().equals("ConsumerList") )
						{
							for (MapEntry mapListEntry : elementEntry.map())
							{
								if (  mapListEntry.key().dataType() == DataTypes.ASCII  &&
									  mapListEntry.key().ascii().ascii().equals(instanceName) && 
									  mapListEntry.loadType() == DataTypes.ELEMENT_LIST		)
								{
									for (ElementEntry eentry : mapListEntry.elementList())
									{
										switch ( eentry.loadType() )
										{
										case DataTypes.INT:
											if ( eentry.name().equals("ObeyOpenWindow"))
											{
												if (eentry.intValue() >= 0)
													((ActiveConfig)activeConfig).obeyOpenWindow = convertToInt(eentry.intValue());
											}
											else if ( eentry.name().equals("PostAckTimeout"))
											{
												if (eentry.intValue() >= 0)
													((ActiveConfig)activeConfig).postAckTimeout = convertToInt(eentry.intValue());
											}
											else if ( eentry.name().equals("MaxOutstandingPosts"))
											{
												if (eentry.intValue() >= 0)
													((ActiveConfig)activeConfig).maxOutstandingPosts = convertToInt(eentry.intValue());
											}
											else if ( eentry.name().equals("DirectoryRequestTimeOut"))
											{
												if (eentry.intValue() >= 0)
													((ActiveConfig)activeConfig).directoryRequestTimeOut = convertToInt(eentry.intValue());
											}
											else if ( eentry.name().equals("DictionaryRequestTimeOut"))
											{
												if (eentry.intValue() >= 0)
													((ActiveConfig)activeConfig).dictionaryRequestTimeOut = convertToInt(eentry.intValue());
											}
											else if ( eentry.name().equals("ReissueTokenAttemptLimit"))
											{
												((ActiveConfig)activeConfig).reissueTokenAttemptLimit = convertToInt(eentry.intValue());
												
												if(((ActiveConfig)activeConfig).reissueTokenAttemptLimit < ActiveConfig.DEFAULT_REISSUE_TOKEN_ATTEMPT_LIMIT)
												{
													((ActiveConfig)activeConfig).reissueTokenAttemptLimit = ActiveConfig.DEFAULT_REISSUE_TOKEN_ATTEMPT_LIMIT;
												}
												
											}
											else if ( eentry.name().equals("ReissueTokenAttemptInterval"))
											{
												if (eentry.intValue() >= 0)
													((ActiveConfig)activeConfig).reissueTokenAttemptInterval = convertToInt(eentry.intValue());
												else
													((ActiveConfig)activeConfig).reissueTokenAttemptInterval = 0;
											}
											break;
										case DataTypes.UINT:
											if ( eentry.name().equals("RestRequestTimeOut"))
											{
												if (eentry.uintValue() >= 0)
													((ActiveConfig)activeConfig).restRequestTimeout = convertToInt(eentry.uintValue());
												else
													((ActiveConfig)activeConfig).restRequestTimeout = 0;
											}
											else if (eentry.name().equals("EnableRtt")) {
												if (eentry.uintValue() > 0) {
													((ActiveConfig)activeConfig).rsslRDMLoginRequest.attrib().applyHasSupportRoundTripLatencyMonitoring();
												}
											}
											break;
										case DataTypes.DOUBLE:
											if ( eentry.name().equals("TokenReissueRatio"))
											{
												if(eentry.doubleValue() > 0)
													((ActiveConfig)activeConfig).tokenReissueRatio = eentry.doubleValue();						
											}
											break;
										default:
											break;
										}
									}
								}
								break;
							}
						}
					}
				}
				else if ( mapEntry.key().ascii().ascii().equals("NiProviderGroup") )
				{
					for (ElementEntry elementEntry : mapEntry.elementList())
					{
						if ( elementEntry.loadType() == DataTypes.MAP && elementEntry.name().equals("NiProviderList") )
						{
							for (MapEntry mapListEntry : elementEntry.map())
							{
								if (  mapListEntry.key().dataType() == DataTypes.ASCII  &&
									  mapListEntry.key().ascii().ascii().equals(instanceName) && 
									  mapListEntry.loadType() == DataTypes.ELEMENT_LIST )
								{
									for (ElementEntry eentry : mapListEntry.elementList())
									{
										switch ( eentry.loadType() )
										{
										case DataTypes.INT:
														if (eentry.name().equals("RefreshFirstRequired"))
														{
															((OmmNiProviderActiveConfig)(activeConfig)).refreshFirstRequired = eentry.intValue() > 0 ? true : false;
														}
														else if (eentry.name().equals("MergeSourceDirectoryStreams"))
														{
															((OmmNiProviderActiveConfig)(activeConfig)).mergeSourceDirectoryStreams = eentry.intValue() > 0 ? true : false;
														}
														else if (eentry.name().equals("RecoverUserSubmitSourceDirectory"))
														{
															((OmmNiProviderActiveConfig)(activeConfig)).recoverUserSubmitSourceDirectory= eentry.intValue() > 0 ? true : false;
														}
														else if (eentry.name().equals("RemoveItemsOnDisconnect"))
														{
															((OmmNiProviderActiveConfig)(activeConfig)).removeItemsOnDisconnect= eentry.intValue() > 0 ? true : false;
														}
											break;
										default:
											break;
										}
									}
								}
								break;
							}
						}
					}
				}
				else if ( mapEntry.key().ascii().ascii().equals("IProviderGroup") )
				{
					for (ElementEntry elementEntry : mapEntry.elementList())
					{
						if ( elementEntry.loadType() == DataTypes.MAP && elementEntry.name().equals("IProviderList") )
						{
							for (MapEntry mapListEntry : elementEntry.map())
							{
								if ( ( mapListEntry.key().dataType() == DataTypes.ASCII ) && ( mapListEntry.key().ascii().ascii().equals(instanceName) ) )
								{
									if ( mapListEntry.loadType() == DataTypes.ELEMENT_LIST )
									{
										for (ElementEntry eentry : mapListEntry.elementList())
										{
											switch ( eentry.loadType() )
											{
											case DataTypes.INT:
													if (eentry.name().equals("AcceptDirMessageWithoutMinFilters"))
													{
														((ActiveServerConfig)activeConfig).acceptDirMessageWithoutMinFilters = eentry.intValue() > 0 ? true : false;
													}
													else if (eentry.name().equals("AcceptMessageSameKeyButDiffStream"))
													{
														((ActiveServerConfig)activeConfig).acceptMessageSameKeyButDiffStream = eentry.intValue() > 0 ? true : false;
													}
													else if (eentry.name().equals("AcceptMessageThatChangesService"))
													{
														((ActiveServerConfig)activeConfig).acceptMessageThatChangesService = eentry.intValue() > 0 ? true : false;
													}
													else if (eentry.name().equals("AcceptMessageWithoutAcceptingRequests"))
													{
														((ActiveServerConfig)activeConfig).acceptMessageWithoutAcceptingRequests = eentry.intValue() > 0 ? true : false;
													}
													else if (eentry.name().equals("AcceptMessageWithoutBeingLogin"))
													{
														((ActiveServerConfig)activeConfig).acceptMessageWithoutBeingLogin = eentry.intValue() > 0 ? true : false;
													}
													else if (eentry.name().equals("EnforceAckIDValidation"))
													{
														((ActiveServerConfig)activeConfig).enforceAckIDValidation = eentry.intValue() > 0 ? true : false;
													}
													else if (eentry.name().equals("AcceptMessageWithoutQosInRange"))
													{
														((ActiveServerConfig)activeConfig).acceptMessageWithoutQosInRange = eentry.intValue() > 0 ? true : false;
													}
													else if (eentry.name().equals("RefreshFirstRequired"))
													{
														((OmmIProviderActiveConfig)activeConfig).refreshFirstRequired = eentry.intValue() > 0 ? true : false;
													}
													else if (eentry.name().equals("EnumTypeFragmentSize"))
													{
														if (eentry.intValue() >= 0)
															((OmmIProviderActiveConfig)activeConfig).maxEnumTypeFragmentSize = convertToInt(eentry.intValue());
													}
													else if (eentry.name().equals("FieldDictionaryFragmentSize"))
													{
														if (eentry.intValue() >= 0)
															((OmmIProviderActiveConfig)activeConfig).maxFieldDictFragmentSize = convertToInt(eentry.intValue());
													}
												break;
											default:
												break;
											}
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
	
	void retrieveChannel( Map map, String channelName, ActiveConfig activeConfig, int hostFnCalled, ChannelConfig fileCfg)
	{
		for (MapEntry mapEntry : map)
		{
			if ( mapEntry.key().dataType() == DataTypes.ASCII &&
				 mapEntry.key().ascii().ascii().equals("ChannelGroup") &&
				 mapEntry.loadType() == DataTypes.ELEMENT_LIST )
			{
				for (ElementEntry elementEntry : mapEntry.elementList())
				{
					if ( elementEntry.loadType() == DataTypes.MAP && elementEntry.name().equals("ChannelList"))
					{
						for (MapEntry mapListEntry : elementEntry.map())
						{
							if ( mapListEntry.key().dataType() == DataTypes.ASCII  &&
								mapListEntry.key().ascii().ascii().equals(channelName) &&
								mapListEntry.loadType() == DataTypes.ELEMENT_LIST )
							{
								retrieveChannelInfo( mapListEntry, channelName, activeConfig, hostFnCalled, fileCfg);
							}
						}
					}
				}
			}
		}
	}
	
	void retrieveServer(Map map, String serverName,
		ActiveServerConfig activeServerConfig, int portFnCalled, ServerConfig fileCfg)
	{
		for (MapEntry mapEntry : map)
		{
			if (mapEntry.key().dataType() == DataTypes.ASCII &&
					mapEntry.key().ascii().ascii().equals("ServerGroup") &&
					mapEntry.loadType() == DataTypes.ELEMENT_LIST)
			{
				for (ElementEntry elementEntry : mapEntry.elementList())
				{
					if (elementEntry.loadType() == DataTypes.MAP && elementEntry.name().equals("ServerList"))
					{
						for (MapEntry mapListEntry : elementEntry.map())
						{
							if ((mapListEntry.key().dataType() == DataTypes.ASCII) && 
									mapListEntry.key().ascii().ascii().equals(serverName) && 
									mapListEntry.loadType() == DataTypes.ELEMENT_LIST)
							{
								retrieveServerInfo(mapListEntry, serverName, activeServerConfig, portFnCalled, fileCfg);
							}
						}
					}
				}
			}
		}
	}

	private MapEntry getElementListByNameFromConfigList(List<Map> configList, String name) {
		for (Map map : configList) {
			MapEntry globalConfigEntry = getElementListByName(map, name);
			if(globalConfigEntry != null){
				return globalConfigEntry;
			}
		}
		return null;
	}
	
	private ElementEntry getIntElementEntry(MapEntry mapEntry, String attributeName) {
		for (ElementEntry elementEntry : mapEntry.elementList()) {
			if(elementEntry.loadType() == DataTypes.INT){
				if (elementEntry.name().equals(attributeName))
				{
					return elementEntry;
				}
			}
		}
		return null;
	}

	private MapEntry getElementListByName(Map map, String name) {
		for (MapEntry mapEntry : map)
		{
			if (mapEntry.key().dataType() == DataTypes.ASCII &&
					mapEntry.key().ascii().ascii().equals(name) &&
					mapEntry.loadType() == DataTypes.ELEMENT_LIST)
			{
				return mapEntry;
			}
		}
		return null;
	}

	@SuppressWarnings("static-access")
	void retrieveChannelInfo( MapEntry mapEntry, String channelName, ActiveConfig activeConfig, int setByFnCalled, ChannelConfig fileCfg)
	{
		String interfaceName = null, host = null, port = null, objectName = null, tunnelingProxyHost = null, tunnelingProxyPort = null, location = null;
		int flags = 0, channelType = 0, compressionType = 0, tunnelingFlags = 0;
		long guaranteedOutputBuffers= 0;
		long compressionThreshold= 0;
		long connectionPingTimeout= 0;
		long numInputBuffers= 0;
		long sysSendBufSize= 0;
		long sysRecvBufSize= 0;
		long highWaterMark= 0;
		long initializationTimeout = 0;
		long tcpNodelay = 0, directWrite = 0, enableSessionMgnt = 0;
	
		for (ElementEntry channelEntry : mapEntry.elementList())
		{
			switch ( channelEntry.loadType() )
			{
			case DataTypes.ASCII:
				if ( channelEntry.name().equals("Host"))
				{
					host = channelEntry.ascii().ascii();
					flags |= ChannelEntryFlag.HOST_FLAG;
				}
				else if ( channelEntry.name().equals("Port"))
				{
					port = channelEntry.ascii().ascii();
					flags |= ChannelEntryFlag.PORT_FLAG;
				}
				else if ( channelEntry.name().equals("InterfaceName"))
				{
					interfaceName = channelEntry.ascii().ascii();
					flags |= ChannelEntryFlag.INTERFACENAME_FLAG;
				}
				else if ( channelEntry.name().equals("ObjectName"))
				{
					objectName = channelEntry.ascii().ascii();
					tunnelingFlags |= TunnelingEntryFlag.OBJECTNAME_FLAG;
				}
				else if (channelEntry.name().equals("ProxyPort"))
				{
					tunnelingProxyPort = channelEntry.ascii().ascii();
					tunnelingFlags |= TunnelingEntryFlag.PROXYPORT_FLAG;
				}
				else if (channelEntry.name().equals("ProxyHost"))
				{
					tunnelingProxyHost = channelEntry.ascii().ascii();
					tunnelingFlags |= TunnelingEntryFlag.PROXYHOST_FLAG;
				}
				else if ( channelEntry.name().equals("ChannelType"))
				{
					channelType = convertToEnum(channelEntry.ascii().ascii());
					
					switch ( channelType )
					{
					case ConnectionTypes.SOCKET:
					case ConnectionTypes.HTTP:
					case ConnectionTypes.ENCRYPTED:
						flags |= ChannelEntryFlag.CHANNELTYPE_FLAG;
						break;
					default:
						_emaConfigErrList.append( "Unsupported ChannelType [")
						.append( channelEntry.ascii().ascii())
						.append( "] in Programmatic Configuration. Use default ChannelType [ChannelType::RSSL_SOCKET]").create(Severity.ERROR);
						break;
					}
				}
				else if ( channelEntry.name().equals("CompressionType"))
				{
					compressionType = convertToEnum(channelEntry.ascii().ascii());
					
					switch ( compressionType )
					{
					case CompressionTypes.NONE:
					case CompressionTypes.ZLIB:
					case CompressionTypes.LZ4:
						flags |= ChannelEntryFlag.COMPRESSION_TYPE_FLAG;
						break;
					default:
						_emaConfigErrList.append( "Invalid CompressionType [" )
						.append( channelEntry.ascii().ascii() )
						.append( "] in Programmatic Configuration. Use default CompressionType [CompressionType::None] " ).create(Severity.ERROR);
						break;
					}
				}
				else if ( channelEntry.name().equals("Location"))
				{
					location = channelEntry.ascii().ascii();
					flags |= ChannelEntryFlag.LOCATION_FLAG;
				}
				break;
	
			case DataTypes.INT:
				if ( channelEntry.name().equals("GuaranteedOutputBuffers"))
				{
					guaranteedOutputBuffers = channelEntry.intValue();
					flags |= ChannelEntryFlag.GUARANTEED_OUTPUTBUFFERS_FLAG;
				}
				else if ( channelEntry.name().equals("NumInputBuffers"))
				{
					numInputBuffers = channelEntry.intValue();
					flags |= ChannelEntryFlag.NUM_INPUTBUFFERS_FLAG;
				}
				else if ( channelEntry.name().equals("SysRecvBufSize"))
				{
					sysRecvBufSize = channelEntry.intValue();
					flags |= ChannelEntryFlag.SYS_RECV_BUFSIZE_FLAG;
				}
				else if ( channelEntry.name().equals("SysSendBufSize"))
				{
					sysSendBufSize = channelEntry.intValue();
					flags |= ChannelEntryFlag.SYS_SEND_BUFSIZE_FLAG;
				}
				else if ( channelEntry.name().equals("HighWaterMark"))
				{
					highWaterMark = channelEntry.intValue();
					flags |= ChannelEntryFlag.HIGH_WATERMARK_FLAG;
				}
				else if ( channelEntry.name().equals("TcpNodelay"))
				{
					tcpNodelay = channelEntry.intValue();
					flags |= ChannelEntryFlag.TCP_NODELAY_FLAG;
				}
				else if ( channelEntry.name().equals("DirectWrite"))
				{
					directWrite = channelEntry.intValue();
					flags |= ChannelEntryFlag.DIRECTWRITE_FLAG;
				}
				else if (channelEntry.name().equals("ConnectionPingTimeout"))
				{
					connectionPingTimeout = channelEntry.intValue();
					flags |= ChannelEntryFlag.CONN_PING_TIMEOUT_FLAG;
				}
				else if ( channelEntry.name().equals("CompressionThreshold"))
				{
					compressionThreshold = channelEntry.intValue();
					flags |= ChannelEntryFlag.COMPRESSION_THRESHOLD_FLAG;
				}
				else if ( channelEntry.name().equals("InitializationTimeout"))
				{
					initializationTimeout = channelEntry.intValue();
					flags |= ChannelEntryFlag.INIT_TIMEOUT_FLAG;
				}
				else if ( channelEntry.name().equals("EnableSessionManagement"))
				{
					enableSessionMgnt = channelEntry.intValue();
					flags |= ChannelEntryFlag.ENABLE_SESSION_MGNT_FLAG;
				}
				break;
			default:
				break;
			}
		}
	
		if ( (flags & ChannelEntryFlag.CHANNELTYPE_FLAG) != 0)
		{
			if ( setByFnCalled == ActiveConfig.SOCKET_CONN_HOST_CONFIG_BY_FUNCTION_CALL )
			{
				channelType = ConnectionTypes.SOCKET;
				activeConfig.channelConfigSet.clear();
			}
			else if ( setByFnCalled > ActiveConfig.SOCKET_CONN_HOST_CONFIG_BY_FUNCTION_CALL )
			{
				if (channelType == ConnectionTypes.SOCKET)
					channelType = ConnectionTypes.ENCRYPTED;
				activeConfig.channelConfigSet.clear();
			}
	
			ChannelConfig currentChannelConfig = null;
	
			if ( channelType == ConnectionTypes.SOCKET )
			{
				SocketChannelConfig socketChannelConfig = new SocketChannelConfig();
				socketChannelConfig.serviceName = activeConfig.defaultServiceName;
				currentChannelConfig = socketChannelConfig;
				activeConfig.channelConfigSet.add( currentChannelConfig );

				SocketChannelConfig fileCfgSocket = null;
				if ( fileCfg != null && fileCfg.rsslConnectionType == ConnectionTypes.SOCKET )
					fileCfgSocket = (SocketChannelConfig)( fileCfg );

				if ( (flags & ChannelEntryFlag.TCP_NODELAY_FLAG) != 0 )
					socketChannelConfig.tcpNodelay = (tcpNodelay == 0 ? false : ActiveConfig.DEFAULT_TCP_NODELAY);
				else if ( fileCfgSocket != null )
					socketChannelConfig.tcpNodelay = fileCfgSocket.tcpNodelay;
				
				if ( (flags & ChannelEntryFlag.DIRECTWRITE_FLAG) != 0 )
					socketChannelConfig.directWrite = (directWrite == 1 ? true : ActiveConfig.DEFAULT_DIRECT_SOCKET_WRITE);
				else if ( fileCfgSocket != null )
					socketChannelConfig.directWrite = fileCfgSocket.directWrite;

				if ( (flags & ChannelEntryFlag.HOST_FLAG) != 0 && setByFnCalled == 0 )
					socketChannelConfig.hostName = host;
				else if ( fileCfgSocket != null )
					socketChannelConfig.hostName = fileCfgSocket.hostName;

				if ( (flags & ChannelEntryFlag.PORT_FLAG) != 0 && setByFnCalled == 0 )
					socketChannelConfig.serviceName = port;
				else if ( fileCfgSocket != null )
					socketChannelConfig.serviceName = fileCfgSocket.serviceName;

			}
			else if ( channelType == ConnectionTypes.ENCRYPTED || channelType == ConnectionTypes.HTTP )
			{
				HttpChannelConfig httpChannelConfig = (channelType == ConnectionTypes.ENCRYPTED) ? new EncryptedChannelConfig() : new HttpChannelConfig();
				httpChannelConfig.rsslConnectionType = channelType;
				currentChannelConfig = httpChannelConfig;
				activeConfig.channelConfigSet.add( currentChannelConfig );

				HttpChannelConfig fileCfgEncrypt = null;
				if ( fileCfg != null && (fileCfg.rsslConnectionType == ConnectionTypes.ENCRYPTED || fileCfg.rsslConnectionType == ConnectionTypes.HTTP) )
					fileCfgEncrypt = (HttpChannelConfig)( fileCfg );

				if ( (flags & ChannelEntryFlag.TCP_NODELAY_FLAG) != 0 )
					httpChannelConfig.tcpNodelay = (tcpNodelay == 0 ? false : ActiveConfig.DEFAULT_TCP_NODELAY);
				else if ( fileCfgEncrypt != null )
					httpChannelConfig.tcpNodelay = fileCfgEncrypt.tcpNodelay;

				if ( (flags & ChannelEntryFlag.HOST_FLAG) != 0 )
					httpChannelConfig.hostName = host;
				else if ( fileCfgEncrypt != null )
					httpChannelConfig.hostName = fileCfgEncrypt.hostName;

				if ( (flags & ChannelEntryFlag.PORT_FLAG) != 0 )
					httpChannelConfig.serviceName = port;
				else if ( fileCfgEncrypt != null )
					httpChannelConfig.serviceName = fileCfgEncrypt.serviceName;
				
				if (channelType == ConnectionTypes.ENCRYPTED)
				{
					if ( (flags & ChannelEntryFlag.ENABLE_SESSION_MGNT_FLAG) != 0 )
						((EncryptedChannelConfig)httpChannelConfig).enableSessionMgnt = enableSessionMgnt == 0 ? false : true;
					else if ( ( fileCfgEncrypt != null ) && (fileCfg.rsslConnectionType == ConnectionTypes.ENCRYPTED) )
					{
						((EncryptedChannelConfig)httpChannelConfig).enableSessionMgnt = ((EncryptedChannelConfig)fileCfgEncrypt).enableSessionMgnt;
					}
					
					if ( (flags & ChannelEntryFlag.LOCATION_FLAG) != 0 )
						((EncryptedChannelConfig)httpChannelConfig).location = location;
					else if ( ( fileCfgEncrypt != null ) && (fileCfg.rsslConnectionType == ConnectionTypes.ENCRYPTED) )
					{
						((EncryptedChannelConfig)httpChannelConfig).location = ((EncryptedChannelConfig)fileCfgEncrypt).location;
					}
				}

				if ( (tunnelingFlags & TunnelingEntryFlag.OBJECTNAME_FLAG) != 0 && (setByFnCalled & ActiveConfig.TUNNELING_OBJNAME_CONFIG_BY_FUNCTION_CALL) == 0 )
					httpChannelConfig.objectName = objectName;
				else if ( fileCfgEncrypt != null )
					httpChannelConfig.objectName = fileCfgEncrypt.objectName;

				if ((tunnelingFlags & TunnelingEntryFlag.PROXYPORT_FLAG) != 0 && (setByFnCalled & ActiveConfig.TUNNELING_PROXY_PORT_CONFIG_BY_FUNCTION_CALL) == 0  )
					httpChannelConfig.httpProxyPort = tunnelingProxyPort;
				else if (fileCfgEncrypt != null)
					httpChannelConfig.httpProxyPort = fileCfgEncrypt.httpProxyPort;

				if ((tunnelingFlags & TunnelingEntryFlag.PROXYHOST_FLAG) != 0 && (setByFnCalled & ActiveConfig.TUNNELING_PROXY_HOST_CONFIG_BY_FUNCTION_CALL) == 0  )
					httpChannelConfig.httpProxyHostName = tunnelingProxyHost;
				else if (fileCfgEncrypt != null)
					httpChannelConfig.httpProxyHostName = fileCfgEncrypt.httpProxyHostName;
				
				if ( (httpChannelConfig.httpProxyPort != null && httpChannelConfig.httpProxyPort.length()  > 0) ||
					     (httpChannelConfig.httpProxyHostName != null && httpChannelConfig.httpProxyHostName.length() > 0))
					httpChannelConfig.httpProxy = true;
				
				//need to copy other tunneling setting from function calls.
				if (fileCfgEncrypt != null)
				{
					httpChannelConfig.httpProxyUserName = fileCfgEncrypt.httpProxyUserName;
					httpChannelConfig.httpproxyPasswd = fileCfgEncrypt.httpproxyPasswd;
					httpChannelConfig.httpProxyDomain = fileCfgEncrypt.httpProxyDomain;
					httpChannelConfig.httpProxyLocalHostName = fileCfgEncrypt.httpProxyLocalHostName;
					httpChannelConfig.httpProxyKRB5ConfigFile = fileCfgEncrypt.httpProxyKRB5ConfigFile;
					
					if (channelType == ConnectionTypes.ENCRYPTED && fileCfgEncrypt.rsslConnectionType == ConnectionTypes.ENCRYPTED)
					{
						((EncryptedChannelConfig)httpChannelConfig).KeyStoreType = ((EncryptedChannelConfig)fileCfgEncrypt).KeyStoreType;
						((EncryptedChannelConfig)httpChannelConfig).KeyStoreFile = ((EncryptedChannelConfig)fileCfgEncrypt).KeyStoreFile;
						((EncryptedChannelConfig)httpChannelConfig).KeyStorePasswd = ((EncryptedChannelConfig)fileCfgEncrypt).KeyStorePasswd;
						((EncryptedChannelConfig)httpChannelConfig).SecurityProvider = ((EncryptedChannelConfig)fileCfgEncrypt).SecurityProvider;
						((EncryptedChannelConfig)httpChannelConfig).SecurityProtocol = ((EncryptedChannelConfig)fileCfgEncrypt).SecurityProtocol;
						((EncryptedChannelConfig)httpChannelConfig).KeyManagerAlgorithm = ((EncryptedChannelConfig)fileCfgEncrypt).KeyManagerAlgorithm;
						((EncryptedChannelConfig)httpChannelConfig).TrustManagerAlgorithm = ((EncryptedChannelConfig)fileCfgEncrypt).TrustManagerAlgorithm;
					}
				}
			}

			currentChannelConfig.name = channelName;
	
			boolean useFileCfg = ( fileCfg != null && fileCfg.rsslConnectionType == currentChannelConfig.rsslConnectionType ) ? true : false;
	
			if ( (flags & ChannelEntryFlag.INTERFACENAME_FLAG) != 0 )
				currentChannelConfig.interfaceName = interfaceName;
			else if ( useFileCfg )
				currentChannelConfig.interfaceName = fileCfg.interfaceName;
	
			if ( (flags & ChannelEntryFlag.COMPRESSION_TYPE_FLAG) != 0 )
				currentChannelConfig.compressionType = compressionType;
			else if ( useFileCfg )
				currentChannelConfig.compressionType = fileCfg.compressionType;

			if ( (flags & ChannelEntryFlag.COMPRESSION_THRESHOLD_FLAG) != 0 && compressionThreshold >= 0)
				currentChannelConfig.compressionThreshold = convertToInt(compressionThreshold);
			else if ( useFileCfg )
				currentChannelConfig.compressionThreshold = fileCfg.compressionThreshold;
	
			if ( (flags & ChannelEntryFlag.GUARANTEED_OUTPUTBUFFERS_FLAG) != 0 && guaranteedOutputBuffers >= 0)
				currentChannelConfig.guaranteedOutputBuffers = convertToInt(guaranteedOutputBuffers);
			else if ( useFileCfg )
				currentChannelConfig.guaranteedOutputBuffers = fileCfg.guaranteedOutputBuffers;
	
			if ( (flags & ChannelEntryFlag.NUM_INPUTBUFFERS_FLAG) != 0 && numInputBuffers >= 0)
				currentChannelConfig.numInputBuffers = convertToInt(numInputBuffers);
			else if ( useFileCfg )
				currentChannelConfig.numInputBuffers = fileCfg.numInputBuffers;
	
			if ( (flags & ChannelEntryFlag.SYS_RECV_BUFSIZE_FLAG) != 0 && sysRecvBufSize >= 0)
				currentChannelConfig.sysRecvBufSize = convertToInt(sysRecvBufSize);
			else if ( useFileCfg )
				currentChannelConfig.sysRecvBufSize = fileCfg.sysRecvBufSize;
	
			if ( (flags & ChannelEntryFlag.SYS_SEND_BUFSIZE_FLAG) != 0 && sysSendBufSize >= 0)
				currentChannelConfig.sysSendBufSize = convertToInt(sysSendBufSize);
			else if ( useFileCfg )
				currentChannelConfig.sysSendBufSize = fileCfg.sysSendBufSize;
	
			if ( (flags & ChannelEntryFlag.HIGH_WATERMARK_FLAG) != 0 && highWaterMark >= 0)
				currentChannelConfig.highWaterMark = convertToInt(highWaterMark);
			else if ( useFileCfg )
				currentChannelConfig.highWaterMark = fileCfg.highWaterMark;
	
			if ( (flags & ChannelEntryFlag.CONN_PING_TIMEOUT_FLAG) != 0 && connectionPingTimeout >= 0)
				currentChannelConfig.connectionPingTimeout = convertToInt(connectionPingTimeout);
			else if ( useFileCfg )
				currentChannelConfig.connectionPingTimeout = fileCfg.connectionPingTimeout;
			
			if ( (flags & ChannelEntryFlag.INIT_TIMEOUT_FLAG) != 0 && initializationTimeout >= 0)
				currentChannelConfig.initializationTimeout = convertToInt(initializationTimeout);
			else if ( useFileCfg )
				currentChannelConfig.initializationTimeout = fileCfg.initializationTimeout;
		}
	}
	
	void retrieveServerInfo(MapEntry mapEntry, String serverName,
		ActiveServerConfig activeServerConfig, int setByFnCalled, ServerConfig fileCfg)
	{
		String interfaceName = null, port = null;
		int flags = 0, serverType = 0, compressionType = 0;
		long guaranteedOutputBuffers= 0;
		long compressionThreshold= 0;
		long connectionPingTimeout= 0;
		long connectionMinPingTimeout= 0;
		long numInputBuffers= 0;
		long sysSendBufSize= 0;
		long sysRecvBufSize= 0;
		long highWaterMark= 0;
		long tcpNodelay = 0;
		long directWrite = 0;
		long initializationTimeout = 0;
		
		for (ElementEntry serverEntry : mapEntry.elementList())
		{
			switch (serverEntry.loadType())
			{
			case DataTypes.ASCII:
				if (serverEntry.name().equals("Port"))
				{
					port = serverEntry.ascii().ascii();
					flags |= ServerEntryFlag.PORT_FLAG;
				}
				else if (serverEntry.name().equals("InterfaceName"))
				{
					interfaceName = serverEntry.ascii().ascii();
					flags |= ServerEntryFlag.INTERFACENAME_FLAG;
				}
				else if (serverEntry.name().equals("ServerType"))
				{
					serverType = convertToEnum(serverEntry.ascii().ascii());
	
					switch (serverType)
					{
					case ConnectionTypes.SOCKET:
						flags |= ServerEntryFlag.SERVERTYPE_FLAG;
						break;
					default:
						_emaConfigErrList.append( "Unsupported ServerType [")
						.append( serverEntry.ascii().ascii())
						.append( "] in Programmatic Configuration. Use default ServerType [RSSL_SOCKET]").create(Severity.ERROR);
						break;
					}
				}
				else if (serverEntry.name().equals("CompressionType"))
				{
					compressionType = convertToEnum(serverEntry.ascii().ascii());
	
					switch ( compressionType )
					{
					case CompressionTypes.NONE:
					case CompressionTypes.ZLIB:
					case CompressionTypes.LZ4:
						flags |= ChannelEntryFlag.COMPRESSION_TYPE_FLAG;
						break;
					default:
						_emaConfigErrList.append( "Invalid CompressionType [" )
						.append( serverEntry.ascii().ascii() )
						.append( "] in Programmatic Configuration. Use default CompressionType [CompressionType::None] " ).create(Severity.ERROR);
						break;
					}
				}
				
				break;
	
			case DataTypes.INT:
				if (serverEntry.name().equals("GuaranteedOutputBuffers"))
				{
					guaranteedOutputBuffers = serverEntry.intValue();
					flags |= ServerEntryFlag.GUARANTEED_OUTPUTBUFFERS_FLAG;
				}
				if (serverEntry.name().equals("NumInputBuffers"))
				{
					numInputBuffers = serverEntry.intValue();
					flags |= ServerEntryFlag.NUMINPUTBUF_FLAG;
				}
				if (serverEntry.name().equals("SysRecvBufSize"))
				{
					sysRecvBufSize = serverEntry.intValue();
					flags |= ServerEntryFlag.SYS_RECV_BUFSIZE_FLAG;
				}
				if (serverEntry.name().equals("SysSendBufSize"))
				{
					sysSendBufSize = serverEntry.intValue();
					flags |= ServerEntryFlag.SYS_SEND_BUFSIZE_FLAG;
				}
				if (serverEntry.name().equals("HighWaterMark"))
				{
					highWaterMark = serverEntry.intValue();
					flags |= ServerEntryFlag.HIGH_WATERMARK_FLAG;
				}
				else if (serverEntry.name().equals("TcpNodelay"))
				{
					tcpNodelay = serverEntry.intValue();
					flags |= ServerEntryFlag.TCP_NODELAY_FLAG;
				}
				if (serverEntry.name().equals("ConnectionMinPingTimeout"))
				{
					connectionMinPingTimeout = serverEntry.intValue();
					flags |= ServerEntryFlag.CONN_MIN_PING_TIMEOUT_FLAG;
				}
				else if (serverEntry.name().equals("ConnectionPingTimeout"))
				{
					connectionPingTimeout = serverEntry.intValue();
					flags |= ServerEntryFlag.CONN_PING_TIMEOUT_FLAG;
				}
				else if (serverEntry.name().equals("CompressionThreshold"))
				{
					compressionThreshold = serverEntry.intValue();
					flags |= ServerEntryFlag.COMPRESSION_THRESHOLD_FLAG;
				}
				else if (serverEntry.name().equals("DirectWrite"))
				{
					directWrite = serverEntry.intValue();
					flags |= ServerEntryFlag.DIRECTWRITE_FLAG;
				}
				else if (serverEntry.name().equals("InitializationTimeout"))
				{
					initializationTimeout = serverEntry.intValue();
					flags |= ServerEntryFlag.INIT_TIMEOUT_FLAG;
				}
				break;
			default:
				break;
			}
		}
	
		if ((flags & ServerEntryFlag.SERVERTYPE_FLAG) != 0)
		{
			activeServerConfig.serverConfig = new SocketServerConfig();
			SocketServerConfig currentServerConfig = (SocketServerConfig)activeServerConfig.serverConfig;
			currentServerConfig.serviceName = ActiveServerConfig.defaultServiceName;
			SocketServerConfig fileCfgSocket = (SocketServerConfig)fileCfg;

			if ((flags & ServerEntryFlag.TCP_NODELAY_FLAG) != 0)
				currentServerConfig.tcpNodelay = (tcpNodelay == 0 ? false : ActiveConfig.DEFAULT_TCP_NODELAY);
			else if ( fileCfgSocket != null )
				currentServerConfig.tcpNodelay = fileCfgSocket.tcpNodelay;

			if ((flags & ServerEntryFlag.PORT_FLAG) != 0 && setByFnCalled == 0)
				currentServerConfig.serviceName = port;
			else if ( fileCfgSocket != null )
				currentServerConfig.serviceName = fileCfgSocket.serviceName;

			currentServerConfig.name = serverName;

			if ( (flags & ServerEntryFlag.DIRECTWRITE_FLAG) != 0 )
				currentServerConfig.directWrite = (directWrite == 1 ? true : ActiveConfig.DEFAULT_DIRECT_SOCKET_WRITE);
			else if ( fileCfgSocket != null )
				currentServerConfig.directWrite = fileCfgSocket.directWrite;
			
			if ((flags & ServerEntryFlag.INTERFACENAME_FLAG) != 0)
				currentServerConfig.interfaceName = interfaceName;
			else  if ( fileCfg != null )
				currentServerConfig.interfaceName = fileCfg.interfaceName;

			if ((flags & ServerEntryFlag.CONN_MIN_PING_TIMEOUT_FLAG) != 0  && connectionMinPingTimeout >= 0)
				currentServerConfig.connectionMinPingTimeout = convertToInt(connectionMinPingTimeout);
			else  if ( fileCfg != null )
				currentServerConfig.connectionMinPingTimeout = fileCfg.connectionMinPingTimeout;

			if ((flags & ServerEntryFlag.COMPRESSION_TYPE_FLAG) != 0)
				currentServerConfig.compressionType = compressionType;
			else  if ( fileCfg != null )
				currentServerConfig.compressionType = fileCfg.compressionType;

			if ((flags & ServerEntryFlag.COMPRESSION_THRESHOLD_FLAG) != 0 && compressionThreshold >= 0)
				currentServerConfig.compressionThreshold = convertToInt(compressionThreshold);
			else if ( fileCfg != null )
				currentServerConfig.compressionThreshold = fileCfg.compressionThreshold;

			if ((flags & ServerEntryFlag.GUARANTEED_OUTPUTBUFFERS_FLAG) != 0  && guaranteedOutputBuffers >= 0)
				currentServerConfig.guaranteedOutputBuffers = convertToInt(guaranteedOutputBuffers);
			else if ( fileCfg != null )
				currentServerConfig.guaranteedOutputBuffers = fileCfg.guaranteedOutputBuffers;

			if ((flags & ServerEntryFlag.NUMINPUTBUF_FLAG) != 0 && numInputBuffers >= 0)
				currentServerConfig.numInputBuffers = convertToInt(numInputBuffers);
			else if ( fileCfg != null )
				currentServerConfig.numInputBuffers = fileCfg.numInputBuffers;

			if ((flags & ServerEntryFlag.SYS_RECV_BUFSIZE_FLAG) != 0 && sysRecvBufSize >= 0)
				currentServerConfig.sysRecvBufSize = convertToInt(sysRecvBufSize);
			else if ( fileCfg != null )
				currentServerConfig.sysRecvBufSize = fileCfg.sysRecvBufSize;

			if ((flags & ServerEntryFlag.SYS_SEND_BUFSIZE_FLAG) != 0 && sysSendBufSize >= 0)
				currentServerConfig.sysSendBufSize = convertToInt(sysSendBufSize);
			else if ( fileCfg != null )
				currentServerConfig.sysSendBufSize = fileCfg.sysSendBufSize;

			if ((flags & ServerEntryFlag.HIGH_WATERMARK_FLAG) != 0 && highWaterMark >= 0)
				currentServerConfig.highWaterMark = convertToInt(highWaterMark);
			else if ( fileCfg != null )
				currentServerConfig.highWaterMark = fileCfg.highWaterMark;

			if ((flags & ServerEntryFlag.CONN_PING_TIMEOUT_FLAG) != 0 && connectionPingTimeout >= 0)
				currentServerConfig.connectionPingTimeout = convertToInt(connectionPingTimeout);
			else if ( fileCfg != null )
				currentServerConfig.connectionPingTimeout = fileCfg.connectionPingTimeout;
			
			if ((flags & ServerEntryFlag.INIT_TIMEOUT_FLAG) != 0 && initializationTimeout >= 0)
				currentServerConfig.initializationTimeout = convertToInt(initializationTimeout);
			else if ( fileCfg != null )
				currentServerConfig.initializationTimeout = fileCfg.initializationTimeout;
		}
	}
	
	
	void retrieveDictionary( Map map, String dictionaryName, DictionaryConfig dictionaryConfig )
	{
		for (MapEntry mapEntry : map)
		{
			if ( mapEntry.key().dataType() == DataTypes.ASCII && mapEntry.key().ascii().ascii().equals("DictionaryGroup") )
			{
				if ( mapEntry.loadType() == DataTypes.ELEMENT_LIST )
				{
					for (ElementEntry elementEntry : mapEntry.elementList())
					{
						if ( elementEntry.loadType() == DataTypes.MAP )
						{
							if ( elementEntry.name().equals("DictionaryList"))
							{
								for (MapEntry mapListEntry : elementEntry.map())
								{
									if ( ( mapListEntry.key().dataType() == DataTypes.ASCII ) && ( mapListEntry.key().ascii().ascii().equals(dictionaryName) ) )
									{
										if ( mapListEntry.loadType() == DataTypes.ELEMENT_LIST )
										{
											for (ElementEntry dictEntry : mapListEntry.elementList())
											{
												switch ( dictEntry.loadType() )
												{
												case DataTypes.ASCII:
	
													if ( dictEntry.name().equals("RdmFieldDictionaryFileName"))
													{
														dictionaryConfig.rdmfieldDictionaryFileName = dictEntry.ascii().ascii();
													}
													else if ( dictEntry.name().equals("EnumTypeDefFileName"))
													{
														dictionaryConfig.enumtypeDefFileName = dictEntry.ascii().ascii();
													}
													if (dictEntry.name().equals("RdmFieldDictionaryItemName"))
													{
														dictionaryConfig.rdmFieldDictionaryItemName = dictEntry.ascii().ascii();
													}
													else if (dictEntry.name().equals("EnumTypeDefItemName"))
													{
														dictionaryConfig.enumTypeDefItemName = dictEntry.ascii().ascii();
													}
													else if ( dictEntry.name().equals("DictionaryType"))
													{
														int dictionaryType = convertToEnum(dictEntry.ascii().ascii());
	
														if ( dictionaryType < 0 )
														{
															_emaConfigErrList.append( "Invalid DictionaryType [" )
															.append( dictEntry.ascii().ascii() )
															.append( "] in Programmatic Configuration. Use default DictionaryType." ).create(Severity.ERROR);
														}
														else
														{
															if (dictionaryType > 0)
																dictionaryConfig.isLocalDictionary = true;
															else
																dictionaryConfig.isLocalDictionary = false;
														}
													}
													break;
												default:
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
	
	void retrieveDirectory( Map map, String directoryName, DirectoryServiceStore dirServiceStore, DirectoryCache directoryCache, List<ServiceDictionaryConfig> serviceDictionaryConfigList)
	{
		
		for (MapEntry mapEntry : map)
		{
			if (mapEntry.key().dataType() == DataTypes.ASCII &&
				mapEntry.key().ascii().ascii().equals("DirectoryGroup") &&
				mapEntry.loadType() == DataTypes.ELEMENT_LIST)
			{
				for (ElementEntry elementEntry : mapEntry.elementList())
				{
					if (elementEntry.loadType() == DataTypes.MAP && elementEntry.name().equals("DirectoryList"))
					{
						for (MapEntry dirListMapEntry : elementEntry.map())
						{
							if (dirListMapEntry.key().dataType() == DataTypes.ASCII &&
								dirListMapEntry.key().ascii().ascii().equals(directoryName) &&
								dirListMapEntry.loadType() == DataTypes.MAP) 
							{
								_serviceNameList.clear();
								ServiceIdInteger origServiceIdInteger;
								for (MapEntry eachServiceEntry : dirListMapEntry.map())
								{
									if (eachServiceEntry.key().dataType() == DataTypes.ASCII &&
										eachServiceEntry.loadType() == DataTypes.ELEMENT_LIST)
									{
										_dictProvided.clear();
										_dictUsed.clear();
										
										String serviceName = eachServiceEntry.key().ascii().ascii();
										origServiceIdInteger = dirServiceStore.serviceId(serviceName);
										Service service = null;
										if (origServiceIdInteger != null)
											service = directoryCache.getService(origServiceIdInteger.value());
										
										boolean addNewService = false;
										if (service == null)
										{
											addNewService = true;
											service = DirectoryMsgFactory.createService();
											service.applyHasInfo();
											service.info().serviceName().data(serviceName);
											
											//allocate default value for qos first.
											service.info().applyHasQos();
									        Qos qos = CodecFactory.createQos();
									        qos.rate(QosRates.TICK_BY_TICK);
									        qos.timeliness(QosTimeliness.REALTIME);
									        service.info().qosList().add(qos);
										}
	
										for (ElementEntry eachFilterEntry : eachServiceEntry.elementList())
										{
											if (eachFilterEntry.loadType() == DataTypes.ELEMENT_LIST &&
												(eachFilterEntry.name().equals("InfoFilter") || eachFilterEntry.name().equals("StateFilter")))
											{
												if (!retrieveServiceInfo(service, eachFilterEntry.elementList(), directoryCache, serviceDictionaryConfigList))
												{
													service = null;
													break;
												}
											}
										}
	
										if (service != null)
										{
											if (addNewService)
											{
												directoryCache.addService(service);
												dirServiceStore.addToMap(service.serviceId(), service.info().serviceName().toString());
											}
											else if (origServiceIdInteger != null && origServiceIdInteger.value() != service.serviceId())
											{
												dirServiceStore.remove(origServiceIdInteger.value());
												dirServiceStore.addToMap(service.serviceId(), service.info().serviceName().toString());
											}
											
											retrieveServerDictionaryConfig(service, serviceDictionaryConfigList);
											_serviceNameList.add(service.info().serviceName().toString());
										}
									}
								}
								
								//remove old ones from config file which is not configured by programmatic
								removeConfigFileService(dirServiceStore, directoryCache);
								break;
							}
						} 
					}
				}
			}
		}
	}
	
	boolean retrieveServiceInfo(Service service, ElementList serviceInfo, 
		DirectoryCache dirCache, List<ServiceDictionaryConfig> serviceDictionaryConfigList)
	{
		long intVal;
		String stringVal = null;
		int rate = 0, timeliness = 0, result = 0;
			
		for (ElementEntry entry : serviceInfo)
		{
			switch (entry.loadType())
			{
			case DataTypes.ASCII:
				stringVal = entry.ascii().ascii();
				if (entry.name().equals("StreamState"))
				{
					service.applyHasState();
					service.state().applyHasStatus();
					service.state().status().streamState(StreamState.OPEN);
					result = convertToEnum(stringVal);
					if (result == INVALID_RETVAL)
					{
						_emaConfigErrList.append( "failed to convert a StreamState from the programmatically configured service [" )
						.append( service.info().serviceName().toString() )
						.append( "]. Will use default StreamState. Suspect value is" )
						.append( stringVal )
						.create(Severity.ERROR);
					}
					else
						service.state().status().streamState(result);
				}
				else if (entry.name().equals("DataState"))
				{
					service.state().applyHasStatus();
					service.state().status().dataState(OmmState.DataState.OK);
					result = convertToEnum(stringVal);
					if (result == INVALID_RETVAL)
					{
						_emaConfigErrList.append( "failed to convert a DataState from the programmatically configured service [" )
						.append( service.info().serviceName().toString() )
						.append( "]. Will use default DataState. Suspect value is" )
						.append( stringVal )
						.create(Severity.ERROR);
					}
					else
						service.state().status().dataState(result);
				}
				else if (entry.name().equals("StatusCode"))
				{
					service.state().applyHasStatus();
					service.state().status().code(OmmState.StatusCode.NONE);
					result = convertToEnum(stringVal);
					if (result == INVALID_RETVAL)
					{
						_emaConfigErrList.append( "failed to convert a StatusCode from the progprogrammaticallyonfigured service [" )
						.append( service.info().serviceName().toString() )
						.append( "]. Will use default StatusCode. Suspect value is" )
						.append( stringVal )
						.create(Severity.ERROR);
					}
					else
						service.state().status().code(result);
				}
				else if (entry.name().equals("StatusText"))
				{
					if (!stringVal.isEmpty())
					{
						service.state().applyHasStatus();
						service.state().status().text().data(stringVal);
					}
					else
						service.state().status().text().clear();
				}
				else if (entry.name().equals("Vendor"))
				{
					service.info().applyHasVendor();
					service.info().vendor().data(stringVal);
				}
				else if (entry.name().equals("ItemList"))
				{
					service.info().applyHasItemList();
					service.info().itemList().data(stringVal);
				}
				else if (entry.name().equals("Timeliness"))
				{
					result = convertToEnum(stringVal);
					if (result == INVALID_RETVAL)
					{
						_emaConfigErrList.append( "failed to convert a QoS Timeliness from the programmatically configured service [" )
						.append( service.info().serviceName().toString() )
						.append( "]. Will use default Timeliness. Suspect value is" )
						.append( stringVal )
						.create(Severity.ERROR);
					}
					else
					{
						timeliness = result;
						service.info().applyHasQos();
						addQos = true;
					}
				}
				else if (entry.name().equals("Rate"))
				{
					result = convertToEnum(stringVal);
					if (result == INVALID_RETVAL)
					{
						_emaConfigErrList.append( "failed to convert a QoS Rate from the programmatically configured service [" )
						.append( service.info().serviceName().toString() )
						.append( "]. Will use default Rate. Suspect value is" )
						.append( stringVal )
						.create(Severity.ERROR);
					}
					else
					{
						rate = result;
						service.info().applyHasQos();
						addQos = true;
					}
				}
				break;
			case DataTypes.INT:
				if (entry.name().equals("ServiceState"))
				{
					service.applyHasState();
					service.state().serviceState(entry.intValue() > 0 ? 1 : 0);
				}
				else if (entry.name().equals("AcceptingRequests"))
				{
					service.applyHasState();
					service.state().applyHasAcceptingRequests();
					service.state().acceptingRequests(entry.intValue() > 0 ? 1 : 0);
				}
				else if (entry.name().equals("ServiceId"))
				{
					service.applyHasInfo();
					intVal = entry.intValue();
					
					if (intVal > Integer.MAX_VALUE || intVal < 0)
					{
						_emaConfigErrList.append( "service [" )
						.append( service.info().serviceName().toString() )
						.append( "] from the programmatically configure specifies out of range ServiceId [" )
						.append((int)intVal)
						.append("]. Will drop this service.")
						.create(Severity.ERROR);
						
						return false;
					}
	
					Service existingService = dirCache.getService((int)intVal);
					if (existingService != null && existingService != service)
					{
						_emaConfigErrList.append( "service [" )
						.append( service.info().serviceName().toString() )
						.append( "] from the programmatically configure specifies the same ServiceId [" )
						.append((int)intVal)
						.append("] as already specified by another service. Will drop this service.")
						.create(Severity.ERROR);
	
						return false;
					}
					
					service.serviceId((int)intVal);
				}
				else if (entry.name().equals("IsSource"))
				{
					intVal = entry.intValue();
					service.info().applyHasIsSource();
					service.info().isSource(intVal > 0 ? 1 : 0);
				}
				else if (entry.name().equals("SupportsQoSRange"))
				{
					intVal = entry.intValue();
					service.info().applyHasSupportsQosRange();
					service.info().supportsQosRange(intVal > 0 ? 1 : 0);
				}
				else if (entry.name().equals("SupportsOutOfBandSnapshots"))
				{
					intVal = entry.intValue();
					service.info().applyHasSupportsOutOfBandSnapshots();
					service.info().supportsOutOfBandSnapshots(intVal > 0 ? 1 : 0);
				}
				else if (entry.name().equals("AcceptingConsumerStatus"))
				{
					intVal = entry.intValue();
					service.info().applyHasAcceptingConsumerStatus();
					service.info().acceptingConsumerStatus(intVal > 0 ? 1 : 0);
				}
				else if (entry.name().equals("Timeliness"))
				{
					intVal = entry.intValue();
					if (intVal >= 0)
					{
						 if (intVal > Integer.MAX_VALUE)
						{
							_emaConfigErrList.append( "service [" )
							.append( service.info().serviceName().toString() )
							.append( "] from the programmatically configure specifies service QoS::Timeliness is greater than allowed maximum. Will use maximum Timeliness.")
							.append(" Suspect value is ").append(timeliness)
							.create(Severity.ERROR);
							
							timeliness = Integer.MAX_VALUE;
						}
						else
							timeliness = (int)intVal;
		
						service.info().applyHasQos();
						addQos = true;
					}
				}
				else if (entry.name().equals("Rate"))
				{
					intVal = entry.intValue();
					if (intVal >= 0)
					{
						if (intVal > Integer.MAX_VALUE)
						{
						 	_emaConfigErrList.append( "service [" )
							.append( service.info().serviceName().toString() )
							.append( "] from the programmatically configure specifies service QoS::Rate is greater than allowed maximum. Will use maximum Rate.")
							.append(" Suspect value is ").append(rate)
							.create(Severity.ERROR);
		
							rate = Integer.MAX_VALUE;
						}
						 else 
							 rate = (int)intVal;
	
						 service.info().applyHasQos();
						 addQos = true;
					}
				}
	
				break;
			case DataTypes.ARRAY: 
				if (entry.name().equals("DictionariesProvided"))
				{
					service.info().dictionariesProvidedList().clear();
					service.info().applyHasDictionariesProvided();
					for (OmmArrayEntry dEntry : entry.array())
					{
						if (dEntry.loadType() == DataTypes.ASCII)
							_dictProvided.add(dEntry.ascii().ascii());
					}
				}
				else if (entry.name().equals("DictionariesUsed"))
				{
					service.info().dictionariesUsedList().clear();
					service.info().applyHasDictionariesUsed();
					for (OmmArrayEntry dEntry : entry.array())
					{
						if (dEntry.loadType() == DataTypes.ASCII)
							_dictUsed.add(dEntry.ascii().ascii());
					}
				}
				else if (entry.name().equals("Capabilities"))
				{
					service.info().capabilitiesList().clear();
					for (OmmArrayEntry arrayEntry : entry.array())
					{
						if (arrayEntry.loadType() == DataTypes.ASCII)
						{
							Integer domainType = ConfigManager.convertDomainType(arrayEntry.ascii().ascii());
							if (domainType == null || domainType.intValue() > MAX_UNSIGNED_INT16 || domainType.intValue() < 0)
							{
								_emaConfigErrList.append( "service [" )
								.append( service.info().serviceName().toString() )
								.append( "] from the programmatically configure specifies the service which contains out of range capability. Will drop this capability. Suspect value is = " )
								.append(arrayEntry.ascii().ascii())
								.create(Severity.ERROR);
								
								continue;
							}
							else
							{
								if (!service.info().capabilitiesList().contains((Object)domainType))
									service.info().capabilitiesList().add((long)domainType.intValue());
							}
						}
						else if (arrayEntry.loadType() == DataTypes.INT)
						{
							long domainType = arrayEntry.intValue();
							if (domainType > MAX_UNSIGNED_INT16 || domainType < 0)
							{
								_emaConfigErrList.append( "service [" )
								.append( service.info().serviceName().toString() )
								.append( "] from the programmatically configure specifies the service which contains out of range capability. Will drop this capability. Suspect value is = " )
								.append((int)domainType)
								.create(Severity.ERROR);
								
								continue;
							}
							else
							{
								if (!service.info().capabilitiesList().contains(domainType))
									service.info().capabilitiesList().add(domainType);
							}
						}
					}
	
					if (service.info().capabilitiesList().isEmpty())
					{
						_emaConfigErrList.append( "service [" )
						.append( service.info().serviceName().toString() )
						.append( "] from the programmatically configure specifies  the service which contains no capabilities. Will drop this service." )
						.create(Severity.ERROR);
						
						return false;
					}
	
					if (!service.info().capabilitiesList().isEmpty())
						Collections.sort(service.info().capabilitiesList());
				}
	
				break;
			case DataTypes.ELEMENT_LIST: 
				if (entry.name().equals("Status"))
				{
					if (entry.loadType() == DataTypes.ELEMENT_LIST)
					{
						ElementList statusEntryInfo = entry.elementList();
						retrieveServiceInfo(service, statusEntryInfo, dirCache, serviceDictionaryConfigList);
					}
					else
					{
						_emaConfigErrList.append( "service [" )
						.append( service.info().serviceName().toString() )
						.append( "] from the programmatically configure specifies the service status which contains invalid data type. Suspect value is = " )
						.append(entry.loadType())
						.create(Severity.ERROR);
					}
				}
	
				break;
			case DataTypes.SERIES: 
				if (entry.name().equals("QoS"))
				{
					service.info().qosList().clear();
					for (SeriesEntry qosEntry : entry.series())
					{
						if (qosEntry.loadType() == DataTypes.ELEMENT_LIST)
						{
							retrieveServiceInfo(service, qosEntry.elementList(), dirCache, serviceDictionaryConfigList);
						}
					}
				}
				break;
			default:
				break;
			};
		}
	
		if (service.info().checkHasQos() && addQos)
		{
			Qos rsslQos = CodecFactory.createQos();
			Utilities.toRsslQos(rate, timeliness, rsslQos);
			service.info().qosList().add(rsslQos);
			addQos = false;
		}
	
		return true;
	}
	
	void retrieveGroupAndListName( Map map )
	{
		if ( _setGroup )
			return;
	
		for (MapEntry mapEntry : map)
		{
			if ( mapEntry.key().dataType() == DataTypes.ASCII )
				if ( mapEntry.key().ascii().ascii().equals("ConsumerGroup") )
				{
					_group = "ConsumerGroup";
					_list = "ConsumerList";
					_setGroup = true;
					break;
				}
				else if ( mapEntry.key().ascii().ascii().equals("NiProviderGroup") )
				{
					_group = "NiProviderGroup";
					_list = "NiProviderList";
					_setGroup = true;
					break;
				}
				else if (mapEntry.key().ascii().ascii().equals("IProviderGroup") )
				{
					_group = "IProviderGroup";
					_list = "IProviderList";
					_setGroup = true;
					break;
				}
		}
	}
	
	int convertToInt(long value)
	{
		return (int)(value > Integer.MAX_VALUE ? Integer.MAX_VALUE : value);
	}
	
	private int convertToEnum(String value)
	{
		int colonPosition = value.indexOf("::");
		if (colonPosition == -1) 
		{
			_emaConfigErrList.append( "invalid Enum value format [" )
			.append( value )
			.append( "]; expected typename::value (e.g., OperationModel::ApiDispatch)" )
			.create(Severity.ERROR);
			return INVALID_RETVAL;
		}

		String enumType = value.substring(0, colonPosition);
		String enumValue = value.substring(colonPosition + 2, value.length());

		if ( enumType.equals("DictionaryType"))
		{
			int localDictonary = 0;

			if(enumValue.equals("FileDictionary"))
				localDictonary = 1;
			else if(enumValue.equals("ChannelDictionary"))
				localDictonary = 0;
			else
			{
				_emaConfigErrList.append( "no conversion in convertToEnum for enumType [" )
				.append( enumValue )
				.append( "]")
				.create(Severity.ERROR);
			}

			return localDictonary;
		}
		else if ( enumType.equals("ChannelType" ) )
		{
			int channelType = INVALID_RETVAL;

			if(enumValue.equals("RSSL_SOCKET"))
				channelType = ConnectionTypes.SOCKET;
			else if(enumValue.equals("RSSL_HTTP"))
				channelType = ConnectionTypes.HTTP;
			else if(enumValue.equals("RSSL_ENCRYPTED"))
				channelType = ConnectionTypes.ENCRYPTED;
			else if(enumValue.equals("RSSL_RELIABLE_MCAST"))
				channelType = ConnectionTypes.RELIABLE_MCAST;
			else
			{
				_emaConfigErrList.append( "no conversion in convertToEnum for enumType [" )
				.append( enumValue )
				.append( "]")
				.create(Severity.ERROR);
			}
			
			return channelType;
		}
		else if ( enumType.equals("ServerType" ) )
		{
			int serverType = INVALID_RETVAL;

			if(enumValue.equals("RSSL_SOCKET"))
				serverType = ConnectionTypes.SOCKET;
			else
			{
				_emaConfigErrList.append( "no conversion in convertToEnum for enumType [" )
				.append( enumValue )
				.append( "]")
				.create(Severity.ERROR);
			}
			
			return serverType;
		}
		else if (enumType.equals("CompressionType") )
		{
			int compressionType = INVALID_RETVAL;

			if(enumValue.equals("None"))
				compressionType = CompressionTypes.NONE;
			else if(enumValue.equals("ZLib"))
				compressionType = CompressionTypes.ZLIB;
			else if(enumValue.equals("LZ4"))
				compressionType = CompressionTypes.LZ4;
			else
			{
				_emaConfigErrList.append( "no conversion in convertToEnum for enumType [" )
				.append( enumValue )
				.append( "]")
				.create(Severity.ERROR);
			}

			return compressionType;
		}
		else if (enumType.equals("StreamState") )
		{
			int streamState = INVALID_RETVAL;

			if(enumValue.equals("Open"))
				streamState = OmmState.StreamState.OPEN;
			else if(enumValue.equals("NonStreaming"))
				streamState = OmmState.StreamState.NON_STREAMING;
			else if(enumValue.equals("ClosedRecover") || enumValue.equals("CloseRecover"))
				streamState = OmmState.StreamState.CLOSED_RECOVER;
			else if(enumValue.equals("Closed") || enumValue.equals("Close"))
				streamState = OmmState.StreamState.CLOSED;
			else if(enumValue.equals("ClosedRedirected") || enumValue.equals("CloseRedirected"))
				streamState = OmmState.StreamState.CLOSED_REDIRECTED;
			else
			{
				_emaConfigErrList.append( "no conversion in convertToEnum for enumType [" )
				.append( enumValue )
				.append( "]")
				.create(Severity.ERROR);
			}
			
			return streamState;
		}
		else if (enumType.equals("DataState") )
		{
			int dataState = INVALID_RETVAL;

			if(enumValue.equals("NoChange"))
				dataState = OmmState.DataState.NO_CHANGE;
			else if(enumValue.equals("Ok"))
				dataState = OmmState.DataState.OK;
			else if(enumValue.equals("Suspect"))
				dataState = OmmState.DataState.SUSPECT;
			else
			{
				_emaConfigErrList.append( "no conversion in convertToEnum for enumType [" )
				.append( enumValue )
				.append( "]")
				.create(Severity.ERROR);
			}
			
			return dataState;
		}
		else if (enumType.equals("StatusCode") )
		{
			int statusCode = INVALID_RETVAL;
			
			if(enumValue.equals("None"))
				statusCode = OmmState.StatusCode.NONE;
			else if(enumValue.equals("NotFound"))
				statusCode = OmmState.StatusCode.NOT_FOUND;
			else if(enumValue.equals("Timeout"))
				statusCode = OmmState.StatusCode.TIMEOUT;
			else if(enumValue.equals("NotAuthorized"))
				statusCode = OmmState.StatusCode.NOT_AUTHORIZED;
			else if(enumValue.equals("InvalidArgument"))
				statusCode = OmmState.StatusCode.INVALID_ARGUMENT;
			else if(enumValue.equals("UsageError"))
				statusCode = OmmState.StatusCode.USAGE_ERROR;
			else if(enumValue.equals("Preempted"))
				statusCode = OmmState.StatusCode.PREEMPTED;
			else if(enumValue.equals("JustInTimeConflationStarted"))
				statusCode = OmmState.StatusCode.JUST_IN_TIME_CONFLATION_STARTED;
			else if(enumValue.equals("TickByTickResumed"))
				statusCode = OmmState.StatusCode.TICK_BY_TICK_RESUMED;
			else if(enumValue.equals("FailoverStarted"))
				statusCode = OmmState.StatusCode.FAILOVER_STARTED;
			else if(enumValue.equals("FailoverCompleted"))
				statusCode = OmmState.StatusCode.FAILOVER_COMPLETED;
			else if(enumValue.equals("GapDetected"))
				statusCode = OmmState.StatusCode.GAP_DETECTED;
			else if(enumValue.equals("NoResources"))
				statusCode = OmmState.StatusCode.NO_RESOURCES;
			else if(enumValue.equals("TooManyItems"))
				statusCode = OmmState.StatusCode.TOO_MANY_ITEMS;
			else if(enumValue.equals("AlreadyOpen"))
				statusCode = OmmState.StatusCode.ALREADY_OPEN;
			else if(enumValue.equals("SourceUnknown"))
				statusCode = OmmState.StatusCode.SOURCE_UNKNOWN;
			else if(enumValue.equals("NotOpen"))
				statusCode = OmmState.StatusCode.NOT_OPEN;
			else if(enumValue.equals("NonUpdatingItem"))
				statusCode = OmmState.StatusCode.NON_UPDATING_ITEM;
			else if(enumValue.equals("UnsupportedViewType"))
				statusCode = OmmState.StatusCode.UNSUPPORTED_VIEW_TYPE;
			else if(enumValue.equals("InvalidView"))
				statusCode = OmmState.StatusCode.INVALID_VIEW;
			else if(enumValue.equals("FullViewProvided"))
				statusCode = OmmState.StatusCode.FULL_VIEW_PROVIDED;
			else if(enumValue.equals("UnableToRequestAsBatch"))
				statusCode = OmmState.StatusCode.UNABLE_TO_REQUEST_AS_BATCH;
			else if(enumValue.equals("NoBatchViewSupportInReq"))
				statusCode = OmmState.StatusCode.NO_BATCH_VIEW_SUPPORT_IN_REQ;
			else if(enumValue.equals("ExceededMaxMountsPerUser"))
				statusCode = OmmState.StatusCode.EXCEEDED_MAX_MOUNTS_PER_USER;
			else if(enumValue.equals("Error"))
				statusCode = OmmState.StatusCode.ERROR;
			else if(enumValue.equals("DacsDown"))
				statusCode = OmmState.StatusCode.DACS_DOWN;
			else if(enumValue.equals("UserUnknownToPermSys"))
				statusCode = OmmState.StatusCode.USER_UNKNOWN_TO_PERM_SYS;
			else if(enumValue.equals("DacsMaxLoginsReached"))
				statusCode = OmmState.StatusCode.DACS_MAX_LOGINS_REACHED;
			else if(enumValue.equals("DacsUserAccessToAppDenied"))
				statusCode = OmmState.StatusCode.DACS_USER_ACCESS_TO_APP_DENIED;
			else if(enumValue.equals("GapFill"))
				statusCode = OmmState.StatusCode.GAP_FILL;
			else if(enumValue.equals("AppAuthorizationFailed"))
				statusCode = OmmState.StatusCode.APP_AUTHORIZATION_FAILED;
			else
			{
				_emaConfigErrList.append( "no conversion in convertToEnum for enumType [" )
				.append( enumValue )
				.append( "]")
				.create(Severity.ERROR);
			}
			
			return statusCode;
		}
		else if ( enumType.equals("Timeliness") )
		{
			int timeliness = INVALID_RETVAL;

			if(enumValue.equals("RealTime"))
				timeliness = OmmQos.Timeliness.REALTIME;
			else if(enumValue.equals("InexactDelayed"))
				timeliness = OmmQos.Timeliness.INEXACT_DELAYED;
			else
			{
				try
				{
					return (int)Long.parseLong(enumValue);
				}
				catch(NumberFormatException excp)
				{
					_emaConfigErrList.append( "no conversion in convertToEnum for enumType [" )
					.append( enumValue )
					.append( "]")
					.create(Severity.ERROR);
				}
			}
			
			return timeliness;
		}
		else if ( enumType.equals("Rate") )
		{
			int rate = INVALID_RETVAL;

			if(enumValue.equals("TickByTick"))
				rate = OmmQos.Rate.TICK_BY_TICK;
			else if(enumValue.equals("JustInTimeConflated"))
				rate = OmmQos.Rate.JUST_IN_TIME_CONFLATED;
			else
			{
				try
				{
					return (int)Long.parseLong(enumValue);
				}
				catch(NumberFormatException excp)
				{
					_emaConfigErrList.append( "no conversion in convertToEnum for enumType [" )
					.append( enumValue )
					.append( "]")
					.create(Severity.ERROR);
				}
			}
			
			return rate;
		}
		else 
		{
			_emaConfigErrList.append( "no conversion in convertToEnum for enumType [" )
			.append( enumValue )
			.append( "]")
			.create(Severity.ERROR);
			return INVALID_RETVAL;
		}
	}

	ServiceDictionaryConfig findServiceDictConfig(List<ServiceDictionaryConfig> serviceDictionaryConfigList, int serviceId)
	{
		if (serviceDictionaryConfigList != null)
		{
			for (ServiceDictionaryConfig serviceDictionaryConfig : serviceDictionaryConfigList)
			{
				if (serviceDictionaryConfig.serviceId == serviceId)
					return serviceDictionaryConfig;
			}
		}

		return null;
	}

	void removeConfigFileService(DirectoryServiceStore dirServiceStore, DirectoryCache directoryCache)
	{
		if (_serviceNameList.size() == 0)
			return;

		for (int i = 0; i < directoryCache.serviceList().size(); )
		{
			Service service = directoryCache.serviceList().get(i);
			boolean found = false;
			for (String serviceName : _serviceNameList)
			{
				if (serviceName.equals(service.info().serviceName().toString()))
				{
					found = true;
					break;
				}
			}
			
			if (!found)
			{
				int serviceId = service.serviceId();
				directoryCache.removeService(serviceId);
				dirServiceStore.remove(serviceId);
			}
			else
				i++;
		}
	}
}
