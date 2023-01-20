/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Common;
using LSEG.Eta.Codec;
using LSEG.Eta.Transports;
using System;
using System.Collections.Generic;
using System.Net.Sockets;
using System.Text;
//APIQA
using System.IO;
//END APIQA

namespace LSEG.Eta.Example.Common
{
    /// <summary>
    /// Manages ETA channel and provides methods for connection establishment,
    /// channel initialization, read, and write methods for Consumer and NIProvider
    /// example applications.
    /// </summary>
    public class ChannelSession
    {
        public const int MAX_MSG_SIZE = 1024;
        public const int NUM_CLIENT_SESSIONS = 5;

        public bool ShouldRecoverConnection { get; set; } = true;

        public IChannel? Channel { get; private set; }

        private ReadArgs readArgs = new ReadArgs();

        public ConnectOptions ConnectOptions { get; set; } = new ConnectOptions();

        private int selectTime = -1;
        public int SelectRetVal { get; set; }

        private bool userSpecifiedSelectTime = false;

        private StringBuilder xmlString = new StringBuilder();

        private XmlTraceDump xmlTraceDump = new XmlTraceDump();
        public bool ShouldXmlTrace { get; set; } = false;

        private DataDictionary? dictionaryForXml;
        private WriteArgs writeArgs = new WriteArgs();

        public long LoginReissueTime { get; set; } // represented by epoch time in milliseconds
        public bool CanSendLoginReissue { get; set; }
        public bool IsLoginReissue { get; set; }

        public long SocketFdValue { get; set; }
        public bool ShouldWrite { get; set; } = false;
        
        // APIQA
        private string xmlLogFileName = "RsslNIProvider.xml";
        //END APIQA
        
        /// <summary>
        /// Instantiates ChannelSession
        /// </summary>
        public ChannelSession()
        {
            ConnectOptions.Clear();
        }

        /// <summary>
        /// Allows the user to specify a select timeout. 
        /// The default select timeout will be one third of the channel's negotiated ping time out. 
        /// Providers will use this method to control their content update time.
        /// </summary>
        /// <param name="selectTime">a value greater than zero</param>
        /// <returns>true if the selectTime was greater than zero</returns>
        public bool SelectTime(int selectTime)
        {
            if (selectTime <= 0)
                return false;

            userSpecifiedSelectTime = true;
            this.selectTime = selectTime;
            return true;
        }

        /// <summary>
        /// Allows the user to trace messages via XML.
        /// </summary>
        /// <param name="dictionary">dictionary for XML tracing (can be null)</param>
        public void EnableXmlTrace(DataDictionary dictionary)
        {
            dictionaryForXml = dictionary;
            ShouldXmlTrace = true;
        }

        /// <summary>
        ///Initializes the ETA transport API and all internal members. 
        ///This is the first method called when using the ETA.It initializes internal data structures.
        /// </summary>
        /// <param name="globalLock">flag to enable global locking on ETA Transport</param>
        /// <param name="error">ETA Error, to be populated in event of an error</param>
        /// <returns><see cref="TransportReturnCode"/> value</returns>
        public TransportReturnCode InitTransport(bool globalLock, out Error error)
        {
            ConnectOptions.GuaranteedOutputBuffers = 500;
            ConnectOptions.MajorVersion = Codec.Codec.MajorVersion();
            ConnectOptions.MinorVersion = Codec.Codec.MinorVersion();
            ConnectOptions.ProtocolType = Transports.ProtocolType.RWF;
            InitArgs initArgs = new InitArgs();
            initArgs.GlobalLocking = globalLock;
            
            return Transport.Initialize(initArgs, out error);
        }

        /// <summary>
        /// Performs flush operation.
        /// </summary>
        /// <param name="error"><see cref="Error"/> instance that contains error information in case of failed flush operation.</param>
        /// <returns><see cref="TransportReturnCode"/> value indicating the status of the flush execution.</returns>
        public TransportReturnCode Flush(out Error? error)
        {
            if (GetChannelState() == ChannelState.INACTIVE)
            {
                error = null;
                return TransportReturnCode.SUCCESS;
            } 
            else
            {
                return Channel!.Flush(out error);
            }
        }

        /// <summary>
        /// Determines channel's state.
        /// </summary>
        /// <returns><see cref="ChannelState"/> value.</returns>
        public ChannelState GetChannelState()
        {
            return Channel == null ? ChannelState.INACTIVE : Channel.State;
        }

        /// <summary>
        /// Retrieves the TransportBuffer from channel.
        /// </summary>
        /// <param name="size">Size of the transport buffer to retrieve.</param>
        /// <param name="packedBuffer">Set to true if you plan on packing multiple messages into the same buffer</param>
        /// <param name="error">ETA error information in case of failure.</param>
        /// <returns><see cref="ITransportBuffer"/> instance</returns>
        public ITransportBuffer? GetTransportBuffer(int size, bool packedBuffer, out Error? error)
        {
            error = null;
            if (Channel != null)
            {
                return Channel.GetBuffer(size, packedBuffer, out error);
            } else
            {
                error = new Error()
                {
                    ErrorId = TransportReturnCode.FAILURE,
                    Text = "Failed getting TransportBuffer: Channel is null"
                };
                return null;
            }
        }

        /// <summary>
        /// Reads from a channel / Writes to channel if necessary.
        /// </summary>
        /// <param name="pingHandler">PingHandler's flag for server message received is set when read is done.</param>
        /// <param name="error">ETA error information in case of read failure.</param>
        /// <returns><see cref="TransportReturnCode"/> value</returns>
        public TransportReturnCode Select(PingHandler pingHandler, ResponseCallback callback, out Error? error)
        {
            if (Channel == null)
            {
                error = new Error()
                {
                    Text = "Channel instance is null..."
                };
                return TransportReturnCode.FAILURE;
            }
            
            try
            {
                if (Channel.Socket != null)
                {
                    bool canRead = Channel.Socket.Poll(selectTime, System.Net.Sockets.SelectMode.SelectRead);
                    bool canWrite = ShouldWrite && Channel.Socket.Poll(0, System.Net.Sockets.SelectMode.SelectWrite);

                    switch (Channel.State)
                    {
                        case ChannelState.ACTIVE:
                            if (canRead)
                            {
                                TransportReturnCode ret = ReadInt(pingHandler, callback, out error);
                                if (ret < TransportReturnCode.SUCCESS)
                                {
                                    return ret;
                                }
                            }

                            // flush for write file descriptor and active state
                            if (canWrite)
                            {
                                if (Channel.Flush(out error) == TransportReturnCode.SUCCESS)
                                {
                                    ShouldWrite = false;
                                }
                            }
                            break;
                        case ChannelState.CLOSED:
                            return RecoverConnection(out error);
                        default:
                            break;
                    }
                }               
            } 
            catch (Exception e)
            {
                error = new Error()
                {
                    ErrorId = TransportReturnCode.FAILURE,
                    Text = "Failed to perform Channel operations : " + e.Message
                };
                return TransportReturnCode.FAILURE;
            }

            error = null;
            return TransportReturnCode.SUCCESS;
        }

        /// <summary>
        /// Initializes ETA non-blocking channel.
        /// </summary>
        /// <param name="inProg"><see cref="InProgInfo"/> instance</param>
        /// <param name="error">ETA error information in case of failure</param>
        /// <returns><see cref="TransportReturnCode"/> value</returns>
        public TransportReturnCode InitChannel(InProgInfo inProg, out Error? error)
        {
            if (Channel == null)
            {
                error = new Error()
                {
                    ErrorId = TransportReturnCode.FAILURE,
                    Text = "Channel is null."
                };
                return TransportReturnCode.FAILURE;
            }

            TransportReturnCode ret = Channel.Init(inProg, out error);
            if (ret < TransportReturnCode.SUCCESS)
            {
                Console.WriteLine("Error initializing channel: " + error.Text + ". Will retry shortly.");
                return RecoverConnection(out error);
            }

            switch (ret)
            {
                case TransportReturnCode.SUCCESS:
                    if (!userSpecifiedSelectTime)
                    {
                        selectTime = Channel.PingTimeOut / 3;
                    }
                    SocketFdValue = Channel.Socket.Handle.ToInt64();
                    ShouldRecoverConnection = false;
                    break;
                case TransportReturnCode.CHAN_INIT_IN_PROGRESS:
                    if (inProg.Flags == InProgFlags.SCKT_CHNL_CHANGE)
                    {
                        return ReRegister(inProg);
                    }
                    break;
                default:
                    error = new Error()
                    {
                        Text = $"Unexpected return code during channel initialization: {ret}"
                    };
                    if (UnInit(out var initError) != TransportReturnCode.SUCCESS) Console.WriteLine($"Uninitialization failure: {initError?.Text}");
                    return TransportReturnCode.FAILURE;
            }

            return ret;
        }

        /// <summary>
        /// Closes the ETA channel and uninitilizes Transport.
        /// </summary>
        /// <param name="error">ETA error information in case of failure</param>
        /// <returns><see cref="TransportReturnCode"/> value</returns>
        public TransportReturnCode UnInit(out Error? error)
        {
            // clean up channel 
            TransportReturnCode ret = Close(out error);
            if (ret != TransportReturnCode.SUCCESS)
                return ret;

            // clear the connect options
            ConnectOptions.Clear();

            return Transport.Uninitialize();
        }

        /// <summary>
        /// Prepares application for reconnection.
        /// </summary>
        /// <param name="error">ETA error information in case of failure</param>
        /// <returns><see cref="TransportReturnCode"/> value</returns>
        public TransportReturnCode RecoverConnection(out Error? error)
        {
            ShouldRecoverConnection = true;
            return Close(out error);
        }

        /// <summary>
        /// Establishes the outbound connection to the ETA server. 
        /// If connection is established, registers the channel for read and write select operation. 
        /// It recovers connection in case of connection failure.It assumes 
        /// application has called <see cref="InitTransport(bool, out Error)"/> before calling this method.
        /// Typical usage is for establishing connection is:<br>
        /// 1. Call <see cref="Connect(out Error)"/><br>
        /// 2.Call <see cref="InitChannel(InProgInfo, out Error)"/> in loop until channel becomes active, ChannelState.ACTIVE
        /// </summary>
        /// <param name="error">ETA error information in case of failure</param> 
        /// <returns><see cref="TransportReturnCode"/> value</returns>
        public TransportReturnCode Connect(out Error? error)
        {
            Channel = Transport.Connect(ConnectOptions, out error);
            if (Channel == null)
            {
                Console.WriteLine($"Connection failure, error: {error?.Text} . Will retry shortly.");
                return RecoverConnection(out error);
            }

            ShouldRecoverConnection = false;
            return TransportReturnCode.SUCCESS;
        }

        /// <summary>
        /// Writes the content of the <see cref="ITransportBuffer"/> to the ETA channel.
        /// </summary>
        /// <param name="buffer">the msg buffer</param>
        /// <param name="error">ETA error information in case of failure</param>
        /// <returns><see cref="TransportReturnCode"/> value</returns>
        public TransportReturnCode Write(ITransportBuffer buffer, out Error error)
        {
            ITransportBuffer tempBuf = buffer;
            if (Channel == null)
            {
                error = new Error()
                {
                    Text = "Failed Write operation: Channel is null.",
                    ErrorId = TransportReturnCode.FAILURE
                };
                return TransportReturnCode.FAILURE;
            }
            writeArgs.Clear();
            writeArgs.Priority = WritePriorities.HIGH;
            writeArgs.Flags = WriteFlags.DIRECT_SOCKET_WRITE;

            if (ShouldXmlTrace)
            {
                xmlString.Clear();
                xmlString.Append($"\nWrite message ({Channel.ProtocolType.ToString()}): ");
                xmlTraceDump.DumpBuffer(Channel, (int)Channel.ProtocolType, tempBuf, dictionaryForXml, xmlString, out error);
                Console.WriteLine(xmlString);
                //APIQA
                File.AppendAllText(xmlLogFileName, xmlString.ToString());
                //END APIQA
            }

            TransportReturnCode ret = Channel.Write(tempBuf, writeArgs, out error);

            if (ret > TransportReturnCode.SUCCESS)
            {
                ShouldWrite = true;
            }
            if (ret <= TransportReturnCode.FAILURE)
            {
                if (ret == TransportReturnCode.WRITE_CALL_AGAIN)
                {
                    while (ret == TransportReturnCode.WRITE_CALL_AGAIN)
                    {
                        ret = Channel.Flush(out error);
                        if (ret < TransportReturnCode.SUCCESS)
                        {
                            Console.WriteLine("Flush failed with return code " + ret + ": " + error.Text);
                        }
                        ret = Channel.Write(tempBuf, writeArgs, out error);
                    }
                    if (ret > TransportReturnCode.SUCCESS)
                    {
                        ShouldWrite = true;
                    }
                }
                else if (ret == TransportReturnCode.WRITE_FLUSH_FAILED && Channel.State != ChannelState.CLOSED)
                {
                     // register for write if flush failed so that we can be notified
                     // when write buffer is available for write/flush again
                    ShouldWrite = true;
                }
                else
                {
                    // write failed, release buffer
                    Channel.ReleaseBuffer(tempBuf, out _);
                    return ret;
                }
            } 

            return TransportReturnCode.SUCCESS;
        }

        /// <summary>
        /// Allows the user to specify the connect options.
        /// </summary>
        /// <param name="cOpts">connect options to set</param>
        public void CopyConnectOptions(ConnectOptions cOpts)
        {
            if (cOpts != null)
            {
                ConnectOptions.Blocking = cOpts.Blocking;
                ConnectOptions.ChannelReadLocking = cOpts.ChannelReadLocking;
                ConnectOptions.ChannelWriteLocking = cOpts.ChannelWriteLocking;
                ConnectOptions.ConnectionType = cOpts.ConnectionType;
                ConnectOptions.GuaranteedOutputBuffers = cOpts.GuaranteedOutputBuffers;
                ConnectOptions.NumInputBuffers = cOpts.NumInputBuffers;
                if (cOpts.SysSendBufSize > 0)
                {
                    ConnectOptions.SysSendBufSize = cOpts.SysSendBufSize;
                }
                if (cOpts.SysRecvBufSize > 0)
                {
                    ConnectOptions.SysRecvBufSize = cOpts.SysRecvBufSize;
                }
                ConnectOptions.PingTimeout = cOpts.PingTimeout;

                if (cOpts.UserSpecObject != null)
                {
                    ConnectOptions.UserSpecObject = cOpts.UserSpecObject;
                }
            }
        }

        private TransportReturnCode ReadInt(PingHandler pingHandler, ResponseCallback responseCallback, out Error? error)
        {
            ITransportBuffer messageBuffer;
            do
            {
                messageBuffer = Channel!.Read(readArgs, out error);
                if (messageBuffer != null)
                {
                    if (ShouldXmlTrace)
                    {
                        xmlString.Clear();
                        xmlString.Append($"\nRead message ({Channel.ProtocolType.ToString()}): ");
                        xmlTraceDump.DumpBuffer(Channel, (int)Channel.ProtocolType, messageBuffer, dictionaryForXml, xmlString, out error);
                        Console.WriteLine(xmlString);
                        //APIQA
                        File.AppendAllText(xmlLogFileName, xmlString.ToString());
                        //END APIQA
                    }
                    responseCallback.ProcessResponse(this, messageBuffer);
                    pingHandler.ReceivedRemoteMsg = true;
                }
                else
                {
                    switch (readArgs.ReadRetVal)
                    {
                        case TransportReturnCode.FAILURE:
                            if (Channel.State == ChannelState.CLOSED)
                            {
                                Console.WriteLine("Channel inactive, recovering connection...");                               
                                if (RecoverConnection(out error)  < TransportReturnCode.SUCCESS)
                                {
                                    Console.WriteLine("Error recovering connection: " + error?.Text);
                                    Console.WriteLine("Consumer exits...");
                                    Environment.Exit((int)TransportReturnCode.FAILURE);
                                }
                                return TransportReturnCode.SUCCESS;
                            } else
                            {
                                error = new Error()
                                {
                                    Text = "Failed reading from the channel that is not closed"
                                };
                                return TransportReturnCode.FAILURE;
                            }
                        case TransportReturnCode.READ_FD_CHANGE:
                            HandleFDChange();
                            break;
                        case TransportReturnCode.READ_PING:
                            pingHandler.ReceivedRemoteMsg = true;
                            break;
                        default:
                            break;
                    }
                }
            } while (readArgs.ReadRetVal > TransportReturnCode.SUCCESS);

            return TransportReturnCode.SUCCESS;
        }

        private void HandleFDChange()
        {
            SocketFdValue = Channel!.Socket.Handle.ToInt64();
        }

        private TransportReturnCode ReRegister(InProgInfo inProg)
        {
            SocketFdValue = inProg.NewSocket.Handle.ToInt64();
            return TransportReturnCode.SUCCESS;
        }

        private TransportReturnCode Close(out Error? error)
        {
            TransportReturnCode ret;
            error = null;
            if (GetChannelState() != ChannelState.INACTIVE && GetChannelState() != ChannelState.CLOSED)
            {
                try
                {
                    if (Channel!.Socket != null && Channel.Socket.Connected)
                    {
                        bool canWrite = Channel.Socket.Poll(0, System.Net.Sockets.SelectMode.SelectWrite);
                        if (canWrite)
                        {
                            ret = Channel.Flush(out error);
                            if (ret < TransportReturnCode.SUCCESS)
                            {
                                Console.WriteLine($"Channel flush failed with return code: {ret}. Error: {error?.Text}");
                            }
                        }
                    }                   

                    ret = Channel.Close(out error);
                    if (ret != TransportReturnCode.SUCCESS)
                    {
                        return ret;
                    }
                    Channel = null;
                } 
                catch (Exception e)
                {
                    error = new Error()
                    {
                        Text = "Error while closing channel: " + e.ToString(),
                        ErrorId = TransportReturnCode.FAILURE
                    };
                    return TransportReturnCode.FAILURE;
                }
                
            }

            return TransportReturnCode.SUCCESS;
        }

    }
}

