/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Access;
using System;
using System.Collections.Generic;
using System.Threading;
using static LSEG.Ema.Access.DataType;
using static LSEG.Ema.Access.OmmReal;

namespace LSEG.Ema.Example.Traning.Consumer;

internal class AppClient : IOmmConsumerClient
{
    private static int postId = 1;
    //API QA
    List<ChannelInformation> channelInList = new();
    //END API QA
    public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmConsumerEvent @event)
    {
        Console.WriteLine("Received Refresh. Item Handle: " + @event.Handle + " Closure: " + (@event.Closure ?? "null"));

        Console.WriteLine("Item Name: " + (refreshMsg.HasName ? refreshMsg.Name() : "<not set>"));
        Console.WriteLine("Service Name: " + (refreshMsg.HasServiceName ? refreshMsg.ServiceName() : "<not set>"));

        Console.WriteLine("Item State: " + refreshMsg.State());

        if (refreshMsg.State().StreamState == OmmState.StreamStates.OPEN &&
            refreshMsg.State().DataState == OmmState.DataStates.OK)
        {
            PostMsg postMsg = new();
            UpdateMsg nestedUpdateMsg = new();
            FieldList nestedFieldList = new();

            nestedFieldList.AddReal(22, 34, MagnitudeTypes.EXPONENT_POS_1);
            nestedFieldList.AddReal(25, 35, MagnitudeTypes.EXPONENT_POS_1);
            nestedFieldList.AddTime(18, 11, 29, 30);
            nestedFieldList.AddEnumValue(37, 3);
            nestedFieldList.Complete();
            nestedUpdateMsg.Payload(nestedFieldList);

            @event.Consumer.Submit(postMsg.PostId(postId++).ServiceName("DIRECT_FEED")
                                                        .Name("LSEG.L").SolicitAck(true).Complete(true)
                                                        .Payload(nestedUpdateMsg), @event.Handle);
        }

        Decode(refreshMsg);
        Console.WriteLine();
        //API QA
        Console.WriteLine("\nevent session info (refresh)");
        PrintSessionInfo(@event);
        //End API QA
    }

    public void OnUpdateMsg(UpdateMsg updateMsg, IOmmConsumerEvent @event)
    {
        Console.WriteLine("Received Update. Item Handle: " + @event.Handle + " Closure: " + (@event.Closure ?? "null"));

        Console.WriteLine("Item Name: " + (updateMsg.HasName ? updateMsg.Name() : "<not set>"));
        Console.WriteLine("Service Name: " + (updateMsg.HasServiceName ? updateMsg.ServiceName() : "<not set>"));

        Decode(updateMsg);

        Console.WriteLine();
        //API QA
        Console.WriteLine("\nevent session info (update)");
        PrintSessionInfo(@event);
        //End API QA
    }

    public void OnStatusMsg(StatusMsg statusMsg, IOmmConsumerEvent @event)
    {
        Console.WriteLine("Received Status. Item Handle: " + @event.Handle + " Closure: " + (@event.Closure ?? "null"));

        Console.WriteLine("Item Name: " + (statusMsg.HasName ? statusMsg.Name() : "<not set>"));
        Console.WriteLine("Service Name: " + (statusMsg.HasServiceName ? statusMsg.ServiceName() : "<not set>"));

        if (statusMsg.HasState)
            Console.WriteLine("Item State: " + statusMsg.State());

        Console.WriteLine();
        //API QA
        Console.WriteLine("\nevent session info (status)");
        PrintSessionInfo(@event);
        //End API QA
    }

    public void OnAckMsg(AckMsg ackMsg, IOmmConsumerEvent @event)
    {
        Console.WriteLine("Received AckMsg. Item Handle: " + @event.Handle + " Closure: " + (@event.Closure ?? "null"));

        Decode(ackMsg);

        Console.WriteLine();
        //API QA
        Console.WriteLine("\nevent session info (ack)");
        PrintSessionInfo(@event);
        //End API QA
    }

    public void OnGenericMsg(GenericMsg genericMsg, IOmmConsumerEvent _)
    { }

    public void OnAllMsg(Msg msg, IOmmConsumerEvent _)
    { }

    private void Decode(AckMsg ackMsg)
    {
        if (ackMsg.HasMsgKey)
            Console.WriteLine("Item Name: " + (ackMsg.HasName ? ackMsg.Name() : "not set") + "\nService Name: "
                    + (ackMsg.HasServiceName ? ackMsg.ServiceName() : "not set"));

        Console.WriteLine("Ack Id: " + ackMsg.AckId());

        if (ackMsg.HasNackCode)
            Console.WriteLine("Nack Code: " + ackMsg.NackCodeAsString());

        if (ackMsg.HasText)
            Console.WriteLine("Text: " + ackMsg.Text());

        switch (ackMsg.Attrib().DataType)
        {
            case DataTypes.ELEMENT_LIST:
                Decode(ackMsg.Attrib().ElementList());
                break;

            case DataTypes.FIELD_LIST:
                Decode(ackMsg.Attrib().FieldList());
                break;

            default:
                break;
        }

        switch (ackMsg.Payload().DataType)
        {
            case DataTypes.ELEMENT_LIST:
                Decode(ackMsg.Payload().ElementList());
                break;

            case DataTypes.FIELD_LIST:
                Decode(ackMsg.Payload().FieldList());
                break;

            default:
                break;
        }
    }

    private void Decode(Msg msg)
    {
        switch (msg.Attrib().DataType)
        {
            case DataTypes.ELEMENT_LIST:
                Decode(msg.Attrib().ElementList());
                break;

            case DataTypes.FIELD_LIST:
                Decode(msg.Attrib().FieldList());
                break;

            default:
                break;
        }

        switch (msg.Payload().DataType)
        {
            case DataTypes.ELEMENT_LIST:
                Decode(msg.Payload().ElementList());
                break;

            case DataTypes.FIELD_LIST:
                Decode(msg.Payload().FieldList());
                break;

            default:
                break;
        }
    }

    private void Decode(ElementList elementList)
    {
        foreach (ElementEntry elementEntry in elementList)
        {
            Console.Write(" Name = " + elementEntry.Name + " DataType: " + DataType.AsString(elementEntry.Load!.DataType) + " Value: ");

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
                        Console.WriteLine(elementEntry.IntValue());
                        break;

                    case DataTypes.UINT:
                        Console.WriteLine(elementEntry.UIntValue());
                        break;

                    case DataTypes.ASCII:
                        Console.WriteLine(elementEntry.OmmAsciiValue());
                        break;

                    case DataTypes.ENUM:
                        Console.WriteLine(elementEntry.EnumValue());
                        break;

                    case DataTypes.RMTES:
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
    //API QA
    void PrintSessionInfo(IOmmConsumerEvent consumerEvent)
    {

        consumerEvent.SessionChannelInfo(channelInList);

        foreach (ChannelInformation channelInfo in channelInList)
        {
            Console.WriteLine(channelInfo);
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
            AppClient appClient = new AppClient();

            //consumer = new(new OmmConsumerConfig().Host("localhost:14002").UserName("user"));
            consumer = new(new OmmConsumerConfig().ConsumerName("Consumer_10").UserName("user"));

            consumer.RegisterClient(new RequestMsg().ServiceName("DIRECT_FEED").Name("LSEG.L"), appClient, consumer);

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