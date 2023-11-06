/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.Md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Ema.Example.Traning.Consumer;

using LSEG.Ema.Access;
using LSEG.Ema.Rdm;
using System;
using static LSEG.Ema.Access.DataType;
using static LSEG.Ema.Access.OmmConsumerConfig;

class AppClient : IOmmConsumerClient
{
	public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmConsumerEvent _)
	{
		Console.WriteLine("Item Name: " + (refreshMsg.HasName ? refreshMsg.Name() : "<not set>"));
		Console.WriteLine("Service Name: " + (refreshMsg.HasServiceName ? refreshMsg.ServiceName() : "<not set>"));
		
		Console.WriteLine("Item State: " + refreshMsg.State());
		
		if (DataTypes.MAP == refreshMsg.Payload().DataType)
			Decode(refreshMsg.Payload().Map());
		
		Console.WriteLine();
	}
	
	public void OnUpdateMsg(UpdateMsg updateMsg, IOmmConsumerEvent _) 
	{
		Console.WriteLine("Item Name: " + (updateMsg.HasName ? updateMsg.Name() : "<not set>"));
		Console.WriteLine("Service Name: " + (updateMsg.HasServiceName ? updateMsg.ServiceName() : "<not set>"));
		
		if (DataTypes.MAP == updateMsg.Payload().DataType)
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

	void Decode(Map map)
	{
		if (DataTypes.FIELD_LIST == map.SummaryData().DataType)
		{
			Console.WriteLine("Map Summary data:");
			Decode(map.SummaryData().FieldList());
			Console.WriteLine();
		}

		foreach (MapEntry mapEntry in map)
		{
			switch (mapEntry.Key.DataType)
                        {
                                case DataTypes.BUFFER :
                                        Console.WriteLine("Action: " + mapEntry.MapActionAsString() + ", key value: " + mapEntry.Key.Buffer() + "\n");
                                        break;
                                case DataTypes.ASCII :
                                        Console.WriteLine("Action: " + mapEntry.MapActionAsString() + ", key value: " + mapEntry.Key.Ascii() + "\n");
                                        break;
                                case DataTypes.RMTES :
                                        Console.WriteLine("Action: " + mapEntry.MapActionAsString() + ", key value: " + mapEntry.Key.Rmtes() + "\n");
                                        break;
                                default:
                                        break;
                        }

			if (DataTypes.FIELD_LIST == mapEntry.LoadType)
			{
				Console.WriteLine("Entry data:");
				Decode(mapEntry.FieldList());
				Console.WriteLine();
			}
		}
	}
	
	void Decode(FieldList fieldList)
	{
		foreach (FieldEntry fieldEntry in fieldList)
		{
            Console.WriteLine("Fid: " + fieldEntry.FieldId + " Name = " + fieldEntry.Name + " DataType: " + AsString(fieldEntry.Load!.DataType) + " Value: ");

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
					{
						OmmTime ommTime = fieldEntry.OmmTimeValue();
						Console.WriteLine($"{ommTime.Hour}:{ommTime.Minute}:{ommTime.Second}:{ommTime.Millisecond}");
						break;
					}
				case DataTypes.INT :
					Console.WriteLine(fieldEntry.IntValue());
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
			consumer = new(new OmmConsumerConfig()
													.OperationModel(OperationModelMode.USER_DISPATCH)
													.Host("localhost:14002").UserName("user"));
			
			consumer.RegisterClient(new RequestMsg().DomainType(EmaRdm.MMT_MARKET_BY_PRICE)
															.ServiceName("DIRECT_FEED").Name("BBH.ITC")
															.PrivateStream(true), new AppClient());

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
