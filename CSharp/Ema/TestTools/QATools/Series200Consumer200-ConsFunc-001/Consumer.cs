/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.Md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Access;
using System;
using System.Threading;
using static LSEG.Ema.Access.DataType;
using static LSEG.Ema.Access.OmmDateTime;

namespace LSEG.Ema.Example.Traning.Consumer;

class AppClient : IOmmConsumerClient
{
	public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmConsumerEvent @ommConsumerEvent)
	{
		Console.WriteLine("Item Name: " + (refreshMsg.HasName ? refreshMsg.Name() : "<not set>"));
		Console.WriteLine("Service Name: " + (refreshMsg.HasServiceName ? refreshMsg.ServiceName() : "<not set>"));
		
		Console.WriteLine("Item State: " + refreshMsg.State());
		
		if (DataTypes.FIELD_LIST == refreshMsg.Payload().DataType)
			Decode(refreshMsg.Payload().FieldList());
		
		Console.WriteLine();
	}
	
	public void OnUpdateMsg(UpdateMsg updateMsg, IOmmConsumerEvent @ommConsumerEvent) 
    {
		Console.WriteLine("Item Name: " + (updateMsg.HasName ? updateMsg.Name() : "<not set>"));
		Console.WriteLine("Service Name: " + (updateMsg.HasServiceName ? updateMsg.ServiceName() : "<not set>"));
		
		if (DataType.DataTypes.FIELD_LIST == updateMsg.Payload().DataType)
			Decode(updateMsg.Payload().FieldList());
		
		Console.WriteLine();
	}

	public void OnStatusMsg(StatusMsg statusMsg, IOmmConsumerEvent @ommConsumerEvent) 

    {
		Console.WriteLine("Item Name: " + (statusMsg.HasName ? statusMsg.Name() : "<not set>"));
		Console.WriteLine("Service Name: " + (statusMsg.HasServiceName ? statusMsg.ServiceName() : "<not set>"));

		if (statusMsg.HasState)
			Console.WriteLine("Item State: " +statusMsg.State());
		
		Console.WriteLine();
	}

	void Decode(FieldList fieldList)
	{
		foreach (FieldEntry fieldEntry in fieldList)
		{
            Console.WriteLine("Fid: " + fieldEntry.FieldId + " Name = " + fieldEntry.Name + " DataType: " + DataType.AsString(fieldEntry.Load!.DataType) + " Value: ");

			if (Data.DataCode.BLANK == fieldEntry.Code)
				Console.WriteLine(" blank");
			else
				switch (fieldEntry.LoadType)
				{
				case DataTypes.REAL :
					Console.WriteLine(fieldEntry.OmmRealValue().AsDouble());
					break;
				case DataTypes.DATE :
					//APIQA 
					DateOnly date = DateOnly.Parse(fieldEntry.OmmDateValue().ToString());
					Console.WriteLine(date.ToString("o"));//ISO 8601
					//END APIQA
					break;
				case DataTypes.TIME :
					//APIQA
					TimeOnly time = new TimeOnly(fieldEntry.OmmTimeValue().Hour, fieldEntry.OmmTimeValue().Minute, fieldEntry.OmmTimeValue().Second, fieldEntry.OmmTimeValue().Millisecond);
					Console.WriteLine(time.ToString("o")); //ISO 8601
					//END APIQA
					break;
				case DataTypes.DATETIME :
					//APIQA
					DateTime dataTime = DateTime.Parse(fieldEntry.OmmDateTimeValue().ToString());
					Console.WriteLine(dataTime.ToString("o"));//ISO 8601
					//END APIQA
					break;
				case DataTypes.INT :
					Console.WriteLine(fieldEntry.OmmIntValue());
					break;
				case DataTypes.UINT :
					Console.WriteLine(fieldEntry.UIntValue());
					break;
				case DataTypes.ASCII :
					Console.WriteLine(fieldEntry.OmmAsciiValue());
					break;
				case DataTypes.ENUM :
					Console.WriteLine(fieldEntry.HasEnumDisplay ? fieldEntry.EnumDisplay() : fieldEntry.EnumValue());
					break;
				case DataTypes.RMTES :
					Console.WriteLine(fieldEntry.OmmRmtesValue());
					break;
				case DataTypes.ERROR :
					Console.WriteLine("(" + fieldEntry.OmmErrorValue().ErrorCodeAsString() + ")");
					break;
				default :
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
			AppClient appClient = new();
            consumer = new(new OmmConsumerConfig().Host("localhost:14002").UserName("user"));
            //APIQA
			consumer.RegisterClient(new RequestMsg().ServiceName("DIRECT_FEED").Name("GOOG.O"), appClient);
            //END APIQA
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


