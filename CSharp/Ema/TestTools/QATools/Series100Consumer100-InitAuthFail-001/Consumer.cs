/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.Md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Access;
using System;
using System.Threading;

namespace LSEG.Ema.Example.Traning.Consumer;

internal class AppErrorClient : IOmmConsumerErrorClient
{
    public void OnInvalidHandle(long handle, string text) =>
        Console.WriteLine("onInvalidHandle: " + handle + " text: " + text);

    public void OnInvalidUsage(string text, int errorCode) =>
        Console.WriteLine("onInvalidUsage text: " + text + " errorCode: " + errorCode);
}

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
        AppErrorClient appErrorClient = new();
        try
        {
            AppClient appClient = new();
            consumer = new (new OmmConsumerConfig().Host("localhost:14002").UserName("user"), appErrorClient);
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
