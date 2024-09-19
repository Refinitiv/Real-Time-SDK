/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Transports;
using LSEG.Eta.Common;
using System.Collections.Generic;
using System.Text;
using System.Security.Authentication;

namespace LSEG.Eta.Transports
{
    /// <summary>
    /// ETA Channel Info available through <see cref="IChannel.Info(ChannelInfo, out Error)"/> method call.
    /// </summary>
    /// <seealso cref="IChannel"/>
    sealed public class ChannelInfo
    {
        /// <summary>
        /// The default constructor
        /// </summary>
        public ChannelInfo()
        {
            Clear();
        }

        /// <summary>
        /// Gets the maximum size buffer allowed to be written to the network. If a larger buffer is required,
        /// ETA Transport will internally fragment the larger buffer into smaller MaxFragmentSize buffers.
        /// This is the largest size a user can request while still being 'packable'.
        /// </summary>
        /// <value>The MaxFragmentSize</value>
        public int MaxFragmentSize { get; internal set; }

        /// <summary>
        /// Gets the maximum number of output buffers allowed for use by each <see cref="IChannel"/>. Shared pool
        /// buffers are only used if all <see cref="GuaranteedOutputBuffers"/> are unavaiable. If equal to
        /// the <see cref="GuaranteedOutputBuffers"/> value, no shared pool buffers are available.
        /// </summary>
        /// <value>The MaxOutputBuffers</value>
        public int MaxOutputBuffers { get; internal set; }

        /// <summary>
        /// Gets the guaranteed number of buffers made available for this <see cref="IChannel"/> to use while
        /// writing data. Each buffer can contain <see cref="MaxFragmentSize"/> bytes. Guaranteed output
        /// buffers are allocated at initialization time.
        /// </summary>
        /// <value>The guaranteed output buffers</value>
        public int GuaranteedOutputBuffers { get; internal set; }

        /// <summary>
        /// Gets the number of sequential input buffers used by this <see cref="IChannel"/> for reading data into.
        /// This controls the maximum number of bytes that can be handled with a single network read operation
        /// on each channel. Each input can contain <see cref="MaxFragmentSize"/> bytes. Input buffers are
        /// allocated at initialization time.
        /// </summary>
        /// <value>The number of input buffers</value>
        public int NumInputBuffers { get; internal set; }

        /// <summary>
        /// Gets the negotiated ping timeout value. The typically used rule of thumb is to send a heartbeat every
        /// <see cref="PingTimeout"/>/3 seconds.
        /// </summary>
        /// <value>The ping timeout</value>
        public int PingTimeout { get; internal set; }

        /// <summary>
        /// Checks whether heartbeat messages are required to flow from the client to the server. 
        /// </summary>
        /// <remarks>
        /// LSEG Real-Time Distribution System and other Refinitiv components typically require this value to be
        /// set to <c>true</c>.
        /// </remarks>
        /// <value><c>true</c> if heartbeat messages are required from the client to the server otherwise <c>false</c></value>
        public bool ClientToServerPings { get; internal set; }

        /// <summary>
        /// Checks whether heartbeat messages are required to flow from the server to the client.
        /// </summary>
        /// <remarks>
        /// LSEG Real-Time Distribution System and other Refinitiv components typically require this value to be
        /// set to <c>true</c>.
        /// </remarks>
        /// <value><c>true</c> if heartbeat messages are required from the server to the client otherwise <c>false</c></value>
        public bool ServerToClientPings { get; internal set; }

        /// <summary>
        /// Gets the size of the send or output buffer associated with the underlying transport. ETA Transport has additional
        /// output buffers, controlled by <see cref="MaxOutputBuffers"/> and <see cref="GuaranteedOutputBuffers"/>. 
        /// </summary>
        /// <value>The system send buffer size</value>
        public int SysSendBufSize { get; internal set; }

        /// <summary>
        /// Gets the size of the receive or input buffer associated with the underlying transport. ETA Transport has an additional
        /// controlled by <see cref="NumInputBuffers"/>.
        /// </summary>
        /// <value>The system receive buffer size</value>
        public int SysRecvBufSize { get; internal set; }

        /// <summary>
        /// Gets the type of compression being performed for this connection.
        /// </summary>
        /// <value>The compression type</value>
        public CompressionType CompressionType { get; internal set; }

        /// <summary>
        /// Gets the threshold to determine when to compress a message. Messages smaller tha this compression threshold will not
        /// be compressed while larger messages will.
        /// </summary>
        /// <value>The compression threshold</value>
        public int CompressionThresHold { get; internal set; }

        /// <summary>
        /// Gets the currently priority level order used when flushing buffers to the connection, where H = High priority, 
        /// M = Medium priority, and L = Low priority. When passed to <see cref="IChannel.Write(ITransportBuffer, WriteArgs, out Error)"/>,
        /// each buffer is also associated with the priority level it should be written at. The default priority flush strategy will
        /// write buffers in the order High, Medium, High, Low, High, Medium. This provides a slight advantage to the medium priority
        /// level and a greater advantage to high priority data. Data order is preserved within each priority level and if all buffers
        /// are written with the same priority, no ordering change will occur.
        /// </summary>
        /// <value>The priority flush strategy</value>
        public string PriorityFlushStrategy { get; internal set; }

        /// <summary>
        /// Gets the IP address of the connecting client.
        /// </summary>
        /// <value>The client IP</value>
        public string ClientIP { get; internal set; }

        /// <summary>
        /// Gets the hostname of the connecting client.
        /// </summary>
        /// <value>The client hostname</value>
        public string ClientHostname { get; internal set; }

        /// <summary>
        /// Gets a list of component information received for each connection.
        /// </summary>
        /// <value>A list of <see cref="ComponentInfo"/></value>
        public List<ComponentInfo> ComponentInfoList { get; internal set; }

        /// <summary>
        /// Gets the encryption protocol type used to authenticate the the <see cref="ConnectionType.ENCRYPTED"/> connection type.
        /// </summary>
        /// <remarks>This property is valid only for the <see cref="ConnectionType.ENCRYPTED"/> connection type.</remarks>
        /// <value>The negotiated protocol for the Channel; otherwise <see cref="SslProtocols.None"/></value>
        public SslProtocols EncryptionProtocol { get; internal set; }

        /// <summary>
        /// Clears ETA Channel Info.
        /// </summary>
        public void Clear()
        {
            MaxOutputBuffers = 0;
            GuaranteedOutputBuffers = 0;
            NumInputBuffers = 0;
            PingTimeout = 0;
            ClientToServerPings = false;
            ServerToClientPings = false;
            SysSendBufSize = 0;
            SysRecvBufSize = 0;
            CompressionType = 0;
            CompressionThresHold = 0;
            ClientIP = null;
            ClientHostname = null;
            ComponentInfoList = null;
            EncryptionProtocol = SslProtocols.None;
        }

        /// <summary>
        /// The string representation of this object
        /// </summary>
        /// <returns>The string value</returns>
        public override string ToString()
        {
            if (stringBuilder is null)
            {
                stringBuilder = new StringBuilder();
            }

            stringBuilder.Clear();

            if (ComponentInfoList != null)
            {
                stringBuilder.AppendLine();
                int ciIndex = 0;
                foreach (var componentInfo in ComponentInfoList)
                {
                    if (componentInfo.ComponentVersion != null)
                    {
                        stringBuilder.Append("\t\tComponentInfo[");
                        stringBuilder.Append(ciIndex);
                        stringBuilder.Append("]: ");
                        stringBuilder.Append(componentInfo.ComponentVersion.ToString());
                        stringBuilder.AppendLine();
                        ++ciIndex;
                    }
                }
            }

            return $"ChannelInfo\n\tmaxFragmentSize: {MaxFragmentSize}\n" +
                $"\tmaxOutputBuffers: {MaxOutputBuffers}\n" +
                $"\tguaranteedOutputBuffers: {GuaranteedOutputBuffers}\n" +
                $"\tnumInputBuffers: {NumInputBuffers}\n" +
                $"\tpingTimeout: {PingTimeout}\n" +
                $"\tclientToServerPings: {ClientToServerPings}\n" +
                $"\tserverToClientPings: {ServerToClientPings}\n" +
                $"\tsysSendBufSize: {SysSendBufSize}\n" +
                $"\tsysRecvBufSize: {SysRecvBufSize}\n" +
                $"\tCompressionType: {(long)CompressionType}\n" +
                $"\tCompressionThreshold: {CompressionThresHold}\n" +
                $"\tpriorityFlushStrategy: {PriorityFlushStrategy}\n" +
                $"\tclientIP: {ClientIP}\n" +
                $"\tclientHostName: {ClientHostname}\n" +
                $"\tComponentInfo: {stringBuilder}\n" +
                $"\tEncryptionProtocol: {EncryptionProtocol}";
        }

        private StringBuilder stringBuilder;
    }
}
