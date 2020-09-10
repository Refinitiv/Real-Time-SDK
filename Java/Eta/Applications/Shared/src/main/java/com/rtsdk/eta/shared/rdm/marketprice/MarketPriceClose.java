package com.rtsdk.eta.shared.rdm.marketprice;

import com.rtsdk.eta.codec.CloseMsg;
import com.rtsdk.eta.codec.CodecFactory;
import com.rtsdk.eta.codec.CodecReturnCodes;
import com.rtsdk.eta.codec.DataTypes;
import com.rtsdk.eta.codec.DecodeIterator;
import com.rtsdk.eta.codec.EncodeIterator;
import com.rtsdk.eta.codec.Msg;
import com.rtsdk.eta.codec.MsgClasses;
import com.rtsdk.eta.rdm.DomainTypes;
import com.rtsdk.eta.valueadd.domainrep.rdm.MsgBaseImpl;

/**
 * The market price close message. Used by an OMM Consumer or OMM
 * Non-Interactive Provider to encode/decode a market price close message.
 */
public class MarketPriceClose extends MsgBaseImpl
{
    private int domainType;

    public MarketPriceClose()
    {
        super();
        domainType = DomainTypes.MARKET_PRICE;
    }

    public void clear()
    {
        super.clear();
        domainType = DomainTypes.MARKET_PRICE;
    }

    public int encode(EncodeIterator encodeIter)
    {
        closeMsg.clear();
        closeMsg.msgClass(MsgClasses.CLOSE);
        closeMsg.streamId(streamId());
        closeMsg.domainType(domainType);
        closeMsg.containerType(DataTypes.NO_DATA);

        return closeMsg.encode(encodeIter);
    }

    public int decode(DecodeIterator dIter, Msg msg)
    {
        clear();
        if (msg.msgClass() != MsgClasses.CLOSE)
            return CodecReturnCodes.FAILURE;

        streamId(msg.streamId());

        return CodecReturnCodes.SUCCESS;
    }

    private final static String eolChar = "\n";
    private final static String tabChar = "\t";

    public String toString()
    {
        StringBuilder toStringBuilder = super.buildStringBuffer();
        toStringBuilder.insert(0, "MarketPriceClose: \n");

        toStringBuilder.append(tabChar);
        toStringBuilder.append("domain: ");
        toStringBuilder.append(domainType());
        toStringBuilder.append(eolChar);
        return toStringBuilder.toString();
    }

    public int domainType()
    {
        return domainType;
    }

    public void domainType(int domainType)
    {
        this.domainType = domainType;
    }

    private CloseMsg closeMsg = (CloseMsg)CodecFactory.createMsg();

}