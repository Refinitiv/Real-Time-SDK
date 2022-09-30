/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using Refinitiv.Common.Interfaces;
using Refinitiv.Eta.Codec;
using Refinitiv.Eta.Common;
using Refinitiv.Eta.Transports;
using Refinitiv.Eta.Transports.Interfaces;
using System.Collections.Generic;
using Buffer = Refinitiv.Eta.Codec.Buffer;

namespace Refinitiv.Eta.Example.Common
{
    /// <summary>
    /// Storage class for managing symbol list items.
    /// <remark>
    /// This handles the generation of example symbol lists, and shows how to encode it.
    /// Provides methods for managing the list, and a method for encoding a symbol list message and payload.
    /// </remark>
    /// </summary>
    public class SymbolListItems
    {
        public const int SYMBOL_LIST_REFRESH = 0;
        public const int SYMBOL_LIST_UPDATE_ADD = 1;
        public const int SYMBOL_LIST_UPDATE_DELETE = 2;

        public static readonly int MAX_SYMBOL_LIST_SIZE = 100;

        private List<SymbolListItem> m_SymbolListItemList = new List<SymbolListItem>(MAX_SYMBOL_LIST_SIZE);
        private int m_ItemCount = 0;

        private EncodeIterator m_EncodeIter = new EncodeIterator();
        private IRefreshMsg m_RefreshMsg = new Msg();
        private IUpdateMsg m_UpdateMsg = new Msg();
        private Map m_TempMap = new Map();
        private MapEntry m_TempMapEntry = new MapEntry();
        private Buffer m_TempBuffer = new Buffer();

        /// <summary>
        /// Instantiates a new symbol list items.
        /// </summary>
        public SymbolListItems()
        {
            for (int i = 0; i < MAX_SYMBOL_LIST_SIZE; i++)
            {
                m_SymbolListItemList.Add(new SymbolListItem());
            }

        }

        /// <summary>
        /// Clears out a symbol list item field.
        /// </summary>
        public void Clear()
        {
            foreach (SymbolListItem symbolListItem in m_SymbolListItemList)
            {
                symbolListItem.Clear();
            }
        }

        /// <summary>
        /// Returns the name of an item in the symbol list.
        /// </summary>
        /// <param name="itemNum">An index into the symbol list</param>
        /// <returns>The name of an item in the symbol list.</returns>
        public bool GetStatus(int itemNum)
        {
            return m_SymbolListItemList[itemNum].IsInUse;
        }

        public Buffer? SymbolListItemName(int itemNum)
        {
            return m_SymbolListItemList[itemNum].ItemName;
        }

        /// <summary>
        /// Sets the status and and name of a symbol list item.
        /// </summary>
        /// <param name="itemName">The name that the symbol list item will be set to</param>
        /// <param name="itemNum">An index into the symbol list array</param>
        public void SymbolListItemName(Buffer itemName, int itemNum)
        {
            // copy item name buffer
            ByteBuffer byteBuffer = new ByteBuffer(itemName.Length);
            itemName.Copy(byteBuffer);
            byteBuffer.Flip();
            m_SymbolListItemList[itemNum].ItemName.Data(byteBuffer);
            m_SymbolListItemList[itemNum].IsInUse = true;
        }

        /// <summary>
        /// Returns the current number of items in the symbol list.
        /// </summary>
        /// <returns>The item count</returns>
        public int ItemCount()
        {
            return m_ItemCount;
        }

        /// <summary>
        /// Increments the number of items in the symbol list.
        /// </summary>
        public void IncrementItemCount()
        {
            m_ItemCount++;
        }

        /// <summary>
        /// Decrements the number of items in the symbol list.
        /// </summary>
        public void DecrementItemCount()
        {
            m_ItemCount--;
        }

        /// <summary>
        /// Increments the interest count of an item in the symbol list.
        /// </summary>
        /// <param name="itemNum">An index into the symbol list array.</param>
        public void IncrementInterestCount(int itemNum)
        {
            m_SymbolListItemList[itemNum].InterestCount++;
        }

        /// <summary>
        /// Decrements the interest count of an item in the symbol list itemNum - An
        /// index into the symbol list array.
        /// </summary>
        /// <param name="itemNum">the item number</param>
        public void DecrementInterestCount(int itemNum)
        {
            m_SymbolListItemList[itemNum].InterestCount--;
        }

        /// <summary>
        /// Returns the current interest count of an item in the symbol list itemNum
        /// - An index into the symbol list array.
        /// </summary>
        /// <param name="itemNum">The item num</param>
        /// <returns>The interest count</returns>
        public int InterestCount(int itemNum)
        {
            return m_SymbolListItemList[itemNum].InterestCount;
        }

        /// <summary>
        /// Initializes the symbol list item fields.
        /// </summary>
        public void Init()
        {
            m_SymbolListItemList[0].IsInUse = true;
            m_SymbolListItemList[0].ItemName?.Data("TRI");
            m_SymbolListItemList[1].IsInUse = true;
            m_SymbolListItemList[1].ItemName?.Data("RES-DS");
            m_ItemCount = 2;

            /* clear out the rest of the entries */
            for (int i = 2; i < MAX_SYMBOL_LIST_SIZE; i++)
            {
                m_SymbolListItemList[i].Clear();
            }
        }

        /// <summary>
        /// Clears out a symbol list item field.
        /// </summary>
        /// <param name="itemNum">The item number to be cleared</param>
        public void Clear(int itemNum)
        {
            m_SymbolListItemList[itemNum].IsInUse = false;
            m_SymbolListItemList[itemNum].InterestCount = 0;
            m_SymbolListItemList[itemNum].ItemName!.Clear();
        }

        /// <summary>
        /// Encodes the symbol list response. Returns success if encoding succeeds or
        /// failure if encoding fails.
        /// </summary>
        /// <param name="chnl">The channel to send a market price response to</param>
        /// <param name="itemInfo">The item information</param>
        /// <param name="msgBuf">The message buffer to encode the market price response into</param>
        /// <param name="streamId">The stream id of the market price response</param>
        /// <param name="isStreaming">Flag for streaming or snapshot</param>
        /// <param name="serviceId">The service id of the market price response</param>
        /// <param name="isSolicited">The response is solicited if set</param>
        /// <param name="dictionary">The dictionary used for encoding</param>
        /// <param name="responseType">The type of response to be encoded: refresh, add update, or delete update</param>
        /// <param name="error">Set in case of encoding error</param>
        /// <returns><see cref="CodecReturnCode"/></returns>
        public CodecReturnCode EncodeResponse(IChannel chnl, ItemInfo itemInfo, ITransportBuffer msgBuf, int streamId, bool isStreaming, int serviceId, bool isSolicited, 
            DataDictionary dictionary, int responseType, out Error? error)
        {
            error = null;
            m_RefreshMsg.Clear();
            m_UpdateMsg.Clear();

            /* set-up message */
            Msg? msg = null;
            /* set message depending on whether refresh or update */
            if (responseType == SymbolListItems.SYMBOL_LIST_REFRESH) /*
                                                                  * this is a
                                                                  * refresh
                                                                  * message
                                                                  */
            {
                m_RefreshMsg.MsgClass = MsgClasses.REFRESH;
                m_RefreshMsg.State.StreamState(StreamStates.OPEN);
                if (isStreaming)
                {
                    m_RefreshMsg.State.DataState(DataStates.OK);
                }
                else
                {
                    m_RefreshMsg.State.StreamState(StreamStates.NON_STREAMING);
                }
                m_RefreshMsg.State.Code(StateCodes.NONE);
                if (isSolicited)
                {
                    m_RefreshMsg.Flags = RefreshMsgFlags.HAS_MSG_KEY | RefreshMsgFlags.SOLICITED | RefreshMsgFlags.REFRESH_COMPLETE | RefreshMsgFlags.HAS_QOS;
                }
                else
                {
                    m_RefreshMsg.Flags =RefreshMsgFlags.HAS_MSG_KEY | RefreshMsgFlags.REFRESH_COMPLETE | RefreshMsgFlags.HAS_QOS;
                }
                m_RefreshMsg.State.Text().Data("Item Refresh Completed");
                m_RefreshMsg.MsgKey.Flags = MsgKeyFlags.HAS_SERVICE_ID | MsgKeyFlags.HAS_NAME;
                /* ServiceId */
                m_RefreshMsg.MsgKey.ServiceId = serviceId;
                /* Itemname */
                m_RefreshMsg.MsgKey.Name.Data(itemInfo.ItemName.Data(), itemInfo.ItemName.Position, itemInfo.ItemName.Length);

                /* Qos */
                m_RefreshMsg.Qos.IsDynamic = false;
                m_RefreshMsg.Qos.Rate(QosRates.TICK_BY_TICK);
                m_RefreshMsg.Qos.Timeliness(QosTimeliness.REALTIME);
                msg = (Msg)m_RefreshMsg;
            }
            else
            /* this is an update message */
            {
                m_UpdateMsg.MsgClass = MsgClasses.UPDATE;
                msg = (Msg)m_UpdateMsg;
            }
            msg.DomainType = (int)Rdm.DomainType.SYMBOL_LIST;
            msg.ContainerType = DataTypes.MAP;

            /* StreamId */
            msg.StreamId = streamId;

            m_EncodeIter.Clear();
            /* encode message */
            CodecReturnCode ret = m_EncodeIter.SetBufferAndRWFVersion(msgBuf, chnl.MajorVersion, chnl.MinorVersion);
            if (ret != CodecReturnCode.SUCCESS)
            {
                error = new Error
                {
                    ErrorId = TransportReturnCode.FAILURE,
                    Text = $"EncodeIter.SetBufferAndRWFVersion() failed with return code: {ret.GetAsString()}"
                };

                return ret;
            }
            ret = msg.EncodeInit(m_EncodeIter, 0);
            if (ret < CodecReturnCode.SUCCESS)
            {
                error = new Error
                {
                    ErrorId = TransportReturnCode.FAILURE,
                    Text = $"Msg.EncodeInit() failed with return code: {ret.GetAsString()}"
                };

                return ret;
            }

            /* encode map */
            m_TempMap.Clear();
            m_TempMap.Flags = 0;
            m_TempMap.ContainerType = DataTypes.NO_DATA;
            m_TempMap.KeyPrimitiveType = DataTypes.BUFFER;
            ret = m_TempMap.EncodeInit(m_EncodeIter, 0, 0);
            if (ret != CodecReturnCode.SUCCESS)
            {
                error = new Error
                {
                    ErrorId = TransportReturnCode.FAILURE,
                    Text = $"map.EncodeInit() failed with return code: {ret.GetAsString()}"
                };

                return ret;
            }

            int i = 0;
            /* encode map entry */
            m_TempMapEntry.Clear();
            m_TempMapEntry.Flags = MapEntryFlags.NONE;
            switch (responseType)
            {
                /* this is a refresh message, so begin encoding the entire symbol list */
                case SYMBOL_LIST_REFRESH:
                    m_TempBuffer.Data(m_SymbolListItemList[i].ItemName?.Data());
                    m_TempMapEntry.Action = MapEntryActions.ADD;
                    break;
                /*
                 * this is an update message adding a name, so only encode the item
                 * being added to the list
                 */
                case SYMBOL_LIST_UPDATE_ADD:
                    m_TempBuffer.Data(itemInfo.ItemName.Data(), itemInfo.ItemName.Position, itemInfo.ItemName.Length);
                    m_TempMapEntry.Action = MapEntryActions.ADD;
                    break;

                /* this is an update message deleting a name */
                case SYMBOL_LIST_UPDATE_DELETE:
                    m_TempBuffer.Data(itemInfo.ItemName.Data(), itemInfo.ItemName.Position, itemInfo.ItemName.Length);
                    m_TempMapEntry.Action = MapEntryActions.DELETE;
                    break;
                default:
                    error = new Error
                    {
                        ErrorId = TransportReturnCode.FAILURE,
                        Text = $"Invalid SymbolListItems responseType: {responseType}"
                    };

                    return CodecReturnCode.FAILURE;
            }

            ret = m_TempMapEntry.EncodeInit(m_EncodeIter, m_TempBuffer, 0);
            if (ret != CodecReturnCode.SUCCESS)
            {
                error = new Error
                {
                    ErrorId = TransportReturnCode.FAILURE,
                    Text = $"MapEntry.EncodeInit() failed with return code: {ret.GetAsString()}"
                };

                return ret;
            }

            ret = m_TempMapEntry.EncodeComplete(m_EncodeIter, true);
            if (ret != CodecReturnCode.SUCCESS)
            {
                error = new Error
                {
                    ErrorId = TransportReturnCode.FAILURE,
                    Text = $"MapEntry.EncodeComplete() failed with return code: {ret.GetAsString()}"
                };

                return ret;
            }

            /*
             * if this is a refresh message, finish encoding the entire symbol list
             * in the response
             */
            if (responseType == SymbolListItems.SYMBOL_LIST_REFRESH)
            {
                for (i = 1; i < SymbolListItems.MAX_SYMBOL_LIST_SIZE; i++)
                {
                    if (m_SymbolListItemList[i].IsInUse == true)
                    {
                        m_TempBuffer.Data(m_SymbolListItemList[i].ItemName?.Data());
                        ret = m_TempMapEntry.Encode(m_EncodeIter, m_TempBuffer);
                        if (ret != CodecReturnCode.SUCCESS)
                        {
                            error = new Error
                            {
                                ErrorId = TransportReturnCode.FAILURE,
                                Text = $"mapEntry.Encode() failed with return code: {ret.GetAsString()}"
                            };

                            return ret;
                        }
                    }
                }
            }

            /* complete map */
            ret = m_TempMap.EncodeComplete(m_EncodeIter, true);
            if (ret != CodecReturnCode.SUCCESS)
            {
                error = new Error
                {
                    ErrorId = TransportReturnCode.FAILURE,
                    Text = $"mapEntry.EncodeComplete() failed with return code: {ret.GetAsString()}"
                };

                return ret;
            }

            /* complete encode message */
            ret = msg.EncodeComplete(m_EncodeIter, true);
            if (ret != CodecReturnCode.SUCCESS)
            {
                error = new Error
                {
                    ErrorId = TransportReturnCode.FAILURE,
                    Text = $"msg.EncodeComplete() failed with return code: {ret.GetAsString()}"
                };

                return ret;
            }

            return CodecReturnCode.SUCCESS;
        }
    }
}
