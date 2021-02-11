package com.refinitiv.eta.transport;

/**
 * ETA Connect Options used in the
 * {@link Transport#connect(ConnectOptions, Error)} call.
 *
 * @see Transport
 */
public interface ConnectOptions
{

    /**
     * A character representation of component version information.
     *
     * @param componentVersion the component version
     */
    public void componentVersion(String componentVersion);

    /**
     * A character representation of component version information.
     *
     * @return the componentVersion
     */
    public String componentVersion();



    /**
     * Type of connection to establish. Must be in the range of
     * {@link ConnectionTypes#SOCKET} - {@link ConnectionTypes#RELIABLE_MCAST}.
     *
     * @param connectionType the connectionType to set
     *
     * @see ConnectionTypes
     */
    public void connectionType(int connectionType);

    /**
     * Type of connection to establish.
     *
     * @return the connectionType
     *
     * @see ConnectionTypes
     */
    public int connectionType();

    /**
     * Connection parameters when sending and receiving on same network.
     * This is typically used with {@link ConnectionTypes#SOCKET},
     * {@link ConnectionTypes#HTTP}, {@link ConnectionTypes#ENCRYPTED},
     * and fully connected/mesh multicast networks.
     *
     * @return the UnifiedNetworkInfo
     *
     */
    public UnifiedNetworkInfo unifiedNetworkInfo();

    /**
     * Connection parameters when sending and receiving on different networks.
     * This is typically used with multicast networks that have different groups
     * of senders and receivers (e.g., NIProvider can send network and ADH on receive network).
     *
     * @return the SegmentedNetworkInfo
     *
     */
    public SegmentedNetworkInfo segmentedNetworkInfo();

    /**
     * Tunneling connection parameters.
     *
     * @return the TunnelingInfo
     *
     */
    public TunnelingInfo tunnelingInfo();

    /**
     * Proxy Credentials.
     *
     * @return the CredentialsInfo
     *
     */
    public CredentialsInfo credentialsInfo();

    /**
     * Encryption connection options
     *
     * @return the EncryptionOptions
     *
     */
    public EncryptionOptions encryptionOptions();

    /**
     * The type of compression the client would like performed for this
     * connection. Compression is negotiated between the client and server and
     * may not be performed if only the client has enabled. Must be in the
     * range of {@link CompressionTypes#NONE} - {@link CompressionTypes#LZ4}.
     *
     * @param compressionType the compressionType to set
     *
     * @see CompressionTypes
     */
    public void compressionType(int compressionType);

    /**
     * The type of compression the client would like performed for this
     * connection. Compression is negotiated between the client and server and
     * may not be performed if only the client has enabled.
     *
     * @return the compressionType
     *
     * @see CompressionTypes
     */
    public int compressionType();

    /**
     * If true, the connection will use lock on reading.
     *
     * @param locking the readLocking to set
     */
    public void channelReadLocking(boolean locking);

    /**
     * If true, the connection uses lock on read.
     *
     * @return the locking
     */
    public boolean channelReadLocking();

    /**
     * If true, the connection will use lock on writing.
     *
     * @param locking the writeLocking to set
     */
    public void channelWriteLocking(boolean locking);

    /**
     * If true, the connection uses lock on write.
     *
     * @return the locking
     */
    public boolean channelWriteLocking();


    /**
     * If set to true, blocking I/O will be used for this {@link Channel}. When
     * I/O is used in a blocking manner on a {@link Channel}, any reading or
     * writing will complete before control is returned to the application. In
     * addition, the connect method will complete any initialization on the
     * {@link Channel} prior to returning it.
     * <p>
     * Blocking I/O prevents the application from performing any operations
     * until the I/O operation is completed. Blocking I/O is typically not
     * recommended. An application can leverage an I/O notification mechanism to
     * allow efficient reading and writing, while using other cycles to perform
     * other necessary work in the application. An I/O notification mechanism
     * enables the application to read when data is available, and write when
     * output space is available.
     *
     *
     * @param blocking the blocking to set
     */
    public void blocking(boolean blocking);

    /**
     * If true, the connection will block.
     *
     * @return the blocking
     */
    public boolean blocking();

    /**
     * The clients desired ping timeout value. This may change through the
     * negotiation process between the client and the server. After the
     * connection becomes active, the actual negotiated value becomes available
     * through the pingTimeout value on the {@link Channel}. When determining
     * the desired ping timeout, the typically used rule of thumb is to send a
     * heartbeat every pingTimeout/3 seconds. Must be in the range of 1 - 255.
     * If the value is 0, it will be adjusted to 1, and if the value is greater
     * than 255, it will be set to 255.
     *
     * @param pingTimeout the pingTimeout to set
     */
    public void pingTimeout(int pingTimeout);

    /**
     * The clients desired ping timeout value. This may change through the
     * negotiation process between the client and the server. After the
     * connection becomes active, the actual negotiated value becomes available
     * through the pingTimeout value on the {@link Channel}. When determining
     * the desired ping timeout, the typically used rule of thumb is to send a
     * heartbeat every pingTimeout/3 seconds.
     *
     * @return the pingTimeout
     */
    public int pingTimeout();

    /**
     * A guaranteed number of buffers made available for this {@link Channel} to
     * use while writing data. Guaranteed output buffers are allocated at
     * initialization time. Must be in the range of 0 - 2,147,483,647. If the
     * argument value is less then 5, the guaranteed number of buffers will be set to 5.
     *
     * @param guaranteedOutputBuffers the guaranteedOutputBuffers to set
     */
    public void guaranteedOutputBuffers(int guaranteedOutputBuffers);

    /**
     * A guaranteed number of buffers made available for this {@link Channel} to
     * use while writing data. Guaranteed output buffers are allocated at
     * initialization time.
     *
     * @return the guaranteedOutputBuffers
     */
    public int guaranteedOutputBuffers();

    /**
     * The number of sequential input buffers to allocate for reading data into.
     * This controls the maximum number of bytes that can be handled with a
     * single network read operation. Input buffers are allocated at
     * initialization time. Must be in the range of 0 - 2,147,483,647.
     *
     * @param numInputBuffers the numInputBuffers to set
     */
    public void numInputBuffers(int numInputBuffers);

    /**
     * The number of sequential input buffers to allocate for reading data into.
     * This controls the maximum number of bytes that can be handled with a
     * single network read operation. Input buffers are allocated at
     * initialization time.
     *
     * @return the numInputBuffers. The number must be greater than 0.
     */
    public int numInputBuffers();

    /**
     * The major version number of the {@link Channel}.<BR>
     * <BR>
     * If the ETA Codec package is being used, this should be set to
     * {@link com.refinitiv.eta.codec.Codec#majorVersion()}.
     *
     * @param majorVersion the majorVersion to set
     */
    public void majorVersion(int majorVersion);

    /**
     * The major version number of the {@link Channel}.
     *
     * @return the majorVersion
     */
    public int majorVersion();

    /**
     * The minor version number of the {@link Channel}.<BR>
     * <BR>
     * If the ETA Codec package is being used, this should be set to
     * {@link com.refinitiv.eta.codec.Codec#minorVersion()}.
     *
     * @param minorVersion the minorVersion to set
     */
    public void minorVersion(int minorVersion);

    /**
     * The major version of the protocol that the client intends to exchange
     * over the connection. This value is negotiated with the server at
     * connection time. The outcome of the negotiation is provided via the
     * majorVersion information on the {@link Channel}. Typically, a major
     * version increase is associated with the introduction of incompatible
     * change. The transport layer is data neutral and does not change nor
     * depend on any information in content being distributed. This information
     * is provided to help client and server applications manage the information
     * they are communicating.
     *
     * @return the minorVersion
     */
    public int minorVersion();

    /**
     * The protocol type that the client intends to exchange over the
     * connection. If the protocolType indicated by a server does not match the
     * protocolType that a client specifies, the connection will be rejected.
     * When a {@link Channel} becomes active for a client or server, this
     * information becomes available via the protocolType on the {@link Channel}.
     * The transport layer is data neutral and does not change nor depend on any
     * information in content being distributed. This information is provided to
     * help client and server applications manage the information they are communicating.<BR>
     * <BR>
     * If the ETA Codec package is being used, this should be set to
     * {@link com.refinitiv.eta.codec.Codec#protocolType()}.
     *
     * @param protocolType the protocolType to set
     */
    public void protocolType(int protocolType);

    /**
     * The protocol type that the client intends to exchange over the
     * connection. If the protocolType indicated by a server does not match the
     * protocolType that a client specifies, the connection will be rejected.
     * When a {@link Channel} becomes active for a client or server, this
     * information becomes available via the protocolType on the {@link Channel}.
     * The transport layer is data neutral and does not change nor depend on any
     * information in content being distributed. This information is provided to
     * help client and server applications manage the information they are communicating.
     *
     * @return the protocolType
     */
    public int protocolType();

    /**
     * A user specified object. This value is not modified by the transport, but
     * will be preserved and stored in the userSpecPtr of the {@link Channel}
     * returned from connect. This information can be useful for coupling this
     * {@link Channel} with other user created information, such as a watch list
     * associated with this connection.
     *
     * @param userSpecObject the userSpecObject to set. The object must be non null.
     */
    public void userSpecObject(Object userSpecObject);

    /**
     * A user specified object. This value is not modified by the transport, but
     * will be preserved and stored in the userSpecPtr of the {@link Channel}
     * returned from connect. This information can be useful for coupling this
     * {@link Channel} with other user created information, such as a watch list
     * associated with this connection.
     *
     * @return the userSpecObject
     */
    public Object userSpecObject();

    /**
     * A substructure containing TCP based connection type specific options.
     * These settings are used for {@link ConnectionTypes#SOCKET}, ConnectionTypes.HTTP,
     * and ConnectionTypes.ENCRYPTED.
     *
     * @return the tcpOpts
     *
     * @see TcpOpts
     */
    public TcpOpts tcpOpts();

    /** Multicast transport specific options (used by {@link ConnectionTypes#RELIABLE_MCAST}).
     *
     * @return the multicastOpts
     *
     * @see MCastOpts
     * */
    public MCastOpts multicastOpts();

    /**
     * Shared memory transport specific options (used by {@link ConnectionTypes#UNIDIR_SHMEM}).
     *
     * @return the shmem opts
     */
    public ShmemOpts shmemOpts();

    /**
     * The size (in kilobytes) of the system's send buffer used for this connection,
     * where applicable.  Setting of 0 indicates to use default sizes. This can also
     * be set or changed via {@link Channel#ioctl(int, int, Error)}.
     * Must be in the range of 0 - 2,147,483,647.
     *
     * @param sysSendBufSize the sysSendBufSize to set
     */
    public void sysSendBufSize(int sysSendBufSize);

    /**
     * The size (in kilobytes) of the system's send buffer used for this connection,
     * where applicable.  Setting of 0 indicates to use default sizes. This can also
     * be set or changed via {@link Channel#ioctl(int, int, Error)}.
     *
     * @return the sysSendBufSize
     */
    public int sysSendBufSize();

    /**
     * The size (in kilobytes) of the system's receive buffer used for this
     * connection, where applicable. Setting of 0 indicates to use default
     * sizes. This can also be set or changed via
     * {@link Channel#ioctl(int, int, Error)} for values less than or equal
     * to 64K. For values larger than 64K, you must use this method so that
     * sysRecvBufSize will be set prior to the connect system call.
     * Must be in the range of 0 - 2,147,483,647.
     *
     * @param sysRecvBufSize the sysRecvBufSize to set
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

    /** Sequenced Multicast transport specific options (used by {@link ConnectionTypes#SEQUENCED_MCAST}).
     *
     * @return the SeqMCastOpts
     *
     * @see SeqMCastOpts
     * */
    public SeqMCastOpts seqMCastOpts();

    /** WebSocket transport specific options (used by {@link ConnectionTypes#WEBSOCKET}).
     *
     * @return the WSocketOpts
     *
     * @see WSocketOpts
     *
     */
    public WSocketOpts wSocketOpts();

    /**
     * Clears ETA Connect Options.
     */
    public void clear();

    /**
     * This method will perform a deep copy into the passed in parameter's
     *          members from the ConnectOptions calling this method.
     *
     * @param destOpts the value getting populated with the values of the calling ConnectOptions
     *
     * @return {@link TransportReturnCodes#SUCCESS} on success,
     *         {@link TransportReturnCodes#FAILURE} if the destOpts is null.
     */
    public int copy(ConnectOptions destOpts);
}
