/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.Md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Ema.Example.Traning.Consumer;

using LSEG.Ema.Access;
using LSEG.Ema.Rdm;
using System;
using System.Threading;
using static LSEG.Ema.Access.DataType;
using Msg = Access.Msg;

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

        Decode(statusMsg);

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
            case DataTypes.ELEMENT_LIST:
                Decode(msg.Payload().ElementList());
                break;
            case DataTypes.FIELD_LIST:
                Decode(msg.Payload().FieldList());
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
                    case DataTypes.VECTOR:
                        Decode(elementEntry.Vector());
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
                    case DataTypes.RMTES :
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

    void Decode(Vector vector)
    {
        switch (vector.SummaryData().DataType)
        {
            case DataTypes.ELEMENT_LIST:
                Decode(vector.SummaryData().ElementList());
                break;
        }

        foreach (VectorEntry vectorEntry in vector)
        {
            Console.Write("Position: " + vectorEntry.Position + " Action = " + vectorEntry.VectorActionAsString() + " DataType: " + AsString(vectorEntry.LoadType) + " Value: ");

            switch (vectorEntry.LoadType)
            {
                case DataTypes.ELEMENT_LIST:
                    Decode(vectorEntry.ElementList());
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

            OmmConsumerConfig config = new();

            ElementList elementList = new();

            elementList.AddUInt("SingleOpen", 1)
                .AddUInt("AllowSuspectData", 1)
                .AddUInt("ProvidePermissionExpressions", 0)
                .AddUInt("ProvidePermissionProfile", 0)
                .AddUInt("DownloadConnectionConfig", 1)
                .AddAscii("InstanceId", "2")
                .AddAscii("ApplicationId", "256")
                .AddAscii("ApplicationName", "Test Application")
                .AddAscii("Password", "secrete")
                .AddAscii("Position", "127.0.0.1")
                .AddUInt("Role", 0)
                .AddUInt("SupportProviderDictionaryDownload", 0)
                .Complete();

            RequestMsg reqMsg = new();
            
            config.AddAdminMsg(reqMsg.DomainType(EmaRdm.MMT_LOGIN).Name("apiqa").NameType(3).Attrib(elementList));

            consumer = new OmmConsumer(config.Host("localhost:14002"));

            int closure = 1;
            int closure2 = 2;

            long loginHandle = consumer.RegisterClient(reqMsg.Clear().DomainType(EmaRdm.MMT_LOGIN), appClient, closure);

            long handle = consumer.RegisterClient(reqMsg.Clear().ServiceName("DIRECT_FEED").Name("CSCO.O"), appClient, closure2);

            Thread.Sleep(1000);

            Console.WriteLine("Login Reissue with different name from apiqa to apiqa2");
            consumer.Reissue(reqMsg.Clear().InitialImage(true).Name("apiqa2").DomainType(EmaRdm.MMT_LOGIN).Attrib(elementList).ServiceName("DIRECT_FEED").NameType(3), loginHandle);

            Console.WriteLine("Login Reissue done with name apiqa2");

            Thread.Sleep(60000); // API calls OnRefreshMsg(), OnUpdateMsg() and
                                 // OnStatusMsg()

            consumer.Uninitialize();
        }
        catch (OmmException ommException)
        {
            Console.WriteLine("Got OmmException from the main() function.");
            Console.WriteLine(ommException.Message);
        }
        finally
        {
            consumer?.Uninitialize();
        }
    }
}
//END APIQA
