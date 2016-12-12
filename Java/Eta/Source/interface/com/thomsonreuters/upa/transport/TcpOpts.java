package com.thomsonreuters.upa.transport;

/**
 * Options used for configuring TCP specific transport options
 * (ConnectionTypes.SOCKET, ConnectionTypes.ENCRYPTED, ConnectionTypes.HTTP).
 * 
 * @see ConnectOptions
 */
public interface TcpOpts
{
    /**
     * Only used with connectionType of SOCKET. If true, disables Nagle's Algorithm.
     * 
     * @param tcpNoDelay the tcpNoDelay to set
     */
    public void tcpNoDelay(boolean tcpNoDelay);

    /**
     * Only used with connectionType of SOCKET. If true, disables Nagle's Algorithm.
     * 
     * @return the tcpNoDelay
     */
    public boolean tcpNoDelay();
}