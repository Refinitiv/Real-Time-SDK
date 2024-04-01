/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.Md for details.                  --
 *|           Copyright (C) 2023-2024 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Ema.Example.Traning.Consumer;

using LSEG.Ema.Access;
using System;
using static LSEG.Ema.Access.DataType;

class AppClient : IOmmConsumerClient
{
	public double BID, BID_1, BID_2, ASK, ASK_1, ASK_2 = 0;

    public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmConsumerEvent ommConsumerEvent)
	{
		Console.WriteLine("Item Name: " + (refreshMsg.HasName ? refreshMsg.Name() : "<not set>"));
		Console.WriteLine("Service Name: " + (refreshMsg.HasServiceName ? refreshMsg.ServiceName() : "<not set>"));
		
		Console.WriteLine("Item State: " + refreshMsg.State());

		Console.WriteLine("Item Handle: " + ommConsumerEvent.Handle + " Item Closure: " + ommConsumerEvent.Closure?.GetHashCode());

		if (DataTypes.FIELD_LIST == refreshMsg.Payload().DataType)
			Decode(refreshMsg.Payload().FieldList());
		
		Console.WriteLine();
	}
	
	public void OnUpdateMsg(UpdateMsg updateMsg, IOmmConsumerEvent ommConsumerEvent) 
	{
		Console.WriteLine("Item Name: " + (updateMsg.HasName ? updateMsg.Name() : "<not set>"));
		Console.WriteLine("Service Name: " + (updateMsg.HasServiceName ? updateMsg.ServiceName() : "<not set>"));
		
		Console.WriteLine("Item Handle: " + ommConsumerEvent.Handle + " Item Closure: " + ommConsumerEvent.Closure?.GetHashCode());

		if (DataTypes.FIELD_LIST == updateMsg.Payload().DataType)
			Decode(updateMsg.Payload().FieldList());
		
		Console.WriteLine();
	}

	public void OnStatusMsg(StatusMsg statusMsg, IOmmConsumerEvent ommConsumerEvent) 
	{
		Console.WriteLine("Item Name: " + (statusMsg.HasName ? statusMsg.Name() : "<not set>"));
		Console.WriteLine("Service Name: " + (statusMsg.HasServiceName ? statusMsg.ServiceName() : "<not set>"));

		if (statusMsg.HasState)
			Console.WriteLine("Item State: " + statusMsg.State());
		
		Console.WriteLine();
	}

	public void Decode(FieldList fieldList)
	{
		foreach(FieldEntry fieldEntry in fieldList)
		{
			switch (fieldEntry.LoadType)
			{
				case DataTypes.REAL:
					if (fieldEntry.FieldId == 22)	// Display data for BID field name and its ripple fields
					{
						if (fieldEntry.RippleTo(fieldEntry.RippleTo()) == 24)
						 BID_2 = BID_1;

						if (fieldEntry.RippleTo() == 23)
							BID_1 = BID;

						BID = fieldEntry.OmmRealValue().AsDouble();

						Console.WriteLine("DataType: " + DataType.AsString(fieldEntry.Load!.DataType));
						Console.WriteLine("Name: " + fieldEntry.Name + " (" + fieldEntry.FieldId + ") Value: " + BID);
						Console.WriteLine("Name: " + fieldEntry.RippleToName() + " (" + fieldEntry.RippleTo() + ") Value: " + BID_1);
						Console.WriteLine("Name: " + fieldEntry.RippleToName(fieldEntry.RippleTo()) + " (" + fieldEntry.RippleTo(fieldEntry.RippleTo())
						+ ") Value: " + BID_2);
					}
					else if (fieldEntry.FieldId == 25) // Display data for ASK field name and its ripple fields
					{
						if (fieldEntry.RippleTo(fieldEntry.RippleTo()) == 27)
							ASK_2 = ASK_1;

						if (fieldEntry.RippleTo() == 26)
							ASK_1 = ASK;

						ASK = fieldEntry.OmmRealValue().AsDouble();

						Console.WriteLine("DataType: " + AsString(fieldEntry.Load!.DataType));
						Console.WriteLine("Name: " + fieldEntry.Name + " (" + fieldEntry.FieldId + ") Value: " + ASK);
						Console.WriteLine("Name: " + fieldEntry.RippleToName() + " (" + fieldEntry.RippleTo() + ") Value: " + ASK_1);
						Console.WriteLine("Name: " + fieldEntry.RippleToName(fieldEntry.RippleTo()) + " (" + fieldEntry.RippleTo(fieldEntry.RippleTo())
						+ ") Value: " + ASK_2);
					}
					break;
				default:
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
			AppClient appClient = new();
			consumer = new(new OmmConsumerConfig().Host("localhost:14002").UserName("user"));
			consumer.RegisterClient(new RequestMsg().ServiceName("DIRECT_FEED").Name("IBM.N"), appClient, 1);
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
