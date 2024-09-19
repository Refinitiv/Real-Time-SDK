///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019, 2024 LSEG. All rights reserved.     
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.nio.file.FileSystems;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Set;
import java.util.ArrayList;
import java.util.Hashtable;
import java.util.List;

import javax.xml.XMLConstants;
import javax.xml.transform.stream.StreamSource;
import javax.xml.validation.Schema;
import javax.xml.validation.SchemaFactory;
import javax.xml.validation.Validator;

import org.apache.commons.configuration2.ex.ConfigurationException;
import org.apache.commons.configuration2.XMLConfiguration;

import com.refinitiv.ema.access.ConfigManager.Branch;
import com.refinitiv.ema.access.ConfigManager.ConfigAttributes;
import com.refinitiv.ema.access.ConfigManager.ConfigElement;
import com.refinitiv.ema.access.ConfigManager.TagDictionary;
import com.refinitiv.ema.access.OmmLoggerClient.Severity;
import com.refinitiv.eta.transport.CompressionTypes;
import com.refinitiv.eta.transport.ConnectionTypes;

import org.apache.commons.configuration2.io.FileHandler;
import org.apache.commons.configuration2.tree.ImmutableNode;
import org.apache.commons.configuration2.tree.NodeHandler;

import org.xml.sax.SAXException;

class ConfigReader 
{
	static ConfigReader configReaderFactory;

	static String defaultXsdFileName = "EmaConfig.xsd";

	EmaConfigBaseImpl _parent;
	
	ConfigReader() 
	{

	}

	ConfigReader(EmaConfigBaseImpl parent)
	{
		this._parent = parent;
	}

	ConfigErrorTracker errorTracker() 
	{
		return _parent.errorTracker();
	}

	String defaultConsumerName() 
	{
		return null;
	}

	static ConfigReader acquire() 
	{
		if( configReaderFactory == null )
			configReaderFactory = new ConfigReader();

		return configReaderFactory;
	}

	static ConfigReader createXMLConfigReader(EmaConfigBaseImpl emaConfigImpl)
	{
		return(ConfigReader.acquire().new XMLConfigReader(emaConfigImpl));
	}

	class XMLnode 
	{
		private XMLnode _parent;
		private String _name;
		private int _tagId;
		private ConfigAttributes _attributeList;
		private ArrayList <XMLnode> _children;
		private int _level;
		ConfigErrorTracker _myErrorTracker;
		
		XMLnode(String name, int level, XMLnode  parent, int tagId)
		{
			_name = name;
			_parent = parent;
			_attributeList = ConfigManager.acquire().createConfigAttributes();
			_children = new ArrayList<XMLnode>();
			
			_level = level;
			_tagId = tagId;
		}
		
		void setErrorTracker(ConfigErrorTracker errorTracker) 
		{
			_myErrorTracker = errorTracker;
		}

		ConfigErrorTracker errorTracker()
		{
			if(_parent == null)
				return _myErrorTracker;
			else
				return(_parent.errorTracker());
		}
		
		void addChild(XMLnode child)
		{
			_children.add(child);
		}
		
		void addAttribute(ConfigElement element)
		{
			_attributeList.put(element._tagId,element);
		}
		
		ConfigAttributes attributeList()
		{
			return _attributeList;
		}
			
		ArrayList <XMLnode> children()
		{
			return _children;
		}

		String name() 
		{ 
			return _name; 
		}
		
		int tagId()
		{
			return _tagId;
		}
		
		int level() 
		{ 
			return _level; 
		}
			
		int size()
		{
			return _children.size(); 
		}
		
		XMLnode parent()
		{
			return _parent;
		}

		void dump(int level) 
		{
			int spaceCount = level * 2;
			
			String space = "";
			for(int i = 0; i < spaceCount; i++)
			{
				space += " ";
			}
			System.out.format("%s%s\n",space,name());
			
			_attributeList.dump(space);
			
			if( _children.size() > 0 )
			{
				++level;
				for( int i = 0; i < _children.size(); i++)
				{
					XMLnode child = _children.get(i);
					child.dump(level);
				}
			}
		}

		XMLnode getChild(int nodeId)
		{
			for( int i = 0; i < _children.size(); i++)
			{
				XMLnode child = _children.get(i);

				// debugging
				//System.out.println("Child Name = "+child._name);
				if( child._tagId == nodeId )
				{
					return child;
				}
			}

			return null;
		}

		XMLnode getChildren(Branch branch, int startIdx)
		{
			XMLnode currentNode = this;

			int endIdx = branch.size();
			
			for( int i = startIdx; i < endIdx; i++)
			{
				int nodeId = branch.get(i);
				
				if ( currentNode == null || currentNode._children.isEmpty() )
					return null;

				currentNode = currentNode.getChild(nodeId);
				if( i == endIdx-1)
					return currentNode;
			}
			return null;
		}

		Object getNode(Branch branch, int startIdx)
		{
			XMLnode xmlChildNode = this;

			int endIdx = branch.size();
			Object attributeValue = null;
			
			for( int i = startIdx; i < endIdx; )
			{
				int nodeId = branch.get(i);
				xmlChildNode = xmlChildNode.getChild(nodeId);

				if (xmlChildNode == null)
				{
					attributeValue = _attributeList.getValue(nodeId);
					if( attributeValue == null )
					{
						errorTracker().append("Unable to find attribute -")
									  .append(branch.nodeAsString(nodeId))
									  .append("- in branch ")
									  .append(branch.toString())
									  .create(Severity.ERROR);
					}
					return attributeValue;
				}

				if( i == endIdx-1 )
				{
					return xmlChildNode;
				}

				Object nextNode = xmlChildNode.getNode(branch,startIdx+1);
				if ( nextNode == null) 
				{
					errorTracker().append("Unable to find child node for ").append(branch.toString()).create(Severity.ERROR);
				}
				
				return nextNode;
			}

			return null;
		}

		ConfigAttributes getNodeWithAttributeList(Branch branch){
			XMLnode list = getChildren(branch,0);
			if(list == null){
				return null;
			}
			return list._attributeList;
		}
		
		ConfigAttributes getNodeWithAttributeList(Branch branch,String nodeName,int attributeId)
		{
			XMLnode list = getChildren(branch,0);
			
			if( list == null )
			{
				return null;
			}

			if( list._children.size() == 0 )
			{
				return null;
			}

			for( int i = 0; i < list._children.size(); i++)
			{
				XMLnode child = list._children.get(i);
				
				ConfigElement actualNodeName = (ConfigElement) child._attributeList.getElement(attributeId);
				if( actualNodeName != null && actualNodeName.asciiValue().equals(nodeName) )
				{
					return child._attributeList;
				}
			}
			return null;
		}
	}

	class XMLConfigReader extends ConfigReader
	{
		String _configFileName;
		String _configFileLocation;
		
		private Hashtable<String, Integer> configkeyTypePair;
		private int level;

		private XMLnode xmlRoot;

		private String _defaultConsumerName;
		private String _defaultNiProviderName;
		private String _defaultIProviderName;
		private XMLnode _defaultConsumerNode;
		private XMLnode	_defaultNiProviderNode;
		private XMLnode	_defaultIProviderNode;
		private boolean _getDefaultName = false;

		boolean _debugDump = false;

		XMLConfigReader(EmaConfigBaseImpl emaConfigImpl)
		{
			super(emaConfigImpl);
		}

		String defaultConsumerName() 
		{
			if(!_getDefaultName)
			{
				verifyAndGetDefaultConsumer();
				_getDefaultName = true;
			}
			
			return _defaultConsumerName;
		}
		
		String defaultNiProviderName()
		{
			if(!_getDefaultName)
			{
				verifyAndGetDefaultNiProvider();
				_getDefaultName = true;
			}
			
			return _defaultNiProviderName;
		}
		
		String defaultIProviderName()
		{
			if(!_getDefaultName)
			{
				verifyAndGetDefaultIProvider();
				_getDefaultName = true;
			}
			
			return _defaultIProviderName;
		}

        	XMLnode getRootNode()
		{
			return xmlRoot;
        	}

		private ConfigElement convertEnum(XMLnode parent, String enumValue)
		{
			int colonPosition = enumValue.indexOf("::");
			if (colonPosition == -1) 
			{
				errorTracker().append( "invalid Enum value format [" )
				.append( enumValue )
				.append( "]; expected typename::value (e.g., OperationModel::ApiDispatch)" )
				.create(Severity.ERROR);
				return null;
			}

			String enumType = enumValue.substring(0, colonPosition);
			enumValue = enumValue.substring(colonPosition + 2, enumValue.length());

			if ( enumType.equals("LoggerSeverity"))
			{
				return ConfigManager.acquire().new IntConfigElement( parent, ConfigElement.Type.Enum,0);
			}
			else if ( enumType.equals("LoggerType"))
			{
				return ConfigManager.acquire().new IntConfigElement(parent, ConfigElement.Type.Enum,0);
			}
			else if ( enumType.equals("DictionaryType"))
			{
				int localDictonary = 0;

				if(enumValue.equals("FileDictionary"))
					localDictonary = 1;
				else if(enumValue.equals("ChannelDictionary"))
					localDictonary = 0;
				else
				{
					errorTracker().append( "no implementation in convertEnum for enumType [" )
					.append( enumValue )
					.append( "]")
					.create(Severity.ERROR);
				}

				return ConfigManager.acquire().new IntConfigElement( parent, ConfigElement.Type.Enum,localDictonary);
			}
			else if ( enumType.equals("ChannelType" ) )
			{
				int channelType = -1;

				if(enumValue.equals("RSSL_SOCKET"))
					channelType = ConnectionTypes.SOCKET;
				else if(enumValue.equals("RSSL_HTTP"))
					channelType = ConnectionTypes.HTTP;
				else if(enumValue.equals("RSSL_ENCRYPTED"))
					channelType = ConnectionTypes.ENCRYPTED;
				else if(enumValue.equals("RSSL_RELIABLE_MCAST"))
					channelType = ConnectionTypes.RELIABLE_MCAST;
				else if (enumValue.equals("RSSL_WEBSOCKET")) {
					channelType = ConnectionTypes.WEBSOCKET;
				} else {
					errorTracker().append( "no implementation in convertEnum for enumType [" )
					.append( enumValue )
					.append( "]")
					.create(Severity.ERROR);
				}
				
				if( channelType != -1 )
					return ConfigManager.acquire().new IntConfigElement( parent, ConfigElement.Type.Enum,channelType);
			}
			else if ( enumType.equals("EncryptedProtocolType" ) )
			{
				int channelType = -1;

				if(enumValue.equals("RSSL_SOCKET"))
					channelType = ConnectionTypes.SOCKET;
				else if(enumValue.equals("RSSL_HTTP"))
					channelType = ConnectionTypes.HTTP;
				else if (enumValue.equals("RSSL_WEBSOCKET")) {
					channelType = ConnectionTypes.WEBSOCKET;
				}
				else
				{
					errorTracker().append( "no implementation in convertEnum for enumType [" )
					.append( enumValue )
					.append( "]")
					.create(Severity.ERROR);
				}
				
				if( channelType != -1 )
					return ConfigManager.acquire().new IntConfigElement( parent, ConfigElement.Type.Enum,channelType);
			}
			else if ( enumType.equals("ServerType" ) ) {
				int serverType = -1;

				if (enumValue.equals("RSSL_SOCKET") || enumValue.equals("RSSL_WEBSOCKET")) {
					serverType = ConnectionTypes.SOCKET;
				}
				else if(enumValue.equals("RSSL_ENCRYPTED") )
					serverType = ConnectionTypes.ENCRYPTED;
				else
				{
					errorTracker().append( "no implementation in convertEnum for enumType [" )
					.append( enumValue )
					.append( "]")
					.create(Severity.ERROR);
				}
				
				if( serverType != -1 )
					return ConfigManager.acquire().new IntConfigElement( parent, ConfigElement.Type.Enum,serverType);
			}
			else if (enumType.equals("CompressionType") )
			{
				int compressionType = -1;

				if(enumValue.equals("None"))
					compressionType = CompressionTypes.NONE;
				else if(enumValue.equals("ZLib"))
					compressionType = CompressionTypes.ZLIB;
				else if(enumValue.equals("LZ4"))
					compressionType = CompressionTypes.LZ4;
				else
				{
					errorTracker().append( "no implementation in convertEnum for enumType [" )
					.append( enumValue )
					.append( "]")
					.create(Severity.ERROR);
				}

				if( compressionType != -1 )
					return ConfigManager.acquire().new IntConfigElement( parent, ConfigElement.Type.Enum,compressionType);
			}
			else if (enumType.equals("StreamState") )
			{
				int streamState = -1;

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
					errorTracker().append( "no implementation in convertEnum for enumType [" )
					.append( enumValue )
					.append( "]")
					.create(Severity.ERROR);
				}
				
				if( streamState != -1 )
					return ConfigManager.acquire().new IntConfigElement( parent, ConfigElement.Type.Enum,streamState);
			}
			else if (enumType.equals("DataState") )
			{
				int dataState = -1;

				if(enumValue.equals("NoChange"))
					dataState = OmmState.DataState.NO_CHANGE;
				else if(enumValue.equals("Ok"))
					dataState = OmmState.DataState.OK;
				else if(enumValue.equals("Suspect"))
					dataState = OmmState.DataState.SUSPECT;
				else
				{
					errorTracker().append( "no implementation in convertEnum for enumType [" )
					.append( enumValue )
					.append( "]")
					.create(Severity.ERROR);
				}
				
				if( dataState != -1 )
					return ConfigManager.acquire().new IntConfigElement( parent, ConfigElement.Type.Enum,dataState);
			}
			else if (enumType.equals("StatusCode") )
			{
				int statusCode = -1;
				
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
					errorTracker().append( "no implementation in convertEnum for enumType [" )
					.append( enumValue )
					.append( "]")
					.create(Severity.ERROR);
				}
				
				if( statusCode != -1 )
					return ConfigManager.acquire().new IntConfigElement( parent, ConfigElement.Type.Enum,statusCode);
			}
			else 
			{
				errorTracker().append( "no implementation in convertEnum for enumType [" )
				.append( enumValue )
				.append( "]")
				.create(Severity.ERROR);
				return null;
			}

			return null;
		}

		private ConfigElement makeConfigEntry( XMLnode parent, String name, String value, int tagId )
		{
			Object oConfigId = configkeyTypePair.get(name);
			if( oConfigId == null )
			{
				String errorMsg = String.format("unsupported configuration element [%s]",name);
				throw _parent.oommICExcept().message( errorMsg );
			}
			
			int elementType = (int) oConfigId;
			
			ConfigElement e = null;
			switch( elementType )
			{
			case ConfigElement.Type.Ascii:
			{
				e = ConfigManager.acquire().new AsciiConfigElement( parent, value );
				break;
			}
			case ConfigElement.Type.Enum:
			{
				e = convertEnum(parent, value);
				break;
			}
			case ConfigElement.Type.Int64:
			{
				e = convertInt(name, parent, value);
				break;
			}

			case ConfigElement.Type.UInt64:
			{
				e = convertUInt(name, parent, value);
				break;
			}
			
			case ConfigElement.Type.Double:
			{
				e = convertDouble(name, parent, value);
				break;
			}

			default:
				errorTracker().append( "config element [")
				.append(name)
				.append("] had unexpected elementType [")
				.append(elementType)
				.append("; element ignored")
				.create(Severity.ERROR);
				break;
			}

			if (e != null)
			{
				e._name = name;
				e._tagId = tagId;
				e._valueStr = value;
				e._type = elementType;
			}
			
			return e;
		}

		private ConfigElement convertInt( String name, XMLnode parent, String value)
		{
			try
			{
				int convertedInt = Integer.parseInt(value);
				ConfigElement e = ConfigManager.acquire().new IntConfigElement( parent, ConfigElement.Type.Int64, convertedInt);
				return e;
			}
			catch(NumberFormatException exception)
			{
				errorTracker().append( "value [").append(value).append("] for config element [").append(name)
				.append("] is not a signed integer; element ignored").create(Severity.ERROR);
			}

			return null;
		}

		private ConfigElement convertUInt( String name,XMLnode parent, String value)
		{
			try
			{
				long convertedUInt = Long.parseLong(value);
				ConfigElement e = ConfigManager.acquire().new LongConfigElement( parent, ConfigElement.Type.UInt64, convertedUInt );
				return e;
			}
			catch(NumberFormatException exception)
			{
				errorTracker().append( "value [").append(value).append("] for config element [").append(name)
				.append("] is not a signed integer; element ignored").create(Severity.ERROR);
			}

			return null;
		}
		
		private ConfigElement convertDouble( String name,XMLnode parent, String value)
		{
			try
			{
				double convertedDouble = Double.parseDouble(value);
				ConfigElement e = ConfigManager.acquire().new DoubleConfigElement( parent, ConfigElement.Type.Double, convertedDouble );
				return e;
			}
			catch(NumberFormatException exception)
			{
				errorTracker().append( "value [").append(value).append("] for config element [").append(name)
				.append("] is not a double; element ignored").create(Severity.ERROR);
			}

			return null;
		}

		private ConfigElement handleConfigEntry(ImmutableNode nodePtr, XMLnode theNode, int tagId)
		{
			Map<String, Object> attributeList = nodePtr.getAttributes();

			String attributeValue = null;

			for (String attributeName : attributeList.keySet())
			{
				if(attributeName.equalsIgnoreCase("value") )
				{
					attributeValue = (String) attributeList.get(attributeName);
				}
				else
				{
					errorTracker().append("Unknown attributeName ").append(attributeName).create(Severity.ERROR);
				}
			}

			ConfigElement e = makeConfigEntry(theNode.parent(), nodePtr.getNodeName(), attributeValue, tagId);
			return e;
		}

		private void processNode(XMLnode theNode, ImmutableNode nodePtr, TagDictionary tagDict)
		{
			List<ImmutableNode> children = nodePtr.getChildren();
			if(children == null || children.size() == 0 )
			{
				errorTracker().append("No children for ").append(nodePtr.getNodeName()).create(Severity.ERROR);
				return;
			}

			for (int i = 0; i < children.size(); i++)
			{
				ImmutableNode configNodeChild = children.get(i);

				if( configNodeChild.getAttributes() != null && configNodeChild.getAttributes().size() > 0 )
				{
					Integer tagId = tagDict.get(configNodeChild.getNodeName());
					if( tagId == null ) 
					{
						if( configNodeChild.getNodeName().equalsIgnoreCase("pipeport"))
						{
							// unsupported in emaj; ignore 
						}
						else
						{
							errorTracker().append("Unable to find tagId for ")
							.append(configNodeChild.getNodeName())
							.create(Severity.ERROR);
						}
					}
					else
					{
						if(_debugDump) debugDump(padLeft(String.format("attribute Count: %d", configNodeChild.getAttributes() != null ? configNodeChild.getAttributes().size() : 0),level+4));
						ConfigElement ce = handleConfigEntry(configNodeChild,theNode,tagId); 

						if( ce != null )
						{
							if(_debugDump) debugDump(padLeft(String.format("add attribute: %s to %s; [tagId: %s - %d", ce._name, theNode.name(), ce._name,tagId),level + 6));
							theNode.addAttribute(ce);
						}
					}
					continue;
				}
				else
				{
					level++;

					if(_debugDump) debugDump(padLeft(String.format("Create XML Node: %s - level: %d",configNodeChild.getNodeName(),level),level));

					if( level == 2)
					{
						if( configNodeChild.getNodeName().equals("ConsumerGroup"))
						{
							tagDict = ConfigManager.ConsumerTagDict;
						}
						else if( configNodeChild.getNodeName().equals("IProviderGroup"))
						{
							tagDict = ConfigManager.IProviderTagDict;
						}
						else if ( configNodeChild.getNodeName().equals("NiProviderGroup") )
						{
							tagDict = ConfigManager.NiProviderTagDict;
						}
						else if( configNodeChild.getNodeName().equals("ChannelGroup"))
						{
							tagDict = ConfigManager.ChannelTagDict;
						}
						else if( configNodeChild.getNodeName().equals("ServerGroup"))
						{
							tagDict = ConfigManager.ServerTagDict;
						}
						else if( configNodeChild.getNodeName().equals("LoggerGroup"))
						{
							skipNode(configNodeChild);
							level--;
							continue;
						}
						else if( configNodeChild.getNodeName().equals("DirectoryGroup"))
						{
							tagDict = ConfigManager.DirectoryTagDict;
						}
						else if( configNodeChild.getNodeName().equals("DictionaryGroup"))
						{
							tagDict = ConfigManager.DictionaryTagDict;
						}
						else if( configNodeChild.getNodeName().equals("GlobalConfig"))
						{
							tagDict = ConfigManager.GlobalConfigDict;
						}
						else if ( configNodeChild.getNodeName().equals("WarmStandbyGroup"))
						{
							tagDict = ConfigManager.WarmStandbyGroupDict;
						}
						else if ( configNodeChild.getNodeName().equals("WarmStandbyServerInfoGroup"))
						{
							tagDict = ConfigManager.WarmStandbyServerDict;
						}
					}
					
					if ( level == 5 )
					{
						if( configNodeChild.getNodeName().equals("Service"))
						{
							tagDict = ConfigManager.ServiceTagDict;
						}
					}

					if( tagDict == null )
					{
						errorTracker().append("Tag dict null for ").append(configNodeChild.getNodeName()).create(Severity.ERROR);
					}

					if( tagDict.isEmpty() )
					{
						errorTracker().append("Tag dict is EMPTY for ").append(configNodeChild.getNodeName()).create(Severity.ERROR);
					}

					Integer tagKey = tagDict.get(configNodeChild.getNodeName());
					if( tagKey == null )
					{
						errorTracker().append("Unable to find tagId for ").append(configNodeChild.getNodeName()).create(Severity.ERROR);
					}
					else
					{
						int tagId = tagKey.intValue();

						if(_debugDump) debugDump(padLeft(String.format("Tag %s - %d", configNodeChild.getNodeName(),tagId),level+2));

						XMLnode xmlChild = new XMLnode(configNodeChild.getNodeName(), level, theNode,tagId);
						processNode(xmlChild,configNodeChild,tagDict);
						if ((configNodeChild.getAttributes() == null || configNodeChild.getAttributes().size() == 0)  && configNodeChild.getChildren() != null && configNodeChild.getChildren().size() > 0)
						{
							theNode.addChild(xmlChild);
						}
					}
				}

				level--;
			}
		}

		private void skipNode(ImmutableNode nodePtr)
		{
			List<ImmutableNode> children = nodePtr.getChildren();
			for (int i = 0; i < children.size(); i++)
			{
				ImmutableNode configNodeChild = children.get(i);

				if( configNodeChild.getAttributes() != null && configNodeChild.getAttributes().size() > 0 )
				{
					continue;
				}
				else
				{
					skipNode(configNodeChild);
				}
			}
		}

		/**
		 * read and parse a configuration file
		 *
		 * if parameter path is empty, attempt to read and parse file EmaConfig.xml in the
		 * current working directory.
		 *
		 * if parameter path is not empty, attempt to read configuration from a classpath
		 * resource located at the specified path.
		 *
		 * if no classpath resource exists at the path, attempt to read the configuration
		 * from path if it is a file or file path/EmaConfig.xml if path is a directory.
		 *
		 * If a configuration cannot be constructed from the given path, throw an
		 * OmmInvalidConfigurationException exception
		 *
		 * @param path  XML Configuration file name
		 * 
		 * @throws {@link OmmInvalidConfigurationException}
		 */
		protected void loadFile(String pathToXml) throws OmmInvalidConfigurationException
		{
			final String defaultXmlFileName = "EmaConfig.xml";
			
			// eventual location of configuration file
			String xmlFileName = getValidatedFileName(pathToXml, defaultXmlFileName);

			if (xmlFileName != null && !xmlFileName.isEmpty())
			{
				try
				{
                    // validate XML Config only when the XML Schema file is present in the current working directory
					if (Files.exists(Paths.get(defaultXsdFileName)))
					{
						Validator validator = initValidator(defaultXsdFileName);
						validator.validate(new StreamSource(new File(xmlFileName)));
					}
				}
				catch (IOException | SAXException e)
				{
					throw _parent.oommICExcept().message(e.getMessage());
				}
			}

			XMLConfiguration config = new XMLConfiguration();

			try
			{
				FileHandler fh = new FileHandler(config);
				if (pathToXml == null || pathToXml.isEmpty())
				{
					InputStream in = ClassLoader.class.getResourceAsStream("/".concat(defaultXmlFileName));
					if (in == null)
					{
						fh.setFileName(defaultXmlFileName);
						fh.load();
					}
					else
					{
						fh.load(in);
					}
				}
				else
				{
					fh.setFileName(xmlFileName);
					fh.load();
				}
			}
			catch (ConfigurationException e)
			{
				if (pathToXml == null || pathToXml.isEmpty())
				{
					errorTracker().append(e.getMessage()).create(Severity.TRACE);
					return;
				}

				// error processing user-specified path; throw
				String errorMsg = String.format(
						"could not construct configuration from file [%s]; working directory was [%s]", xmlFileName,
						System.getProperty("user.dir"));
				throw _parent.oommICExcept().message(errorMsg);
			}

			NodeHandler<ImmutableNode> nh = config.getNodeModel().getNodeHandler();

			configkeyTypePair = new Hashtable<String, Integer>();

			for (int i = 0; i < ConfigManager.AsciiValues.length; i++)
				configkeyTypePair.put(ConfigManager.AsciiValues[i], ConfigElement.Type.Ascii);

			for (int i = 0; i < ConfigManager.EnumeratedValues.length; i++)
				configkeyTypePair.put(ConfigManager.EnumeratedValues[i], ConfigElement.Type.Enum);

			for (int i = 0; i < ConfigManager.Int64Values.length; i++)
				configkeyTypePair.put(ConfigManager.Int64Values[i], ConfigElement.Type.Int64);

			for (int i = 0; i < ConfigManager.UInt64Values.length; i++)
				configkeyTypePair.put(ConfigManager.UInt64Values[i], ConfigElement.Type.UInt64);

			for (int i = 0; i < ConfigManager.DoubleValues.length; i++)
				configkeyTypePair.put(ConfigManager.DoubleValues[i], ConfigElement.Type.Double);

			level = 1;

			xmlRoot = new XMLnode("root", level, null, ConfigManager.ROOT);
			xmlRoot.setErrorTracker(errorTracker());

			if (_debugDump)
				debugDump("=== Start: XMLConfig file read dump ========================");

			processNode(xmlRoot, nh.getRootNode(), ConfigManager.ConsumerTagDict);

			if (_debugDump)
				debugDump("=== End ====================================================");

			// debugging
			// xmlRoot.dump(0);
		}

		/**
		 * Given an optional directory and a file name determines full file name. 
		 *
		 * When <tt>pathToFile</tt> is <tt>null</tt> the <tt>defaultFileName</tt> is
		 * looked up in the current working directory.
		 *
		 * @param pathToFile  either a directory containing <tt>defaultFileName</tt> or a filename itself
		 * @param defaultFileName  name of the file to look for
		 *
		 * @return file name, either inside provided <tt>pathToFile</tt> or the
		 *   <tt>defaultFileName</tt>. An empty string, when the <tt>pathToFile</tt> is
		 *   empty and the <tt>defaultFileName</tt> does not exist
		 *
		 * @throws OmmInvalidConfigurationException when the <tt>pathToFile</tt> is
		 *   specified but is invalid or <tt>defaultFileName</tt> is not a file inside
		 *   this directory
		 */
		private String getValidatedFileName(String pathToFile, String defaultFileName) {
			String fileName;
			final String separator = FileSystems.getDefault().getSeparator();

			if (pathToFile == null || pathToFile.isEmpty()) {
				fileName = defaultFileName;
				File tmp = new File(fileName);
				fileName = System.getProperty("user.dir") + separator + fileName;

				if (!Files.isReadable(Paths.get(fileName)) || tmp.length() == 0)
				{
					errorTracker()
					.append(String.format("Missing, unreadable or empty file configuration, path=[%s",
							fileName)).append( "]" )
					.create(Severity.INFO);

					fileName = "";
				}
			} else {
				File tmp = new File(pathToFile);
				if(!tmp.exists()) {
					String errorMsg = String.format("configuration path [%s] does not exist; working directory was [%s]", pathToFile,
							System.getProperty("user.dir"));
					throw _parent.oommICExcept().message(errorMsg);
				}

				// path must be a file or directory
				if (tmp.isFile())
					fileName = pathToFile;

				// if path is a directory, create fileName and verify existence, and verify that it is a file
				else if (tmp.isDirectory()) {
					fileName = pathToFile + separator + defaultFileName;

					// file must exist
					tmp = new File(fileName);
					if (!tmp.exists()) {
						String errorMsg = String.format("fileName [%s] does not exist; working directory was [%s]", fileName,
								System.getProperty("user.dir"));
						throw _parent.oommICExcept().message(errorMsg);
					}

					// file must be a file
					if (!tmp.isFile()) {
						String errorMsg = String.format("fileName [%s] is not a file; working directory was [%s]", fileName,
								System.getProperty("user.dir"));
						throw _parent.oommICExcept().message(errorMsg);
					}
				} else {
					String errorMsg = String.format("configuration path [%s] must be either a file or directory; working directory was [%s]",
							pathToFile, System.getProperty("user.dir"));
					throw _parent.oommICExcept().message(errorMsg);
				}

				// at this point, we have a fileName, a path with that name exists, and that path is a file
				errorTracker().append( "reading configuration file [" ).append( fileName ).append( "]; working directory is [" )
						.append( System.getProperty("user.dir") ).append( "]" ).create(Severity.TRACE);
			}
			return fileName;
		}

		private Validator initValidator(String pathToXsd) throws SAXException {
			SchemaFactory factory = SchemaFactory.newInstance(XMLConstants.W3C_XML_SCHEMA_NS_URI);
			Schema schema = factory.newSchema(new File(pathToXsd));
			return schema.newValidator();
		}

		void verifyAndGetDefaultConsumer()
		{
			if ( xmlRoot == null )
				return;
			
			_defaultConsumerName = (String) xmlRoot.getNode(ConfigManager.DEFAULT_CONSUMER,0);
			if ( _defaultConsumerName == null ) 
				return;

			XMLnode consumerList = xmlRoot.getChildren(ConfigManager.CONSUMER_LIST,0);
			if ( consumerList != null )
			{
				for( int i = 0; i < consumerList.children().size(); i++)
				{
					XMLnode child = consumerList.children().get(i);

					// debugging 
					// child._attributeNode.dump("**");

					String childName = (String) child.attributeList().getValue(ConfigManager.ConsumerName);

					if( childName != null && childName.equals(_defaultConsumerName) )
					{
						_defaultConsumerNode = child;
						break;
					}
				}

				if( _defaultConsumerNode != null ) 
					return;

				if( _defaultConsumerName.equals("EmaConsumer") ) 
					return;

				String errorMsg = String.format("specified default consumer name [%s] was not found in the configured consumers",_defaultConsumerName);
			    errorTracker().append(errorMsg).create(Severity.TRACE);
				_defaultConsumerName = null;
			}
			else if ( _defaultConsumerName.equals("EmaConsumer") == false )
			{
				String errorMsg = String.format("default consumer name [%s] was specified, but no consumers were configured",_defaultConsumerName);
				throw _parent.oommICExcept().message( errorMsg );
			}
		}
		
		void verifyAndGetDefaultNiProvider()
		{
			if ( xmlRoot == null )
				return;
			
			_defaultNiProviderName = (String) xmlRoot.getNode(ConfigManager.DEFAULT_NIPROVIDER,0);
			if ( _defaultNiProviderName == null ) 
				return;

			XMLnode niProviderList = xmlRoot.getChildren(ConfigManager.NIPROVIDER_LIST,0);
			if ( niProviderList != null )
			{
				for( int i = 0; i < niProviderList.children().size(); i++)
				{
					XMLnode child = niProviderList.children().get(i);

					// debugging 
					// child._attributeNode.dump("**");

					String childName = (String) child.attributeList().getValue(ConfigManager.NiProviderName);

					if( childName != null && childName.equals(_defaultNiProviderName) )
					{
						_defaultNiProviderNode = child;
						break;
					}
				}

				if( _defaultNiProviderNode != null ) 
					return;

				if( _defaultNiProviderName.equals("EmaNiProvider") ) 
					return;

				String errorMsg = String.format("specified default niprovider name [%s] was not found in the configured niproviders",_defaultNiProviderName);
			    errorTracker().append(errorMsg).create(Severity.TRACE);
				_defaultNiProviderName = null;
			}
			else if ( _defaultNiProviderName.equals("EmaNiProvider") == false )
			{
				String errorMsg = String.format("default niprovider name [%s] was specified, but no niproviders were configured",_defaultNiProviderName);
				throw _parent.oommICExcept().message( errorMsg );
			}
		}
		
		void verifyAndGetDefaultIProvider()
		{
			if ( xmlRoot == null )
				return;
			
			_defaultIProviderName = (String) xmlRoot.getNode(ConfigManager.DEFAULT_IPROVIDER,0);
			if ( _defaultIProviderName == null ) 
				return;

			XMLnode iProviderList = xmlRoot.getChildren(ConfigManager.IPROVIDER_LIST,0);
			if ( iProviderList != null )
			{
				for( int i = 0; i < iProviderList.children().size(); i++)
				{
					XMLnode child = iProviderList.children().get(i);

					// debugging 
					// child._attributeNode.dump("**");

					String childName = (String) child.attributeList().getValue(ConfigManager.IProviderName);

					if( childName != null && childName.equals(_defaultIProviderName) )
					{
						_defaultIProviderNode = child;
						break;
					}
				}

				if( _defaultIProviderNode != null ) 
					return;

				if( _defaultIProviderName.equals("EmaIProvider") ) 
					return;

				String errorMsg = String.format("specified default iprovider name [%s] was not found in the configured iproviders",_defaultIProviderName);
			    errorTracker().append(errorMsg).create(Severity.TRACE);
			    _defaultIProviderName = null;
			}
			else if ( _defaultIProviderName.equals("EmaIProvider") == false )
			{
				String errorMsg = String.format("default iprovider name [%s] was specified, but no iproviders were configured",_defaultIProviderName);
				throw _parent.oommICExcept().message( errorMsg );
			}
		}
		
		String getDefaultDirectoryName()
		{
			if ( xmlRoot != null )
			{
				String defaultDirectoryName = (String) xmlRoot.getNode(ConfigManager.DEFAULT_DIRECTORY,0);
				return defaultDirectoryName;
			}
			
			return null;
		}

		boolean setDefaultConsumer(String consumerName) 
		{
			XMLnode theNode = xmlRoot.getChildren(ConfigManager.CONSUMER_GROUP, 0);
			if( theNode == null )
			{
				return false;
			}

			int elementType = configkeyTypePair.get("DefaultConsumer");
			boolean bSetAttribute = theNode.attributeList().setValue(ConfigManager.DefaultConsumer, consumerName, elementType);
			if(bSetAttribute)
			{
				_defaultConsumerName = consumerName;
			}

			return bSetAttribute;
		}
		
		boolean setDefaultNiProvider(String niproviderName) 
		{
			XMLnode theNode = xmlRoot.getChildren(ConfigManager.NIPROVIDER_GROUP, 0);
			if( theNode == null )
			{
				return false;
			}

			int elementType = configkeyTypePair.get("DefaultNiProvider");
			boolean bSetAttribute = theNode.attributeList().setValue(ConfigManager.DefaultNiProvider, niproviderName, elementType);
			if(bSetAttribute)
			{
				_defaultNiProviderName = niproviderName;
			}

			return bSetAttribute;
		}
		
		boolean setDefaultIProvider(String iproviderName) 
		{
			XMLnode theNode = xmlRoot.getChildren(ConfigManager.IPROVIDER_GROUP, 0);
			if( theNode == null )
			{
				return false;
			}

			int elementType = configkeyTypePair.get("DefaultIProvider");
			boolean bSetAttribute = theNode.attributeList().setValue(ConfigManager.DefaultIProvider, iproviderName, elementType);
			if(bSetAttribute)
			{
				_defaultIProviderName = iproviderName;
			}

			return bSetAttribute;
		}

		boolean isConsumerChildAvailable()
		{
			XMLnode consumerList = xmlRoot.getChildren(ConfigManager.CONSUMER_LIST,0);

			if( consumerList == null )
				return false;

			boolean bFoundChildName = false;
			for( int i = 0; i < consumerList.children().size(); i++)
			{
				XMLnode child = consumerList.children().get(i);
				String childName = (String) child.attributeList().getValue(ConfigManager.ConsumerName);

				if( childName == null )
				{
					bFoundChildName = false;
				}
				else
				{
					bFoundChildName = true;
					break;
				}
			}			

			return bFoundChildName;
		}
		
		boolean isNiProviderChildAvailable()
		{
			XMLnode niproviderList = xmlRoot.getChildren(ConfigManager.NIPROVIDER_LIST,0);

			if( niproviderList == null )
				return false;

			boolean bFoundChildName = false;
			for( int i = 0; i < niproviderList.children().size(); i++)
			{
				XMLnode child = niproviderList.children().get(i);
				String childName = (String) child.attributeList().getValue(ConfigManager.NiProviderName);

				if( childName == null )
				{
					bFoundChildName = false;
				}
				else
				{
					bFoundChildName = true;
					break;
				}
			}			

			return bFoundChildName;
		}

		Object getConsumerAttributeValue( String forConsumerName, int findAttributeKey )
		{
			return(getAttributeValue(ConfigManager.CONSUMER_LIST,ConfigManager.ConsumerName,forConsumerName,findAttributeKey));
		}
		
		Object getNiProviderAttributeValue( String forNiProviderName, int findAttributeKey )
		{
			return(getAttributeValue(ConfigManager.NIPROVIDER_LIST,ConfigManager.NiProviderName,forNiProviderName,findAttributeKey));
		}
		
		Object getIProviderAttributeValue( String forIProviderName, int findAttributeKey )
		{
			return(getAttributeValue(ConfigManager.IPROVIDER_LIST, ConfigManager.IProviderName, forIProviderName, findAttributeKey));
		}

		Object getDictionaryAttributeValue( String consumerName, int attributeKey )
		{
			return(getAttributeValue(ConfigManager.DICTIONARY_LIST,ConfigManager.DictionaryName, consumerName, attributeKey));
		}

		Object getAttributeValue( Branch searchNode, int forNodeKey, String findNodeName, int findAttributeKey )
		{
			if( xmlRoot == null )
				return null;
			
			XMLnode list = xmlRoot.getChildren(searchNode,0);

			if( list == null )
			{
				errorTracker().append("could not get ").append(searchNode.branchName).create(Severity.TRACE);
				return null;
			}

			if( list.children().size() == 0 )
			{
				errorTracker().append("children unavailable for ").append(searchNode.branchName).create(Severity.TRACE);
				return null;
			}

			String attributeValue = null;
			for( int i = 0; i < list.children().size(); i++)
			{
				XMLnode child = list.children().get(i);

				String nodeNameMatch = (String) child.attributeList().getValue(forNodeKey);
				if( findNodeName.equals(nodeNameMatch) )
				{
					attributeValue = (String) child.attributeList().getValue(findAttributeKey);
					break;
				}
			}
			return attributeValue;
		}
		Object getMutualExclusiveAttribute( Branch searchNode, int forNodeKey, String findNodeName, List<Integer> mExclusiveTags)
		{
		
			if( xmlRoot == null )
				return null;
			
			XMLnode list = xmlRoot.getChildren(searchNode,0);

			if( list == null )
			{
				errorTracker().append("could not get ").append(searchNode.branchName).create(Severity.TRACE);
				return null;
			}
			
			XMLnode child = null;
			
			ConfigAttributes consAttribs = null;
			
			for( int i = 0; i < list.children().size(); i++ )
			{
				child = list.children().get(i);

				String nodeNameMatch = (String) child.attributeList().getValue(forNodeKey);
				if( findNodeName.equals(nodeNameMatch) )
				{
					consAttribs = child.attributeList();
					break;
				}
			}

			if( consAttribs == null )
			{
				errorTracker().append("could not get ").append(findNodeName).create(Severity.TRACE);
				return null;
			}
			
			String attributeValue = null;		
		       
			int maxPos = 0;
			int position = 0;
			List<ConfigElement> maxCfgElement = null;
			Map<Integer, List<ConfigElement>> listOfAttribs = consAttribs.getList();
			Set< Entry<Integer, List<ConfigElement>> > entrySet =  listOfAttribs.entrySet();
			for( Entry<Integer, List<ConfigElement>> entry : entrySet)
			{
				int intVal = entry.getKey().intValue();
				position++;
			    for(int i = 0; i <  mExclusiveTags.size(); ++i)
			    {
			    	if(mExclusiveTags.get(i).intValue() == intVal)
			    	{
			    		if( position > maxPos )	
			    		{		    			
			    			maxPos = position;
			    			maxCfgElement = entry.getValue();		    			
			    		}
			    	}		    	
			    }
			}
			
			if(maxCfgElement != null)
			{
				ConfigElement ce = maxCfgElement.get(maxCfgElement.size() - 1);
				if(ce != null)
					attributeValue = (String) ce.value();
			}
			
			return attributeValue;
			
		}
		
		void appendAttributeValue( Branch setNode, String elementName, int setAttributeKey, String setAttributeValue )
		{
			XMLnode node = (XMLnode) xmlRoot.getNode(setNode, 0);

			if( node == null )
			{
				errorTracker().append("could not get node for").append(setNode.branchName).create(Severity.TRACE);
				return;
			}

			ConfigElement ce = makeConfigEntry(node,elementName,setAttributeValue,setAttributeKey);
			node.addAttribute(ce);
		}

		Object getFirstConsumer()
		{
			if( xmlRoot == null )
			{
				return null;
			}
			
			XMLnode list = xmlRoot.getChildren(ConfigManager.CONSUMER_LIST,0);

			if( list == null )
			{
				errorTracker().append("could not get ConsumerList").create(Severity.TRACE);
				return null;
			}

			if( list.children().size() == 0 )
			{
				errorTracker().append("could not get consumer nodes for ConsumerList").create(Severity.TRACE);
				return null;
			}

			String attributeValue = null;
			XMLnode child = list.children().get(0);

			attributeValue = (String) child.attributeList().getValue(ConfigManager.ConsumerName);
			if( attributeValue == null )
			{
				errorTracker().append("could not get the first consumer node for ConsumerList").create(Severity.TRACE);
			}
			
			return attributeValue;		
		}
		
		Object getFirstNiProvider()
		{
			if( xmlRoot == null )
			{
				return null;
			}
			
			XMLnode list = xmlRoot.getChildren(ConfigManager.NIPROVIDER_LIST,0);

			if( list == null )
			{
				errorTracker().append("could not get NiProviderList").create(Severity.TRACE);
				return null;
			}

			if( list.children().size() == 0 )
			{
				errorTracker().append("could not get niprovider nodes for NiProviderList").create(Severity.TRACE);
				return null;
			}

			String attributeValue = null;
			XMLnode child = list.children().get(0);

			attributeValue = (String) child.attributeList().getValue(ConfigManager.NiProviderName);
			if( attributeValue == null )
			{
				errorTracker().append("could not get the first niprovider node for NiProviderList").create(Severity.TRACE);
			}
			return attributeValue;		
		}
		
		Object getFirstIProvider()
		{
			if( xmlRoot == null )
			{
				return null;
			}
			
			XMLnode list = xmlRoot.getChildren(ConfigManager.IPROVIDER_LIST,0);

			if( list == null )
			{
				errorTracker().append("could not get IProviderList").create(Severity.TRACE);
				return null;
			}

			if( list.children().size() == 0 )
			{
				errorTracker().append("could not get iprovider nodes for IProviderList").create(Severity.TRACE);
				return null;
			}

			String attributeValue = null;
			XMLnode child = list.children().get(0);

			attributeValue = (String) child.attributeList().getValue(ConfigManager.IProviderName);
			if( attributeValue == null )
			{
				errorTracker().append("could not get the first iprovider node for IProviderList").create(Severity.TRACE);
			}
			return attributeValue;		
		}
		
		Object getFirstDirectory()
		{
			if( xmlRoot == null )
			{
				return null;
			}
			
			XMLnode list = xmlRoot.getChildren(ConfigManager.DIRECTORY_LIST,0);

			if( list == null )
			{
				errorTracker().append("could not get DirectoryList").create(Severity.TRACE);
				return null;
			}

			if( list.children().size() == 0 )
			{
				errorTracker().append("could not get directory nodes for DirectoryList").create(Severity.TRACE);
				return null;
			}
			
			XMLnode child = list.children().get(0);
			
			String attributeValue = (String) child.attributeList().getValue(ConfigManager.DirectoryName);
			if( attributeValue == null )
			{
				errorTracker().append("could not get the first directory node for DirectoryList").create(Severity.TRACE);
			}
			
			return attributeValue;		
		}
		
		XMLnode getDirectory(String directoryName)
		{
			if( xmlRoot == null )
			{
				return null;
			}
			
			XMLnode list = xmlRoot.getChildren(ConfigManager.DIRECTORY_LIST,0);

			if( list == null )
			{
				errorTracker().append("could not get DirectoryList").create(Severity.TRACE);
				return null;
			}

			if( list.children().size() == 0 )
			{
				errorTracker().append("could not get directory nodes for DirectoryList").create(Severity.TRACE);
				return null;
			}

			String attributeValue = null;
			
			XMLnode child = null;
			for( int i = 0; i < list.children().size(); i++)
			{
				child = list.children().get(i);

				attributeValue = (String) child.attributeList().getValue(ConfigManager.DirectoryName);
				
				if( attributeValue == null )
				{
					errorTracker().append("could not get the first directory node for DirectoryList").create(Severity.TRACE);
				}
				else if ( attributeValue.equals(directoryName))
				{
					return child;
				}
			}
		
			return null;		
		}
		
		

		ConfigAttributes getConsumerAttributes(String consumerName) 
		{
			if( xmlRoot == null )
			{
				return null;
			}

			return(xmlRoot.getNodeWithAttributeList(ConfigManager.CONSUMER_LIST, consumerName, ConfigManager.ConsumerName));
		}
		
		ConfigAttributes getNiProviderAttributes(String niProviderName) 
		{
			if( xmlRoot == null )
			{
				return null;
			}

			return(xmlRoot.getNodeWithAttributeList(ConfigManager.NIPROVIDER_LIST, niProviderName, ConfigManager.NiProviderName));
		}
		
		ConfigAttributes getIProviderAttributes(String iProviderName) 
		{
			if( xmlRoot == null )
			{
				return null;
			}

			return(xmlRoot.getNodeWithAttributeList(ConfigManager.IPROVIDER_LIST, iProviderName, ConfigManager.IProviderName));
		}

		ConfigAttributes getDictionaryAttributes(String dictionaryName) 
		{
			if( xmlRoot == null )
			{
				return null;
			}

			return(xmlRoot.getNodeWithAttributeList(ConfigManager.DICTIONARY_LIST,dictionaryName,ConfigManager.DictionaryName));
		}

		ConfigAttributes getChannelAttributes(String channelName) 
		{
			if( xmlRoot == null )
			{
				return null;
			}

			return(xmlRoot.getNodeWithAttributeList(ConfigManager.CHANNEL_LIST,channelName,ConfigManager.ChannelName));
		}
		
		ConfigAttributes getServerAttributes(String serverName) 
		{
			if( xmlRoot == null )
			{
				return null;
			}

			return(xmlRoot.getNodeWithAttributeList(ConfigManager.SERVER_LIST,serverName,ConfigManager.ServerName));
		}
		
		ConfigAttributes getWSBGroupAttributes(String WSBGroup) 
		{
			if( xmlRoot == null )
			{
				return null;
			}

			return(xmlRoot.getNodeWithAttributeList(ConfigManager.WARMSTANDBYGROUP_LIST,WSBGroup,ConfigManager.WarmStandbyChannelName));
		}
		
		ConfigAttributes getWSBServerInfoAttributes(String WSBServerInfo) 
		{
			if( xmlRoot == null )
			{
				return null;
			}

			return(xmlRoot.getNodeWithAttributeList(ConfigManager.WARMSTANDBYSERVER_LIST,WSBServerInfo,ConfigManager.WarmStandbyServerName));
		}
		
		ConfigAttributes getGlobalConfig(){
			if( xmlRoot == null )
			{
				return null;
			}

			return(xmlRoot.getNodeWithAttributeList(ConfigManager.GLOBAL_CONFIG));
		}

		void debugDump(String txt)
		{
			System.out.println(txt);
		}
		
		String padLeft(String txt, int level)
		{
			String spaces = "";
			for(int i = 1; i < level; i++)
			{
				spaces += " ";
			}
			return spaces + txt;
		}
	}
}
