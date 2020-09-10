package com.rtsdk.eta.shared.rdm.marketprice;

import com.rtsdk.eta.codec.CodecFactory;
import com.rtsdk.eta.codec.DataTypes;
import com.rtsdk.eta.codec.Msg;
import com.rtsdk.eta.codec.MsgClasses;
import com.rtsdk.eta.codec.UpdateMsg;
import com.rtsdk.eta.rdm.DomainTypes;
import com.rtsdk.eta.rdm.InstrumentNameTypes;

/**
 * The market price update message. Used by an OMM Consumer, OMM Interactive provider and OMM Non-Interactive provider to
 * encode/decode a market price update message.
 */
public class MarketPriceUpdate extends MarketPriceResponseBase
{
    private UpdateMsg updateMsg = (UpdateMsg)CodecFactory.createMsg();

    public MarketPriceUpdate()
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
    
    
    @Override
    public Msg encodeMsg()
    {
        updateMsg.clear();
        updateMsg.msgClass(MsgClasses.UPDATE);
        updateMsg.streamId(streamId());
        updateMsg.domainType(DomainTypes.MARKET_PRICE);
        updateMsg.containerType(DataTypes.FIELD_LIST);
        
        if (checkHasServiceId())
        {
            /* attrib info */
            updateMsg.applyHasMsgKey();
            /* service Id */
            updateMsg.msgKey().serviceId(serviceId());
            updateMsg.msgKey().applyHasServiceId();
            /* name */
            updateMsg.msgKey().name(itemName());
            updateMsg.msgKey().applyHasName();
            /* name type */
            updateMsg.msgKey().nameType(InstrumentNameTypes.RIC);
            updateMsg.msgKey().applyHasNameType();
        }
        
        return updateMsg;
    }
    
    public String toString()
    {
        StringBuilder stringBuf = super.buildStringBuffer();
        stringBuf.insert(0, "MarketPriceUpdate: \n");
        return stringBuf.toString();
    }
    
    @Override
    public int domainType()
    {
        return DomainTypes.MARKET_PRICE;
    }
}