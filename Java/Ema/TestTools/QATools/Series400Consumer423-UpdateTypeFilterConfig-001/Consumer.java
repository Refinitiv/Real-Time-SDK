///*|----------------------------------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019,2025 LSEG. All rights reserved.
///*|----------------------------------------------------------------------------------------------------

package com.refinitiv.ema.examples.training.consumer.series400.ex423_MP_AdmDomainCfg_DomainRep;

import com.refinitiv.ema.access.*;
import com.refinitiv.ema.access.DataType.DataTypes;
import com.refinitiv.ema.domain.login.Login.LoginReq;
import com.refinitiv.ema.rdm.EmaRdm;

import java.util.Iterator;


class AppClient implements OmmConsumerClient
{
	public void onRefreshMsg(RefreshMsg refreshMsg, OmmConsumerEvent event)
	{
		if (refreshMsg.hasMsgKey())
			System.out.println("Item Name: " + refreshMsg.name() + " Service Name: " + refreshMsg.serviceName());

		System.out.println("Item State: " + refreshMsg.state());

		if (DataTypes.FIELD_LIST == refreshMsg.payload().dataType())
			decode(refreshMsg.payload().fieldList());

		System.out.println();
	}

	public void onUpdateMsg(UpdateMsg updateMsg, OmmConsumerEvent event)
	{
		if (updateMsg.hasMsgKey())
			System.out.println("Item Name: " + updateMsg.name() + " Service Name: " + updateMsg.serviceName());

		if (DataTypes.FIELD_LIST == updateMsg.payload().dataType())
			decode(updateMsg.payload().fieldList());

		System.out.println();
	}

	public void onStatusMsg(StatusMsg statusMsg, OmmConsumerEvent event)
	{
		if (statusMsg.hasMsgKey())
			System.out.println("Item Name: " + statusMsg.name() + " Service Name: " + statusMsg.serviceName());

		if (statusMsg.hasState())
			System.out.println("Item State: " +statusMsg.state());

		System.out.println();
	}

	public void onGenericMsg(GenericMsg genericMsg, OmmConsumerEvent consumerEvent){}
	public void onAckMsg(AckMsg ackMsg, OmmConsumerEvent consumerEvent){}
	public void onAllMsg(Msg msg, OmmConsumerEvent consumerEvent){}

	void decode(FieldList fieldList)
	{
		Iterator<FieldEntry> iter = fieldList.iterator();
		FieldEntry fieldEntry;
		while (iter.hasNext())
		{
			fieldEntry = iter.next();
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
					case DataTypes.RMTES:
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
	static void printHelp()
	{

		System.out.println("\nOptions:\n" + "  -?\tShows this usage\n"
				+ "  -putf UpdateTypeFilter in programmatic config.\n"
				+ "  -pnutf NegativeUpdateTypeFilter in programmatic config.\n"
				+ "  -futf UpdateTypeFilter via functional call.\n"
				+ "  -fnutf NegativeUpdateTypeFilter via functional call.\n"
				+ "  -clmutf UpdateTypeFilter via custom Login message.\n"
				+ "  -clmnutf NegativeUpdateTypeFilter via custom Login message.\n"
				+ "  -utfFuncClm UpdateTypeFilter setting via functional call will precede setting the message.\n"
				+ "  -nutfFuncClm NegativeUpdateTypeFilter setting via functional call will precede setting the message.\n"
				+ "  -utfClmFunc UpdateTypeFilter setting via functional call will take place after setting the custom Login message.\n"
				+ "  -nutfClmFunc NegativeUpdateTypeFilter setting via functional call will take place after setting the custom Login message.\n"
				+ "  -setCLM if present, custom LoginRequest message will be set via config interface, otherwise it won't be set."
				+ "  -service set service name. Default is DIRECT_FEED. \n"
				+ "  -item set item name. Default is IBM.N. \n"
				+ "  -progConf enable programmatic config. Default is disable. \n"
				+ "  -progHost set host for programmatic config. Default is localhost. \n"
				+ "  -progPort set port for programmatic config. Default is 14002. \n"
				+ "\n");
		System.out.println("For instance, the following arguments \"-putf 1 -pnutf 2 -futf 4 -fnutf 8 -clmutf 16 -clmnutf 32 -utfFuncClm -nutfClmFunc\"" +
				" indicate that UpdateTypeFilter set via programmatic config equals 1, \n" +
				" NegativeUpdateTypeFilter set via programmatic config equals 2, \n" +
				" UpdateTypeFilter set via functional call equals 4, \n" +
				" NegativeUpdateTypeFilter set via functional call equals 8, \n" +
				" UpdateTypeFilter set via custom login message equals 16, \n" +
				" NegativeUpdateTypeFilter set via custom login message equals 32, \n" +
				" setting UpdateTypeFilter via functional call will precede setting custom login message with this value set, " +
				" and setting custom login message with NegativeUpdateTypeFilter will precede setting it via function call.");
	}

	static Map createProgramaticConfig(long updateTypeFilter,
									   long negativeUpdateTypeFilter,
									   String progHost,
									   String progPort)
	{
		Map innerMap = EmaFactory.createMap();
		Map configMap = EmaFactory.createMap();
		ElementList elementList = EmaFactory.createElementList();
		ElementList innerElementList = EmaFactory.createElementList();

		elementList.add(EmaFactory.createElementEntry().ascii("DefaultConsumer", "Consumer_UTF" ));

		innerElementList.add(EmaFactory.createElementEntry().ascii( "Channel", "Channel_1" ));
		innerElementList.add(EmaFactory.createElementEntry().ascii( "Dictionary", "Dictionary_1" ));
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
		innerElementList.add(EmaFactory.createElementEntry().intValue( "MsgKeyInUpdates", 1 ));
		if (updateTypeFilter > 0) {
			innerElementList.add(EmaFactory.createElementEntry().uintValue( "UpdateTypeFilter", updateTypeFilter ));
		}
		if (negativeUpdateTypeFilter > 0) {
			innerElementList.add(EmaFactory.createElementEntry().uintValue( "NegativeUpdateTypeFilter", negativeUpdateTypeFilter ));
		}

		innerMap.add(EmaFactory.createMapEntry().keyAscii( "Consumer_UTF", MapEntry.MapAction.ADD, innerElementList));
		innerElementList.clear();

		elementList.add(EmaFactory.createElementEntry().map( "ConsumerList", innerMap ));
		innerMap.clear();

		configMap.add(EmaFactory.createMapEntry().keyAscii( "ConsumerGroup", MapEntry.MapAction.ADD, elementList ));
		elementList.clear();

		innerElementList.add(EmaFactory.createElementEntry().ascii( "ChannelType", "ChannelType::RSSL_SOCKET" ));
		innerElementList.add(EmaFactory.createElementEntry().ascii( "CompressionType", "CompressionType::ZLib"));
		innerElementList.add(EmaFactory.createElementEntry().intValue( "GuaranteedOutputBuffers", 5000));
		innerElementList.add(EmaFactory.createElementEntry().intValue( "ConnectionPingTimeout", 50000));
		innerElementList.add(EmaFactory.createElementEntry().ascii( "Host", progHost));
		innerElementList.add(EmaFactory.createElementEntry().ascii("Port", progPort));
		innerElementList.add(EmaFactory.createElementEntry().intValue( "TcpNodelay", 0));

		innerMap.add(EmaFactory.createMapEntry().keyAscii( "Channel_1", MapEntry.MapAction.ADD, innerElementList));
		innerElementList.clear();

		elementList.add(EmaFactory.createElementEntry().map( "ChannelList", innerMap ));
		innerMap.clear();

		configMap.add(EmaFactory.createMapEntry().keyAscii("ChannelGroup", MapEntry.MapAction.ADD, elementList ));
		elementList.clear();

		return configMap;
	}

	public static void main(String[] args)
	{
		OmmConsumer consumer = null;
		try
		{
			AppClient appClient = new AppClient();

			LoginReq loginReq = EmaFactory.Domain.createLoginReq();
			ReqMsg reqMsg = EmaFactory.createReqMsg();
			long pUpdateTypeFilter = 0;
			long pNegativeUpdateTypeFilter = 0;
			long fUpdateTypeFilter = 0;
			long fNegativeUpdateTypeFilter = 0;
			long clmUpdateTypeFilter = 0;
			long clmNegativeUpdateTypeFilter = 0;
			boolean utfFuncClm = true;
			boolean nutfFuncClm = true;
			boolean setCLM = false;
			boolean programmaticConfig = false;
			String serviceName = "DIRECT_FEED";
			String itemName = "IBM.N";
			String progHost = "localhost";
			String progPort = "14002";

			OmmConsumerConfig config = EmaFactory.createOmmConsumerConfig();

			try
			{
				int argsCount = 0;

				while (argsCount < args.length)
				{
					if (0 == args[argsCount].compareTo("-?"))
					{
						printHelp();
					}
					else if ("-putf".equals(args[argsCount]))
					{
						pUpdateTypeFilter = Integer.parseInt(args[++argsCount]);
						++argsCount;
					}
					else if ("-pnutf".equals(args[argsCount]))
					{
						pNegativeUpdateTypeFilter = Integer.parseInt(args[++argsCount]);
						++argsCount;
					}
					else if ("-futf".equals(args[argsCount]))
					{
						fUpdateTypeFilter = Integer.parseInt(args[++argsCount]);
						++argsCount;
					}
					else if ("-fnutf".equals(args[argsCount]))
					{
						fNegativeUpdateTypeFilter = Integer.parseInt(args[++argsCount]);
						++argsCount;
					}
					else if ("-clmutf".equals(args[argsCount]))
					{
						clmUpdateTypeFilter = Integer.parseInt(args[++argsCount]);
						++argsCount;
					}
					else if ("-clmnutf".equals(args[argsCount]))
					{
						clmNegativeUpdateTypeFilter = Integer.parseInt(args[++argsCount]);
						++argsCount;
					}
					else if ("-utfFuncClm".equals(args[argsCount]))
					{
						utfFuncClm = true;
						++argsCount;
					}
					else if ("-nutfFuncClm".equals(args[argsCount]))
					{
						nutfFuncClm = true;
						++argsCount;
					}
					else if ("-utfClmFunc".equals(args[argsCount]))
					{
						utfFuncClm = false;
						++argsCount;
					}
					else if ("-nutfClmFunc".equals(args[argsCount]))
					{
						nutfFuncClm = false;
						++argsCount;
					}
					else if ("-setCLM".equals(args[argsCount]))
					{
						setCLM = true;
						++argsCount;
					}
					else if ("-progConf".equals(args[argsCount]))
					{
						programmaticConfig = true;
						++argsCount;
					}
					else if ("-service".equals(args[argsCount]))
					{
						serviceName = args[++argsCount];
						++argsCount;
					}
					else if ("-item".equals(args[argsCount]))
					{
						itemName = args[++argsCount];
						++argsCount;
					}
					else if ("-progHost".equals(args[argsCount]))
					{
						progHost = args[++argsCount];
						++argsCount;
					}
					else if ("-progPort".equals(args[argsCount]))
					{
						progPort = args[++argsCount];
						++argsCount;
					}
					else // unrecognized command line argument
					{
						printHelp();
						++argsCount;
					}
				}
			}
			catch (Exception e)
			{
				printHelp();
			}

			loginReq.name("user")
					.nameType(EmaRdm.USER_NAME)
					.applicationId("127")
					.position("127.0.0.1/net")
					.allowSuspectData(true);

			if (clmUpdateTypeFilter > 0) loginReq.updateTypeFilter(clmUpdateTypeFilter);
			if (clmNegativeUpdateTypeFilter > 0) loginReq.negativeUpdateTypeFilter(clmNegativeUpdateTypeFilter);

			if (programmaticConfig) {
				config.config(
						createProgramaticConfig(pUpdateTypeFilter,
								pNegativeUpdateTypeFilter,
								progHost,
								progPort));
			}

			config.operationModel(OmmConsumerConfig.OperationModel.USER_DISPATCH);

			if (fUpdateTypeFilter > 0 && utfFuncClm) config.updateTypeFilter(fUpdateTypeFilter);
			if (fNegativeUpdateTypeFilter > 0 && nutfFuncClm) config.negativeUpdateTypeFilter(fNegativeUpdateTypeFilter);
			if (setCLM) config.addAdminMsg(loginReq.message());
			if (fUpdateTypeFilter > 0 && !utfFuncClm) config.updateTypeFilter(fUpdateTypeFilter);
			if (fNegativeUpdateTypeFilter > 0 && !nutfFuncClm) config.negativeUpdateTypeFilter(fNegativeUpdateTypeFilter);

			consumer = EmaFactory.createOmmConsumer(config);

			consumer.registerClient(reqMsg.clear().serviceName(serviceName).name(itemName), appClient, null);

			long startTime = System.currentTimeMillis();
			while (startTime + 60000 > System.currentTimeMillis())
				consumer.dispatch(10);		// calls to onRefreshMsg(), onUpdateMsg(), or onStatusMsg() execute on this thread
		}
		catch (OmmException excp)
		{
			System.out.println(excp);
		}
		finally
		{
			if (consumer != null) consumer.uninitialize();
		}
	}
}

//END APIQA