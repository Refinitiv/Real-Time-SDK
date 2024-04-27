/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.Md for details.                  --
 *|           Copyright (C) 2024 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Collections.Generic;
using System.Text;

using LSEG.Ema.Access;
using LSEG.Ema.Rdm;

namespace LSEG.Ema.Example.Traning.IProvider;

class ReqInfo
{
    private long m_Handle;

    public void Value(long handle)
    {
        m_Handle = handle;
    }

    public long Handle()
    {
        return m_Handle;
    }
}

class AppClient : IOmmProviderClient
{
    public List<ReqInfo> reqInfoList = new List<ReqInfo>(1000);
    public int numberOfRequestItems = 0;
    public FieldList fieldList = new FieldList();
    public string statusText = "Refresh Completed";

    private RefreshMsg refreshMsg = new RefreshMsg();
    private DateTime startRefreshTime;
    private DateTime endRefreshTime;

    public void OnReqMsg(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
    {
        switch (reqMsg.DomainType())
        {
            case EmaRdm.MMT_LOGIN:
                ProcessLoginRequest(reqMsg, providerEvent);
                break;
            case EmaRdm.MMT_MARKET_PRICE:
                ProcessMarketPriceRequest(reqMsg, providerEvent);
                break;
            default:
                ProcessInvalidItemRequest(reqMsg, providerEvent);
                break;
        }
    }

    void ProcessLoginRequest(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
    {
        providerEvent.Provider.Submit(refreshMsg.Clear().DomainType(EmaRdm.MMT_LOGIN)
            .Name(reqMsg.Name()).NameType(EmaRdm.USER_NAME)
            .Complete(true).Solicited(true)
            .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "Login accepted"),
            providerEvent.Handle);
    }

    void ProcessMarketPriceRequest(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
    {
        if (numberOfRequestItems == 1000)
        {
            providerEvent.Provider.Submit(new StatusMsg().Name(reqMsg.Name()).ServiceName(reqMsg.ServiceName())
                .DomainType(reqMsg.DomainType())
                .State(OmmState.StreamStates.CLOSED, OmmState.DataStates.SUSPECT, OmmState.StatusCodes.TOO_MANY_ITEMS,
                    "Request more than 1000 items"),
                providerEvent.Handle);
            return;
        }

        fieldList.Clear();
        fieldList.AddInt(1, 6560);
        fieldList.AddInt(2, 66);
        fieldList.AddInt(3855, 52832001);
        fieldList.AddRmtes(296, new EmaBuffer(Encoding.ASCII.GetBytes("BOS")));
        fieldList.AddTime(375, 21, 0);
        fieldList.AddTime(1025, 14, 40, 32);
        fieldList.AddReal(22, 14400, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
        fieldList.AddReal(25, 14700, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
        fieldList.AddReal(30, 9, OmmReal.MagnitudeTypes.EXPONENT_0);
        fieldList.AddReal(31, 19, OmmReal.MagnitudeTypes.EXPONENT_0);

        providerEvent.Provider.Submit(new RefreshMsg().ServiceId(reqMsg.ServiceId()).Name(reqMsg.Name())
            .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, statusText)
            .Solicited(true).Payload(fieldList.Complete()).Complete(true),
            providerEvent.Handle);

        if (++numberOfRequestItems == 1)
        {
            startRefreshTime = DateTime.Now;
        }
        else if (numberOfRequestItems == 1000)
        {
            endRefreshTime = DateTime.Now;

            TimeSpan timeSpent = endRefreshTime - startRefreshTime;

            Console.WriteLine("total refresh count = " + numberOfRequestItems +
                    "\ttotal time = " + timeSpent.TotalSeconds + " sec" +
                    "\tupdate rate = " + numberOfRequestItems / timeSpent.TotalSeconds + " refresh per sec");
        }

        ReqInfo reqInfo = new ReqInfo();
        reqInfo.Value(providerEvent.Handle);
        reqInfoList.Add(reqInfo);
    }

    void ProcessInvalidItemRequest(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
    {
        providerEvent.Provider.Submit(new StatusMsg().DomainType(reqMsg.DomainType())
            .Name(reqMsg.Name()).ServiceName(reqMsg.ServiceName())
            .State(OmmState.StreamStates.CLOSED, OmmState.DataStates.SUSPECT, OmmState.StatusCodes.NOT_FOUND, "Item not found"),
            providerEvent.Handle);
    }
}

public class IProvider
{
    public static void Main(string[] args)
    {
        OmmProvider? provider = null;
        try
        {
            AppClient appClient = new AppClient();

            provider = new OmmProvider(new OmmIProviderConfig().
                    OperationModel(OmmIProviderConfig.OperationModelMode.USER_DISPATCH), appClient);

            while (appClient.numberOfRequestItems < 1000)
            {
                provider.Dispatch(1000);
            }

            int updateMsgCount = 0;
            UpdateMsg updateMsg = new UpdateMsg();
            DateTime startUpdateTime;
            ReqInfo reqInfo;
            bool submitting = true;
            int numberOfSeconds = 0;
            int index = 0;
            int submittingTime = 300;

            startUpdateTime = DateTime.Now;

            while (submitting)
            {
                for (index = 0; index < appClient.numberOfRequestItems; index++)
                {
                    reqInfo = appClient.reqInfoList[index];

                    appClient.fieldList.Clear();
                    appClient.fieldList.AddTime(1025, 14, 40, 32);
                    appClient.fieldList.AddInt(3855, 52832001);
                    appClient.fieldList.AddReal(22, 14400 + (((reqInfo.Handle() & 0x1) == 1) ? 1 : 10), OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
                    appClient.fieldList.AddReal(30, 10 + (((reqInfo.Handle() & 0x1) == 1) ? 10 : 20), OmmReal.MagnitudeTypes.EXPONENT_0);
                    appClient.fieldList.AddRmtes(296, new EmaBuffer(Encoding.ASCII.GetBytes("NAS")));

                    provider.Submit(updateMsg.Clear().Payload(appClient.fieldList.Complete()), reqInfo.Handle());

                    if (index % 10 == 0)
                        provider.Dispatch(10);

                    updateMsgCount++;
                }

                TimeSpan timeSpent = DateTime.Now - startUpdateTime;

                if (timeSpent.TotalSeconds > 1)
                {
                    Console.WriteLine("update count = " + updateMsgCount +
                            "\tupdate rate = " + (int)(updateMsgCount / timeSpent.TotalSeconds) + " update per sec");

                    updateMsgCount = 0;

                    startUpdateTime = DateTime.Now;

                    if (++numberOfSeconds == submittingTime)
                        submitting = false;
                }
            }
        }
        catch (OmmException excp)
        {
            Console.WriteLine(excp.Message);
        }
        finally
        {
            provider?.Uninitialize();
        }
    }
}
