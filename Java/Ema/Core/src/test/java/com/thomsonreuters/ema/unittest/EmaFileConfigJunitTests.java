package com.thomsonreuters.ema.unittest;


import com.thomsonreuters.ema.access.JUnitTestConnect;
import com.thomsonreuters.ema.access.Msg;
import com.thomsonreuters.ema.access.OmmConsumer;

import junit.framework.TestCase;
import com.thomsonreuters.ema.access.OmmConsumerConfig;
import com.thomsonreuters.ema.access.OmmException;
import com.thomsonreuters.ema.access.OmmProvider;
import com.thomsonreuters.ema.access.OmmProviderClient;
import com.thomsonreuters.ema.access.OmmProviderEvent;
import com.thomsonreuters.ema.access.OmmState;
import com.thomsonreuters.ema.access.PostMsg;
import com.thomsonreuters.ema.access.RefreshMsg;
import com.thomsonreuters.ema.access.ReqMsg;
import com.thomsonreuters.ema.access.StatusMsg;
import com.thomsonreuters.ema.rdm.EmaRdm;
import com.thomsonreuters.ema.access.EmaFactory;
import com.thomsonreuters.ema.access.GenericMsg;

public class EmaFileConfigJunitTests extends TestCase  
{
	
	public static final int ChannelTypeSocket = 0;
	public static final int ChannelTypeEncrypted = 1;
	public static final int ChannelTypeHttp = 2;
	public static final int ChannelTypeMcast = 4;
	public static final int CompressionTypeNone = 0;
	public static final int CompressionTypeZLib = 1;
	public static final int CompressionTypeLZ4 = 2;
	
	public EmaFileConfigJunitTests(String name)
	{
		super(name);
	}
	
	public void testLoadingConfigurarionsFromFile()
	{
		TestUtilities.printTestHead("testLoadingConfigurarionsFromFile","Test loading all configuration parameters from the EmaConfig.xml file");

		// To specify EmaConfig.xml file location use -DEmaConfigFileLocation=EmaConfig.xml
		String EmaConfigFileLocation = System.getProperty("EmaConfigFileLocation");
		if ( EmaConfigFileLocation == null )
		{
			EmaConfigFileLocation = "./src/test/resources/com/thomsonreuters/ema/unittest/EmaFileConfigTests/EmaConfigTest.xml";
			System.out.println("EmaConfig.xml file not specified, using default file");
		}
		else
		{
			System.out.println("Using Ema Config: " + EmaConfigFileLocation);
		}
		
		OmmConsumerConfig testConfig = EmaFactory.createOmmConsumerConfig(EmaConfigFileLocation);
		
		// Check default consumer name (Conusmer_2) and associated values
		System.out.println("Retrieving DefaultConsumer configuration values: (DefaultConsumer value=Consumer_2) "); 
	
		String defaultConsName = JUnitTestConnect.configGetConsumerName(testConfig);
		TestUtilities.checkResult("DefaultConsumer value != null", defaultConsName != null);
		TestUtilities.checkResult("DefaultConsumer value == Consumer_2", defaultConsName.contentEquals("Consumer_2") );
		String ConsChannelVal = JUnitTestConnect.configGetChannelName(testConfig, defaultConsName);
		TestUtilities.checkResult("Channel value != null", ConsChannelVal != null);
		TestUtilities.checkResult("Channel value == Channel_2", ConsChannelVal.contentEquals("Channel_2") );
		String ConsDictionary = JUnitTestConnect.configGetDictionaryName(testConfig, defaultConsName);
		TestUtilities.checkResult("Dictionary != null", ConsDictionary != null);
		TestUtilities.checkResult("Dictionary value == Dictionary_2", ConsDictionary.contentEquals("Dictionary_2") );
		int intLongValue = JUnitTestConnect.configGetIntLongValue(testConfig, defaultConsName, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ConsumerItemCountHint);
		TestUtilities.checkResult("ItemCountHint value == 500000", intLongValue == 500000 );
		intLongValue = JUnitTestConnect.configGetIntLongValue(testConfig, defaultConsName, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ConsumerServiceCountHint);
		TestUtilities.checkResult("ServiceCountHint value == 655", intLongValue == 655 );
		intLongValue = JUnitTestConnect.configGetIntLongValue(testConfig, defaultConsName, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ConsumerObeyOpenWindow);
		intLongValue = JUnitTestConnect.configGetIntLongValue(testConfig, defaultConsName, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ConsumerPostAckTimeout);
		TestUtilities.checkResult("PostAckTimeout value == 7000", intLongValue == 7000 );
		intLongValue = JUnitTestConnect.configGetIntLongValue(testConfig, defaultConsName, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ConsumerRequestTimeout);
		TestUtilities.checkResult("RequestTimeout value == 8000", intLongValue == 8000 );
		intLongValue = JUnitTestConnect.configGetIntLongValue(testConfig, defaultConsName, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ConsumerMaxOutstandingPosts);
		TestUtilities.checkResult("MaxOutstandingPosts value == 90000", intLongValue == 90000 );
		int intValue = JUnitTestConnect.configGetIntValue(testConfig, defaultConsName, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ConsumerDispatchTimeoutApiThread);
		TestUtilities.checkResult("DispatchTimeoutApiThread value == 90", intValue == 90 );

		intLongValue = JUnitTestConnect.configGetIntLongValue(testConfig, defaultConsName, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ConsumerMaxDispatchCountApiThread);
		TestUtilities.checkResult("MaxDispatchCountApiThread value == 400", intLongValue == 400 );
		intLongValue = JUnitTestConnect.configGetIntLongValue(testConfig, defaultConsName, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ConsumerMaxDispatchCountUserThread);
		TestUtilities.checkResult("MaxDispatchCountUserThread value == 5", intLongValue == 5 );

		intValue = JUnitTestConnect.configGetIntValue(testConfig, defaultConsName, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ConsumerReconnectAttemptLimit);
		TestUtilities.checkResult("ReconnectAttemptLimit == 5", intValue == 10);
		intValue = JUnitTestConnect.configGetIntValue(testConfig, defaultConsName, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ConsumerReconnectMinDelay);
		TestUtilities.checkResult("ReconnectMinDelay == 330", intValue == 123);
		intValue = JUnitTestConnect.configGetIntValue(testConfig, defaultConsName, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ConsumerReconnectMaxDelay);
		TestUtilities.checkResult("ReconnectMaxDelay == 450", intValue == 456);
		String strValue = JUnitTestConnect.configGetStringValue(testConfig, defaultConsName, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ConsumerXmlTraceFileName);
		TestUtilities.checkResult("XmlTraceFileName == EmaMyTrace2", strValue.contentEquals("EmaMyTrace2"));
		intLongValue = JUnitTestConnect.configGetIntLongValue(testConfig, defaultConsName, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ConsumerXmlTraceMaxFileSize); 
		TestUtilities.checkResult("XmlTraceMaxFileSize == 1", intLongValue == 66666666);
		Boolean boolValue = JUnitTestConnect.configGetBooleanValue(testConfig, defaultConsName, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ConsumerXmlTraceToFile);
		TestUtilities.checkResult("XmlTraceToFile == 1", boolValue == true);
		boolValue = JUnitTestConnect.configGetBooleanValue(testConfig, defaultConsName, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ConsumerXmlTraceToStdout);
		TestUtilities.checkResult("XmlTraceToStdout == 0", boolValue == false);
		boolValue = JUnitTestConnect.configGetBooleanValue(testConfig, defaultConsName, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ConsumerXmlTraceToMultipleFiles);
		TestUtilities.checkResult("XmlTraceToMultipleFiles == 1", boolValue == true);
		boolValue = JUnitTestConnect.configGetBooleanValue(testConfig, defaultConsName, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ConsumerXmlTraceWrite);
		TestUtilities.checkResult("XmlTraceWrite == 1", boolValue == true);
		boolValue = JUnitTestConnect.configGetBooleanValue(testConfig, defaultConsName, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ConsumerXmlTraceRead);
		TestUtilities.checkResult("XmlTraceRead == 1", boolValue == true);
		boolValue = JUnitTestConnect.configGetBooleanValue(testConfig, defaultConsName, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ConsumerXmlTracePing);
		TestUtilities.checkResult("XmlTracePing == 1", boolValue == true);
		boolValue = JUnitTestConnect.configGetBooleanValue(testConfig, defaultConsName, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ConsumerXmlTraceHex);
		TestUtilities.checkResult("XmlTraceHex == 1", boolValue == true);
		boolValue = JUnitTestConnect.configGetBooleanValue(testConfig, defaultConsName, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ConsumerMsgKeyInUpdates);
		TestUtilities.checkResult("MsgKeyInUpdates == 1", boolValue == true);
		
		
		// Check values of Consumer_1
		System.out.println("\nRetrieving Consumer_1 configuration values "); 

		ConsChannelVal = JUnitTestConnect.configGetChannelName(testConfig, "Consumer_1");
		TestUtilities.checkResult("Channel value != null", ConsChannelVal != null);
		TestUtilities.checkResult("Channel value == Channel_1", ConsChannelVal.contentEquals("Channel_1") );
		ConsDictionary = JUnitTestConnect.configGetDictionaryName(testConfig, "Consumer_1");
		TestUtilities.checkResult("Dictionary != null", ConsDictionary != null);
		TestUtilities.checkResult("Dictionary value == Dictionary_1", ConsDictionary.contentEquals("Dictionary_1") );
		
		// Check values of Consumer_3
		System.out.println("\nRetrieving Consumer_3 configuration values "); 

		ConsChannelVal = JUnitTestConnect.configGetChannelName(testConfig, "Consumer_3");
		TestUtilities.checkResult("Channel value != null", ConsChannelVal != null);
		TestUtilities.checkResult("Channel value == Channel_1", ConsChannelVal.contentEquals("Channel_1") );
		ConsDictionary = JUnitTestConnect.configGetDictionaryName(testConfig, "Consumer_3");
		TestUtilities.checkResult("Dictionary != null", ConsDictionary != null);
		TestUtilities.checkResult("Dictionary value == Dictionary_1", ConsDictionary.contentEquals("Dictionary_1") );

		// Check values of Consumer_4
		System.out.println("\nRetrieving Consumer_4 configuration values "); 

		ConsChannelVal =  JUnitTestConnect.configGetChannelName(testConfig, "Consumer_4");
		TestUtilities.checkResult("ChannelSet value != null", ConsChannelVal != null);
		TestUtilities.checkResult("ChannelSet value == Channel_4, Channel_5", ConsChannelVal.contentEquals("Channel_4,Channel_5") );
		ConsDictionary = JUnitTestConnect.configGetDictionaryName(testConfig, "Consumer_4");
		TestUtilities.checkResult("Dictionary != null", ConsDictionary != null);
		TestUtilities.checkResult("Dictionary value == Dictionary_1", ConsDictionary.contentEquals("Dictionary_1") );

		// Check values of Consumer_5
		System.out.println("\nRetrieving Consumer_5 configuration values "); 

		ConsChannelVal = JUnitTestConnect.configGetChannelName(testConfig, "Consumer_5");
		TestUtilities.checkResult("Channel value != null", ConsChannelVal != null);
		TestUtilities.checkResult("Channel value == Channel_3", ConsChannelVal.contentEquals("Channel_3") );
		ConsDictionary = JUnitTestConnect.configGetDictionaryName(testConfig, "Consumer_5");
		TestUtilities.checkResult("Dictionary != null", ConsDictionary != null);
		TestUtilities.checkResult("Dictionary value == Dictionary_2", ConsDictionary.contentEquals("Dictionary_2") );
	
		// Check Channel configuration:
		// Check Channel_1 configuration.
		ConsChannelVal = "Channel_1";
		System.out.println("\nRetrieving Channel_1 configuration values "); 
		int channelConnType = JUnitTestConnect.configGetChannelType(testConfig, ConsChannelVal);
		TestUtilities.checkResult("channelConnType == ChannelType::RSSL_SOCKET", channelConnType == ChannelTypeSocket);
	
		intValue = JUnitTestConnect.configGetIntValue(testConfig, ConsChannelVal, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelCompressionType);
		TestUtilities.checkResult("CompressionType == CompressionType::None", intValue == CompressionTypeNone);
		
		intLongValue = JUnitTestConnect.configGetIntLongValue(testConfig, ConsChannelVal, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelGuaranteedOutputBuffers);
		TestUtilities.checkResult("GuaranteedOutputBuffers == 5000", intLongValue == 5000);
		intLongValue = JUnitTestConnect.configGetIntLongValue(testConfig, ConsChannelVal, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelNumInputBuffers);
		TestUtilities.checkResult("NumInputBuffers == 7000", intLongValue == 7000);
		intLongValue = JUnitTestConnect.configGetIntLongValue(testConfig, ConsChannelVal, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelSysRecvBufSize);
		TestUtilities.checkResult("SysRecvBufSize == 125236", intLongValue == 125236);
		intLongValue = JUnitTestConnect.configGetIntLongValue(testConfig, ConsChannelVal, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelSysSendBufSize);
		TestUtilities.checkResult("SysSendBufSize == 569823", intLongValue == 569823);
		intLongValue = JUnitTestConnect.configGetIntLongValue(testConfig, ConsChannelVal, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelHighWaterMark);
		TestUtilities.checkResult("HighWaterMark == 3000", intLongValue == 3000);
		intLongValue = JUnitTestConnect.configGetIntLongValue(testConfig, ConsChannelVal, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelCompressionThreshold);
		TestUtilities.checkResult("CompressionThreshold == 2048", intLongValue == 2048);
		intLongValue = JUnitTestConnect.configGetIntLongValue(testConfig, ConsChannelVal, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelConnectionPingTimeout);
		TestUtilities.checkResult("ConnectionPingTimeout == 30000", intLongValue == 30000);
		boolValue = JUnitTestConnect.configGetBooleanValue(testConfig, ConsChannelVal, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelTcpNodelay);
		TestUtilities.checkResult("TcpNodelay == 1", boolValue == true);
		boolValue = JUnitTestConnect.configGetBooleanValue(testConfig, ConsChannelVal, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelDirectWrite);
		TestUtilities.checkResult("DirectWrite == 1", boolValue == true);
		String chanHost = JUnitTestConnect.configGetChanHost(testConfig, ConsChannelVal);
		TestUtilities.checkResult("Host == 0.0.0.1", chanHost.contentEquals("0.0.0.1"));
		String chanPort = JUnitTestConnect.configGetChanPort(testConfig, ConsChannelVal);
		TestUtilities.checkResult("Port == 19001", chanPort.contentEquals("19001"));
		
		// Check Channel_2 configuration.
		ConsChannelVal = "Channel_2";
		System.out.println("\nRetrieving Channel_2 configuration values "); 
		channelConnType = JUnitTestConnect.configGetChannelType(testConfig, ConsChannelVal);
		//change to Socket connection due to non-support conn type.
		//TestUtilities.checkResult("channelConnType == ChannelType::RSSL_ENCRYPTED", channelConnType == ChannelTypeEncrypted);
		TestUtilities.checkResult("channelConnType == ChannelType::RSSL_SOCKET", channelConnType == ChannelTypeSocket);
		strValue = JUnitTestConnect.configGetStringValue(testConfig, ConsChannelVal, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelInterfaceName);
		TestUtilities.checkResult("InterfaceName == localhost4file", strValue.contentEquals("localhost4file"));
		intValue = JUnitTestConnect.configGetIntValue(testConfig, ConsChannelVal, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelCompressionType);
		TestUtilities.checkResult("CompressionType == CompressionType::Zlib", intValue == CompressionTypeZLib);
		intLongValue = JUnitTestConnect.configGetIntLongValue(testConfig, ConsChannelVal, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelGuaranteedOutputBuffers);
		TestUtilities.checkResult("GuaranteedOutputBuffers == 6000", intLongValue == 6000);
		intLongValue = JUnitTestConnect.configGetIntLongValue(testConfig, ConsChannelVal, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelNumInputBuffers);
		TestUtilities.checkResult("NumInputBuffers == 9000", intLongValue == 9000);
		intLongValue = JUnitTestConnect.configGetIntLongValue(testConfig, ConsChannelVal, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelSysRecvBufSize);
		TestUtilities.checkResult("SysRecvBufSize == 23656", intLongValue == 23656);
		intLongValue = JUnitTestConnect.configGetIntLongValue(testConfig, ConsChannelVal, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelSysSendBufSize);
		TestUtilities.checkResult("SysSendBufSize == 63656", intLongValue == 63656);
		intLongValue = JUnitTestConnect.configGetIntLongValue(testConfig, ConsChannelVal, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelCompressionThreshold);
		TestUtilities.checkResult("CompressionThreshold == 4096", intLongValue == 4096);
		intLongValue = JUnitTestConnect.configGetIntLongValue(testConfig, ConsChannelVal, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelConnectionPingTimeout);
		TestUtilities.checkResult("ConnectionPingTimeout == 55555", intLongValue == 55555);
		chanHost = JUnitTestConnect.configGetChanHost(testConfig, ConsChannelVal);
		TestUtilities.checkResult("Host == 0.0.0.2", chanHost.contentEquals("0.0.0.2"));
		chanPort = JUnitTestConnect.configGetChanPort(testConfig, ConsChannelVal);
		TestUtilities.checkResult("Port == 15008", chanPort.contentEquals("15008"));
		boolValue = JUnitTestConnect.configGetBooleanValue(testConfig, ConsChannelVal, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelTcpNodelay);
		TestUtilities.checkResult("TcpNodelay == 0", boolValue == false);
		strValue = JUnitTestConnect.configGetStringValue(testConfig, ConsChannelVal, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelObjectName);
		TestUtilities.checkResult("ObjectName == HttpObjectName", strValue.contentEquals("HttpObjectName"));

		// Check Channel_3 configuration.
		ConsChannelVal = "Channel_3";
		System.out.println("\nRetrieving Channel_3 configuration values "); 
		channelConnType = JUnitTestConnect.configGetChannelType(testConfig, ConsChannelVal);
		//change to Socket connection due to non-support conn type.
		//TestUtilities.checkResult("channelConnType == ChannelType::RSSL_RELIABLE_MCAST", channelConnType == ChannelTypeMcast);
		TestUtilities.checkResult("channelConnType == ChannelType::RSSL_SOCKET", channelConnType == ChannelTypeSocket);
		strValue = JUnitTestConnect.configGetStringValue(testConfig, ConsChannelVal, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelInterfaceName);
		TestUtilities.checkResult("InterfaceName != null", strValue != null);
		TestUtilities.checkResult("InterfaceName == localhost", strValue.contentEquals("localhost"));
		strValue = JUnitTestConnect.configGetStringValue(testConfig, ConsChannelVal, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelUnicastPort);
		TestUtilities.checkResult("Unicastport != null", strValue != null);
		TestUtilities.checkResult("UnicastPort == 40102", strValue.contentEquals("40102"));
		strValue = JUnitTestConnect.configGetStringValue(testConfig, ConsChannelVal, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelRecvAddress);
		TestUtilities.checkResult("RecvAddress != null", strValue != null);
		TestUtilities.checkResult("RecvAddress == 0.0.0.3", strValue.contentEquals("0.0.0.3"));
		strValue = JUnitTestConnect.configGetStringValue(testConfig, ConsChannelVal, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelRecvPort);
		TestUtilities.checkResult("RecvPort != null", strValue != null);
		TestUtilities.checkResult("RecvPort == 15008", strValue.contentEquals("15008"));
		strValue = JUnitTestConnect.configGetStringValue(testConfig, ConsChannelVal, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelSendAddress);
		TestUtilities.checkResult("SendAddress != null", strValue != null);
		TestUtilities.checkResult("SendAddress == 0.0.0.4", strValue.contentEquals("0.0.0.4"));
		strValue = JUnitTestConnect.configGetStringValue(testConfig, ConsChannelVal, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelSendPort);
		TestUtilities.checkResult("SendPort != null", strValue != null);
		TestUtilities.checkResult("SendPort == 15007", strValue.contentEquals("15007"));
		strValue = JUnitTestConnect.configGetStringValue(testConfig, ConsChannelVal, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelHsmInterface);
		TestUtilities.checkResult("HsmInterface != null", strValue != null);
		TestUtilities.checkResult("HsmInterface == 0.0.0.5", strValue.contentEquals("0.0.0.5"));
		strValue = JUnitTestConnect.configGetStringValue(testConfig, ConsChannelVal, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelHsmMultAddress);
		TestUtilities.checkResult("HsmMultAddress != null", strValue != null);
		TestUtilities.checkResult("HsmMultAddress == 0.0.0.6", strValue.contentEquals("0.0.0.6"));
		strValue = JUnitTestConnect.configGetStringValue(testConfig, ConsChannelVal, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelHsmPort);
		TestUtilities.checkResult("HsmPort != null", strValue != null);
		TestUtilities.checkResult("HsmPort == 15005", strValue.contentEquals("15005"));
		strValue = JUnitTestConnect.configGetStringValue(testConfig, ConsChannelVal, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChanneltcpControlPort);
		TestUtilities.checkResult("TcpControlPort != null", strValue != null);
		TestUtilities.checkResult("TcpControlPort == 15018", strValue.contentEquals("15018"));
		intLongValue = JUnitTestConnect.configGetIntLongValue(testConfig, ConsChannelVal, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelPacketTTL);
		TestUtilities.checkResult("PacketTTL == 10", intLongValue == 10);
		boolValue = JUnitTestConnect.configGetBooleanValue(testConfig, ConsChannelVal, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelDisconnectOnGap);
		TestUtilities.checkResult("DisconnectOnGap == 1", boolValue == true);
		intLongValue = JUnitTestConnect.configGetIntLongValue(testConfig, ConsChannelVal, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.Channelndata);
		TestUtilities.checkResult("ndata == 8", intLongValue == 8);
		intLongValue = JUnitTestConnect.configGetIntLongValue(testConfig, ConsChannelVal, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.Channelnmissing);
		TestUtilities.checkResult("nmissing == 130", intLongValue == 130);
		intLongValue = JUnitTestConnect.configGetIntLongValue(testConfig, ConsChannelVal, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.Channelnrreq);
		TestUtilities.checkResult("nrreq == 5", intLongValue == 5);
		intLongValue = JUnitTestConnect.configGetIntLongValue(testConfig, ConsChannelVal, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.Channeltdata);
		TestUtilities.checkResult("tdata == 0", intLongValue == 0);
		intLongValue = JUnitTestConnect.configGetIntLongValue(testConfig, ConsChannelVal, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.Channeltrreq);
		TestUtilities.checkResult("trreq == 5", intLongValue == 5);
		intLongValue = JUnitTestConnect.configGetIntLongValue(testConfig, ConsChannelVal, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelpktPoolLimitHigh);
		TestUtilities.checkResult("pktPoolLimitHigh == 190500", intLongValue == 190500);
		intLongValue = JUnitTestConnect.configGetIntLongValue(testConfig, ConsChannelVal, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelpktPoolLimitLow);
		TestUtilities.checkResult("pktPoolLimitLow == 180500", intLongValue == 180500);
		intLongValue = JUnitTestConnect.configGetIntLongValue(testConfig, ConsChannelVal, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.Channeltwait);
		TestUtilities.checkResult("twait == 4", intLongValue == 4);
		intLongValue = JUnitTestConnect.configGetIntLongValue(testConfig, ConsChannelVal, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.Channeltbchold);
		TestUtilities.checkResult("tbchold == 4", intLongValue == 4);
		intLongValue = JUnitTestConnect.configGetIntLongValue(testConfig, ConsChannelVal, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.Channeltpphold);
		TestUtilities.checkResult("tpphold == 4", intLongValue == 4);
		intLongValue = JUnitTestConnect.configGetIntLongValue(testConfig, ConsChannelVal, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChanneluserQLimit);
		TestUtilities.checkResult("userQLimit == 65535", intLongValue == 65535);
		intLongValue = JUnitTestConnect.configGetIntLongValue(testConfig, ConsChannelVal, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelHsmInterval);
		TestUtilities.checkResult("HsmInterval == 10", intLongValue == 10);
		intLongValue = JUnitTestConnect.configGetIntLongValue(testConfig, ConsChannelVal, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelGuaranteedOutputBuffers);
		TestUtilities.checkResult("GuaranteedOutputBuffers == 5500", intLongValue == 5500);
		intLongValue = JUnitTestConnect.configGetIntLongValue(testConfig, ConsChannelVal, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelNumInputBuffers);
		TestUtilities.checkResult("NumInputBuffers == 9500", intLongValue == 9500);
		intLongValue = JUnitTestConnect.configGetIntLongValue(testConfig, ConsChannelVal, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelSysRecvBufSize);
		TestUtilities.checkResult("SysRecvBufSize == 125000", intLongValue == 125000);
		intLongValue = JUnitTestConnect.configGetIntLongValue(testConfig, ConsChannelVal, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelSysSendBufSize);
		TestUtilities.checkResult("SysSendBufSize == 550000", intLongValue == 550000);
		intLongValue = JUnitTestConnect.configGetIntLongValue(testConfig, ConsChannelVal, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelConnectionPingTimeout);
		TestUtilities.checkResult("ConnectionPingTimeout == 3555", intLongValue == 3555);
	
		// Check Dictionary_1 configuration.
		ConsDictionary = "Dictionary_1";
		System.out.println("\nRetrieving Dictionary_1 configuration values ");
		intValue = JUnitTestConnect.configGetIntValue(testConfig, ConsDictionary, JUnitTestConnect.ConfigGroupTypeDictionary, JUnitTestConnect.DictionaryType);
		TestUtilities.checkResult("DictionaryType == DictionaryType::ChannelDictionary (0)", intValue == 0);		
	
		// Check Dictionary_2 configuration.
		ConsDictionary = "Dictionary_2";
		System.out.println("\nRetrieving Dictionary_2 configuration values ");
		intValue = JUnitTestConnect.configGetIntValue(testConfig, ConsDictionary, JUnitTestConnect.ConfigGroupTypeDictionary, JUnitTestConnect.DictionaryType);
		TestUtilities.checkResult("DictionaryType == DictionaryType::FileDictionary (1)", intValue == 1);		
		strValue =  JUnitTestConnect.configGetStringValue(testConfig, ConsDictionary, JUnitTestConnect.ConfigGroupTypeDictionary, JUnitTestConnect.DictionaryRDMFieldDictFileName);
		TestUtilities.checkResult("RdmFieldDictionaryFileName == ./RDMFieldDictionary_2", strValue.contentEquals("./RDMFieldDictionary_2"));		
		strValue =   JUnitTestConnect.configGetStringValue(testConfig, ConsDictionary, JUnitTestConnect.ConfigGroupTypeDictionary, JUnitTestConnect.DictionaryEnumTypeDefFileName);
		TestUtilities.checkResult("RdmFieldDictionaryFileName == ./enumtype_2.def", strValue.contentEquals("./enumtype_2.def"));		

		// Check user specified host and port.		
		ConsChannelVal = "Channel_2";
		System.out.println("\nCheck user specified host and port "); 
		String tmpString = "usrLocalhost:usr14002";
		testConfig.host(tmpString);
		channelConnType = JUnitTestConnect.configGetChannelType(testConfig, ConsChannelVal);
		TestUtilities.checkResult("channelConnType == ChannelType::RSSL_SOCKET", channelConnType == ChannelTypeSocket);
		chanHost = JUnitTestConnect.configGetChanHost(testConfig, ConsChannelVal);
		TestUtilities.checkResult("Host == usrLocalhost", chanHost.contentEquals("usrLocalhost"));
		chanPort = JUnitTestConnect.configGetChanPort(testConfig, ConsChannelVal);
		TestUtilities.checkResult("Port == usr14002", chanPort.contentEquals("usr14002"));
	
	}
	
	public void testLoadFromFileCfgChannelSet()
	{
		String consumerName = "";
		TestUtilities.printTestHead("testLoadFromFileCfgChannelSet","Test reading Channel and ChannelSet when both parameters are configured in EmaConfig.xml file");

		// To specify EmaConfig.xml file location use -DEmaConfigFileLocation=EmaConfig.xml
		String EmaConfigFileLocation = System.getProperty("EmaConfigFileLocation");
		if ( EmaConfigFileLocation == null )
		{
			EmaConfigFileLocation = "./src/test/resources/com/thomsonreuters/ema/unittest/EmaFileConfigTests/EmaConfigTest.xml";
			System.out.println("EmaConfig.xml file not specified, using default file");
		}
		else
		{
			System.out.println("Using Ema Config: " + EmaConfigFileLocation);
		}
		
		OmmConsumerConfig testConfig = EmaFactory.createOmmConsumerConfig(EmaConfigFileLocation);
		
		// Consumer_3 ChannelSet values
		System.out.println("Retrieving Consumer_3 both ChannelSet & Channel configuration values: (Channel after ChannelSet) "); 
		
		consumerName = "Consumer_3";
		String cons3Channel = JUnitTestConnect.configGetChannelName(testConfig, consumerName);
		TestUtilities.checkResult("Channel value=Channel_1", cons3Channel.contentEquals("Channel_1"));
	
		// Consumer_4 ChannelSet values
		System.out.println("Retrieving Consumer_4 both ChannelSet & Channel configuration values: (ChannelSet after Channel) "); 
		
		consumerName = "Consumer_4";
		String cons4Channel = JUnitTestConnect.configGetChannelName(testConfig, consumerName);
		TestUtilities.checkResult("ChannelSet value=Channel_4, Channel_5", cons4Channel.contentEquals("Channel_4,Channel_5"));
		
		// Test Individual Channel attributes and Common attributes.
		String tempTestName = "channelConnType == ChannelType::RSSL_SOCKET ";
		
		//the follow code was commented out due to non-support conn type.
//		String channelName = pieces[0];	// Channel_4
//		int channelConnType = JUnitTestConnect.configGetChannelType(testConfig, channelName);
//		tempTestName.concat(channelName);
//		TestUtilities.checkResult(tempTestName, channelConnType == ChannelTypeSocket);
//		
//		channelName = pieces[1];	// Channel_5
//		channelConnType = JUnitTestConnect.configGetChannelType(testConfig, channelName);
//		tempTestName = "channelConnType == ChannelType::RSSL_ENCRYPTED ";
//		tempTestName.concat(channelName);
//		TestUtilities.checkResult(tempTestName, channelConnType == ChannelTypeEncrypted);
//		
		// Verify config after the connection.
		System.out.println( "Must connect to a provider with localhost and 14002 port\n" );
		System.out.println( "Must load data dictionary files from current working location\n" );

		consumerName = "Consumer_6";
		OmmConsumer chanelSetConsumer = null;
		testConfig.consumerName(consumerName);
		try 
		{
			OmmProvider provider = EmaFactory.createOmmProvider(EmaFactory.createOmmIProviderConfig(), new AppClient());
			
			chanelSetConsumer = EmaFactory.createOmmConsumer(testConfig);
			
			int result = JUnitTestConnect.configVerifyConsChannelSetAttribs(chanelSetConsumer, testConfig, consumerName);
			tempTestName = "VerifyIndividualChannelAttributes are valid";
			TestUtilities.checkResult(tempTestName, result == 0);		
			
			chanelSetConsumer.uninitialize();
			provider.uninitialize();
		}
		catch ( OmmException excp)
		{
			System.out.println(excp.getMessage());
		}
	}
	
	class AppClient implements OmmProviderClient
	{
		public long itemHandle = 0;
		
		public void onReqMsg(ReqMsg reqMsg, OmmProviderEvent event)
		{
			switch (reqMsg.domainType())
			{
				case EmaRdm.MMT_LOGIN :
					processLoginRequest(reqMsg, event);
					break;
				default :
					break;
			}
		}
		
		public void onRefreshMsg(RefreshMsg refreshMsg,	OmmProviderEvent event){}
		public void onStatusMsg(StatusMsg statusMsg, OmmProviderEvent event){}
		public void onGenericMsg(GenericMsg genericMsg, OmmProviderEvent event){}
		public void onPostMsg(PostMsg postMsg, OmmProviderEvent event){}
		public void onReissue(ReqMsg reqMsg, OmmProviderEvent event){}
		public void onClose(ReqMsg reqMsg, OmmProviderEvent event){}
		public void onAllMsg(Msg msg, OmmProviderEvent event){}
		
		void processLoginRequest(ReqMsg reqMsg, OmmProviderEvent event)
		{
			event.provider().submit( EmaFactory.createRefreshMsg().domainType(EmaRdm.MMT_LOGIN).name(reqMsg.name()).
					nameType(EmaRdm.USER_NAME).complete(true).solicited(true).
					state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "Login accepted"),
					event.handle() );
		}
	}
}
