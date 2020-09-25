package com.refinitiv.eta.transport;

/**
 * UPA IOCtl codes for {@link Channel#ioctl(int, Object, Error)},
 * {@link Channel#ioctl(int, int, Error)}, {@link Server#ioctl(int, Object, Error)}
 * and {@link Server#ioctl(int, int, Error)}.
 * 
 * @see Channel
 * @see Server
 */
public class IoctlCodes
{
    
    /**
     * Instantiates a new ioctl codes.
     */
    // IoctlCodes class cannot be instantiated
    private IoctlCodes()
    {
        throw new AssertionError();
    }

    /**
     * Allows a {@link Channel} to change its maxOutputBuffers setting.
     * Value is an int. Default is 50.
     * Refer to {@link BindOptions#maxOutputBuffers(int)} for more information.
     */
    public static final int MAX_NUM_BUFFERS = 1;

    /**
     * Allows a {@link Channel} to change its guaranteedOutputBuffers setting.
     * Value is an int. Default is 50.
     * Refer to {@link BindOptions#guaranteedOutputBuffers(int)} or
     * {@link ConnectOptions#guaranteedOutputBuffers(int)} for more information.
     */
    public static final int NUM_GUARANTEED_BUFFERS = 2;

    /**
     * Allows a {@link Channel} to change the internal UPA output queue depth
     * water mark, which has a default value of 6,144 bytes. When the UPA output
     * queue exceeds this number of bytes, the write method internally
     * attempts to flush content to the network.
     * Value is an int. Default is 6144.
     */
    public static final int HIGH_WATER_MARK = 3;

    /**
     * Allows a {@link Channel} to change the TCP receive buffer size
     * associated with the connection. Value is an int. Default is 64K.
     * <br>
     * Please note that if the value is larger than 64K, the value needs to
     * be specified before the socket is connected to the remote peer. 
     * <ul>
     * <li>
     * For servers and SYSTEM_READ_BUFFERS larger than 64K, use
     * {@link BindOptions#sysRecvBufSize(int)} to set the receive buffer size,
     * prior to calling {@link Transport#bind(BindOptions, Error)}.
     * <li>
     * For clients and SYSTEM_READ_BUFFERS larger than 64K, use
     * {@link ConnectOptions#sysRecvBufSize(int)} to set the receive buffer size,
     * prior to calling {@link Transport#connect(ConnectOptions, Error)}.
     * </ul>
     */
    public static final int SYSTEM_READ_BUFFERS = 4;

    /**
     * Allows a {@link Channel} to change the TCP send buffer size associated
     * with the connection. Value is an int. Default is 64K.
     */
    public static final int SYSTEM_WRITE_BUFFERS = 5;

    /* Reserved */
    static final int RESERVED = 6;

    /**
     * <p>
     * Allows a {@link Channel} to change its priorityFlushStrategy. Value
     * is a String, where each entry in the String is either:
     * <ul>
     * <li>H for high priority</li>
     * <li>M for medium priority</li>
     * <li>L for low priority</li>
     * </ul>
     * <p>
     * The String should not exceed 32 entries. At least one H and one M must be
     * present, however no L is required. If no low priority flushing is
     * specified, the low priority queue will only be flushed when no other data
     * is available for output. Default is "HMHLHM".
     */
    public static final int PRIORITY_FLUSH_ORDER = 7;

    /**
     * Allows a {@link Channel} to change the size (in bytes) at which buffer
     * compression will occur, must be greater than 30 bytes. Value is an int.
     * Default is 30 bytes.
     */
    public static final int COMPRESSION_THRESHOLD = 9;

    /**
     * Allows a {@link Server} to change its sharedPoolSize setting.
     * Value is an int.
     */
    public static final int SERVER_NUM_POOL_BUFFERS = 8;

    /**
     * Allows a {@link Server} to reset the peakBufferUsage statistic.
     * Value is not required.
     */
    public static final int SERVER_PEAK_BUF_RESET = 10;
    
    /*
     * This is for internal client use only. Not exposed on public interface and javadoc.
     */
    static final int COMPONENT_INFO = 13;
    
}
