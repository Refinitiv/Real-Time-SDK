/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.Md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Access;
using LSEG.Ema.Rdm;
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

class ConsumerThread
{
    readonly AppClient _appClient;
    readonly RequestMsg _reqMsg;
    readonly StringBuilder _itemStrBuilder = new();
	String? _itemNamePrefix;
	String? _serviceName;
	bool _stopSendReq = false;
    private readonly OmmConsumer _consumer;
    private readonly Thread _thread;

    public ConsumerThread(OmmConsumer consumer)
	{
		_appClient = new();
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
	
	void SendRequest()
	{
        while (!Consumer.ISEXIT.IsCancellationRequested)
        {
			++Consumer.ITEM_INDEX;
			Thread.Sleep(1000);
			
				for (int idx = 1; idx <= Consumer.NUMOFITEMPERLOOP && !_stopSendReq ; ++idx)
				{
					_itemStrBuilder.Length = 0;
					 
					String itemName = _itemStrBuilder.Append(_itemNamePrefix).Append('_').Append(Consumer.ITEM_INDEX).Append('_').Append(idx).ToString();
					 
					Console.WriteLine(itemName);
					 
				_reqMsg.Clear().Name(itemName).ServiceName(_serviceName!).InterestAfterRefresh(!Consumer.SNAPSHOT);
					
				Consumer.APPLOCK.EnterWriteLock();
				if (!Consumer.SNAPSHOT && Consumer.START_WAITING_AFTER_NUM_REQ > 0 && ResultValidation._numRequestOpen == Consumer.START_WAITING_AFTER_NUM_REQ)
					Thread.Sleep(Consumer.START_WAITING_TIME_AFTER_NUM_REQ);
					
				_consumer.RegisterClient(_reqMsg, _appClient, new ResultValidation(itemName));
					
				++ResultValidation._numRequestOpen;
				Consumer.APPLOCK.ExitWriteLock();
				}
				_stopSendReq = true;
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
		
		int refreshFactor = Consumer.NUMOFKILLPROVIDER + 1;
		if (Consumer.SNAPSHOT)
			refreshFactor = 1;
		
		if (!Consumer.SNAPSHOT && Consumer.START_WAITING_AFTER_NUM_REQ > 0 &&
			(_numRequestOpen + Consumer.START_WAITING_AFTER_NUM_REQ*(refreshFactor-1)) == _numRefreshReceived )
		{
			_testResultPrint.Append(">>Request/Refresh validation succeeded \n");
			_testResultPrint.Append(Consumer.START_WAITING_AFTER_NUM_REQ).Append(" Requests has been recovered for ").Append(Consumer.NUMOFKILLPROVIDER).Append( " times\n");
		}
		else if ( _numRequestOpen*refreshFactor == _numRefreshReceived )
			_testResultPrint.Append(">>Request/Refresh validation succeeded with ").Append(Consumer.NUMOFKILLPROVIDER).Append( " times of recovery\n");
		else
			_testResultPrint.Append(">>Request/Refresh validation failed with ").Append(Consumer.NUMOFKILLPROVIDER).Append( " times of recovery\n");
		
		_testResultPrint.Append("_numRequestOpen = ").Append(_numRequestOpen).Append(" \n_numRefreshReceived = ").Append(_numRefreshReceived);
		
		if (_numInvalidClosure > 0 )
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
    public String _itemName;
}

public class Consumer 
{
	public static ReaderWriterLockSlim APPLOCK {get;} = new();
	public static CancellationTokenSource ISEXIT { get; } = new();
	public static int ITEM_INDEX { get; set; }

    public static bool SNAPSHOT { get; set; } = true;
	public static int NUMOFITEMPERLOOP { get; set; } = 50;
	public static bool USERDISPATCH { get; set; } = false;
	public static int USERDISPATCHTIMEOUT { get; set; } = 1000;
	public static int RUNTIME { get; set; } = 60000;
	public static int NUMOFKILLPROVIDER { get; set; } = 0;
	public static int START_WAITING_AFTER_NUM_REQ { get; set; }
    public static int START_WAITING_TIME_AFTER_NUM_REQ { get; set; }

    public static void PrintHelp()
	{
		Console.WriteLine("\nOptions:\n"
		+ "  -?\tShows this usage\n\n"
		+ "  -snapshot \tSend item snapshot requests [default = true]\n"
		+ "  -numOfItemPerLoop \tSend the number of item request per loop [default = 50]\n"
		+ "  -userDispatch \tUse UserDispatch Operation Model [default = false]\n"
		+ "  -userDispatchTimeout \tSet dispatch timeout period in microseconds if UserDispatch Operation Model [default = 1000]\n"
		+ "  -numOfKillProvier [default = 1]\n"
		+ " -startWaitAfterNumReqs \tconsumer stops sending requests after num requests has been sent. Default means sending all, only work with -snapshot false  [default = 0], \n"
		+ " -startWaitTimeAfterNumReqs \tconsumer waits for the period time before continuing sending requests. Default means no waiting, only work with -snapshot false  [default = 0], \n"
		+ "  -runtime \tRun time for test case in milliseconds [default = 60000]\n"
		+ "\n" );
	}
	
	public static bool ReadCommandlineArgs(String[] argv)
	{
		int count = argv.Length;
		int idx = 0;
		
		while ( idx < count )
		{
			if ( 0 == argv[idx].CompareTo("-?") )
			{
				PrintHelp();
				return false;
			}
			else if ( argv[idx].Equals(comparisonType: StringComparison.InvariantCultureIgnoreCase, value: "-snapshot") )
			{
				if ( ++idx >= count )
				{
					PrintHelp();
					return false;
				}
                SNAPSHOT = (argv[idx].Equals("TRUE", StringComparison.InvariantCultureIgnoreCase));
				++idx;
			}
			else if ( argv[idx].Equals(comparisonType: StringComparison.InvariantCultureIgnoreCase, value: "-numOfItemPerLoop") )
			{
				if ( ++idx >= count )
				{
					PrintHelp();
					return false;
				}
                NUMOFITEMPERLOOP = int.Parse(argv[idx]);
				++idx;
			}
			else if ( argv[idx].Equals(comparisonType: StringComparison.InvariantCultureIgnoreCase, value: "-userDispatch") )
			{
				if ( ++idx >= count )
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
			else if ( argv[idx].Equals(comparisonType: StringComparison.InvariantCultureIgnoreCase, value: "-numOfKillProvier") )
			{
				if ( ++idx >= count )
				{
					PrintHelp();
					return false;
				}
                NUMOFKILLPROVIDER = (int.Parse(argv[idx]) > 0 ? int.Parse(argv[idx]) : 0);
				++idx;
			}
			else if ( argv[idx].Equals(comparisonType: StringComparison.InvariantCultureIgnoreCase, value: "-startWaitAfterNumReqs") )
			{
				if ( ++idx >= count )
				{
					PrintHelp();
					return false;
				}
                START_WAITING_AFTER_NUM_REQ = (int.Parse(argv[idx]) > 0 ? int.Parse(argv[idx]) : 0);
				++idx;
			}
			else if ( argv[idx].Equals(comparisonType: StringComparison.InvariantCultureIgnoreCase, value: "-startWaitTimeAfterNumReqs") )
			{
				if ( ++idx >= count )
				{
					PrintHelp();
					return false;
				}
                START_WAITING_TIME_AFTER_NUM_REQ = (int.Parse(argv[idx]) > 0 ? int.Parse(argv[idx]) : 0);
				++idx;
			}
			else if ( argv[idx].Equals(comparisonType: StringComparison.InvariantCultureIgnoreCase, value: "-runtime") )
			{
				if ( ++idx >= count )
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
			if ( !ReadCommandlineArgs(args) ) return;
			
			/*allow test with userDispatch or apiDispatch*/
			if (USERDISPATCH)
			{
				consumer = new(new OmmConsumerConfig().UserName("user")
																						.OperationModel(OmmConsumerConfig.OperationModelMode.USER_DISPATCH));
				userDispathThread = new AppThread(consumer);
				userDispathThread.Start();
			}
			else
				consumer = new(new OmmConsumerConfig().UserName("user"));

			Console.Write("The consumer application starts testing with ");
			Console.WriteLine(USERDISPATCH ? "UserDispatch..." : "ApiDispatch..." );
			
			/*allow test with multiple threads On the same consumer instance*/
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
			
			Thread.Sleep(5000);
			
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
//END APIQA
