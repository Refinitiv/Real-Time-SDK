/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Common;
using LSEG.Eta.Codec;
using LSEG.Eta.Transports;
using System.Collections.Generic;

namespace LSEG.Eta.Example.Common
{
    public class MarketPriceItems
    {
        //APIQA
        private static int MAX_MARKET_PRICE_ITEM_LIST_SIZE = 10000;
        // END APIQA

        // item information list
        public List<MarketPriceItem> MarketPriceList { get; private set; } = new List<MarketPriceItem>(MAX_MARKET_PRICE_ITEM_LIST_SIZE);

        private Real m_TempReal = new Real();
        private Enum m_TempEnum = new Enum();
        private UInt m_TempUInt = new UInt();
        private FieldList m_TempFieldList = new FieldList();
        private FieldEntry m_TempFieldEntry = new FieldEntry();

        protected MarketPriceRefresh m_MarketPriceRefresh = new MarketPriceRefresh();
        protected MarketPriceUpdate m_MarketPriceUpdate = new MarketPriceUpdate();
        protected EncodeIterator m_EncodeIter = new EncodeIterator();

        /// <summary>
        /// Instantiates a new market price items.
        /// </summary>
        public MarketPriceItems()
        {
            for (int i = 0; i < MAX_MARKET_PRICE_ITEM_LIST_SIZE; i++)
            {
                MarketPriceList.Add(new MarketPriceItem());
            }

        }

        /// <summary>
        /// Initializes market price item list.
        /// </summary>
        public void Init()
        {
            foreach (MarketPriceItem mpItem in MarketPriceList)
            {
                mpItem.Clear();
            }
        }

        /// <summary>
        /// Updates any item that's currently in use.
        /// </summary>
        public void Update()
        {
            foreach (MarketPriceItem mpItem in MarketPriceList)
            {
                if (mpItem.IsInUse)
                    mpItem.UpdateFields();
            }
        }

        /// <summary>
        /// Gets storage for a market price item from the list.
        /// </summary>
        /// <param name="itemName">the item name</param>
        /// <returns>the market price item</returns>
        public MarketPriceItem? Get(string itemName)
        {
            // first try to find one with same name and reuse
            foreach (MarketPriceItem mpItem in MarketPriceList)
            {
                if (mpItem.IsInUse && mpItem.itemName != null && mpItem.itemName.Equals(itemName))
                {
                    return mpItem;
                }
            }

            // next get a new one
            foreach (MarketPriceItem mpItem in MarketPriceList)
            {
                if (!mpItem.IsInUse)
                {
                    mpItem.InitFields();
                    mpItem.itemName = itemName;
                    return mpItem;
                }
            }

            return null;
        }

        /// <summary>
        /// Clears the item information.
        /// </summary>
        public void Clear()
        {
            foreach (MarketPriceItem mpItem in MarketPriceList)
            {
                mpItem.IsInUse = false;
                mpItem.RDNDISPLAY = 0;
                mpItem.RDN_EXCHID = 0;
                mpItem.DIVPAYDATE.Clear();
                mpItem.TRDPRC_1 = 0;
                mpItem.BID = 0;
                mpItem.ASK = 0;
                mpItem.ACVOL_1 = 0;
                mpItem.NETCHNG_1 = 0;
            }
        }

        /// <summary>
        /// Updates the item's data from the post we got.
        /// </summary>
        /// <param name="mpItem">the mp item</param>
        /// <param name="dIter">the decode iterator</param>
        /// <param name="error">the error in an event of failure</param>
        /// <returns><see cref="CodecReturnCode"/></returns>
        public CodecReturnCode UpdateFieldsFromPost(MarketPriceItem mpItem, DecodeIterator dIter, out Error? error)
        {
            // decode field list
            error = null;
            CodecReturnCode ret = m_TempFieldList.Decode(dIter, null);
            if (ret != CodecReturnCode.SUCCESS)
            {
                error = new Error
                {
                    ErrorId = TransportReturnCode.FAILURE,
                    Text = $"FieldList.decode() failed with return code: {ret.GetAsString()}"
                };

                return ret;
            }

            // decode each field entry in list and apply it to the market price item
            while ((ret = m_TempFieldEntry.Decode(dIter)) != CodecReturnCode.END_OF_CONTAINER)
            {
                if (ret != CodecReturnCode.SUCCESS)
                {
                    error = new Error
                    {
                        ErrorId = TransportReturnCode.FAILURE,
                        Text = $"FieldEntry.decode() failed with return code: {ret.GetAsString()}"
                    };

                    return ret;
                }
                switch (m_TempFieldEntry.FieldId)
                {
                    case MarketPriceItem.RDNDISPLAY_FID:
                        ret = m_TempUInt.Decode(dIter);
                        if (ret != CodecReturnCode.SUCCESS)
                        {
                            error = new Error
                            {
                                ErrorId = TransportReturnCode.FAILURE,
                                Text = $"Error= {ret.GetAsString()} Decoding RDNDISPLAY"
                            };

                            return ret;
                        }
                        mpItem.RDNDISPLAY = (int)m_TempUInt.ToLong();
                        break;

                    case MarketPriceItem.RDN_EXCHID_FID:
                        ret = m_TempEnum.Decode(dIter);
                        if (ret != CodecReturnCode.SUCCESS)
                        {
                            error = new Error
                            {
                                ErrorId = TransportReturnCode.FAILURE,
                                Text = $"Error= {ret.GetAsString()} Decoding RDN_EXCHID_FID"
                            };

                            return ret;
                        }

                        mpItem.RDN_EXCHID = m_TempEnum.ToInt();
                        break;

                    case MarketPriceItem.DIVPAYDATE_FID:
                        ret = mpItem.DIVPAYDATE.Decode(dIter);
                        if (ret != CodecReturnCode.SUCCESS)
                        {
                            error = new Error
                            {
                                ErrorId = TransportReturnCode.FAILURE,
                                Text = $"Error= {ret.GetAsString()} Decoding DIVPAYDATE_FID"
                            };

                            return ret;
                        }
                        break;

                    case MarketPriceItem.TRDPRC_1_FID:
                        ret = m_TempReal.Decode(dIter);
                        if (ret != CodecReturnCode.SUCCESS)
                        {
                            error = new Error
                            {
                                ErrorId = TransportReturnCode.FAILURE,
                                Text = $"Error= {ret.GetAsString()} Decoding TRDPRC_1_FID"
                            };

                            return ret;
                        }
                        mpItem.TRDPRC_1 = m_TempReal.ToDouble();
                        break;

                    case MarketPriceItem.BID_FID:
                        ret = m_TempReal.Decode(dIter);
                        if (ret != CodecReturnCode.SUCCESS)
                        {
                            error = new Error
                            {
                                ErrorId = TransportReturnCode.FAILURE,
                                Text = $"Error= {ret.GetAsString()} Decoding BID_FID"
                            };

                            return ret;
                        }
                        mpItem.BID = m_TempReal.ToDouble();
                        break;
                    case MarketPriceItem.ASK_FID:
                        ret = m_TempReal.Decode(dIter);
                        if (ret != CodecReturnCode.SUCCESS)
                        {
                            error = new Error
                            {
                                ErrorId = TransportReturnCode.FAILURE,
                                Text = $"Error= {ret.GetAsString()} Decoding ASK_FID"
                            };
                            return ret;
                        }
                        mpItem.ASK = m_TempReal.ToDouble();
                        break;

                    case MarketPriceItem.ACVOL_1_FID:
                        ret = m_TempReal.Decode(dIter);
                        if (ret != CodecReturnCode.SUCCESS)
                        {
                            error = new Error
                            {
                                ErrorId = TransportReturnCode.FAILURE,
                                Text = $"Error= {ret.GetAsString()} Decoding ACVOL_1"
                            };

                            return ret;
                        }
                        mpItem.ACVOL_1 = m_TempReal.ToDouble();
                        break;

                    case MarketPriceItem.NETCHNG_1_FID:
                        ret = m_TempReal.Decode(dIter);
                        if (ret != CodecReturnCode.SUCCESS)
                        {
                            error = new Error
                            {
                                ErrorId = TransportReturnCode.FAILURE,
                                Text = $"Error= {ret.GetAsString()} Decoding NETCHNG_1"
                            };

                            return ret;
                        }
                        mpItem.NETCHNG_1 = m_TempReal.ToDouble();
                        break;

                    case MarketPriceItem.ASK_TIME_FID:
                        ret = mpItem.ASK_TIME.Time().Decode(dIter);
                        if (ret != CodecReturnCode.SUCCESS)
                        {
                            error = new Error
                            {
                                ErrorId = TransportReturnCode.FAILURE,
                                Text = $"Error= {ret.GetAsString()} Decoding ASK_TIME"
                            };

                            return ret;
                        }
                        break;
                    case MarketPriceItem.PERATIO_FID:
                        ret = m_TempReal.Decode(dIter);
                        if (ret != CodecReturnCode.SUCCESS)
                        {
                            error = new Error
                            {
                                ErrorId = TransportReturnCode.FAILURE,
                                Text = $"Error= {ret.GetAsString()} Decoding PERATIO_FID"
                            };

                            return ret;
                        }
                        mpItem.PERATIO = m_TempReal.ToDouble();

                        break;
                    case MarketPriceItem.SALTIME_FID:
                        ret = mpItem.SALTIME.Time().Decode(dIter);
                        if (ret != CodecReturnCode.SUCCESS)
                        {
                            error = new Error
                            {
                                ErrorId = TransportReturnCode.FAILURE,
                                Text = $"Error= {ret.GetAsString()} Decoding PERATIO_FID"
                            };

                            return ret;
                        }
                        break;

                    default:

                        error = new Error
                        {
                            ErrorId = TransportReturnCode.FAILURE,
                            Text = $"Unknown field ID ={m_TempFieldEntry.FieldId} in post"
                        };

                        return CodecReturnCode.FAILURE;
                }
            }
            return CodecReturnCode.SUCCESS;
        }

        /// <summary>
        /// Encodes the market price response.
        /// </summary>
        /// <param name="channel">The channel to send a market price response to</param>
        /// <param name="itemInfo">The item information</param>
        /// <param name="msgBuf">The message buffer to encode the market price response into</param>
        /// <param name="isSolicited">The response is solicited if set</param>
        /// <param name="streamId">The stream id of the market price response</param>
        /// <param name="isStreaming">Flag for streaming or snapshot</param>
        /// <param name="isPrivateStream">Flag for private stream</param>
        /// <param name="serviceId">The service id of the market price response</param>
        /// <param name="dictionary">The dictionary used for encoding</param>
        /// <param name="error">error in case of encoding failure</param>
        /// <returns><see cref="CodecReturnCode"/></returns>
        public CodecReturnCode EncodeResponse(IChannel channel, ItemInfo itemInfo, ITransportBuffer msgBuf, bool isSolicited, int streamId,
            bool isStreaming, bool isPrivateStream, int serviceId, DataDictionary dictionary, out Error? error)
        {
            error = null;
            MarketPriceItem mpItem = (MarketPriceItem)itemInfo.ItemData!;

            // set message depending on whether refresh or update
            if (itemInfo.IsRefreshRequired)
            {
                m_MarketPriceRefresh.Clear();
                m_MarketPriceRefresh.DataDictionary = dictionary;
                if (isStreaming)
                {
                    m_MarketPriceRefresh.State.StreamState(StreamStates.OPEN);
                }
                else
                {
                    m_MarketPriceRefresh.State.StreamState(StreamStates.NON_STREAMING);
                }
                m_MarketPriceRefresh.State.DataState(DataStates.OK);
                m_MarketPriceRefresh.State.Code(StateCodes.NONE);
                m_MarketPriceRefresh.State.Text().Data("Item Refresh Completed");
                m_MarketPriceRefresh.RefreshComplete = true;
                m_MarketPriceRefresh.HasQos = true;

                if (isSolicited)
                {
                    m_MarketPriceRefresh.Solicited = true;

                    // clear cache for solicited refresh messages.
                    m_MarketPriceRefresh.ClearCache = true;
                }

                if (isPrivateStream)
                {
                    m_MarketPriceRefresh.PrivateStream = true;
                }

                // Service Id
                m_MarketPriceRefresh.HasServiceId = true;
                m_MarketPriceRefresh.ServiceId = serviceId;

                // ItemName
                m_MarketPriceRefresh.ItemName.Data(itemInfo.ItemName.Data(), itemInfo.ItemName.Position, itemInfo.ItemName.Length);

                // Qos
                m_MarketPriceRefresh.Qos.IsDynamic = false;
                m_MarketPriceRefresh.Qos.Rate(QosRates.TICK_BY_TICK);
                m_MarketPriceRefresh.Qos.Timeliness(QosTimeliness.REALTIME);

                // StreamId
                m_MarketPriceRefresh.StreamId = streamId;
                m_MarketPriceRefresh.MarketPriceItem = mpItem;

                // encode
                m_EncodeIter.Clear();
                CodecReturnCode ret = m_EncodeIter.SetBufferAndRWFVersion(msgBuf, channel.MajorVersion, channel.MinorVersion);
                if (ret != CodecReturnCode.SUCCESS)
                {
                    error = new Error
                    {
                        ErrorId = TransportReturnCode.FAILURE,
                        Text = $"EncodeIterator.setBufferAndRWFVersion() failed with return code: {ret.GetAsString()}"
                    };

                    return ret;
                }
                ret = m_MarketPriceRefresh.Encode(m_EncodeIter);
                if (ret != CodecReturnCode.SUCCESS)
                {
                    error = new Error
                    {
                        ErrorId = TransportReturnCode.FAILURE,
                        Text = $"MarketPriceRefresh.Encode() failed"
                    };
                }

                return ret;
            }
            else
            {
                m_MarketPriceUpdate.Clear();
                m_MarketPriceUpdate.DataDictionary = dictionary;

                // this is an update message
                m_MarketPriceUpdate.StreamId = streamId;

                // include msg key in updates for non-interactive provider streams
                if (streamId < 0)
                {
                    m_MarketPriceUpdate.HasServiceId = true;

                    // ServiceId
                    m_MarketPriceUpdate.ServiceId = serviceId;

                    // Itemname
                    m_MarketPriceUpdate.ItemName.Data(itemInfo.ItemName.Data(), itemInfo.ItemName.Position, itemInfo.ItemName.Length);
                }

                if (isPrivateStream)
                {
                    m_MarketPriceUpdate.PrivateStream = true;
                }

                m_MarketPriceUpdate.MarketPriceItem = mpItem;

                // encode
                m_EncodeIter.Clear();
                CodecReturnCode ret = m_EncodeIter.SetBufferAndRWFVersion(msgBuf, channel.MajorVersion, channel.MinorVersion);
                if (ret != CodecReturnCode.SUCCESS)
                {
                    error = new Error
                    {
                        ErrorId = TransportReturnCode.FAILURE,
                        Text = $"EncodeIterator.SetBufferAndRWFVersion() failed with return code: {ret.GetAsString()}"
                    };

                    return ret;
                }
                ret = m_MarketPriceUpdate.Encode(m_EncodeIter);
                if (ret != CodecReturnCode.SUCCESS)
                {
                    error = new Error
                    {
                        ErrorId = TransportReturnCode.FAILURE,
                        Text = $"MarketPriceUpdate.Encode() failed"
                    };
                }

                return ret;
            }
        }
    }
}
