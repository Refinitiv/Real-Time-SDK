/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.examples.training.niprovider.series500.ex511_MP_Session_ProgrammaticCfg;

import java.util.ArrayList;
import java.util.List;

import com.refinitiv.ema.access.ChannelInformation;
import com.refinitiv.ema.access.ElementList;
import com.refinitiv.ema.access.EmaFactory;
import com.refinitiv.ema.access.FieldList;
import com.refinitiv.ema.access.GenericMsg;
import com.refinitiv.ema.access.Map;
import com.refinitiv.ema.access.MapEntry;
import com.refinitiv.ema.access.Msg;
import com.refinitiv.ema.access.OmmArray;
import com.refinitiv.ema.access.OmmException;
import com.refinitiv.ema.access.OmmInvalidUsageException;
import com.refinitiv.ema.access.OmmNiProviderConfig;
import com.refinitiv.ema.access.OmmProvider;
import com.refinitiv.ema.access.OmmProviderClient;
import com.refinitiv.ema.access.OmmProviderEvent;
import com.refinitiv.ema.access.OmmReal;
import com.refinitiv.ema.access.OmmState;
import com.refinitiv.ema.access.PostMsg;
import com.refinitiv.ema.access.RefreshMsg;
import com.refinitiv.ema.access.ReqMsg;
import com.refinitiv.ema.access.Series;
import com.refinitiv.ema.access.StatusMsg;
import com.refinitiv.ema.access.UpdateMsg;

class AppClient implements OmmProviderClient
{
	List<ChannelInformation> channelInList = new ArrayList<ChannelInformation>();
	
	public void onRefreshMsg(RefreshMsg refreshMsg, OmmProviderEvent event)
	{
		System.out.println(refreshMsg + "\nevent session info (refresh)\n");
		
		printSessionInfo(event);
	}
	
	public void onUpdateMsg(UpdateMsg updateMsg, OmmProviderEvent event) 
	{
		System.out.println(updateMsg + "\nevent session info (update)\n");
		
		printSessionInfo(event);
	}

	public void onStatusMsg(StatusMsg statusMsg, OmmProviderEvent event) 
	{
		System.out.println(statusMsg + "\nevent session info (status)\n");
		
		printSessionInfo(event);
	}
	
	void printSessionInfo(OmmProviderEvent event)
	{
		event.sessionChannelInfo(channelInList);
		
		for(ChannelInformation channelInfo : channelInList) 
		{
			System.out.println(channelInfo);
		}
	}
	
	public void onGenericMsg(GenericMsg genericMsg, OmmProviderEvent providerEvent) {}
	public void onPostMsg(PostMsg postMsg, OmmProviderEvent providerEvent) {}
	public void onReqMsg(ReqMsg reqMsg, OmmProviderEvent providerEvent) {}
	public void onReissue(ReqMsg reqMsg, OmmProviderEvent providerEvent) {}
	public void onClose(ReqMsg reqMsg, OmmProviderEvent providerEvent) {}
	public void onAllMsg(Msg msg, OmmProviderEvent providerEvent) {}
}

public class NiProvider 
{
	static Map createProgramaticConfig()
	{
		Map innerMap = EmaFactory.createMap();
		Map configMap = EmaFactory.createMap();
		ElementList elementList = EmaFactory.createElementList();
		ElementList innerElementList = EmaFactory.createElementList();
		
		elementList.add(EmaFactory.createElementEntry().ascii("DefaultNiProvider", "NiProvider_1" ));
		
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
		innerElementList.add(EmaFactory.createElementEntry().intValue( "XmlTraceWrite", 1 ));
        innerElementList.add(EmaFactory.createElementEntry().intValue( "XmlTraceRead", 1 ));
		innerElementList.add(EmaFactory.createElementEntry().intValue( "MsgKeyInUpdates", 1 ));
		innerElementList.add(EmaFactory.createElementEntry().uintValue( "SessionEnhancedItemRecovery", 1 ));

		innerMap.add(EmaFactory.createMapEntry().keyAscii( "NiProvider_1", MapEntry.MapAction.ADD, innerElementList));
		innerElementList.clear();
		
		elementList.add(EmaFactory.createElementEntry().map( "NiProviderList", innerMap ));
		innerMap.clear();

		configMap.add(EmaFactory.createMapEntry().keyAscii( "NiProviderGroup", MapEntry.MapAction.ADD, elementList ));
		elementList.clear();
		
		innerElementList.add(EmaFactory.createElementEntry().ascii( "ChannelSet", "Channel_1, Channel_2" ));
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
		innerElementList.add(EmaFactory.createElementEntry().intValue( "GuaranteedOutputBuffers", 5000));
		innerElementList.add(EmaFactory.createElementEntry().intValue( "ConnectionPingTimeout", 50000));
		innerElementList.add(EmaFactory.createElementEntry().ascii( "Host", "localhost"));
		innerElementList.add(EmaFactory.createElementEntry().ascii("Port", "14003"));
		innerElementList.add(EmaFactory.createElementEntry().intValue( "TcpNodelay", 0));

		innerMap.add(EmaFactory.createMapEntry().keyAscii( "Channel_1", MapEntry.MapAction.ADD, innerElementList));
		innerElementList.clear();
		
		innerElementList.add(EmaFactory.createElementEntry().ascii( "ChannelType", "ChannelType::RSSL_SOCKET" ));
		innerElementList.add(EmaFactory.createElementEntry().intValue( "GuaranteedOutputBuffers", 5000));
		innerElementList.add(EmaFactory.createElementEntry().intValue( "ConnectionPingTimeout", 50000));
		innerElementList.add(EmaFactory.createElementEntry().ascii( "Host", "localhost"));
		innerElementList.add(EmaFactory.createElementEntry().ascii("Port", "14004"));
		innerElementList.add(EmaFactory.createElementEntry().intValue( "TcpNodelay", 0));

		innerMap.add(EmaFactory.createMapEntry().keyAscii( "Channel_2", MapEntry.MapAction.ADD, innerElementList));
		innerElementList.clear();
		
		innerElementList.add(EmaFactory.createElementEntry().ascii( "ChannelType", "ChannelType::RSSL_SOCKET" ));
		innerElementList.add(EmaFactory.createElementEntry().intValue( "GuaranteedOutputBuffers", 5000));
		innerElementList.add(EmaFactory.createElementEntry().intValue( "ConnectionPingTimeout", 50000));
		innerElementList.add(EmaFactory.createElementEntry().ascii( "Host", "localhost"));
		innerElementList.add(EmaFactory.createElementEntry().ascii("Port", "14005"));
		innerElementList.add(EmaFactory.createElementEntry().intValue( "TcpNodelay", 0));

		innerMap.add(EmaFactory.createMapEntry().keyAscii( "Channel_10", MapEntry.MapAction.ADD, innerElementList));
		innerElementList.clear();
		
		innerElementList.add(EmaFactory.createElementEntry().ascii( "ChannelType", "ChannelType::RSSL_SOCKET" ));
		innerElementList.add(EmaFactory.createElementEntry().intValue( "GuaranteedOutputBuffers", 5000));
		innerElementList.add(EmaFactory.createElementEntry().intValue( "ConnectionPingTimeout", 50000));
		innerElementList.add(EmaFactory.createElementEntry().ascii( "Host", "localhost"));
		innerElementList.add(EmaFactory.createElementEntry().ascii("Port", "14006"));
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
		
		Map serviceMap = EmaFactory.createMap();
		innerElementList.add( EmaFactory.createElementEntry().intValue( "ServiceId", 2 ));
		innerElementList.add( EmaFactory.createElementEntry().ascii( "Vendor", "company name" ));
		innerElementList.add( EmaFactory.createElementEntry().intValue( "IsSource", 0 ));		
		innerElementList.add( EmaFactory.createElementEntry().intValue( "AcceptingConsumerStatus", 0 ));	
		innerElementList.add( EmaFactory.createElementEntry().intValue( "SupportsQoSRange", 0 ));	
		innerElementList.add( EmaFactory.createElementEntry().intValue(  "SupportsOutOfBandSnapshots", 0 ));	
		innerElementList.add( EmaFactory.createElementEntry().ascii( "ItemList", "#.itemlist" ));
		
		OmmArray array = EmaFactory.createOmmArray();
		array.add( EmaFactory.createOmmArrayEntry().ascii( "MMT_MARKET_PRICE" ));
		array.add( EmaFactory.createOmmArrayEntry().ascii( "MMT_MARKET_BY_PRICE" ));
		array.add( EmaFactory.createOmmArrayEntry().ascii( "200" ));
		innerElementList.add( EmaFactory.createElementEntry().array( "Capabilities", array ));
		array.clear();
		
		array.add( EmaFactory.createOmmArrayEntry().ascii( "Dictionary_1" ));
		innerElementList.add( EmaFactory.createElementEntry().array( "DictionariesUsed", array ));
		array.clear();			
		
		ElementList inner2 = EmaFactory.createElementList();

		Series series = EmaFactory.createSeries();
		inner2.add( EmaFactory.createElementEntry().ascii( "Timeliness", "Timeliness::RealTime" ));		
		inner2.add( EmaFactory.createElementEntry().ascii( "Rate", "Rate::TickByTick" ));			
		series.add( EmaFactory.createSeriesEntry().elementList( inner2 ));
		inner2.clear();
		
		inner2.add( EmaFactory.createElementEntry().intValue( "Timeliness", 100 ));	
		inner2.add( EmaFactory.createElementEntry().intValue( "Rate", 100 ));			
		series.add( EmaFactory.createSeriesEntry().elementList( inner2 ));
		inner2.clear();
		
		innerElementList.add( EmaFactory.createElementEntry().series( "QoS", series ));	
		
		elementList.add( EmaFactory.createElementEntry().elementList( "InfoFilter", innerElementList ));
		innerElementList.clear();
		
		innerElementList.add( EmaFactory.createElementEntry().intValue( "ServiceState", 1 ));
		innerElementList.add( EmaFactory.createElementEntry().intValue( "AcceptingRequests", 1 ));
		elementList.add( EmaFactory.createElementEntry().elementList( "StateFilter", innerElementList ));
		innerElementList.clear();		
		
		serviceMap.add( EmaFactory.createMapEntry().keyAscii( "NI_PUB", MapEntry.MapAction.ADD, elementList ));
		elementList.clear();
		innerMap.add( EmaFactory.createMapEntry().keyAscii( "Directory_1", MapEntry.MapAction.ADD, serviceMap ));

		elementList.add( EmaFactory.createElementEntry().ascii( "DefaultDirectory", "Directory_1" ));			
		elementList.add( EmaFactory.createElementEntry().map( "DirectoryList", innerMap ));
		innerMap.clear();
		
		configMap.add( EmaFactory.createMapEntry().keyAscii( "DirectoryGroup", MapEntry.MapAction.ADD, elementList ));
		elementList.clear();
		
		return configMap;
	}

	public static void main(String[] args)
	{
		OmmProvider provider = null;
		AppClient appClient = new AppClient();
		try
		{
			OmmNiProviderConfig config = EmaFactory.createOmmNiProviderConfig().config(createProgramaticConfig());			
						
			provider = EmaFactory.createOmmProvider(config.username("user"), appClient);
			
			long ibmHandle = 5;
			long triHandle = 6;
			
			FieldList fieldList = EmaFactory.createFieldList();
			
			fieldList.add( EmaFactory.createFieldEntry().real(22, 14400, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(25, 14700, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(30, 9,  OmmReal.MagnitudeType.EXPONENT_0));
			fieldList.add( EmaFactory.createFieldEntry().real(31, 19, OmmReal.MagnitudeType.EXPONENT_0));
			
			provider.submit( EmaFactory.createRefreshMsg().serviceName("NI_PUB").name("IBM.N")
					.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
					.payload(fieldList).complete(true), ibmHandle);
			
			fieldList.clear();
			
			fieldList.add( EmaFactory.createFieldEntry().real(22, 4100, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(25, 4200, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(30, 20,  OmmReal.MagnitudeType.EXPONENT_0));
			fieldList.add( EmaFactory.createFieldEntry().real(31, 40, OmmReal.MagnitudeType.EXPONENT_0));
			
			provider.submit( EmaFactory.createRefreshMsg().serviceName("NI_PUB").name("TRI.N")
					.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
					.payload(fieldList).complete(true), triHandle);
			
			
			for( int i = 0; i < 60; i++ )
			{
				Thread.sleep(1000);
				try
				{
					fieldList.clear();
					fieldList.add(EmaFactory.createFieldEntry().real(22, 14400 + i, OmmReal.MagnitudeType.EXPONENT_NEG_2));
					fieldList.add(EmaFactory.createFieldEntry().real(30, 10 + i, OmmReal.MagnitudeType.EXPONENT_0));
					
					provider.submit( EmaFactory.createUpdateMsg().serviceName("TEST_NI_PUB").name("IBM.N").payload( fieldList ), ibmHandle );
					
					fieldList.clear();
					fieldList.add(EmaFactory.createFieldEntry().real(22, 4100 + i, OmmReal.MagnitudeType.EXPONENT_NEG_2));
					fieldList.add(EmaFactory.createFieldEntry().real(30, 21 + i, OmmReal.MagnitudeType.EXPONENT_0));
					
					provider.submit( EmaFactory.createUpdateMsg().serviceName("TEST_NI_PUB").name("TRI.N").payload( fieldList ), triHandle );
				}
				catch(OmmInvalidUsageException excp) 
				{
					// There is no active channel currently associated with this session, so just continue until the channels are back up.
					if(excp.errorCode() == OmmInvalidUsageException.ErrorCode.NO_ACTIVE_CHANNEL)
					{
						continue;
					}
					else
					{
						System.out.println(excp.getMessage());
						break;
					}
				}
			}
		} 
		catch (InterruptedException | OmmException excp)
		{
			System.out.println(excp.getMessage());
		}
		finally 
		{
			if (provider != null) provider.uninitialize();
		}
	}

}
