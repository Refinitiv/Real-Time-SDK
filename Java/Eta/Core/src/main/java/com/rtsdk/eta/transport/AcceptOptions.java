package com.rtsdk.eta.transport;

/**
 * UPA Accept Options used in the {@link Server#accept(AcceptOptions, Error)} call.
 * 
 * @see Server
 */
public interface AcceptOptions
{
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
     * If true, indicates that the server wants to reject the incoming
     * connection. This may be due to some kind of connection limit being
     * reached. For non-blocking connections to successfully complete rejection,
     * the initialization process must still be completed.
     * 
     * @param nakMount the nakMount to set
     */
    public void nakMount(boolean nakMount);

    /**
     * If true, indicates that the server wants to reject the incoming
     * connection. This may be due to some kind of connection limit being
     * reached. For non-blocking connections to successfully complete rejection,
     * the initialization process must still be completed.
     * 
     * @return the nakMount
     */
    public boolean nakMount();

    /**
     * A user specified object. This value is not modified by the transport, but
     * will be preserved and stored in the userSpecObject of the {@link Channel}
     * returned from {@link Server#accept(AcceptOptions, Error)}. If this value
     * is not set, the {@link Channel#userSpecObject()} will be set to the
     * userSpecObject associated with the Server that is accepting this connection.
     * 
     * @param userSpecObject the userSpecObject to set
     */
    public void userSpecObject(Object userSpecObject);

    /**
     * A user specified object. This value is not modified by the transport, but
     * will be preserved and stored in the userSpecObject of the {@link Channel}
     * returned from {@link Server#accept(AcceptOptions, Error)}. If this value
     * is not set, the {@link Channel#userSpecObject()} will be set to the
     * userSpecObject associated with the Server that is accepting this connection.
     * 
     * @return the userSpecObject
     */
    public Object userSpecObject();
    
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
     * where applicable.  Setting of 0 indicates to use default sizes.
     * This can also be set or changed via {@link Channel#ioctl(int, int, Error)}.
     * 
     * @return the sysSendBufSize
     */
    public int sysSendBufSize();

    /**
     * Clears UPA Accept Options.
     */
    public void clear();

}
