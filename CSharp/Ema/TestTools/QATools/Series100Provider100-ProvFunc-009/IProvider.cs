/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.Md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Collections.Generic;
using System.Text;
using System.Threading;

using LSEG.Ema.Access;
using LSEG.Ema.Rdm;

namespace LSEG.Ema.Example.Traning.IProvider;

class AppClient : IOmmProviderClient
{
    public long ItemHandle = 0;

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
        providerEvent.Provider.Submit(new RefreshMsg().DomainType(EmaRdm.MMT_LOGIN)
            .Name(reqMsg.Name()).NameType(EmaRdm.USER_NAME).Complete(true).Solicited(true)
            .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "Login accepted"),
            providerEvent.Handle);
    }

    void ProcessMarketPriceRequest(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
    {
        if (ItemHandle != 0)
        {
            ProcessInvalidItemRequest(reqMsg, providerEvent);
            return;
        }

        FieldList fieldList = new FieldList();
        fieldList.AddReal(22, 3990, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
        fieldList.AddReal(25, 3994, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
        fieldList.AddReal(30, 9, OmmReal.MagnitudeTypes.EXPONENT_0);
        fieldList.AddReal(31, 19, OmmReal.MagnitudeTypes.EXPONENT_0);

        providerEvent.Provider.Submit(new RefreshMsg().Name(reqMsg.Name()).ServiceId(reqMsg.ServiceId()).Solicited(true)
            .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "Refresh Completed")
            .Payload(fieldList.Complete()).Complete(true),
            providerEvent.Handle);

        ItemHandle = providerEvent.Handle;
    }

    void ProcessInvalidItemRequest(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
    {
        providerEvent.Provider.Submit(new StatusMsg().Name(reqMsg.Name()).ServiceName(reqMsg.ServiceName())
            .State(OmmState.StreamStates.CLOSED, OmmState.DataStates.SUSPECT, OmmState.StatusCodes.NOT_FOUND, "Item not found"),
            providerEvent.Handle);
    }
}

//APIQA
class AppErrorClient : IOmmProviderErrorClient
{
    public void OnInvalidHandle(long handle, string text)
    {
        Console.WriteLine("onInvalidHandle callback function" + "\nInvalid handle: " + handle + "\nError text: " + text);
    }

    public void OnInvalidUsage(String text, int errorCode)
    {
        Console.WriteLine("onInvalidUsage callback function" + "\nError text: " + text + " , Error code: " + errorCode);
    }
}
//END API QA

public class IProvider
{
    public static void Main(string[] args)
    {
        OmmProvider? provider = null;
        try
        {
            AppClient appClient = new AppClient();
            FieldList fieldList = new FieldList();

            OmmIProviderConfig config = new OmmIProviderConfig();

            provider = new OmmProvider(config.Port("14002"), appClient);
            int modifyGuaranteedOutputBuff = 10000;

            while (appClient.ItemHandle == 0)
            {
                Thread.Sleep(1000);
            }

            for (int i = 0; i < 60; i++)
            {
                fieldList.Clear();
                fieldList.AddReal(22, 3991 + i, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
                fieldList.AddReal(30, 10 + i, OmmReal.MagnitudeTypes.EXPONENT_0);
                fieldList.AddRmtes(315, new EmaBuffer(Encoding.ASCII.GetBytes("A1234567890123456789012345678901234567890123546789012345678901234567890")));
                fieldList.AddRmtes(316, new EmaBuffer(Encoding.ASCII.GetBytes("A1234567890123456789012345678901234567890123546789012345678901234567890")));
                fieldList.AddRmtes(317, new EmaBuffer(Encoding.ASCII.GetBytes("A1234567890123456789012345678901234567890123546789012345678901234567890")));
                fieldList.AddRmtes(318, new EmaBuffer(Encoding.ASCII.GetBytes("A1234567890123456789012345678901234567890123546789012345678901234567890")));
                fieldList.AddRmtes(319, new EmaBuffer(Encoding.ASCII.GetBytes("A1234567890123456789012345678901234567890123546789012345678901234567890")));
                fieldList.AddRmtes(320, new EmaBuffer(Encoding.ASCII.GetBytes("A1234567890123456789012345678901234567890123546789012345678901234567890")));
                fieldList.AddRmtes(321, new EmaBuffer(Encoding.ASCII.GetBytes("A1234567890123456789012345678901234567890123546789012345678901234567890")));
                fieldList.AddRmtes(322, new EmaBuffer(Encoding.ASCII.GetBytes("A1234567890123456789012345678901234567890123546789012345678901234567890")));
                fieldList.AddRmtes(323, new EmaBuffer(Encoding.ASCII.GetBytes("A1234567890123456789012345678901234567890123546789012345678901234567890")));
                fieldList.AddRmtes(324, new EmaBuffer(Encoding.ASCII.GetBytes("A1234567890123456789012345678901234567890123546789012345678901234567890")));
                fieldList.AddRmtes(325, new EmaBuffer(Encoding.ASCII.GetBytes("A1234567890123456789012345678901234567890123546789012345678901234567890")));
                fieldList.AddRmtes(326, new EmaBuffer(Encoding.ASCII.GetBytes("A1234567890123456789012345678901234567890123546789012345678901234567890")));
                fieldList.AddRmtes(327, new EmaBuffer(Encoding.ASCII.GetBytes("A1234567890123456789012345678901234567890123546789012345678901234567890")));
                fieldList.AddRmtes(328, new EmaBuffer(Encoding.ASCII.GetBytes("A1234567890123456789012345678901234567890123546789012345678901234567890")));
                fieldList.AddRmtes(329, new EmaBuffer(Encoding.ASCII.GetBytes("A1234567890123456789012345678901234567890123546789012345678901234567890")));
                fieldList.AddRmtes(330, new EmaBuffer(Encoding.ASCII.GetBytes("A1234567890123456789012345678901234567890123546789012345678901234567890")));
                fieldList.AddRmtes(331, new EmaBuffer(Encoding.ASCII.GetBytes("A1234567890123456789012345678901234567890123546789012345678901234567890")));
                fieldList.AddRmtes(332, new EmaBuffer(Encoding.ASCII.GetBytes("A1234567890123456789012345678901234567890123546789012345678901234567890")));
                fieldList.AddRmtes(333, new EmaBuffer(Encoding.ASCII.GetBytes("A1234567890123456789012345678901234567890123546789012345678901234567890")));
                fieldList.AddRmtes(334, new EmaBuffer(Encoding.ASCII.GetBytes("A1234567890123456789012345678901234567890123546789012345678901234567890")));
                fieldList.AddRmtes(335, new EmaBuffer(Encoding.ASCII.GetBytes("A1234567890123456789012345678901234567890123546789012345678901234567890")));
                fieldList.AddRmtes(336, new EmaBuffer(Encoding.ASCII.GetBytes("A1234567890123456789012345678901234567890123546789012345678901234567890")));
                fieldList.AddRmtes(337, new EmaBuffer(Encoding.ASCII.GetBytes("A1234567890123456789012345678901234567890123546789012345678901234567890")));
                fieldList.AddRmtes(338, new EmaBuffer(Encoding.ASCII.GetBytes("A1234567890123456789012345678901234567890123546789012345678901234567890")));

                try
                {
                    for (int j = 0; j < 100; j++)
                    {
                        provider.Submit(new UpdateMsg().Payload(fieldList.Complete()), appClient.ItemHandle);
                    }

                }
                catch (OmmException exp)
                {
                    Console.WriteLine("Exception when submit for loop : " + i);
                    Console.WriteLine(exp.Message);
                    Console.WriteLine("Modify guaranteedBuffers to increase guaranteedOutputBuffers to : " + modifyGuaranteedOutputBuff);

                    provider.ModifyIOCtl(IOCtlCode.NUM_GUARANTEED_BUFFERS, modifyGuaranteedOutputBuff, appClient.ItemHandle);

                    Console.WriteLine("Try to resubmit message for handle: " + appClient.ItemHandle);

                    provider.Submit(new UpdateMsg().Payload(fieldList.Complete()), appClient.ItemHandle);

                    Console.WriteLine("Resubmitted message for handle: " + appClient.ItemHandle);
                }

                Thread.Sleep(1000);
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
