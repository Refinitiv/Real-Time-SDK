/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.Md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Ema.Example.Traning.Consumer;

using LSEG.Ema.Access;
using LSEG.Ema.Rdm;
using System;
using System.Threading;
//APIQA this file is QATools standalone. See qa_readme.txt for details about this tool.

internal class AppClient : IOmmConsumerClient
{
    readonly RequestMsg req = new();
    readonly Access.ElementList eleList = new();
    readonly OmmArray array = new();

    public void Iter(OmmConsumer consumer, AppClient client)
    {

        // repeat send ten same items in batch
        req.Clear();
        eleList.Clear();
        array.Clear();
        array.AddAscii("IBM.N");
        array.AddAscii("MSFT.O");
        array.AddAscii("GOOG.O");
        array.AddAscii("TRI.N");
        array.AddAscii("GAZP.MM");
        array.AddAscii(".09IY");
        array.AddAscii(".TRXFLDESP");
        array.AddAscii("A3M.MC");
        array.AddAscii("ABE.MC");
        array.AddAscii("ACS.MC");
        eleList.AddArray(EmaRdm.ENAME_BATCH_ITEM_LIST, array.Complete());

#pragma warning disable IDE0059 // Unnecessary assignment of a value
        long handle = consumer.RegisterClient(req.ServiceName("DIRECT_FEED").InterestAfterRefresh(false).Payload(eleList.Complete()), client, 1);
#pragma warning restore IDE0059 // Unnecessary assignment of a value
    }

    public void Iter1(OmmConsumer consumer, AppClient client, int count)
    {

        // repeat send ten different items in batch
        req.Clear();
        eleList.Clear();
        array.Clear();
        array.AddAscii("IBM.N" + count);
        array.AddAscii("MSFT.O" + count);
        array.AddAscii("GOOG.O" + count);
        array.AddAscii("TRI.N" + count);
        array.AddAscii("GAZP.MM" + count);
        array.AddAscii(".09IY" + count);
        array.AddAscii(".TRXFLDESP" + count);
        array.AddAscii("A3M.MC" + count);
        array.AddAscii("ABE.MC" + count);
        array.AddAscii("ACS.MC" + count);
        eleList.AddArray(EmaRdm.ENAME_BATCH_ITEM_LIST, array.Complete());

#pragma warning disable IDE0059 // Unnecessary assignment of a value
        long handle = consumer.RegisterClient(req.ServiceName("DIRECT_FEED").InterestAfterRefresh(false).Payload(eleList.Complete()), client, 1);
#pragma warning restore IDE0059 // Unnecessary assignment of a value
    }

    public void Iter2(OmmConsumer consumer, AppClient client, int count)
    {

        // repeat send 10 diff item as single item request
        consumer.RegisterClient(req.Clear().ServiceName("DIRECT_FEED").Name("IBM.N" + count).InterestAfterRefresh(false), client, 1);
        consumer.RegisterClient(req.Clear().ServiceName("DIRECT_FEED").Name("MSFT.O" + count).InterestAfterRefresh(false), client, 1);
        consumer.RegisterClient(req.Clear().ServiceName("DIRECT_FEED").Name("GOOG.O" + count).InterestAfterRefresh(false), client, 1);
        consumer.RegisterClient(req.Clear().ServiceName("DIRECT_FEED").Name("TRI.N" + count).InterestAfterRefresh(false), client, 1);
        consumer.RegisterClient(req.Clear().ServiceName("DIRECT_FEED").Name("GAZP.MM" + count).InterestAfterRefresh(false), client, 1);
        consumer.RegisterClient(req.Clear().ServiceName("DIRECT_FEED").Name(".09IY" + count).InterestAfterRefresh(false), client, 1);
        consumer.RegisterClient(req.Clear().ServiceName("DIRECT_FEED").Name(".TRXFLDESP" + count).InterestAfterRefresh(false), client, 1);
        consumer.RegisterClient(req.Clear().ServiceName("DIRECT_FEED").Name("A3M.MC" + count).InterestAfterRefresh(false), client, 1);
        consumer.RegisterClient(req.Clear().ServiceName("DIRECT_FEED").Name("ABE.MC" + count).InterestAfterRefresh(false), client, 1);
        consumer.RegisterClient(req.Clear().ServiceName("DIRECT_FEED").Name("ACS.MC" + count).InterestAfterRefresh(false), client, 1);

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

            consumer = new(new OmmConsumerConfig().UserName("rmds"));

            var startTime = DateTime.Now;
            int count = 0;
            while (true)
            {
                count++;
                appClient.Iter1(consumer, appClient, count);
                Thread.Sleep(500);
                Console.WriteLine("*** cnt= " + count);
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
//END APIQA
