/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023-2024 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Collections.Generic;
using System.Text;
using System.Threading;

using LSEG.Eta.Codec;
using LSEG.Eta.ValueAdd.Rdm;
using LSEG.Eta.ValueAdd.Reactor;

namespace LSEG.Ema.Access;

/// <summary>
/// Implements Dictionary domain handling and management.
/// </summary>
/// <typeparam name="T">Application interface/type to receive callbacks.</typeparam>
/// <seealso cref="IOmmConsumerClient"/>
internal class DictionaryCallbackClient<T> : CallbackClient<T>, IDictionaryMsgCallback
{
    protected const string CLIENT_NAME = "DictionaryCallbackClient";
    private const int MAX_DICTIONARY_BUFFER_SIZE = 448000;
    internal const string DICTIONARY_RWFFID = "RWFFld";
    internal const string DICTIONARY_RWFENUM = "RWFEnum";

    internal const int FIELD_DICTIONARY_STREAM_ID = 3;
    internal const int ENUM_TYPE_DICTIONARY_STREAM_ID = 4;

    private List<ChannelDictionary<T>>? m_ChannelDictionaryList;
    private List<ChannelDictionary<T>>? m_ChannelDictPool;
    private Eta.Codec.DataDictionary? m_rsslLocalDictionary;
    private ChannelDictionary<T>? m_ChannelDictionary;
    private Eta.Codec.Buffer? m_rsslEncBuffer;
    private Eta.Codec.Int? m_rsslCurrentFid;
    protected OmmBaseImpl<T> m_OmmBaseImpl;

    internal DictionaryCallbackClient(OmmBaseImpl<T> baseImpl) : base(baseImpl, "")
    {
        m_OmmBaseImpl = baseImpl;
    }

    internal void Initialize()
    {
        if (m_OmmBaseImpl.ConfigImpl.AdminFieldDictionaryRequest != null
            && m_OmmBaseImpl.ConfigImpl.AdminEnumDictionaryRequest != null)
        {
            m_OmmBaseImpl.ConfigImpl.DictionaryConfig.IsLocalDictionary = false;
        }
        else if (m_OmmBaseImpl.ConfigImpl.DictionaryConfig.DataDictionary is null
            && m_OmmBaseImpl.ConfigImpl.AdminFieldDictionaryRequest is not null
            && m_OmmBaseImpl.ConfigImpl.AdminEnumDictionaryRequest is null)
        {
            StringBuilder temp = commonImpl.GetStrBuilder();
            temp.Append("Invalid dictionary configuration was specified through the AddAdminMsg() method").Append(ILoggerClient.CR)
                .Append("Enumeration type definition request message was not populated.");

            if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, temp.ToString());

            m_OmmBaseImpl.HandleInvalidUsage(temp.ToString(), OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
            return;
        }
        else if (m_OmmBaseImpl.ConfigImpl.DictionaryConfig.DataDictionary is null
            && m_OmmBaseImpl.ConfigImpl.AdminFieldDictionaryRequest is null
            && m_OmmBaseImpl.ConfigImpl.AdminEnumDictionaryRequest is not null)
        {
            StringBuilder temp = commonImpl.GetStrBuilder();
            temp.Append("Invalid dictionary configuration was specified through the AddAdminMsg() method").Append(ILoggerClient.CR)
                .Append("RDM Field Dictionary request message was not populated.");

            if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, temp.ToString());

            m_OmmBaseImpl.HandleInvalidUsage(temp.ToString(), OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
        }

        Rdm.DataDictionary? dictionary = m_OmmBaseImpl.ConfigImpl.DictionaryConfig.DataDictionary;
        if ((dictionary is Rdm.DataDictionary)
            && dictionary.rsslDataDictionary() != null)
        {
            m_rsslLocalDictionary = ((Rdm.DataDictionary)dictionary).rsslDataDictionary();
            m_OmmBaseImpl.ConfigImpl.DictionaryConfig.IsLocalDictionary = true;
        }
        else if (m_OmmBaseImpl.ConfigImpl.DictionaryConfig.IsLocalDictionary)
            LoadDictionaryFromFile();
        else
        {
            m_ChannelDictionaryList = new();
            m_ChannelDictPool = new();
            m_ChannelDictionary = new ChannelDictionary<T>(m_OmmBaseImpl);
            m_ChannelDictPool.Add(m_ChannelDictionary);
        }
    }

    public ReactorCallbackReturnCode RdmDictionaryMsgCallback(RDMDictionaryMsgEvent msgEvent)
    {
        commonImpl.EventReceived();

        if (m_ChannelDictionaryList != null)
        {
            foreach (ChannelDictionary<T> entry in m_ChannelDictionaryList)
            {
                if (entry == msgEvent.StreamInfo.UserSpec)
                {
                    return entry.ProcessCallback(msgEvent);
                }
            }
        }

        return ProcessCallback(msgEvent, (DictionaryItem<T>)(msgEvent.StreamInfo.UserSpec!));
    }

    internal bool DownloadDictionary(ServiceDirectory directory)
    {
        if (m_OmmBaseImpl.ConfigImpl.DictionaryConfig.IsLocalDictionary)
        {
            if (m_rsslLocalDictionary != null
                && m_rsslLocalDictionary.NumberOfEntries > 0)
            {
                if (directory!.ChannelInfo!.ParentChannel != null)
                {
                    directory.ChannelInfo.ParentChannel.DataDictionary = m_rsslLocalDictionary;
                }
                else
                    directory.ChannelInfo.DataDictionary = m_rsslLocalDictionary;
            }

            return true;
        }

        if (directory.ChannelInfo?.DataDictionary != null
            || m_OmmBaseImpl.ConfigImpl.DictionaryConfig.IsLocalDictionary)
            return true;

        if (m_OmmBaseImpl.ConfigImpl.AdminFieldDictionaryRequest != null
            && m_OmmBaseImpl.ConfigImpl.AdminEnumDictionaryRequest != null)
        {
            if (m_OmmBaseImpl.ConfigImpl.AdminFieldDictionaryRequest.ServiceId == directory!.Service!.ServiceId
                || (m_OmmBaseImpl.ConfigImpl.FieldDictionaryRequestServiceName != null
                    && m_OmmBaseImpl.ConfigImpl.FieldDictionaryRequestServiceName.Equals(directory.ServiceName)))
                DownloadDictionaryFromService(directory);

            return true;
        }

        Eta.Codec.IRequestMsg rsslRequestMsg = RequestMsg();

        rsslRequestMsg.DomainType = (int)Eta.Rdm.DomainType.DICTIONARY;
        rsslRequestMsg.ContainerType = Eta.Codec.DataTypes.NO_DATA;
        rsslRequestMsg.ApplyStreaming();
        MsgKey msgKey = rsslRequestMsg.MsgKey;
        msgKey.ApplyHasName();
        msgKey.ApplyHasFilter();
        msgKey.Filter = Eta.Rdm.Dictionary.VerbosityValues.NORMAL;

        ChannelDictionary<T> dictionary = PollChannelDict(m_OmmBaseImpl);
        dictionary.ChannelInfo(directory!.ChannelInfo!);

        ReactorSubmitOptions rsslSubmitOptions = m_OmmBaseImpl.GetSubmitOptions();
        ReactorChannel rsslChannel = directory!.ChannelInfo!.ReactorChannel!;

        rsslSubmitOptions.ServiceName = directory.ServiceName;
        rsslSubmitOptions.RequestMsgOptions.UserSpecObj = (dictionary);

        int streamId = FIELD_DICTIONARY_STREAM_ID;

        List<string> dictionariesUsed = directory.Service!.Info.DictionariesUsedList;
        foreach (string dictName in dictionariesUsed)
        {
            if (!directory.Service.Info.DictionariesProvidedList.Contains(dictName))
            {
                continue;
            }
            msgKey.Name.Data(dictName);
            rsslRequestMsg.StreamId = streamId++;

            if (ReactorReturnCode.SUCCESS > rsslChannel.Submit((Eta.Codec.Msg)rsslRequestMsg, rsslSubmitOptions, out var rsslErrorInfo))
            {
                if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                {
                    StringBuilder temp = commonImpl.GetStrBuilder();
                    temp.Append("Internal Error: ReactorChannel.Submit() failed");

                    Eta.Transports.Error? error = rsslErrorInfo?.Error;
                    if (error != null)
                        temp.Append(ILoggerClient.CR)
                            .Append(directory.ChannelInfo.ToString()).Append(ILoggerClient.CR)
                            .Append("ReactorChannel ").Append((error.Channel?.GetHashCode() ?? 0).ToString("X")).Append(ILoggerClient.CR)
                            .Append("Error Id ").Append(error.ErrorId).Append(ILoggerClient.CR)
                            .Append("Internal sysError ").Append(error.SysError).Append(ILoggerClient.CR)
                            .Append("Error Location ").Append(rsslErrorInfo!.Location).Append(ILoggerClient.CR)
                            .Append("Error Text ").Append(error.Text);

                    m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, temp.ToString());

                    ReturnToChannelDictPool(dictionary);
                }

                return false;
            }
            else
            {
                if (m_OmmBaseImpl.LoggerClient.IsTraceEnabled)
                {
                    StringBuilder temp = commonImpl.GetStrBuilder();
                    temp.Append("Requested Dictionary ")
                        .Append(dictName).Append(ILoggerClient.CR)
                        .Append("from Service ").Append(directory.ServiceName).Append(ILoggerClient.CR)
                        .Append("on Channel ").Append(ILoggerClient.CR)
                        .Append(directory.ChannelInfo.ToString());
                    m_OmmBaseImpl.LoggerClient.Trace(CLIENT_NAME, temp.ToString());
                }
            }
        }

        m_ChannelDictionaryList!.Add(dictionary);

        return true;
    }

    internal ReactorCallbackReturnCode ProcessCallback(RDMDictionaryMsgEvent msgEvent, DictionaryItem<T> item)
    {
        IMsg? msg = msgEvent.Msg;
        ReactorChannel rsslChannel = msgEvent!.ReactorChannel!;

        if (msg == null)
        {
            if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
            {
                m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME,
                    "Internal Error. Received RDMDictionaryMsgEvent with no Msg in DictionaryCallbackClient.ProcessCallback()");
            }

            return ReactorCallbackReturnCode.SUCCESS;
        }

        switch (msg.MsgClass)
        {
            case MsgClasses.REFRESH:
                {
                    m_RefreshMsg ??= m_OmmBaseImpl.GetEmaObjManager().GetOmmRefreshMsg();

                    m_RefreshMsg.Decode(msg, rsslChannel!.MajorVersion, rsslChannel.MinorVersion, null);

                    ProcessRefreshMsg(m_RefreshMsg, item);

                    return ReactorCallbackReturnCode.SUCCESS;
                }
            case MsgClasses.STATUS:
                {
                    m_StatusMsg ??= m_OmmBaseImpl.GetEmaObjManager().GetOmmStatusMsg();

                    m_StatusMsg.Decode(msg, rsslChannel.MajorVersion, rsslChannel.MinorVersion, null);

                    ProcessStatusMsg(m_StatusMsg, item);

                    return ReactorCallbackReturnCode.SUCCESS;
                }
            default:
                {
                    if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                    {
                        StringBuilder temp = commonImpl.GetStrBuilder();
                        temp.Append("Received unknown RDMDictionary message type").Append(ILoggerClient.CR)
                            .Append("message type ").Append(msg.MsgClass).Append(ILoggerClient.CR)
                            .Append("streamId ").Append(msg.StreamId).Append(ILoggerClient.CR);

                        m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, temp.ToString());
                    }
                    break;
                }
        }

        return ReactorCallbackReturnCode.SUCCESS;
    }

    internal void LoadDictionaryFromFile()
    {
        if (m_rsslLocalDictionary == null)
            m_rsslLocalDictionary = new Eta.Codec.DataDictionary();
        else
            m_rsslLocalDictionary.Clear();

        CodecError codecError;
        if (m_rsslLocalDictionary.LoadFieldDictionary(m_OmmBaseImpl.ConfigImpl.DictionaryConfig.RdmFieldDictionaryFileName, out codecError) < 0)
        {
            StringBuilder temp = commonImpl.GetStrBuilder();
            temp.Append("Unable to load RDMFieldDictionary from file named ")
                .Append(m_OmmBaseImpl.ConfigImpl.DictionaryConfig.RdmFieldDictionaryFileName)
                .Append(ILoggerClient.CR)
                .Append("Current working directory ")
                .Append(System.IO.Directory.GetCurrentDirectory())
                .Append(ILoggerClient.CR)
                .Append("Error text ")
                .Append(codecError.ToString());

            if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, temp.ToString());

            m_OmmBaseImpl.HandleInvalidUsage(temp.ToString(), (int)codecError.ErrorId);
            return;
        }

        if (m_rsslLocalDictionary.LoadEnumTypeDictionary(m_OmmBaseImpl.ConfigImpl.DictionaryConfig.EnumTypeDefFileName, out codecError) < 0)
        {
            StringBuilder temp = commonImpl.GetStrBuilder();
            temp.Append("Unable to load EnumType Dictionary from file named ")
                .Append(m_OmmBaseImpl.ConfigImpl.DictionaryConfig.EnumTypeDefFileName)
                .Append(ILoggerClient.CR)
                .Append("Current working directory ")
                .Append(System.IO.Directory.GetCurrentDirectory())
                .Append(ILoggerClient.CR)
                .Append("Error text ").Append(codecError.ToString());

            if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, temp.ToString());

            m_OmmBaseImpl.HandleInvalidUsage(temp.ToString(), (int)codecError.ErrorId);
            return;
        }

        if (m_OmmBaseImpl.LoggerClient.IsTraceEnabled)
        {
            StringBuilder temp = commonImpl.GetStrBuilder();
            temp.Append("Successfully loaded local dictionaries: ")
                .Append(ILoggerClient.CR)
                .Append("RDMFieldDictionary file named ")
                .Append(m_OmmBaseImpl.ConfigImpl.DictionaryConfig.RdmFieldDictionaryFileName)
                .Append(ILoggerClient.CR)
                .Append("EnumTypeDef file named ")
                .Append(m_OmmBaseImpl.ConfigImpl.DictionaryConfig.EnumTypeDefFileName);

            m_OmmBaseImpl.LoggerClient.Trace(CLIENT_NAME, temp.ToString());
        }
    }

    internal void ProcessRefreshMsg(RefreshMsg refreshMsg, DictionaryItem<T> dictItem)
    {
        if (refreshMsg.HasServiceId)
        {
            SpecifyServiceNameFromId(refreshMsg);
        }

        EventImpl.Item = dictItem;

        NotifyOnAllMsg(refreshMsg);
        NotifyOnRefreshMsg();

        if (refreshMsg.State().StreamState == OmmState.StreamStates.NON_STREAMING)
        {
            if (refreshMsg.Complete())
                EventImpl.Item.Remove();
        }
        else if (refreshMsg.State().StreamState != OmmState.StreamStates.OPEN)
            EventImpl.Item.Remove();

        return;
    }

    internal void ProcessStatusMsg(StatusMsg statusMsg, DictionaryItem<T> dictItem)
    {
        if (statusMsg.HasServiceId)
        {
            SpecifyServiceNameFromId(statusMsg);
        }

        EventImpl.Item = dictItem;

        NotifyOnAllMsg(statusMsg);
        NotifyOnStatusMsg();

        if (statusMsg.State().StreamState != OmmState.StreamStates.OPEN)
            EventImpl.Item.Remove();

        return;
    }

    private void SpecifyServiceNameFromId(Ema.Access.Msg msgImpl)
    {
        ServiceDirectory? directory = m_OmmBaseImpl.DirectoryCallbackClient!.GetService(msgImpl.m_rsslMsg.MsgKey.ServiceId);

        if (directory != null)
        {
            int flags = msgImpl.m_rsslMsg.MsgKey.Flags;

            flags &= ~MsgKeyFlags.HAS_SERVICE_ID;

            msgImpl.m_rsslMsg.MsgKey.Flags = flags;

            msgImpl.SetMsgServiceName(directory.ServiceName!);

            msgImpl.m_rsslMsg.MsgKey.Flags = flags | MsgKeyFlags.HAS_SERVICE_ID;
        }
    }


    internal ChannelDictionary<T> PollChannelDict(OmmBaseImpl<T> baseImpl)
    {
        if (m_ChannelDictPool != null && m_ChannelDictPool.Count != 0)
            return (m_ChannelDictPool[0].Clear());
        else
            return (new ChannelDictionary<T>(baseImpl));
    }

    internal void ReturnToChannelDictPool(ChannelDictionary<T> channelDict)
    {
        m_ChannelDictPool!.Add(channelDict);
    }

    internal DataDictionary DefaultRsslDictionary()
    {
        if (IsLocalDictionary)
        {
            return m_rsslLocalDictionary!;
        }
        return m_ChannelDictionary!.rsslDictionary();
    }

    internal int FldStreamId()
    {
        if (m_ChannelDictionaryList != null && m_ChannelDictionaryList.Count != 0)
            return m_ChannelDictionaryList[0].FieldStreamId;
        else
            return FIELD_DICTIONARY_STREAM_ID;
    }

    internal int EnumStreamId()
    {
        if (m_ChannelDictionaryList != null && m_ChannelDictionaryList.Count != 0)
            return m_ChannelDictionaryList[0].EnumStreamId;
        else
            return ENUM_TYPE_DICTIONARY_STREAM_ID;
    }

    internal List<ChannelDictionary<T>> ChannelDictionaryList
    {
        get => m_ChannelDictionaryList!;
    }

    internal DictionaryItem<T>? DictionaryItem(Ema.Access.RequestMsg reqMsg, T client, Object obj)
    {
        return null;
    }

    internal bool DownloadDictionaryFromService(ServiceDirectory directory)
    {
        Eta.Codec.IRequestMsg rsslRequestMsg = RequestMsg();

        rsslRequestMsg.DomainType = (int)Eta.Rdm.DomainType.DICTIONARY;
        rsslRequestMsg.ContainerType = Eta.Codec.DataTypes.NO_DATA;

        DictionaryRequest rsslDictRequest = m_OmmBaseImpl.ConfigImpl.AdminFieldDictionaryRequest!;
        if (rsslDictRequest.Streaming == true)
            rsslRequestMsg.ApplyStreaming();

        MsgKey msgKey = rsslRequestMsg.MsgKey;
        msgKey.ApplyHasName();
        msgKey.ApplyHasFilter();
        msgKey.Filter = rsslDictRequest.Verbosity;
        msgKey.Name = rsslDictRequest.DictionaryName;
        rsslRequestMsg.StreamId = FIELD_DICTIONARY_STREAM_ID;

        ChannelDictionary<T> dictionary = PollChannelDict(m_OmmBaseImpl);
        dictionary.ChannelInfo(directory.ChannelInfo!);

        ReactorSubmitOptions rsslSubmitOptions = m_OmmBaseImpl.GetSubmitOptions();
        ReactorChannel rsslChannel = directory.ChannelInfo!.ReactorChannel!;

        rsslSubmitOptions.ServiceName = directory.ServiceName;
        rsslSubmitOptions.RequestMsgOptions.UserSpecObj = dictionary;

        if (ReactorReturnCode.SUCCESS > rsslChannel.Submit((Eta.Codec.Msg)rsslRequestMsg, rsslSubmitOptions, out var rsslErrorInfo))
        {
            if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
            {
                StringBuilder temp = commonImpl.GetStrBuilder();
                temp.Append("Internal Error: ReactorChannel.Submit() failed").Append(ILoggerClient.CR);
                Eta.Transports.Error? error = rsslErrorInfo?.Error;

                if (error != null)
                    temp.Append(directory.ChannelInfo.ToString()).Append(ILoggerClient.CR)
                        .Append("RsslChannel ").Append((error.Channel?.GetHashCode() ?? 0).ToString("X")).Append(ILoggerClient.CR)
                        .Append("Error Id ").Append(error.ErrorId).Append(ILoggerClient.CR)
                        .Append("Internal sysError ").Append(error.SysError).Append(ILoggerClient.CR)
                        .Append("Error Location ").Append(rsslErrorInfo!.Location).Append(ILoggerClient.CR)
                        .Append("Error Text ").Append(error.Text);

                m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, temp.ToString());

                ReturnToChannelDictPool(dictionary);
            }

            return false;
        }
        else
        {
            if (m_OmmBaseImpl.LoggerClient.IsTraceEnabled)
            {
                StringBuilder temp = commonImpl.GetStrBuilder();
                temp.Append("Requested Dictionary ")
                    .Append(rsslDictRequest.DictionaryName.ToString()).Append(ILoggerClient.CR)
                    .Append("from Service ").Append(directory.ServiceName).Append(ILoggerClient.CR)
                    .Append("on Channel ").Append(ILoggerClient.CR)
                    .Append(directory.ChannelInfo.ToString());
                m_OmmBaseImpl.LoggerClient.Trace(CLIENT_NAME, temp.ToString());
            }
        }

        rsslRequestMsg.Clear();
        rsslRequestMsg.MsgClass = MsgClasses.REQUEST;
        rsslRequestMsg.DomainType = (int)Eta.Rdm.DomainType.DICTIONARY;
        rsslRequestMsg.ContainerType = Eta.Codec.DataTypes.NO_DATA;

        DictionaryRequest rsslEnumDictRequest = m_OmmBaseImpl.ConfigImpl.AdminEnumDictionaryRequest!;
        if (rsslEnumDictRequest!.Streaming)
        {
            rsslRequestMsg.ApplyStreaming();
        }

        msgKey = rsslRequestMsg.MsgKey;
        msgKey.ApplyHasName();
        msgKey.ApplyHasFilter();
        msgKey.Filter = rsslEnumDictRequest.Verbosity;
        msgKey.Name = rsslEnumDictRequest.DictionaryName;
        rsslRequestMsg.StreamId = ENUM_TYPE_DICTIONARY_STREAM_ID;

        if (ReactorReturnCode.SUCCESS > rsslChannel.Submit((Eta.Codec.Msg)rsslRequestMsg, rsslSubmitOptions, out rsslErrorInfo))
        {
            if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
            {
                StringBuilder temp = commonImpl.GetStrBuilder();

                temp.Append("Internal Error: ReactorChannel.Submit() failed").Append(ILoggerClient.CR);

                Eta.Transports.Error? error = rsslErrorInfo?.Error;
                if (error != null)
                    temp.Append(directory.ChannelInfo.ToString()).Append(ILoggerClient.CR)
                        .Append("RsslChannel ").Append((error.Channel?.GetHashCode() ?? 0).ToString("X")).Append(ILoggerClient.CR)
                        .Append("Error Id ").Append(error.ErrorId).Append(ILoggerClient.CR)
                        .Append("Internal sysError ").Append(error.SysError).Append(ILoggerClient.CR)
                        .Append("Error Location ").Append(rsslErrorInfo!.Location).Append(ILoggerClient.CR)
                        .Append("Error Text ").Append(error.Text);

                m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, temp.ToString());

                ReturnToChannelDictPool(dictionary);
            }

            return false;
        }
        else
        {
            if (m_OmmBaseImpl.LoggerClient.IsTraceEnabled)
            {
                StringBuilder temp = commonImpl.GetStrBuilder();
                temp.Append("Requested Dictionary ")
                    .Append(rsslDictRequest.DictionaryName.ToString()).Append(ILoggerClient.CR)
                    .Append("from Service ").Append(directory.ServiceName).Append(ILoggerClient.CR)
                    .Append("on Channel ").Append(ILoggerClient.CR)
                    .Append(directory.ChannelInfo.ToString());

                m_OmmBaseImpl.LoggerClient.Trace(CLIENT_NAME, temp.ToString());
            }
        }

        m_ChannelDictionaryList!.Add(dictionary);

        return true;
    }

    internal bool IsDictionaryReady
    {
        get
        {
            if (m_rsslLocalDictionary != null
                && m_rsslLocalDictionary.NumberOfEntries > 0
                && m_rsslLocalDictionary.EnumTableCount > 0)
                return true;
            else
            {
                if (m_ChannelDictionaryList == null || m_ChannelDictionaryList.Count == 0)
                    return false;

                foreach (ChannelDictionary<T> entry in m_ChannelDictionaryList)
                {
                    if (!(entry.IsLoaded))
                        return false;
                }
                return true;
            }
        }
    }

    internal bool IsLocalDictionary
    {
        get => m_rsslLocalDictionary != null;
    }

    internal Int rsslCurrentFid()
    {
        if (m_rsslCurrentFid == null)
            m_rsslCurrentFid = new();
        else
            m_rsslCurrentFid.Clear();

        return m_rsslCurrentFid;
    }

    internal Eta.Codec.Buffer rsslDictEncBuffer()
    {
        if (m_rsslEncBuffer == null)
        {
            m_rsslEncBuffer = new();
            m_rsslEncBuffer.Data(new Eta.Common.ByteBuffer(MAX_DICTIONARY_BUFFER_SIZE));
        }
        else
        {
            Eta.Common.ByteBuffer byteBuf = m_rsslEncBuffer.Data();
            byteBuf.Clear();
            m_rsslEncBuffer.Data(byteBuf, 0, byteBuf.Capacity);
        }

        return m_rsslEncBuffer;
    }
}

#region Helper classes

internal class DictionaryCallbackClientConsumer : DictionaryCallbackClient<IOmmConsumerClient>
{
    internal DictionaryCallbackClientConsumer(OmmBaseImpl<IOmmConsumerClient> baseImpl) : base(baseImpl)
    {
        OmmConsumerImpl ommConsumerImpl = (OmmConsumerImpl)baseImpl;
        EventImpl.SetOmmConsumer(ommConsumerImpl.Consumer);
        NotifyOnAllMsg = NotifyOnAllMsgImpl;
        NotifyOnRefreshMsg = NotifyOnRefreshMsgImpl;
        NotifyOnStatusMsg = NotifyOnStatusMsgImpl;
    }

    public void NotifyOnAllMsgImpl(Access.Msg msg)
    {
        if (EventImpl.Item?.Client == null)
        {
            if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
            {
                StringBuilder message = m_OmmBaseImpl.GetStrBuilder();
                message.AppendLine("An incoming Msg to non-existent IOmmConsumerClient has been dropped.");
                m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, message.ToString());
            }
        }
        else
        {
            EventImpl.Item!.Client.OnAllMsg(msg, EventImpl);
        }
    }

    public void NotifyOnRefreshMsgImpl()
    {
        if (EventImpl.Item?.Client == null)
        {
            if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
            {
                StringBuilder message = m_OmmBaseImpl.GetStrBuilder();
                message.AppendLine("An incoming RefreshMsg to non-existent IOmmConsumerClient has been dropped.");
                m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, message.ToString());
            }
        }
        else
        {
            EventImpl.Item!.Client.OnRefreshMsg(m_RefreshMsg!, EventImpl);
        }
    }

    public void NotifyOnStatusMsgImpl()
    {
        if (EventImpl.Item?.Client == null)
        {
            if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
            {
                StringBuilder message = m_OmmBaseImpl.GetStrBuilder();
                message.AppendLine("An incoming StatusMsg to non-existent IOmmConsumerClient has been dropped.");
                m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, message.ToString());
            }
        }
        else
        {
            EventImpl.Item!.Client!.OnStatusMsg(m_StatusMsg!, EventImpl);
        }
    }
}


class ChannelDictionary<T>
{
    internal const string CLIENT_NAME = "ChannelDictionary";

    private OmmBaseImpl<T> m_OmmBaseImpl;
    private ChannelInfo? m_ChannelInfo;
    private bool m_IsFldLoaded;
    private bool m_IsEnumLoaded;
    private int m_FldStreamId;
    private int m_EnumStreamId;
    private Eta.Common.Locker? m_ChannelDictLock;
    private List<DictionaryItem<T>>? m_ListenerList;
    private EncodeIterator? m_EncodeIterator;
    private DataDictionary m_rsslDictionary = new();


    internal ChannelDictionary(OmmBaseImpl<T> baseImpl)
    {
        m_OmmBaseImpl = baseImpl;
    }

    internal ChannelInfo? ChannelInfo()
    {
        return m_ChannelInfo;
    }

    internal ChannelDictionary<T> ChannelInfo(ChannelInfo channelInfo)
    {
        m_ChannelInfo = channelInfo;
        m_ChannelInfo.DataDictionary = m_rsslDictionary;
        return this;
    }

    internal ChannelDictionary<T> Clear()
    {
        m_ChannelInfo = null;
        m_IsFldLoaded = false;
        m_IsEnumLoaded = false;
        m_FldStreamId = 0;
        m_EnumStreamId = 0;
        m_rsslDictionary.Clear();
        m_EncodeIterator?.Clear();

        if (m_ListenerList?.Count > 0)
        {
            foreach (DictionaryItem<T> entry in m_ListenerList)
            {
                entry.ReturnToPool();
            }

            m_ListenerList.Clear();
        }

        return this;
    }

    internal DataDictionary rsslDictionary()
    {
        return m_rsslDictionary;
    }

    internal bool IsLoaded
    {
        get => m_IsEnumLoaded && m_IsFldLoaded;
    }

    internal int FieldStreamId
    {
        get => m_FldStreamId;
    }

    internal int EnumStreamId
    {
        get => m_EnumStreamId;
    }

    internal Eta.Common.Locker ChannelDictionaryLock
    {
        get
        {
            m_ChannelDictLock ??= new Eta.Common.MonitorWriteLocker(new object());
            return m_ChannelDictLock;
        }
    }

    internal void AddListener(DictionaryItem<T> item)
    {
        if (m_ListenerList == null)
            m_ListenerList = new();

        m_ListenerList.Add(item);
    }

    internal void RemoveListener(DictionaryItem<T> item)
    {
        if (m_ListenerList == null || m_ListenerList.Count == 0)
            return;

        m_ListenerList.Remove(item);
    }

    internal void NotifyStatusToListener(OmmBaseImpl<T> ommBaseImpl, Eta.Codec.State rsslStatus, int streamId)
    {
        ChannelDictionaryLock.Enter();

        if (m_ListenerList == null || m_ListenerList.Count == 0)
        {
            ChannelDictionaryLock.Exit();
            return;
        }

        try
        {
            DictionaryCallbackClient<T> dictCallbackClient = ommBaseImpl.DictionaryCallbackClient!;
            IStatusMsg rsslStatusMsg = dictCallbackClient.StatusMsg();
            Eta.Codec.Buffer rsslEncDictBuf;

            if (m_EncodeIterator == null)
                m_EncodeIterator = new EncodeIterator();

            for (int index = 0; index < m_ListenerList.Count; index++)
            {
                DictionaryItem<T> dictItem = m_ListenerList[index];

                if (dictItem.StreamId != streamId)
                    continue;

                rsslStatusMsg.MsgClass = MsgClasses.STATUS;
                rsslStatusMsg.StreamId = streamId;
                rsslStatusMsg.DomainType = (int)Eta.Rdm.DomainType.DICTIONARY;
                rsslStatusMsg.ContainerType = DataTypes.NO_DATA;
                rsslStatusMsg.ApplyHasState();
                rsslStatusMsg.State.StreamState(rsslStatus.StreamState());
                rsslStatusMsg.State.DataState(rsslStatus.DataState());
                rsslStatusMsg.State.Code(rsslStatus.Code());
                rsslStatusMsg.State.Text(rsslStatus.Text());
                rsslStatusMsg.ApplyHasMsgKey();
                rsslStatusMsg.MsgKey.ApplyHasName();
                rsslStatusMsg.MsgKey.Name.Data(dictItem.Name);

                m_EncodeIterator.Clear();
                rsslEncDictBuf = dictCallbackClient.rsslDictEncBuffer();
                CodecReturnCode retCode = m_EncodeIterator.SetBufferAndRWFVersion(rsslEncDictBuf, Codec.MajorVersion(), Codec.MinorVersion());
                if (retCode != CodecReturnCode.SUCCESS)
                {
                    if (ommBaseImpl.LoggerClient.IsErrorEnabled)
                    {
                        ommBaseImpl.LoggerClient.Error(CLIENT_NAME,
                            "Internal Error. Failed to set encode iterator with buffer in ChannelDictionary.notifyStatusToListener()");
                    }
                    return;
                }

                if ((retCode = rsslStatusMsg.Encode(m_EncodeIterator)) != CodecReturnCode.SUCCESS)
                {
                    if (ommBaseImpl.LoggerClient.IsErrorEnabled)
                    {
                        ommBaseImpl.LoggerClient.Error(CLIENT_NAME,
                            "Internal Error. Failed to encode msg in ChannelDictionary.notifyStatusToListener()");
                    }
                    return;
                }

                dictCallbackClient.m_StatusMsg ??= m_OmmBaseImpl.GetEmaObjManager().GetOmmStatusMsg();
                dictCallbackClient.m_StatusMsg.Decode(rsslEncDictBuf, Codec.MajorVersion(), Codec.MinorVersion(), null, null);

                dictCallbackClient.ProcessStatusMsg(dictCallbackClient.m_StatusMsg, dictItem);

                if (rsslStatus.StreamState() != StreamStates.OPEN)
                {
                    m_ListenerList.RemoveAt(index);
                    --index;
                }

                break;
            }
        }
        finally
        {
            ChannelDictionaryLock.Exit();
        }
    }

    internal ReactorCallbackReturnCode ProcessCallback(RDMDictionaryMsgEvent msgEvent)

    {
        IMsg msg = msgEvent.Msg!;
        ReactorChannel rsslChannel = msgEvent.ReactorChannel!;
        ChannelInfo channelInfo = (ChannelInfo)rsslChannel.UserSpecObj!;

        if (msg == null)
        {
            LSEG.Eta.Transports.Error error = msgEvent.ReactorErrorInfo.Error;

            if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
            {
                StringBuilder temp = m_OmmBaseImpl.GetStrBuilder();

                temp.Append("Received event without RDMDictionary message").Append(ILoggerClient.CR)
                    .Append("ChannelInfo ").Append(ILoggerClient.CR)
                    .Append(channelInfo.ToString()).Append(ILoggerClient.CR)
                    .Append("RsslChannel ").Append((error.Channel?.GetHashCode() ?? 0).ToString("X")).Append(ILoggerClient.CR)
                    .Append("Error Id ").Append(error.ErrorId).Append(ILoggerClient.CR)
                    .Append("Internal sysError ").Append(error.SysError).Append(ILoggerClient.CR)
                    .Append("Error Location ").Append(msgEvent.ReactorErrorInfo.Location).Append(ILoggerClient.CR)
                    .Append("Error Text ").Append(error.Text);

                m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, temp.ToString());
            }

            m_OmmBaseImpl.CloseReactorChannel(msgEvent.ReactorChannel);

            return ReactorCallbackReturnCode.SUCCESS;
        }

        switch (msg.MsgClass)
        {
            case MsgClasses.REFRESH:
                {
                    Eta.Codec.IRefreshMsg rsslMsg = (Eta.Codec.IRefreshMsg)msg;
                    Eta.Codec.State state = rsslMsg.State;

                    if (state.StreamState() != StreamStates.OPEN
                        && state.StreamState() != StreamStates.NON_STREAMING)
                    {
                        if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                        {
                            StringBuilder temp = m_OmmBaseImpl.GetStrBuilder();
                            temp.Append("RDMDictionary stream was closed with refresh message").Append(ILoggerClient.CR)
                                .Append("ChannelInfo").Append(ILoggerClient.CR)
                                .Append(channelInfo.ToString()).Append(ILoggerClient.CR)
                                .Append("Reason ").Append(state.ToString());

                            m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, temp.ToString());
                        }
                        break;
                    }

                    else if (state.DataState() == DataStates.SUSPECT)
                    {
                        if (m_OmmBaseImpl.LoggerClient.IsWarnEnabled)
                        {

                            StringBuilder temp = m_OmmBaseImpl.GetStrBuilder();
                            temp.Append("RDMDictionary stream state was changed to suspect with refresh message").Append(ILoggerClient.CR)
                                .Append("ChannelInfo").Append(ILoggerClient.CR)
                                .Append(channelInfo.ToString()).Append(ILoggerClient.CR)
                                .Append("Reason ").Append(state.ToString());

                            m_OmmBaseImpl.LoggerClient.Warn(CLIENT_NAME, temp.ToString());
                        }
                        break;
                    }

                    if (m_OmmBaseImpl.LoggerClient.IsTraceEnabled)
                    {

                        StringBuilder temp = m_OmmBaseImpl.GetStrBuilder();
                        temp.Append("Received RDMDictionary refresh message").Append(ILoggerClient.CR)
                            .Append("Dictionary name ").Append(rsslMsg.MsgKey.Name.ToString()).Append(ILoggerClient.CR)
                            .Append("streamId ").Append(rsslMsg.StreamId);

                        m_OmmBaseImpl.LoggerClient.Trace(CLIENT_NAME, temp.ToString());
                    }

                    DictionaryRefresh rsslRefresh = msgEvent.DictionaryMsg!.DictionaryRefresh!;

                    if (rsslRefresh.HasInfo)
                    {
                        switch (rsslRefresh.DictionaryType)
                        {
                            case Eta.Rdm.Dictionary.Types.FIELD_DEFINITIONS:
                                {
                                    if (m_FldStreamId == 0)
                                        m_FldStreamId = rsslMsg.StreamId;
                                    else if (m_FldStreamId != rsslMsg.StreamId)
                                    {
                                        if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                                        {

                                            StringBuilder temp = m_OmmBaseImpl.GetStrBuilder();
                                            temp.Append("Received RDMDictionary refresh message with FieldDefinitions but changed streamId")
                                                .Append(ILoggerClient.CR)
                                                .Append("Initial streamId ").Append(m_FldStreamId).Append(ILoggerClient.CR)
                                                .Append("New streamId ").Append(rsslMsg.StreamId);

                                            m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, temp.ToString());
                                        }
                                        return ReactorCallbackReturnCode.SUCCESS;
                                    }
                                    break;
                                }
                            case Eta.Rdm.Dictionary.Types.ENUM_TABLES:
                                {
                                    if (m_EnumStreamId == 0)
                                        m_EnumStreamId = rsslMsg.StreamId;
                                    else if (m_EnumStreamId != rsslMsg.StreamId)
                                    {
                                        if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                                        {

                                            StringBuilder temp = m_OmmBaseImpl.GetStrBuilder();
                                            temp.Append("Received RDMDictionary refresh message with EnumTables but changed streamId")
                                                .Append(ILoggerClient.CR)
                                                .Append("Initial streamId ").Append(m_FldStreamId).Append(ILoggerClient.CR)
                                                .Append("New streamId ").Append(rsslMsg.StreamId);

                                            m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, temp.ToString());
                                        }
                                        return ReactorCallbackReturnCode.SUCCESS;
                                    }
                                    break;
                                }
                            default:
                                {
                                    if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                                    {
                                        StringBuilder temp = m_OmmBaseImpl.GetStrBuilder();
                                        temp.Append("Received RDMDictionary message with unknown dictionary type").Append(ILoggerClient.CR)
                                            .Append("Dictionary type ").Append(rsslRefresh.DictionaryType);

                                        m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, temp.ToString());
                                    }
                                    return ReactorCallbackReturnCode.SUCCESS;
                                }
                        }
                    }

                    DecodeIterator dIter = new();
                    dIter.Clear();
                    if (CodecReturnCode.SUCCESS != dIter.SetBufferAndRWFVersion(rsslMsg.EncodedDataBody, rsslChannel.MajorVersion, rsslChannel.MinorVersion))
                    {
                        if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                        {
                            StringBuilder temp = m_OmmBaseImpl.GetStrBuilder();
                            temp.Append("Internal Error: failed to set buffer while decoding dictionary").Append(ILoggerClient.CR)
                                .Append("Trying to set ").Append(rsslChannel.MajorVersion)
                                .Append('.').Append(rsslChannel.MinorVersion);

                            m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, temp.ToString());
                        }
                        return ReactorCallbackReturnCode.SUCCESS;
                    }

                    if (m_FldStreamId == rsslMsg.StreamId)
                    {
                        if (m_IsFldLoaded == true && m_IsEnumLoaded == true)
                            m_rsslDictionary.Clear();

                        if (CodecReturnCode.SUCCESS == m_rsslDictionary.DecodeFieldDictionary(dIter,
                            LSEG.Eta.Rdm.Dictionary.VerbosityValues.VERBOSE, out var codecError))
                        {
                            if (rsslRefresh.RefreshComplete)
                            {
                                m_IsFldLoaded = true;

                                if (m_OmmBaseImpl.LoggerClient.IsTraceEnabled)
                                {
                                    StringBuilder temp = m_OmmBaseImpl.GetStrBuilder();
                                    temp.Append("Received RDMDictionary refresh complete message").Append(ILoggerClient.CR)
                                        .Append("dictionary name ").Append(rsslRefresh.DictionaryName.ToString()).Append(ILoggerClient.CR)
                                        .Append("streamId ").Append(rsslMsg.StreamId);

                                    m_OmmBaseImpl.LoggerClient.Trace(CLIENT_NAME, temp.ToString());
                                }
                            }
                            else
                                m_IsFldLoaded = false;
                        }
                        else
                        {
                            m_IsFldLoaded = false;

                            if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                            {
                                StringBuilder temp = m_OmmBaseImpl.GetStrBuilder();
                                temp.Append("Internal Error: failed to decode FieldDictionary").Append(ILoggerClient.CR)
                                    .Append("Error text ").Append(codecError.Text);

                                m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, temp.ToString());
                            }

                            return ReactorCallbackReturnCode.SUCCESS;
                        }
                    }
                    else if (m_EnumStreamId == rsslMsg.StreamId)
                    {
                        if (CodecReturnCode.SUCCESS == m_rsslDictionary.DecodeEnumTypeDictionary(dIter,
                                Eta.Rdm.Dictionary.VerbosityValues.VERBOSE, out var codecError))
                        {
                            if (rsslRefresh.RefreshComplete)
                            {
                                m_IsEnumLoaded = true;

                                if (m_OmmBaseImpl.LoggerClient.IsTraceEnabled)
                                {
                                    StringBuilder temp = m_OmmBaseImpl.GetStrBuilder();
                                    temp.Append("Received RDMDictionary refresh complete message").Append(ILoggerClient.CR)
                                        .Append("dictionary name ").Append(rsslRefresh.DictionaryName.ToString()).Append(ILoggerClient.CR)
                                        .Append("streamId ").Append(rsslMsg.StreamId);

                                    m_OmmBaseImpl.LoggerClient.Trace(CLIENT_NAME, temp.ToString());
                                }
                            }
                            else
                                m_IsEnumLoaded = false;
                        }
                        else
                        {
                            m_IsEnumLoaded = false;

                            if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                            {
                                StringBuilder temp = m_OmmBaseImpl.GetStrBuilder();
                                temp.Append("Internal Error: failed to decode EnumTable dictionary").Append(ILoggerClient.CR)
                                    .Append("Error text ").Append(codecError.Text);

                                m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, temp.ToString());
                            }

                            return ReactorCallbackReturnCode.SUCCESS;
                        }
                    }
                    else
                    {
                        if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                        {
                            StringBuilder temp = m_OmmBaseImpl.GetStrBuilder();
                            temp.Append("Received unexpected RDMDictionary refresh message on streamId ").Append(ILoggerClient.CR)
                                .Append(rsslMsg.StreamId);

                            m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, temp.ToString());
                        }

                        return ReactorCallbackReturnCode.SUCCESS;
                    }

                    break;
                }
            case MsgClasses.STATUS:
                {
                    Eta.Codec.IStatusMsg rsslMsg = (Eta.Codec.IStatusMsg)msg;
                    DictionaryStatus rsslStatus = msgEvent.DictionaryMsg!.DictionaryStatus!;

                    if (rsslMsg.CheckHasState())
                    {
                        State state = rsslMsg.State;

                        if (state.StreamState() != StreamStates.OPEN)
                        {
                            if (m_OmmBaseImpl.LoggerClient.IsWarnEnabled)
                            {
                                StringBuilder temp = m_OmmBaseImpl.GetStrBuilder();
                                temp.Append("RDMDictionary stream was closed with status message").Append(ILoggerClient.CR)
                                    .Append("streamId ").Append(rsslMsg.StreamId).Append(ILoggerClient.CR)
                                    .Append("Reason ").Append(state.ToString());

                                m_OmmBaseImpl.LoggerClient.Warn(CLIENT_NAME, temp.ToString());
                            }

                            NotifyStatusToListener(m_OmmBaseImpl, rsslStatus.State, rsslMsg.StreamId);
                            break;
                        }
                        else if (state.DataState() == DataStates.SUSPECT)
                        {
                            if (m_OmmBaseImpl.LoggerClient.IsWarnEnabled)
                            {
                                StringBuilder temp = m_OmmBaseImpl.GetStrBuilder();
                                temp.Append("RDMDictionary stream state was changed to suspect with status message").Append(ILoggerClient.CR)
                                    .Append("streamId ").Append(rsslMsg.StreamId).Append(ILoggerClient.CR)
                                    .Append("Reason ").Append(state.ToString());

                                m_OmmBaseImpl.LoggerClient.Warn(CLIENT_NAME, temp.ToString());
                            }

                            NotifyStatusToListener(m_OmmBaseImpl, rsslStatus.State, rsslMsg.StreamId);
                            break;
                        }

                        if (m_OmmBaseImpl.LoggerClient.IsTraceEnabled)
                        {
                            StringBuilder temp = m_OmmBaseImpl.GetStrBuilder();
                            temp.Append("RDMDictionary stream was open with status message").Append(ILoggerClient.CR)
                                .Append("streamId ").Append(rsslMsg.StreamId).Append(ILoggerClient.CR)
                                .Append("Reason ").Append(state.ToString());

                            m_OmmBaseImpl.LoggerClient.Trace(CLIENT_NAME, temp.ToString());
                        }
                    }
                    else
                    {
                        if (m_OmmBaseImpl.LoggerClient.IsWarnEnabled)
                        {
                            StringBuilder temp = m_OmmBaseImpl.GetStrBuilder();
                            temp.Append("Received RDMDictionary status message without the state").Append(ILoggerClient.CR)
                                .Append("streamId ").Append(rsslMsg.StreamId).Append(ILoggerClient.CR);

                            m_OmmBaseImpl.LoggerClient.Warn(CLIENT_NAME, temp.ToString());
                        }
                    }
                    break;
                }
            default:
                {
                    if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                    {

                        StringBuilder temp = m_OmmBaseImpl.GetStrBuilder();
                        temp.Append("Received unknown RDMDictionary message type").Append(ILoggerClient.CR)
                            .Append("message type ").Append(msg.MsgClass).Append(ILoggerClient.CR)
                            .Append("streamId ").Append(msg.StreamId).Append(ILoggerClient.CR);

                        m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, temp.ToString());
                    }
                    break;
                }
        }

        return ReactorCallbackReturnCode.SUCCESS;
    }
}

internal class DictionaryItem<T> : SingleItem<T>, ITimeoutClient
{
    internal const string CLIENT_NAME = "DictionaryItem";

    private const int OPEN_ITEM_TIMEOUT = 500;
    private const int CLOSE_ITEM_TIMEOUT = 2000;
    private const int REMOVE_ITEM_TIMEOUT = 2000;
    private const int PART_REFRESH_TIMEOUT = 500;

    private int m_rsslFilter;
    private int m_CurrentFid;
    private bool m_NeedRemoved;
    private bool m_Removed;
    private string? m_Name;
    private EncodeIterator? m_EncodeIterator;

    public DictionaryItem() : base()
    {
        m_type = ItemType.DICTIONARY_ITEM;
    }

    public DictionaryItem(OmmBaseImpl<T> baseImpl, T client, object? closure) : base(baseImpl, client, closure, null)
    {
        m_rsslFilter = 0;
        m_CurrentFid = 0;
        m_NeedRemoved = false;
        m_Removed = false;
        m_type = ItemType.DICTIONARY_ITEM;
    }

    public void ResetDictionaryItem(OmmBaseImpl<T> baseImpl, T client, object? closure)
    {
        base.Reset(baseImpl, client, closure, null);

        m_rsslFilter = 0;
        m_CurrentFid = 0;
        m_NeedRemoved = false;
    }

    public bool OpenDictionaryItem(Ema.Access.RequestMsg reqMsg)
    {
        IRequestMsg rsslReqMsg = reqMsg.m_rsslMsg;
        m_Name = rsslReqMsg.MsgKey.Name.ToString();
        DictionaryCallbackClient<T> dictCBClient = m_OmmBaseImpl.DictionaryCallbackClient!;

        if (rsslReqMsg.MsgKey.CheckHasFilter())
            m_rsslFilter = (int)rsslReqMsg.MsgKey.Filter;

        /* User is asking for a specific service's dictionary, submit the request to the Reactor */
        if (reqMsg.HasServiceName || rsslReqMsg.MsgKey.CheckHasServiceId())
        {
            return base.Open(reqMsg);
        }
        else
        {
            /* This ensures that a valid handle is assigned to the request. */
            m_OmmBaseImpl.ItemCallbackClient!.AddToItemMap(m_OmmBaseImpl.NextLongId(), this);

            DataDictionary rsslDictionary = dictCBClient.DefaultRsslDictionary();

            if (rsslDictionary != null)
            {
                /* EMA will generate the Dictionary from the cached values */
                if (m_Name.Equals(DictionaryCallbackClient<T>.DICTIONARY_RWFFID))
                {
                    m_CurrentFid = rsslDictionary.MinFid;
                    StreamId = dictCBClient.FldStreamId();
                }
                else if (m_Name.Equals(DictionaryCallbackClient<T>.DICTIONARY_RWFENUM))
                {
                    m_CurrentFid = 0;
                    StreamId = dictCBClient.EnumStreamId();
                }
                else
                {
                    StringBuilder temp = m_OmmBaseImpl.GetStrBuilder();

                    temp.Append("Invalid ReqMsg's name : ")
                        .Append(m_Name)
                        .Append("\nReqMsg's name must be \"").Append(DictionaryCallbackClient<T>.DICTIONARY_RWFFID)
                        .Append("\" or \"").Append(DictionaryCallbackClient<T>.DICTIONARY_RWFENUM).Append("\" for MMT_DICTIONARY domain type. ")
                        .Append("Instance name='").Append(m_OmmBaseImpl.InstanceName).Append("'.");

                    if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                    {
                        m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, temp.ToString());
                    }

                    m_OmmBaseImpl.HandleInvalidUsage(temp.ToString(), OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);

                    return false;
                }

                if (!dictCBClient.IsLocalDictionary)
                {
                    ChannelDictionary<T> channelDict = dictCBClient.ChannelDictionaryList[0];

                    channelDict.ChannelDictionaryLock.Enter();

                    channelDict.AddListener(this);

                    channelDict.ChannelDictionaryLock.Exit();
                }

                m_OmmBaseImpl.TimeoutEventManager!.AddTimeoutEvent(OPEN_ITEM_TIMEOUT, this);

                return true;
            }
            else
            {
                if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                    m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME,
                        "Ema must have to receive a dictionary before open a dictionary request");
                return false;
            }
        }
    }

    public override bool Modify(Ema.Access.RequestMsg reqMsg)
    {
        StringBuilder temp = m_OmmBaseImpl.GetStrBuilder();
        temp.Append("Invalid attempt to modify dictionary stream. ")
            .Append("Instance name='").Append(m_OmmBaseImpl.InstanceName).Append("'.");

        if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
            m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, temp.ToString());

        m_OmmBaseImpl.HandleInvalidUsage(temp.ToString(), OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);

        return false;
    }

    public override bool Submit(PostMsg _)
    {
        StringBuilder temp = m_OmmBaseImpl.GetStrBuilder();
        temp.Append("Invalid attempt to submit PostMsg on dictionary stream. ")
            .Append("Instance name='").Append(m_OmmBaseImpl.InstanceName).Append("'.");

        if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
            m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, temp.ToString());

        m_OmmBaseImpl.HandleInvalidUsage(temp.ToString(), OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);

        return false;
    }

    public override bool Submit(GenericMsg _)
    {
        StringBuilder temp = m_OmmBaseImpl.GetStrBuilder();
        temp.Append("Invalid attempt to submit GenericMsg on dictionary stream. ")
            .Append("Instance name='").Append(m_OmmBaseImpl.InstanceName).Append("'.");

        if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
            m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, temp.ToString());

        m_OmmBaseImpl.HandleInvalidUsage(temp.ToString(), OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);

        return false;
    }

    public override bool Close()
    {
        if (StreamId > DictionaryCallbackClient<T>.ENUM_TYPE_DICTIONARY_STREAM_ID)
        {
            base.Close();
        }
        else
        {
            if (m_NeedRemoved == false)
            {
                m_NeedRemoved = true;

                DictionaryCallbackClient<T> dictCBClient = m_OmmBaseImpl.DictionaryCallbackClient!;
                if (dictCBClient.ChannelDictionaryList != null
                    && dictCBClient.ChannelDictionaryList.Count != 0)
                {
                    ChannelDictionary<T> channelDict = dictCBClient.ChannelDictionaryList[0];

                    channelDict.ChannelDictionaryLock.Enter();

                    channelDict.RemoveListener(this);

                    channelDict.ChannelDictionaryLock.Exit();
                }

                m_OmmBaseImpl.TimeoutEventManager!.AddTimeoutEvent(CLOSE_ITEM_TIMEOUT, this);
            }
        }

        return true;
    }

    public override void Remove()
    {
        if (!m_NeedRemoved)
        {
            m_NeedRemoved = true;
            m_OmmBaseImpl.TimeoutEventManager!.AddTimeoutEvent(REMOVE_ITEM_TIMEOUT, this);
        }
    }

    public void HandleTimeoutEvent()
    {
        if (m_NeedRemoved)
        {
            if (!m_Removed)
            {
                m_OmmBaseImpl.ItemCallbackClient!.RemoveFromMap(this);
                this.ReturnToPool();
                m_Removed = true;
            }
            return;
        }

        DictionaryCallbackClient<T> dictCallbackClient = m_OmmBaseImpl.DictionaryCallbackClient!;
        DataDictionary rsslDictionary = dictCallbackClient.DefaultRsslDictionary();
        bool firstPart = false;
        CodecReturnCode ret = CodecReturnCode.FAILURE;

        LSEG.Eta.Codec.Buffer rsslDictEncBuffer = m_OmmBaseImpl.DictionaryCallbackClient!.rsslDictEncBuffer();

        if (rsslDictionary != null
            && (rsslDictionary.EnumTableCount > 0
                || rsslDictionary.NumberOfEntries > 0))
        {
            if (m_Name!.Equals(DictionaryCallbackClient<T>.DICTIONARY_RWFFID))
            {
                if (m_CurrentFid == rsslDictionary.MinFid)
                    firstPart = true;

                ret = EncodeDataDictionaryResp(firstPart, rsslDictionary, rsslDictEncBuffer);
            }
            else if (m_Name.Equals(DictionaryCallbackClient<T>.DICTIONARY_RWFENUM))
            {
                if (m_CurrentFid == 0)
                    firstPart = true;

                ret = EncodeDataDictionaryResp(firstPart, rsslDictionary, rsslDictEncBuffer);
            }

            if ((ret == CodecReturnCode.SUCCESS) || (ret == CodecReturnCode.DICT_PART_ENCODED))
            {
                dictCallbackClient.m_RefreshMsg ??= m_OmmBaseImpl.GetEmaObjManager().GetOmmRefreshMsg();
                dictCallbackClient.m_RefreshMsg.Decode((Eta.Codec.Buffer)rsslDictEncBuffer, Codec.MajorVersion(), Codec.MinorVersion(), null, null);

                if (ret == CodecReturnCode.SUCCESS)
                    dictCallbackClient.m_RefreshMsg.Complete(true);

                dictCallbackClient.ProcessRefreshMsg(dictCallbackClient.m_RefreshMsg, this);
            }

            if (ret == CodecReturnCode.DICT_PART_ENCODED)
            {
                m_OmmBaseImpl.TimeoutEventManager!.AddTimeoutEvent(PART_REFRESH_TIMEOUT, this);
                return;
            }

            if (ret != CodecReturnCode.SUCCESS)
            {
                IStatusMsg rsslStatusMsg = dictCallbackClient.StatusMsg();

                rsslStatusMsg.StreamId = StreamId;
                rsslStatusMsg.DomainType = (int)Eta.Rdm.DomainType.DICTIONARY;
                rsslStatusMsg.ContainerType = DataTypes.NO_DATA;
                rsslStatusMsg.ApplyHasState();
                rsslStatusMsg.State.StreamState(StreamStates.CLOSED);
                rsslStatusMsg.State.DataState(DataStates.SUSPECT);
                rsslStatusMsg.State.Code(StateCodes.NONE);
                rsslStatusMsg.State.Text().Data("Failed to provide data dictionary: Internal Error.");
                rsslStatusMsg.ApplyHasMsgKey();
                rsslStatusMsg.MsgKey.ApplyHasName();
                rsslStatusMsg.MsgKey.Name.Data(m_Name);

                if (m_EncodeIterator == null)
                    m_EncodeIterator = new EncodeIterator();
                else
                    m_EncodeIterator.Clear();

                CodecReturnCode retCode = m_EncodeIterator.SetBufferAndRWFVersion(rsslDictEncBuffer, Codec.MajorVersion(), Codec.MinorVersion());
                if (retCode != CodecReturnCode.SUCCESS)
                {
                    if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                    {
                        m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME,
                            "Internal Error. Failed to set encode iterator RWF version in DictionatyItem.HandleTimeoutEvent()");
                    }
                    return;
                }

                if ((retCode = rsslStatusMsg.Encode(m_EncodeIterator)) != CodecReturnCode.SUCCESS)
                {
                    if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                    {
                        m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME,
                            $"Internal Error. Failed to encode msg in DictionatyItem.HandleTimeoutEvent(): {retCode}");
                    }
                    return;
                }

                dictCallbackClient.m_StatusMsg ??= m_OmmBaseImpl.GetEmaObjManager().GetOmmStatusMsg();
                dictCallbackClient.m_StatusMsg.Decode(rsslDictEncBuffer, Codec.MajorVersion(), Codec.MinorVersion(), null, null);

                dictCallbackClient.ProcessStatusMsg(dictCallbackClient.m_StatusMsg, this);
                return;
            }
        }
        else
        {
            IStatusMsg rsslStatusMsg = dictCallbackClient.StatusMsg();

            rsslStatusMsg.StreamId = StreamId;
            rsslStatusMsg.DomainType = (int)Eta.Rdm.DomainType.DICTIONARY;
            rsslStatusMsg.ContainerType = DataTypes.NO_DATA;
            rsslStatusMsg.ApplyHasState();
            rsslStatusMsg.State.StreamState(StreamStates.CLOSED_RECOVER);
            rsslStatusMsg.State.DataState(DataStates.SUSPECT);
            rsslStatusMsg.State.Code(StateCodes.NONE);
            rsslStatusMsg.State.Text().Data("Data dictionary is not ready to provide.");

            if (m_EncodeIterator == null)
                m_EncodeIterator = new EncodeIterator();
            else
                m_EncodeIterator.Clear();

            CodecReturnCode retCode = m_EncodeIterator.SetBufferAndRWFVersion(rsslDictEncBuffer, Codec.MajorVersion(), Codec.MinorVersion());
            if (retCode != CodecReturnCode.SUCCESS)
            {
                if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                {
                    m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME,
                        "Internal Error. Failed to set encode iterator RWF version in DictionatyItem.HandleTimeoutEvent()");
                }
                return;
            }

            if ((retCode = rsslStatusMsg.Encode(m_EncodeIterator)) != CodecReturnCode.SUCCESS)
            {
                if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                {
                    m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME,
                        "Internal Error. Failed to encode msg in DictionatyItem.HandleTimeoutEvent()");
                }
                return;
            }

            dictCallbackClient.m_StatusMsg ??= m_OmmBaseImpl.GetEmaObjManager().GetOmmStatusMsg();
            dictCallbackClient.m_StatusMsg.Decode(rsslDictEncBuffer, Codec.MajorVersion(), Codec.MinorVersion(), null, null);

            dictCallbackClient.ProcessStatusMsg(dictCallbackClient.m_StatusMsg, this);

            return;
        }
    }

    public Eta.Common.MonitorWriteLocker UserLock
    {
        get => m_OmmBaseImpl.UserLock;
    }

    public int CurrentFid
    {
        get => m_CurrentFid;
    }

    public string Name
    {
        get => m_Name!;
    }

    public int RsslFilters
    {
        get => m_rsslFilter;
    }

    private CodecReturnCode EncodeDataDictionaryResp(bool firstMultiRefresh, DataDictionary rsslDataDictionary, LSEG.Eta.Codec.Buffer rsslDictEncBuffer)
    {
        DictionaryCallbackClient<T> dictCallbackClient = m_OmmBaseImpl.DictionaryCallbackClient!;

        if (m_EncodeIterator == null)
            m_EncodeIterator = new EncodeIterator();
        else
            m_EncodeIterator.Clear();

        CodecReturnCode retCode = m_EncodeIterator.SetBufferAndRWFVersion(rsslDictEncBuffer, Codec.MajorVersion(), Codec.MinorVersion());
        if (retCode != CodecReturnCode.SUCCESS)
            return retCode;

        IRefreshMsg rsslRefreshMsg = dictCallbackClient.RefreshMsg();

        rsslRefreshMsg.DomainType = (int)Eta.Rdm.DomainType.DICTIONARY;
        rsslRefreshMsg.ContainerType = DataTypes.SERIES;
        rsslRefreshMsg.State.StreamState(StreamStates.OPEN);
        rsslRefreshMsg.State.DataState(DataStates.OK);
        rsslRefreshMsg.State.Code(StateCodes.NONE);
        rsslRefreshMsg.ApplySolicited();
        rsslRefreshMsg.ApplyHasMsgKey();
        rsslRefreshMsg.MsgKey.Filter = m_rsslFilter;
        rsslRefreshMsg.MsgKey.ApplyHasFilter();

        if (firstMultiRefresh)
            rsslRefreshMsg.ApplyClearCache();

        rsslRefreshMsg.MsgKey.ApplyHasName();
        rsslRefreshMsg.MsgKey.Name.Data(m_Name);

        rsslRefreshMsg.StreamId = StreamId;

        Int rsslCurrentFid = dictCallbackClient.rsslCurrentFid();
        rsslCurrentFid.Value(m_CurrentFid);

        bool complete = false;

        if ((retCode = rsslRefreshMsg.EncodeInit(m_EncodeIterator, 0)) < CodecReturnCode.SUCCESS)
            return retCode;

        if (m_Name!.Equals(DictionaryCallbackClient<T>.DICTIONARY_RWFFID))
        {
            retCode = rsslDataDictionary.EncodeFieldDictionary(m_EncodeIterator, rsslCurrentFid, m_rsslFilter, out _);
        }
        else if (m_Name.Equals(DictionaryCallbackClient<T>.DICTIONARY_RWFENUM))
        {
            retCode = rsslDataDictionary.EncodeEnumTypeDictionaryAsMultiPart(m_EncodeIterator, rsslCurrentFid, m_rsslFilter, out _);
        }
        else
            return CodecReturnCode.FAILURE;

        if (retCode != CodecReturnCode.SUCCESS)
        {
            if (retCode == CodecReturnCode.DICT_PART_ENCODED)
                m_CurrentFid = (int)rsslCurrentFid.ToLong();
            else
                return retCode;
        }
        else
        {
            complete = true;
        }

        if ((retCode = rsslRefreshMsg.EncodeComplete(m_EncodeIterator, true)) < CodecReturnCode.SUCCESS)
            return retCode;

        return complete ? CodecReturnCode.SUCCESS : CodecReturnCode.DICT_PART_ENCODED;
    }
}

#endregion
