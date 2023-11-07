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
using LSEG.Eta.ValueAdd.Rdm;
using System;
using System.Collections.Generic;

namespace LSEG.Eta.Example.Common
{
    /// <summary>
    /// Base class for NiProvider Item domain handlers.
    /// </summary>
    public abstract class MarketHandlerBase
    {
        public static int TRANSPORT_BUFFER_SIZE_MESSAGE = ChannelSession.MAX_MSG_SIZE;
        public static int TRANSPORT_BUFFER_SIZE_CLOSE = ChannelSession.MAX_MSG_SIZE;

        private int domainType = (int)Rdm.DomainType.MARKET_PRICE;
        protected MarketPriceClose closeMessage = new MarketPriceClose();        
        protected NiStreamIdWatchList? watchList;

        protected FieldList fieldList = new FieldList();
        protected FieldEntry fieldEntry = new FieldEntry();
        protected EncodeIterator encIter = new EncodeIterator();
       
        protected TransportReturnCode CloseStream(ChannelSession chnl, int streamId, out Error? error)
        {
            ITransportBuffer? msgBuf = chnl.GetTransportBuffer(TRANSPORT_BUFFER_SIZE_CLOSE, false, out error);
            if (msgBuf == null)
                return TransportReturnCode.FAILURE;

            closeMessage.Clear();
            closeMessage.StreamId = streamId;
            closeMessage.DomainType = domainType;
            encIter.Clear();
            encIter.SetBufferAndRWFVersion(msgBuf, chnl.Channel!.MajorVersion, chnl.Channel.MinorVersion);

            CodecReturnCode ret = closeMessage.Encode(encIter);
            if (ret != CodecReturnCode.SUCCESS)
            {
                error = new Error()
                {
                    Text = $"Encode MarketPriceClose Failed: {ret.GetAsString()}",
                    ErrorId = TransportReturnCode.FAILURE
                };
                return TransportReturnCode.FAILURE;
            }
            return chnl.Write(msgBuf, out error);
        }

        /// <summary>
        /// Close all item streams.
        /// </summary>
        /// <param name="chnl">The channel to send a item stream close to.</param>
        /// <param name="error">Error instance that carries information about failure in case of unsuccessful operation.</param>
        /// <returns><see cref="TransportReturnCode"/> instance</returns>
        public TransportReturnCode CloseStreams(ChannelSession chnl, out Error? error)
        {
            var enumerator = watchList!.GetEnumerator();
            List<int> itemsToRemove = new List<int>();
            while (enumerator.MoveNext())
            {
                KeyValuePair<int, NiWatchListEntry> entry = (KeyValuePair<int, NiWatchListEntry>)enumerator.Current;

                if (entry.Value.Type != domainType)
                {
                    //this entry is from a different domainType, skip
                    continue;
                }

                TransportReturnCode ret;
                if ((ret = CloseStream(chnl, entry.Key, out error)) != TransportReturnCode.SUCCESS)
                {
                    Console.WriteLine($"Failed to close stream {entry.Key}, code: {ret}, error: {error?.Text}");
                }
                itemsToRemove.Add(entry.Key);
            }
            itemsToRemove.ForEach(item => watchList.Remove(item));

            error = null;
            return TransportReturnCode.SUCCESS;
        }

        /// <summary>
        /// Encodes and sends item refreshes for MarketPrice domain.
        /// </summary>
        /// <param name="chnl">The channel to send a refresh to.</param>
        /// <param name="itemNames">List of item names.</param>
        /// <param name="serviceInfo">RDM directory response information.</param>
        /// <param name="error">Populated if an error occurs.</param>
        /// <returns>success if item refreshes can be made, can be encoded and sent successfully.Failure if encoding/sending refresh failed.</returns>
        public TransportReturnCode SendItemRefreshes(ChannelSession chnl, List<string> itemNames, Service serviceInfo, out Error? error)
        {
            if (itemNames == null || itemNames.Count == 0)
            {
                error = null;
                return TransportReturnCode.SUCCESS;
            }
            GenerateRefreshAndUpdate(serviceInfo);

            return SendRefreshes(chnl, itemNames, out error);
        }

        protected TransportReturnCode EncodeAndSendContent(ChannelSession chnl, MsgBase content, out Error? error)
        {
            ITransportBuffer? msgBuf = chnl.GetTransportBuffer(TRANSPORT_BUFFER_SIZE_MESSAGE, false, out error);
            if (msgBuf == null)
            {
                return TransportReturnCode.FAILURE;
            }

            encIter.Clear();
            encIter.SetBufferAndRWFVersion(msgBuf, chnl.Channel!.MajorVersion, chnl.Channel.MinorVersion);

            CodecReturnCode ret = content.Encode(encIter);
            if (ret < CodecReturnCode.SUCCESS)
            {
                error = new Error()
                {
                    Text = "MarketPriceResponse Encode failed: " + ret
                };
                return TransportReturnCode.FAILURE;
            }

            return chnl.Write(msgBuf, out error);
        }

        protected abstract void GenerateRefreshAndUpdate(Service serviceInfo);

        protected abstract TransportReturnCode SendRefreshes(ChannelSession chnl, List<string> itemNames, out Error? error);
    }
}
