/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.Md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Access;
using LSEG.Ema.Rdm;

using static LSEG.Ema.Access.DataType;

namespace LSEG.Ema.Example.Traning.Consumer;

class AppClient : IOmmConsumerClient
{
    public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmConsumerEvent _)
    {
        Console.WriteLine("Item Name: " + (refreshMsg.HasName ? refreshMsg.Name() : "<not set>"));
        Console.WriteLine("Service Name: " + (refreshMsg.HasServiceName ? refreshMsg.ServiceName() : "<not set>"));

        Console.WriteLine("Item State: " + refreshMsg.State());

        if (DataType.DataTypes.FIELD_LIST == refreshMsg.Payload().DataType)
            Decode(refreshMsg.Payload().FieldList());

        Console.WriteLine();
    }

    public void OnUpdateMsg(UpdateMsg updateMsg, IOmmConsumerEvent _)
    {
        Console.WriteLine("Item Name: " + (updateMsg.HasName ? updateMsg.Name() : "<not set>"));
        Console.WriteLine("Service Name: " + (updateMsg.HasServiceName ? updateMsg.ServiceName() : "<not set>"));

        if (DataType.DataTypes.FIELD_LIST == updateMsg.Payload().DataType)
            Decode(updateMsg.Payload().FieldList());

        Console.WriteLine();
    }

    public void OnStatusMsg(StatusMsg statusMsg, IOmmConsumerEvent _)
    {
        Console.WriteLine("Item Name: " + (statusMsg.HasName ? statusMsg.Name() : "<not set>"));
        Console.WriteLine("Service Name: " + (statusMsg.HasServiceName ? statusMsg.ServiceName() : "<not set>"));

        if (statusMsg.HasState)
            Console.WriteLine("Item State: " + statusMsg.State());

        Console.WriteLine();
    }

    void Decode(FieldList fieldList)
    {
        foreach (FieldEntry fieldEntry in fieldList)
        {
            Console.Write($"Fid: {fieldEntry.FieldId} Name = {fieldEntry.Name} DataType: {DataType.AsString(fieldEntry.LoadType)} Value: ");

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

                    case DataTypes.DATETIME:
                        Console.WriteLine(fieldEntry.OmmDateTimeValue().Day + " / " + fieldEntry.OmmDateTimeValue().Month + " / " +
                            fieldEntry.OmmDateTimeValue().Year + "." + fieldEntry.OmmDateTimeValue().Hour + ":" +
                            fieldEntry.OmmDateTimeValue().Minute + ":" + fieldEntry.OmmDateTimeValue().Second + ":" +
                            fieldEntry.OmmDateTimeValue().Millisecond + ":" + fieldEntry.OmmDateTimeValue().Microsecond + ":" +
                            fieldEntry.OmmDateTimeValue().Nanosecond);
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
                        Console.WriteLine("(" + fieldEntry.OmmErrorValue().ErrorCodeAsString() + ")");
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
    static bool testClearConfig = false;

    static void PrintHelp()
    {
        Console.WriteLine("\nOptions:\n"
                + "  -?\tShows this usage\n"
                + "  -shouldCopyIntoAPI to enable shouldCopyIntoAPI (default is false) \r\n"
                + "  -testClearConfig to enable testClearConfig (default is false) \r\n"
                + "\n");
    }

    static bool ReadCommandlineArgs(string[] args)
    {
        try
        {
            int argsCount = 0;

            while (argsCount < args.Length)
            {
                if ("-?".Equals(args[argsCount]))
                {
                    PrintHelp();
                    return false;
                }
                else if ("-shouldCopyIntoAPI".Equals(args[argsCount]))
                {
                    shouldCopyIntoAPI = true;
                    ++argsCount;
                }
                else if ("-testClearConfig".Equals(args[argsCount]))
                {
                    testClearConfig = true;
                    ++argsCount;
                }
                else // unrecognized command line argument
                {
                    PrintHelp();
                    return false;
                }
            }
        }
        catch (Exception)
        {
            PrintHelp();
            return false;
        }

        return true;
    }

    public static void Main(string[] args)
    {
        OmmConsumer? consumer1 = null;
        OmmConsumer? consumer2 = null;
        try
        {
            AppClient appClient1 = new AppClient();
            AppClient appClient2 = new AppClient();

            if (!ReadCommandlineArgs(args))
                return;

            OmmConsumerConfig config = new OmmConsumerConfig();
            RequestMsg reqMsg = new RequestMsg();

            // Create DataDictionary and load it from our dictionary files
            Rdm.DataDictionary dictionary = new Rdm.DataDictionary();
            dictionary.LoadFieldDictionary("./RDMFieldDictionary");
            dictionary.LoadEnumTypeDictionary("./enumtype.def");

            // Specify DataDictionary inside of our OmmConsumerConfig
            config.DataDictionary(dictionary, shouldCopyIntoAPI);
            Console.WriteLine("Config shouldCopyIntoAPI set to : " + shouldCopyIntoAPI);

            consumer1 = new OmmConsumer(config.OperationModel(OmmConsumerConfig.OperationModelMode.USER_DISPATCH).ConsumerName("Consumer_1"));
            consumer1.RegisterClient(reqMsg.Clear().ServiceName("DIRECT_FEED").Name("IBM.N"), appClient2, 1);

            //Load new dictionary which includes a negative FID
            dictionary.Clear();
            dictionary.LoadFieldDictionary("./RDMFieldDictionary1");
            dictionary.LoadEnumTypeDictionary("./enumtype.def");

            Rdm.DictionaryEntry entry_Neg32 = new DictionaryEntry();
            dictionary.Entry(-32, entry_Neg32);
            Console.WriteLine("QA Prints entry_Neg32 fname : " + entry_Neg32.Acronym);

            if (testClearConfig)
            {
                config.Clear();
                config.DataDictionary(dictionary, false);
            }
            consumer2 = new OmmConsumer(config.OperationModel(OmmConsumerConfig.OperationModelMode.USER_DISPATCH).ConsumerName("Consumer_2"));
            consumer2.RegisterClient(reqMsg.Clear().ServiceName("DIRECT_FEED").Name("TRI.N"), appClient2, 1);

            DateTime startTime = DateTime.Now;
            while (startTime + TimeSpan.FromSeconds(60) > DateTime.Now)
            {
                consumer1.Dispatch(30000000);     // calls to onRefreshMsg(), onUpdateMsg(), or onStatusMsg() execute on this thread
                consumer2.Dispatch(30000000);
            }
        }
        catch (OmmException excp)
        {
            Console.WriteLine(excp.Message);
        }
        finally
        {
            consumer1?.Uninitialize();
            consumer2?.Uninitialize();
        }
    }
}
