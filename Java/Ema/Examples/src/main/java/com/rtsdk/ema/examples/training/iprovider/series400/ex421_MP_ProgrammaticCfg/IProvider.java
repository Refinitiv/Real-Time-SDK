///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.rtsdk.ema.examples.training.iprovider.series400.ex421_MP_ProgrammaticCfg;

import com.rtsdk.ema.access.ElementList;
import com.rtsdk.ema.access.EmaFactory;
import com.rtsdk.ema.access.FieldList;
import com.rtsdk.ema.access.GenericMsg;
import com.rtsdk.ema.access.Map;
import com.rtsdk.ema.access.MapEntry;
import com.rtsdk.ema.access.Msg;
import com.rtsdk.ema.access.OmmArray;
import com.rtsdk.ema.access.OmmException;
import com.rtsdk.ema.access.OmmIProviderConfig;
import com.rtsdk.ema.access.OmmProvider;
import com.rtsdk.ema.access.OmmProviderClient;
import com.rtsdk.ema.access.OmmProviderEvent;
import com.rtsdk.ema.access.OmmReal;
import com.rtsdk.ema.access.OmmState;
import com.rtsdk.ema.access.PostMsg;
import com.rtsdk.ema.access.RefreshMsg;
import com.rtsdk.ema.access.ReqMsg;
import com.rtsdk.ema.access.Series;
import com.rtsdk.ema.access.StatusMsg;
import com.rtsdk.ema.access.UpdateMsg;
import com.rtsdk.ema.rdm.EmaRdm;

class AppClient implements OmmProviderClient
{
	public long itemHandle = 0;

	public void onReqMsg(ReqMsg reqMsg, OmmProviderEvent providerEvent)
	{
		switch(reqMsg.domainType())
		{
		case EmaRdm.MMT_LOGIN:
			processLoginRequest(reqMsg,providerEvent);
			break;
		case EmaRdm.MMT_MARKET_PRICE:
			processMarketPriceRequest(reqMsg,providerEvent);
			break;
		default:
			processInvalidItemRequest(reqMsg,providerEvent);
			break;
		}
	}
	
	public void onRefreshMsg(RefreshMsg refreshMsg, OmmProviderEvent providerEvent) {}
	public void onStatusMsg(StatusMsg statusMsg, OmmProviderEvent providerEvent) {}
	public void onGenericMsg(GenericMsg genericMsg, OmmProviderEvent providerEvent) {}
	public void onPostMsg(PostMsg postMsg, OmmProviderEvent providerEvent) {}
	public void onReissue(ReqMsg reqMsg, OmmProviderEvent providerEvent) {}
	public void onClose(ReqMsg reqMsg, OmmProviderEvent providerEvent) {}
	public void onAllMsg(Msg msg, OmmProviderEvent providerEvent) {}
	
	void processLoginRequest(ReqMsg reqMsg, OmmProviderEvent event)
	{
		event.provider().submit(EmaFactory.createRefreshMsg().domainType(EmaRdm.MMT_LOGIN).name(reqMsg.name()).nameType(EmaRdm.USER_NAME).
				complete(true).solicited(true).state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "Login accepted").
				attrib( EmaFactory.createElementList() ), event.handle());
	}
	
	void processMarketPriceRequest(ReqMsg reqMsg, OmmProviderEvent event)
	{
		if ( itemHandle != 0 )
		{
			processInvalidItemRequest(reqMsg, event);
			return;
		}
		
		FieldList fieldList = EmaFactory.createFieldList();
		
		fieldList.add(EmaFactory.createFieldEntry().ascii(3,  reqMsg.name()));
		fieldList.add(EmaFactory.createFieldEntry().enumValue(15, 840));
		fieldList.add(EmaFactory.createFieldEntry().real(21, 3900, OmmReal.MagnitudeType.EXPONENT_NEG_2));
		fieldList.add(EmaFactory.createFieldEntry().real(22, 3990, OmmReal.MagnitudeType.EXPONENT_NEG_2));
		fieldList.add(EmaFactory.createFieldEntry().real(25, 3994, OmmReal.MagnitudeType.EXPONENT_NEG_2));
		fieldList.add(EmaFactory.createFieldEntry().real(30, 9,  OmmReal.MagnitudeType.EXPONENT_0));
		fieldList.add(EmaFactory.createFieldEntry().real(31, 19, OmmReal.MagnitudeType.EXPONENT_0));
		
		event.provider().submit(EmaFactory.createRefreshMsg().serviceName(reqMsg.serviceName()).name(reqMsg.name()).
				state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "Refresh Completed").solicited(true).
				payload(fieldList).complete(true), event.handle());
		
		itemHandle = event.handle();
	}
	
	void processInvalidItemRequest(ReqMsg reqMsg, OmmProviderEvent event)
	{
		event.provider().submit(EmaFactory.createStatusMsg().name(reqMsg.name()).serviceName(reqMsg.serviceName()).
				domainType(reqMsg.domainType()).
				state(OmmState.StreamState.CLOSED, OmmState.DataState.SUSPECT, OmmState.StatusCode.NOT_FOUND, "Item not found"),
				event.handle());
	}
	
}

public class IProvider
{
	
	static Map createProgrammaticConfig()
	{
		Map innerMap = EmaFactory.createMap();
		Map configMap = EmaFactory.createMap();
		ElementList elementList = EmaFactory.createElementList();
		ElementList innerElementList = EmaFactory.createElementList();
		
		elementList.add( EmaFactory.createElementEntry().ascii( "DefaultIProvider", "Provider_1" ));
		
		innerElementList.add( EmaFactory.createElementEntry().ascii( "Server", "Server_1" ));
		innerElementList.add( EmaFactory.createElementEntry().ascii( "Directory", "Directory_2" ));
		innerElementList.add( EmaFactory.createElementEntry().intValue( "ItemCountHint", 10000 ));
		innerElementList.add( EmaFactory.createElementEntry().intValue( "ServiceCountHint", 10000 ));
		innerElementList.add( EmaFactory.createElementEntry().intValue( "DispatchTimeoutUserThread", 500 ));
		innerElementList.add( EmaFactory.createElementEntry().intValue( "MaxDispatchCountApiThread", 500 ));
		innerElementList.add( EmaFactory.createElementEntry().intValue( "MaxDispatchCountUserThread", 500 ));
		innerElementList.add( EmaFactory.createElementEntry().intValue( "RefreshFirstRequired", 1 ));
		innerElementList.add( EmaFactory.createElementEntry().intValue( "XmlTraceToStdout", 0 ));

		innerMap.add( EmaFactory.createMapEntry().keyAscii( "Provider_1", MapEntry.MapAction.ADD, innerElementList));
		innerElementList.clear();
		
		elementList.add( EmaFactory.createElementEntry().map( "IProviderList", innerMap ));
		innerMap.clear();

		configMap.add(EmaFactory.createMapEntry().keyAscii( "IProviderGroup", MapEntry.MapAction.ADD, elementList ));
		elementList.clear();

		innerElementList.add( EmaFactory.createElementEntry().ascii( "ServerType", "ServerType::RSSL_SOCKET" ));
		innerElementList.add( EmaFactory.createElementEntry().ascii( "CompressionType", "CompressionType::ZLib" ));
		innerElementList.add( EmaFactory.createElementEntry().intValue( "GuaranteedOutputBuffers", 5000 ));
		innerElementList.add( EmaFactory.createElementEntry().intValue( "ConnectionPingTimeout", 30000 ));
		innerElementList.add( EmaFactory.createElementEntry().intValue( "TcpNodelay", 1 ));
		innerElementList.add( EmaFactory.createElementEntry().ascii( "Port", "14002"));

		innerMap.add( EmaFactory.createMapEntry().keyAscii( "Server_1", MapEntry.MapAction.ADD, innerElementList));
		innerElementList.clear();
				
		elementList.add( EmaFactory.createElementEntry().map( "ServerList", innerMap ));
		innerMap.clear();

		configMap.add( EmaFactory.createMapEntry().keyAscii("ServerGroup", MapEntry.MapAction.ADD, elementList ));
		elementList.clear();

		innerElementList.add( EmaFactory.createElementEntry().ascii( "DictionaryType", "DictionaryType::FileDictionary" ));
		innerElementList.add( EmaFactory.createElementEntry().ascii( "RdmFieldDictionaryFileName", "./RDMFieldDictionary" ));
		innerElementList.add( EmaFactory.createElementEntry().ascii( "EnumTypeDefFileName", "./enumtype.def" ));
		innerElementList.add( EmaFactory.createElementEntry().ascii( "RdmFieldDictionaryItemName", "RWFFld" ));
		innerElementList.add( EmaFactory.createElementEntry().ascii( "EnumTypeDefItemName", "RWFEnum" ));
		innerMap.add( EmaFactory.createMapEntry().keyAscii( "Dictionary_3", MapEntry.MapAction.ADD, innerElementList ));
		innerElementList.clear();
		
		elementList.add( EmaFactory.createElementEntry().map( "DictionaryList", innerMap ));
		innerMap.clear();
		
		configMap.add( EmaFactory.createMapEntry().keyAscii( "DictionaryGroup", MapEntry.MapAction.ADD, elementList ));
		elementList.clear();
		
		Map serviceMap = EmaFactory.createMap();

		innerElementList.add( EmaFactory.createElementEntry().intValue( "ServiceId", 1 ));
		innerElementList.add( EmaFactory.createElementEntry().ascii( "Vendor", "company name" ));
		innerElementList.add( EmaFactory.createElementEntry().intValue( "IsSource", 0 ));		
		innerElementList.add( EmaFactory.createElementEntry().intValue( "AcceptingConsumerStatus", 0 ));	
		innerElementList.add( EmaFactory.createElementEntry().intValue( "SupportsQoSRange", 0 ));	
		innerElementList.add( EmaFactory.createElementEntry().intValue(  "SupportsOutOfBandSnapshots", 0 ));	
		innerElementList.add( EmaFactory.createElementEntry().ascii( "ItemList", "#.itemlist" ));
		
		OmmArray array = EmaFactory.createOmmArray();
		array.add( EmaFactory.createOmmArrayEntry().ascii( "MMT_DICTIONARY" ));
		array.add( EmaFactory.createOmmArrayEntry().ascii( "MMT_MARKET_PRICE" ));
		array.add( EmaFactory.createOmmArrayEntry().ascii( "MMT_MARKET_BY_PRICE" ));
		array.add( EmaFactory.createOmmArrayEntry().ascii( "200" ));
		innerElementList.add( EmaFactory.createElementEntry().array( "Capabilities", array ));
		array.clear();
	
		array.add( EmaFactory.createOmmArrayEntry().ascii( "Dictionary_3" ));
		innerElementList.add( EmaFactory.createElementEntry().array( "DictionariesProvided", array ));
		array.clear();
		
		array.add( EmaFactory.createOmmArrayEntry().ascii( "Dictionary_3" ));
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
		
		serviceMap.add( EmaFactory.createMapEntry().keyAscii( "DIRECT_FEED", MapEntry.MapAction.ADD, elementList ));
		elementList.clear();
		innerMap.add( EmaFactory.createMapEntry().keyAscii( "Directory_2", MapEntry.MapAction.ADD, serviceMap ));

		elementList.add( EmaFactory.createElementEntry().ascii( "DefaultDirectory", "Directory_2" ));			
		elementList.add( EmaFactory.createElementEntry().map( "DirectoryList", innerMap ));
		innerMap.clear();
		
		configMap.add( EmaFactory.createMapEntry().keyAscii( "DirectoryGroup", MapEntry.MapAction.ADD, elementList ));

		return configMap;
	}
	
	public static void main(String[] args)
	{
		OmmProvider provider = null;
		try
		{
			AppClient appClient = new AppClient();
			FieldList fieldList = EmaFactory.createFieldList();
			UpdateMsg updateMsg = EmaFactory.createUpdateMsg();

			provider = EmaFactory.createOmmProvider( EmaFactory.createOmmIProviderConfig().config( createProgrammaticConfig() )
																						.operationModel( OmmIProviderConfig.OperationModel.USER_DISPATCH ),	appClient );
			
			while(appClient.itemHandle == 0)
			{
				provider.dispatch(1000);
				Thread.sleep(1000);
			}
			
			for(int i = 0; i < 60; i++)
			{
				provider.dispatch(1000);
		
				fieldList.clear();
				fieldList.add(EmaFactory.createFieldEntry().real(22, 3991 + i, OmmReal.MagnitudeType.EXPONENT_NEG_2));
				fieldList.add(EmaFactory.createFieldEntry().real(25, 3994 + i, OmmReal.MagnitudeType.EXPONENT_NEG_2));
				fieldList.add(EmaFactory.createFieldEntry().real(30, 10 + i, OmmReal.MagnitudeType.EXPONENT_0));
				fieldList.add(EmaFactory.createFieldEntry().real(31, 19 + i, OmmReal.MagnitudeType.EXPONENT_0));
			
				provider.submit(updateMsg.clear().payload(fieldList), appClient.itemHandle );
				
				Thread.sleep(1000);
			}
		}
		catch (OmmException | InterruptedException excp)
		{
			System.out.println(excp.getMessage());
		} 
		finally 
		{
			if (provider != null) provider.uninitialize();
		}
	}
}
