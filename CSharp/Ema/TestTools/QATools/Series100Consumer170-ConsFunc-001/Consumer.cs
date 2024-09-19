/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.Md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Ema.Example.Traning.Consumer;

using LSEG.Ema.Access;
using LSEG.Ema.Rdm;
using LSEG.Eta.Codec;
using System;
using System.Threading;
using static LSEG.Ema.Access.OmmConsumerConfig;
using DateTime = System.DateTime;

//APIQA this file is QATools standalone. See qa_readme.txt for details about this tool.
internal class AppClient : IOmmConsumerClient
{
	public static bool USERDISPATCH = false;
    public static bool TESTCHANNELINFOWITHLOGINHANDLE = false;
    public static bool TESTCHANNELINFOVALUE = false;
    public bool updateCalled = false;

	public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmConsumerEvent @event)
	{
		Console.WriteLine( refreshMsg + "\nevent channel info (refresh)\n" +@event.ChannelInformation());
	}

	public void OnUpdateMsg(UpdateMsg updateMsg, IOmmConsumerEvent @event) 
	{
		if (!updateCalled)
        {
            updateCalled = true;
            Console.WriteLine( updateMsg + "\nevent channel info (update)\n" +@event.ChannelInformation());
        }
        else
             Console.WriteLine( "skipped printing updateMsg" );

	}
	public void OnStatusMsg(StatusMsg statusMsg, IOmmConsumerEvent @event) 
	{
		Console.WriteLine( statusMsg + "\nevent channel info (status)\n" +@event.ChannelInformation());
	}
}

public class Consumer 
{
	public static void PrintHelp(bool reflect)
	{
		if (!reflect)
		{
			Console.WriteLine("\nOptions:\n" + "  -?\tShows this usage\n\n" + "  -userDispatch \tUse UserDispatch Operation Model [default = false]\n"
					+ "  -testChannelInfoWithLoginHandle \tSet for testing ChannelInformation and register Login client [default = false]\n" 
					+ "  -testChannelInfoValue \tSet for testing getting each attribute's value of ChannelInformation [default = false]\n"+ "\n");

			Environment.Exit(-1);
		}
		else
		{
			Console.WriteLine("\n  Options will be used:\n" + "  -userDispatch \t " + AppClient.USERDISPATCH + "\n" 
			+ "  -testChannelInfoWithLoginHandle \t " + AppClient.TESTCHANNELINFOWITHLOGINHANDLE + "\n" 
			+ "  -testChannelInfValue \t " + AppClient.TESTCHANNELINFOVALUE + "\n"+ "\n");
		}
	}
	public static bool ReadCommandlineArgs(String[] argv)
	{
		int count = argv.Length;
		int idx = 0;

		while (idx < count)
		{
			if (0 == argv[idx].CompareTo("-?"))
			{
				PrintHelp(false);
				return false;
			}
			else if (argv[idx].Equals("-userDispatch",StringComparison.InvariantCultureIgnoreCase))
			{
				if (++idx >= count)
				{
					PrintHelp(false);
					return false;
				}
				AppClient.USERDISPATCH = ((argv[idx].Equals("TRUE")) ? true : false);
				++idx;
			}
			else if (argv[idx].Equals("-testChannelInfoWithLoginHandle", StringComparison.InvariantCultureIgnoreCase))
			{
				if (++idx >= count)
				{
					PrintHelp(false);
					return false;
				}
				AppClient.TESTCHANNELINFOWITHLOGINHANDLE = argv[idx].Equals("TRUE", StringComparison.InvariantCultureIgnoreCase) ? true : false;
				++idx;
			}
		    else if (argv[idx].Equals("-testChannelInfoValue", StringComparison.InvariantCultureIgnoreCase))
			{
				if (++idx >= count)
				{
					PrintHelp(false);
					return false;
				}
				AppClient.TESTCHANNELINFOVALUE = (argv[idx].Equals("TRUE", StringComparison.InvariantCultureIgnoreCase) ? true : false);
				++idx;
			}
			else
			{
				PrintHelp(false);
				return false;
			}
		}

		PrintHelp(true);
		return true;
	}

	public static void Main(string[] args)
	{
		OmmConsumer? consumer = null;
		AppClient appClient = new();		
		try
		{
			ChannelInformation ci = new();
			//API QA
			if (!ReadCommandlineArgs(args))
				return;	
			OmmConsumerConfig config = new OmmConsumerConfig();
			//END API QA          
			if (AppClient.TESTCHANNELINFOWITHLOGINHANDLE)
			{
				consumer  = new OmmConsumer(config.UserName("user"));
			    Console.WriteLine("API QA Test with consumer  = new OmmConsumer(config.UserName(\"user\")");
			} else {
				consumer  = new OmmConsumer(config.UserName("user"), appClient);
			    Console.WriteLine("API QA Test with consumer  = new OmmConsumer(config.UserName(\"user\", appClient)");
			}
			//API QA
			if (AppClient.USERDISPATCH)
				config.OperationModel(OperationModelMode.USER_DISPATCH);
            if (AppClient.TESTCHANNELINFOWITHLOGINHANDLE)
			    consumer.RegisterClient(new RequestMsg().DomainType(EmaRdm.MMT_LOGIN), appClient);
			//END API QA
			consumer.ChannelInformation(ci);
			Console.WriteLine( "channel information (consumer):\n\t" + ci );
			
			consumer.RegisterClient( new RequestMsg().ServiceName("DIRECT_FEED").Name("IBM.N"), appClient, 0);
			if (AppClient.TESTCHANNELINFOVALUE)
				Console.WriteLine(" \n\tAPI QA get each ChannelInformatin attribute.\n\thostname: " + ci.Hostname + "\n\tip address: " + ci.IpAddress +
					"\n\tcomponent information: " + ci.ComponentInfo +
					"\n\tconnection type: " + System.Enum.GetName(ci.ConnectionType) +
					"\n\tchannel state: " + ci.ChannelState +
					"\n\tprotocol type: " + (ci.ProtocolType == Codec.RWF_PROTOCOL_TYPE ? "Rssl wire format" : "unknown wire format") +
					"\n\tmajor version: " + ci.MajorVersion + "\n\tminor version: " + ci.MinorVersion + "\n\tping timeout: " + ci.PingTimeout);

			if (AppClient.USERDISPATCH)
			{
				DateTime startTime = DateTime.Now;
				TimeSpan duration = TimeSpan.FromMilliseconds(60000);
                while (DateTime.Now > startTime + duration)
					consumer.Dispatch(10);

			} else {
				Thread.Sleep(60000);			// API calls OnRefreshMsg(), OnUpdateMsg() and OnStatusMsg()
			}
		}
        catch (ThreadInterruptedException threadInterruptedException)
        {
            Console.WriteLine(threadInterruptedException.Message);
        }
        catch (OmmException ommException)
        {
            Console.WriteLine(ommException.Message);
        }
        finally 
		{
			consumer?.Uninitialize();
		}
	}
}
