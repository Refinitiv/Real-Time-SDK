/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Internal;
using LSEG.Eta.Transports;

namespace LSEG.Eta.Transports
{
    /// <summary>
    /// ETA Connect Options used in the <see cref="Transport.Connect(ConnectOptions, out Error)"/> call
    /// </summary>
    sealed public class ConnectOptions
    {
        /// <summary>
        /// The default constructor
        /// </summary>
       public ConnectOptions()
        {
            Clear();
        }

       /// <summary>
       /// A character representation of component version information.
       /// </summary>
       /// <value>The component version</value>
        public string ComponentVersion { get; set; }

        /// <summary>
        /// Type of connection to establish.
        /// </summary>
        /// <value>The connection type</value>
        public ConnectionType ConnectionType { get; set; } = ConnectionType.SOCKET;

        /// <summary>
        /// Connection parameters when sending and receiving on same network.
        /// This is typically used with<see cref="ConnectionType.SOCKET"/>
        /// </summary>
        /// <vale><see cref="UnifiedNetworkInfo"/></vale>
        public UnifiedNetworkInfo UnifiedNetworkInfo { get; internal set; } = new UnifiedNetworkInfo();

        /// <summary>
        /// Proxy parameters for creating a connection via a proxy server.
        /// </summary>
        /// <value><see cref="ProxyOptions"/></value>
        public ProxyOptions ProxyOptions { get; internal set; } = new ProxyOptions();

        /// <summary>
        /// The type of compression the client would like performed for this
        /// connection. Compression is negotiated between the client and server and
        /// may not be performed if only the client has enabled.
        /// </summary>
        /// <value><see cref="CompressionType"/></value>
        public CompressionType CompressionType { get; set; } = CompressionType.NONE;

        /// <summary>
        /// Checks whether the connection uses lock on read.
        /// </summary>
        /// <value><c>true</c> if the connection users lock on read otherwise <c>false</c></value>
        public bool ChannelReadLocking { get; set; }

        /// <summary>
        /// Checks whether the connection uses lock on write.
        /// </summary>
        /// <value><c>true</c> if the connection users lock on write otherwise <c>false</c></value>
        public bool ChannelWriteLocking { get; set; }

        /// <summary>
        /// If set to true, blocking I/O will be used for this <see cref="IChannel"/>. When
        /// I/O is used in a blocking manner on a <see cref="IChannel"/>, any reading or
        /// writing will complete before control is returned to the application. In
        /// addition, the connect method will complete any initialization on the
        /// <see cref="IChannel"/> prior to returning it.
        ///
        /// Blocking I/O prevents the application from performing any operations
        /// until the I/O operation is completed. Blocking I/O is typically not
        /// recommended. An application can leverage an I/O notification mechanism to
        /// allow efficient reading and writing, while using other cycles to perform
        /// other necessary work in the application. An I/O notification mechanism
        /// enables the application to read when data is available, and write when
        /// output space is available.
        /// </summary>
        /// <value><c>true</c> if this channel is block otherwise <c>false</c></value>
        public bool Blocking { get; set; }

        /// <summary>
        /// The clients desired ping timeout value. This may change through the
        /// negotiation process between the client and the server. After the
        /// connection becomes active, the actual negotiated value becomes available
        /// through the pingTimeout value on the <see cref="IChannel"/>. When determining
        /// the desired ping timeout, the typically used rule of thumb is to send a
        /// heartbeat every pingTimeout/3 seconds. Must be in the range of 1 - 255.
        /// If the value is 0, it will be adjusted to 1, and if the value is greater
        /// than 255, it will be set to 255.
        /// </summary>
        /// <value>The ping timeout</value>
        public int PingTimeout { get; set; } = 60;

        /// <summary>
        /// A guaranteed number of buffers made available for this <see cref="IChannel"/> to
        /// use while writing data. Guaranteed output buffers are allocated at
        /// initialization time. Must be in the range of 0 - 2,147,483,647. If the
        /// argument value is less then 5, the guaranteed number of buffers will be set to 5.
        /// </summary>
        /// <value>The guaranteed number of buffers</value>
        public int GuaranteedOutputBuffers
        {
            get
            {
                return m_GuaranteedOutputBuffers;
            }

            set
            {
                if (value < 5)
                {
                    m_GuaranteedOutputBuffers = 5;
                }
                else
                {
                    m_GuaranteedOutputBuffers = value;
                }
            }
        }

        /// <summary>
        /// The number of sequential input buffers to allocate for reading data into.
        /// This controls the maximum number of bytes that can be handled with a
        /// single network read operation. Input buffers are allocated at
        /// initialization time. Must be in the range of 0 - 2,147,483,647.
        /// </summary>
        /// <value>The number of sequential input buffers</value>
        public int NumInputBuffers { get; set; } = 20;

        /// <summary>
        /// The major version number of the <see cref="IChannel"/>.
        /// If the ETA Codec package is being used, this should be set to
        /// <c>LSEG.Eta.Codec.Codec.MajorVersion()</c>.
        /// </summary>
        /// <value>The major version</value>
        public int MajorVersion { get; set; }

        /// <summary>
        /// The major version of the protocol that the client intends to exchange
        /// over the connection. This value is negotiated with the server at
        /// connection time. The outcome of the negotiation is provided via the
        /// majorVersion information on the <see cref="IChannel"/>. Typically, a major
        /// version increase is associated with the introduction of incompatible
        /// change. The transport layer is data neutral and does not change nor
        /// depend on any information in content being distributed. This information
        /// is provided to help client and server applications manage the information
        /// they are communicating. If the ETA Codec package is being used, this should
        /// be set to <c>LSEG.Eta.Codec.Codec.MinorVersion()</c>.
        /// </summary>
        /// <value>The minor version</value>
        public int MinorVersion { get; set; }

        /// <summary>
        /// The protocol type that the client intends to exchange over the
        /// connection. If the protocolType indicated by a server does not match the
        /// protocolType that a client specifies, the connection will be rejected.
        /// When a <see cref="IChannel"/> becomes active for a client or server, this
        /// information becomes available via the protocolType on the <see cref="IChannel"/>.
        /// The transport layer is data neutral and does not change nor depend on any
        /// information in content being distributed. This information is provided to
        /// help client and server applications manage the information they are communicating.
        /// </summary>
        /// <value><see cref="ProtocolType"/></value>
        public ProtocolType ProtocolType { get; set; }

        /// <summary>
        /// A user specified object. This value is not modified by the transport, but
        /// will be preserved and stored in the UserSpecObject of the <see cref="IChannel"/>
        /// returned from connect. This information can be useful for coupling this
        /// <see cref="IChannel"/> with other user created information, such as a watch list
        /// associated with this connection. 
        /// </summary>
        /// <value>The user specified object</value>
        public object UserSpecObject { get; set; }

        /// <summary>
        /// A substructure containing TCP based connection type specific options.
        /// These settings are used for <see cref="ConnectionType.SOCKET"/>,
        /// and <see cref="ConnectionType.ENCRYPTED"/>.
        /// </summary>
        /// <value><see cref="TcpOpts"/></value>
        public TcpOpts TcpOpts { get; internal set; } = new TcpOpts();

        /// <summary>
        /// The size (in bytes) of the system's send buffer used for this connection,
        /// where applicable.  Setting of 0 indicates to use default sizes.
        /// </summary>
        /// <value>The system's send buffer size</value>
        public int SysSendBufSize { get; set; }

        /// <summary>
        /// The size (in bytes) of the system's receive buffer used for this
        /// connection, where applicable. Setting of 0 indicates to use default
        /// sizes. Must be in the range of 0 - 2,147,483,647.
        /// </summary>
        /// <value>The system's receive buffer size</value>
        public int SysRecvBufSize { get; set; }

        /// <summary>
        /// Gets encryption options to configure an encrypted connection.
        /// </summary>
        /// <value><see cref="EncryptionOptions"/></value>
        public EncryptionOptions EncryptionOpts { get; internal set; } = new EncryptionOptions();

        /// <summary>
        /// Gets or sets the connect timeout when establishing a connect to a remote host in milliseconds.
        /// The default value is 10000 miliseconds.
        /// </summary>
        public int ConnectTimeout { get; set; }

        /// <summary>
        /// The timeout to be set for <see cref="System.Net.Sockets.Socket.SendTimeout"/>
        /// </summary>
        public int SendTimeout { get; set; }

        /// <summary>
        /// The timeout to be set for <see cref="System.Net.Sockets.Socket.ReceiveTimeout"/>
        /// </summary>
        public int ReceiveTimeout { get; set; }

       /// <summary>
       /// Clear all values to default
       /// </summary>
        public void Clear()
        {
            ComponentVersion = null;
            ConnectionType = ConnectionType.SOCKET;
            CompressionType = CompressionType.NONE;
            Blocking = false;
            PingTimeout = 60;
            m_GuaranteedOutputBuffers = 50;
            NumInputBuffers = 10;
            MajorVersion = 0;
            MinorVersion = 0;
            ProtocolType = ProtocolType.RWF;
            UserSpecObject = null;
            TcpOpts.TcpNoDelay = false;
            UnifiedNetworkInfo.Clear();
            SysSendBufSize = 0;
            SysRecvBufSize = 0;
            ChannelReadLocking = false;
            ChannelWriteLocking = false;
            EncryptionOpts.Clear();
            ProxyOptions.Clear();
            ConnectTimeout = 10000;
            SendTimeout = 0;
            ReceiveTimeout = 0;
        }

        /// <summary>
        /// This method will perform a deep copy into the passed in parameter's members from the
        /// ConnectOptions calling this method.
        /// </summary>
        /// <param name="destOpts">the value getting populated with the values of the calling ConnectOptions</param>
        /// <returns><see cref="TransportReturnCode.SUCCESS"/> on success, <see cref="TransportReturnCode.FAILURE"/>
        /// if the destOpts is null.
        /// </returns>
        public TransportReturnCode Copy(ConnectOptions destOpts)
        {
            if (destOpts is null)
                return TransportReturnCode.FAILURE;

            this.CopyTo(destOpts);
            return TransportReturnCode.SUCCESS;
        }

        /// <summary>
        /// The string representation of this object
        /// </summary>
        /// <returns>The string value</returns>
        public override string ToString()
        {
            return $"ConnectionType: {ConnectionType}, Blocking: {Blocking}, UnifiedNetworkInfo: {UnifiedNetworkInfo}";
        }

        private int m_GuaranteedOutputBuffers;
    }
}
