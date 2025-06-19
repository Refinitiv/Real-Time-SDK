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
using System.Threading;

class AppClient : IOmmConsumerClient
{
	public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmConsumerEvent @event)
	{
		Console.WriteLine("Item Name: " + (refreshMsg.HasName ? refreshMsg.Name() : "<not set>"));
		Console.WriteLine("Service Name: " + (refreshMsg.HasServiceName ? refreshMsg.ServiceName() : "<not set>"));
		
		Console.WriteLine("Item State: " + refreshMsg.State());
		
		if (DataType.DataTypes.OPAQUE == refreshMsg.Payload().DataType)
            Decode(refreshMsg.Payload().OmmOpaque());
		
		Console.WriteLine();
	}
	
	public void OnUpdateMsg(UpdateMsg updateMsg, IOmmConsumerEvent @event) 
	{
		Console.WriteLine("Item Name: " + (updateMsg.HasName ? updateMsg.Name() : "<not set>"));
		Console.WriteLine("Service Name: " + (updateMsg.HasServiceName ? updateMsg.ServiceName() : "<not set>"));
		
		if (DataType.DataTypes.OPAQUE == updateMsg.Payload().DataType)
            Decode(updateMsg.Payload().OmmOpaque());
		
		Console.WriteLine();
	}

	public void OnStatusMsg(StatusMsg statusMsg, IOmmConsumerEvent @event) 
	{
		Console.WriteLine("Item Name: " + (statusMsg.HasName ? statusMsg.Name() : "<not set>"));
		Console.WriteLine("Service Name: " + (statusMsg.HasServiceName ? statusMsg.ServiceName() : "<not set>"));

		if (statusMsg.HasState)
			Console.WriteLine("Item State: " +statusMsg.State());
		
		Console.WriteLine();
	}

    private static void Decode(OmmOpaque oq) => Console.WriteLine($"OmmOpaque data: {oq}");
}

public class Consumer 
{
	public static void Main()
	{
		OmmConsumer? consumer = null;
		try
		{
			consumer = new (new OmmConsumerConfig().Host("localhost:14002").UserName("user"));
			consumer.RegisterClient(new RequestMsg().DomainType(200).ServiceName("DIRECT_FEED").Name("IBM.XYZ"), new AppClient(), 0);
			Thread.Sleep(60000);            // API calls OnRefreshMsg(), OnUpdateMsg() and OnStatusMsg()
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
