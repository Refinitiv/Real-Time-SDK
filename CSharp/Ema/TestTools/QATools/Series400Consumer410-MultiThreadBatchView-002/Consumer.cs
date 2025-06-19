/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Access;
using LSEG.Ema.Rdm;
using Microsoft.IdentityModel.Tokens;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Threading;
using static LSEG.Ema.Access.OmmConsumerConfig;
using static LSEG.Ema.Access.OmmState.DataStates;
using static LSEG.Ema.Access.OmmState.StatusCodes;

namespace LSEG.Ema.Example.Traning.Consumer;

public class Consumer
{
    private const string TorPath = "tor_500.txt";
    private readonly IEmaConsumerService emaConsumerService = new EmaConsumerServiceImpl();

    public static void Main()
    {
        Console.WriteLine("Starting Consumer");
        Consumer demo = new();

        List<string>? unifiedRequest = null;
        try
        {
            unifiedRequest = PopulateUnifiedRequestList().ToList();
        }
        catch (Exception e)
        {
            // TODO Auto-generated catch block
            Console.WriteLine(e.StackTrace);
        }

        try
        {
            demo.emaConsumerService.FetchMarketData(unifiedRequest!);
            Console.WriteLine("Exiting Main");
        }
        catch (Exception e)
        {
            // TODO Auto-generated catch block
            Console.WriteLine("####### Exception (In main): " + e.Message);
            Console.WriteLine(e.StackTrace);
        }
    }

    private static IEnumerable<string> PopulateUnifiedRequestList()
    {
        foreach (var line in File.ReadLines(TorPath).Where(l => l is not null))
        {
            yield return line.Trim();
        }
    }
}

internal class EmaClient : IOmmConsumerClient
{
    private readonly CountdownEvent countDown;
    private readonly OmmConsumer ommConsumer;

    public EmaClient(CountdownEvent countDown, OmmConsumer ommConsumer)
    {
        this.countDown = countDown;
        this.ommConsumer = ommConsumer;
    }

    public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmConsumerEvent @event)
    {
        String name = refreshMsg.Name();

        Console.WriteLine("onRefreshMsg, Name: " + name + "  OMMConsumer: " + ommConsumer.ConsumerName);
        Console.WriteLine("Item Name: " + (refreshMsg.HasName ? refreshMsg.Name() : "<not set>"));
        Console.WriteLine("Item State: " + refreshMsg.State());

        ommConsumer.Unregister(@event.Handle);//--unregister item individually
        countDown.Signal();
    }

    public void OnStatusMsg(StatusMsg statusMsg, IOmmConsumerEvent @event)
    {
        string name = statusMsg.HasName ? statusMsg.Name() : "StatusMsg";
        Console.WriteLine("onStatusMsg, Name: " + name + "  OMMConsumer: " + ommConsumer.ConsumerName + " State; " + statusMsg.State());
        OmmState state = statusMsg.State();

        if (EvaluateError(state))
        {
            countDown.Signal();
            ommConsumer.Unregister(@event.Handle);//--unregister item individually
        }
    }

    public void OnUpdateMsg(UpdateMsg updateMsg, IOmmConsumerEvent _)
    {
        Console.WriteLine("onUpdateMsg, Name: " + updateMsg.Name());
    }

    public void OnAckMsg(AckMsg arg0, IOmmConsumerEvent arg1)
    {
        Console.WriteLine("onAckMsg: " + arg0);
    }

    public void OnAllMsg(Msg arg0, IOmmConsumerEvent arg1)
    {
        string name = arg0.HasName ? arg0.Name() : "AllMsg";
        Console.WriteLine("onAllMsg: " + name);
    }

    public void OnGenericMsg(GenericMsg arg0, IOmmConsumerEvent arg1)
    {
        Console.WriteLine("inside onGenericMsg: " + arg0);
    }

    private static bool EvaluateError(OmmState state) =>
        new[] { NOT_AUTHORIZED, NOT_FOUND, TIMEOUT }.Contains(state.StatusCode) || state.DataState == SUSPECT;
}

internal class EmaConsumer
{
    private readonly OmmConsumer ommConsumer;
    private readonly EmaClient emaClient;
    private readonly CountdownEvent countDownLatch;
    private readonly RequestMsg reqMsg;
    private readonly EmaConsumerServiceImpl myCaller;
    private readonly Thread thread;

    public EmaConsumer(OmmConsumer ommConsumer, EmaClient emaClient,
            CountdownEvent countDownLatch, RequestMsg reqMsg, EmaConsumerServiceImpl myCaller)
    {
        this.ommConsumer = ommConsumer;
        this.emaClient = emaClient;
        this.countDownLatch = countDownLatch;
        this.reqMsg = reqMsg;
        this.myCaller = myCaller;
        thread = new Thread(new ParameterizedThreadStart(_ => Run()));
    }

    public void Start() => thread.Start();

    private void Run()
    {
        try
        {
            //User dispatch
            ommConsumer.RegisterClient(reqMsg, emaClient);
            while (countDownLatch.CurrentCount != 0)
            {
                ommConsumer.Dispatch(1000); //Let API do the call backs
            }
        }
        catch (Exception excp)
        {
            Console.WriteLine("+++++Exception while executing dispatch, errorMessage:  " + excp.Message);
            Console.WriteLine(excp.StackTrace);
        }
        finally
        {
            if (null != ommConsumer)
            {
                ommConsumer.Uninitialize();
                myCaller.IncCount();
            }
        }
        return;
    }
}

internal interface IEmaConsumerService
{
    void FetchMarketData(List<string> unifiedRequest);
}

internal class EmaConsumerServiceImpl : IEmaConsumerService
{
    private readonly object _syncObject = new();
    private int threadCount = 0;

    public void FetchMarketData(List<string> unifiedRequest)
    {
        //https://learn.microsoft.com/en-us/dotnet/standard/threading/countdownevent
        try
        {
            for (int i = 0; i < 20; i++)
            {
                OmmConsumer? ommConsumer = FetchConsumerFromFactory();
                CountdownEvent countDown = new(unifiedRequest.Count);
                EmaClient emaClient = new(countDown, ommConsumer);
                var reqMsg = PrepareRequestObject(unifiedRequest);
                EmaConsumer? emaConsumer = new(ommConsumer, emaClient, countDown, reqMsg, this);
                emaConsumer.Start();
            }
            Thread.Sleep(12000);
            while (threadCount < 20)
                Thread.Sleep(2000);
        }
        catch (OmmInvalidUsageException excp)
        {
            Console.WriteLine(excp.Message);
            Console.WriteLine(excp.StackTrace);
        }
        return;
    }

    public void IncCount()
    {
        lock (_syncObject)
        {
            threadCount++;
        }
    }

    private static OmmConsumer FetchConsumerFromFactory()
    {
        return new OmmConsumer(new OmmConsumerConfig().UserName("User")
                .ApplicationId("256").ConsumerName("Consumer_1")
                .OperationModel(OperationModelMode.USER_DISPATCH));
    }

    private static RequestMsg PrepareRequestObject(List<string> unifiedRequest)
    {
        return new RequestMsg()
                .Clear()
                .DomainType(EmaRdm.MMT_MARKET_PRICE)
                .ServiceName("DIRECT_FEED")
                .Payload(
                        GetDynamicBatchRequestView("3,6,15,21,79,134,1059",
                                unifiedRequest))
                .InterestAfterRefresh(false);
    }

    private static ElementList GetDynamicBatchRequestView(String viewFieldList,
            List<string> unifiedRequest)
    {
        ElementList finalElements = new();
        // batch input
        OmmArray batchInputArray = CreateBatchArray(unifiedRequest);
        finalElements.AddArray(
                EmaRdm.ENAME_BATCH_ITEM_LIST, batchInputArray);
        // View input
        OmmArray viewElement = new();
        List<string> viewFieldIdList = viewFieldList.Trim().Split(",").ToList();
        if (!viewFieldList.IsNullOrEmpty())
        {
            foreach (string field in viewFieldIdList)
            {
                viewElement.AddInt(
                        int.Parse(field));
            }
            finalElements.AddUInt(EmaRdm.ENAME_VIEW_TYPE, 1);
            finalElements.AddArray(EmaRdm.ENAME_VIEW_DATA, viewElement.Complete());
        }
		finalElements.Complete();
        return finalElements;
    }

    private static OmmArray CreateBatchArray(List<string> unifiedRequest)
    {
        OmmArray ommArray = new();
        if (!unifiedRequest.IsNullOrEmpty())
        {
            unifiedRequest.ForEach(
                    requestString =>
                    {
                        ommArray.AddAscii(requestString);
                    });
        }
		ommArray.Complete();
        return ommArray;
    }
}

internal class MarketDataError
{
    public string? ErrorCode { get; set; }
    public string? Information { get; set; }

    public override string ToString() => "MarketDataError [errorCode=" + ErrorCode
                + ", information=" + Information + "]";
}

internal class MarketDataRequest
{
    public ISet<string>? RequestRics { get; set; }
    public ISet<string>? Currencies { get; set; }
    public bool IsDelayed { get; set; } = true;
}