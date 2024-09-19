///*|----------------------------------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019 LSEG. All rights reserved.     
///*|----------------------------------------------------------------------------------------------------

package com.refinitiv.ema.examples.training.consumer.series400.ex490_Specify_Dict_Object;

import com.refinitiv.ema.access.Msg;
import com.refinitiv.ema.access.AckMsg;
import com.refinitiv.ema.access.GenericMsg;
import com.refinitiv.ema.access.RefreshMsg;
import com.refinitiv.ema.access.ReqMsg;
import com.refinitiv.ema.access.StatusMsg;
import com.refinitiv.ema.access.UpdateMsg;
import com.refinitiv.ema.rdm.DataDictionary;
import com.refinitiv.ema.access.Data;
import com.refinitiv.ema.access.DataType;
import com.refinitiv.ema.access.DataType.DataTypes;
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
	public void onRefreshMsg(RefreshMsg refreshMsg, OmmConsumerEvent event)
	{
		System.out.println("Item Name: " + (refreshMsg.hasName() ? refreshMsg.name() : "<not set>"));
		System.out.println("Service Name: " + (refreshMsg.hasServiceName() ? refreshMsg.serviceName() : "<not set>"));
		
		System.out.println("Item State: " + refreshMsg.state());
		
		if (DataType.DataTypes.FIELD_LIST == refreshMsg.payload().dataType())
			decode(refreshMsg.payload().fieldList());
		
		System.out.println();
	}
	
	public void onUpdateMsg(UpdateMsg updateMsg, OmmConsumerEvent event) 
	{
		System.out.println("Item Name: " + (updateMsg.hasName() ? updateMsg.name() : "<not set>"));
		System.out.println("Service Name: " + (updateMsg.hasServiceName() ? updateMsg.serviceName() : "<not set>"));
		
		if (DataType.DataTypes.FIELD_LIST == updateMsg.payload().dataType())
			decode(updateMsg.payload().fieldList());
		
		System.out.println();
	}

	public void onStatusMsg(StatusMsg statusMsg, OmmConsumerEvent event) 
	{
		System.out.println("Item Name: " + (statusMsg.hasName() ? statusMsg.name() : "<not set>"));
		System.out.println("Service Name: " + (statusMsg.hasServiceName() ? statusMsg.serviceName() : "<not set>"));

		if (statusMsg.hasState())
			System.out.println("Item State: " +statusMsg.state());
		
		System.out.println();
	}
	
	public void onGenericMsg(GenericMsg genericMsg, OmmConsumerEvent consumerEvent){}
	public void onAckMsg(AckMsg ackMsg, OmmConsumerEvent consumerEvent){}
	public void onAllMsg(Msg msg, OmmConsumerEvent consumerEvent){}

	void decode(FieldList fieldList)
	{
		for (FieldEntry fieldEntry : fieldList)
		{
			System.out.print("Fid: " + fieldEntry.fieldId() + " Name = " + fieldEntry.name() + " DataType: " + DataType.asString(fieldEntry.load().dataType()) + " Value: ");

			if (Data.DataCode.BLANK == fieldEntry.code())
				System.out.println(" blank");
			else
				switch (fieldEntry.loadType())
				{
				case DataTypes.REAL :
					System.out.println(fieldEntry.real().asDouble());
					break;
				case DataTypes.DATE :
					System.out.println(fieldEntry.date().day() + " / " + fieldEntry.date().month() + " / " + fieldEntry.date().year());
					break;
				case DataTypes.TIME :
					System.out.println(fieldEntry.time().hour() + ":" + fieldEntry.time().minute() + ":" + fieldEntry.time().second() + ":" + fieldEntry.time().millisecond());
					break;
				case DataTypes.DATETIME :
					System.out.println(fieldEntry.dateTime().day() + " / " + fieldEntry.dateTime().month() + " / " +
						fieldEntry.dateTime().year() + "." + fieldEntry.dateTime().hour() + ":" + 
						fieldEntry.dateTime().minute() + ":" + fieldEntry.dateTime().second() + ":" + 
						fieldEntry.dateTime().millisecond() + ":" + fieldEntry.dateTime().microsecond()+ ":" + 
						fieldEntry.dateTime().nanosecond());
					break;
				case DataTypes.INT :
					System.out.println(fieldEntry.intValue());
					break;
				case DataTypes.UINT :
					System.out.println(fieldEntry.uintValue());
					break;
				case DataTypes.ASCII :
					System.out.println(fieldEntry.ascii());
					break;
				case DataTypes.ENUM :
					System.out.println(fieldEntry.hasEnumDisplay() ? fieldEntry.enumDisplay() : fieldEntry.enumValue());
					break;
				case DataTypes.RMTES :
					System.out.println(fieldEntry.rmtes());
					break;
				case DataTypes.ERROR :
					System.out.println("(" + fieldEntry.error().errorCodeAsString() + ")");
					break;
				default :
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
		OmmConsumer consumer1 = null;
		OmmConsumer consumer2 = null;
		try
		{
			AppClient appClient = new AppClient();
			OmmConsumerConfig config = EmaFactory.createOmmConsumerConfig();
			ReqMsg reqMsg = EmaFactory.createReqMsg();
			if (!readCommandlineArgs(args)) return;
			// Create DataDictionary and load it from our dictionary files
			DataDictionary dictionary = EmaFactory.createDataDictionary();
			dictionary.loadFieldDictionary("./RDMFieldDictionary");
			dictionary.loadEnumTypeDictionary("./enumtype.def");
			
			// Specify DataDictionary inside of our OmmConsumerConfig
			config.dataDictionary(dictionary, shouldCopyIntoAPI);
			System.out.println("Config shouldCopyIntoAPI set to : " + shouldCopyIntoAPI);	
			
			consumer1  = EmaFactory.createOmmConsumer(config.consumerName("Consumer_1"));	
			consumer1.registerClient(reqMsg.serviceName("DIRECT_FEED").name("IBM.N"), appClient);		
			Thread.sleep(30000);			// API calls onRefreshMsg(), onUpdateMsg() and onStatusMsg()
			consumer1.uninitialize();
			consumer1 = null;
			System.out.println("Consumer_1 has been uninitialized");
			
			consumer2  = EmaFactory.createOmmConsumer(config.consumerName("Consumer_2"));	
			consumer2.registerClient(reqMsg.clear().serviceName("DIRECT_FEED").name("TRI.N"), appClient);		
			Thread.sleep(30000);			// API calls onRefreshMsg(), onUpdateMsg() and onStatusMsg()
		}
		catch (InterruptedException | OmmException excp)
		{
			System.out.println(excp.getMessage());
		}
		finally 
		{
			if (consumer1 != null) consumer1.uninitialize();
			if (consumer2 != null) consumer2.uninitialize();
		}
	}
}


