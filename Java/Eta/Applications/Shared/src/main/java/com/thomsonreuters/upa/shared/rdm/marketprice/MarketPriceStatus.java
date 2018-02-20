package com.thomsonreuters.upa.shared.rdm.marketprice;

import java.nio.ByteBuffer;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataTypes;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.Msg;
import com.thomsonreuters.upa.codec.MsgClasses;
import com.thomsonreuters.upa.codec.State;
import com.thomsonreuters.upa.codec.StatusMsg;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.MsgBaseImpl;

/**
 * The Market price Status. Used by an OMM Provider to indicate changes to the
 * Market price stream.
 */
public class MarketPriceStatus extends MsgBaseImpl
{
    private State state;
    private int flags;
    private int domainType;
    
    private final static String eol = System.getProperty("line.separator");
    private final static String tab = "\t";
    private StatusMsg statusMsg = (StatusMsg)CodecFactory.createMsg();           
    
    /**
     * Instantiates a new market price status.
     */
    public MarketPriceStatus()
    {
        state = CodecFactory.createState();
        streamId(1);
    }
    
    /**
     * Fully copies a {@link MarketPriceStatus}.
     * 
     * @param destStatusMsg The resulting copy of the RDM Market price Status
     * 
     * @return UPA return value
     */
    public int copy(MarketPriceStatus destStatusMsg)
    {
        destStatusMsg.streamId(streamId());
        if(checkHasState())
        {
            destStatusMsg.state().streamState(this.state.streamState());
            destStatusMsg.state().dataState(this.state.dataState());
            destStatusMsg.state().code(this.state.code());
            if(this.state.text().length() >  0)
            {
                ByteBuffer byteBuffer = ByteBuffer.allocate(this.state.text().length());
                this.state.text().copy(byteBuffer);
                destStatusMsg.state().text().data(byteBuffer);
            }    
            destStatusMsg.applyHasState();
        }
        
       
        return CodecReturnCodes.SUCCESS;
    }
    
    /**
     * The Market price Status Status flags. Populated by {@link MarketPriceStatusFlags}.
     *
     * @param flags the flags
     */
    public void flags(int flags)
    {
        this.flags = flags;
    }
    
    /**
     * The Market price Status Status flags. Populated by {@link MarketPriceStatusFlags}.
     * 
     * @return flags
     */
    public int flags()
    {
        return flags;
    }
    
    /**
     * Clears the current contents of the message and prepares it for re-use.
     */
    public void clear()
    {
        super.clear();
        flags = 0;
        state.clear();
        streamId(1);
    }

    /**
     * Encode a Market price status message.
     * 
     * @param encodeIter The Encode Iterator
     * 
     * @return {@link CodecReturnCodes}
     */
    public int encode(EncodeIterator encodeIter)
    {
        statusMsg.clear();
        statusMsg.streamId(streamId());
        statusMsg.containerType(DataTypes.NO_DATA);
        statusMsg.msgClass(MsgClasses.STATUS);
        statusMsg.domainType(domainType());
        
        if(checkPrivateStream())
            statusMsg.applyPrivateStream();
        
        if(checkHasState())
        {
            statusMsg.applyHasState();
            statusMsg.state().dataState(state().dataState());
            statusMsg.state().streamState(state().streamState());
            statusMsg.state().code(state().code());
            statusMsg.state().text(state().text());
        }
       
        int ret = statusMsg.encode(encodeIter);
        if (ret < CodecReturnCodes.SUCCESS)
            return ret;

        return CodecReturnCodes.SUCCESS;
    }
    
    /**
     * Decode a Market price status message.
     * 
     * @param dIter The Decode Iterator
     * @param msg Message to decode
     * 
     * @return {@link CodecReturnCodes}
     */
    public int decode(DecodeIterator dIter, Msg msg)
    {
        clear();
        if (msg.msgClass() != MsgClasses.STATUS)
             return CodecReturnCodes.FAILURE;
        
        
        StatusMsg statusMsg = (StatusMsg) msg;
        streamId(msg.streamId());
        
        if(statusMsg.checkPrivateStream())
            statusMsg.applyPrivateStream();

        if(statusMsg.checkHasState())
        {
            state().streamState(statusMsg.state().streamState());
            state().dataState(statusMsg.state().dataState());
            state().code(statusMsg.state().code());
            if(statusMsg.state().text().length() >  0)
            {
                Buffer text = statusMsg.state().text();
                this.state.text().data(text.data(), text.position(), text.length());
            }    
            applyHasState();
        }
        
       
        return CodecReturnCodes.SUCCESS;
    }

    /**
     * The current state of the market price stream.
     * 
     * @return state.
     */
    public State state()
    {
        return state;
    }
    
    /**
     * Checks the presence of state field.
     * 
     * This flag can also be bulk-get by {@link #flags()}
     * 
     * @return true - if state field is present, false - if not.
     */
    public boolean checkHasState()
    {
        return (flags() & MarketPriceStatusFlags.HAS_STATE) != 0;
    }
    
    /**
     * Applies state presence flag.
     * 
     * This flag can also be bulk-set by {@link #flags(int)}
     */
    public void applyHasState()
    {
        flags |= MarketPriceStatusFlags.HAS_STATE;
    }
    
    /**
     * Checks the presence of private stream flag.
     * 
     * This flag can also be bulk-get by {@link #flags()}
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkPrivateStream()
    {
        return (flags() & MarketPriceStatusFlags.PRIVATE_STREAM) != 0;
    }
    
    /**
     * Applies private stream flag.
     * 
     * This flag can also be bulk-set by {@link #flags(int)}
     */
    public void applyPrivateStream()
    {
        flags |= MarketPriceStatusFlags.PRIVATE_STREAM;
    }
    
    /* (non-Javadoc)
     * @see com.thomsonreuters.upa.valueadd.domainrep.rdm.MsgBaseImpl#toString()
     */
    public String toString()
    {
        StringBuilder stringBuf = super.buildStringBuffer();
        stringBuf.insert(0, "MarketPriceStatus: \n");
     
        if (checkHasState())
        {
            stringBuf.append(tab);
            stringBuf.append("state: ");
            stringBuf.append(state());
            stringBuf.append(eol);
        }

        return stringBuf.toString();
    }

    /**
     * Domain type.
     *
     * @return domain type
     */
    public int domainType()
    {
        return domainType;
    }

    /**
     * Domain type.
     *
     * @param domainType the domain type
     */
    public void domainType(int domainType)
    {
        this.domainType = domainType;
    }
}