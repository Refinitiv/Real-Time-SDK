///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2018. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.examples.training.niprovider.series400.example421__MarketPrice__ProgrammaticConfig;

import com.thomsonreuters.ema.access.ElementList;
import com.thomsonreuters.ema.access.EmaFactory;
import com.thomsonreuters.ema.access.FieldList;
import com.thomsonreuters.ema.access.Map;
import com.thomsonreuters.ema.access.MapEntry;
import com.thomsonreuters.ema.access.OmmArray;
import com.thomsonreuters.ema.access.OmmException;
import com.thomsonreuters.ema.access.OmmNiProviderConfig;
import com.thomsonreuters.ema.access.OmmProvider;
import com.thomsonreuters.ema.access.OmmReal;
import com.thomsonreuters.ema.access.OmmState;
import com.thomsonreuters.ema.access.Series;

public class NiProvider {

	static Map createProgrammaticConfig()
	{
		Map innerMap = EmaFactory.createMap();
		Map configMap = EmaFactory.createMap();
		ElementList elementList = EmaFactory.createElementList();
		ElementList innerElementList = EmaFactory.createElementList();
		
		elementList.add( EmaFactory.createElementEntry().ascii( "DefaultNiProvider", "Provider_1" ));
		
		innerElementList.add( EmaFactory.createElementEntry().ascii( "Channel", "Channel_10" ));
		innerElementList.add( EmaFactory.createElementEntry().ascii( "Directory", "Directory_1" ));
		innerElementList.add( EmaFactory.createElementEntry().intValue( "XmlTraceToStdout", 0 ));
		innerElementList.add( EmaFactory.createElementEntry().intValue( "RefreshFirstRequired", 1 ));

		innerMap.add( EmaFactory.createMapEntry().keyAscii( "Provider_1", MapEntry.MapAction.ADD, innerElementList));
		innerElementList.clear();
		
		elementList.add( EmaFactory.createElementEntry().map( "NiProviderList", innerMap ));
		innerMap.clear();

		configMap.add(EmaFactory.createMapEntry().keyAscii( "NiProviderGroup", MapEntry.MapAction.ADD, elementList ));
		elementList.clear();

		innerElementList.add( EmaFactory.createElementEntry().ascii( "ChannelType", "ChannelType::RSSL_SOCKET" ));
		innerElementList.add( EmaFactory.createElementEntry().intValue( "GuaranteedOutputBuffers", 5000 ));
		innerElementList.add( EmaFactory.createElementEntry().intValue( "ConnectionPingTimeout", 30000 ));
		innerElementList.add( EmaFactory.createElementEntry().ascii( "Host", "localhost"));
		innerElementList.add( EmaFactory.createElementEntry().ascii( "Port", "14003"));
		innerElementList.add( EmaFactory.createElementEntry().intValue( "TcpNodelay", 1 ));

		innerMap.add( EmaFactory.createMapEntry().keyAscii( "Channel_10", MapEntry.MapAction.ADD, innerElementList));
		innerElementList.clear();
				
		elementList.add( EmaFactory.createElementEntry().map( "ChannelList", innerMap ));
		innerMap.clear();

		configMap.add( EmaFactory.createMapEntry().keyAscii("ChannelGroup", MapEntry.MapAction.ADD, elementList ));
		elementList.clear();

		Map serviceMap = EmaFactory.createMap();
		innerElementList.add( EmaFactory.createElementEntry().intValue( "ServiceId", 0 ));
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

		return configMap;
	}	
	
	public static void main(String[] args)
	{
		OmmProvider provider = null;
		try
		{
			OmmNiProviderConfig config = EmaFactory.createOmmNiProviderConfig().config( createProgrammaticConfig() );
			
			provider = EmaFactory.createOmmProvider(config.username("user"));
			
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
			
			Thread.sleep(1000);
			
			for( int i = 0; i < 60; i++ )
			{
				fieldList.clear();
				fieldList.add(EmaFactory.createFieldEntry().real(22, 14400 + i, OmmReal.MagnitudeType.EXPONENT_NEG_2));
				fieldList.add(EmaFactory.createFieldEntry().real(30, 10 + i, OmmReal.MagnitudeType.EXPONENT_0));
				
				provider.submit( EmaFactory.createUpdateMsg().serviceName("NI_PUB").name("IBM.N").payload( fieldList ), ibmHandle );
				
				fieldList.clear();
				fieldList.add(EmaFactory.createFieldEntry().real(22, 4100 + i, OmmReal.MagnitudeType.EXPONENT_NEG_2));
				fieldList.add(EmaFactory.createFieldEntry().real(30, 21 + i, OmmReal.MagnitudeType.EXPONENT_0));
				
				provider.submit( EmaFactory.createUpdateMsg().serviceName("NI_PUB").name("TRI.N").payload( fieldList ), triHandle );
				Thread.sleep(1000);
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
