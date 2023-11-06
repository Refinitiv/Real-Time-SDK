/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.Md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Access;
using System;
using System.Text;
using System.Threading;

namespace LSEG.Ema.Example.Traning.Consumer;

class AppClient : IOmmConsumerClient
{
    public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmConsumerEvent @event)
    {
        ResultValidation? closure = (ResultValidation?)@event.Closure;

        closure?.ClosureValidate(refreshMsg.Name());
        ++ResultValidation._numRefreshReceived;
    }

    public void OnUpdateMsg(UpdateMsg updateMsg, IOmmConsumerEvent @event)
    {
        ++ResultValidation._numUpdateReceived;
    }

    public void OnStatusMsg(StatusMsg statusMsg, IOmmConsumerEvent @event)
    {
        ++ResultValidation._numStatusReceived;
    }
}

public class AppThread
{
    private readonly OmmConsumer _consumer;
    private readonly CancellationTokenSource _stop;
    private readonly Thread _thread;

    public AppThread(OmmConsumer consumer)
    {
        _consumer = consumer;
        _thread = new Thread(new ParameterizedThreadStart(_ => Run()));
        _stop = new CancellationTokenSource();
    }

    public void Start()
    {
        _thread.Start();
    }

    public void Stop()
    {
        _stop.Cancel();
    }

    private void Run()
    {
        while (!_stop.IsCancellationRequested)
            _consumer.Dispatch(Consumer.USERDISPATCHTIMEOUT);

        Thread.Sleep(1000);
    }
}

class ConsumerThread
{
    private readonly OmmConsumer _consumer;
    readonly AppClient _appClient;
    readonly RequestMsg _reqMsg;
    readonly StringBuilder _itemStrBuilder = new();
    String? _itemNamePrefix;
    String? _serviceName;
    private readonly Thread _thread;

    public ConsumerThread(OmmConsumer consumer)
    {
        _appClient = new AppClient();
        _reqMsg = new();
        _consumer = consumer;
        _thread = new Thread(_ => Run());
    }

    public void OpenItem(String item, String serviceName)
    {
        _itemNamePrefix = item;
        _serviceName = serviceName;

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

                    _reqMsg.Clear().Name(itemName).ServiceName(_serviceName!).InterestAfterRefresh(!Consumer.SNAPSHOT);

                    _consumer.RegisterClient(_reqMsg, _appClient, new ResultValidation(itemName));

                    Consumer.APPLOCK.EnterWriteLock();
                    ++ResultValidation._numRequestOpen;
                    Consumer.APPLOCK.ExitWriteLock();
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

class ResultValidation
{
    public ResultValidation(String itemName)
    {
        _itemName = itemName;
    }

    public bool ClosureValidate(String receivedItemName)
    {
        bool result = receivedItemName.Equals(_itemName);
        if (result)
            ++_numValidClosure;
        else
            ++_numInvalidClosure;

        return result;
    }

    public static void PrintTestResult()
    {
        StringBuilder _testResultPrint = new();
        _testResultPrint.Append("\n\n***ResultValidation Test Result*** \n\n");

        if (_numRequestOpen == _numRefreshReceived)
            _testResultPrint.Append(">>Request/Refresh validation succeeded \n");
        else
            _testResultPrint.Append(">>Request/Refresh validation failed \n");

        _testResultPrint.Append("_numRequestOpen = ").Append(_numRequestOpen).Append(" \n_numRefreshReceived = ").Append(_numRefreshReceived);

        if (_numInvalidClosure > 0)
            _testResultPrint.Append("\n\n>>Closure validation failed \n").Append("_numInvalidClosure = ").Append(_numInvalidClosure);
        else
            _testResultPrint.Append("\n\n>>Closure validation succeeded \n").Append("_numValidClosure = ").Append(_numValidClosure);

        _testResultPrint.Append("\n\n>>Update msg validation\n").Append("_numUpdateReceived = ").Append(_numUpdateReceived);

        _testResultPrint.Append("\n\n>>Status msg validation\n").Append("_numStatusReceived = ").Append(_numStatusReceived);

        Console.WriteLine(_testResultPrint.ToString());
    }

    public static int _numRequestOpen;
    public static int _numRefreshReceived;
    public static int _numUpdateReceived;
    public static int _numStatusReceived;
    public static int _numInvalidClosure;
    public static int _numValidClosure;
    readonly String _itemName;
}

public class Consumer
{
    public static ReaderWriterLockSlim APPLOCK {get;} = new();
    public static CancellationTokenSource ISEXIT { get; } = new();
    public static bool SNAPSHOT { get; set; } = true;
    public static int NUMOFITEMPERLOOP { get; set; } = 50;
    public static bool USERDISPATCH { get; set; } = false;
	public static int USERDISPATCHTIMEOUT { get; set; } = 1000;
    public static int RUNTIME { get; set; } = 60000;

    public static void PrintHelp()
    {
        Console.WriteLine("\nOptions:\n" + "  -?\tShows this usage\n\n" + "  -snapshot <true/false>\tSend item snapshot requests [default = true]\n"
                + "  -numOfItemPerLoop <50>\tSend the number of item request per loop [default = 50]\n" + "  -userDispatch <true/false>\tUse UserDispatch Operation Model [default = false]\n"
                + "  -runtime <60000>\tRun time for test case in milliseconds [default = 60000]\n" + "\n");
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
            else if (argv[idx].Equals(comparisonType: StringComparison.InvariantCultureIgnoreCase, value: "-snapshot"))
            {
                if (++idx >= count)
                {
                    PrintHelp();
                    return false;
                }
                SNAPSHOT = (argv[idx].Equals("TRUE", StringComparison.InvariantCultureIgnoreCase));
                ++idx;
            }
            else if (argv[idx].Equals(comparisonType: StringComparison.InvariantCultureIgnoreCase, value: "-numOfItemPerLoop"))
            {
                if (++idx >= count)
                {
                    PrintHelp();
                    return false;
                }
                NUMOFITEMPERLOOP = int.Parse(argv[idx]);
                ++idx;
            }
            else if (argv[idx].Equals(comparisonType: StringComparison.InvariantCultureIgnoreCase, value: "-userDispatch"))
            {
                if (++idx >= count)
                {
                    PrintHelp();
                    return false;
                }
                USERDISPATCH = (argv[idx].Equals("TRUE", StringComparison.InvariantCultureIgnoreCase));
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

            /*allow test with userDispatch or apiDispatch*/
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

            /*allow test with multiple threads on the same consumer instance*/
            ConsumerThread consumerThread1 = new(consumer);
            ConsumerThread consumerThread2 = new(consumer);
            ConsumerThread consumerThread3 = new(consumer);

            /*allow test with different item from different service name*/
            consumerThread1.OpenItem("IBM", "DIRECT_FEED");
            consumerThread2.OpenItem("TRI", "DIRECT_FEED");
            consumerThread3.OpenItem("YHOO", "DIRECT_FEED");

            /*allow test with different long run period*/
            Thread.Sleep(RUNTIME);

            ISEXIT.Cancel();
            Console.WriteLine("The consumer application is waiting for all responses back before exit...");

            Thread.Sleep(10000);

            if (USERDISPATCH)
                userDispathThread!.Stop();

            /*allow to validate different specification*/
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
