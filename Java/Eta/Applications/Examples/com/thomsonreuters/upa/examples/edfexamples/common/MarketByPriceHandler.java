package com.thomsonreuters.upa.examples.edfexamples.common;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataDictionary;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.GlobalFieldSetDefDb;
import com.thomsonreuters.upa.codec.LocalFieldSetDefDb;
import com.thomsonreuters.upa.codec.Map;
import com.thomsonreuters.upa.codec.MapEntry;
import com.thomsonreuters.upa.codec.MapEntryActions;
import com.thomsonreuters.upa.codec.Msg;
import com.thomsonreuters.upa.codec.MsgClasses;
import com.thomsonreuters.upa.codec.RefreshMsg;
import com.thomsonreuters.upa.examples.edfexamples.common.EDFWatchList.Item;
import com.thomsonreuters.upa.examples.rdm.marketbyprice.MarketByPriceRequest;
import com.thomsonreuters.upa.examples.rdm.marketprice.MarketPriceRequest;
import com.thomsonreuters.upa.rdm.DomainTypes;

/**
 * This is the market by price handler for the UPA consumer application. It
 * provides method decoding a field entry from a response.
 * 
 * Methods for sending the market by price request(s) to a provider,
 * processing the response(s) and closing streams are inherited from
 * MarketPriceHandler.
 * 
 */
public class MarketByPriceHandler extends MarketDataHandler
{
    private Map map = CodecFactory.createMap();
    private MapEntry mapEntry = CodecFactory.createMapEntry();
    private Buffer mapKey = CodecFactory.createBuffer();
    
    protected LocalFieldSetDefDb localSetDefDb = CodecFactory.createLocalFieldSetDefDb();
    protected GlobalFieldSetDefDb globalSetDefDb = null;
    
    
    public MarketByPriceHandler()
    {
        super(DomainTypes.MARKET_BY_PRICE);
    }

    protected MarketPriceRequest createMarketDataRequest()
    {
        return new MarketByPriceRequest();
    }
    
    @Override
    protected int decode(Msg msg, DecodeIterator dIter, Item item, DataDictionary dictionary)
    {
        StringBuilder outputString = new StringBuilder("\n");
        outputString.append(item.getName().toString());
        if (msg.msgClass() == MsgClasses.REFRESH)
            outputString.append((((RefreshMsg)msg).state()).toString() + "\n");
        
        if(globalSetDefDb != null)
        {
            dIter.setGlobalFieldSetDefDb(globalSetDefDb);
        }

        int ret = map.decode(dIter);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            System.out.println("DecodeMap() failed with return code: " + ret);
            return ret;
        }

        if (map.checkHasSetDefs())
        {
            localSetDefDb.clear();
            ret = localSetDefDb.decode(dIter);
            if (ret != CodecReturnCodes.SUCCESS)
            {
                System.out.println("DecodeLocalFieldSetDefDb() failed");
                return ret;
            }
        }
        if (map.checkHasSummaryData())
        {
            ret = decodeSummaryData(dIter, dictionary, outputString);
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
        }

        ret = decodeMap(dIter, dictionary, outputString);
        if (ret != CodecReturnCodes.SUCCESS)
            return ret;

        System.out.println(outputString.toString());
        return CodecReturnCodes.SUCCESS;
    }

    private int decodeMap(DecodeIterator dIter, DataDictionary dictionary, StringBuilder outputString)
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
                outputString.append("PRICE POINT: " + mapKey.toString() + "\nACTION: "
                            + getMapActionString() + "\n");
            }

            if (mapEntry.action() != MapEntryActions.DELETE)
            {
                ret = decodeFieldList(dIter, dictionary, outputString);
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

    private int decodeSummaryData(DecodeIterator dIter, DataDictionary dictionary, StringBuilder outputString)
    {
        int ret = fieldList.decode(dIter, localSetDefDb);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            System.out.println("decodeFieldList() failed: <" + CodecReturnCodes.toString(ret) + ">");
            return ret;
        }

        outputString.append("SUMMARY DATA\n");
        
        //decode each field entry in list
        while ((ret = fieldEntry.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER)
        {
            if (ret != CodecReturnCodes.SUCCESS)
            {
                System.out.println("decodeFieldEntry() failed: <" + CodecReturnCodes.toString(ret) + ">");

                return ret;
            }

            ret = decodeFieldEntry(fieldEntry, dIter, dictionary, outputString);
            if (ret != CodecReturnCodes.SUCCESS)
            {
                System.out.println("decodeFieldEntry() failed");
                return ret;
            }
            outputString.append("\n");
        }

        return CodecReturnCodes.SUCCESS;
    }
    
}