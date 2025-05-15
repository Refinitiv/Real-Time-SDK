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
using System.IO;
using System.Threading;
using LSEG.Ema.Access;
using LSEG.Ema.Rdm;

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
    //API QA
    static int _TEST = 0;
    static void PrintHelp()
    {
        Console.WriteLine("\nOptions:\n" + "  -?\tShows this usage\n"
                + "  -test0 Testing default, no qos in ReqMsg.\n"
                + "  -test11 Testing ReqMsg.qos(Timeliness.REALTIME, Rate.TICK_BY_TICK).\n"
                + "  -test12 Testing ReqMsg.qos(Timeliness.BEST_DELAYED_TIMELINESS, Rate.TICK_BY_TICK).\n"
                + "  -test21 Testing ReqMsg.qos(qos(Timeliness.REALTIME, Rate.JIT_CONFLATED).\n"
                + "\n");
    }

    static bool ReadCommandlineArgs(string[] args)
    {
        try
        {
            int argsCount = 0;
            bool test0 = false;
            bool test11 = false;
            bool test12 = false;
            bool test21 = false;

            while (argsCount < args.Length)
            {
                if (0 == args[argsCount].CompareTo("-?"))
                {
                    PrintHelp();
                    return false;
                }
                else if ("-test0".Equals(args[argsCount]))
                {
                    test0 = true;
                    if (test0)
                        Consumer._TEST = 0;
                    ++argsCount;
                }
                else if ("-test11".Equals(args[argsCount]))
                {
                    test11 = true;
                    if (test11)
                        Consumer._TEST = 11;
                    ++argsCount;
                }
                else if ("-test12".Equals(args[argsCount]))
                {
                    test12 = true;
                    if (test12)
                        Consumer._TEST = 12;
                    ++argsCount;
                }
                else if ("-test21".Equals(args[argsCount]))
                {
                    test21 = true;
                    if (test21)
                        Consumer._TEST = 21;
                    ++argsCount;
                }
                else // unrecognized command line argument
                {
                    PrintHelp();
                    return false;
                }
            }
        }
        catch (Exception)
        {
            PrintHelp();
            return false;
        }

        return true;
    }
    //END API QA
    static void Main(string[] args)
    {
        OmmConsumer? consumer = null;
        try
        {
            //API QA
            if (!ReadCommandlineArgs(args))
                return;
            //END API QA
            AppClient appClient = new();

            /* Create a service list which can subscribe data using any concrete services in this list */
            ServiceList serviceList = new("SVG1");

            serviceList.ConcreteServiceList.Add("DIRECT_FEED");
            serviceList.ConcreteServiceList.Add("DIRECT_FEED_2");

            consumer = new OmmConsumer(new OmmConsumerConfig().ConsumerName("Consumer_10").AddServiceList(serviceList), appClient);

            //API QA
            OmmArray array1 = new()
            {
                FixedWidth = 2
            };

            array1.AddInt(2)
                .AddInt(22)
                .AddInt(25)
                .Complete();

            var view1 = new ElementList()
                .AddUInt(EmaRdm.ENAME_VIEW_TYPE, 1)
                .AddArray(EmaRdm.ENAME_VIEW_DATA, array1)
                .Complete();

            OmmArray array2 = new()
            {
                FixedWidth = 2
            };

            array2.AddInt(2)
                .AddInt(30)
                .AddInt(31)
                .Complete();

            var view2 = new ElementList()
                .AddUInt(EmaRdm.ENAME_VIEW_TYPE, 1)
                .AddArray(EmaRdm.ENAME_VIEW_DATA, array2)
                .Complete();


            switch (Consumer._TEST)
            {
                default:
                case 0:
                    Console.WriteLine("***APIQA TEST 0 : ReqMsg does NOT set qos & view1(2,22,25) view2(2,30,31).***");
                    consumer.RegisterClient(new RequestMsg().ServiceListName("SVG1").Name("LSEG.L").Payload(view1), appClient, 1);
                    consumer.RegisterClient(new RequestMsg().ServiceListName("SVG1").Name("LSEG.L").Payload(view2), appClient, 2);
                    break;
                case 11:
                    Console.WriteLine("***APIQA TEST 11 : ReqMsg.qos(Timeliness.REALTIME, Rate.TICK_BY_TICK) & view1(2,22,25) view2(2,30,31).***");
                    consumer.RegisterClient(new RequestMsg().ServiceListName("SVG1").Name("LSEG.L").Payload(view1).Qos(OmmQos.Timelinesses.REALTIME, OmmQos.Rates.TICK_BY_TICK), appClient, 1);
                    consumer.RegisterClient(new RequestMsg().ServiceListName("SVG1").Name("LSEG.L").Payload(view2).Qos(OmmQos.Timelinesses.REALTIME, OmmQos.Rates.TICK_BY_TICK), appClient, 2);
                    break;
                case 12:
                    Console.WriteLine("***APIQA TEST 12 : ReqMsg.qos(Timeliness.BEST_DELAYED_TIMELINESS, Rate.TICK_BY_TICK) & view1(2,22,25) view2(2,30,31).***");
                    consumer.RegisterClient(new RequestMsg().ServiceListName("SVG1").Name("LSEG.L").Payload(view1).Qos(OmmQos.Timelinesses.INEXACT_DELAYED, OmmQos.Rates.TICK_BY_TICK), appClient, 1);
                    consumer.RegisterClient(new RequestMsg().ServiceListName("SVG1").Name("LSEG.L").Payload(view2).Qos(OmmQos.Timelinesses.INEXACT_DELAYED, OmmQos.Rates.TICK_BY_TICK), appClient, 2);
                    break;
                case 21:
                    Console.WriteLine("***APIQA TEST 21 : ReqMsg.qos(Timeliness.REALTIME, Rate.JIT_CONFLATED) & view1(2,22,25) view2(2,30,31).***");
                    consumer.RegisterClient(new RequestMsg().ServiceListName("SVG1").Name("LSEG.L").Payload(view1).Qos(OmmQos.Timelinesses.REALTIME, OmmQos.Rates.JUST_IN_TIME_CONFLATED), appClient, 1);
                    consumer.RegisterClient(new RequestMsg().ServiceListName("SVG1").Name("LSEG.L").Payload(view2).Qos(OmmQos.Timelinesses.REALTIME, OmmQos.Rates.JUST_IN_TIME_CONFLATED), appClient, 2);
                    break;

            }
            //END API QA
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