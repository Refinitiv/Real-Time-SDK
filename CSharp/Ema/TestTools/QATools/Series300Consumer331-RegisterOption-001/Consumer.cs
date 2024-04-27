/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.Md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
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
	public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmConsumerEvent @event)
	{
		Console.WriteLine("Received Refresh. Item Handle: " + @event.Handle + " Closure: " + (@event.Closure ?? "null"));
		
		Console.WriteLine("Item Name: " + (refreshMsg.HasName ? refreshMsg.Name() : "<not set>"));
		Console.WriteLine("Service Name: " + (refreshMsg.HasServiceName ? refreshMsg.ServiceName() : "<not set>"));

		Console.WriteLine("Item State: " + refreshMsg.State());

		Decode(refreshMsg);
			
		Console.WriteLine();
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
	public void OnGenericMsg(GenericMsg genericMsg, IOmmConsumerEvent _){}
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
			//APIQA
			Console.WriteLine("Action: " + mapEntry.MapActionAsString() + ", key = " + mapEntry.Key.UInt());  
			//END APIQA
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
	public static void Main(String[] args)
	{
		OmmConsumer? consumer = null;
		try
		{
			AppClient appClient = new();
			
			consumer  = new(new OmmConsumerConfig().Host("localhost:14002").UserName("user"));
			
			RequestMsg reqMsg = new();

			//APIQA

			int closure = 1;

			if (args[0].Equals("-m", StringComparison.OrdinalIgnoreCase))
			{
				int temp = int.Parse(args[1]);

				switch (temp)
				{
					case 0:
						Console.WriteLine("APIQA: Requesting directory without service name specified or filter specified\n\n");
						consumer.RegisterClient(reqMsg.DomainType(EmaRdm.MMT_DIRECTORY), appClient);
						break;
					case 1:
						Console.WriteLine("APIQA: Requesting directory with service name of DIRECT_FEED specified and no filter specified\n\n");
						consumer.RegisterClient(reqMsg.DomainType(EmaRdm.MMT_DIRECTORY).ServiceName("DIRECT_FEED"), appClient);
						break;
					case 2:
						Console.WriteLine("APIQA: Requesting directory with service name of DF415 specified and no filter specified\n\n");
						consumer.RegisterClient(reqMsg.DomainType(EmaRdm.MMT_DIRECTORY).ServiceName("DF415"), appClient);
						break;
					case 3:
						Console.WriteLine("APIQA: Requesting directory without service name and with filter 0 specified\n\n");
						consumer.RegisterClient(reqMsg.DomainType(EmaRdm.MMT_DIRECTORY).Filter(0), appClient);
						break;
					case 4:
						Console.WriteLine("APIQA: Requesting directory without service name and with filter SERVICE_INFO_FILTER specified\n\n");
						consumer.RegisterClient(reqMsg.DomainType(EmaRdm.MMT_DIRECTORY).Filter(EmaRdm.SERVICE_INFO_FILTER), appClient);
						break;
					case 5:
						Console.WriteLine("APIQA: Requesting directory without service name and with filter SERVICE_STATE_FILTER specified\n\n");
						consumer.RegisterClient(reqMsg.DomainType(EmaRdm.MMT_DIRECTORY).Filter(EmaRdm.SERVICE_STATE_FILTER), appClient);
						break;
					case 6:
						Console.WriteLine("APIQA: Requesting directory without service name and with filter SERVICE_GROUP_FILTER specified\n\n");
						consumer.RegisterClient(reqMsg.DomainType(EmaRdm.MMT_DIRECTORY).Filter(EmaRdm.SERVICE_GROUP_FILTER), appClient);
						break;
					case 7:
						Console.WriteLine("APIQA: Requesting directory without service name and with filter SERVICE_LOAD_FILTER specified\n\n");
						consumer.RegisterClient(reqMsg.DomainType(EmaRdm.MMT_DIRECTORY).Filter(EmaRdm.SERVICE_LOAD_FILTER), appClient);
						break;
					case 8:
						Console.WriteLine("APIQA: Requesting directory without service name and with filter SERVICE_DATA_FILTER specified\n\n");
						consumer.RegisterClient(reqMsg.DomainType(EmaRdm.MMT_DIRECTORY).Filter(EmaRdm.SERVICE_DATA_FILTER), appClient);
						break;
					case 9:
						Console.WriteLine("APIQA: Requesting directory without service name and with filter SERVICE_LINK_FILTER specified\n\n");
						consumer.RegisterClient(reqMsg.DomainType(EmaRdm.MMT_DIRECTORY).Filter(EmaRdm.SERVICE_LINK_FILTER), appClient);
						break;
					case 10:
						Console.WriteLine("APIQA: Requesting directory with service id of 8090 specified and no filter specified\n\n");
						consumer.RegisterClient(reqMsg.DomainType(EmaRdm.MMT_DIRECTORY).ServiceId(8090), appClient);
						break;
					case 11:
						Console.WriteLine("APIQA: Requesting directory with service id of 8090 specified and with filter 0 specified\n\n");
						consumer.RegisterClient(reqMsg.DomainType(EmaRdm.MMT_DIRECTORY).ServiceId(8090).Filter(0), appClient);
						break;
					case 12:
						Console.WriteLine("APIQA: Requesting directory with service id of 8090 specified and with filter 29 specified\n\n");
						consumer.RegisterClient(reqMsg.DomainType(EmaRdm.MMT_DIRECTORY).ServiceId(8090).Filter(29), appClient);
						break;
					case 13:
						Console.WriteLine("APIQA: Requesting directory with service name of DIRECT_FEED specified, name of IBM.N specified, and no filter specified\n\n");
						consumer.RegisterClient(reqMsg.Clear().ServiceName("DIRECT_FEED").Name("IBM.N"), appClient);
						break;
					case 14:
						Console.WriteLine("APIQA: Requesting directory with service name of DF415 specified, name of JPY= specified, and no filter specified\n\n");
						consumer.RegisterClient(reqMsg.Clear().ServiceName("DF415").Name("JPY="), appClient, closure);
						break;
					case 15:
						Console.WriteLine("APIQA: Requesting directory with service id of 8090 specified, name of JPY= specified, and no filter specified\n\n");
						consumer.RegisterClient(reqMsg.Clear().ServiceId(8090).Name("JPY="), appClient, closure);
						break;
					default:
						Console.WriteLine("APIQA: Requesting directory with service id of 8090 specified and with filter 0 specified\n\n");
						consumer.RegisterClient(reqMsg.DomainType(EmaRdm.MMT_DIRECTORY).ServiceId(8090).Filter(0), appClient);
						break;
				}
			}
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
