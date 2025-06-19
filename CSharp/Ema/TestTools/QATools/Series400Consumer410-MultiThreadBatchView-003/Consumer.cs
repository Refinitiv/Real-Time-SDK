/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Access;
using LSEG.Ema.Rdm;
using System;
using System.Text;
using System.Threading;

namespace LSEG.Ema.Example.Traning.Consumer;

internal class AppClient : IOmmConsumerClient
{
    public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmConsumerEvent @event)
    {
        ResultValidation? closure = (ResultValidation?)@event.Closure;

        closure?.ClosureValidate(refreshMsg.Name());

        if (refreshMsg.Solicited())
            ++ResultValidation.NumRefreshReceived;
        else
            ++ResultValidation.NumUnsolicitedRefreshReceived;
    }

    public void OnUpdateMsg(UpdateMsg updateMsg, IOmmConsumerEvent @event)
    {
        ++ResultValidation.NumUpdateReceived;
    }

    public void OnStatusMsg(StatusMsg statusMsg, IOmmConsumerEvent @event)
    {
        ++ResultValidation.NumStatusReceived;
    }
}

public class AppThread
{
    private readonly OmmConsumer _consumer;
    private readonly CancellationTokenSource _cancellationTokenSource;
    private readonly Thread _thread;

    public AppThread(OmmConsumer consumer)
    {
        _consumer = consumer;
        _thread = new Thread(_ => Run());
        _cancellationTokenSource = new CancellationTokenSource();
    }

    public void Start()
    {
        _thread.Start();
    }

    public void Stop()
    {
        _cancellationTokenSource.Cancel();
    }

    private void Run()
    {
        while (!_cancellationTokenSource.IsCancellationRequested)
            _consumer.Dispatch(Consumer.USERDISPATCHTIMEOUT);

        Thread.Sleep(1000);
    }
}

public class ConsumerThread
{
    private readonly AppClient _appClient;
    private readonly RequestMsg _reqMsg;
    private readonly StringBuilder _itemStrBuilder = new();
    private string? _itemNamePrefix;
    private string? _serviceName;
    private readonly Thread _thread;

    /*
     * Mode permits user to choose what kind of request the thread will send:
     * streaming view1 request snapshot view2 request batch snapshot requests -
     * 1st item will be what user specified, 2nd will append a "2" to the item
     * name
     */
    private int _mode = 1;
    private readonly OmmConsumer _consumer;

    public ConsumerThread(OmmConsumer consumer)
    {
        _appClient = new AppClient();
        _reqMsg = new();
        _consumer = consumer;
        _thread = new Thread(_ => Run());
    }

    public void OpenItem(string item, string serviceName, int mode)
    {
        _itemNamePrefix = item;
        _serviceName = serviceName;
        _mode = mode;

        _thread.Start();
    }

    private void SendRequest()
    {
        while (!Consumer.ISEXIT.IsCancellationRequested)
        {
            Thread.Sleep(1000);

            for (int idx = 1; idx <= Consumer.NUMOFITEMPERLOOP; ++idx)
            {
                _itemStrBuilder.Length = 0;

                string itemName = _itemStrBuilder.Append(_itemNamePrefix).Append(idx).ToString();
                if (_mode == 4)
                {
                    // SEND STREAMING MPs
                    _reqMsg.Clear().Name(itemName).ServiceName(_serviceName!).InterestAfterRefresh(true);
                }
                else if (_mode == 5)
                {
                    // SEND SNAPSHOT MPs
                    _reqMsg.Clear().Name(itemName).ServiceName(_serviceName!).InterestAfterRefresh(false);
                }
                else if (_mode == 1)
                {
                    // SEND STREAMING MPs with VIEW 1
                    ElementList view = new();
                    OmmArray array = new()
                    {
                        FixedWidth = 2
                    };
                    array.AddInt(22);
                    array.AddInt(25);
                    view.AddUInt(EmaRdm.ENAME_VIEW_TYPE, 1);

                    view.AddArray(EmaRdm.ENAME_VIEW_DATA, array.Complete());

                    _reqMsg.Clear().Name(itemName).ServiceName(_serviceName!).InterestAfterRefresh(true);
                    //_reqMsg.hasView();
                    _reqMsg.Payload(view.Complete());
                    Consumer.APPLOCK.EnterWriteLock();
                    ++ResultValidation.NumRequestOpen;
                    Consumer.APPLOCK.ExitWriteLock();
                }
                else if (_mode == 2)
                {
                    // SEND SNAPSHOT MPs with VIEW 2
                    ElementList view = new();
                    OmmArray array = new();

                    // array.FixedWidth(105);
                    array.AddInt(2);
                    array.AddInt(3);
                    array.AddInt(4);
                    array.AddInt(6);
                    array.AddInt(7);
                    array.AddInt(8);
                    array.AddInt(9);
                    array.AddInt(10);
                    array.AddInt(11);
                    array.AddInt(12);
                    array.AddInt(13);
                    array.AddInt(14);
                    array.AddInt(15);
                    array.AddInt(16);
                    array.AddInt(18);
                    array.AddInt(19);
                    array.AddInt(21);
                    array.AddInt(22);
                    array.AddInt(23);
                    array.AddInt(24);
                    array.AddInt(25);
                    array.AddInt(26);
                    array.AddInt(27);
                    array.AddInt(28);
                    array.AddInt(29);
                    array.AddInt(30);
                    array.AddInt(31);
                    array.AddInt(32);
                    array.AddInt(33);

                    view.AddUInt(EmaRdm.ENAME_VIEW_TYPE, 1);
                    view.AddArray(EmaRdm.ENAME_VIEW_DATA, array.Complete());

                    _reqMsg.Clear().Name(itemName).ServiceName(_serviceName!).InterestAfterRefresh(false);
                    //_reqMsg.hasView();
                    _reqMsg.Payload(view.Complete());
                    Consumer.APPLOCK.EnterWriteLock();
                    ++ResultValidation.NumRequestOpen;
                    Consumer.APPLOCK.ExitWriteLock();
                }
                else if (_mode == 3)
                {
                    // SEND STREAMING MPs with BATCH
                    ElementList batch = new();
                    OmmArray array = new();

                    array.AddAscii("BADNAME" + itemName);
                    array.AddAscii("BATCHITEM2" + itemName);

                    batch.AddArray(EmaRdm.ENAME_BATCH_ITEM_LIST, array.Complete());
                    //_reqMsg.Clear().HasBatch = true;
                    _reqMsg.InterestAfterRefresh(true).ServiceName(_serviceName!).Payload(batch.Complete());
                    Consumer.APPLOCK.EnterWriteLock();
                    ResultValidation.NumRequestOpen += 2;
                    Consumer.APPLOCK.ExitWriteLock();
                }
                _consumer.RegisterClient(_reqMsg, _appClient, new ResultValidation(itemName));
            }
        }
    }

    private void Run()
    {
        try
        {
            SendRequest();
            Thread.Sleep(1000);
        }
        catch (Exception excp)
        {
            Console.WriteLine(excp.Message);
        }
    }
}

public class ResultValidation
{
    public ResultValidation(String itemName)
    {
        ItemName = itemName;
    }

    public bool ClosureValidate(String receivedItemName)
    {
        bool result = receivedItemName.Equals(ItemName);
        if (result)
            ++NumValidClosure;
        else
            ++NumInvalidClosure;

        return result;
    }

    public static void PrintTestResult()
    {
        StringBuilder _testResultPrint = new();
        _testResultPrint.Append("\n\n***ResultValidation Test Result*** \n\n");

        _testResultPrint.Append(">>Request/Refresh validation\n");
        _testResultPrint.Append("_numRequestOpen = ").Append(NumRequestOpen).Append(" \n_numRefreshReceived = ").Append(NumRefreshReceived);

        _testResultPrint.Append("\n\n>>Update msg validation\n").Append("_numUpdateReceived = ").Append(NumUpdateReceived);

        _testResultPrint.Append("\n\n>>Status msg validation\n").Append("_numStatusReceived = ").Append(NumStatusReceived);

        _testResultPrint.Append("\n\n>>Closure validation\n").Append("_numValidClosure = ").Append(NumValidClosure);
        _testResultPrint.Append("\n_numInvalidClosure = ").Append(NumInvalidClosure);
        _testResultPrint.Append("\n********************************** \n\n");

        Console.WriteLine(_testResultPrint.ToString());
    }

    public static int NumRequestOpen { get; set; }
    public static int NumRefreshReceived { get; set; }
    public static int NumUnsolicitedRefreshReceived { get; set; }
    public static int NumUpdateReceived { get; set; }
    public static int NumStatusReceived { get; set; }
    public static int NumInvalidClosure { get; set; }
    public static int NumValidClosure { get; set; }
    public String ItemName { get; set; }
}

public class Consumer
{
    public static ReaderWriterLockSlim APPLOCK {get;} = new();
    public static CancellationTokenSource ISEXIT { get; } = new();
    public static int NUMOFITEMPERLOOP { get; set; } = 5000;
    public static bool USERDISPATCH { get; set; } = false;
    public static int USERDISPATCHTIMEOUT { get; set; } = 1000;
    public static int RUNTIME { get; set; } = 60000;

    public static void PrintHelp()
    {
        Console.WriteLine("\nOptions:\n" + "  -?\tShows this usage\n\n" + "  -snapshot \tSend item snapshot requests [default = true]\n"
                + "  -numOfItemPerLoop \tSend the number of item request per loop [default = 50]\n" + "  -userDispatch \tUse UserDispatch Operation Model [default = false]\n"
                + "  -userDispatchTimeout \tSet dispatch timeout period in microseconds if UserDispatch Operation Model [default = 1000]\n"
                + "  -runtime \tRun time for test case in milliseconds [default = 60000]\n" + "\n");
    }

    public static bool ReadCommandlineArgs(String[] argv)
    {
        int count = argv.Length;
        int idx = 0;

        while (idx < count)
        {
            if (0 == argv[idx].CompareTo("-?"))
            {
                PrintHelp();
                return false;
            }
            else if (argv[idx].Equals(comparisonType: StringComparison.InvariantCultureIgnoreCase, value: "-userDispatch"))
            {
                if (++idx >= count)
                {
                    PrintHelp();
                    return false;
                }
                USERDISPATCH = argv[idx].Equals("TRUE", StringComparison.InvariantCultureIgnoreCase);
                ++idx;
            }
            else if (argv[idx].Equals(comparisonType: StringComparison.InvariantCultureIgnoreCase, value: "-userDispatchTimeout"))
            {
                if (++idx >= count)
                {
                    PrintHelp();
                    return false;
                }
                USERDISPATCHTIMEOUT = int.Parse(argv[idx]);
                ++idx;
            }
            else if (argv[idx].Equals(comparisonType: StringComparison.InvariantCultureIgnoreCase, value: "-runtime"))
            {
                if (++idx >= count)
                {
                    PrintHelp();
                    return false;
                }
                RUNTIME = int.Parse(argv[idx]);
                ++idx;
            }
            else
            {
                PrintHelp();
                return false;
            }
        }

        return true;
    }

    public static void Main(string[] args)
    {
        OmmConsumer? consumer = null;
        AppThread? userDispathThread = null;

        try
        {
            if (!ReadCommandlineArgs(args))
                return;

            /* allow test with userDispatch or apiDispatch */
            if (USERDISPATCH)
            {
                consumer = new(new OmmConsumerConfig().Host("localhost:14002").UserName("user").OperationModel(OmmConsumerConfig.OperationModelMode.USER_DISPATCH));
                userDispathThread = new AppThread(consumer);
                userDispathThread.Start();
            }
            else
                consumer = new(new OmmConsumerConfig().Host("localhost:14002").UserName("user"));

            Console.Write("The consumer application starts testing with ");
            Console.WriteLine(USERDISPATCH ? "UserDispatch..." : "ApiDispatch...");

            /* allow test with multiple threads On the same consumer instance */
            ConsumerThread consumerThread1 = new(consumer);
            ConsumerThread consumerThread2 = new(consumer);
            ConsumerThread consumerThread3 = new(consumer);

            /* allow test with different item from different service name */
            consumerThread1.OpenItem("TRI.N", "DIRECT_FEED", 4);
            Thread.Sleep(1000);
            consumerThread2.OpenItem("TRI.N", "DIRECT_FEED", 5);
            consumerThread3.OpenItem("TRI.N", "DIRECT_FEED", 2);

            /* allow test with different long run period */
            Thread.Sleep(RUNTIME);

            ISEXIT.Cancel();
            Console.WriteLine("The consumer application is waiting for all responses back before exit...");

            Thread.Sleep(10000);

            if (USERDISPATCH)
                userDispathThread!.Stop();

            /* allow to validate different specification */
            ResultValidation.PrintTestResult();
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