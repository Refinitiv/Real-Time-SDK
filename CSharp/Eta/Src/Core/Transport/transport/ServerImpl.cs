/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System.Net.Sockets;
using LSEG.Eta.Internal.Interfaces;
using LSEG.Eta.Internal;
using System.Net;
using System;
using LSEG.Eta.Common;
using System.Text;
using System.Security.Cryptography.X509Certificates;
using System.Threading;
using LSEG.Eta.Transports.Internal;

namespace LSEG.Eta.Transports
{
    internal class ServerImpl : IServer
    {
        internal class SharedPool : Pool
        {
            ServerImpl m_ServerImpl;
            int m_CurrentUse = 0; // number of shared buffers currently in use.
            int m_PeakUse = 0;    // peak number of shared buffers used.
            int m_SharedPoolBufferCount = 0; // number of shared pool buffers created.
            public Locker SharedPoolLock;

            public SharedPool(Object owner) : base(owner)
            {
                IsSharedPoolBuffer = true;
                m_ServerImpl = owner as ServerImpl;
            }

            public override void Add(EtaNode node)
            {
                try
                {
                    SharedPoolLock.Enter();
                    base.Add(node);
                    --m_CurrentUse;
                }
                finally
                {
                    SharedPoolLock.Exit();
                }
            }

            public override EtaNode Poll()
            {
                SharedPoolLock.Enter();
                SocketBuffer buffer = null;
                try
                {
                    buffer = (SocketBuffer)base.Poll();
                    if (buffer != null)
                    {
                        ++m_CurrentUse;
                    }
                    else if (m_SharedPoolBufferCount < m_ServerImpl.BindOptions.SharedPoolSize)
                    {
                        // first create one buffer and use it
                        buffer = new SocketBuffer(this, m_ServerImpl.BufferSize());
                        ++m_PeakUse;
                        ++m_CurrentUse;
                        ++m_SharedPoolBufferCount;

                        // then create more buffers, as they should be added to pool in bulk
                        int buffersToAdd = ADDED_BUFFERS - 1;
                        if (buffersToAdd > m_ServerImpl.BindOptions.SharedPoolSize - m_CurrentUse)
                            buffersToAdd = m_ServerImpl.BindOptions.SharedPoolSize - m_CurrentUse;
                        for (int i = 0; i < buffersToAdd; i++)
                        {
                            EtaNode node;
                            node = new SocketBuffer(this, m_ServerImpl.BufferSize());
                            ++m_SharedPoolBufferCount;
                            base.Add(node);
                        }
                    }
                    if (m_CurrentUse > m_PeakUse)
                    {
                        m_PeakUse = m_CurrentUse;
                    }
                }
                finally
                {
                    SharedPoolLock.Exit();
                }

                return buffer;
            }

            public TransportReturnCode Info(ServerInfo info, out Error error)
            {
                error = null;
                TransportReturnCode ret = TransportReturnCode.SUCCESS;
                try
                {
                    SharedPoolLock.Enter();
                    if (m_ServerImpl.State == ChannelState.ACTIVE)
                    {
                        info.CurrentBufferUsage = m_CurrentUse;
                        info.PeakBufferUsage = m_PeakUse;
                    }
                    else
                    {
                        error = new Error
                        {
                            ErrorId = TransportReturnCode.FAILURE,
                            Text = "Server not in active state."
                        };

                        ret = TransportReturnCode.FAILURE;
                    }
                }
                finally
                {
                    SharedPoolLock.Exit();
                }
                return ret;
            }

            public TransportReturnCode BufferUsage(out Error error)
            {
                error = null;
                TransportReturnCode ret;
                try
                {
                    SharedPoolLock.Enter();
                    if (m_ServerImpl.State == ChannelState.ACTIVE)
                        ret = (TransportReturnCode)m_CurrentUse;
                    else
                    {
                        error = new Error
                        {
                            ErrorId = TransportReturnCode.FAILURE,
                            Text = "Server not in active state."
                        };

                        ret = TransportReturnCode.FAILURE;
                    }
                }
                finally
                {
                    SharedPoolLock.Exit();
                }
                return ret;
            }

            public void ResetPeakUse()
            {
                m_PeakUse = m_CurrentUse;
            }
        }

        internal const int ADDED_BUFFERS = 100;
        internal ProtocolBase m_ProtocolBase;
        private Socket m_ServerSocket;
        internal BindOptions BindOptions { get; set; } = new BindOptions();

        internal ComponentInfo ComponentInfo { get; set; } = new ComponentInfo();

        internal AcceptOptions AcceptOptions { get; set; } = new AcceptOptions();

        internal Pool m_SharedPool;

        #region Encrypted connection

        internal X509Certificate ServerCertificate { get; set; }

        #endregion

        internal ServerImpl(ProtocolBase protocol)
        {
            if (protocol == null)
                throw new ArgumentNullException(nameof(protocol));

            m_ProtocolBase = protocol;
            State = ChannelState.INACTIVE;
            m_SharedPool = new SharedPool(this);
            ComponentInfo.ComponentVersion.Data(Transport.DefaultComponentVersionBuffer, 0, Transport.DefaultComponentVersionBuffer.Limit);
        }

        public Socket Socket => m_ServerSocket;

        public int PortNumber { get; internal set; }

        public object UserSpecObject { get; internal set; }

        public ChannelState State { get; internal set; }

        public ConnectionType ConnectionType { get; set; }

        public IChannel Accept(AcceptOptions opts, out Error error)
        {
            error = null;
            IChannel channel = null;

            if (State != ChannelState.ACTIVE)
            {
                error = new Error
                {
                    Channel = null,
                    ErrorId = TransportReturnCode.FAILURE,
                    SysError = 0,
                    Text = "Socket not in active state"
                };

                return null;
            }

            Socket socketChannel;

            try
            {
                socketChannel = Socket.Accept();
                if(socketChannel == null)
                {
                    error = new Error
                    {
                        Channel = null,
                        ErrorId = TransportReturnCode.FAILURE,
                        SysError = 0,
                        Text = "This channel is in non-blocking mode and no connection is available to be accepted",
                    };

                    return null;
                }

                opts.CopyTo(AcceptOptions);

                if (opts.SysSendBufSize > 0)
                {
                    socketChannel.SendBufferSize = AcceptOptions.SysSendBufSize;
                }
                else if(socketChannel.SendBufferSize < SocketProtocol.DefaultSystemBufferSize)
                {
                    socketChannel.SendBufferSize = SocketProtocol.DefaultSystemBufferSize;
                    AcceptOptions.SysSendBufSize = socketChannel.SendBufferSize;
                }

                if (opts.SysRecvBufSize > 0)
                {
                    socketChannel.ReceiveBufferSize = opts.SysRecvBufSize;
                }
                else if (socketChannel.ReceiveBufferSize < SocketProtocol.DefaultSystemBufferSize)
                {
                    socketChannel.ReceiveBufferSize = SocketProtocol.DefaultSystemBufferSize;
                    AcceptOptions.SysRecvBufSize = socketChannel.ReceiveBufferSize;
                }

                socketChannel.NoDelay = BindOptions.TcpOpts.TcpNoDelay;
                socketChannel.SendTimeout = AcceptOptions.SendTimeout;
                socketChannel.ReceiveTimeout = AcceptOptions.ReceiveTimeout;

                channel = m_ProtocolBase.CreateChannel(AcceptOptions, this, socketChannel, out error);

                if(channel != null)
                {
                    (channel as ChannelBase).ComponentInfo.ComponentVersion = ComponentInfo.ComponentVersion;
                }
            }
            catch(Exception exp)
            {
                error = new Error
                {
                    Channel = null,
                    ErrorId = TransportReturnCode.FAILURE,
                    SysError = 0,
                    Text = exp.Message
                };
            }

            return channel;
        }

        public TransportReturnCode Close(out Error error)
        {
            error = null;

            if(m_ServerSocket == null || State != ChannelState.ACTIVE)
            {
                error = new Error
                {
                    Channel = null,
                    ErrorId = TransportReturnCode.FAILURE,
                    SysError = 0,
                    Text = "The server channel is not active"
                };

                return TransportReturnCode.FAILURE;
            }

            m_ServerSocket.Close();
            m_ServerSocket.Dispose();
            m_ServerSocket = null;

            State = (State == ChannelState.INITIALIZING)
                           ? ChannelState.INACTIVE
                           : ChannelState.CLOSED;

            ReleaseServer();
            m_ProtocolBase.CloseServer(this);

            return TransportReturnCode.SUCCESS;
        }

        public TransportReturnCode Info(ServerInfo info, out Error error)
        {
            return ((SharedPool)m_SharedPool).Info(info, out error);
        }

        public TransportReturnCode Bind(BindOptions options, out Error error)
        {
            error = null;
            TransportReturnCode ret = TransportReturnCode.SUCCESS;

            // Ensures that the certificate and private key files are valid for the encrypted connection.
            if (options.ConnectionType == ConnectionType.ENCRYPTED)
            {
                try
                {
                    X509Certificate serverCertificate = X509Certificate2.CreateFromPemFile(options.BindEncryptionOpts.ServerCertificate,
                        options.BindEncryptionOpts.ServerPrivateKey);

                    /* This is workaround for SChannel on Windows as persisted store is required */
                    ServerCertificate = new X509Certificate2(serverCertificate.Export(X509ContentType.Pkcs12));
                }
                catch (Exception ex)
                {
                    error = new Error
                    {
                        ErrorId = TransportReturnCode.FAILURE,
                        Text = $"Failed to validate server's certificate and private key files. Reason:{ex.Message}"
                    };

                    return TransportReturnCode.FAILURE;
                }
            }

            options.CopyTo(BindOptions);

            if (int.TryParse(BindOptions.ServiceName, out int port))
                PortNumber = port; // the service is specified as a port number
            else
                PortNumber = GetServiceByName.Get(BindOptions.ServiceName);

            UserSpecObject = options.UserSpecObject;
            ConnectionType = options.ConnectionType;

            // Create TCP/IP Socket
            m_ServerSocket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, System.Net.Sockets.ProtocolType.Tcp)
            {
                Blocking = options.ServerBlocking,
                NoDelay = BindOptions.TcpOpts.TcpNoDelay,
                ReceiveBufferSize = BindOptions.SysRecvBufSize == 0 ? SocketProtocol.DefaultSystemBufferSize : BindOptions.SysRecvBufSize,
                SendBufferSize = BindOptions.SysSendBufSize == 0 ? SocketProtocol.DefaultSystemBufferSize : BindOptions.SysSendBufSize
            };

            IPAddress localAddress = IPAddress.Any;

            if(string.IsNullOrEmpty(BindOptions.InterfaceName) == false)
            {
                localAddress = SocketProtocol.ParseIPAddress(BindOptions.InterfaceName, out error);

                if (localAddress == null)
                {
                    return error.ErrorId;
                }
            }
           
            IPEndPoint localEp = new IPEndPoint(localAddress, PortNumber);

            try
            {
                m_ServerSocket.Bind(localEp);

                m_ServerSocket.Listen(1000);

                ((SharedPool)m_SharedPool).SharedPoolLock = BindOptions.SharedPoolLock
                         ? new MonitorWriteLocker(new object())
                         : new NoLocker();

                State = ChannelState.ACTIVE;

                if (BindOptions.ComponentVersion != null && BindOptions.ComponentVersion.Length != 0)
                {
                    try
                    {
                        Transport.GlobalLocker.Enter();

                        // user specified info was passed in through the bind Opts
                        ByteBuffer connectOptsCompVerBB = new ByteBuffer(Encoding.ASCII.GetBytes(BindOptions.ComponentVersion));
                        int totalLength = connectOptsCompVerBB.Limit + 1 + Transport.DefaultComponentVersionBuffer.Limit;
                        if(totalLength > 253)
                        {
                            // the total component data length is too long, so truncate the user defined data
                            totalLength = 253;
                            connectOptsCompVerBB.Limit = 253 - Transport.DefaultComponentVersionBuffer.Limit - 1;
                        }

                        // append the user defined connect opts componentVersionInfo to the default value
                        ByteBuffer combinedBuf = new ByteBuffer(totalLength);

                        combinedBuf.Put(Transport.DefaultComponentVersionBuffer);
                        combinedBuf.Write(Transport.COMP_VERSION_DIVIDER);
                        combinedBuf.Put(connectOptsCompVerBB);

                        // the combined length of the new buffer includes the user defined data, the '|', and the default component version data
                        ComponentInfo.ComponentVersion.Data(combinedBuf, 0, totalLength);
                    }
                    finally
                    {
                        Transport.GlobalLocker.Exit();
                    }
                }
            }
            catch (SocketException socketExp)
            {
                error = new Error
                {
                    ErrorId = TransportReturnCode.FAILURE,
                    SysError = socketExp.ErrorCode,
                    Text = socketExp.Message
                };

                ret = TransportReturnCode.FAILURE;
            }
            catch (Exception exp)
            {
                error = new Error
                {
                    ErrorId = TransportReturnCode.FAILURE,
                    SysError = 0,
                    Text = exp.Message

                };

                ret = TransportReturnCode.FAILURE;
            }

            return ret;
        }

        public TransportReturnCode IOCtl(IOCtlCode code, int value, out Error error)
        {
            error = null;
            TransportReturnCode retCode = TransportReturnCode.SUCCESS;

            try
            {
                ((SharedPool)m_SharedPool).SharedPoolLock.Enter();

                if (State != ChannelState.ACTIVE && State != ChannelState.INITIALIZING)
                {
                    error = new Error
                    {
                        ErrorId= TransportReturnCode.FAILURE,
                        Text = $"Server is not in the active or initializing state"
                    };

                    return TransportReturnCode.FAILURE;
                }

                switch(code)
                {
                    case IOCtlCode.SERVER_NUM_POOL_BUFFERS:
                        {
                            /* SERVER_NUM_POOL_BUFFERS: the per server number of
                             * sharedPool buffers that ETA will share with channels of a server.
                             */
                            if (value > 0)
                            {
                                retCode = (TransportReturnCode)AdjustSharedPoolBuffers(value);
                            }
                            else
                            {
                                error = new Error
                                {
                                    ErrorId = TransportReturnCode.FAILURE,
                                    Text = $"value must be greater than zero"
                                };

                                retCode = TransportReturnCode.FAILURE;
                            }

                            break;
                        }
                    case IOCtlCode.SYSTEM_READ_BUFFERS:
                        {
                            if (value >= 0)
                            {
                                m_ServerSocket.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.ReceiveBuffer, value);
                            }
                            else
                            {
                                error = new Error
                                {
                                    ErrorId = TransportReturnCode.FAILURE,
                                    Text = $"value must be (0 >= value < 2^31"
                                };

                                retCode = TransportReturnCode.FAILURE;
                            }
                            break;
                        }
                    case IOCtlCode.SERVER_PEAK_BUF_RESET:
                        {
                            ((SharedPool)m_SharedPool).ResetPeakUse();
                            break;
                        }
                    default:
                        {
                            error = new Error
                            {
                                ErrorId = TransportReturnCode.FAILURE,
                                Text = "Code is not valid."
                            };
                            retCode = TransportReturnCode.FAILURE;
                            break;
                        }
                }
            }
            catch(Exception exp)
            {
                State = ChannelState.CLOSED;

                error = new Error
                {
                    ErrorId = TransportReturnCode.FAILURE,
                    Text = $"exception occurred when setting value '{value}' for IOCtlCode '{code}', exception={exp.Message}"
                };
            }
            finally
            {
                ((SharedPool)m_SharedPool).SharedPoolLock.Exit();
            }

            return retCode;
        }

        internal int BufferSize()
        {
            /* This buffer size is used internal and represents the size of the buffers that are used internally. */
            return BindOptions.MaxFragmentSize + RipcLengths.HEADER;
        }

        internal void SocketBufferToRecycle(SocketBuffer m_SocketBuffer)
        {
            m_SocketBuffer.ReturnToPool();
        }

        internal SocketBuffer GetBufferFromServerPool()
        {
            return (SocketBuffer)((SharedPool)m_SharedPool).Poll();
        }

        private int ShrinkSharedPoolBuffers(int numToShrink)
        {
            Pool bufferPool = m_ProtocolBase.GetPool(BufferSize());
            return bufferPool.Add(m_SharedPool, numToShrink);
        }

        private int AdjustSharedPoolBuffers(int value)
        {
            int diff = value - BindOptions.SharedPoolSize;
            if (diff > 0)
            {
                // the new value is larger, update sharedPoolSize.
                return BindOptions.SharedPoolSize = value;
            }
            else if (diff < 0)
            {
                // shrink buffers
                return BindOptions.SharedPoolSize -= ShrinkSharedPoolBuffers(diff * -1);
            }
            else
            {
                // nothing changed.
                return BindOptions.SharedPoolSize;
            }
        }

        private void ReleaseServer()
        {
            try
            {
                Transport.GlobalLocker.Enter();

                // return buffers from the shared pool to global pool
                Pool pool = m_ProtocolBase.GetPool(BufferSize());
                pool.Add(m_SharedPool, m_SharedPool.Size);
            }
            finally
            {
                Transport.GlobalLocker.Exit();
            }
        }

        public TransportReturnCode BufferUsage(out Error error)
        {
            return ((SharedPool)m_SharedPool).BufferUsage(out error);
        }

        public override string ToString()
        {
            return "Server" + NewLine +
                   "\tsrvrSckt: " + (m_ServerSocket != null ? m_ServerSocket.Handle.ToInt64() : "null") + NewLine +
                   "\tstate: " + State + NewLine +
                   "\tportNumber: " + PortNumber + NewLine +
                   "\tuserSpecObject: " + UserSpecObject + NewLine;
        }

    }
}
