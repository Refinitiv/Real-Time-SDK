/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using System.Net.Sockets;
using LSEG.Eta.Common;

namespace LSEG.Eta.Transports
{
    /// <summary>
    /// The ETA Channel is used to represent a connection that can send or receive 
    /// information across a network. This class is used to represent a connection,
    /// regardless of if that was outbound connection or a connection that was
    /// accepted by a listening socket. The <see cref="IChannel"/> is typically used
    /// to perform any action on the connection that it represents (e.g. reading, 
    /// writing, disconnecting, and etc.)
    /// </summary>
    public interface IChannel
    {
        /// <summary>
        /// Continues channel initialization for non-blocking channels.
        /// <para>
        /// Typical use:
        /// 1. Connect using <see cref="Transport.Connect(ConnectOptions, out Error)"/>
        /// 2.  While Channel state is <see cref="ChannelState.INITIALIZING"/> and the
        /// channel detects data to read, call this method.
        /// </para>
        /// </summary>
        /// This method is not necessary for blocking channels, which will
        /// return the <see cref="ChannelState.ACTIVE"/> state after the
        /// <see cref="Transport.Connect(ConnectOptions, out Error)"/> call.
        /// <remarks>
        /// </remarks>
        /// <param name="inProg">InProg Info for compatibility mode reconnection</param>
        /// <param name="error">To be set in event of an error</param>
        /// <returns><see cref="TransportReturnCode"/></returns>
        TransportReturnCode Init(InProgInfo inProg, out Error error);

        /// <summary>
        /// Reads on a given channel.
        /// <para>
        /// Typical use:
        /// This method is called and returns a buffer with any data read from the
        /// channel. The buffer is only good until the next time this method is
        /// called. The buffer used for reading is populated by this call and it is
        /// not necessary to use <see cref="IChannel.GetBuffer(int, bool, out Error)"/>
        /// to create a buffer. This method will assign <see cref="ReadArgs.ReadRetVal"/> 
        /// more than <see cref="TransportReturnCode.SUCCESS"/> if there is more data to read,
        /// <see cref="TransportReturnCode.READ_WOULD_BLOCK"/> if the read call is blocked,
        /// or <see cref="TransportReturnCode.FAILURE"/>. If the socket of the channel is closed
        /// by the far end, this method returns error and the channel state is set to 
        /// <see cref="ChannelState.CLOSED"/>
        /// </para>
        /// </summary>
        /// <param name="readArgs">read arguments</param>
        /// <param name="error">To be set in event of an error</param>
        /// <returns><see cref="ITransportBuffer"/></returns>
        ITransportBuffer Read(ReadArgs readArgs, out Error error);

        /// <summary>
        /// Retrieves a <see cref="ITransportBuffer"/> for use.
        /// <para>
        /// Typical use: <para />
        /// This is called when a buffer is needed to write data to. Generally, the
        /// user will populate the <see cref="ITransportBuffer"/> structure and then pass
        /// it to the <see cref="IChannel.Write(ITransportBuffer, WriteArgs, out Error)"/> method.
        /// </para>
        /// </summary>
        /// <param name="size">Size of the requested buffer</param>
        /// <param name="packedBuffer">Set to <c>true</c> if you plan on packing multiple messages into the same buffer</param>
        /// <param name="error">To be set in event of an error</param>
        /// <returns><see cref="TransportReturnCode"/></returns>
        ITransportBuffer GetBuffer(int size, bool packedBuffer, out Error error);

        /// <summary>
        /// Releases a <see cref="ITransportBuffer"/> after use.
        /// <para>
        /// A buffer obtained through <see cref="IChannel.GetBuffer(int, bool, out Error)"/> call is returned 
        /// to the buffer pool by <see cref="IChannel.Write(ITransportBuffer, WriteArgs, out Error)"/> method or
        /// <see cref="IChannel.ReleaseBuffer(ITransportBuffer, out Error)"/>method. In successful scenarios this
        /// method is not used since the buffers are returned to pool in <see cref="IChannel.Write(ITransportBuffer, WriteArgs, out Error)"/>.
        /// This method must be called when <see cref="IChannel.Write(ITransportBuffer, WriteArgs, out Error)"/> returned error. 
        /// It also must be called for each buffer that has not been written before the channel is closed
        /// (i.e. by calling <see cref="IChannel.Close(out Error)"/> method.)
        /// </para>
        /// </summary>
        /// <param name="buffer">Buffer to be released</param>
        /// <param name="error">To be set in event of an error</param>
        /// <returns><see cref="TransportReturnCode"/></returns>
        TransportReturnCode ReleaseBuffer(ITransportBuffer buffer, out Error error);

        /// <summary>
        /// Writes on a given channel.
        /// <para>
        /// Typical use:<para />
        /// This method is called after buffer is populated with a message.This
        /// message is then written to the channel internal write buffer.If write is
        /// successful, the passed in buffer will be released automatically.In the
        /// event of a failure the user needs to call <see cref="IChannel.ReleaseBuffer(ITransportBuffer, out Error)"/>.
        /// In the success case, this method will return the number of bytes pending
        /// flush. Note: Data is not written across the network until flush is called
        /// except setting <see cref="WriteFlags.DIRECT_SOCKET_WRITE"/> with <see cref="WriteArgs.Flags"/> to write
        /// data directly to transport. If the channel is closed by the far end, this method returns error
        /// and the channel state is set to <see cref="ChannelState.CLOSED"/>.
        /// </para>
        /// </summary>
        /// <param name="buffer">Buffer to write to the network</param>
        /// <param name="writeArgs">Arguments for writing the buffer</param>
        /// <param name="error">To be set in event of an error</param>
        /// <returns><see cref="TransportReturnCode"/></returns>
        TransportReturnCode Write(ITransportBuffer buffer, WriteArgs writeArgs, out Error error);

        /// <summary>
        /// Sends a heartbeat message.
        /// <para>
        /// This method is called to send some type of ping or heartbeat message.
        /// This will send only the message header across the network. This helps
        /// reduce overhead on the network, and does not incur any cost of parsing or
        /// assembling a special ping message type. It is the user's responsibility
        /// to send the ping message in the correct time frame. Since it is assumed
        /// a ping or heartbeat is only sent when other writing is not taking place,
        /// flush is automatically called once. The return value will be the number
        /// of bytes left to flush.
        /// If the channel is closed by the far end, this method
        /// returns error and the channel state is set to <see cref="ChannelState.CLOSED"/>
        /// </para>
        /// </summary>
        /// <param name="error">To be set in event of an error</param>
        /// <returns><see cref="TransportReturnCode"/></returns>
        TransportReturnCode Ping(out Error error);

        /// <summary>
        /// Flushes data waiting to be written on a given channel.
        /// <para>
        /// This method pushes the data in the write buffer out to the network.
        /// This should be called when write returns that there is data to flush.
        /// Under certain circumstances, write will automatically flush data.
        /// If the channel is closed by the far end, this method returns error
        /// and the channel state is set to <see cref="ChannelState.CLOSED"/>
        /// </para>
        /// </summary>
        /// <param name="error">To be set in event of an error</param>
        /// <returns><see cref="TransportReturnCode"/></returns>
        TransportReturnCode Flush(out Error error);

        /// <summary>
        /// Closes a Channel.
        /// When done using a Channel, this call closes it.
        /// This method should also be used when any method on the channel
        /// returned error and the channel state is <see cref="ChannelState.CLOSED"/>.
        /// This method sets the channel's state to <see cref="ChannelState.INACTIVE"/>.
        /// Before calling this method, all <see cref="ITransportBuffer"/> that has been obtained
        /// through <see cref="IChannel.GetBuffer(int, bool, out Error)"/> method and were not written
        /// (through <see cref="IChannel.Write(ITransportBuffer, WriteArgs, out Error)"/> method) should
        /// be released to pool by calling <see cref="IChannel.ReleaseBuffer(ITransportBuffer, out Error)"/> method.
        /// This method returns this channel to the channel pool. 
        /// </summary>
        /// <param name="error">To be set in event of an error</param>
        /// <returns><see cref="TransportReturnCode"/></returns>
        TransportReturnCode Close(out Error error);

        /// <summary>
        /// Gets information about the channel.
        /// </summary>
        /// <remarks>
        /// If information about the <see cref="IChannel"/> is needed, such as MaxFragmentSize or MaxOutputBuffers,
        /// this method can be called to retrieve this information. If the channel is closed by the far end, this
        /// method returns error and the channel sate is set to <see cref="ChannelState.CLOSED"/>.
        /// </remarks>
        /// <param name="info"><see cref="ChannelInfo"/> structure to be populated</param>
        /// <param name="error">To be set in event of an error</param>
        /// <returns><see cref="TransportReturnCode"/></returns>
        TransportReturnCode Info(ChannelInfo info, out Error error);

        /// <summary>
        /// Allows changing some I/O values programmatically.
        /// If an I/O value needs to be changed, this method is used.
        /// </summary>
        /// <remarks>
        /// Valid codes are:
        /// <see cref="IOCtlCode.PRIORITY_FLUSH_ORDER"/>
        /// </remarks>
        /// <param name="code"><see cref="IOCtlCode"/> code of I/O Option to change</param>
        /// <param name="value">Value to change Option to</param>
        /// <param name="error"><see cref="Error"/> to be set in event of an error</param>
        /// <returns><see cref="TransportReturnCode"/></returns>
        TransportReturnCode IOCtl(IOCtlCode code, object value, out Error error);

        /// <summary>
        /// Allows changing some I/O values programmatically.
        /// If an I/O value needs to be changed, this method is used.
        /// </summary>
        /// <remarks>
        /// Valid codes are:
        /// <see cref="IOCtlCode.MAX_NUM_BUFFERS"/> - the return code will be the new value
        /// of MaxOutputBuffers or <see cref="TransportReturnCode.FAILURE"/>. If the specified
        /// value is less than GuaranteedOutputBuffers, the GuaranteedOutputBuffers value will be used.
        /// <see cref="IOCtlCode.NUM_GUARANTEED_BUFFERS"/> - the return code will be the new value
        /// of GuarenteedOutputBuffers or <see cref="TransportReturnCode.FAILURE"/>. If the specified
        /// value is larger than MaxOutputBuffers, MaxOutputBuffers will be set to the value.
        /// <see cref="IOCtlCode.HIGH_WATER_MARK"/>
        /// <see cref="IOCtlCode.SYSTEM_READ_BUFFERS"/> - for values larger than 64K, use
        /// <see cref="ConnectOptions.SysRecvBufSize"/> to set the receive buffer size, prior to
        /// calling <see cref="Transport.Connect(ConnectOptions, out Error)"/>.
        /// <see cref="IOCtlCode.SYSTEM_WRITE_BUFFERS"/>
        /// <see cref="IOCtlCode.COMPRESSION_THRESHOLD"/>
        /// If the channel is closed by the far end, this method returns error and the channel state
        /// is set to CLOSED.
        /// </remarks>
        /// <param name="code"><see cref="IOCtlCode"/> code of I/O Option to change</param>
        /// <param name="value">Value to change Option to</param>
        /// <param name="error"><see cref="Error"/> to be set in event of an error</param>
        /// <returns><see cref="TransportReturnCode"/></returns>
        TransportReturnCode IOCtl(IOCtlCode code, int value, out Error error);

        /// <summary>
        /// Returns the total number of used buffers for this channel.
        /// </summary>
        /// <remarks>
        /// This method can be called to find out the number of used buffers for the calling channel.
        /// This, in combination with the MaxOutputBuffers obtained from the <see cref="Info(ChannelInfo, out Error)"/> call,
        /// can be used to monitor and potentially throttle buffer usage.
        /// </remarks>
        /// <param name="error">Error, to be populated in event of an error</param>
        /// <returns>If less than 0, this is a <see cref="TransportReturnCode"/> otherwise
        /// it is the total number of buffers in use by this channel
        /// </returns>
        TransportReturnCode BufferUsage(out Error error);

        /// <summary>
        /// Packs the provided input buffer.
        /// </summary>
        /// <param name="buffer">The input buffer to be packed</param>
        /// <param name="error"></param>
        /// <returns><see cref="TransportReturnCode"/> value indicating failure in case packing was not successful,
        /// in case of successful packing returns the number of bytes left in the input buffer.</returns>
        TransportReturnCode PackBuffer(ITransportBuffer buffer, out Error error);

        /// <summary>
        /// The state associated with the <see cref="IChannel"/>. Until the channel has
        /// completed its initialization handshake and has transitioned to an <see cref="ChannelState.ACTIVE"/>
        /// state, no reading or writing can be performed.
        /// </summary>
        /// <value><see cref="ChannelState"/></value>
        ChannelState State { get; }

        /// <summary>
        /// The underlying <see cref="System.Net.Sockets.Socket"/> for this channel.
        /// <para>
        /// This is used only to check for readiness of the Socket for readability, writability and erros with
        /// <see cref="System.Net.Sockets.Socket.Select(System.Collections.IList, System.Collections.IList, System.Collections.IList, int)"/>
        /// </para>
        /// </summary>
        /// <value><see cref="System.Net.Sockets.Socket"/></value>
        Socket Socket { get; }

        /// <summary>
        /// It is possible for a channel to change over time, typically due to
        /// some kind of connection keep-alive mechanism. If this occurs, this is
        /// typically communicated via a return code of <see cref="TransportReturnCode.READ_FD_CHANGE"/>
        /// he previous <see cref="System.Net.Sockets.Socket"/> is stored in <see cref="OldSocket"/> so the application
        /// can properly unregister and then register the new <see cref="Socket"/> with their I/O notification mechanism.
        /// </summary>
        /// <value><see cref="System.Net.Sockets.Socket"/></value>
        Socket OldSocket { get; }

        /// <summary>
        /// When a <see cref="IChannel"/> becomes active for a client or server, this is
        /// populated with the negotiated major version number that is associated
        /// with the content being sent on this connection. Typically, a major
        /// version increase is associated with the introduction of incompatible
        /// change. The transport layer is data neutral and does not change nor
        /// depend on any information in content being distributed. This information
        /// is provided to help client and server applications manage the information
        /// they are communicating.
        /// </summary>
        /// <value>The major version number</value>
        int MajorVersion { get; }

        /// <summary>
        /// When a <see cref="IChannel"/> becomes active for a client or server, this is
        /// populated with the negotiated minor version number that is associated
        /// with the content being sent on this connection. Typically, a minor
        /// version increase is associated with a fully backward compatible change or
        /// extension. The transport layer is data neutral and does not change nor
        /// depend on any information in content being distributed.This information
        /// is provided to help client and server applications manage the information
        /// they are communicating.
        /// </summary>
        /// <value>The minor version number</value>
        int MinorVersion { get; }

        /// <summary>
        /// When a <see cref="IChannel"/> becomes active for a client or server, this is
        /// populated with the protocolType associated with the content being sent on
        /// this connection. If the protocolType indicated by a server does not match
        /// the protocolType that a client specifies, the connection will be
        /// rejected. The transport layer is data neutral and does not change nor
        /// depend on any information in content being distributed. This information
        /// is provided to help client and server applications manage the information
        /// they are communicating.
        /// </summary>
        /// 
        ProtocolType ProtocolType { get; }

        /// <summary>
        /// When a <see cref="IChannel"/>
        /// becomes active for a client or server, this is
        /// populated with the negotiated ping timeout value.This is the number of
        /// seconds after which no communication can result in a connection being
        /// terminated.Both client and server applications should send heartbeat
        /// information within this interval. The typically used rule of thumb is to
        /// send a heartbeat every pingTimeout/3 seconds.
        /// </summary>
        /// <value>The ping timeout</value>
        int PingTimeOut { get; }

        /// <summary>
        /// A user specified object, possibly a closure. This value can be set
        /// via the connection options and is not modified by the transport.
        /// This information can be useful for coupling this <see cref="IChannel"/>
        /// with other user created information, such as a watch list associated
        /// with this connection.
        /// </summary>
        object UserSpecObject { get; }

        /// <summary>
        /// Check whether the channel is blocking mode.
        /// </summary>
        /// <value><c>True</c> if the channel is in blocking mode otherwise false</value>
        bool Blocking { get; }

        /// <summary>
        /// The connection type associated with the <see cref="IChannel"/>.
        /// </summary>
        /// <value><see cref="CompressionType"/></value>
        ConnectionType ConnectionType { get; }

        /// <summary>
        /// For NIProvider and Consumer applications, remote server host name associated with the <see cref="IChannel"/>.
        /// </summary>
        /// <value>The host name</value>
        string HostName { get; }

        /// <summary>
        /// For NIProvider and Consumer applications, remote server port number associated with the <see cref="IChannel"/>.
        /// Relevant for Socket connection, zero for other
        /// </summary>
        /// <value>The port number</value>
        int Port { get;  }
    }
}
