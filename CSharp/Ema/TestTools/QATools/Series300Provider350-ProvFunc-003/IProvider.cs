/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.Md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

//APIQA this file is QATools standalone. See qa_readme.txt for details about this tool.

using System;
using System.Threading;

using LSEG.Ema.Access;
using LSEG.Ema.Rdm;

namespace LSEG.Ema.Example.Traning.IProvider;

class AppClient : IOmmProviderClient
{
    public long ItemHandle = 0;
    public long LoginHandle = 0;

    private DataDictionary dataDictionary = new DataDictionary();
    private bool fldDictComplete = false;
    private bool enumTypeComplete = false;
    private bool dumpDictionary = false;
    // APIQA
    public int filter = EmaRdm.DICTIONARY_NORMAL;

    public AppClient(string[] args)
    {
        int idx = 0;

        while (idx < args.Length)
        {
            if (args[idx].Equals("-dumpDictionary"))
            {
                dumpDictionary = true;
            }
            // APIQA
            else if (args[idx].Equals("-filter"))
            {
                if (++idx == args.Length)
                    break;

                if (args[idx].Equals("INFO", StringComparison.InvariantCultureIgnoreCase))
                {
                    filter = EmaRdm.DICTIONARY_INFO;
                }
                if (args[idx].Equals("MINIMAL", StringComparison.InvariantCultureIgnoreCase))
                {
                    filter = EmaRdm.DICTIONARY_MINIMAL;
                }
                if (args[idx].Equals("NORMAL", StringComparison.InvariantCultureIgnoreCase))
                {
                    filter = EmaRdm.DICTIONARY_NORMAL;
                }
                if (args[idx].Equals("VERBOSE", StringComparison.InvariantCultureIgnoreCase))
                {
                    filter = EmaRdm.DICTIONARY_VERBOSE;
                }
            }
            // END APIQA
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
                // APIQA
                Console.WriteLine("Send Login Status Close ");
                SendLoginClose(reqMsg, providerEvent);
                // END APIQA
                break;
            default:
                ProcessInvalidItemRequest(reqMsg, providerEvent);
                break;
        }
    }

    public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmProviderEvent providerEvent)
    {
        // Console.WriteLine("Received Refresh. Item Handle: " + providerEvent.Handle
        // + " Closure: " + providerEvent.closure());
        Console.WriteLine("Received Refresh. Item Handle: " + providerEvent.Handle);

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
        providerEvent.Provider.Submit(new RefreshMsg().DomainType(EmaRdm.MMT_LOGIN)
            .Name(reqMsg.Name()).NameType(EmaRdm.USER_NAME).Complete(true).Solicited(true)
            .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "Login accepted")
            .Attrib(new ElementList().Complete()),
            providerEvent.Handle);

        LoginHandle = providerEvent.Handle;
    }

    // APIQA
    void SendLoginClose(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
    {
        providerEvent.Provider.Submit(new StatusMsg().DomainType(EmaRdm.MMT_LOGIN).NameType(EmaRdm.USER_NAME)
            .State(OmmState.StreamStates.CLOSED, OmmState.DataStates.SUSPECT, OmmState.StatusCodes.NOT_AUTHORIZED, "Login denied"),
            LoginHandle);
    }

    // END APIQA
    void updateSourceDirectory(OmmProvider provider)
    {
        ElementList elementList = new ElementList();
        elementList.AddUInt(EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_DOWN);

        FilterList filterList = new FilterList();
        filterList.AddEntry(EmaRdm.SERVICE_STATE_ID, FilterAction.UPDATE, elementList.Complete());

        Map map = new Map();
        map.AddKeyUInt(1, MapAction.DELETE, filterList.Complete());

        provider.Submit(new UpdateMsg().DomainType(EmaRdm.MMT_DIRECTORY).Payload(map.Complete()), 0);
    }

    // END APIQA
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
        providerEvent.Provider.Submit(new StatusMsg().Name(reqMsg.Name()).ServiceName(reqMsg.ServiceName()).DomainType(reqMsg.DomainType())
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
                    dataDictionary.DecodeFieldDictionary(msg.Payload().Series(), filter);

                    if (complete)
                    {
                        fldDictComplete = true;
                    }
                }
                else if (msg.Name().Equals("RWFEnum"))
                {
                    dataDictionary.DecodeEnumTypeDictionary(msg.Payload().Series(), filter);

                    if (complete)
                    {
                        enumTypeComplete = true;
                    }
                }

                if (fldDictComplete && enumTypeComplete)
                {
                    Console.WriteLine();
                    Console.WriteLine("\nDictionary download complete");
                    Console.WriteLine("Dictionary Id : " + dataDictionary.DictionaryId);
                    Console.WriteLine("Dictionary field version : " + dataDictionary.FieldVersion);
                    Console.WriteLine("Number of Fld Table dictionary entries : " + dataDictionary.Entries().Count);
                    Console.WriteLine("Number of Enum Table dictionary entries : " + dataDictionary.EnumTables().Count);

                    if (dumpDictionary)
                        Console.WriteLine(dataDictionary);

                    dataDictionary = new DataDictionary();
                    enumTypeComplete = false;
                    fldDictComplete = false;
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

            // APIQA

            long rwfFld = provider.RegisterClient(new RequestMsg().Name("RWFFld")
                .Filter(appClient.filter).ServiceName("DIRECT_FEED").DomainType(EmaRdm.MMT_DICTIONARY),
                appClient);

            long rwfEnum = provider.RegisterClient(new RequestMsg().Name("RWFEnum")
                .Filter(appClient.filter).ServiceName("DIRECT_FEED").DomainType(EmaRdm.MMT_DICTIONARY),
                appClient);

            // END APIQA

            while (appClient.ItemHandle == 0)
            {
                Thread.Sleep(1000);
            }

            for (int i = 0; i < 10; i++)
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
