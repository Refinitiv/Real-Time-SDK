/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.Md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Ema.Example.Traning.Consumer;

using System;
using System.Collections.Generic;
using System.Threading;
using LSEG.Ema.Access;

public class AppClient : IOmmConsumerClient
{
    List<ChannelInformation> channelInList = new();

    public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmConsumerEvent consumerEvent)
    {
        //API QA
        if (refreshMsg.DomainType() == 1)
        {
            Console.WriteLine($"{refreshMsg}\nevent session info (refresh)");
            PrintSessionInfo(consumerEvent);
        }
        else
        {
            Console.WriteLine($"{refreshMsg}\nevent channel info (refresh)\n{consumerEvent.ChannelInformation()}");
        }
    }

    public void OnUpdateMsg(UpdateMsg updateMsg, IOmmConsumerEvent consumerEvent)
    {
        //API QA
        if (updateMsg.DomainType() == 1)
        {
            Console.WriteLine($"{updateMsg}\nevent session info (update)");
            PrintSessionInfo(consumerEvent);
        }
        else
        {
            Console.WriteLine($"{updateMsg}\nevent channel info (update)\n{consumerEvent.ChannelInformation()}");
        }
    }

    public void OnStatusMsg(StatusMsg statusMsg, IOmmConsumerEvent consumerEvent)
    {
        //API QA
        if (statusMsg.DomainType() == 1)
        {
            Console.WriteLine($"{statusMsg}\nevent session info (status)");
            PrintSessionInfo(consumerEvent);
        }
        else
        {
            Console.WriteLine($"{statusMsg}\nevent channel info (status)\n{consumerEvent.ChannelInformation()}");
        }
    }

    void PrintSessionInfo(IOmmConsumerEvent consumerEvent)
    {

        consumerEvent.SessionChannelInfo(channelInList);
        
        foreach (ChannelInformation channelInfo in channelInList) 
        {
            Console.WriteLine(channelInfo);
        }
    }
}

public class Consumer
{
    static void Main()
    {
        OmmConsumer? consumer = null;
        try
        {
            AppClient appClient = new();

            /* Create a service list which can subscribe data using any concrete services in this list */
            ServiceList serviceList = new("SVG1");

            serviceList.ConcreteServiceList.Add("DIRECT_FEED");
            serviceList.ConcreteServiceList.Add("DIRECT_FEED_2");

            consumer = new OmmConsumer(new OmmConsumerConfig().ConsumerName("Consumer_10").AddServiceList(serviceList).UserName("user"), appClient);

            consumer.RegisterClient(new RequestMsg().ServiceListName("SVG1").Name("LSEG.L"), appClient);
            consumer.RegisterClient(new RequestMsg().ServiceListName("SVG1").Name("TRI.N"), appClient);
            Thread.Sleep(10000);
            Console.WriteLine("\nAPIQA calls consumer.Unitiailized()!!\n");
            consumer.Uninitialize();
            Thread.Sleep(60000);		// API calls onRefreshMsg(), onUpdateMsg() and onStatusMsg()
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