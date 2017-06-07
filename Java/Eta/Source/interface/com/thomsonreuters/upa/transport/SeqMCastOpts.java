package com.thomsonreuters.upa.transport;

/**
 * Options used for configuring Sequenced Multicast specific transport options (ConnectionTypes.SEQUENCED_MULTICAST).
 * 
 * @see ConnectOptions
 */

public interface SeqMCastOpts
{
    /**
     * Only used with connectionType of SEQUENCED_MULTICAST.
     * 
     * @param size The maximum length of the message
     */
    public void maxMsgSize(int size);

    /**
     * Returns the maximum message size.
     * 
     * @return maxMsgSize
     */
    public int maxMsgSize();
    
    /**
     * When combined with the sender's IP address and port, this is used to uniquely identify a sequenced multicast channel.  
     * This is a 16-bit unsigned value, with a valid range of 0-65535.
     *
     * @param id the id
     */
    public void instanceId(int id);
    
    /**
     * Returns the instance ID;.
     *
     * @return instanceId
     */
    public int instanceId();
    
}
