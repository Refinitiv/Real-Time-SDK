///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.rtsdk.ema.examples.training.iprovider.series100.example100__MarketPrice__Streaming;

import com.rtsdk.ema.access.*;
import com.rtsdk.ema.rdm.EmaRdm;

import java.nio.ByteBuffer;

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
		RefreshMsg refreshMsg = EmaFactory.createRefreshMsg().domainType(EmaRdm.MMT_LOGIN)
				.domainType(EmaRdm.MMT_LOGIN).name(reqMsg.name())
				.nameType(EmaRdm.USER_NAME)
				.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "Login accepted")
				.complete(true).solicited(true);
		event.provider().submit(refreshMsg, event.handle());
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

		FieldList fieldListAtrtibs = EmaFactory.createFieldList();
		fieldListAtrtibs.add( EmaFactory.createFieldEntry().rmtes(548, ByteBuffer.wrap(new byte[] {'T', -'E', 'S', -'T'})));
		fieldListAtrtibs.add( EmaFactory.createFieldEntry().ascii(254, "SN#12345"));

		RefreshMsg refreshMsg =  EmaFactory.createRefreshMsg()
				.domainType(EmaRdm.MMT_MARKET_PRICE)
				.name(reqMsg.name()).serviceId(reqMsg.serviceId()).solicited(true)
				.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "Refresh Completed")
				.nameType(50)
				.id(60)
				.filter(70)
				.seqNum(80)
				.partNum(90)
				.publisherId(100, 110)
				.clearCache(true)
				.privateStream(true)
				.doNotCache(true)
				.attrib(fieldListAtrtibs)
				.qos(OmmQos.Timeliness.INEXACT_DELAYED, OmmQos.Rate.JUST_IN_TIME_CONFLATED)
				.extendedHeader(ByteBuffer.wrap(new byte[] {0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x45, 0x4e, 0x44}))
				.itemGroup(ByteBuffer.wrap(new byte[] {0x58, 0x59, 60, 77, 77,77, 77,77}))
				.permissionData(ByteBuffer.wrap(new byte[] {0x03, 0x01, 0x2c, 0x56, 0x25, -0x40}))
				.payload(fieldList).complete(true);

		event.provider().submit(refreshMsg, event.handle());

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
			FieldList fieldList = EmaFactory.createFieldList();

			OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig();
			
			provider = EmaFactory.createOmmProvider(config.port("14002"), appClient);
			
			while( appClient.itemHandle == 0 ) Thread.sleep(1000);
				
			//sends 3 messages, 1st - updateMsg, 2nd - statusMsg, 3rd - genericMsg
			for( int i = 1; i <= 4; i++ )
			{
				fieldList.clear();
				fieldList.add(EmaFactory.createFieldEntry().real(22, 3991 + i, OmmReal.MagnitudeType.EXPONENT_NEG_2));
				fieldList.add(EmaFactory.createFieldEntry().real(30, 10 + i, OmmReal.MagnitudeType.EXPONENT_0));


				FieldList fieldListAtrtibs = EmaFactory.createFieldList();
				fieldListAtrtibs.add( EmaFactory.createFieldEntry().rmtes(548, ByteBuffer.wrap(new byte[] {'T', -'E', 'S', -'T', 'U', -'P', 'D'})));
				fieldListAtrtibs.add( EmaFactory.createFieldEntry().ascii(254, "NO#54321"));

				UpdateMsg updateMsgShort = EmaFactory.createUpdateMsg()
//						.attrib(fieldListAtrtibs)
						.payload( fieldList );

				UpdateMsg updateMsg = EmaFactory.createUpdateMsg()
						.domainType(EmaRdm.MMT_MARKET_PRICE)
						.nameType(50 + i)
						.id(60 + i)
						.filter(70 + i)
						.extendedHeader(ByteBuffer.wrap(new byte[] {(byte) (0x61 + i), 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x45, 0x4e, 0x44}))
						.permissionData(ByteBuffer.wrap(new byte[] {(byte) (0x03 + i), 0x01, 0x2c, 0x56, 0x25, -0x40}))
						.attrib(fieldListAtrtibs)
						.payload( fieldList );


				StatusMsg statusMsg = EmaFactory.createStatusMsg()
						.domainType(EmaRdm.MMT_LOGIN).name("dummyName")
						.nameType(EmaRdm.USER_NAME)
						.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "Login accepted")
				.id(20)
				.filter(10)
				.publisherId(110, 100)
				.clearCache(true)
				.privateStream(true)
				.attrib(fieldListAtrtibs)
				.extendedHeader(ByteBuffer.wrap(new byte[] {10, 20, 30}))
				.itemGroup(ByteBuffer.wrap(new byte[] {30, 40}))
				.permissionData(ByteBuffer.wrap(new byte[] {50, 51, 52, 53}))
				.serviceId(10)//?
				.payload(fieldList);

				GenericMsg genericMsg = EmaFactory.createGenericMsg().domainType(EmaRdm.MMT_MARKET_PRICE)
						.name("genericDummyName")
						.nameType(EmaRdm.INSTRUMENT_NAME_RIC)

						.id(20)
						.filter(10)
						.attrib(fieldListAtrtibs)
						.extendedHeader(ByteBuffer.wrap(new byte[] {10, 20, 30}))
						.permissionData(ByteBuffer.wrap(new byte[] {50, 51, 52, 53}))
						.seqNum(33)
						.partNum(44)
						.complete(true)
						.secondarySeqNum(77)
						.serviceId(10)//?
						.payload(fieldList);
						;

				switch (i)
				{
					case 1:
						provider.submit(updateMsg, appClient.itemHandle );
						break;
					case 2:
						provider.submit(genericMsg, appClient.itemHandle);
						break;
					case 3:
						provider.submit(statusMsg, appClient.itemHandle );
						break;
					case 4:
						provider.submit(updateMsgShort, appClient.itemHandle );
						break;
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
