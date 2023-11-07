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
using System.Threading;

internal class AppClient : IOmmConsumerClient
{
	public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmConsumerEvent @event)
	{
		if (refreshMsg.HasName)
			Console.WriteLine("Item Name: " + refreshMsg.Name());
		
		if (refreshMsg.HasServiceName)
			Console.WriteLine("Service Name: " + refreshMsg.ServiceName());
		
		Console.WriteLine("Item State: " + refreshMsg.State());
		
		if (DataType.DataTypes.FIELD_LIST == refreshMsg.Payload().DataType)
			Decode(refreshMsg.Payload().FieldList());
		
		Console.WriteLine("\n");
	}
	
	public void OnUpdateMsg(UpdateMsg updateMsg, IOmmConsumerEvent @event) 
	{
		if (updateMsg.HasName)
			Console.WriteLine("Item Name: " + updateMsg.Name().ToString());
		
		if (updateMsg.HasServiceName)
			Console.WriteLine("Service Name: " + updateMsg.ServiceName().ToString());
		
		if (DataType.DataTypes.FIELD_LIST == updateMsg.Payload().DataType)
			Decode(updateMsg.Payload().FieldList());
		
		Console.WriteLine("\n");
	}

	public void OnStatusMsg(StatusMsg statusMsg, IOmmConsumerEvent @event) 
	{
		if (statusMsg.HasName)
			Console.WriteLine("Item Name: " + statusMsg.Name().ToString());
		
		if (statusMsg.HasServiceName)
			Console.WriteLine("Service Name: " + statusMsg.ServiceName().ToString());
		
		if (statusMsg.HasState)
			Console.WriteLine("Service State: " + statusMsg.State().ToString());
		
		Console.WriteLine("\n");
	}

    static void Decode(FieldList fieldList)
	{
		foreach(var fieldEntry in fieldList)
		{
            Console.WriteLine($"Fid: {fieldEntry.FieldId} Name: {fieldEntry.Name} value: {fieldEntry.Load}");
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
			consumer = new (new OmmConsumerConfig().Host("localhost:14002").UserName("user"));
			consumer.RegisterClient(new RequestMsg().ServiceName("DIRECT_FEED").Name("IBM.N"), new AppClient(), 0);
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


