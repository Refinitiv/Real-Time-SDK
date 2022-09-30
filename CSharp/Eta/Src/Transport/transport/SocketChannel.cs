/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Diagnostics;
using System.Net;
using System.Net.Security;
using System.Net.Sockets;
using System.Security.Cryptography.X509Certificates;
using System.Text;

using Refinitiv.Common.Logger;
using Refinitiv.Eta.Common;
using Refinitiv.Eta.Internal;
using System.Linq;
using System.Security.Authentication;
using System.Collections.Generic;
using Refinitiv.Eta.Internal.Interfaces;
using System.IO;

namespace Refinitiv.Eta.Transports
{
    /////////////////////////////////////////////////////////////////////////
    ///
    /// <summary>
    ///   Aggregates Socket object; maintains connection between this object and event sinks.
    /// </summary>
    /// 
    internal sealed class SocketChannel : IDisposable, ISocketChannel
    {
        /// <summary>
        /// Controls appearence of deep-debugging displays. Manually set during development
        /// by developer.
        /// </summary>
        private static bool debug = true;

        #region Delegates/Events

        /// <summary>
        ///   Raised when connection state of the channel changes to CONNECTED.
        /// </summary>
        public event ConnectionStateChangeHandler OnConnected;

        /// <summary>
        ///   Raised when connection state of the channel changes to DISCONNECTED.
        /// </summary>
        public event ConnectionStateChangeHandler OnDisconnected;

        /// <summary>
        /// Raised when an error has been detected during processing.
        /// </summary>
        public event ConnectionStateChangeHandler OnError;

        private ReceiveHandler receiveHandler;

        private SendHandler sendHandler;

        #endregion

        #region Properties

        /// <summary>
        /// Returns true if Socket exists and is active. 
        /// </summary>
        public bool IsConnected
        {
            get => m_socket != null && m_socket.Connected;
            set => throw new InvalidOperationException();
        }

        /// <summary>
        /// Returns true if SocketChanel is a 'zombie' object: disposed, but not yet release from GC heap. 
        /// </summary>
        public bool IsDisposed => m_alreadyDisposed;

        /// <summary>
        /// Role served by this channel.
        /// <list>
        ///   <item>
        ///      <term>
        ///         true
        ///      </term>
        ///      <description>
        ///         Created from SocektServer::Accept call.
        ///      </description>
        ///   </item>
        ///   <item>
        ///   <term>
        ///      false
        ///   </term>
        ///   <description>
        ///      Created from SocketChannel::SocketChannel.
        ///   </description>
        ///   </item>
        /// </list>
        /// </summary>
        public bool IsServer
        {
            get;
        }

        public bool IsDataReady
        {
            get => m_socket != null && m_socket.Available > 0;
            set => throw new InvalidOperationException("SocketChannel cannot set IsDataReady");
        }

        /// <summary>
        /// Entry-POint at other end of Socket
        /// </summary>
        public EndPoint RemoteEP => m_remote_ep;

#if DEBUG
        public Guid ChannelId { get; } = Guid.NewGuid();
#endif
        public Socket Socket { get => m_socket; }

        public SslStream SslStream { get => m_sslStream; set => m_sslStream = value; }

        public TcpClient TcpClient { get => m_tcpClient;}

        public bool IsEncrypted { get => m_Encrypted; }

        #endregion

        #region ToString

        /// <summary>
        /// Provides some reasonable string representation
        /// </summary>
        /// <returns>presentation string</returns>
        public override string ToString()
        {
            var s = m_remote_ep != null ? m_remote_ep.ToString() : "<Undefined>";
            return s;
        }

        /// <summary>
        /// Port number of remote end-point.
        /// </summary>
        public int RemotePort
        {
            get
            {
                if (m_remote_ep == null)
                    return 0x0;
                SocketAddress address = m_remote_ep.Serialize();
                return Convert.ToInt16(address[2]) * 256 + Convert.ToInt16(address[3]);
            }
        }

        #endregion

        #region Instance Vbls

        /// <summary>
        /// Suppress certain actions if we are in the proces of being torn-down.
        /// </summary>
        private bool m_alreadyDisposed;

        /// <summary>
        /// Holding this will allow us to tear the TCP/IP connection down explicitly.
        /// </summary>
        private Socket m_socket;
        private EndPoint m_remote_ep;
        private SslStream m_sslStream;
        private TcpClient m_tcpClient;
        private X509Certificate m_ServerCertificate;
        private EncryptionProtocolFlags m_ProtocolFlags;
        private IEnumerable<TlsCipherSuite> m_CipherSuites;
        private bool m_Encrypted;
        private bool m_CompletedHandshake;
        private bool m_CompleteProxy;
        private string m_EncryptedRemoteAddress;

        private int m_receive_packet_size;

        #endregion

        /// <summary>
        /// The default contructor
        /// </summary>
        public SocketChannel()
        {
            m_receive_packet_size = ResultObject.DefaultBufferSize;
            m_Encrypted = false;
            m_CompletedHandshake = false;
        }

        /// <summary>
        /// The constructor for server's channel
        /// </summary>
        /// <param name="socket">The underlying <c>Socket</c></param>
        /// <param name="serverCertificate">The server's certificate for encrypted connection</param>
        /// <param name="protocolFlags">The encryption protocol flags</param>
        /// <param name="cipherSuites">The collection of cipher suites</param>
        public SocketChannel(Socket socket, X509Certificate serverCertificate = null, EncryptionProtocolFlags protocolFlags = EncryptionProtocolFlags.ENC_NONE,
            IEnumerable<TlsCipherSuite> cipherSuites = null)
        {
            m_receive_packet_size = ResultObject.DefaultBufferSize;
            m_socket = socket;
            m_Encrypted = serverCertificate != null ? true : false;
            m_ServerCertificate = serverCertificate;
            m_ProtocolFlags = protocolFlags;
            m_CipherSuites = cipherSuites;
            m_CompletedHandshake = false;
            m_CompleteProxy = true; /* Always true there is no proxy connection on server side. */
        }

        /// <summary>
        /// Destructor / Finalizer. Because Dispose() calls GC.SupressFinalize(), this method
        /// is only called by the garbage collection process if the consumer of the object does
        /// not call Dispose() as they should.
        /// </summary>
        ~SocketChannel()
        {
            // Call the Dispose() method as opposed to duplicating the code to clean up any unmanaged
            // resources. Use the protected Dispose() overload and pass a value of "false" to indicate
            // that Dispose() is being called during the garbage collection process, not by consumer
            // code.
            Dispose(false);
        }

        /// <summary>
        /// Initiate a connection to the remote node.
        /// </summary>
        /// <param name="connectOptions"></param>
        /// <param name="remoteAddr">
        /// <param name="error">To be set in event of an error</param>
        ///   IP Address in string form. This string will be resolved by DNS into a IPEndPoint object.
        /// </param>
        /// <param name="port">The IP Port number we are trying to reach at the remote node.</param>
        /// <param name="isProxyEnabled">This is used to indicate whether the proxy option is enabled.</param>
        public bool Connect(ConnectOptions connectOptions, IPAddress remoteAddr, int port, bool isProxyEnabled, out Error error)
        {
            error = null;
            ResultObject aro = null;
            bool status;
            try
            {
                if (IsServer)
                    return false;

                // Create TCP/IP Socket
                m_socket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, System.Net.Sockets.ProtocolType.Tcp)
                {
                    Blocking = true,
                    NoDelay = connectOptions.TcpOpts.TcpNoDelay,
                    ReceiveBufferSize = connectOptions.SysRecvBufSize == 0 ? SocketProtocol.DefaultSystemBufferSize : connectOptions.SysRecvBufSize,
                    SendBufferSize = connectOptions.SysSendBufSize == 0 ? SocketProtocol.DefaultSystemBufferSize : connectOptions.SysSendBufSize,
                    SendTimeout = 500,
                    ReceiveTimeout = 500
                };

                // Translate Address:Port from configuration file into a local end-point.
                IPEndPoint remote_ep = new IPEndPoint(remoteAddr, port);

                aro = new ResultObject(m_socket, m_receive_packet_size, null);

                status = true;

                if (string.IsNullOrEmpty(connectOptions.UnifiedNetworkInfo.InterfaceName) == false)
                {
                    IPAddress localAddress = SocketProtocol.ParseIPAddress(connectOptions.UnifiedNetworkInfo.InterfaceName, out error);

                    if (localAddress == null)
                    {
                        return false;
                    }

                    IPEndPoint bindEndPoint = new IPEndPoint(localAddress, 0);
                    m_socket.Bind(bindEndPoint);
                }

                m_socket.Connect(remote_ep);

                m_remote_ep = m_socket.RemoteEndPoint;

                if(connectOptions.ConnectionType == ConnectionType.ENCRYPTED)
                {
                    m_Encrypted = true;
                    m_CompleteProxy = isProxyEnabled ? false : true; // Sets to true if proxy is not enabled for encrypted connection.
                    m_EncryptedRemoteAddress = isProxyEnabled ? connectOptions.ProxyOptions.ProxyHostName :
                        connectOptions.UnifiedNetworkInfo.Address;
                    m_ProtocolFlags = connectOptions.EncryptionOpts.EncryptionProtocolFlags;
                    m_CipherSuites = connectOptions.EncryptionOpts.TlsCipherSuites;
                }

                SetReadWriteHandlers(false);
                
                m_socket.Blocking = connectOptions.Blocking;
                RaiseConnected(new SocketEventArgs(aro.UserState, null));
            }
            catch (Exception exp)
            {
                error = new Error
                {
                    ErrorId = TransportReturnCode.FAILURE,
                    Text = $"Failed to connect to the specified remote endpoint. Reason:{exp.Message}"
                };

                Trace.TraceError(@"{0} : {1}", aro, exp.Message);
                RaiseError(aro, exp.Message);
                status = false;
            }
            return status;
        }

        public bool Connect(ConnectOptions connectOptions, out Error error)
        {
            IPAddress ipAddress = null;
            error = null;

            m_receive_packet_size = (connectOptions.SysRecvBufSize > 0)
                                                        ? connectOptions.SysRecvBufSize 
                                                        : ResultObject.DefaultBufferSize;

            bool isProxyEnabled = connectOptions.IsProxyEnabled();
            string address = isProxyEnabled ? connectOptions.ProxyOptions.ProxyHostName :
                connectOptions.UnifiedNetworkInfo.Address;

            ipAddress = SocketProtocol.ParseIPAddress(address, out error);

            if (ipAddress != null)
            {
                return Connect(connectOptions, ipAddress, isProxyEnabled ? connectOptions.ProxyOptions.Port :
                    connectOptions.UnifiedNetworkInfo.Port, isProxyEnabled, out error);
            }
            else
            {
                RaiseError(null, $"Could not parse Address [{address}], Text: {error.Text}");
                return false;
            }
        }

        /// <summary>
        /// Shutdown all outstanding I/O requests.
        /// </summary>
        public void Disconnect()
        {
            try
            {
                if (m_socket != null && m_socket.Connected)
                {
                    m_socket.Shutdown(SocketShutdown.Both);
                    RaiseDisconnected();
                }
            }
            catch (Exception exp)
            {
                Trace.TraceError(exp.Message);
                RaiseError(null, exp.Message);
            }
        }

        #region Send

        /// <summary>
        /// Send a binary packet synchronously
        /// </summary>
        /// <param name="packet">Payload to send out on the socket.</param>
        /// <param name="user_state"></param>
        public void Send(byte[] packet, object user_state)
        {
            // throw an exception if this object has already been disposed
            if (m_alreadyDisposed)
            {
                throw new ObjectDisposedException("ErrorHandler");
            }

            ResultObject wro = null;
            try
            {
                wro = new ResultObject(m_socket, packet, user_state);
                byte[] ar = wro.Buffer.Contents;

                int byteSent = sendHandler(ar, wro.Buffer.Position, ar.Length - wro.Buffer.Position);

                OnSocketWrite(wro, byteSent);
            }
            catch (Exception exp)
            {
                Trace.TraceError(@"{0} : {1}", wro, exp.Message);
                RaiseError(wro, exp.Message);
            }
        }

        public int Send(byte[] buffer, int position, int length, out Error error)
        {
            int byteWritten = 0;
            error = null;

            try
            {
                if (m_socket == null)
                    throw new SocketException((int)SocketError.NotSocket); // WSAENOTSOCK

                byteWritten = sendHandler(buffer, position, length);
            }
            catch (SocketException sockExp)
            {
                error = new Error
                {
                    ErrorId = (sockExp.SocketErrorCode == SocketError.WouldBlock ||
                    sockExp.SocketErrorCode == SocketError.TryAgain) ? 
                    TransportReturnCode.WRITE_FLUSH_FAILED : TransportReturnCode.FAILURE,
                    SysError = sockExp.ErrorCode,
                    Text = sockExp.Message
                };

                if (error.ErrorId == TransportReturnCode.FAILURE)
                {
                    var wro = new ResultObject(m_socket, buffer);
                    Trace.TraceError(@"{0} : {1}", wro, sockExp.Message);
                    RaiseError(wro, sockExp.Message);
                }
            }
            catch (Exception exp)
            {
                error = new Error
                {
                    ErrorId = TransportReturnCode.FAILURE,
                    SysError = 0,
                    Text = exp.Message
                };

                var wro = new ResultObject(m_socket, buffer);
                Trace.TraceError(@"{0} : {1}", wro, exp.Message);
                RaiseError(wro, exp.Message);
            }

            return byteWritten;
        }

        public int Send(IList<ArraySegment<byte>> buffers, out Error error)
        {
            error = null;

            int byteWritten = 0;

            try
            {
                if (m_socket == null)
                    throw new SocketException((int)SocketError.NotSocket); // WSAENOTSOCK

                byteWritten = m_socket.Send(buffers, SocketFlags.None, out SocketError socketError);

                if (socketError != SocketError.Success)
                {
                    error = new Error
                    {
                        ErrorId = (socketError == SocketError.WouldBlock||
                        socketError == SocketError.TryAgain) ?
                   TransportReturnCode.WRITE_FLUSH_FAILED : TransportReturnCode.FAILURE,
                        SysError = (int)socketError
                    };
                }
            }
            catch (SocketException sockExp)
            {
                error = new Error
                {
                    ErrorId = (sockExp.SocketErrorCode == SocketError.WouldBlock ||
                    sockExp.SocketErrorCode == SocketError.TryAgain) ?
                    TransportReturnCode.WRITE_FLUSH_FAILED : TransportReturnCode.FAILURE,
                    SysError = sockExp.ErrorCode,
                    Text = sockExp.Message
                };
            }
            catch (Exception exp)
            {
                error = new Error
                {
                    ErrorId = TransportReturnCode.FAILURE,
                    SysError = 0,
                    Text = exp.Message
                };
            }

            return byteWritten;
        }

        #endregion

        #region Receive

        public int Receive(ResultObject resultObject, out Error error)
        {
            error = null;

            // Initiate the return read on this socket.
            int retResult = 0;
            try
            {
                if (m_socket == null)
                    throw new SocketException(10038); // WSAENOTSOCK

                if (resultObject.InitiatingSocket == null)
                    resultObject.InitiatingSocket = this.m_socket;

                retResult = receiveHandler(resultObject.Buffer.Contents, resultObject.Buffer.WritePosition,
                    resultObject.Buffer.Capacity - resultObject.Buffer.WritePosition);

                if (retResult > 0)
                    resultObject.Buffer.WritePosition += retResult;
            }
            catch (SocketException socketException)
            {
                if (socketException.SocketErrorCode != SocketError.WouldBlock && 
                    socketException.SocketErrorCode != SocketError.TryAgain)
                {
                    Trace.TraceError(socketException.Message);
                    RaiseError(resultObject, socketException.Message);

                    error = new Error
                    {
                        ErrorId = TransportReturnCode.FAILURE,
                        SysError = (int)socketException.SocketErrorCode,
                        Text = socketException.Message
                    };

                    retResult = -1; // End of stream
                }
            }
            catch (IOException ioException)
            {
                if (ioException.InnerException is SocketException socketException)
                {
                    if (socketException.SocketErrorCode != SocketError.WouldBlock &&
                        socketException.SocketErrorCode != SocketError.TryAgain)
                    {
                        Trace.TraceError(socketException.Message);
                        RaiseError(resultObject, socketException.Message);

                        error = new Error
                        {
                            ErrorId = TransportReturnCode.FAILURE,
                            Text = ioException.Message
                        };

                        retResult = -1; // End of stream
                    }
                }
            }
            catch (Exception exception)
            {
                Trace.TraceError(exception.Message);
                RaiseError(resultObject, exception.Message);

                error = new Error
                {
                    ErrorId = TransportReturnCode.FAILURE,
                    Text = exception.Message
                };

                retResult = -1; // End of stream
            }

            return retResult;
        }

        public int Receive(ByteBuffer dstBuffer, out SocketError socketError)
        {
            socketError = SocketError.Success;

            // Initiate the return read on this socket.
            int retResult = 0;
            try
            {
                if (m_socket == null)
                    throw new SocketException(10038); // WSAENOTSOCK

                retResult = receiveHandler(dstBuffer.Contents, dstBuffer.WritePosition,
                    dstBuffer.Capacity - dstBuffer.WritePosition);

                if (retResult > 0)
                    dstBuffer.WritePosition += retResult;
            }
            catch (SocketException socketException)
            {
                if (socketException.SocketErrorCode != SocketError.WouldBlock &&
                    socketException.SocketErrorCode != SocketError.TryAgain)
                {
                    socketError = socketException.SocketErrorCode;
                }
            }
            catch (IOException ioException)
            {
                if (ioException.InnerException is SocketException socketException)
                {
                    if (socketException.SocketErrorCode != SocketError.WouldBlock &&
                        socketException.SocketErrorCode != SocketError.TryAgain)
                    {
                        socketError = socketException.SocketErrorCode;
                    }
                }
            }
            catch (Exception)
            {
                socketError = SocketError.SocketError;
            }

            return retResult;
        }

        #endregion

        #region Socket Event Handlers


        /// <summary>
        /// Complete 'Send' request.
        /// </summary>
        /// <param name="iar">State object: AsyncResultObject.</param>
        /// <param name="byteSent"></param>
        private void OnSocketWrite(ResultObject iar, int byteSent)
        {
            ResultObject aro = null;
            try
            {
                // Fetch the results.
                aro = iar;

                // Terminate the Send call
                int bytes_sent = byteSent;
                if (m_alreadyDisposed)
                    return;

                if (aro.Buffer.Limit != bytes_sent)
                    throw new ApplicationException($"Error on socket write; Buffer.Length={aro.Buffer.Limit},Sent={bytes_sent}");
            }
            catch (Exception exp)
            {
                m_socket.Shutdown(SocketShutdown.Receive);
                Trace.TraceError(@"{0} : {1}", aro, exp.Message);
                RaiseError(aro, exp.Message);
            }
        }

        #endregion

        #region Raise Events

        /// <summary>
        /// Fire the OnConnected event.
        /// </summary>
        /// <returns></returns>
        private void RaiseConnected(SocketEventArgs sea)
        {
            if (debug)
            {
                StringBuilder sb = new StringBuilder();
                sb.AppendFormat("Channel connecting to {0}", m_remote_ep?.ToString());
                Trace.WriteLine(debug, sb.ToString());
            }

            OnConnected?.Invoke(this, sea);
        }

        /// <summary>
        /// Fire the OnDisconnected event. <see cref="ResultObject"/>
        /// </summary>
        private void RaiseDisconnected()
        {
            if (debug)
            {
                Trace.WriteLine(String.Format("Channel Disconnecting from {0}",
                   m_remote_ep?.ToString()));
            }

            OnDisconnected?.Invoke(this, new SocketEventArgs(null, ""));
        }

        /// <summary>
        /// Fire OnError event.
        /// </summary>
        /// <param name="aro"></param>
        /// <param name="message"></param>
        private void RaiseError(ResultObject aro, string message)
        {
            StringBuilder sb = new StringBuilder();
            sb.AppendFormat("SocketChannel::RaiseError:  {0}", message);
            Trace.WriteLine(sb.ToString());

            OnError?.Invoke(this, new SocketEventArgs(aro?.UserState, message));
        }

        #endregion

        #region IDisposable Members

        /// <summary>
        /// Public implementation of the IDisposable.Dispose() method, called by the consumer of the
        /// object in order to free unmanaged resources deterministically.
        /// </summary>
        public void Dispose()
        {
            // Call the protected Dispose() overload and pass a value of "true" to indicate that
            // Dispose() is being called by consumer code, not by the garbage collector.
            Dispose(true);

            // Because the Dispose() method performs all necessary cleanup, ensure that the garbage
            // collector does not call the class destructor.
            GC.SuppressFinalize(this);
        }

        /// <summary>
        /// Release any resources held by this object.
        /// </summary>
        /// <param name="isDisposing">
        /// <list>
        /// <listheader>Denotes who called this method:</listheader>
        /// <term>true</term>
        /// <description>Client of this class.</description>
        /// <term>false</term>
        /// <description>GC through the finalizer.</description>
        /// </list>
        /// </param>
        internal void Dispose(bool isDisposing)
        {
            try
            {
                if (m_alreadyDisposed)
                    return;

                StringBuilder sb = new StringBuilder();
                sb.AppendFormat("Channel disposing {0}", ToString());
                Trace.WriteLineIf(debug, sb.ToString());

                m_alreadyDisposed = true;
                if (isDisposing)
                {  // Release the delegates - besides we don't want them firing anyway.
                    OnConnected = null;
                    OnDisconnected = null;
                    OnError = null;

                    if (m_socket != null)
                    {
                        if (m_socket.Connected)
                            m_socket.Shutdown(SocketShutdown.Both);

                        // Dispose the Socket
                        m_socket.Close();
                    }
                }
            }
            catch (Exception exp)
            {
                Trace.TraceError(exp.Message);
            }
        }

        #endregion

        public static bool ValidateServerCertificate(
             object sender,
#nullable enable
             X509Certificate? certificate,
             X509Chain? chain,
#nullable disable
             SslPolicyErrors sslPolicyErrors)
        {
            if (sslPolicyErrors == SslPolicyErrors.None)
                return true;

            return false;
        }

        public void SetReadWriteHandlers(bool isSslStream)
        {
            if(isSslStream)
            {
                receiveHandler = SslStreamReceive;
                sendHandler = SslStreamSend;
            }
            else
            {
                receiveHandler = SocketReceive;
                sendHandler = SocketSend;
            }
        }

        private int SocketReceive(byte[] buffer, int offset, int size)
        {
            return m_socket.Receive(buffer, offset, size, SocketFlags.None);
        }

        private int SslStreamReceive(byte[] buffer, int offset, int size)
        {
            return m_sslStream.Read(buffer, offset, size);
        }

        private int SocketSend(byte[] buffer, int offset, int size)
        {
            return m_socket.Send(buffer, offset, size, SocketFlags.None);
        }

        private int SslStreamSend(byte[] buffer, int offset, int size)
        {
            m_sslStream.Write(buffer, offset, size);
            m_sslStream.Flush();
            return size;
        }

        public bool FinishConnect(ChannelBase channel)
        {
            if (m_Encrypted && m_CompleteProxy && !m_CompletedHandshake)
            {
                bool blcokingMode = m_socket.Blocking;
                m_socket.Blocking = true;
                m_tcpClient = new TcpClient
                {
                    Client = m_socket
                };

                if (m_ServerCertificate != null) // Indicates server's side channel
                {
                    try
                    {
                        // A client has connected. Create the
                        // SslStream using the client's network stream.
                        m_sslStream = new SslStream(
                            m_tcpClient.GetStream(), true);

                        CipherSuitesPolicy cipherSuitesPolicy = null;

                        if (m_CipherSuites != null &&
                            m_CipherSuites.Count() > 0)
                        {
                            cipherSuitesPolicy = new CipherSuitesPolicy(m_CipherSuites);
                        }

                        SslServerAuthenticationOptions sslServerAuthenticationOptions = new SslServerAuthenticationOptions
                        {
                            ServerCertificate = m_ServerCertificate,
                            EnabledSslProtocols = ProtocolFlagsToSslProtocols(m_ProtocolFlags),
                            EncryptionPolicy = EncryptionPolicy.RequireEncryption,
                            ClientCertificateRequired = false,
                            CertificateRevocationCheckMode = System.Security.Cryptography.X509Certificates.X509RevocationMode.NoCheck,
                            AllowRenegotiation = true,
                            CipherSuitesPolicy = cipherSuitesPolicy
                        };

                        m_sslStream.AuthenticateAsServer(sslServerAuthenticationOptions);
                    }
                    catch (Exception ex)
                    {
                        if (m_sslStream != null)
                        {
                            m_sslStream.Close();
                            m_sslStream = null;
                        }

                        if (m_tcpClient != null)
                        {
                            m_tcpClient.Close();
                            m_tcpClient = null;
                        }

                        throw new TransportException($"Failed to create a client encrypted channel. Reason:{ex.Message}");
                    }
                }
                else
                {
                    try
                    {
                        // Create an SSL stream that will close the client's stream.
                         m_sslStream = new SslStream(
                        m_tcpClient.GetStream(),
                        true,
                        new RemoteCertificateValidationCallback(ValidateServerCertificate),
                        null
                        );

                        CipherSuitesPolicy cipherSuitesPolicy = null;

                        if (m_CipherSuites != null &&
                             m_CipherSuites.Count() > 0)
                        {
                            cipherSuitesPolicy = new CipherSuitesPolicy(m_CipherSuites);
                        }

                        SslClientAuthenticationOptions options = new SslClientAuthenticationOptions
                        {
                            TargetHost = m_EncryptedRemoteAddress,
                            CertificateRevocationCheckMode = X509RevocationMode.NoCheck, // Server's certificate may not support this option causing authentication failure.
                            AllowRenegotiation = true,
                            EncryptionPolicy = EncryptionPolicy.RequireEncryption,
                            EnabledSslProtocols = ProtocolFlagsToSslProtocols(m_ProtocolFlags),
                            CipherSuitesPolicy = cipherSuitesPolicy
                        };

                        m_sslStream.AuthenticateAsClient(options);
                    }
                    catch (Exception ex)
                    {
                        if (m_sslStream != null)
                        {
                            m_sslStream.Close();
                            m_sslStream = null;
                        }

                        if (m_tcpClient != null)
                        {
                            m_tcpClient.Close();
                            m_tcpClient = null;
                        }

                        throw new TransportException($"Failed to create an encrypted connection to the remote endpoint. Reason:{ex.Message}");
                    }
                }

                m_socket.Blocking = blcokingMode;
                SetReadWriteHandlers(true);
                m_CompletedHandshake = true;

                /* Set to ChannelBase to cleanup by the Close() function. */
                channel.SslStream = m_sslStream;
                channel.TcpClient = m_tcpClient;

                return true;
            }
            else
            {
                return m_socket.Connected;
            }
        }

        static SslProtocols ProtocolFlagsToSslProtocols(EncryptionProtocolFlags protocolFlags)
        {
            SslProtocols sslProtocols = SslProtocols.None;

            if ((protocolFlags & EncryptionProtocolFlags.ENC_TLSV1_2) != 0)
            {
                sslProtocols |= SslProtocols.Tls12;
            }

            return sslProtocols;
        }

        public void PostProxyInit()
        {
            m_CompleteProxy = true;
        }
    }
}


