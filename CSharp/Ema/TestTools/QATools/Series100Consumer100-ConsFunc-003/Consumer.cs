/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.Md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Ema.Example.Traning.Consumer;

using LSEG.Ema.Access;
using System;
using System.Threading;

internal class AppClient : IOmmConsumerClient
{
    public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmConsumerEvent consumerEvent)
    {
        try
        {
            Console.WriteLine(refreshMsg);
            Thread.Sleep(2000);
        }
        catch (OmmException ommException)
        {
            Console.WriteLine(ommException.Message);
        }
    }

    public void OnUpdateMsg(UpdateMsg updateMsg, IOmmConsumerEvent consumerEvent)
    {
        try
        {
            Console.WriteLine(updateMsg);
            Thread.Sleep(2000);
        }
        catch (OmmException ommException)
        {
            Console.WriteLine(ommException.Message);
        }
    }

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
            AppClient appClient = new();
            consumer = new OmmConsumer(new OmmConsumerConfig().Host("localhost:14002").UserName("user"));
            RequestMsg reqMsg = new();
            consumer.RegisterClient(reqMsg.ServiceName("DIRECT_FEED").Name("IBM.N"), appClient);
            Thread.Sleep(60000);            // API calls OnRefreshMsg(), OnUpdateMsg() and OnStatusMsg()
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