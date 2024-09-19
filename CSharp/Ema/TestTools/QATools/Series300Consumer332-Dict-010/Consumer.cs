/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.Md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Access;
using System.Threading;
using System;
using LSEG.Ema.Rdm;
using static LSEG.Ema.Access.DataType;

namespace LSEG.Ema.Example.Traning.Consumer;

internal class AppClient : IOmmConsumerClient
{
    public DataDictionary dataDictionary = new();
    public bool fldDictComplete = false;
    public bool enumTypeComplete = false;

    public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmConsumerEvent @event)
    {
        Console.WriteLine("Received Refresh. Item Handle: " + @event.Handle + " Closure: " + @event.Closure);

        Console.WriteLine("Item Name: " + (refreshMsg.HasName ? refreshMsg.Name() : "<not set>"));
        Console.WriteLine("Service Name: " + (refreshMsg.HasServiceName ? refreshMsg.ServiceName() : "<not set>"));

        Console.WriteLine("Item State: " + refreshMsg.State());

        Decode(refreshMsg, refreshMsg.Complete());

        Console.WriteLine();
    }

    public void OnUpdateMsg(UpdateMsg updateMsg, IOmmConsumerEvent @event)
    {
        Console.WriteLine("Received Update. Item Handle: " + @event.Handle + " Closure: " + @event.Closure);

        Console.WriteLine("Item Name: " + (updateMsg.HasName ? updateMsg.Name() : "<not set>"));
        Console.WriteLine("Service Name: " + (updateMsg.HasServiceName ? updateMsg.ServiceName() : "<not set>"));

        Decode(updateMsg, false);

        Console.WriteLine();
    }

    public void OnStatusMsg(StatusMsg statusMsg, IOmmConsumerEvent @event)
    {
        Console.WriteLine("Received Status. Item Handle: " + @event.Handle + " Closure: " + @event.Closure);

        Console.WriteLine("Item Name: " + (statusMsg.HasName ? statusMsg.Name() : "<not set>"));
        Console.WriteLine("Service Name: " + (statusMsg.HasServiceName ? statusMsg.ServiceName() : "<not set>"));

        if (statusMsg.HasState)
            Console.WriteLine("Item State: " + statusMsg.State());

        Console.WriteLine();
    }

    void Decode(Msg msg, bool complete)
    {
        switch (msg.Payload().DataType)
        {
            case DataTypes.SERIES:

                if (msg.Name().Equals("RWFFld"))
                {
                    dataDictionary.DecodeFieldDictionary(msg.Payload().Series(), EmaRdm.DICTIONARY_NORMAL);

                    if (complete)
                    {
                        fldDictComplete = true;
                    }
                }
                else if (msg.Name().Equals("RWFEnum"))
                {
                    dataDictionary.DecodeEnumTypeDictionary(msg.Payload().Series(), EmaRdm.DICTIONARY_NORMAL);

                    if (complete)
                    {
                        enumTypeComplete = true;
                    }
                }

                if (fldDictComplete && enumTypeComplete)
                {
                    Console.WriteLine(dataDictionary);
                }

                break;
            case DataTypes.FIELD_LIST:
                Decode(msg.Payload().FieldList());
                break;
            default:
                break;
        }
    }

    void Decode(FieldList fieldList)
    {
        foreach (FieldEntry fieldEntry in fieldList)
        {
            Console.Write("Fid: " + fieldEntry.FieldId + " Name = " + fieldEntry.Name + " DataType: "
                    + DataType.AsString(fieldEntry.Load!.DataType) + " Value: ");

            if (Data.DataCode.BLANK == fieldEntry.Code)
                Console.WriteLine(" blank");
            else
                switch (fieldEntry.LoadType)
                {
                    case DataTypes.REAL:
                        Console.WriteLine(fieldEntry.OmmRealValue().AsDouble());
                        break;
                    case DataTypes.DATE:
                        Console.WriteLine(fieldEntry.OmmDateValue().Day + " / " + fieldEntry.OmmDateValue().Month + " / "
                                + fieldEntry.OmmDateValue().Year);
                        break;
                    case DataTypes.TIME:
                        Console.WriteLine(fieldEntry.OmmTimeValue().Hour + ":" + fieldEntry.OmmTimeValue().Minute + ":"
                                + fieldEntry.OmmTimeValue().Second + ":" + fieldEntry.OmmTimeValue().Millisecond);
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
                        Console.WriteLine(
                                fieldEntry.OmmErrorValue().ErrorCode + " (" + fieldEntry.OmmErrorValue().ErrorCodeAsString() + ")");
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
    static bool shouldCopyIntoAPI = false;
    static void printHelp()
    {
        Console.WriteLine("\nOptions:\n" + "  -?\tShows this usage\n"
                + "  -shouldCopyIntoAPI to enable shouldCopyIntoAPI (default is false)\r\n"
                + "\n");
    }
    static bool readCommandlineArgs(string[] args)
    {
        try
        {
            int argsCount = 0;

            while (argsCount < args.Length)
            {
                if (0 == args[argsCount].CompareTo("-?"))
                {
                    printHelp();
                    return false;
                }
                else if ("-shouldCopyIntoAPI".Equals(args[argsCount]))
                {
                    shouldCopyIntoAPI = true;
                    ++argsCount;
                }
                else // unrecognized command line argument
                {
                    printHelp();
                    return false;
                }
            }
        }
        catch
        {
            printHelp();
            return false;
        }

        return true;
    }
    public static void Main(string[] args)
    {
        if (!readCommandlineArgs(args)) return;
        OmmConsumer? consumer = null;
        OmmConsumer? consumer2 = null;
        try
        {
            AppClient appClient = new();
            OmmConsumerConfig config = new OmmConsumerConfig();

            consumer = new OmmConsumer(config.ConsumerName("Consumer_1").UserName("user1"));

            RequestMsg reqMsg = new();

            consumer.RegisterClient(reqMsg.DomainType(EmaRdm.MMT_DICTIONARY).Name("RWFFld").
                     Filter(EmaRdm.DICTIONARY_NORMAL), appClient);

            consumer.RegisterClient(reqMsg.Clear().DomainType(EmaRdm.MMT_DICTIONARY).Name("RWFEnum").
                    Filter(EmaRdm.DICTIONARY_NORMAL), appClient);

            Thread.Sleep(20000);
            if (appClient.enumTypeComplete && appClient.fldDictComplete)
            {
                Console.WriteLine("enumTypeComplete and fldDictComplete. Set DataDictionary to OmmmConsumerConfig");
                config.DataDictionary(appClient.dataDictionary, shouldCopyIntoAPI);
            }

            Console.WriteLine("Config shouldCopyIntoAPI set to : " + shouldCopyIntoAPI);
            consumer.RegisterClient(reqMsg.Clear().ServiceName("ELEKTRON_DD").Name("TRI.N"), appClient);
            Thread.Sleep(30000);
            consumer.Uninitialize();
            consumer = null;
            Console.WriteLine("Consumer_1 has been uninitialized.");

            consumer2 = new OmmConsumer(config.ConsumerName("Consumer_2").UserName("user2"));
            consumer2.RegisterClient(reqMsg.Clear().ServiceName("ELEKTRON_DD").Name("IBM.N"), appClient);

            Thread.Sleep(30000); // API calls OnRefreshMsg(), OnUpdateMsg() and
                                 // OnStatusMsg()
        }
        catch (OmmException ommException)
        {
            Console.WriteLine(ommException.Message);
        }
        finally
        {
            consumer?.Uninitialize();
            consumer2?.Uninitialize();
        }
    }
}
