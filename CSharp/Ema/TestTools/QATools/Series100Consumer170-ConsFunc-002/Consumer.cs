/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.Md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Access;

namespace LSEG.Ema.Example.Traning.Consumer;

using LSEG.Ema.Access;
using System;
using System.Threading;
//APIQA this file is QATools standalone. See qa_readme.txt for details about this tool.
internal class AppClient : IOmmConsumerClient
{
	bool updateCalled = false;

    public void OnRefreshMsg( RefreshMsg refreshMsg, IOmmConsumerEvent @event )
	{
		Console.WriteLine( refreshMsg + "\nevent channel info (refresh)\n" +@event.ChannelInformation() );
	}

	public void OnUpdateMsg( UpdateMsg updateMsg, IOmmConsumerEvent @event ) 
	{
		if (!updateCalled)
		{
			updateCalled = true;
			Console.WriteLine( updateMsg + "\nevent channel info (update)\n" +@event.ChannelInformation() );
			//API QA
			Console.WriteLine("Test getMaxOutputBuffers() : " +@event.ChannelInformation()); 
			Console.WriteLine("Test getGuaranteedOutputBuffers() : " +@event.ChannelInformation().GuaranteedOutputBuffers);
			Console.WriteLine("Test getCompressionThreshold() : " +@event.ChannelInformation().CompressionThreshold);
			//END API QA
		}
		else
			Console.WriteLine( "skipped printing updateMsg" );			
	}

	public void OnStatusMsg( StatusMsg statusMsg, IOmmConsumerEvent @event ) 
	{
		Console.WriteLine( statusMsg + "\nevent channel info (status)\n" +@event.ChannelInformation() );
	}	
}
//API QA
class AppErrorClient : IOmmConsumerErrorClient
{
	public void OnInvalidHandle(long handle, String text)
	{
		Console.WriteLine("onInvalidHandle callback function" + "\nInvalid handle: " + handle + "\nError text: " + text); 
	}

	public void OnInvalidUsage(String text, int errorCode) {
		Console.WriteLine("onInvalidUsage callback function" + "\nError text: " + text +" , Error code: " + errorCode); 
	}
}
//END API QA
public class Consumer 
{
	//API QA
	static int maxOutputBuffers = 2000;
	static int guaranteedOutputBuffers = 2000;
	static int highWaterMark = 1000;
	static int serverNumPoolBuffers = 3000;
	static int compressionThreshold = 40;

	static void PrintHelp()
	{
	    Console.WriteLine("\nOptions:\n" + "  -?\tShows this usage\n"
	    		+ "  -maxOutputBuffers : value of maxOutputBuffer to modify.\r\n" 
	    		+ "  -guaranteedOutputBuffers : value of guaranteedOutputBuffers to modify.\n"
	    		+ "  -highWaterMark : value of highWaterMark to modify.\r\n" 
	    		+ "  -serverNumPoolBuffers : value of serverNumPoolBuffer to modify.\r\n" 
	    		+ "  -compressionThreshold : value of compressionThreshold to modify.\n"
	    		+ "\n");
	}
	static bool ReadCommandlineArgs(String[] args)
	{
	    try
	    {
	        int argsCount = 0;

	        while (argsCount < args.Length)
	        {
	            if (0 == args[argsCount].CompareTo("-?"))
	            {
	                PrintHelp();
	                return false;
	            }
	            else if ("-maxOutputBuffers".Equals(args[argsCount]))
				{
	            	maxOutputBuffers = argsCount < (args.Length-1) ? int.Parse(args[++argsCount]) : maxOutputBuffers;
					++argsCount;				
				}
	            else if ("-guaranteedOutputBuffers".Equals(args[argsCount]))
				{
	            	guaranteedOutputBuffers = argsCount < (args.Length-1) ? int.Parse(args[++argsCount]) : guaranteedOutputBuffers;
					++argsCount;				
				}
	            else if ("-highWaterMark".Equals(args[argsCount]))
				{
	            	highWaterMark = argsCount < (args.Length-1) ? int.Parse(args[++argsCount]) : highWaterMark;
					++argsCount;				
				}
				else if ("-serverNumPoolBuffers".Equals(args[argsCount]))
				{
					serverNumPoolBuffers = argsCount < (args.Length-1) ? int.Parse(args[++argsCount]) : serverNumPoolBuffers;
					++argsCount;		
				}	
				else if ("-compressionThreshold".Equals(args[argsCount]))
				{
					compressionThreshold = argsCount < (args.Length-1) ? int.Parse(args[++argsCount]) : compressionThreshold;
					++argsCount;					
				}
				else // unrecognized command line argument
				{
					PrintHelp();
					return false;
				}			
			}
	        
	    }
	    catch 
	    {
	    	PrintHelp();
	        return false;
	    }
		
		return true;
	}

	//END API QA
	public static void Main(string[] args)
	{
		OmmConsumer? consumer = null;
		try
		{
			AppClient appClient = new();
			if (!ReadCommandlineArgs(args))
                return;
			AppErrorClient appErrorClient = new AppErrorClient();
			ChannelInformation ci = new();

			consumer = new OmmConsumer(new OmmConsumerConfig( "EmaConfig.xml" ).UserName( "user" ),appErrorClient);
			consumer.ChannelInformation(ci);
			Console.WriteLine( "channel information (consumer):\n\t" + ci );
			consumer.RegisterClient( new RequestMsg().ServiceName( "DIRECT_FEED" ).Name( "IBM.N" ), appClient, 0);
			//API QA
			Console.WriteLine("Modify maxOutputBuffers to " + maxOutputBuffers );
			Console.WriteLine("Modify guaranteedOutputBuffers to " + guaranteedOutputBuffers );
			Console.WriteLine("Modify highWaterMark to " + highWaterMark );
			Console.WriteLine("Modify serverNumPoolBuffers to " + serverNumPoolBuffers );
			Console.WriteLine("Modify compressionThreshold to " + compressionThreshold );
			consumer.ModifyIOCtl(IOCtlCode.MAX_NUM_BUFFERS, maxOutputBuffers); // maxNumBuffer
			consumer.ModifyIOCtl(IOCtlCode.NUM_GUARANTEED_BUFFERS, guaranteedOutputBuffers); //guaranteedOutputBuffers
			consumer.ModifyIOCtl(IOCtlCode.HIGH_WATER_MARK, highWaterMark); //highWaterMark
			consumer.ModifyIOCtl((IOCtlCode)8, serverNumPoolBuffers); //serverNumPoolBuffers
			consumer.ModifyIOCtl(IOCtlCode.COMPRESSION_THRESHOLD, compressionThreshold); //compressionThreshold
			// END API QA

            Thread.Sleep( 60000 );			// API calls OnRefreshMsg(), OnUpdateMsg() and OnStatusMsg()
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
