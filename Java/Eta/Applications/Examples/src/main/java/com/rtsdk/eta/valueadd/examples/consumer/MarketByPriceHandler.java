package com.rtsdk.eta.valueadd.examples.consumer;

import com.rtsdk.eta.codec.Buffer;
import com.rtsdk.eta.codec.CodecFactory;
import com.rtsdk.eta.codec.CodecReturnCodes;
import com.rtsdk.eta.codec.DataDictionary;
import com.rtsdk.eta.codec.DecodeIterator;
import com.rtsdk.eta.codec.LocalFieldSetDefDb;
import com.rtsdk.eta.codec.Map;
import com.rtsdk.eta.codec.MapEntry;
import com.rtsdk.eta.codec.MapEntryActions;
import com.rtsdk.eta.shared.rdm.marketbyprice.MarketByPriceRequest;
import com.rtsdk.eta.shared.rdm.marketprice.MarketPriceRequest;
import com.rtsdk.eta.rdm.DomainTypes;

/*
 * This is the market by price handler for the UPA Value Add consumer
 * application. It provides method decoding a field entry from a response.
 * 
 * Methods for sending the market by price request(s) to a provider,
 * processing the response(s) and closing streams are inherited from
 * MarketPriceHandler.
 */
class MarketByPriceHandler extends MarketPriceHandler
{
    private Map map = CodecFactory.createMap();
    private MapEntry mapEntry = CodecFactory.createMapEntry();
    private Buffer mapKey = CodecFactory.createBuffer();
    private LocalFieldSetDefDb localFieldSetDefDb = CodecFactory.createLocalFieldSetDefDb();
    
    MarketByPriceHandler(StreamIdWatchList watchList)
    {
        super(DomainTypes.MARKET_BY_PRICE, watchList);
    }

    protected MarketPriceRequest createMarketPriceRequest()
    {
        return new MarketByPriceRequest();
    }
    
    @Override
    public int decodePayload(DecodeIterator dIter, DataDictionary dictionary, StringBuilder fieldValue)
    {
        int ret = map.decode(dIter);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            System.out.println("DecodeMap() failed with return code: " + ret);
            return ret;
        }

        if (map.checkHasSetDefs())
        {
            localFieldSetDefDb.clear();
            ret = localFieldSetDefDb.decode(dIter);
            if (ret != CodecReturnCodes.SUCCESS)
            {
                System.out.println("DecodeLocalFieldSetDefDb() failed");
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
        int ret;
        while ((ret = mapEntry.decode(dIter, mapKey)) != CodecReturnCodes.END_OF_CONTAINER)
        {
            if (ret != CodecReturnCodes.SUCCESS)
            {
                System.out.println("DecodeMapEntry() failed: <" + CodecReturnCodes.toString(ret) + ">");
                return ret;
            }

            if (mapKey.length() > 0)
            {
                fieldValue.append("PRICE POINT: " + mapKey.toString() + "\nACTION: "
                            + getMapActionString() + "\n");
            }

            if (mapEntry.action() != MapEntryActions.DELETE)
            {
                ret = decodeUpdateFieldList(dIter, dictionary, fieldValue);
                if (ret != CodecReturnCodes.SUCCESS)
                    return ret;
            }
        }
        return CodecReturnCodes.SUCCESS;
    }

    private String getMapActionString()
    {
        String actionString;
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
        return actionString;
    }

    private int decodeUpdateFieldList(DecodeIterator dIter, DataDictionary dictionary, StringBuilder fieldValue)
    {
        //decode field list
        int ret = fieldList.decode(dIter, localFieldSetDefDb);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            System.out.println("DecodeFieldList() failed with return code: " + ret);
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
            System.out.println("decodeFieldList() failed: <" + CodecReturnCodes.toString(ret) + ">");
            return ret;
        }

        fieldValue.append("SUMMARY DATA\n");
        
        //decode each field entry in list
        while ((ret = fieldEntry.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER)
        {
            if (ret != CodecReturnCodes.SUCCESS)
            {
                System.out.println("decodeFieldEntry() failed: <" + CodecReturnCodes.toString(ret) + ">");

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
