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
using System.Collections.Generic;
using System.Threading;
using static LSEG.Ema.Access.DataType;

//APIQA this file is QATools standalone. See qa_readme.txt for details about this tool.
internal class AppClient : IOmmConsumerClient
{
    public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmConsumerEvent @event)
    {
        if (refreshMsg.HasMsgKey)
        {
            Console.WriteLine("Item Name: " + (refreshMsg.HasName ? refreshMsg.Name() : "<not set>"));
            Console.WriteLine("Service Name: " + (refreshMsg.HasServiceName ? refreshMsg.ServiceName() : "<not set>"));

        }

        Console.WriteLine("Item State: " + refreshMsg.State());

        if (DataTypes.FIELD_LIST == refreshMsg.Payload().DataType)
            Decode(refreshMsg.Payload().FieldList());

        Console.WriteLine(refreshMsg);

        Console.WriteLine();
    }

    public void OnUpdateMsg(UpdateMsg updateMsg, IOmmConsumerEvent @event)
    {
        if (updateMsg.HasMsgKey)
        {
            Console.WriteLine("Item Name: " + (updateMsg.HasName ? updateMsg.Name() : "<not set>"));
            Console.WriteLine("Service Name: " + (updateMsg.HasServiceName ? updateMsg.ServiceName() : "<not set>"));
        }

        if (DataTypes.FIELD_LIST == updateMsg.Payload().DataType)
            Decode(updateMsg.Payload().FieldList());

        Console.WriteLine(updateMsg);

        Console.WriteLine();
    }

    public void OnStatusMsg(StatusMsg statusMsg, IOmmConsumerEvent @event)
    {
        if (statusMsg.HasMsgKey)
        {
            Console.WriteLine("Item Name: " + (statusMsg.HasName ? statusMsg.Name() : "<not set>"));
            Console.WriteLine("Service Name: " + (statusMsg.HasServiceName ? statusMsg.ServiceName() : "<not set>"));
        }

        if (statusMsg.HasState)
            Console.WriteLine("Item State: " + statusMsg.State());

        Console.WriteLine();
    }

    public void OnGenericMsg(GenericMsg genericMsg, IOmmConsumerEvent consumerEvent)
    {
    }

    public void OnAckMsg(AckMsg ackMsg, IOmmConsumerEvent consumerEvent)
    {
    }

    public void OnAllMsg(Msg msg, IOmmConsumerEvent consumerEvent)
    {
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
            case DataTypes.MAP:
                Decode(msg.Payload().Map());
                break;
            case DataTypes.FIELD_LIST:
                Decode(msg.Payload().FieldList());
                break;
        }
    }

    static void Decode(ElementList elementList)
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
                    case DataTypes.ARRAY:
                    {
                        bool first = true;
                        foreach (OmmArrayEntry arrayEntry in elementEntry.OmmArrayValue())
                        {
                            if (!first)
                                Console.Write(", ");
                            else
                                first = false;
                            switch (arrayEntry.LoadType)
                            {
                                case DataTypes.ASCII:
                                    Console.Write(arrayEntry.OmmAsciiValue());
                                    break;
                                case DataTypes.UINT:
                                    Console.Write(arrayEntry.OmmUIntValue());
                                    break;
                                case DataTypes.QOS:
                                    Console.Write(arrayEntry.OmmQosValue());
                                    break;
                            }
                        }
                        Console.WriteLine();
                    }
                        break;
                    case DataTypes.RMTES :
                        Console.WriteLine(elementEntry.OmmRmtesValue());
                        break;
                    case DataTypes.ERROR:
                        Console.WriteLine(elementEntry.OmmErrorValue().ErrorCode + " (" + elementEntry.OmmErrorValue().ErrorCodeAsString() + ")");
                        break;
                    default:
                        Console.WriteLine();
                        break;
                }
        }
    }

    static void Decode(FieldList fieldList)
    {
        IEnumerator<FieldEntry> iter = fieldList.GetEnumerator();
        FieldEntry fieldEntry;
        while (iter.MoveNext())
        {
            fieldEntry = iter.Current;
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
                    case DataTypes.RMTES :
                        Console.WriteLine(fieldEntry.OmmRmtesValue());
                        break;
                    case DataTypes.ERROR:
                        Console.WriteLine("(" + fieldEntry.OmmErrorValue().ErrorCodeAsString() + ")");
                        break;
                    default:
                        Console.WriteLine();
                        break;
                }
        }
    }

    void Decode(Map map)
    {
        foreach (MapEntry mapEntry in map)
        {
            switch (mapEntry.LoadType)
            {
                case DataTypes.FILTER_LIST:
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
        foreach (FilterEntry filterEntry in filterList)
        {
            Console.WriteLine("ID: " + filterEntry.FilterId + " Action = " + filterEntry.FilterActionAsString() + " DataType: " + AsString(filterEntry.LoadType) + " Value: ");

            switch (filterEntry.LoadType)
            {
                case DataTypes.ELEMENT_LIST:
                    Decode(filterEntry.ElementList());
                    break;
                case DataTypes.MAP:
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
        try
        {
            AppClient appClient = new();
            OmmConsumerConfig config = new();
            RequestMsg reqMsg = new();
            OmmConsumer consumer = new(config.AddAdminMsg(reqMsg.Clear().DomainType(EmaRdm.MMT_DIRECTORY).Filter(29).InterestAfterRefresh(true).ServiceId(999)));
            int closure = 1;
            long loginHandle = consumer.RegisterClient(reqMsg.Clear().DomainType(EmaRdm.MMT_LOGIN), appClient, closure);
            long itemHandle = consumer.RegisterClient(reqMsg.Clear().ServiceId(999).Name("JPY="), appClient, closure);
            Thread.Sleep(100);
            long dirHandle = consumer.RegisterClient(reqMsg.Clear().DomainType(EmaRdm.MMT_DIRECTORY).InterestAfterRefresh(true).Filter(29).ServiceId(999), appClient, closure);
            Console.WriteLine("REISSUE DIRECTORY");
            consumer.Reissue(reqMsg.Clear().DomainType(EmaRdm.MMT_DIRECTORY).Filter(29).InterestAfterRefresh(true).ServiceId(999), dirHandle);
            Thread.Sleep(60000);
            consumer.Uninitialize();
        }
        catch (OmmException ommException)
        {
            Console.WriteLine(ommException.Message);
        }
    }
}

//END APIQA
