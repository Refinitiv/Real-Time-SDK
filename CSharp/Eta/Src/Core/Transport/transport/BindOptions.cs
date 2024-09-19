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
    /// ETA Bind Options used in the <see cref="Transport.Bind(BindOptions, out Error)"/> call.
    /// </summary>
    sealed public class BindOptions
    {
        /// <summary>
        /// The default constructor to clear all options to default values.
        /// </summary>
        public BindOptions()
        {
            Clear();
        }

        /// <summary>
        /// Gets or sets a string representation of a component version information.
        /// </summary>
        /// <value>The component version</value>
        public string ComponentVersion { get; set; }

        /// <summary>
        /// Gets or sets a string representation of a numeric port number or service name
        /// (as defined in etc/services file) to bind and open a listening socket on.
        /// </summary>
        /// <value>The service name</value>
        public string ServiceName { get; set; }

        /// <summary>
        /// Gets or sets a string representation of an IP address or hostname asociated
        /// with the local network interface to bind to. ETA Transport will establish
        /// connections on the specified interface. This value is intended for se in 
        /// systems which have multiple network interface cards. If the loopback address
        /// is specified, connections can be accepted only when instantiating from from
        /// the local machine.
        /// </summary>
        /// <value>The interface name</value>
        public string InterfaceName { get; set; }

        /// <summary>
        /// Gets or sets the type of compression the servr would like to perform for
        /// this connection. Compression is negotiated between the client and server
        /// and may not be performed if only the server has this enabled. The server
        /// can force compression, regrdless of client settings, by using the
        /// ForceOption option.
        /// </summary>
        /// <value>The compression type</value>
        public CompressionType CompressionType { get; set; }

        /// <summary>
        /// Gets or sets the level of compression to apply.</summary>
        /// <remarks>
        /// Allowable compression level values are 0 to 9.
        /// A compression level of 1 results in the fastest compression.
        /// A compression level of 9 results in the best compression.
        /// A compression level of 6 is a compromise between speed and compression.
        /// A compression level of 0 will apply the data with no compression applied.
        /// Compression level must be int the range of 0 - 9.
        /// </remarks>
        /// <vaue>The compression level</vaue>
        public int CompressionLevel { get; set; }

        /// <summary>
        /// Gets or sets force compression from server.
        /// </summary>
        /// <remarks>
        /// If <c>true</c>, force compression is enabled, regardless of clients
        /// desire for compression. When enabled, compression will use the
        /// <see cref="CompressionType"/> and <see cref="CompressionLevel"/>
        /// specified by server. if set <c>false</c>, the compression negotiation
        /// algorithm will be used to determine compression setting.
        /// </remarks>
        /// <value>The force compression</value>
        public bool ForceCompression { get; set; }

        /// <summary>
        /// Gets or sets wheter blocking I/O will be used for this <see cref="IServer"/>.
        /// </summary>
        /// <remarks>
        /// If set to <c>true</c>, blocking I/O will be used for this <see cref="IServer"/>,
        /// the accept method will complete any initialization on the <see cref="IChannel"/> 
        /// prior to returning it. Blocking I/O prevents the application from performing
        /// any until the I/O operation is completed. Blocking I/O is typically not recommended.
        /// An application can leverage an I/O notification mechanism to allow efficient use,
        /// while using other cycles to perform other necessary work in the application.
        /// </remarks>
        /// <value>The server blocking</value>
        public bool ServerBlocking { get; set; }

        /// <summary>
        /// Gets or sets blocking I/O will be used for all connected <see cref="IChannel"/>.
        /// </summary>
        /// <remarks>
        /// If <c>true</c>, blocking I/O will be used for all connected <see cref="IChannel"/>.
        /// When I/O is used in a blocking manner on a <see cref="IChannel"/>, any reading or
        /// writing will complete before control is returned to the application. Blocking I/O
        /// prevents the application from performing any operations until the I/O operation is
        /// completed. Blocking I/O is typically not recommended. An application can leverage
        /// an I/O notification mechanism to allow efficient reading and writing, while using
        /// other cycles to perform other necessary work in the application. An I/O notification
        /// mechanism enables the application to read when data is avaliable, and write when
        /// output space is available.
        /// </remarks>
        /// <value>The channel is blocking</value>
        public bool ChannelIsBlocking { get; set; }

        /// <summary>
        /// Gets or sets whether heartbeat messages are required to flow from the server to
        /// the client. 
        /// </summary>
        /// <remarks>
        /// LSEG Real-Time Distribution System and other Refinitiv components typically
        /// require this value to be set to <c>true</c>.
        /// </remarks>
        /// <value><c>true</c> indicates heartbeat messages are required from the server otherwise <c>false</c></value>
        public bool ServerToClientPings { get; set; }

        /// <summary>
        /// Gets or sets whether messages are required from the client to the server.
        /// </summary>
        /// <remarks>
        /// LSEG Real-Time Distribution System and other Refinitiv components typically
        /// require this value to be set to <c>true</c>.
        /// </remarks>
        /// <value><c>true</c> indicates heartbeat messages are required from the client otherwise <c>false</c></value>
        public bool ClientToServerPings { get; set; }

        /// <summary>
        /// Gets or set the type of underlying connection being used.
        /// </summary>
        /// <remarks>
        /// Must be either <see cref="ConnectionType.SOCKET"/> or <see cref="ConnectionType.ENCRYPTED"/>
        /// </remarks>
        /// <value>The <see cref="ConnectionType"/></value>
        public ConnectionType ConnectionType { get; set; }

        /// <summary>
        /// Gets or sets the server's maximum allowable ping timeout value.
        /// </summary>
        /// <remarks>
        /// This is the largest possible value allowed in the negotiation between client and server's
        /// ping timeout values. After the connection becomes active, the actual negotiated value becomes 
        /// available through the <see cref="IChannel.PingTimeOut"/>. When determining the desired ping
        /// timeout, the typically used rule of thumb is to send a heartbeat every <see cref="PingTimeout"/>/3
        /// seconds. Must be in the range of 1 - 255. If the value is 0, it will be adjusted to 1, and
        /// if the value is greater than 255, it will be set to 255.
        /// </remarks>
        /// <value>The ping timeout</value>
        public int PingTimeout { get; set; }

        /// <summary>
        /// Gets or set the server's lowest allowable ping timeout value. 
        /// </summary>
        /// <remarks>
        /// This is the lowest possible value allowed in the negotiation between client and server's 
        /// ping timeout values. After the connection becomes active, the actual negotiated value 
        /// becomes available through the <see cref="IChannel.PingTimeOut"/>. When determining the
        /// desired ping timeout, the typically used rule of thumb is to send a heartbeat every
        /// <see cref="PingTimeout"/>/3 seconds. Must be in the range of 1 - 255. If the value is 0,
        /// it will be adjusted to 1, and if the value is greater than 255, it will be set to 255.
        /// </remarks>
        /// <value>The min ping timeout</value>
        public int MinPingTimeout { get; set; }

        /// <summary>
        /// Gets or sets the maximum size buffer that will be written to the network. 
        /// </summary>
        /// <remarks>
        /// If a larger buffer is required, ETA Transport will internally fragment the larger buffer
        /// into smaller <see cref="MaxFragmentSize"/> buffers. This is different from application
        /// level message fragmentation via the Message Package. Any guaranteed, shred, or input buffers
        /// created will use this size. This value is passed to all connected client applications and
        /// enforces a common message size between components. Must be in the range of 20 - 2,147,483,647.
        /// If the value is outside of this range, it will be set to default value of 6144.
        /// </remarks>
        /// <value>The max fragment size</value>
        public int MaxFragmentSize
        {
            get
            {
                return m_MaxFragmentSize;
            }

            set
            {
                if( value >= 20 )
                {
                    if( value <= 0xFFFF) // the maximum RIPC message size(0xFFFF)
                    {
                        m_MaxFragmentSize = value;
                    }
                }
            }
        }

        /// <summary>
        /// Gets or sets the maximum number of output buffers allowed for use by each <see cref="IChannel"/>.
        /// (<see cref="MaxOutputBuffers"/> - <see cref="GuaranteedOutputBuffers"/>) is equal to the number
        /// of shared pool buffers that each <see cref="IChannel"/> is allowed to use. Shared pool buffers
        /// are only used if all <see cref="GuaranteedOutputBuffers"/> are unavailable. If equal to
        /// <see cref="GuaranteedOutputBuffers"/> value, no shared pool buffers are available.
        /// Must be in the range of 0 - 2,147,483,647.
        /// </summary>
        /// <value>The max output buffers</value>
        public int MaxOutputBuffers 
        { 
            get
            {
                return m_MaxOutputBuffers;
            }
            
            set
            {
                if(value > 0)
                {
                    if(m_MaxOutputBuffers < m_GuaranteedOutputBuffers)
                    {
                        m_MaxOutputBuffers = m_GuaranteedOutputBuffers;
                    }
                    else
                    {
                        m_MaxOutputBuffers = value;
                    }
                }
            }
        }

        /// <summary>
        /// Gets or sets a guaranteed number of buffers made available for each <see cref="IChannel"/> to use
        /// while writing data. 
        /// </summary>
        /// <remarks>
        /// Each buffer will be created to contain <see cref="MaxFragmentSize"/> bytes. Guaranteed output 
        /// buffers are allocated at initialization time. Must be in the range of 0 - 2,147,483,647. If the
        /// argument value is less than 5, the guaranteed number of buffer will be set to 5.
        /// </remarks>
        /// <value>The guaranteed output buffers</value>
        public int GuaranteedOutputBuffers
        {
            get 
            { 
                return m_GuaranteedOutputBuffers; 
            }

            set
            {
                if(value < 5)
                {
                    m_GuaranteedOutputBuffers = 5;
                }
                else
                {
                    m_GuaranteedOutputBuffers = value;
                }

                // Max output buffers must be at least as large as guaranteed output buffers.
                if(m_GuaranteedOutputBuffers > m_MaxOutputBuffers)
                {
                    m_MaxOutputBuffers = m_GuaranteedOutputBuffers;
                }
                
            }
        }

        /// <summary>
        /// Gets or sets the number of input buffers(of <see cref="MaxFragmentSize"/> for reading data into.
        /// </summary>
        /// <remarks>
        /// This controls the maximum number of bytes that can be handled with a single read operation on
        /// each channel. Must be in the range of 0 - 2,147,483,647. Input buffers are allocated at 
        /// initialization time.
        /// </remarks>
        /// <vale>The number of input buffers</vale>
        public int NumInputBuffers { get; set; }

        /// <summary>
        /// Gets or sets the maximum number of buffers to make available as part of the shared buffer pool.
        /// </summary>
        /// <remarks>
        /// The shared buffer pool can be drawn upon by any connected <see cref="IChannel"/>, where each
        /// channel is allowed to use up to (<see cref="MaxOutputBuffers"/> - <see cref="GuaranteedOutputBuffers"/>)
        /// number of buffers. Each shared pool buffer will be created to contain <see cref="MaxFragmentSize"/> bytes.
        /// The shared pool is not fully allocated at bind time. As needed, shared pool buffers are added and
        /// reused until the server is shut down.
        /// </remarks>
        /// <value>The shared pool size</value>
        public int SharedPoolSize { get; set; }

        /// <summary>
        /// Gets or sets whether the shared buffer pool will have its own locking performed.
        /// </summary>
        /// <remarks>
        /// This setting is independent of any initialize locking mode options. Enabling a shared pool lock
        /// allows shared pool use to remain thread safe while still disabling channel locking.
        /// </remarks>
        /// <value><c>true</c> to enable the shared pool lock otherwise <c>false</c></value>
        public bool SharedPoolLock { get; set; }

        /// <summary>
        /// Gets or sets the major version of the protocol that the server is capable of exchanging over
        /// all connections.
        /// </summary>
        /// <remarks>
        /// The value is negotiated with the client at connection time. The outcome of the negotiation is
        /// provided via the <see cref="IChannel.MajorVersion"/>. Typically, a major version increase is
        /// associated with the introduction of incompatible change. The transport layer is data neutral
        /// and does not change nor depend on any information in content being distributed. This information
        /// is provided to help client and server applications manage the information they are communicating.
        /// </remarks>
        /// <value>The major version</value>
        public int MajorVersion { get; set; }

        /// <summary>
        /// Gets or sets the minor version of the protocol that the server is capable of exchanging over
        /// all connections. 
        /// </summary>
        /// <remarks>
        /// This value is negotiated with the client at connection time. The outcome of the negotiation is
        /// provided via the <see cref="IChannel.MinorVersion"/>. Typically, a minor version increase is
        /// associated with a fully backward compatible change or extension. The transport layer is data
        /// neutral and does not change nor depend on any information in content being distributed. This
        /// information is provided to help client and server applications manage the information they are
        /// communicating.
        /// </remarks>
        /// <value>The minor version</value>
        public int MinorVersion { get; set; }

        /// <summary>
        /// Gets or sets the protocol type that server intends to exchange over all connection established
        /// to it.
        /// </summary>
        /// <remarks>
        /// If the protocol type indicated by a client does not match the protocol type that a server specifies,
        /// the connection will be rejected. When a <see cref="IChannel"/> becomes active for a client or server,
        /// this information becomes available via the <see cref="IChannel.ProtocolType"/>. The transport layer
        /// is data neutral and does not change nor depend on any information in content being distributed. This
        /// information is provided to help client and server application manage the information theyare
        /// communicating.
        /// </remarks>
        /// <value>The protocol type</value>
        public ProtocolType ProtocolType { get; set; }

        /// <summary>
        /// The size (in kilobytes) of the system's send buffer used for this connection,
        /// where applicable.  Setting of 0 indicates to use default sizes.
        /// </summary>
        /// <value>The system's send buffer size</value>
        public int SysSendBufSize { get; set; }

        /// <summary>
        /// The size (in kilobytes) of the system's receive buffer used for this
        /// connection, where applicable. Setting of 0 indicates to use default
        /// sizes. Must be in the range of 0 - 2,147,483,647.
        /// </summary>
        /// <value>The system's receive buffer size</value>
        public int SysRecvBufSize { get; set; }

        /// <summary>
        /// A user specified object that can be set by the application. This value is not modified by the transport,
        /// but will be preserved and stored in the <see cref="IServer.UserSpecObject"/> returned from 
        /// <see cref="Transport.Bind(BindOptions, out Error)"/>.
        /// </summary>
        /// <remarks>
        /// This information can be useful for coupling this <see cref="IServer"/> with other user created information,
        /// such as a list of connected <see cref="IChannel"/> structures.
        /// </remarks>
        /// <value>The user specified object</value>
        public object UserSpecObject { get; set; }

        /// <summary>
        /// A substructure containing TCP based connection type specific options.
        /// These settings are used for <see cref="ConnectionType.SOCKET"/>, and <see cref="ConnectionType.ENCRYPTED"/>.
        /// </summary>
        /// <value><see cref="TcpOpts"/></value>
        public TcpOpts TcpOpts { get; internal set; } = new TcpOpts();

        /// <summary>
        /// Gets encryption options to configure an encrypted connection.
        /// </summary>
        /// <value><see cref="BindEncryptionOptions"/></value>
        public BindEncryptionOptions BindEncryptionOpts { get; internal set; } = new BindEncryptionOptions();

        /// <summary>
        /// This method will perform a deep copy into the passed in parameter's members from the
        /// BindOptions calling this method.
        /// </summary>
        /// <param name="destOpts">the value getting populated with the values of the calling BindOptions</param>
        /// <returns><see cref="TransportReturnCode.SUCCESS"/> on success, <see cref="TransportReturnCode.FAILURE"/>
        /// if the destOpts is null.
        /// </returns>
        public TransportReturnCode Copy(BindOptions destOpts)
        {
            if (destOpts is null)
                return TransportReturnCode.FAILURE;

            this.CopyTo(destOpts);
            return TransportReturnCode.SUCCESS;
        }

        /// <summary>
        /// Clears Bind options.
        /// </summary>
        public void Clear()
        {
            ComponentVersion = null;
            ServiceName = null;
            InterfaceName = null;
            CompressionType = CompressionType.NONE;
            CompressionLevel = 0;
            ForceCompression = false;
            ServerBlocking = false;
            ChannelIsBlocking = false;
            ServerToClientPings = true;
            ClientToServerPings = true;
            ConnectionType = ConnectionType.SOCKET;
            PingTimeout = 60;
            MinPingTimeout = 20;
            m_MaxFragmentSize = 6144;
            NumInputBuffers = 10;
            SharedPoolSize = 0;
            SharedPoolLock = false;
            MajorVersion = 0;
            MinorVersion = 0;
            ProtocolType = 0;
            SysSendBufSize = 0;
            SysRecvBufSize = 0;
            UserSpecObject = null;
            TcpOpts.TcpNoDelay = false;
            BindEncryptionOpts.Clear();
            m_MaxOutputBuffers = 50;
            m_GuaranteedOutputBuffers = 50;
        }

        private int m_MaxOutputBuffers;
        private int m_GuaranteedOutputBuffers;
        private int m_MaxFragmentSize;
    }
}
