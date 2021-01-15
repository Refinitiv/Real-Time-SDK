package com.refinitiv.eta.transport;

import java.nio.channels.ServerSocketChannel;
import java.nio.channels.SelectableChannel;

import com.refinitiv.eta.transport.Channel;

/**
 * The ETA Server class is used to represent a server that is listening for
 * incoming connection requests. Any memory associated with a {@link Server}
 * class is internally managed by the ETA Transport Package, and the application
 * does not need to create nor destroy this type.
 * The {@link Server} is typically used to accept or reject incoming connection attempts.
 */
public interface Server
{
    /**
     * Gets information about the server.<BR>
     * 
     * Typical use:<BR>
     * If information about the {@link Server} is needed, such as
     * peakBufferUsage, this method can be called to retrieve this information.
     * 
     * @param info ETA Server Info structure to be populated
     * @param error ETA Error, to be populated in event of an error
     * 
     * @return {@link TransportReturnCodes}
     * 
     * @see ServerInfo
     */
    public int info(ServerInfo info, Error error);

    /**
     * Allows changing some I/O values programmatically for a ETA Server.<BR>
     * 
     * Typical use:<BR>
     * If an I/O value needs to be changed for a server, this is used. Currently
     * this only supports triggering the server peak buffer reset.
     * 
     * <p>
     * Valid codes are:
     * <ul>
     * <li>{@link IoctlCodes#SERVER_PEAK_BUF_RESET}</li>
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
     * Allows changing some I/O values programmatically for a ETA Server.<BR>
     * 
     * Typical use:<BR>
     * If an I/O value needs to be changed for a server, this is used. Currently
     * this only supports changing the shared pool and receive buffer size.
     * 
     * <p>
     * Valid codes are:
     * <ul>
     * <li>{@link IoctlCodes#SERVER_NUM_POOL_BUFFERS} - the return code will be
     * the new value of sharedPoolSize or
     * {@link TransportReturnCodes#FAILURE}. If the
     * value is less than the number of free buffers in the shared pool (due
     * to buffers being in use), the shared pool will be reduced by the
     * number of free buffers.</li>
     * <li>{@link IoctlCodes#SYSTEM_READ_BUFFERS} - for values larger than 64K,
     * use {@link BindOptions#sysRecvBufSize(int)} to set the receive buffer
     * size, prior to calling {@link Transport#bind(BindOptions, Error)}.</li>
     * </ul>
     * 
     * @param code {@link IoctlCodes} code of I/O Option to change
     * @param value Value to change Option to
     * @param error {@link Error} to be populated in event of an error
     * 
     * @return {@link TransportReturnCodes} or a positive number specific to the specified code.
     * 
     * @see BindOptions
     */
    public int ioctl(int code, int value, Error error);

    /**
     * Returns the total number of used buffers for the server.<BR>
     * 
     * Typical use: <BR>
     * This method can be called to find out the number of used buffers for
     * the server. This, in combination with the bufferPoolSize used as input to
     * the Bind call, can be used to monitor and potentially throttle buffer usage.
     * 
     * @param error ETA Error, to be populated in event of an error
     * 
     * @return If less than 0, this is a {@link TransportReturnCodes},
     *         otherwise it is the total number of buffers in use by the server
     */
    public int bufferUsage(Error error);

    /**
     * Closes a ETA Server.<BR>
     * 
     * Typical use:<BR>
     * When done using a Server, this call closes it. Active channels connected
     * to this server will not be closed; this allows them to continue receiving
     * data even if the server is not accepting more connections.
     * 
     * @param error ETA Error, to be populated in event of an error
     * 
     * @return {@link TransportReturnCodes}
     * 
     */
    public int close(Error error);

    /**
     * Accepts an incoming connection and returns a channel which corresponds to
     * it.<BR>
     * 
     * Typical use:<BR>
     * After a server is created using the Bind call, the Accept call can be
     * made. When the socketId of the server detects something to be read, this
     * will check for incoming client connections. When a client successfully
     * connects, a {@link Channel} is returned which corresponds to this
     * connection. This channel can be used to read or write with the connected
     * client. If a clients connect message is not accepted, a negative
     * acknowledgment is sent to the client and no {@link Channel} is returned.
     * 
     * @param opts ETA Accept Options
     * @param error ETA Error, to be populated in event of an error
     * 
     * @return Accepted ETA channel or NULL
     * 
     * @see AcceptOptions
     */
    public Channel accept(AcceptOptions opts, Error error);

    /**
     * ServerSocketChannel of this ETA server.
     * 
     * @return the srvrScktChannel
     * 
     * @deprecated use {@link #selectableChannel()} instead
     */
    @Deprecated
    public ServerSocketChannel srvrScktChannel();

    /**
     * SelectableChannel of this ETA server. Use to register selector.
     * 
     * @return the srvrScktChannel
     */
    public SelectableChannel selectableChannel();

    /**
     * Port number this server is bound to.
     * 
     * @return the portNumber
     */
    public int portNumber();

    /**
     * A user specified object, possibly a closure. This value can be set
     * directly or via the bind options and is not modified by the transport.
     * This information can be useful for coupling this {@link Server} with
     * other user created information, such as a list of associated
     * {@link Channel} structures.
     * 
     * @return the userSpecObject
     */
    public Object userSpecObject();
    
    /**
     * The state associated with this {@link Server}. 
     * 
     * @return the state
     * 
     * @see ChannelState
     */
    public int state();
    
    /** 
	 * The current connection type of the server.  This can be either {@link ConnectionTypes.SOCKET} or {@link ConnectionTypes.ENCRYPTED}.
	 *
	 * @return the server's connection type
	 */
	public int connectionType();
}