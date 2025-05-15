﻿/*|-----------------------------------------------------------------------------
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
    static int _domainType = 6;
    static String _itemName = "LSEG.L";

    static void PrintHelp()
    {
        Console.WriteLine("\nOptions:\n" + "  -?\tShows this usage\n"
                + "  -i to identify itemName.\n"
                + "  -domain follow by domainType mp=MARKET_PRICE, mbp=MARKET_BY_PRICE, mbo=MARKET_BY_ORDER.\n"
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
            String domainName = "mp";

            while (argsCount < args.Length)
            {
                if (0 == args[argsCount].CompareTo("-?"))
                {
                    PrintHelp();
                    return false;
                }
                else if ("-i".Equals(args[argsCount]))
                {
                    Consumer._itemName = argsCount < (args.Length - 1) ? args[++argsCount] : null;
                    ++argsCount;
                }

                else if ("-domain".Equals(args[argsCount]))
                {
                    domainName = argsCount < (args.Length - 1) ? args[++argsCount] : null;
                    if (domainName.Equals("mp"))
                        Consumer._domainType = EmaRdm.MMT_MARKET_PRICE;
                    else if (domainName.Equals("mbp"))
                        Consumer._domainType = EmaRdm.MMT_MARKET_BY_PRICE;
                    else if (domainName.Equals("mbo"))
                        Consumer._domainType = EmaRdm.MMT_MARKET_BY_ORDER;
                    else
                        Console.WriteLine("Incorect domain type, turn to default mp.");
                    ++argsCount;
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
            switch (Consumer._TEST)
            {
                default:
                case 0:
                    Console.WriteLine("***APIQA TEST 0 : ReqMsg does NOT set qos and same itemName, one is normal, one is private stream.***");
                    consumer.RegisterClient(new RequestMsg().DomainType(Consumer._domainType).ServiceListName("SVG1").Name(Consumer._itemName).PrivateStream(true), appClient);
                    consumer.RegisterClient(new RequestMsg().DomainType(Consumer._domainType).ServiceListName("SVG1").Name(Consumer._itemName), appClient);
                    break;
                case 11:
                    Console.WriteLine("***APIQA TEST 11 : ReqMsg.qos(Timeliness.REALTIME, Rate.TICK_BY_TICK), one is normal, one is private stream.***");
                    consumer.RegisterClient(new RequestMsg().DomainType(Consumer._domainType).ServiceListName("SVG1").Name(Consumer._itemName).Qos(OmmQos.Timelinesses.REALTIME, OmmQos.Rates.TICK_BY_TICK).PrivateStream(true), appClient);
                    consumer.RegisterClient(new RequestMsg().DomainType(Consumer._domainType).ServiceListName("SVG1").Name(Consumer._itemName).Qos(OmmQos.Timelinesses.REALTIME, OmmQos.Rates.TICK_BY_TICK), appClient);
                    break;
                case 12:
                    Console.WriteLine("***APIQA TEST 12 : ReqMsg.qos(Timeliness.BEST_DELAYED_TIMELINESS, Rate.TICK_BY_TICK), one is normal, one is private stream.***");
                    consumer.RegisterClient(new RequestMsg().DomainType(Consumer._domainType).ServiceListName("SVG1").Name(Consumer._itemName).Qos(OmmQos.Timelinesses.INEXACT_DELAYED, OmmQos.Rates.TICK_BY_TICK).PrivateStream(true), appClient);
                    consumer.RegisterClient(new RequestMsg().DomainType(Consumer._domainType).ServiceListName("SVG1").Name(Consumer._itemName).Qos(OmmQos.Timelinesses.INEXACT_DELAYED, OmmQos.Rates.TICK_BY_TICK), appClient);
                    break;
                case 21:
                    Console.WriteLine("***APIQA TEST 21 : ReqMsg.qos(Timeliness.REALTIME, Rate.JIT_CONFLATED), one is normal, one is private stream.***");
                    consumer.RegisterClient(new RequestMsg().DomainType(Consumer._domainType).ServiceListName("SVG1").Name(Consumer._itemName).Qos(OmmQos.Timelinesses.REALTIME, OmmQos.Rates.JUST_IN_TIME_CONFLATED).PrivateStream(true), appClient);
                    consumer.RegisterClient(new RequestMsg().DomainType(Consumer._domainType).ServiceListName("SVG1").Name(Consumer._itemName).Qos(OmmQos.Timelinesses.REALTIME, OmmQos.Rates.JUST_IN_TIME_CONFLATED), appClient);
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