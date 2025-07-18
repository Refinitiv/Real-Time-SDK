/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
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
    public DataDictionary dataDictionary = new DataDictionary();

    private int m_FragmentationSize = 96_000;
    private Series m_Series = new Series();
    private RefreshMsg m_RefreshMsg = new RefreshMsg();
    private int m_CurrentValue;
    private bool m_Result;

    public void OnReqMsg(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
    {
        switch (reqMsg.DomainType())
        {
            case EmaRdm.MMT_LOGIN:
                ProcessLoginRequest(reqMsg, providerEvent);
                break;
            case EmaRdm.MMT_DICTIONARY:
                ProcessDictionaryRequest(reqMsg, providerEvent);
                break;
            case EmaRdm.MMT_MARKET_PRICE:
                ProcessMarketPriceRequest(reqMsg, providerEvent);
                break;
            default:
                ProcessInvalidItemRequest(reqMsg, providerEvent);
                break;
        }
    }

    public void OnReissue(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
    {
        switch (reqMsg.DomainType())
        {
            case EmaRdm.MMT_DICTIONARY:
                ProcessDictionaryRequest(reqMsg, providerEvent);
                break;
            default:
                ProcessInvalidItemRequest(reqMsg, providerEvent);
                break;
        }
    }

    void ProcessLoginRequest(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
    {
        providerEvent.Provider.Submit(m_RefreshMsg.Clear().DomainType(EmaRdm.MMT_LOGIN)
            .Name(reqMsg.Name()).NameType(EmaRdm.USER_NAME).Complete(true).Solicited(true)
            .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "Login accepted"),
            providerEvent.Handle);
    }

    void ProcessDictionaryRequest(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
    {
        m_Result = false;
        m_RefreshMsg.Clear().ClearCache(true);

        if (reqMsg.Name().Equals("RWFFld"))
        {
            m_CurrentValue = dataDictionary.MinFid;

            while (!m_Result)
            {
                m_CurrentValue = dataDictionary.EncodeFieldDictionary(m_Series, m_CurrentValue, reqMsg.Filter(), m_FragmentationSize);

                m_Result = (m_CurrentValue == dataDictionary.MaxFid);

                //API QA
                try
                {
                    Thread.Sleep(IProvider._delayDict);
                    providerEvent.Provider.Submit(m_RefreshMsg.Name(reqMsg.Name()).ServiceName(reqMsg.ServiceName())
                        .DomainType(EmaRdm.MMT_DICTIONARY).Filter(reqMsg.Filter()).Payload(m_Series)
                        .Complete(m_Result).Solicited(true),
                        providerEvent.Handle);
                    m_RefreshMsg.Clear();
                }
                catch (OmmException excp)
                {
                    Console.WriteLine(excp.Message);
                }              
            }
        }
        else if (reqMsg.Name().Equals("RWFEnum"))
        {
            m_CurrentValue = 0;

            while (!m_Result)
            {
                m_CurrentValue = dataDictionary.EncodeEnumTypeDictionary(m_Series, m_CurrentValue, reqMsg.Filter(), m_FragmentationSize);

                m_Result = (m_CurrentValue == dataDictionary.EnumTables().Count);

                
                //API QA
                try
                {
                    Thread.Sleep(IProvider._delayDict);
                    providerEvent.Provider.Submit(m_RefreshMsg.Name(reqMsg.Name()).ServiceName(reqMsg.ServiceName())
                        .DomainType(EmaRdm.MMT_DICTIONARY).Filter(reqMsg.Filter()).Payload(m_Series)
                        .Complete(m_Result).Solicited(true),
                         providerEvent.Handle);

                    m_RefreshMsg.Clear();
                }
                catch (OmmException excp)
                {
                    Console.WriteLine(excp.Message);
                }
            }
        }
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

        providerEvent.Provider.Submit(m_RefreshMsg.Clear().Name(reqMsg.Name()).ServiceName(reqMsg.ServiceName()).Solicited(true)
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

public class IProvider
{
    //API QA
    public static String _portNumber = "14002";
    public static int _delayDict = 0;
    static void PrintHelp()
    {
        Console.WriteLine("\nOptions:\n" + "  -?\tShows this usage\n"
                + "  -p port number.\n"
                + "  -delayDict Time to send directory response in milliseconds (default 0).\n"
                + "\n");
    }
    static bool ReadCommandlineArgs(string[] args)
    {
        try
        {
            int argsCount = 0;
            int delaydict = 0;

            while (argsCount < args.Length)
            {
                if (0 == args[argsCount].CompareTo("-?"))
                {
                    PrintHelp();
                    return false;
                }
                else if ("-p".Equals(args[argsCount]))
                {
                    IProvider._portNumber = argsCount < (args.Length - 1) ? args[++argsCount] : null;
                    Console.WriteLine("Port Number: " + IProvider._portNumber);
                    ++argsCount;
                }
                else if ("-delayDict".Equals(args[argsCount]))
                {
                    if (Int32.TryParse(args[++argsCount], out delaydict))
                    {
                        IProvider._delayDict = delaydict;
                    }
                    else
                    {
                        Console.WriteLine($"Error: failed to parse delayDir '{args[argsCount]}'");
                        return false;
                    }
                    ++argsCount;
                }
                else // unrecognized command line argument
                {
                    PrintHelp();
                    return false;
                }
            }
        }
        catch (Exception)
        {
            PrintHelp();
            return false;
        }

        return true;
    }

    //END APIQA
    public static void Main(string[] args)
    {
        OmmProvider? provider = null;
        try
        {
            // API QA
            if (!ReadCommandlineArgs(args))
                return;
            //END API QA
            AppClient appClient = new AppClient();

            appClient.dataDictionary.LoadFieldDictionary("RDMFieldDictionary");
            appClient.dataDictionary.LoadEnumTypeDictionary("enumtype.def");

            provider = new OmmProvider(new OmmIProviderConfig()
                .AdminControlDictionary(OmmIProviderConfig.AdminControlMode.USER_CONTROL).Port(IProvider._portNumber), appClient);

            while (appClient.ItemHandle == 0)
            {
                Thread.Sleep(1000);
            }

            FieldList fieldList = new FieldList();
            for (int i = 0; i < 60; i++)
            {
                fieldList.Clear();
                fieldList.AddReal(22, 3991 + i, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
                fieldList.AddReal(30, 10 + i, OmmReal.MagnitudeTypes.EXPONENT_0);

                provider.Submit(new UpdateMsg().Payload(fieldList.Complete()), appClient.ItemHandle);

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
