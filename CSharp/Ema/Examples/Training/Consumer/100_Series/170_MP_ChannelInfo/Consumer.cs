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
    private bool updateCalled = false;

    public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmConsumerEvent @event)
    {
        Console.WriteLine(refreshMsg + "\nevent channel info (refresh)\n" + @event.ChannelInformation());
    }

    public void OnUpdateMsg(UpdateMsg updateMsg, IOmmConsumerEvent @event)
    {
        if (!updateCalled)
        {
            updateCalled = true;
            Console.WriteLine(updateMsg + "\nevent channel info (update)\n" + @event.ChannelInformation());
        }
        else
            Console.WriteLine("skipped printing updateMsg");
    }

    public void OnStatusMsg(StatusMsg statusMsg, IOmmConsumerEvent @event)
    {
        Console.WriteLine(statusMsg + "\nevent channel info (status)\n" + @event.ChannelInformation());
    }
}

public class Consumer
{
    public static void Main()
    {
        OmmConsumer? consumer = null;
        try
        {
            AppClient appClient = new();
            ChannelInformation ci = new();

            consumer = new OmmConsumer(new OmmConsumerConfig("EmaConfig.xml").UserName("user"));
            consumer.ChannelInformation(ci);
            Console.WriteLine("channel information (consumer):\n\t" + ci);

            consumer.RegisterClient(new RequestMsg().ServiceName("DIRECT_FEED").Name("IBM.N"), appClient, 0);

            Thread.Sleep(60000);            // API calls OnRefreshMsg(), OnUpdateMsg() and OnStatusMsg()
        }
        catch (OmmException excp)
        {
            Console.WriteLine(excp.Message);
        }
        finally
        {
            consumer?.Uninitialize();
        }
    }
}