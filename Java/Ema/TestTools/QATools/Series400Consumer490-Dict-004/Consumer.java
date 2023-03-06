package com.refinitiv.ema.examples.training.consumer.series400.ex490_Specify_Dict_Object;

import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStreamReader;
import java.lang.management.ManagementFactory;
import java.util.Vector;

import javax.management.MBeanServer;

///*|----------------------------------------------------------------------------------------------------
	// *|            This source code is provided under the Apache 2.0 license      	--
	// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
	// *|                See the project's LICENSE.md for details.                  					--
	// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            		--
	///*|----------------------------------------------------------------------------------------------------


	import com.refinitiv.ema.access.AckMsg;
	import com.refinitiv.ema.access.Data;
	import com.refinitiv.ema.access.DataType;
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
	import com.refinitiv.ema.access.RefreshMsg;
	import com.refinitiv.ema.access.ReqMsg;
	import com.refinitiv.ema.access.StatusMsg;
	import com.refinitiv.ema.access.UpdateMsg;
	import com.refinitiv.ema.rdm.DataDictionary;
	import com.refinitiv.ema.rdm.EmaRdm;
	import com.sun.management.*;

	class AppClient implements OmmConsumerClient
	{
		public DataDictionary dataDictionary = EmaFactory.createDataDictionary();
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
		public static void dumpHeap(String filePath, boolean live) throws IOException {
			File f = new File(filePath);
			if(f.exists()) { 
			    // do something
				f.delete();
			}
			MBeanServer server = ManagementFactory.getPlatformMBeanServer();
			HotSpotDiagnosticMXBean mxBean = ManagementFactory.newPlatformMXBeanProxy
			  (server, "com.sun.management:type=HotSpotDiagnostic",
			HotSpotDiagnosticMXBean.class);
			mxBean.dumpHeap(filePath, live);
			System.out.println("Generated head dump file:" + filePath);
			}
		public static void triggerFullGC() throws IOException, InterruptedException {
		    String pid = ManagementFactory.getRuntimeMXBean().getName().split("@")[0];
		    Process process = Runtime.getRuntime().exec(
		            String.format("jmap -histo:live %s", pid)
		    );
		    System.out.println("Process completed with exit code :" + process.waitFor());
		}
		public static void main(String[] args)
		{
			if(args.length!=1)
			{
				System.out.print("Usage: number_Consumers");
				System.exit(0);
			}
			int _numConsumer = Integer.valueOf(args[0]);
			Vector<OmmConsumer> consumerList = new Vector<OmmConsumer>();
			
			OmmConsumer consumer1 = null;
			OmmConsumer consumer2 = null;
			OmmConsumer consumer3 = null;
			OmmConsumer consumer4 = null;
			OmmConsumer consumer5 = null;
			OmmConsumer consumer6 = null;
			OmmConsumer consumer7 = null;
			OmmConsumer consumer8 = null;
			OmmConsumer consumer9 = null;
			OmmConsumer consumer10 = null;
			OmmConsumer consumer11 = null;
			OmmConsumer consumer12 = null;
			OmmConsumer consumer13 = null;
			OmmConsumer consumer14 = null;
			OmmConsumer consumer15 = null;
			OmmConsumer consumer16 = null;
			OmmConsumer consumer17 = null;
			OmmConsumer consumer18 = null;
			OmmConsumer consumer19 = null;
			OmmConsumer consumer20 = null;
			OmmConsumer consumer21 = null;
			OmmConsumer consumer22 = null;
			OmmConsumer consumer23 = null;
			OmmConsumer consumer24 = null;
			OmmConsumer consumer25 = null;
			OmmConsumer consumer26 = null;
			OmmConsumer consumer27 = null;
			OmmConsumer consumer28 = null;
			OmmConsumer consumer29 = null;
			OmmConsumer consumer30 = null;
			try
			{
				AppClient appClient = new AppClient();
				
				OmmConsumerConfig config = EmaFactory.createOmmConsumerConfig();
				
				DataDictionary dictionary = EmaFactory.createDataDictionary();
				dictionary.loadFieldDictionary("./RDMFieldDictionary");
				dictionary.loadEnumTypeDictionary("./enumtype.def");
				config.dataDictionary(dictionary,false);
				
				
				ReqMsg reqMsg = EmaFactory.createReqMsg().serviceName("ELEKTRON_DD").name("IBM.N");
			/*	for(int i=0; i<_numConsumer;++i) {
					consumerList.add(EmaFactory.createOmmConsumer(config.host("10.187.5.157:14002").username("user")));
					consumerList.get(i).registerClient(reqMsg, appClient);
				}
				System.out.println("Created " + consumerList.size() + " OmmConsumer.");*/
				consumer1=EmaFactory.createOmmConsumer(config.host("10.187.5.157:14002").username("user"));
				consumer2=EmaFactory.createOmmConsumer(config.host("10.187.5.157:14002").username("user"));
				consumer3=EmaFactory.createOmmConsumer(config.host("10.187.5.157:14002").username("user"));
				consumer4=EmaFactory.createOmmConsumer(config.host("10.187.5.157:14002").username("user"));
				consumer5=EmaFactory.createOmmConsumer(config.host("10.187.5.157:14002").username("user"));
				consumer6=EmaFactory.createOmmConsumer(config.host("10.187.5.157:14002").username("user"));
				consumer7=EmaFactory.createOmmConsumer(config.host("10.187.5.157:14002").username("user"));
				consumer8=EmaFactory.createOmmConsumer(config.host("10.187.5.157:14002").username("user"));
				consumer9=EmaFactory.createOmmConsumer(config.host("10.187.5.157:14002").username("user"));
				consumer10=EmaFactory.createOmmConsumer(config.host("10.187.5.157:14002").username("user"));
				consumer11=EmaFactory.createOmmConsumer(config.host("10.187.5.157:14002").username("user"));
				consumer12=EmaFactory.createOmmConsumer(config.host("10.187.5.157:14002").username("user"));
				consumer13=EmaFactory.createOmmConsumer(config.host("10.187.5.157:14002").username("user"));
				consumer14=EmaFactory.createOmmConsumer(config.host("10.187.5.157:14002").username("user"));
				consumer15=EmaFactory.createOmmConsumer(config.host("10.187.5.157:14002").username("user"));
				consumer16=EmaFactory.createOmmConsumer(config.host("10.187.5.157:14002").username("user"));
				consumer17=EmaFactory.createOmmConsumer(config.host("10.187.5.157:14002").username("user"));
				consumer18=EmaFactory.createOmmConsumer(config.host("10.187.5.157:14002").username("user"));
				consumer19=EmaFactory.createOmmConsumer(config.host("10.187.5.157:14002").username("user"));
				consumer20=EmaFactory.createOmmConsumer(config.host("10.187.5.157:14002").username("user"));
				consumer21=EmaFactory.createOmmConsumer(config.host("10.187.5.157:14002").username("user"));
				consumer22=EmaFactory.createOmmConsumer(config.host("10.187.5.157:14002").username("user"));
				consumer23=EmaFactory.createOmmConsumer(config.host("10.187.5.157:14002").username("user"));
				consumer24=EmaFactory.createOmmConsumer(config.host("10.187.5.157:14002").username("user"));
				consumer25=EmaFactory.createOmmConsumer(config.host("10.187.5.157:14002").username("user"));
				consumer26=EmaFactory.createOmmConsumer(config.host("10.187.5.157:14002").username("user"));
				consumer27=EmaFactory.createOmmConsumer(config.host("10.187.5.157:14002").username("user"));
				consumer28=EmaFactory.createOmmConsumer(config.host("10.187.5.157:14002").username("user"));
				consumer29=EmaFactory.createOmmConsumer(config.host("10.187.5.157:14002").username("user"));
				consumer30=EmaFactory.createOmmConsumer(config.host("10.187.5.157:14002").username("user"));
				

						
				consumer1.registerClient(reqMsg, appClient);
				consumer2.registerClient(reqMsg, appClient);
				consumer3.registerClient(reqMsg, appClient);
				consumer4.registerClient(reqMsg, appClient);
				consumer5.registerClient(reqMsg, appClient);
				consumer6.registerClient(reqMsg, appClient);
				consumer7.registerClient(reqMsg, appClient);
				consumer8.registerClient(reqMsg, appClient);
				consumer9.registerClient(reqMsg, appClient);
				consumer10.registerClient(reqMsg, appClient);
				consumer11.registerClient(reqMsg, appClient);
				consumer12.registerClient(reqMsg, appClient);
				consumer13.registerClient(reqMsg, appClient);
				consumer14.registerClient(reqMsg, appClient);
				consumer15.registerClient(reqMsg, appClient);
				consumer16.registerClient(reqMsg, appClient);
				consumer17.registerClient(reqMsg, appClient);
				consumer18.registerClient(reqMsg, appClient);
				consumer19.registerClient(reqMsg, appClient);
				consumer20.registerClient(reqMsg, appClient);
				consumer21.registerClient(reqMsg, appClient);
				consumer22.registerClient(reqMsg, appClient);
				consumer23.registerClient(reqMsg, appClient);
				consumer24.registerClient(reqMsg, appClient);
				consumer25.registerClient(reqMsg, appClient);
				consumer26.registerClient(reqMsg, appClient);
				consumer27.registerClient(reqMsg, appClient);
				consumer28.registerClient(reqMsg, appClient);
				consumer29.registerClient(reqMsg, appClient);
				consumer30.registerClient(reqMsg, appClient);
				/*consumer.registerClient(reqMsg.domainType(EmaRdm.MMT_DICTIONARY).name("RWFFld").
						 filter(EmaRdm.DICTIONARY_NORMAL), appClient);

				consumer.registerClient(reqMsg.clear().domainType(EmaRdm.MMT_DICTIONARY).name("RWFEnum").
						filter(EmaRdm.DICTIONARY_NORMAL),appClient);*/

				//consumer.registerClient(reqMsg.clear().serviceName("DIRECT_FEED").name("IBM.N"), appClient);
				//Thread.sleep(2000);
				/*OmmConsumerConfig config = config.host("10.187.5.157:14002").username("user");
				
				consumer2 = EmaFactory.createOmmConsumer(config);
				
				consumer2.registerClient(reqMsg.clear().serviceName("DIRECT_FEED").name("IBM.N"), appClient);
				
				*/
				Thread.sleep(30000); // API calls onRefreshMsg(), onUpdateMsg() and
										// onStatusMsg()
			} catch (InterruptedException | OmmException excp)
			{
				System.out.println(excp.getMessage());
			}
			finally 
			{
				//if (consumer1 != null) consumer1.uninitialize();
				try {	
					Consumer.dumpHeap("beforeUnitialize.hprof", true);
					
					if (consumer1 != null) consumer1.uninitialize();
					if (consumer2 != null) consumer2.uninitialize();
					if (consumer3 != null) consumer3.uninitialize();
					if (consumer4 != null) consumer4.uninitialize();
					if (consumer5 != null) consumer5.uninitialize();
					if (consumer6 != null) consumer6.uninitialize();
					if (consumer7 != null) consumer7.uninitialize();
					if (consumer8 != null) consumer8.uninitialize();
					if (consumer9 != null) consumer9.uninitialize();
					if (consumer10 != null) consumer10.uninitialize();
					if (consumer11 != null) consumer11.uninitialize();
					if (consumer12 != null) consumer12.uninitialize();
					if (consumer13 != null) consumer13.uninitialize();
					if (consumer14 != null) consumer14.uninitialize();
					if (consumer15 != null) consumer15.uninitialize();
					if (consumer16 != null) consumer16.uninitialize();
					if (consumer17 != null) consumer17.uninitialize();
					if (consumer18 != null) consumer18.uninitialize();
					if (consumer19 != null) consumer19.uninitialize();
					if (consumer20 != null) consumer20.uninitialize();
					if (consumer21 != null) consumer21.uninitialize();
					if (consumer22 != null) consumer22.uninitialize();
					if (consumer23 != null) consumer23.uninitialize();
					if (consumer24 != null) consumer24.uninitialize();
					if (consumer25 != null) consumer25.uninitialize();
					if (consumer26 != null) consumer26.uninitialize();
					if (consumer27 != null) consumer27.uninitialize();
					if (consumer28 != null) consumer28.uninitialize();
					if (consumer29 != null) consumer29.uninitialize();
					if (consumer30 != null) consumer30.uninitialize();
					Consumer.dumpHeap("afterUnitialize.hprof", true);
					consumer1 = null;
					consumer2 = null;
					consumer3 = null;
					consumer4 = null;
					consumer5 = null;
					consumer6 = null;
					consumer7 = null;
					consumer8 = null;
					consumer9 = null;
					consumer10 = null;
					consumer11 = null;
					consumer12 = null;
					consumer13 = null;
					consumer14 = null;
					consumer15 = null;
					consumer16 = null;
					consumer17 = null;
					consumer18 = null;
					consumer19 = null;
					consumer20 = null;
					consumer21 = null;
					consumer22 = null;
					consumer23 = null;
					consumer24 = null;
					consumer25 = null;
					consumer26 = null;
					consumer27 = null;
					consumer28 = null;
					consumer29 = null;
					consumer30 = null;
					
				    
					System.gc ();
					Runtime.getRuntime().gc();
					System.runFinalization ();
					Consumer.dumpHeap("afterGC.hprof", true);
					//Consumer.triggerFullGC();
					//Thread.sleep(60*1000);
					//Consumer.dumpHeap("C:\\Users\\U8007607\\OneDrive - London Stock Exchange Group\\Case\\12263752\\test\\afterTriggerFullGC.hprof", true);
					
					//Thread.sleep(60*1000);
			//		BufferedReader reader = new BufferedReader(new InputStreamReader(System.in));
					//System.out.println("Garbage Collection?");
				        // Reading data using readLine
				     //String name = reader.readLine();
				
					
					//Thread.sleep(60*1000);
				
				}catch(Exception e) {
					e.printStackTrace();
				}
			}
		}
	}
