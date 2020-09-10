package com.rtsdk.ema.access;

import junit.framework.TestCase;
import com.rtsdk.ema.unittest.TestUtilities;
import com.rtsdk.eta.codec.Qos;
import com.rtsdk.eta.codec.QosRates;
import com.rtsdk.eta.codec.QosTimeliness;
import com.rtsdk.eta.valueadd.domainrep.rdm.directory.Service;
import com.rtsdk.eta.valueadd.domainrep.rdm.directory.Service.ServiceInfo;
import com.rtsdk.eta.valueadd.domainrep.rdm.directory.Service.ServiceState;

import java.util.List;

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
	
	public void testLoadCfgFromFile()
	{
		TestUtilities.printTestHead("testLoadCfgFromFile","Test loading all configuration parameters from the EmaConfig.xml file");

		// To specify EmaConfig.xml file location use -DEmaConfigFileLocation=EmaConfig.xml
		String EmaConfigFileLocation = System.getProperty("EmaConfigFileLocation");
		if ( EmaConfigFileLocation == null )
		{
			EmaConfigFileLocation = "./src/test/resources/com/rtsdk/ema/unittest/EmaFileConfigTests/EmaConfigTest.xml";
			System.out.println("EmaConfig.xml file not specified, using default file");
		}
		else
		{
			System.out.println("Using Ema Config: " + EmaConfigFileLocation);
		}
		
		OmmConsumerConfigImpl testConfig = (OmmConsumerConfigImpl) EmaFactory.createOmmConsumerConfig(EmaConfigFileLocation);
		
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
		int intLongValue = JUnitTestConnect.configGetIntLongValue(testConfig, defaultConsName, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ItemCountHint);
		TestUtilities.checkResult("ItemCountHint value == 500000", intLongValue == 500000 );
		intLongValue = JUnitTestConnect.configGetIntLongValue(testConfig, defaultConsName, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ServiceCountHint);
		TestUtilities.checkResult("ServiceCountHint value == 655", intLongValue == 655 );
		intLongValue = JUnitTestConnect.configGetIntLongValue(testConfig, defaultConsName, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ConsumerObeyOpenWindow);
		intLongValue = JUnitTestConnect.configGetIntLongValue(testConfig, defaultConsName, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ConsumerPostAckTimeout);
		TestUtilities.checkResult("PostAckTimeout value == 7000", intLongValue == 7000 );
		intLongValue = JUnitTestConnect.configGetIntLongValue(testConfig, defaultConsName, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.RequestTimeout);
		TestUtilities.checkResult("RequestTimeout value == 8000", intLongValue == 8000 );
		intLongValue = JUnitTestConnect.configGetIntLongValue(testConfig, defaultConsName, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ConsumerMaxOutstandingPosts);
		TestUtilities.checkResult("MaxOutstandingPosts value == 90000", intLongValue == 90000 );
		int intValue = JUnitTestConnect.configGetIntValue(testConfig, defaultConsName, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.DispatchTimeoutApiThread);
		TestUtilities.checkResult("DispatchTimeoutApiThread value == 90", intValue == 90 );
		intValue = testConfig.xmlConfig().getGlobalConfig().getPrimitiveValue(ConfigManager.ReactorMsgEventPoolLimit).intValue();
		TestUtilities.checkResult("ReactorMsgEventPoolLimit value == 2000", intValue == 2000);
		intValue = testConfig.xmlConfig().getGlobalConfig().getPrimitiveValue(ConfigManager.ReactorChannelEventPoolLimit).intValue();
		TestUtilities.checkResult("ReactorChannelEventPoolLimit value == 1500", intValue == 1500);
		intValue = testConfig.xmlConfig().getGlobalConfig().getPrimitiveValue(ConfigManager.WorkerEventPoolLimit).intValue();
		TestUtilities.checkResult("WorkerEventPoolLimit value == 1000", intValue == 1000);
		intValue = testConfig.xmlConfig().getGlobalConfig().getPrimitiveValue(ConfigManager.TunnelStreamMsgEventPoolLimit).intValue();
		TestUtilities.checkResult("TunnelStreamMsgEventPoolLimit value == 2500", intValue == 2500);
		intValue = testConfig.xmlConfig().getGlobalConfig().getPrimitiveValue(ConfigManager.TunnelStreamStatusEventPoolLimit).intValue();
		TestUtilities.checkResult("TunnelStreamStatusEventPoolLimit value == 3000", intValue == 3000);

		intLongValue = JUnitTestConnect.configGetIntLongValue(testConfig, defaultConsName, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.MaxDispatchCountApiThread);
		TestUtilities.checkResult("MaxDispatchCountApiThread value == 400", intLongValue == 400 );
		intLongValue = JUnitTestConnect.configGetIntLongValue(testConfig, defaultConsName, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.MaxDispatchCountUserThread);
		TestUtilities.checkResult("MaxDispatchCountUserThread value == 5", intLongValue == 5 );

		intValue = JUnitTestConnect.configGetIntValue(testConfig, defaultConsName, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ReconnectAttemptLimit);
		TestUtilities.checkResult("ReconnectAttemptLimit == 5", intValue == 10);
		intValue = JUnitTestConnect.configGetIntValue(testConfig, defaultConsName, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ReconnectMinDelay);
		TestUtilities.checkResult("ReconnectMinDelay == 330", intValue == 123);
		intValue = JUnitTestConnect.configGetIntValue(testConfig, defaultConsName, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ReconnectMaxDelay);
		TestUtilities.checkResult("ReconnectMaxDelay == 450", intValue == 456);
		boolean boolValue = JUnitTestConnect.configGetBooleanValue(testConfig, defaultConsName, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.XmlTraceToStdout);
		TestUtilities.checkResult("XmlTraceToStdout == 0", boolValue == false);
		boolValue = JUnitTestConnect.configGetBooleanValue(testConfig, defaultConsName, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ConsumerMsgKeyInUpdates);
		TestUtilities.checkResult("MsgKeyInUpdates == 1", boolValue == true);
		
		intValue = JUnitTestConnect.configGetIntLongValue(testConfig, defaultConsName, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.RestRequestTimeout);
		TestUtilities.checkResult("RestRequestTimeOut == 60000", intValue == 60000);
		intValue = JUnitTestConnect.configGetIntValue(testConfig, defaultConsName, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ReissueTokenAttemptLimit);
		TestUtilities.checkResult("ReissueTokenAttemptLimit == 5", intValue == 5);
		intValue = JUnitTestConnect.configGetIntValue(testConfig, defaultConsName, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ReissueTokenAttemptInterval);
		TestUtilities.checkResult("ReissueTokenAttemptInterval == 7000", intValue == 7000);
		double doubleValue = JUnitTestConnect.configDoubleIntValue(testConfig, defaultConsName, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.TokenReissueRatio);
		TestUtilities.checkResult("TokenReissueRatio == 0.5", doubleValue == 0.5);
		intLongValue = JUnitTestConnect.configGetIntLongValue(testConfig, defaultConsName, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.EnableRtt);
		TestUtilities.checkResult("EnableRtt value > 0", intLongValue > 0 );
		
		// Check values of Consumer_1
		System.out.println("\nRetrieving Consumer_1 configuration values "); 

		ConsChannelVal = JUnitTestConnect.configGetChannelName(testConfig, "Consumer_1");
		TestUtilities.checkResult("Channel value != null", ConsChannelVal != null);
		TestUtilities.checkResult("Channel value == Channel_1", ConsChannelVal.contentEquals("Channel_1") );
		ConsDictionary = JUnitTestConnect.configGetDictionaryName(testConfig, "Consumer_1");
		TestUtilities.checkResult("Dictionary != null", ConsDictionary != null);
		TestUtilities.checkResult("Dictionary value == Dictionary_1", ConsDictionary.contentEquals("Dictionary_1") );
		intLongValue = JUnitTestConnect.configGetIntLongValue(testConfig, "Consumer_1", JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.EnableRtt);
		TestUtilities.checkResult("EnableRtt value == 0", intLongValue == 0 );
		
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
		
		// Check values of Consumer_7
		System.out.println("\nRetrieving Consumer_7 configuration values "); 

		ConsChannelVal = JUnitTestConnect.configGetChannelName(testConfig, "Consumer_7");
		TestUtilities.checkResult("Channel value != null", ConsChannelVal != null);
		TestUtilities.checkResult("Channel value == Channel_6", ConsChannelVal.contentEquals("Channel_6") );
		ConsDictionary = JUnitTestConnect.configGetDictionaryName(testConfig, "Consumer_7");
		TestUtilities.checkResult("Dictionary != null", ConsDictionary != null);
		TestUtilities.checkResult("Dictionary value == Dictionary_2", ConsDictionary.contentEquals("Dictionary_2") );
	
		// Check Channel configuration:
		// Check Channel_1 configuration.
		ConsChannelVal = "Channel_1";
		System.out.println("\nRetrieving Channel_1 configuration values "); 
		int channelConnType = JUnitTestConnect.configGetChannelType(testConfig, ConsChannelVal);
		TestUtilities.checkResult("channelConnType == ChannelType::RSSL_SOCKET", channelConnType == ChannelTypeSocket);
	
		intValue = JUnitTestConnect.configGetIntValue(testConfig, ConsChannelVal, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.CompressionType);
		TestUtilities.checkResult("CompressionType == CompressionType::None", intValue == CompressionTypeNone);
		
		intLongValue = JUnitTestConnect.configGetIntLongValue(testConfig, ConsChannelVal, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.GuaranteedOutputBuffers);
		TestUtilities.checkResult("GuaranteedOutputBuffers == 5000", intLongValue == 5000);
		intLongValue = JUnitTestConnect.configGetIntLongValue(testConfig, ConsChannelVal, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.NumInputBuffers);
		TestUtilities.checkResult("NumInputBuffers == 7000", intLongValue == 7000);
		intLongValue = JUnitTestConnect.configGetIntLongValue(testConfig, ConsChannelVal, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.SysRecvBufSize);
		TestUtilities.checkResult("SysRecvBufSize == 125236", intLongValue == 125236);
		intLongValue = JUnitTestConnect.configGetIntLongValue(testConfig, ConsChannelVal, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.SysSendBufSize);
		TestUtilities.checkResult("SysSendBufSize == 569823", intLongValue == 569823);
		intLongValue = JUnitTestConnect.configGetIntLongValue(testConfig, ConsChannelVal, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.HighWaterMark);
		TestUtilities.checkResult("HighWaterMark == 3000", intLongValue == 3000);
		intLongValue = JUnitTestConnect.configGetIntLongValue(testConfig, ConsChannelVal, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.CompressionThreshold);
		TestUtilities.checkResult("CompressionThreshold == 2048", intLongValue == 2048);
		intLongValue = JUnitTestConnect.configGetIntLongValue(testConfig, ConsChannelVal, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ConnectionPingTimeout);
		TestUtilities.checkResult("ConnectionPingTimeout == 30000", intLongValue == 30000);
		boolValue = JUnitTestConnect.configGetBooleanValue(testConfig, ConsChannelVal, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.TcpNodelay);
		TestUtilities.checkResult("TcpNodelay == 1", boolValue == true);
		boolValue = JUnitTestConnect.configGetBooleanValue(testConfig, ConsChannelVal, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.DirectWrite);
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
		String strValue = JUnitTestConnect.configGetStringValue(testConfig, ConsChannelVal, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.InterfaceName);
		TestUtilities.checkResult("InterfaceName == localhost4file", strValue.contentEquals("localhost4file"));
		intValue = JUnitTestConnect.configGetIntValue(testConfig, ConsChannelVal, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.CompressionType);
		TestUtilities.checkResult("CompressionType == CompressionType::Zlib", intValue == CompressionTypeZLib);
		intLongValue = JUnitTestConnect.configGetIntLongValue(testConfig, ConsChannelVal, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.GuaranteedOutputBuffers);
		TestUtilities.checkResult("GuaranteedOutputBuffers == 6000", intLongValue == 6000);
		intLongValue = JUnitTestConnect.configGetIntLongValue(testConfig, ConsChannelVal, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.NumInputBuffers);
		TestUtilities.checkResult("NumInputBuffers == 9000", intLongValue == 9000);
		intLongValue = JUnitTestConnect.configGetIntLongValue(testConfig, ConsChannelVal, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.SysRecvBufSize);
		TestUtilities.checkResult("SysRecvBufSize == 23656", intLongValue == 23656);
		intLongValue = JUnitTestConnect.configGetIntLongValue(testConfig, ConsChannelVal, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.SysSendBufSize);
		TestUtilities.checkResult("SysSendBufSize == 63656", intLongValue == 63656);
		intLongValue = JUnitTestConnect.configGetIntLongValue(testConfig, ConsChannelVal, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.CompressionThreshold);
		TestUtilities.checkResult("CompressionThreshold == 4096", intLongValue == 4096);
		intLongValue = JUnitTestConnect.configGetIntLongValue(testConfig, ConsChannelVal, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ConnectionPingTimeout);
		TestUtilities.checkResult("ConnectionPingTimeout == 55555", intLongValue == 55555);
		chanHost = JUnitTestConnect.configGetChanHost(testConfig, ConsChannelVal);
		TestUtilities.checkResult("Host == 0.0.0.2", chanHost.contentEquals("0.0.0.2"));
		chanPort = JUnitTestConnect.configGetChanPort(testConfig, ConsChannelVal);
		TestUtilities.checkResult("Port == 15008", chanPort.contentEquals("15008"));
		boolValue = JUnitTestConnect.configGetBooleanValue(testConfig, ConsChannelVal, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.TcpNodelay);
		TestUtilities.checkResult("TcpNodelay == 0", boolValue == false);
		strValue = JUnitTestConnect.configGetStringValue(testConfig, ConsChannelVal, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ObjectName);
		TestUtilities.checkResult("ObjectName == HttpObjectName", strValue.contentEquals("HttpObjectName"));
		intLongValue = JUnitTestConnect.configGetIntLongValue(testConfig, ConsChannelVal, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelInitTimeout);
		TestUtilities.checkResult("InitializationTimeout == 55", intLongValue == 55);

		// Check Channel_3 configuration.
		ConsChannelVal = "Channel_3";
		System.out.println("\nRetrieving Channel_3 configuration values "); 
		channelConnType = JUnitTestConnect.configGetChannelType(testConfig, ConsChannelVal);
		//change to Socket connection due to non-support conn type.
		//TestUtilities.checkResult("channelConnType == ChannelType::RSSL_RELIABLE_MCAST", channelConnType == ChannelTypeMcast);
		TestUtilities.checkResult("channelConnType == ChannelType::RSSL_SOCKET", channelConnType == ChannelTypeSocket);
		strValue = JUnitTestConnect.configGetStringValue(testConfig, ConsChannelVal, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.InterfaceName);
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
		intLongValue = JUnitTestConnect.configGetIntLongValue(testConfig, ConsChannelVal, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.GuaranteedOutputBuffers);
		TestUtilities.checkResult("GuaranteedOutputBuffers == 5500", intLongValue == 5500);
		intLongValue = JUnitTestConnect.configGetIntLongValue(testConfig, ConsChannelVal, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.NumInputBuffers);
		TestUtilities.checkResult("NumInputBuffers == 9500", intLongValue == 9500);
		intLongValue = JUnitTestConnect.configGetIntLongValue(testConfig, ConsChannelVal, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.SysRecvBufSize);
		TestUtilities.checkResult("SysRecvBufSize == 125000", intLongValue == 125000);
		intLongValue = JUnitTestConnect.configGetIntLongValue(testConfig, ConsChannelVal, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.SysSendBufSize);
		TestUtilities.checkResult("SysSendBufSize == 550000", intLongValue == 550000);
		intLongValue = JUnitTestConnect.configGetIntLongValue(testConfig, ConsChannelVal, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ConnectionPingTimeout);
		TestUtilities.checkResult("ConnectionPingTimeout == 3555", intLongValue == 3555);
		
		// Check Channel_6 configuration.
		ConsChannelVal = "Channel_6";
		System.out.println("\nRetrieving Channel_6 configuration values "); 
		channelConnType = JUnitTestConnect.configGetChannelType(testConfig, ConsChannelVal);
		TestUtilities.checkResult("channelConnType == ChannelType::RSSL_ENCRYPTED", channelConnType == ChannelTypeEncrypted);
				
		chanHost = JUnitTestConnect.configGetChanHost(testConfig, ConsChannelVal);
		TestUtilities.checkResult("Host == 122.1.1.200", chanHost.contentEquals("122.1.1.200"));
		chanPort = JUnitTestConnect.configGetChanPort(testConfig, ConsChannelVal);
		TestUtilities.checkResult("Port == 14010", chanPort.contentEquals("14010"));
		strValue = JUnitTestConnect.configGetStringValue(testConfig, ConsChannelVal, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ObjectName);
		TestUtilities.checkResult("ObjectName == EncrpyptedObjectName2", strValue.contentEquals("EncrpyptedObjectName2"));
		strValue = JUnitTestConnect.configGetStringValue(testConfig, ConsChannelVal, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.Location);
		TestUtilities.checkResult("Location == us-east", strValue.contentEquals("us-east"));
		boolValue = JUnitTestConnect.configGetBooleanValue(testConfig, ConsChannelVal, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.EnableSessionMgnt);
		TestUtilities.checkResult("EnableSessionManagement == 1", boolValue == true);
		
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
		TestUtilities.checkResult("RdmFieldDictionaryFileName == ./RDMFieldDictionary", strValue.contentEquals("./RDMFieldDictionary"));		
		strValue =   JUnitTestConnect.configGetStringValue(testConfig, ConsDictionary, JUnitTestConnect.ConfigGroupTypeDictionary, JUnitTestConnect.DictionaryEnumTypeDefFileName);
		TestUtilities.checkResult("RdmFieldDictionaryFileName == ./enumtype.def", strValue.contentEquals("./enumtype.def"));		

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
			EmaConfigFileLocation = "./src/test/resources/com/rtsdk/ema/unittest/EmaFileConfigTests/EmaConfigTest.xml";
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
		
		consumerName = "Consumer_6";
		OmmConsumer chanelSetConsumer = null;
		testConfig.consumerName(consumerName);
		try 
		{
			chanelSetConsumer = JUnitTestConnect.createOmmConsumer(testConfig);
			int result = JUnitTestConnect.configVerifyConsChannelSetAttribs(chanelSetConsumer, testConfig, consumerName);
			tempTestName = "VerifyIndividualChannelAttributes are valid";
			TestUtilities.checkResult(tempTestName, result == 0);		
			chanelSetConsumer = null;
		}
		catch ( OmmException excp)
		{
			System.out.println(excp.getMessage());
			TestUtilities.checkResult("Receiving exception, test failed.", false );
		}
	}
	
	public void testLoadChannelSetBwteenFileProgrammatic()
	{
		TestUtilities.printTestHead("testLoadChannelSetBwteenFileProgrammatic","Test reading Channel and ChannelSet when both parameters are configured programmatcally and from file");

		//testcase 1: test setting channel, then channelset. ChannelSet takes priority.
		//testcase 2: test setting channelset, then channel. Channel takes priority.

		for (int testCase = 1; testCase < 3; testCase++)
		{
			System.out.println("\n #####Now it is running test case " + testCase + "\n");
					
			Map outermostMap = EmaFactory.createMap();
			Map innerMap = EmaFactory.createMap();
			ElementList elementList = EmaFactory.createElementList();
			ElementList innerElementList = EmaFactory.createElementList();
			
			
			try
			{
				elementList.add(EmaFactory.createElementEntry().ascii("DefaultConsumer", "Consumer_2"));
		
				if (testCase == 1)
				{
					innerElementList.add(EmaFactory.createElementEntry().ascii("Channel", "Channel_1"));
					innerElementList.add(EmaFactory.createElementEntry().ascii("ChannelSet", "Channel_2, Channel_1"));
				}
				else if (testCase == 2)
				{
					innerElementList.add(EmaFactory.createElementEntry().ascii("ChannelSet", "Channel_2, Channel_1"));
					innerElementList.add(EmaFactory.createElementEntry().ascii("Channel", "Channel_1"));
				}
				
				innerElementList.add(EmaFactory.createElementEntry().ascii("Dictionary", "Dictionary_1"));
				innerElementList.add(EmaFactory.createElementEntry().intValue("ItemCountHint", 5000));
				
				
				innerMap.add(EmaFactory.createMapEntry().keyAscii( "Consumer_2", MapEntry.MapAction.ADD, innerElementList));
				innerElementList.clear();
									
				elementList.add(EmaFactory.createElementEntry().map( "ConsumerList", innerMap ));
				innerMap.clear();
		
				outermostMap.add(EmaFactory.createMapEntry().keyAscii( "ConsumerGroup", MapEntry.MapAction.ADD, elementList ));
				elementList.clear();
		
				innerElementList.add(EmaFactory.createElementEntry().ascii("ChannelType", "ChannelType::RSSL_SOCKET"));
				innerElementList.add(EmaFactory.createElementEntry().ascii("Host", "localhost"));
				innerElementList.add(EmaFactory.createElementEntry().ascii("Port", "14002"));
				innerElementList.add(EmaFactory.createElementEntry().intValue("GuaranteedOutputBuffers", 8000));
				innerMap.add(EmaFactory.createMapEntry().keyAscii( "Channel_1", MapEntry.MapAction.ADD, innerElementList));
				innerElementList.clear();
				
				innerElementList.add(EmaFactory.createElementEntry().ascii("ChannelType", "ChannelType::RSSL_SOCKET"));
				innerElementList.add(EmaFactory.createElementEntry().ascii("Host", "localhost1"));
				innerElementList.add(EmaFactory.createElementEntry().ascii("Port", "14003"));
				innerElementList.add(EmaFactory.createElementEntry().intValue("GuaranteedOutputBuffers", 8001));
				innerMap.add(EmaFactory.createMapEntry().keyAscii( "Channel_2", MapEntry.MapAction.ADD, innerElementList));
				innerElementList.clear();
				
				elementList.add(EmaFactory.createElementEntry().map( "ChannelList", innerMap ));
				innerMap.clear();
		
				outermostMap.add(EmaFactory.createMapEntry().keyAscii( "ChannelGroup", MapEntry.MapAction.ADD, elementList ));
				elementList.clear();
		
				// To specify EmaConfig.xml file location use -DEmaConfigFileLocation=EmaConfig.xml
				String EmaConfigFileLocation = System.getProperty("EmaConfigFileLocation");
				if ( EmaConfigFileLocation == null )
				{
					EmaConfigFileLocation = "./src/test/resources/com/rtsdk/ema/unittest/EmaFileConfigTests/EmaConfigTest.xml";
					System.out.println("EmaConfig.xml file not specified, using default file");
				}
				else
				{
					System.out.println("Using Ema Config: " + EmaConfigFileLocation);
				}
				
				OmmConsumerConfig testConfig = EmaFactory.createOmmConsumerConfig(EmaConfigFileLocation).config(outermostMap);
				OmmConsumer cons = JUnitTestConnect.createOmmConsumer(testConfig);
				
				// Check default consumer name (Conusmer_2) and associated values
				System.out.println("Retrieving DefaultConsumer configuration values: (DefaultConsumer value=Consumer_2) "); 
			
				String defaultConsName = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ConsumerDefaultConsumerName, -1);
				TestUtilities.checkResult("DefaultConsumer value != null", defaultConsName != null);
				TestUtilities.checkResult("DefaultConsumer value == Consumer_2", defaultConsName.contentEquals("Consumer_2") );
				
				String ConsDictionary = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeDictionary, JUnitTestConnect.DictionaryName, -1);
				TestUtilities.checkResult("Dictionary != null", ConsDictionary != null);
				TestUtilities.checkResult("Dictionary value == Dictionary_1", ConsDictionary.contentEquals("Dictionary_1") );
				int intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ItemCountHint, -1);
				TestUtilities.checkResult("ItemCountHint value == 5000", intLongValue == 5000 );
					
				
				// Check values of Consumer_1
				System.out.println("\nRetrieving Consumer_2 configuration values "); 
		
				// Check Channel configuration:
				// Check Channel_1 configuration.
				if (testCase == 1)
				{
					String ConsChannelVal = null;
					System.out.println("\nRetrieving Channel_2 configuration values "); 
					ConsChannelVal = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelName, 0);
					TestUtilities.checkResult("Channel value != null", ConsChannelVal != null);
					TestUtilities.checkResult("Channel value == Channel_2", ConsChannelVal.contentEquals("Channel_2") );
					int channelConnType = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelType, 0);
					TestUtilities.checkResult("channelConnType == ChannelType::RSSL_SOCKET", channelConnType == ChannelTypeSocket);
					intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.GuaranteedOutputBuffers, 0);
					TestUtilities.checkResult("GuaranteedOutputBuffers == 8001", intLongValue == 8001);
					String chanHost = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.Host, 0);
					TestUtilities.checkResult("Host == localhost1", chanHost.contentEquals("localhost1"));
					String chanPort = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.Port, 0);
					TestUtilities.checkResult("Port == 14003", chanPort.contentEquals("14003"));
					
					System.out.println("\nRetrieving Channel_1 configuration values "); 
					ConsChannelVal = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelName, 1);
					TestUtilities.checkResult("Channel value != null", ConsChannelVal != null);
					TestUtilities.checkResult("Channel value == Channel_1", ConsChannelVal.contentEquals("Channel_1") );
		  		    channelConnType = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelType, 1);
					TestUtilities.checkResult("channelConnType == ChannelType::RSSL_SOCKET", channelConnType == ChannelTypeSocket);
					intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.GuaranteedOutputBuffers, 1);
					TestUtilities.checkResult("GuaranteedOutputBuffers == 8000", intLongValue == 8000);
					chanHost = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.Host, 1);
					TestUtilities.checkResult("Host == localhost", chanHost.contentEquals("localhost"));
					chanPort = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.Port, 1);
					TestUtilities.checkResult("Port == 14002", chanPort.contentEquals("14002"));
				}
				else if (testCase == 2)
				{
					System.out.println("\nRetrieving Channel_1 configuration values "); 
					String ConsChannelVal = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelName, 0);
					TestUtilities.checkResult("Channel value != null", ConsChannelVal != null);
					TestUtilities.checkResult("Channel value == Channel_1", ConsChannelVal.contentEquals("Channel_1") );
		  		    int channelConnType = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelType, 0);
					TestUtilities.checkResult("channelConnType == ChannelType::RSSL_SOCKET", channelConnType == ChannelTypeSocket);
					intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.GuaranteedOutputBuffers, 0);
					TestUtilities.checkResult("GuaranteedOutputBuffers == 8000", intLongValue == 8000);
					String chanHost = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.Host, 0);
					TestUtilities.checkResult("Host == localhost", chanHost.contentEquals("localhost"));
					String chanPort = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.Port, 0);
					TestUtilities.checkResult("Port == 14002", chanPort.contentEquals("14002"));
				}
					
				cons = null;
			}
			catch ( OmmException excp)
			{
				System.out.println(excp.getMessage());
				TestUtilities.checkResult("Receiving exception, test failed.", false );
			}
		}
	}
	
	public void testLoadCfgFromProgrammaticConfig()
	{
		TestUtilities.printTestHead("testLoadCfgFromProgrammaticConfig","Test loading all configuration parameters programmatically");
		
		Map outermostMap = EmaFactory.createMap();
		Map innerMap = EmaFactory.createMap();
		ElementList elementList = EmaFactory.createElementList();
		ElementList innerElementList = EmaFactory.createElementList();
		
		try
		{
			innerElementList.add(EmaFactory.createElementEntry().intValue("ReactorMsgEventPoolLimit", 100));
			innerElementList.add(EmaFactory.createElementEntry().intValue("ReactorChannelEventPoolLimit", 150));
			innerElementList.add(EmaFactory.createElementEntry().intValue("WorkerEventPoolLimit", 200));
			innerElementList.add(EmaFactory.createElementEntry().intValue("TunnelStreamMsgEventPoolLimit", 250));
			innerElementList.add(EmaFactory.createElementEntry().intValue("TunnelStreamStatusEventPoolLimit", 300));
			outermostMap.add(EmaFactory.createMapEntry().keyAscii( "GlobalConfig", MapEntry.MapAction.ADD, innerElementList ));
			innerElementList.clear();
			
			elementList.add(EmaFactory.createElementEntry().ascii("DefaultConsumer", "Consumer_1"));

			innerElementList.add(EmaFactory.createElementEntry().ascii("Channel", "Channel_1"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("Dictionary", "Dictionary_1"));
			innerElementList.add(EmaFactory.createElementEntry().intValue("ItemCountHint", 5000));
			innerElementList.add(EmaFactory.createElementEntry().intValue("ServiceCountHint", 2000));
			innerElementList.add(EmaFactory.createElementEntry().intValue("ObeyOpenWindow", 1));
			innerElementList.add(EmaFactory.createElementEntry().intValue("PostAckTimeout", 1200));
			innerElementList.add(EmaFactory.createElementEntry().intValue("RequestTimeout", 2400));
			innerElementList.add(EmaFactory.createElementEntry().intValue("MaxOutstandingPosts", 9999));
			innerElementList.add(EmaFactory.createElementEntry().intValue("DispatchTimeoutApiThread", 60));
			innerElementList.add(EmaFactory.createElementEntry().intValue("CatchUnhandledException", 1));
			innerElementList.add(EmaFactory.createElementEntry().intValue("MaxDispatchCountApiThread", 300));
			innerElementList.add(EmaFactory.createElementEntry().intValue("MaxDispatchCountUserThread", 700));
			innerElementList.add(EmaFactory.createElementEntry().intValue("XmlTraceToStdout", 1));
			innerElementList.add(EmaFactory.createElementEntry().intValue("MsgKeyInUpdates", 1));
			innerElementList.add(EmaFactory.createElementEntry().intValue("ReconnectAttemptLimit", 10));
			innerElementList.add(EmaFactory.createElementEntry().intValue("ReconnectMinDelay", 4444));
			innerElementList.add(EmaFactory.createElementEntry().intValue("ReconnectMaxDelay", 7777));
			innerElementList.add(EmaFactory.createElementEntry().uintValue("RestRequestTimeOut", 65000));
			innerElementList.add(EmaFactory.createElementEntry().intValue("ReissueTokenAttemptLimit", 9));
			innerElementList.add(EmaFactory.createElementEntry().intValue("ReissueTokenAttemptInterval", 9000));
			innerElementList.add(EmaFactory.createElementEntry().doubleValue("TokenReissueRatio", 0.9));
			innerElementList.add(EmaFactory.createElementEntry().uintValue( "EnableRtt", 1 ));
			
			innerMap.add(EmaFactory.createMapEntry().keyAscii( "Consumer_1", MapEntry.MapAction.ADD, innerElementList));
			innerElementList.clear();
			
			innerElementList.add(EmaFactory.createElementEntry().ascii("Channel", "Channel_2"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("Dictionary", "Dictionary_1"));

			innerMap.add(EmaFactory.createMapEntry().keyAscii( "Consumer_2", MapEntry.MapAction.ADD, innerElementList));
			innerElementList.clear();
			
			elementList.add(EmaFactory.createElementEntry().map( "ConsumerList", innerMap ));
			innerMap.clear();

			outermostMap.add(EmaFactory.createMapEntry().keyAscii( "ConsumerGroup", MapEntry.MapAction.ADD, elementList ));
			elementList.clear();

			innerElementList.add(EmaFactory.createElementEntry().ascii("ChannelType", "ChannelType::RSSL_SOCKET"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("InterfaceName", "localhost"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("CompressionType", "CompressionType::ZLib"));
			innerElementList.add(EmaFactory.createElementEntry().intValue("GuaranteedOutputBuffers", 8000));
			innerElementList.add(EmaFactory.createElementEntry().intValue("NumInputBuffers", 7777));
			innerElementList.add(EmaFactory.createElementEntry().intValue("SysRecvBufSize", 150000));
			innerElementList.add(EmaFactory.createElementEntry().intValue("SysSendBufSize", 200000));
			innerElementList.add(EmaFactory.createElementEntry().intValue("HighWaterMark", 3000));
			innerElementList.add(EmaFactory.createElementEntry().intValue("CompressionThreshold", 12856));
			innerElementList.add(EmaFactory.createElementEntry().intValue("ConnectionPingTimeout", 30000));
			innerElementList.add(EmaFactory.createElementEntry().ascii("Host", "localhost"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("Port", "14002"));
			innerElementList.add(EmaFactory.createElementEntry().intValue("TcpNodelay", 0));
			innerElementList.add(EmaFactory.createElementEntry().intValue("DirectWrite", 1));
			innerElementList.add(EmaFactory.createElementEntry().intValue("InitializationTimeout", 66));
			
			innerMap.add(EmaFactory.createMapEntry().keyAscii( "Channel_1", MapEntry.MapAction.ADD, innerElementList));
			innerElementList.clear();
			
			innerElementList.add(EmaFactory.createElementEntry().ascii("ChannelType", "ChannelType::RSSL_ENCRYPTED"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("Host", "localhost"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("Port", "14002"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("ObjectName", "MyHttpObject"));
				
			innerMap.add(EmaFactory.createMapEntry().keyAscii( "Channel_2", MapEntry.MapAction.ADD, innerElementList));
			innerElementList.clear();
			
			elementList.add(EmaFactory.createElementEntry().map( "ChannelList", innerMap ));
			innerMap.clear();

			outermostMap.add(EmaFactory.createMapEntry().keyAscii( "ChannelGroup", MapEntry.MapAction.ADD, elementList ));
			elementList.clear();

			innerElementList.add(EmaFactory.createElementEntry().ascii("DictionaryType", "DictionaryType::ChannelDictionary"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("RdmFieldDictionaryFileName", "./RDMFieldDictionary"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("EnumTypeDefFileName", "./enumtype.def"));
			
			innerMap.add(EmaFactory.createMapEntry().keyAscii( "Dictionary_1", MapEntry.MapAction.ADD, innerElementList));
			innerElementList.clear();
			
			elementList.add(EmaFactory.createElementEntry().map( "DictionaryList", innerMap ));
			
			outermostMap.add(EmaFactory.createMapEntry().keyAscii( "DictionaryGroup", MapEntry.MapAction.ADD, elementList ));

			OmmConsumerConfig testConfig = EmaFactory.createOmmConsumerConfig().config(outermostMap);
			OmmConsumer cons = JUnitTestConnect.createOmmConsumer(testConfig);
			
			// Check default consumer name (Conusmer_2) and associated values
			System.out.println("Retrieving DefaultConsumer configuration values: (DefaultConsumer value=Consumer_2) "); 
		
			String defaultConsName = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ConsumerDefaultConsumerName, -1);
			TestUtilities.checkResult("DefaultConsumer value != null", defaultConsName != null);
			TestUtilities.checkResult("DefaultConsumer value == Consumer_1", defaultConsName.contentEquals("Consumer_1") );
			String ConsChannelVal = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelName, 0);
			TestUtilities.checkResult("Channel value != null", ConsChannelVal != null);
			TestUtilities.checkResult("Channel value == Channel_1", ConsChannelVal.contentEquals("Channel_1") );
			String ConsDictionary = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeDictionary, JUnitTestConnect.DictionaryName, -1);
			TestUtilities.checkResult("Dictionary != null", ConsDictionary != null);
			TestUtilities.checkResult("Dictionary value == Dictionary_1", ConsDictionary.contentEquals("Dictionary_1") );
			int intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ItemCountHint, -1);
			TestUtilities.checkResult("ItemCountHint value == 5000", intLongValue == 5000 );
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ServiceCountHint, -1);
			TestUtilities.checkResult("ServiceCountHint value == 2000", intLongValue == 2000 );
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ConsumerObeyOpenWindow, -1);
			TestUtilities.checkResult("ObeyOpenWindow value == 1", intLongValue == 1 );
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ConsumerPostAckTimeout, -1);
			TestUtilities.checkResult("PostAckTimeout value == 1200", intLongValue == 1200 );
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.RequestTimeout, -1);
			TestUtilities.checkResult("RequestTimeout value == 2400", intLongValue == 2400 );
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ConsumerMaxOutstandingPosts, -1);
			TestUtilities.checkResult("MaxOutstandingPosts value == 9999", intLongValue == 9999 );
			int intValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.DispatchTimeoutApiThread, -1);
			TestUtilities.checkResult("DispatchTimeoutApiThread value == 60", intValue == 60 );

			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.MaxDispatchCountApiThread, -1);
			TestUtilities.checkResult("MaxDispatchCountApiThread value == 300", intLongValue == 300 );
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.MaxDispatchCountUserThread, -1);
			TestUtilities.checkResult("MaxDispatchCountUserThread value == 700", intLongValue == 700 );

			intValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ReconnectAttemptLimit, -1);
			TestUtilities.checkResult("ReconnectAttemptLimit == 10", intValue == 10);
			intValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ReconnectMinDelay, -1);
			TestUtilities.checkResult("ReconnectMinDelay == 4444", intValue == 4444);
			intValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ReconnectMaxDelay, -1);
			TestUtilities.checkResult("ReconnectMaxDelay == 7777", intValue == 7777);
			boolean boolValue = JUnitTestConnect.activeConfigGetBooleanValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.XmlTraceToStdout, -1);
			TestUtilities.checkResult("XmlTraceToStdout == 0", boolValue == true);
			boolValue = JUnitTestConnect.activeConfigGetBooleanValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ConsumerMsgKeyInUpdates, -1);
			TestUtilities.checkResult("MsgKeyInUpdates == 1", boolValue == true);

			intValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.RestRequestTimeout, -1);
			TestUtilities.checkResult("RestRequestTimeout == 65000", intValue == 65000);
			intValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ReissueTokenAttemptLimit, -1);
			TestUtilities.checkResult("ReissueTokenAttemptLimit == 9", intValue == 9);
			intValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ReissueTokenAttemptInterval, -1);
			TestUtilities.checkResult("ReissueTokenAttemptInterval == 9000", intValue == 9000);
			double doubleValue = JUnitTestConnect.activeConfigGetDoubleValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.TokenReissueRatio, -1);
			TestUtilities.checkResult("TokenReissueRatio == 0.9", doubleValue == 0.9);
			String enableRtt = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.EnableRtt, -1);
			TestUtilities.checkResult("EnableRtt value == true", enableRtt.contentEquals("true") );
			int value = ((OmmConsumerImpl) cons).activeConfig().globalConfig.reactorMsgEventPoolLimit;
			TestUtilities.checkResult("ReactorMsgEventPoolLimit ==  100", value == 100);
			value = ((OmmConsumerImpl) cons).activeConfig().globalConfig.reactorChannelEventPoolLimit;
			TestUtilities.checkResult("ReactorChannelEventPoolLimit == 150", value == 150);
			value = ((OmmConsumerImpl) cons).activeConfig().globalConfig.workerEventPoolLimit;
			TestUtilities.checkResult("WorkerEventPoolLimit == 200", value == 200);
			value = ((OmmConsumerImpl) cons).activeConfig().globalConfig.tunnelStreamMsgEventPoolLimit;
			TestUtilities.checkResult("TunnelStreamMsgEventPoolLimit == 250", value == 250);
			value = ((OmmConsumerImpl) cons).activeConfig().globalConfig.tunnelStreamStatusEventPoolLimit;
			TestUtilities.checkResult("TunnelStreamStatusEventPoolLimit == 300", value == 300);
			
			
			// Check values of Consumer_1
			System.out.println("\nRetrieving Consumer_1 configuration values "); 

			ConsChannelVal = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelName, 0);
			TestUtilities.checkResult("Channel value != null", ConsChannelVal != null);
			TestUtilities.checkResult("Channel value == Channel_1", ConsChannelVal.contentEquals("Channel_1") );
			ConsDictionary = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeDictionary, JUnitTestConnect.DictionaryName, -1);
			TestUtilities.checkResult("Dictionary != null", ConsDictionary != null);
			TestUtilities.checkResult("Dictionary value == Dictionary_1", ConsDictionary.contentEquals("Dictionary_1") );
			
			// Check Channel configuration:
			// Check Channel_1 configuration.
			ConsChannelVal = "Channel_1";
			System.out.println("\nRetrieving Channel_1 configuration values "); 
			int channelConnType = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelType, 0);
			TestUtilities.checkResult("channelConnType == ChannelType::RSSL_SOCKET", channelConnType == ChannelTypeSocket);
		
			String strValue = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.InterfaceName, 0);
			TestUtilities.checkResult("InterfaceName == localhost", strValue.contentEquals("localhost"));
			
			intValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.CompressionType, 0);
			TestUtilities.checkResult("CompressionType == CompressionType::ZLib", intValue == CompressionTypeZLib);
			
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.GuaranteedOutputBuffers, 0);
			TestUtilities.checkResult("GuaranteedOutputBuffers == 8000", intLongValue == 8000);
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.NumInputBuffers, 0);
			TestUtilities.checkResult("NumInputBuffers == 7777", intLongValue == 7777);
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.SysRecvBufSize, 0);
			TestUtilities.checkResult("SysRecvBufSize == 150000", intLongValue == 150000);
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.SysSendBufSize, 0);
			TestUtilities.checkResult("SysSendBufSize == 200000", intLongValue == 200000);
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.HighWaterMark, 0);
			TestUtilities.checkResult("HighWaterMark == 3000", intLongValue == 3000);
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.CompressionThreshold, 0);
			TestUtilities.checkResult("CompressionThreshold == 12856", intLongValue == 12856);
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ConnectionPingTimeout, 0);
			TestUtilities.checkResult("ConnectionPingTimeout == 30000", intLongValue == 30000);
			boolValue = JUnitTestConnect.activeConfigGetBooleanValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.TcpNodelay, 0);
			TestUtilities.checkResult("TcpNodelay == 0", boolValue == false);
			boolValue = JUnitTestConnect.activeConfigGetBooleanValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.DirectWrite, 0);
			TestUtilities.checkResult("DirectWrite == 1", boolValue == true);
			String chanHost = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.Host, 0);
			TestUtilities.checkResult("Host == localhost", chanHost.contentEquals("localhost"));
			String chanPort = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.Port, 0);
			TestUtilities.checkResult("Port == 14002", chanPort.contentEquals("14002"));
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelInitTimeout, 0);
			TestUtilities.checkResult("InitializationTimeout == 66", intLongValue == 66);
			
			// Check Dictionary_1 configuration.
			ConsDictionary = "Dictionary_1";
			System.out.println("\nRetrieving Dictionary_1 configuration values ");
			intValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeDictionary, JUnitTestConnect.DictionaryType, -1);
			TestUtilities.checkResult("DictionaryType == DictionaryType::ChannelDictionary (0)", intValue == 0);		
			strValue =  JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeDictionary, JUnitTestConnect.DictionaryRDMFieldDictFileName, -1);
			TestUtilities.checkResult("RdmFieldDictionaryFileName == ./RDMFieldDictionary", strValue.contentEquals("./RDMFieldDictionary"));		
			strValue =   JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeDictionary, JUnitTestConnect.DictionaryEnumTypeDefFileName, -1);
			TestUtilities.checkResult("RdmFieldDictionaryFileName == ./enumtype.def", strValue.contentEquals("./enumtype.def"));		

			cons = null;
		}
		catch ( OmmException excp)
		{
			System.out.println(excp.getMessage());
			TestUtilities.checkResult("Receiving exception, test failed.", false );
		}
	}
	
	public void testLoadCfgFromProgrammaticConfigHttp()
	{
		TestUtilities.printTestHead("testLoadCfgFromProgrammaticConfigHttp","Test loading all http configuration parameters programmatically");
		
		Map outermostMap = EmaFactory.createMap();
		Map innerMap = EmaFactory.createMap();
		ElementList elementList = EmaFactory.createElementList();
		ElementList innerElementList = EmaFactory.createElementList();
		
		try
		{
			elementList.add(EmaFactory.createElementEntry().ascii("DefaultConsumer", "Consumer_1"));

			innerElementList.add(EmaFactory.createElementEntry().ascii("Channel", "Channel_1"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("Dictionary", "Dictionary_1"));
			innerElementList.add(EmaFactory.createElementEntry().intValue("ItemCountHint", 5000));
			innerElementList.add(EmaFactory.createElementEntry().intValue("ServiceCountHint", 2000));
			innerElementList.add(EmaFactory.createElementEntry().intValue("ObeyOpenWindow", 1));
			innerElementList.add(EmaFactory.createElementEntry().intValue("PostAckTimeout", 1200));
			innerElementList.add(EmaFactory.createElementEntry().intValue("RequestTimeout", 2400));
			innerElementList.add(EmaFactory.createElementEntry().intValue("MaxOutstandingPosts", 9999));
			innerElementList.add(EmaFactory.createElementEntry().intValue("DispatchTimeoutApiThread", 60));
			innerElementList.add(EmaFactory.createElementEntry().intValue("CatchUnhandledException", 1));
			innerElementList.add(EmaFactory.createElementEntry().intValue("MaxDispatchCountApiThread", 300));
			innerElementList.add(EmaFactory.createElementEntry().intValue("MaxDispatchCountUserThread", 700));
			innerElementList.add(EmaFactory.createElementEntry().intValue("ReconnectAttemptLimit", 1));
			innerElementList.add(EmaFactory.createElementEntry().intValue("ReconnectMinDelay", 500));
			innerElementList.add(EmaFactory.createElementEntry().intValue("ReconnectMaxDelay", 500));
			innerElementList.add(EmaFactory.createElementEntry().intValue("XmlTraceToStdout", 1));
			innerElementList.add(EmaFactory.createElementEntry().intValue("MsgKeyInUpdates", 1));
			
			innerMap.add(EmaFactory.createMapEntry().keyAscii( "Consumer_1", MapEntry.MapAction.ADD, innerElementList));
			innerElementList.clear();
			
			elementList.add(EmaFactory.createElementEntry().map( "ConsumerList", innerMap ));
			innerMap.clear();

			outermostMap.add(EmaFactory.createMapEntry().keyAscii( "ConsumerGroup", MapEntry.MapAction.ADD, elementList ));
			elementList.clear();

			innerElementList.add(EmaFactory.createElementEntry().ascii("ChannelType", "ChannelType::RSSL_ENCRYPTED"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("EncryptedProtocolType", "EncryptedProtocolType::RSSL_HTTP"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("InterfaceName", "localhost"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("CompressionType", "CompressionType::ZLib"));
			innerElementList.add(EmaFactory.createElementEntry().intValue("GuaranteedOutputBuffers", 8000));
			innerElementList.add(EmaFactory.createElementEntry().intValue("NumInputBuffers", 7777));
			innerElementList.add(EmaFactory.createElementEntry().intValue("SysRecvBufSize", 150000));
			innerElementList.add(EmaFactory.createElementEntry().intValue("SysSendBufSize", 200000));
			innerElementList.add(EmaFactory.createElementEntry().intValue("ConnectionPingTimeout", 30000));
			innerElementList.add(EmaFactory.createElementEntry().intValue("HighWaterMark", 3000));
			innerElementList.add(EmaFactory.createElementEntry().ascii("Host", "localhost"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("Port", "14002"));
			innerElementList.add(EmaFactory.createElementEntry().intValue("TcpNodelay", 0));
			innerElementList.add(EmaFactory.createElementEntry().ascii("ObjectName", "MyHttpObject"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("Location", "eu-west"));
			innerElementList.add(EmaFactory.createElementEntry().intValue("EnableSessionManagement", 1));
			innerMap.add(EmaFactory.createMapEntry().keyAscii( "Channel_1", MapEntry.MapAction.ADD, innerElementList));
			innerElementList.clear();
			
			elementList.add(EmaFactory.createElementEntry().map( "ChannelList", innerMap ));
			innerMap.clear();

			outermostMap.add(EmaFactory.createMapEntry().keyAscii( "ChannelGroup", MapEntry.MapAction.ADD, elementList ));
			elementList.clear();


			innerElementList.add(EmaFactory.createElementEntry().ascii("DictionaryType", "DictionaryType::ChannelDictionary"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("RdmFieldDictionaryFileName", "./RDMFieldDictionary"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("EnumTypeDefFileName", "./enumtype.def"));
			
			innerMap.add(EmaFactory.createMapEntry().keyAscii( "Dictionary_1", MapEntry.MapAction.ADD, innerElementList));
			innerElementList.clear();
			
			elementList.add(EmaFactory.createElementEntry().map( "DictionaryList", innerMap ));

			outermostMap.add(EmaFactory.createMapEntry().keyAscii( "DictionaryGroup", MapEntry.MapAction.ADD, elementList ));

			OmmConsumerConfig testConfig = EmaFactory.createOmmConsumerConfig().config(outermostMap);
			OmmConsumer cons = JUnitTestConnect.createOmmConsumer(testConfig);

			// Check default consumer name (Conusmer_2) and associated values
			System.out.println("Retrieving DefaultConsumer configuration values: (DefaultConsumer value=Consumer_2) "); 
		
			String defaultConsName = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ConsumerDefaultConsumerName, -1);
			TestUtilities.checkResult("DefaultConsumer value != null", defaultConsName != null);
			TestUtilities.checkResult("DefaultConsumer value == Consumer_1", defaultConsName.contentEquals("Consumer_1") );
			String ConsChannelVal = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelName, 0);
			TestUtilities.checkResult("Channel value != null", ConsChannelVal != null);
			TestUtilities.checkResult("Channel value == Channel_1", ConsChannelVal.contentEquals("Channel_1") );
			String ConsDictionary = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeDictionary, JUnitTestConnect.DictionaryName, -1);
			TestUtilities.checkResult("Dictionary != null", ConsDictionary != null);
			TestUtilities.checkResult("Dictionary value == Dictionary_1", ConsDictionary.contentEquals("Dictionary_1") );
			int intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ItemCountHint, -1);
			TestUtilities.checkResult("ItemCountHint value == 5000", intLongValue == 5000 );
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ServiceCountHint, -1);
			TestUtilities.checkResult("ServiceCountHint value == 2000", intLongValue == 2000 );
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ConsumerObeyOpenWindow, -1);
			TestUtilities.checkResult("ObeyOpenWindow value == 1", intLongValue == 1 );
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ConsumerPostAckTimeout, -1);
			TestUtilities.checkResult("PostAckTimeout value == 1200", intLongValue == 1200 );
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.RequestTimeout, -1);
			TestUtilities.checkResult("RequestTimeout value == 2400", intLongValue == 2400 );
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ConsumerMaxOutstandingPosts, -1);
			TestUtilities.checkResult("MaxOutstandingPosts value == 9999", intLongValue == 9999 );
			int intValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.DispatchTimeoutApiThread, -1);
			TestUtilities.checkResult("DispatchTimeoutApiThread value == 60", intValue == 60 );

			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.MaxDispatchCountApiThread, -1);
			TestUtilities.checkResult("MaxDispatchCountApiThread value == 300", intLongValue == 300 );
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.MaxDispatchCountUserThread, -1);
			TestUtilities.checkResult("MaxDispatchCountUserThread value == 700", intLongValue == 700 );

			intValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ReconnectAttemptLimit, -1);
			TestUtilities.checkResult("ReconnectAttemptLimit == 1", intValue == 1);
			intValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ReconnectMinDelay, -1);
			TestUtilities.checkResult("ReconnectMinDelay == 500", intValue == 500);
			intValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ReconnectMaxDelay, -1);
			TestUtilities.checkResult("ReconnectMaxDelay == 500", intValue == 500);
			boolean boolValue = JUnitTestConnect.activeConfigGetBooleanValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.XmlTraceToStdout, -1);
			TestUtilities.checkResult("XmlTraceToStdout == 1", boolValue == true);
			boolValue = JUnitTestConnect.activeConfigGetBooleanValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ConsumerMsgKeyInUpdates, -1);
			TestUtilities.checkResult("MsgKeyInUpdates == 1", boolValue == true);
			
			
			// Check values of Consumer_1
			System.out.println("\nRetrieving Consumer_1 configuration values "); 

			ConsChannelVal = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelName, 0);
			TestUtilities.checkResult("Channel value != null", ConsChannelVal != null);
			TestUtilities.checkResult("Channel value == Channel_1", ConsChannelVal.contentEquals("Channel_1") );
			ConsDictionary = JUnitTestConnect.configGetDictionaryName(testConfig, "Consumer_1");
			TestUtilities.checkResult("Dictionary != null", ConsDictionary != null);
			TestUtilities.checkResult("Dictionary value == Dictionary_1", ConsDictionary.contentEquals("Dictionary_1") );
			
			// Check Channel configuration:
			// Check Channel_1 configuration.
			ConsChannelVal = "Channel_1";
			System.out.println("\nRetrieving Channel_1 configuration values "); 
			int channelConnType = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelType, 0);
			TestUtilities.checkResult("channelConnType == ChannelType::RSSL_ENCRYPTED", channelConnType == ChannelTypeEncrypted);
			int encryptedChannelType = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.EncryptedProtocolType, 0);
			TestUtilities.checkResult("channelConnType == ChannelType::RSSL_ENCRYPTED", channelConnType == ChannelTypeEncrypted);
		
			String strValue = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.InterfaceName, 0);
			TestUtilities.checkResult("InterfaceName == localhost", strValue.contentEquals("localhost"));
			
			intValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.CompressionType, 0);
			TestUtilities.checkResult("CompressionType == CompressionType::ZLib", intValue == CompressionTypeZLib);
			
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.GuaranteedOutputBuffers, 0);
			TestUtilities.checkResult("GuaranteedOutputBuffers == 8000", intLongValue == 8000);
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.NumInputBuffers, 0);
			TestUtilities.checkResult("NumInputBuffers == 7777", intLongValue == 7777);
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.SysRecvBufSize, 0);
			TestUtilities.checkResult("SysRecvBufSize == 150000", intLongValue == 150000);
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.SysSendBufSize, 0);
			TestUtilities.checkResult("SysSendBufSize == 200000", intLongValue == 200000);
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.HighWaterMark, 0);
			TestUtilities.checkResult("HighWaterMark == 3000", intLongValue == 3000);
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ConnectionPingTimeout, 0);
			TestUtilities.checkResult("ConnectionPingTimeout == 30000", intLongValue == 30000);
			boolValue = JUnitTestConnect.activeConfigGetBooleanValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.TcpNodelay, 0);
			TestUtilities.checkResult("TcpNodelay == 0", boolValue == false);
			String chanHost = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.Host, 0);
			TestUtilities.checkResult("Host == localhost", chanHost.contentEquals("localhost"));
			String chanPort = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.Port, 0);
			TestUtilities.checkResult("Port == 14002", chanPort.contentEquals("14002"));
			String objectName = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ObjectName, 0);
			TestUtilities.checkResult("ObjectName == MyHttpObject", objectName.contentEquals("MyHttpObject"));
			String location = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.Location, 0);
			TestUtilities.checkResult("Location == eu-west", location.contentEquals("eu-west"));
			boolValue = JUnitTestConnect.activeConfigGetBooleanValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.EnableSessionMgnt, 0);
			TestUtilities.checkResult("EnableSessionManagement == 0", boolValue == true);
			
			// Check Dictionary_1 configuration.
			ConsDictionary = "Dictionary_1";
			System.out.println("\nRetrieving Dictionary_1 configuration values ");
			intValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeDictionary, JUnitTestConnect.DictionaryType, -1);
			TestUtilities.checkResult("DictionaryType == DictionaryType::ChannelDictionary (0)", intValue == 0);		
			strValue =  JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeDictionary, JUnitTestConnect.DictionaryRDMFieldDictFileName, -1);
			TestUtilities.checkResult("RdmFieldDictionaryFileName == ./RDMFieldDictionary", strValue.contentEquals("./RDMFieldDictionary"));		
			strValue =   JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeDictionary, JUnitTestConnect.DictionaryEnumTypeDefFileName, -1);
			TestUtilities.checkResult("RdmFieldDictionaryFileName == ./enumtype.def", strValue.contentEquals("./enumtype.def"));		

			cons = null;
		}
		catch ( OmmException excp)
		{
			System.out.println(excp.getMessage());
			TestUtilities.checkResult("Receiving exception, test failed.", false );
		}
	}
	
	public void testMergCfgBetweenFileAndProgrammaticConfig()
	{
		TestUtilities.printTestHead("testMergCfgBetweenFileAndProgrammaticConfig","Test merge all configuration parameters between config file and programmatically config");
		
		Map configDB1 = EmaFactory.createMap();
		Map configDB2 = EmaFactory.createMap();
		Map configDB3 = EmaFactory.createMap();
		Map innerMap = EmaFactory.createMap();
		ElementList elementList = EmaFactory.createElementList();
		ElementList innerElementList = EmaFactory.createElementList();
		
		try
		{
			innerElementList.add(EmaFactory.createElementEntry().ascii("Channel", "Channel_2"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("Dictionary", "Dictionary_2"));
			innerElementList.add(EmaFactory.createElementEntry().intValue("ItemCountHint", 9000));
			innerElementList.add(EmaFactory.createElementEntry().intValue("ServiceCountHint", 9000));
			innerElementList.add(EmaFactory.createElementEntry().intValue("ObeyOpenWindow", 1));
			innerElementList.add(EmaFactory.createElementEntry().intValue("PostAckTimeout", 9000));
			innerElementList.add(EmaFactory.createElementEntry().intValue("RequestTimeout", 9000));
			innerElementList.add(EmaFactory.createElementEntry().intValue("MaxOutstandingPosts", 9000));
			innerElementList.add(EmaFactory.createElementEntry().intValue("DispatchTimeoutApiThread", 5656));
			innerElementList.add(EmaFactory.createElementEntry().intValue("MaxDispatchCountApiThread", 900));
			innerElementList.add(EmaFactory.createElementEntry().intValue("MaxDispatchCountUserThread", 900));
			innerElementList.add(EmaFactory.createElementEntry().intValue("XmlTraceToStdout", 0));
			innerElementList.add(EmaFactory.createElementEntry().intValue("MsgKeyInUpdates", 0));
			innerElementList.add(EmaFactory.createElementEntry().intValue("ReconnectAttemptLimit", 70));
			innerElementList.add(EmaFactory.createElementEntry().intValue("ReconnectMinDelay", 7000));
			innerElementList.add(EmaFactory.createElementEntry().intValue("ReconnectMaxDelay", 7000));
			
			innerMap.add(EmaFactory.createMapEntry().keyAscii( "Consumer_2", MapEntry.MapAction.ADD, innerElementList));
			innerElementList.clear();
			
			elementList.add(EmaFactory.createElementEntry().map( "ConsumerList", innerMap ));
			innerMap.clear();

			configDB1.add(EmaFactory.createMapEntry().keyAscii( "ConsumerGroup", MapEntry.MapAction.ADD, elementList ));
			elementList.clear();

			innerElementList.add(EmaFactory.createElementEntry().ascii("ChannelType", "ChannelType::RSSL_SOCKET"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("InterfaceName", "localhost"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("CompressionType", "CompressionType::LZ4"));
			innerElementList.add(EmaFactory.createElementEntry().intValue("GuaranteedOutputBuffers", 7000));
			innerElementList.add(EmaFactory.createElementEntry().intValue("NumInputBuffers", 5000));
			innerElementList.add(EmaFactory.createElementEntry().intValue("SysRecvBufSize", 550000));
			innerElementList.add(EmaFactory.createElementEntry().intValue("SysSendBufSize", 700000));
			innerElementList.add(EmaFactory.createElementEntry().intValue("CompressionThreshold", 12758));
			innerElementList.add(EmaFactory.createElementEntry().intValue("ConnectionPingTimeout", 70000));
			innerElementList.add(EmaFactory.createElementEntry().ascii("Host", "localhost1"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("Port", "14012"));
			innerElementList.add(EmaFactory.createElementEntry().intValue("TcpNodelay", 1));
			innerElementList.add(EmaFactory.createElementEntry().intValue("HighWaterMark", 5000));
			innerElementList.add(EmaFactory.createElementEntry().intValue("DirectWrite", 1));
			innerElementList.add(EmaFactory.createElementEntry().intValue("InitializationTimeout", 77));
			
			innerMap.add(EmaFactory.createMapEntry().keyAscii( "Channel_2", MapEntry.MapAction.ADD, innerElementList));
			innerElementList.clear();

			elementList.add(EmaFactory.createElementEntry().map( "ChannelList", innerMap ));
			innerMap.clear();

			configDB2.add(EmaFactory.createMapEntry().keyAscii( "ChannelGroup", MapEntry.MapAction.ADD, elementList ));
			elementList.clear();

			innerElementList.add(EmaFactory.createElementEntry().ascii("DictionaryType", "DictionaryType::ChannelDictionary"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("RdmFieldDictionaryFileName", "./ConfigDB3_RDMFieldDictionary"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("EnumTypeDefFileName", "./ConfigDB3_enumtype.def"));
			
			innerMap.add(EmaFactory.createMapEntry().keyAscii( "Dictionary_2", MapEntry.MapAction.ADD, innerElementList));
			innerElementList.clear();
			
			elementList.add(EmaFactory.createElementEntry().map( "DictionaryList", innerMap ));

			configDB3.add(EmaFactory.createMapEntry().keyAscii( "DictionaryGroup", MapEntry.MapAction.ADD, elementList ));

			// To specify EmaConfig.xml file location use -DEmaConfigFileLocation=EmaConfig.xml
			String EmaConfigFileLocation = System.getProperty("EmaConfigFileLocation");
			if ( EmaConfigFileLocation == null )
			{
				EmaConfigFileLocation = "./src/test/resources/com/rtsdk/ema/unittest/EmaFileConfigTests/EmaConfigTest.xml";
				System.out.println("EmaConfig.xml file not specified, using default file");
			}
			else
			{
				System.out.println("Using Ema Config: " + EmaConfigFileLocation);
			}
			
			OmmConsumerConfig testConfig = EmaFactory.createOmmConsumerConfig(EmaConfigFileLocation).config(configDB1).config(configDB2).config(configDB3);
			OmmConsumer cons = JUnitTestConnect.createOmmConsumer(testConfig);

			// Check default consumer name (Conusmer_2) and associated values
			System.out.println("Retrieving DefaultConsumer configuration values: (DefaultConsumer value=Consumer_2) "); 
		
			String defaultConsName = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ConsumerDefaultConsumerName, -1);
			TestUtilities.checkResult("DefaultConsumer value != null", defaultConsName != null);
			TestUtilities.checkResult("DefaultConsumer value == Consumer_2", defaultConsName.contentEquals("Consumer_2") );
			String ConsChannelVal = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelName, 0);
			TestUtilities.checkResult("Channel value != null", ConsChannelVal != null);
			TestUtilities.checkResult("Channel value == Channel_2", ConsChannelVal.contentEquals("Channel_2") );
			String ConsDictionary = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeDictionary, JUnitTestConnect.DictionaryName, -1);
			TestUtilities.checkResult("Dictionary != null", ConsDictionary != null);
			TestUtilities.checkResult("Dictionary value == Dictionary_2", ConsDictionary.contentEquals("Dictionary_2") );
			int intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ItemCountHint, -1);
			TestUtilities.checkResult("ItemCountHint value == 9000", intLongValue == 9000 );
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ServiceCountHint, -1);
			TestUtilities.checkResult("ServiceCountHint value == 9000", intLongValue == 9000 );
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ConsumerObeyOpenWindow, -1);
			TestUtilities.checkResult("ObeyOpenWindow value == 1", intLongValue == 1 );
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ConsumerPostAckTimeout, -1);
			TestUtilities.checkResult("PostAckTimeout value == 9000", intLongValue == 9000 );
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.RequestTimeout, -1);
			TestUtilities.checkResult("RequestTimeout value == 9000", intLongValue == 9000 );
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ConsumerMaxOutstandingPosts, -1);
			TestUtilities.checkResult("MaxOutstandingPosts value == 9000", intLongValue == 9000 );
			int intValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.DispatchTimeoutApiThread, -1);
			TestUtilities.checkResult("DispatchTimeoutApiThread value == 5656", intValue == 5656 );

			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.MaxDispatchCountApiThread, -1);
			TestUtilities.checkResult("MaxDispatchCountApiThread value == 900", intLongValue == 900 );
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.MaxDispatchCountUserThread, -1);
			TestUtilities.checkResult("MaxDispatchCountUserThread value == 900", intLongValue == 900 );

			intValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ReconnectAttemptLimit, -1);
			TestUtilities.checkResult("ReconnectAttemptLimit == 70", intValue == 70);
			intValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ReconnectMinDelay, -1);
			TestUtilities.checkResult("ReconnectMinDelay == 7000", intValue == 7000);
			intValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ReconnectMaxDelay, -1);
			TestUtilities.checkResult("ReconnectMaxDelay == 7000", intValue == 7000);
			boolean boolValue = JUnitTestConnect.activeConfigGetBooleanValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.XmlTraceToStdout, -1);
			TestUtilities.checkResult("XmlTraceToStdout == 0", boolValue == false);
			boolValue = JUnitTestConnect.activeConfigGetBooleanValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ConsumerMsgKeyInUpdates, -1);
			TestUtilities.checkResult("MsgKeyInUpdates == 0", boolValue == false);
			
			
			// Check values of Consumer_2
			System.out.println("\nRetrieving Consumer_1 configuration values "); 

			ConsChannelVal = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelName, 0);
			TestUtilities.checkResult("Channel value != null", ConsChannelVal != null);
			TestUtilities.checkResult("Channel value == Channel_2", ConsChannelVal.contentEquals("Channel_2") );
			ConsDictionary = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeDictionary, JUnitTestConnect.DictionaryName, -1);
			TestUtilities.checkResult("Dictionary != null", ConsDictionary != null);
			TestUtilities.checkResult("Dictionary value == Dictionary_2", ConsDictionary.contentEquals("Dictionary_2") );
			
			// Check Channel configuration:
			// Check Channel_2 configuration.
			ConsChannelVal = "Channel_2";
			System.out.println("\nRetrieving Channel_2 configuration values "); 
			int channelConnType = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelType, 0);
			TestUtilities.checkResult("channelConnType == ChannelType::RSSL_SOCKET", channelConnType == ChannelTypeSocket);
		
			String strValue = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.InterfaceName, 0);
			TestUtilities.checkResult("InterfaceName == localhost", strValue.contentEquals("localhost"));
			
			intValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.CompressionType, 0);
			TestUtilities.checkResult("CompressionType == CompressionType::LZ4", intValue == CompressionTypeLZ4);
			
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.GuaranteedOutputBuffers, 0);
			TestUtilities.checkResult("GuaranteedOutputBuffers == 7000", intLongValue == 7000);
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.NumInputBuffers, 0);
			TestUtilities.checkResult("NumInputBuffers == 5000", intLongValue == 5000);
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.SysRecvBufSize, 0);
			TestUtilities.checkResult("SysRecvBufSize == 550000", intLongValue == 550000);
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.SysSendBufSize, 0);
			TestUtilities.checkResult("SysSendBufSize == 700000", intLongValue == 700000);
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.CompressionThreshold, 0);
			TestUtilities.checkResult("CompressionThreshold == 12758", intLongValue == 12758);
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ConnectionPingTimeout, 0);
			TestUtilities.checkResult("ConnectionPingTimeout == 70000", intLongValue == 70000);
			boolValue = JUnitTestConnect.activeConfigGetBooleanValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.TcpNodelay, 0);
			TestUtilities.checkResult("TcpNodelay == 1", boolValue == true);
			boolValue = JUnitTestConnect.activeConfigGetBooleanValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.DirectWrite, 0);
			TestUtilities.checkResult("DirectWrite == 1", boolValue == true);
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.HighWaterMark, 0);
			TestUtilities.checkResult("HighWaterMark == 5000", intLongValue == 5000);
			String chanHost = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.Host, 0);
			TestUtilities.checkResult("Host == localhost1", chanHost.contentEquals("localhost1"));
			String chanPort = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.Port, 0);
			TestUtilities.checkResult("Port == 14012", chanPort.contentEquals("14012"));
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelInitTimeout, 0);
			TestUtilities.checkResult("InitializationTimeout == 77", intLongValue == 77);
			
			// Check Dictionary_2 configuration.
			ConsDictionary = "Dictionary_2";
			System.out.println("\nRetrieving Dictionary_2 configuration values ");
			intValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeDictionary, JUnitTestConnect.DictionaryType, -1);
			TestUtilities.checkResult("DictionaryType == DictionaryType::ChannelDictionary (0)", intValue == 0);		
			strValue =  JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeDictionary, JUnitTestConnect.DictionaryRDMFieldDictFileName, -1);
			TestUtilities.checkResult("RdmFieldDictionaryFileName == ./ConfigDB3_RDMFieldDictionary", strValue.contentEquals("./ConfigDB3_RDMFieldDictionary"));		
			strValue =   JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeDictionary, JUnitTestConnect.DictionaryEnumTypeDefFileName, -1);
			TestUtilities.checkResult("RdmFieldDictionaryFileName == ./ConfigDB3_enumtype.def", strValue.contentEquals("./ConfigDB3_enumtype.def"));		

			cons = null;
		}
		catch ( OmmException excp)
		{
			System.out.println(excp.getMessage());
			TestUtilities.checkResult("Receiving exception, test failed.", false );
		}
	}
	
	public void testMergCfgBetweenFileAndProgrammaticConfigExtra()
	{
		TestUtilities.printTestHead("testMergCfgBetweenFileAndProgrammaticConfig1",
				"Test overwrite Common paramters/Channel/Dict config parameters between programmtically, and config file");
		
		//testcase 1: consumer level para from programmatically, others from config file
		//testcase 2: consumer level para from config file, others from programmatically

		for (int testCase = 1; testCase < 3; testCase++)
		{
			System.out.println("\n##### Now it is running test case " + testCase + "\n");
			
			Map configDB1 = EmaFactory.createMap();
			Map configDB2 = EmaFactory.createMap();
			Map configDB3 = EmaFactory.createMap();
			Map innerMap = EmaFactory.createMap();
			ElementList elementList = EmaFactory.createElementList();
			ElementList innerElementList = EmaFactory.createElementList();
			
			try
			{
				if (testCase == 1)
				{
					innerElementList.add(EmaFactory.createElementEntry().ascii("Channel", "Channel_2"));
					innerElementList.add(EmaFactory.createElementEntry().ascii("Dictionary", "Dictionary_2"));
					
					innerElementList.add(EmaFactory.createElementEntry().intValue("ItemCountHint", 9000));
					innerElementList.add(EmaFactory.createElementEntry().intValue("ServiceCountHint", 9000));
					innerElementList.add(EmaFactory.createElementEntry().intValue("ObeyOpenWindow", 1));
					innerElementList.add(EmaFactory.createElementEntry().intValue("PostAckTimeout", 9000));
					innerElementList.add(EmaFactory.createElementEntry().intValue("RequestTimeout", 9000));
					innerElementList.add(EmaFactory.createElementEntry().intValue("MaxOutstandingPosts", 9000));
					innerElementList.add(EmaFactory.createElementEntry().intValue("DispatchTimeoutApiThread", 5656));
					innerElementList.add(EmaFactory.createElementEntry().intValue("MaxDispatchCountApiThread", 900));
					innerElementList.add(EmaFactory.createElementEntry().intValue("MaxDispatchCountUserThread", 900));
					innerElementList.add(EmaFactory.createElementEntry().intValue("XmlTraceToStdout", 0));
					innerElementList.add(EmaFactory.createElementEntry().intValue("MsgKeyInUpdates", 0));
					innerElementList.add(EmaFactory.createElementEntry().intValue("ReconnectAttemptLimit", 70));
					innerElementList.add(EmaFactory.createElementEntry().intValue("ReconnectMinDelay", 7000));
					innerElementList.add(EmaFactory.createElementEntry().intValue("ReconnectMaxDelay", 7000));
					
					innerMap.add(EmaFactory.createMapEntry().keyAscii( "Consumer_2", MapEntry.MapAction.ADD, innerElementList));
					innerElementList.clear();
				}
				else if (testCase == 2)
				{
					innerElementList.add(EmaFactory.createElementEntry().ascii("Channel", "Channel_2"));
										
					innerMap.add(EmaFactory.createMapEntry().keyAscii( "Consumer_2", MapEntry.MapAction.ADD, innerElementList));
					innerElementList.clear();
				}
				
				elementList.add(EmaFactory.createElementEntry().map( "ConsumerList", innerMap ));
				innerMap.clear();
	
				configDB1.add(EmaFactory.createMapEntry().keyAscii( "ConsumerGroup", MapEntry.MapAction.ADD, elementList ));
				elementList.clear();
	
				innerElementList.add(EmaFactory.createElementEntry().ascii("ChannelType", "ChannelType::RSSL_SOCKET"));
				innerElementList.add(EmaFactory.createElementEntry().ascii("InterfaceName", "localhost"));
				innerElementList.add(EmaFactory.createElementEntry().ascii("CompressionType", "CompressionType::LZ4"));
				innerElementList.add(EmaFactory.createElementEntry().intValue("GuaranteedOutputBuffers", 7000));
				innerElementList.add(EmaFactory.createElementEntry().intValue("NumInputBuffers", 5000));
				innerElementList.add(EmaFactory.createElementEntry().intValue("SysRecvBufSize", 550000));
				innerElementList.add(EmaFactory.createElementEntry().intValue("SysSendBufSize", 700000));
				innerElementList.add(EmaFactory.createElementEntry().intValue("CompressionThreshold", 12758));
				innerElementList.add(EmaFactory.createElementEntry().intValue("ConnectionPingTimeout", 70000));
				innerElementList.add(EmaFactory.createElementEntry().ascii("Host", "localhost1"));
				innerElementList.add(EmaFactory.createElementEntry().ascii("Port", "14012"));
				innerElementList.add(EmaFactory.createElementEntry().intValue("TcpNodelay", 1));
				innerElementList.add(EmaFactory.createElementEntry().intValue("HighWaterMark", 5000));
				innerElementList.add(EmaFactory.createElementEntry().intValue("DirectWrite", 1));
				if (testCase == 1)
				{
					//here config Channel_3, so application will use channel parameters from config file
					innerMap.add(EmaFactory.createMapEntry().keyAscii( "Channel_3", MapEntry.MapAction.ADD, innerElementList));
					innerElementList.clear();
				}
				else if (testCase == 2)
				{
					//here config Channel_2, so application will use channel parameters here programmatically configured
					innerMap.add(EmaFactory.createMapEntry().keyAscii( "Channel_2", MapEntry.MapAction.ADD, innerElementList));
					innerElementList.clear();
	
				}
				
				elementList.add(EmaFactory.createElementEntry().map( "ChannelList", innerMap ));
				innerMap.clear();
	
				configDB2.add(EmaFactory.createMapEntry().keyAscii( "ChannelGroup", MapEntry.MapAction.ADD, elementList ));
				elementList.clear();
	
				innerElementList.add(EmaFactory.createElementEntry().ascii("DictionaryType", "DictionaryType::ChannelDictionary"));
				innerElementList.add(EmaFactory.createElementEntry().ascii("RdmFieldDictionaryFileName", "./ConfigDB3_RDMFieldDictionary"));
				innerElementList.add(EmaFactory.createElementEntry().ascii("EnumTypeDefFileName", "./ConfigDB3_enumtype.def"));
					
				if (testCase == 1)
				{	
					//here config Dictionary_3, so application will use dict parameters from config file
					innerMap.add(EmaFactory.createMapEntry().keyAscii( "Dictionary_3", MapEntry.MapAction.ADD, innerElementList));
					innerElementList.clear();
				}
				else if (testCase == 2)
				{
					//here config Dictionary_2, so application will use dict parameters here programmatically configured
					innerMap.add(EmaFactory.createMapEntry().keyAscii( "Dictionary_2", MapEntry.MapAction.ADD, innerElementList));
					innerElementList.clear();
				}
				
				elementList.add(EmaFactory.createElementEntry().map( "DictionaryList", innerMap ));
	
				configDB3.add(EmaFactory.createMapEntry().keyAscii( "DictionaryGroup", MapEntry.MapAction.ADD, elementList ));
	
				// To specify EmaConfig.xml file location use -DEmaConfigFileLocation=EmaConfig.xml
				String EmaConfigFileLocation = System.getProperty("EmaConfigFileLocation");
				if ( EmaConfigFileLocation == null )
				{
					EmaConfigFileLocation = "./src/test/resources/com/rtsdk/ema/unittest/EmaFileConfigTests/EmaConfigTest.xml";
					System.out.println("EmaConfig.xml file not specified, using default file");
				}
				else
				{
					System.out.println("Using Ema Config: " + EmaConfigFileLocation);
				}
				
				OmmConsumerConfig testConfig = EmaFactory.createOmmConsumerConfig(EmaConfigFileLocation).config(configDB1).config(configDB2).config(configDB3);
				OmmConsumer cons = JUnitTestConnect.createOmmConsumer(testConfig);
	
				// Check default consumer name (Conusmer_2) and associated values
				System.out.println("Retrieving DefaultConsumer configuration values: (DefaultConsumer value=Consumer_2) "); 
			
				String defaultConsName = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ConsumerDefaultConsumerName, -1);
				TestUtilities.checkResult("DefaultConsumer value != null", defaultConsName != null);
				TestUtilities.checkResult("DefaultConsumer value == Consumer_2", defaultConsName.contentEquals("Consumer_2") );
				String ConsChannelVal = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelName, 0);
				TestUtilities.checkResult("Channel value != null", ConsChannelVal != null);
				TestUtilities.checkResult("Channel value == Channel_2", ConsChannelVal.contentEquals("Channel_2") );
				String ConsDictionary = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeDictionary, JUnitTestConnect.DictionaryName, -1);
				TestUtilities.checkResult("Dictionary != null", ConsDictionary != null);
				TestUtilities.checkResult("Dictionary value == Dictionary_2", ConsDictionary.contentEquals("Dictionary_2") );
				
				if (testCase == 1)
				{
					int intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ItemCountHint, -1);
					TestUtilities.checkResult("ItemCountHint value == 9000", intLongValue == 9000 );
					intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ServiceCountHint, -1);
					TestUtilities.checkResult("ServiceCountHint value == 9000", intLongValue == 9000 );
					intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ConsumerObeyOpenWindow, -1);
					TestUtilities.checkResult("ObeyOpenWindow value == 1", intLongValue == 1 );
					intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ConsumerPostAckTimeout, -1);
					TestUtilities.checkResult("PostAckTimeout value == 9000", intLongValue == 9000 );
					intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.RequestTimeout, -1);
					TestUtilities.checkResult("RequestTimeout value == 9000", intLongValue == 9000 );
					intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ConsumerMaxOutstandingPosts, -1);
					TestUtilities.checkResult("MaxOutstandingPosts value == 9000", intLongValue == 9000 );
					int intValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.DispatchTimeoutApiThread, -1);
					TestUtilities.checkResult("DispatchTimeoutApiThread value == 5656", intValue == 5656 );
		
					intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.MaxDispatchCountApiThread, -1);
					TestUtilities.checkResult("MaxDispatchCountApiThread value == 900", intLongValue == 900 );
					intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.MaxDispatchCountUserThread, -1);
					TestUtilities.checkResult("MaxDispatchCountUserThread value == 900", intLongValue == 900 );
		
					intValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ReconnectAttemptLimit, -1);
					TestUtilities.checkResult("ReconnectAttemptLimit == 70", intValue == 70);
					intValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ReconnectMinDelay, -1);
					TestUtilities.checkResult("ReconnectMinDelay == 7000", intValue == 7000);
					intValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ReconnectMaxDelay, -1);
					TestUtilities.checkResult("ReconnectMaxDelay == 7000", intValue == 7000);
					boolean boolValue = JUnitTestConnect.activeConfigGetBooleanValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.XmlTraceToStdout, -1);
					TestUtilities.checkResult("XmlTraceToStdout == 0", boolValue == false);
					boolValue = JUnitTestConnect.activeConfigGetBooleanValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ConsumerMsgKeyInUpdates, -1);
					TestUtilities.checkResult("MsgKeyInUpdates == 0", boolValue == false);
				}
				else if (testCase == 2)
				{
					int intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ItemCountHint, -1);
					TestUtilities.checkResult("ItemCountHint value == 500000", intLongValue == 500000 );
					intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ServiceCountHint, -1);
					TestUtilities.checkResult("ServiceCountHint value == 655", intLongValue == 655 );
					intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ConsumerObeyOpenWindow, -1);
					TestUtilities.checkResult("ObeyOpenWindow value == 0", intLongValue == 0 );
					intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ConsumerPostAckTimeout, -1);
					TestUtilities.checkResult("PostAckTimeout value == 7000", intLongValue == 7000 );
					intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.RequestTimeout, -1);
					TestUtilities.checkResult("RequestTimeout value == 8000", intLongValue == 8000 );
					intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ConsumerMaxOutstandingPosts, -1);
					TestUtilities.checkResult("MaxOutstandingPosts value == 90000", intLongValue == 90000 );
					int intValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.DispatchTimeoutApiThread, -1);
					TestUtilities.checkResult("DispatchTimeoutApiThread value == 90", intValue == 90 );
		
					intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.MaxDispatchCountApiThread, -1);
					TestUtilities.checkResult("MaxDispatchCountApiThread value == 400", intLongValue == 400 );
					intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.MaxDispatchCountUserThread, -1);
					TestUtilities.checkResult("MaxDispatchCountUserThread value == 5", intLongValue == 5 );
		
					intValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ReconnectAttemptLimit, -1);
					TestUtilities.checkResult("ReconnectAttemptLimit == 10", intValue == 10);
					intValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ReconnectMinDelay, -1);
					TestUtilities.checkResult("ReconnectMinDelay == 123", intValue == 123);
					intValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ReconnectMaxDelay, -1);
					TestUtilities.checkResult("ReconnectMaxDelay == 456", intValue == 456);
					boolean boolValue = JUnitTestConnect.activeConfigGetBooleanValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.XmlTraceToStdout, -1);
					TestUtilities.checkResult("XmlTraceToStdout == 0", boolValue == false);
					boolValue = JUnitTestConnect.activeConfigGetBooleanValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ConsumerMsgKeyInUpdates, -1);
					TestUtilities.checkResult("MsgKeyInUpdates == 1", boolValue == true);
				}
				
				// Check values of Consumer_2
				System.out.println("\nRetrieving Consumer_1 configuration values "); 
	
				ConsChannelVal = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelName, 0);
				TestUtilities.checkResult("Channel value != null", ConsChannelVal != null);
				TestUtilities.checkResult("Channel value == Channel_2", ConsChannelVal.contentEquals("Channel_2") );
				ConsDictionary = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeDictionary, JUnitTestConnect.DictionaryName, -1);
				TestUtilities.checkResult("Dictionary != null", ConsDictionary != null);
				TestUtilities.checkResult("Dictionary value == Dictionary_2", ConsDictionary.contentEquals("Dictionary_2") );
				
				// Check Channel configuration:
				// Check Channel_2 configuration.
				ConsChannelVal = "Channel_2";
				System.out.println("\nRetrieving Channel_2 configuration values "); 
				
				if (testCase == 1)
				{
					int channelConnType = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelType, 0);
					TestUtilities.checkResult("channelConnType == ChannelType::RSSL_SOCKET", channelConnType == ChannelTypeSocket);
				
					String strValue = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.InterfaceName, 0);
					TestUtilities.checkResult("InterfaceName == localhost4file", strValue.contentEquals("localhost4file"));
					
					int intValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.CompressionType, 0);
					TestUtilities.checkResult("CompressionType == CompressionType::ZLib", intValue == CompressionTypeZLib);
					
					int intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.GuaranteedOutputBuffers, 0);
					TestUtilities.checkResult("GuaranteedOutputBuffers == 6000", intLongValue == 6000);
					intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.NumInputBuffers, 0);
					TestUtilities.checkResult("NumInputBuffers == 9000", intLongValue == 9000);
					intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.SysRecvBufSize, 0);
					TestUtilities.checkResult("SysRecvBufSize == 23656", intLongValue == 23656);
					intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.SysSendBufSize, 0);
					TestUtilities.checkResult("SysSendBufSize == 63656", intLongValue == 63656);
					intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.CompressionThreshold, 0);
					TestUtilities.checkResult("CompressionThreshold == 4096", intLongValue == 4096);
					intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ConnectionPingTimeout, 0);
					TestUtilities.checkResult("ConnectionPingTimeout == 55555", intLongValue == 55555);
					boolean boolValue = JUnitTestConnect.activeConfigGetBooleanValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.TcpNodelay, 0);
					TestUtilities.checkResult("TcpNodelay == 0", boolValue == false);
					boolValue = JUnitTestConnect.activeConfigGetBooleanValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.DirectWrite, 0);
					TestUtilities.checkResult("DirectWrite == 0", boolValue == false);
					intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.HighWaterMark, 0);
					TestUtilities.checkResult("HighWaterMark == 0", intLongValue == 0);
					String chanHost = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.Host, 0);
					TestUtilities.checkResult("Host == 0.0.0.2", chanHost.contentEquals("0.0.0.2"));
					String chanPort = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.Port, 0);
					TestUtilities.checkResult("Port == 15008", chanPort.contentEquals("15008"));
					
					// Check Dictionary_2 configuration.
					ConsDictionary = "Dictionary_2";
					System.out.println("\nRetrieving Dictionary_2 configuration values ");
					intValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeDictionary, JUnitTestConnect.DictionaryType, -1);
					TestUtilities.checkResult("DictionaryType == DictionaryType::FileDictionary (1)", intValue == 1);		
					strValue =  JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeDictionary, JUnitTestConnect.DictionaryRDMFieldDictFileName, -1);
					TestUtilities.checkResult("RdmFieldDictionaryFileName == ./RDMFieldDictionary", strValue.contentEquals("./RDMFieldDictionary"));		
					strValue =   JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeDictionary, JUnitTestConnect.DictionaryEnumTypeDefFileName, -1);
					TestUtilities.checkResult("RdmFieldDictionaryFileName == ./enumtype.def", strValue.contentEquals("./enumtype.def"));
				}
				else if (testCase == 2)
				{
					int channelConnType = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelType, 0);
					TestUtilities.checkResult("channelConnType == ChannelType::RSSL_SOCKET", channelConnType == ChannelTypeSocket);
				
					String strValue = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.InterfaceName, 0);
					TestUtilities.checkResult("InterfaceName == localhost", strValue.contentEquals("localhost"));
					
					int intValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.CompressionType, 0);
					TestUtilities.checkResult("CompressionType == CompressionType::LZ4", intValue == CompressionTypeLZ4);
					
					int intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.GuaranteedOutputBuffers, 0);
					TestUtilities.checkResult("GuaranteedOutputBuffers == 7000", intLongValue == 7000);
					intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.NumInputBuffers, 0);
					TestUtilities.checkResult("NumInputBuffers == 5000", intLongValue == 5000);
					intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.SysRecvBufSize, 0);
					TestUtilities.checkResult("SysRecvBufSize == 550000", intLongValue == 550000);
					intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.SysSendBufSize, 0);
					TestUtilities.checkResult("SysSendBufSize == 700000", intLongValue == 700000);
					intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.CompressionThreshold, 0);
					TestUtilities.checkResult("CompressionThreshold == 12758", intLongValue == 12758);
					intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ConnectionPingTimeout, 0);
					TestUtilities.checkResult("ConnectionPingTimeout == 70000", intLongValue == 70000);
					boolean boolValue = JUnitTestConnect.activeConfigGetBooleanValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.TcpNodelay, 0);
					TestUtilities.checkResult("TcpNodelay == 1", boolValue == true);
					boolValue = JUnitTestConnect.activeConfigGetBooleanValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.DirectWrite, 0);
					TestUtilities.checkResult("DirectWrite == 1", boolValue == true);
					intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.HighWaterMark, 0);
					TestUtilities.checkResult("HighWaterMark == 5000", intLongValue == 5000);
					String chanHost = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.Host, 0);
					TestUtilities.checkResult("Host == localhost1", chanHost.contentEquals("localhost1"));
					String chanPort = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.Port, 0);
					TestUtilities.checkResult("Port == 14012", chanPort.contentEquals("14012"));
					
					// Check Dictionary_2 configuration.
					ConsDictionary = "Dictionary_2";
					System.out.println("\nRetrieving Dictionary_2 configuration values ");
					intValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeDictionary, JUnitTestConnect.DictionaryType, -1);
					TestUtilities.checkResult("DictionaryType == DictionaryType::ChannelDictionary (0)", intValue == 0);		
					strValue =  JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeDictionary, JUnitTestConnect.DictionaryRDMFieldDictFileName, -1);
					TestUtilities.checkResult("RdmFieldDictionaryFileName == ./ConfigDB3_RDMFieldDictionary", strValue.contentEquals("./ConfigDB3_RDMFieldDictionary"));		
					strValue =   JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeDictionary, JUnitTestConnect.DictionaryEnumTypeDefFileName, -1);
					TestUtilities.checkResult("RdmFieldDictionaryFileName == ./ConfigDB3_enumtype.def", strValue.contentEquals("./ConfigDB3_enumtype.def"));	
				}
	
				cons = null;
			}
		
			catch ( OmmException excp)
			{
				System.out.println(excp.getMessage());
				TestUtilities.checkResult("Receiving exception, test failed.", false );
			}
		}
	}
	
	public void testMergCfgBetweenFunctionCallAndFileAndProgrammatic()
	{
		TestUtilities.printTestHead("testMergCfgBetweenFunctionCallAndFileAndProgrammatic","Test merge all configuration parameters between "
				+ "function call, config file and programmatically config");
		
		//testcase 1: test function call for socket connection
		//testcase 2: test function call for http/encrypted connection

		int[] testCases = {ChannelTypeSocket, ChannelTypeEncrypted};
		for (int testCase : testCases)
		{
			System.out.println("\n #####Now it is running test case " + testCase + "\n");
					
			Map configDB1 = EmaFactory.createMap();
			Map configDB2 = EmaFactory.createMap();
			Map configDB3 = EmaFactory.createMap();
			Map innerMap = EmaFactory.createMap();
			ElementList elementList = EmaFactory.createElementList();
			ElementList innerElementList = EmaFactory.createElementList();
			
			try
			{
				if (testCase == ChannelTypeSocket)
					innerElementList.add(EmaFactory.createElementEntry().ascii("Channel", "Channel_2"));
				else if (testCase == ChannelTypeEncrypted)
					innerElementList.add(EmaFactory.createElementEntry().ascii("Channel", "Channel_6"));
				
				innerElementList.add(EmaFactory.createElementEntry().ascii("Dictionary", "Dictionary_2"));
				innerElementList.add(EmaFactory.createElementEntry().intValue("ItemCountHint", 9000));
				
				innerMap.add(EmaFactory.createMapEntry().keyAscii( "Consumer_2", MapEntry.MapAction.ADD, innerElementList));
				innerElementList.clear();
				
				elementList.add(EmaFactory.createElementEntry().map( "ConsumerList", innerMap ));
				innerMap.clear();
	
				configDB1.add(EmaFactory.createMapEntry().keyAscii( "ConsumerGroup", MapEntry.MapAction.ADD, elementList ));
				elementList.clear();
	
				if (testCase == ChannelTypeSocket)
				{
					innerElementList.add(EmaFactory.createElementEntry().ascii("ChannelType", "ChannelType::RSSL_SOCKET"));
					innerElementList.add(EmaFactory.createElementEntry().ascii("Host", "localhost1"));
					innerElementList.add(EmaFactory.createElementEntry().ascii("Port", "14012"));
				}
				else if (testCase == ChannelTypeEncrypted)
				{
					innerElementList.add(EmaFactory.createElementEntry().ascii("ChannelType", "ChannelType::RSSL_ENCRYPTED"));
					innerElementList.add(EmaFactory.createElementEntry().ascii("ProxyHost", "proxyhost6"));
					innerElementList.add(EmaFactory.createElementEntry().ascii("ProxyPort", "proxyport6"));
					innerElementList.add(EmaFactory.createElementEntry().ascii("ObjectName", "objectname6"));
				}
				
				innerElementList.add(EmaFactory.createElementEntry().ascii("InterfaceName", "localhost"));
				innerElementList.add(EmaFactory.createElementEntry().ascii("CompressionType", "CompressionType::LZ4"));
				innerElementList.add(EmaFactory.createElementEntry().intValue("GuaranteedOutputBuffers", 7000));
				innerElementList.add(EmaFactory.createElementEntry().intValue("NumInputBuffers", 5000));
				innerElementList.add(EmaFactory.createElementEntry().intValue("SysRecvBufSize", 550000));
				innerElementList.add(EmaFactory.createElementEntry().intValue("SysSendBufSize", 700000));
				innerElementList.add(EmaFactory.createElementEntry().intValue("CompressionThreshold", 12758));
				innerElementList.add(EmaFactory.createElementEntry().intValue("ConnectionPingTimeout", 70000));
				innerElementList.add(EmaFactory.createElementEntry().intValue("TcpNodelay", 1));
				
				if (testCase == ChannelTypeSocket)
					innerMap.add(EmaFactory.createMapEntry().keyAscii( "Channel_2", MapEntry.MapAction.ADD, innerElementList));
				else if (testCase == ChannelTypeEncrypted)
					innerMap.add(EmaFactory.createMapEntry().keyAscii( "Channel_6", MapEntry.MapAction.ADD, innerElementList));
				innerElementList.clear();
	
				elementList.add(EmaFactory.createElementEntry().map( "ChannelList", innerMap ));
				innerMap.clear();
	
				configDB2.add(EmaFactory.createMapEntry().keyAscii( "ChannelGroup", MapEntry.MapAction.ADD, elementList ));
				elementList.clear();
	
				innerElementList.add(EmaFactory.createElementEntry().ascii("DictionaryType", "DictionaryType::ChannelDictionary"));
				innerElementList.add(EmaFactory.createElementEntry().ascii("RdmFieldDictionaryFileName", "./ConfigDB3_RDMFieldDictionary"));
				innerElementList.add(EmaFactory.createElementEntry().ascii("EnumTypeDefFileName", "./ConfigDB3_enumtype.def"));
				
				innerMap.add(EmaFactory.createMapEntry().keyAscii( "Dictionary_2", MapEntry.MapAction.ADD, innerElementList));
				innerElementList.clear();
				
				elementList.add(EmaFactory.createElementEntry().map( "DictionaryList", innerMap ));
	
				configDB3.add(EmaFactory.createMapEntry().keyAscii( "DictionaryGroup", MapEntry.MapAction.ADD, elementList ));
	
				// To specify EmaConfig.xml file location use -DEmaConfigFileLocation=EmaConfig.xml
				String EmaConfigFileLocation = System.getProperty("EmaConfigFileLocation");
				if ( EmaConfigFileLocation == null )
				{
					EmaConfigFileLocation = "./src/test/resources/com/rtsdk/ema/unittest/EmaFileConfigTests/EmaConfigTest.xml";
					System.out.println("EmaConfig.xml file not specified, using default file");
				}
				else
				{
					System.out.println("Using Ema Config: " + EmaConfigFileLocation);
				}
				
				OmmConsumerConfig testConfig = null;
				if (testCase == ChannelTypeSocket)
					testConfig = EmaFactory.createOmmConsumerConfig(EmaConfigFileLocation).config(configDB1).config(configDB2).config(configDB3).host("localhostFC:14022");
				else if (testCase == ChannelTypeEncrypted)
				{
					testConfig = EmaFactory.createOmmConsumerConfig(EmaConfigFileLocation).config(configDB1).config(configDB2).config(configDB3)
							.tunnelingCredentialDomain("domain").tunnelingProxyHostName("proxyHost").tunnelingProxyPort("14032").tunnelingObjectName("objectName");
				}
				
				OmmConsumer cons = JUnitTestConnect.createOmmConsumer(testConfig);
	
				// Check default consumer name (Conusmer_2) and associated values
				System.out.println("Retrieving DefaultConsumer configuration values: (DefaultConsumer value=Consumer_2) "); 
			
				String defaultConsName = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ConsumerDefaultConsumerName, -1);
				TestUtilities.checkResult("DefaultConsumer value != null", defaultConsName != null);
				TestUtilities.checkResult("DefaultConsumer value == Consumer_2", defaultConsName.contentEquals("Consumer_2") );
				String ConsChannelVal = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelName, 0);
				String ConsDictionary = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeDictionary, JUnitTestConnect.DictionaryName, -1);
				TestUtilities.checkResult("Dictionary != null", ConsDictionary != null);
				TestUtilities.checkResult("Dictionary value == Dictionary_2", ConsDictionary.contentEquals("Dictionary_2") );
				int intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ItemCountHint, -1);
				TestUtilities.checkResult("ItemCountHint value == 9000", intLongValue == 9000 );
				intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ServiceCountHint, -1);
				TestUtilities.checkResult("ServiceCountHint value == 655", intLongValue == 655 );
				intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ConsumerObeyOpenWindow, -1);
				TestUtilities.checkResult("ObeyOpenWindow value == 0", intLongValue == 0 );
				intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ConsumerPostAckTimeout, -1);
				TestUtilities.checkResult("PostAckTimeout value == 7000", intLongValue == 7000 );
				intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.RequestTimeout, -1);
				TestUtilities.checkResult("RequestTimeout value == 8000", intLongValue == 8000 );
				intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ConsumerMaxOutstandingPosts, -1);
				TestUtilities.checkResult("MaxOutstandingPosts value == 90000", intLongValue == 90000 );
				int intValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.DispatchTimeoutApiThread, -1);
				TestUtilities.checkResult("DispatchTimeoutApiThread value == 90", intValue == 90 );
	
				intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.MaxDispatchCountApiThread, -1);
				TestUtilities.checkResult("MaxDispatchCountApiThread value == 400", intLongValue == 400 );
				intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.MaxDispatchCountUserThread, -1);
				TestUtilities.checkResult("MaxDispatchCountUserThread value == 5", intLongValue == 5 );
	
				intValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ReconnectAttemptLimit, -1);
				TestUtilities.checkResult("ReconnectAttemptLimit == 10", intValue == 10);
				intValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ReconnectMinDelay, -1);
				TestUtilities.checkResult("ReconnectMinDelay == 123", intValue == 123);
				intValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ReconnectMaxDelay, -1);
				TestUtilities.checkResult("ReconnectMaxDelay == 456", intValue == 456);
				boolean boolValue = JUnitTestConnect.activeConfigGetBooleanValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.XmlTraceToStdout, -1);
				TestUtilities.checkResult("XmlTraceToStdout == 0", boolValue == false);
				boolValue = JUnitTestConnect.activeConfigGetBooleanValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ConsumerMsgKeyInUpdates, -1);
				TestUtilities.checkResult("MsgKeyInUpdates == 1", boolValue == true);
				
				
				// Check values of Consumer_2
				System.out.println("\nRetrieving Consumer_1 configuration values "); 
	
				ConsDictionary = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeDictionary, JUnitTestConnect.DictionaryName, -1);
				TestUtilities.checkResult("Dictionary != null", ConsDictionary != null);
				TestUtilities.checkResult("Dictionary value == Dictionary_2", ConsDictionary.contentEquals("Dictionary_2") );
				
				// Check Channel configuration:
				// Check Channel_2 configuration.
				if (testCase == ChannelTypeSocket)
				{
					ConsChannelVal = "Channel_2";
					System.out.println("\nRetrieving Channel_2 configuration values "); 
					ConsChannelVal = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelName, 0);
					TestUtilities.checkResult("Channel value != null", ConsChannelVal != null);
					TestUtilities.checkResult("Channel value == Channel_2", ConsChannelVal.contentEquals("Channel_2") );
					int channelConnType = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelType, 0);
					TestUtilities.checkResult("channelConnType == ChannelType::RSSL_SOCKET", channelConnType == ChannelTypeSocket);
				}
				else if (testCase == ChannelTypeEncrypted)
				{
					ConsChannelVal = "Channel_6";
					System.out.println("\nRetrieving Channel_6 configuration values "); 
					ConsChannelVal = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelName, 0);
					TestUtilities.checkResult("Channel value != null", ConsChannelVal != null);
					TestUtilities.checkResult("Channel value == Channel_6", ConsChannelVal.contentEquals("Channel_6") );
					int channelConnType = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelType, 0);
					TestUtilities.checkResult("channelConnType == ChannelType::RSSL_ENCRYPTED", channelConnType == ChannelTypeEncrypted);
				}
				
				String strValue = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.InterfaceName, 0);
				TestUtilities.checkResult("InterfaceName == localhost", strValue.contentEquals("localhost"));
				
				intValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.CompressionType, 0);
				TestUtilities.checkResult("CompressionType == CompressionType::LZ4", intValue == CompressionTypeLZ4);
				
				intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.GuaranteedOutputBuffers, 0);
				TestUtilities.checkResult("GuaranteedOutputBuffers == 7000", intLongValue == 7000);
				intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.NumInputBuffers, 0);
				TestUtilities.checkResult("NumInputBuffers == 5000", intLongValue == 5000);
				intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.SysRecvBufSize, 0);
				TestUtilities.checkResult("SysRecvBufSize == 550000", intLongValue == 550000);
				intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.SysSendBufSize, 0);
				TestUtilities.checkResult("SysSendBufSize == 700000", intLongValue == 700000);
				intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.CompressionThreshold, 0);
				TestUtilities.checkResult("CompressionThreshold == 12758", intLongValue == 12758);
				intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ConnectionPingTimeout, 0);
				TestUtilities.checkResult("ConnectionPingTimeout == 70000", intLongValue == 70000);
				boolValue = JUnitTestConnect.activeConfigGetBooleanValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.TcpNodelay, 0);
				TestUtilities.checkResult("TcpNodelay == 1", boolValue == true);
				if (testCase == ChannelTypeSocket)
				{
					String chanHost = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.Host, 0);
					TestUtilities.checkResult("Host == localhostFC", chanHost.contentEquals("localhostFC"));
					String chanPort = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.Port, 0);
					TestUtilities.checkResult("Port == 14022", chanPort.contentEquals("14022"));
				}
				else if (testCase == ChannelTypeEncrypted)
				{
					String chanHost = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ProxyHost, 0);
					TestUtilities.checkResult("ProxyHost == proxyHost", chanHost.contentEquals("proxyHost"));
					String chanPort = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ProxyPort, 0);
					TestUtilities.checkResult("ProxyPort == 14032", chanPort.contentEquals("14032"));
					String chanObj = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ObjectName, 0);
					TestUtilities.checkResult("ObjectName == objectName", chanObj.contentEquals("objectName"));
				}
				
				cons = null;
			}
			catch ( OmmException excp)
			{
				System.out.println(excp.getMessage());
				TestUtilities.checkResult("Receiving exception, test failed.", false );
			}
		}
	}
	
	public void testMergCfgBetweenFunctionCallAndFileAndProgrammaticNiProv()
	{
		TestUtilities.printTestHead("testMergCfgBetweenFunctionCallAndFileAndProgrammaticNiProv","Test merge all configuration parameters between "
				+ "function call, config file and programmatically config");
		
		// test function call for http/encrypted connection
				
		Map configDB1 = EmaFactory.createMap();
		Map configDB2 = EmaFactory.createMap();
		Map configDB3 = EmaFactory.createMap();
		Map innerMap = EmaFactory.createMap();
		ElementList elementList = EmaFactory.createElementList();
		ElementList innerElementList = EmaFactory.createElementList();
		
		try
		{
			innerElementList.add(EmaFactory.createElementEntry().ascii("Channel", "Channel_6"));
			
			innerElementList.add(EmaFactory.createElementEntry().intValue("ItemCountHint", 9000));
			
			innerMap.add(EmaFactory.createMapEntry().keyAscii( "Provider_2", MapEntry.MapAction.ADD, innerElementList));
			innerElementList.clear();
			
			elementList.add(EmaFactory.createElementEntry().map( "NiProviderList", innerMap ));
			innerMap.clear();

			configDB1.add(EmaFactory.createMapEntry().keyAscii( "NiProviderGroup", MapEntry.MapAction.ADD, elementList ));
			elementList.clear();

			innerElementList.add(EmaFactory.createElementEntry().ascii("ChannelType", "ChannelType::RSSL_ENCRYPTED"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("ProxyHost", "proxyhost6"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("ProxyPort", "proxyport6"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("ObjectName", "objectname6"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("InterfaceName", "localhost"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("CompressionType", "CompressionType::LZ4"));
			innerElementList.add(EmaFactory.createElementEntry().intValue("GuaranteedOutputBuffers", 7000));
			innerElementList.add(EmaFactory.createElementEntry().intValue("NumInputBuffers", 5000));
			innerElementList.add(EmaFactory.createElementEntry().intValue("SysRecvBufSize", 550000));
			innerElementList.add(EmaFactory.createElementEntry().intValue("SysSendBufSize", 700000));
			innerElementList.add(EmaFactory.createElementEntry().intValue("CompressionThreshold", 12758));
			innerElementList.add(EmaFactory.createElementEntry().intValue("ConnectionPingTimeout", 70000));
			innerElementList.add(EmaFactory.createElementEntry().intValue("TcpNodelay", 1));
			innerElementList.add(EmaFactory.createElementEntry().intValue("InitializationTimeout", 99));
			
			innerMap.add(EmaFactory.createMapEntry().keyAscii( "Channel_6", MapEntry.MapAction.ADD, innerElementList));
			
			innerElementList.clear();

			elementList.add(EmaFactory.createElementEntry().map( "ChannelList", innerMap ));
			innerMap.clear();

			configDB2.add(EmaFactory.createMapEntry().keyAscii( "ChannelGroup", MapEntry.MapAction.ADD, elementList ));
			elementList.clear();

			innerElementList.add(EmaFactory.createElementEntry().ascii("RdmFieldDictionaryFileName", "./ConfigDB3_RDMFieldDictionary"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("EnumTypeDefFileName", "./ConfigDB3_enumtype.def"));
			
			innerElementList.clear();

			// To specify EmaConfig.xml file location use -DEmaConfigFileLocation=EmaConfig.xml
			String EmaConfigFileLocation = System.getProperty("EmaConfigFileLocation");
			if ( EmaConfigFileLocation == null )
			{
				EmaConfigFileLocation = "./src/test/resources/com/rtsdk/ema/unittest/EmaFileConfigTests/EmaConfigTest.xml";
				System.out.println("EmaConfig.xml file not specified, using default file");
			}
			else
			{
				System.out.println("Using Ema Config: " + EmaConfigFileLocation);
			}
			
			OmmNiProviderConfig testConfig = null;
			testConfig = EmaFactory.createOmmNiProviderConfig(EmaConfigFileLocation).config(configDB1).config(configDB2).config(configDB3)
					.tunnelingCredentialDomain("domain").tunnelingProxyHostName("proxyHost").tunnelingProxyPort("14032").tunnelingObjectName("objectName");
			
			OmmProvider niProv = JUnitTestConnect.createOmmNiProvider(testConfig);

			// Check default niprovider name (Provider_2) and associated values
			System.out.println("Retrieving DefaultNiProvider configuration values: (DefaultNiProvider value=Provider_2) "); 
		
			String defaultNiProvName = JUnitTestConnect.activeConfigGetStringValue(niProv, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.NiProviderName, -1);
			TestUtilities.checkResult("DefaultNiProvider value != null", defaultNiProvName != null);
			TestUtilities.checkResult("DefaultNiProvider value == Provider_2", defaultNiProvName.contentEquals("Provider_2") );
			String NiProvChannelVal = JUnitTestConnect.activeConfigGetStringValue(niProv, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelName, 0);
			boolean boolValue = JUnitTestConnect.activeConfigGetBooleanValue(niProv, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.XmlTraceToStdout, -1);
			TestUtilities.checkResult("XmlTraceToStdout == 0", boolValue == false);
			
			// Check values of Provider_2
			System.out.println("\nRetrieving Provider_1 configuration values "); 
			
			NiProvChannelVal = "Channel_6";
			System.out.println("\nRetrieving Channel_6 configuration values "); 
			NiProvChannelVal = JUnitTestConnect.activeConfigGetStringValue(niProv, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelName, 0);
			TestUtilities.checkResult("Channel value != null", NiProvChannelVal != null);
			TestUtilities.checkResult("Channel value == Channel_6", NiProvChannelVal.contentEquals("Channel_6") );
			int channelConnType = JUnitTestConnect.activeConfigGetIntLongValue(niProv, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelType, 0);
			TestUtilities.checkResult("channelConnType == ChannelType::RSSL_ENCRYPTED", channelConnType == ChannelTypeEncrypted);
			
			String strValue = JUnitTestConnect.activeConfigGetStringValue(niProv, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.InterfaceName, 0);
			TestUtilities.checkResult("InterfaceName == localhost", strValue.contentEquals("localhost"));
			
			int intValue = JUnitTestConnect.activeConfigGetIntLongValue(niProv, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.CompressionType, 0);
			TestUtilities.checkResult("CompressionType == CompressionType::LZ4", intValue == CompressionTypeLZ4);
			
			int intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(niProv, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.GuaranteedOutputBuffers, 0);
			TestUtilities.checkResult("GuaranteedOutputBuffers == 7000", intLongValue == 7000);
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(niProv, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.NumInputBuffers, 0);
			TestUtilities.checkResult("NumInputBuffers == 5000", intLongValue == 5000);
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(niProv, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.SysRecvBufSize, 0);
			TestUtilities.checkResult("SysRecvBufSize == 550000", intLongValue == 550000);
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(niProv, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.SysSendBufSize, 0);
			TestUtilities.checkResult("SysSendBufSize == 700000", intLongValue == 700000);
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(niProv, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.CompressionThreshold, 0);
			TestUtilities.checkResult("CompressionThreshold == 12758", intLongValue == 12758);
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(niProv, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ConnectionPingTimeout, 0);
			TestUtilities.checkResult("ConnectionPingTimeout == 70000", intLongValue == 70000);
			boolValue = JUnitTestConnect.activeConfigGetBooleanValue(niProv, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.TcpNodelay, 0);
			TestUtilities.checkResult("TcpNodelay == 1", boolValue == true);

			String chanHost = JUnitTestConnect.activeConfigGetStringValue(niProv, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ProxyHost, 0);
			TestUtilities.checkResult("ProxyHost == proxyHost", chanHost.contentEquals("proxyHost"));
			String chanPort = JUnitTestConnect.activeConfigGetStringValue(niProv, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ProxyPort, 0);
			TestUtilities.checkResult("ProxyPort == 14032", chanPort.contentEquals("14032"));
			String chanObj = JUnitTestConnect.activeConfigGetStringValue(niProv, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ObjectName, 0);
			TestUtilities.checkResult("ObjectName == objectName", chanObj.contentEquals("objectName"));
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(niProv, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelInitTimeout, 0);
			TestUtilities.checkResult("InitializationTimeout == 99", intLongValue == 99);
			
			niProv = null;
		}
		catch ( OmmException excp)
		{
			System.out.println(excp.getMessage());
			TestUtilities.checkResult("Receiving exception, test failed.", false );
		}
		
	}
	
	public void testMergCfgBetweenFunctionCallAndProgrammatic()
	{
		TestUtilities.printTestHead("testMergCfgBetweenFunctionCallAndProgrammatic","Test merge all configuration parameters between "
				+ "function call and programmatically config");
		
		//testcase 1: test function call for socket connection
		//testcase 2: test function call for encrypted connection
		//testcase 3: test function call for http connection

		int[] testCases = {ChannelTypeSocket, ChannelTypeEncrypted, ChannelTypeHttp};
		for (int testCase : testCases)
		{
			System.out.println("\n #####Now it is running test case " + testCase + "\n");
					
			Map configDB1 = EmaFactory.createMap();
			Map configDB2 = EmaFactory.createMap();
			Map configDB3 = EmaFactory.createMap();
			Map innerMap = EmaFactory.createMap();
			ElementList elementList = EmaFactory.createElementList();
			ElementList innerElementList = EmaFactory.createElementList();
			
			try
			{
				elementList.add(EmaFactory.createElementEntry().ascii("DefaultConsumer", "Consumer_2"));
				
				if (testCase == ChannelTypeSocket)
					innerElementList.add(EmaFactory.createElementEntry().ascii("Channel", "Channel_2"));
				else if (testCase == ChannelTypeEncrypted || testCase == ChannelTypeHttp)
					innerElementList.add(EmaFactory.createElementEntry().ascii("Channel", "Channel_6"));
				
				innerElementList.add(EmaFactory.createElementEntry().ascii("Dictionary", "Dictionary_2"));
				innerElementList.add(EmaFactory.createElementEntry().intValue("ItemCountHint", 9000));
				innerElementList.add(EmaFactory.createElementEntry().intValue("ServiceCountHint", 9000));
				innerElementList.add(EmaFactory.createElementEntry().intValue("ObeyOpenWindow", 1));
				innerElementList.add(EmaFactory.createElementEntry().intValue("PostAckTimeout", 9000));
				innerElementList.add(EmaFactory.createElementEntry().intValue("RequestTimeout", 9000));
				innerElementList.add(EmaFactory.createElementEntry().intValue("MaxOutstandingPosts", 9000));
				innerElementList.add(EmaFactory.createElementEntry().intValue("DispatchTimeoutApiThread", 5656));
				innerElementList.add(EmaFactory.createElementEntry().intValue("MaxDispatchCountApiThread", 900));
				innerElementList.add(EmaFactory.createElementEntry().intValue("MaxDispatchCountUserThread", 900));
				innerElementList.add(EmaFactory.createElementEntry().intValue("XmlTraceToStdout", 0));
				innerElementList.add(EmaFactory.createElementEntry().intValue("MsgKeyInUpdates", 0));
				innerElementList.add(EmaFactory.createElementEntry().intValue("ReconnectAttemptLimit", 70));
				innerElementList.add(EmaFactory.createElementEntry().intValue("ReconnectMinDelay", 7000));
				innerElementList.add(EmaFactory.createElementEntry().intValue("ReconnectMaxDelay", 7000));
				
				innerMap.add(EmaFactory.createMapEntry().keyAscii( "Consumer_2", MapEntry.MapAction.ADD, innerElementList));
				innerElementList.clear();
				
				elementList.add(EmaFactory.createElementEntry().map( "ConsumerList", innerMap ));
				innerMap.clear();
	
				configDB1.add(EmaFactory.createMapEntry().keyAscii( "ConsumerGroup", MapEntry.MapAction.ADD, elementList ));
				elementList.clear();
	
				if (testCase == ChannelTypeSocket)
					innerElementList.add(EmaFactory.createElementEntry().ascii("ChannelType", "ChannelType::RSSL_SOCKET"));
				else if (testCase == ChannelTypeEncrypted)
					innerElementList.add(EmaFactory.createElementEntry().ascii("ChannelType", "ChannelType::RSSL_ENCRYPTED"));
				else if (testCase == ChannelTypeHttp)
					innerElementList.add(EmaFactory.createElementEntry().ascii("ChannelType", "ChannelType::RSSL_HTTP"));
				
				innerElementList.add(EmaFactory.createElementEntry().ascii("InterfaceName", "localhost"));
				innerElementList.add(EmaFactory.createElementEntry().ascii("CompressionType", "CompressionType::LZ4"));
				innerElementList.add(EmaFactory.createElementEntry().intValue("GuaranteedOutputBuffers", 7000));
				innerElementList.add(EmaFactory.createElementEntry().intValue("NumInputBuffers", 5000));
				innerElementList.add(EmaFactory.createElementEntry().intValue("SysRecvBufSize", 550000));
				innerElementList.add(EmaFactory.createElementEntry().intValue("SysSendBufSize", 700000));
				innerElementList.add(EmaFactory.createElementEntry().intValue("CompressionThreshold", 12758));
				innerElementList.add(EmaFactory.createElementEntry().intValue("ConnectionPingTimeout", 70000));
				innerElementList.add(EmaFactory.createElementEntry().ascii("Host", "localhost1"));
				innerElementList.add(EmaFactory.createElementEntry().ascii("Port", "14012"));
				innerElementList.add(EmaFactory.createElementEntry().intValue("TcpNodelay", 1));
				
				if (testCase == ChannelTypeSocket)
					innerMap.add(EmaFactory.createMapEntry().keyAscii( "Channel_2", MapEntry.MapAction.ADD, innerElementList));
				else if (testCase == ChannelTypeEncrypted || testCase == ChannelTypeHttp)
					innerMap.add(EmaFactory.createMapEntry().keyAscii( "Channel_6", MapEntry.MapAction.ADD, innerElementList));
				innerElementList.clear();
	
				elementList.add(EmaFactory.createElementEntry().map( "ChannelList", innerMap ));
				innerMap.clear();
	
				configDB2.add(EmaFactory.createMapEntry().keyAscii( "ChannelGroup", MapEntry.MapAction.ADD, elementList ));
				elementList.clear();
	
				innerElementList.add(EmaFactory.createElementEntry().ascii("DictionaryType", "DictionaryType::ChannelDictionary"));
				innerElementList.add(EmaFactory.createElementEntry().ascii("RdmFieldDictionaryFileName", "./ConfigDB3_RDMFieldDictionary"));
				innerElementList.add(EmaFactory.createElementEntry().ascii("EnumTypeDefFileName", "./ConfigDB3_enumtype.def"));
				
				innerMap.add(EmaFactory.createMapEntry().keyAscii( "Dictionary_2", MapEntry.MapAction.ADD, innerElementList));
				innerElementList.clear();
				
				elementList.add(EmaFactory.createElementEntry().map( "DictionaryList", innerMap ));
	
				configDB3.add(EmaFactory.createMapEntry().keyAscii( "DictionaryGroup", MapEntry.MapAction.ADD, elementList ));
	
				System.out.println("\n###### NOT USE Ema Config file\n");
				
				OmmConsumerConfig testConfig = null;
				if (testCase == ChannelTypeSocket)
					testConfig = EmaFactory.createOmmConsumerConfig().config(configDB1).config(configDB2).config(configDB3).host("localhostFC:14022");
				else if (testCase == ChannelTypeEncrypted)
				{
					testConfig = EmaFactory.createOmmConsumerConfig().config(configDB1).config(configDB2).config(configDB3)
							.tunnelingProxyHostName("proxyHost").tunnelingProxyPort("14032").tunnelingObjectName("objectName");
				}
				else if (testCase == ChannelTypeHttp)
				{
					testConfig = EmaFactory.createOmmConsumerConfig().config(configDB1).config(configDB2).config(configDB3)
							.tunnelingProxyHostName("proxyHost1").tunnelingProxyPort("14033").tunnelingObjectName("objectName1");
				}
				
				OmmConsumer cons = JUnitTestConnect.createOmmConsumer(testConfig);
	
				// Check default consumer name (Conusmer_2) and associated values
				System.out.println("Retrieving DefaultConsumer configuration values: (DefaultConsumer value=Consumer_2) "); 
			
				String defaultConsName = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ConsumerDefaultConsumerName, -1);
				TestUtilities.checkResult("DefaultConsumer value != null", defaultConsName != null);
				TestUtilities.checkResult("DefaultConsumer value == Consumer_2", defaultConsName.contentEquals("Consumer_2") );
				String ConsChannelVal = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelName, 0);
				String ConsDictionary = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeDictionary, JUnitTestConnect.DictionaryName, -1);
				TestUtilities.checkResult("Dictionary != null", ConsDictionary != null);
				TestUtilities.checkResult("Dictionary value == Dictionary_2", ConsDictionary.contentEquals("Dictionary_2") );
				int intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ItemCountHint, -1);
				TestUtilities.checkResult("ItemCountHint value == 9000", intLongValue == 9000 );
				intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ServiceCountHint, -1);
				TestUtilities.checkResult("ServiceCountHint value == 9000", intLongValue == 9000 );
				intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ConsumerObeyOpenWindow, -1);
				TestUtilities.checkResult("ObeyOpenWindow value == 1", intLongValue == 1 );
				intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ConsumerPostAckTimeout, -1);
				TestUtilities.checkResult("PostAckTimeout value == 9000", intLongValue == 9000 );
				intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.RequestTimeout, -1);
				TestUtilities.checkResult("RequestTimeout value == 9000", intLongValue == 9000 );
				intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ConsumerMaxOutstandingPosts, -1);
				TestUtilities.checkResult("MaxOutstandingPosts value == 9000", intLongValue == 9000 );
				int intValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.DispatchTimeoutApiThread, -1);
				TestUtilities.checkResult("DispatchTimeoutApiThread value == 5656", intValue == 5656 );

				intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.MaxDispatchCountApiThread, -1);
				TestUtilities.checkResult("MaxDispatchCountApiThread value == 900", intLongValue == 900 );
				intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.MaxDispatchCountUserThread, -1);
				TestUtilities.checkResult("MaxDispatchCountUserThread value == 900", intLongValue == 900 );

				intValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ReconnectAttemptLimit, -1);
				TestUtilities.checkResult("ReconnectAttemptLimit == 70", intValue == 70);
				intValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ReconnectMinDelay, -1);
				TestUtilities.checkResult("ReconnectMinDelay == 7000", intValue == 7000);
				intValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ReconnectMaxDelay, -1);
				TestUtilities.checkResult("ReconnectMaxDelay == 7000", intValue == 7000);
				boolean boolValue = JUnitTestConnect.activeConfigGetBooleanValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.XmlTraceToStdout, -1);
				TestUtilities.checkResult("XmlTraceToStdout == 0", boolValue == false);
				boolValue = JUnitTestConnect.activeConfigGetBooleanValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ConsumerMsgKeyInUpdates, -1);
				TestUtilities.checkResult("MsgKeyInUpdates == 0", boolValue == false);
				
				// Check values of Consumer_2
				System.out.println("\nRetrieving Consumer_1 configuration values "); 
	
				ConsDictionary = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeDictionary, JUnitTestConnect.DictionaryName, -1);
				TestUtilities.checkResult("Dictionary != null", ConsDictionary != null);
				TestUtilities.checkResult("Dictionary value == Dictionary_2", ConsDictionary.contentEquals("Dictionary_2") );
				
				// Check Channel configuration:
				// Check Channel_2 configuration.
				if (testCase == ChannelTypeSocket)
				{
					ConsChannelVal = "Channel_2";
					System.out.println("\nRetrieving Channel_2 configuration values "); 
					ConsChannelVal = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelName, 0);
					TestUtilities.checkResult("Channel value != null", ConsChannelVal != null);
					TestUtilities.checkResult("Channel value == Channel_2", ConsChannelVal.contentEquals("Channel_2") );
					int channelConnType = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelType, 0);
					TestUtilities.checkResult("channelConnType == ChannelType::RSSL_SOCKET", channelConnType == ChannelTypeSocket);
				}
				else if (testCase == ChannelTypeEncrypted)
				{
					ConsChannelVal = "Channel_6";
					System.out.println("\nRetrieving Channel_6 configuration values "); 
					ConsChannelVal = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelName, 0);
					TestUtilities.checkResult("Channel value != null", ConsChannelVal != null);
					TestUtilities.checkResult("Channel value == Channel_6", ConsChannelVal.contentEquals("Channel_6") );
					int channelConnType = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelType, 0);
					TestUtilities.checkResult("channelConnType == ChannelType::RSSL_ENCRYPTED", channelConnType == ChannelTypeEncrypted);
				}
				else if (testCase == ChannelTypeHttp)
				{
					ConsChannelVal = "Channel_6";
					System.out.println("\nRetrieving Channel_6 configuration values "); 
					ConsChannelVal = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelName, 0);
					TestUtilities.checkResult("Channel value != null", ConsChannelVal != null);
					TestUtilities.checkResult("Channel value == Channel_6", ConsChannelVal.contentEquals("Channel_6") );
					int channelConnType = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelType, 0);
					TestUtilities.checkResult("channelConnType == ChannelType::RSSL_HTTP", channelConnType == ChannelTypeHttp);
				}
				
				String strValue = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.InterfaceName, 0);
				TestUtilities.checkResult("InterfaceName == localhost", strValue.contentEquals("localhost"));
				
				intValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.CompressionType, 0);
				TestUtilities.checkResult("CompressionType == CompressionType::LZ4", intValue == CompressionTypeLZ4);
				
				intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.GuaranteedOutputBuffers, 0);
				TestUtilities.checkResult("GuaranteedOutputBuffers == 7000", intLongValue == 7000);
				intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.NumInputBuffers, 0);
				TestUtilities.checkResult("NumInputBuffers == 5000", intLongValue == 5000);
				intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.SysRecvBufSize, 0);
				TestUtilities.checkResult("SysRecvBufSize == 550000", intLongValue == 550000);
				intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.SysSendBufSize, 0);
				TestUtilities.checkResult("SysSendBufSize == 700000", intLongValue == 700000);
				intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.CompressionThreshold, 0);
				TestUtilities.checkResult("CompressionThreshold == 12758", intLongValue == 12758);
				intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ConnectionPingTimeout, 0);
				TestUtilities.checkResult("ConnectionPingTimeout == 70000", intLongValue == 70000);
				boolValue = JUnitTestConnect.activeConfigGetBooleanValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.TcpNodelay, 0);
				TestUtilities.checkResult("TcpNodelay == 1", boolValue == true);
				if (testCase == ChannelTypeSocket)
				{
					String chanHost = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.Host, 0);
					TestUtilities.checkResult("Host == localhostFC", chanHost.contentEquals("localhostFC"));
					String chanPort = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.Port, 0);
					TestUtilities.checkResult("Port == 14022", chanPort.contentEquals("14022"));
				}
				else if (testCase == ChannelTypeEncrypted)
				{
					String chanHost = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ProxyHost, 0);
					TestUtilities.checkResult("ProxyHost == proxyHost", chanHost.contentEquals("proxyHost"));
					String chanPort = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ProxyPort, 0);
					TestUtilities.checkResult("ProxyPort == 14032", chanPort.contentEquals("14032"));
					String chanObj = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ObjectName, 0);
					TestUtilities.checkResult("ObjectName == objectName", chanObj.contentEquals("objectName"));
				}
				else if (testCase == ChannelTypeHttp)
				{
					String chanHost = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ProxyHost, 0);
					TestUtilities.checkResult("ProxyHost == proxyHost1", chanHost.contentEquals("proxyHost1"));
					String chanPort = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ProxyPort, 0);
					TestUtilities.checkResult("ProxyPort == 14033", chanPort.contentEquals("14033"));
					String chanObj = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ObjectName, 0);
					TestUtilities.checkResult("ObjectName == objectName1", chanObj.contentEquals("objectName1"));
				}
				
				cons = null;
			}
			catch ( OmmException excp)
			{
				System.out.println(excp.getMessage());
				TestUtilities.checkResult("Receiving exception, test failed.", false );
			}
		}
	}

public void testLoadChannelSetBwteenFileProgrammaticForNiProv()
{
	TestUtilities.printTestHead("testLoadChannelSetBwteenFileProgrammaticForNiProv","Test loading NiProvider Channel and ChannelSet between "
			+ "config file and programmatically config");
	
	//testcase 1: test setting channel, then channelset. ChannelSet takes priority.
	//testcase 2: test setting channelset, then channel. Channel takes priority.
	for (int testCase = 0; testCase < 2; testCase++)
	{
		System.out.println(" #####Now it is running test case " + testCase);

		Map outermostMap = EmaFactory.createMap();
		Map innerMap = EmaFactory.createMap();
		ElementList elementList = EmaFactory.createElementList();
		ElementList innerElementList = EmaFactory.createElementList();
		try
		{
			elementList.add(EmaFactory.createElementEntry().ascii("DefaultNiProvider", "Provider_3"));
			
			if (testCase == 0)
			{
				innerElementList.add(EmaFactory.createElementEntry().ascii("Channel", "Channel_1"));
				innerElementList.add(EmaFactory.createElementEntry().ascii("ChannelSet", "Channel_2, Channel_3"));
				innerElementList.add(EmaFactory.createElementEntry().ascii("Directory", "Directory_5"));
			}
			else if (testCase == 1)
			{
				innerElementList.add(EmaFactory.createElementEntry().ascii("ChannelSet", "Channel_2, Channel_3"));
				innerElementList.add(EmaFactory.createElementEntry().ascii("Channel", "Channel_1"));
				innerElementList.add(EmaFactory.createElementEntry().ascii("Directory", "Directory_5"));
			}
			
			innerMap.add(EmaFactory.createMapEntry().keyAscii( "Provider_3", MapEntry.MapAction.ADD, innerElementList));
			innerElementList.clear();
			
			elementList.add(EmaFactory.createElementEntry().map("NiProviderList", innerMap));
			innerMap.clear();

			outermostMap.add(EmaFactory.createMapEntry().keyAscii( "NiProviderGroup", MapEntry.MapAction.ADD, elementList));
			elementList.clear();
			
			//encode Channel_1
			innerElementList.add(EmaFactory.createElementEntry().ascii("ChannelType", "ChannelType::RSSL_SOCKET"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("InterfaceName", "localhost"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("Host", "localhost1"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("Port", "14002"));
			innerElementList.add(EmaFactory.createElementEntry().intValue("TcpNodelay", 0));
			innerMap.add(EmaFactory.createMapEntry().keyAscii( "Channel_1", MapEntry.MapAction.ADD, innerElementList)); 
			innerElementList.clear();

			//encode Channel_2
			innerElementList.add(EmaFactory.createElementEntry().ascii("ChannelType", "ChannelType::RSSL_HTTP"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("InterfaceName", "localhost"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("Host", "localhost2"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("Port", "14008"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("ProxyHost", "proxyhost2"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("ProxyPort", "proxyport2"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("ObjectName", "objectname2"));
			innerElementList.add(EmaFactory.createElementEntry().intValue("TcpNodelay", 0));
			innerMap.add(EmaFactory.createMapEntry().keyAscii( "Channel_2", MapEntry.MapAction.ADD, innerElementList)); 
			innerElementList.clear();

			//encode Channel_3
			innerElementList.add(EmaFactory.createElementEntry().ascii("ChannelType", "ChannelType::RSSL_ENCRYPTED"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("InterfaceName", "localhost"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("Host", "localhost3"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("Port", "14009"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("ProxyHost", "proxyhost3"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("ProxyPort", "proxyport3"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("ObjectName", "objectname3"));
			innerElementList.add(EmaFactory.createElementEntry().intValue("TcpNodelay", 0));
			innerMap.add(EmaFactory.createMapEntry().keyAscii( "Channel_3", MapEntry.MapAction.ADD, innerElementList)); 
			innerElementList.clear();
			
			elementList.add(EmaFactory.createElementEntry().map("ChannelList", innerMap));
			innerMap.clear();

			outermostMap.add(EmaFactory.createMapEntry().keyAscii( "ChannelGroup", MapEntry.MapAction.ADD, elementList));
			elementList.clear();
			
			innerElementList.add(EmaFactory.createElementEntry().ascii("DictionaryType", "DictionaryType::FileDictionary"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("RdmFieldDictionaryFileName", "./RDMFieldDictionary"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("EnumTypeDefFileName", "./enumtype.def"));
			innerMap.add(EmaFactory.createMapEntry().keyAscii( "Dictionary_1", MapEntry.MapAction.ADD, innerElementList));
			innerElementList.clear();

			elementList.add(EmaFactory.createElementEntry().map("DictionaryList", innerMap));
			innerMap.clear();

			outermostMap.add(EmaFactory.createMapEntry().keyAscii( "DictionaryGroup", MapEntry.MapAction.ADD, elementList));
			elementList.clear();

			String localConfigPath = "./src/test/resources/com/rtsdk/ema/unittest/EmaFileConfigTests/EmaConfigTest.xml";
			System.out.println("Using Ema Config: " + localConfigPath);
			
			OmmProvider prov = JUnitTestConnect.createOmmNiProvider(EmaFactory.createOmmNiProviderConfig(localConfigPath).config(outermostMap));
			
			String defaultProvName = JUnitTestConnect.activeConfigGetStringValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.NiProviderName, -1);
			TestUtilities.checkResult("DefaultProvider value != null", defaultProvName != null);
			TestUtilities.checkResult("DefaultProvider value == Provider_3", defaultProvName.contentEquals("Provider_3") );
						
			// Check Channel configuration:
			if (testCase == 0)
			{
				System.out.println("\nRetrieving Channel_2 configuration values "); 
				String consChannelVal = JUnitTestConnect.activeConfigGetStringValue(prov, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelName, 0);
				TestUtilities.checkResult("Channel value != null", consChannelVal != null);
				TestUtilities.checkResult("Channel value == Channel_2", consChannelVal.contentEquals("Channel_2") );
				int channelConnType = JUnitTestConnect.activeConfigGetIntLongValue(prov, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelType, 0);
				TestUtilities.checkResult("ChannelConnType == ChannelType::RSSL_HTTP", channelConnType == ChannelTypeHttp);
				String strValue = JUnitTestConnect.activeConfigGetStringValue(prov, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.InterfaceName, 0);
				TestUtilities.checkResult("InterfaceName == localhost", strValue.contentEquals("localhost"));
				boolean boolValue = JUnitTestConnect.activeConfigGetBooleanValue(prov, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.TcpNodelay, 0);
				TestUtilities.checkResult("TcpNodelay == 0", boolValue == false);
				strValue = JUnitTestConnect.activeConfigGetStringValue(prov, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.Host, 0);
				TestUtilities.checkResult("Host == localhost2", strValue.contentEquals("localhost2"));
				strValue = JUnitTestConnect.activeConfigGetStringValue(prov, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.Port, 0);
				TestUtilities.checkResult("Port == 14008", strValue.contentEquals("14008"));
				strValue = JUnitTestConnect.activeConfigGetStringValue(prov, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ProxyHost, 0);
				TestUtilities.checkResult("ProxyHost == proxyhost2", strValue.contentEquals("proxyhost2"));
				strValue = JUnitTestConnect.activeConfigGetStringValue(prov, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ProxyPort, 0);
				TestUtilities.checkResult("ProxyPort == proxyport2", strValue.contentEquals("proxyport2"));
				strValue = JUnitTestConnect.activeConfigGetStringValue(prov, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ObjectName, 0);
				TestUtilities.checkResult("ObjectName == objectname2", strValue.contentEquals("objectname2"));
				
				System.out.println("\nRetrieving Channel_3 configuration values "); 
				consChannelVal = JUnitTestConnect.activeConfigGetStringValue(prov, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelName, 1);
				TestUtilities.checkResult("Channel value != null", consChannelVal != null);
				TestUtilities.checkResult("Channel value == Channel_3", consChannelVal.contentEquals("Channel_3") );
				channelConnType = JUnitTestConnect.activeConfigGetIntLongValue(prov, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelType, 1);
				TestUtilities.checkResult("ChannelConnType == ChannelType::RSSL_ENCRYPTED", channelConnType == ChannelTypeEncrypted);
				strValue = JUnitTestConnect.activeConfigGetStringValue(prov, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.InterfaceName, 1);
				TestUtilities.checkResult("InterfaceName == localhost", strValue.contentEquals("localhost"));
				boolValue = JUnitTestConnect.activeConfigGetBooleanValue(prov, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.TcpNodelay, 1);
				TestUtilities.checkResult("TcpNodelay == 0", boolValue == false);
				strValue = JUnitTestConnect.activeConfigGetStringValue(prov, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.Host, 1);
				TestUtilities.checkResult("Host == localhost3", strValue.contentEquals("localhost3"));
				strValue = JUnitTestConnect.activeConfigGetStringValue(prov, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.Port, 1);
				TestUtilities.checkResult("Port == 14009", strValue.contentEquals("14009"));
				strValue = JUnitTestConnect.activeConfigGetStringValue(prov, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ProxyHost, 1);
				TestUtilities.checkResult("ProxyHost == proxyhost3", strValue.contentEquals("proxyhost3"));
				strValue = JUnitTestConnect.activeConfigGetStringValue(prov, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ProxyPort, 1);
				TestUtilities.checkResult("ProxyPort == proxyport3", strValue.contentEquals("proxyport3"));
				strValue = JUnitTestConnect.activeConfigGetStringValue(prov, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ObjectName, 1);
				TestUtilities.checkResult("ObjectName == objectname3", strValue.contentEquals("objectname3"));
			}
			else if (testCase == 1)
			{
				System.out.println("\nRetrieving Channel_1 configuration values "); 
				String consChannelVal = JUnitTestConnect.activeConfigGetStringValue(prov, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelName, 0);
				TestUtilities.checkResult("Channel value != null", consChannelVal != null);
				TestUtilities.checkResult("Channel value == Channel_1", consChannelVal.contentEquals("Channel_1") );
				int channelConnType = JUnitTestConnect.activeConfigGetIntLongValue(prov, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelType, 0);
				TestUtilities.checkResult("ChannelConnType == ChannelType::RSSL_SOCKET", channelConnType == ChannelTypeSocket);
				String strValue = JUnitTestConnect.activeConfigGetStringValue(prov, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.InterfaceName, 0);
				TestUtilities.checkResult("InterfaceName == localhost", strValue.contentEquals("localhost"));
				boolean boolValue = JUnitTestConnect.activeConfigGetBooleanValue(prov, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.TcpNodelay, 0);
				TestUtilities.checkResult("TcpNodelay == 0", boolValue == false);
				boolValue = JUnitTestConnect.activeConfigGetBooleanValue(prov, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.DirectWrite, 0);
				strValue = JUnitTestConnect.activeConfigGetStringValue(prov, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.Host, 0);
				TestUtilities.checkResult("Host == localhost1", strValue.contentEquals("localhost1"));
				strValue = JUnitTestConnect.activeConfigGetStringValue(prov, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.Port, 0);
				TestUtilities.checkResult("Port == 14002", strValue.contentEquals("14002"));
			}
			
			//retrieve directory
			String strValue = JUnitTestConnect.activeConfigGetStringValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.DirectoryName, -1);
			TestUtilities.checkResult("DirectoryName == Directory_5", strValue.contentEquals("Directory_5"));
			
			List<Service> services = JUnitTestConnect.activeConfigGetService(prov, false);
			TestUtilities.checkResult("services.size() == 1", services.size() == 1);
			
			/*********retrieve first service *************/
			System.out.println("\nRetrieving DIRECT_FEED service configuration values "); 
			Service temp = services.get(0);
			TestUtilities.checkResult("temp != null", temp != null);
			
			TestUtilities.checkResult("checkHasInfo() == true", temp.checkHasInfo() == true);
			
			TestUtilities.checkResult("serviceId == 0", temp.serviceId() == 0);
			ServiceInfo info = temp.info();
			TestUtilities.checkResult("infoFilter.serviceName == NI_PUB", info.serviceName().toString().equals("NI_PUB"));
			TestUtilities.checkResult("infoFilter.acceptingConsumerStatus == 0", info.checkHasAcceptingConsumerStatus() && info.acceptingConsumerStatus() == 1);
			
			//retrieve capabilities
			List<Long> capabilities =  info.capabilitiesList();
			TestUtilities.checkResult("capabilities.size() == 4", capabilities.size() == 4);
			TestUtilities.checkResult("capabilities.get(0) == MMT_MARKET_PRICE", capabilities.get(0) == 6);
			TestUtilities.checkResult("capabilities.get(1) == MMT_MARKET_BY_ORDER", capabilities.get(1) == 7);
			TestUtilities.checkResult("capabilities.get(2) == MMT_MARKET_BY_PRICE", capabilities.get(2) == 8);
			TestUtilities.checkResult("capabilities.get(3) == MMT_MARKET_MAKER", capabilities.get(3) == 9);
			
			//retrieve qos
			TestUtilities.checkResult("info.checkHasQos()", info.checkHasQos());
			List<Qos> qos = info.qosList();
			TestUtilities.checkResult("qos.size() == 1", qos.size() == 1);
			TestUtilities.checkResult("qos.get(0).rate() == QosRates.TICK_BY_TICK", qos.get(0).rate() == QosRates.TICK_BY_TICK);
			TestUtilities.checkResult("qos.get(0).timeliness() == QosTimeliness.REALTIME", qos.get(0).timeliness() == QosTimeliness.REALTIME);
			
			//retrieve dictionary provided/used by this service
			TestUtilities.checkResult("info.checkHasDictionariesProvided() == false", !info.checkHasDictionariesProvided());
			TestUtilities.checkResult("info.checkHasDictionariesUsed() == true", info.checkHasDictionariesUsed());
			List<String> dictProvided = info.dictionariesProvidedList();
			TestUtilities.checkResult("dictProvided.size() == 0", dictProvided.size() == 0);
			List<String> dictUsed = info.dictionariesUsedList();
			TestUtilities.checkResult("dictUsed.size() == 2", dictUsed.size() == 2);
			TestUtilities.checkResult("info.dictionariesUsedList().get(0) == RWFFld",dictUsed.get(0).equals("RWFFld"));
			TestUtilities.checkResult("info.dictionariesUsedList().get(1) == RWFEnum",dictUsed.get(1).equals("RWFEnum"));
			
			prov = null;
		}
		catch ( OmmException excp)
		{
			System.out.println(excp.getMessage());
			TestUtilities.checkResult("Receiving exception, test failed.", false );
		}
	}
}

public void testLoadCfgFromProgrammaticConfigForIProv()
{
	TestUtilities.printTestHead("testLoadCfgFromProgrammaticConfigForIProv","Test loading all IProvider configuration parameters programmatically");
	
	//two testcases:
	//test case 0: NOT loading EmaConfig file from working dir.
	//test case 1: loading EmaConfigTest file
	for (int testCase = 0; testCase < 2; testCase++)
	{
		System.out.println(" #####Now it is running test case " + testCase);

		Map outermostMap = EmaFactory.createMap();
		Map innerMap = EmaFactory.createMap();
		ElementList elementList = EmaFactory.createElementList();
		ElementList innerElementList = EmaFactory.createElementList();
		try
		{
			innerElementList.add(EmaFactory.createElementEntry().intValue("ReactorMsgEventPoolLimit", 2000));
			innerElementList.add(EmaFactory.createElementEntry().intValue("ReactorChannelEventPoolLimit", 1500));
			innerElementList.add(EmaFactory.createElementEntry().intValue("WorkerEventPoolLimit", 1000));
			innerElementList.add(EmaFactory.createElementEntry().intValue("TunnelStreamMsgEventPoolLimit", 2500));
			innerElementList.add(EmaFactory.createElementEntry().intValue("TunnelStreamStatusEventPoolLimit", 3000));
			outermostMap.add(EmaFactory.createMapEntry().keyAscii( "GlobalConfig", MapEntry.MapAction.ADD, innerElementList ));
			innerElementList.clear();
			
			elementList.add(EmaFactory.createElementEntry().ascii("DefaultIProvider", "Provider_1"));

			innerElementList.add(EmaFactory.createElementEntry().ascii("Server", "Server_1"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("Directory", "Directory_1"));
			innerElementList.add(EmaFactory.createElementEntry().intValue("ItemCountHint", 5000));
			innerElementList.add(EmaFactory.createElementEntry().intValue("ServiceCountHint", 2000));
			innerElementList.add(EmaFactory.createElementEntry().intValue("AcceptDirMessageWithoutMinFilters", 1));
			innerElementList.add(EmaFactory.createElementEntry().intValue("AcceptMessageSameKeyButDiffStream", 1));
			innerElementList.add(EmaFactory.createElementEntry().intValue("AcceptMessageThatChangesService", 1));
			innerElementList.add(EmaFactory.createElementEntry().intValue("AcceptMessageWithoutAcceptingRequests", 1));
			innerElementList.add(EmaFactory.createElementEntry().intValue("AcceptMessageWithoutBeingLogin", 1));
			innerElementList.add(EmaFactory.createElementEntry().intValue("AcceptMessageWithoutQosInRange", 1));
			innerElementList.add(EmaFactory.createElementEntry().intValue("EnforceAckIDValidation", 1));
			innerElementList.add(EmaFactory.createElementEntry().intValue("FieldDictionaryFragmentSize", 2000));
			innerElementList.add(EmaFactory.createElementEntry().intValue("EnumTypeFragmentSize", 1000));
			innerElementList.add(EmaFactory.createElementEntry().intValue("RefreshFirstRequired", 0));
			innerElementList.add(EmaFactory.createElementEntry().intValue("RequestTimeout", 2400));
			innerElementList.add(EmaFactory.createElementEntry().intValue("DispatchTimeoutApiThread", 60));
			innerElementList.add(EmaFactory.createElementEntry().intValue("MaxDispatchCountApiThread", 300));
			innerElementList.add(EmaFactory.createElementEntry().intValue("MaxDispatchCountUserThread", 700));
			innerElementList.add(EmaFactory.createElementEntry().intValue("XmlTraceToStdout", 1));
			innerMap.add(EmaFactory.createMapEntry().keyAscii( "Provider_1", MapEntry.MapAction.ADD, innerElementList));
			innerElementList.clear();
			
			innerElementList.add(EmaFactory.createElementEntry().ascii("Server", "Server_2"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("Directory", "Directory_2"));
			innerMap.add(EmaFactory.createMapEntry().keyAscii( "Provider_2", MapEntry.MapAction.ADD, innerElementList)); 
			innerElementList.clear();
					
			elementList.add(EmaFactory.createElementEntry().map("IProviderList", innerMap));
			innerMap.clear();

			outermostMap.add(EmaFactory.createMapEntry().keyAscii( "IProviderGroup", MapEntry.MapAction.ADD, elementList));
			elementList.clear();
			
			innerElementList.add(EmaFactory.createElementEntry().ascii("ServerType", "ServerType::RSSL_SOCKET"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("CompressionType", "CompressionType::ZLib"));
			innerElementList.add(EmaFactory.createElementEntry().intValue("GuaranteedOutputBuffers", 8000));
			innerElementList.add(EmaFactory.createElementEntry().intValue("NumInputBuffers", 7777));
			innerElementList.add(EmaFactory.createElementEntry().intValue("SysRecvBufSize", 150000));
			innerElementList.add(EmaFactory.createElementEntry().intValue("SysSendBufSize", 200000));
			innerElementList.add(EmaFactory.createElementEntry().intValue("CompressionThreshold", 12856));
			innerElementList.add(EmaFactory.createElementEntry().intValue("ConnectionPingTimeout", 30000));
			innerElementList.add(EmaFactory.createElementEntry().intValue("ConnectionMinPingTimeout", 8000));
			innerElementList.add(EmaFactory.createElementEntry().ascii("InterfaceName", "localhost"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("Port", "14010"));
			innerElementList.add(EmaFactory.createElementEntry().intValue("TcpNodelay", 0));
			innerElementList.add(EmaFactory.createElementEntry().intValue("DirectWrite", 1));
			innerElementList.add(EmaFactory.createElementEntry().intValue("HighWaterMark", 5000));
			innerElementList.add(EmaFactory.createElementEntry().intValue("InitializationTimeout", 100));
			innerMap.add(EmaFactory.createMapEntry().keyAscii( "Server_1", MapEntry.MapAction.ADD, innerElementList)); 
			innerElementList.clear();

			innerElementList.add(EmaFactory.createElementEntry().ascii("ServerType", "ServerType::RSSL_SOCKET"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("Port", "14011"));
			innerMap.add(EmaFactory.createMapEntry().keyAscii( "Server_2", MapEntry.MapAction.ADD, innerElementList)); 
			innerElementList.clear();

			elementList.add(EmaFactory.createElementEntry().map("ServerList", innerMap));
			innerMap.clear();

			outermostMap.add(EmaFactory.createMapEntry().keyAscii( "ServerGroup", MapEntry.MapAction.ADD, elementList));
			elementList.clear();

			innerElementList.add(EmaFactory.createElementEntry().ascii("DictionaryType", "DictionaryType::FileDictionary"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("RdmFieldDictionaryItemName", "RWFFld"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("EnumTypeDefItemName", "RWFEnum"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("RdmFieldDictionaryFileName", "./RDMFieldDictionary"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("EnumTypeDefFileName", "./enumtype.def"));
			innerMap.add(EmaFactory.createMapEntry().keyAscii( "Dictionary_3", MapEntry.MapAction.ADD, innerElementList));
			innerElementList.clear();
			
			innerElementList.add(EmaFactory.createElementEntry().ascii("DictionaryType", "DictionaryType::FileDictionary"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("RdmFieldDictionaryItemName", "RWFFld_ID4"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("EnumTypeDefItemName", "RWFEnum_ID4"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("RdmFieldDictionaryFileName", "./RDMFieldDictionary_ID4"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("EnumTypeDefFileName", "./enumtype_ID4.def"));
			innerMap.add(EmaFactory.createMapEntry().keyAscii( "Dictionary_4", MapEntry.MapAction.ADD, innerElementList));
			innerElementList.clear();

			elementList.add(EmaFactory.createElementEntry().map("DictionaryList", innerMap));
			innerMap.clear();

			outermostMap.add(EmaFactory.createMapEntry().keyAscii( "DictionaryGroup", MapEntry.MapAction.ADD, elementList));
			elementList.clear();

			/////////////////////////////////////
			//DirectoryGroup
			Map serviceMap = EmaFactory.createMap();
			ElementList infoElementList = EmaFactory.createElementList();
			ElementList stateElementList = EmaFactory.createElementList();
			OmmArray infoArray = EmaFactory.createOmmArray();
			Series qosSeries = EmaFactory.createSeries();

			//encode service1
			infoElementList.add(EmaFactory.createElementEntry().intValue("ServiceId", 3));
			infoElementList.add(EmaFactory.createElementEntry().ascii("Vendor", "company name"));
			infoElementList.add(EmaFactory.createElementEntry().intValue("IsSource", 0));
			infoElementList.add(EmaFactory.createElementEntry().intValue("AcceptingConsumerStatus", 0));
			infoElementList.add(EmaFactory.createElementEntry().intValue("SupportsQoSRange", 0));
			infoElementList.add(EmaFactory.createElementEntry().intValue("SupportsOutOfBandSnapshots", 0));
			infoElementList.add(EmaFactory.createElementEntry().ascii("ItemList", "#.itemlist"));

			infoArray.add(EmaFactory.createOmmArrayEntry().ascii("MMT_DICTIONARY"));
			infoArray.add(EmaFactory.createOmmArrayEntry().ascii("MMT_MARKET_PRICE"));
			infoArray.add(EmaFactory.createOmmArrayEntry().ascii("MMT_MARKET_BY_ORDER"));
			infoArray.add(EmaFactory.createOmmArrayEntry().ascii("MMT_MARKET_BY_PRICE"));
			infoArray.add(EmaFactory.createOmmArrayEntry().ascii("200"));
			infoElementList.add(EmaFactory.createElementEntry().array("Capabilities", infoArray));
			infoArray.clear();
			
			infoArray.add(EmaFactory.createOmmArrayEntry().ascii("Dictionary_3"));
			infoElementList.add(EmaFactory.createElementEntry().array("DictionariesProvided", infoArray));
			infoArray.clear();
			infoArray.add(EmaFactory.createOmmArrayEntry().ascii("Dictionary_4"));
			infoElementList.add(EmaFactory.createElementEntry().array("DictionariesUsed", infoArray));
			infoArray.clear();
			
			innerElementList.add(EmaFactory.createElementEntry().ascii("Timeliness", "Timeliness::RealTime"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("Rate", "Rate::TickByTick"));
			qosSeries.add(EmaFactory.createSeriesEntry().elementList(innerElementList));
			innerElementList.clear();
			
			innerElementList.add(EmaFactory.createElementEntry().intValue("Timeliness", 100));
			innerElementList.add(EmaFactory.createElementEntry().intValue("Rate", 100));
			qosSeries.add(EmaFactory.createSeriesEntry().elementList(innerElementList));
			innerElementList.clear();
			infoElementList.add(EmaFactory.createElementEntry().series("QoS", qosSeries));
			qosSeries.clear();
			
			stateElementList.add(EmaFactory.createElementEntry().intValue("ServiceState", 1));
			stateElementList.add(EmaFactory.createElementEntry().intValue("AcceptingRequests", 1));
			
			innerElementList.add(EmaFactory.createElementEntry().ascii("StreamState", "StreamState::CloseRecover"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("DataState", "DataState::Suspect"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("StatusCode", "StatusCode::DacsDown"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("StatusText", "dacsDown"));
			stateElementList.add(EmaFactory.createElementEntry().elementList("Status", innerElementList));
			innerElementList.clear();
						
			innerElementList.add(EmaFactory.createElementEntry().elementList("InfoFilter", infoElementList));
			infoElementList.clear();
			innerElementList.add(EmaFactory.createElementEntry().elementList("StateFilter", stateElementList));
			stateElementList.clear();
			
			serviceMap.add(EmaFactory.createMapEntry().keyAscii( "DIRECT_FEED", MapEntry.MapAction.ADD, innerElementList));
			innerElementList.clear();
			
			//encode service2
			infoElementList.add(EmaFactory.createElementEntry().intValue("ServiceId", 4));
			infoElementList.add(EmaFactory.createElementEntry().ascii("Vendor", "company name"));
			infoElementList.add(EmaFactory.createElementEntry().intValue("AcceptingConsumerStatus", 0));
			infoElementList.add(EmaFactory.createElementEntry().intValue("SupportsQoSRange", 0));
			infoElementList.add(EmaFactory.createElementEntry().intValue("SupportsOutOfBandSnapshots", 0));
			infoElementList.add(EmaFactory.createElementEntry().ascii("ItemList", "#.itemlist"));

			infoArray.add(EmaFactory.createOmmArrayEntry().ascii("MMT_DICTIONARY"));
			infoArray.add(EmaFactory.createOmmArrayEntry().ascii("MMT_MARKET_PRICE"));
			infoArray.add(EmaFactory.createOmmArrayEntry().ascii("MMT_MARKET_BY_ORDER"));
			infoArray.add(EmaFactory.createOmmArrayEntry().ascii("200"));
			infoElementList.add(EmaFactory.createElementEntry().array("Capabilities", infoArray));
			infoArray.clear();
			
			infoArray.add(EmaFactory.createOmmArrayEntry().ascii("Dictionary_6"));
			infoElementList.add(EmaFactory.createElementEntry().array("DictionariesUsed", infoArray));
			infoArray.clear();
			infoArray.add(EmaFactory.createOmmArrayEntry().ascii("Dictionary_2"));
			infoElementList.add(EmaFactory.createElementEntry().array("DictionariesProvided", infoArray));
			infoArray.clear();
			
			stateElementList.add(EmaFactory.createElementEntry().intValue("ServiceState", 1));
			stateElementList.add(EmaFactory.createElementEntry().intValue("AcceptingRequests", 1));

			innerElementList.add(EmaFactory.createElementEntry().elementList("InfoFilter", infoElementList));
			infoElementList.clear();
			innerElementList.add(EmaFactory.createElementEntry().elementList("StateFilter", stateElementList));
			stateElementList.clear();
			
			serviceMap.add(EmaFactory.createMapEntry().keyAscii( "DIRECT_FEED1", MapEntry.MapAction.ADD, innerElementList));
			innerElementList.clear();
			
			innerMap.add(EmaFactory.createMapEntry().keyAscii( "Directory_1", MapEntry.MapAction.ADD, serviceMap));
			serviceMap.clear();

			elementList.add(EmaFactory.createElementEntry().ascii("DefaultDirectory", "Directory_1"));
			elementList.add(EmaFactory.createElementEntry().map("DirectoryList", innerMap));
			innerMap.clear();
			
			outermostMap.add(EmaFactory.createMapEntry().keyAscii( "DirectoryGroup", MapEntry.MapAction.ADD, elementList));
			elementList.clear();

			String localConfigPath = null;
			if (testCase == 1)
				localConfigPath = "./src/test/resources/com/rtsdk/ema/unittest/EmaFileConfigTests/EmaConfigTest.xml";

			System.out.println("Using Ema Config: " + localConfigPath);
			
			OmmIProviderImpl prov = null;
			if (testCase == 0)
				prov = (OmmIProviderImpl) JUnitTestConnect.createOmmIProvider(EmaFactory.createOmmIProviderConfig().config(outermostMap));
			else if (testCase == 1)
				prov = (OmmIProviderImpl) JUnitTestConnect.createOmmIProvider(EmaFactory.createOmmIProviderConfig(localConfigPath).config(outermostMap));
			
			String defaultProvName = JUnitTestConnect.activeConfigGetStringValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.IProviderName);
			TestUtilities.checkResult("DefaultProvider value != null", defaultProvName != null);
			TestUtilities.checkResult("DefaultProvider value == Provider_1", defaultProvName.contentEquals("Provider_1") );
			String provServerVal = JUnitTestConnect.activeConfigGetStringValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.ServerName);
			TestUtilities.checkResult("Server value != null", provServerVal != null);
			TestUtilities.checkResult("Server value == Server_1", provServerVal.contentEquals("Server_1") );

			boolean boolValue = JUnitTestConnect.activeConfigGetBooleanValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.IProviderAcceptDirMessageWithoutMinFilters);
			TestUtilities.checkResult("AcceptDirMessageWithoutMinFilters == 1", boolValue == true);
			boolValue = JUnitTestConnect.activeConfigGetBooleanValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.IProviderAcceptMessageWithoutAcceptingRequests);
			TestUtilities.checkResult("AcceptMessageWithoutAcceptingRequests == 1", boolValue == true);
			boolValue = JUnitTestConnect.activeConfigGetBooleanValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.IProviderAcceptMessageWithoutBeingLogin);
			TestUtilities.checkResult("AcceptMessageWithoutBeingLogin == 1", boolValue == true);
			boolValue = JUnitTestConnect.activeConfigGetBooleanValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.IProviderAcceptMessageWithoutQosInRange);
			TestUtilities.checkResult("AcceptMessageWithoutQosInRange == 1", boolValue == true);
			boolValue = JUnitTestConnect.activeConfigGetBooleanValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.IProviderAcceptMessageSameKeyButDiffStream);
			TestUtilities.checkResult("AcceptMessageSameKeyButDiffStream == 1", boolValue == true);
			boolValue = JUnitTestConnect.activeConfigGetBooleanValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.IProviderAcceptMessageThatChangesService);
			TestUtilities.checkResult("AcceptMessageThatChangesService == 1", boolValue == true);
			boolValue = JUnitTestConnect.activeConfigGetBooleanValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.IProviderEnforceAckIDValidation);
			TestUtilities.checkResult("IProviderEnforceAckIDValidation == 1", boolValue == true);
			boolValue = JUnitTestConnect.activeConfigGetBooleanValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.IProviderRefreshFirstRequired);
			TestUtilities.checkResult("RefreshFirstRequired == 0", boolValue == false);
			int intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.DictionaryFieldDictFragmentSize);
			TestUtilities.checkResult("MaxFieldDictFragmentSize value == 2000", intLongValue == 2000 );
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.DictionaryEnumTypeFragmentSize);
			TestUtilities.checkResult("MaxEnumTypeFragmentSize value == 1000", intLongValue == 1000 );
			
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.ItemCountHint);
			TestUtilities.checkResult("ItemCountHint value == 5000", intLongValue == 5000 );
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.ServiceCountHint);
			TestUtilities.checkResult("ServiceCountHint value == 2000", intLongValue == 2000 );
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.RequestTimeout);
			TestUtilities.checkResult("RequestTimeout value == 2400", intLongValue == 2400 );
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.DispatchTimeoutApiThread);
			TestUtilities.checkResult("DispatchTimeoutApiThread value == 60", intLongValue == 60 );

			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.MaxDispatchCountApiThread);
			TestUtilities.checkResult("MaxDispatchCountApiThread value == 300", intLongValue == 300 );
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.MaxDispatchCountUserThread);
			TestUtilities.checkResult("MaxDispatchCountUserThread value == 700", intLongValue == 700 );

			boolValue = JUnitTestConnect.activeConfigGetBooleanValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.XmlTraceToStdout);
			TestUtilities.checkResult("XmlTraceToStdout == 1", boolValue == true);

			// Check Global configuration:
			int value = prov.activeConfig().globalConfig.reactorMsgEventPoolLimit;
			TestUtilities.checkResult("ReactorMsgEventPoolLimit ==  2000", value == 2000);
			value = prov.activeConfig().globalConfig.reactorChannelEventPoolLimit;
			TestUtilities.checkResult("ReactorChannelEventPoolLimit == 1500", value == 1500);
			value = prov.activeConfig().globalConfig.workerEventPoolLimit;
			TestUtilities.checkResult("WorkerEventPoolLimit == 1000", value == 1000);
			value = prov.activeConfig().globalConfig.tunnelStreamMsgEventPoolLimit;
			TestUtilities.checkResult("TunnelStreamMsgEventPoolLimit == 2500", value == 2500);
			value = prov.activeConfig().globalConfig.tunnelStreamStatusEventPoolLimit;
			TestUtilities.checkResult("TunnelStreamStatusEventPoolLimit == 3000", value == 3000);
			
			// Check Server configuration:
			// Check Server_1 configuration.
			provServerVal = "Server_1";
			System.out.println("\nRetrieving Server_1 configuration values "); 
			int serverConnType = JUnitTestConnect.activeConfigGetIntLongValue(prov, JUnitTestConnect.ConfigGroupTypeServer, JUnitTestConnect.ServerType);
			TestUtilities.checkResult("serverConnType == ServerType::RSSL_SOCKET", serverConnType == ChannelTypeSocket);
		
			String strValue = JUnitTestConnect.activeConfigGetStringValue(prov, JUnitTestConnect.ConfigGroupTypeServer, JUnitTestConnect.InterfaceName);
			TestUtilities.checkResult("InterfaceName == localhost", strValue.contentEquals("localhost"));
			
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(prov, JUnitTestConnect.ConfigGroupTypeServer, JUnitTestConnect.CompressionType);
			TestUtilities.checkResult("CompressionType == CompressionType::ZLIB", intLongValue == CompressionTypeZLib);
			
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(prov, JUnitTestConnect.ConfigGroupTypeServer, JUnitTestConnect.GuaranteedOutputBuffers);
			TestUtilities.checkResult("GuaranteedOutputBuffers == 8000", intLongValue == 8000);
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(prov, JUnitTestConnect.ConfigGroupTypeServer, JUnitTestConnect.NumInputBuffers);
			TestUtilities.checkResult("NumInputBuffers == 7777", intLongValue == 7777);
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(prov, JUnitTestConnect.ConfigGroupTypeServer, JUnitTestConnect.SysRecvBufSize);
			TestUtilities.checkResult("SysRecvBufSize == 150000", intLongValue == 150000);
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(prov, JUnitTestConnect.ConfigGroupTypeServer, JUnitTestConnect.SysSendBufSize);
			TestUtilities.checkResult("SysSendBufSize == 200000", intLongValue == 200000);
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(prov, JUnitTestConnect.ConfigGroupTypeServer, JUnitTestConnect.CompressionThreshold);
			TestUtilities.checkResult("CompressionThreshold == 12856", intLongValue == 12856);
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(prov, JUnitTestConnect.ConfigGroupTypeServer, JUnitTestConnect.ConnectionPingTimeout);
			TestUtilities.checkResult("ConnectionPingTimeout == 30000", intLongValue == 30000);
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(prov, JUnitTestConnect.ConfigGroupTypeServer, JUnitTestConnect.ConnectionMinPingTimeout);
			TestUtilities.checkResult("ConnectionMinPingTimeout == 8000", intLongValue == 8000);
			boolValue = JUnitTestConnect.activeConfigGetBooleanValue(prov, JUnitTestConnect.ConfigGroupTypeServer, JUnitTestConnect.TcpNodelay);
			TestUtilities.checkResult("TcpNodelay == 0", boolValue == false);
			boolValue = JUnitTestConnect.activeConfigGetBooleanValue(prov, JUnitTestConnect.ConfigGroupTypeServer, JUnitTestConnect.DirectWrite);
			TestUtilities.checkResult("DirectWrite == 1", boolValue == true);
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(prov, JUnitTestConnect.ConfigGroupTypeServer, JUnitTestConnect.HighWaterMark);
			TestUtilities.checkResult("HighWaterMark == 5000", intLongValue == 5000);
			String chanPort = JUnitTestConnect.activeConfigGetStringValue(prov, JUnitTestConnect.ConfigGroupTypeServer, JUnitTestConnect.Port);
			TestUtilities.checkResult("Port == 14010", chanPort.contentEquals("14010"));
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(prov, JUnitTestConnect.ConfigGroupTypeServer, JUnitTestConnect.ServerInitTimeout);
			TestUtilities.checkResult("InitializationTimeout value == 100", intLongValue == 100 );
			
			//retrieve directory
			strValue = JUnitTestConnect.activeConfigGetStringValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.DirectoryName);
			TestUtilities.checkResult("DirectoryName == Directory_1", strValue.contentEquals("Directory_1"));
			
			List<Service> services = JUnitTestConnect.activeConfigGetService(prov, true);
			TestUtilities.checkResult("services.size() == 2", services.size() == 2);
			
			/*********retrieve first service *************/
			System.out.println("\nRetrieving DIRECT_FEED service configuration values "); 
			Service temp = services.get(0);
			TestUtilities.checkResult("temp != null", temp != null);
			
			TestUtilities.checkResult("checkHasInfo() == true", temp.checkHasInfo() == true);
			
			TestUtilities.checkResult("serviceId == 3", temp.serviceId() == 3);
			ServiceInfo info = temp.info();
			TestUtilities.checkResult("infoFilter.serviceName == DIRECT_FEED", info.serviceName().toString().equals("DIRECT_FEED"));
			TestUtilities.checkResult("infoFilter.vendorName == company name", info.checkHasVendor() && info.vendor().toString().equals("company name"));
			TestUtilities.checkResult("infoFilter.isSource == false", info.checkHasIsSource() && info.isSource() == 0);
			TestUtilities.checkResult("infoFilter.itemList == #.itemlist", info.checkHasItemList() && info.itemList().toString().equals("#.itemlist"));
			TestUtilities.checkResult("infoFilter.acceptingConsumerStatus == 0", info.checkHasAcceptingConsumerStatus() && info.acceptingConsumerStatus() == 0);
			TestUtilities.checkResult("infoFilter.supportsQosRange == 0", info.checkHasSupportsQosRange() && info.supportsQosRange() == 0);
			TestUtilities.checkResult("infoFilter.supportsOutOfBandSnapshots == 0", info.checkHasSupportsOutOfBandSnapshots() && info.supportsOutOfBandSnapshots() == 0);
			
			//retrieve capabilities
			List<Long> capabilities =  info.capabilitiesList();
			TestUtilities.checkResult("capabilities.size() == 5", capabilities.size() == 5);
			TestUtilities.checkResult("capabilities.get(0) == MMT_DICTIONARY", capabilities.get(0) == 5);
			TestUtilities.checkResult("capabilities.get(1) == MMT_MARKET_PRICE", capabilities.get(1) == 6);
			TestUtilities.checkResult("capabilities.get(2) == MMT_MARKET_BY_ORDER", capabilities.get(2) == 7);
			TestUtilities.checkResult("capabilities.get(3) == MMT_MARKET_BY_PRICE", capabilities.get(3) == 8);
			TestUtilities.checkResult("capabilities.get(4) == 200", capabilities.get(4) == 200);
			
			//retrieve qos
			TestUtilities.checkResult("info.checkHasQos()", info.checkHasQos());
			List<Qos> qos = info.qosList();
			TestUtilities.checkResult("qos.size() == 2", qos.size() == 2);
			TestUtilities.checkResult("qos.get(0).rate() == QosRates.TICK_BY_TICK", qos.get(0).rate() == QosRates.TICK_BY_TICK);
			TestUtilities.checkResult("qos.get(0).timeliness() == QosTimeliness.REALTIME", qos.get(0).timeliness() == QosTimeliness.REALTIME);
			TestUtilities.checkResult("qos.get(1).rate() == QosRates.TIME_CONFLATED", qos.get(1).rate() == QosRates.TIME_CONFLATED);
			TestUtilities.checkResult("qos.get(1).rateInfo() == 100", qos.get(1).rateInfo() == 100);
			TestUtilities.checkResult("qos.get(1).timeliness() == QosTimeliness.DELAYED", qos.get(1).timeliness() == QosTimeliness.DELAYED);
			TestUtilities.checkResult("qos.get(1).timeInfo() == 100", qos.get(1).timeInfo() == 100);
			
			//retrieve dictionary provided/used by this service
			TestUtilities.checkResult("info.checkHasDictionariesProvided() == true", info.checkHasDictionariesProvided());
			TestUtilities.checkResult("info.checkHasDictionariesUsed() == true", info.checkHasDictionariesUsed());
			List<String> dictProvided = info.dictionariesProvidedList();
			TestUtilities.checkResult("dictProvided.size() == 2", dictProvided.size() == 2);
			TestUtilities.checkResult("info.dictionariesProvidedList().get(0) == RWFFld",dictProvided.get(0).equals("RWFFld"));
			TestUtilities.checkResult("info.dictionariesProvidedList().get(1) == RWFEnum",dictProvided.get(1).equals("RWFEnum"));
			List<String> dictUsed = info.dictionariesUsedList();
			TestUtilities.checkResult("dictUsed.size() == 2", dictUsed.size() == 2);
			TestUtilities.checkResult("info.dictionariesUsedList().get(0) == RWFFld_ID4",dictUsed.get(0).equals("RWFFld_ID4"));
			TestUtilities.checkResult("info.dictionariesUsedList().get(1) == RWFEnum_ID4",dictUsed.get(1).equals("RWFEnum_ID4"));
			
			//retrieve dictionary defined in directory
			int serviceId = 3;
			int dictId = 0;
			strValue = JUnitTestConnect.activeConfigGetServiceDict(prov,JUnitTestConnect.DictionaryType, serviceId, dictId, true);
			TestUtilities.checkResult("dictProvided DictionaryType = Dictionary::FileDictionary",strValue != null && strValue.equals("FileDictionary"));
			strValue = JUnitTestConnect.activeConfigGetServiceDict(prov,JUnitTestConnect.DictionaryName, serviceId, dictId, true);
			TestUtilities.checkResult("dictProvided DictionaryName = Dictionary_3",strValue != null && strValue.equals("Dictionary_3"));
			strValue = JUnitTestConnect.activeConfigGetServiceDict(prov,JUnitTestConnect.DictionaryRDMFieldDictFileName, serviceId, dictId, true);
			TestUtilities.checkResult("dictProvided DictionaryRDMFieldDictFileName = ./RDMFieldDictionary",strValue != null && strValue.equals("./RDMFieldDictionary"));
			strValue = JUnitTestConnect.activeConfigGetServiceDict(prov,JUnitTestConnect.DictionaryEnumTypeDefFileName, serviceId, dictId, true);
			TestUtilities.checkResult("dictProvided DictionaryEnumTypeDefFileName = ./enumtype.def",strValue != null && strValue.equals("./enumtype.def"));
			strValue = JUnitTestConnect.activeConfigGetServiceDict(prov,JUnitTestConnect.DictionaryRdmFieldDictionaryItemName, serviceId, dictId, true);
			TestUtilities.checkResult("dictProvided DictionaryRdmFieldDictionaryItemName = RWFFld",strValue != null && strValue.equals("RWFFld"));
			strValue = JUnitTestConnect.activeConfigGetServiceDict(prov,JUnitTestConnect.DictionaryEnumTypeDefItemName, serviceId, dictId, true);
			TestUtilities.checkResult("dictProvided DictionaryEnumTypeDefItemName = RWFEnum",strValue != null && strValue.equals("RWFEnum"));
			
			strValue = JUnitTestConnect.activeConfigGetServiceDict(prov,JUnitTestConnect.DictionaryType, serviceId, dictId, false);
			TestUtilities.checkResult("dictUsed DictionaryType = Dictionary::FileDictionary",strValue != null && strValue.equals("FileDictionary"));
			strValue = JUnitTestConnect.activeConfigGetServiceDict(prov,JUnitTestConnect.DictionaryName, serviceId, dictId, false);
			TestUtilities.checkResult("dictUsed DictionaryName = Dictionary_4",strValue != null && strValue.equals("Dictionary_4"));
			strValue = JUnitTestConnect.activeConfigGetServiceDict(prov,JUnitTestConnect.DictionaryRDMFieldDictFileName, serviceId, dictId, false);
			TestUtilities.checkResult("dictUsed DictionaryRDMFieldDictFileName = ./RDMFieldDictionary_ID4",strValue != null && strValue.equals("./RDMFieldDictionary_ID4"));
			strValue = JUnitTestConnect.activeConfigGetServiceDict(prov,JUnitTestConnect.DictionaryEnumTypeDefFileName, serviceId, dictId, false);
			TestUtilities.checkResult("dictUsed DictionaryEnumTypeDefFileName = ./enumtype_ID4.def",strValue != null && strValue.equals("./enumtype_ID4.def"));
			strValue = JUnitTestConnect.activeConfigGetServiceDict(prov,JUnitTestConnect.DictionaryRdmFieldDictionaryItemName, serviceId, dictId, false);
			TestUtilities.checkResult("dictUsed DictionaryRdmFieldDictionaryItemName = RDMFieldDictionary_ID4",strValue != null && strValue.equals("RWFFld_ID4"));
			strValue = JUnitTestConnect.activeConfigGetServiceDict(prov,JUnitTestConnect.DictionaryEnumTypeDefItemName, serviceId, dictId, false);
			TestUtilities.checkResult("dictUsed DictionaryEnumTypeDefItemName = RWFEnum_ID4",strValue != null && strValue.equals("RWFEnum_ID4"));
			
			//retrieve Service state
			TestUtilities.checkResult("checkHasState() == true", temp.checkHasState() == true);
			ServiceState state = temp.state();
			TestUtilities.checkResult("state.checkHasAcceptingRequests() == true", state.checkHasAcceptingRequests());
			TestUtilities.checkResult("state.acceptingRequests() == 1", state.acceptingRequests() == 1);
			TestUtilities.checkResult("state.serviceState() == 1", state.serviceState() == 1);
			TestUtilities.checkResult("checkHasStatus() = true", state.checkHasStatus());
			TestUtilities.checkResult("state.status().streamState() == 3", state.status().streamState() == 3);
			TestUtilities.checkResult("state.status().dataState() == 2", state.status().dataState() == 2);
			TestUtilities.checkResult("state.status().code() == 29", state.status().code() == 29);
			TestUtilities.checkResult("state.status().text() == dacsDown", state.status().text() != null &&  state.status().text().toString().equals("dacsDown"));

			/*********retrieve second service *************/
			System.out.println("\nRetrieving DIRECT_FEED1 service configuration values "); 
			temp = services.get(1);
			TestUtilities.checkResult("temp != null", temp != null);
			
			TestUtilities.checkResult("checkHasInfo() == true", temp.checkHasInfo() == true);
			
			TestUtilities.checkResult("serviceId == 4", temp.serviceId() == 4);
			info = temp.info();
			TestUtilities.checkResult("infoFilter.serviceName == DIRECT_FEED1", info.serviceName().toString().equals("DIRECT_FEED1"));
			TestUtilities.checkResult("infoFilter.vendorName == company name", info.checkHasVendor() && info.vendor().toString().equals("company name"));
			TestUtilities.checkResult("infoFilter.isSource == false", info.checkHasIsSource() == false);
			TestUtilities.checkResult("infoFilter.itemList == #.itemlist", info.checkHasItemList() && info.itemList().toString().equals("#.itemlist"));
			TestUtilities.checkResult("infoFilter.acceptingConsumerStatus == 0", info.checkHasAcceptingConsumerStatus() && info.acceptingConsumerStatus() == 0);
			TestUtilities.checkResult("infoFilter.supportsQosRange == 0", info.checkHasSupportsQosRange() && info.supportsQosRange() == 0);
			TestUtilities.checkResult("infoFilter.supportsOutOfBandSnapshots == 0", info.checkHasSupportsOutOfBandSnapshots() && info.supportsOutOfBandSnapshots() == 0);
			
			//retrieve capabilities
			capabilities =  info.capabilitiesList();
			TestUtilities.checkResult("capabilities.size() == 4", capabilities.size() == 4);
			TestUtilities.checkResult("capabilities.get(0) == MMT_DICTIONARY", capabilities.get(0) == 5);
			TestUtilities.checkResult("capabilities.get(1) == MMT_MARKET_PRICE", capabilities.get(1) == 6);
			TestUtilities.checkResult("capabilities.get(2) == MMT_MARKET_BY_ORDER", capabilities.get(2) == 7);
			TestUtilities.checkResult("capabilities.get(3) == 200", capabilities.get(3) == 200);
			
			//retrieve qos
			TestUtilities.checkResult("info.checkHasQos()", info.checkHasQos());
			qos = info.qosList();
			TestUtilities.checkResult("qos.size() == 1", qos.size() == 1);
			TestUtilities.checkResult("qos.get(0).rate() == QosRates.TICK_BY_TICK", qos.get(0).rate() == QosRates.TICK_BY_TICK);
			TestUtilities.checkResult("qos.get(0).timeliness() == QosTimeliness.REALTIME", qos.get(0).timeliness() == QosTimeliness.REALTIME);
			
			//retrieve dictionary provided/used by this service
			TestUtilities.checkResult("info.checkHasDictionariesProvided() == true", info.checkHasDictionariesProvided());
			TestUtilities.checkResult("info.checkHasDictionariesUsed() == true", info.checkHasDictionariesUsed());
			dictProvided = info.dictionariesProvidedList();
			TestUtilities.checkResult("dictProvided.size() == 2", dictProvided.size() == 2);
			TestUtilities.checkResult("info.dictionariesProvidedList().get(0) == RWFFld",dictProvided.get(0).equals("RWFFld"));
			TestUtilities.checkResult("info.dictionariesProvidedList().get(1) == RWFEnum",dictProvided.get(1).equals("RWFEnum"));
			dictUsed = info.dictionariesUsedList();
			TestUtilities.checkResult("dictUsed.size() == 2", dictUsed.size() == 2);
			TestUtilities.checkResult("info.dictionariesUsedList().get(0) == RWFFld",dictUsed.get(0).equals("RWFFld"));
			TestUtilities.checkResult("info.dictionariesUsedList().get(1) == RWFEnum",dictUsed.get(1).equals("RWFEnum"));
			
			//retrieve dictionary defined in directory
			serviceId = 4;
			dictId = 0;
			strValue = JUnitTestConnect.activeConfigGetServiceDict(prov,JUnitTestConnect.DictionaryType, serviceId, dictId, true);
			TestUtilities.checkResult("dictProvided DictionaryType = Dictionary::FileDictionary",strValue != null && strValue.equals("FileDictionary"));
			strValue = JUnitTestConnect.activeConfigGetServiceDict(prov,JUnitTestConnect.DictionaryName, serviceId, dictId, true);
			TestUtilities.checkResult("dictProvided DictionaryName = Dictionary_2",strValue != null && strValue.equals("Dictionary_2"));
			strValue = JUnitTestConnect.activeConfigGetServiceDict(prov,JUnitTestConnect.DictionaryRDMFieldDictFileName, serviceId, dictId, true);
			TestUtilities.checkResult("dictProvided DictionaryRDMFieldDictFileName = ./RDMFieldDictionary",strValue != null && strValue.equals("./RDMFieldDictionary"));
			strValue = JUnitTestConnect.activeConfigGetServiceDict(prov,JUnitTestConnect.DictionaryEnumTypeDefFileName, serviceId, dictId, true);
			TestUtilities.checkResult("dictProvided DictionaryEnumTypeDefFileName = ./enumtype.def",strValue != null && strValue.equals("./enumtype.def"));
			strValue = JUnitTestConnect.activeConfigGetServiceDict(prov,JUnitTestConnect.DictionaryRdmFieldDictionaryItemName, serviceId, dictId, true);
			TestUtilities.checkResult("dictProvided DictionaryRdmFieldDictionaryItemName = RWFFld",strValue != null && strValue.equals("RWFFld"));
			strValue = JUnitTestConnect.activeConfigGetServiceDict(prov,JUnitTestConnect.DictionaryEnumTypeDefItemName, serviceId, dictId, true);
			TestUtilities.checkResult("dictProvided DictionaryEnumTypeDefItemName = RWFEnum",strValue != null && strValue.equals("RWFEnum"));
			
			strValue = JUnitTestConnect.activeConfigGetServiceDict(prov,JUnitTestConnect.DictionaryType, serviceId, dictId, false);
			TestUtilities.checkResult("dictUsed DictionaryType = Dictionary::FileDictionary",strValue != null && strValue.equals("FileDictionary"));
			strValue = JUnitTestConnect.activeConfigGetServiceDict(prov,JUnitTestConnect.DictionaryName, serviceId, dictId, false);
			TestUtilities.checkResult("dictUsed DictionaryName = Dictionary_6",strValue != null && strValue.equals("Dictionary_6"));
			strValue = JUnitTestConnect.activeConfigGetServiceDict(prov,JUnitTestConnect.DictionaryRDMFieldDictFileName, serviceId, dictId, false);
			TestUtilities.checkResult("dictUsed DictionaryRDMFieldDictFileName = ./RDMFieldDictionary",strValue != null && strValue.equals("./RDMFieldDictionary"));
			strValue = JUnitTestConnect.activeConfigGetServiceDict(prov,JUnitTestConnect.DictionaryEnumTypeDefFileName, serviceId, dictId, false);
			TestUtilities.checkResult("dictUsed DictionaryEnumTypeDefFileName = ./enumtype.def",strValue != null && strValue.equals("./enumtype.def"));
			strValue = JUnitTestConnect.activeConfigGetServiceDict(prov,JUnitTestConnect.DictionaryRdmFieldDictionaryItemName, serviceId, dictId, false);
			TestUtilities.checkResult("dictUsed DictionaryRdmFieldDictionaryItemName = RWFFld",strValue != null && strValue.equals("RWFFld"));
			strValue = JUnitTestConnect.activeConfigGetServiceDict(prov,JUnitTestConnect.DictionaryEnumTypeDefItemName, serviceId, dictId, false);
			TestUtilities.checkResult("dictUsed DictionaryEnumTypeDefItemName = RWFEnum",strValue != null && strValue.equals("RWFEnum"));
			
			//retrieve Service state
			TestUtilities.checkResult("checkHasState() == true", temp.checkHasState() == true);
			state = temp.state();
			TestUtilities.checkResult("state.checkHasAcceptingRequests() == true", state.checkHasAcceptingRequests());
			TestUtilities.checkResult("state.acceptingRequests() == 1", state.acceptingRequests() == 1);
			TestUtilities.checkResult("state.serviceState() == 1", state.serviceState() == 1);
			TestUtilities.checkResult("state.checkHasStatus() = false", !state.checkHasStatus());
			
			prov = null;
		}
		catch ( OmmException excp)
		{
			System.out.println(excp.getMessage());
			TestUtilities.checkResult("Receiving exception, test failed.", false );
		}
	}
}

public void testLoadCfgFromProgrammaticConfigForNiProv()
{
	TestUtilities.printTestHead("testLoadCfgFromProgrammaticConfigForNiProv","Test loading all NiProvider configuration parameters programmatically");
	
	//two testcases:
	//test case 1: NOT loading EmaConfig file from working dir.
	//test case 2: loading EmaConfigTest file
	for (int testCase = 0; testCase < 2; testCase++)
	{
		System.out.println(" #####Now it is running test case " + testCase);

		Map outermostMap = EmaFactory.createMap();
		Map innerMap = EmaFactory.createMap();
		ElementList elementList = EmaFactory.createElementList();
		ElementList innerElementList = EmaFactory.createElementList();
		try
		{
			elementList.add(EmaFactory.createElementEntry().ascii("DefaultNiProvider", "Provider_1"));

			innerElementList.add(EmaFactory.createElementEntry().ascii("Channel", "Channel_10"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("Directory", "Directory_1"));
			innerElementList.add(EmaFactory.createElementEntry().intValue("ItemCountHint", 5000));
			innerElementList.add(EmaFactory.createElementEntry().intValue("ServiceCountHint", 2000));
			innerElementList.add(EmaFactory.createElementEntry().intValue("MergeSourceDirectoryStreams", 0));
			innerElementList.add(EmaFactory.createElementEntry().intValue("RefreshFirstRequired", 0));
			innerElementList.add(EmaFactory.createElementEntry().intValue("RecoverUserSubmitSourceDirectory", 0));
			innerElementList.add(EmaFactory.createElementEntry().intValue("RemoveItemsOnDisconnect", 0));
			innerElementList.add(EmaFactory.createElementEntry().intValue("LoginRequestTimeOut", 50000));
			innerElementList.add(EmaFactory.createElementEntry().intValue("ReconnectAttemptLimit", 1));
			innerElementList.add(EmaFactory.createElementEntry().intValue("ReconnectMinDelay", 500));
			innerElementList.add(EmaFactory.createElementEntry().intValue("ReconnectMaxDelay", 600));
			innerElementList.add(EmaFactory.createElementEntry().intValue("RequestTimeout", 2400));
			innerElementList.add(EmaFactory.createElementEntry().intValue("DispatchTimeoutApiThread", 60));
			innerElementList.add(EmaFactory.createElementEntry().intValue("MaxDispatchCountApiThread", 300));
			innerElementList.add(EmaFactory.createElementEntry().intValue("MaxDispatchCountUserThread", 700));
			innerElementList.add(EmaFactory.createElementEntry().intValue("XmlTraceToStdout", 1));
			innerMap.add(EmaFactory.createMapEntry().keyAscii( "Provider_1", MapEntry.MapAction.ADD, innerElementList));
			innerElementList.clear();
			
			innerElementList.add(EmaFactory.createElementEntry().ascii("Channel", "Channel_2"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("Directory", "Directory_2"));
			innerMap.add(EmaFactory.createMapEntry().keyAscii( "Provider_2", MapEntry.MapAction.ADD, innerElementList)); 
			innerElementList.clear();
					
			elementList.add(EmaFactory.createElementEntry().map("NiProviderList", innerMap));
			innerMap.clear();

			outermostMap.add(EmaFactory.createMapEntry().keyAscii( "NiProviderGroup", MapEntry.MapAction.ADD, elementList));
			elementList.clear();
			
			innerElementList.add(EmaFactory.createElementEntry().ascii("ChannelType", "ChannelType::RSSL_SOCKET"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("CompressionType", "CompressionType::ZLib"));
			innerElementList.add(EmaFactory.createElementEntry().intValue("GuaranteedOutputBuffers", 8000));
			innerElementList.add(EmaFactory.createElementEntry().intValue("NumInputBuffers", 7777));
			innerElementList.add(EmaFactory.createElementEntry().intValue("SysRecvBufSize", 150000));
			innerElementList.add(EmaFactory.createElementEntry().intValue("SysSendBufSize", 200000));
			innerElementList.add(EmaFactory.createElementEntry().intValue("CompressionThreshold", 12856));
			innerElementList.add(EmaFactory.createElementEntry().intValue("ConnectionPingTimeout", 30000));
			innerElementList.add(EmaFactory.createElementEntry().ascii("InterfaceName", "localhost"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("Host", "10.0.0.1"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("Port", "8001"));
			innerElementList.add(EmaFactory.createElementEntry().intValue("TcpNodelay", 0));
			innerElementList.add(EmaFactory.createElementEntry().intValue("DirectWrite", 1));
			innerElementList.add(EmaFactory.createElementEntry().intValue("HighWaterMark", 5000));
			innerMap.add(EmaFactory.createMapEntry().keyAscii( "Channel_10", MapEntry.MapAction.ADD, innerElementList)); 
			innerElementList.clear();

			innerElementList.add(EmaFactory.createElementEntry().ascii("ChannelType", "ChannelType::ENCRYPTED"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("Host", "10.0.0.2"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("Port", "8002"));
			innerMap.add(EmaFactory.createMapEntry().keyAscii( "Channel_2", MapEntry.MapAction.ADD, innerElementList)); 
			innerElementList.clear();

			elementList.add(EmaFactory.createElementEntry().map("ChannelList", innerMap));
			innerMap.clear();

			outermostMap.add(EmaFactory.createMapEntry().keyAscii( "ChannelGroup", MapEntry.MapAction.ADD, elementList));
			elementList.clear();

			innerElementList.add(EmaFactory.createElementEntry().ascii("DictionaryType", "DictionaryType::ChannelDictionary"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("RdmFieldDictionaryFileName", "./RDMFieldDictionary_ID4"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("EnumTypeDefFileName", "./enumtype_ID4.def"));
			innerMap.add(EmaFactory.createMapEntry().keyAscii( "Dictionary_4", MapEntry.MapAction.ADD, innerElementList));
			innerElementList.clear();

			elementList.add(EmaFactory.createElementEntry().map("DictionaryList", innerMap));
			innerMap.clear();

			outermostMap.add(EmaFactory.createMapEntry().keyAscii( "DictionaryGroup", MapEntry.MapAction.ADD, elementList));
			elementList.clear();

			/////////////////////////////////////
			//DirectoryGroup
			Map serviceMap = EmaFactory.createMap();
			ElementList infoElementList = EmaFactory.createElementList();
			ElementList stateElementList = EmaFactory.createElementList();
			OmmArray infoArray = EmaFactory.createOmmArray();
			Series qosSeries = EmaFactory.createSeries();

			//encode service1
			infoElementList.add(EmaFactory.createElementEntry().intValue("ServiceId", 3));
			infoElementList.add(EmaFactory.createElementEntry().ascii("Vendor", "company name"));
			infoElementList.add(EmaFactory.createElementEntry().intValue("IsSource", 0));
			infoElementList.add(EmaFactory.createElementEntry().intValue("AcceptingConsumerStatus", 0));
			infoElementList.add(EmaFactory.createElementEntry().intValue("SupportsQoSRange", 0));
			infoElementList.add(EmaFactory.createElementEntry().intValue("SupportsOutOfBandSnapshots", 0));
			infoElementList.add(EmaFactory.createElementEntry().ascii("ItemList", "#.itemlist"));

			infoArray.add(EmaFactory.createOmmArrayEntry().intValue(5));
			infoArray.add(EmaFactory.createOmmArrayEntry().intValue(6));
			infoArray.add(EmaFactory.createOmmArrayEntry().intValue(7));
			infoArray.add(EmaFactory.createOmmArrayEntry().intValue(8));
			infoArray.add(EmaFactory.createOmmArrayEntry().intValue(200));
			infoElementList.add(EmaFactory.createElementEntry().array("Capabilities", infoArray));
			infoArray.clear();
			
			infoArray.add(EmaFactory.createOmmArrayEntry().ascii("Dictionary_4"));
			infoElementList.add(EmaFactory.createElementEntry().array("DictionariesUsed", infoArray));
			infoArray.clear();
			
			innerElementList.add(EmaFactory.createElementEntry().ascii("Timeliness", "Timeliness::RealTime"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("Rate", "Rate::TickByTick"));
			qosSeries.add(EmaFactory.createSeriesEntry().elementList(innerElementList));
			innerElementList.clear();
			
			innerElementList.add(EmaFactory.createElementEntry().intValue("Timeliness", 100));
			innerElementList.add(EmaFactory.createElementEntry().intValue("Rate", 100));
			qosSeries.add(EmaFactory.createSeriesEntry().elementList(innerElementList));
			innerElementList.clear();
			infoElementList.add(EmaFactory.createElementEntry().series("QoS", qosSeries));
			qosSeries.clear();
			
			stateElementList.add(EmaFactory.createElementEntry().intValue("ServiceState", 1));
			stateElementList.add(EmaFactory.createElementEntry().intValue("AcceptingRequests", 1));
			
			innerElementList.add(EmaFactory.createElementEntry().ascii("StreamState", "StreamState::CloseRecover"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("DataState", "DataState::Suspect"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("StatusCode", "StatusCode::DacsDown"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("StatusText", "dacsDown"));
			stateElementList.add(EmaFactory.createElementEntry().elementList("Status", innerElementList));
			innerElementList.clear();
						
			innerElementList.add(EmaFactory.createElementEntry().elementList("InfoFilter", infoElementList));
			infoElementList.clear();
			innerElementList.add(EmaFactory.createElementEntry().elementList("StateFilter", stateElementList));
			stateElementList.clear();
			
			serviceMap.add(EmaFactory.createMapEntry().keyAscii( "DIRECT_FEED", MapEntry.MapAction.ADD, innerElementList));
			innerElementList.clear();
			
			//encode service2
			infoElementList.add(EmaFactory.createElementEntry().intValue("ServiceId", 4));
			infoElementList.add(EmaFactory.createElementEntry().ascii("Vendor", "company name"));
			infoElementList.add(EmaFactory.createElementEntry().intValue("AcceptingConsumerStatus", 0));
			infoElementList.add(EmaFactory.createElementEntry().intValue("SupportsQoSRange", 0));
			infoElementList.add(EmaFactory.createElementEntry().intValue("SupportsOutOfBandSnapshots", 0));
			infoElementList.add(EmaFactory.createElementEntry().ascii("ItemList", "#.itemlist"));

			infoArray.add(EmaFactory.createOmmArrayEntry().ascii("MMT_DICTIONARY"));
			infoArray.add(EmaFactory.createOmmArrayEntry().ascii("MMT_MARKET_PRICE"));
			infoArray.add(EmaFactory.createOmmArrayEntry().ascii("MMT_MARKET_BY_ORDER"));
			infoArray.add(EmaFactory.createOmmArrayEntry().ascii("200"));
			infoElementList.add(EmaFactory.createElementEntry().array("Capabilities", infoArray));
			infoArray.clear();
			
			infoArray.add(EmaFactory.createOmmArrayEntry().ascii("Dictionary_6"));
			infoElementList.add(EmaFactory.createElementEntry().array("DictionariesUsed", infoArray));
			infoArray.clear();
			
			stateElementList.add(EmaFactory.createElementEntry().intValue("ServiceState", 1));
			stateElementList.add(EmaFactory.createElementEntry().intValue("AcceptingRequests", 1));

			innerElementList.add(EmaFactory.createElementEntry().elementList("InfoFilter", infoElementList));
			infoElementList.clear();
			innerElementList.add(EmaFactory.createElementEntry().elementList("StateFilter", stateElementList));
			stateElementList.clear();
			
			serviceMap.add(EmaFactory.createMapEntry().keyAscii( "DIRECT_FEED1", MapEntry.MapAction.ADD, innerElementList));
			innerElementList.clear();
			
			innerMap.add(EmaFactory.createMapEntry().keyAscii( "Directory_1", MapEntry.MapAction.ADD, serviceMap));
			serviceMap.clear();

			elementList.add(EmaFactory.createElementEntry().ascii("DefaultDirectory", "Directory_1"));
			elementList.add(EmaFactory.createElementEntry().map("DirectoryList", innerMap));
			innerMap.clear();
			
			outermostMap.add(EmaFactory.createMapEntry().keyAscii( "DirectoryGroup", MapEntry.MapAction.ADD, elementList));
			elementList.clear();

			String localConfigPath = null;
			if (testCase == 1)
				localConfigPath = "./src/test/resources/com/rtsdk/ema/unittest/EmaFileConfigTests/EmaConfigTest.xml";

			System.out.println("Using Ema Config: " + localConfigPath);
			
			OmmProvider prov = null;
			if (testCase == 0)
				prov = JUnitTestConnect.createOmmNiProvider(EmaFactory.createOmmNiProviderConfig().config(outermostMap));
			else if (testCase == 1)
				prov = JUnitTestConnect.createOmmNiProvider(EmaFactory.createOmmNiProviderConfig(localConfigPath).config(outermostMap));
			
			String defaultProvName = JUnitTestConnect.activeConfigGetStringValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.NiProviderName, -1);
			TestUtilities.checkResult("DefaultProvider value != null", defaultProvName != null);
			TestUtilities.checkResult("DefaultProvider value == Provider_1", defaultProvName.contentEquals("Provider_1") );
			String provChannelVal = JUnitTestConnect.activeConfigGetStringValue(prov, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelName, 0);
			TestUtilities.checkResult("Channel value != null", provChannelVal != null);
			TestUtilities.checkResult("Channel value == Channel_10", provChannelVal.contentEquals("Channel_10") );

			boolean boolValue = JUnitTestConnect.activeConfigGetBooleanValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.NiProviderMergeSourceDirectoryStreams, -1);
			TestUtilities.checkResult("MergeSourceDirectoryStreams == 0", boolValue == false);
			boolValue = JUnitTestConnect.activeConfigGetBooleanValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.NiProviderRecoverUserSubmitSourceDirectory, -1);
			TestUtilities.checkResult("RecoverUserSubmitSourceDirectory == 0", boolValue == false);
			boolValue = JUnitTestConnect.activeConfigGetBooleanValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.NiProviderRemoveItemsOnDisconnect, -1);
			TestUtilities.checkResult("RemoveItemsOnDisconnect == 0", boolValue == false);
			boolValue = JUnitTestConnect.activeConfigGetBooleanValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.NiProviderRefreshFirstRequired, -1);
			TestUtilities.checkResult("RefreshFirstRequired == 0", boolValue == false);
			long intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.ItemCountHint, -1);
			TestUtilities.checkResult("ItemCountHint value == 5000", intLongValue == 5000 );
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.ServiceCountHint, -1);
			TestUtilities.checkResult("ServiceCountHint value == 2000", intLongValue == 2000 );
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.RequestTimeout, -1);
			TestUtilities.checkResult("RequestTimeout value == 2400", intLongValue == 2400 );
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.LoginRequestTimeOut, -1);
			TestUtilities.checkResult("LoginRequestTimeOut value == 50000", intLongValue == 50000 );
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.DispatchTimeoutApiThread, -1);
			TestUtilities.checkResult("DispatchTimeoutApiThread value == 60", intLongValue == 60 );
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.MaxDispatchCountApiThread, -1);
			TestUtilities.checkResult("MaxDispatchCountApiThread value == 300", intLongValue == 300 );
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.MaxDispatchCountUserThread, -1);
			TestUtilities.checkResult("MaxDispatchCountUserThread value == 700", intLongValue == 700 );
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.ReconnectAttemptLimit, -1);
			TestUtilities.checkResult("ReconnectAttemptLimit == 1", intLongValue == 1);
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.ReconnectMinDelay, -1);
			TestUtilities.checkResult("ReconnectMinDelay == 500", intLongValue == 500);
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.ReconnectMaxDelay, -1);
			TestUtilities.checkResult("ReconnectMaxDelay == 600", intLongValue == 600);

			boolValue = JUnitTestConnect.activeConfigGetBooleanValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.XmlTraceToStdout, -1);
			TestUtilities.checkResult("XmlTraceToStdout == 1", boolValue == true);
			
			// Check Channel configuration:
			// Check Channel_1 configuration.
			provChannelVal = "Channel_10";
			System.out.println("\nRetrieving Channel_10 configuration values "); 
			String consChannelVal = JUnitTestConnect.activeConfigGetStringValue(prov, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelName, 0);
			TestUtilities.checkResult("Channel value != null", consChannelVal != null);
			TestUtilities.checkResult("Channel value == Channel_10", consChannelVal.contentEquals("Channel_10") );
			int channelConnType = JUnitTestConnect.activeConfigGetIntLongValue(prov, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelType, 0);
			TestUtilities.checkResult("ChannelConnType == ChannelType::RSSL_SOCKET", channelConnType == ChannelTypeSocket);
		
			String strValue = JUnitTestConnect.activeConfigGetStringValue(prov, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.InterfaceName, 0);
			TestUtilities.checkResult("InterfaceName == localhost", strValue.contentEquals("localhost"));
			
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(prov, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.CompressionType, 0);
			TestUtilities.checkResult("CompressionType == CompressionType::ZLIB", intLongValue == CompressionTypeZLib);
			
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(prov, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.GuaranteedOutputBuffers, 0);
			TestUtilities.checkResult("GuaranteedOutputBuffers == 8000", intLongValue == 8000);
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(prov, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.NumInputBuffers, 0);
			TestUtilities.checkResult("NumInputBuffers == 7777", intLongValue == 7777);
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(prov, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.SysRecvBufSize, 0);
			TestUtilities.checkResult("SysRecvBufSize == 150000", intLongValue == 150000);
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(prov, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.SysSendBufSize, 0);
			TestUtilities.checkResult("SysSendBufSize == 200000", intLongValue == 200000);
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(prov, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.CompressionThreshold, 0);
			TestUtilities.checkResult("CompressionThreshold == 12856", intLongValue == 12856);
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(prov, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ConnectionPingTimeout, 0);
			TestUtilities.checkResult("ConnectionPingTimeout == 30000", intLongValue == 30000);
			boolValue = JUnitTestConnect.activeConfigGetBooleanValue(prov, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.TcpNodelay, 0);
			TestUtilities.checkResult("TcpNodelay == 0", boolValue == false);
			boolValue = JUnitTestConnect.activeConfigGetBooleanValue(prov, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.DirectWrite, 0);
			TestUtilities.checkResult("DirectWrite == 1", boolValue == true);
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(prov, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.HighWaterMark, 0);
			TestUtilities.checkResult("HighWaterMark == 5000", intLongValue == 5000);
			strValue = JUnitTestConnect.activeConfigGetStringValue(prov, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.Host, 0);
			TestUtilities.checkResult("Host == 10.0.0.1", strValue.contentEquals("10.0.0.1"));
			strValue = JUnitTestConnect.activeConfigGetStringValue(prov, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.Port, 0);
			TestUtilities.checkResult("Port == 8001", strValue.contentEquals("8001"));
			
			
			//retrieve directory
			strValue = JUnitTestConnect.activeConfigGetStringValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.DirectoryName, -1);
			TestUtilities.checkResult("DirectoryName == Directory_1", strValue.contentEquals("Directory_1"));
			
			List<Service> services = JUnitTestConnect.activeConfigGetService(prov, false);
			TestUtilities.checkResult("services.size() == 2", services.size() == 2);
			
			/*********retrieve first service *************/
			System.out.println("\nRetrieving DIRECT_FEED service configuration values "); 
			Service temp = services.get(0);
			TestUtilities.checkResult("temp != null", temp != null);
			
			TestUtilities.checkResult("checkHasInfo() == true", temp.checkHasInfo() == true);
			
			TestUtilities.checkResult("serviceId == 3", temp.serviceId() == 3);
			ServiceInfo info = temp.info();
			TestUtilities.checkResult("infoFilter.serviceName == DIRECT_FEED", info.serviceName().toString().equals("DIRECT_FEED"));
			TestUtilities.checkResult("infoFilter.vendorName == company name", info.checkHasVendor() && info.vendor().toString().equals("company name"));
			TestUtilities.checkResult("infoFilter.isSource == false", info.checkHasIsSource() && info.isSource() == 0);
			TestUtilities.checkResult("infoFilter.itemList == #.itemlist", info.checkHasItemList() && info.itemList().toString().equals("#.itemlist"));
			TestUtilities.checkResult("infoFilter.acceptingConsumerStatus == 0", info.checkHasAcceptingConsumerStatus() && info.acceptingConsumerStatus() == 0);
			TestUtilities.checkResult("infoFilter.supportsQosRange == 0", info.checkHasSupportsQosRange() && info.supportsQosRange() == 0);
			TestUtilities.checkResult("infoFilter.supportsOutOfBandSnapshots == 0", info.checkHasSupportsOutOfBandSnapshots() && info.supportsOutOfBandSnapshots() == 0);
			
			//retrieve capabilities
			List<Long> capabilities =  info.capabilitiesList();
			TestUtilities.checkResult("capabilities.size() == 5", capabilities.size() == 5);
			TestUtilities.checkResult("capabilities.get(0) == MMT_DICTIONARY", capabilities.get(0) == 5);
			TestUtilities.checkResult("capabilities.get(1) == MMT_MARKET_PRICE", capabilities.get(1) == 6);
			TestUtilities.checkResult("capabilities.get(2) == MMT_MARKET_BY_ORDER", capabilities.get(2) == 7);
			TestUtilities.checkResult("capabilities.get(3) == MMT_MARKET_BY_PRICE", capabilities.get(3) == 8);
			TestUtilities.checkResult("capabilities.get(4) == 200", capabilities.get(4) == 200);
			
			//retrieve qos
			TestUtilities.checkResult("info.checkHasQos()", info.checkHasQos());
			List<Qos> qos = info.qosList();
			TestUtilities.checkResult("qos.size() == 2", qos.size() == 2);
			TestUtilities.checkResult("qos.get(0).rate() == QosRates.TICK_BY_TICK", qos.get(0).rate() == QosRates.TICK_BY_TICK);
			TestUtilities.checkResult("qos.get(0).timeliness() == QosTimeliness.REALTIME", qos.get(0).timeliness() == QosTimeliness.REALTIME);
			TestUtilities.checkResult("qos.get(1).rate() == QosRates.TIME_CONFLATED", qos.get(1).rate() == QosRates.TIME_CONFLATED);
			TestUtilities.checkResult("qos.get(1).rateInfo() == 100", qos.get(1).rateInfo() == 100);
			TestUtilities.checkResult("qos.get(1).timeliness() == QosTimeliness.DELAYED", qos.get(1).timeliness() == QosTimeliness.DELAYED);
			TestUtilities.checkResult("qos.get(1).timeInfo() == 100", qos.get(1).timeInfo() == 100);
			
			//retrieve dictionary provided/used by this service
			TestUtilities.checkResult("info.checkHasDictionariesProvided() == false", !info.checkHasDictionariesProvided());
			TestUtilities.checkResult("info.checkHasDictionariesUsed() == true", info.checkHasDictionariesUsed());
			List<String> dictProvided = info.dictionariesProvidedList();
			TestUtilities.checkResult("dictProvided.size() == 0", dictProvided.size() == 0);
			List<String> dictUsed = info.dictionariesUsedList();
			TestUtilities.checkResult("dictUsed.size() == 2", dictUsed.size() == 2);
			TestUtilities.checkResult("info.dictionariesUsedList().get(0) == RWFFld",dictUsed.get(0).equals("RWFFld"));
			TestUtilities.checkResult("info.dictionariesUsedList().get(1) == RWFEnum",dictUsed.get(1).equals("RWFEnum"));
			
			//retrieve Service state
			TestUtilities.checkResult("checkHasState() == true", temp.checkHasState() == true);
			ServiceState state = temp.state();
			TestUtilities.checkResult("state.checkHasAcceptingRequests() == true", state.checkHasAcceptingRequests());
			TestUtilities.checkResult("state.acceptingRequests() == 1", state.acceptingRequests() == 1);
			TestUtilities.checkResult("state.serviceState() == 1", state.serviceState() == 1);
			TestUtilities.checkResult("checkHasStatus() = true", state.checkHasStatus());
			TestUtilities.checkResult("state.status().streamState() == 3", state.status().streamState() == 3);
			TestUtilities.checkResult("state.status().dataState() == 2", state.status().dataState() == 2);
			TestUtilities.checkResult("state.status().code() == 29", state.status().code() == 29);
			TestUtilities.checkResult("state.status().text() == dacsDown", state.status().text() != null &&  state.status().text().toString().equals("dacsDown"));

			/*********retrieve second service *************/
			System.out.println("\nRetrieving DIRECT_FEED1 service configuration values "); 
			temp = services.get(1);
			TestUtilities.checkResult("temp != null", temp != null);
			
			TestUtilities.checkResult("checkHasInfo() == true", temp.checkHasInfo() == true);
			
			TestUtilities.checkResult("serviceId == 4", temp.serviceId() == 4);
			info = temp.info();
			TestUtilities.checkResult("infoFilter.serviceName == DIRECT_FEED1", info.serviceName().toString().equals("DIRECT_FEED1"));
			TestUtilities.checkResult("infoFilter.vendorName == company name", info.checkHasVendor() && info.vendor().toString().equals("company name"));
			TestUtilities.checkResult("infoFilter.isSource == false", info.checkHasIsSource() == false);
			TestUtilities.checkResult("infoFilter.itemList == #.itemlist", info.checkHasItemList() && info.itemList().toString().equals("#.itemlist"));
			TestUtilities.checkResult("infoFilter.acceptingConsumerStatus == 0", info.checkHasAcceptingConsumerStatus() && info.acceptingConsumerStatus() == 0);
			TestUtilities.checkResult("infoFilter.supportsQosRange == 0", info.checkHasSupportsQosRange() && info.supportsQosRange() == 0);
			TestUtilities.checkResult("infoFilter.supportsOutOfBandSnapshots == 0", info.checkHasSupportsOutOfBandSnapshots() && info.supportsOutOfBandSnapshots() == 0);
			
			//retrieve capabilities
			capabilities =  info.capabilitiesList();
			TestUtilities.checkResult("capabilities.size() == 4", capabilities.size() == 4);
			TestUtilities.checkResult("capabilities.get(0) == MMT_DICTIONARY", capabilities.get(0) == 5);
			TestUtilities.checkResult("capabilities.get(1) == MMT_MARKET_PRICE", capabilities.get(1) == 6);
			TestUtilities.checkResult("capabilities.get(2) == MMT_MARKET_BY_ORDER", capabilities.get(2) == 7);
			TestUtilities.checkResult("capabilities.get(3) == 200", capabilities.get(3) == 200);
			
			//retrieve qos
			TestUtilities.checkResult("info.checkHasQos()", info.checkHasQos());
			qos = info.qosList();
			TestUtilities.checkResult("qos.size() == 1", qos.size() == 1);
			TestUtilities.checkResult("qos.get(0).rate() == QosRates.TICK_BY_TICK", qos.get(0).rate() == QosRates.TICK_BY_TICK);
			TestUtilities.checkResult("qos.get(0).timeliness() == QosTimeliness.REALTIME", qos.get(0).timeliness() == QosTimeliness.REALTIME);
			
			//retrieve dictionary provided/used by this service
			TestUtilities.checkResult("!info.checkHasDictionariesProvided() == true", !info.checkHasDictionariesProvided());
			TestUtilities.checkResult("info.checkHasDictionariesUsed() == true", info.checkHasDictionariesUsed());
			dictProvided = info.dictionariesProvidedList();
			TestUtilities.checkResult("dictProvided.size() == 0", dictProvided.size() == 0);
			dictUsed = info.dictionariesUsedList();
			TestUtilities.checkResult("dictUsed.size() == 2", dictUsed.size() == 2);
			TestUtilities.checkResult("info.dictionariesUsedList().get(0) == RWFFld",dictUsed.get(0).equals("RWFFld"));
			TestUtilities.checkResult("info.dictionariesUsedList().get(1) == RWFEnum",dictUsed.get(1).equals("RWFEnum"));
			
			//retrieve Service state
			TestUtilities.checkResult("checkHasState() == true", temp.checkHasState() == true);
			state = temp.state();
			TestUtilities.checkResult("state.checkHasAcceptingRequests() == true", state.checkHasAcceptingRequests());
			TestUtilities.checkResult("state.acceptingRequests() == 1", state.acceptingRequests() == 1);
			TestUtilities.checkResult("state.serviceState() == 1", state.serviceState() == 1);
			TestUtilities.checkResult("state.checkHasStatus() = false", !state.checkHasStatus());
			
			prov = null;
		}
		catch ( OmmException excp)
		{
			System.out.println(excp.getMessage());
			TestUtilities.checkResult("Receiving exception, test failed.", false );
		}
	}
}

public void testMergCfgBetweenFileAndProgrammaticConfigForIProv()
{
	TestUtilities.printTestHead("testMergCfgBetweenFileAndProgrammaticConfigForIProv","Test merge all IProvider configuration parameters between config file and programmatically config");
	
	//two testcases:
	//test case 1: NOT loading EmaConfig file from resource directory.
	//test case 2: loading EmaConfigTest file
	for (int testCase = 0; testCase < 2; testCase++)
	{
		System.out.println(" #####Now it is running test case " + testCase);

		Map outermostMap1 = EmaFactory.createMap();
		Map outermostMap2 = EmaFactory.createMap();
		Map outermostMap3 = EmaFactory.createMap();
		Map outermostMap4 = EmaFactory.createMap();
		Map innerMap = EmaFactory.createMap();
		ElementList elementList = EmaFactory.createElementList();
		ElementList innerElementList = EmaFactory.createElementList();
		try
		{
			innerElementList.add(EmaFactory.createElementEntry().ascii("Server", "Server_2"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("Directory", "Directory_2"));
			innerElementList.add(EmaFactory.createElementEntry().intValue("ItemCountHint", 9000));
			innerElementList.add(EmaFactory.createElementEntry().intValue("ServiceCountHint", 9000));
			innerElementList.add(EmaFactory.createElementEntry().intValue("AcceptDirMessageWithoutMinFilters", 1));
			innerElementList.add(EmaFactory.createElementEntry().intValue("AcceptMessageSameKeyButDiffStream", 1));
			innerElementList.add(EmaFactory.createElementEntry().intValue("AcceptMessageThatChangesService", 1));
			innerElementList.add(EmaFactory.createElementEntry().intValue("AcceptMessageWithoutAcceptingRequests", 1));
			innerElementList.add(EmaFactory.createElementEntry().intValue("AcceptMessageWithoutBeingLogin", 1));
			innerElementList.add(EmaFactory.createElementEntry().intValue("AcceptMessageWithoutQosInRange", 1));
			innerElementList.add(EmaFactory.createElementEntry().intValue("FieldDictionaryFragmentSize", 2000));
			innerElementList.add(EmaFactory.createElementEntry().intValue("EnumTypeFragmentSize", 1000));
			innerElementList.add(EmaFactory.createElementEntry().intValue("RefreshFirstRequired", 0));
			innerElementList.add(EmaFactory.createElementEntry().intValue("RequestTimeout", 9000));
			innerElementList.add(EmaFactory.createElementEntry().intValue("DispatchTimeoutApiThread", 5656));
			innerElementList.add(EmaFactory.createElementEntry().intValue("MaxDispatchCountApiThread", 900));
			innerElementList.add(EmaFactory.createElementEntry().intValue("MaxDispatchCountUserThread", 900));
			innerElementList.add(EmaFactory.createElementEntry().intValue("XmlTraceToStdout", 1));
			innerElementList.add(EmaFactory.createElementEntry().intValue("EnforceAckIDValidation", 1));
			innerMap.add(EmaFactory.createMapEntry().keyAscii( "Provider_2", MapEntry.MapAction.ADD, innerElementList));
			innerElementList.clear();
			
			elementList.add(EmaFactory.createElementEntry().map("IProviderList", innerMap));
			innerMap.clear();

			outermostMap1.add(EmaFactory.createMapEntry().keyAscii( "IProviderGroup", MapEntry.MapAction.ADD, elementList));
			elementList.clear();
			
			innerElementList.add(EmaFactory.createElementEntry().ascii("ServerType", "ServerType::RSSL_SOCKET"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("CompressionType", "CompressionType::LZ4"));
			innerElementList.add(EmaFactory.createElementEntry().intValue("GuaranteedOutputBuffers", 7000));
			innerElementList.add(EmaFactory.createElementEntry().intValue("NumInputBuffers", 5000));
			innerElementList.add(EmaFactory.createElementEntry().intValue("SysRecvBufSize", 550000));
			innerElementList.add(EmaFactory.createElementEntry().intValue("SysSendBufSize", 700000));
			innerElementList.add(EmaFactory.createElementEntry().intValue("CompressionThreshold", 12758));
			innerElementList.add(EmaFactory.createElementEntry().intValue("ConnectionPingTimeout", 70000));
			innerElementList.add(EmaFactory.createElementEntry().intValue("ConnectionMinPingTimeout", 4000));
			innerElementList.add(EmaFactory.createElementEntry().ascii("InterfaceName", "localhost"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("Port", "8003"));
			innerElementList.add(EmaFactory.createElementEntry().intValue("TcpNodelay", 0));
			innerElementList.add(EmaFactory.createElementEntry().intValue("DirectWrite", 1));
			innerElementList.add(EmaFactory.createElementEntry().intValue("HighWaterMark", 5000));
			innerMap.add(EmaFactory.createMapEntry().keyAscii( "Server_2", MapEntry.MapAction.ADD, innerElementList)); 
			innerElementList.clear();

			elementList.add(EmaFactory.createElementEntry().map("ServerList", innerMap));
			innerMap.clear();

			outermostMap2.add(EmaFactory.createMapEntry().keyAscii( "ServerGroup", MapEntry.MapAction.ADD, elementList));
			elementList.clear();

			innerElementList.add(EmaFactory.createElementEntry().ascii("DictionaryType", "DictionaryType::ChannelDictionary"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("RdmFieldDictionaryItemName", "./ConfigDB3_RWFFld"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("EnumTypeDefItemName", "./ConfigDB3_RWFEnum"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("RdmFieldDictionaryFileName", "./ConfigDB3_RDMFieldDictionary"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("EnumTypeDefFileName", "./ConfigDB3_enumtype.def"));
			innerMap.add(EmaFactory.createMapEntry().keyAscii( "Dictionary_2", MapEntry.MapAction.ADD, innerElementList));
			innerElementList.clear();
			
			elementList.add(EmaFactory.createElementEntry().map("DictionaryList", innerMap));
			innerMap.clear();

			outermostMap3.add(EmaFactory.createMapEntry().keyAscii( "DictionaryGroup", MapEntry.MapAction.ADD, elementList));
			elementList.clear();

			/////////////////////////////////////
			//DirectoryGroup
			Map serviceMap = EmaFactory.createMap();
			ElementList infoElementList = EmaFactory.createElementList();
			ElementList stateElementList = EmaFactory.createElementList();
			OmmArray infoArray = EmaFactory.createOmmArray();
			Series qosSeries = EmaFactory.createSeries();

			//encode service1
			infoElementList.add(EmaFactory.createElementEntry().intValue("ServiceId", 1));
			infoElementList.add(EmaFactory.createElementEntry().ascii("Vendor", "Vendor"));
			infoElementList.add(EmaFactory.createElementEntry().intValue("IsSource", 1));
			infoElementList.add(EmaFactory.createElementEntry().intValue("AcceptingConsumerStatus", 1));
			infoElementList.add(EmaFactory.createElementEntry().intValue("SupportsQoSRange", 1));
			infoElementList.add(EmaFactory.createElementEntry().intValue("SupportsOutOfBandSnapshots", 1));
			infoElementList.add(EmaFactory.createElementEntry().ascii("ItemList", "#.itemlist2"));

			infoArray.add(EmaFactory.createOmmArrayEntry().ascii("8"));
			infoArray.add(EmaFactory.createOmmArrayEntry().ascii("9"));
			infoArray.add(EmaFactory.createOmmArrayEntry().ascii("MMT_MARKET_BY_ORDER"));
			infoArray.add(EmaFactory.createOmmArrayEntry().ascii("130"));
			infoElementList.add(EmaFactory.createElementEntry().array("Capabilities", infoArray));
			infoArray.clear();
			
			infoArray.add(EmaFactory.createOmmArrayEntry().ascii("Dictionary_2"));
			infoElementList.add(EmaFactory.createElementEntry().array("DictionariesProvided", infoArray));
			infoArray.clear();
			infoArray.add(EmaFactory.createOmmArrayEntry().ascii("Dictionary_3"));
			infoElementList.add(EmaFactory.createElementEntry().array("DictionariesUsed", infoArray));
			infoArray.clear();
			
			innerElementList.add(EmaFactory.createElementEntry().intValue("Timeliness", 200));
			innerElementList.add(EmaFactory.createElementEntry().intValue("Rate", 200));
			qosSeries.add(EmaFactory.createSeriesEntry().elementList(innerElementList));
			innerElementList.clear();
			
			innerElementList.add(EmaFactory.createElementEntry().ascii("Timeliness", "Timeliness::InexactDelayed"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("Rate", "Rate::JustInTimeConflated"));
			qosSeries.add(EmaFactory.createSeriesEntry().elementList(innerElementList));
			innerElementList.clear();
			
			infoElementList.add(EmaFactory.createElementEntry().series("QoS", qosSeries));
			qosSeries.clear();
			
			stateElementList.add(EmaFactory.createElementEntry().intValue("ServiceState", 0));
			stateElementList.add(EmaFactory.createElementEntry().intValue("AcceptingRequests", 0));
			
			innerElementList.add(EmaFactory.createElementEntry().ascii("StreamState", "StreamState::CloseRecover"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("DataState", "DataState::Suspect"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("StatusCode", "StatusCode::DacsDown"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("StatusText", "dacsDown"));
			stateElementList.add(EmaFactory.createElementEntry().elementList("Status", innerElementList));
			innerElementList.clear();
						
			innerElementList.add(EmaFactory.createElementEntry().elementList("InfoFilter", infoElementList));
			infoElementList.clear();
			innerElementList.add(EmaFactory.createElementEntry().elementList("StateFilter", stateElementList));
			stateElementList.clear();
			
			serviceMap.add(EmaFactory.createMapEntry().keyAscii( "DIRECT_FEED", MapEntry.MapAction.ADD, innerElementList));
			innerElementList.clear();
			
			innerMap.add(EmaFactory.createMapEntry().keyAscii( "Directory_2", MapEntry.MapAction.ADD, serviceMap));
			serviceMap.clear();

			elementList.add(EmaFactory.createElementEntry().map("DirectoryList", innerMap));
			innerMap.clear();
			
			outermostMap4.add(EmaFactory.createMapEntry().keyAscii( "DirectoryGroup", MapEntry.MapAction.ADD, elementList));
			elementList.clear();

			String localConfigPath = null;
			if (testCase == 1)
				localConfigPath = "./src/test/resources/com/rtsdk/ema/unittest/EmaFileConfigTests/EmaConfigTest.xml";

			System.out.println("Using Ema Config: " + localConfigPath);
			
			OmmProvider prov = null;
			if (testCase == 0)
				prov = JUnitTestConnect.createOmmIProvider(EmaFactory.createOmmIProviderConfig()
						.config(outermostMap1)
						.config(outermostMap2)
						.config(outermostMap3)
						.config(outermostMap4).providerName("Provider_2"));
			else if (testCase == 1)
				prov = JUnitTestConnect.createOmmIProvider(EmaFactory.createOmmIProviderConfig(localConfigPath)
						.config(outermostMap1)
						.config(outermostMap2)
						.config(outermostMap3)
						.config(outermostMap4).providerName("Provider_2"));
			
			String defaultProvName = JUnitTestConnect.activeConfigGetStringValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.IProviderName);
			TestUtilities.checkResult("DefaultProvider value != null", defaultProvName != null);
			TestUtilities.checkResult("DefaultProvider value == Provider_2", defaultProvName.contentEquals("Provider_2") );
			String provServerVal = JUnitTestConnect.activeConfigGetStringValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.ServerName);
			TestUtilities.checkResult("Server value != null", provServerVal != null);
			TestUtilities.checkResult("Server value == Server_2", provServerVal.contentEquals("Server_2") );

			boolean boolValue = JUnitTestConnect.activeConfigGetBooleanValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.IProviderAcceptDirMessageWithoutMinFilters);
			TestUtilities.checkResult("AcceptDirMessageWithoutMinFilters == 1", boolValue == true);
			boolValue = JUnitTestConnect.activeConfigGetBooleanValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.IProviderAcceptMessageWithoutAcceptingRequests);
			TestUtilities.checkResult("AcceptMessageWithoutAcceptingRequests == 1", boolValue == true);
			boolValue = JUnitTestConnect.activeConfigGetBooleanValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.IProviderAcceptMessageWithoutBeingLogin);
			TestUtilities.checkResult("AcceptMessageWithoutBeingLogin == 1", boolValue == true);
			boolValue = JUnitTestConnect.activeConfigGetBooleanValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.IProviderAcceptMessageWithoutQosInRange);
			TestUtilities.checkResult("AcceptMessageWithoutQosInRange == 1", boolValue == true);
			boolValue = JUnitTestConnect.activeConfigGetBooleanValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.IProviderAcceptMessageSameKeyButDiffStream);
			TestUtilities.checkResult("AcceptMessageSameKeyButDiffStream == 1", boolValue == true);
			boolValue = JUnitTestConnect.activeConfigGetBooleanValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.IProviderAcceptMessageThatChangesService);
			TestUtilities.checkResult("AcceptMessageThatChangesService == 1", boolValue == true);
			boolValue = JUnitTestConnect.activeConfigGetBooleanValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.IProviderEnforceAckIDValidation);
			TestUtilities.checkResult("EnforceAckIDValidation == 1", boolValue == true);
			boolValue = JUnitTestConnect.activeConfigGetBooleanValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.IProviderRefreshFirstRequired);
			TestUtilities.checkResult("RefreshFirstRequired == 0", boolValue == false);
			int intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.DictionaryFieldDictFragmentSize);
			TestUtilities.checkResult("MaxFieldDictFragmentSize value == 2000", intLongValue == 2000 );
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.DictionaryEnumTypeFragmentSize);
			TestUtilities.checkResult("MaxEnumTypeFragmentSize value == 1000", intLongValue == 1000 );
			
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.ItemCountHint);
			TestUtilities.checkResult("ItemCountHint value == 9000", intLongValue == 9000 );
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.ServiceCountHint);
			TestUtilities.checkResult("ServiceCountHint value == 9000", intLongValue == 9000 );
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.RequestTimeout);
			TestUtilities.checkResult("RequestTimeout value == 9000", intLongValue == 9000 );
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.DispatchTimeoutApiThread);
			TestUtilities.checkResult("DispatchTimeoutApiThread value == 5656", intLongValue == 5656 );

			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.MaxDispatchCountApiThread);
			TestUtilities.checkResult("MaxDispatchCountApiThread value == 900", intLongValue == 900 );
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.MaxDispatchCountUserThread);
			TestUtilities.checkResult("MaxDispatchCountUserThread value == 900", intLongValue == 900 );

			boolValue = JUnitTestConnect.activeConfigGetBooleanValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.XmlTraceToStdout);
			TestUtilities.checkResult("XmlTraceToStdout == 1", boolValue == true);
			
			// Check Server configuration:
			// Check Server_2 configuration.
			provServerVal = "Server_2";
			System.out.println("\nRetrieving Server_2 configuration values "); 
			int serverConnType = JUnitTestConnect.activeConfigGetIntLongValue(prov, JUnitTestConnect.ConfigGroupTypeServer, JUnitTestConnect.ServerType);
			TestUtilities.checkResult("serverConnType == ServerType::RSSL_SOCKET", serverConnType == ChannelTypeSocket);
		
			String strValue = JUnitTestConnect.activeConfigGetStringValue(prov, JUnitTestConnect.ConfigGroupTypeServer, JUnitTestConnect.InterfaceName);
			TestUtilities.checkResult("InterfaceName == localhost", strValue.contentEquals("localhost"));
			
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(prov, JUnitTestConnect.ConfigGroupTypeServer, JUnitTestConnect.CompressionType);
			TestUtilities.checkResult("CompressionType == CompressionType::LZ4", intLongValue == CompressionTypeLZ4);
			
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(prov, JUnitTestConnect.ConfigGroupTypeServer, JUnitTestConnect.GuaranteedOutputBuffers);
			TestUtilities.checkResult("GuaranteedOutputBuffers == 7000", intLongValue == 7000);
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(prov, JUnitTestConnect.ConfigGroupTypeServer, JUnitTestConnect.NumInputBuffers);
			TestUtilities.checkResult("NumInputBuffers == 5000", intLongValue == 5000);
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(prov, JUnitTestConnect.ConfigGroupTypeServer, JUnitTestConnect.SysRecvBufSize);
			TestUtilities.checkResult("SysRecvBufSize == 550000", intLongValue == 550000);
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(prov, JUnitTestConnect.ConfigGroupTypeServer, JUnitTestConnect.SysSendBufSize);
			TestUtilities.checkResult("SysSendBufSize == 700000", intLongValue == 700000);
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(prov, JUnitTestConnect.ConfigGroupTypeServer, JUnitTestConnect.CompressionThreshold);
			TestUtilities.checkResult("CompressionThreshold == 12758", intLongValue == 12758);
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(prov, JUnitTestConnect.ConfigGroupTypeServer, JUnitTestConnect.ConnectionPingTimeout);
			TestUtilities.checkResult("ConnectionPingTimeout == 70000", intLongValue == 70000);
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(prov, JUnitTestConnect.ConfigGroupTypeServer, JUnitTestConnect.ConnectionMinPingTimeout);
			TestUtilities.checkResult("ConnectionMinPingTimeout == 4000", intLongValue == 4000);
			boolValue = JUnitTestConnect.activeConfigGetBooleanValue(prov, JUnitTestConnect.ConfigGroupTypeServer, JUnitTestConnect.TcpNodelay);
			TestUtilities.checkResult("TcpNodelay == 0", boolValue == false);
			boolValue = JUnitTestConnect.activeConfigGetBooleanValue(prov, JUnitTestConnect.ConfigGroupTypeServer, JUnitTestConnect.DirectWrite);
			TestUtilities.checkResult("DirectWrite == 1", boolValue == true);
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(prov, JUnitTestConnect.ConfigGroupTypeServer, JUnitTestConnect.HighWaterMark);
			TestUtilities.checkResult("HighWaterMark == 5000", intLongValue == 5000);
			String chanPort = JUnitTestConnect.activeConfigGetStringValue(prov, JUnitTestConnect.ConfigGroupTypeServer, JUnitTestConnect.Port);
			TestUtilities.checkResult("Port == 8003", chanPort.contentEquals("8003"));
			
			
			//retrieve directory
			strValue = JUnitTestConnect.activeConfigGetStringValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.DirectoryName);
			TestUtilities.checkResult("DirectoryName == Directory_2", strValue.contentEquals("Directory_2"));
			
			List<Service> services = JUnitTestConnect.activeConfigGetService(prov, true);
			TestUtilities.checkResult("services.size() == 1", services.size() == 1);
			
			/*********retrieve first service *************/
			System.out.println("\nRetrieving DIRECT_FEED service configuration values "); 
			Service temp = services.get(0);
			TestUtilities.checkResult("temp != null", temp != null);
			
			TestUtilities.checkResult("checkHasInfo() == true", temp.checkHasInfo() == true);
			
			TestUtilities.checkResult("serviceId == 1", temp.serviceId() == 1);
			ServiceInfo info = temp.info();
			TestUtilities.checkResult("infoFilter.serviceName == DIRECT_FEED", info.serviceName().toString().equals("DIRECT_FEED"));
			TestUtilities.checkResult("infoFilter.vendorName == Vendor", info.checkHasVendor() && info.vendor().toString().equals("Vendor"));
			TestUtilities.checkResult("infoFilter.isSource == true", info.checkHasIsSource() && info.isSource() == 1);
			TestUtilities.checkResult("infoFilter.itemList == #.itemlist2", info.checkHasItemList() && info.itemList().toString().equals("#.itemlist2"));
			TestUtilities.checkResult("infoFilter.acceptingConsumerStatus == 1", info.checkHasAcceptingConsumerStatus() && info.acceptingConsumerStatus() == 1);
			TestUtilities.checkResult("infoFilter.supportsQosRange == 1", info.checkHasSupportsQosRange() && info.supportsQosRange() == 1);
			TestUtilities.checkResult("infoFilter.supportsOutOfBandSnapshots == 1", info.checkHasSupportsOutOfBandSnapshots() && info.supportsOutOfBandSnapshots() == 1);
			
			//retrieve capabilities
			List<Long> capabilities =  info.capabilitiesList();
			TestUtilities.checkResult("capabilities.size() == 4", capabilities.size() == 4);
			TestUtilities.checkResult("capabilities.get(0) == MMT_MARKET_BY_ORDER", capabilities.get(0) == 7);
			TestUtilities.checkResult("capabilities.get(1) == MMT_MARKET_BY_PRICE", capabilities.get(1) == 8);
			TestUtilities.checkResult("capabilities.get(2) == MMT_MARKET_MAKER", capabilities.get(2) == 9);
			TestUtilities.checkResult("capabilities.get(3) == 130", capabilities.get(3) == 130);
			
			//retrieve qos
			TestUtilities.checkResult("info.checkHasQos()", info.checkHasQos());
			List<Qos> qos = info.qosList();
			TestUtilities.checkResult("qos.size() == 2", qos.size() == 2);
			TestUtilities.checkResult("qos.get(0).rate() == QosRates.TIME_CONFLATED", qos.get(0).rate() == QosRates.TIME_CONFLATED);
			TestUtilities.checkResult("qos.get(0).rateInfo() == 200", qos.get(0).rateInfo() == 200);
			TestUtilities.checkResult("qos.get(0).timeliness() == QosTimeliness.DELAYED", qos.get(0).timeliness() == QosTimeliness.DELAYED);
			TestUtilities.checkResult("qos.get(0).timeInfo() == 200", qos.get(0).timeInfo() == 200);
			TestUtilities.checkResult("qos.get(1).rate() == QosRates.JIT_CONFLATED", qos.get(1).rate() == QosRates.JIT_CONFLATED);
			TestUtilities.checkResult("qos.get(1).timeliness() == QosTimeliness.DELAYED_UNKNOWN", qos.get(1).timeliness() == QosTimeliness.DELAYED_UNKNOWN);
			
			//retrieve dictionary provided/used by this service
			TestUtilities.checkResult("info.checkHasDictionariesProvided() == true", info.checkHasDictionariesProvided());
			TestUtilities.checkResult("info.checkHasDictionariesUsed() == true", info.checkHasDictionariesUsed());
			List<String> dictProvided = info.dictionariesProvidedList();
			TestUtilities.checkResult("dictProvided.size() == 2", dictProvided.size() == 2);
			TestUtilities.checkResult("info.dictionariesProvidedList().get(0) == ./ConfigDB3_RWFFld",dictProvided.get(0).equals("./ConfigDB3_RWFFld"));
			TestUtilities.checkResult("info.dictionariesProvidedList().get(1) == ./ConfigDB3_RWFEnum",dictProvided.get(1).equals("./ConfigDB3_RWFEnum"));
			List<String> dictUsed = info.dictionariesUsedList();
			TestUtilities.checkResult("dictUsed.size() == 2", dictUsed.size() == 2);
			TestUtilities.checkResult("info.dictionariesUsedList().get(0) == RWFFld",dictUsed.get(0).equals("RWFFld"));
			TestUtilities.checkResult("info.dictionariesUsedList().get(1) == RWFEnum",dictUsed.get(1).equals("RWFEnum"));
			
			//retrieve dictionary defined in directory
			int serviceId = 1;
			int dictId = 0;
			strValue = JUnitTestConnect.activeConfigGetServiceDict(prov,JUnitTestConnect.DictionaryType, serviceId, dictId, true);
			TestUtilities.checkResult("dictProvided DictionaryType = Dictionary::FileDictionary",strValue != null && strValue.equals("FileDictionary"));
			strValue = JUnitTestConnect.activeConfigGetServiceDict(prov,JUnitTestConnect.DictionaryName, serviceId, dictId, true);
			TestUtilities.checkResult("dictProvided DictionaryName = Dictionary_2",strValue != null && strValue.equals("Dictionary_2"));
			strValue = JUnitTestConnect.activeConfigGetServiceDict(prov,JUnitTestConnect.DictionaryRDMFieldDictFileName, serviceId, dictId, true);
			TestUtilities.checkResult("dictProvided DictionaryRDMFieldDictFileName = ./ConfigDB3_RDMFieldDictionary",strValue != null && strValue.equals("./ConfigDB3_RDMFieldDictionary"));
			strValue = JUnitTestConnect.activeConfigGetServiceDict(prov,JUnitTestConnect.DictionaryEnumTypeDefFileName, serviceId, dictId, true);
			TestUtilities.checkResult("dictProvided DictionaryEnumTypeDefFileName = ./ConfigDB3_enumtype.def",strValue != null && strValue.equals("./ConfigDB3_enumtype.def"));
			strValue = JUnitTestConnect.activeConfigGetServiceDict(prov,JUnitTestConnect.DictionaryRdmFieldDictionaryItemName, serviceId, dictId, true);
			TestUtilities.checkResult("dictProvided DictionaryRdmFieldDictionaryItemName = ./ConfigDB3_RWFFld",strValue != null && strValue.equals("./ConfigDB3_RWFFld"));
			strValue = JUnitTestConnect.activeConfigGetServiceDict(prov,JUnitTestConnect.DictionaryEnumTypeDefItemName, serviceId, dictId, true);
			TestUtilities.checkResult("dictProvided DictionaryEnumTypeDefItemName = ./ConfigDB3_RWFEnum",strValue != null && strValue.equals("./ConfigDB3_RWFEnum"));
			
			strValue = JUnitTestConnect.activeConfigGetServiceDict(prov,JUnitTestConnect.DictionaryType, serviceId, dictId, false);
			TestUtilities.checkResult("dictUsed DictionaryType = Dictionary::FileDictionary",strValue != null && strValue.equals("FileDictionary"));
			strValue = JUnitTestConnect.activeConfigGetServiceDict(prov,JUnitTestConnect.DictionaryName, serviceId, dictId, false);
			TestUtilities.checkResult("dictUsed DictionaryName = Dictionary_4",strValue != null && strValue.equals("Dictionary_3"));
			strValue = JUnitTestConnect.activeConfigGetServiceDict(prov,JUnitTestConnect.DictionaryRDMFieldDictFileName, serviceId, dictId, false);
			TestUtilities.checkResult("dictUsed DictionaryRDMFieldDictFileName = ./RDMFieldDictionary",strValue != null && strValue.equals("./RDMFieldDictionary"));
			strValue = JUnitTestConnect.activeConfigGetServiceDict(prov,JUnitTestConnect.DictionaryEnumTypeDefFileName, serviceId, dictId, false);
			TestUtilities.checkResult("dictUsed DictionaryEnumTypeDefFileName = ./enumtype.def",strValue != null && strValue.equals("./enumtype.def"));
			strValue = JUnitTestConnect.activeConfigGetServiceDict(prov,JUnitTestConnect.DictionaryRdmFieldDictionaryItemName, serviceId, dictId, false);
			TestUtilities.checkResult("dictUsed DictionaryRdmFieldDictionaryItemName = RWFFld",strValue != null && strValue.equals("RWFFld"));
			strValue = JUnitTestConnect.activeConfigGetServiceDict(prov,JUnitTestConnect.DictionaryEnumTypeDefItemName, serviceId, dictId, false);
			TestUtilities.checkResult("dictUsed DictionaryEnumTypeDefItemName = RWFEnum",strValue != null && strValue.equals("RWFEnum"));
			
			//retrieve Service state
			TestUtilities.checkResult("checkHasState() == true", temp.checkHasState() == true);
			ServiceState state = temp.state();
			TestUtilities.checkResult("state.checkHasAcceptingRequests() == true", state.checkHasAcceptingRequests());
			TestUtilities.checkResult("state.acceptingRequests() == 0", state.acceptingRequests() == 0);
			TestUtilities.checkResult("state.serviceState() == 0", state.serviceState() == 0);
			TestUtilities.checkResult("checkHasStatus() = true", state.checkHasStatus());
			TestUtilities.checkResult("state.status().streamState() == 3", state.status().streamState() == 3);
			TestUtilities.checkResult("state.status().dataState() == 2", state.status().dataState() == 2);
			TestUtilities.checkResult("state.status().code() == 29", state.status().code() == 29);
			TestUtilities.checkResult("state.status().text() == dacsDown", state.status().text() != null &&  state.status().text().toString().equals("dacsDown"));

			prov = null;
		}
		catch ( OmmException excp)
		{
			System.out.println(excp.getMessage());
			TestUtilities.checkResult("Receiving exception, test failed.", false );
		}
	}
}

public void testMergCfgBetweenFileAndProgrammaticConfigForNiProv()
{
	TestUtilities.printTestHead("testMergCfgBetweenFileAndProgrammaticConfigForNiProv","Test merge all NiProvider configuration parameters between config file and programmatically config");
	
	//two testcases:
	//test case 1: NOT loading EmaConfig file from working dir.
	//test case 2: loading EmaConfigTest file
	for (int testCase = 0; testCase < 2; testCase++)
	{
		System.out.println(" #####Now it is running test case " + testCase);

		Map outermostMap = EmaFactory.createMap();
		Map innerMap = EmaFactory.createMap();
		ElementList elementList = EmaFactory.createElementList();
		ElementList innerElementList = EmaFactory.createElementList();
		try
		{
			innerElementList.add(EmaFactory.createElementEntry().ascii("Channel", "Channel_2"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("Directory", "Directory_2"));
			innerElementList.add(EmaFactory.createElementEntry().intValue("ItemCountHint", 9000));
			innerElementList.add(EmaFactory.createElementEntry().intValue("ServiceCountHint", 9000));
			innerElementList.add(EmaFactory.createElementEntry().intValue("MergeSourceDirectoryStreams", 0));
			innerElementList.add(EmaFactory.createElementEntry().intValue("RefreshFirstRequired", 0));
			innerElementList.add(EmaFactory.createElementEntry().intValue("RecoverUserSubmitSourceDirectory", 0));
			innerElementList.add(EmaFactory.createElementEntry().intValue("RemoveItemsOnDisconnect", 0));
			innerElementList.add(EmaFactory.createElementEntry().intValue("LoginRequestTimeOut", 50000));
			innerElementList.add(EmaFactory.createElementEntry().intValue("ReconnectAttemptLimit", 70));
			innerElementList.add(EmaFactory.createElementEntry().intValue("ReconnectMinDelay", 7000));
			innerElementList.add(EmaFactory.createElementEntry().intValue("ReconnectMaxDelay", 7000));
			innerElementList.add(EmaFactory.createElementEntry().intValue("RequestTimeout", 9000));
			innerElementList.add(EmaFactory.createElementEntry().intValue("DispatchTimeoutApiThread", 5656));
			innerElementList.add(EmaFactory.createElementEntry().intValue("MaxDispatchCountApiThread", 900));
			innerElementList.add(EmaFactory.createElementEntry().intValue("MaxDispatchCountUserThread", 900));
			innerElementList.add(EmaFactory.createElementEntry().intValue("XmlTraceToStdout", 0));
			innerMap.add(EmaFactory.createMapEntry().keyAscii( "Provider_2", MapEntry.MapAction.ADD, innerElementList));
			innerElementList.clear();
			
			elementList.add(EmaFactory.createElementEntry().map("NiProviderList", innerMap));
			innerMap.clear();

			outermostMap.add(EmaFactory.createMapEntry().keyAscii( "NiProviderGroup", MapEntry.MapAction.ADD, elementList));
			elementList.clear();
			
			innerElementList.add(EmaFactory.createElementEntry().ascii("ChannelType", "ChannelType::RSSL_SOCKET"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("CompressionType", "CompressionType::LZ4"));
			innerElementList.add(EmaFactory.createElementEntry().intValue("GuaranteedOutputBuffers", 7000));
			innerElementList.add(EmaFactory.createElementEntry().intValue("NumInputBuffers", 5000));
			innerElementList.add(EmaFactory.createElementEntry().intValue("SysRecvBufSize", 550000));
			innerElementList.add(EmaFactory.createElementEntry().intValue("SysSendBufSize", 700000));
			innerElementList.add(EmaFactory.createElementEntry().intValue("CompressionThreshold", 12758));
			innerElementList.add(EmaFactory.createElementEntry().intValue("ConnectionPingTimeout", 70000));
			innerElementList.add(EmaFactory.createElementEntry().ascii("InterfaceName", "localhost"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("Host", "10.0.0.1"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("Port", "8001"));
			innerElementList.add(EmaFactory.createElementEntry().intValue("TcpNodelay", 1));
			innerElementList.add(EmaFactory.createElementEntry().intValue("DirectWrite", 1));
			innerElementList.add(EmaFactory.createElementEntry().intValue("HighWaterMark", 5000));
			innerMap.add(EmaFactory.createMapEntry().keyAscii( "Channel_2", MapEntry.MapAction.ADD, innerElementList)); 
			innerElementList.clear();

			elementList.add(EmaFactory.createElementEntry().map("ChannelList", innerMap));
			innerMap.clear();

			outermostMap.add(EmaFactory.createMapEntry().keyAscii( "ChannelGroup", MapEntry.MapAction.ADD, elementList));
			elementList.clear();

			/////////////////////////////////////
			//DirectoryGroup
			Map serviceMap = EmaFactory.createMap();
			ElementList infoElementList = EmaFactory.createElementList();
			ElementList stateElementList = EmaFactory.createElementList();
			OmmArray infoArray = EmaFactory.createOmmArray();
			Series qosSeries = EmaFactory.createSeries();

			//encode service1
			infoElementList.add(EmaFactory.createElementEntry().intValue("ServiceId", 3));
			infoElementList.add(EmaFactory.createElementEntry().ascii("Vendor", "Vendor"));
			infoElementList.add(EmaFactory.createElementEntry().intValue("IsSource", 1));
			infoElementList.add(EmaFactory.createElementEntry().intValue("AcceptingConsumerStatus", 1));
			infoElementList.add(EmaFactory.createElementEntry().intValue("SupportsQoSRange", 1));
			infoElementList.add(EmaFactory.createElementEntry().intValue("SupportsOutOfBandSnapshots", 1));
			infoElementList.add(EmaFactory.createElementEntry().ascii("ItemList", "#.itemlist2"));

			infoArray.add(EmaFactory.createOmmArrayEntry().ascii("8"));
			infoArray.add(EmaFactory.createOmmArrayEntry().ascii("9"));
			infoArray.add(EmaFactory.createOmmArrayEntry().ascii("MMT_MARKET_BY_ORDER"));
			infoArray.add(EmaFactory.createOmmArrayEntry().ascii("130"));
			infoElementList.add(EmaFactory.createElementEntry().array("Capabilities", infoArray));
			infoArray.clear();
			
			infoArray.add(EmaFactory.createOmmArrayEntry().ascii("Dictionary_2"));
			infoElementList.add(EmaFactory.createElementEntry().array("DictionariesUsed", infoArray));
			infoArray.clear();
			
			innerElementList.add(EmaFactory.createElementEntry().intValue("Timeliness", 200));
			innerElementList.add(EmaFactory.createElementEntry().intValue("Rate", 200));
			qosSeries.add(EmaFactory.createSeriesEntry().elementList(innerElementList));
			innerElementList.clear();
			
			innerElementList.add(EmaFactory.createElementEntry().ascii("Timeliness", "Timeliness::InexactDelayed"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("Rate", "Rate::JustInTimeConflated"));
			qosSeries.add(EmaFactory.createSeriesEntry().elementList(innerElementList));
			innerElementList.clear();
			
			infoElementList.add(EmaFactory.createElementEntry().series("QoS", qosSeries));
			qosSeries.clear();
			
			stateElementList.add(EmaFactory.createElementEntry().intValue("ServiceState", 0));
			stateElementList.add(EmaFactory.createElementEntry().intValue("AcceptingRequests", 0));
			
			innerElementList.add(EmaFactory.createElementEntry().ascii("StreamState", "StreamState::CloseRecover"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("DataState", "DataState::Suspect"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("StatusCode", "StatusCode::DacsDown"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("StatusText", "dacsDown"));
			stateElementList.add(EmaFactory.createElementEntry().elementList("Status", innerElementList));
			innerElementList.clear();
						
			innerElementList.add(EmaFactory.createElementEntry().elementList("InfoFilter", infoElementList));
			infoElementList.clear();
			innerElementList.add(EmaFactory.createElementEntry().elementList("StateFilter", stateElementList));
			stateElementList.clear();
			
			serviceMap.add(EmaFactory.createMapEntry().keyAscii( "DIRECT_FEED", MapEntry.MapAction.ADD, innerElementList));
			innerElementList.clear();
						
			innerMap.add(EmaFactory.createMapEntry().keyAscii( "Directory_2", MapEntry.MapAction.ADD, serviceMap));
			serviceMap.clear();

			elementList.add(EmaFactory.createElementEntry().map("DirectoryList", innerMap));
			innerMap.clear();
			
			outermostMap.add(EmaFactory.createMapEntry().keyAscii( "DirectoryGroup", MapEntry.MapAction.ADD, elementList));
			elementList.clear();

			String localConfigPath = null;
			if (testCase == 1)
				localConfigPath = "./src/test/resources/com/rtsdk/ema/unittest/EmaFileConfigTests/EmaConfigTest.xml";

			System.out.println("Using Ema Config: " + localConfigPath);
			
			OmmProvider prov = null;
			if (testCase == 0)
				prov = JUnitTestConnect.createOmmNiProvider(EmaFactory.createOmmNiProviderConfig().config(outermostMap).providerName("Provider_2"));
			else if (testCase == 1)
				prov = JUnitTestConnect.createOmmNiProvider(EmaFactory.createOmmNiProviderConfig(localConfigPath).config(outermostMap).providerName("Provider_2"));
			
			String defaultProvName = JUnitTestConnect.activeConfigGetStringValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.NiProviderName, -1);
			TestUtilities.checkResult("DefaultProvider value != null", defaultProvName != null);
			TestUtilities.checkResult("DefaultProvider value == Provider_2", defaultProvName.contentEquals("Provider_2") );
			String provChannelVal = JUnitTestConnect.activeConfigGetStringValue(prov, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelName, 0);
			TestUtilities.checkResult("Channel value != null", provChannelVal != null);
			TestUtilities.checkResult("Channel value == Channel_2", provChannelVal.contentEquals("Channel_2") );

			boolean boolValue = JUnitTestConnect.activeConfigGetBooleanValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.NiProviderMergeSourceDirectoryStreams, -1);
			TestUtilities.checkResult("MergeSourceDirectoryStreams == 0", boolValue == false);
			boolValue = JUnitTestConnect.activeConfigGetBooleanValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.NiProviderRecoverUserSubmitSourceDirectory, -1);
			TestUtilities.checkResult("RecoverUserSubmitSourceDirectory == 0", boolValue == false);
			boolValue = JUnitTestConnect.activeConfigGetBooleanValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.NiProviderRemoveItemsOnDisconnect, -1);
			TestUtilities.checkResult("RemoveItemsOnDisconnect == 0", boolValue == false);
			boolValue = JUnitTestConnect.activeConfigGetBooleanValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.NiProviderRefreshFirstRequired, -1);
			TestUtilities.checkResult("RefreshFirstRequired == 0", boolValue == false);
			long intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.ItemCountHint, -1);
			TestUtilities.checkResult("ItemCountHint value == 9000", intLongValue == 9000 );
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.ServiceCountHint, -1);
			TestUtilities.checkResult("ServiceCountHint value == 9000", intLongValue == 9000 );
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.RequestTimeout, -1);
			TestUtilities.checkResult("RequestTimeout value == 9000", intLongValue == 9000 );
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.LoginRequestTimeOut, -1);
			TestUtilities.checkResult("LoginRequestTimeOut value == 50000", intLongValue == 50000 );
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.DispatchTimeoutApiThread, -1);
			TestUtilities.checkResult("DispatchTimeoutApiThread value == 5656", intLongValue == 5656 );
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.MaxDispatchCountApiThread, -1);
			TestUtilities.checkResult("MaxDispatchCountApiThread value == 900", intLongValue == 900 );
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.MaxDispatchCountUserThread, -1);
			TestUtilities.checkResult("MaxDispatchCountUserThread value == 900", intLongValue == 900 );
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.ReconnectAttemptLimit, -1);
			TestUtilities.checkResult("ReconnectAttemptLimit == 70", intLongValue == 70);
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.ReconnectMinDelay, -1);
			TestUtilities.checkResult("ReconnectMinDelay == 7000", intLongValue == 7000);
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.ReconnectMaxDelay, -1);
			TestUtilities.checkResult("ReconnectMaxDelay == 7000", intLongValue == 7000);

			boolValue = JUnitTestConnect.activeConfigGetBooleanValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.XmlTraceToStdout, -1);
			TestUtilities.checkResult("XmlTraceToStdout == 0", boolValue == false);
			
			// Check Channel configuration:
			// Check Channel_1 configuration.
			provChannelVal = "Channel_2";
			System.out.println("\nRetrieving Channel_2 configuration values "); 
			String consChannelVal = JUnitTestConnect.activeConfigGetStringValue(prov, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelName, 0);
			TestUtilities.checkResult("Channel value != null", consChannelVal != null);
			TestUtilities.checkResult("Channel value == Channel_2", consChannelVal.contentEquals("Channel_2") );
			int channelConnType = JUnitTestConnect.activeConfigGetIntLongValue(prov, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelType, 0);
			TestUtilities.checkResult("ChannelConnType == ChannelType::RSSL_SOCKET", channelConnType == ChannelTypeSocket);
		
			String strValue = JUnitTestConnect.activeConfigGetStringValue(prov, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.InterfaceName, 0);
			TestUtilities.checkResult("InterfaceName == localhost", strValue.contentEquals("localhost"));
			
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(prov, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.CompressionType, 0);
			TestUtilities.checkResult("CompressionType == CompressionType::LZ4", intLongValue == CompressionTypeLZ4);
			
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(prov, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.GuaranteedOutputBuffers, 0);
			TestUtilities.checkResult("GuaranteedOutputBuffers == 7000", intLongValue == 7000);
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(prov, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.NumInputBuffers, 0);
			TestUtilities.checkResult("NumInputBuffers == 5000", intLongValue == 5000);
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(prov, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.SysRecvBufSize, 0);
			TestUtilities.checkResult("SysRecvBufSize == 550000", intLongValue == 550000);
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(prov, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.SysSendBufSize, 0);
			TestUtilities.checkResult("SysSendBufSize == 700000", intLongValue == 700000);
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(prov, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.CompressionThreshold, 0);
			TestUtilities.checkResult("CompressionThreshold == 12758", intLongValue == 12758);
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(prov, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ConnectionPingTimeout, 0);
			TestUtilities.checkResult("ConnectionPingTimeout == 70000", intLongValue == 70000);
			boolValue = JUnitTestConnect.activeConfigGetBooleanValue(prov, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.TcpNodelay, 0);
			TestUtilities.checkResult("TcpNodelay == 1", boolValue == true);
			boolValue = JUnitTestConnect.activeConfigGetBooleanValue(prov, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.DirectWrite, 0);
			TestUtilities.checkResult("DirectWrite == 1", boolValue == true);
			intLongValue = JUnitTestConnect.activeConfigGetIntLongValue(prov, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.HighWaterMark, 0);
			TestUtilities.checkResult("HighWaterMark == 5000", intLongValue == 5000);
			strValue = JUnitTestConnect.activeConfigGetStringValue(prov, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.Host, 0);
			TestUtilities.checkResult("Host == 10.0.0.1", strValue.contentEquals("10.0.0.1"));
			strValue = JUnitTestConnect.activeConfigGetStringValue(prov, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.Port, 0);
			TestUtilities.checkResult("Port == 8001", strValue.contentEquals("8001"));
			
			
			//retrieve directory
			strValue = JUnitTestConnect.activeConfigGetStringValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.DirectoryName, -1);
			TestUtilities.checkResult("DirectoryName == Directory_2", strValue.contentEquals("Directory_2"));
			
			List<Service> services = JUnitTestConnect.activeConfigGetService(prov, false);
			TestUtilities.checkResult("services.size() == 1", services.size() == 1);
			
			/*********retrieve first service *************/
			System.out.println("\nRetrieving DIRECT_FEED service configuration values "); 
			Service temp = services.get(0);
			TestUtilities.checkResult("temp != null", temp != null);
			
			TestUtilities.checkResult("checkHasInfo() == true", temp.checkHasInfo() == true);
			
			TestUtilities.checkResult("serviceId == 3", temp.serviceId() == 3);
			ServiceInfo info = temp.info();
			TestUtilities.checkResult("infoFilter.serviceName == DIRECT_FEED", info.serviceName().toString().equals("DIRECT_FEED"));
			TestUtilities.checkResult("infoFilter.vendorName == Vendor", info.checkHasVendor() && info.vendor().toString().equals("Vendor"));
			TestUtilities.checkResult("infoFilter.isSource == 1", info.checkHasIsSource() && info.isSource() == 1);
			TestUtilities.checkResult("infoFilter.itemList == #.itemlist2", info.checkHasItemList() && info.itemList().toString().equals("#.itemlist2"));
			TestUtilities.checkResult("infoFilter.acceptingConsumerStatus == 1", info.checkHasAcceptingConsumerStatus() && info.acceptingConsumerStatus() == 1);
			TestUtilities.checkResult("infoFilter.supportsQosRange == 1", info.checkHasSupportsQosRange() && info.supportsQosRange() == 1);
			TestUtilities.checkResult("infoFilter.supportsOutOfBandSnapshots == 1", info.checkHasSupportsOutOfBandSnapshots() && info.supportsOutOfBandSnapshots() == 1);
			
			//retrieve capabilities
			List<Long> capabilities =  info.capabilitiesList();
			TestUtilities.checkResult("capabilities.size() == 4", capabilities.size() == 4);
			TestUtilities.checkResult("capabilities.get(0) == MMT_MARKET_BY_ORDER", capabilities.get(0) == 7);
			TestUtilities.checkResult("capabilities.get(1) == MMT_MARKET_BY_PRICE", capabilities.get(1) == 8);
			TestUtilities.checkResult("capabilities.get(2) == MMT_MARKET_MAKER", capabilities.get(2) == 9);
			TestUtilities.checkResult("capabilities.get(3) == 130", capabilities.get(3) == 130);
			
			//retrieve qos
			TestUtilities.checkResult("info.checkHasQos()", info.checkHasQos());
			List<Qos> qos = info.qosList();
			TestUtilities.checkResult("qos.size() == 2", qos.size() == 2);
			TestUtilities.checkResult("qos.get(0).rate() == QosRates.TIME_CONFLATED", qos.get(0).rate() == QosRates.TIME_CONFLATED);
			TestUtilities.checkResult("qos.get(0).rateInfo() == 200", qos.get(0).rateInfo() == 200);
			TestUtilities.checkResult("qos.get(0).timeliness() == QosTimeliness.DELAYED", qos.get(0).timeliness() == QosTimeliness.DELAYED);
			TestUtilities.checkResult("qos.get(0).timeInfo() == 200", qos.get(0).timeInfo() == 200);
			TestUtilities.checkResult("qos.get(1).rate() == QosRates.TICK_BY_TICK", qos.get(1).rate() == QosRates.JIT_CONFLATED);
			TestUtilities.checkResult("qos.get(1).timeliness() == QosTimeliness.REALTIME", qos.get(1).timeliness() == QosTimeliness.DELAYED_UNKNOWN);
			
			//retrieve dictionary provided/used by this service
			if (testCase == 0)
			{
				TestUtilities.checkResult("info.checkHasDictionariesProvided() == false", !info.checkHasDictionariesProvided());
				TestUtilities.checkResult("info.checkHasDictionariesUsed() == true", info.checkHasDictionariesUsed());
				List<String> dictProvided = info.dictionariesProvidedList();
				TestUtilities.checkResult("dictProvided.size() == 0", dictProvided.size() == 0);
				List<String> dictUsed = info.dictionariesUsedList();
				TestUtilities.checkResult("dictUsed.size() == 2", dictUsed.size() == 2);
				TestUtilities.checkResult("info.dictionariesUsedList().get(0) == RWFFld",dictUsed.get(0).equals("RWFFld"));
				TestUtilities.checkResult("info.dictionariesUsedList().get(1) == RWFEnum",dictUsed.get(1).equals("RWFEnum"));
			}
			else if (testCase == 1)
			{
				//there is one dictionariesProvided config from file.
				TestUtilities.checkResult("info.checkHasDictionariesProvided() == true", info.checkHasDictionariesProvided());
				TestUtilities.checkResult("info.checkHasDictionariesUsed() == true", info.checkHasDictionariesUsed());
				List<String> dictProvided = info.dictionariesProvidedList();
				TestUtilities.checkResult("dictProvided.size() == 2", dictProvided.size() == 2);
				TestUtilities.checkResult("info.dictionariesProvidedList().get(0) == RWFFld",dictProvided.get(0).equals("RWFFld"));
				TestUtilities.checkResult("info.dictionariesProvidedList().get(1) == RWFEnum",dictProvided.get(1).equals("RWFEnum"));
				List<String> dictUsed = info.dictionariesUsedList();
				TestUtilities.checkResult("dictUsed.size() == 2", dictUsed.size() == 2);
				TestUtilities.checkResult("info.dictionariesUsedList().get(0) == RWFFld",dictUsed.get(0).equals("RWFFld"));
				TestUtilities.checkResult("info.dictionariesUsedList().get(1) == RWFEnum",dictUsed.get(1).equals("RWFEnum"));
			}
			
			//retrieve Service state
			TestUtilities.checkResult("checkHasState() == true", temp.checkHasState() == true);
			ServiceState state = temp.state();
			TestUtilities.checkResult("state.checkHasAcceptingRequests() == true", state.checkHasAcceptingRequests());
			TestUtilities.checkResult("state.acceptingRequests() == 0", state.acceptingRequests() == 0);
			TestUtilities.checkResult("state.serviceState() == 0", state.serviceState() == 0);
			TestUtilities.checkResult("checkHasStatus() = true", state.checkHasStatus());
			TestUtilities.checkResult("state.status().streamState() == 3", state.status().streamState() == 3);
			TestUtilities.checkResult("state.status().dataState() == 2", state.status().dataState() == 2);
			TestUtilities.checkResult("state.status().code() == 29", state.status().code() == 29);
			TestUtilities.checkResult("state.status().text() == dacsDown", state.status().text() != null &&  state.status().text().toString().equals("dacsDown"));

			prov = null;
		}
		catch ( OmmException excp)
		{
			System.out.println(excp.getMessage());
			TestUtilities.checkResult("Receiving exception, test failed.", false );
		}
	}
}	

public void testSetInstanceNameByFunctionCall()
{
	TestUtilities.printTestHead("testSetInstanceNameByFunctionCall","Test setting consumer or provider name through function call");
	
	//three testcases:
	//test case 1: set niprovideName through function call.
	//test case 2: set iprovideName through function call.
	//test case 3: set consumerName through function call.
	for (int testCase = 0; testCase < 3; testCase++)
	{
		System.out.println(" #####Now it is running test case " + testCase);

		Map outermostMap = EmaFactory.createMap();
		Map innerMap = EmaFactory.createMap();
		ElementList elementList = EmaFactory.createElementList();
		ElementList innerElementList = EmaFactory.createElementList();
		try
		{
			if (testCase == 0)
			{
				elementList.add(EmaFactory.createElementEntry().ascii("DefaultNiProvider", "Provider_5"));
				
				innerElementList.add(EmaFactory.createElementEntry().ascii("Channel", "Channel_5"));
				innerElementList.add(EmaFactory.createElementEntry().ascii("Directory", "Directory_5"));
				innerMap.add(EmaFactory.createMapEntry().keyAscii( "Provider_5", MapEntry.MapAction.ADD, innerElementList));
				innerElementList.clear();
				
				elementList.add(EmaFactory.createElementEntry().map("NiProviderList", innerMap));
				innerMap.clear();
	
				outermostMap.add(EmaFactory.createMapEntry().keyAscii( "NiProviderGroup", MapEntry.MapAction.ADD, elementList));
				elementList.clear();
			}
			else if (testCase == 1)
			{
				elementList.add(EmaFactory.createElementEntry().ascii("DefaultIProvider", "Provider_5"));
				
				innerElementList.add(EmaFactory.createElementEntry().ascii("Server", "Server_5"));
				innerElementList.add(EmaFactory.createElementEntry().ascii("Directory", "Directory_5"));
				innerMap.add(EmaFactory.createMapEntry().keyAscii( "Provider_5", MapEntry.MapAction.ADD, innerElementList));
				innerElementList.clear();
				
				elementList.add(EmaFactory.createElementEntry().map("IProviderList", innerMap));
				innerMap.clear();
	
				outermostMap.add(EmaFactory.createMapEntry().keyAscii( "IProviderGroup", MapEntry.MapAction.ADD, elementList));
				elementList.clear();
			}
			else if (testCase == 2)
			{
				elementList.add(EmaFactory.createElementEntry().ascii("DefaultConsumer", "Consumer_5"));
				
				innerElementList.add(EmaFactory.createElementEntry().ascii("Channel", "Channel_5"));
				innerMap.add(EmaFactory.createMapEntry().keyAscii( "Consumer_5", MapEntry.MapAction.ADD, innerElementList));
				innerElementList.clear();
				
				elementList.add(EmaFactory.createElementEntry().map("ConsumerList", innerMap));
				innerMap.clear();
	
				outermostMap.add(EmaFactory.createMapEntry().keyAscii( "ConsumerGroup", MapEntry.MapAction.ADD, elementList));
				elementList.clear();
			}
			
			innerElementList.add(EmaFactory.createElementEntry().ascii("ChannelType", "ChannelType::RSSL_SOCKET"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("Host", "host5"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("Port", "port5"));
			innerElementList.add(EmaFactory.createElementEntry().intValue("TcpNodelay", 1));
			innerMap.add(EmaFactory.createMapEntry().keyAscii( "Channel_5", MapEntry.MapAction.ADD, innerElementList)); 
			innerElementList.clear();

			elementList.add(EmaFactory.createElementEntry().map("ChannelList", innerMap));
			innerMap.clear();

			outermostMap.add(EmaFactory.createMapEntry().keyAscii( "ChannelGroup", MapEntry.MapAction.ADD, elementList));
			elementList.clear();
			
			innerElementList.add(EmaFactory.createElementEntry().ascii("ServerType", "ChannelType::RSSL_SOCKET"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("Port", "port5"));
			innerElementList.add(EmaFactory.createElementEntry().intValue("TcpNodelay", 1));
			innerMap.add(EmaFactory.createMapEntry().keyAscii( "Server_5", MapEntry.MapAction.ADD, innerElementList)); 
			innerElementList.clear();

			elementList.add(EmaFactory.createElementEntry().map("ServerList", innerMap));
			innerMap.clear();

			outermostMap.add(EmaFactory.createMapEntry().keyAscii( "ServerGroup", MapEntry.MapAction.ADD, elementList));
			elementList.clear();
			

			String localConfigPath = null;
			localConfigPath = "./src/test/resources/com/rtsdk/ema/unittest/EmaFileConfigTests/EmaConfigTest.xml";
			System.out.println("Using Ema Config: " + localConfigPath);
			
			if (testCase == 0)
			{
			
				OmmProvider prov = null;
				prov = JUnitTestConnect.createOmmNiProvider(EmaFactory.createOmmNiProviderConfig(localConfigPath).config(outermostMap).providerName("Provider_1"));
				
				String defaultProvName = JUnitTestConnect.activeConfigGetStringValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.NiProviderName, -1);
				TestUtilities.checkResult("DefaultProvider value != null", defaultProvName != null);
				TestUtilities.checkResult("DefaultProvider value == Provider_1", defaultProvName.contentEquals("Provider_1") );
				String provChannelVal = JUnitTestConnect.activeConfigGetStringValue(prov, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelName, 0);
				TestUtilities.checkResult("Channel value != null", provChannelVal != null);
				TestUtilities.checkResult("Channel value == Channel_10", provChannelVal.contentEquals("Channel_10") );
				String strValue = JUnitTestConnect.activeConfigGetStringValue(prov, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.Host, 0);
				TestUtilities.checkResult("Host == localhost", strValue.contentEquals("localhost"));
				strValue = JUnitTestConnect.activeConfigGetStringValue(prov, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.Port, 0);
				TestUtilities.checkResult("Port == 14003", strValue.contentEquals("14003"));
				prov = null;
			}
			else if (testCase == 1)
			{
			
				OmmProvider prov = null;
				prov = JUnitTestConnect.createOmmIProvider(EmaFactory.createOmmIProviderConfig(localConfigPath).config(outermostMap).providerName("Provider_1"));
				
				String defaultProvName = JUnitTestConnect.activeConfigGetStringValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.IProviderName);
				TestUtilities.checkResult("DefaultProvider value != null", defaultProvName != null);
				TestUtilities.checkResult("DefaultProvider value == Provider_1", defaultProvName.contentEquals("Provider_1") );
				String consServerVal = JUnitTestConnect.activeConfigGetStringValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.ServerName);
				TestUtilities.checkResult("Server value != null", consServerVal != null);
				TestUtilities.checkResult("Server value == Server_1", consServerVal.contentEquals("Server_1") );
				String strValue = JUnitTestConnect.activeConfigGetStringValue(prov, JUnitTestConnect.ConfigGroupTypeServer, JUnitTestConnect.Port);
				TestUtilities.checkResult("Port == 14002", strValue.contentEquals("14002"));
				prov = null;
			}
			else if (testCase == 2)
			{
			
				OmmConsumer cons = null;
				cons = JUnitTestConnect.createOmmConsumer(EmaFactory.createOmmConsumerConfig(localConfigPath).config(outermostMap).consumerName("Consumer_1"));
				
				String defaultProvName = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ConsumerDefaultConsumerName, -1);
				TestUtilities.checkResult("DefaultConsumer value != null", defaultProvName != null);
				TestUtilities.checkResult("DefaultConsumer value == Consumer_1", defaultProvName.contentEquals("Consumer_1") );
				String provChannelVal = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelName, 0);
				TestUtilities.checkResult("Channel value != null", provChannelVal != null);
				TestUtilities.checkResult("Channel value == Channel_1", provChannelVal.contentEquals("Channel_1") );
				String strValue = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.Host, 0);
				TestUtilities.checkResult("Host == 0.0.0.1", strValue.contentEquals("0.0.0.1"));
				strValue = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.Port, 0);
				TestUtilities.checkResult("Port == 19001", strValue.contentEquals("19001"));
				cons = null;
			}
		}
		catch ( OmmException excp)
		{
			System.out.println(excp.getMessage());
			TestUtilities.checkResult("Receiving exception, test failed.", false );
		}
	}
}	
	
public void testLoadDictConfigBetweenProgrammaticAndFileForIProv()
{
	TestUtilities.printTestHead("testLoadDictConfigBetweenProgrammaticAndFileForIProv","Test dict config from programmatic will overwrite ones from file");
	

	Map outermostMap = EmaFactory.createMap();
	Map innerMap = EmaFactory.createMap();
	ElementList elementList = EmaFactory.createElementList();
	ElementList innerElementList = EmaFactory.createElementList();
	try
	{
		
		innerElementList.add(EmaFactory.createElementEntry().ascii("DictionaryType", "DictionaryType::FileDictionary"));
		innerElementList.add(EmaFactory.createElementEntry().ascii("RdmFieldDictionaryItemName", "./ConfigDB3_RWFFld"));
		innerElementList.add(EmaFactory.createElementEntry().ascii("EnumTypeDefItemName", "./ConfigDB3_RWFEnum"));
		innerElementList.add(EmaFactory.createElementEntry().ascii("RdmFieldDictionaryFileName", "./ConfigDB3_RDMFieldDictionary"));
		innerElementList.add(EmaFactory.createElementEntry().ascii("EnumTypeDefFileName", "./ConfigDB3_enumtype.def"));
		innerMap.add(EmaFactory.createMapEntry().keyAscii( "Dictionary_3", MapEntry.MapAction.ADD, innerElementList));
		innerElementList.clear();
		
		elementList.add(EmaFactory.createElementEntry().map("DictionaryList", innerMap));
		innerMap.clear();

		outermostMap.add(EmaFactory.createMapEntry().keyAscii( "DictionaryGroup", MapEntry.MapAction.ADD, elementList));
		elementList.clear();

		
		String localConfigPath = null;
		localConfigPath = "./src/test/resources/com/rtsdk/ema/unittest/EmaFileConfigTests/EmaConfigTest.xml";
		System.out.println("Using Ema Config: " + localConfigPath);
		
		OmmProvider prov = null;
		prov = JUnitTestConnect.createOmmIProvider(EmaFactory.createOmmIProviderConfig(localConfigPath).config(outermostMap).providerName("Provider_2"));
		
		String defaultProvName = JUnitTestConnect.activeConfigGetStringValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.IProviderName);
		TestUtilities.checkResult("DefaultProvider value != null", defaultProvName != null);
		TestUtilities.checkResult("DefaultProvider value == Provider_2", defaultProvName.contentEquals("Provider_2") );
				
		List<Service> services = JUnitTestConnect.activeConfigGetService(prov, true);
		TestUtilities.checkResult("services.size() == 1", services.size() == 1);
		
		/*********retrieve first service *************/
		System.out.println("\nRetrieving DIRECT_FEED service configuration values "); 
		Service temp = services.get(0);
		TestUtilities.checkResult("temp != null", temp != null);
		
		TestUtilities.checkResult("checkHasInfo() == true", temp.checkHasInfo() == true);
		
		TestUtilities.checkResult("serviceId == 1", temp.serviceId() == 1);
		ServiceInfo info = temp.info();
		
		//retrieve dictionary provided/used by this service
		TestUtilities.checkResult("info.checkHasDictionariesProvided() == true", info.checkHasDictionariesProvided());
		TestUtilities.checkResult("info.checkHasDictionariesUsed() == true", info.checkHasDictionariesUsed());
		List<String> dictProvided = info.dictionariesProvidedList();
		TestUtilities.checkResult("dictProvided.size() == 2", dictProvided.size() == 2);
		TestUtilities.checkResult("info.dictionariesProvidedList().get(0) == ./ConfigDB3_RWFFld",dictProvided.get(0).equals("./ConfigDB3_RWFFld"));
		TestUtilities.checkResult("info.dictionariesProvidedList().get(1) == ./ConfigDB3_RWFEnum",dictProvided.get(1).equals("./ConfigDB3_RWFEnum"));
		List<String> dictUsed = info.dictionariesUsedList();
		TestUtilities.checkResult("dictUsed.size() == 2", dictUsed.size() == 2);
		TestUtilities.checkResult("info.dictionariesUsedList().get(0) == ./ConfigDB3_RWFFld",dictUsed.get(0).equals("./ConfigDB3_RWFFld"));
		TestUtilities.checkResult("info.dictionariesUsedList().get(1) == ./ConfigDB3_RWFEnum",dictUsed.get(1).equals("./ConfigDB3_RWFEnum"));
		
		//retrieve dictionary defined in directory
		int serviceId = 1;
		int dictId = 0;
		String strValue = JUnitTestConnect.activeConfigGetServiceDict(prov,JUnitTestConnect.DictionaryType, serviceId, dictId, true);
		TestUtilities.checkResult("dictProvided DictionaryType = Dictionary::FileDictionary",strValue != null && strValue.equals("FileDictionary"));
		strValue = JUnitTestConnect.activeConfigGetServiceDict(prov,JUnitTestConnect.DictionaryName, serviceId, dictId, true);
		TestUtilities.checkResult("dictProvided DictionaryName = Dictionary_3",strValue != null && strValue.equals("Dictionary_3"));
		strValue = JUnitTestConnect.activeConfigGetServiceDict(prov,JUnitTestConnect.DictionaryRDMFieldDictFileName, serviceId, dictId, true);
		TestUtilities.checkResult("dictProvided DictionaryRDMFieldDictFileName = ./ConfigDB3_RDMFieldDictionary",strValue != null && strValue.equals("./ConfigDB3_RDMFieldDictionary"));
		strValue = JUnitTestConnect.activeConfigGetServiceDict(prov,JUnitTestConnect.DictionaryEnumTypeDefFileName, serviceId, dictId, true);
		TestUtilities.checkResult("dictProvided DictionaryEnumTypeDefFileName = ./ConfigDB3_enumtype.def",strValue != null && strValue.equals("./ConfigDB3_enumtype.def"));
		strValue = JUnitTestConnect.activeConfigGetServiceDict(prov,JUnitTestConnect.DictionaryRdmFieldDictionaryItemName, serviceId, dictId, true);
		TestUtilities.checkResult("dictProvided DictionaryRdmFieldDictionaryItemName = ./ConfigDB3_RWFFld",strValue != null && strValue.equals("./ConfigDB3_RWFFld"));
		strValue = JUnitTestConnect.activeConfigGetServiceDict(prov,JUnitTestConnect.DictionaryEnumTypeDefItemName, serviceId, dictId, true);
		TestUtilities.checkResult("dictProvided DictionaryEnumTypeDefItemName = ./ConfigDB3_RWFEnum",strValue != null && strValue.equals("./ConfigDB3_RWFEnum"));
		
		strValue = JUnitTestConnect.activeConfigGetServiceDict(prov,JUnitTestConnect.DictionaryType, serviceId, dictId, false);
		TestUtilities.checkResult("dictUsed DictionaryType = Dictionary::FileDictionary",strValue != null && strValue.equals("FileDictionary"));
		strValue = JUnitTestConnect.activeConfigGetServiceDict(prov,JUnitTestConnect.DictionaryName, serviceId, dictId, false);
		TestUtilities.checkResult("dictUsed DictionaryName = Dictionary_3",strValue != null && strValue.equals("Dictionary_3"));
		strValue = JUnitTestConnect.activeConfigGetServiceDict(prov,JUnitTestConnect.DictionaryRDMFieldDictFileName, serviceId, dictId, false);
		TestUtilities.checkResult("dictUsed DictionaryRDMFieldDictFileName = ./ConfigDB3_RDMFieldDictionary",strValue != null && strValue.equals("./ConfigDB3_RDMFieldDictionary"));
		strValue = JUnitTestConnect.activeConfigGetServiceDict(prov,JUnitTestConnect.DictionaryEnumTypeDefFileName, serviceId, dictId, false);
		TestUtilities.checkResult("dictUsed DictionaryEnumTypeDefFileName = ./ConfigDB3_enumtype.def",strValue != null && strValue.equals("./ConfigDB3_enumtype.def"));
		strValue = JUnitTestConnect.activeConfigGetServiceDict(prov,JUnitTestConnect.DictionaryRdmFieldDictionaryItemName, serviceId, dictId, false);
		TestUtilities.checkResult("dictUsed DictionaryRdmFieldDictionaryItemName = ./ConfigDB3_RWFFld",strValue != null && strValue.equals("./ConfigDB3_RWFFld"));
		strValue = JUnitTestConnect.activeConfigGetServiceDict(prov,JUnitTestConnect.DictionaryEnumTypeDefItemName, serviceId, dictId, false);
		TestUtilities.checkResult("dictUsed DictionaryEnumTypeDefItemName = ./ConfigDB3_RWFEnum",strValue != null && strValue.equals("./ConfigDB3_RWFEnum"));
		
		prov = null;
	}
	catch ( OmmException excp)
	{
		System.out.println(excp.getMessage());
		TestUtilities.checkResult("Receiving exception, test failed.", false );
	}
}

public void testLoadConfigFromProgrammaticForIProvConsMix()
{
	TestUtilities.printTestHead("testLoadConfigFromProgrammaticForIProvConsMix","Test loading configuration parameters programmatically from IProv and Cons at same time");
	

	Map outermostMapIProv = EmaFactory.createMap();
	Map outermostMapCons = EmaFactory.createMap();
	Map innerMap = EmaFactory.createMap();
	ElementList elementList = EmaFactory.createElementList();
	ElementList innerElementList = EmaFactory.createElementList();
	try
	{
		//programmatically config for iprovider
		elementList.add(EmaFactory.createElementEntry().ascii("DefaultIProvider", "Provider_1"));

		innerElementList.add(EmaFactory.createElementEntry().ascii("Server", "Server_1"));
		innerElementList.add(EmaFactory.createElementEntry().ascii("Directory", "Directory_1"));
		innerMap.add(EmaFactory.createMapEntry().keyAscii( "Provider_1", MapEntry.MapAction.ADD, innerElementList));
		innerElementList.clear();
		
		elementList.add(EmaFactory.createElementEntry().map("IProviderList", innerMap));
		innerMap.clear();

		outermostMapIProv.add(EmaFactory.createMapEntry().keyAscii( "IProviderGroup", MapEntry.MapAction.ADD, elementList));
		elementList.clear();
		
		innerElementList.add(EmaFactory.createElementEntry().ascii("ServerType", "ServerType::RSSL_SOCKET"));
		innerElementList.add(EmaFactory.createElementEntry().ascii("CompressionType", "CompressionType::ZLib"));
		innerMap.add(EmaFactory.createMapEntry().keyAscii( "Server_1", MapEntry.MapAction.ADD, innerElementList)); 
		innerElementList.clear();

		elementList.add(EmaFactory.createElementEntry().map("ServerList", innerMap));
		innerMap.clear();

		outermostMapIProv.add(EmaFactory.createMapEntry().keyAscii( "ServerGroup", MapEntry.MapAction.ADD, elementList));
		elementList.clear();
		
		//programmatically config for consumer
		elementList.add(EmaFactory.createElementEntry().ascii("DefaultConsumer", "Consumer_1"));

		innerElementList.add(EmaFactory.createElementEntry().ascii("Channel", "Channel_2"));
		innerElementList.add(EmaFactory.createElementEntry().ascii("Dictionary", "Dictionary_2"));
		innerElementList.add(EmaFactory.createElementEntry().uintValue( "EnableRtt", 1 ));

		innerMap.add(EmaFactory.createMapEntry().keyAscii( "Consumer_1", MapEntry.MapAction.ADD, innerElementList));
		innerElementList.clear();
		
		elementList.add(EmaFactory.createElementEntry().map( "ConsumerList", innerMap ));
		innerMap.clear();

		outermostMapCons.add(EmaFactory.createMapEntry().keyAscii( "ConsumerGroup", MapEntry.MapAction.ADD, elementList ));
		elementList.clear();

		innerElementList.add(EmaFactory.createElementEntry().ascii("ChannelType", "ChannelType::RSSL_ENCRYPTED"));
		innerElementList.add(EmaFactory.createElementEntry().ascii("Host", "localhost"));
		innerElementList.add(EmaFactory.createElementEntry().ascii("Port", "14002"));
		innerElementList.add(EmaFactory.createElementEntry().ascii("ObjectName", "MyHttpObject"));
			
		innerMap.add(EmaFactory.createMapEntry().keyAscii( "Channel_2", MapEntry.MapAction.ADD, innerElementList));
		innerElementList.clear();
		
		elementList.add(EmaFactory.createElementEntry().map( "ChannelList", innerMap ));
		innerMap.clear();

		outermostMapCons.add(EmaFactory.createMapEntry().keyAscii( "ChannelGroup", MapEntry.MapAction.ADD, elementList ));
		elementList.clear();
		

		OmmProvider prov = JUnitTestConnect.createOmmIProvider(EmaFactory.createOmmIProviderConfig().config(outermostMapIProv));
		
		String defaultProvName = JUnitTestConnect.activeConfigGetStringValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.IProviderName);
		TestUtilities.checkResult("DefaultProvider value != null", defaultProvName != null);
		TestUtilities.checkResult("DefaultProvider value == Provider_1", defaultProvName.contentEquals("Provider_1") );
		String provServerVal = JUnitTestConnect.activeConfigGetStringValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.ServerName);
		TestUtilities.checkResult("Server value != null", provServerVal != null);
		TestUtilities.checkResult("Server value == Server_1", provServerVal.contentEquals("Server_1") );
		int serverConnType = JUnitTestConnect.activeConfigGetIntLongValue(prov, JUnitTestConnect.ConfigGroupTypeServer, JUnitTestConnect.ServerType);
		TestUtilities.checkResult("serverConnType == ServerType::RSSL_SOCKET", serverConnType == ChannelTypeSocket);
		String strValue = JUnitTestConnect.activeConfigGetStringValue(prov, JUnitTestConnect.ConfigGroupTypeProvider, JUnitTestConnect.DirectoryName);
		TestUtilities.checkResult("DirectoryName == Directory_1", strValue.contentEquals("Directory_1"));
		
		OmmConsumerConfig testConfig = EmaFactory.createOmmConsumerConfig().config(outermostMapCons);
		OmmConsumer cons = JUnitTestConnect.createOmmConsumer(testConfig);
		
		String defaultConsName = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ConsumerDefaultConsumerName, -1);
		TestUtilities.checkResult("DefaultConsumer value != null", defaultConsName != null);
		TestUtilities.checkResult("DefaultConsumer value == Consumer_1", defaultConsName.contentEquals("Consumer_1") );
		String ConsChannelVal = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelName, 0);
		TestUtilities.checkResult("Channel value != null", ConsChannelVal != null);
		TestUtilities.checkResult("Channel value == Channel_2", ConsChannelVal.contentEquals("Channel_2") );
		int channelConnType = JUnitTestConnect.activeConfigGetIntLongValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelType, 0);
		TestUtilities.checkResult("channelConnType == ChannelType::RSSL_ENCRYPTED", channelConnType == ChannelTypeEncrypted);
		String ConsDictionary = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeDictionary, JUnitTestConnect.DictionaryName, -1);
		TestUtilities.checkResult("Dictionary != null", ConsDictionary != null);
		TestUtilities.checkResult("Dictionary value == Dictionary_2", ConsDictionary.contentEquals("Dictionary_2") );
		String enableRtt = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.EnableRtt, -1);
		TestUtilities.checkResult("EnableRtt value == true", enableRtt.contentEquals("true") );
	}
	catch ( OmmException excp)
	{
		System.out.println(excp.getMessage());
		TestUtilities.checkResult("Receiving exception, test failed.", false );
	}
}

public void testReuseProgrammaticInterface()
{
	TestUtilities.printTestHead("testReuseProgrammaticInterface","Test reuse Programmtic interface");
	

		Map outermostMap = EmaFactory.createMap();
		Map innerMap = EmaFactory.createMap();
		ElementList elementList = EmaFactory.createElementList();
		ElementList innerElementList = EmaFactory.createElementList();
		try
		{
			elementList.add(EmaFactory.createElementEntry().ascii("DefaultConsumer", "Consumer_5"));
			
			innerElementList.add(EmaFactory.createElementEntry().ascii("Channel", "Channel_5"));
			innerMap.add(EmaFactory.createMapEntry().keyAscii( "Consumer_5", MapEntry.MapAction.ADD, innerElementList));
			innerElementList.clear();
			
			elementList.add(EmaFactory.createElementEntry().map("ConsumerList", innerMap));
			innerMap.clear();

			outermostMap.add(EmaFactory.createMapEntry().keyAscii( "ConsumerGroup", MapEntry.MapAction.ADD, elementList));
			elementList.clear();
		
			innerElementList.add(EmaFactory.createElementEntry().ascii("ChannelType", "ChannelType::RSSL_SOCKET"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("Host", "host5"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("Port", "port5"));
			innerElementList.add(EmaFactory.createElementEntry().intValue("TcpNodelay", 1));
			innerMap.add(EmaFactory.createMapEntry().keyAscii( "Channel_5", MapEntry.MapAction.ADD, innerElementList)); 
			innerElementList.clear();

			elementList.add(EmaFactory.createElementEntry().map("ChannelList", innerMap));
			innerMap.clear();

			outermostMap.add(EmaFactory.createMapEntry().keyAscii( "ChannelGroup", MapEntry.MapAction.ADD, elementList));
			elementList.clear();
			
			
			OmmConsumer cons = null;
			OmmConsumerConfig consConfig = EmaFactory.createOmmConsumerConfig();
			cons = JUnitTestConnect.createOmmConsumer(consConfig.config(outermostMap));
			
			String defaultProvName = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ConsumerDefaultConsumerName, -1);
			TestUtilities.checkResult("DefaultConsumer value != null", defaultProvName != null);
			TestUtilities.checkResult("DefaultConsumer value == Consumer_5", defaultProvName.contentEquals("Consumer_5") );
			String consChannelVal = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelName, 0);
			TestUtilities.checkResult("Channel value != null", consChannelVal != null);
			TestUtilities.checkResult("Channel value == Channel_5", consChannelVal.contentEquals("Channel_5") );
			String strValue = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.Host, 0);
			TestUtilities.checkResult("Host == host5", strValue.contentEquals("host5"));
			strValue = JUnitTestConnect.activeConfigGetStringValue(cons, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.Port, 0);
			TestUtilities.checkResult("Port == port5", strValue.contentEquals("port5"));
			cons = null;
			
			//reuse programmatic interface
			outermostMap.clear();
			elementList.add(EmaFactory.createElementEntry().ascii("DefaultConsumer", "Consumer_6"));
			
			innerElementList.add(EmaFactory.createElementEntry().ascii("Channel", "Channel_6"));
			innerMap.add(EmaFactory.createMapEntry().keyAscii( "Consumer_6", MapEntry.MapAction.ADD, innerElementList));
			innerElementList.clear();
			
			elementList.add(EmaFactory.createElementEntry().map("ConsumerList", innerMap));
			innerMap.clear();

			outermostMap.add(EmaFactory.createMapEntry().keyAscii( "ConsumerGroup", MapEntry.MapAction.ADD, elementList));
			elementList.clear();
		
			innerElementList.add(EmaFactory.createElementEntry().ascii("ChannelType", "ChannelType::RSSL_SOCKET"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("Host", "host6"));
			innerElementList.add(EmaFactory.createElementEntry().ascii("Port", "port6"));
			innerElementList.add(EmaFactory.createElementEntry().intValue("TcpNodelay", 1));
			innerMap.add(EmaFactory.createMapEntry().keyAscii( "Channel_6", MapEntry.MapAction.ADD, innerElementList)); 
			innerElementList.clear();

			elementList.add(EmaFactory.createElementEntry().map("ChannelList", innerMap));
			innerMap.clear();

			outermostMap.add(EmaFactory.createMapEntry().keyAscii( "ChannelGroup", MapEntry.MapAction.ADD, elementList));
			elementList.clear();
			
			
			OmmConsumer cons1 = null;
			cons1 = JUnitTestConnect.createOmmConsumer(consConfig.clear().config(outermostMap));
			
			defaultProvName = JUnitTestConnect.activeConfigGetStringValue(cons1, JUnitTestConnect.ConfigGroupTypeConsumer, JUnitTestConnect.ConsumerDefaultConsumerName, -1);
			TestUtilities.checkResult("DefaultConsumer value != null", defaultProvName != null);
			TestUtilities.checkResult("DefaultConsumer value == Consumer_6", defaultProvName.contentEquals("Consumer_6") );
			consChannelVal = JUnitTestConnect.activeConfigGetStringValue(cons1, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ChannelName, 0);
			TestUtilities.checkResult("Channel value != null", consChannelVal != null);
			TestUtilities.checkResult("Channel value == Channel_6", consChannelVal.contentEquals("Channel_6") );
			strValue = JUnitTestConnect.activeConfigGetStringValue(cons1, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.Host, 0);
			TestUtilities.checkResult("Host == host6", strValue.contentEquals("host6"));
			strValue = JUnitTestConnect.activeConfigGetStringValue(cons1, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.Port, 0);
			TestUtilities.checkResult("Port == port6", strValue.contentEquals("port6"));
			cons = null;
		}
		catch ( OmmException excp)
		{
			System.out.println(excp.getMessage());
			TestUtilities.checkResult("Receiving exception, test failed.", false );
		}
}	
	
}
