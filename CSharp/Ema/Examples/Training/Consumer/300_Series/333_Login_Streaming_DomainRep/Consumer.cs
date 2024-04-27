/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023, 2024 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Access;
using LSEG.Ema.Domain.Login;
using LSEG.Ema.Rdm;
using System;
using System.Threading;
using DataTypes = LSEG.Ema.Access.DataType.DataTypes;
using FieldList = LSEG.Ema.Access.FieldList;

namespace LSEG.Ema.Example.Traning.Consumer;

internal class AppClient : IOmmConsumerClient
{
    private readonly LoginRefresh _loginRefresh = new();
    private readonly LoginStatus _loginStatus = new();

    public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmConsumerEvent @event)
    {
        Console.WriteLine("Received Refresh. Item Handle: " + @event.Handle + " Closure: " + (@event.Closure ?? "null"));

        Console.WriteLine("Item Name: " + (refreshMsg.HasName ? refreshMsg.Name() : "<not set>"));
        Console.WriteLine("Service Name: " + (refreshMsg.HasServiceName ? refreshMsg.ServiceName() : "<not set>"));

        Console.WriteLine("Item State: " + refreshMsg.State());

        if (refreshMsg.DomainType() == EmaRdm.MMT_LOGIN)
        {
            _loginRefresh.Clear();
            Console.WriteLine(_loginRefresh.Message(refreshMsg).ToString());
        }
        else
        {
            Decode(refreshMsg);
        }

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

        if (statusMsg.DomainType() == EmaRdm.MMT_LOGIN)
        {
            _loginStatus.Clear();
            Console.WriteLine(_loginStatus.Message(statusMsg).ToString());
        }

        Console.WriteLine();
    }

    private void Decode(Access.Msg msg)
    {
        if (msg.Attrib().DataType == DataTypes.FIELD_LIST)
            Decode(msg.Attrib().FieldList());

        if (msg.Payload().DataType == DataTypes.FIELD_LIST)
            Decode(msg.Payload().FieldList());
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
}

public class Consumer
{
    public static void Main()
    {
        OmmConsumer? consumer = null;
        try
        {
            AppClient appClient = new();
            consumer = new(new OmmConsumerConfig().Host("localhost:14002").UserName("user"));

            LoginReq loginReq = new();
            consumer.RegisterClient(loginReq.Message(), appClient);

            RequestMsg reqMsg = new();
            consumer.RegisterClient(reqMsg.ServiceName("DIRECT_FEED").Name("IBM.N"), appClient);

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