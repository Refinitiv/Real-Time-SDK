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

public class AppClient : IOmmConsumerClient
{
    public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmConsumerEvent consumerEvent)
    {
        Console.WriteLine($"Handle: {consumerEvent.Handle} Closure: {consumerEvent.Closure}");
        Console.WriteLine(refreshMsg);
    }

    public void OnUpdateMsg(UpdateMsg updateMsg, IOmmConsumerEvent consumerEvent)
    {
        Console.WriteLine($"Handle: {consumerEvent.Handle} Closure: {consumerEvent.Closure}");
        Console.WriteLine(updateMsg);
    }

    public void OnStatusMsg(StatusMsg statusMsg, IOmmConsumerEvent consumerEvent)
    {
        Console.WriteLine($"Handle: {consumerEvent.Handle} Closure: {consumerEvent.Closure}");
        Console.WriteLine(statusMsg);
    }
}

public class Consumer
{
    static void Main()
    {
        OmmConsumer? consumer = null;
        try
        {
            consumer = new OmmConsumer(new OmmConsumerConfig().ConsumerName("Consumer_2"));
            consumer.RegisterClient(new RequestMsg().ServiceName("DIRECT_FEED").Name("LSEG.L"), new AppClient());
            Thread.Sleep(60000); // API calls OnRefreshMsg(), OnUpdateMsg() and OnStatusMsg()
        }
        catch(OmmException excp)
        {
            Console.WriteLine(excp.Message);
        }
        finally
        {
            consumer?.Uninitialize();
        }
    }
}
