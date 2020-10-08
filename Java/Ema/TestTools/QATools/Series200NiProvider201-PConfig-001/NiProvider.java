///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.examples.training.niprovider.series200.ex201_MP_TunnelingConnection;

import com.refinitiv.ema.access.ElementList;
import com.refinitiv.ema.access.EmaFactory;
import com.refinitiv.ema.access.FieldList;
import com.refinitiv.ema.access.Map;
import com.refinitiv.ema.access.MapEntry;
import com.refinitiv.ema.access.OmmArray;
import com.refinitiv.ema.access.OmmException;
import com.refinitiv.ema.access.OmmNiProviderConfig;
import com.refinitiv.ema.access.OmmProvider;
import com.refinitiv.ema.access.OmmReal;
import com.refinitiv.ema.access.OmmState;
import com.refinitiv.ema.access.Series;

public class NiProvider {

	static void printHelp()
	{

	    System.out.println("\nOptions:\n" + "  -?\tShows this usage\n" 
	    		+ "  if the application will attempt to make an http or encrypted \n "
	    		+ "           connection, ChannelType must need to be set to ChannelType::RSSL_HTTP \n"
	    		+ "            or ChannelType::RSSL_ENCRYPTED in EMA configuration file.\n"
	    		+ "  -ph Proxy host name.\n"
	    		+ "  -pp Proxy port number.\n"
	    		+ "  -plogin User name on proxy server.\n"
	    		+ "  -ppasswd Password on proxy server.\n" 
	    		+ "  -pdomain Proxy Domain.\n"
	    		+ "  -krbfile Proxy KRB file.\n" 
	    		+ "  -keyfile keystore file for encryption.\n"
	    		+ "  -keypasswd keystore password for encryption.\n"
	    		+ "\n");
	}

	static boolean readCommandlineArgs(String[] args, OmmNiProviderConfig config)
	{
		    try
		    {
		        int argsCount = 0;

		        while (argsCount < args.length)
		        {
		            if (0 == args[argsCount].compareTo("-?"))
		            {
		                printHelp();
		                return false;
		            }
	    			else if ("-keyfile".equals(args[argsCount]))
	    			{
	    				config.tunnelingKeyStoreFile(argsCount < (args.length-1) ? args[++argsCount] : null);
	    				config.tunnelingSecurityProtocol("TLS");
	    				++argsCount;				
	    			}	
	    			else if ("-keypasswd".equals(args[argsCount]))
	    			{
	    				config.tunnelingKeyStorePasswd(argsCount < (args.length-1) ? args[++argsCount] : null);
	    				++argsCount;				
	    			}
	    			else if ("-ph".equals(args[argsCount]))
	    			{
	    				config.tunnelingProxyHostName(args[++argsCount]);
	    				++argsCount;				
	    			}	
	    			else if ("-pp".equals(args[argsCount]))
	    			{
	    				config.tunnelingProxyPort(argsCount < (args.length-1) ? args[++argsCount] : null);
	    				++argsCount;				
	    			}			
	    			else if ("-plogin".equals(args[argsCount]))
	    			{
	    				config.tunnelingCredentialUserName(argsCount < (args.length-1) ? args[++argsCount] : null);
	    				++argsCount;				
	    			}	
	    			else if ("-ppasswd".equals(args[argsCount]))
	    			{
	    				config.tunnelingCredentialPasswd(argsCount < (args.length-1) ? args[++argsCount] : null);
	    				++argsCount;				
	    			}			
	    			else if ("-pdomain".equals(args[argsCount]))
	    			{
	    				config.tunnelingCredentialDomain(argsCount < (args.length-1) ? args[++argsCount] : null);
	    				++argsCount;				
	    			}	
	    			else if ("-krbfile".equals(args[argsCount]))
	    			{
	    				config.tunnelingCredentialKRB5ConfigFile(argsCount < (args.length-1) ? args[++argsCount] : null);
	    				++argsCount;				
	    			}	
	    			else // unrecognized command line argument
	    			{
	    				printHelp();
	    				return false;
	    			}			
	    		}
	        }
	        catch (Exception e)
	        {
	        	printHelp();
	            return false;
	        }
			
			return true;
	}
	
	static Map createProgrammaticConfig()
	{
		Map innerMap = EmaFactory.createMap();
		Map configMap = EmaFactory.createMap();
		ElementList elementList = EmaFactory.createElementList();
		ElementList innerElementList = EmaFactory.createElementList();
		
		elementList.add( EmaFactory.createElementEntry().ascii( "DefaultNiProvider", "Provider_4" ));
		
		innerElementList.add( EmaFactory.createElementEntry().ascii( "Channel", "Channel_13" ));
		innerElementList.add( EmaFactory.createElementEntry().ascii( "Directory", "Directory_1" ));
		innerElementList.add( EmaFactory.createElementEntry().intValue( "XmlTraceToStdout", 1 ));
		innerElementList.add( EmaFactory.createElementEntry().intValue( "RefreshFirstRequired", 1 ));

		innerMap.add( EmaFactory.createMapEntry().keyAscii( "Provider_4", MapEntry.MapAction.ADD, innerElementList));
		innerElementList.clear();
		
		elementList.add( EmaFactory.createElementEntry().map( "NiProviderList", innerMap ));
		innerMap.clear();

		configMap.add(EmaFactory.createMapEntry().keyAscii( "NiProviderGroup", MapEntry.MapAction.ADD, elementList ));
		elementList.clear();

		innerElementList.add( EmaFactory.createElementEntry().ascii( "ChannelType", "ChannelType::RSSL_ENCRYPTED" ));
		innerElementList.add( EmaFactory.createElementEntry().ascii( "CompressionType", "CompressionType::None" ));
        innerElementList.add( EmaFactory.createElementEntry().intValue( "GuaranteedOutputBuffers", 5000 ));
        innerElementList.add( EmaFactory.createElementEntry().intValue( "ConnectionPingTimeout", 50000 ));
		innerElementList.add( EmaFactory.createElementEntry().ascii( "Host", "localhost" ));
		innerElementList.add( EmaFactory.createElementEntry().ascii( "Port", "14002" ));
        innerElementList.add( EmaFactory.createElementEntry().ascii("ObjectName", "P_ObjectName" ));
        innerElementList.add( EmaFactory.createElementEntry().intValue( "TcpNodelay", 1));
        innerElementList.add( EmaFactory.createElementEntry().ascii("ProxyHost", "proxyHostToConnectTo" ));
        innerElementList.add( EmaFactory.createElementEntry().ascii("ProxyPort", "proxyPortToConnectTo" ));


		innerMap.add( EmaFactory.createMapEntry().keyAscii( "Channel_13", MapEntry.MapAction.ADD, innerElementList));
		innerElementList.clear();
				
		elementList.add( EmaFactory.createElementEntry().map( "ChannelList", innerMap ));
		innerMap.clear();

		configMap.add( EmaFactory.createMapEntry().keyAscii("ChannelGroup", MapEntry.MapAction.ADD, elementList ));
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
		
		serviceMap.add( EmaFactory.createMapEntry().keyAscii( "TEST_NI_PUB", MapEntry.MapAction.ADD, elementList ));
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
			OmmNiProviderConfig config = EmaFactory.createOmmNiProviderConfig();
			
			if (!readCommandlineArgs(args, config))
                return;
			
			provider = EmaFactory.createOmmProvider(config.config(createProgrammaticConfig()).providerName("Provider_4").username("user"));
	
			
			long ibmHandle = 5;
			long triHandle = 6;
			
			FieldList fieldList = EmaFactory.createFieldList();
			
			fieldList.add( EmaFactory.createFieldEntry().real(22, 14400, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(25, 14700, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(30, 9,  OmmReal.MagnitudeType.EXPONENT_0));
			fieldList.add( EmaFactory.createFieldEntry().real(31, 19, OmmReal.MagnitudeType.EXPONENT_0));
			
			provider.submit( EmaFactory.createRefreshMsg().serviceName("TEST_NI_PUB").name("IBM.N")
					.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
					.payload(fieldList).complete(true), ibmHandle);
			
			fieldList.clear();
			
			fieldList.add( EmaFactory.createFieldEntry().real(22, 4100, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(25, 4200, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(30, 20,  OmmReal.MagnitudeType.EXPONENT_0));
			fieldList.add( EmaFactory.createFieldEntry().real(31, 40, OmmReal.MagnitudeType.EXPONENT_0));
			
			provider.submit( EmaFactory.createRefreshMsg().serviceName("TEST_NI_PUB").name("TRI.N")
					.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
					.payload(fieldList).complete(true), triHandle);
			
			Thread.sleep(1000);
			
			for( int i = 0; i < 60; i++ )
			{
				fieldList.clear();
				fieldList.add(EmaFactory.createFieldEntry().real(22, 14400 + i, OmmReal.MagnitudeType.EXPONENT_NEG_2));
				fieldList.add(EmaFactory.createFieldEntry().real(30, 10 + i, OmmReal.MagnitudeType.EXPONENT_0));
				
				provider.submit( EmaFactory.createUpdateMsg().serviceName("TEST_NI_PUB").name("IBM.N").payload( fieldList ), ibmHandle );
				
				fieldList.clear();
				fieldList.add(EmaFactory.createFieldEntry().real(22, 4100 + i, OmmReal.MagnitudeType.EXPONENT_NEG_2));
				fieldList.add(EmaFactory.createFieldEntry().real(30, 21 + i, OmmReal.MagnitudeType.EXPONENT_0));
				
				provider.submit( EmaFactory.createUpdateMsg().serviceName("TEST_NI_PUB").name("TRI.N").payload( fieldList ), triHandle );
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
