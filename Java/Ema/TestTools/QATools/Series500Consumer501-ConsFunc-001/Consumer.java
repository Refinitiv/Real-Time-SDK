///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2025 LSEG. All rights reserved.                   --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.examples.training.consumer.series500.ex501_PreferredHost_ProgCfg;

import com.refinitiv.ema.access.Msg;
import com.refinitiv.ema.access.AckMsg;
import com.refinitiv.ema.access.GenericMsg;
import com.refinitiv.ema.access.Map;
import com.refinitiv.ema.access.MapEntry;
import com.refinitiv.ema.access.RefreshMsg;
import com.refinitiv.ema.access.StatusMsg;
import com.refinitiv.ema.access.UpdateMsg;
import com.refinitiv.ema.access.Data;
import com.refinitiv.ema.access.DataType;
import com.refinitiv.ema.access.DataType.DataTypes;
import com.refinitiv.ema.access.ElementList;
import com.refinitiv.ema.access.EmaFactory;
import com.refinitiv.ema.access.FieldEntry;
import com.refinitiv.ema.access.FieldList;
import com.refinitiv.ema.access.OmmConsumer;
import com.refinitiv.ema.access.OmmConsumerClient;
import com.refinitiv.ema.access.OmmConsumerEvent;
import com.refinitiv.ema.access.OmmException;
import com.refinitiv.ema.examples.training.common.CommandLine;

class AppClient implements OmmConsumerClient
{
	private boolean updateCalled = false;

	public void onRefreshMsg(RefreshMsg refreshMsg, OmmConsumerEvent event)
	{
		System.out.println("Item Name: " + (refreshMsg.hasName() ? refreshMsg.name() : "<not set>"));
		System.out.println("Service Name: " + (refreshMsg.hasServiceName() ? refreshMsg.serviceName() : "<not set>"));

		System.out.println("Item State: " + refreshMsg.state());

		if (DataTypes.FIELD_LIST == refreshMsg.payload().dataType())
			decode(refreshMsg.payload().fieldList());

		System.out.println("\nEvent channel info (refresh)\n" + event.channelInformation());
		System.out.println();
	}

	public void onUpdateMsg(UpdateMsg updateMsg, OmmConsumerEvent event)
	{
		if (!updateCalled)
		{
			updateCalled = true;
			System.out.println("Item Name: " + (updateMsg.hasName() ? updateMsg.name() : "<not set>"));
			System.out.println("Service Name: " + (updateMsg.hasServiceName() ? updateMsg.serviceName() : "<not set>"));

			if (DataTypes.FIELD_LIST == updateMsg.payload().dataType())
				decode(updateMsg.payload().fieldList());

			System.out.println("\nEvent channel info (update)\n" + event.channelInformation());
			System.out.println();
		}
		else {
			System.out.println("skipped printing updateMsg");
		}
	}

	public void onStatusMsg(StatusMsg statusMsg, OmmConsumerEvent event)
	{
		System.out.println("Item Name: " + (statusMsg.hasName() ? statusMsg.name() : "<not set>"));
		System.out.println("Service Name: " + (statusMsg.hasServiceName() ? statusMsg.serviceName() : "<not set>"));

		if (statusMsg.hasState())
			System.out.println("Item State: " +statusMsg.state());

		System.out.println("\nEvent channel info (status)\n" + event.channelInformation());
		System.out.println();
	}

	public void onGenericMsg(GenericMsg genericMsg, OmmConsumerEvent consumerEvent){}
	public void onAckMsg(AckMsg ackMsg, OmmConsumerEvent consumerEvent){}
	public void onAllMsg(Msg msg, OmmConsumerEvent consumerEvent){}

	void decode(FieldList fieldList)
	{
		for (FieldEntry fieldEntry : fieldList)
		{
			System.out.print("Fid: " + fieldEntry.fieldId() + " Name = " + fieldEntry.name() + " DataType: " + DataType.asString(fieldEntry.load().dataType()) + " Value: ");

			if (Data.DataCode.BLANK == fieldEntry.code())
				System.out.println(" blank");
			else
				switch (fieldEntry.loadType())
				{
					case DataTypes.REAL :
						System.out.println(fieldEntry.real().asDouble());
						break;
					case DataTypes.DATE :
						System.out.println(fieldEntry.date().day() + " / " + fieldEntry.date().month() + " / " + fieldEntry.date().year());
						break;
					case DataTypes.TIME :
						System.out.println(fieldEntry.time().hour() + ":" + fieldEntry.time().minute() + ":" + fieldEntry.time().second() + ":" + fieldEntry.time().millisecond());
						break;
					case DataTypes.DATETIME :
						System.out.println(fieldEntry.dateTime().day() + " / " + fieldEntry.dateTime().month() + " / " +
								fieldEntry.dateTime().year() + "." + fieldEntry.dateTime().hour() + ":" +
								fieldEntry.dateTime().minute() + ":" + fieldEntry.dateTime().second() + ":" +
								fieldEntry.dateTime().millisecond() + ":" + fieldEntry.dateTime().microsecond()+ ":" +
								fieldEntry.dateTime().nanosecond());
						break;
					case DataTypes.INT :
						System.out.println(fieldEntry.intValue());
						break;
					case DataTypes.UINT :
						System.out.println(fieldEntry.uintValue());
						break;
					case DataTypes.ASCII :
						System.out.println(fieldEntry.ascii());
						break;
					case DataTypes.ENUM :
						System.out.println(fieldEntry.hasEnumDisplay() ? fieldEntry.enumDisplay() : fieldEntry.enumValue());
						break;
					case DataTypes.RMTES :
						System.out.println(fieldEntry.rmtes());
						break;
					case DataTypes.ERROR :
						System.out.println("(" + fieldEntry.error().errorCodeAsString() + ")");
						break;
					default :
						System.out.println();
						break;
				}
		}
	}
}

public class Consumer
{
	// PORTS, HOSTS and CHANNEL_NAMES should have the same size
	// List of ports for channels
	private static final String[] PORTS =
			{"15001", "15002", "15003",
			"15004", "15005", "15006",
			"15007", "15008", "15009"};
	// List of hosts for channels
	private static final String[] HOSTS =
			{"host1", "host2", "host3",
			"host4", "host5", "host6",
			"host7", "host8", "host9"};
	// List of channel names
	private static final String[] CHANNEL_NAMES =
			{"Channel_A", "Channel_B", "Channel_C",
			"Channel_D", "Channel_E", "Channel_F",
			"Channel_G", "Channel_H", "Channel_I"};
	// SERVER_INFOS size should be <= CHANNEL_NAMES size
	// List of server info
	private static final String[] SERVER_INFOS =
			{"Server_Info_A", "Server_Info_B", "Server_Info_C",
			"Server_Info_D", "Server_Info_E", "Server_Info_F"};
	// WSB_CHANNEL_NAMES size should be half of SERVER_INFOS size
	// List of WSB channel names
	private static final String[] WSB_CHANNEL_NAMES =
			{"WarmStandbyChannel_A", "WarmStandbyChannel_B", "WarmStandbyChannel_C"};

	private static final String DEFAULT_SERVICE_NAME = "DIRECT_FEED";
	private static final String INFRA_SERVICE_NAME = "ELEKTRON_DD";
	private static final String DEFAULT_ITEM_NAME = "IBM.N";
	private static final String CS_CONSUMER_NAME = "Consumer_A";
	private static final String WSB_CONSUMER_NAME = "Consumer_B";

	private static final boolean DEFAULT_ENABLE_PREFERRED_HOST_OPTIONS = true;
	private static final String DEFAULT_DETECTION_TIME_SCHEDULE = "";
	private static final int DEFAULT_DETECTION_TIME_INTERVAL = 15;
	// Default channel name is a last name from CHANNEL_NAMES
	private static final String DEFAULT_CHANNEL_NAME = CHANNEL_NAMES[CHANNEL_NAMES.length-1];
	// Default WSB channel name is a last name from WSB_CHANNEL_NAMES
	private static final String DEFAULT_WSB_CHANNEL_NAME = WSB_CHANNEL_NAMES[WSB_CHANNEL_NAMES.length-1];
	private static final boolean DEFAULT_FALLBACK_WITHIN_WSB_GROUP = false;

	private static void addCommandLineArgs()
	{
		CommandLine.programName("Consumer");

		CommandLine.addOption("enablePH", DEFAULT_ENABLE_PREFERRED_HOST_OPTIONS, "Enables preferred host options");
		CommandLine.addOption("detectionTimeSchedule", DEFAULT_DETECTION_TIME_SCHEDULE, "Specifies Cron time format for detection time schedule");
		CommandLine.addOption("detectionTimeInterval", DEFAULT_DETECTION_TIME_INTERVAL, "Specifies detection time interval in seconds. 0 indicates that the detection time interval is disabled");
		CommandLine.addOption("channelNamePreferred", DEFAULT_CHANNEL_NAME, "Specifies a channel name in the Channel or ChannelSet element. Empty string indicates the first channel name in the ChannelSet is used");
		CommandLine.addOption("wsbChannelNamePreferred", DEFAULT_WSB_CHANNEL_NAME, "Specifies a WSB channel name in the WarmStandbyChannelSet element. Empty string indicates the first WSB channel name in the WarmStandbyChannelSet is used");
		CommandLine.addOption("fallBackWithInWSBGroup", DEFAULT_FALLBACK_WITHIN_WSB_GROUP, "Specifies whether to fallback within a WSB group instead of moving into a preferred WSB group");
	}

	private static void init(String[] args) {
		// process command line args
		addCommandLineArgs();
		try
		{
			CommandLine.parseArgs(args);
		}
		catch (IllegalArgumentException ex)
		{
			System.err.println("Error loading command line arguments:\t");
			System.err.println(ex.getMessage());
			System.err.println();
			System.err.println(CommandLine.optionHelpString());
			System.out.println("Consumer exits...");
			System.exit(-1);
		}
	}

	private static Map createProgrammaticConfig(boolean enablePreferredHostOptions, int detectionTimeInterval,
												String detectionTimeSchedule, String channelName, String wsbChannelName,
												boolean fallBackWithInWSBGroup, String serviceName)
	{
		// Channel set contains last three channels from CHANNEL_NAMES
		String channelSet = String.join(",", CHANNEL_NAMES[CHANNEL_NAMES.length-3],
				CHANNEL_NAMES[CHANNEL_NAMES.length-2], CHANNEL_NAMES[CHANNEL_NAMES.length-1]);

		String wsbChannelSet = String.join(",", WSB_CHANNEL_NAMES);

		Map innerMap = EmaFactory.createMap();
		Map configMap = EmaFactory.createMap();
		ElementList elementList = EmaFactory.createElementList();
		ElementList innerElementList = EmaFactory.createElementList();

		elementList.add(EmaFactory.createElementEntry().ascii("DefaultConsumer", "Consumer_A" ));

		// ConsumerGroup
		// CS_Consumer
		innerElementList.add(EmaFactory.createElementEntry().ascii( "ChannelSet", channelSet ));
		innerElementList.add(EmaFactory.createElementEntry().ascii( "PreferredChannelName", channelName ));
		innerElementList.add(EmaFactory.createElementEntry().ascii( "Dictionary", "Dictionary_1" ));
		if (detectionTimeSchedule != null && !detectionTimeSchedule.isEmpty()) {
			innerElementList.add(EmaFactory.createElementEntry().ascii( "PHDetectionTimeSchedule", detectionTimeSchedule ));
		}
		innerElementList.add(EmaFactory.createElementEntry().uintValue( "EnablePreferredHostOptions", enablePreferredHostOptions ? 1 : 0 ));
		innerElementList.add(EmaFactory.createElementEntry().uintValue( "PHDetectionTimeInterval", detectionTimeInterval ));
		innerElementList.add(EmaFactory.createElementEntry().intValue( "ItemCountHint", 5000 ));
		innerElementList.add(EmaFactory.createElementEntry().intValue( "ServiceCountHint", 5000 ));
		innerElementList.add(EmaFactory.createElementEntry().intValue( "ObeyOpenWindow", 0 ));
		innerElementList.add(EmaFactory.createElementEntry().intValue( "PostAckTimeout", 5000 ));
		innerElementList.add(EmaFactory.createElementEntry().intValue( "RequestTimeout", 5000 ));
		innerElementList.add(EmaFactory.createElementEntry().intValue( "MaxOutstandingPosts", 5000 ));
		innerElementList.add(EmaFactory.createElementEntry().intValue( "DispatchTimeoutApiThread", 1 ));
		innerElementList.add(EmaFactory.createElementEntry().intValue( "HandleException", 0 ));
		innerElementList.add(EmaFactory.createElementEntry().intValue( "MaxDispatchCountApiThread", 500 ));
		innerElementList.add(EmaFactory.createElementEntry().intValue( "MaxDispatchCountUserThread", 500 ));
		innerElementList.add(EmaFactory.createElementEntry().intValue( "PipePort", 4001 ));
		innerElementList.add(EmaFactory.createElementEntry().intValue( "ReconnectAttemptLimit", 10 ));
		innerElementList.add(EmaFactory.createElementEntry().intValue( "ReconnectMinDelay", 2000 ));
		innerElementList.add(EmaFactory.createElementEntry().intValue( "ReconnectMaxDelay", 6000 ));
		innerElementList.add(EmaFactory.createElementEntry().intValue( "XmlTraceToStdout", 0 ));
		innerElementList.add(EmaFactory.createElementEntry().intValue( "XmlTraceToFile", 0 ));
		innerElementList.add(EmaFactory.createElementEntry().intValue( "XmlTraceWrite", 0 ));
		innerElementList.add(EmaFactory.createElementEntry().intValue( "XmlTraceRead", 0 ));
		innerElementList.add(EmaFactory.createElementEntry().intValue( "XmlTracePing", 0 ));
		innerElementList.add(EmaFactory.createElementEntry().intValue( "MsgKeyInUpdates", 1 ));

		innerMap.add(EmaFactory.createMapEntry().keyAscii( CS_CONSUMER_NAME, MapEntry.MapAction.ADD, innerElementList ));
		innerElementList.clear();

		// WSB_Consumer
		innerElementList.add(EmaFactory.createElementEntry().ascii( "ChannelSet", channelSet ));
		innerElementList.add(EmaFactory.createElementEntry().ascii( "PreferredChannelName", channelName ));
		innerElementList.add(EmaFactory.createElementEntry().ascii("WarmStandbyChannelSet", wsbChannelSet ));
		innerElementList.add(EmaFactory.createElementEntry().ascii( "Dictionary", "Dictionary_1" ));
		innerElementList.add(EmaFactory.createElementEntry().ascii( "PreferredWSBChannelName", wsbChannelName ));
		if (detectionTimeSchedule != null && !detectionTimeSchedule.isEmpty()) {
			innerElementList.add(EmaFactory.createElementEntry().ascii( "PHDetectionTimeSchedule", detectionTimeSchedule ));
		}
		innerElementList.add(EmaFactory.createElementEntry().uintValue( "EnablePreferredHostOptions", enablePreferredHostOptions ? 1 : 0 ));
		innerElementList.add(EmaFactory.createElementEntry().uintValue( "PHDetectionTimeInterval", detectionTimeInterval ));
		innerElementList.add(EmaFactory.createElementEntry().uintValue( "PHFallBackWithInWSBGroup", fallBackWithInWSBGroup ? 1 : 0 ));
		innerElementList.add(EmaFactory.createElementEntry().intValue( "XmlTraceToStdout", 0 ));
		innerElementList.add(EmaFactory.createElementEntry().intValue( "XmlTraceToFile", 0));
		innerElementList.add(EmaFactory.createElementEntry().intValue( "XmlTraceWrite", 0 ));
		innerElementList.add(EmaFactory.createElementEntry().intValue( "XmlTraceRead", 0 ));
		innerElementList.add(EmaFactory.createElementEntry().intValue( "XmlTracePing", 0 ));

		innerMap.add(EmaFactory.createMapEntry().keyAscii( WSB_CONSUMER_NAME, MapEntry.MapAction.ADD, innerElementList ));

		elementList.add(EmaFactory.createElementEntry().map( "ConsumerList", innerMap ));
		innerMap.clear();

		configMap.add(EmaFactory.createMapEntry().keyAscii( "ConsumerGroup", MapEntry.MapAction.ADD, elementList ));
		elementList.clear();

		// ChannelGroup
		// Channels
		for (int i = 0; i < PORTS.length; i++) {
			innerElementList.add(EmaFactory.createElementEntry().ascii( "ChannelType", "ChannelType::RSSL_SOCKET" ));
			innerElementList.add(EmaFactory.createElementEntry().ascii( "CompressionType", "CompressionType::ZLib"));
			innerElementList.add(EmaFactory.createElementEntry().intValue( "GuaranteedOutputBuffers", 5000));
			innerElementList.add(EmaFactory.createElementEntry().intValue( "ConnectionPingTimeout", 50000));
			innerElementList.add(EmaFactory.createElementEntry().intValue( "InitializationTimeout", 30000));
			innerElementList.add(EmaFactory.createElementEntry().ascii( "Host", HOSTS[i]));
			innerElementList.add(EmaFactory.createElementEntry().ascii("Port", PORTS[i]));
			innerElementList.add(EmaFactory.createElementEntry().intValue( "TcpNodelay", 0));

			innerMap.add(EmaFactory.createMapEntry().keyAscii( CHANNEL_NAMES[i], MapEntry.MapAction.ADD, innerElementList));
			innerElementList.clear();
		}

		elementList.add(EmaFactory.createElementEntry().map( "ChannelList", innerMap ));
		innerMap.clear();

		configMap.add(EmaFactory.createMapEntry().keyAscii("ChannelGroup", MapEntry.MapAction.ADD, elementList ));
		elementList.clear();

		// DictionaryGroup
		// Dictionary_1
		innerElementList.add(EmaFactory.createElementEntry().ascii( "DictionaryType", "DictionaryType::FileDictionary"));
		innerElementList.add(EmaFactory.createElementEntry().ascii( "RdmFieldDictionaryFileName", "./RDMFieldDictionary"));
		innerElementList.add(EmaFactory.createElementEntry().ascii( "EnumTypeDefFileName", "./enumtype.def" ));
		innerMap.add(EmaFactory.createMapEntry().keyAscii( "Dictionary_1", MapEntry.MapAction.ADD, innerElementList));
		innerElementList.clear();

		elementList.add(EmaFactory.createElementEntry().map( "DictionaryList", innerMap ));
		innerMap.clear();

		configMap.add(EmaFactory.createMapEntry().keyAscii( "DictionaryGroup", MapEntry.MapAction.ADD, elementList ));
		elementList.clear();

		// WarmStandbyServerInfoGroup
		// Server_Infos
		for (int i = 0; i < SERVER_INFOS.length; i++) {
			innerElementList.add(EmaFactory.createElementEntry().ascii("Channel", CHANNEL_NAMES[i]));
			innerElementList.add(EmaFactory.createElementEntry().ascii("PerServiceNameSet", serviceName));
			innerMap.add(EmaFactory.createMapEntry().keyAscii(SERVER_INFOS[i], MapEntry.MapAction.ADD, innerElementList));
			innerElementList.clear();
		}

		elementList.add(EmaFactory.createElementEntry().map("WarmStandbyServerInfoList", innerMap));
		innerMap.clear();

		configMap.add(EmaFactory.createMapEntry().keyAscii("WarmStandbyServerInfoGroup", MapEntry.MapAction.ADD, elementList));
		elementList.clear();

		// WarmStandbyGroup
		// WarmStandbyChannels
		for (int i = 0; i < WSB_CHANNEL_NAMES.length; i++) {
			innerElementList.add(EmaFactory.createElementEntry().ascii("StartingActiveServer", SERVER_INFOS[2*i]));
			innerElementList.add(EmaFactory.createElementEntry().ascii("StandbyServerSet", SERVER_INFOS[2*i+1]));
			innerElementList.add(EmaFactory.createElementEntry().enumValue("WarmStandbyMode", 1)); /* 2 for service based while 1 for login based warm standby */
			innerMap.add(EmaFactory.createMapEntry().keyAscii(WSB_CHANNEL_NAMES[i], MapEntry.MapAction.ADD, innerElementList));
			innerElementList.clear();
		}

		elementList.add(EmaFactory.createElementEntry().map("WarmStandbyList", innerMap));
		innerMap.clear();

		configMap.add(EmaFactory.createMapEntry().keyAscii("WarmStandbyGroup", MapEntry.MapAction.ADD, elementList));
		elementList.clear();

		return configMap;
	}

	public static void main(String[] args)
	{
		init(args);

		OmmConsumer consumer = null;
		try
		{
			AppClient appClient = new AppClient();

			boolean enablePreferredHostOptions = CommandLine.booleanValue("enablePH");
			boolean fallBackWithInWSBGroup = CommandLine.booleanValue("fallBackWithInWSBGroup");
			int detectionTimeInterval = CommandLine.intValue("detectionTimeInterval");
			String detectionTimeSchedule = CommandLine.value("detectionTimeSchedule");
			String channelName = CommandLine.value("channelNamePreferred");
			String wsbChannelName = CommandLine.value("wsbChannelNamePreferred");

			String serviceName = DEFAULT_SERVICE_NAME;
			Map progConfig = createProgrammaticConfig(enablePreferredHostOptions, detectionTimeInterval,
								detectionTimeSchedule, channelName, wsbChannelName,
								fallBackWithInWSBGroup, serviceName);
			consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig()
					.config(progConfig)
					.consumerName(CS_CONSUMER_NAME));
			consumer.registerClient(EmaFactory.createReqMsg()
					.serviceName(serviceName)
					.name(DEFAULT_ITEM_NAME), appClient);

			int printInterval = 1;
			ChannelInformation ci = EmaFactory.createChannelInformation();
			for (int i = 0; i < 600; i++) {
				Thread.sleep(1000); // API calls onRefreshMsg(), onUpdateMsg() and onStatusMsg()

				if ((i % printInterval == 0)) {
					consumer.channelInformation(ci);
					if (ci.channelState() != ChannelInformation.ChannelState.INACTIVE) {
						System.out.println("\nChannel information (consumer):\n\t" + ci);
						System.out.println();
					}
				}
			}
		}
		catch (InterruptedException | OmmException ex)
		{
			System.out.println(ex.getMessage());
		}
		finally
		{
			if (consumer != null) consumer.uninitialize();
		}
	}
}


