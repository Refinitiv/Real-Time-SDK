/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Access;
using System.Threading;
using System;
using static LSEG.Ema.Access.DataType;
using LSEG.Ema.Rdm;

namespace LSEG.Ema.Example.Traning.Consumer;

internal class AppClient : IOmmConsumerClient
{
	// APIQA
	public bool SendConsumerStatus = true;
	public OmmConsumer? Consumer = null;
	public long DirectoryHandle = 0;
	//END APIQA

	public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmConsumerEvent @event)
	{
		Console.WriteLine("Received Refresh. Item Handle: " + @event.Handle + " Closure: " + (@event.Closure ?? "null"));
		
		Console.WriteLine("Item Name: " + (refreshMsg.HasName ? refreshMsg.Name() : "<not set>"));
		Console.WriteLine("Service Name: " + (refreshMsg.HasServiceName ? refreshMsg.ServiceName() : "<not set>"));

		Console.WriteLine("Item State: " + refreshMsg.State());

		Decode(refreshMsg);
			
		Console.WriteLine();

		// APIQA
		if (SendConsumerStatus)
			SendConsumerStatusMsg(@event);
	// END APIQA
	}

	public void OnUpdateMsg(UpdateMsg updateMsg, IOmmConsumerEvent @event) 
	{
		Console.WriteLine("Received Update. Item Handle: " + @event.Handle + " Closure: " + (@event.Closure ?? "null"));
		
		Console.WriteLine("Item Name: " + (updateMsg.HasName ? updateMsg.Name() : "<not set>"));
		Console.WriteLine("Service Name: " + (updateMsg.HasServiceName ? updateMsg.ServiceName() : "<not set>"));
		
		Decode(updateMsg);
		
		Console.WriteLine();
	}

	public void OnStatusMsg(StatusMsg statusMsg, IOmmConsumerEvent @event) 
	{
		Console.WriteLine("Received Status. Item Handle: " + @event.Handle + " Closure: " + (@event.Closure ?? "null"));

		Console.WriteLine("Item Name: " + (statusMsg.HasName ? statusMsg.Name() : "<not set>"));
		Console.WriteLine("Service Name: " + (statusMsg.HasServiceName ? statusMsg.ServiceName() : "<not set>"));

		if (statusMsg.HasState)
			Console.WriteLine("Item State: " +statusMsg.State());
		
		Console.WriteLine();
	}
	
	public void OnAckMsg(AckMsg ackMsg, IOmmConsumerEvent _) {}
	public void OnGenericMsg(GenericMsg genericMsg, IOmmConsumerEvent @event)
	{
		//APIQA
		Console.WriteLine("#####Received GenericMsg. Item Handle: " + @event.Handle + " Closure: " + (@event.Closure ?? "null"));
		Console.WriteLine(genericMsg);
		//END APIQA
	}
	public void OnAllMsg(Msg msg, IOmmConsumerEvent _){}

	void Decode(Msg msg)
	{
		switch(msg.Attrib().DataType)
		{
		case DataTypes.ELEMENT_LIST:
			Decode(msg.Attrib().ElementList());
			break;
		default:
			break;
		}

		switch(msg.Payload().DataType)
		{
		case  DataTypes.MAP:
			Decode(msg.Payload().Map());
			break;
		case DataTypes.FIELD_LIST:
			Decode(msg.Payload().FieldList());
			break;
		default:
			break;
		}
	}
	
	void Decode(ElementList elementList)
	{
		foreach (ElementEntry elementEntry in elementList)
		{
			Console.Write(" Name = " + elementEntry.Name + " DataType: " + DataType.AsString(elementEntry.Load!.DataType) + " Value: ");

			if (Data.DataCode.BLANK == elementEntry.Code)
				Console.WriteLine(" blank");
			else
				switch (elementEntry.LoadType)
				{
					case DataTypes.REAL :
						Console.WriteLine(elementEntry.OmmRealValue().AsDouble());
						break;
					case DataTypes.DATE :
						Console.WriteLine(elementEntry.OmmDateValue().Day + " / " + elementEntry.OmmDateValue().Month + " / " + elementEntry.OmmDateValue().Year);
						break;
					case DataTypes.TIME :
						Console.WriteLine(elementEntry.OmmTimeValue().Hour + ":" + elementEntry.OmmTimeValue().Minute + ":" + elementEntry.OmmTimeValue().Second + ":" + elementEntry.OmmTimeValue().Millisecond);
						break;
					case DataTypes.INT :
						Console.WriteLine(elementEntry.IntValue());
						break;
					case DataTypes.UINT :
						Console.WriteLine(elementEntry.UIntValue());
						// APIQA
						if (elementEntry.Name.Equals(EmaRdm.ENAME_ACCEPTING_CONS_STATUS) && elementEntry.UIntValue() == 1)
							SendConsumerStatus = false;
						// END APIQA
						break;
					case DataTypes.ASCII :
						Console.WriteLine(elementEntry.OmmAsciiValue());
						break;
					case DataTypes.ENUM :
						Console.WriteLine(elementEntry.EnumValue());
						break;
					case DataTypes.ARRAY :
						bool first = true;
						foreach(OmmArrayEntry arrayEntry in elementEntry.OmmArrayValue())
						{
							if ( !first )
								Console.Write(", ");
							else
								first = false;
							switch(arrayEntry.LoadType)
							{
								case DataTypes.ASCII :
									Console.Write(arrayEntry.OmmAsciiValue());
									break;
								case DataTypes.UINT :
									Console.Write(arrayEntry.OmmUIntValue());
									break;
								case DataTypes.QOS :
									Console.Write(arrayEntry.OmmQosValue());
									break;
								default:
									break;
							}
						}
						Console.WriteLine();
						break;
					case DataTypes.RMTES :
						Console.WriteLine(elementEntry.OmmRmtesValue());
						break;
					case DataTypes.ERROR :
						Console.WriteLine(elementEntry.OmmErrorValue().ErrorCode +" (" + elementEntry.OmmErrorValue().ErrorCodeAsString() + ")");
						break;
					default :
						Console.WriteLine();
						break;
				}
		}
	}

	// APIQA
	void SendConsumerStatusMsg(IOmmConsumerEvent evnt)
	{
		SendConsumerStatus = false;
		ElementList serviceInfoId = new ElementList();
		serviceInfoId.AddUInt(EmaRdm.ENAME_CONS_SOURCE_MIROR_MODE, 2).Complete();

		Map map = new Map();
		map.AddKeyUInt(1, MapAction.ADD, serviceInfoId);
		GenericMsg genericMsg = new GenericMsg();
		Consumer?.Submit(genericMsg.DomainType(EmaRdm.MMT_DIRECTORY).Name(EmaRdm.ENAME_CONS_STATUS).Payload(map).Complete(true), DirectoryHandle);
	}
	// END APIQA

	void Decode(FieldList fieldList)
	{
		foreach (FieldEntry fieldEntry in fieldList)
		{
			Console.Write("Fid: " + fieldEntry.FieldId + " Name = " + fieldEntry.Name + " DataType: " + DataType.AsString(fieldEntry.Load!.DataType) + " Value: ");

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
					case DataTypes.ENUM :
						Console.WriteLine(fieldEntry.HasEnumDisplay ? fieldEntry.EnumDisplay() : fieldEntry.EnumValue());
						break;
					case DataTypes.ARRAY :
						Console.WriteLine(fieldEntry.OmmArrayValue());
						break;
					case DataTypes.RMTES :
						Console.WriteLine(fieldEntry.OmmRmtesValue());
						break;
					case DataTypes.ERROR :
						Console.WriteLine(fieldEntry.OmmErrorValue().ErrorCode +" (" + fieldEntry.OmmErrorValue().ErrorCodeAsString() + ")");
						break;
					default :
						Console.WriteLine();
						break;
				}
		}
	}
	
	void Decode(Map map)
	{
		foreach(MapEntry mapEntry in map)
		{
			switch (mapEntry.LoadType)
			{
				case DataTypes.FILTER_LIST :
					Decode(mapEntry.FilterList());
					break;
				default:
					Console.WriteLine();
					break;
			}
		}
	}

	void Decode(FilterList filterList)
	{
		foreach(FilterEntry filterEntry in filterList)
		{
			Console.WriteLine("ID: " + filterEntry.FilterId
					+ " Action = " + filterEntry.FilterActionAsString() 
					+ " DataType: " + DataType.AsString(filterEntry.LoadType) + " Value: ");

			switch (filterEntry.LoadType)
			{
				case DataTypes.ELEMENT_LIST :
					Decode(filterEntry.ElementList());
					break;
				case DataTypes.MAP :
					Decode(filterEntry.Map());
					break;
				default:
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
			//APIQA
			consumer  = new(new OmmConsumerConfig().UserName("user"));
			
			appClient.Consumer = consumer;

			RequestMsg reqMsg = new();

			appClient.DirectoryHandle  = consumer.RegisterClient(reqMsg.DomainType(EmaRdm.MMT_DIRECTORY).ServiceName("DIRECT_FEED"), appClient);
			
			Thread.Sleep(600000);			// API calls OnRefreshMsg(), OnUpdateMsg() and OnStatusMsg()
			//END APIQA
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
