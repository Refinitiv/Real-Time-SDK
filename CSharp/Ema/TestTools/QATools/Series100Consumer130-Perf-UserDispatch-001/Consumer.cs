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
using System.Text;
using System.Threading;
using static LSEG.Ema.Access.OmmConsumerConfig;

//APIQA this file is QATools standalone. See qa_readme.txt for details about this tool.

internal class AppClient : IOmmConsumerClient
{
    public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmConsumerEvent @event)
    {
        Console.WriteLine("Item Name: " + (refreshMsg.HasName ? refreshMsg.Name() : "<not set>"));
        Console.WriteLine("Service Name: " + (refreshMsg.HasServiceName ? refreshMsg.ServiceName() : "<not set>"));

        Console.WriteLine("Item State: " + refreshMsg.State());

        //APIQA
        //	if (DataType.DataTypes.FIELD_LIST == refreshMsg.Payload().DataType)
        //		decode(refreshMsg.Payload().FieldList());
        //END APIQA

        Console.WriteLine();
    }

    public void OnUpdateMsg(UpdateMsg updateMsg, IOmmConsumerEvent @event)
    {
        Console.WriteLine("Item Name: " + (updateMsg.HasName ? updateMsg.Name() : "<not set>"));
        Console.WriteLine("Service Name: " + (updateMsg.HasServiceName ? updateMsg.ServiceName() : "<not set>"));

        if (DataType.DataTypes.FIELD_LIST == updateMsg.Payload().DataType)
            Decode(updateMsg.Payload().FieldList());

        Console.WriteLine();
    }

    public void OnStatusMsg(StatusMsg statusMsg, IOmmConsumerEvent @event)
    {
        Console.WriteLine("Item Name: " + (statusMsg.HasName ? statusMsg.Name() : "<not set>"));
        Console.WriteLine("Service Name: " + (statusMsg.HasServiceName ? statusMsg.ServiceName() : "<not set>"));

        if (statusMsg.HasState)
            Console.WriteLine("Item State: " + statusMsg.State());

        Console.WriteLine();
    }

    private static void Decode(FieldList fieldList)
    {
        foreach (FieldEntry fieldEntry in fieldList)
        {
            Console.WriteLine("Fid: " + fieldEntry.FieldId + " Name: " + fieldEntry.Name + " value: " + fieldEntry.Load);
        }
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

            //APIQA
            consumer = new(new OmmConsumerConfig().ConsumerName("Consumer_2")
                                                    .OperationModel(OperationModelMode.USER_DISPATCH)
                                                    .UserName("user"));

            // 	consumer.RegisterClient(new RequestMsg().ServiceName("DIRECT_FEED").Name("IBM.N"), appClient, 0);
            RequestMsg reqMsg = new();

            String itemPreName = "RTR";
            StringBuilder itemName = new();
            long[] handles = new long[10000];

            Thread dThread = new(() => { while (true) { consumer.Dispatch(10); } });
            dThread.Start();

            while (true)
            {
                Console.WriteLine("############## Starting a new iteration ###############");
                Thread.Sleep(1000);

                for (int idx = 0; idx < 10000; ++idx)
                {
                    itemName.Append(itemPreName).Append(idx).Append(".N");
                    reqMsg.Clear().ServiceName("DIRECT_FEED").Name(itemName.ToString());
                    handles[idx] = consumer.RegisterClient(reqMsg, appClient);
                    itemName.Length = 0;
                }

                Thread.Sleep(980000);

                for (int idx = 0; idx < 10000; ++idx)
                {
                    consumer.Unregister(handles[idx]);
                }
            }
        }
        //END APIQA
        catch (OmmException excp)
        {
            Console.WriteLine(excp);
        }
        catch (ThreadInterruptedException excp)
        {
            Console.WriteLine(excp);
        }
        finally
        {
            consumer?.Uninitialize();
        }
    }
}