/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.Md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Access;
using System;
using static LSEG.Ema.Access.OmmConsumerConfig;

namespace LSEG.Ema.Example.Traning.Consumer;

internal class AppClient : IOmmConsumerClient
{
    private readonly RmtesBuffer rmtesBuffer = new(new byte[0]);

    public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmConsumerEvent @event)
    {
        Console.WriteLine("Received Refresh. Item Handle: " + @event.Handle + " Closure: " + @event.Closure);
        Console.WriteLine("Item Name: " + (refreshMsg.HasName ? refreshMsg.Name() : "<not set>"));
        Console.WriteLine("Service Name: " + (refreshMsg.HasServiceName ? refreshMsg.ServiceName() : "<not set>"));
        Console.WriteLine("Item State: " + refreshMsg.State());
        if (DataType.DataTypes.FIELD_LIST == refreshMsg.Payload().DataType)
            Decode(refreshMsg.Payload().FieldList());

        Console.WriteLine();
    }

    public void OnUpdateMsg(UpdateMsg updateMsg, IOmmConsumerEvent @event)
    {
        Console.WriteLine("Received Update. Item Handle: " + @event.Handle + " Closure: " + @event.Closure);
        Console.WriteLine("Item Name: " + (updateMsg.HasName ? updateMsg.Name() : "<not set>"));
        Console.WriteLine("Service Name: " + (updateMsg.HasServiceName ? updateMsg.ServiceName() : "<not set>"));

        if (DataType.DataTypes.FIELD_LIST == updateMsg.Payload().DataType)
            Decode(updateMsg.Payload().FieldList());

        Console.WriteLine();
    }

    public void OnStatusMsg(StatusMsg statusMsg, IOmmConsumerEvent @event)
    {
        Console.WriteLine("Received Status. Item Handle: " + @event.Handle + " Closure: " + @event.Closure);

        Console.WriteLine("Item Name: " + (statusMsg.HasName ? statusMsg.Name() : "<not set>"));
        Console.WriteLine("Service Name: " + (statusMsg.HasServiceName ? statusMsg.ServiceName() : "<not set>"));

        if (statusMsg.HasState)
            Console.WriteLine("Item State: " + statusMsg.State());

        Console.WriteLine();
    }

    private void Decode(FieldList fieldList)
    {
        // In the below loop partial updates for the specific field of RMTES type are handled.
        // Note that in case it is necessary to handle partial updates for multiple fields,
        // the application has to cache each RMTES string in a separate RmtesBuffer
        // (e.g., use a hashmap to track RmtesBuffer instances corresponding to specific FIDs)
        // and apply the updates accordingly.
        foreach (FieldEntry fieldEntry in fieldList)
        {
            if (fieldEntry.Name.Equals("HEADLINE1"))
            {
                Console.Write("Fid: " + fieldEntry.FieldId + " Name = " + fieldEntry.Name + " DataType: " + DataType.AsString(fieldEntry.Load!.DataType) + " Value: ");

                if (Data.DataCode.BLANK == fieldEntry.Code)
                    Console.WriteLine(" blank");
                else
                    Console.WriteLine(rmtesBuffer.Apply(fieldEntry.OmmRmtesValue()).ToString());
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
            AppClient appClient = new();
            consumer = new(new OmmConsumerConfig().OperationModel(OperationModelMode.USER_DISPATCH).Host("localhost:14002").UserName("user"));
            consumer.RegisterClient(new RequestMsg().ServiceName("DIRECT_FEED").Name("NFCP_UBMS"), appClient, 0);
            var endTime = DateTime.Now + TimeSpan.FromMilliseconds(60000);
            while (DateTime.Now < endTime)
            {
                consumer.Dispatch(10);      // calls to OnRefreshMsg(), OnUpdateMsg(), or OnStatusMsg() execute on this thread
            }
        }
        catch (OmmException excp)
        {
            Console.WriteLine(excp);
        }
        finally
        {
            consumer?.Uninitialize();
        }
    }
}