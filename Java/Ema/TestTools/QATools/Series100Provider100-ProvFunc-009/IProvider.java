///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.rtsdk.ema.examples.training.iprovider.series100.ex100_MP_Streaming;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.List;

import com.rtsdk.ema.access.ChannelInformation;
import com.rtsdk.ema.access.EmaFactory;
import com.rtsdk.ema.access.FieldList;
import com.rtsdk.ema.access.GenericMsg;
import com.rtsdk.ema.access.Msg;
import com.rtsdk.ema.access.OmmException;
import com.rtsdk.ema.access.OmmIProviderConfig;
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
//API QA
import com.rtsdk.ema.access.OmmProviderErrorClient;
//End API QA

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
	
	void processInvalidItemRequest(ReqMsg reqMsg, OmmProviderEvent event)
	{
		event.provider().submit( EmaFactory.createStatusMsg().name(reqMsg.name()).serviceName(reqMsg.serviceName()).
				state(OmmState.StreamState.CLOSED, OmmState.DataState.SUSPECT,	OmmState.StatusCode.NOT_FOUND, "Item not found"),
				event.handle() );
	}
}

//APIQA
class AppErrorClient implements OmmProviderErrorClient
{
	public void onInvalidHandle(long handle, String text)
	{
		System.out.println("onInvalidHandle callback function" + "\nInvalid handle: " + handle + "\nError text: " + text); 
	}

	public void onInvalidUsage(String text, int errorCode) {
		System.out.println("onInvalidUsage callback function" + "\nError text: " + text +" , Error code: " + errorCode); 
	}
}
//END API QA

public class IProvider
{
	public static void main(String[] args)
	{
		OmmProvider provider = null;
		try
		{
			AppClient appClient = new AppClient();
			FieldList fieldList = EmaFactory.createFieldList();

			OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig();
			
			provider = EmaFactory.createOmmProvider(config.port("14002"), appClient);	
			int modifyGuaranteedOutputBuff = 10000;
				
			while( appClient.itemHandle == 0 ) Thread.sleep(1000);
			
			for( int i = 0; i < 60; i++ )
			{
				fieldList.clear();
				fieldList.add(EmaFactory.createFieldEntry().real(22, 3991 + i, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			    fieldList.add(EmaFactory.createFieldEntry().real(30, 10 + i, OmmReal.MagnitudeType.EXPONENT_0));
				fieldList.add(EmaFactory.createFieldEntry().rmtes(315, ByteBuffer.wrap("A1234567890123456789012345678901234567890123546789012345678901234567890".getBytes() )));
				fieldList.add(EmaFactory.createFieldEntry().rmtes(316, ByteBuffer.wrap("A1234567890123456789012345678901234567890123546789012345678901234567890".getBytes() )));
				fieldList.add(EmaFactory.createFieldEntry().rmtes(317, ByteBuffer.wrap("A1234567890123456789012345678901234567890123546789012345678901234567890".getBytes() )));
				fieldList.add(EmaFactory.createFieldEntry().rmtes(318, ByteBuffer.wrap("A1234567890123456789012345678901234567890123546789012345678901234567890".getBytes() )));
				fieldList.add(EmaFactory.createFieldEntry().rmtes(319, ByteBuffer.wrap("A1234567890123456789012345678901234567890123546789012345678901234567890".getBytes() )));
				fieldList.add(EmaFactory.createFieldEntry().rmtes(320, ByteBuffer.wrap("A1234567890123456789012345678901234567890123546789012345678901234567890".getBytes() )));
				fieldList.add(EmaFactory.createFieldEntry().rmtes(321, ByteBuffer.wrap("A1234567890123456789012345678901234567890123546789012345678901234567890".getBytes() )));
				fieldList.add(EmaFactory.createFieldEntry().rmtes(322, ByteBuffer.wrap("A1234567890123456789012345678901234567890123546789012345678901234567890".getBytes() )));
				fieldList.add(EmaFactory.createFieldEntry().rmtes(323, ByteBuffer.wrap("A1234567890123456789012345678901234567890123546789012345678901234567890".getBytes() )));
				fieldList.add(EmaFactory.createFieldEntry().rmtes(324, ByteBuffer.wrap("A1234567890123456789012345678901234567890123546789012345678901234567890".getBytes() )));
				fieldList.add(EmaFactory.createFieldEntry().rmtes(325, ByteBuffer.wrap("A1234567890123456789012345678901234567890123546789012345678901234567890".getBytes() )));
				fieldList.add(EmaFactory.createFieldEntry().rmtes(326, ByteBuffer.wrap("A1234567890123456789012345678901234567890123546789012345678901234567890".getBytes() )));
				fieldList.add(EmaFactory.createFieldEntry().rmtes(327, ByteBuffer.wrap("A1234567890123456789012345678901234567890123546789012345678901234567890".getBytes() )));
				fieldList.add(EmaFactory.createFieldEntry().rmtes(328, ByteBuffer.wrap("A1234567890123456789012345678901234567890123546789012345678901234567890".getBytes() )));
				fieldList.add(EmaFactory.createFieldEntry().rmtes(329, ByteBuffer.wrap("A1234567890123456789012345678901234567890123546789012345678901234567890".getBytes() )));
				fieldList.add(EmaFactory.createFieldEntry().rmtes(330, ByteBuffer.wrap("A1234567890123456789012345678901234567890123546789012345678901234567890".getBytes() )));
				fieldList.add(EmaFactory.createFieldEntry().rmtes(331, ByteBuffer.wrap("A1234567890123456789012345678901234567890123546789012345678901234567890".getBytes() )));
				fieldList.add(EmaFactory.createFieldEntry().rmtes(332, ByteBuffer.wrap("A1234567890123456789012345678901234567890123546789012345678901234567890".getBytes() )));
				fieldList.add(EmaFactory.createFieldEntry().rmtes(333, ByteBuffer.wrap("A1234567890123456789012345678901234567890123546789012345678901234567890".getBytes() )));
				fieldList.add(EmaFactory.createFieldEntry().rmtes(334, ByteBuffer.wrap("A1234567890123456789012345678901234567890123546789012345678901234567890".getBytes() )));
				fieldList.add(EmaFactory.createFieldEntry().rmtes(335, ByteBuffer.wrap("A1234567890123456789012345678901234567890123546789012345678901234567890".getBytes() )));
				fieldList.add(EmaFactory.createFieldEntry().rmtes(336, ByteBuffer.wrap("A1234567890123456789012345678901234567890123546789012345678901234567890".getBytes() )));
				fieldList.add(EmaFactory.createFieldEntry().rmtes(337, ByteBuffer.wrap("A1234567890123456789012345678901234567890123546789012345678901234567890".getBytes() )));
				fieldList.add(EmaFactory.createFieldEntry().rmtes(338, ByteBuffer.wrap("A1234567890123456789012345678901234567890123546789012345678901234567890".getBytes() )));
				try {
					for (int j=0; j < 100; j++)
						provider.submit( EmaFactory.createUpdateMsg().payload( fieldList ), appClient.itemHandle );
						
				}catch (OmmException exp) {
					System.out.println("Exception when submit for loop : " + i);
					System.out.println(exp.getMessage());
					System.out.println("Modify guaranteedBuffers to increase guaranteedOutputBuffers to : " + modifyGuaranteedOutputBuff);
					provider.modifyIOCtl(2, modifyGuaranteedOutputBuff, appClient.itemHandle);
					System.out.println("Try to resubmit message for handle: " + appClient.itemHandle);
					provider.submit( EmaFactory.createUpdateMsg().payload( fieldList ), appClient.itemHandle );
					System.out.println("Resubmitted message for handle: " + appClient.itemHandle);
				}
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
