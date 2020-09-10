package com.rtsdk.eta.valueadd.domainrep.rdm;

import com.rtsdk.eta.codec.DecodeIterator;
import com.rtsdk.eta.codec.EncodeIterator;
import com.rtsdk.eta.codec.Msg;

/**
 * This message structure contains the information that is common across all RDM
 * Message formats. It is included in all Value Added RDM Components.
 */
public interface MsgBase
{
    /**
     * The Stream Id for the given item.
     * 
     * @return the streamId
     */
    public int streamId();

    /**
     * The Stream Id for the given item.
     *
     * @param streamId the streamId to set
     * 
     */
    public void streamId(int streamId);
    
    /**
     * Returns the domain type of the RDM message.
     *
     * @return the int
     */
    public int domainType();

    /**
     * Encode an RDM message.
     * 
     * @param eIter The Encode Iterator
     * 
     * @return UPA return value
     */
    public int encode(EncodeIterator eIter);

    /**
     * Decode a UPA message into an RDM message.
     *
     * @param dIter The Decode Iterator
     * @param msg the msg
     * @return UPA return value
     */
    public int decode(DecodeIterator dIter, Msg msg);

    /**
     * Clears the current contents of the message and prepares it for re-use.
     */
    public void clear();
}