///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2025 LSEG. All rights reserved.
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.examples.training.iprovider.series100.ex100_MP_Streaming;

import com.refinitiv.ema.access.*;
import com.refinitiv.ema.rdm.EmaRdm;

class AppClient implements OmmProviderClient
{
	public long itemHandle = 0;
	
	public void onReqMsg(ReqMsg reqMsg, OmmProviderEvent event)
	{
		switch (reqMsg.domainType())
		{
			case EmaRdm.MMT_LOGIN :
				processLoginRequest(reqMsg, event);
				break;
			//API QA
			case EmaRdm.MMT_DIRECTORY:
				processDirectoryRequest(reqMsg, event);
				break;
			// END API QA
			case EmaRdm.MMT_MARKET_PRICE :
				processMarketPriceRequest(reqMsg, event);
				break;
			default :
				processInvalidItemRequest(reqMsg, event);
				break;
		}
	}
	
	public void onRefreshMsg(RefreshMsg refreshMsg,	OmmProviderEvent event){}
	public void onStatusMsg(StatusMsg statusMsg, OmmProviderEvent event){}
	public void onGenericMsg(GenericMsg genericMsg, OmmProviderEvent event){}
	public void onPostMsg(PostMsg postMsg, OmmProviderEvent event){}
	public void onReissue(ReqMsg reqMsg, OmmProviderEvent event){}
	public void onClose(ReqMsg reqMsg, OmmProviderEvent event){}
	public void onAllMsg(Msg msg, OmmProviderEvent event){}
	
	void processLoginRequest(ReqMsg reqMsg, OmmProviderEvent event)
	{
		event.provider().submit( EmaFactory.createRefreshMsg().domainType(EmaRdm.MMT_LOGIN).name(reqMsg.name()).
						nameType(EmaRdm.USER_NAME).complete(true).solicited(true).
						state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "Login accepted"),
				event.handle() );
	}

	//API QA
	void processDirectoryRequest(ReqMsg reqMsg, OmmProviderEvent event)
	{
		OmmArray capablities = EmaFactory.createOmmArray();
		capablities.add(EmaFactory.createOmmArrayEntry().uintValue( EmaRdm.MMT_MARKET_PRICE));
		capablities.add(EmaFactory.createOmmArrayEntry().uintValue( EmaRdm.MMT_MARKET_BY_PRICE));
		OmmArray dictionaryUsed = EmaFactory.createOmmArray();
		dictionaryUsed.add(EmaFactory.createOmmArrayEntry().ascii( "RWFFld"));
		dictionaryUsed.add(EmaFactory.createOmmArrayEntry().ascii( "RWFEnum"));

		ElementList serviceInfoId = EmaFactory.createElementList();

		serviceInfoId.add( EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_NAME, IProvider._serviceName));
		serviceInfoId.add( EmaFactory.createElementEntry().array(EmaRdm.ENAME_CAPABILITIES, capablities));
		serviceInfoId.add( EmaFactory.createElementEntry().array(EmaRdm.ENAME_DICTIONARYS_USED, dictionaryUsed));

		ElementList serviceStateId = EmaFactory.createElementList();
		serviceStateId.add( EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_UP));

		FilterList filterList = EmaFactory.createFilterList();
		filterList.add( EmaFactory.createFilterEntry().elementList(EmaRdm.SERVICE_INFO_ID, FilterEntry.FilterAction.SET, serviceInfoId) );
		filterList.add( EmaFactory.createFilterEntry().elementList(EmaRdm.SERVICE_STATE_ID, FilterEntry.FilterAction.SET, serviceStateId));

		Map map = EmaFactory.createMap();
		map.add( EmaFactory.createMapEntry().keyUInt(2, MapEntry.MapAction.ADD, filterList));

		RefreshMsg refreshMsg = EmaFactory.createRefreshMsg();
		event.provider().submit( refreshMsg.domainType(EmaRdm.MMT_DIRECTORY).clearCache(true).
				filter( EmaRdm.SERVICE_INFO_FILTER | EmaRdm.SERVICE_STATE_FILTER).
				payload(map).solicited(true).complete(true), event.handle());
	}
	// END API QA

	void processMarketPriceRequest(ReqMsg reqMsg, OmmProviderEvent event)
	{
		if( itemHandle != 0 )
		{
			processInvalidItemRequest(reqMsg, event);
			return;
		}

		FieldList fieldList = EmaFactory.createFieldList();
		fieldList.add( EmaFactory.createFieldEntry().real(22, 3990, OmmReal.MagnitudeType.EXPONENT_NEG_2));
		fieldList.add( EmaFactory.createFieldEntry().real(25, 3994, OmmReal.MagnitudeType.EXPONENT_NEG_2));
		fieldList.add( EmaFactory.createFieldEntry().real(30, 9,  OmmReal.MagnitudeType.EXPONENT_0));
		fieldList.add( EmaFactory.createFieldEntry().real(31, 19, OmmReal.MagnitudeType.EXPONENT_0));
		
		event.provider().submit( EmaFactory.createRefreshMsg().name(reqMsg.name()).serviceId(reqMsg.serviceId()).solicited(true).
				state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "Refresh Completed").
				payload(fieldList).complete(true),
				event.handle() );

		itemHandle = event.handle();
	}
	
	void processInvalidItemRequest(ReqMsg reqMsg, OmmProviderEvent event)
	{
		event.provider().submit( EmaFactory.createStatusMsg().name(reqMsg.name()).serviceName(reqMsg.serviceName()).
				state(OmmState.StreamState.CLOSED, OmmState.DataState.SUSPECT,	OmmState.StatusCode.NOT_FOUND, "Item not found"),
				event.handle() );
	}
}

public class IProvider
{
	//API QA
		public static int _runTime;
		public static String _portNumber;
		public static String _serviceName;
		static void printHelp()
		{
			System.out.println("\nOptions:\n" + "  -?\tShows this usage\n" 
					+ "  -p port number (defaults to 14002);\n"
					+ "  -s service name (defaults to DIRECT_FEED);\n"
					+ "  -runtime application runtime in seconds (default is 60).\n"
				    + "\n");
		}
		static boolean readCommandlineArgs(String[] args)
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
				else if ("-p".equals(args[argsCount]))
				{
					IProvider._portNumber = argsCount < (args.length-1) ? args[++argsCount] : "14002";
					System.out.println("Port Number: " + IProvider._portNumber);
					++argsCount;
				}
				else if ("-s".equals(args[argsCount]))
				{
					IProvider._serviceName = argsCount < (args.length-1) ? args[++argsCount] : "DIRECT_FEED";
					System.out.println("Service Name: " + IProvider._serviceName);
					++argsCount;
				}
				else if ("-runtime".equals(args[argsCount]))
				{
					IProvider._runTime = argsCount < (args.length-1) ? Integer.parseInt(args[++argsCount]) : 60;
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
		// END API QA
	public static void main(String[] args)
	{
		OmmProvider provider = null;
		try
		{
			//API QA
			if ( !readCommandlineArgs(args) ) return;
			//END API QA
			
			AppClient appClient = new AppClient();
			FieldList fieldList = EmaFactory.createFieldList();

			OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig();
			
			provider = EmaFactory.createOmmProvider(config
													.port(IProvider._portNumber)
													.adminControlDirectory(OmmIProviderConfig.AdminControl.USER_CONTROL), appClient);
			
			while( appClient.itemHandle == 0 ) Thread.sleep(1000);
				
			for( int i = 0; i < IProvider._runTime; i++ )
			{
				fieldList.clear();
				fieldList.add(EmaFactory.createFieldEntry().real(22, 3991 + i, OmmReal.MagnitudeType.EXPONENT_NEG_2));
				fieldList.add(EmaFactory.createFieldEntry().real(30, 10 + i, OmmReal.MagnitudeType.EXPONENT_0));
				
				provider.submit( EmaFactory.createUpdateMsg().payload( fieldList ), appClient.itemHandle );
				
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
