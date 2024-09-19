/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.ValueAdd.Reactor;
using System;
using System.Collections.Generic;
using System.Text;
using System.Threading;

namespace LSEG.Ema.Access
{
    internal class ServerChannelHandler : IReactorChannelEventCallback
    {
        private static readonly string CLIENT_NAME = "ServerChannelHandler";

        internal Dictionary<long, ClientSession> m_ClientSessionDict;
        OmmServerBaseImpl m_ServerBaseImpl;
        ReactorErrorInfo? m_ReactorErrorInfo;

        public ServerChannelHandler(OmmServerBaseImpl ommServerBaseImpl)
        {
            m_ServerBaseImpl = ommServerBaseImpl;
            m_ClientSessionDict = new Dictionary<long, ClientSession>();
        }

        public void AddClientSession(ClientSession clientSession)
        {
            if(!m_ClientSessionDict.ContainsKey(clientSession.ClientHandle))
                m_ClientSessionDict.Add(clientSession.ClientHandle, clientSession);
        }

        public void RemoveClientSession(ClientSession clientSession)
        {
            clientSession.CloseAllItemInfo();
            m_ClientSessionDict.Remove(clientSession.ClientHandle);
        }

        public ClientSession? GetClientSession(long clientHandle)
        {
            m_ClientSessionDict.TryGetValue(clientHandle, out ClientSession? clientSession);

            return clientSession;
        }

        public ReactorCallbackReturnCode ReactorChannelEventCallback(ReactorChannelEvent evt)
        {
            ClientSession? clientSession = (ClientSession?)evt.ReactorChannel?.UserSpecObj;
            ReactorChannel? reactorChannel = evt.ReactorChannel;

            m_ServerBaseImpl.EventReceived();

            if (clientSession == null || reactorChannel == null)
            {
                if (m_ServerBaseImpl.GetLoggerClient().IsErrorEnabled)
                {
                    StringBuilder strBuilder = m_ServerBaseImpl.GetStrBuilder();

                    if (clientSession == null)
                    {
                        strBuilder.AppendLine("Received ReactorChannelEvent without a ClientSession")
                            .AppendLine($"\tInstance Name {m_ServerBaseImpl.InstanceName}");
                    }
                    else
                    {
                        strBuilder.AppendLine("Received ReactorChannelEvent without a ReactorChannel")
                            .AppendLine($"\tInstance Name {m_ServerBaseImpl.InstanceName}");
                    }

                    m_ServerBaseImpl.GetLoggerClient().Error(CLIENT_NAME, strBuilder.ToString());
                }

                return ReactorCallbackReturnCode.SUCCESS;
            }

            switch(evt.EventType)
            {
                case ReactorChannelEventType.CHANNEL_OPENED:
                    {
                        if (m_ServerBaseImpl.GetLoggerClient().IsTraceEnabled)
                        {
                            StringBuilder strBuilder = m_ServerBaseImpl.GetStrBuilder();

                            strBuilder.AppendLine($"Received ChannelOpened on client handle {clientSession.ClientHandle}")
                                .AppendLine($"\tInstance Name {m_ServerBaseImpl.InstanceName}");

                            m_ServerBaseImpl.GetLoggerClient().Trace(CLIENT_NAME, strBuilder.ToString());
                        }

                        break;
                    }
                case ReactorChannelEventType.CHANNEL_UP:
                    {
                        AddClientSession(clientSession);
                        clientSession.Channel(reactorChannel);

                        m_ServerBaseImpl.RegisterSocket(reactorChannel.Socket!);

                        string componentInfoString = "";
                        ReactorChannelInfo reactorChannelInfo = new();
                        if (clientSession.Channel().Info(reactorChannelInfo, out m_ReactorErrorInfo) == ReactorReturnCode.SUCCESS)
                        {
                            var componentInfoList = reactorChannelInfo.ChannelInfo.ComponentInfoList;
                            if (componentInfoList != null && componentInfoList.Count > 0)
                            {
                                componentInfoString = componentInfoList[0].ComponentVersion.ToString();

                                if (componentInfoString.IndexOf("adh") != -1)
                                {
                                    clientSession.IsADHSession = true;
                                }
                            }
                        }

                        if (m_ServerBaseImpl.GetLoggerClient().IsInfoEnabled)
                        {
                            StringBuilder strBuilder = m_ServerBaseImpl.GetStrBuilder();

                            strBuilder.AppendLine($"Received ChannelUp on client handle {clientSession.ClientHandle}")
                                .AppendLine($"\tInstance Name {m_ServerBaseImpl.InstanceName}")
                                .Append($"\tComponent Version {componentInfoString}");

                            m_ServerBaseImpl.GetLoggerClient().Info(CLIENT_NAME, strBuilder.ToString());
                        }

                        m_ServerBaseImpl.AddConnectedChannel(reactorChannel, clientSession);

                        if(reactorChannel.IOCtl(Eta.Transports.IOCtlCode.SYSTEM_WRITE_BUFFERS, m_ServerBaseImpl.ConfigImpl.ServerConfig.BindOptions.SysSendBufSize, out m_ReactorErrorInfo)
                            != ReactorReturnCode.SUCCESS)
                        {
                            if (m_ServerBaseImpl.GetLoggerClient().IsErrorEnabled)
                            {
                                StringBuilder strBuilder = m_ServerBaseImpl.GetStrBuilder();

                                strBuilder.AppendLine($"Failed to set send buffer size on client handle  {clientSession.ClientHandle}")
                                    .AppendLine($"\tInstance Name {m_ServerBaseImpl.InstanceName}");

                                if (reactorChannel != null && reactorChannel.Channel != null)
                                {
                                    strBuilder.AppendLine($"\tReactor {reactorChannel.Reactor?.GetHashCode()}")
                                        .AppendLine($"\tChannel {reactorChannel.Channel.GetHashCode()}");
                                }
                                else
                                {
                                    strBuilder.AppendLine($"\tChannel is null");
                                }

                                if (m_ReactorErrorInfo != null)
                                {
                                    strBuilder.AppendLine($"\tError Id {m_ReactorErrorInfo.Error.ErrorId}")
                                        .AppendLine($"\tInternal sysError {m_ReactorErrorInfo.Error.SysError}")
                                        .AppendLine($"\tError Location {m_ReactorErrorInfo.Location}")
                                        .Append($"\tError text {m_ReactorErrorInfo.Error.Text}");
                                }

                                m_ServerBaseImpl.GetLoggerClient().Error(CLIENT_NAME, strBuilder.ToString());
                            }

                            CloseChannel(reactorChannel!);

                            return ReactorCallbackReturnCode.SUCCESS;
                        }

                        if (reactorChannel.IOCtl(Eta.Transports.IOCtlCode.SYSTEM_READ_BUFFERS, m_ServerBaseImpl.ConfigImpl.ServerConfig.BindOptions.SysRecvBufSize, out m_ReactorErrorInfo)
                            != ReactorReturnCode.SUCCESS)
                        {
                            if (m_ServerBaseImpl.GetLoggerClient().IsErrorEnabled)
                            {
                                StringBuilder strBuilder = m_ServerBaseImpl.GetStrBuilder();

                                strBuilder.AppendLine($"Failed to set receive buffer size on client handle  {clientSession.ClientHandle}")
                                    .AppendLine($"\tInstance Name {m_ServerBaseImpl.InstanceName}");

                                if (reactorChannel != null && reactorChannel.Channel != null)
                                {
                                    strBuilder.AppendLine($"\tReactor {reactorChannel.Reactor?.GetHashCode()}")
                                        .AppendLine($"\tChannel {reactorChannel.Channel.GetHashCode()}");
                                }
                                else
                                {
                                    strBuilder.AppendLine($"\tChannel is null");
                                }

                                if (m_ReactorErrorInfo != null)
                                {
                                    strBuilder.AppendLine($"\tError Id {m_ReactorErrorInfo.Error.ErrorId}")
                                        .AppendLine($"\tInternal sysError {m_ReactorErrorInfo.Error.SysError}")
                                        .AppendLine($"\tError Location {m_ReactorErrorInfo.Location}")
                                        .Append($"\tError text {m_ReactorErrorInfo.Error.Text}");
                                }

                                m_ServerBaseImpl.GetLoggerClient().Error(CLIENT_NAME, strBuilder.ToString());
                            }

                            CloseChannel(reactorChannel!);

                            return ReactorCallbackReturnCode.SUCCESS;
                        }

                        if(m_ServerBaseImpl.ConfigImpl.ServerConfig.CompressionThresholdSet)
                        {
                            if (reactorChannel.IOCtl(Eta.Transports.IOCtlCode.COMPRESSION_THRESHOLD, m_ServerBaseImpl.ConfigImpl.ServerConfig.CompressionThreshold, out m_ReactorErrorInfo)
                           != ReactorReturnCode.SUCCESS)
                            {
                                if (m_ServerBaseImpl.GetLoggerClient().IsErrorEnabled)
                                {
                                    StringBuilder strBuilder = m_ServerBaseImpl.GetStrBuilder();

                                    strBuilder.AppendLine($"Failed to set compression threshold on client handle  {clientSession.ClientHandle}")
                                        .AppendLine($"\tInstance Name {m_ServerBaseImpl.InstanceName}");

                                    if (reactorChannel != null && reactorChannel.Channel != null)
                                    {
                                        strBuilder.AppendLine($"\tReactor {reactorChannel.Reactor?.GetHashCode()}")
                                            .AppendLine($"\tChannel {reactorChannel.Channel.GetHashCode()}");
                                    }
                                    else
                                    {
                                        strBuilder.AppendLine($"\tChannel is null");
                                    }

                                    if (m_ReactorErrorInfo != null)
                                    {
                                        strBuilder.AppendLine($"\tError Id {m_ReactorErrorInfo.Error.ErrorId}")
                                            .AppendLine($"\tInternal sysError {m_ReactorErrorInfo.Error.SysError}")
                                            .AppendLine($"\tError Location {m_ReactorErrorInfo.Location}")
                                            .Append($"\tError text {m_ReactorErrorInfo.Error.Text}");
                                    }

                                    m_ServerBaseImpl.GetLoggerClient().Error(CLIENT_NAME, strBuilder.ToString());
                                }

                                CloseChannel(reactorChannel!);

                                return ReactorCallbackReturnCode.SUCCESS;
                            }
                        }

                        if (reactorChannel.IOCtl(Eta.Transports.IOCtlCode.HIGH_WATER_MARK, m_ServerBaseImpl.ConfigImpl.ServerConfig.HighWaterMark, out m_ReactorErrorInfo)
                            != ReactorReturnCode.SUCCESS)
                        {
                            if (m_ServerBaseImpl.GetLoggerClient().IsErrorEnabled)
                            {
                                StringBuilder strBuilder = m_ServerBaseImpl.GetStrBuilder();

                                strBuilder.AppendLine($"Failed to set high water mark on on client handle  {clientSession.ClientHandle}")
                                    .AppendLine($"\tInstance Name {m_ServerBaseImpl.InstanceName}");

                                if (reactorChannel != null && reactorChannel.Channel != null)
                                {
                                    strBuilder.AppendLine($"\tReactor {reactorChannel.Reactor?.GetHashCode()}")
                                        .AppendLine($"\tChannel {reactorChannel.Channel.GetHashCode()}");
                                }
                                else
                                {
                                    strBuilder.AppendLine($"\tChannel is null");
                                }

                                if (m_ReactorErrorInfo != null)
                                {
                                    strBuilder.AppendLine($"\tError Id {m_ReactorErrorInfo.Error.ErrorId}")
                                        .AppendLine($"\tInternal sysError {m_ReactorErrorInfo.Error.SysError}")
                                        .AppendLine($"\tError Location {m_ReactorErrorInfo.Location}")
                                        .Append($"\tError text {m_ReactorErrorInfo.Error.Text}");
                                }

                                m_ServerBaseImpl.GetLoggerClient().Error(CLIENT_NAME, strBuilder.ToString());
                            }

                            CloseChannel(reactorChannel!);

                            return ReactorCallbackReturnCode.SUCCESS;
                        }
                        else
                        {
                            if (m_ServerBaseImpl.GetLoggerClient().IsTraceEnabled)
                            {
                                StringBuilder strBuilder = m_ServerBaseImpl.GetStrBuilder();

                                strBuilder.AppendLine($"High water mark set on client handle {clientSession.ClientHandle}")
                                    .AppendLine($"\tInstance Name {m_ServerBaseImpl.InstanceName}");

                                m_ServerBaseImpl.GetLoggerClient().Trace(CLIENT_NAME, strBuilder.ToString());
                            }
                        }

                        break;
                    }
                case ReactorChannelEventType.FD_CHANGE:
                    {
                        m_ServerBaseImpl.UnregisterSocket(reactorChannel.OldSocket!);

                        m_ServerBaseImpl.RegisterSocket(reactorChannel.Socket!);

                        if (m_ServerBaseImpl.GetLoggerClient().IsTraceEnabled)
                        {
                            StringBuilder strBuilder = m_ServerBaseImpl.GetStrBuilder();
                            strBuilder.AppendLine($"Received FD Change event on client handle {clientSession.ClientHandle}")
                                .Append($"\tInstance Name {m_ServerBaseImpl.InstanceName}");

                            m_ServerBaseImpl.GetLoggerClient().Trace(CLIENT_NAME, strBuilder.ToString());
                        }

                        clientSession.Channel(reactorChannel);

                        break;
                    }
                case ReactorChannelEventType.CHANNEL_READY:
                    {
                        if (m_ServerBaseImpl.GetLoggerClient().IsTraceEnabled)
                        {
                            StringBuilder strBuilder = m_ServerBaseImpl.GetStrBuilder();
                            strBuilder.AppendLine($"Received ChannelReady event on client handle {clientSession.ClientHandle}")
                                .Append($"\tInstance Name {m_ServerBaseImpl.InstanceName}");

                            m_ServerBaseImpl.GetLoggerClient().Trace(CLIENT_NAME, strBuilder.ToString());
                        }

                        break;
                    }
                case ReactorChannelEventType.CHANNEL_DOWN:
                    {
                        if (m_ServerBaseImpl.GetLoggerClient().IsWarnEnabled)
                        {
                            StringBuilder strBuilder = m_ServerBaseImpl.GetStrBuilder();
                            strBuilder.AppendLine($"Received ChannelDown event on client handle {clientSession.ClientHandle}")
                                .Append($"\tInstance Name {m_ServerBaseImpl.InstanceName}");

                            if (reactorChannel != null && reactorChannel.Channel != null)
                            {
                                strBuilder.AppendLine($"\tReactor {reactorChannel.Reactor?.GetHashCode()}")
                                    .AppendLine($"\tChannel {reactorChannel.Channel.GetHashCode()}");
                            }
                            else
                            {
                                strBuilder.AppendLine($"\tChannel is null");
                            }

                            if (m_ReactorErrorInfo != null)
                            {
                                strBuilder.AppendLine($"\tError Id {m_ReactorErrorInfo.Error.ErrorId}")
                                    .AppendLine($"\tInternal sysError {m_ReactorErrorInfo.Error.SysError}")
                                    .AppendLine($"\tError Location {m_ReactorErrorInfo.Location}")
                                    .Append($"\tError text {m_ReactorErrorInfo.Error.Text}");
                            }

                            m_ServerBaseImpl.GetLoggerClient().Warn(CLIENT_NAME, strBuilder.ToString());
                        }

                        if (Interlocked.Read(ref m_ServerBaseImpl.ImplState) != OmmServerBaseImpl.OmmImplState.NOT_INITIALIZED)
                        {
                            m_ServerBaseImpl.LoginHandler.NotifyChannelDown(clientSession);
                            m_ServerBaseImpl.ProcessChannelEvent(evt);
                        }

                        CloseChannel(reactorChannel!);

                        break;
                    }
                case ReactorChannelEventType.WARNING:
                    {
                        if (m_ServerBaseImpl.GetLoggerClient().IsWarnEnabled)
                        {
                            StringBuilder strBuilder = m_ServerBaseImpl.GetStrBuilder();
                            strBuilder.AppendLine($"Received Channel warning event on client handle {clientSession.ClientHandle}")
                                .Append($"\tInstance Name {m_ServerBaseImpl.InstanceName}");

                            if (reactorChannel != null && reactorChannel.Channel != null)
                            {
                                strBuilder.AppendLine($"\tReactor {reactorChannel.Reactor?.GetHashCode()}")
                                    .AppendLine($"\tChannel {reactorChannel.Channel.GetHashCode()}");
                            }
                            else
                            {
                                strBuilder.AppendLine($"\tChannel is null");
                            }

                            if (m_ReactorErrorInfo != null)
                            {
                                strBuilder.AppendLine($"\tError Id {m_ReactorErrorInfo.Error.ErrorId}")
                                    .AppendLine($"\tInternal sysError {m_ReactorErrorInfo.Error.SysError}")
                                    .AppendLine($"\tError Location {m_ReactorErrorInfo.Location}")
                                    .Append($"\tError text {m_ReactorErrorInfo.Error.Text}");
                            }

                            m_ServerBaseImpl.GetLoggerClient().Warn(CLIENT_NAME, strBuilder.ToString());
                        }

                        break;
                    }
                default:
                    {
                        if (m_ServerBaseImpl.GetLoggerClient().IsErrorEnabled)
                        {
                            StringBuilder strBuilder = m_ServerBaseImpl.GetStrBuilder();
                            strBuilder.AppendLine($"Received unknown channel event type {evt.EventType}")
                                .Append($"\tInstance Name {m_ServerBaseImpl.InstanceName}");

                            if (reactorChannel != null && reactorChannel.Channel != null)
                            {
                                strBuilder.AppendLine($"\tReactor {reactorChannel.Reactor?.GetHashCode()}")
                                    .AppendLine($"\tChannel {reactorChannel.Channel.GetHashCode()}");
                            }
                            else
                            {
                                strBuilder.AppendLine($"\tChannel is null");
                            }

                            if (m_ReactorErrorInfo != null)
                            {
                                strBuilder.AppendLine($"\tError Id {m_ReactorErrorInfo.Error.ErrorId}")
                                    .AppendLine($"\tInternal sysError {m_ReactorErrorInfo.Error.SysError}")
                                    .AppendLine($"\tError Location {m_ReactorErrorInfo.Location}")
                                    .Append($"\tError text {m_ReactorErrorInfo.Error.Text}");
                            }

                            m_ServerBaseImpl.GetLoggerClient().Error(CLIENT_NAME, strBuilder.ToString());
                        }

                        break;
                    }
            }

            return ReactorCallbackReturnCode.SUCCESS;
        }

        internal void CloseChannel(ReactorChannel reactorChannel)
        {
            ClientSession? clientSession = (ClientSession?)reactorChannel.UserSpecObj;

            m_ServerBaseImpl.UnregisterSocket(reactorChannel.Socket!);

            if (clientSession != null)
            {
                if (reactorChannel.Reactor != null && reactorChannel.Close(out m_ReactorErrorInfo) != ReactorReturnCode.SUCCESS)
                {
                    StringBuilder strBuilder = m_ServerBaseImpl.GetStrBuilder();

                    strBuilder.AppendLine($"Failed to close reactor channel  {clientSession.ClientHandle}")
                        .AppendLine($"\tError Id {m_ReactorErrorInfo?.Error.ErrorId}")
                        .AppendLine($"\tInternal sysError {m_ReactorErrorInfo?.Error.SysError}")
                                .AppendLine($"\tError Location {m_ReactorErrorInfo?.Location}")
                                .Append($"\tError text {m_ReactorErrorInfo?.Error.Text}");
                }

                RemoveClientSession(clientSession);

                m_ServerBaseImpl.RemoveConnectedChannel(reactorChannel, clientSession);
            }
        }

        internal void CloseActiveSessions()
        {
            foreach(var clientSession in m_ClientSessionDict.Values)
            {
                clientSession.CloseAllItemInfo();
                clientSession.ReturnToPool();
            }

            m_ClientSessionDict.Clear();
        }

        // returns either the first ADH session, or the first session otherwise, or null
        // when no sessions are present in the m_ClientSessionDict
        internal ClientSession? ClientSessionForDictReq()
        {
            if (m_ClientSessionDict.Count != 0)
            {
                foreach (ClientSession clientSessionTemp in m_ClientSessionDict.Values)
                {
                    if (clientSessionTemp.IsADHSession)
                    {
                        return clientSessionTemp;
                    }
                }

                // active ADH session is not found, pick the first available instead
                var values = m_ClientSessionDict.Values.GetEnumerator();
                if (values.MoveNext())
                {
                    return values.Current;
                }
            }

            return null;
        }
    }
}
