/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|              Copyright (C) 2025 LSEG. All rights reserved.                --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using LSEG.Eta.Common;
using LSEG.Eta.Rdm;
using LSEG.Eta.ValueAdd.Rdm;
using LSEG.Eta.ValueAdd.Reactor;
using Microsoft.Extensions.Primitives;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;
using Buffer = LSEG.Eta.Codec.Buffer;

namespace LSEG.Ema.Access;

internal interface IDirectoryServiceClient<T>
{
    void OnServiceAdd(SessionChannelInfo<T> sessionChannelInfo, ServiceDirectory<T> serviceDirectory);

    void OnServiceDelete(SessionChannelInfo<T> sessionChannelInfo, ServiceDirectory<T> serviceDirectory);

    void OnServiceStateChange(SessionChannelInfo<T> sessionChannelInfo, ServiceDirectory<T> serviceDirectory, ServiceState serviceState);

    void OnServiceInfoChange(SessionChannelInfo<T> sessionChanelInfo, ServiceDirectory<T> serviceDirectory, ServiceInfo serviceInfo);
}

internal class ConsumerSessionTimeOut<T> : ITimeoutClient
{
    private ConsumerSession<T> m_ConsumerSession;
    private bool m_InstalledTimeout = false;

    public ConsumerSessionTimeOut(ConsumerSession<T> consumerSession)
    {
        m_ConsumerSession = consumerSession;
    }

    /* timeout is in microsecond */
    public void InstallTimeout(long timeout)
    {
        if (m_InstalledTimeout) return;

        m_ConsumerSession.OmmBaseImpl().TimeoutEventManager!.AddTimeoutEvent(timeout, this);

        m_InstalledTimeout = true;
    }

    public void HandleTimeoutEvent()
    {
        m_InstalledTimeout = false;
        m_ConsumerSession.Dispatch();
    }
}

internal class ScheduleCloseSessionChannel<T> : ITimeoutClient
{
    private ConsumerSession<T>? m_ConsumerSession;
    private SessionChannelInfo<T>? m_SessionChannelInfo;
    private ChannelInfo m_ChannelInfo;

    public ScheduleCloseSessionChannel(ConsumerSession<T> consumerSession, SessionChannelInfo<T> sessionChannelInfo, ChannelInfo channelInfo)
    {
        m_ConsumerSession= consumerSession;
        m_SessionChannelInfo= sessionChannelInfo;
        m_ChannelInfo= channelInfo;
    }

    public void HandleTimeoutEvent()
    {
        if(m_SessionChannelInfo != null && m_ConsumerSession != null)
        {
            /* Keep the current size before SessionChannelInfo is removed from CloseSessionChannel() */
            int size = m_ConsumerSession.SessionChannelList.Count;

            /* Closes ReactorChannel only and removes SessionChannelInfo later below */
            m_ConsumerSession.OmmBaseImpl().CloseSessionChannelOnly((dynamic)m_SessionChannelInfo);

            m_ConsumerSession.HandleLoginStreamForChannelDown(m_ConsumerSession.OmmBaseImpl().LoginCallbackClient!.LoginItems, m_SessionChannelInfo.ReactorChannel!, size);

            m_ConsumerSession.OmmBaseImpl().ChannelCallbackClient!.RemoveSessionChannel((dynamic)m_SessionChannelInfo);

            m_SessionChannelInfo.OnChannelClose(m_ChannelInfo);

            m_SessionChannelInfo = null;
        }

        m_ConsumerSession = null;
    }
}

internal class ConsumerSession<T> : IDirectoryServiceClient<T>
{
    private const int INIT_DIRECTORY_MSG_BUFFER_SIZE = 8192;
    private static readonly string CLIENT_NAME = "ConsumerSession";

    // Login section
    private OmmBaseImpl<T> m_OmmBaseImpl;
    private List<SessionChannelInfo<IOmmConsumerClient>> m_SessionChannelList;
    private List<SessionChannelInfo<IOmmConsumerClient>> m_TempActiveSessionChannelList;
    private int m_NumOfSessionChannelCount;
    private int m_NumOfLoginOk = 0;
    private int m_NumOfLoginClose = 0;
    private int m_NumOfDirectoryOk = 0;
    private LoginMsg m_LoginRefresh; // login refresh aggregation for ConsumerSession

    private int m_generateServiceId; // This is used to generate an unique service Id for a concrete service name.

    private Dictionary<string, SessionDirectory<T>> m_SessionDirByName;

    private Dictionary<int, SessionDirectory<T>> m_SessionDirById;

    private HashSet<SessionDirectory<T>> m_RemovedSessionDirSet; // This keeps the list of deleted SessionDirectory

    private Eta.Codec.Buffer? m_EncBuffer;

    private ConsumerConfig m_ConsumerConfig;

    private State m_State; /* This is used to set the state of login status message for the login stream */

    private Eta.Codec.Msg m_EtaStatusMsg; /* This is used to set the state of login status message for the login stream */

    private StatusMsg m_StatusMsg;  /* This is used to set login status message for the login stream */

    private OmmEventImpl<T> m_EventImpl;

    private bool m_SendDirectoryResp;

    private DirectoryMsg? m_DirectoryMsg;

    private EncodeIterator m_EncIter = new();

    private readonly ConsumerSessionTimeOut<T> m_DispatchTimeout;

    private DictionaryConfig m_DictionaryConfig;

    /* This is used to download data dictionary from network */
    private ChannelDictionary<T>? m_ChannelDictionary;

    internal Dictionary<string, ServiceList>? ServiceListDict;

    internal OmmConsumerConfigImpl ConsumerConfigImpl { get; set; }

    /* This is used to indicate whether the ConsumerSession instance is initialized and ready to use. */
    private bool _isConsumerSessionInitialized = false;

    /* Keeps a list of mismatch service names between session channels that need to be handled */
    private HashSet<string>? _mismatchServiceSet;

    internal ConsumerSession(OmmBaseImpl<T> baseImpl)
    {
        m_OmmBaseImpl = baseImpl;
        m_OmmBaseImpl.ConsumerSession = this;
        m_SessionChannelList = new List<SessionChannelInfo<IOmmConsumerClient>>();

        m_TempActiveSessionChannelList = new List<SessionChannelInfo<IOmmConsumerClient>>();

        m_LoginRefresh = new();
        m_LoginRefresh.LoginMsgType = LoginMsgType.REFRESH;

        ConsumerConfigImpl = ((OmmConsumerConfigImpl)m_OmmBaseImpl.OmmConfigBaseImpl);

        m_DictionaryConfig = ConsumerConfigImpl.DictionaryConfig;

        m_ConsumerConfig = ConsumerConfigImpl.ConsumerConfig;

        int initialHashSize = (int)(ConsumerConfigImpl.ConsumerConfig.ServiceCountHint / 0.75 + 1);

        m_SessionDirByName = new Dictionary<string, SessionDirectory<T>>(initialHashSize);

        m_SessionDirById = new Dictionary<int, SessionDirectory<T>>(initialHashSize);

        m_RemovedSessionDirSet = new HashSet<SessionDirectory<T>>();

        SessionWatchlist = new SessionWatchlist<T>(this, Utilities.Convert_uint_int(m_ConsumerConfig.ItemCountHint));

        m_generateServiceId = 32766;

        m_State = new State();

        m_EtaStatusMsg = new Eta.Codec.Msg();

        m_StatusMsg = new StatusMsg();

        m_EventImpl = new OmmEventImpl<T>(baseImpl);

        m_DispatchTimeout = new ConsumerSessionTimeOut<T>(this);

        ServiceListDict = ConsumerConfigImpl.ServiceListDict;

        m_OmmBaseImpl.ServiceListDict = ServiceListDict;

        /* Generate an unique service ID for ServiceList if any */
        GenerateServiceIdForServiceMap();
    }

    private void GenerateServiceIdForServiceMap()
    {
        if(ServiceListDict != null)
        {
            foreach(var entry in ServiceListDict.Values)
            {
                entry.ServiceId = ++m_generateServiceId;
            }
        }
    }

    internal SessionWatchlist<T> SessionWatchlist { get; private set; }

    internal OmmBaseImpl<T> OmmBaseImpl()
    {
        return m_OmmBaseImpl;
    }

    internal long ImplState => m_OmmBaseImpl.ImplState;

    internal void AddSessionChannelInfo(SessionChannelInfo<IOmmConsumerClient> sessionChannelInfo)
    {
        if (!m_SessionChannelList.Contains(sessionChannelInfo))
        {
            m_SessionChannelList.Add(sessionChannelInfo);
            m_NumOfSessionChannelCount++;
        }
    }

    internal void RemoveSessionChannelInfo(SessionChannelInfo<IOmmConsumerClient> sessionChannelInfo)
    {
        if (m_SessionChannelList.Contains(sessionChannelInfo))
        {
            m_SessionChannelList.Remove(sessionChannelInfo);
            m_NumOfSessionChannelCount--;
        }
    }

    public List<SessionChannelInfo<IOmmConsumerClient>> SessionChannelList => m_SessionChannelList;

    public bool SendInitialLoginRefresh { get; internal set; } = false;

    internal int NumOfSessionChannelCount()
    {
        return m_NumOfSessionChannelCount;
    }

    internal void IncreaseNumOfLoginOk()
    {
        m_NumOfLoginOk++;
    }

    internal int NumOfLoginOk()
    {
        return m_NumOfLoginOk;
    }

    internal void IncreaseNumOfLoginClose()
    {
        m_NumOfLoginClose++;
    }

    internal int NumOfLoginClose()
    {
        return m_NumOfLoginClose;
    }

    void IncreaseNumOfDirectoryOk()
    {
        m_NumOfDirectoryOk++;
    }

    int NumOfDirectoryOk()
    {
        return m_NumOfDirectoryOk;
    }

    private List<SessionChannelInfo<IOmmConsumerClient>> ActiveSessionChannelList()
    {
        m_TempActiveSessionChannelList.Clear();

        SessionChannelInfo<IOmmConsumerClient> sessionChannelInfo;
        for(int index = 0; index < m_SessionChannelList.Count; index++)
        {
            sessionChannelInfo = m_SessionChannelList[index];

            if(sessionChannelInfo.ReactorChannel != null)
            {
                if(sessionChannelInfo.ReactorChannel.State == ReactorChannelState.UP ||
                    sessionChannelInfo.ReactorChannel.State == ReactorChannelState.READY)
                {
                    m_TempActiveSessionChannelList.Add(sessionChannelInfo);
                }
            }
        }

        return m_TempActiveSessionChannelList;
    }

    public SessionChannelInfo<IOmmConsumerClient>? AggregateLoginResponse()
    {
        List<SessionChannelInfo<IOmmConsumerClient>> activeSessionChannelList = ActiveSessionChannelList();

        if (activeSessionChannelList.Count == 0)
            return null;

        m_LoginRefresh.Clear();
        m_LoginRefresh.LoginMsgType = LoginMsgType.REFRESH;
        SessionChannelInfo<IOmmConsumerClient>? firstSessionChannel = null;

        LoginAttribFlags originalAttribFlags = LoginAttribFlags.NONE;
        LoginSupportFeaturesFlags originalFeaturesFlags = LoginSupportFeaturesFlags.NONE;

        LoginAttribFlags attribFlags = LoginAttribFlags.NONE;
        LoginSupportFeaturesFlags featuresFlags = LoginSupportFeaturesFlags.NONE;

        for(int index = 0;  index < activeSessionChannelList.Count; index++)
        {
            SessionChannelInfo<IOmmConsumerClient> sessionChannel = activeSessionChannelList[index];

            if(sessionChannel.LoginRefresh().Flags > 0)
            {
                if(firstSessionChannel == null)
                {
                    originalAttribFlags = attribFlags = sessionChannel.LoginRefresh().LoginAttrib.Flags;
                    originalFeaturesFlags = featuresFlags = sessionChannel.LoginRefresh().SupportedFeatures.Flags;

                    firstSessionChannel = sessionChannel;
                }
                else
                {
                    attribFlags &= sessionChannel.LoginRefresh().LoginAttrib.Flags;
                    featuresFlags &= sessionChannel.LoginRefresh().SupportedFeatures.Flags;
                }
            }
        }

        if (firstSessionChannel != null)
        {
            firstSessionChannel.LoginRefresh().LoginAttrib.Flags = attribFlags;
            firstSessionChannel.LoginRefresh().SupportedFeatures.Flags = featuresFlags;

            /* Aggregate login response attributes and features from all session channels */
            for (int index = 1; index < activeSessionChannelList.Count; index++)
            {
                SessionChannelInfo<IOmmConsumerClient> sessionChannel = activeSessionChannelList[index];

                if ((attribFlags & LoginAttribFlags.HAS_ALLOW_SUSPECT_DATA) != 0)
                {
                    firstSessionChannel.LoginRefresh().LoginAttrib.AllowSuspectData &= sessionChannel.LoginRefresh().LoginAttrib.AllowSuspectData;
                }

                if ((attribFlags & LoginAttribFlags.HAS_PROVIDE_PERM_EXPR) != 0)
                {
                    firstSessionChannel.LoginRefresh().LoginAttrib.ProvidePermissionExpressions &= sessionChannel.LoginRefresh().LoginAttrib.ProvidePermissionExpressions;
                }

                if ((attribFlags & LoginAttribFlags.HAS_PROVIDE_PERM_PROFILE) != 0)
                {
                    firstSessionChannel.LoginRefresh().LoginAttrib.ProvidePermissionProfile &= sessionChannel.LoginRefresh().LoginAttrib.ProvidePermissionProfile;
                }

                if ((attribFlags & LoginAttribFlags.HAS_CONSUMER_SUPPORT_RTT) != 0)
                {
                    firstSessionChannel.LoginRefresh().LoginAttrib.SupportConsumerRTTMonitoring &= sessionChannel.LoginRefresh().LoginAttrib.SupportConsumerRTTMonitoring;
                }

                if ((featuresFlags & LoginSupportFeaturesFlags.HAS_SUPPORT_BATCH_REQUESTS) != 0)
                {
                    firstSessionChannel.LoginRefresh().SupportedFeatures.SupportBatchRequests &= sessionChannel.LoginRefresh().SupportedFeatures.SupportBatchRequests;
                }

                if ((featuresFlags & LoginSupportFeaturesFlags.HAS_SUPPORT_POST) != 0)
                {
                    firstSessionChannel.LoginRefresh().SupportedFeatures.SupportOMMPost &= sessionChannel.LoginRefresh().SupportedFeatures.SupportOMMPost;
                }

                if ((featuresFlags & LoginSupportFeaturesFlags.HAS_SUPPORT_OPT_PAUSE) != 0)
                {
                    firstSessionChannel.LoginRefresh().SupportedFeatures.SupportOptimizedPauseResume &= sessionChannel.LoginRefresh().SupportedFeatures.SupportOptimizedPauseResume;
                }

                if ((featuresFlags & LoginSupportFeaturesFlags.HAS_SUPPORT_VIEW) != 0)
                {
                    firstSessionChannel.LoginRefresh().SupportedFeatures.SupportViewRequests &= sessionChannel.LoginRefresh().SupportedFeatures.SupportViewRequests;
                }

                if ((featuresFlags & LoginSupportFeaturesFlags.HAS_SUPPORT_BATCH_REISSUES) != 0)
                {
                    firstSessionChannel.LoginRefresh().SupportedFeatures.SupportBatchReissues &= sessionChannel.LoginRefresh().SupportedFeatures.SupportBatchReissues;
                }

                if ((featuresFlags & LoginSupportFeaturesFlags.HAS_SUPPORT_BATCH_CLOSES) != 0)
                {
                    firstSessionChannel.LoginRefresh().SupportedFeatures.SupportBatchCloses &= sessionChannel.LoginRefresh().SupportedFeatures.SupportBatchCloses;
                }

                if ((featuresFlags & LoginSupportFeaturesFlags.HAS_SUPPORT_ENH_SL) != 0)
                {
                    firstSessionChannel.LoginRefresh().SupportedFeatures.SupportEnhancedSymbolList &= sessionChannel.LoginRefresh().SupportedFeatures.SupportEnhancedSymbolList;
                }
            }

            /* Makes sure that only supported attributes and features are provided to users */
            attribFlags &= (LoginAttribFlags.HAS_ALLOW_SUSPECT_DATA | LoginAttribFlags.HAS_PROVIDE_PERM_EXPR | LoginAttribFlags.HAS_PROVIDE_PERM_PROFILE | LoginAttribFlags.HAS_CONSUMER_SUPPORT_RTT);
            featuresFlags &= (LoginSupportFeaturesFlags.HAS_SUPPORT_BATCH_REQUESTS | LoginSupportFeaturesFlags.HAS_SUPPORT_POST | LoginSupportFeaturesFlags.HAS_SUPPORT_OPT_PAUSE |
                    LoginSupportFeaturesFlags.HAS_SUPPORT_VIEW | LoginSupportFeaturesFlags.HAS_SUPPORT_BATCH_REISSUES | LoginSupportFeaturesFlags.HAS_SUPPORT_BATCH_CLOSES | LoginSupportFeaturesFlags.HAS_SUPPORT_ENH_SL);

            /* Always enables the single open and allow suspect data flags */
            attribFlags |= (LoginAttribFlags.HAS_SINGLE_OPEN | LoginAttribFlags.HAS_ALLOW_SUSPECT_DATA);
            firstSessionChannel.LoginRefresh().LoginAttrib.Flags = attribFlags;
            firstSessionChannel.LoginRefresh().SupportedFeatures.Flags = featuresFlags;

            /* Copy to the aggregated login refresh message */
            firstSessionChannel.LoginRefresh().Copy(m_LoginRefresh.LoginRefresh!);

            /* Always enable the single open feature */
            m_LoginRefresh.LoginRefresh!.LoginAttrib.AllowSuspectData = 1;
            m_LoginRefresh.LoginRefresh!.LoginAttrib.SingleOpen = 1;

            firstSessionChannel.LoginRefresh().LoginAttrib.Flags = originalAttribFlags;
            firstSessionChannel.LoginRefresh().SupportedFeatures.Flags = originalFeaturesFlags;
        }

        return firstSessionChannel;
    }

    public bool SupportSingleOpen()
    {
        /* SingleOpen feature is always enabled */
        return true;
    }

    public LoginMsg LoginRefresh()
    {
        return m_LoginRefresh;
    }

    void RestoreServiceFlags()
    {
        foreach (var sessionDirectory in m_SessionDirByName.Values)
        {
            sessionDirectory.RestoreServiceFlags();
        }
    }

    void ResetUpdateFlags()
    {
        foreach(var sessionDirectory in m_SessionDirByName.Values)
        {
            sessionDirectory.ResetUpdateFlags();
        }
    }

    public void ProcessDirectoryPayload(List<Service> serviceList, ReactorChannel? reactorChannel)
    {
        m_SendDirectoryResp = false;

        ResetUpdateFlags();

        ChannelInfo? channelInfo = (ChannelInfo?)reactorChannel?.UserSpecObj;
        if (channelInfo != null)
        {
            SessionChannelInfo<IOmmConsumerClient>? sessionChannelInfo = channelInfo.SessionChannelInfo;

            sessionChannelInfo?.ProcessDirectoryPayload(serviceList, channelInfo);
        }
    }

    /* This is used to check and close a ReactorChannel which doesn't provide a login response in time for the HandleLoginReqTimeout() method */
    public void CheckLoginResponseAndCloseReactorChannel()
    {
        /* Closes channels if it doesn't receive a login response */
        if ((m_NumOfLoginOk + m_NumOfLoginClose) < m_SessionChannelList.Count)
        {
            List<SessionChannelInfo<IOmmConsumerClient>> removeChannelList = new List<SessionChannelInfo<IOmmConsumerClient>>();

            for (int i = 0; i < m_SessionChannelList.Count; i++)
            {
                SessionChannelInfo<IOmmConsumerClient> sessionChannelInfo = m_SessionChannelList[i];

                if (!sessionChannelInfo.ReceivedLoginRefresh)
                {
                    removeChannelList.Add(sessionChannelInfo);
                }
            }

            for (int index = removeChannelList.Count - 1; index >= 0; index--)
            {
                m_OmmBaseImpl.CloseSessionChannel(removeChannelList[index]);
            }
        }
    }

    public bool CheckAllSessionChannelHasState(long state)
    {
        long result = state;
        for(int i = 0;i < m_SessionChannelList.Count; i++)
        {
            SessionChannelInfo<IOmmConsumerClient> sessionChannelInfo = m_SessionChannelList[i];

            result &= sessionChannelInfo.State;
        }

        return result == state;
    }

    public bool CheckAtLeastOneSessionChannelHasState(long state)
    {
        for (int i = 0; i < m_SessionChannelList.Count; i++)
        {
            SessionChannelInfo<IOmmConsumerClient> sessionChannelInfo = m_SessionChannelList[i];

            if ((sessionChannelInfo.State & state) == state)
                return true;
        }

        return false;
    }

    public void HandleLoginReqTimeout()
    {
        if(m_ConsumerConfig.LoginRequestTimeOut == 0)
        {
            while(ImplState < OmmBaseImpl<T>.OmmImplState.LOGIN_STREAM_OPEN_OK && ImplState != OmmBaseImpl<T>.OmmImplState.CHANNEL_UP_STREAM_NOT_OPEN
                && ImplState != OmmBaseImpl<T>.OmmImplState.CHANNEL_CLOSED)
            {
                m_OmmBaseImpl.ReactorDispatchLoop(m_ConsumerConfig.DispatchTimeoutApiThread, m_ConsumerConfig.MaxDispatchCountApiThread);
            }

            /* Throws OmmInvalidUsageException when EMA receives login reject from the data source. */
            if(ImplState == OmmBaseImpl<T>.OmmImplState.CHANNEL_UP_STREAM_NOT_OPEN)
            {
                throw new OmmInvalidUsageException(m_OmmBaseImpl.LoginCallbackClient!.LoginFailureMsg, OmmInvalidUsageException.ErrorCodes.LOGIN_REQUEST_REJECTED);
            }

            if(m_NumOfLoginOk > 0 && SendInitialLoginRefresh == false)
            {
                SessionChannelInfo<IOmmConsumerClient>? sesionChannelInfo = AggregateLoginResponse();

                m_OmmBaseImpl.LoginCallbackClient!.ProcessRefreshMsg(null, sesionChannelInfo!.ReactorChannel!, LoginRefresh());

                SendInitialLoginRefresh = true;

                CheckLoginResponseAndCloseReactorChannel();

                return;
            }
            else
            {
                StringBuilder strBuilder = m_OmmBaseImpl.GetStrBuilder().Append("login failed (timed out after waiting ").Append(m_ConsumerConfig.LoginRequestTimeOut)
                    .Append(" milliseconds) for ");
                int count = m_ConsumerConfig.SessionChannelSet.Count;
                foreach(string configName in m_ConsumerConfig.SessionChannelSet)
                {
                    if(--count > 0)
                        strBuilder.Append(configName + ", ");
                    else
                        strBuilder.Append(configName);
                }

                string excepText = strBuilder.ToString();

                if(m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                {
                    m_OmmBaseImpl.LoggerClient.Error(m_OmmBaseImpl.InstanceName, excepText);
                }

                throw new OmmInvalidUsageException(excepText, OmmInvalidUsageException.ErrorCodes.LOGIN_REQUEST_TIME_OUT);
            }
        }
        else
        {
            m_OmmBaseImpl.ResetTimeoutEvent();
            TimeoutEvent timeoutEvent = m_OmmBaseImpl.TimeoutEventManager!.AddTimeoutEvent(m_ConsumerConfig.LoginRequestTimeOut * 1000, m_OmmBaseImpl);

            while(!m_OmmBaseImpl.IsEventTimeout && (ImplState < OmmBaseImpl<T>.OmmImplState.LOGIN_STREAM_OPEN_OK) && (ImplState != OmmBaseImpl<T>.OmmImplState.CHANNEL_UP_STREAM_NOT_OPEN)
                && (ImplState != OmmBaseImpl<T>.OmmImplState.CHANNEL_CLOSED))
            {
                m_OmmBaseImpl.ReactorDispatchLoop(m_ConsumerConfig.DispatchTimeoutApiThread, m_ConsumerConfig.MaxDispatchCountApiThread);
            }

            if(m_OmmBaseImpl.IsEventTimeout)
            {
                if (m_NumOfLoginOk > 0 && SendInitialLoginRefresh == false)
                {
                    SessionChannelInfo<IOmmConsumerClient>? sesionChannelInfo = AggregateLoginResponse();

                    m_OmmBaseImpl.LoginCallbackClient!.ProcessRefreshMsg(null, sesionChannelInfo!.ReactorChannel!, LoginRefresh());

                    SendInitialLoginRefresh = true;

                    CheckLoginResponseAndCloseReactorChannel();

                    return;
                }

                StringBuilder strBuilder = m_OmmBaseImpl.GetStrBuilder().Append("login failed (timed out after waiting ").Append(m_ConsumerConfig.LoginRequestTimeOut)
                   .Append(" milliseconds) for ");
                int count = m_ConsumerConfig.SessionChannelSet.Count;
                foreach (string configName in m_ConsumerConfig.SessionChannelSet)
                {
                    if (--count > 0)
                        strBuilder.Append(configName + ", ");
                    else
                        strBuilder.Append(configName);
                }

                string excepText = strBuilder.ToString();

                if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                {
                    m_OmmBaseImpl.LoggerClient.Error(m_OmmBaseImpl.InstanceName, excepText);
                }

                throw new OmmInvalidUsageException(excepText, OmmInvalidUsageException.ErrorCodes.LOGIN_REQUEST_TIME_OUT);
            }
            else if (ImplState == OmmBaseImpl<T>.OmmImplState.CHANNEL_CLOSED) /* Throws OmmInvalidUsageException when all session channels are down. */
            {
                timeoutEvent.Cancel();

                StringBuilder strBuilder = m_OmmBaseImpl.GetStrBuilder().Append("login failed (timed out after waiting ").Append(m_ConsumerConfig.LoginRequestTimeOut)
                   .Append(" milliseconds) for ");
                int count = m_ConsumerConfig.SessionChannelSet.Count;
                foreach (string configName in m_ConsumerConfig.SessionChannelSet)
                {
                    if (--count > 0)
                        strBuilder.Append(configName + ", ");
                    else
                        strBuilder.Append(configName);
                }

                string excepText = strBuilder.ToString();

                if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                {
                    m_OmmBaseImpl.LoggerClient.Error(m_OmmBaseImpl.InstanceName, excepText);
                }

                throw new OmmInvalidUsageException(excepText, OmmInvalidUsageException.ErrorCodes.LOGIN_REQUEST_TIME_OUT);
            }
            else if(ImplState == OmmBaseImpl<T>.OmmImplState.CHANNEL_UP_STREAM_NOT_OPEN) /* Throws OmmInvalidUsageException when EMA receives login reject from the data source for all session channels. */
            {
                timeoutEvent.Cancel();

                throw new OmmInvalidUsageException(m_OmmBaseImpl.LoginCallbackClient!.LoginFailureMsg, OmmInvalidUsageException.ErrorCodes.LOGIN_REQUEST_REJECTED);
            }
            else
            {
                timeoutEvent.Cancel();

                /* This is used to notify login refresh after receiving source directory response */
                if (m_NumOfLoginOk > 0 && SendInitialLoginRefresh == false)
                {
                    SessionChannelInfo<IOmmConsumerClient>? sesionChannelInfo = AggregateLoginResponse();

                    m_OmmBaseImpl.LoginCallbackClient!.ProcessRefreshMsg(null, sesionChannelInfo!.ReactorChannel!, LoginRefresh());

                    SendInitialLoginRefresh = true;

                    CheckLoginResponseAndCloseReactorChannel();

                    return;
                }
            }
        }
    }

    internal void HandleDirectoryReqTimeout()
    {
        if (m_ConsumerConfig.DirectoryRequestTimeOut == 0)
        {
            while (ImplState < OmmBaseImpl<T>.OmmImplState.DIRECTORY_STREAM_OPEN_OK)
            {
                m_OmmBaseImpl.ReactorDispatchLoop(m_ConsumerConfig.DispatchTimeoutApiThread, m_ConsumerConfig.MaxDispatchCountApiThread);

                if(CheckAllSessionChannelHasState(OmmBaseImpl<T>.OmmImplState.DIRECTORY_STREAM_OPEN_OK))
                {
                    m_OmmBaseImpl.SetOmmImplState(OmmBaseImpl<T>.OmmImplState.DIRECTORY_STREAM_OPEN_OK);
                }
            }
        }
        else
        {
            if (CheckAllSessionChannelHasState(OmmBaseImpl<T>.OmmImplState.DIRECTORY_STREAM_OPEN_OK))
            {
                m_OmmBaseImpl.SetOmmImplState(OmmBaseImpl<T>.OmmImplState.DIRECTORY_STREAM_OPEN_OK);
                return;
            }

            m_OmmBaseImpl.ResetTimeoutEvent();
            TimeoutEvent timeoutEvent = m_OmmBaseImpl.TimeoutEventManager!.AddTimeoutEvent(m_ConsumerConfig.DirectoryRequestTimeOut * 1000, m_OmmBaseImpl);

            while (!m_OmmBaseImpl.IsEventTimeout && (ImplState < OmmBaseImpl<T>.OmmImplState.DIRECTORY_STREAM_OPEN_OK))
            {
                m_OmmBaseImpl.ReactorDispatchLoop(m_ConsumerConfig.DispatchTimeoutApiThread, m_ConsumerConfig.MaxDispatchCountApiThread);

                if (CheckAllSessionChannelHasState(OmmBaseImpl<T>.OmmImplState.DIRECTORY_STREAM_OPEN_OK))
                {
                    m_OmmBaseImpl.SetOmmImplState(OmmBaseImpl<T>.OmmImplState.DIRECTORY_STREAM_OPEN_OK);
                }
            }

            if(m_OmmBaseImpl.IsEventTimeout)
            {
                if (CheckAtLeastOneSessionChannelHasState(OmmBaseImpl<T>.OmmImplState.DIRECTORY_STREAM_OPEN_OK))
                {
                    m_OmmBaseImpl.SetOmmImplState(OmmBaseImpl<T>.OmmImplState.DIRECTORY_STREAM_OPEN_OK);
                    return;
                }

                StringBuilder strBuilder = m_OmmBaseImpl.GetStrBuilder().Append("directory failed (timed out after waiting ").Append(m_ConsumerConfig.DirectoryRequestTimeOut)
                   .Append(" milliseconds) for ");
                int count = m_ConsumerConfig.SessionChannelSet.Count;
                foreach (string configName in m_ConsumerConfig.SessionChannelSet)
                {
                    if (--count > 0)
                        strBuilder.Append(configName + ", ");
                    else
                        strBuilder.Append(configName);
                }

                string excepText = strBuilder.ToString();

                if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                {
                    m_OmmBaseImpl.LoggerClient.Error(m_OmmBaseImpl.InstanceName, excepText);
                }

                throw new OmmInvalidUsageException(excepText, OmmInvalidUsageException.ErrorCodes.DIRECTORY_REQUEST_TIME_OUT);
            }
            else
            {
                timeoutEvent.Cancel();
            }
        }
    }

    internal void HandleDictionaryReqTimeout()
    {
        if (m_ConsumerConfig.DictionaryRequestTimeOut == 0)
        {
            while (!m_OmmBaseImpl.DictionaryCallbackClient!.IsDictionaryReady)
            {
                m_OmmBaseImpl.ReactorDispatchLoop(m_ConsumerConfig.DispatchTimeoutApiThread, m_ConsumerConfig.MaxDispatchCountApiThread);
            }
        }
        else
        {
            m_OmmBaseImpl.ResetTimeoutEvent();
            TimeoutEvent timeoutEvent = m_OmmBaseImpl.TimeoutEventManager!.AddTimeoutEvent(m_ConsumerConfig.DictionaryRequestTimeOut * 1000, m_OmmBaseImpl);

            while (!m_OmmBaseImpl.IsEventTimeout && !m_OmmBaseImpl.DictionaryCallbackClient!.IsDictionaryReady)
            {
                m_OmmBaseImpl.ReactorDispatchLoop(m_ConsumerConfig.DispatchTimeoutApiThread, m_ConsumerConfig.MaxDispatchCountApiThread);
            }

            if(m_OmmBaseImpl.IsEventTimeout)
            {
                StringBuilder strBuilder = m_OmmBaseImpl.GetStrBuilder().Append("dictionary failed (timed out after waiting ").Append(m_ConsumerConfig.DictionaryRequestTimeOut)
                   .Append(" milliseconds) for ");
                int count = m_ConsumerConfig.SessionChannelSet.Count;
                foreach (string configName in m_ConsumerConfig.SessionChannelSet)
                {
                    if (--count > 0)
                        strBuilder.Append(configName + ", ");
                    else
                        strBuilder.Append(configName);
                }

                string excepText = strBuilder.ToString();

                if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                {
                    m_OmmBaseImpl.LoggerClient.Error(m_OmmBaseImpl.InstanceName, excepText);
                }

                throw new OmmInvalidUsageException(excepText, OmmInvalidUsageException.ErrorCodes.DICTIONARY_REQUEST_TIME_OUT);
            }
            else
            {
                timeoutEvent.Cancel();
            }
        }
    }

    /* This is used to reorder all SessionChannelInfo of every SessionDirectory according to the session's connection list */
    internal void ReorderSessionChannelInfoForSessionDirectory()
    {
        if (_isConsumerSessionInitialized)
            return;

        if (_mismatchServiceSet != null)
        {
            /* Removed the session channel which has mismatch service names */
            foreach (var serviceName in _mismatchServiceSet)
            {
                m_SessionDirByName.TryGetValue(serviceName, out SessionDirectory<T>? sessionDirectory);
                if (sessionDirectory != null && sessionDirectory.Service != null)
                {
                    int generatedServiceId = sessionDirectory.Service.ServiceId;
                    bool isFirstSessionChannel = false;
                    for (int index = 0; index < m_SessionChannelList.Count;)
                    {
                        var sessionChanelInfo = m_SessionChannelList[index];
                        var directory = sessionChanelInfo.GetDirectoryByName(sessionDirectory.ServiceName);
                        if (directory != null && directory.Service != null)
                        {
                            if (!isFirstSessionChannel)
                            {
                                /* Update the service from the first SessionChannelInfo which has the service name */
                                directory.Service.Copy(sessionDirectory.Service);
                                sessionDirectory.Service.ServiceId = generatedServiceId;
                                directory.GeneratedServiceId(generatedServiceId);
                                sessionDirectory.SessionChannelInfoList.Clear();
                                sessionDirectory.SessionChannelInfoList.Add(sessionChanelInfo);
                                isFirstSessionChannel = true;
                            }
                            else
                            {
                                bool result = CompareServiceForAggregation(sessionDirectory, (dynamic)directory);

                                if (result == false)
                                {
                                    if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                                    {
                                        StringBuilder temp = m_OmmBaseImpl.GetStrBuilder();

                                        temp.Append($"Failed to compare service name {directory.ServiceName} for aggregation, closing session channel: {sessionChanelInfo}")
                                            .Append(ILoggerClient.CR).Append($"Instance Name {m_OmmBaseImpl.InstanceName}");

                                        m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, temp.ToString());
                                    }

                                    /* Closes this SessionChannelInfo from the consumer session */
                                    new ScheduleCloseSessionChannel<T>(this, (dynamic)sessionChanelInfo, (dynamic)directory.ChannelInfo!).HandleTimeoutEvent();

                                    /* Don't increase the index as this session channel is removed from the list */
                                    continue;
                                }
                                else
                                {
                                    directory.GeneratedServiceId(generatedServiceId);
                                    sessionDirectory.SessionChannelInfoList.Add(sessionChanelInfo);
                                }
                            }
                        }

                        /* Increase the index to the next session channel info if any*/
                        ++index;
                    }
                }
            }

            _mismatchServiceSet = null;
        }

        var sessionDirIt = m_SessionDirByName.Values.GetEnumerator();

        List<SessionChannelInfo<IOmmConsumerClient>> tempChannelInfoList;

        while (sessionDirIt.MoveNext())
        {
            SessionDirectory<T> sessionDir = sessionDirIt.Current;

            IEnumerator<SessionChannelInfo<IOmmConsumerClient>> sessionChannelInfoIt;

            tempChannelInfoList = new List<SessionChannelInfo<IOmmConsumerClient>>();

            foreach (var channelInfo in m_SessionChannelList)
            {
                sessionChannelInfoIt = sessionDir.SessionChannelInfoList.GetEnumerator();
                while (sessionChannelInfoIt.MoveNext())
                {
                    SessionChannelInfo<IOmmConsumerClient> sessionChannelInfo = sessionChannelInfoIt.Current;

                    if (ReferenceEquals(channelInfo,sessionChannelInfo))
                    {
                        tempChannelInfoList.Add(sessionChannelInfo);
                        break;
                    }
                }

            }

            /* Replace with the ordered list according to the connection list */
            sessionDir.SessionChannelInfoList.Clear();
            sessionDir.SessionChannelInfoList = tempChannelInfoList;
        }

        _isConsumerSessionInitialized = true; // Indicates that the ConsumerSession is initialized.
    }

    private void PopulateStatusMsg()
    {
        m_EtaStatusMsg.Clear();
        m_EtaStatusMsg.MsgClass = MsgClasses.STATUS;
        m_EtaStatusMsg.StreamId = 1;
        m_EtaStatusMsg.DomainType = (int)DomainType.LOGIN;
        m_EtaStatusMsg.ApplyHasMsgKey();
        MsgKey msgKey = m_EtaStatusMsg.MsgKey;
        msgKey.Clear();

        if(m_LoginRefresh.LoginRefresh!.HasUserNameType)
        {
            msgKey.ApplyHasNameType();
            msgKey.NameType = (int)m_LoginRefresh.LoginRefresh.UserNameType;
        }

        if(m_LoginRefresh.LoginRefresh!.HasUserName)
        {
            msgKey.ApplyHasName();
            msgKey.Name = m_LoginRefresh.LoginRefresh.UserName;
        }
    }

    internal void HandleLoginStreamForChannelDown(List<LoginItem<T>>? loginItemList, ReactorChannel reactorChannel, int sessionListSize)
    {
        /* Update the aggregated Login response as this session channel is not ready. */
        AggregateLoginResponse();

        if (loginItemList == null)
            return;

        /* Checks whether this is the last channel being closed */
        bool closedStream = sessionListSize == 1;

        PopulateStatusMsg();

        if(closedStream)
        {
            m_State.StreamState(StreamStates.CLOSED);

            m_OmmBaseImpl.SetOmmImplState(OmmBaseImpl<T>.OmmImplState.CHANNEL_CLOSED);
        }
        else
        {
            m_State.StreamState(StreamStates.OPEN);
        }

        if(SendInitialLoginRefresh)
        {
            if(CheckConnectionsIsDown())
            {
                m_State.DataState(DataStates.SUSPECT);
            }
            else
            {
                m_State.DataState(DataStates.OK);
            }
        }
        else
        {
            m_State.DataState(DataStates.SUSPECT);
        }

        m_State.Code(StateCodes.NONE);
        m_State.Text().Data("session channel closed");
        m_EtaStatusMsg.State = m_State;
        m_EtaStatusMsg.ApplyHasState();

        m_StatusMsg.Decode(m_EtaStatusMsg, reactorChannel.MajorVersion, reactorChannel.MinorVersion, null);

        for (int idx = 0; idx < loginItemList.Count; idx++)
        {
            m_EventImpl.Item = loginItemList[idx];
            m_EventImpl.ReactorChannel = reactorChannel;

            ((IOmmConsumerClient?)m_EventImpl.Item.Client)?.OnAllMsg(m_StatusMsg, m_EventImpl);
            ((IOmmConsumerClient?)m_EventImpl.Item.Client)?.OnStatusMsg(m_StatusMsg, m_EventImpl);
        }
    }

    internal bool CheckConnectionsIsDown()
    {
        foreach (var entry in m_SessionChannelList)
        {
            var reactorChannel = entry.ReactorChannel;

            if (reactorChannel != null)
            {
                if (reactorChannel.State == ReactorChannelState.UP || 
                    reactorChannel.State == ReactorChannelState.READY)
                {
                    return false;
                }
            }
        }

        return true;
    }

    internal bool CheckUserStillLogin()
    {
        foreach(var entry in m_SessionChannelList)
        {
            if(entry.LoginRefresh().State.StreamState() == StreamStates.OPEN && 
                entry.LoginRefresh().State.DataState() == DataStates.OK)
            {
                return true;
            }
        }

        return false;
    }

    internal void ProcessChannelEvent(SessionChannelInfo<IOmmConsumerClient> sessionChannelInfo, ReactorChannelEvent @event)
    {
        List<LoginItem<T>>? loginItemList = m_OmmBaseImpl.LoginCallbackClient!.LoginItems;

        m_State.Clear();

        switch(@event.EventType)
        {
            case ReactorChannelEventType.CHANNEL_UP:

                if (SendInitialLoginRefresh)
                    return;

                sessionChannelInfo.SendChannelUp = true;

                if (loginItemList == null)
                    return;

                PopulateStatusMsg();

                m_State.StreamState(StreamStates.OPEN);
                m_State.DataState(DataStates.SUSPECT);

                m_State.Code(StateCodes.NONE);
                m_State.Text().Data("session channel up");
                m_EtaStatusMsg.State = m_State;
                m_EtaStatusMsg.ApplyHasState();

                m_StatusMsg.Decode(m_EtaStatusMsg, @event.ReactorChannel!.MajorVersion, @event.ReactorChannel!.MinorVersion, null);

                for (int idx = 0; idx < loginItemList.Count; idx++)
                {
                    m_EventImpl.Item = loginItemList[idx];
                    m_EventImpl.ReactorChannel = @event.ReactorChannel;

                    ((IOmmConsumerClient?)m_EventImpl.Item.Client)?.OnAllMsg(m_StatusMsg, m_EventImpl);
                    ((IOmmConsumerClient?)m_EventImpl.Item.Client)?.OnStatusMsg(m_StatusMsg, m_EventImpl);
                }

                break;

            case ReactorChannelEventType.CHANNEL_READY:

                if (sessionChannelInfo.SendChannelUp || loginItemList == null)
                    return;

                PopulateStatusMsg();

                m_State.StreamState(StreamStates.OPEN);

                if(SendInitialLoginRefresh)
                {
                    m_State.DataState(DataStates.OK);
                }
                else
                {
                    m_State.DataState(DataStates.SUSPECT);
                }

                m_State.Code(StateCodes.NONE);
                m_State.Text().Data("session channel up");
                m_EtaStatusMsg.State = m_State;
                m_EtaStatusMsg.ApplyHasState();

                m_StatusMsg.Decode(m_EtaStatusMsg, @event.ReactorChannel!.MajorVersion, @event.ReactorChannel!.MinorVersion, null);

                for (int idx = 0; idx < loginItemList.Count; idx++)
                {
                    m_EventImpl.Item = loginItemList[idx];
                    m_EventImpl.ReactorChannel = @event.ReactorChannel;

                    ((IOmmConsumerClient?)m_EventImpl.Item.Client)?.OnAllMsg(m_StatusMsg, m_EventImpl);
                    ((IOmmConsumerClient?)m_EventImpl.Item.Client)?.OnStatusMsg(m_StatusMsg, m_EventImpl);
                }

                sessionChannelInfo.SendChannelUp = true;
                break;

            case ReactorChannelEventType.CHANNEL_DOWN_RECONNECTING:

                sessionChannelInfo.SendChannelUp = false;

                if (loginItemList == null)
                    return;

                PopulateStatusMsg();

                m_State.StreamState(StreamStates.OPEN);

                /* Update the aggregated Login response as this session channel is not ready. */
                AggregateLoginResponse();

                if (SendInitialLoginRefresh)
                {
                    if(CheckConnectionsIsDown())
                    {
                        m_State.DataState(DataStates.SUSPECT);
                    }
                    else
                    {
                        if (CheckUserStillLogin())
                        {
                            m_State.DataState(DataStates.OK);
                        }
                        else
                        {
                            m_State.DataState(DataStates.SUSPECT);
                        }
                    }
                }
                else
                {
                    m_State.DataState(DataStates.SUSPECT);
                }

                m_State.Code(StateCodes.NONE);
                m_State.Text().Data("session channel down reconnecting");
                m_EtaStatusMsg.State = m_State;
                m_EtaStatusMsg.ApplyHasState();

                m_StatusMsg.Decode(m_EtaStatusMsg, @event.ReactorChannel!.MajorVersion, @event.ReactorChannel!.MinorVersion, null);

                for (int idx = 0; idx < loginItemList.Count; idx++)
                {
                    m_EventImpl.Item = loginItemList[idx];
                    m_EventImpl.ReactorChannel = @event.ReactorChannel;

                    ((IOmmConsumerClient?)m_EventImpl.Item.Client)?.OnAllMsg(m_StatusMsg, m_EventImpl);
                    ((IOmmConsumerClient?)m_EventImpl.Item.Client)?.OnStatusMsg(m_StatusMsg, m_EventImpl);
                }

                break;

            case ReactorChannelEventType.CHANNEL_DOWN:

                sessionChannelInfo.SendChannelUp = false;

                HandleLoginStreamForChannelDown(loginItemList, @event.ReactorChannel!, m_SessionChannelList.Count);
                break;
        }
    }

    ReactorReturnCode ConvertRdmDirectoryToBuffer(DirectoryMsg directoryMsg)
    {
        if(m_EncBuffer == null)
        {
            m_EncBuffer = new Buffer();
            m_EncBuffer.Data(new Eta.Common.ByteBuffer(INIT_DIRECTORY_MSG_BUFFER_SIZE));
        }
        else
        {
            ByteBuffer byteBuffer = m_EncBuffer.Data();
            byteBuffer.Clear();
            m_EncBuffer.Data(byteBuffer, 0, byteBuffer.Capacity);
        }

        m_EncIter.Clear();
        if(m_EncIter.SetBufferAndRWFVersion(m_EncBuffer, Codec.MajorVersion(), Codec.MinorVersion()) != CodecReturnCode.SUCCESS)
        {
            if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
            {
                StringBuilder temp = m_OmmBaseImpl.GetStrBuilder();

                temp.Append("Internal error. Failed to set encode iterator buffer in ConsumerSession.ConvertRdmDirectoryToBuffer()");

                m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, temp.ToString());
            }

            return ReactorReturnCode.FAILURE;
        }

        CodecReturnCode retCode = CodecReturnCode.SUCCESS;
        if((retCode = directoryMsg.Encode(m_EncIter)) != CodecReturnCode.SUCCESS)
        {
            if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
            {
                StringBuilder temp = m_OmmBaseImpl.GetStrBuilder();

                temp.Append("Internal error: failed to encode DirectoryMsg in ConsumerSession.ConvertRdmDirectoryToBuffer()")
                    .Append(ILoggerClient.CR)
                    .Append("Error num ").Append(retCode).Append(ILoggerClient.CR)
                    .Append("Error text ").Append(retCode.GetAsInfo());

                m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, temp.ToString());
            }

            return ReactorReturnCode.FAILURE;
        }

        return ReactorReturnCode.SUCCESS;
    }

    public void FanoutSourceDirectoryResponse(DirectoryMsgType msgType)
    {
        if (m_SendDirectoryResp == false || m_OmmBaseImpl.DirectoryCallbackClient!.DirectoryItemList.Count == 0)
        {
            return;
        }

        foreach (var item in m_OmmBaseImpl.DirectoryCallbackClient.DirectoryItemList)
        {
            FanoutSourceDirectoryResponsePerItem(item, msgType, false);
        }

        foreach(var sessionDir in m_SessionDirByName.Values)
        {
            if (sessionDir.SessionChannelInfoList.Count == 0)
            {
                m_RemovedSessionDirSet.Add(sessionDir);
            }
        }
    }

    public void FanoutSourceDirectoryResponsePerItem(DirectoryItem<T> item, DirectoryMsgType msgType, bool isInitialRequest)
    {
        if (item == null)
            return;

        /* Generate DirectoryMsg from ConsumerSession's source directory cache */
        DirectoryMsg? directoryMsg = GenerateDirectoryMsg(item, msgType, isInitialRequest);

        /* Don't fanout directory message if there is no update. */
        if (directoryMsg == null)
            return;

        ReactorReturnCode retCode = ReactorReturnCode.SUCCESS;
        if( (retCode = ConvertRdmDirectoryToBuffer(directoryMsg)) != ReactorReturnCode.SUCCESS)
        {
            if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
            {
                StringBuilder temp = m_OmmBaseImpl.GetStrBuilder();

                temp.Append("Internal error: failed to convert DirectoryMsg to encoded buffer in ConsumerSession.FanoutSourceDirectoryResponsePerItem()");

                m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, temp.ToString());
            }
        }

        ReactorChannel? reactorChannel = SessionChannelList.Count > 0 ? SessionChannelList[0].ReactorChannel : null;
        if (reactorChannel != null && m_EncBuffer != null)
        {
            RestoreServiceFlags();

            if(msgType == DirectoryMsgType.REFRESH)
            {
                RefreshMsg refreshMsg = m_OmmBaseImpl.DirectoryCallbackClient!.m_RefreshMsg!;
                refreshMsg.Clear();

                refreshMsg.Decode(m_EncBuffer, reactorChannel.MajorVersion, reactorChannel.MinorVersion, null, null);

                if(!string.IsNullOrEmpty(item.ServiceName))
                {
                    refreshMsg.SetServiceName(item.ServiceName);
                }

                m_OmmBaseImpl.DirectoryCallbackClient.EventImpl.Item = item;
                m_OmmBaseImpl.DirectoryCallbackClient.EventImpl.ReactorChannel = reactorChannel;
                m_OmmBaseImpl.DirectoryCallbackClient.NotifyOnAllMsg(refreshMsg);
                m_OmmBaseImpl.DirectoryCallbackClient.NotifyOnRefreshMsg();
            }
            else if(msgType == DirectoryMsgType.UPDATE)
            {
                UpdateMsg updateMsg = m_OmmBaseImpl.DirectoryCallbackClient!.m_UpdateMsg!;
                updateMsg.Clear();

                updateMsg.Decode(m_EncBuffer, reactorChannel.MajorVersion, reactorChannel.MinorVersion, null, null);

                if (!string.IsNullOrEmpty(item.ServiceName))
                {
                    updateMsg.SetServiceName(item.ServiceName);
                }

                m_OmmBaseImpl.DirectoryCallbackClient.EventImpl.Item = item;
                m_OmmBaseImpl.DirectoryCallbackClient.EventImpl.ReactorChannel = reactorChannel;
                m_OmmBaseImpl.DirectoryCallbackClient.NotifyOnAllMsg(updateMsg);
                m_OmmBaseImpl.DirectoryCallbackClient.NotifyOnUpdateMsg();
            }
        }
    }

    DirectoryMsg? GenerateDirectoryMsg(DirectoryItem<T> item, DirectoryMsgType msgType, bool isInitialRequest)
    {
        Debug.Assert(msgType == DirectoryMsgType.REFRESH || msgType == DirectoryMsgType.UPDATE);

        m_DirectoryMsg ??= new DirectoryMsg();
        m_DirectoryMsg.Clear();
        m_DirectoryMsg.DirectoryMsgType = msgType;
        m_DirectoryMsg.StreamId = 2;

        if(msgType == DirectoryMsgType.REFRESH)
        {
            DirectoryRefresh directoryRefresh = m_DirectoryMsg.DirectoryRefresh!;
            directoryRefresh.Filter = item.FilterId;
            directoryRefresh.Solicited = true;
            directoryRefresh.State.StreamState(StreamStates.OPEN);
            directoryRefresh.State.DataState(DataStates.OK);
            directoryRefresh.State.Code(StateCodes.NONE);

            if (string.IsNullOrEmpty(item.ServiceName) && !item.HasServiceId)
            {
                Service? service;
                foreach (var sessionDir in m_SessionDirByName.Values)
                {
                    if (isInitialRequest || sessionDir.IsUpdated)
                    {
                        service = sessionDir.Service;
                        if (service != null)
                        {
                            service.Flags &= (ServiceFlags)item.FilterId;
                            directoryRefresh.ServiceList.Add(service);
                        }
                    }
                }
            }
            else
            {
                if(!string.IsNullOrEmpty(item.ServiceName))
                {
                    if(m_SessionDirByName.TryGetValue(item.ServiceName, out var sessionDir))
                    {
                        /* Checks to ensure that the SessionDir is not removed */
                        if (!m_RemovedSessionDirSet.Contains(sessionDir))
                        {
                            if (sessionDir != null && sessionDir.Service != null)
                            {
                                sessionDir.Service.Flags &= (ServiceFlags)item.FilterId;
                                directoryRefresh.ServiceList.Add(sessionDir.Service);
                            }
                        }
                    }
                }
                else if(item.HasServiceId)
                {
                    directoryRefresh.HasServiceId = true;
                    directoryRefresh.ServiceId = item.ServiceId();

                    if(m_SessionDirById.TryGetValue(item.ServiceId(), out var sessionDir))
                    {
                        /* Checks to ensure that the SessionDir is not removed */
                        if (!m_RemovedSessionDirSet.Contains(sessionDir))
                        {
                            if (sessionDir != null && sessionDir.Service != null)
                            {
                                sessionDir.Service.Flags &= (ServiceFlags)item.FilterId;
                                directoryRefresh.ServiceList.Add(sessionDir.Service);
                            }
                        }
                    }
                }
            }
        }
        else if (msgType == DirectoryMsgType.UPDATE)
        {
            DirectoryUpdate directoryUpdate = m_DirectoryMsg.DirectoryUpdate!;
            directoryUpdate.HasFilter = true;
            directoryUpdate.Filter = item.FilterId;

            if(string.IsNullOrEmpty(item.ServiceName) && !item.HasServiceId)
            {
                bool isUpdated = false;
                foreach(var sessionDir in m_SessionDirByName.Values)
                {
                    /* Checks to ensure that the SessionDir is not removed */
                    if (!m_RemovedSessionDirSet.Contains(sessionDir))
                    {
                        if (sessionDir.IsUpdated && sessionDir.Service != null && sessionDir.IsUpdated)
                        {
                            sessionDir.Service.Flags &= (ServiceFlags)item.FilterId;
                            directoryUpdate.ServiceList.Add(sessionDir.Service);
                            isUpdated = true;
                        }
                    }
                }

                if(!isUpdated)
                {
                    return null;
                }
            }
            else
            {
                if (!string.IsNullOrEmpty(item.ServiceName))
                {
                    if (m_SessionDirByName.TryGetValue(item.ServiceName, out var sessionDir))
                    {
                        /* Checks to ensure that the SessionDir is not removed */
                        if (!m_RemovedSessionDirSet.Contains(sessionDir))
                        {
                            if (sessionDir != null && sessionDir.IsUpdated && sessionDir.Service != null)
                            {
                                sessionDir.Service.Flags &= (ServiceFlags)item.FilterId;
                                directoryUpdate.ServiceList.Add(sessionDir.Service);
                            }
                            else
                            {
                                return null;
                            }
                        }
                    }
                }
                else if (item.HasServiceId)
                {
                    directoryUpdate.HasServiceId = true;
                    directoryUpdate.ServiceId = item.ServiceId();

                    if (m_SessionDirById.TryGetValue(item.ServiceId(), out var sessionDir))
                    {
                        /* Checks to ensure that the SessionDir is not removed */
                        if (!m_RemovedSessionDirSet.Contains(sessionDir))
                        {
                            if (sessionDir != null && sessionDir.IsUpdated && sessionDir.Service != null)
                            {
                                sessionDir.Service.Flags &= (ServiceFlags)item.FilterId;
                                directoryUpdate.ServiceList.Add(sessionDir.Service);
                            }
                            else
                            {
                                return null;
                            }
                        }
                    }
                }
            }
        }

        return m_DirectoryMsg;
    }

    public void OnServiceAdd(SessionChannelInfo<T> sessionChannelInfo, ServiceDirectory<T> serviceDirectory)
    {
        m_SessionDirByName.TryGetValue(serviceDirectory.ServiceName!, out var sessionDirectory);

        if(sessionDirectory == null)
        {
            Service newService = new ();
            serviceDirectory.Service!.Copy(newService);

            // Generates an unique service ID for source directory response
            m_generateServiceId++;

            newService.ServiceId = m_generateServiceId;

            serviceDirectory.GeneratedServiceId(newService.ServiceId);

            SessionDirectory<T> newSessionDirectory = new(this, serviceDirectory.ServiceName!);
            newSessionDirectory.SetService(newService);

            /* Adds this SessionChannelInfo to the SessionDirectory for this service */
            newSessionDirectory.SessionChannelInfoList.Add((dynamic)sessionChannelInfo);

            m_SessionDirByName[serviceDirectory.ServiceName!] = newSessionDirectory;
            m_SessionDirById[m_generateServiceId] = newSessionDirectory;

            m_SendDirectoryResp = true;
            newSessionDirectory.IsUpdated = true;

            if (_isConsumerSessionInitialized)
            {
                HandlePendingRequestsForServiceList(newSessionDirectory);
            }
        }
        else if(sessionDirectory.Service!.HasInfo == false)
        {
            /* This is blank SessionDirectory waiting to add this service. This also covers recovery case after the the SessionChannelInfo is removed when the service is deleted */
            int generatedServiceId = sessionDirectory.Service.ServiceId;
            serviceDirectory.Service!.Copy(sessionDirectory.Service);

            sessionDirectory.Service.ServiceId = generatedServiceId;
            serviceDirectory.GeneratedServiceId(generatedServiceId);

            sessionDirectory.SessionChannelInfoList.Add((dynamic)sessionChannelInfo);

            m_SendDirectoryResp = true;
            sessionDirectory.IsUpdated = true;

            sessionDirectory.HandlePendingRequests(sessionChannelInfo, sessionDirectory.Service);

            HandlePendingRequestsForServiceList(sessionDirectory);

            /* Recover items in the item recovery queue if any. */
            NextDispatchTime(1000);

            /* Removed the SessionDirectory from the list as it is added again */
            m_RemovedSessionDirSet.Remove(sessionDirectory);
        }
        else
        {
            bool result = CompareServiceForAggregation(sessionDirectory, serviceDirectory);

            if(result == false)
            {
                if (_isConsumerSessionInitialized)
                {
                    if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                    {
                        StringBuilder temp = m_OmmBaseImpl.GetStrBuilder();

                        temp.Append($"Failed to compare service name {serviceDirectory.ServiceName} for aggregation, ignoring the service from session channel name: {sessionChannelInfo.SessionChannelConfig.Name}, ")
                            .Append($"channel name: {serviceDirectory.ChannelInfo?.ChannelConfig.Name}")
                            .Append(ILoggerClient.CR).Append($"Instance Name {m_OmmBaseImpl.InstanceName}");

                        m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, temp.ToString());
                    }
                }
                else
                {
                    if (_mismatchServiceSet == null)
                    {
                        _mismatchServiceSet = new HashSet<string>();
                    }

                    _mismatchServiceSet.Add(sessionDirectory.ServiceName);
                }
            }
            else
            {
                serviceDirectory.GeneratedServiceId(sessionDirectory.Service.ServiceId);

                /* Adds this SessionChannelInfo to the SessionDirectory for this service if it is not added yet.*/
                bool isAdded = false;

                foreach(var entry in sessionDirectory.SessionChannelInfoList)
                {
                    if(object.ReferenceEquals(sessionChannelInfo,entry))
                    {
                        isAdded = true; break;
                    }
                }

                if(!isAdded)
                {
                    sessionDirectory.SessionChannelInfoList.Add((dynamic)sessionChannelInfo);
                }

                /* Recovers the pending requests after the consumer session initialization */
                if (_isConsumerSessionInitialized)
                {
                    sessionDirectory.HandlePendingRequests(sessionChannelInfo, sessionDirectory.Service);

                    HandlePendingRequestsForServiceList(sessionDirectory);

                    /* Recover items in the item recovery queue if any. */
                    NextDispatchTime(1000);
                }
            }
        }

        if (m_OmmBaseImpl.LoggerClient.IsTraceEnabled)
        {
            StringBuilder temp = m_OmmBaseImpl.GetStrBuilder();

            temp.Append($"Session name: {sessionChannelInfo.SessionChannelConfig.Name}").Append(ILoggerClient.CR)
                .Append($"  OnServiceAdd for {serviceDirectory}").Append(ILoggerClient.CR)
                .Append($"  Instance name {m_OmmBaseImpl.InstanceName}");

            m_OmmBaseImpl.LoggerClient.Trace(CLIENT_NAME, temp.ToString());
        }
    }

    private bool CompareServiceForAggregation(SessionDirectory<T> sessionDirectory, ServiceDirectory<T> serviceDirectory)
    {
        // Compares ItemList, QoS, SupportsQoSRange
        var sessionServiceInfo = sessionDirectory.Service!.Info;
        var serviceInfo = serviceDirectory.Service!.Info;

        if(sessionServiceInfo.HasQos)
        {
            if(!serviceInfo.HasQos)
                return false;

            if(sessionServiceInfo.QosList.Count != serviceInfo.QosList.Count) 
                return false;

            foreach(var qos in sessionServiceInfo.QosList)
            {
                bool found = false;
                foreach(var other in serviceInfo.QosList)
                {
                    if(qos.Equals(other))
                    {
                        found = true;
                        break;
                    }
                }

                if (!found)
                    return false;
            }
        }

        if(!sessionServiceInfo.ItemList.Equals(serviceInfo.ItemList))
        {
            return false;
        }

        if(sessionServiceInfo.HasSupportQosRange != serviceInfo.HasSupportQosRange)
        {
            return false;
        }

        if(sessionServiceInfo.HasSupportQosRange)
        {
            if(sessionServiceInfo.SupportsQosRange != serviceInfo.SupportsQosRange)
            {
                return false;
            }
        }

        return true;
    }

    /* timeout is in microsecond to dispatch events from ConsumerSession */
    internal void NextDispatchTime(long timeout)
    {
        m_DispatchTimeout.InstallTimeout(timeout);
    }

    private void HandlePendingRequestsForServiceList(SessionDirectory<T> sessionDirectory)
    {
        if (ServiceListDict != null)
        {
            var serviceListIt = ServiceListDict.Values.GetEnumerator();

            ServiceList serviceList;
            while (serviceListIt.MoveNext())
            {
                serviceList = serviceListIt.Current;

                if (serviceList.PendingQueueSize() > 0)
                {
                    serviceList.HandlePendingRequests((dynamic)sessionDirectory);
                }
            }
        }
    }

    public void OnServiceDelete(SessionChannelInfo<T> sessionChannelInfo, ServiceDirectory<T> serviceDirectory)
    {
        if (m_OmmBaseImpl.LoggerClient.IsTraceEnabled)
        {
            StringBuilder temp = m_OmmBaseImpl.GetStrBuilder();

            temp.Append($"Session name: {sessionChannelInfo.SessionChannelConfig.Name}").Append(ILoggerClient.CR)
                .Append($"  OnServiceDelete for {serviceDirectory}").Append(ILoggerClient.CR)
                .Append($"  Instance name {m_OmmBaseImpl.InstanceName}");

            m_OmmBaseImpl.LoggerClient.Trace(CLIENT_NAME, temp.ToString());
        }

        m_SessionDirByName.TryGetValue(serviceDirectory.ServiceName!, out var sessionDirectory);

        if (sessionDirectory != null)
        {
            sessionDirectory.SessionChannelInfoList.Remove((dynamic)sessionChannelInfo);

            if(sessionDirectory.SessionChannelInfoList.Count == 0)
            {
                sessionDirectory.Service!.Action = MapEntryActions.DELETE;
                sessionDirectory.Service.Flags = ServiceFlags.NONE;
                sessionDirectory.OriginalServiceFlags = ServiceFlags.NONE;
                m_SendDirectoryResp = true;
                sessionDirectory.IsUpdated = true;
            }
        }
    }

    public void OnServiceStateChange(SessionChannelInfo<T> sessionChannelInfo, ServiceDirectory<T> serviceDirectory, ServiceState serviceState)
    {
        string serviceName = serviceDirectory.ServiceName!;
        m_SessionDirByName.TryGetValue(serviceName, out var sessionDirectory);

        if (sessionDirectory != null)
        {
            // Checks service state change from all session
            var currentState = sessionDirectory.Service!.State;

            if(currentState.ServiceStateVal == 0 && serviceState.ServiceStateVal != 0)
            {
                currentState.ServiceStateVal = 1;
                sessionDirectory.IsUpdated = true;
                m_SendDirectoryResp = true;
            }
            else if (currentState.ServiceStateVal == 1 && serviceState.ServiceStateVal == 0)
            {
                long serviceStateValue = 0;

                foreach(var channelInfo in sessionDirectory.SessionChannelInfoList)
                {
                    var tempServiceDirectory = channelInfo.GetDirectoryByName(serviceName);

                    if (tempServiceDirectory != null)
                    {
                        serviceStateValue |= tempServiceDirectory.Service!.State.ServiceStateVal;
                    }
                }

                if(serviceStateValue == 0)
                {
                    currentState.ServiceStateVal = serviceStateValue;
                    sessionDirectory.IsUpdated = true;
                    m_SendDirectoryResp = true;
                }
            }

            // Checks Accepting request change from all session
            if(sessionDirectory.Service.State.HasAcceptingRequests)
            {
                if(serviceState.HasAcceptingRequests)
                {
                    if(currentState.AcceptingRequests == 0 && serviceState.AcceptingRequests != 0)
                    {
                        currentState.AcceptingRequests = 1;
                        sessionDirectory.IsUpdated = true;
                        m_SendDirectoryResp = true;
                    }
                    else if(currentState.AcceptingRequests == 1 && serviceState.AcceptingRequests == 0)
                    {
                        long acceptingRequests = 0;

                        foreach(var channelInfo in sessionDirectory.SessionChannelInfoList)
                        {
                            var tempServiceDirectory = channelInfo.GetDirectoryByName(serviceName);

                            if(tempServiceDirectory != null)
                            {
                                acceptingRequests |= tempServiceDirectory.Service!.State.AcceptingRequests;
                            }
                        }

                        if(acceptingRequests == 0)
                        {
                            currentState.AcceptingRequests = acceptingRequests;
                            sessionDirectory.IsUpdated = true;
                            m_SendDirectoryResp = true;
                        }
                    }
                }
            }
            else if (serviceState.HasAcceptingRequests)
            {
                sessionDirectory.Service.State.HasAcceptingRequests = true;
                sessionDirectory.Service.State.AcceptingRequests = serviceState.AcceptingRequests;
                sessionDirectory.IsUpdated = true;
                m_SendDirectoryResp = true;
            }

            sessionDirectory.HandlePendingRequests(sessionChannelInfo, sessionDirectory.Service);

            HandlePendingRequestsForServiceList(sessionDirectory);

            if (m_OmmBaseImpl.LoggerClient.IsTraceEnabled)
            {
                StringBuilder temp = m_OmmBaseImpl.GetStrBuilder();

                temp.Append($"Session name: {sessionChannelInfo.SessionChannelConfig.Name}").Append(ILoggerClient.CR)
                    .Append($"  OnServiceStateChange for {serviceDirectory}").Append(ILoggerClient.CR)
                    .Append($"  Instance name {m_OmmBaseImpl.InstanceName}");

                m_OmmBaseImpl.LoggerClient.Trace(CLIENT_NAME, temp.ToString());
            }

            NextDispatchTime(1000); // Wait for 1 millisecond to recover
        }
    }

    public void OnServiceInfoChange(SessionChannelInfo<T> sessionChannelInfo, ServiceDirectory<T> serviceDirectory, ServiceInfo serviceInfo)
    {
        string serviceName = serviceDirectory.ServiceName!;

        m_SessionDirByName.TryGetValue(serviceName, out var sessionDirectory);

        if(sessionDirectory != null)
        {
            sessionDirectory.HandlePendingRequests(sessionChannelInfo, sessionDirectory.Service!);

            HandlePendingRequestsForServiceList(sessionDirectory);

            /* Recover items in the item recovery queue if any. */
            NextDispatchTime(1000);

            if (m_OmmBaseImpl.LoggerClient.IsTraceEnabled)
            {
                StringBuilder temp = m_OmmBaseImpl.GetStrBuilder();

                temp.Append($"Session name: {sessionChannelInfo.SessionChannelConfig.Name}").Append(ILoggerClient.CR)
                    .Append($"  OnServiceInfoChange for {serviceDirectory}").Append(ILoggerClient.CR)
                    .Append($"  Instance name {m_OmmBaseImpl.InstanceName}");

                m_OmmBaseImpl.LoggerClient.Trace(CLIENT_NAME, temp.ToString());
            }
        }
    }

    internal void DownloadDictionary(ServiceDirectory<T> serviceDirectory)
    {
        if(!m_DictionaryConfig.IsLocalDictionary)
        {
            if(m_ChannelDictionary == null)
            {
                m_ChannelDictionary = m_OmmBaseImpl.DictionaryCallbackClient!.PollChannelDict(m_OmmBaseImpl);

                m_OmmBaseImpl.DictionaryCallbackClient!.DownloadDictionary(serviceDirectory, m_ChannelDictionary);
            }
            else
            {
                /* Shares the same data dictionary from network with the channel dictionary */
                serviceDirectory.ChannelInfo!.DataDictionary = m_ChannelDictionary.rsslDictionary();
            }
        }
        else
        {
            /* This gets data dictionary from the local file only */
            m_OmmBaseImpl.DictionaryCallbackClient!.DownloadDictionary(serviceDirectory, m_ChannelDictionary!);
        }
    }

    internal SessionDirectory<T>? GetSessionDirectoryByName(string serviceName)
    {
        if (m_SessionDirByName.TryGetValue(serviceName, out var sessionDir))
        {
            return sessionDir;
        }

        return null;
    }

    internal SessionDirectory<T>? GetSessionDirectoryById(int serviceId)
    {
        if(m_SessionDirById.TryGetValue(serviceId, out var sessionDir))
        {
            return sessionDir;
        }

        return null;
    }

    internal void Dispatch()
    {
        SessionWatchlist.SubmitItemRecovery();
    }

    /* This function is used to add to SessionDirectory's pending queue as the requested service is not available yet. 
	 * The request will be recovered once the service is available. */
    internal void AddPendingRequestByServiceName(string serviceName, SingleItem<T> singleItem, RequestMsg reqMsg)
    {
        singleItem.State = SingleItem<T>.StateEnum.RECOVERING;

        m_SessionDirByName.TryGetValue(serviceName, out SessionDirectory<T>? sessionDirectory);

        if (sessionDirectory == null)
        {
            Service newService = new Service();

            // Generates an unique service ID for source directory response
            m_generateServiceId++;

            newService.ServiceId = m_generateServiceId;

            sessionDirectory = new SessionDirectory<T>(this, serviceName);
            sessionDirectory.Service = newService;

            m_SessionDirByName[serviceName] = sessionDirectory;
            m_SessionDirById[m_generateServiceId] = sessionDirectory;
        }

        sessionDirectory.AddPendingRequest(singleItem, reqMsg);
    }

    /* This function is used to add to SessionDirectory's pending queue with an existing Session directory
	 * The request will be recovered once the service is ready to accept requests. */
    internal void AddPendingRequestByServiceId(SessionDirectory<T> sessionDirectory, SingleItem<T> singleItem, RequestMsg reqMsg)
    {
        singleItem.State = SingleItem<T>.StateEnum.RECOVERING;
        sessionDirectory.AddPendingRequest(singleItem, reqMsg);
    }

    /* Fan-out all pending and recovering items */
    internal void Close()
    {
        SessionWatchlist.Close();

        foreach (SessionDirectory<T> sessionDirectory in m_SessionDirByName.Values)
        {
            sessionDirectory.Close();
        }

        if (ServiceListDict != null)
        {
            var serviceListIt = ServiceListDict.Values.GetEnumerator();

            ServiceList serviceList;
            while (serviceListIt.MoveNext())
            {
                serviceList = serviceListIt.Current; ;

                if (serviceList.PendingQueueSize() > 0)
                {
                    serviceList.Close((dynamic)this);
                }
            }

            ServiceListDict.Clear();
        }

        /* Clears all SessionDirectory mapping */
        m_SessionDirByName.Clear();
        m_SessionDirById.Clear();
        m_RemovedSessionDirSet.Clear();
    }

    internal static ServiceList? GetServiceList(Dictionary<string, ServiceList>? serviceListDict, string serviceListName)
    {
        if (serviceListDict == null || serviceListName == null)
            return null;

        if (serviceListDict.TryGetValue(serviceListName, out var serviceList))
        {
            return serviceList;
        }

        return null;
    }

    /* The IMsgKey.CheckHasServiceId method is checked before calling this function */
    private bool TranslateUserServiceId(SessionChannelInfo<IOmmConsumerClient> sessionChannelInfo, IMsgKey msgKey)
    {
        var sessionDir = GetSessionDirectoryById(msgKey.ServiceId);

        if (sessionDir != null)
        {
            /* Search from the generated service Id */
            var directory = sessionChannelInfo.GetDirectoryByName(sessionDir.ServiceName);

            if (directory?.Service != null && directory.Service.Action != MapEntryActions.DELETE)
            {
                /* Translate to the actual service ID from the provider */
                msgKey.ServiceId = directory.Service.ServiceId;
                return true;
            }
        }
        return false;
    }

    public bool ValidateServiceName(SessionChannelInfo<IOmmConsumerClient> sessionChannelInfo, Eta.Codec.IPostMsg rsslPostMsg, string? serviceName)
    {
        if (serviceName != null)
        {
            var directory = sessionChannelInfo.GetDirectoryByName(serviceName);

            if (directory?.Service != null && directory.Service.Action != MapEntryActions.DELETE)
            {
                return true;
            }

            if (m_OmmBaseImpl.LoggerClient.IsWarnEnabled)
            {
                StringBuilder temp = m_OmmBaseImpl.GetStrBuilder();

                temp.Append($"The specified service name {serviceName} does not exist for {sessionChannelInfo.SessionChannelConfig.Name}. Droping this PosgMsg.")
                    .Append(ILoggerClient.CR);

                m_OmmBaseImpl.LoggerClient.Warn(CLIENT_NAME, temp.ToString());
            }
        }
        else if (rsslPostMsg.CheckHasMsgKey() && rsslPostMsg.MsgKey.CheckHasServiceId())
        {
            if (TranslateUserServiceId(sessionChannelInfo, rsslPostMsg.MsgKey))
            {
                return true;
            }

            if (m_OmmBaseImpl.LoggerClient.IsWarnEnabled)
            {
                StringBuilder temp = m_OmmBaseImpl.GetStrBuilder();

                temp.Append($"The specified service Id {rsslPostMsg.MsgKey.ServiceId} does not exist for {sessionChannelInfo.SessionChannelConfig.Name}. Droping this PosgMsg.")
                    .Append(ILoggerClient.CR);

                m_OmmBaseImpl.LoggerClient.Warn(CLIENT_NAME, temp.ToString());
            }
        }
        else
        {
            if (m_OmmBaseImpl.LoggerClient.IsWarnEnabled)
            {
                StringBuilder temp = m_OmmBaseImpl.GetStrBuilder();

                temp.Append($"Niether service Id or name is specified for the PostMsg. Droping this PosgMsg from {sessionChannelInfo.SessionChannelConfig.Name}.")
                    .Append(ILoggerClient.CR);

                m_OmmBaseImpl.LoggerClient.Warn(CLIENT_NAME, temp.ToString());
            }
        }

        return false;
    }

    public bool CheckServiceId(SessionChannelInfo<IOmmConsumerClient> sessionChannelInfo, IGenericMsg rsslGenericMsg)
    {
        if (rsslGenericMsg.CheckHasMsgKey() && rsslGenericMsg.MsgKey.CheckHasServiceId())
        {
            return TranslateUserServiceId(sessionChannelInfo, rsslGenericMsg.MsgKey);
        }

        return false;
    }
}
