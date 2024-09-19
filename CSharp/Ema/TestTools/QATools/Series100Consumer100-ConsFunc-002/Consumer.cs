/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.Md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Ema.Example.Traning.Consumer;

using LSEG.Ema.Access;
using System;
using System.Threading;

internal class AppClient : IOmmConsumerClient
{
    public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmConsumerEvent consumerEvent) => 
        Console.WriteLine(refreshMsg);

    public void OnUpdateMsg(UpdateMsg updateMsg, IOmmConsumerEvent consumerEvent) => 
        Console.WriteLine(updateMsg);

    public void OnStatusMsg(StatusMsg statusMsg, IOmmConsumerEvent consumerEvent) => 
        Console.WriteLine(statusMsg);
}

public class Consumer
{
    public static void Main()
    {
        OmmConsumer? consumer = null;
        try
        {
            //APIQA
            for (int i = 0; i < 100000; i++)
            {
                AppClient appClient = new();
                OmmConsumerConfig config = new();
                consumer = new OmmConsumer(new OmmConsumerConfig().ConsumerName("Consumer_1"));
                RequestMsg reqMsg = new();
                consumer.RegisterClient(reqMsg.ServiceName("DIRECT_FEED").Name("IBM.N"), appClient);
                Thread.Sleep(1000);         // API calls OnRefreshMsg(), OnUpdateMsg() and OnStatusMsg()
                consumer.Uninitialize();
            }
            //END APIQA
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