///*|----------------------------------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      	--
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  					--
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            		--
///*|----------------------------------------------------------------------------------------------------

package com.rtsdk.ema.examples.training.consumer.series100.ex170_MP_ChannelInfo;

import com.rtsdk.ema.access.Msg;

import com.rtsdk.ema.access.AckMsg;
import com.rtsdk.ema.access.GenericMsg;
import com.rtsdk.ema.access.RefreshMsg;
import com.rtsdk.ema.access.StatusMsg;
import com.rtsdk.ema.access.UpdateMsg;
import com.rtsdk.eta.codec.Codec;
import com.rtsdk.eta.transport.ConnectionTypes;
import com.rtsdk.ema.access.EmaFactory;
import com.rtsdk.ema.access.OmmConsumer;
import com.rtsdk.ema.access.OmmConsumerClient;
import com.rtsdk.ema.access.OmmConsumerEvent;
import com.rtsdk.ema.access.OmmException;
import com.rtsdk.ema.access.ChannelInformation;
import com.rtsdk.ema.access.OmmConsumerConfig;
import com.rtsdk.ema.rdm.EmaRdm;
import com.rtsdk.ema.access.OmmConsumerConfig.OperationModel;

class AppClient implements OmmConsumerClient
{
	static boolean USERDISPATCH = false;
	static boolean TESTCHANNELINFOWITHLOGINHANDLE = false;
	static boolean TESTCHANNELINFOVALUE = false;
	static boolean updateCalled = false;

	public void onRefreshMsg(RefreshMsg refreshMsg, OmmConsumerEvent event)
	{
		System.out.println( refreshMsg + "\nevent channel info (refresh)\n" + event.channelInformation());
	}

	public void onUpdateMsg(UpdateMsg updateMsg, OmmConsumerEvent event) 
	{
		if (!updateCalled)
        {
            updateCalled = true;
            System.out.println( updateMsg + "\nevent channel info (update)\n" + event.channelInformation());
        }
        else
             System.out.println( "skipped printing updateMsg" );

	}
	public void onStatusMsg(StatusMsg statusMsg, OmmConsumerEvent event) 
	{
		System.out.println( statusMsg + "\nevent channel info (status)\n" + event.channelInformation());
	}

	public void onGenericMsg(GenericMsg genericMsg, OmmConsumerEvent consumerEvent){}
	public void onAckMsg(AckMsg ackMsg, OmmConsumerEvent consumerEvent){}
	public void onAllMsg(Msg msg, OmmConsumerEvent consumerEvent){}
}

public class Consumer 
{
	public static void printHelp(boolean reflect)
	{
		if (!reflect)
		{
			System.out.println("\nOptions:\n" + "  -?\tShows this usage\n\n" + "  -userDispatch \tUse UserDispatch Operation Model [default = false]\n"
					+ "  -testChannelInfoWithLoginHandle \tSet for testing ChannelInformation and register Login client [default = false]\n" 
					+ "  -testChannelInfoValue \tSet for testing getting each attribute's value of ChannelInformation [default = false]\n"+ "\n");

			System.exit(-1);
		}
		else
		{
			System.out.println("\n  Options will be used:\n" + "  -userDispatch \t " + AppClient.USERDISPATCH + "\n" 
			+ "  -testChannelInfoWithLoginHandle \t " + AppClient.TESTCHANNELINFOWITHLOGINHANDLE + "\n" 
			+ "  -testChannelInfValue \t " + AppClient.TESTCHANNELINFOVALUE + "\n"+ "\n");
		}
	}
	public static boolean readCommandlineArgs(String[] argv)
	{
		int count = argv.length;
		int idx = 0;

		while (idx < count)
		{
			if (0 == argv[idx].compareTo("-?"))
			{
				printHelp(false);
				return false;
			}
			else if (0 == argv[idx].compareToIgnoreCase("-userDispatch"))
			{
				if (++idx >= count)
				{
					printHelp(false);
					return false;
				}
				AppClient.USERDISPATCH = ((argv[idx].compareToIgnoreCase("TRUE") == 0) ? true : false);
				++idx;
			}
			else if (0 == argv[idx].compareToIgnoreCase("-testChannelInfoWithLoginHandle"))
			{
				if (++idx >= count)
				{
					printHelp(false);
					return false;
				}
				AppClient.TESTCHANNELINFOWITHLOGINHANDLE = ((argv[idx].compareToIgnoreCase("TRUE") == 0) ? true : false);
				++idx;
			}
		    else if (0 == argv[idx].compareToIgnoreCase("-testChannelInfoValue"))
			{
				if (++idx >= count)
				{
					printHelp(false);
					return false;
				}
				AppClient.TESTCHANNELINFOVALUE = ((argv[idx].compareToIgnoreCase("TRUE") == 0) ? true : false);
				++idx;
			}
			else
			{
				printHelp(false);
				return false;
			}
		}

		printHelp(true);
		return true;
	}

	public static void main(String[] args)
	{
		OmmConsumer consumer = null;
		AppClient appClient = new AppClient();		
		try
		{
			ChannelInformation ci = EmaFactory.createChannelInformation();
			//API QA
			if (!readCommandlineArgs(args))
				return;	
			OmmConsumerConfig config = EmaFactory.createOmmConsumerConfig();
			//END API QA          
			if (appClient.TESTCHANNELINFOWITHLOGINHANDLE)
			{
				consumer  = EmaFactory.createOmmConsumer(config.username("user"));
			    System.out.println("API QA Test with consumer  = EmaFactory.createOmmConsumer(config.username(\"user\")");
			} else {
				consumer  = EmaFactory.createOmmConsumer(config.username("user"), appClient);
			    System.out.println("API QA Test with consumer  = EmaFactory.createOmmConsumer(config.username(\"user\", appClient)");
			}
			//API QA
			if (appClient.USERDISPATCH)
				config.operationModel(OperationModel.USER_DISPATCH);
            if (appClient.TESTCHANNELINFOWITHLOGINHANDLE)
			    consumer.registerClient(EmaFactory.createReqMsg().domainType(EmaRdm.MMT_LOGIN), appClient);
			//END API QA
			consumer.channelInformation(ci);
			System.out.println( "channel information (consumer):\n\t" + ci );
			
			consumer.registerClient( EmaFactory.createReqMsg().serviceName("DIRECT_FEED").name("IBM.N"), appClient, 0);
			if (appClient.TESTCHANNELINFOVALUE)
				System.out.println(" \n\tAPI QA get each ChannelInformatin attribute.\n\thostname: " + ci.hostname() + "\n\tip address: " + ci.ipAddress() +
					"\n\tcomponent information: " + ci.componentInformation() +
					"\n\tconnection type: " + ConnectionTypes.toString(ci.connectionType()) +
					"\n\tchannel state: " + ci.channelState() +
					"\n\tprotocol type: " + (ci.protocolType() == Codec.RWF_PROTOCOL_TYPE ? "Refinitiv wire format" : "unknown wire format") +
					"\n\tmajor version: " + ci.majorVersion() + "\n\tminor version: " + ci.minorVersion() + "\n\tping timeout: " + ci.pingTimeout());

			if (appClient.USERDISPATCH)
			{
				long startTime = System.currentTimeMillis();
				while (startTime + 60000 > System.currentTimeMillis())
					consumer.dispatch(10);
			} else {
				Thread.sleep(60000);			// API calls onRefreshMsg(), onUpdateMsg() and onStatusMsg()
			}
		}
		catch (InterruptedException | OmmException excp)
		{
			System.out.println(excp.getMessage());
		}
		finally 
		{
			if (consumer != null) consumer.uninitialize();
		}
	}
}
