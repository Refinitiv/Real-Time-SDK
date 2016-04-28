package com.thomsonreuters.upa.valueadd.domainrep.rdm.queue;

import com.thomsonreuters.upa.valueadd.reactor.TunnelStreamFactory;


/**
 * Factory for RDM queue messages.
 */
public class QueueMsgFactory
{
    /**
     * This class is not instantiated
     */  
    private QueueMsgFactory()
    {
        throw new AssertionError();
    }
    
    /**
     * Creates a queue request message.
     * 
     * @return {@link QueueRequest} object
     */
    public static QueueRequest createQueueRequest()
    {
        return TunnelStreamFactory.createQueueRequest();
    }

    /**
     * Creates a queue close message.
     * 
     * @return {@link QueueClose} object
     */
    public static QueueClose createQueueClose()
    {
        return TunnelStreamFactory.createQueueClose();
    }

    /**
     * Creates a queue refresh message.
     * 
     * @return {@link QueueRefresh} object
     */
    public static QueueRefresh createQueueRefresh()
    {
        return TunnelStreamFactory.createQueueRefresh();
    }

    /**
     * Creates a queue status message.
     * 
     * @return {@link QueueStatus} object
     */
    public static QueueStatus createQueueStatus()
    {
    	return TunnelStreamFactory.createQueueStatus();
    }
    
    /**
     * Creates a queue data message.
     * 
     * @return {@link QueueData} object
     */
    public static QueueData createQueueData()
    {
    	return TunnelStreamFactory.createQueueData();
    }

    /**
     * Creates a queue acknowledgment message.
     * 
     * @return {@link QueueAck} object
     */
    public static QueueAck createQueueAck()
    {
    	return TunnelStreamFactory.createQueueAck();
    }
    
    /**
     * Creates a queue dataExpired message.
     * 
     * @return {@link QueueData} object
     */
    public static QueueDataExpired createQueueDataExpired()
    {
    	return TunnelStreamFactory.createQueueDataExpired();
    }    
}
