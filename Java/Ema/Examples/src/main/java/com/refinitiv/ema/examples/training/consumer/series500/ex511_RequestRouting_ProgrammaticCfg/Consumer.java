///*|--------------------------------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      	                 --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.                       --
// *|                See the project's LICENSE.md for details.                  					 --
// *|              Copyright (C) 2024 LSEG. All rights reserved.            		                 --
///*|--------------------------------------------------------------------------------------------------

package com.refinitiv.ema.examples.training.consumer.series500.ex511_RequestRouting_ProgrammaticCfg;

import com.refinitiv.ema.access.AckMsg;
import com.refinitiv.ema.access.ElementList;
import com.refinitiv.ema.access.EmaFactory;
import com.refinitiv.ema.access.GenericMsg;
import com.refinitiv.ema.access.Map;
import com.refinitiv.ema.access.MapEntry;
import com.refinitiv.ema.access.Msg;
import com.refinitiv.ema.access.OmmConsumer;
import com.refinitiv.ema.access.OmmConsumerClient;
import com.refinitiv.ema.access.OmmConsumerEvent;
import com.refinitiv.ema.access.OmmException;
import com.refinitiv.ema.access.RefreshMsg;
import com.refinitiv.ema.access.StatusMsg;
import com.refinitiv.ema.access.UpdateMsg;

class AppClient implements OmmConsumerClient
{
	public void onRefreshMsg(RefreshMsg refreshMsg, OmmConsumerEvent event)
	{
		System.out.println( refreshMsg + "\nevent channel info (refresh)\n" + event.channelInformation() );
	}
	
	public void onUpdateMsg(UpdateMsg updateMsg, OmmConsumerEvent event) 
	{
		System.out.println( updateMsg + "\nevent channel info (update)\n" + event.channelInformation() );
	}

	public void onStatusMsg(StatusMsg statusMsg, OmmConsumerEvent event) 
	{
		System.out.println( statusMsg + "\nevent channel info (status)\n" + event.channelInformation() );
	}
	
	public void onGenericMsg(GenericMsg genericMsg, OmmConsumerEvent consumerEvent){}
	public void onAckMsg(AckMsg ackMsg, OmmConsumerEvent consumerEvent){}
	public void onAllMsg(Msg msg, OmmConsumerEvent consumerEvent){}
}

public class Consumer 
{
	static Map createProgramaticConfig()
	{
		Map innerMap = EmaFactory.createMap();
		Map configMap = EmaFactory.createMap();
		ElementList elementList = EmaFactory.createElementList();
		ElementList innerElementList = EmaFactory.createElementList();
		
		elementList.add(EmaFactory.createElementEntry().ascii("DefaultConsumer", "Consumer_1" ));
		
		innerElementList.add(EmaFactory.createElementEntry().ascii( "SessionChannelSet", "Connection_1, Connection_2" ));
		innerElementList.add(EmaFactory.createElementEntry().ascii( "Dictionary", "Dictionary_1" ));
		innerElementList.add(EmaFactory.createElementEntry().intValue( "ItemCountHint", 5000 ));
		innerElementList.add(EmaFactory.createElementEntry().intValue( "ServiceCountHint", 5000 ));
		innerElementList.add(EmaFactory.createElementEntry().intValue( "ObeyOpenWindow", 0 ));
		innerElementList.add(EmaFactory.createElementEntry().intValue( "PostAckTimeout", 5000 ));
		innerElementList.add(EmaFactory.createElementEntry().intValue( "RequestTimeout", 5000 ));
		innerElementList.add(EmaFactory.createElementEntry().intValue( "MaxOutstandingPosts", 5000 ));
		innerElementList.add(EmaFactory.createElementEntry().intValue( "DispatchTimeoutApiThread", 1 ));
		innerElementList.add(EmaFactory.createElementEntry().intValue( "MaxDispatchCountApiThread", 500 ));
		innerElementList.add(EmaFactory.createElementEntry().intValue( "MaxDispatchCountUserThread", 500 ));
		innerElementList.add(EmaFactory.createElementEntry().intValue( "ReconnectAttemptLimit", 10 ));
		innerElementList.add(EmaFactory.createElementEntry().intValue( "ReconnectMinDelay", 2000 ));
		innerElementList.add(EmaFactory.createElementEntry().intValue( "ReconnectMaxDelay", 6000 ));
		innerElementList.add(EmaFactory.createElementEntry().intValue( "XmlTraceToStdout", 0 ));
		innerElementList.add(EmaFactory.createElementEntry().intValue( "MsgKeyInUpdates", 1 ));
		innerElementList.add(EmaFactory.createElementEntry().uintValue( "SessionEnhancedItemRecovery", 1 ));

		innerMap.add(EmaFactory.createMapEntry().keyAscii( "Consumer_1", MapEntry.MapAction.ADD, innerElementList));
		innerElementList.clear();
		
		elementList.add(EmaFactory.createElementEntry().map( "ConsumerList", innerMap ));
		innerMap.clear();

		configMap.add(EmaFactory.createMapEntry().keyAscii( "ConsumerGroup", MapEntry.MapAction.ADD, elementList ));
		elementList.clear();
		
		innerElementList.add(EmaFactory.createElementEntry().ascii( "ChannelSet", "Channel_1, Channel_7" ));
		innerElementList.add(EmaFactory.createElementEntry().intValue( "ReconnectAttemptLimit", 4 ));
		innerElementList.add(EmaFactory.createElementEntry().intValue( "ReconnectMinDelay", 2000 ));
		innerElementList.add(EmaFactory.createElementEntry().intValue( "ReconnectMaxDelay", 6000 ));
		
		innerMap.add(EmaFactory.createMapEntry().keyAscii( "Connection_1", MapEntry.MapAction.ADD, innerElementList));
		innerElementList.clear();
		
		innerElementList.add(EmaFactory.createElementEntry().ascii( "ChannelSet", "Channel_10, Channel_11" ));
		innerElementList.add(EmaFactory.createElementEntry().intValue( "ReconnectAttemptLimit", 4 ));
		innerElementList.add(EmaFactory.createElementEntry().intValue( "ReconnectMinDelay", 3000 ));
		innerElementList.add(EmaFactory.createElementEntry().intValue( "ReconnectMaxDelay", 4000 ));
		
		innerMap.add(EmaFactory.createMapEntry().keyAscii( "Connection_2", MapEntry.MapAction.ADD, innerElementList));
		innerElementList.clear();
		
		elementList.add(EmaFactory.createElementEntry().map( "SessionChannelList", innerMap ));
		innerMap.clear();
		
		configMap.add(EmaFactory.createMapEntry().keyAscii( "SessionChannelGroup", MapEntry.MapAction.ADD, elementList ));
		elementList.clear();

		innerElementList.add(EmaFactory.createElementEntry().ascii( "ChannelType", "ChannelType::RSSL_SOCKET" ));
		innerElementList.add(EmaFactory.createElementEntry().ascii( "CompressionType", "CompressionType::ZLib"));
		innerElementList.add(EmaFactory.createElementEntry().intValue( "GuaranteedOutputBuffers", 5000));
		innerElementList.add(EmaFactory.createElementEntry().intValue( "ConnectionPingTimeout", 50000));
		innerElementList.add(EmaFactory.createElementEntry().ascii( "Host", "localhost"));
		innerElementList.add(EmaFactory.createElementEntry().ascii("Port", "14002"));
		innerElementList.add(EmaFactory.createElementEntry().intValue( "TcpNodelay", 0));

		innerMap.add(EmaFactory.createMapEntry().keyAscii( "Channel_1", MapEntry.MapAction.ADD, innerElementList));
		innerElementList.clear();
		
		innerElementList.add(EmaFactory.createElementEntry().ascii( "ChannelType", "ChannelType::RSSL_SOCKET" ));
		innerElementList.add(EmaFactory.createElementEntry().ascii( "CompressionType", "CompressionType::ZLib"));
		innerElementList.add(EmaFactory.createElementEntry().intValue( "GuaranteedOutputBuffers", 5000));
		innerElementList.add(EmaFactory.createElementEntry().intValue( "ConnectionPingTimeout", 50000));
		innerElementList.add(EmaFactory.createElementEntry().ascii( "Host", "localhost"));
		innerElementList.add(EmaFactory.createElementEntry().ascii("Port", "14003"));
		innerElementList.add(EmaFactory.createElementEntry().intValue( "TcpNodelay", 0));

		innerMap.add(EmaFactory.createMapEntry().keyAscii( "Channel_7", MapEntry.MapAction.ADD, innerElementList));
		innerElementList.clear();
		
		innerElementList.add(EmaFactory.createElementEntry().ascii( "ChannelType", "ChannelType::RSSL_SOCKET" ));
		innerElementList.add(EmaFactory.createElementEntry().ascii( "CompressionType", "CompressionType::ZLib"));
		innerElementList.add(EmaFactory.createElementEntry().intValue( "GuaranteedOutputBuffers", 5000));
		innerElementList.add(EmaFactory.createElementEntry().intValue( "ConnectionPingTimeout", 50000));
		innerElementList.add(EmaFactory.createElementEntry().ascii( "Host", "localhost"));
		innerElementList.add(EmaFactory.createElementEntry().ascii("Port", "14004"));
		innerElementList.add(EmaFactory.createElementEntry().intValue( "TcpNodelay", 0));

		innerMap.add(EmaFactory.createMapEntry().keyAscii( "Channel_10", MapEntry.MapAction.ADD, innerElementList));
		innerElementList.clear();
		
		innerElementList.add(EmaFactory.createElementEntry().ascii( "ChannelType", "ChannelType::RSSL_SOCKET" ));
		innerElementList.add(EmaFactory.createElementEntry().ascii( "CompressionType", "CompressionType::ZLib"));
		innerElementList.add(EmaFactory.createElementEntry().intValue( "GuaranteedOutputBuffers", 5000));
		innerElementList.add(EmaFactory.createElementEntry().intValue( "ConnectionPingTimeout", 50000));
		innerElementList.add(EmaFactory.createElementEntry().ascii( "Host", "localhost"));
		innerElementList.add(EmaFactory.createElementEntry().ascii("Port", "14005"));
		innerElementList.add(EmaFactory.createElementEntry().intValue( "TcpNodelay", 0));

		innerMap.add(EmaFactory.createMapEntry().keyAscii( "Channel_11", MapEntry.MapAction.ADD, innerElementList));
		innerElementList.clear();
				
		elementList.add(EmaFactory.createElementEntry().map( "ChannelList", innerMap ));
		innerMap.clear();

		configMap.add(EmaFactory.createMapEntry().keyAscii("ChannelGroup", MapEntry.MapAction.ADD, elementList ));
		elementList.clear();

		innerElementList.add(EmaFactory.createElementEntry().ascii( "DictionaryType", "DictionaryType::FileDictionary"));
		innerElementList.add(EmaFactory.createElementEntry().ascii( "RdmFieldDictionaryFileName", "./RDMFieldDictionary"));
		innerElementList.add(EmaFactory.createElementEntry().ascii( "EnumTypeDefFileName", "./enumtype.def" ));
		innerMap.add(EmaFactory.createMapEntry().keyAscii( "Dictionary_1", MapEntry.MapAction.ADD, innerElementList));
		innerElementList.clear();
		
		elementList.add(EmaFactory.createElementEntry().map( "DictionaryList", innerMap ));
		innerMap.clear();
		
		configMap.add(EmaFactory.createMapEntry().keyAscii( "DictionaryGroup", MapEntry.MapAction.ADD, elementList ));
		elementList.clear();
		
		return configMap;
	}

	public static void main(String[] args)
	{
		OmmConsumer consumer = null;
		try
		{
			AppClient appClient = new AppClient();
			
			consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig().config( createProgramaticConfig()), appClient); // use programmatic configuration parameters
			
			consumer.registerClient(EmaFactory.createReqMsg().serviceName("DIRECT_FEED").name("LSEG.L"), appClient);
			
			Thread.sleep(60000);			// API calls onRefreshMsg(), onUpdateMsg() and onStatusMsg()
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
