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
    static int _serviceID = 910;

    static void PrintHelp()
    {
        Console.WriteLine("\nOptions:\n" + "  -?\tShows this usage\n"
                + "  -sid service Id.\n"
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
            int serviceid = 910;

            while (argsCount < args.Length)
            {
                if (0 == args[argsCount].CompareTo("-?"))
                {
                    PrintHelp();
                    return false;
                }
                else if ("-sid".Equals(args[argsCount]))
                {
                    if (Int32.TryParse(args[++argsCount], out serviceid))
                    {
                        Consumer._serviceID = serviceid;
                    }
                    else
                    {
                        Console.WriteLine($"Error: failed to parse serviceid '{args[argsCount]}'");
                        return false;
                    }
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
            consumer = new OmmConsumer(new OmmConsumerConfig().ConsumerName("Consumer_10"), appClient);

            //API QA
            switch (Consumer._TEST)
            {
                default:
                case 0:
                    Console.WriteLine("***APIQA TEST 0 : Request with service id = " + Consumer._serviceID + ". & ReqMsg does NOT set qos.***");
                    consumer.RegisterClient(new RequestMsg().ServiceId(Consumer._serviceID).Name("LSEG.L"), appClient);
                    break;
                case 11:
                    Console.WriteLine("***APIQA TEST 11 : Request with service id = " + Consumer._serviceID + ". & ReqMsg.qos(Timeliness.REALTIME, Rate.TICK_BY_TICK).***");
                    consumer.RegisterClient(new RequestMsg().ServiceId(Consumer._serviceID).Name("LSEG.L").Qos(OmmQos.Timelinesses.REALTIME, OmmQos.Rates.TICK_BY_TICK), appClient);
                    break;
                case 12:
                    Console.WriteLine("***APIQA TEST 12 : Request with service id = " + Consumer._serviceID + ". & ReqMsg.qos(Timeliness.BEST_DELAYED_TIMELINESS, Rate.TICK_BY_TICK).***");
                    consumer.RegisterClient(new RequestMsg().ServiceId(Consumer._serviceID).Name("LSEG.L").Qos(OmmQos.Timelinesses.INEXACT_DELAYED, OmmQos.Rates.TICK_BY_TICK), appClient);
                    break;
                case 21:
                    Console.WriteLine("***APIQA TEST 21 : Request with service id = " + Consumer._serviceID + ". & ReqMsg.qos(Timeliness.REALTIME, Rate.JIT_CONFLATED).***");
                    consumer.RegisterClient(new RequestMsg().ServiceId(Consumer._serviceID).Name("LSEG.L").Qos(OmmQos.Timelinesses.REALTIME, OmmQos.Rates.JUST_IN_TIME_CONFLATED), appClient);
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