/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Collections.Generic;
using System.Net;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

using Refinitiv.Eta.Common;
using Refinitiv.Common.Logger;
using Refinitiv.Eta.Internal.Interfaces;
using Refinitiv.Eta.Transports;
using Refinitiv.Eta.Transports.Internal;
using System.Net.Sockets;
using System.Net.Security;

namespace Refinitiv.Eta.Tests
{
    internal class MockChannel : ISocketChannel, IDisposable
    {
        public bool IsConnected { get; set; }
        public bool IsDisposed { get; }
        public bool IsServer { get; }
        public bool IsDataReady { get; set; }

        public EndPoint RemoteEP { get; private set; }
        public int RemotePort { get; private set; }

        public Guid ChannelId { get; } = Guid.NewGuid();

        public long ReceivedCount { get; private set; }

        public Socket Socket => null;

        public event Transports.ConnectionStateChangeHandler OnConnected;
        public event DataXferHandler OnDataReady;
        public event DataXferHandler OnDataSent;
        public event Transports.ConnectionStateChangeHandler OnDisconnected;
        public event Transports.ConnectionStateChangeHandler OnError;

        private ByteBuffer m_networkBuffer = new ByteBuffer(131072); // Default to write mode

        public int MaxWrite { get; set; }

        public int MaxRead { get; set; }

        public enum WriteActions
        {
            NORMAL = 1,

            WOULD_BLOCK = 2,

            ERROR = 3
        }

        public WriteActions SocketWriteAction { get; set; }

        public TcpClient TcpClient => throw new NotImplementedException();

        public SslStream SslStream => throw new NotImplementedException();

        SslStream ISocketChannel.SslStream { get => throw new NotImplementedException(); set => throw new NotImplementedException(); }

        public bool IsEncrypted { get => false;}

        public MockChannel()
        {
            Clear();
        }

        public void Clear()
        {
            MaxWrite = -1; // No limit
            MaxWrite = -1; // No limit
            SocketWriteAction = WriteActions.NORMAL;

            m_networkBuffer.Clear();
        }

        public ByteBuffer GetNetworkBuffer()
        {
            return m_networkBuffer;
        }

        private int GetMaxWrite(int writeBufferLength)
        {
            if(MaxWrite == -1)
            {
                return writeBufferLength;
            }
            else
            {
                return MaxWrite <= writeBufferLength ? MaxWrite : writeBufferLength;
            }
        }

        #region Raise Events
        /// <summary>
        /// Fire the OnConnected event.
        /// </summary>
        /// <returns></returns>
        internal void RaiseConnected(ResultObject aro)
        {
            EtaLogger.Instance.Trace($"{DateTime.UtcNow:HH:mm:ss.ffff} MockChannel({ChannelId})::EndConnect AsynchResultObject: {aro}");

            IsConnected = true;
            Recv(aro.UserState);
            OnConnected?.Invoke(this, new SocketEventArgs(this, "Connected"));
        }

        /// <summary>
        /// Fire the OnDisconnected event. <see cref="ResultObject"/>
        /// </summary>
        internal void RaiseDisconnected()
        {
            EtaLogger.Instance.Trace($"{DateTime.UtcNow:HH:mm:ss.ffff} MockChannel({ChannelId})::Channel Disconnecting from {RemoteEP}");

            OnDisconnected?.Invoke(this, new SocketEventArgs(null, ""));
        }

        internal void RaiseDataSent(ResultObject aro)
        {
            EtaLogger.Instance.Trace($"{DateTime.UtcNow:HH:mm:ss.ffff} MockChannel({ChannelId})::DataSent - AsyncResultObject: {aro}, ");
            OnDataSent?.Invoke(aro);
        }

        internal void RaiseDataReady(ResultObject aro)
        {
            ReceivedCount++;

            EtaLogger.Instance.Trace($"{DateTime.UtcNow:HH:mm:ss.ffff} MockChannel({ChannelId})::DataReady - AsyncResultObject: {aro}, ");
            OnDataReady?.Invoke(aro);
        }
        /// <summary>
        /// Fire OnError event.
        /// </summary>
        /// <param name="aro"></param>
        /// <param name="message"></param>
        internal void RaiseError(ResultObject aro, string message)
        {
            StringBuilder sb = new StringBuilder();
            sb.AppendFormat("SocketChannel::RaiseError:  {0}", message);
            EtaLogger.Instance.Trace($"{DateTime.UtcNow:HH:mm:ss.ffff} MockChannel({ChannelId})::RaiseError Message:{sb}");

            OnError?.Invoke(this, new SocketEventArgs(aro.UserState, message));
        }


        #endregion

        #region ISocketChannel

        public virtual bool Connect(ConnectOptions connectOptions, IPAddress remoteAddr, int port, bool isProxyEnabled, out Error error)
        {
            error = null;
            // Translate Address:Port from configuration file into a local end-point.
            RemoteEP = new IPEndPoint(remoteAddr, port);
            RemotePort = port;

            EtaLogger.Instance.Trace($"{DateTime.UtcNow:HH:mm:ss.ffff} MockChannel({ChannelId})::BeginConnect Address: {remoteAddr}, Port: {port}");

            Task task = Task.Factory.StartNew(() =>
                {
                    // Pretend to receive a ReplyAck following Connect
                    RipcConnectionReplyAck replyAck = default(RipcConnectionReplyAck);
                    replyAck.RipcReply.MessageLength = replyAck.MessageLength;
                    replyAck.RipcReply.Flags = RipcFlags.HAS_OPTIONAL_FLAGS;
                    replyAck.RipcReply.OpCode = RipcOpCode.CONNECT_ACK;
                    replyAck.RipcVersion = RipcVersionInfo.CurrentVersion;
                    replyAck.MaxUserMsgSize = RipcVersionInfo.MaxUserMsgSize;
                    replyAck.MajorVersion = RipcVersionInfo.MajorVersion;
                    replyAck.MinorVersion = RipcVersionInfo.MinorVersion;
                    ByteBuffer byteBuffer = new ByteBuffer(replyAck.MessageLength);
                    byteBuffer.Write(replyAck);

                    var aro = new ResultObject(null, byteBuffer);

                    if (port == 0)
                        RaiseConnected(aro);

                    if (port == MockProtocol.PortActionAfter50ms)
                    {
                        Thread.Sleep(50);
                        RaiseConnected(aro);
                    }

                    if (port == MockProtocol.PortActionOnSignal)
                    {
                        do
                        {
                            Thread.Sleep(0);
                            if (IsConnected)
                                // Allow external probe to mark "IsConnected"
                                RaiseConnected(aro);
                        } while (!IsConnected);
                    }

                });

            return true;
        }

        public bool Connect(ConnectOptions connectOptions, out Error error)
        {
            error = null;
            IPHostEntry ipHostInfo = Dns.GetHostEntry(connectOptions.UnifiedNetworkInfo.Address);
            IPAddress ipAddress = ipHostInfo.AddressList[0];
            return Connect(connectOptions, ipAddress, connectOptions.UnifiedNetworkInfo.Port, false, out error);
        }

        public void Disconnect()
        {
            Task task = Task.Factory.StartNew(() =>
            {
                Thread.Sleep(10);
                RaiseDisconnected();
            });
        }

        public bool Recv(object user_state)
        {
            ResultObject aro = new ResultObject(null, 8192, user_state);
            return Recv(aro);
        }

        public bool Recv(ResultObject aro)
        {
            bool status = false;

            if (!_isDisposed)
            {
                try
                {
                    EtaLogger.Instance.Trace($"{ DateTime.UtcNow:HH:mm:ss.ffff} MockChannel({ChannelId})::BeginRecv UserState: {aro.UserState?.ToString() ?? "*null*"}");

                    // Initiate the return read on this socket.
                    byte[] packet = aro.Buffer.Contents;

                    Task task = Task.Factory.StartNew(() =>
                    {
                        IsDataReady = false;

                        if (RemotePort == 0)
                            RaiseDataReady(aro);

                        if (RemotePort == MockProtocol.PortActionAfter50ms)
                        {
                            Thread.Sleep(50);
                            RaiseDataReady(aro);
                        }

                        if (RemotePort == MockProtocol.PortActionOnSignal)
                        {
                            do
                            {
                                Thread.Sleep(1);
                                if (IsDataReady)
                                    // Allow external probe to mark "IsDataReady"
                                    RaiseDataReady(aro);
                            } while (!IsDataReady);
                        }

                    });

                    status = true;
                }
                catch (Exception exp)
                {
                    EtaLogger.Instance.Error(exp);
                    RaiseError(aro, exp.Message);
                    status = false;
                }
            }
            return status;
        }

        public void Send(byte[] packet, object user_state)
        {
            if (!_isDisposed)
            {
                var aro = new ResultObject(null, packet, user_state);
                EtaLogger.Instance.Trace($"{ DateTime.UtcNow:HH:mm:ss.ffff} MockChannel({ChannelId})::BeginSend AsynchResultObject: {aro}");

                RaiseDataSent(aro);
            }
            return;
        }
        bool _isDisposed = false;

        public void Dispose()
        {
            Dispose(true);

            GC.SuppressFinalize(this);
        }

        public void Dispose(bool disposedCalled)
        {
            if (_isDisposed)
                return;

            _isDisposed = true;

            if (disposedCalled)
            {
                OnConnected = null;
                OnDisconnected = null;
                OnError = null;
                OnDataReady = null;
                OnDataSent = null;
            }
        }

        public int Send(byte[] buffer, int position, int length, out Error error)
        {
            error = null;
            int byteWritten = -1;

            if (SocketWriteAction == WriteActions.NORMAL)
            {
                int maxLength = GetMaxWrite(length);

                m_networkBuffer.Put(buffer, position, maxLength);

                byteWritten = maxLength;
            }
            else if (SocketWriteAction == WriteActions.WOULD_BLOCK)
            {
                error = new Error
                {
                    ErrorId = TransportReturnCode.WRITE_FLUSH_FAILED,
                    SysError = (int)SocketError.WouldBlock,
                    Text = "An operation on a nonblocking socket cannot be completed immediately"
                };
            }
            else if (SocketWriteAction == WriteActions.ERROR)
            {
                error = new Error
                {
                    ErrorId = TransportReturnCode.FAILURE,
                    SysError = (int)SocketError.ConnectionReset,
                    Text = "The connection was reset by the remote peer"
                };
            }

            return byteWritten;
        }

        public int Receive(ResultObject resultObject, out Error error)
        {
            error = null;

            // Initiate the return read on this socket.
            int retResult = 0;

            resultObject.Buffer.Put(m_networkBuffer); // This method also increases the write position of the destination buffer.

            retResult = m_networkBuffer.WritePosition - m_networkBuffer.ReadPosition;
            m_networkBuffer.ReadPosition += retResult;

            return retResult;
        }

        public int Receive(ByteBuffer dstBuffer, out SocketError socketError)
        {
            throw new NotImplementedException();
        }

        public void ClearNetworkBuffer()
        {
            m_networkBuffer.Clear();
        }
        
        public void SetReadWriteHandlers(bool isSslStream)
        {
            throw new NotImplementedException();
        }

        public bool FinishConnect()
        {
            throw new NotImplementedException();
        }

        public bool FinishConnect(ChannelBase channel)
        {
            throw new NotImplementedException();
        }

        public int Send(IList<ArraySegment<byte>> buffers, out Error error)
        {
            error = null;
            int byteWritten = 0;

            foreach (ArraySegment<byte> buffer in buffers)
            {
                if (SocketWriteAction == WriteActions.NORMAL)
                {
                    int maxLength = GetMaxWrite(buffer.Count);

                    m_networkBuffer.Put(buffer.Array, buffer.Offset, maxLength);

                    byteWritten += maxLength;

                    if (maxLength != buffer.Count)
                        break; // Break if reaches the maximum write size
                }
                else if (SocketWriteAction == WriteActions.WOULD_BLOCK)
                {
                    error = new Error
                    {
                        ErrorId = TransportReturnCode.WRITE_FLUSH_FAILED,
                        SysError = (int)SocketError.WouldBlock,
                        Text = "An operation on a nonblocking socket cannot be completed immediately"
                    };

                    byteWritten = -1;

                    break;
                }
                else if (SocketWriteAction == WriteActions.ERROR)
                {
                    error = new Error
                    {
                        ErrorId = TransportReturnCode.FAILURE,
                        SysError = (int)SocketError.ConnectionReset,
                        Text = "The connection was reset by the remote peer"
                    };

                    byteWritten = -1;

                    break;
                }
            }

            return byteWritten;
        }

        public void PostProxyInit()
        {
            throw new NotImplementedException();
        }
    }


    #endregion
}

