package com.refinitiv.eta.transport;

import java.nio.channels.SelectableChannel;

import com.refinitiv.eta.transport.InProgInfo;
import com.refinitiv.eta.transport.TransportBuffer;

/**
 * The ETA Channel is used to represent a connection that can send or receive
 * information across a network. This class is used to represent a connection,
 * regardless of if that was an outbound connection or a connection that was
 * accepted by a listening socket. Any memory associated with a {@link Channel}
 * object is internally managed by the ETA Transport Package, and the
 * application does not need to create nor destroy any memory associated with a
 * {@link Channel}. The {@link Channel} is typically used to perform any action
 * on the connection that it represents (e.g. reading, writing, disconnecting, * etc).
 */
public interface Channel
{
    /**
     * Gets information about the channel.<br>
     * 
     * Typical use:<br>
     * If information about the {@link Channel} is needed, such as
     * maxFragmentSize or maxOutputBuffers, this method can be called to retrieve this information.
     * If the channel is closed by the far end, this method returns error and the channel state is set to CLOSED.
     * 
     * @param info Channel Info structure to be populated
     * @param error Error, to be populated in event of an error
     * 
     * @return {@link TransportReturnCodes}
     * 
     * @see ChannelInfo
     */
    public int info(ChannelInfo info, Error error);

    /**
     * Allows changing some I/O values programmatically.
     * 
     * Typical use:<br>
     * If an I/O value needs to be changed, this method is used.
     * <p>
     * Valid codes are:
     * <ul>
     * <li>{@link IoctlCodes#PRIORITY_FLUSH_ORDER}</li>
     * </ul>
     * 
     * @param code {@link IoctlCodes} code of I/O Option to change
     * @param value Value to change Option to
     * @param error {@link Error} to be populated in event of an error
     * 
     * @return {@link TransportReturnCodes}
     * 
     * @see IoctlCodes
     */
    public int ioctl(int code, Object value, Error error);
    
    /**
     * Allows changing some I/O values programmatically.
     * 
     * Typical use:<br>
     * If an I/O value needs to be changed, this method is used.
     * <p>
     * Valid codes are:
     * <ul>
     * <li>{@link IoctlCodes#MAX_NUM_BUFFERS} - the return code will be the new
     * value of maxOutputBuffers or {@link TransportReturnCodes#FAILURE}. If the
     * specified value is less than guaranteedOutputBuffers, the
     * guaranteedOutputBuffers value will be used.</li>
     * <li>{@link IoctlCodes#NUM_GUARANTEED_BUFFERS} - the return code will be
     * the new value of guarenteedOutputBuffers or
     * {@link TransportReturnCodes#FAILURE}. If the specified value is larger
     * than maxOutputBuffers, maxOutputBuffers will be set to the value. If the
     * value is less than the number of free buffers in the guaranteed pool (due
     * to buffers being in use), the guaranteed pool will be reduced by the
     * number of free buffers.</li>
     * <li>{@link IoctlCodes#HIGH_WATER_MARK}</li>
     * <li>{@link IoctlCodes#SYSTEM_READ_BUFFERS} - for values larger than 64K,
     * use {@link ConnectOptions#sysRecvBufSize(int)} to set the receive buffer
     * size, prior to calling {@link Transport#connect(ConnectOptions, Error)}.</li>
     * <li>{@link IoctlCodes#SYSTEM_WRITE_BUFFERS}</li>
     * <li>{@link IoctlCodes#COMPRESSION_THRESHOLD}</li>
     * <li>If the channel is closed by the far end, this method returns error and
     * the channel state is set to CLOSED.</li>
     * </ul>
     * 
     * @param code {@link IoctlCodes} code of I/O Option to change.
     * @param value Value to change Option to
     * @param error {@link Error} to be populated in event of an error
     * 
     * @return {@link TransportReturnCodes} or a positive number specific to the specified code.
     * 
     * @see IoctlCodes ChannelInfo
     */
    public int ioctl(int code, int value, Error error);

    /**
     * Returns the total number of used buffers for this channel.<BR>
     * 
     * Typical use: <br>
     * This method can be called to find out the number of used buffers for the calling channel.
     * This, in combination with the maxOutputBuffers obtained from the getInfo call,
     * can be used to monitor and potentially throttle buffer usage.
     * 
     * @param error Error, to be populated in event of an error
     * 
     * @return If less than 0, this is a {@link TransportReturnCodes},
     *         otherwise it is the total number of buffers in use by this channel
     */
    public int bufferUsage(Error error);

    /**
     * Continues channel initialization for non-blocking channels.<br>
     * 
     * Typical use:<br>
     * 1. Connect using Transport.connect<br>
     * 2. While Channel state is INITIALIZING and the channel detects data to
     * read, call this method.<br>
     * Note: This method is not necessary for blocking channels, which will
     * return in the ACTIVE state after the Transport.connect call.
     * 
     * @param inProg InProg Info for compatibility mode reconnection
     * @param error Error, to be populated in event of an error
     * 
     * @return {@link TransportReturnCodes}
     * 
     * @see InProgInfo
     * @see ChannelState
     */
    public int init(InProgInfo inProg, Error error);

    /**
     * Closes a Channel.<br>
     * 
     * Typical use:<br>
     * When done using a Channel, this call closes it.
     * This method should also be used when any method on the channel
     * returned error and the channel state is CLOSED.
     * This method sets the channel's state to INACTIVE.
     * Before calling this method, all TransportBuffers that has been obtained
     * through getBuffer() method and were not written (through write()
     * method) should be released to pool by calling releaseBuffer() method.
     * This method returns this channel to the channel pool.
     * 
     * @param error Error, to be populated in event of an error
     * 
     * @return {@link TransportReturnCodes}
     */
    public int close(Error error);

    /**
     * Reads on a given channel.<br>
     * 
     * Typical use:<br>
     * This method is called and returns a buffer with any data read from the
     * channel. The buffer is only good until the next time this method is
     * called. The buffer used for reading is populated by this call and it is
     * not necessary to use getBuffer to create a buffer. This method will
     * assign {@link ReadArgs#readRetVal()} a positive value if there is more
     * data to read, {@link TransportReturnCodes#READ_WOULD_BLOCK} if the read
     * call is blocked, or a failure code. If the socket of the channel is closed
     * by the far end, this method returns error and the channel state is set to CLOSED.
     * 
     * @param readArgs read arguments
     * @param error Error, to be populated in event of an error
     * 
     * @return Buffer that contains data read from network
     * 
     * @see ReadArgs
     */
    public TransportBuffer read(ReadArgs readArgs, Error error);

    /**
     * Retrieves a {@link TransportBuffer} for use.<br>
     * 
     * Typical use: <br>
     * This is called when a buffer is needed to write data to. Generally, the
     * user will populate the {@link TransportBuffer} structure and then pass it to the write method.
     * 
     * @param size Size of the requested buffer
     * @param packedBuffer Set to true if you plan on packing multiple messages into the same buffer
     * @param error Error, to be populated in event of an error
     * 
     * @return buffer to be filled in with valid memory (null returned for no more buffers or error condition) 
     * 
     * @see TransportBuffer
     */
    public TransportBuffer getBuffer(int size, boolean packedBuffer, Error error);

    /**
     * Releases a {@link TransportBuffer} after use.<br>
     * 
     * A buffer obtained through getBuffer() call is returned to the buffer pool 
     * by write() method or releaseBuffer() method. In successful scenarios this 
     * method is not used since the buffers are returned to pool in write().
     * This method must be called when write returned error. It also must be called 
     * for each buffer that has not been written before the channel is closed (i.e. by calling close() method.)
     * 
     * @param buffer buffer to be released
     * @param error Error, to be populated in event of an error
     * 
     * @return {@link TransportReturnCodes}
     * 
     * @see TransportBuffer
     */
    public int releaseBuffer(TransportBuffer buffer, Error error);

    /**
     * Allows user to pack multiple ETA encoded messages into the same ETA Buffer.<br>
     * 
     * Typical use: <br>
     * This is called when the application wants to perform message packing.
     * <ul>
     * <li>Call getBuffer (with packedBuffer set to true) to get a new buffer.
     * <li>Call necessary encode methods.
     * <li>Call this method to write size of packed buffer and prepare for next packed buffer.
     * <li>Repeat above two steps until ready to write, or until there is not
     * enough available bytes remaining to continue packing into the buffer.
     * <li>Call write with the packed buffer.
     * <li>Repeat the entire procedure until all messages are written.
     * </ul>
     * 
     * @param buffer buffer to be packed
     * @param error Error, to be populated in event of an error
     * 
     * @return {@link TransportReturnCodes} or the amount of available bytes remaining in the buffer for packing.
     * 
     * @see TransportBuffer
     */
    public int packBuffer(TransportBuffer buffer, Error error);

    /**
     * Writes on a given channel.<br>
     * 
     * Typical use:<br>
     * This method is called after buffer is populated with a message. This
     * message is then written to the channel internal write buffer. If write is
     * successful, the passed in buffer will be released automatically. In the
     * event of a failure the user needs to call releaseBuffer. In the success
     * case, this method will return the number of bytes pending flush. Note:
     * Data is not written across the network until flush is called.
     * If the channel is closed by the far end, this method
     * returns error and the channel state is set to CLOSED.
     * 
     * @param buffer Buffer to write to the network
     * @param writeArgs Arguments for writing the buffer (WriteArgs)
     * @param error Error, to be populated in event of an error
     * 
     * @return {@link TransportReturnCodes} or the number of bytes pending flush
     * 
     * @see WriteArgs
     */
    public int write(TransportBuffer buffer, WriteArgs writeArgs, Error error);

    /**
     * Flushes data waiting to be written on a given channel.<br>
     * 
     * Typical use:<br>
     * This method pushes the data in the write buffer out to the network.
     * This should be called when write returns that there is data to flush.
     * Under certain circumstances, write will automatically flush data.
     * If the channel is closed by the far end, this method
     * returns error and the channel state is set to CLOSED.
     * 
     * @param error Error, to be populated in event of an error
     * 
     * @return {@link TransportReturnCodes} or the number of bytes pending flush
     */
    public int flush(Error error);

    /**
     * Sends a heartbeat message.<br>
     * 
     * Typical use:<br>
     * This method is called to send some type of ping or heartbeat message.
     * This will send only the message header across the network. This helps
     * reduce overhead on the network, and does not incur any cost of parsing or
     * assembling a special ping message type. It is the user's responsibility
     * to send the ping message in the correct time frame. Since it is assumed a
     * ping or heartbeat is only sent when other writing is not taking place,
     * flush is automatically called once. The return value will be the number
     * of bytes left to flush.
     * If the channel is closed by the far end, this method
     * returns error and the channel state is set to CLOSED.
     * 
     * @param error Error, to be populated in event of an error
     * 
     * @return {@link TransportReturnCodes} or the number of bytes pending flush
     */
    public int ping(Error error);

    /**
     * When a {@link Channel} becomes active for a client or server, this is
     * populated with the negotiated major version number that is associated
     * with the content being sent on this connection. Typically, a major
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
     * When a {@link Channel} becomes active for a client or server, this is
     * populated with the negotiated minor version number that is associated
     * with the content being sent on this connection. Typically, a minor
     * version increase is associated with a fully backward compatible change or
     * extension. The transport layer is data neutral and does not change nor
     * depend on any information in content being distributed. This information
     * is provided to help client and server applications manage the information
     * they are communicating.
     * 
     * 
     * @return the minorVersion
     */
    public int minorVersion();

    /**
     * When a {@link Channel} becomes active for a client or server, this is
     * populated with the protocolType associated with the content being sent on
     * this connection. If the protocolType indicated by a server does not match
     * the protocolType that a client specifies, the connection will be
     * rejected. The transport layer is data neutral and does not change nor
     * depend on any information in content being distributed. This information
     * is provided to help client and server applications manage the information
     * they are communicating.
     * 
     * 
     * @return the protocolType
     */
    public int protocolType();

    /**
     * The state associated with the {@link Channel}. Until the channel has
     * completed its initialization handshake and has transitioned to an active
     * state, no reading or writing can be performed.
     * 
     * @return the state
     * 
     * @see ChannelState
     */
    public int state();

    /**
     * java.nio.channels.SocketChannel of the channel.
     * 
     * @return the scktChannel
     * 
     * @deprecated use {@link #selectableChannel()} instead
     */
    @Deprecated
    public java.nio.channels.SocketChannel scktChannel();

    /**
     * It is possible for a socket channel to change over time, typically due to
     * some kind of connection keep-alive mechanism. If this occurs, this is
     * typically communicated via a return code of
     * {@link TransportReturnCodes#READ_FD_CHANGE}. The previous socketId is
     * stored in oldSocketId so the application can properly unregister and then
     * register the new socketId with their I/O notification mechanism.
     * 
     * @return the oldScktChannel
     * 
     * @deprecated use {@link #oldSelectableChannel()} instead
     */
    @Deprecated
    public java.nio.channels.SocketChannel oldScktChannel();

    /**
     * java.nio.channels.SelectableChannel of the channel.
     * 
     * @return the selectableChannel
     */
    public SelectableChannel selectableChannel();

    /**
     * It is possible for a channel to change over time, typically due to
     * some kind of connection keep-alive mechanism. If this occurs, this is
     * typically communicated via a return code of
     * {@link TransportReturnCodes#READ_FD_CHANGE}. The previous socketId is
     * stored in oldSocketId so the application can properly unregister and then
     * register the new socketId with their I/O notification mechanism.
     * 
     * @return the oldSelectableChannel
     */
    public java.nio.channels.SelectableChannel oldSelectableChannel();

    /**
     * When a {@link Channel} becomes active for a client or server, this is
     * populated with the negotiated ping timeout value. This is the number of
     * seconds after which no communication can result in a connection being
     * terminated. Both client and server applications should send heartbeat
     * information within this interval. The typically used rule of thumb is to
     * send a heartbeat every pingTimeout/3 seconds.
     * 
     * @return the pingTimeout
     */
    public int pingTimeout();

    /**
     * A user specified object, possibly a closure.This value can be set
     * directly or via the connection options and is not modified by the
     * transport. This information can be useful for coupling this
     * {@link Channel} with other user created information, such as a watch list
     * associated with this connection.
     * 
     * @return the userSpecObject
     */
    public Object userSpecObject();

    /**
     * The blocking mode of the channel.
     * 
     * @return the blocking mode
     */
    public boolean blocking();
    
    /**
     * Used for tunneling solutions to reconnect and bridge connections.
     * This only applies to http and encrypted connections, where it might
     * be needed to keep connections alive through proxy servers.
     *
     * @param error Error, to be populated in event of an error
     * 
     * @return {@link TransportReturnCodes}
     */
    public int reconnectClient(Error error);
    
    /**
     * The connection type associated with the {@link Channel}.
     * 
     * @return the connection type
     * 
     * @see ConnectionTypes
     */
    public int connectionType();

    /**
     * For NIProvider and Consumer applications, remote server host name associated with the {@link Channel}.
     * 
     * @return the host name
     */
    public String hostname();

    /**
     * For NIProvider and Consumer applications, remote server port number associated with the {@link Channel}.
     * Relevant for Socket connection, zero for other
     * 
     * @return the port number
     */
    default public int port() {
        return 0;
    }
}
