/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.examples.training.consumer.series300.ex332_Dictionary_Streaming;

import com.refinitiv.ema.access.Msg;

import java.util.ArrayList;
import java.util.List;

import com.refinitiv.ema.access.AckMsg;
import com.refinitiv.ema.access.ChannelInformation;
import com.refinitiv.ema.access.GenericMsg;
import com.refinitiv.ema.access.RefreshMsg;
import com.refinitiv.ema.access.ReqMsg;
import com.refinitiv.ema.access.StatusMsg;
import com.refinitiv.ema.access.UpdateMsg;
import com.refinitiv.ema.access.Data;
import com.refinitiv.ema.access.DataType;
import com.refinitiv.ema.access.DataType.DataTypes;
import com.refinitiv.ema.rdm.DataDictionary;
import com.refinitiv.ema.rdm.EmaRdm;
import com.refinitiv.ema.access.EmaFactory;
import com.refinitiv.ema.access.FieldEntry;
import com.refinitiv.ema.access.FieldList;
import com.refinitiv.ema.access.OmmConsumer;
import com.refinitiv.ema.access.OmmConsumerClient;
import com.refinitiv.ema.access.OmmConsumerEvent;
import com.refinitiv.ema.access.OmmException;

class AppClient implements OmmConsumerClient
{
	private DataDictionary dataDictionary = EmaFactory.createDataDictionary();
	private boolean fldDictComplete = false;
	private boolean enumTypeComplete = false;
	//API QA
		List<ChannelInformation> channelInList = new ArrayList<ChannelInformation>();
	//END API QA
	
	public void onRefreshMsg(RefreshMsg refreshMsg, OmmConsumerEvent event)
	{
		System.out.println("Received Refresh. Item Handle: " + event.handle() + " Closure: " + event.closure());

		System.out.println("Item Name: " + (refreshMsg.hasName() ? refreshMsg.name() : "<not set>"));
		System.out.println("Service Name: " + (refreshMsg.hasServiceName() ? refreshMsg.serviceName() : "<not set>"));

		System.out.println("Item State: " + refreshMsg.state());

		decode(refreshMsg, refreshMsg.complete());

		System.out.println();
		//API QA
		if (refreshMsg.domainType() == 1)
		{
			System.out.println("\nevent session info (refresh)\n");
			printSessionInfo(event);
		}
		else 
		{
			System.out.println("\nevent channel info (refresh)\n" + event.channelInformation() );
		}
		//END API QA
	}

	public void onUpdateMsg(UpdateMsg updateMsg, OmmConsumerEvent event)
	{
		System.out.println("Received Update. Item Handle: " + event.handle() + " Closure: " + event.closure());

		System.out.println("Item Name: " + (updateMsg.hasName() ? updateMsg.name() : "<not set>"));
		System.out.println("Service Name: " + (updateMsg.hasServiceName() ? updateMsg.serviceName() : "<not set>"));

		decode(updateMsg, false);

		System.out.println();
		//API QA
		if (updateMsg.domainType() == 1)
		{
			System.out.println("\nevent session info (update)\n");
			printSessionInfo(event);
		}
		else 
		{
			System.out.println("\nevent channel info (update)\n" + event.channelInformation() );
		}
		//END API QA
	}

	public void onStatusMsg(StatusMsg statusMsg, OmmConsumerEvent event)
	{
		System.out.println("Received Status. Item Handle: " + event.handle() + " Closure: " + event.closure());

		System.out.println("Item Name: " + (statusMsg.hasName() ? statusMsg.name() : "<not set>"));
		System.out.println("Service Name: " + (statusMsg.hasServiceName() ? statusMsg.serviceName() : "<not set>"));

		if (statusMsg.hasState())
			System.out.println("Item State: " + statusMsg.state());

		System.out.println();
		//API QA
		if (statusMsg.domainType() == 1)
		{
			System.out.println("\nevent session info (status)\n");
			printSessionInfo(event);
		}
		else 
		{
			System.out.println("\nevent channel info (status)\n" + event.channelInformation() );
		}
		//END API QA
	}
	//API QA
		void printSessionInfo(OmmConsumerEvent event)
		{
			event.sessionChannelInfo(channelInList);
			
			for(ChannelInformation channelInfo : channelInList) 
			{
				System.out.println(channelInfo);
			}
		}
	//END API QA
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

			consumer = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig().consumerName("Consumer_10").username("user"));

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
