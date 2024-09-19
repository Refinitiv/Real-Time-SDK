/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.Md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Threading;

using LSEG.Ema.Access;
using LSEG.Ema.Rdm;

namespace LSEG.Ema.Example.Traning.IProvider;

class AppClient : IOmmProviderClient
{
    public long ItemHandle = 0;
    public long LoginHandle = 0;

    private DataDictionary m_DataDictionary = new DataDictionary();
    private bool m_FldDictComplete = false;
    private bool m_EnumTypeComplete = false;
    private bool m_DumpDictionary = false;

    public AppClient(string[] args)
    {
        int idx = 0;

        while (idx < args.Length)
        {
            if (args[idx].Equals("-dumpDictionary"))
            {
                m_DumpDictionary = true;
            }

            ++idx;
        }
    }

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

    public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmProviderEvent providerEvent)
    {
        Console.WriteLine("Received Refresh. Item Handle: " + providerEvent.Handle + " Closure: " + providerEvent.Closure);
        Console.WriteLine("Item Name: " + (refreshMsg.HasName ? refreshMsg.Name() : "<not set>"));
        Console.WriteLine("Service Name: " + (refreshMsg.HasServiceName ? refreshMsg.ServiceName() : "<not set>"));
        Console.WriteLine("Item State: " + refreshMsg.State());

        Decode(refreshMsg, refreshMsg.Complete());

        Console.WriteLine();
    }

    public void OnStatusMsg(StatusMsg statusMsg, IOmmProviderEvent providerEvent)
    {
        Console.WriteLine("Received Status. Item Handle: " + providerEvent.Handle + " Closure: " + providerEvent.Closure);
        Console.WriteLine("Item Name: " + (statusMsg.HasName ? statusMsg.Name() : "<not set>"));
        Console.WriteLine("Service Name: " + (statusMsg.HasServiceName ? statusMsg.ServiceName() : "<not set>"));

        if (statusMsg.HasState)
            Console.WriteLine("Item State: " + statusMsg.State());

        Console.WriteLine();
    }

    void ProcessLoginRequest(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
    {
        providerEvent.Provider.Submit(new RefreshMsg().DomainType(EmaRdm.MMT_LOGIN).Name(reqMsg.Name()).NameType(EmaRdm.USER_NAME)
            .Complete(true).Solicited(true)
            .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "Login accepted")
            .Attrib(new ElementList().Complete()),
            providerEvent.Handle);

        LoginHandle = providerEvent.Handle;
    }

    void ProcessMarketPriceRequest(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
    {
        if (ItemHandle != 0)
        {
            ProcessInvalidItemRequest(reqMsg, providerEvent);
            return;
        }

        FieldList fieldList = new FieldList();

        fieldList.AddAscii(3, reqMsg.Name());
        fieldList.AddEnumValue(15, 840);
        fieldList.AddReal(21, 3900, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
        fieldList.AddReal(22, 3990, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
        fieldList.AddReal(25, 3994, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
        fieldList.AddReal(30, 9, OmmReal.MagnitudeTypes.EXPONENT_0);
        fieldList.AddReal(31, 19, OmmReal.MagnitudeTypes.EXPONENT_0);

        providerEvent.Provider.Submit(new RefreshMsg().ServiceName(reqMsg.ServiceName()).Name(reqMsg.Name())
            .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "Refresh Completed")
            .Solicited(true).Payload(fieldList.Complete()).Complete(true),
            providerEvent.Handle);

        ItemHandle = providerEvent.Handle;
    }

    void ProcessInvalidItemRequest(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
    {
        providerEvent.Provider.Submit(new StatusMsg().Name(reqMsg.Name()).ServiceName(reqMsg.ServiceName())
            .DomainType(reqMsg.DomainType())
            .State(OmmState.StreamStates.CLOSED, OmmState.DataStates.SUSPECT, OmmState.StatusCodes.NOT_FOUND, "Item not found"),
            providerEvent.Handle);
    }

    void Decode(Msg msg, bool complete)
    {
        switch (msg.Payload().DataType)
        {
            case DataType.DataTypes.SERIES:

                if (msg.Name().Equals("RWFFld"))
                {
                    m_DataDictionary.DecodeFieldDictionary(msg.Payload().Series(), EmaRdm.DICTIONARY_NORMAL);

                    if (complete)
                    {
                        m_FldDictComplete = true;
                    }
                }
                else if (msg.Name().Equals("RWFEnum"))
                {
                    m_DataDictionary.DecodeEnumTypeDictionary(msg.Payload().Series(), EmaRdm.DICTIONARY_NORMAL);

                    if (complete)
                    {
                        m_EnumTypeComplete = true;
                    }
                }

                if (m_FldDictComplete && m_EnumTypeComplete)
                {
                    Console.WriteLine();
                    Console.WriteLine("\nDictionary download complete");
                    Console.WriteLine("Dictionary Id : " + m_DataDictionary.DictionaryId);
                    Console.WriteLine("Dictionary field version : " + m_DataDictionary.FieldVersion);
                    Console.WriteLine("Number of dictionary entries : " + m_DataDictionary.Entries().Count);

                    if (m_DumpDictionary)
                    {
                        Console.WriteLine(m_DataDictionary);
                    }
                }

                break;
            default:
                break;
        }
    }
}

public class IProvider
{
    public static void Main(string[] args)
    {
        OmmProvider? provider = null;

        try
        {
            AppClient appClient = new AppClient(args);
            FieldList fieldList = new FieldList();
            UpdateMsg updateMsg = new UpdateMsg();

            provider = new OmmProvider(new OmmIProviderConfig(), appClient);

            while (appClient.LoginHandle == 0)
            {
                Thread.Sleep(1000);
            }

            provider.RegisterClient(new RequestMsg().Name("RWFFld").Filter(EmaRdm.DICTIONARY_NORMAL)
                .ServiceName("DIRECT_FEED").DomainType(EmaRdm.MMT_DICTIONARY), appClient);

            provider.RegisterClient(new RequestMsg().Name("RWFEnum").Filter(EmaRdm.DICTIONARY_NORMAL)
                .ServiceName("DIRECT_FEED").DomainType(EmaRdm.MMT_DICTIONARY), appClient);

            while (appClient.ItemHandle == 0)
            {
                Thread.Sleep(1000);
            }

            for (int i = 0; i < 60; i++)
            {
                fieldList.Clear();
                fieldList.AddReal(22, 3991 + i, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
                fieldList.AddReal(25, 3994 + i, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
                fieldList.AddReal(30, 10 + i, OmmReal.MagnitudeTypes.EXPONENT_0);
                fieldList.AddReal(31, 19 + i, OmmReal.MagnitudeTypes.EXPONENT_0);

                provider.Submit(updateMsg.Clear().Payload(fieldList.Complete()), appClient.ItemHandle);

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
