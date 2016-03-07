package com.thomsonreuters.ema.access;

import java.util.ArrayList;
import java.util.Hashtable;
import java.util.List;

import org.apache.commons.configuration.ConfigurationException;
import org.apache.commons.configuration.XMLConfiguration;
import org.apache.commons.configuration.tree.ConfigurationNode;

import com.thomsonreuters.ema.access.ConfigManager.Branch;
import com.thomsonreuters.ema.access.ConfigManager.ConfigAttributes;
import com.thomsonreuters.ema.access.ConfigManager.ConfigElement;
import com.thomsonreuters.ema.access.ConfigManager.TagDictionary;
import com.thomsonreuters.ema.access.OmmLoggerClient.Severity;
import com.thomsonreuters.upa.transport.CompressionTypes;
import com.thomsonreuters.upa.transport.ConnectionTypes;

class ConfigReader 
{
	static ConfigReader configReaderFactory;
	
	OmmConsumerConfigImpl _parent;
	
	ConfigReader() 
	{

	}

	ConfigReader(OmmConsumerConfigImpl parent)
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

	protected void loadFile() {}

	static ConfigReader acquire() 
	{
		if( configReaderFactory == null )
			configReaderFactory = new ConfigReader();

		return configReaderFactory;
	}

	static ConfigReader createXMLConfigReader(OmmConsumerConfigImpl ommConsumerConfigImpl)
	{
		return(ConfigReader.acquire().new XMLConfigReader(ommConsumerConfigImpl));
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
				if( child._tagId==nodeId )
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
				
				if ( currentNode._children.isEmpty() )
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
			
			for( int i = startIdx; i < endIdx; i++)
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

		ConfigAttributes getNodeWithAttributeList(Branch branch,String nodeName,int attributeId)
		{
			XMLnode list = (XMLnode) getChildren(branch,0);
			
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
				if( actualNodeName.asciiValue().equals(nodeName) )
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
		private XMLnode _defaultConsumerNode;

		boolean _debugDump = false;

		class ConfigNode
		{
			ArrayList <Integer> nodeList;

			void add(int nodeId,Hashtable<String, Integer> dict)
			{
				nodeList.add(nodeId);
			}
		}

		XMLConfigReader(OmmConsumerConfigImpl ommConsumerConfigImpl)
		{
			super(ommConsumerConfigImpl);
		}

		String defaultConsumerName() 
		{
			return _defaultConsumerName;
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
				int localDictonary = 1;

				if(enumValue.equals("FileDictionary"))
					localDictonary = 1;
				else if(enumValue.equals("ChannelDictionary"))
					localDictonary = 0;

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

				if( channelType != -1 )
					return ConfigManager.acquire().new IntConfigElement( parent, ConfigElement.Type.Enum,channelType);
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

				if( compressionType != -1 )
					return ConfigManager.acquire().new IntConfigElement( parent, ConfigElement.Type.Enum,compressionType);
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

			default:
				errorTracker().append( "config element [")
				.append(name)
				.append("] had unexpected elementType [")
				.append(elementType)
				.append("; element ignored")
				.create(Severity.ERROR);
				break;
			}

			e._name = name;
			e._tagId = tagId;
			e._valueStr = value;
			e._type = elementType;
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

		private ConfigElement handleConfigEntry(ConfigurationNode nodePtr,XMLnode theNode, int tagId) 
		{
			List<ConfigurationNode> attributeList = nodePtr.getAttributes();

			String attributeValue = null;

			for (int i = 0; i < attributeList.size(); i++)
			{
				ConfigurationNode attribute = attributeList.get(i);

				String attributeName = attribute.getName();
				if(attributeName.equalsIgnoreCase("value") )
				{
					attributeValue = (String) attribute.getValue();	
				}
				else
				{
					errorTracker().append("Unknown attributeName ").append(attributeName).create(Severity.ERROR);
				}
			}

			ConfigElement e = makeConfigEntry(theNode.parent(),nodePtr.getName(),attributeValue,tagId);
			return e;
		}

		private void processNode(XMLnode theNode, ConfigurationNode nodePtr, TagDictionary tagDict)
		{
			List<ConfigurationNode> children = nodePtr.getChildren();
			if(nodePtr.getChildrenCount() == 0 )
			{
				errorTracker().append("No children for ").append(nodePtr.getName()).create(Severity.ERROR);
				return;
			}

			for (int i = 0; i < children.size(); i++)
			{
				ConfigurationNode configNodeChild = children.get(i); 

				if( configNodeChild.getAttributeCount() > 0 )
				{
					Integer tagId = tagDict.get(configNodeChild.getName());
					if( tagId == null ) 
					{
						if( configNodeChild.getName().equalsIgnoreCase("pipeport"))
						{
							// unsupported in emaj; ignore 
						}
						else
						{
							errorTracker().append("Unable to find tagId for ")
							.append(configNodeChild.getName())
							.create(Severity.ERROR);
						}
					}
					else
					{
						if(_debugDump) debugDump(padLeft(String.format("attribute Count: %d",configNodeChild.getAttributeCount()),level+4));
						ConfigElement ce = handleConfigEntry(configNodeChild,theNode,tagId); 

						if( ce != null )
						{
							if(_debugDump) debugDump(padLeft(String.format("add attribute: %s to %s; [tagId: %s - %d",ce._name,theNode.name(),ce._name,tagId),level+6));
							theNode.addAttribute(ce);
						}
					}
					continue;
				}
				else
				{
					level++;

					if(_debugDump) debugDump(padLeft(String.format("Create XML Node: %s - level: %d",configNodeChild.getName(),level),level));

					if( level == 2)
					{
						if( configNodeChild.getName().equals("ConsumerGroup"))
						{
							tagDict = ConfigManager.ConsumerTagDict;
						}
						else if( configNodeChild.getName().equals("ChannelGroup"))
						{
							tagDict = ConfigManager.ChannelTagDict;
						}
						else if( configNodeChild.getName().equals("LoggerGroup"))
						{
							skipNode(configNodeChild);
							level--;
							continue;
						}
						else if( configNodeChild.getName().equals("DictionaryGroup"))
						{
							tagDict = ConfigManager.DictionaryTagDict;
						}
					}

					if( tagDict == null )
					{
						errorTracker().append("Tag dict null for ").append(configNodeChild.getName()).create(Severity.ERROR);
					}

					if( tagDict.isEmpty() )
					{
						errorTracker().append("Tag dict is EMPTY for ").append(configNodeChild.getName()).create(Severity.ERROR);
					}

					Integer tagKey = tagDict.get(configNodeChild.getName());
					if( tagKey == null )
					{
						errorTracker().append("Unable to find tagId for ").append(configNodeChild.getName()).create(Severity.ERROR);
					}
					else
					{
						int tagId = tagKey.intValue();

						if(_debugDump) debugDump(padLeft(String.format("Tag %s - %d", configNodeChild.getName(),tagId),level+2));

						XMLnode xmlChild = new XMLnode(configNodeChild.getName(), level, theNode,tagId); 
						processNode(xmlChild,configNodeChild,tagDict);
						if (configNodeChild.getAttributeCount() == 0  && configNodeChild.getChildrenCount() > 0)
						{
							theNode.addChild(xmlChild);
						}
					}
				}

				level--;
			}
		}

		private void skipNode(ConfigurationNode nodePtr)
		{
			List<ConfigurationNode> children = nodePtr.getChildren();
			for (int i = 0; i < children.size(); i++)
			{
				ConfigurationNode configNodeChild = children.get(i); 

				if( configNodeChild.getAttributeCount() > 0 )
				{
					continue;
				}
				else
				{
					skipNode(configNodeChild);
				}
			}
		}

		protected void loadFile() 
		{
			_configFileName = "EmaConfig.xml";
			_configFileLocation = System.getProperty("user.dir");

			errorTracker().append( "reading configuration file [" )
						.append( _configFileName )
						.append( "] from [" ).append( _configFileLocation ).append( "]" )
						.create(Severity.INFO);
			
			XMLConfiguration config = null;
			try 
			{
				config = new XMLConfiguration(_configFileName);
			} 
			catch (ConfigurationException e) 
			{
				errorTracker().append(e.getMessage()).create(Severity.ERROR);
				return;
			}

			configkeyTypePair = new Hashtable<String, Integer>();

			for( int i = 0; i < ConfigManager.AsciiValues.length; i++ )
				configkeyTypePair.put(ConfigManager.AsciiValues[i], ConfigElement.Type.Ascii);

			for( int i = 0; i < ConfigManager.EnumeratedValues.length; i++ )
				configkeyTypePair.put(ConfigManager.EnumeratedValues[i], ConfigElement.Type.Enum);

			for( int i = 0; i < ConfigManager.Int64Values.length; i++ )
				configkeyTypePair.put(ConfigManager.Int64Values[i], ConfigElement.Type.Int64);

			for( int i = 0; i < ConfigManager.UInt64Values.length; i++ )
				configkeyTypePair.put(ConfigManager.UInt64Values[i], ConfigElement.Type.UInt64);

			ConfigurationNode doc =  config.getRootNode();
			level = 1;
			
			xmlRoot = new XMLnode("root",level,null,ConfigManager.ROOT);
			xmlRoot.setErrorTracker(errorTracker());

			if(_debugDump) debugDump("=== Start: XMLConfig file read dump ========================");

			processNode(xmlRoot,doc,ConfigManager.ConsumerTagDict);
			
			if(_debugDump) debugDump("=== End ====================================================");

			// debugging
			// xmlRoot.dump(0);

			verifyAndGetDefaultConsumer();
		}

		void verifyAndGetDefaultConsumer()
		{
			_defaultConsumerName = (String) xmlRoot.getNode(ConfigManager.DEFAULT_CONSUMER,0);
			if ( _defaultConsumerName == null ) 
				return;

			XMLnode consumerList = (XMLnode) xmlRoot.getChildren(ConfigManager.CONSUMER_LIST,0);
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
				throw _parent.oommICExcept().message( errorMsg );
			}
			else if ( _defaultConsumerName.equals("EmaConsumer") == false )
			{
				String errorMsg = String.format("default consumer name [%s] was specified, but no consumers were configured",_defaultConsumerName);
				throw _parent.oommICExcept().message( errorMsg );
			}
		}

		boolean setDefaultConsumer(String consumerName) 
		{
			XMLnode theNode = (XMLnode) xmlRoot.getChildren(ConfigManager.CONSUMER_GROUP, 0);
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

		boolean isConsumerChildAvailable()
		{
			XMLnode consumerList = (XMLnode) xmlRoot.getChildren(ConfigManager.CONSUMER_LIST,0);

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


		Object getConsumerAttributeValue( String forConsumerName, int findAttributeKey )
		{
			return(getAttributeValue(ConfigManager.CONSUMER_LIST,ConfigManager.ConsumerName,forConsumerName,findAttributeKey));
		}

		Object getDictionaryAttributeValue( String consumerName, int attributeKey )
		{
			return(getAttributeValue(ConfigManager.DICTIONARY_LIST,ConfigManager.DictionaryName,consumerName,attributeKey));
		}

		Object getAttributeValue( Branch searchNode, int forNodeKey, String findNodeName, int findAttributeKey )
		{
			if( xmlRoot == null )
				return null;
			
			XMLnode list = (XMLnode) xmlRoot.getChildren(searchNode,0);

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

		void appendAttributeValue( Branch setNode, int setAttributeKey, String setAttributeValue )
		{
			XMLnode node = (XMLnode) xmlRoot.getNode(ConfigManager.CONSUMER_GROUP,0);

			if( node == null )
			{
				errorTracker().append("could not get node for").append(setNode.branchName).create(Severity.TRACE);
				return;
			}

			ConfigElement ce = makeConfigEntry(node,"DefaultConsumer",setAttributeValue,setAttributeKey);
			node.addAttribute(ce);
		}

		Object getFirstConsumer()
		{
			if( xmlRoot == null )
			{
				return null;
			}
			
			XMLnode list = (XMLnode) xmlRoot.getChildren(ConfigManager.CONSUMER_LIST,0);

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
			for( int i = 0; i < list.children().size(); i++)
			{
				XMLnode child = list.children().get(i);

				attributeValue = (String) child.attributeList().getValue(ConfigManager.ConsumerName);
				if( attributeValue == null )
				{
					errorTracker().append("could not get the first consumer node for ConsumerList").create(Severity.TRACE);
				}
			}
			return attributeValue;		
		}

		ConfigAttributes getConsumerAttributes(String consumerName) 
		{
			if( xmlRoot == null )
			{
				return null;
			}

			return(xmlRoot.getNodeWithAttributeList(ConfigManager.CONSUMER_LIST,consumerName,ConfigManager.ConsumerName));
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