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
		Console.WriteLine("Received Refresh. Item Handle: " + @event.Handle + " Closure: " + @event.Closure);
		
		Console.WriteLine("Item Name: " + (refreshMsg.HasName ? refreshMsg.Name() : "<not set>"));
		Console.WriteLine("Service Name: " + (refreshMsg.HasServiceName ? refreshMsg.ServiceName() : "<not set>"));

		Console.WriteLine("Item State: " + refreshMsg.State());

		Decode(refreshMsg);
			
		Console.WriteLine();
	}
	
	public void OnUpdateMsg(UpdateMsg updateMsg, IOmmConsumerEvent @event) 
	{
		Console.WriteLine("Received Update. Item Handle: " + @event.Handle + " Closure: " + @event.Closure);
		
		Console.WriteLine("Item Name: " + (updateMsg.HasName ? updateMsg.Name() : "<not set>"));
		Console.WriteLine("Service Name: " + (updateMsg.HasServiceName ? updateMsg.ServiceName() : "<not set>"));
		
		Decode(updateMsg);
		
		Console.WriteLine();
	}

	public void OnStatusMsg(StatusMsg statusMsg, IOmmConsumerEvent @event) 
	{
		Console.WriteLine("Received Status. Item Handle: " + @event.Handle + " Closure: " + @event.Closure);

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

	//APIQA
	public static int _OPTION = 0;
	public static int _FILTER = -1;
	public static int _SLEEPTIME = 0;

	public static void PrintHelp()
	{
		Console.WriteLine("\nOptions:\n" +
			"  -?\t\t\tShows this usage\n\n" +
			"  -f <source directory filter in decimal; default = no filter is specified>\n" +
			"     Possible values for filter, valid range = 0-63:\n" +
			"     0 :  No Filter \n" +
			"     1 :  SERVICE_INFO_FILTER 0x01 \n" +
			"     2 :  SERVICE_STATE_FILTER 0x02 \n" +
			"     4 :  SERVICE_GROUP_FILTER 0x04 \n" +
			"     8 :  SERVICE_LOAD_FILTER 0x08 \n" +
			"    16 :  SERVICE_DATA_FILTER 0x10 \n" +
			"    32 :  SERVICE_LINK_FILTER 0x20 \n" +
			"    ?? :  Mix of above values upto 63 \n\n" +
			"  -m <option>; default = option 0\n" +
			"     Possible values for option, valid range = 0-4:\n" +
			"     0 :  Request source directory without serviceName or serviceId\n" +
			"     1 :  Request source directory with serviceName\n" +
			"     2 :  Request source directory with serviceName; Request item on that service\n" +
			"     3 :  Request source directory with serviceId\n" +
			"     4 :  Request source directory with serviceId; Request item on that service\n\n" +
			"  -s <amount of time to wait before requesting an item in seconds; default = no wait>\n" +
			"     This option only applies to -m 2 or -m 4\n" +
			" \n");
	}

	public static bool ReadCommandlineArgs(String[] argv)
	{
		int count = argv.Length;
		int idx = 0;

		while (idx < count)
		{
			if (0 == argv[idx].CompareTo("-?"))
			{
				PrintHelp();
				return false;
			}
			else if (0 == argv[idx].CompareTo("-f"))
			{
				if (++idx >= count)
				{
					PrintHelp();
					return false;
				}
				Consumer._FILTER = int.Parse(argv[idx]);
				++idx;
			}
			else if (0 == argv[idx].CompareTo("-m"))
			{
				if (++idx >= count)
				{
					PrintHelp();
					return false;
				}
				Consumer._OPTION = int.Parse(argv[idx]);
				++idx;
			}
			else if (0 == argv[idx].CompareTo("-s"))
			{
				if (++idx >= count)
				{
					PrintHelp();
					return false;
				}
				Consumer._SLEEPTIME = int.Parse(argv[idx]);
				++idx;
			}
			else
			{
				PrintHelp();
				return false;
			}
		}
		return true;
	}

	//END APIQA
	public static void Main(String[] args)
	{
		OmmConsumer? consumer = null;
		try
		{
			//APIQA
			if (!ReadCommandlineArgs(args)) return;
			//END APIQA
			AppClient appClient = new();
			
			consumer  = new(new OmmConsumerConfig().Host("localhost:14002").UserName("user"));
			
			RequestMsg reqMsg = new();

			//APIQA
			switch (Consumer._OPTION)
			{
				default:
				case 0:
				case 5:
					if (Consumer._FILTER >= 0)
					{
						Console.WriteLine("********APIQA: Requesting directory without service name, service id, and filter=" + Consumer._FILTER + "\n\n");
						consumer.RegisterClient(reqMsg.DomainType(EmaRdm.MMT_DIRECTORY).Filter(Consumer._FILTER), appClient);
					}
					else
					{
						Console.WriteLine("********APIQA: Requesting directory without service name, service id\n\n");
						consumer.RegisterClient(reqMsg.DomainType(EmaRdm.MMT_DIRECTORY), appClient);
					}
					break;
				case 1:
				case 2:
					if (Consumer._FILTER >= 0)
					{
						Console.WriteLine("********APIQA: Requesting directory with service=DIRECT_FEED and filter=" + Consumer._FILTER + "\n\n");
						consumer.RegisterClient(reqMsg.DomainType(EmaRdm.MMT_DIRECTORY).ServiceName("DIRECT_FEED").Filter(Consumer._FILTER), appClient);
					}
					else
					{
						Console.WriteLine("********APIQA: Requesting directory with service=DIRECT_FEED\n\n");
						consumer.RegisterClient(reqMsg.DomainType(EmaRdm.MMT_DIRECTORY).ServiceName("DIRECT_FEED"), appClient);
					}
					break;
				case 3:
				case 4:
					if (Consumer._FILTER >= 0)
					{
						Console.WriteLine("********APIQA: Requesting directory with service=serviceID and filter=" + Consumer._FILTER + "\n\n");
						consumer.RegisterClient(reqMsg.DomainType(EmaRdm.MMT_DIRECTORY).ServiceId(8090).Filter(Consumer._FILTER), appClient);
					}
					else
					{
						Console.WriteLine("********APIQA: Requesting directory with service=serviceID\n\n");
						consumer.RegisterClient(reqMsg.DomainType(EmaRdm.MMT_DIRECTORY).ServiceId(8090), appClient);
					}
					break;
			}
			if ((Consumer._OPTION == 2) || (Consumer._OPTION == 4) || (Consumer._OPTION == 5))
			{
				if (Consumer._SLEEPTIME > 0)
				{
					Console.WriteLine("********APIQA: Sleeping (in seconds): " + Consumer._SLEEPTIME + "\n");
					Thread.Sleep(Consumer._SLEEPTIME * 1000);            // API calls onRefreshMsg(), onUpdateMsg() and onStatusMsg()
				}
				if ((Consumer._OPTION == 2) || (Consumer._OPTION == 5))
				{
					Console.WriteLine("********APIQA: Requesting item wth service=DIRECT_FEED\n\n");
					consumer.RegisterClient(reqMsg.Clear().ServiceName("DIRECT_FEED").Name("IBM.N"), appClient);
				}
				else
				{
					Console.WriteLine("********APIQA: Requesting item wth service=serviceID\n\n");
					consumer.RegisterClient(reqMsg.Clear().ServiceId(8090).Name("IBM.N"), appClient);
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
