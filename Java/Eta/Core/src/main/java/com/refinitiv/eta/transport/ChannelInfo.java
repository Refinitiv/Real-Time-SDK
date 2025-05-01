/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport;

import java.util.List;

/**
 * ETA Channel Info available through {@link Channel#info(ChannelInfo, Error)} method call.
 * 
 * @see Channel
 */
public interface ChannelInfo
{
    /**
     * The maximum size buffer allowed to be written to the network. If a larger
     * buffer is required, ETA Transport will internally fragment the larger
     * buffer into smaller maxFragmentSize buffers. This is the largest size a
     * user can request while still being 'packable.'
     * 
     * @return the maxFragmentSize
     */
    public int maxFragmentSize();

    /**
     * The maximum size buffer allowed to be written to the network. If a larger
     * buffer is required, ETA Transport will internally fragment the larger
     * buffer into smaller maxFragmentSize buffers. This is the largest size a
     * user can request while still being 'packable.'
     * 
     * @return the maxOutputBuffers
     */
    public int maxOutputBuffers();

    /**
     * The guaranteed number of buffers made available for this {@link Channel}
     * to use while writing data. Each buffer can contain maxFragmentSize bytes.
     * Guaranteed output buffers are allocated at initialization time.
     * 
     * @return the guaranteedOutputBuffers
     */
    public int guaranteedOutputBuffers();

    /**
     * The number of sequential input buffers used by this {@link Channel} for
     * reading data into. This controls the maximum number of bytes that can be
     * handled with a single network read operation on each channel. Each input
     * buffer can contain maxFragmentSize bytes. Input buffers are allocated at
     * initialization time.
     * 
     * @return the numInputBuffers
     */
    public int numInputBuffers();

    /**
     * The negotiated ping timeout value. The typically used rule of thumb is to
     * send a heartbeat every pingTimeout/3 seconds.
     * 
     * @return the pingTimeout
     */
    public int pingTimeout();

    /**
     * If set to true, heartbeat messages are required to flow from the client
     * to the server. If set to false, the client is not required to send
     * heartbeats. LSEG Real-Time Distribution System and other LSEG components
     * typically require this value to be set to true.
     * 
     * @return the clientToServerPings
     */
    public boolean clientToServerPings();

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
     * The size of the send or output buffer associated with the underlying
     * transport. ETA Transport has additional output buffers, controlled by
     * maxOutputBuffers and guaranteedOutputBuffers. For some connection types,
     * this value can be changed through the use of
     * {@link Channel#ioctl(int, int, Error)}.
     * 
     * @return the sysSendBufSize
     */
    public int sysSendBufSize();

    /**
     * The size of the receive or input buffer associated with the underlying
     * transport. ETA Transport has an additional input buffer controlled by
     * numInputBuffers. For some connection types, this value can be changed
     * through the use of {@link Channel#ioctl(int, int, Error)}.
     * 
     * @return the sysRecvBufSize
     */
    public int sysRecvBufSize();

    /**
     * The type of compression being performed for this connection.
     * 
     * @return the compressionType
     */
    public int compressionType();

    /**
     * Used to determine when to compress a message. Messages smaller than this
     * compression threshold will not be compressed while larger messages will.
     * 
     * @return the compressionThreshold
     */
    public int compressionThreshold();

    /**
     * The currently priority level order used when flushing buffers to the
     * connection, where H = High priority, M = Medium priority, and L = Low priority.
     * When passed to {@link Channel#write(TransportBuffer, WriteArgs, Error)},
     * each buffer is also associated with the priority level it should be written at.
     * The default priorityFlushStrategy will write buffers in the order High, Medium, High,
     * Low, High, Medium. This provides a slight advantage to the medium
     * priority level and a greater advantage to high priority data. Data order
     * is preserved within each priority level and if all buffers are written
     * with the same priority, no ordering change will occur. This order can be
     * changed through the use of {@link Channel#ioctl(int, Object, Error)},
     * also described in this section.
     * 
     * @return the priorityFlushStrategy
     */
    public String priorityFlushStrategy();
    
    /**
     * This substructure will report information about the componentInfo received for each connection.
     * 
     * @return a List of {@link ComponentInfo}
     */
    public List<ComponentInfo> componentInfo();

    /**
     * Clears ETA Channel Info.
     */
    public void clear();
    
    /**
     * The IP address of the connecting client.
     * 
     * @return the clientIP
     */
    public String clientIP();

    /**
     * The hostname of the connecting client.
     * 
     * @return the clientHostname
     */
    public String clientHostname();
    
    /**
     * When using a multicast connection type, this will be populated with information about the multicast protocol.
     * 
     * @return the multicastStats
     */
    public MCastStats multicastStats();
    
    /**
     * The security protocol of the connection.
     * 
     * @return the securityProtocol
     */
    public String securityProtocol();  
}
