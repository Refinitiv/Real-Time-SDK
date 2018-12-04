///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2018. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.examples.training.niprovider.series200.example201__MarketPrice__TunnelingConnection;

import com.thomsonreuters.ema.access.EmaFactory;
import com.thomsonreuters.ema.access.FieldList;
import com.thomsonreuters.ema.access.OmmException;
import com.thomsonreuters.ema.access.OmmNiProviderConfig;
import com.thomsonreuters.ema.access.OmmProvider;
import com.thomsonreuters.ema.access.OmmReal;
import com.thomsonreuters.ema.access.OmmState;
//APIQA
import com.thomsonreuters.ema.access.Map;
import com.thomsonreuters.ema.access.MapEntry;
import java.nio.ByteBuffer;
import com.thomsonreuters.ema.rdm.EmaRdm;
import com.thomsonreuters.ema.access.OmmProviderClient;
import com.thomsonreuters.ema.access.OmmProviderEvent;
import com.thomsonreuters.ema.access.RefreshMsg;
import com.thomsonreuters.ema.access.ReqMsg;
import com.thomsonreuters.ema.access.StatusMsg;
import com.thomsonreuters.ema.access.GenericMsg;
import com.thomsonreuters.ema.access.Msg;
import com.thomsonreuters.ema.access.PostMsg;

import com.thomsonreuters.ema.access.DataType.DataTypes;
import com.thomsonreuters.ema.rdm.DataDictionary;

//END APIQA
// APIQA
class AppClient implements OmmProviderClient
{
        public DataDictionary dataDictionary = EmaFactory.createDataDictionary();
        public boolean fldDictComplete = false;
        public boolean enumTypeComplete = false;
	    public boolean dumpDictionary = false;
	    

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
				decode(refreshMsg, refreshMsg.complete());
				
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
		
		void decode(Msg msg, boolean complete)
        {
                switch (msg.payload().dataType())
                {
                case DataTypes.SERIES:

                        if ( msg.name().equals("RWFFld") )
                        {
                                dataDictionary.decodeFieldDictionary(msg.payload().series(), EmaRdm.DICTIONARY_NORMAL);

                                if ( complete )
                                {
                                        fldDictComplete = true;
                                }
                        }
                        else if ( msg.name().equals("RWFEnum") )
                        {
                                dataDictionary.decodeEnumTypeDictionary(msg.payload().series(), EmaRdm.DICTIONARY_NORMAL);

                                if ( complete )
                                {
                                        enumTypeComplete = true;
                                }
                        }

                        if ( fldDictComplete && enumTypeComplete )
                        {
                                System.out.println();
                                System.out.println("Dictionary download complete");
                                System.out.println("Dictionary Id : " + dataDictionary.dictionaryId());
                                System.out.println("Dictionary field version : " + dataDictionary.fieldVersion());
                                System.out.println("Number of dictionary entries : " + dataDictionary.entries().size());

                                if ( dumpDictionary )
                                        System.out.println(dataDictionary);
                        }

                        break;
                default:
                        break;
                }
        }
		
}
// END APIQA

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
				//APIQA
	    		+ "  -objectname objectName set.\n"
				+ "  -reqDict enable to run the test with dictionary streaming.\n"
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
						//APIQA
	    			else if ("-objectname".equals(args[argsCount]))
	    			{
	    				config.tunnelingObjectName(args[++argsCount]);
	    				++argsCount;								
	    			}
					else if (args[argsCount].compareTo("-reqDict") == 0)
	    			{
	    				++argsCount;								
	    			}
						//END APIQA
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

	
	public static void main(String[] args)
	{
		AppClient appClient = new AppClient();
        boolean sendRefreshMsg = false;
		boolean reqDict = false;

		OmmProvider provider = null;
		try
		{
			OmmNiProviderConfig config = EmaFactory.createOmmNiProviderConfig();
			
			if (!readCommandlineArgs(args, config))
                return;
			
			provider = EmaFactory.createOmmProvider(config.operationModel(OmmNiProviderConfig.OperationModel.USER_DISPATCH)
			            .username("user").providerName("Provider_3"), appClient);
						
			provider.dispatch( 1000000 );
			
			int idx = 0;

            while ( idx < args.length )
            {
                if ( args[idx].compareTo("-reqDict") == 0 )
                {
                    reqDict = true;
					appClient.dumpDictionary = true;
                }

                ++idx;
            }

			if ( reqDict ) 
			{
				provider.registerClient(EmaFactory.createReqMsg().name("RWFFld").filter(EmaRdm.DICTIONARY_NORMAL)
                                        .serviceName("NI_PUB").domainType(EmaRdm.MMT_DICTIONARY), appClient);

                provider.registerClient(EmaFactory.createReqMsg().name("RWFEnum").filter(EmaRdm.DICTIONARY_NORMAL)
                                        .serviceName("NI_PUB").domainType(EmaRdm.MMT_DICTIONARY), appClient);

			}
			long ibmHandle = 5;
			long triHandle = 6;
		  	//APIQA	
		    long aaoHandle = 7;
            long aggHandle = 8;
            Map map = EmaFactory.createMap();            
			FieldList summary = EmaFactory.createFieldList();
			FieldList entryLoad = EmaFactory.createFieldList();
            
            summary.add(EmaFactory.createFieldEntry().enumValue(15, 840));
            summary.add(EmaFactory.createFieldEntry().enumValue(53, 1));
            summary.add(EmaFactory.createFieldEntry().enumValue(3423, 1));
            summary.add(EmaFactory.createFieldEntry().enumValue(1709, 2));
            
            map.summaryData(summary);
            
            entryLoad.add(EmaFactory.createFieldEntry().realFromDouble(3427, 7.76, OmmReal.MagnitudeType.EXPONENT_NEG_2));
            entryLoad.add(EmaFactory.createFieldEntry().realFromDouble(3429, 9600));
            entryLoad.add(EmaFactory.createFieldEntry().enumValue(3428, 2));
            entryLoad.add(EmaFactory.createFieldEntry().rmtes(212, ByteBuffer.wrap("Market Maker".getBytes())));
            
            map.add(EmaFactory.createMapEntry().keyAscii("100", MapEntry.MapAction.ADD, entryLoad));

            provider.submit(EmaFactory.createRefreshMsg().domainType(EmaRdm.MMT_MARKET_BY_ORDER).serviceName("NI_PUB").name("AAO.V")
                            .state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
                            .payload(map).complete(true), aaoHandle);

            summary.clear();
            
            summary.add(EmaFactory.createFieldEntry().enumValue(15, 840));
            summary.add(EmaFactory.createFieldEntry().enumValue(53, 1));
            summary.add(EmaFactory.createFieldEntry().enumValue(3423, 1));
            summary.add(EmaFactory.createFieldEntry().enumValue(1709, 2));
            
            map.clear();
            
            map.summaryData(summary);

            entryLoad.clear();
            
            entryLoad.add(EmaFactory.createFieldEntry().realFromDouble(3427, 9.92, OmmReal.MagnitudeType.EXPONENT_NEG_2));
            entryLoad.add(EmaFactory.createFieldEntry().realFromDouble(3429, 1200));
            entryLoad.add(EmaFactory.createFieldEntry().enumValue(3428, 2));
            entryLoad.add(EmaFactory.createFieldEntry().rmtes(212, ByteBuffer.wrap("Market Maker".getBytes())));
            
            map.add(EmaFactory.createMapEntry().keyAscii("222", MapEntry.MapAction.ADD, entryLoad));
            
            provider.submit(EmaFactory.createRefreshMsg().domainType(EmaRdm.MMT_MARKET_BY_ORDER).serviceName("NI_PUB").name("AGG.V")
                            .state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
                            .payload(map).complete(true), aggHandle);
		  	//END APIQA	
	
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
			
			//Thread.sleep(1000);
			provider.dispatch( 1000000 );

			for( int i = 0; i < 60; i++ )
			{
				if ( appClient.isConnectionUp())
				{
					if ( sendRefreshMsg )
					{
					    summary.clear();
						summary.add(EmaFactory.createFieldEntry().enumValue(15, 840));
						summary.add(EmaFactory.createFieldEntry().enumValue(53, 1));
						summary.add(EmaFactory.createFieldEntry().enumValue(3423, 1));
						summary.add(EmaFactory.createFieldEntry().enumValue(1709, 2));
						
						map.clear();
						
						map.summaryData(summary);
						
						entryLoad.clear();
						
						entryLoad.add(EmaFactory.createFieldEntry().realFromDouble(3427, 7.76, OmmReal.MagnitudeType.EXPONENT_NEG_2));
						entryLoad.add(EmaFactory.createFieldEntry().realFromDouble(3429, 9600));
						entryLoad.add(EmaFactory.createFieldEntry().enumValue(3428, 2));
						entryLoad.add(EmaFactory.createFieldEntry().rmtes(212, ByteBuffer.wrap("Market Maker".getBytes())));
            
						map.add(EmaFactory.createMapEntry().keyAscii("100", MapEntry.MapAction.ADD, entryLoad));

						provider.submit(EmaFactory.createRefreshMsg().domainType(EmaRdm.MMT_MARKET_BY_ORDER).serviceName("NI_PUB").name("AAO.V")
                            .state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
                            .payload(map).complete(true), aaoHandle);

						summary.clear();
            
						summary.add(EmaFactory.createFieldEntry().enumValue(15, 840));
						summary.add(EmaFactory.createFieldEntry().enumValue(53, 1));
						summary.add(EmaFactory.createFieldEntry().enumValue(3423, 1));
						summary.add(EmaFactory.createFieldEntry().enumValue(1709, 2));
            
						map.clear();
            
						map.summaryData(summary);

						entryLoad.clear();
            
						entryLoad.add(EmaFactory.createFieldEntry().realFromDouble(3427, 9.92, OmmReal.MagnitudeType.EXPONENT_NEG_2));
						entryLoad.add(EmaFactory.createFieldEntry().realFromDouble(3429, 1200));
						entryLoad.add(EmaFactory.createFieldEntry().enumValue(3428, 2));
						entryLoad.add(EmaFactory.createFieldEntry().rmtes(212, ByteBuffer.wrap("Market Maker".getBytes())));
            
						map.add(EmaFactory.createMapEntry().keyAscii("222", MapEntry.MapAction.ADD, entryLoad));
            
						provider.submit(EmaFactory.createRefreshMsg().domainType(EmaRdm.MMT_MARKET_BY_ORDER).serviceName("NI_PUB").name("AGG.V")
                            .state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
                            .payload(map).complete(true), aggHandle);

                        fieldList.clear();
			
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
						sendRefreshMsg = false;
					}
					else
					{
						fieldList.clear();
						fieldList.add(EmaFactory.createFieldEntry().real(22, 14400 + i, OmmReal.MagnitudeType.EXPONENT_NEG_2));
						fieldList.add(EmaFactory.createFieldEntry().real(30, 10 + i, OmmReal.MagnitudeType.EXPONENT_0));
						
						provider.submit( EmaFactory.createUpdateMsg().serviceName("NI_PUB").name("IBM.N").payload( fieldList ), ibmHandle );
						
						fieldList.clear();
						fieldList.add(EmaFactory.createFieldEntry().real(22, 4100 + i, OmmReal.MagnitudeType.EXPONENT_NEG_2));
						fieldList.add(EmaFactory.createFieldEntry().real(30, 21 + i, OmmReal.MagnitudeType.EXPONENT_0));
						
						provider.submit( EmaFactory.createUpdateMsg().serviceName("NI_PUB").name("TRI.N").payload( fieldList ), triHandle );
						//APIQA
						entryLoad.clear();
						
						entryLoad.add(EmaFactory.createFieldEntry().realFromDouble(3427, 7.76 + i * 0.1, OmmReal.MagnitudeType.EXPONENT_NEG_2));
						entryLoad.add(EmaFactory.createFieldEntry().realFromDouble(3429, 9600));
						entryLoad.add(EmaFactory.createFieldEntry().enumValue(3428, 2));
						entryLoad.add(EmaFactory.createFieldEntry().rmtes(212, ByteBuffer.wrap("Market Maker".getBytes())));
						
						map.clear();
						
						map.add(EmaFactory.createMapEntry().keyAscii("100", MapEntry.MapAction.UPDATE, entryLoad));

						provider.submit(EmaFactory.createUpdateMsg().domainType(EmaRdm.MMT_MARKET_BY_ORDER).serviceName("NI_PUB").name("AAO.V")
										.payload(map), aaoHandle);
						
						entryLoad.clear();
						
						entryLoad.add(EmaFactory.createFieldEntry().realFromDouble(3427, 9.92 + i * 0.1, OmmReal.MagnitudeType.EXPONENT_NEG_2));
						entryLoad.add(EmaFactory.createFieldEntry().realFromDouble(3429, 1200));
						entryLoad.add(EmaFactory.createFieldEntry().enumValue(3428, 2));
						entryLoad.add(EmaFactory.createFieldEntry().rmtes(212, ByteBuffer.wrap("Market Maker".getBytes())));
						
						map.clear();
						
						map.add(EmaFactory.createMapEntry().keyAscii("222", MapEntry.MapAction.UPDATE, entryLoad));
						
						provider.submit(EmaFactory.createUpdateMsg().domainType(EmaRdm.MMT_MARKET_BY_ORDER).serviceName("NI_PUB").name("AGG.V")
										.payload(map), aggHandle);  
						//END APIQA
					}
				}
				else
				{
					sendRefreshMsg = true;
				}
				provider.dispatch( 1000000 );
				//Thread.sleep(1000);
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
