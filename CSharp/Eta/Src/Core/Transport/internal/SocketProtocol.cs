/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System.Linq;
using System.Threading;

using LSEG.Eta.Common;
using LSEG.Eta.Transports;
using LSEG.Eta.Internal.Interfaces;
using System.Net;
using System;
using System.Net.Sockets;
using System.Collections.Generic;

namespace LSEG.Eta.Internal
{
    internal class SocketProtocol : ProtocolBase
    {
        static ReaderWriterLockSlim _slimLock = new ReaderWriterLockSlim(LockRecursionPolicy.SupportsRecursion);
        static WriteLocker _locker = new WriteLocker(_slimLock);

        internal static int DefaultSystemBufferSize = 65535;

        private Dictionary<Int32, Pool> m_WriteBufferChannelsPools = new Dictionary<Int32, Pool>(10);

        private bool uninitializing = false;

        public override IChannel CreateChannel(ConnectOptions connectOptions, out Error error)
        {
            error = null;
            ChannelBase channel;

            channel = new ChannelBase(this, connectOptions, new SocketChannel());
            if (channel != null)
            {
                if (channel.Connect(out error) != TransportReturnCode.SUCCESS)
                {
                    return null;
                }
            }

            _locker.Enter();

            try
            {
                Channels.Add(channel);
            }
            finally
            {
                _locker.Exit();
            }

            return channel;
        }

        public override IServer CreateServer(BindOptions bindOptions, out Error error)
        {
            error = null;
            ServerImpl server;

            server = new ServerImpl(this);

            if (server.Bind(bindOptions, out error) != TransportReturnCode.SUCCESS)
            {
                return null;
            }

            _locker.Enter();

            try
            {
                Servers.Add(server);
            }
            finally
            {
                _locker.Exit();
            }

            return server;
        }

        public override IChannel CreateChannel(AcceptOptions acceptOptions, IServer server, Socket socket, out Error error)
        {
            error = null;
            ChannelBase channel;

            ServerImpl serverImpl = (ServerImpl)server;
            var socketChannel = new SocketChannel(socket, serverImpl.ServerCertificate, serverImpl.BindOptions.BindEncryptionOpts.EncryptionProtocolFlags,
                serverImpl.BindOptions.BindEncryptionOpts.TlsCipherSuites, serverImpl.BindOptions.BindEncryptionOpts.AuthenticationTimeout);

            channel = new ChannelBase(this, acceptOptions, server, socketChannel);

            if (channel.Blocking)
            {
                socketChannel.Socket.Blocking = true;

                TransportReturnCode ret = TransportReturnCode.SUCCESS;
                while (channel.State != ChannelState.ACTIVE)
                {
                    InProgInfo inProg = new InProgInfo();
                    if ((ret = channel.Init(inProg, out error)) < TransportReturnCode.SUCCESS)
                    {
                        channel = null;
                        return null;
                    }
                }
            }
            else
            {
                socketChannel.Socket.Blocking = false;
            }

            try
            {

                _locker.Enter();
                Channels.Add(channel);
            }
            finally
            {
                _locker.Exit();
            }

            return channel;
        }

        public override void Uninitialize(out Error error)
        {
            try
            {
                _locker.Enter();

                uninitializing = true;

                var activeChannels = Channels
                    .Where((c) => c.State == ChannelState.ACTIVE || c.State == ChannelState.INITIALIZING);

                foreach (var channel in Channels)
                {
                    channel.Close(out error);
                }

                Channels.Clear();

                foreach (var server in Servers)
                {
                    server.Close(out error);
                }

                Servers.Clear();

                foreach(var key in m_WriteBufferChannelsPools.Keys)
                {
                    m_WriteBufferChannelsPools[key].Clear();
                }

                m_WriteBufferChannelsPools.Clear();

                uninitializing = false;

                error = null;
            }
            finally
            {
                _locker.Exit();
            }
        }

        public override void CloseChannel(IChannel channel)
        {
            if (uninitializing)
                return;

            _locker.Enter();

            try
            {
                Channels.Remove(channel);
            }
            finally
            {
                _locker.Exit();
            }
        }

        public override void CloseServer(IServer server)
        {
            if (uninitializing)
                return;

            _locker.Enter();

            try
            {
                Servers.Remove(server);
            }
            finally
            {
                _locker.Exit();
            }
        }

        public static IPAddress ParseIPAddress(string address, out Error error)
        {
            error = null;
            IPHostEntry hostEntry = null;

            if(!IPAddress.TryParse(address, out IPAddress ipAddress))
            {   /* Fails to parse to IPAddress so look up hostname from dns instead. */
                try
                {
                    hostEntry = Dns.GetHostEntry(address);
                }
                catch(Exception exp)
                {
                    error = new Error
                    {
                        ErrorId = TransportReturnCode.FAILURE,
                        SysError = 0,
                        Text = exp.Message
                    };
                }

                if(hostEntry != null)
                {
                    var remoteHost = new System.Net.IPHostEntry
                    {
                        AddressList = (from addr in hostEntry.AddressList
                                       where addr.AddressFamily == System.Net.Sockets.AddressFamily.InterNetwork
                                       select addr).ToArray()
                    };

                    ipAddress = remoteHost.AddressList[0];
                }
            }

            return ipAddress;
        }

        public static string GetLocalHostname()
        {
            string hostName = null;

            try
            {
                hostName = Dns.GetHostName();
            }
            catch (Exception) { }

            return hostName;
        }

        public static string GetAddressByHostName(string hostName)
        {
            string ipAddress = null;
            IPHostEntry hostEntry = null;

            try
            {
                hostEntry = Dns.GetHostEntry(hostName);
            }
            catch (Exception) { }

            if (hostEntry != null)
            {
                var localHost = new System.Net.IPHostEntry
                {
                    AddressList = (from addr in hostEntry.AddressList
                                   where addr.AddressFamily == System.Net.Sockets.AddressFamily.InterNetwork
                                   select addr).ToArray()
                };

                ipAddress = localHost.AddressList.Length != 0 ? localHost.AddressList[0].ToString() : null;
            }

            return ipAddress;
        }

        public static int ParsePort(string serviceName)
        {
            if (int.TryParse(serviceName, out int port) == false)
            {
                port = GetServiceByName.Get(serviceName); // the service is a name
            }

            return port;
        }

        /// <summary>
        /// Gets a write buffer pool according to the size of buffer.
        /// </summary>
        /// <param name="poolSpec">The size of each buffer in the pool</param>
        /// <returns>A pool of buffers which has the specified size</returns>
        public override Pool GetPool(int poolSpec)
        {
            if (m_WriteBufferChannelsPools.TryGetValue(poolSpec, out Pool pool) == false)
            {
                pool = new Pool(this);
                pool.IsProtocolBuffer = true;
                m_WriteBufferChannelsPools[poolSpec] = pool;
            }

            return pool;
        }
    }
}
