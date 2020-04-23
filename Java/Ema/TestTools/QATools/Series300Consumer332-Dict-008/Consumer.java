///*|----------------------------------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      	--
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  					--
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            		--
///*|----------------------------------------------------------------------------------------------------

package com.thomsonreuters.ema.examples.training.consumer.series300.example332__Dictionary__Streaming;

import com.thomsonreuters.ema.access.Msg;
import com.thomsonreuters.ema.access.AckMsg;
import com.thomsonreuters.ema.access.GenericMsg;
import com.thomsonreuters.ema.access.RefreshMsg;
import com.thomsonreuters.ema.access.ReqMsg;
import com.thomsonreuters.ema.access.StatusMsg;
import com.thomsonreuters.ema.access.UpdateMsg;
import com.thomsonreuters.ema.access.Data;
import com.thomsonreuters.ema.access.DataType;
import com.thomsonreuters.ema.access.DataType.DataTypes;
import com.thomsonreuters.ema.rdm.DataDictionary;
import com.thomsonreuters.ema.rdm.EmaRdm;
import com.thomsonreuters.ema.access.EmaFactory;
import com.thomsonreuters.ema.access.FieldEntry;
import com.thomsonreuters.ema.access.FieldList;
import com.thomsonreuters.ema.access.OmmConsumer;
import com.thomsonreuters.ema.access.OmmConsumerClient;
import com.thomsonreuters.ema.access.OmmConsumerEvent;
import com.thomsonreuters.ema.access.OmmException;

class AppClient implements OmmConsumerClient
{
	private DataDictionary dataDictionary = EmaFactory.createDataDictionary();
	private boolean fldDictComplete = false;
	private boolean enumTypeComplete = false;
	
	public void onRefreshMsg(RefreshMsg refreshMsg, OmmConsumerEvent event)
	{
		System.out.println("Received Refresh. Item Handle: " + event.handle() + " Closure: " + event.closure());

		System.out.println("Item Name: " + (refreshMsg.hasName() ? refreshMsg.name() : "<not set>"));
		System.out.println("Service Name: " + (refreshMsg.hasServiceName() ? refreshMsg.serviceName() : "<not set>"));

		System.out.println("Item State: " + refreshMsg.state());

		decode(refreshMsg, refreshMsg.complete());

		System.out.println();
	}

	public void onUpdateMsg(UpdateMsg updateMsg, OmmConsumerEvent event)
	{
		System.out.println("Received Update. Item Handle: " + event.handle() + " Closure: " + event.closure());

		System.out.println("Item Name: " + (updateMsg.hasName() ? updateMsg.name() : "<not set>"));
		System.out.println("Service Name: " + (updateMsg.hasServiceName() ? updateMsg.serviceName() : "<not set>"));

		decode(updateMsg, false);

		System.out.println();
	}

	public void onStatusMsg(StatusMsg statusMsg, OmmConsumerEvent event)
	{
		System.out.println("Received Status. Item Handle: " + event.handle() + " Closure: " + event.closure());

		System.out.println("Item Name: " + (statusMsg.hasName() ? statusMsg.name() : "<not set>"));
		System.out.println("Service Name: " + (statusMsg.hasServiceName() ? statusMsg.serviceName() : "<not set>"));

		if (statusMsg.hasState())
			System.out.println("Item State: " + statusMsg.state());

		System.out.println();
	}

	public void onAckMsg(AckMsg ackMsg, OmmConsumerEvent event){}
	public void onGenericMsg(GenericMsg genericMsg, OmmConsumerEvent event){}
	public void onAllMsg(Msg msg, OmmConsumerEvent event){}

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
				System.out.println(dataDictionary);
				//API QA
				com.thomsonreuters.ema.rdm.DictionaryEntry entry22 = dataDictionary.entry(22);
				com.thomsonreuters.ema.rdm.DictionaryEntry entry25 = dataDictionary.entry(25);
				System.out.println("QA Prints entry22 : " + entry22.acronym());
				System.out.println("QA Prints entry25 : " + entry25.acronym());
				if (entry22==entry25) 
					System.out.println("Test 1 : QA Prints check entry1 and entry2 are equal ");
				com.thomsonreuters.ema.rdm.DictionaryEntry entryBID = dataDictionary.entry("BID");
				com.thomsonreuters.ema.rdm.DictionaryEntry entryASK = dataDictionary.entry("ASK");
				System.out.println("QA Prints entryBID fid : " + entryBID.fid());
				System.out.println("QA Prints entryASK fid : " + entryASK.fid());
				if (entryBID==entryASK) 
					System.out.println("Test 2 : QA Prints check entryBID and entryASK are equal ");
				
				com.thomsonreuters.ema.rdm.DictionaryEntry entry30 = EmaFactory.createDictionaryEntry();
				dataDictionary.entry(30, entry30);
				com.thomsonreuters.ema.rdm.DictionaryEntry entry31 = EmaFactory.createDictionaryEntry();
				dataDictionary.entry(31, entry31);
				System.out.println("QA Prints entry30 fname : " + entry30.acronym());	
				System.out.println("QA Prints entry31 fname : " + entry31.acronym());	
				if (entry30==entry31) 
					System.out.println("Test 3 : QA Prints check entry30 and entry31 are equal ");
				else
					System.out.println("Test 3 : QA Prints check entry30 and entry31 are NOT equal ");
				
				com.thomsonreuters.ema.rdm.DictionaryEntry entryBIDSIZE = EmaFactory.createDictionaryEntry();
				dataDictionary.entry("BIDSIZE", entryBIDSIZE);
				com.thomsonreuters.ema.rdm.DictionaryEntry entryASKSIZE = EmaFactory.createDictionaryEntry();
				dataDictionary.entry("ASKSIZE", entryASKSIZE);
				System.out.println("QA Prints entryBIDSIZE fid : " + entryBIDSIZE.fid());	
				System.out.println("QA Prints entryASKSIZE fid : " + entryASKSIZE.fid());	
				if (entryBIDSIZE==entryASKSIZE) 
					System.out.println("Test 4 : QA Prints check entryBIDSIZE and entryASKSIZE are equal ");
				else
					System.out.println("Test 4 : QA Prints check entryBIDSIZE and entryASKSIZE are NOT equal ");
				
				try
				{
					System.out.println("Test 5 Error Case : Trying to use entry owned by API (entry22)...");
					dataDictionary.entry(22,entry22);
				}
				catch (com.thomsonreuters.ema.access.OmmException ex)
				{
					System.out.println("QA Prints Exception Type : " + ex.exceptionTypeAsString());
					System.out.println("QA Prints Exception Message : " + ex.getMessage());
				}
				try
				{
					System.out.println("Test 6 Error Case : Trying to use entry which is null (entryNull)...");
					com.thomsonreuters.ema.rdm.DictionaryEntry entryNull = null;
					dataDictionary.entry(25,entryNull);
				}
				catch (com.thomsonreuters.ema.access.OmmException ex)
				{
					//ex.printStackTrace();
					System.out.println("QA Prints Exception Type : " + ex.exceptionTypeAsString());
					System.out.println("QA Prints Exception Message : " + ex.getMessage());
				}
				try
				{
					System.out.println("Test 7 Error Case : Trying to use entry owned by API (entry25)...");
					dataDictionary.entry("BID",entry22);
				}
				catch (com.thomsonreuters.ema.access.OmmException ex)
				{
					System.out.println("QA Prints Exception Type : " + ex.exceptionTypeAsString());
					System.out.println("QA Prints Exception Message : " + ex.getMessage());
				}
				try
				{
					System.out.println("Test 8 Error Case : Trying to use entry which is null (entryNull)...");
					com.thomsonreuters.ema.rdm.DictionaryEntry entryNull = null;
					dataDictionary.entry("ASK",entryNull);
				}
				catch (com.thomsonreuters.ema.access.OmmException ex)
				{
					//ex.printStackTrace();
					System.out.println("QA Prints Exception Type : " + ex.exceptionTypeAsString());
					System.out.println("QA Prints Exception Message : " + ex.getMessage());
				}
				//End API QA				
			}
		
			break;
		case DataTypes.FIELD_LIST:
			decode(msg.payload().fieldList());
			break;
		default:
			break;
		}
	}
	
	void decode(FieldList fieldList)
	{
		for (FieldEntry fieldEntry : fieldList)
		{
			System.out.print("Fid: " + fieldEntry.fieldId() + " Name = " + fieldEntry.name() + " DataType: "
					+ DataType.asString(fieldEntry.load().dataType()) + " Value: ");

			if (Data.DataCode.BLANK == fieldEntry.code())
				System.out.println(" blank");
			else
				switch (fieldEntry.loadType())
				{
				case DataTypes.REAL:
					System.out.println(fieldEntry.real().asDouble());
					break;
				case DataTypes.DATE:
					System.out.println(fieldEntry.date().day() + " / " + fieldEntry.date().month() + " / "
							+ fieldEntry.date().year());
					break;
				case DataTypes.TIME:
					System.out.println(fieldEntry.time().hour() + ":" + fieldEntry.time().minute() + ":"
							+ fieldEntry.time().second() + ":" + fieldEntry.time().millisecond());
					break;
				case DataTypes.INT:
					System.out.println(fieldEntry.intValue());
					break;
				case DataTypes.UINT:
					System.out.println(fieldEntry.uintValue());
					break;
				case DataTypes.ASCII:
					System.out.println(fieldEntry.ascii());
					break;
				case DataTypes.ENUM:
					System.out.println(fieldEntry.hasEnumDisplay() ? fieldEntry.enumDisplay() : fieldEntry.enumValue());
					break;
				case DataTypes.RMTES:
					System.out.println(fieldEntry.rmtes());
					break;
				case DataTypes.ERROR:
					System.out.println(
							fieldEntry.error().errorCode() + " (" + fieldEntry.error().errorCodeAsString() + ")");
					break;
				default:
					System.out.println();
					break;
				}
		}
	}
}

public class Consumer
{
	public static void main(String[] args)
	{
		OmmConsumer consumer = null;
		try
		{
			AppClient appClient = new AppClient();

			consumer = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig().host("localhost:14002").username("user"));

			ReqMsg reqMsg = EmaFactory.createReqMsg();

			consumer.registerClient(reqMsg.domainType(EmaRdm.MMT_DICTIONARY).name("RWFFld").
					 filter(EmaRdm.DICTIONARY_NORMAL), appClient);

			consumer.registerClient(reqMsg.clear().domainType(EmaRdm.MMT_DICTIONARY).name("RWFEnum").
					filter(EmaRdm.DICTIONARY_NORMAL),appClient);

			consumer.registerClient(reqMsg.clear().serviceName("DIRECT_FEED").name("IBM.N"), appClient);

			Thread.sleep(60000); // API calls onRefreshMsg(), onUpdateMsg() and
									// onStatusMsg()
		} catch (InterruptedException | OmmException excp)
		{
			System.out.println(excp.getMessage());
		}
		finally 
		{
			if (consumer != null) consumer.uninitialize();
		}
	}
}
