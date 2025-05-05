/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Collections.Generic;
using System.Text;

using LSEG.Eta.Codec;
using LSEG.Eta.Common;
using LSEG.Eta.ValueAdd.Rdm;
using LSEG.Eta.ValueAdd.Reactor;

namespace LSEG.Ema.Access;

/// <summary>
/// Used to lookup items by the ItemName and respond with the cached dictionary information
/// </summary>
internal class DictionaryPayload
{
    internal enum DictionaryType
    {
        FIELD_DICTIONARY,
        ENUM_TYPE
    }

    internal DictionaryPayload(DataDictionary dataDictionary, DictionaryType dictionaryType)
    {
        Dictionary = dataDictionary;
        DataDictionaryType = dictionaryType;
    }

    internal readonly DataDictionary Dictionary;

    internal readonly DictionaryType DataDictionaryType;

}

/// <summary>
/// Handles Dictionaries for OMM Interactive Provider.
/// </summary>
///
/// <seealso cref="IOmmProviderImpl"/>
/// <seealso cref="OmmIProviderImpl"/>
internal class DictionaryHandler : IDictionaryMsgCallback
{
    private const int INIT_DICTIONARY_STATUS_MSG_SIZE = 256;
    private const string CLIENT_NAME = "DictionaryHandler";

    private enum DictionaryRejectEnum
    {
        DICTIONARY_NOT_LOADED,
        DICTIONARY_ENCODING_FAILED,
        USER_IS_NOT_LOGGED_IN,
        DICTIONARY_NAME_NOT_FOUND,
        SERVICE_ID_NOT_FOUND,
        DICTIONARY_INVALID_MESSAGE,
        DICTIONARY_UNHANDLED_MESSAGE
    };

    protected OmmServerBaseImpl m_ommServerBaseImpl;

    private readonly EncodeIterator m_EncodeIter = new EncodeIterator();
    private readonly DictionaryRefresh m_DictionaryRefresh = new DictionaryRefresh();
    private readonly DictionaryStatus m_DictionaryStatus = new DictionaryStatus();

    private int m_MaxFieldDictFragmentSize;
    private int m_MaxEnumTypeFragmentSize;

    /// <summary>
    /// Assigned <c>true</c> when the Dictionary domain is controlled by the API, <c>false</c>
    /// otherwise (i.e. by the User).
    /// </summary>
    private bool m_ApiAdminControl;

    /// <summary>
    /// Maps "{ItemName}{ServiceId}" to DataDictionaries.
    /// </summary>
    private readonly Dictionary<string, DictionaryPayload> m_DictionaryInfoHash = new();
    private readonly Dictionary<long, DataDictionary> m_ServiceDictionaryByIdHash = new();
    private readonly StringBuilder m_DictionaryNameAndServiceId = new StringBuilder();
    private readonly List<ItemInfo> m_ItemInfoList = new List<ItemInfo>();

    internal DictionaryHandler(OmmServerBaseImpl ommServerBaseImpl)
    {
        m_ommServerBaseImpl = ommServerBaseImpl;
    }

    internal List<ItemInfo> ItemInfoList
    {
        get { return m_ItemInfoList; }
    }

    public void Initialize()
    {
        m_ApiAdminControl = (m_ommServerBaseImpl.ConfigImpl.AdminControlDictionary == OmmIProviderConfig.AdminControlMode.API_CONTROL);

        if (m_ApiAdminControl)
        {
            m_MaxFieldDictFragmentSize = m_ommServerBaseImpl.ConfigImpl.IProviderConfig.FieldDictionaryFragmentSize;
            m_MaxEnumTypeFragmentSize = m_ommServerBaseImpl.ConfigImpl.IProviderConfig.EnumTypeFragmentSize;

            LoadDictionaryFromFile();
        }
    }

    /// <summary>
    /// Loads Enum and RDM Fields dictionaries defined in the config./>
    /// into <see cref="m_DictionaryInfoHash"/>.
    /// </summary>
    /// <exception cref="OmmInvalidUsageException">
    ///   when <see cref="DataDictionary.LoadEnumTypeDictionary(string, out CodecError)"/>
    ///   or <see cref="DataDictionary.LoadFieldDictionary(string, out CodecError)"/>
    ///   fails to load a dictionary.</exception>
    internal void LoadDictionaryFromFile()
    {
        StringBuilder fieldNameAndServiceId = new StringBuilder();
        StringBuilder enumTypeAndServiceId = new StringBuilder();
        bool existingFieldName;
        bool existingEnumName;

        foreach (EmaServiceConfig emaService in m_ommServerBaseImpl.ConfigImpl.DirectoryConfig.ServiceMap.Values)
        {
            int serviceId = emaService.Service.ServiceId;

            foreach (string dictionaryConfigName in emaService.DictionariesProvidedList)
            {
                DictionaryConfig? dictionaryConfig;

                if (!m_ommServerBaseImpl.ConfigImpl.DictionaryConfigMap.TryGetValue(dictionaryConfigName, out dictionaryConfig) || dictionaryConfig == null)
                {
                    throw new OmmInvalidUsageException($"DictionaryHandler.LoadDictionaryFromFile() Dictionary {dictionaryConfigName} is missing from the dictionary config.");
                }
                fieldNameAndServiceId.Clear();
                fieldNameAndServiceId.Append(dictionaryConfig.RdmFieldDictionaryItemName).Append(serviceId);

                enumTypeAndServiceId.Clear();
                enumTypeAndServiceId.Append(dictionaryConfig.EnumTypeDefItemName).Append(serviceId);

                existingFieldName = m_DictionaryInfoHash.ContainsKey(fieldNameAndServiceId.ToString())
                    && m_DictionaryInfoHash[fieldNameAndServiceId.ToString()] is not null;

                existingEnumName = m_DictionaryInfoHash.ContainsKey(enumTypeAndServiceId.ToString())
                    && m_DictionaryInfoHash[enumTypeAndServiceId.ToString()] is not null;

                // The dictionary already has been loaded and stored, so we don't need to do anything here.
                if (existingFieldName && existingEnumName)
                {
                    continue;
                }

                DataDictionary dictionary = new DataDictionary();

                if (dictionary.LoadFieldDictionary(dictionaryConfig.RdmFieldDictionaryFileName, out var loadFieldsError) < CodecReturnCode.SUCCESS)
                {
                    StringBuilder temp = m_ommServerBaseImpl.GetStrBuilder();
                    temp.AppendLine("DictionaryHandler.LoadDictionaryFromFile() failed while initializing DictionaryHandler.")
                        .AppendLine($"Unable to load RDMFieldDictionary file named {dictionaryConfig.RdmFieldDictionaryFileName}")
                        .Append($"Error Text: {loadFieldsError.Text}");

                    if (m_ommServerBaseImpl.GetLoggerClient().IsErrorEnabled)
                    {
                        temp.AppendLine().Append("Instance Name ").Append(m_ommServerBaseImpl.InstanceName);

                        m_ommServerBaseImpl.GetLoggerClient().Error(CLIENT_NAME, temp.ToString());
                    }

                    throw new OmmInvalidUsageException(temp.ToString(), (int)loadFieldsError.ErrorId);
                }

                if (dictionary.LoadEnumTypeDictionary(dictionaryConfig.EnumTypeDefFileName, out var loadEnumsError) < CodecReturnCode.SUCCESS)
                {
                    StringBuilder temp = m_ommServerBaseImpl.GetStrBuilder();
                    temp.AppendLine("DictionaryHandler.LoadDictionaryFromFile() failed while initializing DictionaryHandler.")
                        .AppendLine($"Unable to load enumtype.def file named {dictionaryConfig.EnumTypeDefFileName}")
                        .Append($"Error Text: {loadEnumsError.Text}");

                    if (m_ommServerBaseImpl.GetLoggerClient().IsErrorEnabled)
                    {
                        temp.AppendLine().Append("Instance Name ").Append(m_ommServerBaseImpl.InstanceName);

                        m_ommServerBaseImpl.GetLoggerClient().Error(CLIENT_NAME, temp.ToString());
                    }

                    throw new OmmInvalidUsageException(temp.ToString(), (int)loadEnumsError.ErrorId);
                }

                if (!existingFieldName)
                {
                    m_DictionaryInfoHash[fieldNameAndServiceId.ToString()] = new DictionaryPayload(dictionary,
                        DictionaryPayload.DictionaryType.FIELD_DICTIONARY);
                }

                if (!existingEnumName)
                {
                    m_DictionaryInfoHash[enumTypeAndServiceId.ToString()] = new DictionaryPayload(dictionary,
                        DictionaryPayload.DictionaryType.ENUM_TYPE);
                }

                m_ServiceDictionaryByIdHash[serviceId] = dictionary;
            }
        }
    }

    internal DataDictionary? GetDictionaryByServiceId(int serviceId)
    {
        m_ServiceDictionaryByIdHash.TryGetValue(serviceId, out var serviceDictionary);

        return serviceDictionary;
    }

    /// <summary>
    /// Implements Reactor <see cref="IDictionaryMsgCallback"/> callback for Dictionary events.
    /// </summary>
    /// <param name="dictEvent"></param>
    /// <returns><see cref="ReactorCallbackReturnCode.SUCCESS"/></returns>
    /// <seealso cref="IDictionaryMsgCallback"/>
    public ReactorCallbackReturnCode RdmDictionaryMsgCallback(RDMDictionaryMsgEvent dictEvent)
    {
        m_ommServerBaseImpl.EventReceived();

        ClientSession clientSession = (ClientSession)dictEvent.ReactorChannel!.UserSpecObj!;
        ReactorChannel rsslReactorChannel = dictEvent.ReactorChannel;
        DictionaryMsg? dictionaryMsg = dictEvent.DictionaryMsg;

        if (dictionaryMsg is null)
        {
            SendRequestReject(rsslReactorChannel, dictEvent.Msg!.StreamId, 0, null,
                DictionaryRejectEnum.DICTIONARY_INVALID_MESSAGE, false);

            if (m_ommServerBaseImpl.GetLoggerClient().IsErrorEnabled)
            {
                using var lockScope = m_ommServerBaseImpl.GetUserLocker().EnterLockScope();
                StringBuilder temp = m_ommServerBaseImpl.GetStrBuilder()
                    .AppendLine("Dictionary message rejected - Invalid dictionary domain message.")
                    .AppendLine($"Stream Id {dictEvent.Msg!.StreamId}")
                    .AppendLine($"Client handle {clientSession.ClientHandle}")
                    .Append("Instance Name ").Append(m_ommServerBaseImpl.InstanceName);

                m_ommServerBaseImpl.GetLoggerClient().Error(CLIENT_NAME, temp.ToString());
            }

            return ReactorCallbackReturnCode.SUCCESS;
        }

        if (!m_ommServerBaseImpl.ConfigImpl.IProviderConfig.AcceptMessageWithoutBeingLogin && !clientSession.IsLogin)
        {
            if (dictionaryMsg.DictionaryMsgType == DictionaryMsgType.REQUEST)
            {
                SendRequestReject(rsslReactorChannel, dictionaryMsg, DictionaryRejectEnum.USER_IS_NOT_LOGGED_IN, true);
            }
            else
            {
                SendRequestReject(rsslReactorChannel, dictEvent.Msg!.StreamId, 0,
                    null, DictionaryRejectEnum.USER_IS_NOT_LOGGED_IN, true);
            }

            return ReactorCallbackReturnCode.SUCCESS;
        }

        switch (dictionaryMsg.DictionaryMsgType)
        {
            case DictionaryMsgType.REQUEST:
                {
                    if (m_ommServerBaseImpl.GetLoggerClient().IsTraceEnabled)
                    {
                        using var lockScope = m_ommServerBaseImpl.GetUserLocker().EnterLockScope();
                        StringBuilder temp = m_ommServerBaseImpl.GetStrBuilder();
                        temp.AppendLine("Received dictionary request message.")
                            .AppendLine($"\tStream Id {dictionaryMsg.StreamId}")
                            .AppendLine($"\tClient handle {clientSession.ClientHandle}")
                            .Append($"\tInstance Name {m_ommServerBaseImpl.InstanceName}");

                        m_ommServerBaseImpl.GetLoggerClient().Trace(CLIENT_NAME, temp.ToString());
                    }

                    ItemInfo? itemInfo = clientSession.GetItemInfo(dictEvent.Msg!.StreamId);

                    if (itemInfo is null)
                    {
                        itemInfo = m_ommServerBaseImpl.ServerPool.GetItemInfo();
                        itemInfo.ClientSession = clientSession;
                        itemInfo.RequestMsg((IRequestMsg)dictEvent.Msg);

                        clientSession.AddItemInfo(itemInfo);
                        m_ommServerBaseImpl.AddItemInfo(itemInfo);
                        m_ItemInfoList.Add(itemInfo);

                        if (!m_ApiAdminControl)
                        {
                            // USER_CONTROL
                            RequestMsg reqMsg = m_ommServerBaseImpl.RequestMsg();

                            int flags = dictEvent.Msg.MsgKey.Flags;

                            if ((flags & MsgKeyFlags.HAS_SERVICE_ID) == MsgKeyFlags.HAS_SERVICE_ID)
                            {
                                reqMsg.Decode(dictEvent.Msg, rsslReactorChannel.MajorVersion,
                                    rsslReactorChannel.MinorVersion, GetDictionaryByServiceId(dictEvent.Msg.MsgKey.ServiceId));

                                bool gotServiceName = m_ommServerBaseImpl.GetDirectoryServiceStore()
                                    .GetServiceNameById(dictEvent.Msg.MsgKey.ServiceId, out string? serviceName);

                                if (gotServiceName && serviceName is not null)
                                {
                                    flags &= ~MsgKeyFlags.HAS_SERVICE_ID;

                                    reqMsg.m_rsslMsg.MsgKey.Flags = flags;
                                    reqMsg.ServiceName(serviceName);
                                    reqMsg.m_rsslMsg.MsgKey.Flags = flags | MsgKeyFlags.HAS_SERVICE_ID;
                                }
                                else
                                {
                                    SendRequestReject(dictEvent.ReactorChannel, dictionaryMsg,
                                        DictionaryRejectEnum.SERVICE_ID_NOT_FOUND, true);

                                    m_ItemInfoList.Remove(itemInfo);
                                    m_ommServerBaseImpl.RemoveItemInfo(itemInfo, false);

                                    return ReactorCallbackReturnCode.SUCCESS;
                                }

                                m_ommServerBaseImpl.OmmProviderEvent.clientHandle = clientSession.ClientHandle;
                                m_ommServerBaseImpl.OmmProviderEvent.SetClosure(m_ommServerBaseImpl.Closure);
                                m_ommServerBaseImpl.OmmProviderEvent.SetOmmProvider(m_ommServerBaseImpl.Provider);
                                m_ommServerBaseImpl.OmmProviderEvent.SetHandle(itemInfo.Handle);
                                m_ommServerBaseImpl.OmmProviderEvent.ReactorChannel = dictEvent.ReactorChannel;

                                m_ommServerBaseImpl.OmmProviderClient.OnAllMsg(reqMsg, m_ommServerBaseImpl.OmmProviderEvent);
                                m_ommServerBaseImpl.OmmProviderClient.OnReqMsg(reqMsg, m_ommServerBaseImpl.OmmProviderEvent);
                            }
                            else
                            {
                                reqMsg.Decode(dictEvent.Msg, rsslReactorChannel.MajorVersion, rsslReactorChannel.MinorVersion, null);
                            }
                        }
                        else
                        {
                            // API_CONTROL
                            if (!SendDictionaryResponse(rsslReactorChannel, dictionaryMsg))
                            {
                                m_ItemInfoList.Remove(itemInfo);
                                m_ommServerBaseImpl.RemoveItemInfo(itemInfo, false);
                            }
                        }
                    }
                    else
                    {
                        itemInfo.RequestMsg((IRequestMsg)dictEvent.Msg);

                        if (!m_ApiAdminControl)
                        {
                            // USER_CONTROL
                            RequestMsg reqMsg = m_ommServerBaseImpl.RequestMsg();

                            int flags = dictEvent.Msg.MsgKey.Flags;

                            if ((flags & MsgKeyFlags.HAS_SERVICE_ID) == MsgKeyFlags.HAS_SERVICE_ID)
                            {
                                reqMsg.Decode(dictEvent.Msg, rsslReactorChannel.MajorVersion,
                                    rsslReactorChannel.MinorVersion, GetDictionaryByServiceId(dictEvent.Msg.MsgKey.ServiceId));

                                bool gotServiceName = m_ommServerBaseImpl.GetDirectoryServiceStore()
                                    .GetServiceNameById(dictEvent.Msg.MsgKey.ServiceId, out string? serviceName);

                                if (gotServiceName && serviceName is not null)
                                {
                                    flags &= ~MsgKeyFlags.HAS_SERVICE_ID;

                                    reqMsg.m_rsslMsg.MsgKey.Flags = flags;
                                    reqMsg.ServiceName(serviceName);
                                    reqMsg.m_rsslMsg.MsgKey.Flags = flags | MsgKeyFlags.HAS_SERVICE_ID;
                                }
                                else
                                {
                                    SendRequestReject(dictEvent.ReactorChannel, dictionaryMsg,
                                        DictionaryRejectEnum.SERVICE_ID_NOT_FOUND, true);

                                    m_ItemInfoList.Remove(itemInfo);
                                    m_ommServerBaseImpl.RemoveItemInfo(itemInfo, false);

                                    return ReactorCallbackReturnCode.SUCCESS;
                                }
                            }

                            m_ommServerBaseImpl.OmmProviderEvent.clientHandle = clientSession.ClientHandle;
                            m_ommServerBaseImpl.OmmProviderEvent.SetClosure(m_ommServerBaseImpl.Closure);
                            m_ommServerBaseImpl.OmmProviderEvent.SetOmmProvider(m_ommServerBaseImpl.Provider);
                            m_ommServerBaseImpl.OmmProviderEvent.SetHandle(itemInfo.Handle);
                            m_ommServerBaseImpl.OmmProviderEvent.ReactorChannel = dictEvent.ReactorChannel;

                            m_ommServerBaseImpl.OmmProviderClient.OnAllMsg(reqMsg, m_ommServerBaseImpl.OmmProviderEvent);
                            m_ommServerBaseImpl.OmmProviderClient.OnReissue(reqMsg, m_ommServerBaseImpl.OmmProviderEvent);
                        }
                        else
                        {
                            // API_CONTROL
                            if (!SendDictionaryResponse(rsslReactorChannel, dictionaryMsg))
                            {
                                m_ItemInfoList.Remove(itemInfo);
                                m_ommServerBaseImpl.RemoveItemInfo(itemInfo, false);
                            }
                        }
                    }

                    break;
                }

            case DictionaryMsgType.CLOSE:
                {
                    if (m_ommServerBaseImpl.GetLoggerClient().IsTraceEnabled)
                    {
                        using var lockScope = m_ommServerBaseImpl.GetUserLocker().EnterLockScope();
                        StringBuilder temp = m_ommServerBaseImpl.GetStrBuilder()
                            .AppendLine("Received dictionary close message.")
                            .AppendLine($"Stream Id {dictionaryMsg.StreamId}")
                            .AppendLine($"Client handle {clientSession.ClientHandle}")
                            .Append("Instance Name ").Append(m_ommServerBaseImpl.InstanceName);

                        m_ommServerBaseImpl.GetLoggerClient().Trace(CLIENT_NAME, temp.ToString());
                    }

                    ItemInfo? itemInfo = clientSession.GetItemInfo(dictEvent.Msg!.StreamId);

                    if (itemInfo is not null)
                    {
                        if (!m_ApiAdminControl)
                        {
                            // USER_CONTROL
                            RequestMsg rsslReqMsg = m_ommServerBaseImpl.RequestMsg();

                            rsslReqMsg.m_rsslMsg.ApplyNoRefresh();
                            rsslReqMsg.StreamId(dictEvent.Msg.StreamId);

                            if (itemInfo.MsgKey.CheckHasName())
                            {
                                rsslReqMsg.m_rsslMsg.MsgKey.ApplyHasName();
                                rsslReqMsg.m_rsslMsg.MsgKey.Name = itemInfo.MsgKey.Name;
                            }

                            if (itemInfo.MsgKey.CheckHasNameType())
                            {
                                rsslReqMsg.m_rsslMsg.MsgKey.ApplyHasNameType();
                                rsslReqMsg.m_rsslMsg.MsgKey.NameType = itemInfo.MsgKey.NameType;
                            }

                            rsslReqMsg.DomainType(dictEvent.Msg.DomainType);

                            RequestMsg reqMsg = m_ommServerBaseImpl.RequestMsg();

                            if (itemInfo.MsgKey.CheckHasServiceId())
                            {
                                rsslReqMsg.m_rsslMsg.MsgKey.ApplyHasServiceId();
                                rsslReqMsg.m_rsslMsg.MsgKey.ServiceId = itemInfo.MsgKey.ServiceId;

                                reqMsg.Decode(rsslReqMsg.m_rsslMsg, rsslReactorChannel.MajorVersion, rsslReactorChannel.MinorVersion, null);

                                m_ommServerBaseImpl.GetDirectoryServiceStore().GetServiceNameById(itemInfo.MsgKey.ServiceId, out string? serviceName);

                                int flags = reqMsg.m_rsslMsg.MsgKey.Flags;

                                if (serviceName is not null)
                                {
                                    flags &= ~MsgKeyFlags.HAS_SERVICE_ID;

                                    reqMsg.m_rsslMsg.MsgKey.Flags = flags;
                                    reqMsg.ServiceName(serviceName);
                                    reqMsg.m_rsslMsg.MsgKey.Flags = flags | MsgKeyFlags.HAS_SERVICE_ID;
                                }
                            }
                            else
                            {
                                reqMsg.Decode(rsslReqMsg.m_rsslMsg, rsslReactorChannel.MajorVersion, rsslReactorChannel.MinorVersion, null);
                            }

                            int noStreamingFlags = reqMsg.m_rsslMsg.Flags;
                            noStreamingFlags &= ~RequestMsgFlags.STREAMING;
                            rsslReqMsg.m_rsslMsg.Flags = noStreamingFlags;

                            m_ommServerBaseImpl.OmmProviderEvent.clientHandle = clientSession.ClientHandle;
                            m_ommServerBaseImpl.OmmProviderEvent.SetClosure(m_ommServerBaseImpl.Closure);
                            m_ommServerBaseImpl.OmmProviderEvent.SetOmmProvider(m_ommServerBaseImpl.Provider);
                            m_ommServerBaseImpl.OmmProviderEvent.SetHandle(itemInfo.Handle);
                            m_ommServerBaseImpl.OmmProviderEvent.ReactorChannel = dictEvent.ReactorChannel;

                            m_ommServerBaseImpl.OmmProviderClient.OnAllMsg(reqMsg, m_ommServerBaseImpl.OmmProviderEvent);
                            m_ommServerBaseImpl.OmmProviderClient.OnClose(reqMsg, m_ommServerBaseImpl.OmmProviderEvent);
                        }

                        m_ItemInfoList.Remove(itemInfo);
                        m_ommServerBaseImpl.RemoveItemInfo(itemInfo, false);
                    }

                    break;
                }

            case DictionaryMsgType.REFRESH:
                {
                    if (m_ommServerBaseImpl.GetLoggerClient().IsTraceEnabled)
                    {
                        using var lockScope = m_ommServerBaseImpl.GetUserLocker().EnterLockScope();
                        StringBuilder temp = m_ommServerBaseImpl.GetStrBuilder()
                            .AppendLine("Received refresh message.")
                            .AppendLine($"Stream Id {dictEvent.Msg!.StreamId}")
                            .AppendLine($"Client handle {clientSession.ClientHandle}")
                            .Append("Instance Name ").Append(m_ommServerBaseImpl.InstanceName);

                        m_ommServerBaseImpl.GetLoggerClient().Trace(CLIENT_NAME, temp.ToString());
                    }

                    DataDictionary? dataDictionary = null;

                    if ((dictEvent.Msg!.Flags & RefreshMsgFlags.HAS_MSG_KEY) != 0)
                    {
                        if (dictEvent.Msg.MsgKey.CheckHasServiceId())
                        {
                            dataDictionary = m_ommServerBaseImpl.DictionaryHandler.GetDictionaryByServiceId(dictEvent.Msg.MsgKey.ServiceId);
                        }
                    }

                    m_ommServerBaseImpl.ItemCallbackClient.ProcessIProviderMsgCallback(dictEvent, dataDictionary);

                    break;
                }

            case DictionaryMsgType.STATUS:
                {
                    if (m_ommServerBaseImpl.GetLoggerClient().IsTraceEnabled)
                    {
                        using var lockScope = m_ommServerBaseImpl.GetUserLocker().EnterLockScope();
                        StringBuilder temp = m_ommServerBaseImpl.GetStrBuilder()
                            .AppendLine("Received status message.")
                            .AppendLine($"Stream Id {dictEvent.Msg!.StreamId}")
                            .AppendLine($"Client handle {clientSession.ClientHandle}")
                            .Append("Instance Name ").Append(m_ommServerBaseImpl.InstanceName);

                        m_ommServerBaseImpl.GetLoggerClient().Trace(CLIENT_NAME, temp.ToString());
                    }

                    DataDictionary? dataDictionary = null;

                    if ((dictEvent.Msg!.Flags & StatusMsgFlags.HAS_MSG_KEY) != 0)
                    {
                        if (dictEvent.Msg.MsgKey.CheckHasServiceId())
                        {
                            dataDictionary = m_ommServerBaseImpl.DictionaryHandler.GetDictionaryByServiceId(dictEvent.Msg.MsgKey.ServiceId);
                        }
                    }

                    m_ommServerBaseImpl.ItemCallbackClient.ProcessIProviderMsgCallback(dictEvent, dataDictionary);

                    break;
                }

            default:
                {
                    using var lockScope = m_ommServerBaseImpl.GetUserLocker().EnterLockScope();
                    StringBuilder temp = m_ommServerBaseImpl.GetStrBuilder()
                        .Append("Rejected unhandled dictionary message type ").Append(dictionaryMsg.DictionaryMsgType.ToString());

                    ItemInfo? itemInfo = clientSession.GetItemInfo(dictEvent.Msg!.StreamId);

                    if (itemInfo is null)
                    {
                        SendRequestReject(rsslReactorChannel, dictEvent.Msg.StreamId, 0,
                            null, DictionaryRejectEnum.DICTIONARY_UNHANDLED_MESSAGE, false);
                    }

                    if (m_ommServerBaseImpl.GetLoggerClient().IsTraceEnabled)
                    {
                        temp.AppendLine()
                            .AppendLine($"Stream Id {dictEvent.Msg.StreamId}")
                            .AppendLine($"Client handle {clientSession.ClientHandle}")
                            .Append("Instance Name ").Append(m_ommServerBaseImpl.InstanceName);

                        m_ommServerBaseImpl.GetLoggerClient().Trace(CLIENT_NAME, temp.ToString());
                    }
                    break;
                }
        }

        return ReactorCallbackReturnCode.SUCCESS;
    }

    private bool SendDictionaryResponse(ReactorChannel reactorChannel, DictionaryMsg dictionaryRequest)
    {
        m_DictionaryNameAndServiceId.Clear();
        m_DictionaryNameAndServiceId
            .Append(dictionaryRequest.DictionaryRequest!.DictionaryName.ToString())
            .Append(dictionaryRequest.DictionaryRequest.ServiceId);

        if (!m_DictionaryInfoHash.TryGetValue(m_DictionaryNameAndServiceId.ToString(), out var dictionaryPayload))
        {
            SendRequestReject(reactorChannel, dictionaryRequest, DictionaryRejectEnum.DICTIONARY_NAME_NOT_FOUND, true);
            return false;
        }
        else
        {
            switch (dictionaryPayload.DataDictionaryType)
            {
                case DictionaryPayload.DictionaryType.FIELD_DICTIONARY:
                    if (SendFieldDictionaryResponse(reactorChannel, dictionaryRequest, dictionaryPayload.Dictionary)
                        != CodecReturnCode.SUCCESS)
                    {
                        SendRequestReject(reactorChannel, dictionaryRequest, DictionaryRejectEnum.DICTIONARY_ENCODING_FAILED, true);
                        return false;
                    }
                    break;
                case DictionaryPayload.DictionaryType.ENUM_TYPE:
                    if (SendEnumTypeDictionaryResponse(reactorChannel, dictionaryRequest, dictionaryPayload.Dictionary)
                        != CodecReturnCode.SUCCESS)
                    {
                        SendRequestReject(reactorChannel, dictionaryRequest, DictionaryRejectEnum.DICTIONARY_ENCODING_FAILED, true);
                        return false;
                    }
                    break;
            }
            return true;
        }
    }

    private CodecReturnCode SendFieldDictionaryResponse(ReactorChannel reactorChannel, DictionaryMsg dictionaryRequest, DataDictionary dataDictionary)
    {
        ClientSession clientSession = (ClientSession)reactorChannel.UserSpecObj!;

        m_DictionaryRefresh.Clear();
        m_DictionaryRefresh.StreamId = dictionaryRequest.StreamId;
        m_DictionaryRefresh.DictionaryType = Eta.Rdm.Dictionary.Types.FIELD_DEFINITIONS;
        m_DictionaryRefresh.DataDictionary = dataDictionary;
        m_DictionaryRefresh.Verbosity = dictionaryRequest.DictionaryRequest!.Verbosity;
        m_DictionaryRefresh.ServiceId = dictionaryRequest.DictionaryRequest!.ServiceId;
        m_DictionaryRefresh.DictionaryName = dictionaryRequest.DictionaryRequest.DictionaryName;
        m_DictionaryRefresh.Solicited = true;

        m_DictionaryRefresh.State.StreamState(StreamStates.OPEN);
        m_DictionaryRefresh.State.DataState(DataStates.OK);
        m_DictionaryRefresh.State.Code(StateCodes.NONE);

        bool firstPartMultiPartRefresh = true;
        DictionaryRefreshFlags flags = m_DictionaryRefresh.Flags;

        while (true)
        {
            if (firstPartMultiPartRefresh)
            {
                m_DictionaryRefresh.ClearCache = true;
                firstPartMultiPartRefresh = false;
                m_DictionaryRefresh.StartFid = dataDictionary.MinFid;
            }
            else
            {
                m_DictionaryRefresh.Flags = flags;
            }

            ITransportBuffer? msgBuf = reactorChannel.GetBuffer(m_MaxFieldDictFragmentSize, false, out var getBufferError);

            if (msgBuf is null)
            {
                if (m_ommServerBaseImpl.GetLoggerClient().IsErrorEnabled)
                {
                    StringBuilder temp = m_ommServerBaseImpl.GetStrBuilder();
                    temp.AppendLine("Internal error. Failed to get bufffer in DictionaryHandler.SendFieldDictionaryResponse()")
                        .AppendLine($"Client handle {clientSession.ClientHandle}")
                        .AppendLine($"Instance Name {m_ommServerBaseImpl.InstanceName}")
                        .AppendLine($"Error Id {getBufferError!.Error.ErrorId}")
                        .AppendLine($"Internal SysError {getBufferError.Error.SysError}")
                        .AppendLine($"Error Location {getBufferError.Location}")
                        .Append("Error Text: ").Append(getBufferError.Error.Text);

                    m_ommServerBaseImpl.GetLoggerClient().Error(CLIENT_NAME, temp.ToString());
                }

                return CodecReturnCode.FAILURE;
            }

            m_DictionaryRefresh.State.Text().Data($"Field Dictionary Refresh (starting fid {m_DictionaryRefresh.StartFid})");

            msgBuf.Data.Limit = m_MaxFieldDictFragmentSize;

            m_EncodeIter.Clear();
            CodecReturnCode ret = m_EncodeIter.SetBufferAndRWFVersion(msgBuf, reactorChannel.MajorVersion, reactorChannel.MinorVersion);
            if (ret != CodecReturnCode.SUCCESS)
            {
                if (m_ommServerBaseImpl.GetLoggerClient().IsErrorEnabled)
                {
                    StringBuilder temp = m_ommServerBaseImpl.GetStrBuilder();
                    temp.AppendLine("Internal error. Failed to set encode iterator in DictionaryHandler.SendFieldDictionaryResponse()")
                        .AppendLine($"Client handle {clientSession.ClientHandle}")
                        .Append("Instance Name ").Append(m_ommServerBaseImpl.InstanceName);

                    m_ommServerBaseImpl.GetLoggerClient().Error(CLIENT_NAME, temp.ToString());
                }

                return CodecReturnCode.FAILURE;
            }

            ret = m_DictionaryRefresh.Encode(m_EncodeIter);
            if (ret < CodecReturnCode.SUCCESS)
            {
                if (m_ommServerBaseImpl.GetLoggerClient().IsErrorEnabled)
                {
                    StringBuilder temp = m_ommServerBaseImpl.GetStrBuilder();
                    temp.AppendLine("Internal error. Failed to encode message in DictionaryHandler.SendFieldDictionaryResponse()")
                        .AppendLine($"Client handle {clientSession.ClientHandle}")
                        .Append("Instance Name ").Append(m_ommServerBaseImpl.InstanceName);

                    m_ommServerBaseImpl.GetLoggerClient().Error(CLIENT_NAME, temp.ToString());
                }

                return CodecReturnCode.FAILURE;
            }

            ReactorReturnCode retCode = reactorChannel.Submit(msgBuf, m_ommServerBaseImpl.GetSubmitOptions(), out var submitError);
            if (retCode < ReactorReturnCode.SUCCESS)
            {
                if (m_ommServerBaseImpl.GetLoggerClient().IsErrorEnabled)
                {
                    StringBuilder temp = m_ommServerBaseImpl.GetStrBuilder();
                    temp.AppendLine("Internal error. Failure to submit dictionary message in DictionaryHandler.SendFieldDictionaryResponse().")
                        .AppendLine($"Client handle {clientSession.ClientHandle}")
                        .AppendLine($"Instance Name {m_ommServerBaseImpl.InstanceName}")
                        .AppendLine($"Error Id {submitError!.Error.ErrorId}")
                        .AppendLine($"Internal sysError {submitError.Error.SysError}")
                        .AppendLine($"Error Location {submitError.Location}")
                        .Append("Error Text: ").Append(submitError.Error.Text);

                    m_ommServerBaseImpl.GetLoggerClient().Error(CLIENT_NAME, temp.ToString());
                }

                return CodecReturnCode.FAILURE;
            }

            if (ret == CodecReturnCode.SUCCESS)
            {
                break;
            }
        }

        if (m_ommServerBaseImpl.GetLoggerClient().IsTraceEnabled)
        {
            using var lockScope = m_ommServerBaseImpl.GetUserLocker().EnterLockScope();
            StringBuilder temp = m_ommServerBaseImpl.GetStrBuilder()
                .AppendLine("Successfully sent field dictionary type.")
                .AppendLine($"Dictionary name {dictionaryRequest.DictionaryRequest.DictionaryName}")
                .AppendLine($"Stream Id {dictionaryRequest.StreamId}")
                .Append("Client handle ").Append(clientSession.ClientHandle);

            m_ommServerBaseImpl.GetLoggerClient().Trace(CLIENT_NAME, temp.ToString());
        }

        return CodecReturnCode.SUCCESS;
    }

    private CodecReturnCode SendEnumTypeDictionaryResponse(ReactorChannel reactorChannel, DictionaryMsg dictionaryRequest, DataDictionary dataDictionary)
    {
        ClientSession clientSession = (ClientSession)reactorChannel.UserSpecObj!;

        m_DictionaryRefresh.Clear();

        m_DictionaryRefresh.StreamId = dictionaryRequest.StreamId;
        m_DictionaryRefresh.DictionaryType = Eta.Rdm.Dictionary.Types.ENUM_TABLES;
        m_DictionaryRefresh.DataDictionary = dataDictionary;
        m_DictionaryRefresh.ServiceId = dictionaryRequest.DictionaryRequest!.ServiceId;
        m_DictionaryRefresh.Verbosity = dictionaryRequest.DictionaryRequest.Verbosity;
        m_DictionaryRefresh.DictionaryName = dictionaryRequest.DictionaryRequest.DictionaryName;
        m_DictionaryRefresh.Solicited = true;

        m_DictionaryRefresh.State.StreamState(StreamStates.OPEN);
        m_DictionaryRefresh.State.DataState(DataStates.OK);
        m_DictionaryRefresh.State.Code(StateCodes.NONE);

        bool firstPartMultiPartRefresh = true;
        DictionaryRefreshFlags flags = m_DictionaryRefresh.Flags;

        while (true)
        {
            if (firstPartMultiPartRefresh)
            {
                m_DictionaryRefresh.ClearCache = true;
                firstPartMultiPartRefresh = false;
            }
            else
            {
                m_DictionaryRefresh.Flags = flags;
            }

            ITransportBuffer? msgBuf = reactorChannel.GetBuffer(m_MaxEnumTypeFragmentSize, false, out var bufferError);
            if (msgBuf is null)
            {
                if (m_ommServerBaseImpl.GetLoggerClient().IsErrorEnabled)
                {
                    using var lockScope = m_ommServerBaseImpl.GetUserLocker().EnterLockScope();
                    StringBuilder temp = m_ommServerBaseImpl.GetStrBuilder()
                        .AppendLine("Internal error. Failed to get buffer in DictionaryHandler.SendEnumTypeDictionaryResponse()")
                        .AppendLine($"Client handle {clientSession.ClientHandle}")
                        .AppendLine($"Instance Name {m_ommServerBaseImpl.InstanceName}")
                        .AppendLine($"Error Id {bufferError!.Error.ErrorId}")
                        .AppendLine($"Internal SysError {bufferError.Error.SysError}")
                        .AppendLine("Error Location {bufferError.Location}")
                        .Append("Error Text: ").Append(bufferError.Error.Text);

                    m_ommServerBaseImpl.GetLoggerClient().Error(CLIENT_NAME, temp.ToString());
                }

                return CodecReturnCode.FAILURE;
            }

            msgBuf.Data.Limit = m_MaxEnumTypeFragmentSize;

            m_EncodeIter.Clear();
            CodecReturnCode ret = m_EncodeIter.SetBufferAndRWFVersion(msgBuf, reactorChannel.MajorVersion, reactorChannel.MinorVersion);
            if (ret < CodecReturnCode.SUCCESS)
            {
                if (m_ommServerBaseImpl.GetLoggerClient().IsErrorEnabled)
                {
                    using var lockScope = m_ommServerBaseImpl.GetUserLocker().EnterLockScope();
                    StringBuilder temp = m_ommServerBaseImpl.GetStrBuilder()
                        .AppendLine("Internal error. Failed to set encode iterator in DictionaryHandler.SendEnumTypeDictionaryResponse()")
                        .AppendLine($"Client handle {clientSession.ClientHandle}")
                        .Append("Instance Name ").Append(m_ommServerBaseImpl.InstanceName);

                    m_ommServerBaseImpl.GetLoggerClient().Error(CLIENT_NAME, temp.ToString());
                }

                return CodecReturnCode.FAILURE;
            }

            m_DictionaryRefresh.State.Text()
                .Data($"Enum Type Dictionary Refresh (starting enum table count {m_DictionaryRefresh.StartEnumTableCount})");

            ret = m_DictionaryRefresh.Encode(m_EncodeIter);
            if (ret < CodecReturnCode.SUCCESS)
            {
                if (m_ommServerBaseImpl.GetLoggerClient().IsErrorEnabled)
                {
                    using var lockScope = m_ommServerBaseImpl.GetUserLocker().EnterLockScope();
                    StringBuilder temp = m_ommServerBaseImpl.GetStrBuilder()
                        .AppendLine("Internal error. Failed to encode message in DictionaryHandler.SendEnumTypeDictionaryResponse()")
                        .AppendLine($"Client handle {clientSession.ClientHandle}")
                        .Append("Instance Name ").Append(m_ommServerBaseImpl.InstanceName);

                    m_ommServerBaseImpl.GetLoggerClient().Error(CLIENT_NAME, temp.ToString());
                }

                return CodecReturnCode.FAILURE;
            }

            if (reactorChannel.Submit(msgBuf, m_ommServerBaseImpl.GetSubmitOptions(), out var submitError) < ReactorReturnCode.SUCCESS)
            {
                if (m_ommServerBaseImpl.GetLoggerClient().IsErrorEnabled)
                {
                    using var lockScope = m_ommServerBaseImpl.GetUserLocker().EnterLockScope();
                    StringBuilder temp = m_ommServerBaseImpl.GetStrBuilder()
                        .AppendLine("Failed to submit dictionary message in DictionaryHandler.SendEnumTypeDictionaryResponse().")
                        .AppendLine($"Error Id {submitError!.Error.ErrorId}")
                        .AppendLine($"Internal SysError {submitError.Error.SysError}")
                        .AppendLine($"Error Location {submitError.Location}")
                        .Append("Error Text: ").Append(submitError.Error.Text);

                    m_ommServerBaseImpl.GetLoggerClient().Error(CLIENT_NAME, temp.ToString());
                }

                return CodecReturnCode.FAILURE;
            }

            if (ret == CodecReturnCode.SUCCESS)
            {
                break;
            }
        }

        if (m_ommServerBaseImpl.GetLoggerClient().IsTraceEnabled)
        {
            using var lockScope = m_ommServerBaseImpl.GetUserLocker().EnterLockScope();
            StringBuilder temp = m_ommServerBaseImpl.GetStrBuilder()
                .AppendLine("Successfully sent enumeration dictionary type.")
                .AppendLine($"Dictionary name {dictionaryRequest.DictionaryRequest.DictionaryName}")
                .AppendLine($"Stream Id {dictionaryRequest.StreamId}")
                .Append("Client handle ").Append(clientSession.ClientHandle);

            m_ommServerBaseImpl.GetLoggerClient().Trace(CLIENT_NAME, temp.ToString());
        }

        return CodecReturnCode.SUCCESS;
    }

    private ReactorReturnCode SendRequestReject(ReactorChannel reactorChannel, DictionaryMsg dictionaryRequest,
        DictionaryRejectEnum reason, bool traceMessage)
    {
        return SendRequestReject(reactorChannel,
            dictionaryRequest.StreamId, dictionaryRequest.DictionaryRequest!.ServiceId, dictionaryRequest.DictionaryRequest!.DictionaryName,
            reason, traceMessage);
    }

    private ReactorReturnCode SendRequestReject(ReactorChannel reactorChannel, int streamId, int serviceId, Eta.Codec.Buffer? dictionaryName,
        DictionaryRejectEnum reason, bool traceMessage)
    {
        int bufferSize = INIT_DICTIONARY_STATUS_MSG_SIZE;

        if (reason == DictionaryRejectEnum.DICTIONARY_NAME_NOT_FOUND)
        {
            bufferSize += dictionaryName?.Length ?? 0;
        }

        ITransportBuffer? msgBuf = reactorChannel.GetBuffer(bufferSize, false, out var bufferError);

        if (msgBuf is not null)
        {
            CodecReturnCode ret = EncodeDictionaryRequestReject(reactorChannel,
                streamId, serviceId, dictionaryName,
                reason, msgBuf, traceMessage);

            if (ret != CodecReturnCode.SUCCESS)
            {
                return ReactorReturnCode.FAILURE;
            }

            return reactorChannel.Submit(msgBuf, m_ommServerBaseImpl.GetSubmitOptions(), out _);
        }
        else
        {
            if (m_ommServerBaseImpl.GetLoggerClient().IsErrorEnabled)
            {
                using var lockScope = m_ommServerBaseImpl.GetUserLocker().EnterLockScope();
                StringBuilder temp = m_ommServerBaseImpl.GetStrBuilder()
                    .AppendLine("Internal error. Failed to get buffer in DictionaryHandler.SendRequestReject()")
                    .AppendLine($"Error Id {bufferError!.Error.ErrorId}")
                    .AppendLine($"Internal SysError {bufferError.Error.SysError}")
                    .AppendLine($"Error Location {bufferError.Location}")
                    .Append("Error Text: ").Append(bufferError.Error.Text);

                m_ommServerBaseImpl.GetLoggerClient().Error(CLIENT_NAME, temp.ToString());
            }

            return ReactorReturnCode.FAILURE;
        }
    }

    private CodecReturnCode EncodeDictionaryRequestReject(ReactorChannel reactorChannel,
        int streamId, int serviceId, Eta.Codec.Buffer? dictionaryName,
        DictionaryRejectEnum reason, ITransportBuffer msgBuf, bool traceMessage)
    {
        ClientSession clientSession = (ClientSession)reactorChannel.UserSpecObj!;

        m_EncodeIter.Clear();
        m_DictionaryStatus.Clear();

        m_DictionaryStatus.StreamId = streamId;
        m_DictionaryStatus.HasState = true;
        m_DictionaryStatus.State.DataState(DataStates.SUSPECT);
        m_DictionaryStatus.State.Code(StateCodes.ERROR);
        m_DictionaryStatus.State.StreamState(StreamStates.CLOSED_RECOVER);

        switch (reason)
        {
            case DictionaryRejectEnum.DICTIONARY_INVALID_MESSAGE:
                m_DictionaryStatus.State.Text().Data("Dictionary message rejected - invalid dictionary domain message.");
                break;

            case DictionaryRejectEnum.DICTIONARY_NOT_LOADED:
                m_DictionaryStatus.State.Text().Data("Dictionary request message rejected - dictionary is not loaded in provider.");
                break;

            case DictionaryRejectEnum.DICTIONARY_ENCODING_FAILED:
                m_DictionaryStatus.State.Text().Data("Dictionary request message rejected - failed to encode dictionary information.");
                break;

            case DictionaryRejectEnum.USER_IS_NOT_LOGGED_IN:
                m_DictionaryStatus.State.Text().Data("Dictionary message rejected - there is no logged in user for this session.");
                break;

            case DictionaryRejectEnum.DICTIONARY_UNHANDLED_MESSAGE:
                m_DictionaryStatus.State.Text().Data("Dictionary message rejected - unhandled dictionary message type.");
                break;

            case DictionaryRejectEnum.DICTIONARY_NAME_NOT_FOUND:
                {
                    using var lockScope = m_ommServerBaseImpl.GetUserLocker().EnterLockScope();
                    StringBuilder text = m_ommServerBaseImpl.GetStrBuilder()
                        .Append("Dictionary request message rejected - the reqesting dictionary name '")
                        .Append(dictionaryName).Append("' not found.");
                    m_DictionaryStatus.State.Text().Data(text.ToString());
                }
                break;

            case DictionaryRejectEnum.SERVICE_ID_NOT_FOUND:
                {
                    using var lockScope = m_ommServerBaseImpl.GetUserLocker().EnterLockScope();
                    StringBuilder text = m_ommServerBaseImpl.GetStrBuilder()
                        .Append("Dictionary request message rejected - the service Id = ")
                        .Append(serviceId)
                        .Append("  does not exist in the source directory");
                    m_DictionaryStatus.State.Text().Data(text.ToString());
                }
                break;

            default:
                return CodecReturnCode.FAILURE;
        }

        if (traceMessage && m_ommServerBaseImpl.GetLoggerClient().IsTraceEnabled)
        {
            using var lockScope = m_ommServerBaseImpl.GetUserLocker().EnterLockScope();
            StringBuilder text = m_ommServerBaseImpl.GetStrBuilder()
                .AppendLine(m_DictionaryStatus.State.Text().ToString())
                .AppendLine($"Stream Id {streamId}")
                .AppendLine($"Client handle {clientSession.ClientHandle}")
                .Append("Instance Name ").Append(m_ommServerBaseImpl.InstanceName);

            m_ommServerBaseImpl.GetLoggerClient().Trace(CLIENT_NAME, text.ToString());
        }

        CodecReturnCode ret = m_EncodeIter.SetBufferAndRWFVersion(msgBuf, reactorChannel.MajorVersion, reactorChannel.MinorVersion);
        if (ret != CodecReturnCode.SUCCESS)
        {
            if (m_ommServerBaseImpl.GetLoggerClient().IsErrorEnabled)
            {
                using var lockScope = m_ommServerBaseImpl.GetUserLocker().EnterLockScope();
                StringBuilder temp = m_ommServerBaseImpl.GetStrBuilder()
                    .AppendLine("Internal error. Failed to set encode iterator in DictionaryHandler.EncodeDictionaryRequestReject()")
                    .AppendLine($"Client handle {clientSession.ClientHandle}")
                    .Append("Instance Name ").Append(m_ommServerBaseImpl.InstanceName);

                m_ommServerBaseImpl.GetLoggerClient().Error(CLIENT_NAME, temp.ToString());
            }

            return ret;
        }

        ret = m_DictionaryStatus.Encode(m_EncodeIter);
        if (ret != CodecReturnCode.SUCCESS)
        {
            if (m_ommServerBaseImpl.GetLoggerClient().IsErrorEnabled)
            {
                using var lockScope = m_ommServerBaseImpl.GetUserLocker().EnterLockScope();
                StringBuilder temp = m_ommServerBaseImpl.GetStrBuilder()
                    .AppendLine("Internal error. Failed to encode status message in DictionaryHandler.EncodeDictionaryRequestReject()")
                    .AppendLine($"Client handle {clientSession.ClientHandle}")
                    .Append("Instance Name ").Append(m_ommServerBaseImpl.InstanceName);

                m_ommServerBaseImpl.GetLoggerClient().Error(CLIENT_NAME, temp.ToString());
            }
            return ret;
        }

        return CodecReturnCode.SUCCESS;
    }
}
