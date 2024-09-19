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
        providerEvent.Provider.Submit(new RefreshMsg().DomainType(EmaRdm.MMT_LOGIN).Name(reqMsg.Name()).NameType(EmaRdm.USER_NAME)
            .Complete(true).Solicited(true)
            .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "Login accepted")
            .Attrib(new ElementList().Complete()),
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
}

public class IProvider
{
    static Map CreateProgrammaticConfig()
    {
        Map innerMap = new Map();
        Map configMap = new Map();
        ElementList elementList = new ElementList();
        ElementList innerElementList = new ElementList();

        // API QA
        configMap.AddKeyAscii("GlobalConfig", MapAction.ADD, elementList.Complete());
        elementList.Clear();
        // End API QA

        elementList.AddAscii("DefaultIProvider", "Provider_1");

        innerElementList.AddAscii("Server", "Server_1");
        innerElementList.AddAscii("Directory", "Directory_2");
        innerElementList.AddAscii("Logger", "Logger_1");
        innerElementList.AddUInt("ItemCountHint", 10_000);
        innerElementList.AddUInt("ServiceCountHint", 10_000);
        innerElementList.AddInt("DispatchTimeoutUserThread", 500);
        innerElementList.AddUInt("MaxDispatchCountApiThread", 500);
        innerElementList.AddUInt("MaxDispatchCountUserThread", 500);
        innerElementList.AddUInt("RefreshFirstRequired", 1);
        innerElementList.AddUInt("XmlTraceToStdout", 0);

        innerMap.AddKeyAscii("Provider_1", MapAction.ADD, innerElementList.Complete());
        innerElementList.Clear();

        elementList.AddMap("IProviderList", innerMap.Complete());
        innerMap.Clear();

        configMap.AddKeyAscii("IProviderGroup", MapAction.ADD, elementList.Complete());
        elementList.Clear();

        innerElementList.AddEnum("ServerType", EmaConfig.ConnectionTypeEnum.SOCKET);
        innerElementList.AddEnum("CompressionType", EmaConfig.CompressionTypeEnum.ZLIB);
        innerElementList.AddUInt("GuaranteedOutputBuffers", 5000);
        innerElementList.AddUInt("ConnectionPingTimeout", 30_000);
        innerElementList.AddUInt("TcpNodelay", 1);
        innerElementList.AddAscii("Port", "14002");

        innerMap.AddKeyAscii("Server_1", MapAction.ADD, innerElementList.Complete());
        innerElementList.Clear();

        elementList.AddMap("ServerList", innerMap.Complete());
        innerMap.Clear();

        configMap.AddKeyAscii("ServerGroup", MapAction.ADD, elementList.Complete());
        elementList.Clear();

        innerElementList.AddEnum("DictionaryType", EmaConfig.DictionaryTypeEnum.FILE);
        innerElementList.AddAscii("RdmFieldDictionaryFileName", "./RDMFieldDictionary");
        innerElementList.AddAscii("EnumTypeDefFileName", "./enumtype.def");
        innerElementList.AddAscii("RdmFieldDictionaryItemName", "RWFFld");
        innerElementList.AddAscii("EnumTypeDefItemName", "RWFEnum");

        innerMap.AddKeyAscii("Dictionary_3", MapAction.ADD, innerElementList.Complete());
        innerElementList.Clear();

        elementList.AddMap("DictionaryList", innerMap.Complete());
        innerMap.Clear();

        configMap.AddKeyAscii("DictionaryGroup", MapAction.ADD, elementList.Complete());
        elementList.Clear();

        innerMap.AddKeyAscii("Logger_1", MapAction.ADD,
            new ElementList()
                .AddEnum("LoggerType", EmaConfig.LoggerTypeEnum.STDOUT)
                .AddAscii("FileName", "logFile")
                .AddEnum("LoggerSeverity", EmaConfig.LoggerLevelEnum.INFO)
                .Complete());

        elementList.AddMap("LoggerList", innerMap.Complete());
        innerMap.Clear();

        configMap.AddKeyAscii("LoggerGroup", MapAction.ADD, elementList.Complete());
        elementList.Clear();

        Map serviceMap = new Map();

        innerElementList.AddUInt("ServiceId", 1);
        innerElementList.AddAscii("Vendor", "company name");
        innerElementList.AddUInt("IsSource", 0);
        innerElementList.AddUInt("AcceptingConsumerStatus", 0);
        innerElementList.AddUInt("SupportsQoSRange", 0);
        innerElementList.AddUInt("SupportsOutOfBandSnapshots", 0);
        innerElementList.AddAscii("ItemList", "#.itemlist");

        OmmArray array = new OmmArray();
        array.AddUInt(EmaConfig.CapabilitiesEnum.MMT_DICTIONARY);
        array.AddUInt(EmaConfig.CapabilitiesEnum.MMT_MARKET_PRICE);
        array.AddUInt(EmaConfig.CapabilitiesEnum.MMT_MARKET_BY_PRICE);
        array.AddUInt(200);

        innerElementList.AddArray("Capabilities", array.Complete());
        array.Clear();

        array.AddAscii("Dictionary_3");

        innerElementList.AddArray("DictionariesProvided", array.Complete());
        array.Clear();

        array.AddAscii("Dictionary_3");

        innerElementList.AddArray("DictionariesUsed", array.Complete());
        array.Clear();

        ElementList inner2 = new ElementList();

        Series series = new Series();

        inner2.AddAscii("Timeliness", "Timeliness::RealTime");
        inner2.AddAscii("Rate", "Rate::TickByTick");

        series.AddEntry(inner2.Complete());
        inner2.Clear();

        inner2.AddUInt("Timeliness", 100);
        inner2.AddUInt("Rate", 100);

        series.AddEntry(inner2.Complete());
        inner2.Clear();

        innerElementList.AddSeries("QoS", series.Complete());

        elementList.AddElementList("InfoFilter", innerElementList.Complete());
        innerElementList.Clear();

        innerElementList.AddUInt("ServiceState", 1);
        innerElementList.AddUInt("AcceptingRequests", 1);

        elementList.AddElementList("StateFilter", innerElementList.Complete());
        innerElementList.Clear();

        serviceMap.AddKeyAscii("DIRECT_FEED", MapAction.ADD, elementList.Complete());
        elementList.Clear();

        innerMap.AddKeyAscii("Directory_2", MapAction.ADD, serviceMap.Complete());

        elementList.AddAscii("DefaultDirectory", "Directory_2");

        elementList.AddMap("DirectoryList", innerMap.Complete());
        innerMap.Clear();

        configMap.AddKeyAscii("DirectoryGroup", MapAction.ADD, elementList.Complete());

        return configMap.Complete();
    }

    public static void Main(string[] args)
    {
        OmmProvider? provider = null;

        try
        {
            AppClient appClient = new AppClient();
            FieldList fieldList = new FieldList();
            UpdateMsg updateMsg = new UpdateMsg();

            provider = new OmmProvider(new OmmIProviderConfig().Config(CreateProgrammaticConfig())
                .OperationModel(OmmIProviderConfig.OperationModelMode.USER_DISPATCH), appClient);

            while (appClient.ItemHandle == 0)
            {
                provider.Dispatch(1000);
                Thread.Sleep(1000);
            }

            for (int i = 0; i < 60; i++)
            {
                provider.Dispatch(1000);

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
