/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Net.Sockets;
using System.Text;
using LSEG.Eta.Common;

namespace LSEG.Eta.Transports
{
    #region Global delegates

    /// <summary>
    /// 
    /// </summary>
    /// <param name="message"></param>
    internal delegate void MessageHandler(string message);

    /// <summary>
    /// Socket state change delegates.
    /// </summary>
    internal delegate bool ConnectionStateChangeHandler(ISocketChannel sender, SocketEventArgs sea);

    /// <summary>
    /// Data transfer delegates
    /// </summary>
    internal delegate void DataXferHandler(ResultObject aro);

    internal delegate int ReceiveHandler(byte[] buffer, int offset, int size);

    internal delegate int SendHandler(byte[] buffer, int offset, int size);

    #endregion

    /// <summary>
    /// Contains state information for TCP/IP interactions.
    /// </summary>
    internal class ResultObject
    {
        /// <summary>
        /// Used to create an in-bound message.
        /// </summary>
        /// <param name="socket"></param>
        /// <param name="packet_size"></param>
        /// <param name="user_state">SocketChannel Client's state object</param>
        public ResultObject(Socket socket, int packet_size, Object user_state)
        {
            InitiatingSocket = socket;
            Buffer = new ByteBuffer(packet_size);
            UserState = user_state;
        }

        /// <summary>
        /// Used to create an out-bound packet.
        /// </summary>
        /// <param name="socket"></param>
        /// <param name="buffer"></param>
        /// <param name="reservedSize"></param>
        /// <param name="user_state"></param>
        public ResultObject(Socket socket, ByteBuffer buffer, int reservedSize = 1024, object user_state = null)
        {
            InitiatingSocket = socket;
            Buffer = buffer;
            Buffer.Reserve(reservedSize);
            UserState = user_state??buffer;
        }

        /// <summary>
        /// Used to create an out-bound packet.
        /// </summary>
        /// <param name="socket"></param>
        /// <param name="buffer"></param>
        /// <param name="user_state"></param>
        public ResultObject(Socket socket, byte[] buffer, object user_state = null)
        {
            InitiatingSocket = socket;
            Buffer = new ByteBuffer(buffer.Length);
            Buffer.Put(buffer);
            Buffer.Flip();
            UserState = user_state ?? buffer;
        }

        /// <summary>
        /// Used to create an out-bound message.
        /// </summary>
        /// <param name="socket"></param>
        /// <param name="message"></param>
        public ResultObject(Socket socket, string message)
        {
            InitiatingSocket = socket;

            byte[] ar = Encoding.ASCII.GetBytes(message);
            Buffer = new ByteBuffer(ar.Length);
            Buffer.Put(ar);
            Buffer.Flip();
            UserState = Buffer;
        }

        /// <summary>
        /// The default buffer size
        /// </summary>
        public const int DefaultBufferSize = 8192;

        /// <summary>
        /// Socket that initiated the asynchronous operation on the packet.
        /// </summary>
        public Socket InitiatingSocket { get; internal set; }

        /// <summary>
        /// Allows caller of <see cref="SocketChannel"/>SocketChannel library 
        /// to piggyback any arbitrary <i>state</i> object onto a packet.
        /// </summary>
        public object UserState { get; }

        /// <summary>
        /// Buffer of data sent/received from synchronous operation
        /// out of SocketChannel.
        /// </summary>
        public ByteBuffer Buffer { get; }

        /// <summary>
        /// Returns formatted, string representation of AsyncResultObject. 
        /// </summary>
        /// <returns></returns>
        public override string ToString()
        {
            return $@"UserState: {{{UserState?.ToString()??"*null*"}}}";
        }

    }

    ////////////////////////////////////////////////////////////////////////////////////////
    ///
    /// <summary>
    /// Generic holder for event context, both benign - 'OnConnected' and
    /// 'OnDisconnected' - and troubling - 'OnError'.
    /// </summary>
    /// 
    internal class SocketEventArgs : EventArgs
    {
        /// <summary>
        /// Arbitrary object sent that provides context to the error message.
        /// </summary>
        public readonly object Context;
        private readonly string m_message;

        /// <summary>
        /// human readable state
        /// </summary>
        /// <returns>state string</returns>
        public override string ToString()
        {
            string s = "";
            if (Context != null)
            {
                s = Context + NewLine;
            }

            s += m_message;
            return s;
        }

        /// <summary>
        /// construction
        /// </summary>
        /// <param name="context">arbitrary object</param>
        /// <param name="message">message state</param>
        public SocketEventArgs(object context, string message)
        {
            Context = context;
            m_message = message;
        }
    }

}
