package com.thomsonreuters.upa.examples.rdm.marketbyorder;

import com.thomsonreuters.upa.codec.CloseMsg;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataTypes;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.Msg;
import com.thomsonreuters.upa.codec.MsgClasses;
import com.thomsonreuters.upa.rdm.DomainTypes;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.MsgBaseImpl;

/**
 * The market by order close message. Used by an OMM Consumer, OMM
 * Non-Interactive Provider and OMM Interactive provider to encode/decode a
 * market by order close message.
 */
public class MarketByOrderClose extends MsgBaseImpl
{
    private CloseMsg closeMsg = (CloseMsg)CodecFactory.createMsg();
    private final static String eolChar = "\n";
    private final static String tabChar = "\t";

    public MarketByOrderClose()
    {
        super();
    }

    /**
     * Clears the current contents of this object and prepares it for re-use.
     */
    public void clear()
    {
        super.clear();
    }

    /**
     * Encode a market by order close message.
     * 
     * @param encodeIter The Encode Iterator
     * 
     * @return {@link CodecReturnCodes}
     */
    @Override
    public int encode(EncodeIterator encodeIter)
    {
        closeMsg.clear();
        closeMsg.msgClass(MsgClasses.CLOSE);
        closeMsg.streamId(streamId());
        closeMsg.domainType(DomainTypes.MARKET_BY_ORDER);
        closeMsg.containerType(DataTypes.NO_DATA);

        return closeMsg.encode(encodeIter);
    }

    /**
     * Decode a UPA message into a market by order close message.
     * 
     * @param dIter The Decode Iterator
     * 
     * @return UPA return value
     */
    @Override
    public int decode(DecodeIterator dIter, Msg msg)
    {
        clear();
        if (msg.msgClass() != MsgClasses.CLOSE)
            return CodecReturnCodes.FAILURE;

        streamId(msg.streamId());

        return CodecReturnCodes.SUCCESS;
    }

    public String toString()
    {
        StringBuilder toStringBuilder = super.buildStringBuffer();
        toStringBuilder.insert(0, "MarketByOrderClose: \n");

        toStringBuilder.append(tabChar);
        toStringBuilder.append("domain: ");
        toStringBuilder.append(DomainTypes.toString(DomainTypes.MARKET_BY_ORDER));
        toStringBuilder.append(eolChar);
        return toStringBuilder.toString();
    }
    
    @Override
    public int domainType()
    {
        return DomainTypes.MARKET_BY_ORDER;
    }
}
