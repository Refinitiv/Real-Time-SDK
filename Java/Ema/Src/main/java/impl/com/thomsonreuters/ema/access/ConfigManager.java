package com.thomsonreuters.ema.access;

import java.util.ArrayList;
import java.util.Hashtable;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.Set;

import com.thomsonreuters.ema.access.ConfigReader.XMLnode;

public class ConfigManager
{
	private static ConfigManager _configManager;
	static Hashtable<Integer, String> _nodeAsText;
	
	static TagDictionary ConsumerTagDict;
	static TagDictionary ChannelTagDict;
	static TagDictionary DictionaryTagDict;
	
	static Branch DEFAULT_CONSUMER;
	static Branch CONSUMER_GROUP;
	static Branch CONSUMER_LIST;
	static Branch DICTIONARY_LIST;
	static Branch CHANNEL_LIST;
	
	private static StringBuilder _stringMaker;
	
	static String ConfigName = "EmaConfig";
	
	static int ROOT = 1001;
	
	// Consumer
	public static int ConsumerGroup = 1;
	public static int DefaultConsumer = 2;
	public static int ConsumerList = 3;
	public static int Consumer = 4;
	
	public static int ConsumerName = 5;
	public static int ConsumerChannelName = 6;
	public static int ConsumerLoggerName = 7;
	public static int ConsumerDictionaryName = 8;
	
	public static int CatchUnhandledException = 9;
	public static int ConsumerCatchUnhandledException = 10;
	public static int ConsumerChannelSet = 11;
	public static int ConsumerDictionaryRequestTimeOut = 12;
	public static int ConsumerDirectoryRequestTimeOut = 13;
	public static int ConsumerDispatchTimeoutApiThread = 14;
	public static int ConsumerItemCountHint = 15;
	public static int ConsumerLoginRequestTimeOut = 16;
	public static int ConsumerMaxDispatchCountApiThread = 17;
	public static int ConsumerMaxDispatchCountUserThread = 18;
	public static int ConsumerMaxOutstandingPosts = 19;
	public static int ConsumerObeyOpenWindow = 20;
	// public static int ConsumerPipePort = 20;
	public static int ConsumerPostAckTimeout = 21;
	public static int ConsumerRequestTimeout = 22;
	public static int ConsumerServiceCountHint = 23;

	// Channel: Global
	public static int ChannelGroup = 24;
	public static int ChannelList = 25;
	public static int Channel = 26;
	public static int ChannelName = 27;
	public static int ChannelType = 28;
	public static int ChannelConnectionPingTimeout = 29;
	public static int ChannelGuaranteedOutputBuffers = 30;
	public static int ChannelInterfaceName = 31;
	public static int ChannelMsgKeyInUpdates = 32;
	public static int ChannelNumInputBuffers = 33;
	public static int ChannelReconnectAttemptLimit = 34;
	public static int ChannelReconnectMaxDelay = 35;	
	public static int ChannelReconnectMinDelay = 36;
	public static int ChannelSysRecvBufSize = 37;
	public static int ChannelSysSendBufSize = 38;
	public static int ChannelXmlTraceFileName = 39;
	public static int ChannelXmlTraceHex = 40;
	public static int ChannelXmlTraceMaxFileSize = 41;
	public static int ChannelXmlTracePing = 42;
	public static int ChannelXmlTraceRead = 43;
	public static int ChannelXmlTraceToFile = 44;
	public static int ChannelXmlTraceToMultipleFiles = 45;
	public static int ChannelXmlTraceToStdout = 46;
	public static int ChannelXmlTraceWrite = 47;
	
	// Channel: Socket, HTTP, Encrypted
	public static int ChannelCompressionThreshold = 48;
	public static int ChannelCompressionType = 49;
	public static int ChannelHost = 50;
	public static int ChannelObjectName = 51;
	public static int ChannelPort = 52;
	public static int ChannelTcpNodelay = 53;
	
	// Channel: Multicast
	public static int ChannelDisconnectOnGap = 54;
	public static int ChannelHsmInterface = 55;
	public static int ChannelHsmInterval = 56;
	public static int ChannelHsmMultAddress = 57;
	public static int ChannelHsmPort = 58;
	public static int Channelndata = 59;
	public static int Channelnmissing = 60;
	public static int Channelnrreq = 61;
	public static int ChannelPacketTTL = 62;
	public static int ChannelpktPoolLimitHigh = 63;
	public static int ChannelpktPoolLimitLow = 64;
	public static int ChannelRecvAddress = 65;
	public static int ChannelRecvPort = 66;
	public static int ChannelSendAddress = 67;
	public static int ChannelSendPort = 68;
	public static int Channeltbchold = 69;
	public static int ChanneltcpControlPort = 70;
	public static int Channeltdata = 71;
	public static int Channeltpphold = 72;
	public static int Channeltrreq = 73;
	public static int Channeltwait = 74;
	public static int ChannelUnicastPort = 75;
	public static int ChanneluserQLimit = 76;
		
	// Channel: Dictionary	
	public static int DictionaryGroup = 77;
	public static int DictionaryList = 78;
	public static int Dictionary = 79;
	public static int DictionaryName = 80;
	public static int DictionaryType = 81;
	public static int DictionaryEnumTypeDefFileName = 82;
	public static int DictionaryRDMFieldDictFileName = 83;
	
	static
	{
		_stringMaker = new StringBuilder(512);
		_nodeAsText = new Hashtable<Integer,String>();
		
		ConsumerTagDict = acquire().new TagDictionary();
		ChannelTagDict = acquire().new TagDictionary();
		DictionaryTagDict = acquire().new TagDictionary();
		
		ConsumerTagDict.add( "ConsumerGroup",ConsumerGroup );
		ConsumerTagDict.add( "DefaultConsumer",DefaultConsumer );
		ConsumerTagDict.add( "ConsumerList",ConsumerList );
		ConsumerTagDict.add( "Consumer",Consumer );
		
		ConsumerTagDict.add( "Name",ConsumerName );
		ConsumerTagDict.add( "Channel",ConsumerChannelName );
		ConsumerTagDict.add( "Logger",ConsumerLoggerName );
		ConsumerTagDict.add( "Dictionary",ConsumerDictionaryName );
		
		ConsumerTagDict.add( "CatchUnhandledException",CatchUnhandledException );
		ConsumerTagDict.add( "ChannelSet",ConsumerChannelSet );
		ConsumerTagDict.add( "DictionaryRequestTimeOut",ConsumerDictionaryRequestTimeOut );
		ConsumerTagDict.add( "DirectoryRequestTimeOut",ConsumerDirectoryRequestTimeOut );
		ConsumerTagDict.add( "DispatchTimeoutApiThread",ConsumerDispatchTimeoutApiThread );
		ConsumerTagDict.add( "ItemCountHint",ConsumerItemCountHint );
		ConsumerTagDict.add( "LoginRequestTimeOut",ConsumerLoginRequestTimeOut );
		ConsumerTagDict.add( "MaxDispatchCountApiThread",ConsumerMaxDispatchCountApiThread );
		ConsumerTagDict.add( "MaxDispatchCountUserThread",ConsumerMaxDispatchCountUserThread );
		ConsumerTagDict.add( "MaxOutstandingPosts",ConsumerMaxOutstandingPosts );
		ConsumerTagDict.add( "ObeyOpenWindow",ConsumerObeyOpenWindow );
		// ignore: ConsumerTagDict.add( "PipePort",PipePort );
		ConsumerTagDict.add( "PostAckTimeout",ConsumerPostAckTimeout );
		ConsumerTagDict.add( "RequestTimeout",ConsumerRequestTimeout );
		ConsumerTagDict.add( "ServiceCountHint",ConsumerServiceCountHint );
		
		ChannelTagDict.add( "ChannelGroup",ChannelGroup );
		ChannelTagDict.add( "ChannelList",ChannelList );
		ChannelTagDict.add( "Channel",Channel );
		ChannelTagDict.add( "Name",ChannelName );
		ChannelTagDict.add( "ChannelType",ChannelType );
		ChannelTagDict.add( "ConnectionPingTimeout",ChannelConnectionPingTimeout );
		ChannelTagDict.add( "GuaranteedOutputBuffers",ChannelGuaranteedOutputBuffers );
		ChannelTagDict.add( "InterfaceName",ChannelInterfaceName );
		ChannelTagDict.add( "MsgKeyInUpdates",ChannelMsgKeyInUpdates );
		ChannelTagDict.add( "NumInputBuffers",ChannelNumInputBuffers );		
		ChannelTagDict.add( "ReconnectAttemptLimit",ChannelReconnectAttemptLimit );		
		ChannelTagDict.add( "ReconnectMaxDelay",ChannelReconnectMaxDelay );
		ChannelTagDict.add( "ReconnectMinDelay",ChannelReconnectMinDelay );
		ChannelTagDict.add( "SysRecvBufSize",ChannelSysRecvBufSize );
		ChannelTagDict.add( "SysSendBufSize",ChannelSysSendBufSize );
		ChannelTagDict.add( "XmlTraceFileName",ChannelXmlTraceFileName );		
		ChannelTagDict.add( "XmlTraceHex", ChannelXmlTraceHex );
		ChannelTagDict.add( "XmlTraceMaxFileSize", ChannelXmlTraceMaxFileSize );
		ChannelTagDict.add( "XmlTracePing", ChannelXmlTracePing );
		ChannelTagDict.add( "XmlTraceRead",ChannelXmlTraceRead );
		ChannelTagDict.add( "XmlTraceToFile",ChannelXmlTraceToFile );
		ChannelTagDict.add( "XmlTraceToMultipleFiles",ChannelXmlTraceToMultipleFiles );
		ChannelTagDict.add( "XmlTraceToStdout",ChannelXmlTraceToStdout );
		ChannelTagDict.add( "XmlTraceWrite",ChannelXmlTraceWrite );
		
		// ConnectionTypes.SOCKET, ConnectionTypes.HTTP, ConnectionTypes.ENCRYPTED 
		ChannelTagDict.add( "CompressionThreshold",ChannelCompressionThreshold );
		ChannelTagDict.add( "CompressionType",ChannelCompressionType );
		ChannelTagDict.add( "Host",ChannelHost );
		ChannelTagDict.add( "ObjectName",ChannelObjectName );
		ChannelTagDict.add( "Port",ChannelPort );
		ChannelTagDict.add( "TcpNodelay",ChannelTcpNodelay );
		
		// ConnectionTypes.MCAST
		ChannelTagDict.add( "DisconnectOnGap",ChannelDisconnectOnGap );
		ChannelTagDict.add( "HsmInterface",ChannelHsmInterface );
		ChannelTagDict.add( "HsmInterval",ChannelHsmInterval );
		ChannelTagDict.add( "HsmMultAddress",ChannelHsmMultAddress );
		ChannelTagDict.add( "HsmPort",ChannelHsmPort );
		ChannelTagDict.add( "ndata",Channelndata );
		ChannelTagDict.add( "nmissing",Channelnmissing );
		ChannelTagDict.add( "nrreq",Channelnrreq );
		ChannelTagDict.add( "PacketTTL",ChannelPacketTTL );
		ChannelTagDict.add( "pktPoolLimitHigh",ChannelpktPoolLimitHigh );
		ChannelTagDict.add( "pktPoolLimitLow",ChannelpktPoolLimitLow );
		ChannelTagDict.add( "RecvAddress",ChannelRecvAddress );
		ChannelTagDict.add( "RecvPort",ChannelRecvPort );
		ChannelTagDict.add( "SendAddress",ChannelSendAddress );
		ChannelTagDict.add( "SendPort",ChannelSendPort );
		ChannelTagDict.add( "tbchold",Channeltbchold );
		ChannelTagDict.add( "tcpControlPort",ChanneltcpControlPort );
		ChannelTagDict.add( "tdata",Channeltdata );
		ChannelTagDict.add( "tpphold",Channeltpphold );
		ChannelTagDict.add( "trreq",Channeltrreq );
		ChannelTagDict.add( "twait",Channeltwait );
		ChannelTagDict.add( "UnicastPort",ChannelUnicastPort );
		ChannelTagDict.add( "userQLimit",ChanneluserQLimit );
				
		DictionaryTagDict.add( "DictionaryGroup",DictionaryGroup );
		DictionaryTagDict.add( "DictionaryList",DictionaryList );
		DictionaryTagDict.add( "Dictionary",Dictionary );
		DictionaryTagDict.add( "Name",DictionaryName );
		DictionaryTagDict.add( "DictionaryType",DictionaryType );
		DictionaryTagDict.add( "RdmFieldDictionaryFileName",DictionaryRDMFieldDictFileName );
		DictionaryTagDict.add( "EnumTypeDefFileName",DictionaryEnumTypeDefFileName );
		
		CONSUMER_GROUP = ConfigManager.acquire().new Branch();
		CONSUMER_GROUP.add(ConfigManager.ConsumerGroup,ConfigManager.ConsumerTagDict);
		CONSUMER_GROUP.complete();

		DEFAULT_CONSUMER = ConfigManager.acquire().new Branch();
		DEFAULT_CONSUMER.add(ConfigManager.ConsumerGroup,ConfigManager.ConsumerTagDict);
		DEFAULT_CONSUMER.add(ConfigManager.DefaultConsumer,ConfigManager.ConsumerTagDict);
		DEFAULT_CONSUMER.complete();

		CONSUMER_LIST = ConfigManager.acquire().new Branch();
		CONSUMER_LIST.add(ConfigManager.ConsumerGroup,ConfigManager.ConsumerTagDict);
		CONSUMER_LIST.add(ConfigManager.ConsumerList,ConfigManager.ConsumerTagDict);
		CONSUMER_LIST.complete();
		
		DICTIONARY_LIST = ConfigManager.acquire().new Branch();
		DICTIONARY_LIST.add(ConfigManager.DictionaryGroup,ConfigManager.DictionaryTagDict);
		DICTIONARY_LIST.add(ConfigManager.DictionaryList,ConfigManager.DictionaryTagDict);
		DICTIONARY_LIST.complete();
		
		CHANNEL_LIST = ConfigManager.acquire().new Branch();
		CHANNEL_LIST.add(ConfigManager.ChannelGroup,ConfigManager.ChannelTagDict);
		CHANNEL_LIST.add(ConfigManager.ChannelList,ConfigManager.ChannelTagDict);
		CHANNEL_LIST.complete();
	}

	public static String AsciiValues[] = {
		"Channel",
		"ChannelSet",
		"ConsumerName",
		"DefaultConsumer",
		"Dictionary",
		"EnumTypeDefFileName",
		"FileName",
		"Host",
		"HsmInterface",
		"HsmMultAddress",
		"HsmPort",
		"InterfaceName",
		"Logger",
		"Name",
		"ObjectName",		
		"Port",
		"RdmFieldDictionaryFileName",
		"RecvAddress",
		"RecvPort",
		"SendAddress",
		"SendPort",
		"tcpControlPort",
		"UnicastPort",
		"XmlTraceFileName"
	};

	public static String EnumeratedValues[] = {
		"ChannelType",
		"CompressionType",
		"DictionaryType"
	};

	public static String Int64Values[] = {
		"DispatchTimeoutApiThread",
		"PipePort",
		"ReconnectAttemptLimit",
		"ReconnectMaxDelay",
		"ReconnectMinDelay",
		"XmlTraceMaxFileSize"
	};

	public static String UInt64Values[] = {
		"CatchUnhandledException",
		"CompressionThreshold",
		"ConnectionPingTimeout",
		"DictionaryRequestTimeOut",
		"DirectoryRequestTimeOut",
		"DisconnectOnGap",
		"GuaranteedOutputBuffers",
		"HsmInterval",
		"IncludeDateInLoggerOutput",
		"ItemCountHint",
		"LoginRequestTimeOut",
		"MaxDispatchCountApiThread",
		"MaxDispatchCountUserThread",
		"MaxOutstandingPosts",
		"MsgKeyInUpdates",
		"ndata",
		"nmissing",
		"nrreq",
		"NumInputBuffers",
		"ObeyOpenWindow",		
		"PacketTTL",
		"pktPoolLimitHigh",
		"pktPoolLimitLow",
		"PostAckTimeout",
		"RequestTimeout",
		"ServiceCountHint",
		"SysRecvBufSize",
		"SysSendBufSize",
		"tbchold",
		"TcpNodelay",
		"tdata",
		"tpphold",
		"trreq",
		"twait",
		"userQLimit",
		"XmlTraceHex",
		"XmlTraceMaxFileSize",
		"XmlTracePing",
		"XmlTraceRead",
		"XmlTraceToFile",		
		"XmlTraceToMultipleFiles",
		"XmlTraceToStdout",
		"XmlTraceWrite"
	};	
	static ConfigManager acquire() 
	{
		if(_configManager==null)
			_configManager = new ConfigManager();
		
		return _configManager;
	}
	
	 ConfigAttributes createConfigAttributes()
	{
		return( new ConfigAttributes());
	}

	static StringBuilder stringMaker() 
	{
		return _stringMaker;
	}

	static String nodeName(int nodeId) 
	{
		return _nodeAsText.get( nodeId );
	}

	static void addNodeText(int tagId, String tagName) 
	{
		_nodeAsText.put( tagId, tagName );
	}

	class TagDictionary 
	{
		Hashtable<String, Integer> dict = new Hashtable<String, Integer>();

		void add(String tagName, int tagId)
		{
			dict.put( tagName, tagId );
			ConfigManager.addNodeText( tagId, tagName );
		}

		 Integer get(String name) 
		{
			Integer tagId = dict.get(name);
			return tagId;
		}

		 boolean isEmpty() 
		{
			return(dict.isEmpty());
		}

		/* String nodeName(int nodeId) 
		{
			String nodeName = dictText.get(nodeId);
			if( nodeName == null )
				nodeName = "Unknown";
			
			return nodeName;
		}*/
	}

	class Branch 
	{
		ArrayList<Integer> branchAsNumeric = new ArrayList<Integer>();
		ArrayList<String> branchAsString = new ArrayList<String>();

		String branchName;
		
		 void add(int nodeId,TagDictionary dictionary) 
		{
			branchAsNumeric.add(nodeId);
			branchAsString.add(ConfigManager.nodeName(nodeId));
		}

		void complete()
		{
			ConfigManager.stringMaker().append(branchAsString.get(0));

			for(int i = 1; i < branchAsNumeric.size(); i++)
			{
				_stringMaker.append("|");
				_stringMaker.append(branchAsString.get(i));
			}
			branchName = _stringMaker.toString();
			_stringMaker.setLength(0);
		}
		
		 int size() 
		{
			return branchAsNumeric.size();
		}
		
		@Override
		public String toString()
		{
			return branchName;
		}

		 int get(int i) 
		{
			return branchAsNumeric.get(i);
		}

		 String nodeAsString(int findNodeId) 
		{
			for( int i = 0; i < branchAsNumeric.size(); i++)
			{
				int nodeId = branchAsNumeric.get(i);
				if(nodeId == findNodeId)
				{
					return(branchAsString.get(i));
				}
			}
			return "Unknown";
		}
	}
	
	class ConfigAttributes 
	{
		Map<Integer, ConfigElement> _list;
		
		 ConfigAttributes()
		{
			_list = new LinkedHashMap<Integer, ConfigElement>();
		}
		
		 void put(int id, ConfigElement element) 
		{
			_list.put(id,element);
		}

		 void dump(String space) 
		{
			if( _list.size() > 0 )
			{
				String aspace = space+" ";

				Set<Integer> keys = _list.keySet();
				for(Integer k:keys)
				{
					ConfigElement ce = _list.get(k);
					System.out.format("%s___ %s (Key:%d) - %s\n",aspace,ce._name,k,ce._valueStr);
				}
			}
		}

		Object getValue(int attributeId)
		{
			ConfigElement ce = _list.get(attributeId);
			if( ce != null )
				return ce.value();
			else
				return null;
		}
		
		Object getElement(int attributeId)
		{
			ConfigElement ce = _list.get(attributeId);
			return ce;
		}

		boolean setValue( int attributeId, String newValue, int dataType )
		{
			Set<Integer> keys = _list.keySet();
			for(Integer k:keys)
			{
				if(k == attributeId)
				{
					ConfigElement ce = _list.get(k);
					if( dataType == ConfigElement.Type.Ascii )
						ce.setAsciiValue(newValue);
					
					return true;
				}
			}
			
			return false;
		}

		 ConfigElement getPrimitiveValue(int id) 
		{
			ConfigElement ce = _list.get(id);
			return ce;
		}
	}
	
	 abstract class ConfigElement 
	{
		int _type;
		String _name;
		XMLnode _parent;
		String _valueStr;
		int _tagId;

		 final class Type
		{
			 final static int Int64 = 0;
			 final static int UInt64 = 1;
			 final static int Ascii = 2;
			 final static int Enum = 3;
			 final static int Boolean = 4;
		};

		 ConfigElement( XMLnode  parent )
		{
			this._parent = parent;
		}

		abstract  Object value();

		void setAsciiValue(String newValue) {}

		 int intValue() 
		{
			try 
			{
				throw new Exception("intValue() not implemented");
			} catch (Exception e) {
				e.printStackTrace();
			}
			
			return 0;
		}

		 int intLongValue() 
		{
			try 
			{
				throw new Exception("intLongValue() not implemented");
			} catch (Exception e) {
				e.printStackTrace();
			}
			
			return 0;
		}

		 String asciiValue() 
		{
			try 
			{
				throw new Exception("asciiValue() not implemented");
			} catch (Exception e) {
				e.printStackTrace();
			}
			
			return null;
		}

		 boolean booleanValue() {
			try 
			{
				throw new Exception("booleanValue() not implemented");
			} catch (Exception e) {
				e.printStackTrace();
			}
			
			return false;
		}
	}

	class IntConfigElement extends ConfigElement
	{
		int intValue;
		Integer intObject;
		
		 IntConfigElement(XMLnode parent, int type, int value) 
		{
			super( parent);
			this._type = type;
			intValue = value;
		}

		@Override
		 Object value() 
		{
			if( intObject == null )
				intObject = new Integer(intValue);
			
			return intObject;
		}
		
		 boolean booleanValue() 
		{
			Integer intValue = (Integer) value();
			if( intValue.intValue() == 1 )
				return true;
			else
				return false;
		}
		
		 int intValue() 
		{
			return( ((Integer)value()).intValue() );
		}
	}

	class LongConfigElement extends ConfigElement
	{
		long longValue;
		Long longObject;

		 LongConfigElement(XMLnode parent, int type, long value) 
		{
			super( parent);
			this._type = type;
			longValue = value;
		}

		 long longValue()
		{
			return longValue;
		}

		 int intLongValue() 
		{
			return (int) longValue;
		}

		@Override
		 Object value() 
		{
			if( longObject == null )
				longObject = new Long(longValue);
			
			return longObject;
		}
		
		 boolean booleanValue() 
		{
			Long longValue = (Long) value();
			if( longValue.longValue() == 1 )
				return true;
			else
				return false;
		}
	}

	class AsciiConfigElement extends ConfigElement
	{
		String asciiValue;
		
		 AsciiConfigElement(XMLnode parent, String value) 
		{
			super( parent);
			asciiValue = value;
		}
		
		 String asciiValue()
		{
			return asciiValue; 
		}

		@Override
		 Object value() 
		{
			return asciiValue;
		}
		
		 void setAsciiValue(String value) 
		{
			asciiValue = value;
		}
	}
}