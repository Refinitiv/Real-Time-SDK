/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using Refinitiv.Common.Interfaces;
using Refinitiv.Eta.Codec;
using Refinitiv.Eta.Example.Common;
using Refinitiv.Eta.Rdm;
using Refinitiv.Eta.Transports;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;
using Buffer = Refinitiv.Eta.Codec.Buffer;

namespace Refinitiv.Eta.Example.Common
{
    /// <summary>
    /// This is the symbol list handler for the ETA consumer application. 
    /// It provides methods for sending the symbol list request(s) 
    /// to a provider and processing the response(s). Method closing stream is also provided.
    /// </summary>
    public class SymbolListHandler
    {
        protected const int SYMBOL_LIST_STREAM_ID_START = 400;
        public const int TRANSPORT_BUFFER_SIZE_REQUEST = ChannelSession.MAX_MSG_SIZE;
        public const int TRANSPORT_BUFFER_SIZE_CLOSE = ChannelSession.MAX_MSG_SIZE;

        protected SymbolListRequest symbolListRequest = new SymbolListRequest();
        protected MarketPriceClose closeMessage = new MarketPriceClose();
        protected Buffer symbolListName = new Buffer();
        protected State state = new State();
        protected Qos qos = new Qos();
        public List<long> capabilities = new List<long>();
        
        protected Map map = new Map();
        protected MapEntry mapEntry = new MapEntry();
        private Buffer mapKey = new Buffer();
        protected EncodeIterator encIter = new EncodeIterator();

        public bool SnapshotRequested { get; set; }
        public int ServiceId { get; set; }
        public Buffer SymbolListName { get => symbolListName; set { Debug.Assert(value != null); symbolListName.Data(value.Data(), value.Position, value.Length); symbolListName.Data().Flip(); } }
        public Qos Qos { get => qos; set { Debug.Assert(value != null); value.Copy(qos); } }
        public List<long> Capabilities { get => capabilities; set { capabilities.Clear(); capabilities.AddRange(value); } }
        public State State { get => state; set { Debug.Assert(value != null); value.Copy(state); } }

        protected bool HasSymbolListCapability(List<long> capabilities)
        {
            foreach (long capability in capabilities)
            {
                if (capability == (int)DomainType.SYMBOL_LIST)
                    return true;
            }
            return false;
        }

        /// <summary>
        /// Processes SymbolList response.
        /// </summary>
        /// <param name="msg">Partially decoded message</param>
        /// <param name="dIter">The decode iterator</param>
        /// <param name="dictionary"><see cref="DataDictionary"/> that can be used to decode the message</param>
        /// <param name="error"><see cref="Error"/> instance that holds error information in case of failure</param>
        /// <returns><see cref="TransportReturnCode"/> value</returns>
        public TransportReturnCode ProcessResponse(Msg msg, DecodeIterator dIter, DataDictionary dictionary, out Error? error)
        {
            error = null;
            map.Clear();
            mapEntry.Clear();
            mapKey.Clear();
            switch (msg.MsgClass)
            {
                case MsgClasses.REFRESH:
                    IRefreshMsg refreshMsg = msg;
                    state.DataState(refreshMsg.State.DataState());
                    state.StreamState(refreshMsg.State.StreamState());
                    HandleMsg(msg, dIter, map, mapEntry, mapKey, true);
                    break;
                case MsgClasses.UPDATE:
                    return HandleMsg(msg, dIter, map, mapEntry, mapKey, false);
                case MsgClasses.STATUS:
                    IStatusMsg statusMsg = msg;
                    Console.WriteLine("Received Item StatusMsg for stream " + msg.StreamId);
                    if (statusMsg.CheckHasState())
                    {
                        state.DataState(statusMsg.State.DataState());
                        this.state.StreamState(statusMsg.State.StreamState());
                        Console.WriteLine("    " + state);
                    }
                    break;
                case MsgClasses.ACK:
                    HandleAck(msg);
                    return TransportReturnCode.SUCCESS;
                default:
                    Console.WriteLine("Received Unhandled Item Msg Class: " + msg.MsgClass);
                    break;
            }

            return TransportReturnCode.SUCCESS;
        }
        
        protected TransportReturnCode HandleMsg(Msg msg, DecodeIterator dIter, Map map, MapEntry mapEntry, Buffer mapKey, bool isRefresh)
        {
            // print the name of the symbolist and the domain
            Console.WriteLine("ITEM NAME: " + SymbolListName + "\nDOMAIN: " + DomainTypes.ToString(msg.DomainType));

            if (isRefresh)
            {
                IRefreshMsg refreshMsg = msg;
                Console.WriteLine(refreshMsg.State.ToString());
            }

            CodecReturnCode ret = map.Decode(dIter);
            if (ret != CodecReturnCode.SUCCESS)
            {
                Console.WriteLine("DecodeMap() failed: < " + ret + ">");
                return TransportReturnCode.FAILURE;
            }

            while ((ret = mapEntry.Decode(dIter, mapKey)) != CodecReturnCode.END_OF_CONTAINER)
            {
                if (ret != CodecReturnCode.SUCCESS)
                {
                    Console.WriteLine("DecodeMapEntry() failed: < " + ret + ">");
                    return TransportReturnCode.FAILURE;
                }
                Console.WriteLine(mapKey.ToString() + "\t" + MapEntryActionToString(mapEntry.Action));
            }

            return TransportReturnCode.SUCCESS;
        }

        protected TransportReturnCode HandleAck(Msg msg)
        {
            // although this application only posts on Market Price, ACK handler is
            // provided for other domains to allow user to extend and post on MBO and MBP domains
            Console.WriteLine("Received AckMsg for stream " + msg.StreamId);
            IMsgKey key = msg.MsgKey;

            //print out item name from key if it has it
            if (key != null && key.CheckHasName())
            {
                Console.WriteLine(key.Name + "\nDOMAIN: " + DomainTypes.ToString(msg.DomainType));
            }

            IAckMsg ackMsg = msg;
            Console.WriteLine("\tackId=" + ackMsg.AckId + 
                (ackMsg.CheckHasSeqNum() ? "\tseqNum=" + ackMsg.SeqNum : "") + 
                (ackMsg.CheckHasNakCode() ? "\tnakCode=" + ackMsg.NakCode : "") + 
                (ackMsg.CheckHasText() ? "\ttext=" + ackMsg.Text.ToString() : ""));

            return TransportReturnCode.SUCCESS;
        }

        private string MapEntryActionToString(MapEntryActions mapEntryAction)
        {
            switch (mapEntryAction)
            {
                case MapEntryActions.UPDATE:
                    return "UPDATE";
                case MapEntryActions.ADD:
                    return "ADD";
                case MapEntryActions.DELETE:
                    return "DELETE";
                default:
                    return "Unknown Map Entry Action";
            }
        }

        /// <summary>
        /// Sends Symbol List request
        /// </summary>
        /// <param name="chnl">The channel to send request to</param>
        /// <param name="error"><see cref="Error"/> instance that holds error information in case of failure</param>
        /// <returns><see cref="TransportReturnCode"/> value</returns>
        public TransportReturnCode SendRequest(ChannelSession chnl, out Error? error)
        {
            // check to see if the provider supports the symbol list domain
            if (!HasSymbolListCapability(Capabilities))
            {
                error = new Error()
                {
                    Text = "SYMBOL_LIST domain is not supported by the indicated provider"
                };
                return TransportReturnCode.FAILURE;
            }

            // get a buffer for the item request 
            ITransportBuffer? msgBuf = chnl.GetTransportBuffer(TRANSPORT_BUFFER_SIZE_REQUEST, false, out error);
            if (msgBuf == null)
            {
                return TransportReturnCode.FAILURE;
            }

            // initialize state management array 
            // these will be updated as refresh and status messages are received
            state.DataState(DataStates.NO_CHANGE);
            state.StreamState(StreamStates.UNSPECIFIED);

            // encode symbol list request
            symbolListRequest.Clear();

            if (!SnapshotRequested)
                symbolListRequest.Streaming = true;
            symbolListRequest.SymbolListName.Data(SymbolListName.Data(), symbolListName.Position, symbolListName.Length);
            symbolListRequest.StreamId = SYMBOL_LIST_STREAM_ID_START;
            symbolListRequest.ServiceId = ServiceId;
            symbolListRequest.HasServiceId = true;
            symbolListRequest.HasQos = true;
            Qos.Copy(symbolListRequest.Qos);
            symbolListRequest.PriorityClass = 1;
            symbolListRequest.PriorityCount = 1;

            encIter.Clear();
            encIter.SetBufferAndRWFVersion(msgBuf, chnl.Channel!.MajorVersion, chnl.Channel.MinorVersion);

            Console.WriteLine(symbolListRequest.ToString());
            if (symbolListRequest.Encode(encIter) != CodecReturnCode.SUCCESS)
            {
                return TransportReturnCode.FAILURE;
            }

            return chnl.Write(msgBuf, out error);
        }

        /// <summary>
        /// Closes the symbol list stream.
        /// </summary>
        /// <param name="chnl">The channel to send a symbol list close to</param>
        /// <param name="error"><see cref="Error"/> instance that holds error information in case of failure</param>
        /// <returns><see cref="TransportReturnCode"/> value</returns>
        public TransportReturnCode CloseStream(ChannelSession chnl, out Error? error)
        {
            error = null;
            // we only want to close a stream if it was not already closed (e.g.
            // rejected by provider, closed via refresh or status, or redirected)
            if (state.StreamState() != StreamStates.OPEN && state.DataState() != DataStates.OK)
                return TransportReturnCode.SUCCESS;

            //get a buffer for the item close
            ITransportBuffer? msgBuf = chnl.GetTransportBuffer(TRANSPORT_BUFFER_SIZE_CLOSE, false, out error);

            if (msgBuf == null)
            {
                return TransportReturnCode.FAILURE;
            }

            //encode item close
            closeMessage.Clear();
            closeMessage.StreamId = SYMBOL_LIST_STREAM_ID_START;
            encIter.Clear();
            encIter.SetBufferAndRWFVersion(msgBuf, chnl.Channel!.MajorVersion, chnl.Channel.MinorVersion);

            CodecReturnCode ret = closeMessage.Encode(encIter);
            if (ret != CodecReturnCode.SUCCESS)
            {
                error = new Error()
                {
                    Text = "encodeSymbolListClose(): Failed <" + ret + ">"
                };
                return TransportReturnCode.FAILURE;
            }
            return chnl.Write(msgBuf, out error);
        }

    }
}
