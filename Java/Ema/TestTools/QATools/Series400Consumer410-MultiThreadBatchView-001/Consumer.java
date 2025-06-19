/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2017-2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.examples.training.consumer.series400.ex410_MP_HorizontalScaling;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.locks.ReentrantLock;

import com.refinitiv.ema.access.Msg;
import com.refinitiv.ema.access.OmmArray;
import com.refinitiv.ema.access.AckMsg;
import com.refinitiv.ema.access.ElementList;
import com.refinitiv.ema.access.GenericMsg;
import com.refinitiv.ema.access.RefreshMsg;
import com.refinitiv.ema.access.ReqMsg;
import com.refinitiv.ema.access.StatusMsg;
import com.refinitiv.ema.access.UpdateMsg;
import com.refinitiv.ema.rdm.EmaRdm;
import com.refinitiv.ema.access.EmaFactory;
import com.refinitiv.ema.access.OmmConsumer;
import com.refinitiv.ema.access.OmmConsumerClient;
import com.refinitiv.ema.access.OmmConsumerConfig;
import com.refinitiv.ema.access.OmmConsumerEvent;
import com.refinitiv.ema.access.OmmException;

class AppClient implements OmmConsumerClient
{
    public void onRefreshMsg(RefreshMsg refreshMsg, OmmConsumerEvent event)
    {
        ResultValidation closure = (ResultValidation)event.closure();

        closure.closureValidate(refreshMsg.name());

        if (refreshMsg.solicited())
            ++ResultValidation._numRefreshReceived;
        else
            ++ResultValidation._numUnsolicitedRefreshReceived;

		if (Consumer._PRINTREFRESH)
			System.out.println("\nAPIQA, Received Refresh:\n" + refreshMsg + "\n");
    }

    public void onUpdateMsg(UpdateMsg updateMsg, OmmConsumerEvent event)
    {
        ++ResultValidation._numUpdateReceived;
    }

    public void onStatusMsg(StatusMsg statusMsg, OmmConsumerEvent event)
    {
        ++ResultValidation._numStatusReceived;
    }

    public void onGenericMsg(GenericMsg genericMsg, OmmConsumerEvent consumerEvent)
    {
    }

    public void onAckMsg(AckMsg ackMsg, OmmConsumerEvent consumerEvent)
    {
    }

    public void onAllMsg(Msg msg, OmmConsumerEvent consumerEvent)
    {
    }

}

class AppThread implements Runnable
{
    OmmConsumer _consumer;
    ExecutorService _executor;
    boolean _stop;

    public AppThread(OmmConsumer consumer)
    {
        _consumer = consumer;
        _executor = Executors.newSingleThreadExecutor();
    }

    public void start()
    {
        _executor.execute(this);
    }

    public void stop()
    {
        _stop = true;
    }

    @Override
    public void run()
    {
        try
        {
            while (!_stop)
                _consumer.dispatch(Consumer._USERDISPATCHTIMEOUT);

            Thread.sleep(1000);

            _executor.shutdown();
            _executor.awaitTermination(5, TimeUnit.SECONDS);
        }
        catch (InterruptedException excp)
        {
            System.out.println(excp.getMessage());
        }
    }
}

class ConsumerThread extends AppThread
{
    static int ITEM_INDEX = 0;
    AppClient _appClient;
    ReqMsg _reqMsg;
    StringBuilder _itemStrBuilder = new StringBuilder();
    String _itemNamePrefix;
    String _serviceName;
    /*
     * Mode permits user to choose what kind of request the thread will send:
     * streaming view1 request snapshot view2 request batch snapshot requests -
     * 1st item will be what user specified, 2nd will append a "2" to the item
     * name
     */
    int _mode = 1;

    public ConsumerThread(OmmConsumer consumer)
    {
        super(consumer);
        _appClient = new AppClient();
        _reqMsg = EmaFactory.createReqMsg();
    }

    public void openItem(String item, String serviceName, int mode)
    {
        _itemNamePrefix = item;
        _serviceName = serviceName;
        _mode = mode;

        start();
    }

    void sendRequest()
    {
        while (!Consumer._ISEXIT)
        {
            try
            {
                Thread.sleep(1000);

                for (int idx = 1; idx <= Consumer._NUMOFITEMPERLOOP; ++idx)
                {
                    _itemStrBuilder.setLength(0);

                    String itemName = _itemStrBuilder.append(_itemNamePrefix).append(idx).toString();
                    if (_mode == 1)
                    {
                        // SEND STREAMING MPs with VIEW 1
                        ElementList view = EmaFactory.createElementList();
                        OmmArray array = EmaFactory.createOmmArray();

                        array.fixedWidth(2);
                        array.add(EmaFactory.createOmmArrayEntry().intValue(22));
                        array.add(EmaFactory.createOmmArrayEntry().intValue(25));

                        view.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_VIEW_TYPE, 1));
                        view.add(EmaFactory.createElementEntry().array(EmaRdm.ENAME_VIEW_DATA, array));

                        _reqMsg.clear().name(itemName).serviceName(_serviceName).interestAfterRefresh(true);
                        _reqMsg.hasView();
                        _reqMsg.payload(view);
                        Consumer._APPLOCK.lock();
                        ++ResultValidation._numRequestOpen;
                        Consumer._APPLOCK.unlock();
                    }
                    else if (_mode == 2)
                    {
                        // SEND SNAPSHOT MPs with VIEW 2
                        ElementList view = EmaFactory.createElementList();
                        OmmArray array = EmaFactory.createOmmArray();

                        // array.fixedWidth(105);
                        array.add(EmaFactory.createOmmArrayEntry().intValue(2));
                        array.add(EmaFactory.createOmmArrayEntry().intValue(3));
                        array.add(EmaFactory.createOmmArrayEntry().intValue(4));
                        array.add(EmaFactory.createOmmArrayEntry().intValue(6));
                        array.add(EmaFactory.createOmmArrayEntry().intValue(7));
                        array.add(EmaFactory.createOmmArrayEntry().intValue(8));
                        array.add(EmaFactory.createOmmArrayEntry().intValue(9));
                        array.add(EmaFactory.createOmmArrayEntry().intValue(10));
                        array.add(EmaFactory.createOmmArrayEntry().intValue(11));
                        array.add(EmaFactory.createOmmArrayEntry().intValue(12));
                        array.add(EmaFactory.createOmmArrayEntry().intValue(13));
                        array.add(EmaFactory.createOmmArrayEntry().intValue(14));
                        array.add(EmaFactory.createOmmArrayEntry().intValue(15));
                        array.add(EmaFactory.createOmmArrayEntry().intValue(16));
                        array.add(EmaFactory.createOmmArrayEntry().intValue(18));
                        array.add(EmaFactory.createOmmArrayEntry().intValue(19));
                        array.add(EmaFactory.createOmmArrayEntry().intValue(21));
                        array.add(EmaFactory.createOmmArrayEntry().intValue(22));
                        array.add(EmaFactory.createOmmArrayEntry().intValue(23));
                        array.add(EmaFactory.createOmmArrayEntry().intValue(24));
                        array.add(EmaFactory.createOmmArrayEntry().intValue(25));
                        array.add(EmaFactory.createOmmArrayEntry().intValue(26));
                        array.add(EmaFactory.createOmmArrayEntry().intValue(27));
                        array.add(EmaFactory.createOmmArrayEntry().intValue(28));
                        array.add(EmaFactory.createOmmArrayEntry().intValue(29));
                        array.add(EmaFactory.createOmmArrayEntry().intValue(30));
                        array.add(EmaFactory.createOmmArrayEntry().intValue(31));
                        array.add(EmaFactory.createOmmArrayEntry().intValue(32));
                        array.add(EmaFactory.createOmmArrayEntry().intValue(33));

                        view.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_VIEW_TYPE, 1));
                        view.add(EmaFactory.createElementEntry().array(EmaRdm.ENAME_VIEW_DATA, array));

                        _reqMsg.clear().name(itemName).serviceName(_serviceName).interestAfterRefresh(false);
                        _reqMsg.hasView();
                        _reqMsg.payload(view);
                        Consumer._APPLOCK.lock();
                        ++ResultValidation._numRequestOpen;
                        Consumer._APPLOCK.unlock();
                    }
                    else if (_mode == 3)
                    {
                        // SEND STREAMING MPs with BATCH
                        ElementList batch = EmaFactory.createElementList();
                        OmmArray array = EmaFactory.createOmmArray();

                        array.add(EmaFactory.createOmmArrayEntry().ascii("BATCHITEM1" + itemName));
                        array.add(EmaFactory.createOmmArrayEntry().ascii("BATCHITEM2" + itemName));

                        batch.add(EmaFactory.createElementEntry().array(EmaRdm.ENAME_BATCH_ITEM_LIST, array));
                        _reqMsg.clear().hasBatch();
                        _reqMsg.interestAfterRefresh(true).serviceName(_serviceName).payload(batch);
                        Consumer._APPLOCK.lock();
                        ResultValidation._numRequestOpen += 2;
                        Consumer._APPLOCK.unlock();
                    }
                    _consumer.registerClient(_reqMsg, _appClient, new ResultValidation(itemName));
                }
            }
            catch (InterruptedException e)
            {
                e.printStackTrace();
            }
        }
    }

    @Override
    public void run()
    {
        try
        {
            sendRequest();

            Thread.sleep(1000);

            _executor.shutdown();
            _executor.awaitTermination(5, TimeUnit.SECONDS);
        }
        catch (InterruptedException excp)
        {
            System.out.println(excp.getMessage());
        }
    }
}

class ResultValidation
{
    ResultValidation(String itemName)
    {
        _itemName = itemName;
    }

    boolean closureValidate(String receivedItemName)
    {
        boolean result = receivedItemName.equals(_itemName);
        if (result)
            ++ResultValidation._numValidClosure;
        else
            ++ResultValidation._numInvalidClosure;

        return result;
    }

    static void printTestResult()
    {
        StringBuilder _testResultPrint = new StringBuilder();
        _testResultPrint.append("\n\n***ResultValidation Test Result*** \n\n");

        if (_numRequestOpen == _numRefreshReceived)
            _testResultPrint.append(">>Request/Refresh validation succeeded \n");
        else
            _testResultPrint.append(">>Request/Refresh validation failed \n");

        _testResultPrint.append("_numRequestOpen = ").append(_numRequestOpen).append(" \n_numRefreshReceived = ").append(_numRefreshReceived);

        if (ResultValidation._numInvalidClosure > 0)
            _testResultPrint.append("\n\n>>Closure validation failed \n").append("_numInvalidClosure = ").append(_numInvalidClosure);
        else
            _testResultPrint.append("\n\n>>Closure validation succeeded \n").append("_numValidClosure = ").append(_numValidClosure);

        _testResultPrint.append("\n\n>>Update msg validation\n").append("_numUpdateReceived = ").append(_numUpdateReceived);

        _testResultPrint.append("\n\n>>Status msg validation\n").append("_numStatusReceived = ").append(_numStatusReceived);

        System.out.println(_testResultPrint.toString());
    }

    static int _numRequestOpen;
    static int _numRefreshReceived;
    static int _numUnsolicitedRefreshReceived;
    static int _numUpdateReceived;
    static int _numStatusReceived;
    static int _numInvalidClosure;
    static int _numValidClosure;
    String _itemName;
}

public class Consumer
{
    public static ReentrantLock _APPLOCK = new java.util.concurrent.locks.ReentrantLock();
    public static boolean _ISEXIT = false;
    public static int _NUMOFITEMPERLOOP = 1;
    public static boolean _USERDISPATCH = false;
    public static boolean _PRINTREFRESH = true;
    public static int _USERDISPATCHTIMEOUT = 1000;
    public static int _RUNTIME = 60000;

    public static void printHelp()
    {

        System.out.println("\nOptions:\n" + "  -?\tShows this usage\n\n" + "  -snapshot \tSend item snapshot requests [default = true]\n"
                + "  -numOfItemPerLoop \tSend the number of item request per loop [default = 50]\n" + "  -userDispatch \tUse UserDispatch Operation Model [default = false]\n"
                + "  -userDispatchTimeout \tSet dispatch timeout period in microseconds if UserDispatch Operation Model [default = 1000]\n"
                + "  -runtime \tRun time for test case in milliseconds [default = 60000]\n" + "\n");
    }

    public static boolean readCommandlineArgs(String[] argv)
    {
        int count = argv.length;
        int idx = 0;

        while (idx < count)
        {
            if (0 == argv[idx].compareTo("-?"))
            {
                printHelp();
                return false;
            }
            else if (0 == argv[idx].compareToIgnoreCase("-userDispatch"))
            {
                if (++idx >= count)
                {
                    printHelp();
                    return false;
                }
                Consumer._USERDISPATCH = ((argv[idx].compareToIgnoreCase("TRUE") == 0) ? true : false);
                ++idx;
            }
            else if (0 == argv[idx].compareToIgnoreCase("-numOfItemPerLoop"))
            {
                if (++idx >= count)
                {
                    printHelp();
                    return false;
                }
                Consumer._NUMOFITEMPERLOOP = Integer.parseInt(argv[idx]);
                ++idx;
            }
            else if (0 == argv[idx].compareToIgnoreCase("-userDispatchTimeout"))
            {
                if (++idx >= count)
                {
                    printHelp();
                    return false;
                }
                Consumer._USERDISPATCHTIMEOUT = Integer.parseInt(argv[idx]);
                ++idx;
            }
            else if (0 == argv[idx].compareToIgnoreCase("-runtime"))
            {
                if (++idx >= count)
                {
                    printHelp();
                    return false;
                }
                Consumer._RUNTIME = Integer.parseInt(argv[idx]);
                ++idx;
            }
            else if (0 == argv[idx].compareToIgnoreCase("-printRefresh"))
            {
                if (++idx >= count)
                {
                    printHelp();
                    return false;
                }
                Consumer._PRINTREFRESH = ((argv[idx].compareToIgnoreCase("TRUE") == 0) ? true : false);
                ++idx;
            }
            else
            {
                printHelp();
                return false;
            }
        }

        return true;
    }

    public static void main(String[] args)
    {
        OmmConsumer consumer = null;
        AppThread userDispathThread = null;

        try
        {
            if (!readCommandlineArgs(args))
                return;

            /* allow test with userDispatch or apiDispatch */
            if (Consumer._USERDISPATCH)
            {
                consumer = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig().host("localhost:14002").username("user").operationModel(OmmConsumerConfig.OperationModel.USER_DISPATCH));
                userDispathThread = new AppThread(consumer);
                userDispathThread.start();
            }
            else
                consumer = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig().host("localhost:14002").username("user"));

            System.out.print("The consumer application starts testing with ");
            System.out.println(Consumer._USERDISPATCH ? "UserDispatch..." : "ApiDispatch...");

            /* allow test with multiple threads on the same consumer instance */
            ConsumerThread consumerThread1 = new ConsumerThread(consumer);
            ConsumerThread consumerThread2 = new ConsumerThread(consumer);
            ConsumerThread consumerThread3 = new ConsumerThread(consumer);

            /* allow test with different item from different service name */
            consumerThread1.openItem("TRI", "DIRECT_FEED", 1);
            Thread.sleep(1000);
            consumerThread2.openItem("TRI", "DIRECT_FEED", 2);
            consumerThread3.openItem("CSCO.O", "DIRECT_FEED", 3);

            /* allow test with different long run period */
            Thread.sleep(Consumer._RUNTIME);

            _ISEXIT = true;
            System.out.println("The consumer application is waiting for all responses back before exit...");

            Thread.sleep(10000);

            if (Consumer._USERDISPATCH)
                userDispathThread.stop();

            /* allow to validate different specification */
            ResultValidation.printTestResult();

        }
        catch (InterruptedException | OmmException excp)
        {
            System.out.println(excp);
        }
        finally
        {
            if (consumer != null)
                consumer.uninitialize();
        }
    }
}
