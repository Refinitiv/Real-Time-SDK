/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.Md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Ema.Example.Traning.Consumer;

using System;
using System.Threading;

using LSEG.Ema.Access;

internal class AppClient : IOmmConsumerClient
{
	public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmConsumerEvent @event)
	{
		Console.WriteLine(refreshMsg);
	}
	
	public void OnUpdateMsg(UpdateMsg updateMsg, IOmmConsumerEvent @event) 
	{
		Console.WriteLine(updateMsg);
	}

	public void OnStatusMsg(StatusMsg statusMsg, IOmmConsumerEvent @event) 
	{
		Console.WriteLine(statusMsg);
	}
}

public class Consumer
{
	public static void Main()
	{
		OmmConsumer? consumer = null;
        try
		{
			consumer = new(new OmmConsumerConfig().Host("localhost:14002").UserName("user"));
            consumer.RegisterClient(new RequestMsg().ServiceName("DIRECT_FEED").Name("IBM.N").InterestAfterRefresh(false), new AppClient());
			Thread.Sleep(60000);// API calls OnRefreshMsg(), OnUpdateMsg() and OnStatusMsg()
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
