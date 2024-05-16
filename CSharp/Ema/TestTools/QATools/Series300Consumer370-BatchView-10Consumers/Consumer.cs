/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.Md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Ema.Example.Traning.Consumer;

using System;
using System.Threading;
using LSEG.Ema.Access;
using LSEG.Ema.Rdm;

public class AppClient : IOmmConsumerClient
{
    public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmConsumerEvent consumerEvent)
    {
	RefreshMsg clone = new RefreshMsg(refreshMsg);

        Console.WriteLine(refreshMsg);
    }

    public void OnUpdateMsg(UpdateMsg updateMsg, IOmmConsumerEvent consumerEvent)
    {
        UpdateMsg clone = new UpdateMsg(updateMsg);

        Console.WriteLine(updateMsg);
    }

    public void OnStatusMsg(StatusMsg statusMsg, IOmmConsumerEvent consumerEvent)
    {
        StatusMsg clone = new StatusMsg(statusMsg);

        Console.WriteLine(statusMsg);
    }
}

public class Consumer
{
    static void Main()
    {
        {
            OmmConsumer? consumer = null;
            OmmConsumerConfig? config = null;

            for (int i = 0; i < 10; i++)
            {
                try
                {
                    AppClient appClient = new();
                    config = new OmmConsumerConfig().Host("localhost:14002").UserName("user");
                    consumer = new OmmConsumer(config);

                    long itemHandle = consumer.RegisterClient(new RequestMsg().ServiceName("ELEKTRON_DD").Name("IBM.N"), appClient);

                    var batchArray = new OmmArray()
                    .AddAscii("TRI.N")
                    .AddAscii("IBM.N")
                    .AddAscii("MSFT.O")
                    .Complete();

                    OmmArray viewArray = new()
                    {
                        FixedWidth = 2
                    };

                    viewArray.AddInt(22)
                        .AddInt(25)
                        .Complete();

                    var batchView = new ElementList()
                        .AddArray(EmaRdm.ENAME_BATCH_ITEM_LIST, batchArray)
                        .AddUInt(EmaRdm.ENAME_VIEW_TYPE, 1)
                        .AddArray(EmaRdm.ENAME_VIEW_DATA, viewArray)
                        .Complete();

                    consumer.RegisterClient(new RequestMsg().ServiceName("ELEKTRON_DD").Payload(batchView), appClient);

                    consumer.RegisterClient(new RequestMsg().DomainType(EmaRdm.MMT_DIRECTORY).ServiceName("ELEKTRON_DD"), appClient);

                    Thread.Sleep(3000);
                }
                catch (OmmException excp)
                {
                    Console.WriteLine(excp.Message);
                }
                finally
                {
                    consumer?.Uninitialize();
                }

                Thread.Sleep(1000);

                System.GC.Collect();
            }
        }

        Console.WriteLine("Waiting to exit application");
        System.GC.Collect();
        System.GC.Collect();

        Thread.Sleep(600000); // API calls OnRefreshMsg(), OnUpdateMsg() and OnStatusMsg()
    }
}

