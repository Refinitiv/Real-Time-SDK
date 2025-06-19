/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.examples.training.iprovider.series100.ex170_MP_ConnectedClientInfo;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import com.refinitiv.ema.access.ChannelInformation;
import com.refinitiv.ema.access.EmaFactory;
import com.refinitiv.ema.access.FieldList;
import com.refinitiv.ema.access.GenericMsg;
import com.refinitiv.ema.access.Msg;
import com.refinitiv.ema.access.OmmException;
import com.refinitiv.ema.access.OmmIProviderConfig;
import com.refinitiv.ema.access.OmmInvalidUsageException;
import com.refinitiv.ema.access.OmmProvider;
import com.refinitiv.ema.access.OmmProviderClient;
import com.refinitiv.ema.access.OmmProviderEvent;
import com.refinitiv.ema.access.OmmReal;
import com.refinitiv.ema.access.OmmState;
import com.refinitiv.ema.access.PostMsg;
import com.refinitiv.ema.access.RefreshMsg;
import com.refinitiv.ema.access.ReqMsg;
import com.refinitiv.ema.access.StatusMsg;
import com.refinitiv.ema.access.OmmIProviderConfig.OperationModel;
import com.refinitiv.ema.rdm.EmaRdm;

class AppClient implements OmmProviderClient
{
	public java.util.Map<Long, ArrayList<Long> >itemHandles = new HashMap<>();
	
	public void onReqMsg(ReqMsg reqMsg, OmmProviderEvent event)
	{
		System.out.println( "channel info for request message event\n\t" + event.channelInformation() );
		switch (reqMsg.domainType())
		{
			case EmaRdm.MMT_LOGIN :
				processLoginRequest(reqMsg, event);
				break;
			case EmaRdm.MMT_MARKET_PRICE :
				processMarketPriceRequest(reqMsg, event);
				break;
			default :
				processInvalidItemRequest(reqMsg, event);
				break;
		}
	}
	
	public void onRefreshMsg( RefreshMsg refreshMsg, OmmProviderEvent providerEvent ) {}
	public void onStatusMsg( StatusMsg statusMsg, OmmProviderEvent providerEvent ) {}
	public void onGenericMsg( GenericMsg genericMsg, OmmProviderEvent providerEvent ) {}
	public void onPostMsg( PostMsg postMsg, OmmProviderEvent providerEvent ) {}
	public void onReissue( ReqMsg reqMsg, OmmProviderEvent providerEvent ) {}

	public void onClose(ReqMsg reqMsg, OmmProviderEvent event){
		// sanity checking
		if (itemHandles.containsKey(event.clientHandle()) == false) {
			System.out.println("did not find client " + event.clientHandle() + " in itemHandles");
			return;
		}

		if (reqMsg.domainType() == EmaRdm.MMT_LOGIN) {	// removing client
			System.out.println("removing client " + event.clientHandle());
			itemHandles.remove(event.clientHandle());
		}
		else if (reqMsg.domainType() == EmaRdm.MMT_MARKET_PRICE) {	// removing item
			System.out.println("removing item " + event.handle() + " from client " + event.clientHandle());
			ArrayList<Long> tmp = itemHandles.get((Long)event.clientHandle());
			if (tmp == null) {
				System.out.println("client " + event.clientHandle() + " had no items");
				return;
			}
			tmp.remove((Long)event.handle());
			itemHandles.put((Long)event.clientHandle(), tmp);
		}
		System.out.println("channel info for close event:\n\t" + event.channelInformation());
	}

	public void onAllMsg(Msg msg, OmmProviderEvent event){}
	
	void processLoginRequest(ReqMsg reqMsg, OmmProviderEvent event)
	{
		event.provider().submit( EmaFactory.createRefreshMsg().domainType(EmaRdm.MMT_LOGIN).name(reqMsg.name()).
				nameType(EmaRdm.USER_NAME).complete(true).solicited(true).
				state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "Login accepted"),
				event.handle() );
		if (itemHandles.containsKey((Long)event.clientHandle()) == true)
			System.out.println("map already contains an element with handle" + event.clientHandle());
		else {
			itemHandles.put((Long)(event.clientHandle()), new ArrayList<Long>());
			System.out.println("added client " + event.clientHandle());
		}
		System.out.println("channel info for login event:\n\t" + event.channelInformation());
	}
	
	void processMarketPriceRequest(ReqMsg reqMsg, OmmProviderEvent event)
	{
		FieldList fieldList = EmaFactory.createFieldList();
		fieldList.add( EmaFactory.createFieldEntry().real(22, 3990, OmmReal.MagnitudeType.EXPONENT_NEG_2));
		fieldList.add( EmaFactory.createFieldEntry().real(25, 3994, OmmReal.MagnitudeType.EXPONENT_NEG_2));
		fieldList.add( EmaFactory.createFieldEntry().real(30, 9,  OmmReal.MagnitudeType.EXPONENT_0));
		fieldList.add( EmaFactory.createFieldEntry().real(31, 19, OmmReal.MagnitudeType.EXPONENT_0));

		event.provider().submit( EmaFactory.createRefreshMsg().name(reqMsg.name()).serviceId(reqMsg.serviceId()).solicited(true).
				state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "Refresh Completed").
				payload(fieldList).complete(true),
				event.handle() );

		ArrayList<Long> handles = itemHandles.get(event.clientHandle());
		if (handles == null) {
			System.out.println("did not find client in itemHandles for processMarketPriceRequest");
			return;
		}

		handles.add(event.handle());
		itemHandles.put(event.clientHandle(), handles);

		System.out.println("added item " + event.handle() + " to client " + event.clientHandle());
		System.out.println("channel info for market price request event:\n\t" + event.channelInformation());
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
		int clientCount = 0;
		
		try
		{
			AppClient appClient = new AppClient();
			FieldList fieldList = EmaFactory.createFieldList();

			OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig("EmaConfig.xml");
			
			provider = EmaFactory.createOmmProvider(config.operationModel(OperationModel.USER_DISPATCH).port("14002"), appClient);
			
			List<ChannelInformation> ci = new ArrayList<ChannelInformation>();
			
			while( appClient.itemHandles.isEmpty() == true ) provider.dispatch(10000);
				
			long startTime = System.currentTimeMillis();
			long nextPublishTime = startTime + 1000;
			int i = 0;
			while( startTime + 60000 > System.currentTimeMillis() )
			{
				fieldList.clear();
				++i;
				fieldList.add(EmaFactory.createFieldEntry().real(22, 3991 + i, OmmReal.MagnitudeType.EXPONENT_NEG_2));
				fieldList.add(EmaFactory.createFieldEntry().real(30, 10 + i, OmmReal.MagnitudeType.EXPONENT_0));


				for (ArrayList<Long> handles : appClient.itemHandles.values())
					for (Long handle : handles)
						try {
							provider.submit( EmaFactory.createUpdateMsg().payload( fieldList ), handle );
						}
				catch (OmmInvalidUsageException e)
				{
					System.out.println("attempted to send message to invalid handle " + handle);
				}
				while (true) {
					long dispatchTime = nextPublishTime - System.currentTimeMillis();
					if (dispatchTime > 0)
						provider.dispatch(dispatchTime*1000);
					else {
						nextPublishTime += 1000;
						break;
					}
				}
				
				if (appClient.itemHandles.size() != clientCount) {
					clientCount = appClient.itemHandles.size();
					provider.connectedClientChannelInfo(ci);
					System.out.println(ci.size() + " connected clients");
					for ( ChannelInformation K : ci)
						System.out.println("client: " + K);
				}
			}
			Thread.sleep(1000);

			if (appClient.itemHandles.size() > 0) {
				System.out.println(ci.size() + " remaining connected clients after main loop");
				provider.connectedClientChannelInfo(ci);
				for ( ChannelInformation K : ci) {
					System.out.println("client: " + K);
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
