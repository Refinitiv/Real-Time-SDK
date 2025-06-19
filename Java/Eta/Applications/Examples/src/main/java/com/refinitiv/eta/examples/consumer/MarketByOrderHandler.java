/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.examples.consumer;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DataDictionary;
import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.LocalFieldSetDefDb;
import com.refinitiv.eta.codec.Map;
import com.refinitiv.eta.codec.MapEntry;
import com.refinitiv.eta.codec.MapEntryActions;
import com.refinitiv.eta.codec.Msg;
import com.refinitiv.eta.codec.MsgClasses;
import com.refinitiv.eta.codec.RefreshMsg;
import com.refinitiv.eta.examples.common.StreamIdWatchList;
import com.refinitiv.eta.shared.rdm.marketbyorder.MarketByOrderRequest;
import com.refinitiv.eta.shared.rdm.marketprice.MarketPriceRequest;
import com.refinitiv.eta.rdm.DomainTypes;

/**
 * This is the market by order handler for the ETA consumer application. It
 * provides methods for decoding a map containing field list contents,
 * decoding field entries from a response.
 * 
 * Methods for sending the market by order request(s) to a provider,
 * processing the response(s) and closing streams are inherited from
 * MarketPriceHandler.
 * 
 */
public class MarketByOrderHandler extends MarketPriceHandler
{
    private Map map = CodecFactory.createMap();
    private MapEntry mapEntry = CodecFactory.createMapEntry();
    private Buffer mapKey = CodecFactory.createBuffer();
    private LocalFieldSetDefDb localFieldSetDefDb = CodecFactory.createLocalFieldSetDefDb();
    
    public MarketByOrderHandler(StreamIdWatchList watchList)
    {
        super(DomainTypes.MARKET_BY_ORDER, watchList);
    }

    protected MarketPriceRequest createMarketPriceRequest()
    {
        return new MarketByOrderRequest();
    }
    
    protected int decode(Msg msg, DecodeIterator dIter, DataDictionary dictionary)
    {
        //decode market by price response
        StringBuilder fieldValue = new StringBuilder("\n");
        getItemName(msg, fieldValue);
        if (msg.msgClass() == MsgClasses.REFRESH)
            fieldValue.append((((RefreshMsg)msg).state()).toString() + "\n");

        //level 2 market by price is a map of field lists
        int ret;
        if ((ret = map.decode(dIter)) != CodecReturnCodes.SUCCESS)
        {
            System.out.println("Map.Decode() failed with return code: " + ret);
            return ret;
        }

        //decode set definition database
        if (map.checkHasSetDefs())
        {
            /*
             * decode set definition - should be field set definition
             */
            /*
             * this needs to be passed in when we decode each field list
             */
            localFieldSetDefDb.clear();
            ret = localFieldSetDefDb.decode(dIter);
            if (ret != CodecReturnCodes.SUCCESS)
            {
                System.out.println("DecodeLocalFieldSetDefDb() failed: <" + CodecReturnCodes.toString(ret) + ">");
                return ret;
            }
        }

        if (map.checkHasSummaryData())
        {
            ret = decodeSummaryData(dIter, dictionary, fieldValue);
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
        }

        ret = decodeMap(dIter, dictionary, fieldValue);
        if (ret != CodecReturnCodes.SUCCESS)
            return ret;
        System.out.println(fieldValue.toString());
        return CodecReturnCodes.SUCCESS;
    }

    private int decodeMap(DecodeIterator dIter, DataDictionary dictionary, StringBuilder fieldValue)
    {
        String actionString;
        int ret;
        /* decode the map */
        while ((ret = mapEntry.decode(dIter, mapKey)) != CodecReturnCodes.END_OF_CONTAINER)
        {
            if (ret != CodecReturnCodes.SUCCESS)
            {
                System.out.println("DecodeMapEntry() failed with return code: " + ret);
                return ret;
            }

            
            //convert the action to a string for display purposes
            switch (mapEntry.action())
            {
                case MapEntryActions.UPDATE:
                    actionString = "UPDATE";
                    break;
                case MapEntryActions.ADD:
                    actionString = "ADD";
                    break;
                case MapEntryActions.DELETE:
                    actionString = "DELETE";
                    break;
                default:
                    actionString = "Unknown";

            }
            //print out the key
            if (mapKey.length() > 0)
            {
                fieldValue.append("ORDER ID: " + mapKey.toString() + "\nACTION: "
                            + actionString + "\n");
            }

            //there is not any payload in delete actions
            if (mapEntry.action() != MapEntryActions.DELETE)
            {
                ret = decodeFieldList(dIter, dictionary, fieldValue);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
            }
        }
        return CodecReturnCodes.SUCCESS;
    }

    private int decodeFieldList(DecodeIterator dIter, DataDictionary dictionary, StringBuilder fieldValue)
    {
        //decode field list
        int ret = fieldList.decode(dIter, localFieldSetDefDb);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            System.out.println("DecodeFieldList() failed: <" + CodecReturnCodes.toString(ret) + ">");
            return ret;
        }

        //decode each field entry in list
        while ((ret = fieldEntry.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER)
        {
            if (ret != CodecReturnCodes.SUCCESS)
            {
                System.out.println("DecodeFieldEntry() failed: <" + CodecReturnCodes.toString(ret) + ">");
                return ret;
            }

            ret = decodeFieldEntry(fieldEntry, dIter, dictionary, fieldValue);
            if (ret != CodecReturnCodes.SUCCESS)
            {
                System.out.println("decodeFieldEntry() failed");
                return ret;
            }
            fieldValue.append("\n");
        }

        return CodecReturnCodes.SUCCESS;
    }

    private int decodeSummaryData(DecodeIterator dIter, DataDictionary dictionary, StringBuilder fieldValue)
    {
       int ret = fieldList.decode(dIter, localFieldSetDefDb);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            System.out.println("DecodeFieldList failed: <" + CodecReturnCodes.toString(ret) + ">");
            return ret;
        }

        fieldValue.append("SUMMARY DATA\n");
        //decode each field entry in list
        while ((ret = fieldEntry.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER)
        {
            if (ret != CodecReturnCodes.SUCCESS)
            {
                System.out.println("DecodeFieldEntry failed: <" + CodecReturnCodes.toString(ret) + ">");
                return ret;
            }

            ret = decodeFieldEntry(fieldEntry, dIter, dictionary, fieldValue);
            if (ret != CodecReturnCodes.SUCCESS)
            {
                System.out.println("decodeFieldEntry() failed");
                return ret;
            }
            fieldValue.append("\n");
        }

        return CodecReturnCodes.SUCCESS;
    }
}
