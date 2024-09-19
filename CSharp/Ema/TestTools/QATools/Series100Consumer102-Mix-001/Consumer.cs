/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.Md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Ema.Example.Traning.Consumer;

using LSEG.Ema.Access;
using System;
using System.Collections.Generic;
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
            Console.WriteLine("Item Name: " + updateMsg.Name());

        if (updateMsg.HasServiceName)
            Console.WriteLine("Service Name: " + updateMsg.ServiceName());

        if (DataType.DataTypes.FIELD_LIST == updateMsg.Payload().DataType)
            Decode(updateMsg.Payload().FieldList());

        Console.WriteLine("\n");
    }

    public void OnStatusMsg(StatusMsg statusMsg, IOmmConsumerEvent @event)
    {
        if (statusMsg.HasName)
            Console.WriteLine("Item Name: " + statusMsg.Name());

        if (statusMsg.HasServiceName)
            Console.WriteLine("Service Name: " + statusMsg.ServiceName());

        if (statusMsg.HasState)
            Console.WriteLine("Service State: " + statusMsg.State());

        Console.WriteLine("\n");
    }

    static void Decode(FieldList fieldList)
    {
        IEnumerator<FieldEntry> iter = fieldList.GetEnumerator();
        FieldEntry fieldEntry;
        while (iter.MoveNext())
        {
            fieldEntry = iter.Current;
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
		   // APIQA 	
			AppClient appClient = new();
            AppClient appClient1 = new();
			
			OmmConsumerConfig config = new();
			
            consumer = new OmmConsumer(new OmmConsumerConfig().ConsumerName("Consumer_1"));
			
			RequestMsg reqMsg = new();
			
			consumer.RegisterClient(reqMsg.ServiceName("NI_PUB").Name("IBM.N").InterestAfterRefresh(false), appClient);
            consumer.RegisterClient(reqMsg.ServiceName("ELEKTRON_DD").Name("IBM.N").InterestAfterRefresh(false), appClient1);
            // APIQA END
			
			Thread.Sleep(60000);			// API calls OnRefreshMsg(), OnUpdateMsg() and OnStatusMsg()
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


