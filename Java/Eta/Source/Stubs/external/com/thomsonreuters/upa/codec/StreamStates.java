package com.thomsonreuters.upa.codec;

/**
 * State Stream States provide information on the health of the stream.
 * 
 * @see State
 */
public class StreamStates
{
    // StreamStates class cannot be instantiated
    private StreamStates()
    {
        throw new AssertionError();
    }

    /**
     * Unspecified. Typically used as a initialization value and is not intended
     * to be encoded or decoded.
     */
    public static final int UNSPECIFIED = 0;

    /**
     * Stream is open. Typically implies that information will be streaming, as
     * information changes updated information will be sent on the stream, after
     * final {@link RefreshMsg} or {@link StatusMsg}.
     */
    public static final int OPEN = 1;

    /**
     * Request was non-streaming. After final {@link RefreshMsg} or
     * {@link StatusMsg} is received, the stream will be closed and no updated
     * information will be delivered without subsequent re-request.
     */
    public static final int NON_STREAMING = 2;

    /**
     * Closed, the applications may attempt to re-open the stream later. Can
     * occur via either a {@link RefreshMsg} or a {@link StatusMsg}.
     */
    public static final int CLOSED_RECOVER = 3;

    /**
     * Closed. Indicates that the data is not available on this
     * service/connection and is not likely to become available.
     */
    public static final int CLOSED = 4;

    /**
     * Closed and Redirected. Indicates that the current stream has been closed
     * and has new identifying information, the user can issue a new request for
     * the data using the new message key information contained in the redirect
     * message.
     */
    public static final int REDIRECTED = 5;

    /**
     * Provide string representation for a stream state value.
     * 
     * @param streamState {@link StreamStates} enumeration to convert to string
     * 
     * @return string representation for a stream state value
     */
    public static String toString(int streamState)
    {
        switch (streamState)
        {
            case UNSPECIFIED:
                return "UNSPECIFIED";
            case OPEN:
                return "OPEN";
            case NON_STREAMING:
                return "NON_STREAMING";
            case CLOSED_RECOVER:
                return "CLOSED_RECOVER";
            case CLOSED:
                return "CLOSED";
            case REDIRECTED:
                return "REDIRECTED";
            default:
                return Integer.toString(streamState);
        }
    }

    /**
     * Provide string representation of meaning associated with stream state.
     * 
     * @param streamState {@link StreamStates} enumeration to get info for
     * 
     * @return string representation of meaning associated with stream state
     */
    public static String info(int streamState)
    {
        switch (streamState)
        {
            case StreamStates.UNSPECIFIED:
                return "Unspecified";
            case StreamStates.OPEN:
                return "Open";
            case StreamStates.NON_STREAMING:
                return "Non-streaming";
            case StreamStates.CLOSED_RECOVER:
                return "Closed, Recoverable";
            case StreamStates.CLOSED:
                return "Closed";
            case StreamStates.REDIRECTED:
                return "Redirected";
            default:
                return "Unknown Stream State";
        }
    }
}
