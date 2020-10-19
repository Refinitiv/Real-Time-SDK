///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.examples.training.iprovider.series100.ex100_MP_Streaming;

import java.nio.ByteBuffer;

import com.refinitiv.ema.access.EmaFactory;
import com.refinitiv.ema.access.FieldList;
import com.refinitiv.ema.access.GenericMsg;
import com.refinitiv.ema.access.Msg;
import com.refinitiv.ema.access.OmmException;
import com.refinitiv.ema.access.OmmIProviderConfig;
import com.refinitiv.ema.access.OmmProvider;
import com.refinitiv.ema.access.OmmProviderClient;
import com.refinitiv.ema.access.OmmProviderEvent;
import com.refinitiv.ema.access.OmmReal;
import com.refinitiv.ema.access.OmmState;
import com.refinitiv.ema.access.PostMsg;
import com.refinitiv.ema.access.RefreshMsg;
import com.refinitiv.ema.access.ReqMsg;
import com.refinitiv.ema.access.StatusMsg;
import com.refinitiv.ema.rdm.EmaRdm;
//APIQA
import com.refinitiv.ema.access.Map;
import com.refinitiv.ema.access.MapEntry;


class AppClient implements OmmProviderClient
{
	public long itemHandle = 0;
	//APIQA
	public static int NUMOFITEMSINSYMBOLLIST = 100; // Number of items in symbolList
    
	public static void printHelp(boolean reflect)
    {
            if (!reflect)
            {
                    System.out.println("\nOptions:\n" + "  -?\tShows this usage\n\n" + "  -c  \tSend the number of items in symbolList [default = 100]\n"
                                    + "\n");

                    System.exit(-1);
            }
            else
            {
                    System.out.println("\n  Options will be used:\n" + "  -c \t " + NUMOFITEMSINSYMBOLLIST + "\n" + "\n");
            }
    }
    public boolean readCommandlineArgs(String[] argv)
    {
            int count = argv.length;
            int idx = 0;

            while (idx < count)
            {
                    if (0 == argv[idx].compareTo("-?"))
                    {
                            printHelp(false);
                            return false;
                    }
                    else if (0 == argv[idx].compareToIgnoreCase("-c"))
                    {
                            if (++idx >= count)
                            {
                                    printHelp(false);
                                    return false;
                            }
                            NUMOFITEMSINSYMBOLLIST = Integer.parseInt(argv[idx]);
                            ++idx;
                    }
                    else
                    {
                            printHelp(false);
                            return false;
                    }
            }
            printHelp(true);
            return true;
    }
    //ENA APIQA
	public void onReqMsg(ReqMsg reqMsg, OmmProviderEvent event)
	{
		switch (reqMsg.domainType())
		{
			case EmaRdm.MMT_LOGIN :
				processLoginRequest(reqMsg, event);
				break;
			//APIQA
			case EmaRdm.MMT_SYMBOL_LIST :
				processSymbolListRequest(reqMsg, event);
				break;
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
	//APIQA
	void processSymbolListRequest(ReqMsg reqMsg, OmmProviderEvent event)
	{
		if( itemHandle != 0 )
		{
			processInvalidItemRequest(reqMsg, event);
			return;
		}
		Map mapEnc = EmaFactory.createMap();
		FieldList fieldList1 = EmaFactory.createFieldList();
		String a;
		for (int i = 0; i < NUMOFITEMSINSYMBOLLIST ; ++i) {
		    
		    
		    a = "A"+i;
		    
		    mapEnc.add(EmaFactory.createMapEntry().keyAscii(a, MapEntry.MapAction.ADD, fieldList1));
		}

		event.provider().submit( EmaFactory.createRefreshMsg().name(reqMsg.name()).serviceId(reqMsg.serviceId()).solicited(true).
				state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "Refresh Completed").
				payload(mapEnc).complete(true),
				event.handle() );

		itemHandle = event.handle();
	}
	// END APIQA
	
	void processInvalidItemRequest(ReqMsg reqMsg, OmmProviderEvent event)
	{
		event.provider().submit( EmaFactory.createStatusMsg().name(reqMsg.name()).serviceName(reqMsg.serviceName()).
				state(OmmState.StreamState.CLOSED, OmmState.DataState.SUSPECT,	OmmState.StatusCode.NOT_FOUND, "Item not found"),
				event.handle() );
	}
}

public class IProvider
{
	
	public static void main(String[] args)
	{
		
		OmmProvider provider = null;
		try
		{
			AppClient appClient = new AppClient();
			//APIQA
			//FieldList fieldList = EmaFactory.createFieldList();
			if (!appClient.readCommandlineArgs(args))
                return;
			OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig();
			
			provider = EmaFactory.createOmmProvider(config.port("14002"), appClient);
			
			while( appClient.itemHandle == 0 ) Thread.sleep(1000);
				
			for( int i = 0; i < 60; i++ )
			{
				Thread.sleep(1000);
			}
			// END APIQA
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
