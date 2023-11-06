/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.Md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Access;
using LSEG.Ema.Rdm;
using Microsoft.IdentityModel.Tokens;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading;

namespace LSEG.Ema.Example.Traning.Consumer;

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
    static readonly ThreadLocal<Random> random = new(() => new Random());
    public static int[] GetView()
    {
        int size = random.Value!.Next(1, 15);
        int[] a = new int[size];
        for (int k = 0; k < size; ++k)
        {
            int v = random.Value!.Next(10, 40);
            a[k] = v;
        }
        return a;
    }

    public static string[] GetElementView()
    { // vector of fid name
        string[] stringFids = {
            "BID",
            "ASK",
            "DSPLY_NAME",
            "RDN_EXCHD2",
            "TRDPRC_1",
            "TRDPRC_2",
            "TRDPRC_3",
            "TRDPRC_4",
            "TRDPRC_5",
            "HIGH_1",
            "LOW_1",
            "CURRENCY",
            "TRADE_DATE",
            "NEWS",
            "NEWS_TIME",
            "BIDSIZE",
            "ASKSIZE",
            "ACVOL_1",
            "BLKCOUNT",
            "BLKVOLUM",
            "OPEN_BID",
            "OPEN_ASK",
            "CLOSE_BID",
            "CLOSE_ASK",
            "YIELD",
            "EARNINGS",
            "PERATIO",
            "DIVIDENDTP",
            "DIVPAYDATE",
            "EXDIVDATE",
            "CTS_QUAL",
        };

        int size = random.Value!.Next(1, 20);
        string[] view = new string[size];
        int stringFidsSize = stringFids.Length;
        for (int i = 0; i < size; ++i)
            view[i] = stringFids[random.Value!.Next(1, stringFidsSize - 1)];
        return view;
    }

    public static void PrintView(string text, int[] a)
    {
        var list = a.ToList();
        list.Sort();  // we are done with this by this point
        Console.Write("array sorted sans duplicates (text: " + text + "): ");
        int previous = 0; // non-legal
        foreach (int i in list)
            if (i != previous)
            {
                Console.Write(i + " ");
                previous = i;
            }
        Console.WriteLine();
    }

    public static void Main(string[] args)
    {
        bool sendingFidsAsStrings = false;
        for (int i = 0; i < args.Length; ++i)
        {
            if (args[i].Equals("-e"))
                sendingFidsAsStrings = true;
        }

        OmmConsumer? consumer = null;
        try
        {
            AppClient appClient = new();

            OmmConsumerConfig config = new("EmaConfig.xml");

            consumer = new(config.UserName("user"));

            RequestMsg reqMsg = new();

            ElementList view = new();
            OmmArray array = new();

            long handle;
            array.Clear();
            view.Clear();
            reqMsg.Clear();
            int requestCount = 0;
            int reissueCount = 0;

            List<long> handles = new();
            int[] fids;
            string[] fidsAsStrings;
            int events = 0;
            while (events < 60)
            {
                int selector = random.Value!.Next(0, 19); // 0, 1, ..., 19
                if (selector < 7)
                { // unregister
                    if (handles.Count < 3)
                        continue;
                    ++events;
                    int itemToUnregister = random.Value!.Next(0, handles.Count - 1);
                    Console.WriteLine("event: removing handle " + handles[itemToUnregister]);
                    consumer.Unregister(handles[itemToUnregister]);
                    handles.Remove(itemToUnregister);
                }
                else if (selector < 12)
                {  // reissue
                    if (handles.IsNullOrEmpty())
                        continue;
                    ++events;
                    array.Clear();

                    String eventMsg = "event: requesting fids for reissue " + ++reissueCount + ": ";
                    if (sendingFidsAsStrings)
                    {
                        fidsAsStrings = GetElementView();
                        foreach (string s in fidsAsStrings)
                        {
                            array.AddAscii(s);
                            eventMsg += s + " ";
                        }
                    }
                    else
                    {
                        fids = GetView();
                        foreach (int i in fids)
                        {
                            array.AddInt(i);
                            eventMsg += i + " ";
                        }
                    }
                    Console.WriteLine(eventMsg);

                    view.Clear();
                    view.AddUInt(EmaRdm.ENAME_VIEW_TYPE, sendingFidsAsStrings == true ? 2u : 1);
                    view.AddArray(EmaRdm.ENAME_VIEW_DATA, array.Complete());

                    reqMsg.Clear();
                    reqMsg.Payload(view.Complete());

                    int itemToReissue = random.Value!.Next(0, handles.Count - 1);
                    Console.WriteLine("event: reissue for handle " + handles[itemToReissue]);
                    consumer.Reissue(reqMsg, handles[itemToReissue]);
                    Thread.Sleep(10000);
                }
                else
                {           // registerClient
                    ++events;
                    array.Clear();
                    view.Clear();
                    reqMsg.Clear();
                    string eventMsg = "event: requesting fids for request " + ++requestCount + ": ";
                    if (sendingFidsAsStrings)
                    {
                        fidsAsStrings = GetElementView();
                        foreach (string S in fidsAsStrings)
                        {
                            array.AddAscii(S);
                            eventMsg += S + " ";
                        }
                    }
                    else
                    {
                        fids = GetView();
                        foreach (int i in fids)
                        {
                            array.AddInt(i);
                            eventMsg += i + " ";
                        }
                    }
                    Console.WriteLine(eventMsg);

                    view.Clear();
                    view.AddUInt(EmaRdm.ENAME_VIEW_TYPE, sendingFidsAsStrings == true ? 2u : 1);
                    view.AddArray(EmaRdm.ENAME_VIEW_DATA, array.Complete());

                    reqMsg.ServiceName("ELEKTRON_DD").Name("IBM.N");
                    reqMsg.InterestAfterRefresh(true);
                    reqMsg.Payload(view.Complete());
                    handle = consumer.RegisterClient(reqMsg, appClient);
                    Console.WriteLine("event: handle " + handle + " created");
                    handles.Add(handle);
                    Thread.Sleep(10000);
                }
            }

            for (int i = 0; i < handles.Count; ++i)
            {
                Console.WriteLine("event: removing handle (end) " + handles[i]);
                consumer.Unregister(handles[i]);
                Thread.Sleep(10000);
            }
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
