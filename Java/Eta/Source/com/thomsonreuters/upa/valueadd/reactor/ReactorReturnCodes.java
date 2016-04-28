package com.thomsonreuters.upa.valueadd.reactor;

/**
 * Return codes associated with Reactor.
 */
public class ReactorReturnCodes
{
    private ReactorReturnCodes()
    {
        throw new AssertionError();
    }

    /**
     * Indicates a success code. Used to inform the user of the success.
     */
    public static final int SUCCESS = 0;

    /**
     * A general failure has occurred. The {@link ReactorErrorInfo} contains
     * more information about the specific error.
     */
    public static final int FAILURE = -1;

    /**
     * Transport Success: {@link ReactorChannel#submit(com.thomsonreuters.upa.transport.TransportBuffer,
     * ReactorSubmitOptions, ReactorErrorInfo)} is fragmenting the buffer and needs to be called again
     * with the same buffer. This indicates that Write was unable to send all
     * fragments with the current call and must continue fragmenting.
     */
    public static final int WRITE_CALL_AGAIN = -2;

    /**
     * Transport Failure: There are no buffers available from the buffer pool,
     * returned from {@link ReactorChannel#submit(com.thomsonreuters.upa.codec.Msg,
     * ReactorSubmitOptions, ReactorErrorInfo)}. Use {@link ReactorChannel#ioctl(int, int, ReactorErrorInfo)}
     * to increase  pool size or wait for the Reactor's Worker thread to flush data and
     * return buffers to pool. Use {@link ReactorChannel#bufferUsage(ReactorErrorInfo)}
     * to monitor for free buffers.
     */
    public static final int NO_BUFFERS = -3;

    /**
     * Indicates that a parameter was out of range.
     */
    public static final int PARAMETER_OUT_OF_RANGE = -4;

    /**
     * Indicates that a parameter was invalid.
     */
    public static final int PARAMETER_INVALID = -5;

    /**
     * The interface is being improperly used.
     */
    public static final int INVALID_USAGE = -6;

    /**
     * An error was encountered during a channel operation.
     */
    public static final int CHANNEL_ERROR = -7;
    
    /**
     * The interface is attempting to write a message to the Reactor
     * with an invalid encoding. 
     */
    public static final int INVALID_ENCODING = -8;

    /**
     * The interface is attempting to write a message to the TunnelStream,
     * but the persistence file is full. 
     */
    public static final int PERSISTENCE_FULL = -9;

    /**
     * Returns a String representation of the specified ReactorReturnCodes type.
     * 
     * @param type
     * 
     * @return String representation of the specified ReactorReturnCodes type
     */
    public static String toString(int type)
    {
        switch (type)
        {
            case 0:
                return "ReactorReturnCodes.SUCCESS";
            case -1:
                return "ReactorReturnCodes.FAILURE";
            case -2:
                return "ReactorReturnCodes.WRITE_CALL_AGAIN";
            case -3:
                return "ReactorReturnCodes.NO_BUFFERS";
            case -4:
                return "ReactorReturnCodes.PARAMETER_OUT_OF_RANGE";
            case -5:
                return "ReactorReturnCodes.PARAMETER_INVALID";
            case -6:
                return "ReactorReturnCodes.INVALID_USAGE";
            case -7:
                return "ReactorReturnCodes.CHANNEL_ERROR";
            case -8:
                return "ReactorReturnCodes.INVALID_ENCODING";
            case -9:
                return "ReactorReturnCodes.PERSISTENCE_FULL";
            default:
                return "ReactorReturnCodes " + type + " - undefined.";
        }
    }
}
