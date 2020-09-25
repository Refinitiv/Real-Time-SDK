///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.examples.training.iprovider.series200.ex280_MP_Perf;

import java.nio.ByteBuffer;
import java.util.ArrayList;

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
import com.refinitiv.ema.access.UpdateMsg;
import com.refinitiv.ema.rdm.EmaRdm;

class ReqInfo
{
	private long _handle;
	
	public void value(long handle)
	{
		_handle = handle;
	}
	
	public long handle()
	{
		return _handle;
	}
}

class AppClient implements OmmProviderClient
{
	public ArrayList<ReqInfo> reqInfoList = new ArrayList<>(1000);
	public int numberOfRequestItems = 0;;
	public FieldList fieldList = EmaFactory.createFieldList();
	public String statusText = new String("Refresh Completed");
	
	private RefreshMsg refreshMsg = EmaFactory.createRefreshMsg();
	private long startRefreshTime;
	private long endRefreshTime;
	
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
		event.provider().submit(refreshMsg.clear().domainType(EmaRdm.MMT_LOGIN).name(reqMsg.name()).nameType(EmaRdm.USER_NAME).
				complete(true).solicited(true).state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "Login accepted"),
				event.handle());
	}
	
	void processMarketPriceRequest(ReqMsg reqMsg, OmmProviderEvent event)
	{
		if ( numberOfRequestItems == 1000 )
		{
			event.provider().submit(EmaFactory.createStatusMsg().name(reqMsg.name()).serviceName(reqMsg.serviceName()).
					domainType(reqMsg.domainType()).
					state(OmmState.StreamState.CLOSED, OmmState.DataState.SUSPECT, OmmState.StatusCode.TOO_MANY_ITEMS, "Request more than 1000 items"),
					event.handle());
			return;
		}
		
		fieldList.clear();
		fieldList.add(EmaFactory.createFieldEntry().intValue(1, 6560));
		fieldList.add(EmaFactory.createFieldEntry().intValue(2, 66));
		fieldList.add(EmaFactory.createFieldEntry().intValue(3855, 52832001));
		fieldList.add(EmaFactory.createFieldEntry().rmtes(296, ByteBuffer.wrap("BOS".getBytes())));
		fieldList.add(EmaFactory.createFieldEntry().time(375, 21, 0));
		fieldList.add(EmaFactory.createFieldEntry().time(1025, 14, 40, 32));
		fieldList.add(EmaFactory.createFieldEntry().real(22, 14400, OmmReal.MagnitudeType.EXPONENT_NEG_2));
		fieldList.add(EmaFactory.createFieldEntry().real(25, 14700, OmmReal.MagnitudeType.EXPONENT_NEG_2));
		fieldList.add(EmaFactory.createFieldEntry().real(30, 9, OmmReal.MagnitudeType.EXPONENT_0));
		fieldList.add(EmaFactory.createFieldEntry().real(31, 19, OmmReal.MagnitudeType.EXPONENT_0));
		
		event.provider().submit(EmaFactory.createRefreshMsg().serviceId(reqMsg.serviceId()).name(reqMsg.name()).
				state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, statusText).solicited(true).
				payload(fieldList).complete(true), event.handle());

		
		if ( ++numberOfRequestItems == 1 )
		{
			startRefreshTime = System.currentTimeMillis();
		}
		else if ( numberOfRequestItems == 1000 )
		{
			endRefreshTime = System.currentTimeMillis();
		
			float timeSpent = (float)(endRefreshTime - startRefreshTime) / (float)1000;
		
			System.out.println("total refresh count = " + numberOfRequestItems +
					"\ttotal time = " + timeSpent + " sec" +
					"\tupdate rate = " + numberOfRequestItems / timeSpent + " refresh per sec");
		}
		
		ReqInfo reqInfo = new ReqInfo();
		reqInfo.value(event.handle());
		reqInfoList.add(reqInfo);
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
		
			provider = EmaFactory.createOmmProvider(EmaFactory.createOmmIProviderConfig().
					operationModel(OmmIProviderConfig.OperationModel.USER_DISPATCH), appClient );

			while(appClient.numberOfRequestItems < 1000)  provider.dispatch(1000);
			
			int updateMsgCount = 0;
			UpdateMsg updateMsg = EmaFactory.createUpdateMsg();
			long startUpdateTime;
			ReqInfo reqInfo;
			boolean submitting = true;
			int numberOfSeconds = 0;
			int index = 0;
			int submittingTime = 300;
			
			startUpdateTime = System.currentTimeMillis();
			
			while(submitting)
			{			
				for(index = 0; index < appClient.numberOfRequestItems; index++ )
				{	
					reqInfo = appClient.reqInfoList.get(index);
				
					appClient.fieldList.clear();
					appClient.fieldList.add(EmaFactory.createFieldEntry().time(1025, 14, 40, 32));
					appClient.fieldList.add(EmaFactory.createFieldEntry().intValue(3855, 52832001));
					appClient.fieldList.add(EmaFactory.createFieldEntry().real(22, 14400 + (((reqInfo.handle() & 0x1) == 1) ? 1 : 10), OmmReal.MagnitudeType.EXPONENT_NEG_2));
					appClient.fieldList.add(EmaFactory.createFieldEntry().real(30, 10 + (((reqInfo.handle() & 0x1) == 1) ? 10 : 20), OmmReal.MagnitudeType.EXPONENT_0));
					appClient.fieldList.add(EmaFactory.createFieldEntry().rmtes(296, ByteBuffer.wrap("NAS".getBytes())));
				
					provider.submit(updateMsg.clear().payload(appClient.fieldList ), reqInfo.handle());
					
					if (index % 10 == 0)
						provider.dispatch(10);
				
					updateMsgCount++;
				}
				
				float timeSpent = (float)(System.currentTimeMillis() - startUpdateTime) / (float)1000;
				
				if (timeSpent > 1)
				{
					System.out.println("update count = " + updateMsgCount +
							"\tupdate rate = " + (int)(updateMsgCount / timeSpent) + " update per sec");
				
					updateMsgCount = 0;

					startUpdateTime = System.currentTimeMillis();

					if (++numberOfSeconds == submittingTime) submitting = false;
				}
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
