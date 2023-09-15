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
using static LSEG.Ema.Access.OmmConsumerConfig;

internal class AppClient : IOmmConsumerClient
{
	public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmConsumerEvent _) => Log(refreshMsg);
	
	public void OnUpdateMsg(UpdateMsg updateMsg, IOmmConsumerEvent @event) => Log(updateMsg);

    public void OnStatusMsg(StatusMsg statusMsg, IOmmConsumerEvent @event) => Log(statusMsg);

    private static void Log(Access.Msg msg)
    {
        Console.WriteLine("Item Name: " + (msg.HasName ? msg.Name() : "<not set>"));
        Console.WriteLine("Service Name: " + (msg.HasServiceName ? msg.ServiceName() : "<not set>"));
        if (DataType.DataTypes.FIELD_LIST == msg.Payload().DataType)
            Decode(msg.Payload().FieldList());
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
