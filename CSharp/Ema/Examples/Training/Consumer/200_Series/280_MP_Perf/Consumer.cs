/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Ema.Example.Traning.Consumer;

using LSEG.Ema.Access;
using System.Threading;
using System;
using static LSEG.Ema.Access.DataType;
using System.Text;

class AppClient : IOmmConsumerClient
{
	public long UpdateCount { get; set; }
    public long RefreshCount { get; set; }
    public long StatusCount { get; set; }
    public ComplexTypeData? Payload { get; set; }

    public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmConsumerEvent @event)
	{
		++RefreshCount;
		
		Payload = refreshMsg.Payload();
		if (DataTypes.FIELD_LIST == Payload.DataType)
			Decode(Payload.FieldList());
	}
	
	public void OnUpdateMsg(UpdateMsg updateMsg, IOmmConsumerEvent @event) 
	{
		++UpdateCount;
		
		Payload = updateMsg.Payload();
		if (DataTypes.FIELD_LIST == Payload.DataType)
			Decode(Payload.FieldList());
	}

	public void OnStatusMsg(StatusMsg _, IOmmConsumerEvent __) => ++StatusCount;

    private static void Decode(FieldList fieldList)
	{
		try
		{
			foreach(FieldEntry fieldEntry in fieldList)
			{
				if (Data.DataCode.NO_CODE == fieldEntry.Code)
					switch (fieldEntry.LoadType)
					{
						case DataTypes.REAL :
						{
							OmmReal re = fieldEntry.OmmRealValue();
						}
						break;
						case DataTypes.DATE :
						{
							OmmDate date = fieldEntry.OmmDateValue();
						}
						break;
						case DataTypes.TIME :
						{
							OmmTime time = fieldEntry.OmmTimeValue();
						}
						break;
						case DataTypes.DATETIME :
						{
							OmmDateTime dateTime = fieldEntry.OmmDateTimeValue();
						}
						break;
						case DataTypes.INT :
						{
							long value = fieldEntry.IntValue();
						}
						break;
						case DataTypes.UINT :
						{
							ulong value = fieldEntry.UIntValue();
						}
						break;
						case DataTypes.FLOAT :
						{
							float value = fieldEntry.FloatValue();
						}
						break;
						case DataTypes.DOUBLE :
						{
							double value = fieldEntry.DoubleValue();
						}
						break;
						case DataTypes.QOS :
						{
							OmmQos value = fieldEntry.OmmQosValue();
						}
						break;
						case DataTypes.STATE :
						{
							OmmState value = fieldEntry.OmmStateValue();
						}
						break;
						case DataTypes.ASCII :
						{
							OmmAscii asciiString = fieldEntry.OmmAsciiValue();
						}
						break;
						case DataTypes.RMTES :
						{
							OmmRmtes rmtesBuffer = fieldEntry.OmmRmtesValue();
						}
						break;
						case DataTypes.UTF8 :
						{
							OmmUtf8 utf8Buffer = fieldEntry.OmmUtf8Value();
						}
						break;
						case DataTypes.BUFFER :
						{
							OmmBuffer value = fieldEntry.OmmBufferValue();
						}
						break;
						case DataTypes.ENUM :
						{
							int value = fieldEntry.EnumValue();
						}
						break;
						case DataTypes.ARRAY :
						{
							OmmArray value = fieldEntry.OmmArrayValue();
						}
						break;
						case DataTypes.ERROR :
						{
							OmmError error = fieldEntry.OmmErrorValue();
						}
						break;
						default :
						break;
					}
			}
		}
		catch (OmmException excp)
		{
			Console.WriteLine(excp);
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
			consumer = new(new OmmConsumerConfig().Host("localhost:14002").UserName("user"));
			AppClient appClient = new();
            const string ItemPreName = "RTR";
			StringBuilder itemName = new();
			for (int idx = 0; idx < 1000; ++idx)
			{
				itemName.Append(ItemPreName).Append(idx).Append(".N");
				consumer.RegisterClient(new RequestMsg().ServiceName("DIRECT_FEED").Name(itemName.ToString()), appClient);
				itemName.Length = 0;
			}

			StringBuilder display = new();
			for (int idx = 0; idx < 300; ++idx)
			{
				Thread.Sleep(1000);
				display.Append("total refresh count: " ).Append(appClient.RefreshCount).Append("\ttotal status count: ").Append(appClient.StatusCount)
							.Append("\tupdate rate (per sec): ").Append(appClient.UpdateCount);
				Console.WriteLine(display.ToString());
				
				display.Length = 0;
				appClient.UpdateCount = 0;
			}
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
