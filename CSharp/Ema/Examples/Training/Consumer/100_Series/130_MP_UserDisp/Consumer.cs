/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Ema.Example.Traning.Consumer;

using LSEG.Ema.Access;
using System;
using static LSEG.Ema.Access.OmmConsumerConfig;

internal class AppClient : IOmmConsumerClient
{
	public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmConsumerEvent consEvent)
    {
        Console.WriteLine("Item Name: " + (refreshMsg.HasName ? refreshMsg.Name() : "<not set>"));
        Console.WriteLine("Service Name: " + (refreshMsg.HasServiceName ? refreshMsg.ServiceName() : "<not set>"));

        Console.WriteLine("Item State: " + refreshMsg.State());

        if (DataType.DataTypes.FIELD_LIST == refreshMsg.Payload().DataType)
            Decode(refreshMsg.Payload().FieldList());

        Console.WriteLine();
    }
	
	public void OnUpdateMsg(UpdateMsg updateMsg, IOmmConsumerEvent consEvent)
    {
        Console.WriteLine("Item Name: " + (updateMsg.HasName ? updateMsg.Name() : "<not set>"));
        Console.WriteLine("Service Name: " + (updateMsg.HasServiceName ? updateMsg.ServiceName() : "<not set>"));

        if (DataType.DataTypes.FIELD_LIST == updateMsg.Payload().DataType)
            Decode(updateMsg.Payload().FieldList());

        Console.WriteLine();
    }

    public void OnStatusMsg(StatusMsg statusMsg, IOmmConsumerEvent consEvent)
    {
        Console.WriteLine("Item Name: " + (statusMsg.HasName ? statusMsg.Name() : "<not set>"));
        Console.WriteLine("Service Name: " + (statusMsg.HasServiceName ? statusMsg.ServiceName() : "<not set>"));
        
        if(statusMsg.HasState)
            Console.WriteLine("Item State: " + statusMsg.State());

        Console.WriteLine();
    }

    static void Decode(Access.FieldList fieldList)
	{
		foreach (var fieldEntry in fieldList)
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
            consumer = new(new OmmConsumerConfig().OperationModel(OperationModelMode.USER_DISPATCH)
													.Host("localhost:14002").UserName("user"));
			consumer.RegisterClient(new RequestMsg().ServiceName("DIRECT_FEED").Name("IBM.N"), new AppClient(), 0);
            var endTime = System.DateTime.Now + TimeSpan.FromMilliseconds(60000);
			while(System.DateTime.Now < endTime)
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
