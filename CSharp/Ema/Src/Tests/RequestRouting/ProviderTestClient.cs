/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Rdm;
using LSEG.Eta.Codec;
using LSEG.Eta.Common;
using System.Collections.Generic;
using System.Text;
using System.Threading;
using Xunit.Abstractions;
using DataDictionary = LSEG.Ema.Rdm.DataDictionary;

namespace LSEG.Ema.Access.Tests.RequestRouting
{
    internal class RequestAttributes
    {
        public long Handle = 0;

        public string? Name;

        public int ServiceId =  -1;
    }

    internal class ProviderTestClient : IOmmProviderClient
    {
        private ProviderTestOptions m_ProviderTestOptions;
        private Timer? m_Timer;
        private OmmProvider? m_Provider;
        private string? m_LoginUserName;
        private ElementList? m_RefreshAttributes;
        Queue<Msg> m_MessageQueue = new(30);
        private Dictionary<string, RequestAttributes> m_ItemNameToHandleDict = new(10);

        /* This is service Id from the request message. -1 indicates that the service Id is not set */
        private int m_ServiceId = -1;

        private static DataDictionary? DataDictionary;

        private const int fragmentationSize = 1280000;
        private Series m_Series = new Series();
        private RefreshMsg m_RefreshMsg = new RefreshMsg();
        private int currentValue;
        private bool result;

        private MonitorWriteLocker AccessLock { get; set; } = new MonitorWriteLocker(new object());

        public long LoginHandle;

        readonly ITestOutputHelper m_Output;

        public ProviderTestClient(ITestOutputHelper output, ProviderTestOptions providerTestOptions)
        {
            m_ProviderTestOptions = providerTestOptions;

            m_Output = output;
        }

        public long RetriveItemHandle(string itemName)
        {
            if(m_ItemNameToHandleDict.TryGetValue(itemName, out var reqAttrib))
            {
                return reqAttrib.Handle;
            }

            return 0;
        }

        public int QueueSize()
        {
            AccessLock.Enter();
            try
            {
                return m_MessageQueue.Count;
            }
            finally
            {
                AccessLock.Exit();
            }
        }

        public Msg PopMessage()
        {
            AccessLock.Enter();
            try
            {
                return m_MessageQueue.Dequeue();
            }
            finally
            {
                AccessLock.Exit();
            }
        }

        public void SendLoginResponse(OmmProvider provider, bool acceptLoginRequest, string? statusText = null)
        {
            using var _ = EtaGlobalPoolTestUtil.CreateClearableSection();

            if (acceptLoginRequest)
            {
                if (m_ProviderTestOptions.SendRefreshAttrib)
                {
                    provider.Submit(new RefreshMsg().DomainType(EmaRdm.MMT_LOGIN).Name(m_LoginUserName!).NameType(EmaRdm.USER_NAME).Complete(true).Solicited(true).Attrib(m_RefreshAttributes!).
                        State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "Login accepted").MarkForClear(),LoginHandle);
                }
                else
                {
                    provider.Submit(new RefreshMsg().DomainType(EmaRdm.MMT_LOGIN).Name(m_LoginUserName!).NameType(EmaRdm.USER_NAME).Complete(true).Solicited(true).
                        State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "Login accepted").MarkForClear(), LoginHandle);
                }
            }
            else
            {
                provider.Submit(new StatusMsg().DomainType(EmaRdm.MMT_LOGIN).Name(m_LoginUserName!).NameType(EmaRdm.USER_NAME).
                        State(OmmState.StreamStates.CLOSED, OmmState.DataStates.SUSPECT, OmmState.StatusCodes.NOT_AUTHORIZED, statusText ?? "Login denied"), LoginHandle);
            }

            m_RefreshAttributes?.ClearAndReturnToPool_All();
        }

        void ProcessDictionaryRequest(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
        {
            using var _ = EtaGlobalPoolTestUtil.CreateClearableSection();

            if (DataDictionary == null)
            {
                DataDictionary = new DataDictionary();

                DataDictionary.LoadFieldDictionary("RDMFieldDictionary");
                DataDictionary.LoadEnumTypeDictionary("enumtype.def");
            }

            result = false;
            m_RefreshMsg.Clear().ClearCache(true);
            m_Series = new Series();

            if (reqMsg.Name().Equals("RWFFld"))
            {
                currentValue = DataDictionary.MinFid;

                while (!result)
                {
                    currentValue = DataDictionary.EncodeFieldDictionary(m_Series, currentValue, reqMsg.Filter(), fragmentationSize);

                    result = currentValue == DataDictionary.MaxFid ? true : false;

                    providerEvent.Provider.Submit(m_RefreshMsg.Name(reqMsg.Name()).ServiceName(reqMsg.ServiceName()).
                            DomainType(EmaRdm.MMT_DICTIONARY).Filter(reqMsg.Filter()).Payload(m_Series).Complete(result).
                            Solicited(true), providerEvent.Handle);

                    m_RefreshMsg.MarkForClear().Clear();
                }
            }
            else if (reqMsg.Name().Equals("RWFEnum"))
            {
                currentValue = 0;

                while (!result)
                {
                    currentValue = DataDictionary.EncodeEnumTypeDictionary(m_Series, currentValue, reqMsg.Filter(), fragmentationSize);

                    result = currentValue == DataDictionary.EnumTables().Count ? true : false;

                    providerEvent.Provider.Submit(m_RefreshMsg.Name(reqMsg.Name()).ServiceName(reqMsg.ServiceName()).
                            DomainType(EmaRdm.MMT_DICTIONARY).Filter(reqMsg.Filter()).Payload(m_Series).Complete(result).
                            Solicited(true), providerEvent.Handle);

                    m_RefreshMsg.MarkForClear().Clear();
                }
            }

            m_Series.MarkForClear();
        }

        public void OnReqMsg(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
        {
            m_Output.WriteLine($"OnReqMsg({providerEvent.Provider.ProviderName})");

            using var _ = EtaGlobalPoolTestUtil.CreateClearableSection();

            switch (reqMsg.DomainType())
            {
                case EmaRdm.MMT_LOGIN:

                    m_Output.WriteLine($"ChannelInformation on login domain:");
                    m_Output.WriteLine(providerEvent.ChannelInformation().ToString());

                    AccessLock.Enter();
                    try
                    {
                        m_Provider = providerEvent.Provider;
                        m_LoginUserName = reqMsg.Name();
                        LoginHandle = providerEvent.Handle;

                        m_RefreshAttributes = new ElementList();

                        if (reqMsg.Attrib().DataType == DataType.DataTypes.ELEMENT_LIST)
                        {
                            ElementList reqAttributes = reqMsg.Attrib().ElementList();

                            if (m_ProviderTestOptions.SupportOMMPosting)
                            {
                                m_RefreshAttributes.AddUInt(EmaRdm.ENAME_SUPPORT_POST, 1);
                            }

                            if(m_ProviderTestOptions.SupportOptimizedPauseAndResume)
                            {
                                m_RefreshAttributes.AddUInt(EmaRdm.ENAME_SUPPORT_OPR, 1);
                            }

                            if (m_ProviderTestOptions.SupportStandby)
                            {
                                m_RefreshAttributes.AddUInt(EmaRdm.ENAME_SUPPORT_STANDBY, 1);
                                m_RefreshAttributes.AddUInt(EmaRdm.ENAME_WARMSTANDBY_MODE, 1);
                            }

                            foreach (var reqAttrib in reqAttributes)
                            {
                                string name = reqAttrib.Name;

                                if (name.Equals(EmaRdm.ENAME_ALLOW_SUSPECT_DATA) || name.Equals(EmaRdm.ENAME_SINGLE_OPEN))
                                {
                                    m_RefreshAttributes.AddUInt(name, reqAttrib.UIntValue());
                                }
                                else if (name.Equals(EmaRdm.ENAME_APP_ID) || name.Equals(EmaRdm.ENAME_POSITION))
                                {
                                    m_RefreshAttributes.AddAscii(name, reqAttrib.OmmAsciiValue().ToString());
                                }
                            }

                            m_RefreshAttributes.Complete();
                        }

                        if (m_ProviderTestOptions.SendLoginResponseInMiliSecond == 0)
                        {
                            SendLoginResponse(providerEvent.Provider, m_ProviderTestOptions.AcceptLoginRequest);
                        }
                        else
                        {
                            m_Timer = new Timer(ProcessTimeout, providerEvent.Provider, m_ProviderTestOptions.SendLoginResponseInMiliSecond, -1);
                        }
                    }
                    finally
                    {
                        AccessLock.Exit();
                    }

                    break;

                case EmaRdm.MMT_MARKET_PRICE:
                case 55:

                    AccessLock.Enter();
                    try
                    {
                        RequestMsg requestMsg = new (reqMsg.MarkForClear());
                        m_Output.WriteLine(requestMsg.ToString());
                        m_MessageQueue.Enqueue(requestMsg);

                        if(reqMsg.HasServiceId)
                        {
                            m_ServiceId = reqMsg.ServiceId();
                        }

                        if(m_ProviderTestOptions.SendItemResponse == false)
                        {
                            m_Output.WriteLine("Skip sending item response for this request");
                            break;
                        }
                        else if(m_ProviderTestOptions.CloseItemRequest)
                        {
                            m_Output.WriteLine("Closes this item request");

                            providerEvent.Provider.Submit(new StatusMsg().DomainType(reqMsg.DomainType()).Name(reqMsg.Name()).
                                State(OmmState.StreamStates.CLOSED, OmmState.DataStates.SUSPECT, OmmState.StatusCodes.NOT_AUTHORIZED, "Unauthorized access to the item."), providerEvent.Handle);
                            break;
                        }

                        FieldList fieldList = new ();
                        fieldList.AddReal(22, 3990, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
                        fieldList.AddReal(25, 3994, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
                        fieldList.AddReal(30, 9, OmmReal.MagnitudeTypes.EXPONENT_0);
                        fieldList.AddReal(31, 19, OmmReal.MagnitudeTypes.EXPONENT_0);
                        fieldList.MarkForClear().Complete();

                        RefreshMsg refreshMsg = new RefreshMsg().Name(reqMsg.Name()).ServiceId(reqMsg.ServiceId()).Solicited(true).DomainType(reqMsg.DomainType()).
                            State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "Refresh Completed").Complete(true);

                        if(m_ProviderTestOptions.SupportStandby)
                        {
                            // TODO: Add code for handling a response message for active and standby servers.
                        }
                        else
                        {
                            refreshMsg.Payload(fieldList);
                        }

                        if(m_ProviderTestOptions.ItemGroupId != null)
                        {
                            refreshMsg.ItemGroup(m_ProviderTestOptions.ItemGroupId);
                        }

                        providerEvent.Provider.Submit(refreshMsg.MarkForClear(), providerEvent.Handle);

                        if(m_ProviderTestOptions.SendUpdateMessage)
                        {
                            FieldList fieldList2 = new FieldList();
                            fieldList2.AddReal(22, 3991, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
                            fieldList2.AddReal(25, 3995, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
                            fieldList2.MarkForClear().Complete();

                            UpdateMsg updateMsg = new UpdateMsg().Name(reqMsg.Name()).ServiceId(reqMsg.ServiceId()).UpdateTypeNum(EmaRdm.INSTRUMENT_UPDATE_TRADE).DomainType(reqMsg.DomainType())
                                .Payload(fieldList2);

                            providerEvent.Provider.Submit( updateMsg.MarkForClear(), providerEvent.Handle);
                        }

                        RequestAttributes requestAttributes = new()
                        {
                            Name = reqMsg.Name(),
                            Handle = providerEvent.Handle,
                            ServiceId = reqMsg.ServiceId()
                        };

                        m_ItemNameToHandleDict[reqMsg.Name()] = requestAttributes;
                    }
                    finally
                    { 
                        AccessLock.Exit();
                    }

                    break;

                case EmaRdm.MMT_DIRECTORY:

                    if(m_ProviderTestOptions.SourceDirectoryPayload != null)
                    {
                        RefreshMsg refreshMsg = new RefreshMsg().DomainType(EmaRdm.MMT_DIRECTORY).ClearCache(true).Filter(EmaRdm.SERVICE_INFO_FILTER | EmaRdm.SERVICE_STATE_FILTER).
                            Payload(m_ProviderTestOptions.SourceDirectoryPayload).Solicited(true).Complete(true).MarkForClear();

                        providerEvent.Provider.Submit(refreshMsg, providerEvent.Handle);
                    }

                    break;

                case EmaRdm.MMT_DICTIONARY:

                    AccessLock.Enter();

                    try
                    {
                        RequestMsg cloneMsg = new (reqMsg);

                        m_Output.WriteLine(cloneMsg.ToString());

                        m_MessageQueue.Enqueue(cloneMsg);

                        ProcessDictionaryRequest(reqMsg, providerEvent);
                    }
                    finally
                    {
                        AccessLock.Exit();
                    }

                    break;

                case EmaRdm.MMT_SYMBOL_LIST:

                    AccessLock.Enter();

                    try
                    {
                        RequestMsg cloneMsg = new(reqMsg);

                        m_Output.WriteLine(cloneMsg.ToString());

                        m_MessageQueue.Enqueue(cloneMsg);

                        FieldList summaryData = new FieldList();
                        summaryData.AddUInt(1, 74);
                        summaryData.AddRmtes(3, new EmaBuffer(Encoding.ASCII.GetBytes("TOP 25 BY VOLUME")));
                        summaryData.AddRealFromDouble(77, 1864.0);
                        summaryData.AddEnumValue(1709, 559);
                        summaryData.AddTime(3798, 15, 45, 47, 0, 0, 0);
                        summaryData.AddTime(14269, 15, 45, 47, 451, 675, 0);
                        summaryData.MarkForClear().Complete();

                        Map map = new Map();

                        map.TotalCountHint(3);
                        map.SummaryData(summaryData);

                        FieldList mapEntryValue = new FieldList();
                        mapEntryValue.AddUInt(6453, 1);
                        mapEntryValue.MarkForClear().Complete();

                        map.AddKeyBuffer(new EmaBuffer(Encoding.ASCII.GetBytes("itemA")), MapAction.ADD, mapEntryValue);

                        mapEntryValue.Clear().AddUInt(6453, 2);
                        mapEntryValue.Complete();
                        map.AddKeyBuffer(new EmaBuffer(Encoding.ASCII.GetBytes("itemB")), MapAction.ADD, mapEntryValue);

                        mapEntryValue.Clear().AddUInt(6453, 3);
                        mapEntryValue.Complete();
                        map.AddKeyBuffer(new EmaBuffer(Encoding.ASCII.GetBytes("itemC")), MapAction.ADD, mapEntryValue);
                        map.MarkForClear().Complete();

                        RefreshMsg refreshMsg = new RefreshMsg().Name(reqMsg.Name()).DomainType(EmaRdm.MMT_SYMBOL_LIST).ServiceId(reqMsg.ServiceId()).Solicited(true).
                                State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "Refresh Completed").
                                Payload(map).Complete(true);

                        providerEvent.Provider.Submit(refreshMsg.MarkForClear(), providerEvent.Handle);
                    }
                    finally
                    {
                        AccessLock.Exit();
                    }

                    break;
            }
        }

        public void OnClose(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
        {
            RequestMsg cloneMsg = new (reqMsg);

            m_Output.WriteLine($"OnClose({providerEvent.Provider.ProviderName}) {cloneMsg}");

            AccessLock.Enter();
            try
            {
                m_MessageQueue.Enqueue(cloneMsg);
            }
            finally
            {
                AccessLock.Exit();
            }
        }

        public void OnPostMsg(PostMsg postMsg, IOmmProviderEvent providerEvent) 
        {
            using var _ = EtaGlobalPoolTestUtil.CreateClearableSection();

            PostMsg cloneMsg = new(postMsg.MarkForClear());

            m_Output.WriteLine($"OnPost({providerEvent.Provider.ProviderName}) {cloneMsg}");

            AccessLock.Enter();
            try
            {
                m_MessageQueue.Enqueue(cloneMsg);

                if (postMsg.SolicitAck())
                {
                    AckMsg ackMsg = new AckMsg();

                    if (postMsg.HasSeqNum)
                    {
                        ackMsg.SeqNum(postMsg.SeqNum());
                    }
                    if (postMsg.HasName)
                    {
                        ackMsg.Name(postMsg.Name());
                    }
                    if (postMsg.HasServiceId)
                    {
                        ackMsg.ServiceId(postMsg.ServiceId());
                    }

                    ackMsg.AckId(postMsg.PostId()).DomainType(postMsg.DomainType());

                    providerEvent.Provider.Submit(ackMsg.MarkForClear(), providerEvent.Handle);
                }
            }
            finally
            {
                AccessLock.Exit();
            }
        }

        public void OnGenericMsg(GenericMsg genericMsg, IOmmProviderEvent providerEvent)
        {
            GenericMsg cloneMsg = new(genericMsg);

            m_Output.WriteLine($"OnGeneric({providerEvent.Provider.ProviderName}) {cloneMsg}");

            AccessLock.Enter();
            try
            {
                m_MessageQueue.Enqueue(cloneMsg);

                if (m_ProviderTestOptions.SendGenericMessage)
                {
                    GenericMsg submitMsg = new GenericMsg().Name(genericMsg.Name()).DomainType(genericMsg.DomainType()).Complete(genericMsg.Complete());

                    if (genericMsg.HasServiceId) // Service ID is set from the request message.
                    {
                        if(m_ProviderTestOptions.SubmitGenericMsgWithServiceId != -1)
                        {
                            submitMsg.ServiceId(m_ProviderTestOptions.SubmitGenericMsgWithServiceId);
                        }
                        else if (m_ServiceId != -1 && genericMsg.ServiceId() == m_ServiceId)
                        {
                            submitMsg.ServiceId(genericMsg.ServiceId());
                        }
                    }

                    providerEvent.Provider.Submit(submitMsg, providerEvent.Handle);
                }
            }
            finally
            {
                AccessLock.Exit();
            }
        }

        public void OnReissue(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
        {
            RequestMsg cloneMsg = new(reqMsg);

            m_Output.WriteLine($"OnReissue({providerEvent.Provider.ProviderName}) {cloneMsg}");

            AccessLock.Enter();
            try
            {
                m_MessageQueue.Enqueue(cloneMsg);

                if (reqMsg.DomainType() == EmaRdm.MMT_DICTIONARY)
                {
                    ProcessDictionaryRequest(reqMsg, providerEvent);
                }
            }
            finally
            {
                AccessLock.Exit();
            }
        }

        public void ForceLogout()
        {
            SendLoginResponse(m_Provider!, false, "Force logout");
        }

        public void ProcessTimeout(object? state)
        {
            OmmProvider? provider = (OmmProvider?)state;
            if(provider != null)
                SendLoginResponse(provider, m_ProviderTestOptions.AcceptLoginRequest);
        }
    }
}
