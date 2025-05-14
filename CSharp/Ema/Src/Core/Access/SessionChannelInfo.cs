/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|              Copyright (C) 2024 LSEG. All rights reserved.                --
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Collections.Generic;
using System.Text;

using LSEG.Eta.Codec;
using LSEG.Eta.Rdm;
using LSEG.Eta.ValueAdd.Rdm;
using LSEG.Eta.ValueAdd.Reactor;

namespace LSEG.Ema.Access;

/// <summary>
/// Encapsulates the SessionChannel's state at run-time.
/// </summary>
/// <seealso cref="ConsumerSession"/>
internal class SessionChannelInfo<T>
{
    #region Private properties

    private static readonly string CLIENT_NAME = "SessionChannelInfo";

    private SessionChannelConfig m_SessionChannelConfig;

    private List<ChannelInfo> m_ChannelInfoList;

    private ConsumerSession<T> m_ConsumerSession;

    private bool m_IsLoggedIn;

    private ReactorChannel? m_ReactorChannel;

    private LoginRefresh m_LoginRefresh; // Stores login refresh for this session channel

    private OmmBaseImpl<T> m_OmmBaseImpl;

    // Stores source directory for this session channel.
    private Dictionary<int, ServiceDirectory<T>> m_ServiceById;
    private Dictionary<string, ServiceDirectory<T>> m_ServiceByName;

    #endregion

    internal SessionChannelInfo(SessionChannelConfig sessionChannelConfig, ConsumerSession<T> consumerSession)
    {
        m_SessionChannelConfig = sessionChannelConfig;
        m_ConsumerSession = consumerSession;
        m_OmmBaseImpl = m_ConsumerSession.OmmBaseImpl();

        m_ChannelInfoList = new List<ChannelInfo>(sessionChannelConfig.ChannelSet.Count);
        m_LoginRefresh = new LoginRefresh();

        int initialHashSize = (int)(((OmmConsumerConfigImpl)m_OmmBaseImpl.OmmConfigBaseImpl).ConsumerConfig.ServiceCountHint / 0.75 + 1);
        m_ServiceById = new Dictionary<int, ServiceDirectory<T>>(initialHashSize);
        m_ServiceByName = new Dictionary<string, ServiceDirectory<T>>(initialHashSize);
    }

    /// <summary>
    /// Channels managed by this SessionChannel
    /// </summary>
    internal List<ChannelInfo> ChannelInfoList { get => m_ChannelInfoList; }

    internal ConsumerSession<T> ConsumerSession => m_ConsumerSession;

    /* This isused to indicate whether the current session is logged in/logged out */
    internal bool IsLoggedIn { get => m_IsLoggedIn; set => m_IsLoggedIn = value; }

    internal ReactorChannel? ReactorChannel { get => m_ReactorChannel; set => m_ReactorChannel = value; }

    internal LoginRefresh LoginRefresh()
    {
        return m_LoginRefresh;
    }

    internal SessionChannelConfig SessionChannelConfig => m_SessionChannelConfig;

    internal ServiceDirectory<T>? GetDirectoryByName(string name)
    {
        if (m_ServiceByName.TryGetValue(name, out var directory))
            return directory;
        else
            return null;
    }

    internal ServiceDirectory<T>? GetDirectoryById(int serviceId)
    {
        if (m_ServiceById.TryGetValue(serviceId, out var directory))
            return directory;
        else
            return null;
    }

    internal void ProcessDirectoryPayload(List<Service> serviceList, ChannelInfo chnlInfo)
    {
        foreach (Service oneService in serviceList)
        {
            switch (oneService.Action)
            {
                case MapEntryActions.ADD:
                    {
                        if (!(oneService.HasInfo))
                        {
                            if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                            {
                                m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME,
                                    "Received RDMService with Add action but no Service Info");
                            }
                            break;
                        }

                        string serviceName = oneService.Info.ServiceName.ToString();
                        if (serviceName == null)
                        {
                            if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                            {
                                m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME,
                                    "Received RDMService with Add action but no Service Info");
                            }
                            break;
                        }

                        Service? existService = null;
                        ServiceDirectory<T>? existDirectory = null;

                        if (m_ServiceByName.Count > 0)
                        {
                            if (m_ServiceByName.TryGetValue(serviceName, out existDirectory))
                            {
                                existService = existDirectory.Service;
                            }
                        }

                        if (existDirectory != null && existService != null)
                        {
                            if (existService.ServiceId != oneService.ServiceId)
                            {
                                m_ServiceById.Remove(existService.ServiceId);
                                existService.ServiceId = oneService.ServiceId;
                                m_ServiceById[existService.ServiceId] = existDirectory;

                            }
                            if (existDirectory.ChannelInfo != chnlInfo)
                            {
                                chnlInfo.DataDictionary = existDirectory.ChannelInfo?.DataDictionary;
                                existDirectory.ChannelInfo = chnlInfo;
                            }

                            oneService.Copy(existService);

                            // Notify service is added to ConsumerSession
                            m_ConsumerSession.OnServiceAdd(this, existDirectory);
                        }
                        else
                        {
                            Service newService = new Service();
                            oneService.Copy(newService);

                            ServiceDirectory<T> directory = new ServiceDirectory<T>(newService);
                            directory.ChannelInfo = chnlInfo;
                            directory.ServiceName = serviceName;

                            m_ServiceById[oneService.ServiceId] = directory;
                            m_ServiceByName[serviceName] = directory;

                            // Notify service is added to ConsumerSession
                            m_ConsumerSession.OnServiceAdd(this, directory);

                            if (((OmmConsumerConfigImpl)m_OmmBaseImpl.OmmConfigBaseImpl).DictionaryConfig.IsLocalDictionary
                                || (newService.State.AcceptingRequests == 1 && newService.State.ServiceStateVal == 1))
                            {
                                m_ConsumerSession.DownloadDictionary(directory);
                            }
                        }

                        break;
                    }
                case MapEntryActions.UPDATE:
                    {
                        Service? existService = null;
                        ServiceDirectory<T>? existDirectory = null;
                        if (m_ServiceById.Count > 0 && m_ServiceById.ContainsKey(oneService.ServiceId))
                        {
                            existDirectory = m_ServiceById[oneService.ServiceId];
                            existService = existDirectory.Service;
                        }

                        if (existService == null)
                        {
                            if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                            {
                                StringBuilder temp = m_OmmBaseImpl.GetStrBuilder();

                                temp.Append("Received Update action for unknown Service with service id ")
                                    .Append(oneService.ServiceId);

                                m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, temp.ToString());
                            }
                            break;
                        }
                        else if (m_OmmBaseImpl.LoggerClient.IsTraceEnabled)
                        {
                            StringBuilder temp = m_OmmBaseImpl.GetStrBuilder();

                            temp.Append("Received Update action for RDMService").Append(ILoggerClient.CR)
                                .Append("Service name ").Append(existService.Info.ServiceName.ToString()).Append(ILoggerClient.CR)
                                .Append("Service id ").Append(existService.ServiceId);

                            m_OmmBaseImpl.LoggerClient.Trace(CLIENT_NAME, temp.ToString());
                        }
                        if ((existDirectory != null) && existDirectory.ChannelInfo != chnlInfo)
                        {
                            chnlInfo.DataDictionary = existDirectory.ChannelInfo?.DataDictionary;
                            existDirectory.ChannelInfo = chnlInfo;
                        }

                        if (oneService.HasInfo)
                        {
                            ServiceInfo existInfo = existService.Info;
                            if (!(existInfo.ServiceName.Equals(oneService.Info.ServiceName)))
                            {
                                if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                                {
                                    StringBuilder temp = m_OmmBaseImpl.GetStrBuilder();

                                    temp.Append("Received Update action for RDMService").Append(ILoggerClient.CR)
                                        .Append("Service name ").Append(existInfo.ServiceName.ToString()).Append(ILoggerClient.CR)
                                        .Append("Service id ").Append(existService.ServiceId).Append(ILoggerClient.CR)
                                        .Append("attempting to change service name to ").Append(oneService.Info.ServiceName.ToString());

                                    m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, temp.ToString());
                                }
                                break;
                            }

                            oneService.Info.Copy(existInfo);

                            // Notify service info change to ConsumerSession
                            m_ConsumerSession.OnServiceInfoChange(this, existDirectory!, existInfo);
                        }

                        if (oneService.HasState)
                        {
                            oneService.State.Copy(existService.State);

                            // Notify service state change to ConsumerSession
                            m_ConsumerSession.OnServiceStateChange(this, existDirectory!, existService.State);

                            if (oneService.State.AcceptingRequests == 1 && oneService.State.ServiceStateVal == 1)
                            {
                                m_OmmBaseImpl.DictionaryCallbackClient!.DownloadDictionary(existDirectory!, 
                                    m_OmmBaseImpl.DictionaryCallbackClient!.PollChannelDict(m_OmmBaseImpl));
                            }
                        }

                        existService.Action = MapEntryActions.UPDATE;

                        break;
                    }
                case MapEntryActions.DELETE:
                    {
                        Service? existService = null;
                        ServiceDirectory<T>? existDirectory = null;
                        if (m_ServiceById.Count > 0 && m_ServiceById.ContainsKey(oneService.ServiceId))
                        {
                            existDirectory = m_ServiceById[oneService.ServiceId];
                            existService = existDirectory.Service;
                        }
                        if (existService == null)
                        {
                            if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                            {
                                StringBuilder temp = m_OmmBaseImpl.GetStrBuilder();

                                temp.Append("Received Delete action for unknown RDMService with service id ")
                                    .Append(oneService.ServiceId);

                                m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, temp.ToString());
                            }
                            break;
                        }
                        else if (m_OmmBaseImpl.LoggerClient.IsTraceEnabled)
                        {
                            StringBuilder temp = m_OmmBaseImpl.GetStrBuilder();

                            temp.Append("Received Delete action for RDMService").Append(ILoggerClient.CR)
                                .Append("Service name ").Append(existService.Info.ServiceName.ToString()).Append(ILoggerClient.CR)
                                .Append("Service id ").Append(existService.ServiceId);

                            m_OmmBaseImpl.LoggerClient.Trace(CLIENT_NAME, temp.ToString());
                        }

                        existService.Clear();
                        existService.Action = MapEntryActions.DELETE;

                        // Notify service is deleted to ConsumerSession
                        m_ConsumerSession.OnServiceDelete(this, existDirectory!);
                        break;
                    }
                default:
                    {
                        if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                        {
                            StringBuilder temp = m_OmmBaseImpl.GetStrBuilder();

                            temp.Append("Received unknown action for RDMService. Action value ")
                                .Append(oneService.Action);

                            m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, temp.ToString());
                        }
                        break;
                    }
            }
        }
    }

    internal void OnChannelClose(ChannelInfo? channelInfo)
    {
       foreach(var service in m_ServiceByName.Values)
        {
            SessionDirectory<T>? sessionDirectory = m_ConsumerSession.GetSessionDirectoryByName(service.ServiceName!);

            sessionDirectory?.HandleSessionChannelClose(this);
        }
    }

    internal bool ReceivedLoginRefresh { get; set; } = false;

    /* This is used to keep the state of this connection for administrative domains from OmmBaseImpl.OmmImplState
	   when initializing OmmConsumer instance */
    public long State { get; internal set; }
    public bool SendChannelUp { get; internal set; }
}
