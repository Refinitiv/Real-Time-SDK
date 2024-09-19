///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2024 LSEG. All rights reserved.     
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.unittest;

import java.math.RoundingMode;
import java.text.DecimalFormat;
import java.util.Iterator;

import com.refinitiv.ema.access.AckMsg;
import com.refinitiv.ema.access.DataType.DataTypes;
import com.refinitiv.ema.access.EmaFactory;
import com.refinitiv.ema.access.FieldEntry;
import com.refinitiv.ema.access.FieldList;
import com.refinitiv.ema.access.GenericMsg;
import com.refinitiv.ema.access.Msg;
import com.refinitiv.ema.access.OmmConsumer;
import com.refinitiv.ema.access.OmmConsumerClient;
import com.refinitiv.ema.access.OmmConsumerConfig;
import com.refinitiv.ema.access.OmmConsumerEvent;
import com.refinitiv.ema.access.OmmException;
import com.refinitiv.ema.access.OmmIProviderConfig;
import com.refinitiv.ema.access.OmmInvalidUsageException;
import com.refinitiv.ema.access.OmmProvider;
import com.refinitiv.ema.access.OmmProviderClient;
import com.refinitiv.ema.access.OmmProviderEvent;
import com.refinitiv.ema.access.OmmReal;
import com.refinitiv.ema.access.OmmState;
import com.refinitiv.ema.access.PackedMsg;
import com.refinitiv.ema.access.PostMsg;
import com.refinitiv.ema.access.RefreshMsg;
import com.refinitiv.ema.access.ReqMsg;
import com.refinitiv.ema.access.Series;
import com.refinitiv.ema.access.StatusMsg;
import com.refinitiv.ema.access.UpdateMsg;
import com.refinitiv.ema.rdm.DataDictionary;
import com.refinitiv.ema.rdm.EmaRdm;

import junit.framework.TestCase;

public class PackedMsgTests extends TestCase 
{
	public PackedMsgTests(String name)
	{
		super(name);
	}
	
	class OmmConsumerTestClient implements OmmConsumerClient
	{
		public int updateMsgCount = 0;
		public void onRefreshMsg(RefreshMsg refreshMsg, OmmConsumerEvent consumerEvent) {
			TestUtilities.checkResult(refreshMsg.streamId() == 5, "RefreshMsg.streamId()");
			
			TestUtilities.checkResult(refreshMsg.serviceId() == 1, "RefreshMsg.serviceId()");
		}

		public void onUpdateMsg(UpdateMsg updateMsg, OmmConsumerEvent consumerEvent) {
			
			TestUtilities.checkResult(updateMsg.streamId() == 5, "UpdateMsg.streamId()");

			TestUtilities.checkResult(updateMsg.hasMsgKey(), "UpdateMsg.hasMsgKey()");

			TestUtilities.checkResult(updateMsg.hasServiceId(), "UpdateMsg.hasServiceId()");
			
			TestUtilities.checkResult(updateMsg.serviceId() == 1 , "UpdateMsg.serviceId()");

			TestUtilities.checkResult(updateMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST, "UpdateMsg.payload().dataType()");

			FieldList fieldList = updateMsg.payload().fieldList();
			Iterator<FieldEntry> fieldEntryIter = fieldList.iterator();
			FieldEntry fieldEntry;
			DecimalFormat df = new DecimalFormat("0.00");
			while (fieldEntryIter.hasNext())
			{
				fieldEntry = fieldEntryIter.next();
				TestUtilities.checkResult(fieldEntry.load().dataType() == DataTypes.REAL, "UpdateMsg FieldList FieldEntry DataType");
				if (fieldEntry.fieldId() == 22)
				{
					double value = (3991 + updateMsgCount) * 0.01;
					df.setRoundingMode(RoundingMode.DOWN);
					TestUtilities.checkResult(fieldEntry.real().asDouble() == (Double.valueOf(df.format(value))));
				}
				else if (fieldEntry.fieldId() == 30)
				{
					double value = (10 + updateMsgCount);
					TestUtilities.checkResult(fieldEntry.real().asDouble() == value);
				}
			}

			updateMsgCount++;
			
			
			//System.out.println(updateMsg);
		}
		
		public void onStatusMsg(StatusMsg statusMsg, OmmConsumerEvent consumerEvent) {
			//System.out.println(statusMsg);
		}

		public void onGenericMsg(GenericMsg genericMsg, OmmConsumerEvent consumerEvent) {}
		public void onAckMsg(AckMsg ackMsg, OmmConsumerEvent consumerEvent) {}
		public void onAllMsg(Msg msg, OmmConsumerEvent consumerEvent) {}
		
	}
	
	class OmmProviderTestClient implements OmmProviderClient
	{
		public long itemHandle = 0;
		public long clientHandle = 0;
		public DataDictionary dataDictionary = EmaFactory.createDataDictionary();
		private int fragmentationSize = 96000;
		private Series series = EmaFactory.createSeries();
		private RefreshMsg refreshMsg = EmaFactory.createRefreshMsg();
		private int currentValue;
		private boolean result;
		
		OmmProviderTestClient()
		{
			dataDictionary.loadFieldDictionary( TestDictionaries.fieldDictionaryFileName );
			dataDictionary.loadEnumTypeDictionary( TestDictionaries.enumTableFileName );
		}

		public void onRefreshMsg(RefreshMsg refreshMsg, OmmProviderEvent providerEvent) {}
		public void onStatusMsg(StatusMsg statusMsg, OmmProviderEvent providerEvent) {}
		public void onGenericMsg(GenericMsg genericMsg, OmmProviderEvent providerEvent) {}
		public void onPostMsg(PostMsg postMsg, OmmProviderEvent providerEvent) {}
		public void onReissue(ReqMsg reqMsg, OmmProviderEvent providerEvent) {}
		public void onClose(ReqMsg reqMsg, OmmProviderEvent providerEvent) {}
		public void onAllMsg(Msg msg, OmmProviderEvent providerEvent) {}
		
		@Override
		public void onReqMsg(ReqMsg reqMsg, OmmProviderEvent providerEvent) {
			switch (reqMsg.domainType())
			{
				case EmaRdm.MMT_LOGIN :
					processLoginRequest(reqMsg, providerEvent);
					break;
				case EmaRdm.MMT_MARKET_PRICE :
					processMarketPriceRequest(reqMsg, providerEvent);
					break;
				case EmaRdm.MMT_DICTIONARY:
					processDictionaryRequest(reqMsg, providerEvent);
					break;
				default :
					processInvalidItemRequest(reqMsg, providerEvent);
					break;
			}
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
		
		void processLoginRequest(ReqMsg reqMsg, OmmProviderEvent event)
		{
			clientHandle = event.clientHandle();
			event.provider().submit( EmaFactory.createRefreshMsg().domainType(EmaRdm.MMT_LOGIN).name(reqMsg.name()).
					nameType(EmaRdm.USER_NAME).complete(true).solicited(true).
					state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "Login accepted"),
					event.handle() );
		}
		
		void processDictionaryRequest(ReqMsg reqMsg, OmmProviderEvent event)
		{
			result = false;
			refreshMsg.clear().clearCache( true );

			if ( reqMsg.name().equals( "RWFFld" ) )
			{
				currentValue = dataDictionary.minFid();

				while ( !result )
				{
					currentValue = dataDictionary.encodeFieldDictionary( series, currentValue, reqMsg.filter(), fragmentationSize );
					
					result = currentValue == dataDictionary.maxFid() ? true : false;

					event.provider().submit( refreshMsg.name( reqMsg.name() ).serviceName( reqMsg.serviceName() ).
							domainType( EmaRdm.MMT_DICTIONARY ).filter( reqMsg.filter() ).payload( series ).complete( result ).
							solicited( true ), event.handle() );
					
					refreshMsg.clear();
				}
			}
			else if ( reqMsg.name().equals( "RWFEnum" ) )
			{
				currentValue = 0;

				while ( !result )
				{
					currentValue = dataDictionary.encodeEnumTypeDictionary( series, currentValue, reqMsg.filter(), fragmentationSize );
					
					result =  currentValue == dataDictionary.enumTables().size() ? true : false;
						
					event.provider().submit( refreshMsg.name( reqMsg.name() ).serviceName( reqMsg.serviceName() ).
							domainType( EmaRdm.MMT_DICTIONARY ).filter( reqMsg.filter() ).payload( series ).complete( result ).
							solicited( true ), event.handle() );
					
					refreshMsg.clear();
				}
			}
		}
		
		void processInvalidItemRequest(ReqMsg reqMsg, OmmProviderEvent event)
		{
			event.provider().submit( EmaFactory.createStatusMsg().name(reqMsg.name()).serviceName(reqMsg.serviceName()).
					state(OmmState.StreamState.CLOSED, OmmState.DataState.SUSPECT,	OmmState.StatusCode.NOT_FOUND, "Item not found"),
					event.handle() );
		}
		
	}
	
	class ConsumerThread extends Thread {
		
		OmmConsumer consumer = null;
		OmmConsumerTestClient consumerClient = null;
		
	    public void run()
	    {
	        try {
	    		OmmConsumerConfig config = EmaFactory.createOmmConsumerConfig();
	    		
	    		consumer  = EmaFactory.createOmmConsumer(config.host("localhost:14002").username("user"));

	    		consumerClient = new OmmConsumerTestClient();
				
				ReqMsg reqMsg = EmaFactory.createReqMsg();
	    		
	    		System.out.println("Start EMA OmmConsumer registerClient() call");
				consumer.registerClient(reqMsg.serviceName("DIRECT_FEED").name("REU"), consumerClient);
				
	        }
	        catch (Exception e) {

	        }
	    }
	    
	    public void shutdown()
	    {
	    	if (consumer != null)
	    		consumer.uninitialize();
	    }
	    
	    public int getUpdateMsgCount()
	    {
	    	return consumerClient.updateMsgCount;
	    }
	}

	public void testPackedMsg_Encode_Decode()
	{
		TestUtilities.printTestHead("testPackedMsg_Encode_Decode", "ETA handling encoding a PackedMsg using EMA OmmProvider, and decoding using EMA OmmConsumer");	
		
		OmmProvider provider = null;
		OmmProviderTestClient providerClient = new OmmProviderTestClient();
		ConsumerThread consumerThread = new ConsumerThread();

		try {
			
			OmmIProviderConfig providerConfig = EmaFactory.createOmmIProviderConfig();
			System.out.println("Start EMA OmmProvider");
			provider = EmaFactory.createOmmProvider(providerConfig.port("14002").
					adminControlDictionary(OmmIProviderConfig.AdminControl.USER_CONTROL), providerClient);

			consumerThread.start();
			
			while(providerClient.itemHandle == 0)
			{
				provider.dispatch(1000);
				Thread.sleep(1000);
			}
			
			PackedMsg packedMsg = EmaFactory.createPackedMsg(provider);
			packedMsg.initBuffer(providerClient.clientHandle);
			FieldList fieldList = EmaFactory.createFieldList();
			
			System.out.println("Begin EMA Encoding of 10 UpdateMsgs, packed into PackedMsg object");
			
			for( int i = 0; i < 10; i++ )
			{
				fieldList.clear();
				fieldList.add(EmaFactory.createFieldEntry().real(22, 3991 + i, OmmReal.MagnitudeType.EXPONENT_NEG_2));
				fieldList.add(EmaFactory.createFieldEntry().real(30, 10 + i, OmmReal.MagnitudeType.EXPONENT_0));
				
				UpdateMsg msg = EmaFactory.createUpdateMsg().payload( fieldList );
				
				packedMsg.addMsg(msg, providerClient.itemHandle);
			}

			TestUtilities.checkResult(packedMsg.remainingSize() < packedMsg.maxSize());
			TestUtilities.checkResult(packedMsg.packedMsgCount() == 10);

			System.out.println("decode PackedMsg object in EMA Consumer and ensure all messages contained in PackedMsg");
		
			provider.submit(packedMsg);
			packedMsg.clear();
			provider.dispatch(1000);
		}
		catch (InterruptedException | OmmException excp)
		{
			System.out.println(excp.getMessage());
			TestUtilities.checkResult(false);
		}
		finally 
		{
			try {
				Thread.sleep(3000);	// Allow time for the consumer to get the messages
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
			TestUtilities.checkResult(consumerThread.getUpdateMsgCount() == 10);
			consumerThread.shutdown();
			if (provider != null) provider.uninitialize();
		}
		
		System.out.println("End EMA PackedMsg Encoding, EMA Packed Message Decoding");
		System.out.println();
	}
	
	public void testPackedMsg_SendRefreshAndUpdates()
	{
		TestUtilities.printTestHead("testPackedMsg_Encode_Decode", "ETA handling encoding a PackedMsg using EMA OmmProvider, and decoding using EMA OmmConsumer");	
		
		OmmProvider provider = null;
		OmmProviderTestClient providerClient = new OmmProviderTestClient();
		ConsumerThread consumerThread = new ConsumerThread();

		try {
			
			OmmIProviderConfig providerConfig = EmaFactory.createOmmIProviderConfig();
			System.out.println("Start EMA OmmProvider");
			provider = EmaFactory.createOmmProvider(providerConfig.port("14002").
					adminControlDictionary(OmmIProviderConfig.AdminControl.USER_CONTROL), providerClient);

			consumerThread.start();
			
			while(providerClient.itemHandle == 0)
			{
				provider.dispatch(1000);
				Thread.sleep(1000);
			}
			
			PackedMsg packedMsg = EmaFactory.createPackedMsg(provider);
			packedMsg.initBuffer(providerClient.clientHandle);
			
			// Pack Refresh Message first
			System.out.println("Begin EMA Encoding of RefreshMsg, packed into PackedMsg object");
			
			RefreshMsg refreshMsg = EmaFactory.createRefreshMsg();
			FieldList fieldList = EmaFactory.createFieldList();
			fieldList.add( EmaFactory.createFieldEntry().real(22, 3990, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(25, 3994, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(30, 9,  OmmReal.MagnitudeType.EXPONENT_0));
			fieldList.add( EmaFactory.createFieldEntry().real(31, 19, OmmReal.MagnitudeType.EXPONENT_0));
			
			refreshMsg = EmaFactory.createRefreshMsg().serviceName("DIRECT_FEED").name("IBM.N")
					.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
					.payload(fieldList).complete(true);
			
			packedMsg.addMsg(refreshMsg, providerClient.itemHandle);
			
			System.out.println("Begin EMA Encoding of 10 UpdateMsgs, packed into PackedMsg object");
			
			for( int i = 0; i < 10; i++ )
			{
				fieldList.clear();
				fieldList.add(EmaFactory.createFieldEntry().real(22, 3991 + i, OmmReal.MagnitudeType.EXPONENT_NEG_2));
				fieldList.add(EmaFactory.createFieldEntry().real(30, 10 + i, OmmReal.MagnitudeType.EXPONENT_0));
				
				UpdateMsg msg = EmaFactory.createUpdateMsg().payload( fieldList );
				
				packedMsg.addMsg(msg, providerClient.itemHandle);
			}

			TestUtilities.checkResult(packedMsg.remainingSize() < packedMsg.maxSize());
			TestUtilities.checkResult(packedMsg.packedMsgCount() == 11);

			System.out.println("decode PackedMsg object in EMA Consumer and ensure all messages contained in PackedMsg");
		
			provider.submit(packedMsg);
			packedMsg.clear();
			provider.dispatch(1000);
		}
		catch (InterruptedException | OmmException excp)
		{
			System.out.println(excp.getMessage());
			TestUtilities.checkResult(false);
		}
		finally 
		{
			try {
				Thread.sleep(3000);	// Allow time for the consumer to get the messages
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
			TestUtilities.checkResult(consumerThread.getUpdateMsgCount() == 10);
			consumerThread.shutdown();
			if (provider != null) provider.uninitialize();
		}
		
		System.out.println("End EMA PackedMsg Encoding, EMA Packed Message Decoding");
		System.out.println();
	}
	
	public void testPackedMsg_AddingMsgToFullPackedMsg()
	{
		TestUtilities.printTestHead("testPackedMsg_AddingMsgToFullPackedMsg", "Attempting to add a message to an already full PackedMsg should return error");	
		
		OmmProvider provider = null;
		OmmProviderTestClient providerClient = new OmmProviderTestClient();
		ConsumerThread consumerThread = new ConsumerThread();

		try {
			
			OmmIProviderConfig providerConfig = EmaFactory.createOmmIProviderConfig();
			System.out.println("Start EMA OmmProvider");
			provider = EmaFactory.createOmmProvider(providerConfig.port("14002").
					adminControlDictionary(OmmIProviderConfig.AdminControl.USER_CONTROL), providerClient);

			consumerThread.start();
			
			while(providerClient.itemHandle == 0)
			{
				provider.dispatch(1000);
				Thread.sleep(1000);
			}
			
			PackedMsg packedMsg = EmaFactory.createPackedMsg(provider);
			packedMsg.initBuffer(providerClient.clientHandle, 50);
			FieldList fieldList = EmaFactory.createFieldList();
			
			System.out.println("Begin EMA Encoding of 10 UpdateMsgs, packed into PackedMsg object");
			
			for( int i = 0; i < 10; i++ )
			{
				fieldList.clear();
				fieldList.add(EmaFactory.createFieldEntry().real(22, 3991 + i, OmmReal.MagnitudeType.EXPONENT_NEG_2));
				fieldList.add(EmaFactory.createFieldEntry().real(30, 10 + i, OmmReal.MagnitudeType.EXPONENT_0));
				
				UpdateMsg msg = EmaFactory.createUpdateMsg().payload( fieldList );
				
				packedMsg.addMsg(msg, providerClient.itemHandle);
			}

			TestUtilities.checkResult(packedMsg.remainingSize() < packedMsg.maxSize());
			TestUtilities.checkResult(packedMsg.packedMsgCount() == 10);

			System.out.println("decode PackedMsg object in EMA Consumer and ensure all messages contained in PackedMsg");
		
			provider.submit(packedMsg);
			packedMsg.clear();
			provider.dispatch(1000);
		}
		catch (InterruptedException | OmmException excp)
		{
			System.out.println(excp.getMessage());
			TestUtilities.checkResult(((OmmInvalidUsageException)excp).errorCode(), OmmInvalidUsageException.ErrorCode.BUFFER_TOO_SMALL);
		}
		finally 
		{
			consumerThread.shutdown();
			if (provider != null) provider.uninitialize();
		}
		
		System.out.println("End Adding To Full Packed Message");
		System.out.println();
	}
	
	public void testPackedMsg_AddingMsgsToMakeExactlyFullPackedMsg()
	{
		TestUtilities.printTestHead("testPackedMsg_AddingMsgsToMakeExactlyFullPackedMsg", "Adding enough message data to exactly fill a packed message should be successful");	
		
		OmmProvider provider = null;
		OmmProviderTestClient providerClient = new OmmProviderTestClient();
		ConsumerThread consumerThread = new ConsumerThread();

		try {
			
			OmmIProviderConfig providerConfig = EmaFactory.createOmmIProviderConfig();
			System.out.println("Start EMA OmmProvider");
			provider = EmaFactory.createOmmProvider(providerConfig.port("14002").
					adminControlDictionary(OmmIProviderConfig.AdminControl.USER_CONTROL), providerClient);

			consumerThread.start();
			
			while(providerClient.itemHandle == 0)
			{
				provider.dispatch(1000);
				Thread.sleep(1000);
			}
			
			PackedMsg packedMsg = EmaFactory.createPackedMsg(provider);
			packedMsg.initBuffer(providerClient.clientHandle, 52);
			FieldList fieldList = EmaFactory.createFieldList();
			
			System.out.println("Begin EMA Encoding of 2 UpdateMsgs, packed into PackedMsg object");
			
			for( int i = 0; i < 2; i++ )
			{
				fieldList.clear();
				fieldList.add(EmaFactory.createFieldEntry().real(22, 3991 + i, OmmReal.MagnitudeType.EXPONENT_NEG_2));
				fieldList.add(EmaFactory.createFieldEntry().real(30, 10 + i, OmmReal.MagnitudeType.EXPONENT_0));
				
				UpdateMsg msg = EmaFactory.createUpdateMsg().payload( fieldList );
				packedMsg.addMsg(msg, providerClient.itemHandle);
			}

			TestUtilities.checkResult(packedMsg.remainingSize() < packedMsg.maxSize());
			TestUtilities.checkResult(packedMsg.packedMsgCount() == 2);
			TestUtilities.checkResult(packedMsg.remainingSize() == 0);

			System.out.println("decode PackedMsg object in EMA Consumer and ensure all messages contained in PackedMsg");
		
			provider.submit(packedMsg);
			packedMsg.clear();
			provider.dispatch(1000);
		}
		catch (InterruptedException | OmmException excp)
		{
			System.out.println(excp.getMessage());
		}
		finally 
		{
			try {
				Thread.sleep(3000);	// Allow time for the consumer to get the messages
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
			TestUtilities.checkResult(consumerThread.getUpdateMsgCount() == 2);
			consumerThread.shutdown();
			if (provider != null) provider.uninitialize();
		}
		
		System.out.println("End Packing Messages Up To Exact Max Capacity Of Packed Message");
		System.out.println();
	}
}
