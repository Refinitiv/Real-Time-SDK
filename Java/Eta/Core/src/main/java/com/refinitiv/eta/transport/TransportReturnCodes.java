package com.refinitiv.eta.transport;

/**
 * Return codes associated with transport package.
 */
public class TransportReturnCodes
{
    // TransportReturnCodes class cannot be instantiated
    private TransportReturnCodes()
    {
        throw new AssertionError();
    }

    /* Multicast Transport Specific Return Codes */
    /* -70 through -61 */
    /** Transport Warning: Network congestion detected. Gaps are likely. */
    public static final int CONGESTION_DETECTED = -63;
    /** Transport Warning: Application is consuming more slowly than data
     * is being provided. Gaps are likely. */
    public static final int SLOW_READER = -62;
    /** Transport Warning: An unrecoverable packet gap was detected and some
     * content may have been lost. */
    public static final int PACKET_GAP_DETECTED = -61;

    /* -35 through -60 reserved */

    /* -20 through -16 reserved */

    /* Transport Return Codes */
    /* Because positive values indicate bytes left to read or write */
    /* Some negative transport layer return codes still indicate success */
    /**
     * Transport Success: Indicates that a {@link Channel#read(ReadArgs, Error)}
     * call on the {@link Channel} is already in progress. This can be due to
     * another thread performing the same operation.
     */
    public static final int READ_IN_PROGRESS = -15;

    /**
     * Transport Success: Indicates that the connections channel has changed.
     * This can occur as a result of internal connection keep-alive mechanisms.
     * The previous channel is stored in the {@link Channel#oldSelectableChannel()} so
     * it can be removed from the I/O notification mechanism. The
     * {@link Channel#selectableChannel()} contains the new channel, which should be
     * registered with the I/O notification mechanism.
     */
    public static final int READ_FD_CHANGE = -14;

    /**
     * Transport Success: Indicates that a heartbeat message was received in
     * Channel.read call. The ping timer should be updated.
     */
    public static final int READ_PING = -13;

    /**
     * Transport Success: Reading was blocked by the OS. Typically indicates
     * that there are no bytes available to read, returned from Channel.read.
     */
    public static final int READ_WOULD_BLOCK = -11;

    /**
     * Transport Success:
     * {@link Channel#write(TransportBuffer, WriteArgs, Error)} is fragmenting
     * the buffer and needs to be called again with the same buffer. This
     * indicates that Write was unable to send all fragments with the current
     * call and must continue fragmenting.
     */
    public static final int WRITE_CALL_AGAIN = -10;

    /**
     * Transport Success:
     * {@link Channel#write(TransportBuffer, WriteArgs, Error)} internally
     * attempted to flush data to the connection but was blocked. This is not a
     * failure and the user should not release their buffer.
     */
    public static final int WRITE_FLUSH_FAILED = -9;

    /* -8 through -5 reserved */
    /**
     * Transport Failure: There are no buffers available from the buffer pool,
     * returned from {@link Channel#getBuffer(int, boolean, Error)}. Use
     * {@link Channel#ioctl(int, int, Error)} to increase pool size or use
     * {@link Channel#flush(Error)} to flush data and return buffers to pool.
     */
    public static final int NO_BUFFERS = -4;

    /**
     * Transport Failure: Not initialized failure code, returned from transport
     * methods when {@link Transport#initialize(InitArgs, Error)} did not succeed.
     */
    public static final int INIT_NOT_INITIALIZED = -3;

    /**
     * Transport Failure: Channel initialization failed/connection refused,
     * returned from {@link Channel#init(InProgInfo, Error)}
     */
    public static final int CHAN_INIT_REFUSED = -2;

    /* General Failure and Success Codes */
    /**
     * General Failure: ETA general failure return code.
     */
    public static final int FAILURE = -1;
    /**
     * General Success: ETA general success return code.
     */
    public static final int SUCCESS = 0;

    /* Transport Return Codes */
    /**
     * Transport Success: Channel initialization is In progress, returned from
     * {@link Channel#init(InProgInfo, Error)}.
     */
    public static final int CHAN_INIT_IN_PROGRESS = 2;

    /* 3 through 9 reserved */

    /**
     * Provides string representation for a transport return code.
     * 
     * @param retCode {@link TransportReturnCodes} enumeration to convert to string
     * 
     * @return string representation for a transport return code
     */
    public static String toString(int retCode)
    {
        switch (retCode)
        {
            case CONGESTION_DETECTED:
                return "CONGESTION_DETECTED";
            case SLOW_READER:
                return "SLOW_READER";
            case PACKET_GAP_DETECTED:
                return "PACKET_GAP_DETECTED";
            case READ_IN_PROGRESS:
                return "READ_IN_PROGRESS";
            case READ_FD_CHANGE:
                return "READ_FD_CHANGE";
            case READ_PING:
                return "READ_PING";
            case READ_WOULD_BLOCK:
                return "READ_WOULD_BLOCK";
            case WRITE_CALL_AGAIN:
                return "WRITE_CALL_AGAIN";
            case WRITE_FLUSH_FAILED:
                return "WRITE_FLUSH_FAILED";
            case NO_BUFFERS:
                return "NO_BUFFERS";
            case INIT_NOT_INITIALIZED:
                return "INIT_NOT_INITIALIZED";
            case CHAN_INIT_REFUSED:
                return "CHAN_INIT_REFUSED";
            case FAILURE:
                return "FAILURE";
            case SUCCESS:
                return "SUCCESS";
            case CHAN_INIT_IN_PROGRESS:
                return "CHAN_INIT_IN_PROGRESS";
            default:
                return Integer.toString(retCode);
        }
    }
}
