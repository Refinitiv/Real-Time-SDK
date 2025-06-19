/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Access;
using System;
using System.Threading;
using static LSEG.Ema.Access.DataType;

namespace LSEG.Ema.Example.Traning.Consumer;

internal class AppClient : IOmmConsumerClient
{
    public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmConsumerEvent @event)
    {
        Console.WriteLine("Item Handle: " + @event.Handle + " Closure: " + @event.Closure!.GetHashCode());

        Decode(refreshMsg);

        Console.WriteLine();
    }

    public void OnUpdateMsg(UpdateMsg updateMsg, IOmmConsumerEvent @event)
    {
        Console.WriteLine("Item Handle: " + @event.Handle + " Closure: " + @event.Closure?.GetHashCode());

        Decode(updateMsg);

        Console.WriteLine();
    }

    public void OnStatusMsg(StatusMsg statusMsg, IOmmConsumerEvent @event)
    {
        Console.WriteLine("Item Name: " + (statusMsg.HasName ? statusMsg.Name() : "<not set>"));
        Console.WriteLine("Service Name: " + (statusMsg.HasServiceName ? statusMsg.ServiceName() : "<not set>"));

        Console.WriteLine("Item Handle: " + @event.Handle + " Closure: " + @event.Closure?.GetHashCode());

        if (statusMsg.HasState)
            Console.WriteLine("Item State: " + statusMsg.State());

        Console.WriteLine();
    }

    private static void Decode(RefreshMsg refreshMsg)
    {
        Console.WriteLine("Item Name: " + (refreshMsg.HasName ? refreshMsg.Name() : "<not set>"));
        Console.WriteLine("Service Name: " + (refreshMsg.HasServiceName ? refreshMsg.ServiceName() : "<not set>"));

        Console.WriteLine("Item State: " + refreshMsg.State());

        DecodeAttrib(refreshMsg.Attrib());

        DecodePayload(refreshMsg.Payload());
    }

    private static void Decode(UpdateMsg updateMsg)
    {
        Console.WriteLine("Item Name: " + (updateMsg.HasName ? updateMsg.Name() : "<not set>"));
        Console.WriteLine("Service Name: " + (updateMsg.HasServiceName ? updateMsg.ServiceName() : "<not set>"));
        DecodeAttrib(updateMsg.Attrib());
        DecodePayload(updateMsg.Payload());
    }

    private static void DecodeAttrib(ComplexTypeData attrib)
    {
        Console.WriteLine("Attribute");

        switch (attrib.DataType)
        {
            case DataTypes.FIELD_LIST:
                Decode(attrib.FieldList());
                break;

            case DataTypes.MAP:
                Decode(attrib.Map());
                break;

            default:
                break;
        }
    }

    private static void DecodePayload(ComplexTypeData payload)
    {
        Console.WriteLine("Payload");

        switch (payload.DataType)
        {
            case DataTypes.FIELD_LIST:
                Decode(payload.FieldList());
                break;

            case DataTypes.MAP:
                Decode(payload.Map());
                break;

            case DataTypes.REFRESH_MSG:
                Decode(payload.RefreshMsg());
                break;

            case DataTypes.UPDATE_MSG:
                Decode(payload.UpdateMsg());
                break;

            default:
                break;
        }
    }

    private static void Decode(Map map)
    {
        switch (map.SummaryData().DataType)
        {
            case DataTypes.FIELD_LIST:
                Decode(map.SummaryData().FieldList());
                break;

            case DataTypes.MAP:
                Decode(map.SummaryData().Map());
                break;

            case DataTypes.REFRESH_MSG:
                Decode(map.SummaryData().RefreshMsg());
                break;

            case DataTypes.UPDATE_MSG:
                Decode(map.SummaryData().UpdateMsg());
                break;

            default:
                break;
        }

        foreach (MapEntry mapEntry in map)
        {
            switch (mapEntry.Key.DataType)
            {
                case DataTypes.BUFFER:
                    Console.WriteLine("Action: " + mapEntry.MapActionAsString() + ", key value: " + mapEntry.Key.Buffer() + "\n");
                    break;

                case DataTypes.ASCII:
                    Console.WriteLine("Action: " + mapEntry.MapActionAsString() + ", key value: " + mapEntry.Key.Ascii() + "\n");
                    break;

                case DataTypes.RMTES:
                    Console.WriteLine("Action: " + mapEntry.MapActionAsString() + ", key value: " + mapEntry.Key.Rmtes() + "\n");
                    break;

                default:
                    break;
            }

            switch (mapEntry.LoadType)
            {
                case DataTypes.FIELD_LIST:
                    Decode(mapEntry.FieldList());
                    break;

                case DataTypes.MAP:
                    Decode(mapEntry.Map());
                    break;

                case DataTypes.REFRESH_MSG:
                    Decode(mapEntry.RefreshMsg());
                    break;

                case DataTypes.UPDATE_MSG:
                    Decode(mapEntry.UpdateMsg());
                    break;

                default:
                    break;
            }
        }
    }

    private static void Decode(FieldList fieldList)
    {
        foreach (FieldEntry fieldEntry in fieldList)
        {
            Console.Write("Fid: " + fieldEntry.FieldId + " Name = " + fieldEntry.Name + " DataType: " + DataType.AsString(fieldEntry.Load!.DataType) + " Value: ");

            if (Data.DataCode.BLANK == fieldEntry.Code)
                Console.WriteLine(" blank");
            else
                switch (fieldEntry.LoadType)
                {
                    case DataTypes.FIELD_LIST:
                        Console.WriteLine(",  contains FieldList.");
                        Decode(fieldEntry.FieldList());
                        break;

                    case DataTypes.MAP:
                        Console.WriteLine(",  contains map.");
                        Decode(fieldEntry.Map());
                        break;

                    case DataTypes.REFRESH_MSG:
                        Console.WriteLine(",  contains refresh message.");
                        Decode(fieldEntry.RefreshMsg());
                        break;

                    case DataTypes.UPDATE_MSG:
                        Console.WriteLine(",  contains update message.");
                        Decode(fieldEntry.UpdateMsg());
                        break;

                    case DataTypes.REAL:
                        Console.WriteLine(fieldEntry.OmmRealValue().AsDouble());
                        break;

                    case DataTypes.DATE:
                        Console.WriteLine(fieldEntry.OmmDateValue().Day + " / " + fieldEntry.OmmDateValue().Month + " / " + fieldEntry.OmmDateValue().Year);
                        break;

                    case DataTypes.TIME:
                        {
                            OmmTime ommTime = fieldEntry.OmmTimeValue();
                            Console.WriteLine($"{ommTime.Hour}:{ommTime.Minute}:{ommTime.Second}:{ommTime.Millisecond}");
                            break;
                        }

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
    public static void Main()
    {
        OmmConsumer? consumer = null;
        try
        {
            consumer = new OmmConsumer(new OmmConsumerConfig().Host("localhost:14002").UserName("user"));
            // request a custom domain (133) item IBM.XYZ
            consumer.RegisterClient(new RequestMsg().DomainType(133).ServiceName("DIRECT_FEED").Name("IBM.XYZ"), new AppClient(), 1);

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