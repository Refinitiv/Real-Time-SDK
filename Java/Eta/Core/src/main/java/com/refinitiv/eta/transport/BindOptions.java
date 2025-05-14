/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport;

/**
 * ETA Bind Options used in the {@link Transport#bind(BindOptions, Error)} call.
 *
 * @see Transport
 */
public interface BindOptions
{

    /**
     * A character representation of a component version information.
     *
     * @param componentVersion the component version
     */
    public void componentVersion(String componentVersion);

    /**
     * A character representation of a component version information.
     *
     * @return the componentVersion
     */
    public String componentVersion();

    /**
     * A character representation of a numeric port number or service name (as
     * defined in etc/services file) to bind and open a listening socket on.
     *
     * @param serviceName the serviceName to set. The serviceName must be non null.
     */
    public void serviceName(String serviceName);

    /**
     * A character representation of a numeric port number or service name (as
     * defined in etc/services file) to bind and open a listening socket on.
     *
     * @return the serviceName
     */
    public String serviceName();

    /**
     * A character representation of an IP address or hostname associated with
     * the local network interface to bind to. ETA Transport will establish
     * connections on the specified interface. This value is intended for use in
     * systems which have multiple network interface cards. If not populated,
     * connection can be accepted on all interfaces. If the loopback address is
     * specified, connections can be accepted only when instantiating from the local machine.
     *
     * @param interfaceName the interfaceName to set
     */
    public void interfaceName(String interfaceName);

    /**
     * A character representation of an IP address or hostname associated with
     * the local network interface to bind to. ETA Transport will establish
     * connections on the specified interface. This value is intended for use in
     * systems which have multiple network interface cards. If not populated,
     * connection can be accepted on all interfaces. If the loopback address is
     * specified, connections can be accepted only when instantiating from the local machine.
     *
     * @return the interfaceName
     */
    public String interfaceName();

    /**
     * The type of compression the server would like performed for this
     * connection. Compression is negotiated between the client and server and
     * may not be performed if only the server has this enabled. The server can
     * force compression, regardless of client settings, by using the
     * forceCompression option
     * Must be in the range of {@link CompressionTypes#NONE} - {@link CompressionTypes#LZ4}.
     *
     * @param compressionType the compressionType to set
     *
     * @see CompressionTypes
     */
    public void compressionType(int compressionType);

    /**
     * The type of compression the server would like performed for this
     * connection. Compression is negotiated between the client and server and
     * may not be performed if only the server has this enabled. The server can
     * force compression, regardless of client settings, by using the
     * forceCompression option.
     *
     * @return the compressionType
     *
     * @see CompressionTypes
     */
    public int compressionType();

    /**
     * Determines the level of compression to apply.
     * Allowable compressionLevel values are 0 to 9.
     * A compressionLevel of 1 results in the fastest compression.
     * A compressionLevel of 9 results in the best compression.
     * A compressionLevel of 6 is a compromise between speed and compression.
     * A compressionLevel of 0 will copy the data with no compression applied.
     * Compression level must be in the range of 0 - 9.
     *
     * @param compressionLevel the compressionLevel to set
     */
    public void compressionLevel(int compressionLevel);

    /**
     * Determines the level of compression to apply. Allowable compressionLevel
     * values are 0 to 9. A compressionLevel of 1 results in the fastest
     * compression. A compressionLevel of 9 results in the best compression. A
     * compressionLevel of 6 is a compromise between speed and compression. A
     * compressionLevel of 0 will copy the data with no compression applied.
     *
     * @return the compressionLevel
     */
    public int compressionLevel();

    /**
     * If set to true, this will force compression to be enabled, regardless of
     * clients desire for compression. When enabled, compression will use the
     * compressionType and compressionLevel specified by the server. If set to
     * false, the compression negotiation algorithm will be used to determine
     * compression setting.
     *
     * @param forceCompression the forceCompression to set
     */
    public void forceCompression(boolean forceCompression);

    /**
     * If true, force compression is enabled, regardless of clients desire for
     * compression. When enabled, compression will use the compressionType and
     * compressionLevel specified by the server. If set to false, the
     * compression negotiation algorithm will be used to determine compression setting.
     *
     * @return the forceCompression
     */
    public boolean forceCompression();

    /**
     * If set to true, blocking I/O will be used for this {@link Server}. When
     * I/O is used in a blocking manner on a {@link Server}, the accept
     * method will complete any initialization on the {@link Channel} prior to
     * returning it. Blocking I/O prevents the application from performing any
     * operations until the I/O operation is completed. Blocking I/O is
     * typically not recommended. An application can leverage an I/O
     * notification mechanism to allow efficient use, while using other cycles
     * to perform other necessary work in the application.
     *
     * @param serverBlocking the serverBlocking to set
     */
    public void serverBlocking(boolean serverBlocking);

    /**
     * If set to true, blocking I/O will be used for this {@link Server}. When
     * I/O is used in a blocking manner on a {@link Server}, the accept
     * method will complete any initialization on the {@link Channel} prior to
     * returning it. Blocking I/O prevents the application from performing any
     * operations until the I/O operation is completed. Blocking I/O is
     * typically not recommended. An application can leverage an I/O
     * notification mechanism to allow efficient use, while using other cycles
     * to perform other necessary work in the application.
     *
     * @return the serverBlocking
     */
    public boolean serverBlocking();

    /**
     * If true, blocking I/O will be used for all connected {@link Channel}
     * structures. When I/O is used in a blocking manner on a {@link Channel},
     * any reading or writing will complete before control is returned to the
     * application. Blocking I/O prevents the application from performing any
     * operations until the I/O operation is completed. Blocking I/O is
     * typically not recommended. An application can leverage an I/O
     * notification mechanism to allow efficient reading and writing, while
     * using other cycles to perform other necessary work in the application. An
     * I/O notification mechanism enables the application to read when data is
     * available, and write when output space is available.
     *
     * @param channelsBlocking the channelsBlocking to set
     */
    public void channelsBlocking(boolean channelsBlocking);

    /**
     * If true, blocking I/O is used for all connected {@link Channel}
     * structures. When I/O is used in a blocking manner on a {@link Channel},
     * any reading or writing will complete before control is returned to the
     * application. Blocking I/O prevents the application from performing any
     * operations until the I/O operation is completed. Blocking I/O is
     * typically not recommended. An application can leverage an I/O
     * notification mechanism to allow efficient reading and writing, while
     * using other cycles to perform other necessary work in the application. An
     * I/O notification mechanism enables the application to read when data is
     * available, and write when output space is available.
     *
     * @return the channelsBlocking
     */
    public boolean channelsBlocking();

    /**
     * If set to true, heartbeat messages are required to flow from the server
     * to the client. If set to false, the server is not required to send
     * heartbeats. LSEG Real-Time Distribution System and other LSEG components
     * typically require this value to be set to true.
     *
     * @param serverToClientPings the serverToClientPings to set
     */
    public void serverToClientPings(boolean serverToClientPings);

    /**
     * If set to true, heartbeat messages are required to flow from the server
     * to the client. If set to false, the server is not required to send
     * heartbeats. LSEG Real-Time Distribution System and other LSEG components
     * typically require this value to be set to true.
     *
     * @return the serverToClientPings
     */
    public boolean serverToClientPings();

    /**
     * If set to true, heartbeat messages are required to flow from the client
     * to the server. If set to false, the client is not required to send
     * heartbeats. LSEG Real-Time Distribution System and other LSEG components
     * typically require this to be set to true.
     *
     * @param clientToServerPings the clientToServerPings to set
     */
    public void clientToServerPings(boolean clientToServerPings);

    /**
     * If set to true, heartbeat messages are required to flow from the client
     * to the server. If set to false, the client is not required to send
     * heartbeats. LSEG Real-Time Distribution System and other LSEG components
     * typically require this to be set to true.
     *
     * @return the clientToServerPings
     */
    public boolean clientToServerPings();

    /**
     * An enumerated value that indicates the type of underlying connection
     * being used. Must be either {@link ConnectionTypes#SOCKET} or
     * {@link ConnectionTypes#ENCRYPTED}.
     * @param connectionType the connectionType to set
     *
     * @see ConnectionTypes
     */
    public void connectionType(int connectionType);

    /**
     * An enumerated value that indicates the type of underlying connection being used.
     *
     * @return the connectionType
     *
     * @see ConnectionTypes
     */
    public int connectionType();

    /**
     * The server's maximum allowable ping timeout value. This is the largest
     * possible value allowed in the negotiation between client and server's
     * pingTimeout values. After the connection becomes active, the actual
     * negotiated value becomes available through the pingTimeout value on the
     * {@link Channel}. When determining the desired ping timeout, the typically
     * used rule of thumb is to send a heartbeat every pingTimeout/3 seconds.
     * Must be in the range of 1 - 255. If the value is 0, it will be adjusted
     * to 1, and if the value is greater than 255, it will be set to 255.
     *
     * @param pingTimeout the pingTimeout to set
     */
    public void pingTimeout(int pingTimeout);

    /**
     * The server's maximum allowable ping timeout value. This is the largest
     * possible value allowed in the negotiation between client and server's
     * pingTimeout values. After the connection becomes active, the actual
     * negotiated value becomes available through the pingTimeout value on the
     * {@link Channel}. When determining the desired ping timeout, the typically
     * used rule of thumb is to send a heartbeat every pingTimeout/3 seconds.
     *
     * @return the pingTimeout
     */
    public int pingTimeout();

    /**
     * The server's lowest allowable ping timeout value. This is the lowest
     * possible value allowed in the negotiation between client and server's
     * pingTimeout values. After the connection becomes active, the actual
     * negotiated value becomes available through the pingTimeout value on the
     * {@link Channel}. When determining the desired ping timeout, the typically
     * used rule of thumb is to send a heartbeat every pingTimeout/3 seconds.
     * Must be in the range of 1 - 255.
     * If the value is 0, it will be adjusted to 1,
     * and if the value is greater than 255, it will be set to 255.
     *
     * @param minPingTimeout the minPingTimeout to set
     */
    public void minPingTimeout(int minPingTimeout);

    /**
     * The server's lowest allowable ping timeout value. This is the lowest
     * possible value allowed in the negotiation between client and server's
     * pingTimeout values. After the connection becomes active, the actual
     * negotiated value becomes available through the pingTimeout value on the
     * {@link Channel}. When determining the desired ping timeout, the typically
     * used rule of thumb is to send a heartbeat every pingTimeout/3 seconds.
     *
     * @return the minPingTimeout
     */
    public int minPingTimeout();

    /**
     * The maximum size buffer that will be written to the network. If a larger
     * buffer is required, ETA Transport will internally fragment the larger
     * buffer into smaller maxFragmentSize buffers. This is different from
     * application level message fragmentation done via the Message Package. Any
     * guaranteed, shared, or input buffers created will use this size. This
     * value is passed to all connected client applications and enforces a
     * common message size between components.
     * Must be in the range of 20 - 2,147,483,647.
     * If the value is outside of this range, it will be set to default value of 6144.
     *
     * @param maxFragmentSize the maxFragmentSize to set
     */
    public void maxFragmentSize(int maxFragmentSize);

    /**
     * The maximum size buffer that will be written to the network. If a larger
     * buffer is required, ETA Transport will internally fragment the larger
     * buffer into smaller maxFragmentSize buffers. This is different from
     * application level message fragmentation done via the Message Package. Any
     * guaranteed, shared, or input buffers created will use this size. This
     * value is passed to all connected client applications and enforces a
     * common message size between components.
     *
     * @return the maxFragmentSize
     */
    public int maxFragmentSize();

    /**
     * The maximum number of output buffers allowed for use by each
     * {@link Channel}. (maxOutputBuffers - guaranteedOutputBuffers) is equal to
     * the number of shared pool buffers that each {@link Channel} is allowed to
     * use. Shared pool buffers are only used if all guaranteedOutputBuffers are
     * unavailable. If equal to the guaranteedOutputBuffers value, no shared
     * pool buffers are available.
     * Must be in the range of 0 - 2,147,483,647.
     *
     * @param maxOutputBuffers the maxOutputBuffers to set
     */
    public void maxOutputBuffers(int maxOutputBuffers);

    /**
     * The maximum number of output buffers allowed for use by each
     * {@link Channel}. (maxOutputBuffers - guaranteedOutputBuffers) is equal to
     * the number of shared pool buffers that each {@link Channel} is allowed to
     * use. Shared pool buffers are only used if all guaranteedOutputBuffers are
     * unavailable. If equal to the guaranteedOutputBuffers value, no shared
     * pool buffers are available.
     *
     * @return the maxOutputBuffers
     */
    public int maxOutputBuffers();

    /**
     * A guaranteed number of buffers made available for each {@link Channel} to
     * use while writing data. Each buffer will be created to contain
     * maxFragmentSize bytes. Guaranteed output buffers are allocated at initialization time.
     * Must be in the range of 0 - 2,147,483,647.
     * If the argument value is less then 5, the guaranteed number of buffers will be set to 5.
     *
     * @param guaranteedOutputBuffers the guaranteedOutputBuffers to set
     */
    public void guaranteedOutputBuffers(int guaranteedOutputBuffers);

    /**
     * A guaranteed number of buffers made available for each {@link Channel} to
     * use while writing data. Each buffer will be created to contain
     * maxFragmentSize bytes. Guaranteed output buffers are allocated at
     * initialization time.
     *
     * @return the guaranteedOutputBuffers
     */
    public int guaranteedOutputBuffers();

    /**
     * Sets the number of input buffers (of maxFragmentSize) for reading data into.
     * Must be in the range of 0 - 2,147,483,647.
     *
     * @param numInputBuffers the numInputBuffers to set
     */
    public void numInputBuffers(int numInputBuffers);

    /**
     * The number of sequential input buffers used by each {@link Channel} for
     * reading data into. This controls the maximum number of bytes that can be
     * handled with a single network read operation on each channel. Each input
     * buffer will be created to contain maxFragmentSize bytes. Input buffers
     * are allocated at initialization time.
     *
     * @return the numInputBuffers
     */
    public int numInputBuffers();

    /**
     * The maximum number of buffers to make available as part of the shared
     * buffer pool. The shared buffer pool can be drawn upon by any connected
     * {@link Channel}, where each channel is allowed to use up to
     * (maxOutputBuffers - guaranteedOutputBuffers) number of buffers. Each
     * shared pool buffer will be created to contain maxFragmentSize bytes.
     * Must be in the range of 0 - 2,147,483,647. If set to 0, a default of
     * 1,048,567 shared pool buffers will be allowed. The shared pool is not
     * fully allocated at bind time. As needed, shared pool buffers are added
     * and reused until the server is shut down.
     *
     * @param sharedPoolSize the sharedPoolSize to set
     */
    public void sharedPoolSize(int sharedPoolSize);

    /**
     * The maximum number of buffers to make available as part of the shared
     * buffer pool. The shared buffer pool can be drawn upon by any connected
     * {@link Channel}, where each channel is allowed to use up to
     * (maxOutputBuffers - guaranteedOutputBuffers) number of buffers. Each
     * shared pool buffer will be created to contain maxFragmentSize bytes.
     * The shared pool is not fully allocated at bind time. As needed, shared
     * pool buffers are added and reused until the server is shut down.
     *
     * @return the sharedPoolSize
     */
    public int sharedPoolSize();

    /**
     * If set to true, the shared buffer pool will have its own locking
     * performed. This setting is independent of any initialize locking mode
     * options. Enabling a shared pool lock allows shared pool use to remain
     * thread safe while still disabling channel locking.
     *
     * @param sharedPoolLock the sharedPoolLock to set
     */
    public void sharedPoolLock(boolean sharedPoolLock);

    /**
     * If set to true, the shared buffer pool will have its own locking
     * performed. This setting is independent of any initialize locking mode
     * options. Enabling a shared pool lock allows shared pool use to remain
     * thread safe while still disabling channel locking.
     *
     * @return the sharedPoolLock
     */
    public boolean sharedPoolLock();

    /**
     * The major version of the protocol that the server is capable of
     * exchanging over all connections. This value is negotiated with the client
     * at connection time. The outcome of the negotiation is provided via the
     * majorVersion information on the {@link Channel}. Typically, a major
     * version increase is associated with the introduction of incompatible
     * change. The transport layer is data neutral and does not change nor
     * depend on any information in content being distributed. This information
     * is provided to help client and server applications manage the information
     * they are communicating.<BR>
     * <BR>
     * If the ETA Codec package is being used, this should be set to
     * {@link com.refinitiv.eta.codec.Codec#majorVersion()}.
     *
     * @param majorVersion the majorVersion to set
     */
    public void majorVersion(int majorVersion);

    /**
     * The major version of the protocol that the server is capable of
     * exchanging over all connections. This value is negotiated with the client
     * at connection time. The outcome of the negotiation is provided via the
     * majorVersion information on the {@link Channel}. Typically, a major
     * version increase is associated with the introduction of incompatible
     * change. The transport layer is data neutral and does not change nor
     * depend on any information in content being distributed. This information
     * is provided to help client and server applications manage the information
     * they are communicating.
     *
     * @return the majorVersion
     */
    public int majorVersion();

    /**
     * The minor version number of the {@link Server}.<BR>
     * <BR>
     * If the ETA Codec package is being used, this should be set to
     * {@link com.refinitiv.eta.codec.Codec#minorVersion()}.
     *
     * @param minorVersion the minorVersion to set
     */
    public void minorVersion(int minorVersion);

    /**
     * The minor version of the protocol that the server is capable of
     * exchanging over all connections. This value is negotiated with the client
     * at connection time. The outcome of the negotiation is provided via the
     * minorVersion information on the {@link Channel}. Typically, a minor
     * version increase is associated with a fully backward compatible change or
     * extension. The transport layer is data neutral and does not change nor
     * depend on any information in content being distributed. This information
     * is provided to help client and server applications manage the information
     * they are communicating.
     *
     * @return the minorVersion
     */
    public int minorVersion();

    /**
     * The protocol type of the {@link Server}.<BR>
     * <BR>
     * If the ETA Codec package is being used, this should be set to
     * {@link com.refinitiv.eta.codec.Codec#protocolType()}.
     *
     * @param protocolType the protocol type
     * @see ConnectionTypes
     */
    public void protocolType(int protocolType);

    /**
     * The protocol type that the server intends to exchange over all connection
     * established to it. If the protocolType indicated by a client does not
     * match the protocolType that a server specifies, the connection will be
     * rejected. When a {@link Channel} becomes active for a client or server,
     * this information becomes available via the protocolType on the
     * {@link Channel}. The transport layer is data neutral and does not change
     * nor depend on any information in content being distributed. This
     * information is provided to help client and server applications manage the
     * information they are communicating.
     *
     * @return the protocolType
     */
    public int protocolType();

    /**
     * A user defined object that can be set by the application. This value is
     * not modified by the transport, but will be preserved and stored in the
     * userSpecObject of the {@link Server} returned from bind() if a
     * userSpecObject was not specified in the {@link AcceptOptions}. This
     * information can be useful for coupling this {@link Server} with other
     * user created information, such as a list of connected {@link Channel} structures.
     *
     * @param userSpecObject the userSpecObject to set. User specific object must be non null.
     */
    public void userSpecObject(Object userSpecObject);

    /**
     * A user defined object that can be set by the application. This value is
     * not modified by the transport, but will be preserved and stored in the
     * userSpecObject of the {@link Server} returned from bind() if a
     * userSpecObject was not specified in the {@link AcceptOptions}. This
     * information can be useful for coupling this {@link Server} with other
     * user created information, such as a list of connected {@link Channel} structures.
     *
     * @return the userSpecObject
     */
    public Object userSpecObject();

    /**
     * A substructure containing TCP based connection type specific options.
     * These settings are used for {@link ConnectionTypes#SOCKET} and ConnectionTypes.HTTP.
     *
     * @return the tcpOpts
     *
     * @see TcpOpts
     */
    public TcpOpts tcpOpts();

    /** WebSocket transport specific options (used by {@link ConnectionTypes#WEBSOCKET}).
     *
     * @return the WSocketOpts
     *
     * @see WSocketOpts
     *
     */
    public WSocketOpts wSocketOpts();

    /**
     * The size (in kilobytes) of the system's receive buffer used for this
     * connection, where applicable. Setting of 0 indicates to use default
     * sizes. This can also be set or changed via
     * {@link Channel#ioctl(int, int, Error)} for values less than or equal to 64K.
     * For values larger than 64K, you must use this method so that
     * sysRecvBufSize will be set prior to the bind system call.
     * Must be in the range of 0 - 2,147,483,647.
     *
     * @param sysRecvBufSize the sysRecvBufSize to set
     *
     * @see java.net.Socket#setReceiveBufferSize(int)
     */
    public void sysRecvBufSize(int sysRecvBufSize);

    /**
     * The size (in kilobytes) of the system's receive buffer used for this
     * connection, where applicable. Setting of 0 indicates to use default
     * sizes. This can also be set or changed via
     * {@link Channel#ioctl(int, int, Error)} for values less than or equal to 64K.
     *
     * @return the sysRecvBufSize
     */
    public int sysRecvBufSize();

    /**
     * Sets the group address for a UDP Multicast connection to write to.
     *
     * @param groupAddress the groupAddress to set
     */
    public void groupAddress(String groupAddress);

    /**
     * Returns the groupAddress for the UDP Multicast connection to write to.
     *
     * @return the groupAddress
     */
    public String groupAddress();

    /**
     * If set to true multiple servers will be able to bind on the same port.
     * The implementation of this flag is OS dependent.
     * 
     * @param serverSharedSocket value
     */
    public void serverSharedSocket(boolean  serverSharedSocket);

    /**
     * Returns true if multiple servers can bind on the same port 
     *
     * @return serverSharedSocket value
     */
    public boolean serverSharedSocket();

    /**
     * Encrypted configuration options. This is only active if the connection type is set to {@link ConnectionTypes#ENCRYPTED}.
     * @return the Server Encryption Options
     */
    public ServerEncryptionOptions encryptionOptions();

    /**
     * Clears Bind Options.
     */
    public void clear();
}
