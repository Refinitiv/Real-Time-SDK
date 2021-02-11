package com.refinitiv.eta.transport;

/**
 * ETA Connection types are used in several areas of the transport. When
 * creating a connection an application can specify the connection type to use.
 */
public class ConnectionTypes
{

    /**
     * Instantiates a new connection types.
     */
    // ConnectionTypes class cannot be instantiated
    private ConnectionTypes()
    {
        throw new AssertionError();
    }

    /**
     * Indicates that the {@link Channel} is using a standard TCP-based socket
     * connection. This type can be used to connect between any ETA Transport
     * based applications.
     */
    public static final int SOCKET = 0;


    /**
     * Indicates that the {@link Channel} is using an SSL/TLS encrypted
     * HTTP TCP-based socket connection. This type can be used by
     * a ETA Transport consumer based application.
     */
    public static final int ENCRYPTED = 1;


    /**
     * Indicates that the {@link Channel} is using an HTTP TCP-based socket
     * connection. This type can be used by a ETA Transport
     * consumer based application.
     */
    public static final int HTTP = 2;

    /**
     * Indicates that the {@link Channel} is using a unidirectional shared
     * memory connection. This type can be used to send from a shared memory
     * server to one or more shared memory clients.
     */
    public static final int UNIDIR_SHMEM = 3;

    /**
     * Indicates that the {@link Channel} is using a reliable multicast based
     * connection. This type can be used to connect on a unified/mesh network
     * where send and receive networks are the same or a segmented network where
     * send and receive networks are different.
     */
    public static final int RELIABLE_MCAST = 4;

    public static final int SEQUENCED_MCAST = 6;

    /**
     * Indicates that the {@link Channel} is using a standard TCP-based WebSocket
     * connection. This type can be used to connect between any ETA Transport
     * based applications.
     */
    public static final int WEBSOCKET = 7;

    /* max defined connectionType */
    static final int MAX_DEFINED = WEBSOCKET;

    /**
     * Provide string representation for a connection type value.
     *
     * @param type the type
     * @return string representation for a connection type value
     */
    public static String toString(int type)
    {
        switch (type)
        {
            case SOCKET:
                return "socket";
            case HTTP:
                return "http";
            case ENCRYPTED:
                return "encrypted";
            case UNIDIR_SHMEM:
                return "shmem";
            case RELIABLE_MCAST:
                return "reliableMCast";
            case WEBSOCKET:
                return "webSocket";
            default:
                return Integer.toString(type);
        }
    }
}
