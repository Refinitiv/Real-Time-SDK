/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Access;

namespace LSEG.Ema.Example.Traning.Consumer;

using LSEG.Ema.Access;
using LSEG.Ema.Rdm;
using System;
using System.Threading;
using static LSEG.Ema.Access.DataType;

//APIQA this file is QATools standalone. See qa_readme.txt for details about this tool.
internal class AppClient : IOmmConsumerClient
{
    public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmConsumerEvent @event)
    {
        Console.WriteLine("Received Refresh. Item Handle: " +@event.Handle + " Closure: " +@event.Closure);

        Console.WriteLine("Item Name: " + (refreshMsg.HasName ? refreshMsg.Name() : "<not set>"));
        Console.WriteLine("Service Name: " + (refreshMsg.HasServiceName ? refreshMsg.ServiceName() : "<not set>"));

        Console.WriteLine("Item State: " + refreshMsg.State());

        Decode(refreshMsg);

        Console.WriteLine();
    }

    public void OnUpdateMsg(UpdateMsg updateMsg, IOmmConsumerEvent @event)
    {
        Console.WriteLine("Received Update. Item Handle: " +@event.Handle + " Closure: " +@event.Closure);

        Console.WriteLine("Item Name: " + (updateMsg.HasName ? updateMsg.Name() : "<not set>"));
        Console.WriteLine("Service Name: " + (updateMsg.HasServiceName ? updateMsg.ServiceName() : "<not set>"));

        Decode(updateMsg);

        Console.WriteLine();
    }

    public void OnStatusMsg(StatusMsg statusMsg, IOmmConsumerEvent @event)
    {
        Console.WriteLine("Received Status. Item Handle: " +@event.Handle + " Closure: " +@event.Closure);

        Console.WriteLine("Item Name: " + (statusMsg.HasName ? statusMsg.Name() : "<not set>"));
        Console.WriteLine("Service Name: " + (statusMsg.HasServiceName ? statusMsg.ServiceName() : "<not set>"));

        if (statusMsg.HasState)
            Console.WriteLine("Item State: " + statusMsg.State());

        Console.WriteLine();
    }

    void Decode(Msg msg)
    {
        switch (msg.Attrib().DataType)
        {
            case DataTypes.ELEMENT_LIST:
                Decode(msg.Attrib().ElementList());
                break;
        }

        switch (msg.Payload().DataType)
        {
            case DataTypes.SERIES:
                Decode(msg.Payload().Series());
                break;
            case DataTypes.FIELD_LIST:
                Decode(msg.Payload().FieldList());
                break;
            case DataTypes.ELEMENT_LIST:
                Decode(msg.Payload().ElementList());
                break;
        }
    }

    void Decode(ElementList elementList)
    {
        foreach (ElementEntry elementEntry in elementList)
        {
            Console.Write(" Name = " + elementEntry.Name + " DataType: " + AsString(elementEntry.Load!.DataType) + " Value: ");

            if (Data.DataCode.BLANK == elementEntry.Code)
                Console.WriteLine(" blank");
            else
                switch (elementEntry.LoadType)
                {
                    case DataTypes.REAL:
                        Console.WriteLine(elementEntry.OmmRealValue().AsDouble());
                        break;
                    case DataTypes.DATE:
                        Console.WriteLine(elementEntry.OmmDateValue().Day + " / " + elementEntry.OmmDateValue().Month + " / " + elementEntry.OmmDateValue().Year);
                        break;
                    case DataTypes.TIME:
                        Console.WriteLine(elementEntry.OmmTimeValue().Hour + ":" + elementEntry.OmmTimeValue().Minute + ":" + elementEntry.OmmTimeValue().Second + ":" + elementEntry.OmmTimeValue().Millisecond);
                        break;
                    case DataTypes.INT:
                        Console.WriteLine(elementEntry.OmmIntValue());
                        break;
                    case DataTypes.UINT:
                        Console.WriteLine(elementEntry.OmmUIntValue());
                        break;
                    case DataTypes.ASCII:
                        Console.WriteLine(elementEntry.OmmAsciiValue());
                        break;
                    case DataTypes.ENUM:
                        Console.WriteLine(elementEntry.OmmEnumValue());
                        break;
                    case DataTypes.RMTES :
                        Console.WriteLine(elementEntry.OmmRmtesValue());
                        break;
                    case DataTypes.ERROR:
                        Console.WriteLine(elementEntry.OmmErrorValue().ErrorCode + " (" + elementEntry.OmmErrorValue().ErrorCodeAsString() + ")");
                        break;
                    case DataTypes.ARRAY:
                        Console.WriteLine();
                        Decode(elementEntry.OmmArrayValue());
                        break;
                    default:
                        Console.WriteLine();
                        break;
                }
        }
    }

    static void Decode(OmmArray array)
    {
        foreach (OmmArrayEntry arrayEntry in array)
        {
            Console.Write(" DataType: " + AsString(arrayEntry.Load.DataType) + " Value: ");

            if (Data.DataCode.BLANK == arrayEntry.Code)
                Console.WriteLine(" blank");
            else
                switch (arrayEntry.LoadType)
                {
                    case DataTypes.REAL:
                        Console.WriteLine(arrayEntry.OmmRealValue().AsDouble());
                        break;
                    case DataTypes.DATE:
                        Console.WriteLine(arrayEntry.OmmDateValue().Day + " / " + arrayEntry.OmmDateValue().Month + " / " + arrayEntry.OmmDateValue().Year);
                        break;
                    case DataTypes.TIME:
                        Console.WriteLine(arrayEntry.OmmTimeValue().Hour + ":" + arrayEntry.OmmTimeValue().Minute + ":" + arrayEntry.OmmTimeValue().Second + ":" + arrayEntry.OmmTimeValue().Millisecond);
                        break;
                    case DataTypes.INT:
                        Console.WriteLine(arrayEntry.OmmIntValue());
                        break;
                    case DataTypes.UINT:
                        Console.WriteLine(arrayEntry.OmmUIntValue());
                        break;
                    case DataTypes.ASCII:
                        Console.WriteLine(arrayEntry.OmmAsciiValue());
                        break;
                    case DataTypes.ENUM:
                        Console.WriteLine(arrayEntry.OmmEnumValue());
                        break;
                    case DataTypes.RMTES :
                        Console.WriteLine(arrayEntry.OmmRmtesValue());
                        break;
                    case DataTypes.ERROR:
                        Console.WriteLine(arrayEntry.OmmErrorValue().ErrorCode + " (" + arrayEntry.OmmErrorValue().ErrorCodeAsString() + ")");
                        break;
                    default:
                        Console.WriteLine();
                        break;
                }
        }
    }

    static void Decode(FieldList fieldList)
    {
        foreach (FieldEntry fieldEntry in fieldList)
        {
            Console.Write("Fid: " + fieldEntry.FieldId + " Name = " + fieldEntry.Name + " DataType: " + AsString(fieldEntry.Load!.DataType) + " Value: ");

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
                        Console.WriteLine(fieldEntry.OmmIntValue());
                        break;
                    case DataTypes.UINT:
                        Console.WriteLine(fieldEntry.OmmUIntValue());
                        break;
                    case DataTypes.ASCII:
                        Console.WriteLine(fieldEntry.OmmAsciiValue());
                        break;
                    case DataTypes.ENUM:
                        Console.WriteLine(fieldEntry.OmmEnumValue());
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

    void Decode(Series series)
    {
        switch (series.SummaryData().DataType)
        {
            case DataTypes.ELEMENT_LIST:
                Decode(series.SummaryData().ElementList());
                break;
        }

        foreach (SeriesEntry seriesEntry in series)
        {
            Console.WriteLine(" DataType: " + AsString(seriesEntry.LoadType) + " Value: ");

            switch (seriesEntry.LoadType)
            {
                case DataTypes.ELEMENT_LIST:
                    Decode(seriesEntry.ElementList());
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
        try
        {
            AppClient appClient = new();
            OmmConsumerConfig config = new();
            RequestMsg reqMsg = new();
            config.AddAdminMsg(reqMsg.Clear().DomainType(EmaRdm.MMT_DICTIONARY).Filter(EmaRdm.DICTIONARY_NORMAL).InterestAfterRefresh(false).Name("RWFFld"));
            config.AddAdminMsg(reqMsg.Clear().DomainType(EmaRdm.MMT_DICTIONARY).Filter(EmaRdm.DICTIONARY_NORMAL).InterestAfterRefresh(false).Name("RWFEnum"));
            config.UserName("user");
            OmmConsumer consumer = new(config);
            long handle = consumer.RegisterClient(reqMsg.Clear().ServiceName("DIRECT_FEED").Name("JPY="), appClient);
            Thread.Sleep(300000); // API calls OnRefreshMsg(), OnUpdateMsg() and
                                  // OnStatusMsg()
            consumer.Uninitialize();
        }
        catch (OmmException ommException)
        {
            Console.WriteLine(ommException.Message);
        }
    }
}

//END APIQA
