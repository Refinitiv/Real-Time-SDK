/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Access;
using LSEG.Ema.Domain.Login;
using LSEG.Ema.Rdm;
using System;
using System.Collections.Generic;
using static LSEG.Ema.Access.OmmConsumerConfig;
using static LSEG.Ema.Access.DataType;

namespace LSEG.Ema.Example.Traning.Consumer;

internal class AppClient : IOmmConsumerClient
{
    private Dictionary<string, RmtesBuffer> rmtesBufferList = new Dictionary<string, RmtesBuffer>();

    public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmConsumerEvent @event)
    {
        Console.WriteLine("Received Refresh. Item Handle: " + @event.Handle + " Closure: " + (@event.Closure ?? "null"));
        Console.WriteLine("Item Name: " + (refreshMsg.HasName ? refreshMsg.Name() : "<not set>"));
        Console.WriteLine("Service Name: " + (refreshMsg.HasServiceName ? refreshMsg.ServiceName() : "<not set>"));
        Console.WriteLine("Item State: " + refreshMsg.State());
        if (DataType.DataTypes.FIELD_LIST == refreshMsg.Payload().DataType)
            Decode(refreshMsg.Payload().FieldList());

        Console.WriteLine();
    }

    public void OnUpdateMsg(UpdateMsg updateMsg, IOmmConsumerEvent @event)
    {
        Console.WriteLine("Received Update. Item Handle: " + @event.Handle + " Closure: " + (@event.Closure ?? "null"));
        Console.WriteLine("Item Name: " + (updateMsg.HasName ? updateMsg.Name() : "<not set>"));
        Console.WriteLine("Service Name: " + (updateMsg.HasServiceName ? updateMsg.ServiceName() : "<not set>"));

        if (DataType.DataTypes.FIELD_LIST == updateMsg.Payload().DataType)
            Decode(updateMsg.Payload().FieldList());

        Console.WriteLine();
    }

    public void OnStatusMsg(StatusMsg statusMsg, IOmmConsumerEvent @event)
    {
        Console.WriteLine("Received Status. Item Handle: " + @event.Handle + " Closure: " + (@event.Closure ?? "null"));

        Console.WriteLine("Item Name: " + (statusMsg.HasName ? statusMsg.Name() : "<not set>"));
        Console.WriteLine("Service Name: " + (statusMsg.HasServiceName ? statusMsg.ServiceName() : "<not set>"));

        if (statusMsg.HasState)
            Console.WriteLine("Item State: " + statusMsg.State());

        Console.WriteLine();
    }

    private void Decode(FieldList fieldList)
    {
        // In the below loop partial updates for the specific field of RMTES type are handled.
        // Note that in order to handle partial updates for multiple fields,
        // the application caches each RMTES string in a separate RmtesBuffer
        foreach (FieldEntry fieldEntry in fieldList)
        {
            Console.Write("Fid: " + fieldEntry.FieldId + " Name = " + fieldEntry.Name + " DataType: " + DataType.AsString(fieldEntry.Load!.DataType) + " Value: ");

            if (Data.DataCode.BLANK == fieldEntry.Code)
                Console.WriteLine(" blank");
            else if (fieldEntry.LoadType == DataTypes.RMTES)
            {
                RmtesBuffer rmtesBuffer = new RmtesBuffer();
                rmtesBuffer.Apply(fieldEntry.OmmRmtesValue());
                if (rmtesBufferList.ContainsKey(fieldEntry.Name))
                    rmtesBufferList[fieldEntry.Name].Apply(rmtesBuffer);
                else
                    rmtesBufferList.Add(fieldEntry.Name, rmtesBuffer);

                Console.WriteLine(rmtesBufferList[fieldEntry.Name].ToString());
            }
            else
            {
                Console.WriteLine(fieldEntry.ToString());
            }
        }
    }
}

public class Consumer
{
    public static void Main()
    {
        try
        {
            AppClient appClient = new();
            using OmmConsumer consumer = new(new OmmConsumerConfig().OperationModel(OperationModelMode.USER_DISPATCH).Host("localhost:14002").UserName("user"));
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
    }
}
