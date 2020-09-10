package com.rtsdk.ema.examples.training.niprovider.series400.example421__MarketPrice__ProgrammaticConfig;

//APIQA
import com.rtsdk.ema.access.EmaFactory;
import com.rtsdk.ema.access.FieldList;
import com.rtsdk.ema.access.GenericMsg;
import com.rtsdk.ema.access.Msg;
import com.rtsdk.ema.access.OmmException;
import com.rtsdk.ema.access.OmmNiProviderConfig;
import com.rtsdk.ema.access.OmmProvider;
import com.rtsdk.ema.access.OmmProviderClient;
import com.rtsdk.ema.access.OmmProviderEvent;
import com.rtsdk.ema.access.OmmReal;
import com.rtsdk.ema.access.OmmState;
import com.rtsdk.ema.access.PostMsg;
import com.rtsdk.ema.access.RefreshMsg;
import com.rtsdk.ema.access.ReqMsg;
import com.rtsdk.ema.access.StatusMsg;
import com.rtsdk.ema.access.ElementList;
import com.rtsdk.ema.access.Map;
import com.rtsdk.ema.access.MapEntry;
import com.rtsdk.ema.access.OmmArray;
import com.rtsdk.ema.access.Series;



class AppClient implements OmmProviderClient
{
	boolean _connectionUp;
	
	boolean isConnectionUp()
	{
		return _connectionUp;
	}
		
	public void onRefreshMsg(RefreshMsg refreshMsg, OmmProviderEvent event)
	{
		System.out.println("Received Refresh. Item Handle: " + event.handle() + " Closure: " + event.closure());
		
		System.out.println("Item Name: " + (refreshMsg.hasName() ? refreshMsg.name() : "<not set>"));
		System.out.println("Service Name: " + (refreshMsg.hasServiceName() ? refreshMsg.serviceName() : "<not set>"));

		System.out.println("Item State: " + refreshMsg.state());
		
		if ( refreshMsg.state().streamState() == OmmState.StreamState.OPEN)
		{
			if (refreshMsg.state().dataState() == OmmState.DataState.OK)
				_connectionUp = true;
			else
				_connectionUp = false;
		}
		else
			_connectionUp = false;		
	}
	
	public void onStatusMsg(StatusMsg statusMsg, OmmProviderEvent event) 
	{
		System.out.println("Received Status. Item Handle: " + event.handle() + " Closure: " + event.closure());

		System.out.println("Item Name: " + (statusMsg.hasName() ? statusMsg.name() : "<not set>"));
		System.out.println("Service Name: " + (statusMsg.hasServiceName() ? statusMsg.serviceName() : "<not set>"));

		if (statusMsg.hasState())
		{
			System.out.println("Item State: " +statusMsg.state());
			if ( statusMsg.state().streamState() == OmmState.StreamState.OPEN)
			{
				if (statusMsg.state().dataState() == OmmState.DataState.OK)
					_connectionUp = true;
				else
				{
					_connectionUp = false;
				}
			}
			else
				_connectionUp = false;					
		}
	}
	
	public void onGenericMsg(GenericMsg genericMsg, OmmProviderEvent event){}
	public void onAllMsg(Msg msg, OmmProviderEvent event){}
	public void onPostMsg(PostMsg postMsg, OmmProviderEvent providerEvent) {}
	public void onReqMsg(ReqMsg reqMsg, OmmProviderEvent providerEvent) {}
	public void onReissue(ReqMsg reqMsg, OmmProviderEvent providerEvent) {}
	public void onClose(ReqMsg reqMsg, OmmProviderEvent providerEvent) {}
}


public class NiProvider 
{
    static Map createProgrammaticConfig()
    {
        Map innerMap = EmaFactory.createMap();
        Map configMap = EmaFactory.createMap();
        ElementList elementList = EmaFactory.createElementList();
        ElementList innerElementList = EmaFactory.createElementList();

        elementList.add( EmaFactory.createElementEntry().ascii( "DefaultNiProvider", "Provider_1" ));

        innerElementList.add( EmaFactory.createElementEntry().ascii( "ChannelSet", "Channel_10,Channel_11" ));
        innerElementList.add( EmaFactory.createElementEntry().ascii( "Directory", "Directory_1" ));
        innerElementList.add( EmaFactory.createElementEntry().intValue( "XmlTraceToStdout", 1 ));
        innerElementList.add( EmaFactory.createElementEntry().intValue( "DispatchTimeoutApiThread", 45000 ));
        innerElementList.add( EmaFactory.createElementEntry().intValue( "ItemCountHint", 45000 ));
        innerElementList.add( EmaFactory.createElementEntry().intValue( "LoginRequestTimeOut", 45000 ));
        innerElementList.add( EmaFactory.createElementEntry().intValue( "MaxDispatchCountApiThread", 500 ));
        innerElementList.add( EmaFactory.createElementEntry().intValue( "MaxDispatchCountUserThread", 500 ));
        innerElementList.add( EmaFactory.createElementEntry().intValue( "MergeSourceDirectoryStreams", 1 ));
        innerElementList.add( EmaFactory.createElementEntry().intValue( "ReconnectAttemptLimit", 10 ));
        innerElementList.add( EmaFactory.createElementEntry().intValue( "ReconnectMaxDelay", 5000 ));
        innerElementList.add( EmaFactory.createElementEntry().intValue( "ReconnectMinDelay", 1000 ));
        innerElementList.add( EmaFactory.createElementEntry().intValue( "RecoverUserSubmitSourceDirectory", 1 ));
        innerElementList.add( EmaFactory.createElementEntry().intValue( "RemoveItemsOnDisconnect", 1 ));
        innerElementList.add( EmaFactory.createElementEntry().intValue( "RequestTimeout", 15000 ));
        innerElementList.add( EmaFactory.createElementEntry().intValue( "ServiceCountHint", 513 ));
        innerElementList.add( EmaFactory.createElementEntry().intValue( "RefreshFirstRequired", 1 ));

        innerMap.add( EmaFactory.createMapEntry().keyAscii( "Provider_1", MapEntry.MapAction.ADD, innerElementList));
        innerElementList.clear();
        elementList.add( EmaFactory.createElementEntry().map( "NiProviderList", innerMap ));
        innerMap.clear();

        configMap.add(EmaFactory.createMapEntry().keyAscii( "NiProviderGroup", MapEntry.MapAction.ADD, elementList ));
        elementList.clear();

        innerElementList.add( EmaFactory.createElementEntry().ascii( "ChannelType", "ChannelType::RSSL_SOCKET" ));
        innerElementList.add( EmaFactory.createElementEntry().ascii( "InterfaceName", "providerInterface" ));
        innerElementList.add( EmaFactory.createElementEntry().intValue( "GuaranteedOutputBuffers", 5000 ));
        innerElementList.add( EmaFactory.createElementEntry().intValue( "ConnectionPingTimeout", 30000 ));
        innerElementList.add( EmaFactory.createElementEntry().ascii( "Host", "localhost"));
        innerElementList.add( EmaFactory.createElementEntry().ascii( "Port", "14003"));
        innerElementList.add( EmaFactory.createElementEntry().intValue( "TcpNodelay", 1 ));
        innerElementList.add( EmaFactory.createElementEntry().intValue( "HighWaterMark", 6144 ));
        innerElementList.add( EmaFactory.createElementEntry().intValue( "NumInputBuffers", 10 ));
        innerElementList.add( EmaFactory.createElementEntry().intValue( "SysRecvBufSize", 0 ));
        innerElementList.add( EmaFactory.createElementEntry().intValue( "SysSendBufSize", 0 ));
        innerElementList.add( EmaFactory.createElementEntry().intValue( "DirectWrite", 1 ));
        innerElementList.add(EmaFactory.createElementEntry().ascii( "CompressionType", "CompressionType::None"));
        innerElementList.add(EmaFactory.createElementEntry().intValue( "CompressionThreshold", 100));

        innerMap.add( EmaFactory.createMapEntry().keyAscii( "Channel_10", MapEntry.MapAction.ADD, innerElementList));
        innerElementList.clear();
        innerElementList.add( EmaFactory.createElementEntry().ascii( "ChannelType", "ChannelType::channl11_type" ));
        innerElementList.add( EmaFactory.createElementEntry().ascii( "InterfaceName", "providerInterface" ));
        innerElementList.add( EmaFactory.createElementEntry().intValue( "GuaranteedOutputBuffers", 5000 ));
        innerElementList.add( EmaFactory.createElementEntry().intValue( "ConnectionPingTimeout", 30000 ));
        innerElementList.add( EmaFactory.createElementEntry().ascii( "Host", "channel11_host"));
        innerElementList.add( EmaFactory.createElementEntry().ascii( "Port", "channel11_port"));
        innerElementList.add( EmaFactory.createElementEntry().intValue( "TcpNodelay", 1 ));
        innerElementList.add( EmaFactory.createElementEntry().intValue( "HighWaterMark", 6144 ));
        innerElementList.add( EmaFactory.createElementEntry().intValue( "NumInputBuffers", 10 ));
        innerElementList.add( EmaFactory.createElementEntry().intValue( "SysRecvBufSize", 0 ));
        innerElementList.add( EmaFactory.createElementEntry().intValue( "SysSendBufSize", 0 ));
        innerElementList.add( EmaFactory.createElementEntry().intValue( "DirectWrite", 1 ));
        innerElementList.add(EmaFactory.createElementEntry().ascii( "CompressionType", "CompressionType::None"));
        innerElementList.add(EmaFactory.createElementEntry().intValue( "CompressionThreshold", 100));

        innerMap.add( EmaFactory.createMapEntry().keyAscii( "Channel_11", MapEntry.MapAction.ADD, innerElementList));
        innerElementList.clear();

        elementList.add( EmaFactory.createElementEntry().map( "ChannelList", innerMap ));
        innerMap.clear();

        configMap.add( EmaFactory.createMapEntry().keyAscii("ChannelGroup", MapEntry.MapAction.ADD, elementList ));
        elementList.clear();

        Map serviceMap = EmaFactory.createMap();
        innerElementList.add( EmaFactory.createElementEntry().intValue( "ServiceId", 0 ));
        innerElementList.add( EmaFactory.createElementEntry().ascii( "Vendor", "company name" ));
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
		AppClient appClient = new AppClient();
		boolean sendRefreshMsg = false;
			
		OmmProvider provider = null;
		try
		{
			OmmNiProviderConfig config = EmaFactory.createOmmNiProviderConfig().config( createProgrammaticConfig() );
							
			provider = EmaFactory.createOmmProvider(config.operationModel(OmmNiProviderConfig.OperationModel.USER_DISPATCH)
					.username("user"), appClient);

            provider.dispatch( 1000000 );
                        
			long itemHandle = 6;
				
			FieldList fieldList = EmaFactory.createFieldList();
				
			fieldList.add( EmaFactory.createFieldEntry().real(22, 14400, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(25, 14700, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(30, 9,  OmmReal.MagnitudeType.EXPONENT_0));
			fieldList.add( EmaFactory.createFieldEntry().real(31, 19, OmmReal.MagnitudeType.EXPONENT_0));
				
			provider.submit( EmaFactory.createRefreshMsg().serviceName("NI_PUB").name("TRI.N")
					.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
					.payload(fieldList).complete(true), itemHandle);
				
			provider.dispatch( 1000000 );
				
			for( int i = 0; i < 60; i++ )
			{
				if ( appClient.isConnectionUp())
				{
					if ( sendRefreshMsg )
					{
						fieldList.clear();
						fieldList.add( EmaFactory.createFieldEntry().real(22, 14400 + i, OmmReal.MagnitudeType.EXPONENT_NEG_2));
						fieldList.add( EmaFactory.createFieldEntry().real(25, 14700 + i, OmmReal.MagnitudeType.EXPONENT_NEG_2));
						fieldList.add( EmaFactory.createFieldEntry().real(30, 10 + i,  OmmReal.MagnitudeType.EXPONENT_0));
						fieldList.add( EmaFactory.createFieldEntry().real(31, 19 + i, OmmReal.MagnitudeType.EXPONENT_0));
							
						provider.submit( EmaFactory.createRefreshMsg().serviceName("NI_PUB").name("TRI.N")
								.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
								.payload(fieldList).complete(true), itemHandle);
						
						sendRefreshMsg = false;
					}
					else
					{
						fieldList.clear();
						fieldList.add(EmaFactory.createFieldEntry().real(22, 14400 + i, OmmReal.MagnitudeType.EXPONENT_NEG_2));
						fieldList.add(EmaFactory.createFieldEntry().real(30, 10 + i, OmmReal.MagnitudeType.EXPONENT_0));
						
						provider.submit( EmaFactory.createUpdateMsg().serviceName("NI_PUB").name("TRI.N").payload( fieldList ), itemHandle );
					}
				}
				else
				{
					sendRefreshMsg = true;
				}
				provider.dispatch( 1000000 );
			}
		}
		catch (OmmException excp)
		{
			System.out.println(excp.getMessage());
		}
		finally 
		{
			if (provider != null) provider.uninitialize();
		}
	}					
}
//END APIQA
