///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.rtsdk.ema.examples.training.iprovider.series300.example380__MarketPrice__ConnectionLost;

import com.rtsdk.ema.access.EmaFactory;
import com.rtsdk.ema.access.FieldList;
import com.rtsdk.ema.access.GenericMsg;
import com.rtsdk.ema.access.Msg;
import com.rtsdk.ema.access.OmmException;
import com.rtsdk.ema.access.OmmProvider;
import com.rtsdk.ema.access.OmmProviderClient;
import com.rtsdk.ema.access.OmmProviderEvent;
import com.rtsdk.ema.access.OmmReal;
import com.rtsdk.ema.access.OmmState;
import com.rtsdk.ema.access.PostMsg;
import com.rtsdk.ema.access.RefreshMsg;
import com.rtsdk.ema.access.ReqMsg;
import com.rtsdk.ema.access.StatusMsg;
import com.rtsdk.ema.rdm.EmaRdm;

class AppClient implements OmmProviderClient
{
	public long itemHandle = 0;

	public void onReqMsg(ReqMsg reqMsg, OmmProviderEvent event)
	{
		switch (reqMsg.domainType())
		{
		case EmaRdm.MMT_LOGIN:
			processLoginRequest(reqMsg, event);
			break;
		case EmaRdm.MMT_MARKET_PRICE:
			processMarketPriceRequest(reqMsg, event);
			break;
		default:
			processInvalidItemRequest(reqMsg, event);
			break;
		}
	}

	public void onClose(ReqMsg reqMsg, OmmProviderEvent event)
	{
		switch (reqMsg.domainType())
		{
		case EmaRdm.MMT_LOGIN:
			processLoginClose(reqMsg, event);
			break;
		default:
			break;
		}
	}

	public void onRefreshMsg(RefreshMsg refreshMsg,	OmmProviderEvent event){}
	public void onStatusMsg(StatusMsg statusMsg, OmmProviderEvent event){}
	public void onGenericMsg(GenericMsg genericMsg, OmmProviderEvent event){}
	public void onPostMsg(PostMsg postMsg, OmmProviderEvent event){}
	public void onReissue(ReqMsg reqMsg, OmmProviderEvent event){}
	public void onAllMsg(Msg msg, OmmProviderEvent event){}
	
	void processLoginRequest(ReqMsg reqMsg, OmmProviderEvent event)
	{
		event.provider().submit( EmaFactory.createRefreshMsg().domainType(EmaRdm.MMT_LOGIN).name(reqMsg.name()).
				nameType(EmaRdm.USER_NAME).complete(true).solicited(true).
				state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "Login accepted"),
				event.handle() );
	}
	
	void processLoginClose(ReqMsg reqMsg, OmmProviderEvent event)
	{
		itemHandle = 0;
		System.out.println("Received Login close request. Login Handle: " + event.handle() + " Closure: " + event.closure());
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
				payload(fieldList).complete(true), event.handle() );

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
	public static void main(String[] args)
	{
		OmmProvider provider = null;
		try
		{
			AppClient appClient = new AppClient();

			provider = EmaFactory.createOmmProvider(EmaFactory.createOmmIProviderConfig(), appClient);
			
			while (appClient.itemHandle == 0) Thread.sleep(1000);

			FieldList fieldList = EmaFactory.createFieldList();
			int i = 0;
			while(true)
			{
				while (appClient.itemHandle != 0)
				{
					fieldList.clear();
					fieldList.add(EmaFactory.createFieldEntry().real(22, 3991 + i, OmmReal.MagnitudeType.EXPONENT_NEG_2));
					fieldList.add(EmaFactory.createFieldEntry().real(30, 10 + i++, OmmReal.MagnitudeType.EXPONENT_0));
	
					provider.submit(EmaFactory.createUpdateMsg().payload(fieldList), appClient.itemHandle);
	
					Thread.sleep(1000);
				}
				
				Thread.sleep(2000);
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
