/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Common;
using LSEG.Eta.Codec;
using LSEG.Eta.Example.Common;
using LSEG.Eta.Rdm;
using LSEG.Eta.ValueAdd.Reactor;

namespace LSEG.Eta.ValueAdd.Consumer
{

    /// <summary>
    /// This is the symbol list handler for the ETA Value Add consumer application.
    /// </summary>
    ///
    /// It provides methods for sending the symbol list request(s) to a provider
    /// and processing the response(s). Method closing stream is also provided.
    ///
    internal class SymbolListHandler
    {
        public SymbolListRequest SymbolListRequest { get; set; } = new();
        public MarketPriceClose CloseMessage { get; set; } = new();
        public Codec.Buffer SymbolListName { get; set; } = new();
        public State State { get; set; } = new();
        public Qos Qos { get; set; } = new();
        public List<long> Capabilities { get; set; } = new List<long>();
        public int ServiceId { get; set; }
        public bool SnapshotRequest { get; set; }

        private const int SYMBOL_LIST_STREAM_ID_START = 400;
        private const int TRANSPORT_BUFFER_SIZE_REQUEST = 1000;
        private const int TRANSPORT_BUFFER_SIZE_CLOSE = 1000;

        private Map m_Map = new();
        private MapEntry m_MapEntry = new();
        private Codec.Buffer m_MapKey = new();
        private EncodeIterator m_EncodeIterator = new();
        private ReactorSubmitOptions m_SubmitOptions = new();


        public SymbolListHandler()
        {
        }

        private bool HasSymbolListCapability(List<long> capabilities)
        {
            return capabilities.Any((capability) => capability == (int)DomainType.SYMBOL_LIST);
        }

        public ReactorReturnCode SendRequest(ReactorChannel chnl, out ReactorErrorInfo? errorInfo)
        {
            /* check to see if the provider supports the symbol list domain */
            if (!HasSymbolListCapability(Capabilities))
            {
                errorInfo = new();
                errorInfo.Error.Text = "SYMBOL_LIST domain is not supported by the indicated provider";
                return ReactorReturnCode.FAILURE;
            }

            /* get a buffer for the item request */
            ITransportBuffer? msgBuf = chnl.GetBuffer(TRANSPORT_BUFFER_SIZE_REQUEST, false, out errorInfo);
            if (msgBuf == null)
            {
                return ReactorReturnCode.FAILURE;
            }

            /* initialize state management array */
            /* these will be updated as refresh and status messages are received */
            State.DataState(DataStates.NO_CHANGE);
            State.StreamState(StreamStates.UNSPECIFIED);

            /* encode symbol list request */
            SymbolListRequest.Clear();

            if (!SnapshotRequest)
                SymbolListRequest.Streaming = true;

            SymbolListRequest.SymbolListName.Data(SymbolListName.Data(), SymbolListName.Position, SymbolListName.Length);
            SymbolListRequest.StreamId = SYMBOL_LIST_STREAM_ID_START;
            SymbolListRequest.ServiceId = ServiceId;
            SymbolListRequest.HasServiceId = true;
            SymbolListRequest.Qos.IsDynamic = Qos.IsDynamic;
            SymbolListRequest.Qos.Rate(Qos.Rate());
            SymbolListRequest.Qos.Timeliness(Qos.Timeliness());
            SymbolListRequest.HasQos = true;
            SymbolListRequest.PriorityClass = 1;
            SymbolListRequest.PriorityCount = 1;

            m_EncodeIterator.Clear();
            m_EncodeIterator.SetBufferAndRWFVersion(msgBuf, chnl.MajorVersion, chnl.MinorVersion);

            Console.WriteLine(SymbolListRequest.ToString());
            CodecReturnCode ret = SymbolListRequest.Encode(m_EncodeIterator);
            if (ret != CodecReturnCode.SUCCESS)
            {
                return ReactorReturnCode.FAILURE;
            }

            return chnl.Submit(msgBuf, m_SubmitOptions, out errorInfo);
        }

        public ReactorReturnCode ProcessResponse(Msg msg, DecodeIterator dIter, DataDictionary dictionary, out ReactorErrorInfo? errorInfo)
        {
            m_Map.Clear();
            m_MapEntry.Clear();
            m_MapKey.Clear();
            switch (msg.MsgClass)
            {
                case MsgClasses.REFRESH:
                    IRefreshMsg refreshMsg = (IRefreshMsg)msg;
                    State.DataState(refreshMsg.State.DataState());
                    State.StreamState(refreshMsg.State.StreamState());
                    return HandleUpdate(msg, dIter, m_Map, m_MapEntry, m_MapKey, out errorInfo);

                case MsgClasses.UPDATE:
                    return HandleUpdate(msg, dIter, m_Map, m_MapEntry, m_MapKey, out errorInfo);

                case MsgClasses.STATUS:
                    IStatusMsg statusMsg = (IStatusMsg)msg;
                    Console.WriteLine("Received Item StatusMsg for stream " + msg.StreamId);
                    if (statusMsg.CheckHasState())
                    {
                        State.DataState(statusMsg.State.DataState());
                        State.StreamState(statusMsg.State.StreamState());
                        State.Text(statusMsg.State.Text());
                        Console.WriteLine("    " + State);
                    }
                    errorInfo = null;
                    return ReactorReturnCode.SUCCESS;
                case MsgClasses.ACK:
                    HandleAck(msg);
                    errorInfo = null;
                    return ReactorReturnCode.SUCCESS;
                default:
                    Console.WriteLine("Received Unhandled Item Msg Class: " + msg.MsgClass);
                    break;
            }

            errorInfo = null;
            return ReactorReturnCode.SUCCESS;
        }

        protected ReactorReturnCode HandleUpdate(Msg msg, DecodeIterator dIter, Map map,
                MapEntry mapEntry, Codec.Buffer mapKey, out ReactorErrorInfo? errorInfo)
        {
            // print the name of the symbolist and the domain
            Console.WriteLine(SymbolListName.ToString() +
                    "\nDOMAIN: " + DomainTypes.ToString(msg.DomainType));

            CodecReturnCode ret = map.Decode(dIter);
            if (ret != CodecReturnCode.SUCCESS)
            {
                errorInfo = new();
                errorInfo.Error.Text = "SymbolListHandler failed to decode update";
                Console.WriteLine($"DecodeMap() failed: <{ret}>");
                return ReactorReturnCode.FAILURE;
            }

            // decode the map
            while ((ret = mapEntry.Decode(dIter, mapKey)) != CodecReturnCode.END_OF_CONTAINER)
            {
                if (ret != CodecReturnCode.SUCCESS)
                {
                    errorInfo = new();
                    errorInfo.Error.Text = "SymbolListHandler failed to decode update";
                    Console.WriteLine($"DecodeMapEntry() failed: <{ret}>");
                    return ReactorReturnCode.FAILURE;
                }
                Console.WriteLine(mapKey.ToString() + "\t" +
                            MapEntryActionToString(mapEntry.Action));
            }

            errorInfo = null;
            return ReactorReturnCode.SUCCESS;
        }

        protected ReactorReturnCode HandleAck(Msg msg)
        {
            /*
             * although this application only posts on Market Price, ACK handler is
             * provided for other domains to allow user to extend and post on MBO
             * and MBP domains
             */
            Console.WriteLine("Received AckMsg for stream " + msg.StreamId);
            IMsgKey key = msg.MsgKey;

            //print out item name from key if it has it
            if (key != null && key.CheckHasName())
            {
                Console.WriteLine(key.Name + "\nDOMAIN: " +
                            DomainTypes.ToString(msg.DomainType));
            }

            IAckMsg ackMsg = (IAckMsg)msg;
            Console.WriteLine("\tackId=" + ackMsg.AckId
                + (ackMsg.CheckHasSeqNum() ? "\tseqNum=" + ackMsg.SeqNum : "")
                + (ackMsg.CheckHasNakCode() ? "\tnakCode=" + ackMsg.NakCode : "")
                + (ackMsg.CheckHasText() ? "\ttext=" + ackMsg.Text.ToString() : ""));
            return ReactorReturnCode.SUCCESS;
        }


        /// <summary>
        /// Close the symbol list stream.
        /// </summary>
        public ReactorReturnCode CloseStream(ReactorChannel chnl, out ReactorErrorInfo? errorInfo)
        {
            /*
             * we only want to close a stream if it was not already closed (e.g.
             * rejected by provider, closed via refresh or status, or redirected)
             */
            if (State.StreamState() != StreamStates.OPEN
                && State.DataState() != DataStates.OK)
            {
                errorInfo = null;
                return ReactorReturnCode.SUCCESS;
            }

            //get a buffer for the item close
            ITransportBuffer? msgBuf = chnl.GetBuffer(TRANSPORT_BUFFER_SIZE_CLOSE, false, out errorInfo);

            if (msgBuf == null)
            {
                return ReactorReturnCode.FAILURE;
            }

            //encode item close
            CloseMessage.Clear();

            CloseMessage.StreamId = SYMBOL_LIST_STREAM_ID_START;
            CloseMessage.DomainType = (int)DomainType.SYMBOL_LIST;
            m_EncodeIterator.Clear();
            m_EncodeIterator.SetBufferAndRWFVersion(msgBuf, chnl.MajorVersion, chnl.MinorVersion);

            CodecReturnCode ret = CloseMessage.Encode(m_EncodeIterator);
            if (ret != CodecReturnCode.SUCCESS)
            {
                if(errorInfo is null)
                {
                    errorInfo = new ReactorErrorInfo();
                }
                errorInfo.Error.Text = $"EncodeSymbolListClose(): Failed <{ret}>";
                return ReactorReturnCode.FAILURE;
            }
            return chnl.Submit(msgBuf, m_SubmitOptions, out errorInfo);
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
    }
}
