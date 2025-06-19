/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2018,2020,2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.shared.provider;

import java.util.ArrayList;
import java.util.List;

import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DataDictionary;
import com.refinitiv.eta.codec.DataStates;
import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.Enum;
import com.refinitiv.eta.codec.FieldEntry;
import com.refinitiv.eta.codec.FieldList;
import com.refinitiv.eta.codec.QosRates;
import com.refinitiv.eta.codec.QosTimeliness;
import com.refinitiv.eta.codec.Real;
import com.refinitiv.eta.codec.StateCodes;
import com.refinitiv.eta.codec.StreamStates;
import com.refinitiv.eta.codec.UInt;
import com.refinitiv.eta.shared.rdm.marketprice.MarketPriceItem;
import com.refinitiv.eta.shared.rdm.marketprice.MarketPriceRefresh;
import com.refinitiv.eta.shared.rdm.marketprice.MarketPriceUpdate;
import com.refinitiv.eta.transport.Channel;
import com.refinitiv.eta.transport.Error;
import com.refinitiv.eta.transport.TransportBuffer;

/**
 * This handles storage of all market price items.
 * 
 * <p>
 * Provides methods generation of market price data, for managing the list, and
 * a method for encoding a market price message and payload.
 */
public class MarketPriceItems
{
    //APIQA
    private static final int MAX_MARKET_PRICE_ITEM_LIST_SIZE = 10000;

    // item information list
    public List<MarketPriceItem> _marketPriceList;

    private Real _tempReal = CodecFactory.createReal();
    private Enum _tempEnum = CodecFactory.createEnum();
    private UInt _tempUInt = CodecFactory.createUInt();
    private FieldList _tempFieldList = CodecFactory.createFieldList();
    private FieldEntry _tempFieldEntry = CodecFactory.createFieldEntry();

    protected MarketPriceRefresh _marketPriceRefresh;
    protected MarketPriceUpdate _marketPriceUpdate;
    protected EncodeIterator _encodeIter = CodecFactory.createEncodeIterator();

    /**
     * Instantiates a new market price items.
     */
    public MarketPriceItems()
    {
        _marketPriceList = new ArrayList<MarketPriceItem>(MAX_MARKET_PRICE_ITEM_LIST_SIZE);
        for (int i = 0; i < MAX_MARKET_PRICE_ITEM_LIST_SIZE; i++)
        {
            _marketPriceList.add(new MarketPriceItem());
        }
        _marketPriceRefresh = new MarketPriceRefresh();
        _marketPriceUpdate = new MarketPriceUpdate();
    }

    /**
     * Initializes market price item list.
     */
    public void init()
    {
        for (MarketPriceItem mpItem : _marketPriceList)
        {
            mpItem.clear();
        }
    }

    /**
     * Updates any item that's currently in use.
     */
    public void update()
    {
        for (MarketPriceItem mpItem : _marketPriceList)
        {
            if (mpItem.isInUse)
                mpItem.updateFields();
        }
    }

    /**
     * Gets storage for a market price item from the list.
     *
     * @param itemName the item name
     * @return the market price item
     */
    public MarketPriceItem get(String itemName)
    {
        // first try to find one with same name and reuse
        for (MarketPriceItem mpItem : _marketPriceList)
        {
            if (mpItem.isInUse && mpItem.itemName != null && mpItem.itemName.equals(itemName))
            {
                return mpItem;
            }
        }
        
        // next get a new one
        for (MarketPriceItem mpItem : _marketPriceList)
        {
            if (!mpItem.isInUse)
            {
                mpItem.initFields();
                mpItem.itemName = itemName;
                return mpItem;
            }
        }

        return null;
    }

    /**
     * Clears the item information.
     */
    public void clear()
    {
        for (MarketPriceItem mpItem : _marketPriceList)
        {
            mpItem.isInUse = false;
            mpItem.RDNDISPLAY = 0;
            mpItem.RDN_EXCHID = 0;
            mpItem.DIVPAYDATE.clear();
            mpItem.TRDPRC_1 = 0;
            mpItem.BID = 0;
            mpItem.ASK = 0;
            mpItem.ACVOL_1 = 0;
            mpItem.NETCHNG_1 = 0;
        }
    }

    /**
     * Updates the item's data from the post we got.
     *
     * @param mpItem the mp item
     * @param dIter the d iter
     * @param error the error
     * @return the int
     */
    public int updateFieldsFromPost(MarketPriceItem mpItem, DecodeIterator dIter, Error error)
    {
        // decode field list
        int ret = _tempFieldList.decode(dIter, null);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            error.text("fList.decode() failed with return code: " + ret);
            error.errorId(ret);
        }

        // decode each field entry in list and apply it to the market price item
        while ((ret = _tempFieldEntry.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER)
        {
            if (ret != CodecReturnCodes.SUCCESS)
            {
                error.text("fEntry.decode() failed with return code: " + ret);
                error.errorId(ret);
                return ret;
            }
            switch (_tempFieldEntry.fieldId())
            {
                case MarketPriceItem.RDNDISPLAY_FID:
                    ret = _tempUInt.decode(dIter);
                    if (ret != CodecReturnCodes.SUCCESS)
                    {
                        error.text("Error=" + ret + " Decoding RDNDISPLAY");
                        error.errorId(ret);
                        return ret;
                    }
                    mpItem.RDNDISPLAY = (int)_tempUInt.toLong();
                    break;

                case MarketPriceItem.RDN_EXCHID_FID:
                    ret = _tempEnum.decode(dIter);
                    if (ret != CodecReturnCodes.SUCCESS)
                    {
                        error.text("Error=" + ret + " Decoding RDN_EXCHID_FID");
                        error.errorId(ret);
                        return ret;
                    }
                    mpItem.RDN_EXCHID = _tempEnum.toInt();
                    break;

                case MarketPriceItem.DIVPAYDATE_FID:
                    ret = mpItem.DIVPAYDATE.decode(dIter);
                    if (ret != CodecReturnCodes.SUCCESS)
                    {
                        error.text("Error=" + ret + " Decoding DIVPAYDATE_FID");
                        error.errorId(ret);
                        return ret;
                    }
                    break;

                case MarketPriceItem.TRDPRC_1_FID:
                    ret = _tempReal.decode(dIter);
                    if (ret != CodecReturnCodes.SUCCESS)
                    {
                        error.text("Error=" + ret + " Decoding TRDPRC_1_FID");
                        error.errorId(ret);
                        return ret;
                    }
                    mpItem.TRDPRC_1 = _tempReal.toDouble();
                    break;

                case MarketPriceItem.BID_FID:
                    ret = _tempReal.decode(dIter);
                    if (ret != CodecReturnCodes.SUCCESS)
                    {
                        error.text("Error=" + ret + " Decoding BID_FID");
                        error.errorId(ret);
                        return ret;
                    }
                    mpItem.BID = _tempReal.toDouble();
                    break;
                case MarketPriceItem.ASK_FID:
                    ret = _tempReal.decode(dIter);
                    if (ret != CodecReturnCodes.SUCCESS)
                    {
                        error.text("Error=" + ret + " Decoding ASK_FID");
                        error.errorId(ret);
                        return ret;
                    }
                    mpItem.ASK = _tempReal.toDouble();
                    break;

                case MarketPriceItem.ACVOL_1_FID:
                    ret = _tempReal.decode(dIter);
                    if (ret != CodecReturnCodes.SUCCESS)
                    {
                        error.text("Error=" + ret + " Decoding ACVOL_1");
                        error.errorId(ret);
                        return ret;
                    }
                    mpItem.ACVOL_1 = _tempReal.toDouble();
                    break;

                case MarketPriceItem.NETCHNG_1_FID:
                    ret = _tempReal.decode(dIter);
                    if (ret != CodecReturnCodes.SUCCESS)
                    {
                        error.text("Error=" + ret + " Decoding NETCHNG_1");
                        error.errorId(ret);
                        return ret;
                    }
                    mpItem.NETCHNG_1 = _tempReal.toDouble();
                    break;

                case MarketPriceItem.ASK_TIME_FID:
                    ret = mpItem.ASK_TIME.time().decode(dIter);
                    if (ret != CodecReturnCodes.SUCCESS)
                    {
                        error.text("Error=" + ret + " Decoding ASK_TIME");
                        error.errorId(ret);
                        return ret;
                    }
                    break;
                case MarketPriceItem.PERATIO_FID:
                    ret = _tempReal.decode(dIter);
                    if (ret != CodecReturnCodes.SUCCESS)
                    {
                        error.text("Error=" + ret + " Decoding PERATIO_FID");
                        error.errorId(ret);
                        return ret;
                    }
                    mpItem.PERATIO = _tempReal.toDouble();

                    break;
                case MarketPriceItem.SALTIME_FID:
                    ret = mpItem.SALTIME.time().decode(dIter);
                    if (ret != CodecReturnCodes.SUCCESS)
                    {
                        error.text("Error=" + ret + " Decoding SALTIME");
                        error.errorId(ret);
                        return ret;
                    }
                    break;

                default:
                    error.text("Unknown field ID =" + _tempFieldEntry.fieldId() + " in post");
                    error.errorId(ret);
                    return CodecReturnCodes.FAILURE;
            }
        }
        return CodecReturnCodes.SUCCESS;
    }

    /**
     * Encodes the market price response. Returns success if encoding succeeds
     * or failure if encoding fails.
     * 
     * @param channel - The channel to send a market price response to
     * @param itemInfo - The item information
     * @param isSolicited - The response is solicited if set
     * @param msgBuf - The message buffer to encode the market price response
     *            into
     * @param streamId - The stream id of the market price response
     * @param isStreaming - Flag for streaming or snapshot
     * @param isPrivateStream - Flag for private stream
     * @param serviceId - The service id of the market price response
     * @param dictionary - The dictionary used for encoding
     * @param error - error in case of encoding failure
     * 
     * @return {@link CodecReturnCodes}
     */
    public int encodeResponse(Channel channel, ItemInfo itemInfo, TransportBuffer msgBuf, boolean isSolicited, int streamId, boolean isStreaming, boolean isPrivateStream, int serviceId, DataDictionary dictionary, Error error)
    {
        MarketPriceItem mpItem = (MarketPriceItem)itemInfo.itemData;

        // set message depending on whether refresh or update
        if (itemInfo.isRefreshRequired)
        {
            _marketPriceRefresh.clear();
            _marketPriceRefresh.dictionary(dictionary);
            if (isStreaming)
            {
                _marketPriceRefresh.state().streamState(StreamStates.OPEN);
            }
            else
            {
                _marketPriceRefresh.state().streamState(StreamStates.NON_STREAMING);
            }
            _marketPriceRefresh.state().dataState(DataStates.OK);
            _marketPriceRefresh.state().code(StateCodes.NONE);
            _marketPriceRefresh.state().text().data("Item Refresh Completed");
            _marketPriceRefresh.applyRefreshComplete();
            _marketPriceRefresh.applyHasQos();

            if (isSolicited)
            {
                _marketPriceRefresh.applySolicited();
                
                // clear cache for solicited refresh messages.
                _marketPriceRefresh.applyClearCache();
            }

            if (isPrivateStream)
            {
                _marketPriceRefresh.applyPrivateStream();
            }

            // Service Id
            _marketPriceRefresh.applyHasServiceId();
            _marketPriceRefresh.serviceId(serviceId);

            // ItemName
            _marketPriceRefresh.itemName().data(itemInfo.itemName.data(), itemInfo.itemName.position(), itemInfo.itemName.length());

            // Qos
            _marketPriceRefresh.qos().dynamic(false);
            _marketPriceRefresh.qos().rate(QosRates.TICK_BY_TICK);
            _marketPriceRefresh.qos().timeliness(QosTimeliness.REALTIME);

            // StreamId
            _marketPriceRefresh.streamId(streamId);
            _marketPriceRefresh.marketPriceItem(mpItem);

            // encode
            _encodeIter.clear();
            int ret = _encodeIter.setBufferAndRWFVersion(msgBuf, channel.majorVersion(), channel.minorVersion());
            if (ret != CodecReturnCodes.SUCCESS)
            {
                error.text("EncodeIterator.setBufferAndRWFVersion() failed with return code: " + CodecReturnCodes.toString(ret));
                return ret;
            }
            ret = _marketPriceRefresh.encode(_encodeIter);
            if (ret != CodecReturnCodes.SUCCESS)
                error.text("MarketPriceRefresh.encode() failed");

            return ret;
        }
        else
        {
            _marketPriceUpdate.clear();
            _marketPriceUpdate.dictionary(dictionary);

            // this is an update message
            _marketPriceUpdate.streamId(streamId);

            // include msg key in updates for non-interactive provider streams
            if (streamId < 0)
            {
                _marketPriceUpdate.applyHasServiceId();

                // ServiceId
                _marketPriceUpdate.serviceId(serviceId);

                // Itemname
                _marketPriceUpdate.itemName().data(itemInfo.itemName.data(), itemInfo.itemName.position(), itemInfo.itemName.length());
            }

            if (isPrivateStream)
            {
                _marketPriceUpdate.applyPrivateStream();
            }

            _marketPriceUpdate.marketPriceItem(mpItem);
            
            // encode
            _encodeIter.clear();
            int ret = _encodeIter.setBufferAndRWFVersion(msgBuf, channel.majorVersion(), channel.minorVersion());
            if (ret != CodecReturnCodes.SUCCESS)
            {
                error.text("EncodeIterator.setBufferAndRWFVersion() failed with return code: " + CodecReturnCodes.toString(ret));
                return ret;
            }
            ret = _marketPriceUpdate.encode(_encodeIter);
            if (ret != CodecReturnCodes.SUCCESS)
                error.text("MarketPriceUpdate.encode() failed");

            return ret;
        }
    }
}
