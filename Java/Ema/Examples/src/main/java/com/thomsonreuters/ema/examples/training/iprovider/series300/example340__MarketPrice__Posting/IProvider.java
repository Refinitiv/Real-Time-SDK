///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.examples.training.iprovider.series300.example340__MarketPrice__Posting;

import com.thomsonreuters.ema.access.AckMsg;
import com.thomsonreuters.ema.access.ElementList;
import com.thomsonreuters.ema.access.EmaFactory;
import com.thomsonreuters.ema.access.FieldList;
import com.thomsonreuters.ema.access.GenericMsg;
import com.thomsonreuters.ema.access.Msg;
import com.thomsonreuters.ema.access.OmmException;
import com.thomsonreuters.ema.access.OmmIProviderConfig;
import com.thomsonreuters.ema.access.OmmProvider;
import com.thomsonreuters.ema.access.OmmProviderClient;
import com.thomsonreuters.ema.access.OmmProviderEvent;
import com.thomsonreuters.ema.access.OmmReal;
import com.thomsonreuters.ema.access.OmmState;
import com.thomsonreuters.ema.access.PostMsg;
import com.thomsonreuters.ema.access.RefreshMsg;
import com.thomsonreuters.ema.access.ReqMsg;
import com.thomsonreuters.ema.access.StatusMsg;
import com.thomsonreuters.ema.rdm.EmaRdm;

class AppClient implements OmmProviderClient
{
	AppClient() {}

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
	
	public void onRefreshMsg(RefreshMsg refreshMsg, OmmProviderEvent event) {}
	public void onStatusMsg(StatusMsg statusMsg, OmmProviderEvent event) {}
	public void onGenericMsg(GenericMsg genericMsg, OmmProviderEvent providerEvent) {}
	public void onPostMsg(PostMsg postMsg, OmmProviderEvent providerEvent) {
		System.out.println("Received PostMsg with id: " + postMsg.postId());
		if(postMsg.solicitAck()) {
			AckMsg ackMsg = EmaFactory.createAckMsg();
			if(postMsg.hasSeqNum()){
				ackMsg.seqNum(postMsg.seqNum());
			}
			if(postMsg.hasName()){
				ackMsg.name(postMsg.name());
			}
			if(postMsg.hasServiceId()){
				ackMsg.serviceId(postMsg.serviceId());
			}
			ackMsg.ackId(postMsg.postId())
					.domainType(postMsg.domainType());
			providerEvent.provider().submit(ackMsg, providerEvent.handle());
		}
	}
	public void onReissue(ReqMsg reqMsg, OmmProviderEvent providerEvent) {}
	public void onClose(ReqMsg reqMsg, OmmProviderEvent providerEvent) {}
	public void onAllMsg(Msg msg, OmmProviderEvent providerEvent) {}
	
	void processLoginRequest(ReqMsg reqMsg, OmmProviderEvent event)
	{
		ElementList elementList = EmaFactory.createElementList();
		elementList.add(EmaFactory.createElementEntry().uintValue("SupportOMMPost", 1));
		event.provider().submit(EmaFactory.createRefreshMsg().domainType(EmaRdm.MMT_LOGIN).name(reqMsg.name()).nameType(EmaRdm.USER_NAME).
				complete(true).solicited(true).state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "Login accepted").
				attrib(elementList), event.handle());
	}

	void processMarketPriceRequest(ReqMsg reqMsg, OmmProviderEvent event)
	{
		FieldList fieldList = EmaFactory.createFieldList();
		fieldList.add(EmaFactory.createFieldEntry().real(22, 3990, OmmReal.MagnitudeType.EXPONENT_NEG_2));
		fieldList.add(EmaFactory.createFieldEntry().real(25, 3994, OmmReal.MagnitudeType.EXPONENT_NEG_2));
		fieldList.add(EmaFactory.createFieldEntry().real(30, 9, OmmReal.MagnitudeType.EXPONENT_0));
		fieldList.add(EmaFactory.createFieldEntry().real(31, 19, OmmReal.MagnitudeType.EXPONENT_0));
		event.provider().submit(
			EmaFactory.createRefreshMsg().serviceName(reqMsg.serviceName()).name(reqMsg.name())
			.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "Refresh Completed")
			.solicited(true)
			.payload(fieldList).complete(true), event.handle());
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
	public static void main(String[] args)
	{
		OmmProvider provider = null;
		try
		{
			AppClient appClient = new AppClient();

			OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig();

			provider = EmaFactory.createOmmProvider(config.port("14002"), appClient);

			Thread.sleep(120000);
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
