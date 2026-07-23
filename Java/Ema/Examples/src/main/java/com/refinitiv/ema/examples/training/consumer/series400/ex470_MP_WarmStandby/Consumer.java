/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022,2024,2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.examples.training.consumer.series400.ex470_MP_WarmStandby;

import com.refinitiv.ema.access.*;

class AppClient implements OmmConsumerClient
{
	public void onRefreshMsg(RefreshMsg refreshMsg, OmmConsumerEvent event)
	{
		System.out.println(refreshMsg);
	}
	
	public void onUpdateMsg(UpdateMsg updateMsg, OmmConsumerEvent event) 
	{
		System.out.println(updateMsg);
	}

	public void onStatusMsg(StatusMsg statusMsg, OmmConsumerEvent event) 
	{
		System.out.println(statusMsg);
		if (statusMsg.state().statusCode() == OmmState.StatusCode.WSB_CHANGE_ACTIVE_COMPLETE)
		{
			System.out.println(event.warmStandbyChangeEventInfo());
		}
	}
	
	public void onGenericMsg(GenericMsg genericMsg, OmmConsumerEvent consumerEvent){}
	public void onAckMsg(AckMsg ackMsg, OmmConsumerEvent consumerEvent){}
	public void onAllMsg(Msg msg, OmmConsumerEvent consumerEvent){}
}

public class Consumer 
{
	static Map createProgramaticConfig()
	{
		Map configDb = EmaFactory.createMap();
		Map elementMap = EmaFactory.createMap();
		ElementList outerElementList = EmaFactory.createElementList();
		outerElementList.add(EmaFactory.createElementEntry().ascii("DefaultConsumer", "Consumer_8"));
		
		ElementList innerElementList = EmaFactory.createElementList();
		innerElementList.add(EmaFactory.createElementEntry().ascii("WarmStandbyChannelSet", "WarmStandbyChannel_1"));
		innerElementList.add(EmaFactory.createElementEntry().intValue("XmlTraceToStdout", 0));
		innerElementList.add(EmaFactory.createElementEntry().ascii("Dictionary", "Dictionary_1"));
		elementMap.add(EmaFactory.createMapEntry().keyAscii("Consumer_8", MapEntry.MapAction.ADD, innerElementList));

		outerElementList.add(EmaFactory.createElementEntry().map("ConsumerList", elementMap));

		elementMap.clear();
		
		configDb.add(EmaFactory.createMapEntry().keyAscii("ConsumerGroup", MapEntry.MapAction.ADD, outerElementList));
		outerElementList.clear();
		innerElementList.clear();
		
		innerElementList.add(EmaFactory.createElementEntry().enumValue("ChannelType", 0));
		innerElementList.add(EmaFactory.createElementEntry().ascii("Host", "localhost"));
		innerElementList.add(EmaFactory.createElementEntry().ascii("Port", "14002"));
		elementMap.add(EmaFactory.createMapEntry().keyAscii("Channel_1", MapEntry.MapAction.ADD, innerElementList));
		innerElementList.clear();
		
		innerElementList.add(EmaFactory.createElementEntry().enumValue("ChannelType", 0));
		innerElementList.add(EmaFactory.createElementEntry().ascii("Host", "localhost"));
		innerElementList.add(EmaFactory.createElementEntry().ascii("Port", "14003"));
		elementMap.add(EmaFactory.createMapEntry().keyAscii("Channel_2", MapEntry.MapAction.ADD, innerElementList));
		innerElementList.clear();

		outerElementList.add(EmaFactory.createElementEntry().map("ChannelList", elementMap));
		elementMap.clear();
		
		configDb.add(EmaFactory.createMapEntry().keyAscii("ChannelGroup", MapEntry.MapAction.ADD, outerElementList));
		outerElementList.clear();
		
		innerElementList.add(EmaFactory.createElementEntry().ascii("Channel", "Channel_1"));
		innerElementList.add(EmaFactory.createElementEntry().ascii("PerServiceNameSet", "DIRECT_FEED"));
		elementMap.add(EmaFactory.createMapEntry().keyAscii("Server_Info_1", MapEntry.MapAction.ADD, innerElementList));
		innerElementList.clear();
		
		innerElementList.add(EmaFactory.createElementEntry().ascii("Channel", "Channel_2"));
		innerElementList.add(EmaFactory.createElementEntry().ascii("PerServiceNameSet", "DIRECT_FEED"));
		elementMap.add(EmaFactory.createMapEntry().keyAscii("Server_Info_2", MapEntry.MapAction.ADD, innerElementList));
		innerElementList.clear();

		outerElementList.add(EmaFactory.createElementEntry().map("WarmStandbyServerInfoList", elementMap));
		elementMap.clear();

		configDb.add(EmaFactory.createMapEntry().keyAscii("WarmStandbyServerInfoGroup", MapEntry.MapAction.ADD, outerElementList));
		outerElementList.clear();

		innerElementList.add(EmaFactory.createElementEntry().ascii("StartingActiveServer", "Server_Info_1"));
		innerElementList.add(EmaFactory.createElementEntry().ascii("StandbyServerSet", "Server_Info_2"));
		innerElementList.add(EmaFactory.createElementEntry().enumValue("WarmStandbyMode", 1)); /* 2 for service based while 1 for login based warm standby */
		elementMap.add(EmaFactory.createMapEntry().keyAscii("WarmStandbyChannel_1", MapEntry.MapAction.ADD, innerElementList));
		innerElementList.clear();

		outerElementList.add(EmaFactory.createElementEntry().map("WarmStandbyList", elementMap));
		elementMap.clear();

		configDb.add(EmaFactory.createMapEntry().keyAscii("WarmStandbyGroup", MapEntry.MapAction.ADD, outerElementList));
		outerElementList.clear();
		
		innerElementList.add( EmaFactory.createElementEntry().ascii( "DictionaryType", "DictionaryType::FileDictionary" ));
		innerElementList.add( EmaFactory.createElementEntry().ascii( "RdmFieldDictionaryFileName", "./RDMFieldDictionary" ));
		innerElementList.add( EmaFactory.createElementEntry().ascii( "EnumTypeDefFileName", "./enumtype.def" ));
		elementMap.add( EmaFactory.createMapEntry().keyAscii( "Dictionary_1", MapEntry.MapAction.ADD, innerElementList ));
		innerElementList.clear();
	
		outerElementList.add( EmaFactory.createElementEntry().map( "DictionaryList", elementMap ));
		elementMap.clear();
	
		configDb.add( EmaFactory.createMapEntry().keyAscii( "DictionaryGroup", MapEntry.MapAction.ADD, outerElementList ));
		outerElementList.clear();
		
		return configDb;
	}
	
	public static void main(String[] args)
	{
		OmmConsumer consumer = null;
		try 
		{
			AppClient appClient = new AppClient();

			consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig()
					.wsbChangeEventInfo(true).config( createProgramaticConfig()), appClient); // use programmatic configuration parameters

			consumer.registerClient(EmaFactory.createReqMsg().serviceName("DIRECT_FEED").name("SPOT"), appClient, 0);

			final int printInterval = 5000;
			long nextPrintTime = System.currentTimeMillis() + printInterval;
			for (int i = 0; i < 60; i++)
			{
				Thread.sleep(1000);
				long currentTime = System.currentTimeMillis();
				if (currentTime >= nextPrintTime)
				{
					// Get warm standby channel info
					System.out.println(consumer.getWarmStandbyChannelInformation());
					nextPrintTime = currentTime + printInterval;
				}
			}
		}
		catch (InterruptedException | OmmException excp)
		{
			System.out.println(excp.getMessage());
		}
		finally 
		{
			if (consumer != null) consumer.uninitialize();
		}
	}
}


