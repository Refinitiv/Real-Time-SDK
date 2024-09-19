/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.Md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Ema.Example.Traning.Consumer;

using System;
using System.Threading;
using LSEG.Ema.Access;
using LSEG.Ema.Rdm;
using static LSEG.Ema.Access.DataType;
//APIQA
using System.Text;
//END APIQA
class AppClient : IOmmConsumerClient
{
	public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmConsumerEvent @event)
	{
		Console.WriteLine("Item Name: " + (refreshMsg.HasName ? refreshMsg.Name() : "<not set>"));
		Console.WriteLine("Service Name: " + (refreshMsg.HasServiceName ? refreshMsg.ServiceName() : "<not set>"));
		
		Console.WriteLine("Item State: " + refreshMsg.State());

		if (DataTypes.MAP == refreshMsg.Payload().DataType)
			Decode(refreshMsg.Payload().Map());
		
		Console.WriteLine();
	}
	
	public void OnUpdateMsg(UpdateMsg updateMsg, IOmmConsumerEvent @event) 
	{
		Console.WriteLine("Item Name: " + (updateMsg.HasName ? updateMsg.Name() : "<not set>"));
		Console.WriteLine("Service Name: " + (updateMsg.HasServiceName ? updateMsg.ServiceName() : "<not set>"));
		
		if (DataType.DataTypes.MAP == updateMsg.Payload().DataType)
			Decode(updateMsg.Payload().Map());
		
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

	void Decode(FieldList fieldList, bool newLine)
	{
		foreach(FieldEntry fieldEntry in fieldList)
		{
			
			Console.WriteLine(fieldEntry.Name + "\t");

			if (Data.DataCode.BLANK == fieldEntry.Code)
				Console.WriteLine(" blank");
			else
				switch (fieldEntry.LoadType)
				{
				case DataTypes.REAL :
					Console.WriteLine(fieldEntry.OmmRealValue().AsDouble());
					break;
				case DataTypes.DATE :
					Console.WriteLine(fieldEntry.OmmDateValue().Day + " / " + fieldEntry.OmmDateValue().Month + " / " + fieldEntry.OmmDateValue().Year);
					break;
				case DataTypes.TIME :
					Console.WriteLine(fieldEntry.OmmTimeValue().Hour + ":" + fieldEntry.OmmTimeValue().Minute + ":" + fieldEntry.OmmTimeValue().Second + ":" + fieldEntry.OmmTimeValue().Millisecond);
					break;
				case DataTypes.INT :
					Console.WriteLine(fieldEntry.IntValue());
					break;
				case DataTypes.UINT :
					Console.WriteLine(fieldEntry.UIntValue());
					break;
				case DataTypes.ASCII :
					Console.WriteLine(fieldEntry.OmmAsciiValue());
					break;
				case DataTypes.RMTES :
					Console.WriteLine(fieldEntry.OmmRmtesValue());
					break;
				case DataTypes.ENUM :
					Console.WriteLine(fieldEntry.HasEnumDisplay ? fieldEntry.EnumDisplay() : fieldEntry.EnumValue());
					break;
				case DataTypes.ERROR :
					Console.WriteLine("(" + fieldEntry.OmmErrorValue().ErrorCodeAsString() + ")");
					break;
				default :
					Console.WriteLine();
					break;
				}
			
			if (newLine)
				Console.WriteLine();
		}
	}
	
	void Decode(Map map)
	{
		if (DataTypes.FIELD_LIST == map.SummaryData().DataType)
		{
			Console.WriteLine("Summary :");
			Decode(map.SummaryData().FieldList(), true);
			Console.WriteLine();
		}

		bool firstEntry = true;
		
		foreach(MapEntry mapEntry in map)
		{
			if (firstEntry)
			{
				firstEntry = false;
				Console.WriteLine("Name\tAction");
				Console.WriteLine();
			}
			
			switch (mapEntry.Key.DataType)
			{
				case DataTypes.BUFFER :
					//APIQA
					//Console.WriteLine(mapEntry.Key.Buffer() + "\t" + mapEntry.MapActionAsString());
					Console.WriteLine("mapEntry.Key.Buffer() " + mapEntry.Key.Buffer());
					//END APIQA
					break;
				case DataTypes.ASCII :
					Console.WriteLine(mapEntry.Key.Ascii() + "\t" + mapEntry.MapActionAsString());
					break;
				case DataTypes.RMTES :
					Console.WriteLine(mapEntry.Key.Rmtes() + "\t" + mapEntry.MapActionAsString());
					break;
				default:
					break;
			}
			//APIQA
			//if (DataTypes.FIELD_LIST == mapEntry.LoadType)
			//{
			//	Console.WriteLine("\t");
			//	Decode(mapEntry.FieldList(), false);
			//}
			//END APIQA
			Console.WriteLine();
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
			AppClient appClient = new ();
            //APIQA
			consumer = new(new OmmConsumerConfig().UserName("user"));
			consumer.RegisterClient(new RequestMsg().DomainType(EmaRdm.MMT_SYMBOL_LIST).ServiceName("DIRECT_FEED").Name(".AV.O"), appClient, 0);
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
