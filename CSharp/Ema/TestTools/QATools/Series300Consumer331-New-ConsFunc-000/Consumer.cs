/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Access;
using LSEG.Ema.Domain.Directory;
using LSEG.Ema.Rdm;
using System;
using System.Threading;
using static LSEG.Ema.Access.DataType;

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
            Console.WriteLine("Item State: " + statusMsg.State());

        Decode(statusMsg);

        Console.WriteLine();
    }

    public void OnAckMsg(AckMsg ackMsg, IOmmConsumerEvent _)
    { }

    public void OnGenericMsg(GenericMsg genericMsg, IOmmConsumerEvent _)
    { }

    public void OnAllMsg(Msg msg, IOmmConsumerEvent _)
    { }

    private void Decode(Msg msg)
    {
        if (msg.DomainType() == EmaRdm.MMT_DIRECTORY)
        {
            switch (msg)
            {
                case RefreshMsg refreshMsg:
                    Console.WriteLine(new DirectoryRefreshMsg().Message(refreshMsg).ToString());
                    return;
                case UpdateMsg updateMsg:
                    Console.WriteLine(new DirectoryUpdateMsg().Message(updateMsg).ToString());
                    return;
                case StatusMsg statusMsg:
                    Console.WriteLine(new DirectoryStatusMsg().Message(statusMsg).ToString());
                    return;
            }
        }

        switch (msg.Payload().DataType)
        {
            case DataTypes.FIELD_LIST:
                Decode(msg.Payload().FieldList());
                break;

            default:
                break;
        }
    }

    private void Decode(FieldList fieldList)
    {
        foreach (FieldEntry fieldEntry in fieldList)
        {
            Console.Write("Fid: " + fieldEntry.FieldId + " Name = " + fieldEntry.Name + " DataType: " + DataType.AsString(fieldEntry.Load!.DataType) + " Value: ");

            if (Data.DataCode.BLANK == fieldEntry.Code)
                Console.WriteLine(" blank");
            else
                switch (fieldEntry.LoadType)
                {
                    case DataTypes.REAL:
                        Console.WriteLine(fieldEntry.OmmRealValue().AsDouble());
                        break;

                    case DataTypes.DATE:
                        Console.WriteLine(fieldEntry.OmmDateValue().Day + " / " + fieldEntry.OmmDateValue().Month + " / " + fieldEntry.OmmDateValue().Year);
                        break;

                    case DataTypes.TIME:
                        Console.WriteLine(fieldEntry.OmmTimeValue().Hour + ":" + fieldEntry.OmmTimeValue().Minute + ":" + fieldEntry.OmmTimeValue().Second + ":" + fieldEntry.OmmTimeValue().Millisecond);
                        break;

                    case DataTypes.INT:
                        Console.WriteLine(fieldEntry.IntValue());
                        break;

                    case DataTypes.UINT:
                        Console.WriteLine(fieldEntry.UIntValue());
                        break;

                    case DataTypes.ASCII:
                        Console.WriteLine(fieldEntry.OmmAsciiValue());
                        break;

                    case DataTypes.ENUM:
                        Console.WriteLine(fieldEntry.HasEnumDisplay ? fieldEntry.EnumDisplay() : fieldEntry.EnumValue());
                        break;

                    case DataTypes.ARRAY:
                        Console.WriteLine(fieldEntry.OmmArrayValue());
                        break;

                    case DataTypes.RMTES:
                        Console.WriteLine(fieldEntry.OmmRmtesValue());
                        break;

                    case DataTypes.ERROR:
                        Console.WriteLine(fieldEntry.OmmErrorValue().ErrorCode + " (" + fieldEntry.OmmErrorValue().ErrorCodeAsString() + ")");
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
	public static String _SERVICE = "DIRECT_FEED";
	public static String _ITEM = "IBM.N";
	public static String _HOST = "localhost";
	public static String _PORT = "14002";


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
			"    -service :  Service name \n\n" +
            "    -host : Host address \n\n" +
            "    -p : Port \n\n" +
            "    -item : item \n\n" +
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
			else if (0 == argv[idx].CompareTo("-service"))
			{
				if (++idx >= count)
				{
					PrintHelp();
					return false;
				}
				Consumer._SERVICE = argv[idx];
				++idx;
			}
			else if (0 == argv[idx].CompareTo("-host"))
			{
				if (++idx >= count)
				{
					PrintHelp();
					return false;
				}
				Consumer._HOST = argv[idx];
				++idx;
			}
			else if (0 == argv[idx].CompareTo("-p"))
			{
				if (++idx >= count)
				{
					PrintHelp();
					return false;
				}
				Consumer._PORT = argv[idx];
				++idx;
			}
			else if (0 == argv[idx].CompareTo("-item"))
			{
				if (++idx >= count)
				{
					PrintHelp();
					return false;
				}
				Consumer._ITEM = argv[idx];
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

            consumer = new(new OmmConsumerConfig().Host(Consumer._HOST + ":" + Consumer._PORT).UserName("user"));

            RequestMsg reqMsg = new();
			DirectoryRequestMsg dirReqMsg = new();


           			//APIQA
			switch (Consumer._OPTION)
			{
				default:
				case 0:
				case 5:
					if (Consumer._FILTER >= 0)
					{
						Console.WriteLine("********APIQA: Requesting directory without service name, service id, and filter=" + Consumer._FILTER + "\n\n");
						consumer.RegisterClient(dirReqMsg.Filter((DirectoryFilters)Consumer._FILTER), appClient);
					}
					else
					{
						Console.WriteLine("********APIQA: Requesting directory without service name, service id\n\n");
						consumer.RegisterClient(dirReqMsg, appClient);
					}
					break;
				case 1:
				case 2:
					if (Consumer._FILTER >= 0)
					{
						Console.WriteLine("********APIQA: Requesting directory with service=" + Consumer._SERVICE + " and filter=" + Consumer._FILTER + "\n\n");
						consumer.RegisterClient(dirReqMsg.ServiceName(Consumer._SERVICE).Filter((DirectoryFilters)Consumer._FILTER), appClient);
					}
					else
					{
						Console.WriteLine("********APIQA: Requesting directory with service=" + Consumer._SERVICE + "\n\n");
						consumer.RegisterClient(dirReqMsg.ServiceName(Consumer._SERVICE), appClient);
					}
					break;
				case 3:
				case 4:
					if (Consumer._FILTER >= 0)
					{
						Console.WriteLine("********APIQA: Requesting directory with service=serviceID and filter=" + Consumer._FILTER + "\n\n");
						consumer.RegisterClient(dirReqMsg.ServiceId(8090).Filter((DirectoryFilters)Consumer._FILTER), appClient);
					}
					else
					{
						Console.WriteLine("********APIQA: Requesting directory with service=serviceID\n\n");
						consumer.RegisterClient(dirReqMsg.ServiceId(8090), appClient);
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
					Console.WriteLine("********APIQA: Requesting item wth service=" + Consumer._SERVICE + "\n\n");
					consumer.RegisterClient(reqMsg.Clear().ServiceName(Consumer._SERVICE).Name(Consumer._ITEM), appClient);
				}
				else
				{
					Console.WriteLine("********APIQA: Requesting item wth service=serviceID\n\n");
					consumer.RegisterClient(reqMsg.Clear().ServiceId(8090).Name(Consumer._ITEM), appClient);
				}
			}
			//END APIQA

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
