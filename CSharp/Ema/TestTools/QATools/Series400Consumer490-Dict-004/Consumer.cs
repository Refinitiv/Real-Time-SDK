/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.Md for details.                  --
 *|           Copyright (C) 2024 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Access;
using LSEG.Ema.Rdm;

using static LSEG.Ema.Access.DataType;

namespace LSEG.Ema.Example.Traning.Consumer;

class AppClient : IOmmConsumerClient
{
    public Rdm.DataDictionary dataDictionary = new Rdm.DataDictionary();
    private bool fldDictComplete = false;
    private bool enumTypeComplete = false;

    public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmConsumerEvent consumerEvent)
    {
        Console.WriteLine("Received Refresh. Item Handle: " + consumerEvent.Handle + " Closure: " + consumerEvent.Closure);

        Console.WriteLine("Item Name: " + (refreshMsg.HasName ? refreshMsg.Name() : "<not set>"));
        Console.WriteLine("Service Name: " + (refreshMsg.HasServiceName ? refreshMsg.ServiceName() : "<not set>"));

        Console.WriteLine("Item State: " + refreshMsg.State());

        Decode(refreshMsg, refreshMsg.Complete());

        Console.WriteLine();
    }

    public void OnUpdateMsg(UpdateMsg updateMsg, IOmmConsumerEvent consumerEvent)
    {
        Console.WriteLine("Received Update. Item Handle: " + consumerEvent.Handle + " Closure: " + consumerEvent.Closure);

        Console.WriteLine("Item Name: " + (updateMsg.HasName ? updateMsg.Name() : "<not set>"));
        Console.WriteLine("Service Name: " + (updateMsg.HasServiceName ? updateMsg.ServiceName() : "<not set>"));

        Decode(updateMsg, false);

        Console.WriteLine();
    }

    public void OnStatusMsg(StatusMsg statusMsg, IOmmConsumerEvent consumerEvent)
    {
        Console.WriteLine("Received Status. Item Handle: " + consumerEvent.Handle + " Closure: " + consumerEvent.Closure);

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
    public const int DEFAULT_CONSUMERS_COUNT = 30;

    public static void DumpHeap(string filePath)
    {
        if (File.Exists(filePath))
        {
            // do something
            File.Delete(filePath);
        }

        System.GC.Collect();
        System.GC.WaitForPendingFinalizers();

        System.GCMemoryInfo memInfo = System.GC.GetGCMemoryInfo();

        using (StreamWriter dumpFile = File.CreateText(filePath))
        {
            dumpFile.WriteLine($"Compacted: {memInfo.Compacted}");
            dumpFile.WriteLine($"Concurrent: {memInfo.Concurrent}");
            dumpFile.WriteLine($"FinalizationPendingCount: {memInfo.FinalizationPendingCount}");
            dumpFile.WriteLine($"FragmentedBytes: {memInfo.FragmentedBytes}");
            dumpFile.WriteLine($"Generation: {memInfo.Generation}");
            dumpFile.WriteLine($"GenerationInfo:");
            int genCount = 0;
            foreach (GCGenerationInfo genInfo in memInfo.GenerationInfo)
            {
                dumpFile.WriteLine($"    Gen: {genCount}");
                dumpFile.WriteLine($"    FragmentationAfterBytes: {genInfo.FragmentationAfterBytes}");
                dumpFile.WriteLine($"    FragmentationBeforeBytes: {genInfo.FragmentationBeforeBytes}");
                dumpFile.WriteLine($"    SizeAfterBytes: {genInfo.SizeAfterBytes}");
                dumpFile.WriteLine($"    SizeBeforeBytes: {genInfo.SizeBeforeBytes}");
                genCount++;
            }
            dumpFile.WriteLine($"HeapSizeBytes: {memInfo.HeapSizeBytes}");
            dumpFile.WriteLine($"HighMemoryLoadThresholdBytes: {memInfo.HighMemoryLoadThresholdBytes}");
            dumpFile.WriteLine($"Index: {memInfo.Index}");
            dumpFile.WriteLine($"MemoryLoadBytes: {memInfo.MemoryLoadBytes}");
            dumpFile.WriteLine($"PauseTimePercentage: {memInfo.PauseTimePercentage}");
            dumpFile.WriteLine($"PinnedObjectsCount: {memInfo.PinnedObjectsCount}");
            dumpFile.WriteLine($"PromotedBytes: {memInfo.PromotedBytes}");
            dumpFile.WriteLine($"TotalAvailableMemoryBytes: {memInfo.TotalAvailableMemoryBytes}");
            dumpFile.WriteLine($"TotalCommittedBytes: {memInfo.TotalCommittedBytes}");
        }

        Console.WriteLine("Generated head dump file:" + filePath);
    }

    public static void Main(string[] args)
    {
        int consumersCount = DEFAULT_CONSUMERS_COUNT;

        if (args.Length != 1)
        {
            Console.WriteLine("You can specify custom consumers count as the first command-line parameter");
        }
        else
        {
            if (int.TryParse(args[0], out int customCount))
                consumersCount = customCount;
        }

        List<OmmConsumer> consumers = new List<OmmConsumer>(consumersCount);


        Console.WriteLine($"Running with {consumersCount} consumers");

        try
        {
            AppClient appClient = new AppClient();

            OmmConsumerConfig config = new OmmConsumerConfig();

            Rdm.DataDictionary dictionary = new Rdm.DataDictionary();
            dictionary.LoadFieldDictionary("./RDMFieldDictionary");
            dictionary.LoadEnumTypeDictionary("./enumtype.def");
            config.DataDictionary(dictionary, false);

            RequestMsg reqMsg = (new RequestMsg()).ServiceName("DIRECT_FEED").Name("IBM.N");

            for (int count = 0; count < consumersCount; count++)
            {
                OmmConsumer consumer = new OmmConsumer(config.Host("localhost:14002").UserName("user"));
                consumers.Add(consumer);
            }

            foreach (OmmConsumer consumer in consumers)
                consumer.RegisterClient(reqMsg, appClient);

            Thread.Sleep(30_000); // API calls onRefreshMsg(), onUpdateMsg() and
                                  // onStatusMsg()
        }
        catch (Exception excp)
        {
            Console.WriteLine(excp.Message);
        }
        finally
        {
            try
            {
                Consumer.DumpHeap("beforeUnitialize.meminfo");

                foreach (OmmConsumer consumer in consumers)
                    consumer.Uninitialize();

                Consumer.DumpHeap("afterUnitialize.meminfo");

            }
            catch (Exception e)
            {
                Console.WriteLine("Failure during application shutdown: " + e.Message);
            }

        }
        consumers.Clear();
        System.GC.Collect();
        Consumer.DumpHeap("afterGC.meminfo");
        Thread.Sleep(60_000);
    }
}
