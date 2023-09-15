///*|----------------------------------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      	--
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  					--
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            		--
///*|----------------------------------------------------------------------------------------------------

package com.refinitiv.ema.examples.training.consumer.series300.ex332_Dictionary_Streaming;

import com.refinitiv.ema.access.Msg;
import com.refinitiv.ema.access.AckMsg;
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
import com.refinitiv.ema.access.OmmConsumerConfig;
import com.refinitiv.ema.access.OmmConsumerEvent;
import com.refinitiv.ema.access.OmmException;

class AppClient implements OmmConsumerClient
{
	DataDictionary dataDictionary = EmaFactory.createDataDictionary();
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
	static boolean shouldCopyIntoAPI = false;
	
	static void printHelp()
	{
	    System.out.println("\nOptions:\n" + "  -?\tShows this usage\n"
	    		+ "  -shouldCopyIntoAPI to enable shouldCopyIntoAPI (default is false) \r\n" 
	    		+ "\n");
	}
	
	static boolean readCommandlineArgs(String[] args)
	{
	    try
	    {
	        int argsCount = 0;

	        while (argsCount < args.length)
	        {
	            if (0 == args[argsCount].compareTo("-?"))
	            {
	                printHelp();
	                return false;
	            }
    			else if ("-shouldCopyIntoAPI".equals(args[argsCount]))
    			{
    				shouldCopyIntoAPI = true;
    				++argsCount;
    			}			
    			else // unrecognized command line argument
    			{
    				printHelp();
    				return false;
    			}			
    		}
        }
        catch (Exception e)
        {
        	printHelp();
            return false;
        }
		
		return true;
	}	
	
	public static void main(String[] args)
	{
		OmmConsumer consumer = null;
		try
		{
			AppClient appClient = new AppClient();
			if (!readCommandlineArgs(args)) return;
			//consumer = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig().host("localhost:14002").username("user"));
			OmmConsumerConfig config = EmaFactory.createOmmConsumerConfig().host("localhost:14002").username("user1");
			consumer = EmaFactory.createOmmConsumer(config);

			ReqMsg reqMsg = EmaFactory.createReqMsg();

			consumer.registerClient(reqMsg.domainType(EmaRdm.MMT_DICTIONARY).name("RWFFld").
					 filter(EmaRdm.DICTIONARY_NORMAL), appClient);

			consumer.registerClient(reqMsg.clear().domainType(EmaRdm.MMT_DICTIONARY).name("RWFEnum").
					filter(EmaRdm.DICTIONARY_NORMAL),appClient);
			
			Thread.sleep(20000);
			config.dataDictionary(appClient.dataDictionary, shouldCopyIntoAPI);
			System.out.println("Config shouldCopyIntoAPI set to : " + shouldCopyIntoAPI);	

			consumer.registerClient(reqMsg.clear().serviceName("DIRECT_FEED").name("IBM.N"), appClient);

			Thread.sleep(40000); // API calls onRefreshMsg(), onUpdateMsg() and
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
