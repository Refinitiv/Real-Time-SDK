/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Ema.Example.Traning.Consumer;

using LSEG.Ema.Access;
using System;
using System.Linq;
using System.Threading;

internal class AppClient : IOmmConsumerClient
{
    public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmConsumerEvent consumerEvent)
    {
        Console.WriteLine(refreshMsg);
    }

    public void OnUpdateMsg(UpdateMsg updateMsg, IOmmConsumerEvent consumerEvent)
    {
        Console.WriteLine(updateMsg);
    }

    public void OnStatusMsg(StatusMsg statusMsg, IOmmConsumerEvent consumerEvent)
    {
        Console.WriteLine(statusMsg);
    }
}

public class Consumer
{
    public static void Main(string[] args)
    {
        var configFile = args.FirstOrDefault(string.Empty);
        OmmConsumer? consumer = null;
        try
        {
            AppClient appClient = new();
            consumer = new(new OmmConsumerConfig(configFile).ConsumerName("Consumer_2"));
            consumer.RegisterClient(new RequestMsg().ServiceName("DIRECT_FEED").Name("IBM.N"), appClient, 0);
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