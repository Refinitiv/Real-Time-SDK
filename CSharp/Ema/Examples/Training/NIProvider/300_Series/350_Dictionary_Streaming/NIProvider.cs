/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.Md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Access;
using LSEG.Ema.Rdm;
using static LSEG.Ema.Access.DataType;
using Msg = LSEG.Ema.Access.Msg;

namespace LSEG.Ema.Example.Traning.NIProvider;

class AppClient : IOmmProviderClient
{

    public DataDictionary m_dataDictionary = new DataDictionary();
    public bool fldDictComplete = false;
    public bool enumTypeComplete = false;
    private bool dumpDictionary = false;

    public AppClient(string[] args)
    {
        int idx = 0;

        while (idx < args.Length)
        {
            if (args[idx].Equals("-dumpDictionary"))
            {
                dumpDictionary = true;
            }

            ++idx;
        }
    }

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

    void Decode(Msg msg, bool complete)
    {
        switch (msg.Payload().DataType)
        {
            case DataTypes.SERIES:

                if (msg.Name().Equals("RWFFld"))
                {
                    m_dataDictionary.DecodeFieldDictionary(msg.Payload().Series(), EmaRdm.DICTIONARY_NORMAL);

                    if (complete)
                    {
                        fldDictComplete = true;
                    }
                }
                else if (msg.Name().Equals("RWFEnum"))
                {
                    m_dataDictionary.DecodeEnumTypeDictionary(msg.Payload().Series(), EmaRdm.DICTIONARY_NORMAL);

                    if (complete)
                    {
                        enumTypeComplete = true;
                    }
                }

                if (fldDictComplete && enumTypeComplete)
                {
                    Console.WriteLine();
                    Console.WriteLine("Dictionary download complete");
                    Console.WriteLine("Dictionary Id : " + m_dataDictionary.DictionaryId);
                    Console.WriteLine("Dictionary field version : " + m_dataDictionary.FieldVersion);
                    Console.WriteLine("Number of dictionary entries : " + m_dataDictionary.Entries().Count);

                    if (dumpDictionary)
                    {
                        Console.WriteLine(m_dataDictionary);
                    }
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

            provider.RegisterClient(new RequestMsg().Name("RWFFld").Filter(EmaRdm.DICTIONARY_NORMAL)
                    .ServiceName("TEST_NI_PUB").DomainType(EmaRdm.MMT_DICTIONARY), appClient);

            provider.RegisterClient(new RequestMsg().Name("RWFEnum").Filter(EmaRdm.DICTIONARY_NORMAL)
                    .ServiceName("TEST_NI_PUB").DomainType(EmaRdm.MMT_DICTIONARY), appClient);

            long triHandle = 6;

            FieldList fieldList = new FieldList();

            fieldList.AddReal(22, 4100, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
            fieldList.AddReal(25, 4200, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
            fieldList.AddReal(30, 20, OmmReal.MagnitudeTypes.EXPONENT_0);
            fieldList.AddReal(31, 40, OmmReal.MagnitudeTypes.EXPONENT_0);

            provider.Submit(new RefreshMsg().ServiceName("TEST_NI_PUB").Name("TRI.N")
                    .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "UnSolicited Refresh Completed")
                    .Payload(fieldList.Complete()).Complete(true), triHandle);

            Thread.Sleep(1000);

            for (int i = 0; i < 60; i++)
            {
                fieldList.Clear();
                fieldList.AddReal(22, 4100 + i, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
                fieldList.AddReal(30, 21 + i, OmmReal.MagnitudeTypes.EXPONENT_0);

                provider.Submit(new UpdateMsg().ServiceName("TEST_NI_PUB").Name("TRI.N").Payload(fieldList.Complete()), triHandle);
                Thread.Sleep(1000);
            }
        }
        catch (Exception excp)
        {
            Console.WriteLine(excp.Message);
        }
        finally
        {
            provider?.Uninitialize();
        }
    }
}
