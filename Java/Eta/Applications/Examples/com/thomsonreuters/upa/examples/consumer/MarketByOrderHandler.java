package com.thomsonreuters.upa.examples.consumer;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataDictionary;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.LocalFieldSetDefDb;
import com.thomsonreuters.upa.codec.Map;
import com.thomsonreuters.upa.codec.MapEntry;
import com.thomsonreuters.upa.codec.MapEntryActions;
import com.thomsonreuters.upa.codec.Msg;
import com.thomsonreuters.upa.codec.MsgClasses;
import com.thomsonreuters.upa.codec.RefreshMsg;
import com.thomsonreuters.upa.examples.rdm.marketbyorder.MarketByOrderRequest;
import com.thomsonreuters.upa.examples.rdm.marketprice.MarketPriceRequest;
import com.thomsonreuters.upa.rdm.DomainTypes;

/**
 * This is the market by order handler for the UPA consumer application. It
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
