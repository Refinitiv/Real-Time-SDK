/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2024 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Diagnostics;
using System.Net;
using System.Net.Security;
using System.Net.Sockets;
using System.Security.Cryptography.X509Certificates;
using System.Text;

using LSEG.Eta.Common;
using LSEG.Eta.Internal;
using System.Linq;
using System.Security.Authentication;
using System.Collections.Generic;
using LSEG.Eta.Internal.Interfaces;
using System.IO;
using System.Threading;
using System.Threading.Tasks;

namespace LSEG.Eta.Transports
{
    /////////////////////////////////////////////////////////////////////////
    ///
    /// <summary>
    ///   Aggregates Socket object; maintains connection between this object and event sinks.
    /// </summary>
    /// 
    internal sealed class SocketChannel : IDisposable, ISocketChannel
    {
        private Error m_error = new Error();
        /// <summary>
        /// Controls appearence of deep-debugging displays. Manually set during development
        /// by developer.
        /// </summary>
        private static readonly bool debug = false;

        #region Delegates/Events

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

        public bool IsDataReady
        {
            get => m_socket != null && m_socket.Available > 0;
            set => throw new InvalidOperationException("SocketChannel cannot set IsDataReady");
        }

        /// <summary>
        /// Entry-POint at other end of Socket
        /// </summary>
        public EndPoint RemoteEP => m_remote_ep;

        public Socket Socket { get => m_socket; }

        public SslStream SslStream { get => m_sslStream; set => m_sslStream = value; }

        public TcpClient TcpClient { get => m_tcpClient;}

        public bool IsEncrypted { get => m_Encrypted; }

        public bool IsAuthenFailure { get; private set; } = false;

        public string AuthenFailureMessage { get; private set; } = string.Empty;
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
        private int m_AuthTimeout = 5000;

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
        /// <param name="authTimeout">The server authentication timeout</param>
        public SocketChannel(Socket socket, X509Certificate serverCertificate = null, EncryptionProtocolFlags protocolFlags = EncryptionProtocolFlags.ENC_NONE,
            IEnumerable<TlsCipherSuite> cipherSuites = null, int authTimeout = 5000)
        {
            m_receive_packet_size = ResultObject.DefaultBufferSize;
            m_socket = socket;
            m_Encrypted = serverCertificate != null ? true : false;
            m_ServerCertificate = serverCertificate;
            m_ProtocolFlags = protocolFlags;
            m_CipherSuites = cipherSuites;
            m_CompletedHandshake = false;
            m_CompleteProxy = true; /* Always true there is no proxy connection on server side. */
            m_AuthTimeout = authTimeout;
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
                // Create TCP/IP Socket
                m_socket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, System.Net.Sockets.ProtocolType.Tcp)
                {
                    Blocking = true,
                    NoDelay = connectOptions.TcpOpts.TcpNoDelay,
                    ReceiveBufferSize = connectOptions.SysRecvBufSize == 0 ? SocketProtocol.DefaultSystemBufferSize : connectOptions.SysRecvBufSize,
                    SendBufferSize = connectOptions.SysSendBufSize == 0 ? SocketProtocol.DefaultSystemBufferSize : connectOptions.SysSendBufSize,
                    SendTimeout = connectOptions.ConnectTimeout, /* Sets with the ConnectTimeout to establish a connection */
                    ReceiveTimeout = connectOptions.ReceiveTimeout
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

                /* Specify the SendTimeout after the connection is established. */
                m_socket.SendTimeout = connectOptions.SendTimeout;

                m_remote_ep = m_socket.RemoteEndPoint;

                if(connectOptions.ConnectionType == ConnectionType.ENCRYPTED)
                {
                    m_Encrypted = true;
                    m_CompleteProxy = isProxyEnabled ? false : true; // Sets to true if proxy is not enabled for encrypted connection.
                    m_EncryptedRemoteAddress = connectOptions.UnifiedNetworkInfo.Address;
                    m_ProtocolFlags = connectOptions.EncryptionOpts.EncryptionProtocolFlags;
                    m_CipherSuites = connectOptions.EncryptionOpts.TlsCipherSuites;
                    m_AuthTimeout = connectOptions.EncryptionOpts.AuthenticationTimeout;
                }

                SetReadWriteHandlers(false);
                
                m_socket.Blocking = connectOptions.Blocking;
            }
            catch (Exception exp)
            {
                error = new Error
                {
                    ErrorId = TransportReturnCode.FAILURE,
                    Text = $"Failed to connect to the specified remote endpoint. Reason:{exp.Message}"
                };

                Trace.TraceError(@"{0} : {1}", aro, exp.Message);
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
                    m_socket.Disconnect(false);
                }
            }
            catch (Exception exp)
            {
                Trace.TraceError(exp.Message);
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
                byte[] ar = wro.Buffer._data;

                int byteSent = sendHandler(ar, wro.Buffer.Position, ar.Length - wro.Buffer.Position);

                OnSocketWrite(wro, byteSent);
            }
            catch (Exception exp)
            {
                Trace.TraceError(@"{0} : {1}", wro, exp.Message);
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
                }
            }
            catch (IOException ioException)
            {
                if (ioException.InnerException is SocketException socketException)
                {
                    if (socketException.SocketErrorCode != SocketError.WouldBlock &&
                        socketException.SocketErrorCode != SocketError.TryAgain)
                    {
                        error = new Error
                        {
                            ErrorId = TransportReturnCode.FAILURE,
                            Text = ioException.Message
                        };

                        var wro = new ResultObject(m_socket, buffer);
                        Trace.TraceError(@"{0} : {1}", wro, ioException.Message);
                    }
                    else
                    {
                        error = new Error
                        {
                            ErrorId = TransportReturnCode.WRITE_FLUSH_FAILED,
                            SysError = (ioException.InnerException as SocketException).ErrorCode,
                            Text = ioException.InnerException.Message
                        };
                    }
                }
                else
                {
                    error = new Error
                    {
                        ErrorId = TransportReturnCode.FAILURE,
                        Text = ioException.Message
                    };

                    var wro = new ResultObject(m_socket, buffer);
                    Trace.TraceError(@"{0} : {1}", wro, ioException.Message);
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
                    m_error.ErrorId = (socketError == SocketError.WouldBlock ||
                        socketError == SocketError.TryAgain) ? TransportReturnCode.WRITE_FLUSH_FAILED : TransportReturnCode.FAILURE;
                    m_error.SysError = (int)socketError;

                    error = m_error;
                }
            }
            catch (SocketException sockExp)
            {
                m_error.ErrorId = (sockExp.SocketErrorCode == SocketError.WouldBlock ||
                   sockExp.SocketErrorCode == SocketError.TryAgain) ?
                   TransportReturnCode.WRITE_FLUSH_FAILED : TransportReturnCode.FAILURE;
                m_error.Text = sockExp.Message;

                error = m_error;
            }
            catch (Exception exp)
            {
                m_error.ErrorId = TransportReturnCode.FAILURE;
                m_error.Text = exp.Message;

                error = m_error;
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
        {
                    resultObject.Buffer.WritePosition += retResult;
        }
        else
        {
                    retResult = -1; // End of stream
        }

            }
            catch (SocketException socketException)
            {
                if (socketException.SocketErrorCode != SocketError.WouldBlock && 
                    socketException.SocketErrorCode != SocketError.TryAgain)
                {
                    Trace.TraceError(socketException.Message);

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
        {
                    dstBuffer.WritePosition += retResult;
        }
        else
        {
                    retResult = -1; // End of stream
        }
            }
            catch (SocketException socketException)
            {
                if (socketException.SocketErrorCode != SocketError.WouldBlock &&
                    socketException.SocketErrorCode != SocketError.TryAgain)
                {
                    socketError = socketException.SocketErrorCode;
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
                        socketError = socketException.SocketErrorCode;
                        retResult = -1; // End of stream
                    }
                }
            }
            catch (Exception)
            {
                socketError = SocketError.SocketError;
                retResult = -1; // End of stream
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
            }
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

                if (debug)
                {
                    StringBuilder sb = new StringBuilder();
                    sb.AppendFormat("Channel disposing {0}", ToString());
                    Trace.WriteLineIf(debug, sb.ToString());
                }

                m_alreadyDisposed = true;
                if (isDisposing)
                {  // Release the delegates - besides we don't want them firing anyway.

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
                if (m_ServerCertificate != null) // Indicates server's side channel
                {
                    try
                    {
                        if (m_sslStream is null)
                        {
                            bool blockingMode = m_socket.Blocking;
                            m_socket.Blocking = true;
                            m_tcpClient = new TcpClient
                            {
                                Client = m_socket
                            };

                            // A client has connected. Create the
                            // SslStream using the client's network stream.
                            m_sslStream = new SslStream(
                                m_tcpClient.GetStream(), true);

                            CipherSuitesPolicy cipherSuitesPolicy = null;

                            if (m_CipherSuites != null &&
                                m_CipherSuites.Any())
                            {
                                // Platform incompatibility causes CipherSuitesPolicy to throw an
                                // exception which is handled in the "catch" block below
#pragma warning disable CA1416
                                cipherSuitesPolicy = new CipherSuitesPolicy(m_CipherSuites);
#pragma warning restore CA1416
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

                            CancellationTokenSource cs = new (m_AuthTimeout);

                            var authenTask = m_sslStream.AuthenticateAsServerAsync(sslServerAuthenticationOptions);

                            try
                            {
                                m_socket.Blocking = blockingMode;
                                authenTask.Wait(cs.Token);

                                if (m_sslStream.IsAuthenticated && m_sslStream.IsEncrypted)
                                {
                                    SetReadWriteHandlers(true);
                                    m_CompletedHandshake = true;
                                }
                                else
                                {
                                    AuthenFailureMessage = ExtractMessageFromException(authenTask.Exception);
                                    IsAuthenFailure = true;
                                    m_socket.Disconnect(false);
                                }
                            }
                            catch (Exception ex)
                            {
                                AuthenFailureMessage = ExtractMessageFromException(ex);
                                IsAuthenFailure = true;

                                try
                                {
                                    m_socket.Disconnect(false);
                                } catch(Exception) {}
                            }
                        }
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

                        throw new TransportException($"Failed to create a client encrypted channel. Reason:{ExtractMessageFromException(ex)}");
                    }
                }
                else
                {
                    RemoteCertificateValidation certificateValidation = null; ;

                    try
                    {

                        if (m_sslStream is null)
                        {
                            certificateValidation = new();
                            RemoteCertificateValidationCallback validationCallback = certificateValidation.ValidateServerCertificate;

                            bool blockingMode = m_socket.Blocking;
                            m_socket.Blocking = true;
                            m_tcpClient = new TcpClient
                            {
                                Client = m_socket
                            };

                            // Create an SSL stream that will close the client's stream.
                            m_sslStream = new SslStream(
                                m_tcpClient.GetStream(),
                                true,
                                validationCallback,
                                null);

                            CipherSuitesPolicy cipherSuitesPolicy = null;

                            if (m_CipherSuites != null &&
                                 m_CipherSuites.Any())
                            {
                                // Platform incompatibility causes CipherSuitesPolicy to throw an
                                // exception which is handled in the "catch" block below
#pragma warning disable CA1416
                                cipherSuitesPolicy = new CipherSuitesPolicy(m_CipherSuites);
#pragma warning restore CA1416
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

                            CancellationTokenSource cs = new (m_AuthTimeout);

                            var authenTask = m_sslStream.AuthenticateAsClientAsync(options);

                            Task.Run(() => {

                                try
                                {
                                    authenTask.Wait(cs.Token);

                                    if (m_sslStream.IsAuthenticated && m_sslStream.IsEncrypted)
                                    {
                                        SetReadWriteHandlers(true);
                                        m_CompletedHandshake = true;
                                    }
                                    else
                                    {
                                        AuthenFailureMessage = ExtractMessageFromException(authenTask.Exception);
                                        IsAuthenFailure = true;
                                    }
                                }
                                catch (Exception ex)
                                {
                                    AuthenFailureMessage = ExtractMessageFromException(ex);
                                    IsAuthenFailure = true;
                                }
                            });

                            m_socket.Blocking = blockingMode;
                        }
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

                        if (certificateValidation != null && certificateValidation.SslPolicyErrors != SslPolicyErrors.None)
                        {
                            throw new TransportException($"Failed to create an encrypted connection to the remote endpoint." +
                                $"Reason:{ExtractMessageFromException(ex)} {certificateValidation.SslPolicyErrors}.");
                        }
                        else
                        {
                            throw new TransportException($"Failed to create an encrypted connection to the remote endpoint. Reason:{ExtractMessageFromException(ex)}");
                        }
                    }
                }

                /* Set to ChannelBase to cleanup by the Close() function. */
                if (channel.SslStream is null)
                {
                    channel.SslStream = m_sslStream;
                    channel.TcpClient = m_tcpClient;
                }

                return m_CompletedHandshake;
            }
            else
            {
                // This is used to indicate that the proxy is enabled for the encrypted connection and it is in progress to complete.
                if (m_CompleteProxy == false)
                {
                    return m_socket.Connected;
                }
                else
                {
                    return m_Encrypted ? m_CompletedHandshake : m_socket.Connected;
                }
            }
        }

        static SslProtocols ProtocolFlagsToSslProtocols(EncryptionProtocolFlags protocolFlags)
        {
            SslProtocols sslProtocols = SslProtocols.None;

            if ((protocolFlags & EncryptionProtocolFlags.ENC_TLSV1_2) != 0)
            {
                sslProtocols |= SslProtocols.Tls12;
            }

            if ((protocolFlags & EncryptionProtocolFlags.ENC_TLSV1_3) != 0)
            {
                sslProtocols |= SslProtocols.Tls13;
            }

            return sslProtocols;
        }

        public void PostProxyInit()
        {
            m_CompleteProxy = true;
        }

        static string ExtractMessageFromException(Exception exception)
        {
            string exceptionMessage = string.Empty;
            if(exception is not null)
            {
                if(exception.InnerException is not null)
                {
                    if (exception.InnerException.InnerException is not null)
                    {
                        exceptionMessage = exception.InnerException.InnerException.Message;
                    }
                    else
                    {
                        exceptionMessage = exception.InnerException.Message;
                    }
                }
                else
                {
                    exceptionMessage = exception.Message;
                }
            }

            return exceptionMessage;
        }
    }

    internal class RemoteCertificateValidation
    {
        public SslPolicyErrors SslPolicyErrors { get; private set; }

        public bool ValidateServerCertificate(
             object sender,
#nullable enable
             X509Certificate? certificate,
             X509Chain? chain,
#nullable disable
             SslPolicyErrors sslPolicyErrors)
        {
            if (sslPolicyErrors == SslPolicyErrors.None)
                return true;

            SslPolicyErrors = sslPolicyErrors;

            return false;
        }
    }

}


