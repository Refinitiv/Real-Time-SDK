/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.shared.provider;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.List;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DataDictionary;
import com.refinitiv.eta.codec.DataStates;
import com.refinitiv.eta.codec.DataTypes;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.Map;
import com.refinitiv.eta.codec.MapEntry;
import com.refinitiv.eta.codec.MapEntryActions;
import com.refinitiv.eta.codec.MapEntryFlags;
import com.refinitiv.eta.codec.Msg;
import com.refinitiv.eta.codec.MsgClasses;
import com.refinitiv.eta.codec.MsgKeyFlags;
import com.refinitiv.eta.codec.QosRates;
import com.refinitiv.eta.codec.QosTimeliness;
import com.refinitiv.eta.codec.RefreshMsg;
import com.refinitiv.eta.codec.RefreshMsgFlags;
import com.refinitiv.eta.codec.StateCodes;
import com.refinitiv.eta.codec.StreamStates;
import com.refinitiv.eta.codec.UpdateMsg;
import com.refinitiv.eta.shared.rdm.symbollist.SymbolListItem;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.transport.Channel;
import com.refinitiv.eta.transport.Error;
import com.refinitiv.eta.transport.TransportBuffer;

/**
 * Storage class for managing symbol list items.
 * 
 * This handles the generation of example symbol lists, and shows how to encode
 * it. Provides methods for managing the list, and a method for encoding a
 * symbol list message and payload.
 */
public class SymbolListItems
{
    public static final int SYMBOL_LIST_REFRESH = 0;
    public static final int SYMBOL_LIST_UPDATE_ADD = 1;
    public static final int SYMBOL_LIST_UPDATE_DELETE = 2;

    public static final int MAX_SYMBOL_LIST_SIZE = 100;

    private List<SymbolListItem> _symbolListItemList = new ArrayList<>(MAX_SYMBOL_LIST_SIZE);
    private int _itemCount = 0;

    private EncodeIterator _encodeIter = CodecFactory.createEncodeIterator();
    private RefreshMsg _refreshMsg = (RefreshMsg)CodecFactory.createMsg();
    private UpdateMsg _updateMsg = (UpdateMsg)CodecFactory.createMsg();
    private Map _tempMap = CodecFactory.createMap();
    private MapEntry _tempMapEntry = CodecFactory.createMapEntry();
    private Buffer _tempBuffer = CodecFactory.createBuffer();

    /**
     * Instantiates a new symbol list items.
     */
    public SymbolListItems()
    {
        for (int i = 0; i < MAX_SYMBOL_LIST_SIZE; i++)
        {
            _symbolListItemList.add(new SymbolListItem());
        }

    }

    /**
     * Clears out a symbol list item field.
     */
    public void clear()
    {
        for (SymbolListItem symbolListItem : _symbolListItemList)
        {
            symbolListItem.clear();
        }
    }

    /**
     * Returns the status of an item in the symbol list.
     * 
     * @param itemNum - An index into the symbol list array
     * 
     * @return the status of an item in the symbol lis
     */
    public boolean getStatus(int itemNum)
    {
        return _symbolListItemList.get(itemNum).isInUse;
    }

    /**
     * Returns the name of an item in the symbol list.
     * 
     * @param itemNum - An index into the symbol list array
     * @return the name of an item in the symbol list.
     */
    public Buffer symbolListItemName(int itemNum)
    {
        return _symbolListItemList.get(itemNum).itemName;
    }

    /**
     * Sets the status and and name of a symbol list item.
     *
     * @param itemName - the name that the symbol list item will be set to
     * @param itemNum - An index into the symbol list array
     */
    public void symbolListItemName(Buffer itemName, int itemNum)
    {
        // copy item name buffer
        ByteBuffer byteBuffer = ByteBuffer.allocate(itemName.length());
        itemName.copy(byteBuffer);
        _symbolListItemList.get(itemNum).itemName = itemName;
        _symbolListItemList.get(itemNum).isInUse = true;
    }

    /**
     * Returns the current number of items in the symbol list.
     *
     * @return the int
     */
    public int itemCount()
    {
        return _itemCount;
    }

    /**
     * Increments the number of items in the symbol list.
     */
    public void incrementItemCount()
    {
        _itemCount++;
    }

    /**
     * Decrements the number of items in the symbol list.
     */
    public void decrementItemCount()
    {
        _itemCount--;
    }

    /**
     * Increments the interest count of an item in the symbol list.
     *
     * @param itemNum - An index into the symbol list array.
     */
    public void incrementInterestCount(int itemNum)
    {
        _symbolListItemList.get(itemNum).interestCount++;
    }

    /**
     * Decrements the interest count of an item in the symbol list itemNum - An
     * index into the symbol list array.
     *
     * @param itemNum the item num
     */
    public void decrementInterestCount(int itemNum)
    {
        _symbolListItemList.get(itemNum).interestCount--;
    }

    /**
     * Returns the current interest count of an item in the symbol list itemNum
     * - An index into the symbol list array.
     *
     * @param itemNum the item num
     * @return the int
     */
    public int interestCount(int itemNum)
    {
        return _symbolListItemList.get(itemNum).interestCount;
    }

    /**
     * Initializes the symbol list item fields.
     */
    public void init()
    {
        _symbolListItemList.get(0).isInUse = true;
        _symbolListItemList.get(0).itemName.data("TRI");
        _symbolListItemList.get(1).isInUse = true;
        _symbolListItemList.get(1).itemName.data("RES-DS");
        _itemCount = 2;

        /* clear out the rest of the entries */
        for (int i = 2; i < MAX_SYMBOL_LIST_SIZE; i++)
        {
            _symbolListItemList.get(i).clear();
        }
    }

    /**
     * Clears out a symbol list item field.
     * 
     * @param itemNum - the item number to be cleared
     */
    public void clear(int itemNum)
    {
        _symbolListItemList.get(itemNum).isInUse = false;
        _symbolListItemList.get(itemNum).interestCount = 0;
        _symbolListItemList.get(itemNum).itemName = null;
    }

    /**
     * Encodes the symbol list response. Returns success if encoding succeeds or
     * failure if encoding fails.
     *
     * @param chnl - The channel to send a market price response to
     * @param itemInfo - The item information
     * @param msgBuf - The message buffer to encode the market price response
     *            into
     * @param streamId - The stream id of the market price response
     * @param isStreaming - Flag for streaming or snapshot
     * @param serviceId - The service id of the market price response
     * @param isSolicited - The response is solicited if set
     * @param dictionary - The dictionary used for encoding
     * @param responseType - The type of response to be encoded: refresh, add
     *            update, or delete update
     * @param error - error in case of encoding error
     * @return {@link CodecReturnCodes}
     */
    public int encodeResponse(Channel chnl, ItemInfo itemInfo, TransportBuffer msgBuf, int streamId, boolean isStreaming, int serviceId, boolean isSolicited, DataDictionary dictionary, int responseType, Error error)
    {
        _refreshMsg.clear();
        _updateMsg.clear();
   
        /* set-up message */
        Msg msg = null;
        /* set message depending on whether refresh or update */
        if (responseType == SymbolListItems.SYMBOL_LIST_REFRESH) /*
                                                                  * this is a
                                                                  * refresh
                                                                  * message
                                                                  */
        {
            _refreshMsg.msgClass(MsgClasses.REFRESH);
            _refreshMsg.state().streamState(StreamStates.OPEN);
            if (isStreaming)
            {
                _refreshMsg.state().dataState(DataStates.OK);
            }
            else
            {
                _refreshMsg.state().streamState(StreamStates.NON_STREAMING);
            }
            _refreshMsg.state().code(StateCodes.NONE);
            if (isSolicited)
            {
                _refreshMsg.flags(RefreshMsgFlags.HAS_MSG_KEY | RefreshMsgFlags.SOLICITED | RefreshMsgFlags.REFRESH_COMPLETE | RefreshMsgFlags.HAS_QOS);
            }
            else
            {
                _refreshMsg.flags(RefreshMsgFlags.HAS_MSG_KEY | RefreshMsgFlags.REFRESH_COMPLETE | RefreshMsgFlags.HAS_QOS);
            }
            _refreshMsg.state().text().data("Item Refresh Completed");
            _refreshMsg.msgKey().flags(MsgKeyFlags.HAS_SERVICE_ID | MsgKeyFlags.HAS_NAME);
            /* ServiceId */
            _refreshMsg.msgKey().serviceId(serviceId);
            /* Itemname */
            _refreshMsg.msgKey().name().data(itemInfo.itemName.data(), itemInfo.itemName.position(), itemInfo.itemName.length());

            /* Qos */
            _refreshMsg.qos().dynamic(false);
            _refreshMsg.qos().rate(QosRates.TICK_BY_TICK);
            _refreshMsg.qos().timeliness(QosTimeliness.REALTIME);
            msg = _refreshMsg;
        }
        else
        /* this is an update message */
        {
            _updateMsg.msgClass(MsgClasses.UPDATE);
            msg = _updateMsg;
        }
        msg.domainType(DomainTypes.SYMBOL_LIST);
        msg.containerType(DataTypes.MAP);

        /* StreamId */
        msg.streamId(streamId);


        _encodeIter.clear();
        /* encode message */
        int ret = _encodeIter.setBufferAndRWFVersion(msgBuf, chnl.majorVersion(), chnl.minorVersion());
        if (ret != CodecReturnCodes.SUCCESS)
        {
            error.text("EncodeIter.setBufferAndRWFVersion() failed with return code: " + CodecReturnCodes.toString(ret));
            return ret;
        }
        ret = msg.encodeInit(_encodeIter, 0);
        if (ret < CodecReturnCodes.SUCCESS)
        {
            error.text("Msg.encodeInit() failed with return code: " + CodecReturnCodes.toString(ret));
            return ret;
        }

        /* encode map */
        _tempMap.clear();
        _tempMap.flags(0);
        _tempMap.containerType(DataTypes.NO_DATA);
        _tempMap.keyPrimitiveType(DataTypes.BUFFER);
        ret = _tempMap.encodeInit(_encodeIter, 0, 0);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            error.text("map.encodeInit() failed with return code: " + ret);
            return ret;
        }

        int i = 0;
        /* encode map entry */
        _tempMapEntry.clear();
        _tempMapEntry.flags(MapEntryFlags.NONE);
        _tempBuffer.clear();
        switch (responseType)
        {
        /* this is a refresh message, so begin encoding the entire symbol list */
            case SymbolListItems.SYMBOL_LIST_REFRESH:
                _tempBuffer.data(_symbolListItemList.get(i).itemName.data());
                _tempMapEntry.action(MapEntryActions.ADD);
                break;
            /*
             * this is an update message adding a name, so only encode the item
             * being added to the list
             */
            case SymbolListItems.SYMBOL_LIST_UPDATE_ADD:
                _tempBuffer.data(itemInfo.itemName.data(), itemInfo.itemName.position(), itemInfo.itemName.length());
                _tempMapEntry.action(MapEntryActions.ADD);
                break;

            /* this is an update message deleting a name */
            case SymbolListItems.SYMBOL_LIST_UPDATE_DELETE:
                _tempBuffer.data(itemInfo.itemName.data(), itemInfo.itemName.position(), itemInfo.itemName.length());
                _tempMapEntry.action(MapEntryActions.DELETE);
                break;
            default:
            	error.text("Invalid SymbolListItems responseType: " + responseType);
            	return CodecReturnCodes.FAILURE;
        }

        ret = _tempMapEntry.encodeInit(_encodeIter, _tempBuffer, 0);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            error.text("MapEntry.encodeInit() failed with return code: " + CodecReturnCodes.toString(ret));
            return ret;
        }

        ret = _tempMapEntry.encodeComplete(_encodeIter, true);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            error.text("MapEntry.encodeComplete() failed with return code: " + CodecReturnCodes.toString(ret));
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
                if (_symbolListItemList.get(i).isInUse == true)
                {
                    _tempBuffer.data(_symbolListItemList.get(i).itemName.data());
                    ret = _tempMapEntry.encode(_encodeIter, _tempBuffer);
                    if (ret != CodecReturnCodes.SUCCESS)
                    {
                        error.text("mapEntry.encode() failed with return code: " + CodecReturnCodes.toString(ret));
                        return ret;
                    }
                }
            }
        }

        /* complete map */
        ret = _tempMap.encodeComplete(_encodeIter, true);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            error.text("mapEntry.encodeComplete() failed with return code: " + CodecReturnCodes.toString(ret));
            return ret;
        }

        /* complete encode message */
        ret = msg.encodeComplete(_encodeIter, true);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            error.text("msg.encodeComplete() failed with return code: " + CodecReturnCodes.toString(ret));
            return ret;
        }

        return CodecReturnCodes.SUCCESS;
    }
}
