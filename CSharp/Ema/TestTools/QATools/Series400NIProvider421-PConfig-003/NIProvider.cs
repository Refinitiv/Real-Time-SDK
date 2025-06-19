/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Access;
using static LSEG.Ema.Access.EmaConfig;
using static LSEG.Ema.Access.EmaConfig.ConnectionTypeEnum;

namespace LSEG.Ema.Example.Traning.NIProvider;

class AppClient : IOmmProviderClient
{
    public bool m_connectionUp;

    public bool IsConnectionUp()
    {
        return m_connectionUp;
    }

    public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmProviderEvent e)
    {
        Console.WriteLine("Received Refresh. Item Handle: " + e.Handle + " Closure: " + e.Closure);

        Console.WriteLine("Item Name: " + (refreshMsg.HasName ? refreshMsg.Name() : "<not set>"));
        Console.WriteLine("Service Name: " + (refreshMsg.HasServiceName ? refreshMsg.ServiceName() : "<not set>"));

        Console.WriteLine("Item State: " + refreshMsg.State());

        if (refreshMsg.State().StreamState == OmmState.StreamStates.OPEN)
        {
            if (refreshMsg.State().DataState == OmmState.DataStates.OK)
                m_connectionUp = true;
            else
                m_connectionUp = false;
        }
        else
            m_connectionUp = false;
    }
    public void OnStatusMsg(StatusMsg statusMsg, IOmmProviderEvent e)
    {
        Console.WriteLine("Received Status. Item Handle: " + e.Handle + " Closure: " + e.Closure);

        Console.WriteLine("Item Name: " + (statusMsg.HasName ? statusMsg.Name() : "<not set>"));
        Console.WriteLine("Service Name: " + (statusMsg.HasServiceName ? statusMsg.ServiceName() : "<not set>"));

        if (statusMsg.HasState)
        {
            Console.WriteLine("Item State: " + statusMsg.State());
            if (statusMsg.State().StreamState == OmmState.StreamStates.OPEN)
            {
                if (statusMsg.State().DataState == OmmState.DataStates.OK)
                    m_connectionUp = true;
                else
                {
                    m_connectionUp = false;
                }
            }
            else
                m_connectionUp = false;
        }
    }
}


public partial class NIProvider
{
    static Map CreateProgrammaticConfig()
    {
        Map innerMap = new Map();
        Map configMap = new Map();
        ElementList elementList = new ElementList();
        ElementList innerElementList = new ElementList();

        elementList.AddAscii("DefaultNiProvider", "Provider_1");

        innerElementList.AddAscii("ChannelSet", "Channel_10,Channel_11");
        innerElementList.AddAscii("Directory", "Directory_1");
        innerElementList.AddAscii("Logger", "Logger_1");
        innerElementList.AddUInt("XmlTraceToStdout", 1);
        innerElementList.AddInt("DispatchTimeoutApiThread", 45000);
        innerElementList.AddUInt("ItemCountHint", 45000);
        innerElementList.AddUInt("LoginRequestTimeOut", 45000);
        innerElementList.AddUInt("MaxDispatchCountApiThread", 500);
        innerElementList.AddUInt("MaxDispatchCountUserThread", 500);
        innerElementList.AddUInt("MergeSourceDirectoryStreams", 1);
        innerElementList.AddInt("ReconnectAttemptLimit", 10);
        innerElementList.AddInt("ReconnectMaxDelay", 5000);
        innerElementList.AddInt("ReconnectMinDelay", 1000);
        innerElementList.AddUInt("RecoverUserSubmitSourceDirectory", 1);
        innerElementList.AddUInt("RemoveItemsOnDisconnect", 1);
        innerElementList.AddUInt("RequestTimeout", 15000);
        innerElementList.AddUInt("ServiceCountHint", 513);
        innerElementList.AddUInt("RefreshFirstRequired", 1);

        innerMap.AddKeyAscii("Provider_1", MapAction.ADD, innerElementList.Complete());
        innerElementList.Clear();
        elementList.AddMap("NiProviderList", innerMap.Complete());
        innerMap.Clear();

        configMap.AddKeyAscii("NiProviderGroup", MapAction.ADD, elementList.Complete());
        elementList.Clear();

        innerElementList.AddEnum("ChannelType", SOCKET);
        innerElementList.AddAscii("InterfaceName", "providerInterface");
        innerElementList.AddUInt("GuaranteedOutputBuffers", 5000);
        innerElementList.AddUInt("ConnectionPingTimeout", 30000);
        innerElementList.AddAscii("Host", "localhost");
        innerElementList.AddAscii("Port", "14003");
        innerElementList.AddUInt("TcpNodelay", 1);
        innerElementList.AddUInt("HighWaterMark", 6144);
        innerElementList.AddUInt("NumInputBuffers", 10);
        innerElementList.AddUInt("SysRecvBufSize", 0);
        innerElementList.AddUInt("SysSendBufSize", 0);
        innerElementList.AddUInt("DirectWrite", 1);
        innerElementList.AddEnum("CompressionType", CompressionTypeEnum.NONE);
        innerElementList.AddUInt("CompressionThreshold", 100);

        innerMap.AddKeyAscii("Channel_10", MapAction.ADD, innerElementList.Complete());
        innerElementList.Clear();
        innerElementList.AddEnum("ChannelType", channl11_type);
        innerElementList.AddAscii("InterfaceName", "providerInterface");
        innerElementList.AddUInt("GuaranteedOutputBuffers", 5000);
        innerElementList.AddUInt("ConnectionPingTimeout", 30000);
        innerElementList.AddAscii("Host", "channel11_host");
        innerElementList.AddAscii("Port", "channel11_port");
        innerElementList.AddUInt("TcpNodelay", 1);
        innerElementList.AddUInt("HighWaterMark", 6144);
        innerElementList.AddUInt("NumInputBuffers", 10);
        innerElementList.AddUInt("SysRecvBufSize", 0);
        innerElementList.AddUInt("SysSendBufSize", 0);
        innerElementList.AddUInt("DirectWrite", 1);
        innerElementList.AddEnum("CompressionType", CompressionTypeEnum.NONE);
        innerElementList.AddUInt("CompressionThreshold", 100);

        innerMap.AddKeyAscii("Channel_11", MapAction.ADD, innerElementList.Complete());
        innerElementList.Clear();

        elementList.AddMap("ChannelList", innerMap.Complete());
        innerMap.Clear();

        configMap.AddKeyAscii("ChannelGroup", MapAction.ADD, elementList.Complete());
        elementList.Clear();

        innerMap.AddKeyAscii("Logger_1", MapAction.ADD,
                    new ElementList()
                    .AddEnum("LoggerType", 1)
                    .AddAscii("FileName", "logFile")
        //APIQA
                    .AddEnum("LoggerSeverity", 0).Complete()).Complete();
        //END APIQA

        elementList.AddMap("LoggerList", innerMap);

        elementList.Complete();
        innerMap.Clear();

        configMap.AddKeyAscii("LoggerGroup", MapAction.ADD, elementList);
        elementList.Clear();

        Map serviceMap = new Map();
        innerElementList.AddUInt("ServiceId", 0);
        innerElementList.AddAscii("Vendor", "company name");
        innerElementList.AddUInt("IsSource", 0);
        innerElementList.AddUInt("AcceptingConsumerStatus", 0);
        innerElementList.AddUInt("SupportsQoSRange", 0);
        innerElementList.AddUInt("SupportsOutOfBandSnapshots", 0);
        innerElementList.AddAscii("ItemList", "#.itemlist");

        OmmArray array = new OmmArray();
        array.AddUInt(CapabilitiesEnum.MMT_MARKET_PRICE);
        array.AddUInt(CapabilitiesEnum.MMT_MARKET_BY_PRICE);
        array.AddUInt(200);
        innerElementList.AddArray("Capabilities", array.Complete());
        array.Clear();

        array.AddAscii("Dictionary_1");
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

        serviceMap.AddKeyAscii("NI_PUB", MapAction.ADD, elementList.Complete());
        elementList.Clear();
        innerMap.AddKeyAscii("Directory_1", MapAction.ADD, serviceMap.Complete());

        elementList.AddAscii("DefaultDirectory", "Directory_1");
        elementList.AddMap("DirectoryList", innerMap.Complete());
        innerMap.Clear();

        configMap.AddKeyAscii("DirectoryGroup", MapAction.ADD, elementList.Complete());
        elementList.Clear();

        innerElementList.AddEnum("DictionaryType", DictionaryTypeEnum.CHANNEL);
        innerMap.AddKeyAscii("Dictionary_1", MapAction.ADD, innerElementList.Complete());
        innerElementList.Clear();

        elementList.AddMap("DictionaryList", innerMap.Complete());
        innerMap.Clear();

        configMap.AddKeyAscii("DictionaryGroup", MapAction.ADD, elementList.Complete());

        return configMap.Complete();
    }

    public static void Main()
    {
        AppClient appClient = new AppClient();
        bool sendRefreshMsg = false;

        OmmProvider? provider = null;
        try
        {
            OmmNiProviderConfig config = new OmmNiProviderConfig().Config(CreateProgrammaticConfig());

            provider = new OmmProvider(config.OperationModel(OmmNiProviderConfig.OperationModelMode.USER_DISPATCH)
                    .UserName("user"), appClient);

            provider.Dispatch(1000000);

            long itemHandle = 6;

            FieldList fieldList = new FieldList();

            fieldList.AddReal(22, 14400, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
            fieldList.AddReal(25, 14700, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
            fieldList.AddReal(30, 9, OmmReal.MagnitudeTypes.EXPONENT_0);
            fieldList.AddReal(31, 19, OmmReal.MagnitudeTypes.EXPONENT_0);

            provider.Submit(new RefreshMsg().ServiceName("NI_PUB").Name("TRI.N")
                    .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "UnSolicited Refresh Completed")
                    .Payload(fieldList.Complete()).Complete(true), itemHandle);

            provider.Dispatch(1000000);

            for (int i = 0; i < 60; i++)
            {
                if (appClient.IsConnectionUp())
                {
                    if (sendRefreshMsg)
                    {
                        fieldList.Clear();
                        fieldList.AddReal(22, 14400 + i, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
                        fieldList.AddReal(25, 14700 + i, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
                        fieldList.AddReal(30, 10 + i, OmmReal.MagnitudeTypes.EXPONENT_0);
                        fieldList.AddReal(31, 19 + i, OmmReal.MagnitudeTypes.EXPONENT_0);

                        provider.Submit(new RefreshMsg().ServiceName("NI_PUB").Name("TRI.N")
                                .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "UnSolicited Refresh Completed")
                                .Payload(fieldList.Complete()).Complete(true), itemHandle);

                        sendRefreshMsg = false;
                    }
                    else
                    {
                        fieldList.Clear();
                        fieldList.AddReal(22, 14400 + i, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
                        fieldList.AddReal(30, 10 + i, OmmReal.MagnitudeTypes.EXPONENT_0);

                        provider.Submit(new UpdateMsg().ServiceName("NI_PUB").Name("TRI.N").Payload(fieldList.Complete()), itemHandle);
                    }
                }
                else
                {
                    sendRefreshMsg = true;
                }
                provider.Dispatch(1000000);
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
//END APIQA
