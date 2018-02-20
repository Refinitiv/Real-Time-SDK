package com.thomsonreuters.upa.examples.edfexamples.common;



import com.thomsonreuters.upa.codec.DataDictionary;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.Msg;
import com.thomsonreuters.upa.codec.MsgClasses;
import com.thomsonreuters.upa.examples.edfexamples.common.EDFWatchList.Item;
import com.thomsonreuters.upa.shared.rdm.marketprice.MarketPriceRequest;
import com.thomsonreuters.upa.rdm.DomainTypes;

/**
 * This is the market price handler for the UPA consumer application. It
 * provides methods for sending the market price request(s) to a provider and
 * processing the response(s). Methods for decoding a field entry from a
 * response, and closing market price streams are also provided.
 * 
 */
public class MarketPriceHandler extends MarketDataHandler
{
    public MarketPriceHandler()
    {
         super(DomainTypes.MARKET_PRICE);
    }

    protected MarketPriceRequest createMarketDataRequest()
    {
        return new MarketPriceRequest();
    }

    protected int decode(Msg msg, DecodeIterator dIter, Item item, DataDictionary dictionary)
    {
        StringBuilder outputString = new StringBuilder();
        int ret;
        
        if (msg.msgClass() == MsgClasses.UPDATE)
            outputString.append("Received UpdateMsg for stream " + msg.streamId() + "\n");
        if (msg.msgClass() == MsgClasses.REFRESH)
            outputString.append("Received RefreshMsg for stream " + msg.streamId() + "\n");
        
        ret = decodeFieldList(dIter, dictionary, outputString);
        System.out.println(outputString.toString());
        
        return ret;
    }

}
