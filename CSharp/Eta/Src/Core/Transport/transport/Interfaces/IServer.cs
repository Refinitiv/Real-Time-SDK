/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System.Net.Sockets;

namespace LSEG.Eta.Transports
{
    /// <summary>
    /// The ETA Sever is used to represent a server that is listening for incoming connection requests.
    /// Any resources associated with a <see cref="IServer"/> is internally managed by the ETA Transport 
    /// and the application does not need release this type.
    /// The <see cref="IServer"/> is typically used to accept and reject incoming connection attempts.
    /// </summary>
    public interface IServer
    {
        /// <summary>
        /// Gets information about the server.
        /// </summary>
        /// <remarks>
        /// If information aobut the <see cref="IServer"/> is needed, such as 
        /// <see cref="ServerInfo.PeakBufferUsage"/>, this method can be called to retrieve this information.
        /// </remarks>
        /// <param name="info">ETA Server Info structure to be populated</param>
        /// <param name="error">ETA Error, to be set in event of an error</param>
        /// <returns><see cref="TransportReturnCode"/> to indicate success or failures</returns>
        TransportReturnCode Info(ServerInfo info, out Error error);

        /// <summary>
        /// Allows changing some I/O values programmatically.
        /// If an I/O value needs to be changed for a server, this is used. Currently
        /// this only supports changing the shared pool and receive buffer size.
        /// </summary>
        /// <remarks>
        /// Valid codes are:
        /// <see cref="IOCtlCode.SERVER_NUM_POOL_BUFFERS"/> - the return code will be the new value
        /// of SharedPoolSize or <see cref="TransportReturnCode.FAILURE"/>.
        /// <see cref="IOCtlCode.SYSTEM_READ_BUFFERS"/> - for values larger than 64K, use
        /// <see cref="BindOptions.SysRecvBufSize"/> to set the receive buffer size, prior to
        /// calling <see cref="Transport.Bind(BindOptions, out Error)"/>.
        /// </remarks>
        /// <param name="code"><see cref="IOCtlCode"/> code of I/O Option to change</param>
        /// <param name="value">Value to change Option to</param>
        /// <param name="error"><see cref="Error"/> to be set in event of an error</param>
        /// <returns><see cref="TransportReturnCode"/></returns>
        TransportReturnCode IOCtl(IOCtlCode code, int value, out Error error);

        /// <summary>
        /// Returns the total number of used buffers for the server.
        /// </summary>
        /// <remarks>
        /// This method can be called to find out the number of used buffers for the server.
        /// This, in combination with the BufferPoolSize used as input to the <see cref="Transport.Bind(BindOptions, out Error)"/> call,
        /// can be used to monitor and potentially throttle buffer usage.
        /// </remarks>
        /// <param name="error">Error, to be populated in event of an error</param>
        /// <returns>If less than 0, this is a <see cref="TransportReturnCode"/> otherwise
        /// it is the total number of buffers in use by the server
        /// </returns>
        TransportReturnCode BufferUsage(out Error error);

        /// <summary>
        /// Closes a ETA Server
        /// </summary>
        /// <remarks>
        /// When done using a Server, this call closes it. Active channels connected to this
        /// servr will not be closed; this allows them to continue receiving data even if
        /// the server is not accepting more connections.
        /// </remarks>
        /// <param name="error">ETA Error, to be set in event of an error</param>
        /// <returns><see cref="TransportReturnCode"/> to indicate success or failures</returns>
        TransportReturnCode Close(out Error error);

        /// <summary>
        /// Accepts an incoming connection and returns a channel which corresponds to it.
        /// </summary>
        /// <remarks>
        /// After a server is created using the Bind call, the Accept call can be made.
        /// When the <c>Socket</c> of the server detects something to be read, this
        /// will check for incoming client connections. When a client successfully 
        /// connects, a <see cref="IChannel"/> is returned which corresponds to this 
        /// connection. This channel can be used to read or write the connected client. 
        /// If a client connect message is not accepted, a negative acknowledgement is 
        /// sent to the client and no <see cref="IChannel"/> is returned.
        /// </remarks>
        /// <param name="opts">ETA Accept options</param>
        /// <param name="error">ETA error, to be set in event of an error</param>
        /// <returns>Accepted ETA channel or <c>null</c></returns>
        IChannel Accept(AcceptOptions opts, out Error error);

        /// <summary>
        /// The <see cref="System.Net.Sockets.Socket"/> for this server.
        /// <para>
        /// This is used only to check for readiness of the Socket for readability, writability and erros with
        /// <see cref="System.Net.Sockets.Socket.Select(System.Collections.IList, System.Collections.IList, System.Collections.IList, int)"/>
        /// </para>
        /// </summary>
        /// <value><see cref="System.Net.Sockets.Socket"/></value>
        Socket Socket { get; }

        /// <summary>
        /// Gets port number this server is bound to.
        /// </summary>
        /// <value>The port number</value>
        int PortNumber { get; }

        /// <summary>
        /// A user specified object, possibly a closure. This value can be set
        /// via the bind options and is not modified by the transport.
        /// This information can be useful for coupling this <see cref="IServer"/>
        /// with other user created information, such as a list of associated
        /// <see cref="IChannel"/> instances.
        /// </summary>
        object UserSpecObject { get; }

        /// <summary>
        /// The state associated with the <see cref="IServer"/>.
        /// </summary>
        /// <value><see cref="ChannelState"/></value>
        ChannelState State { get; }

        /// <summary>
        /// The connection type associated with the <see cref="IServer"/>.
        /// </summary>
        /// <value><see cref="CompressionType"/></value>
        ConnectionType ConnectionType { get; }
    }
}
