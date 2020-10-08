package com.refinitiv.eta.valueadd.domainrep.rdm.queue;

import com.refinitiv.eta.valueadd.domainrep.rdm.MsgBase;

/**
 * The RDM Queue Base Message. This RDM dictionary messages may be reused or
 * pooled in a single collection via their common {@link QueueMsg} interface
 * and re-used as a different {@link QueueMsgType}. QueueMsgType
 * member may be set to one of these to indicate the specific QueueMsg
 * class.
 * 
 * @see QueueClose
 * @see QueueRefresh
 * @see QueueRequest
 * @see QueueStatus
 * @see QueueAck
 * @see QueueData
 * @see QueueDataExpired
 * 
 * @see QueueMsgFactory - Factory for creating RDM login messages
 * 
 * @see QueueMsgType
 */
public interface QueueMsg extends MsgBase
{	
    /**
     * Queue message type. These are defined per-message class basis for queue
     * domain.
     * 
     * @see QueueClose
     * @see QueueRefresh
     * @see QueueRequest
     * @see QueueStatus
     * @see QueueAck
     * @see QueueData
     * @see QueueDataExpired
     * 
     * @return queue message type.
     */	
    public QueueMsgType rdmMsgType();
    
	/**
     * Sets the domain type of the queue data message.
     * @param domainType type of the queue data message
     */	
    public void domainType(int domainType);
    
	/**
     * Sets the service id of the queue data message.
     * @param serviceId of the queue data message
     */	    
    public void serviceId(int serviceId);
    
	/**
     * Retrieves the service id of the queue data message.
     * @return the service id of the queue data message
     */	    
    public int serviceId();	
}
