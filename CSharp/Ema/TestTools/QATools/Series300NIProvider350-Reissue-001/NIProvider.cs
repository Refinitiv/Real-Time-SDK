/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.Md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Access;
using LSEG.Ema.Rdm;

namespace LSEG.Ema.Example.Traning.NIProvider;

class AppClient : IOmmProviderClient
{
    public DataDictionary dataDictionary = new DataDictionary();
    public bool fldDictComplete = false;
    public bool enumTypeComplete = false;
    private bool dumpDictionary = false;
    public int Filter = EmaRdm.DICTIONARY_NORMAL;

    public AppClient(string[] args)
    {
        int idx = 0;

        while (idx < args.Length)
        {
            if (args[idx] == "-dumpDictionary")
            {
                dumpDictionary = true;
            }
            else if (args[idx] == "-filter")
            {
                if (++idx == args.Length)
                    break;

                if (args[idx].ToLower() == "info")
                {
                    Filter = EmaRdm.DICTIONARY_INFO;
                }
                if (args[idx].ToLower() == "minimal")
                {
                    Filter = EmaRdm.DICTIONARY_MINIMAL;
                }
                if (args[idx].ToLower() == "normal")
                {
                    Filter = EmaRdm.DICTIONARY_NORMAL;
                }
                if (args[idx].ToLower() == "verbose")
                {
                    Filter = EmaRdm.DICTIONARY_VERBOSE;
                }
            }

            ++idx;
        }
    }

    //APIQA
    public void UpdateSourceDirectory(OmmProvider provider)
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

    public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmProviderEvent providerEvent)
    {
        Console.WriteLine("Item Name: " + (refreshMsg.HasName ? refreshMsg.Name() : "<not set>"));
        Console.WriteLine("Service Name: " + (refreshMsg.HasServiceName ? refreshMsg.ServiceName() : "<not set>"));

        Console.WriteLine("Item State: " + refreshMsg.State());

        Decode(refreshMsg, refreshMsg.Complete());
    }

    public void OnStatusMsg(StatusMsg statusMsg, IOmmProviderEvent providerEvent)
    {
        Console.WriteLine("Item Name: " + (statusMsg.HasName ? statusMsg.Name() : "<not set>"));
        Console.WriteLine("Service Name: " + (statusMsg.HasServiceName ? statusMsg.ServiceName() : "<not set>"));

        if (statusMsg.HasState)
            Console.WriteLine("Item State: " + statusMsg.State());

        Console.WriteLine();
    }

    public void OnGenericMsg(GenericMsg genericMsg, IOmmProviderEvent providerEvent) { }

    public void OnAllMsg(Msg msg, IOmmProviderEvent providerEvent) { }

    void Decode(Msg msg, bool complete)
    {
        switch (msg.Payload().DataType)
        {
            case DataType.DataTypes.SERIES:

                if (msg.Name().Equals("RWFFld"))
                {
                    dataDictionary.Clear();
                    dataDictionary.DecodeFieldDictionary(msg.Payload().Series(), Filter);

                    if (complete)
                    {
                        fldDictComplete = true;
                    }
                }
                else if (msg.Name().Equals("RWFEnum"))
                {
                    dataDictionary.Clear();
                    dataDictionary.DecodeEnumTypeDictionary(msg.Payload().Series(), Filter);

                    if (complete)
                    {
                        enumTypeComplete = true;
                    }
                }

                if (fldDictComplete && enumTypeComplete)
                {
                    Console.WriteLine();
                    Console.WriteLine("Dictionary download complete");
                    Console.WriteLine("Dictionary Id : " + dataDictionary.DictionaryId);
                    Console.WriteLine("Dictionary field version : " + dataDictionary.FieldVersion);
                    Console.WriteLine("Number of dictionary entries : " + dataDictionary.Entries().Count);

                    if (dumpDictionary)
                        Console.WriteLine(dataDictionary);
                }

                break;
            default:
                break;
        }
    }
}

public class NIProvider
{
    public static void Main(string[] args)
    {
        OmmProvider? provider = null;
        try
        {
            AppClient appClient = new AppClient(args);
            OmmNiProviderConfig config = new OmmNiProviderConfig();

            provider = new OmmProvider(config.UserName("user"));

            long rwfFld = provider.RegisterClient(new RequestMsg().Name("RWFFld").Filter(appClient.Filter)
                .ServiceName("NI_PUB").DomainType(EmaRdm.MMT_DICTIONARY).InterestAfterRefresh(true),
                                                  appClient);

            long rwfEnum = provider
                    .RegisterClient(new RequestMsg().Name("RWFEnum").InterestAfterRefresh(true).Filter(appClient.Filter)
                    .ServiceName("NI_PUB").DomainType(EmaRdm.MMT_DICTIONARY), appClient);

            long triHandle = 6;

            FieldList fieldList = new FieldList();

            fieldList.AddReal(22, 4100, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
            fieldList.AddReal(25, 4200, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
            fieldList.AddReal(30, 20, OmmReal.MagnitudeTypes.EXPONENT_0);
            fieldList.AddReal(31, 40, OmmReal.MagnitudeTypes.EXPONENT_0);

            provider.Submit(new RefreshMsg().ServiceName("NI_PUB").Name("TRI.N")
                    .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "UnSolicited Refresh Completed").Payload(fieldList.Complete()).Complete(true), triHandle);

            Thread.Sleep(1000);

            for (int i = 0; i < 10; i++)
            {
                fieldList.Clear();
                fieldList.AddReal(22, 4100 + i, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
                fieldList.AddReal(30, 21 + i, OmmReal.MagnitudeTypes.EXPONENT_0);

                provider.Submit(new UpdateMsg().ServiceName("NI_PUB").Name("TRI.N").Payload(fieldList.Complete()), triHandle);
                Thread.Sleep(1000);
                // APIQA
                if (i == 6)
                {
                    Console.WriteLine("Reissue Dictionary handles with filter unchange");
                    provider.Reissue(new RequestMsg().Clear().DomainType(EmaRdm.MMT_DICTIONARY).ServiceName("NI_PUB")
                        .Filter(appClient.Filter), rwfFld);
                    provider.Reissue(new RequestMsg().Clear().DomainType(EmaRdm.MMT_DICTIONARY).ServiceName("NI_PUB")
                        .Filter(appClient.Filter), rwfEnum);
                    Thread.Sleep(5000);
                }
                if (i == 8)
                {
                    Console.WriteLine("Reissue Dictionary handles with filter change");
                    appClient.Filter = EmaRdm.DICTIONARY_NORMAL;
                    provider.Reissue(new RequestMsg().Clear().DomainType(EmaRdm.MMT_DICTIONARY).ServiceName("NI_PUB")
                        .Filter(appClient.Filter), rwfFld);
                    provider.Reissue(new RequestMsg().Clear().DomainType(EmaRdm.MMT_DICTIONARY).ServiceName("NI_PUB")
                        .Filter(appClient.Filter), rwfEnum);
                    Thread.Sleep(5000);
                }
                if (i == 9)
                {
                    Console.WriteLine("Update Source Directory with delete service from DIRECTORY ");
                    appClient.UpdateSourceDirectory(provider);
                }
                // END APIQA

            }
            Thread.Sleep(5000);
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
