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
using static LSEG.Ema.Access.DataType;

//APIQA this file is QATools standalone. See qa_readme.txt for details about this tool.
internal class AppClient : IOmmConsumerClient
{
    public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmConsumerEvent @event)
    {
        Console.WriteLine("Item Name: " + (refreshMsg.HasName ? refreshMsg.Name() : "<not set>"));
        Console.WriteLine("Service Name: " + (refreshMsg.HasServiceName ? refreshMsg.ServiceName() : "<not set>"));

        Console.WriteLine("Item State: " + refreshMsg.State());
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

    static void Decode(FieldList fieldList)
    {
        foreach (FieldEntry fieldEntry in fieldList)
        {
            Console.Write("Fid: " + fieldEntry.FieldId + " Name = " + fieldEntry.Name + " DataType: " + AsString(fieldEntry.Load!.DataType) + " Value: ");

            if (Data.DataCode.BLANK == fieldEntry.Code)
                Console.WriteLine(" blank");
            else
                switch (fieldEntry.LoadType)
                {
                    case DataTypes.REAL:
                        Console.WriteLine(fieldEntry.OmmRealValue().AsDouble());
                        break;
                    case DataTypes.DATE:
                        Console.WriteLine(fieldEntry.OmmDateValue().Day + " / " + fieldEntry.OmmDateValue().Month + " / " + fieldEntry.OmmDateValue().Year);
                        break;
                    case DataTypes.TIME:
                        Console.WriteLine(fieldEntry.OmmTimeValue().Hour + ":" + fieldEntry.OmmTimeValue().Minute + ":" + fieldEntry.OmmTimeValue().Second + ":" + fieldEntry.OmmTimeValue().Millisecond);
                        break;
                    case DataTypes.INT:
                        Console.WriteLine(fieldEntry.OmmIntValue());
                        break;
                    case DataTypes.UINT:
                        Console.WriteLine(fieldEntry.OmmUIntValue());
                        break;
                    case DataTypes.ASCII:
                        Console.WriteLine(fieldEntry.OmmAsciiValue());
                        break;
                    case DataTypes.ENUM:
                        Console.WriteLine(fieldEntry.OmmEnumValue());
                        break;
                    case DataTypes.RMTES :
                        Console.WriteLine(fieldEntry.OmmRmtesValue());
                        break;
                    case DataTypes.ERROR:
                        Console.WriteLine("(" + fieldEntry.OmmErrorValue().ErrorCodeAsString() + ")");
                        break;
                    default:
                        Console.WriteLine();
                        break;
                }
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
            /*need to run rsslProvider app with only send refresh without updates
             * turn on logging.properties as FINEST level
             * In ItemCallbackClient.java, sets CONSUMER_STARTING_STREAM_ID = 2147483636
             * run 4 minutes
            * validate the following:
            * capture "Reach max number available for next stream id, will wrap around"
            * capture 4 status msg with text "Stream closed for batch"
            * capture 2 msg with text "treamId 0 to item map" for batch item which was created by EMA internal for splitting
            * capture "Added Item 1 of StreamId 2147483638 to item map"
            * capture "Removed Item 1 of StreamId 2147483638 from item map"
            * capture more than 2 times for "2147483638"
            * capture "
            * Item Name: TRI.N_14
            Service Name: DIRECT_FEED
            Item State: Open / Ok / None / 'Item Refresh Completed'
            */
            AppClient appClient = new();

            consumer = new(new OmmConsumerConfig().UserName("user"));
            int requiredNum = 14;
            int numItems = 0;
            bool snapshot = false;
            ElementList batch = new();
            OmmArray array = new();
            while (numItems < requiredNum)
            {
                array.Clear();
                batch.Clear();
                if (snapshot)
                    snapshot = false;
                else
                    snapshot = true;

                if (snapshot)
                {
                    array.AddAscii("TRI.N_" + ++numItems);
                    array.AddAscii("TRI.N_" + ++numItems);
                    array.AddAscii("TRI.N_" + ++numItems);
                    array.AddAscii("TRI.N_" + ++numItems);
                    array.AddAscii("TRI.N_" + ++numItems);
                    batch.AddArray(EmaRdm.ENAME_BATCH_ITEM_LIST, array.Complete());
                    consumer.RegisterClient(new RequestMsg().ServiceName("DIRECT_FEED").InterestAfterRefresh(false).Payload(batch.Complete()), appClient);
                }
                else
                {
                    array.AddAscii("TRI.N_" + ++numItems);
                    array.AddAscii("TRI.N_" + ++numItems);
                    batch.AddArray(EmaRdm.ENAME_BATCH_ITEM_LIST, array.Complete());
                    consumer.RegisterClient(new RequestMsg().ServiceName("DIRECT_FEED").InterestAfterRefresh(true).Payload(batch.Complete()), appClient);
                }

                Thread.Sleep(2000);
            }

            Thread.Sleep(5000); // API calls OnRefreshMsg(), OnUpdateMsg() and
                                // OnStatusMsg()
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
