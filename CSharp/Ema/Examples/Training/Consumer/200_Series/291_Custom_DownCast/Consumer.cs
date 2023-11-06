/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.Md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Access;
using System;

namespace LSEG.Ema.Example.Traning.Consumer;

internal class AppClient : IOmmConsumerClient
{
    public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmConsumerEvent ommConsumerEvent)
    {
        Console.WriteLine("Received Refresh. Item Handle: " + ommConsumerEvent.Handle + " Closure: " + ommConsumerEvent.Closure?.GetHashCode());

        DecodeRefreshMsg(refreshMsg);

        Console.WriteLine();
    }

    public void OnUpdateMsg(UpdateMsg updateMsg, IOmmConsumerEvent @event)
    {
        Console.WriteLine("Received Update. Item Handle: " + @event.Handle + " Closure: " + @event.Closure?.GetHashCode());

		DecodeUpdateMsg(updateMsg);
        Console.WriteLine();
	}

    public void OnStatusMsg(StatusMsg statusMsg, IOmmConsumerEvent @event)
    {
        Console.WriteLine("Received Status. Item Handle: " + @event.Handle + " Closure: " + @event.Closure?.GetHashCode());

        DecodeStatusMsg(statusMsg);

        Console.WriteLine();
    }

void DecodeRefreshMsg(RefreshMsg refreshMsg)
{
    Console.WriteLine("Item Name: " + (refreshMsg.HasName ? refreshMsg.Name() : "<not set>"));
    Console.WriteLine("Service Name: " + (refreshMsg.HasServiceName ? refreshMsg.ServiceName() : "<not set>"));

    Console.WriteLine("Item State: " + refreshMsg.State());

    Console.WriteLine("Attribute");
    Decode(refreshMsg.Attrib().Data);

    Console.WriteLine("Payload");
    Decode(refreshMsg.Payload().Data);
}

void DecodeUpdateMsg(UpdateMsg updateMsg)
{
    Console.WriteLine("Item Name: " + (updateMsg.HasName ? updateMsg.Name() : "<not set>"));
    Console.WriteLine("Service Name: " + (updateMsg.HasServiceName ? updateMsg.ServiceName() : "<not set>"));

    Console.WriteLine("Attribute");
    Decode(updateMsg.Attrib().Data);

    Console.WriteLine("Payload");
    Decode(updateMsg.Payload().Data);
}

void DecodeStatusMsg(StatusMsg statusMsg)
{
    Console.WriteLine("Item Name: " + (statusMsg.HasName ? statusMsg.Name() : "<not set>"));
    Console.WriteLine("Service Name: " + (statusMsg.HasServiceName ? statusMsg.ServiceName() : "<not set>"));

    if (statusMsg.HasState)
        Console.WriteLine("Item State: " + statusMsg.State());
}

void Decode(Data data)
{
    if (Data.DataCode.BLANK == data.Code)
        Console.WriteLine("Blank data");
    else
        switch (data.DataType)
        {
            case DataType.DataTypes.REFRESH_MSG:
                DecodeRefreshMsg((RefreshMsg)data);
                break;

            case DataType.DataTypes.UPDATE_MSG:
                DecodeUpdateMsg((UpdateMsg)data);
                break;

            case DataType.DataTypes.STATUS_MSG:
                    DecodeStatusMsg((StatusMsg)data);
                break;

            case DataType.DataTypes.FIELD_LIST:
                    DecodeFieldList((FieldList)data);
                break;

            case DataType.DataTypes.MAP:
                DecodeMap((Map)data);
                break;

            case DataType.DataTypes.NO_DATA:
                Console.WriteLine("NoData");
                break;

            case DataType.DataTypes.TIME:
                Console.WriteLine("OmmTime: " + ((OmmTime)data).ToString());
                break;

            case DataType.DataTypes.DATE:
                Console.WriteLine("OmmDate: " + ((OmmDate)data).ToString());
                break;

            case DataType.DataTypes.REAL:
                Console.WriteLine("OmmReal::asDouble: " + ((OmmReal)data).AsDouble());
                break;

            case DataType.DataTypes.INT:
                Console.WriteLine("OmmInt: " + ((OmmInt)data).Value);
                break;

            case DataType.DataTypes.UINT:
                Console.WriteLine("OmmUInt: " + ((OmmUInt)data).Value);
                break;

            case DataType.DataTypes.ENUM:
                Console.WriteLine("OmmEnum: " + ((OmmEnum)data).Value);
                break;

            case DataType.DataTypes.ASCII:
                Console.WriteLine("OmmAscii: " + ((OmmAscii)data).Value);
                break;

            case DataType.DataTypes.RMTES:
                Console.WriteLine("OmmRmtes: " + ((OmmRmtes)data).Value);
                break;

            case DataType.DataTypes.ERROR:
                Console.WriteLine("Decoding error: " + ((OmmError)data).ErrorCodeAsString());
                break;

            default:
                break;
        }
}

void DecodeMap(Map map)
{
    Console.WriteLine("Map Summary");
    Decode(map.SummaryData().Data);

    foreach (MapEntry mapEntry in map)
    {
        Console.WriteLine("Action = " + mapEntry.MapActionAsString());

        Console.WriteLine("Key");
        Decode(mapEntry.Key.Data);

        Console.WriteLine("Load");
        Decode(mapEntry.Load!);
    }
}

void DecodeFieldList(FieldList fieldList)
{
    if (fieldList.HasInfo)
        Console.WriteLine("FieldListNum: " + fieldList.InfoFieldListNum() + " DictionaryId: " + fieldList.InfoDictionaryId());

    foreach (FieldEntry fieldEntry in fieldList)
    {
        Console.WriteLine("Load");
        Decode(fieldEntry.Load!);
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