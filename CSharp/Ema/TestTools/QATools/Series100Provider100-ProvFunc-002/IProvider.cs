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
using System.Threading;

using LSEG.Ema.Access;
using LSEG.Ema.Rdm;

namespace LSEG.Ema.Example.Traning.IProvider;

//APIQA
class AppClient : IOmmProviderClient
{
    public long ItemHandle = 0;
    public bool submitting = true;

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

    public void OnClose(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
    {
        Console.WriteLine(reqMsg);

        submitting = false;
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
            providerEvent.Provider.Submit(new StatusMsg().Name(reqMsg.Name())
                .ServiceName(reqMsg.ServiceName())
                .State(OmmState.StreamStates.CLOSED, OmmState.DataStates.SUSPECT, OmmState.StatusCodes.NOT_FOUND,
                    "This test tool support only one item."),
                providerEvent.Handle);
            return;
        }

        FieldList fieldList = new FieldList();
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

        providerEvent.Provider.Submit(new RefreshMsg().Name(reqMsg.Name())
            .ServiceName(reqMsg.ServiceName()).Solicited(true).ItemGroup(IProvider.groupId)
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
    public enum TestFlags
    {
        NONE,
        SERVICE_DOWN,
        OPEN_WINDOWS_AND_LOAD_FACTOR,
        MERGED_TO_GROUP,
        GROUP_STATUS_DATA_OK,
        GROUP_STATUS_DATA_SUSPECT,
        GROUP_STATUS_CLOSED_RECOVER,
        CHANGE_DATA_TYPE,
        CHANGE_LINK_TYPE_AND_STATE_AND_CODE,
        INFO_ADD_CAPABILITIES,
        SEQUENCED_MULTICAST,
        ACCEPTING_REQUEST,
        MERGED_TO_GROUP_AND_SEND_GROUP_STATUS_CLOSED_RECOVER,
        SERVICE_DELETE
    }

    private static TestFlags testFlag = TestFlags.NONE;
    public static EmaBuffer groupId = new EmaBuffer(Encoding.ASCII.GetBytes("00"));
    private static int numberOfUpdates = 600;
    private static string link1 = "Link1";
    private static string link2 = "Link2";

    private static string PrintUsage()
    {
        StringBuilder text = new StringBuilder();
        text.AppendLine("Usage: SourceDirectoryUpdate [UpdateInfo]")
            .AppendLine()
            .AppendLine("\t -serviceDown")
            .AppendLine("\t -openWindowsAndLoadFactor")
            .AppendLine("\t -mergedToGroup")
            .AppendLine("\t -mergedToGroupAndSendGroupStatusClosedRecover")
            .AppendLine("\t -groupStatusDataOk")
            .AppendLine("\t -groupStatusDataSuspect")
            .AppendLine("\t -groupStatusClosedRecover")
            .AppendLine("\t -changeDataType")
            .AppendLine("\t -chagneLinkTypeAndStateAndCode")
            .AppendLine("\t -infoAddCapabilities")
            .AppendLine("\t -MCastInformation")
            .AppendLine("\t -sequencedMulticast")
            .AppendLine("\t -serviceDelete")
            .AppendLine("\t -acceptingRequest");

        return text.ToString();
    }

    private static void AddSourceDirectory(OmmProvider provider, long handle)
    {
        switch (testFlag)
        {
            case TestFlags.SERVICE_DOWN:
                {
                    // Do nothing for the state filter
                }
                break;
            case TestFlags.OPEN_WINDOWS_AND_LOAD_FACTOR:
                {
                    ElementList elementList = new ElementList();

                    elementList.AddUInt(EmaRdm.ENAME_OPEN_LIMIT, 100);
                    elementList.AddUInt(EmaRdm.ENAME_OPEN_WINDOW, 50);
                    elementList.AddUInt(EmaRdm.ENAME_LOAD_FACT, 5555);

                    FilterList filterList = new FilterList();

                    filterList.AddEntry(EmaRdm.SERVICE_LOAD_ID, FilterAction.SET, elementList.Complete());

                    Map map = new Map();

                    map.AddKeyUInt(1, MapAction.UPDATE, filterList.Complete());

                    provider.Submit(new UpdateMsg().DomainType(EmaRdm.MMT_DIRECTORY).Filter(EmaRdm.SERVICE_LOAD_FILTER).Payload(map.Complete()),
                        handle);
                }
                break;
            case TestFlags.MERGED_TO_GROUP:
            case TestFlags.MERGED_TO_GROUP_AND_SEND_GROUP_STATUS_CLOSED_RECOVER:
            case TestFlags.GROUP_STATUS_CLOSED_RECOVER:
            case TestFlags.GROUP_STATUS_DATA_SUSPECT:
                {
                    ElementList elementList = new ElementList();

                    elementList.AddBuffer(EmaRdm.ENAME_GROUP, groupId);
                    elementList.AddState(EmaRdm.ENAME_STATUS, OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "");

                    FilterList filterList = new FilterList();

                    filterList.AddEntry(EmaRdm.SERVICE_GROUP_ID, FilterAction.SET, elementList.Complete());

                    Map map = new Map();

                    map.AddKeyUInt(1, MapAction.UPDATE, filterList.Complete());

                    provider.Submit(new UpdateMsg().DomainType(EmaRdm.MMT_DIRECTORY).Filter(EmaRdm.SERVICE_GROUP_FILTER).Payload(map.Complete()),
                        handle);
                }
                break;
            case TestFlags.GROUP_STATUS_DATA_OK:
                { // Send initial state as suspect

                    ElementList elementList = new ElementList();

                    elementList.AddBuffer(EmaRdm.ENAME_GROUP, groupId);
                    elementList.AddState(EmaRdm.ENAME_STATUS, OmmState.StreamStates.OPEN, OmmState.DataStates.SUSPECT, OmmState.StatusCodes.NONE, "");

                    FilterList filterList = new FilterList();

                    filterList.AddEntry(EmaRdm.SERVICE_GROUP_ID, FilterAction.SET, elementList.Complete());

                    Map map = new Map();

                    map.AddKeyUInt(1, MapAction.UPDATE, filterList.Complete());

                    provider.Submit(new UpdateMsg().DomainType(EmaRdm.MMT_DIRECTORY).Filter(EmaRdm.SERVICE_GROUP_FILTER).Payload(map.Complete()),
                        handle);
                }
                break;
            case TestFlags.CHANGE_DATA_TYPE:
                {
                    ElementList elementList = new ElementList();

                    elementList.AddUInt(EmaRdm.ENAME_TYPE, 1);
                    elementList.AddAscii(EmaRdm.ENAME_DATA, "Data");

                    FilterList filterList = new FilterList();

                    filterList.AddEntry(EmaRdm.SERVICE_DATA_ID, FilterAction.SET, elementList.Complete());

                    Map map = new Map();

                    map.AddKeyUInt(1, MapAction.UPDATE, filterList.Complete());

                    provider.Submit(new UpdateMsg().DomainType(EmaRdm.MMT_DIRECTORY).Filter(EmaRdm.SERVICE_DATA_FILTER).Payload(map.Complete()),
                        handle);
                }
                break;
            case TestFlags.CHANGE_LINK_TYPE_AND_STATE_AND_CODE:
                {
                    ElementList elementListLink1 = new ElementList();

                    elementListLink1.AddUInt(EmaRdm.ENAME_TYPE, 1);
                    elementListLink1.AddUInt(EmaRdm.ENAME_LINK_STATE, 1);
                    elementListLink1.AddUInt(EmaRdm.ENAME_LINK_CODE, 1);
                    elementListLink1.AddAscii(EmaRdm.ENAME_TEXT, "Text of Link1");

                    ElementList elementListLink2 = new ElementList();

                    elementListLink2.AddUInt(EmaRdm.ENAME_TYPE, 2);
                    elementListLink2.AddUInt(EmaRdm.ENAME_LINK_STATE, 1);
                    elementListLink2.AddUInt(EmaRdm.ENAME_LINK_CODE, 1);
                    elementListLink2.AddAscii(EmaRdm.ENAME_TEXT, "Text of Link2");

                    Map linkMap = new Map();

                    linkMap.AddKeyAscii(link1, MapAction.ADD, elementListLink1.Complete());
                    linkMap.AddKeyAscii(link2, MapAction.ADD, elementListLink2.Complete());

                    FilterList filterList = new FilterList();

                    filterList.AddEntry(EmaRdm.SERVICE_LINK_ID, FilterAction.SET, linkMap.Complete());

                    Map map = new Map();

                    map.AddKeyUInt(1, MapAction.UPDATE, filterList.Complete());

                    provider.Submit(new UpdateMsg().DomainType(EmaRdm.MMT_DIRECTORY).Filter(EmaRdm.SERVICE_LINK_FILTER).Payload(map.Complete()),
                        handle);
                }
                break;
            case TestFlags.SEQUENCED_MULTICAST:
                {
                    ElementList mCastEntry = new ElementList();

                    mCastEntry.AddAscii("MulticastGroup", "224.1.62.2");
                    mCastEntry.AddUInt("Port", 30001);
                    mCastEntry.AddUInt("Domain", EmaRdm.MMT_MARKET_PRICE);

                    Access.Vector vectorMCastEntry = new Access.Vector();

                    vectorMCastEntry.Add(1, VectorAction.INSERT, mCastEntry.Complete(), null);

                    ElementList elementList = new ElementList();

                    elementList.AddAscii("ReferenceDataServerHost", "10.0.1.125");
                    elementList.AddUInt("ReferenceDataServerPort", 19000);
                    elementList.AddAscii("SnapshotServerHost", "10.0.1.125");
                    elementList.AddUInt("SnapshotServerPort", 14002);
                    elementList.AddAscii("GapRecoveryServerHost", "10.0.1.125");
                    elementList.AddUInt("GapRecoveryServerPort", 14001);
                    elementList.AddVector("StreamingMulticastChannels", vectorMCastEntry.Complete());

                    FilterList filterList = new FilterList();

                    filterList.AddEntry(7, FilterAction.SET, elementList.Complete());

                    Map map = new Map();

                    map.AddKeyUInt(1, MapAction.UPDATE, filterList.Complete());

                    provider.Submit(new UpdateMsg().DomainType(EmaRdm.MMT_DIRECTORY).Filter(64).Payload(map.Complete()),
                        handle);
                }
                break;
        }
    }

    private static void UpdateSourceDirectory(OmmProvider provider, long handle)
    {
        switch (testFlag)
        {
            case TestFlags.SERVICE_DELETE:
                {
                    ElementList elementList = new ElementList();

                    elementList.AddUInt(EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_DOWN);

                    FilterList filterList = new FilterList();

                    filterList.AddEntry(EmaRdm.SERVICE_STATE_ID, FilterAction.UPDATE, elementList.Complete());

                    Map map = new Map();

                    map.AddKeyUInt(1, MapAction.DELETE, filterList.Complete());

                    provider.Submit(new UpdateMsg().DomainType(EmaRdm.MMT_DIRECTORY).Payload(map.Complete()),
                        handle);
                }
                break;
            case TestFlags.SERVICE_DOWN:
                {
                    ElementList elementList = new ElementList();

                    elementList.AddUInt(EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_DOWN);

                    FilterList filterList = new FilterList();

                    filterList.AddEntry(EmaRdm.SERVICE_STATE_ID, FilterAction.UPDATE, elementList.Complete());

                    Map map = new Map();

                    map.AddKeyUInt(1, MapAction.UPDATE, filterList.Complete());

                    provider.Submit(new UpdateMsg().DomainType(EmaRdm.MMT_DIRECTORY).Filter(EmaRdm.SERVICE_STATE_FILTER).Payload(map.Complete()),
                        handle);
                }
                break;
            case TestFlags.ACCEPTING_REQUEST:
                {
                    ElementList elementList = new ElementList();

                    elementList.AddUInt(EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_UP);
                    elementList.AddUInt(EmaRdm.ENAME_ACCEPTING_REQS, 0);

                    FilterList filterList = new FilterList();

                    filterList.AddEntry(EmaRdm.SERVICE_STATE_ID, FilterAction.UPDATE, elementList.Complete());

                    Map map = new Map();

                    map.AddKeyUInt(1, MapAction.UPDATE, filterList.Complete());

                    provider.Submit(new UpdateMsg().DomainType(EmaRdm.MMT_DIRECTORY).Filter(EmaRdm.SERVICE_STATE_FILTER).Payload(map.Complete()),
                        handle);
                }
                break;
            case TestFlags.OPEN_WINDOWS_AND_LOAD_FACTOR:
                {
                    ElementList elementList = new ElementList();

                    elementList.AddUInt(EmaRdm.ENAME_OPEN_WINDOW, 90);
                    elementList.AddUInt(EmaRdm.ENAME_LOAD_FACT, 9999);

                    FilterList filterList = new FilterList();

                    filterList.AddEntry(EmaRdm.SERVICE_LOAD_ID, FilterAction.UPDATE, elementList.Complete());

                    Map map = new Map();

                    map.AddKeyUInt(1, MapAction.UPDATE, filterList.Complete());

                    provider.Submit(new UpdateMsg().DomainType(EmaRdm.MMT_DIRECTORY).Filter(EmaRdm.SERVICE_LOAD_FILTER).Payload(map.Complete()),
                        handle);
                }
                break;
            case TestFlags.MERGED_TO_GROUP:
            case TestFlags.MERGED_TO_GROUP_AND_SEND_GROUP_STATUS_CLOSED_RECOVER:
                {
                    ElementList elementList = new ElementList();

                    elementList.AddBuffer(EmaRdm.ENAME_GROUP, groupId);

                    byte[] buffer = new byte[2];
                    buffer[0] = 0;
                    buffer[1] = 7;

                    elementList.AddBuffer(EmaRdm.ENAME_MERG_TO_GRP, new EmaBuffer(buffer));

                    elementList.AddBuffer(EmaRdm.ENAME_GROUP, groupId);

                    FilterList filterList = new FilterList();

                    filterList.AddEntry(EmaRdm.SERVICE_GROUP_ID, FilterAction.UPDATE, elementList.Complete());

                    Map map = new Map();

                    map.AddKeyUInt(1, MapAction.UPDATE, filterList.Complete());

                    provider.Submit(new UpdateMsg().DomainType(EmaRdm.MMT_DIRECTORY).Filter(EmaRdm.SERVICE_GROUP_FILTER).Payload(map.Complete()),
                        handle);

                    if (testFlag == TestFlags.MERGED_TO_GROUP_AND_SEND_GROUP_STATUS_CLOSED_RECOVER)
                    {
                        elementList.Clear();

                        elementList.AddBuffer(EmaRdm.ENAME_GROUP, new EmaBuffer(buffer));
                        elementList.AddState(EmaRdm.ENAME_STATUS, OmmState.StreamStates.CLOSED_RECOVER, OmmState.DataStates.SUSPECT, OmmState.StatusCodes.NONE, "");

                        filterList.Clear();

                        filterList.AddEntry(EmaRdm.SERVICE_GROUP_ID, FilterAction.UPDATE, elementList.Complete());

                        map.Clear();

                        map.AddKeyUInt(1, MapAction.UPDATE, filterList.Complete());

                        provider.Submit(new UpdateMsg().DomainType(EmaRdm.MMT_DIRECTORY).Filter(EmaRdm.SERVICE_GROUP_FILTER).Payload(map.Complete()),
                            handle);
                    }
                }
                break;
            case TestFlags.GROUP_STATUS_DATA_OK:
                {
                    ElementList elementList = new ElementList();

                    elementList.AddBuffer(EmaRdm.ENAME_GROUP, groupId);
                    elementList.AddState(EmaRdm.ENAME_STATUS, OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "");

                    FilterList filterList = new FilterList();

                    filterList.AddEntry(EmaRdm.SERVICE_GROUP_ID, FilterAction.UPDATE, elementList.Complete());

                    Map map = new Map();

                    map.AddKeyUInt(1, MapAction.UPDATE, filterList.Complete());

                    provider.Submit(new UpdateMsg().DomainType(EmaRdm.MMT_DIRECTORY).Filter(EmaRdm.SERVICE_GROUP_FILTER).Payload(map.Complete()), handle);
                }
                break;
            case TestFlags.GROUP_STATUS_DATA_SUSPECT:
                {
                    ElementList elementList = new ElementList();

                    elementList.AddBuffer(EmaRdm.ENAME_GROUP, groupId);
                    elementList.AddState(EmaRdm.ENAME_STATUS, OmmState.StreamStates.OPEN, OmmState.DataStates.SUSPECT, OmmState.StatusCodes.NONE, "");

                    FilterList filterList = new FilterList();

                    filterList.AddEntry(EmaRdm.SERVICE_GROUP_ID, FilterAction.UPDATE, elementList.Complete());

                    Map map = new Map();

                    map.AddKeyUInt(1, MapAction.UPDATE, filterList.Complete());

                    provider.Submit(new UpdateMsg().DomainType(EmaRdm.MMT_DIRECTORY).Filter(EmaRdm.SERVICE_GROUP_FILTER).Payload(map.Complete()),
                        handle);
                }
                break;
            case TestFlags.GROUP_STATUS_CLOSED_RECOVER:
                {
                    ElementList elementList = new ElementList();

                    elementList.AddBuffer(EmaRdm.ENAME_GROUP, groupId);
                    elementList.AddState(EmaRdm.ENAME_STATUS, OmmState.StreamStates.CLOSED_RECOVER, OmmState.DataStates.SUSPECT, OmmState.StatusCodes.NONE, "");

                    FilterList filterList = new FilterList();

                    filterList.AddEntry(EmaRdm.SERVICE_GROUP_ID, FilterAction.UPDATE, elementList.Complete());

                    Map map = new Map();

                    map.AddKeyUInt(1, MapAction.UPDATE, filterList.Complete());

                    provider.Submit(new UpdateMsg().DomainType(EmaRdm.MMT_DIRECTORY).Filter(EmaRdm.SERVICE_GROUP_FILTER).Payload(map.Complete()),
                        handle);
                }
                break;
            case TestFlags.CHANGE_DATA_TYPE:
                {
                    ElementList elementList = new ElementList();

                    elementList.AddUInt(EmaRdm.ENAME_TYPE, 0);
                    elementList.AddAscii(EmaRdm.ENAME_DATA, "UpdatedData");

                    FilterList filterList = new FilterList();

                    filterList.AddEntry(EmaRdm.SERVICE_DATA_ID, FilterAction.UPDATE, elementList.Complete());

                    Map map = new Map();

                    map.AddKeyUInt(1, MapAction.UPDATE, filterList.Complete());

                    provider.Submit(new UpdateMsg().DomainType(EmaRdm.MMT_DIRECTORY).Filter(EmaRdm.SERVICE_DATA_FILTER).Payload(map.Complete()),
                        handle);
                }
                break;
            case TestFlags.CHANGE_LINK_TYPE_AND_STATE_AND_CODE:
                {
                    ElementList elementListLink1 = new ElementList();

                    elementListLink1.AddUInt(EmaRdm.ENAME_TYPE, 1);
                    elementListLink1.AddUInt(EmaRdm.ENAME_LINK_STATE, 0);
                    elementListLink1.AddUInt(EmaRdm.ENAME_LINK_CODE, 2);
                    elementListLink1.AddAscii(EmaRdm.ENAME_TEXT, "Text of Link1");

                    ElementList elementListLink2 = new ElementList();

                    elementListLink2.AddUInt(EmaRdm.ENAME_TYPE, 2);
                    elementListLink2.AddUInt(EmaRdm.ENAME_LINK_STATE, 0);
                    elementListLink2.AddUInt(EmaRdm.ENAME_LINK_CODE, 2);
                    elementListLink2.AddAscii(EmaRdm.ENAME_TEXT, "Text of Link2");

                    Map linkMap = new Map();

                    linkMap.AddKeyAscii(link1, MapAction.UPDATE, elementListLink1.Complete());
                    linkMap.AddKeyAscii(link2, MapAction.UPDATE, elementListLink2.Complete());

                    FilterList filterList = new FilterList();

                    filterList.AddEntry(EmaRdm.SERVICE_LINK_ID, FilterAction.UPDATE, linkMap.Complete());

                    Map map = new Map();

                    map.AddKeyUInt(1, MapAction.UPDATE, filterList.Complete());

                    provider.Submit(new UpdateMsg().DomainType(EmaRdm.MMT_DIRECTORY).Filter(EmaRdm.SERVICE_LINK_FILTER).Payload(map.Complete()),
                        handle);
                }
                break;
            case TestFlags.INFO_ADD_CAPABILITIES:
                {
                    OmmArray capabilites = new OmmArray();

                    capabilites.AddUInt(EmaRdm.MMT_MARKET_PRICE);
                    capabilites.AddUInt(EmaRdm.MMT_MARKET_BY_PRICE);
                    capabilites.AddUInt(EmaRdm.MMT_DICTIONARY);
                    capabilites.AddUInt(EmaRdm.MMT_MARKET_MAKER);

                    ElementList elementList = new ElementList();

                    elementList.AddArray(EmaRdm.ENAME_CAPABILITIES, capabilites.Complete());

                    FilterList filterList = new FilterList();

                    filterList.AddEntry(EmaRdm.SERVICE_INFO_ID, FilterAction.UPDATE, elementList.Complete());

                    Map map = new Map();

                    map.AddKeyUInt(1, MapAction.UPDATE, filterList.Complete());

                    provider.Submit(new UpdateMsg().DomainType(EmaRdm.MMT_DIRECTORY).Filter(EmaRdm.SERVICE_INFO_FILTER).Payload(map.Complete()),
                        handle);
                }
                break;
            case TestFlags.SEQUENCED_MULTICAST:
                {
                    ElementList mCastEntry = new ElementList();

                    mCastEntry.AddAscii("MulticastGroup", "224.1.62.5");
                    mCastEntry.AddUInt("Port", 50001);
                    mCastEntry.AddUInt("Domain", EmaRdm.MMT_MARKET_BY_PRICE);

                    Vector vectorMCastEntry = new Vector();

                    vectorMCastEntry.Add(1, VectorAction.UPDATE, mCastEntry.Complete(), null);

                    ElementList elementList = new ElementList();

                    elementList.AddAscii("ReferenceDataServerHost", "10.0.5.125");
                    elementList.AddUInt("ReferenceDataServerPort", 19005);
                    elementList.AddAscii("SnapshotServerHost", "10.0.5.125");
                    elementList.AddUInt("SnapshotServerPort", 15002);
                    elementList.AddAscii("GapRecoveryServerHost", "10.0.5.125");
                    elementList.AddUInt("GapRecoveryServerPort", 15001);
                    elementList.AddVector("StreamingMulticastChannels", vectorMCastEntry.Complete());

                    FilterList filterList = new FilterList();

                    filterList.AddEntry(7, FilterAction.UPDATE, elementList.Complete());

                    Map map = new Map();

                    map.AddKeyUInt(1, MapAction.UPDATE, filterList.Complete());

                    provider.Submit(new UpdateMsg().DomainType(EmaRdm.MMT_DIRECTORY).Filter(64).Payload(map.Complete()),
                        handle);
                }
                break;
        }
    }

    public static void Main(string[] args)
    {
        if (args.Length != 1)
        {
            PrintUsage();
            return;
        }
        else
        {
            if (args[0].Equals("-serviceDown"))
            {
                testFlag |= TestFlags.SERVICE_DOWN;
            }
            else if (args[0].Equals("-openWindowsAndLoadFactor"))
            {
                testFlag |= TestFlags.OPEN_WINDOWS_AND_LOAD_FACTOR;
            }
            else if (args[0].Equals("-mergedToGroup"))
            {
                testFlag |= TestFlags.MERGED_TO_GROUP;

                byte[] buffer = new byte[2];
                buffer[0] = 0;
                buffer[1] = 5;

                groupId = new EmaBuffer(buffer);
            }
            else if (args[0].Equals("-mergedToGroupAndSendGroupStatusClosedRecover"))
            {
                testFlag |= TestFlags.MERGED_TO_GROUP_AND_SEND_GROUP_STATUS_CLOSED_RECOVER;

                byte[] buffer = new byte[2];
                buffer[0] = 0;
                buffer[1] = 5;

                groupId = new EmaBuffer(buffer);
            }
            else if (args[0].Equals("-groupStatusDataOk"))
            {
                testFlag |= TestFlags.GROUP_STATUS_DATA_OK;

                byte[] buffer = new byte[2];

                buffer[0] = 0;
                buffer[1] = 5;

                groupId = new EmaBuffer(buffer);
            }
            else if (args[0].Equals("-groupStatusDataSuspect"))
            {
                testFlag |= TestFlags.GROUP_STATUS_DATA_SUSPECT;

                byte[] buffer = new byte[2];

                buffer[0] = 0;
                buffer[1] = 5;

                groupId = new EmaBuffer(buffer);
            }
            else if (args[0].Equals("-groupStatusClosedRecover"))
            {
                testFlag |= TestFlags.GROUP_STATUS_CLOSED_RECOVER;

                byte[] buffer = new byte[2];

                buffer[0] = 0;
                buffer[1] = 5;

                groupId = new EmaBuffer(buffer);
            }
            else if (args[0].Equals("-changeDataType"))
            {
                testFlag |= TestFlags.CHANGE_DATA_TYPE;
            }
            else if (args[0].Equals("-chagneLinkTypeAndStateAndCode"))
            {
                testFlag |= TestFlags.CHANGE_LINK_TYPE_AND_STATE_AND_CODE;
            }
            else if (args[0].Equals("-infoAddCapabilities"))
            {
                testFlag |= TestFlags.INFO_ADD_CAPABILITIES;
            }
            else if (args[0].Equals("-sequencedMulticast"))
            {
                testFlag |= TestFlags.SEQUENCED_MULTICAST;
            }
            else if (args[0].Equals("-acceptingRequest"))
            {
                testFlag |= TestFlags.ACCEPTING_REQUEST;
            }
            else if (args[0].Equals("-serviceDelete"))
            {
                testFlag |= TestFlags.SERVICE_DELETE;
            }

            if (testFlag == TestFlags.NONE)
            {
                Console.WriteLine("Invalid argument " + args[0]);

                return;
            }
        }

        OmmProvider? provider = null;
        try
        {
            AppClient appClient = new AppClient();
            FieldList fieldList = new FieldList();

            OmmIProviderConfig config = new OmmIProviderConfig();

            provider = new OmmProvider(config.OperationModel(OmmIProviderConfig.OperationModelMode.USER_DISPATCH), appClient);

            while (appClient.ItemHandle == 0)
            {
                provider.Dispatch(1000);
            }

            for (int index = 0; index < numberOfUpdates; index++)
            {
                fieldList.Clear();
                fieldList.AddTime(1025, 14, 40, 32);
                fieldList.AddInt(3855, 52832001);
                fieldList.AddReal(22, 14400 + (((appClient.ItemHandle & 0x1) == 1) ? 1 : 10), OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
                fieldList.AddReal(30, 10 + (((appClient.ItemHandle & 0x1) == 1) ? 10 : 20), OmmReal.MagnitudeTypes.EXPONENT_0);
                fieldList.AddRmtes(296, new EmaBuffer(Encoding.ASCII.GetBytes("NAS")));

                if (appClient.submitting)
                {
                    try
                    {
                        provider.Submit(new UpdateMsg().Payload(fieldList.Complete()), appClient.ItemHandle);
                    }
                    catch (Exception excp) 
                    {
                        Console.WriteLine(excp.Message);
                    }
                }

                if (index == 1)
                {
                    AddSourceDirectory(provider, 0);
                }
                else if (index == 3)
                {
                    UpdateSourceDirectory(provider, 0);
                    provider.Dispatch(1000);
                }

                if (index >= 3)
                {
                    Thread.Sleep(2000);
                }

                provider.Dispatch(1000);
            }
        }
        catch (OmmInvalidUsageException excp)
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
