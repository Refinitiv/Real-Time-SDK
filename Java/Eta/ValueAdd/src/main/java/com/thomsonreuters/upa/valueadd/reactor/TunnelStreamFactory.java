package com.thomsonreuters.upa.valueadd.reactor;

import com.thomsonreuters.upa.valueadd.domainrep.rdm.queue.QueueAck;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.queue.QueueClose;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.queue.QueueData;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.queue.QueueDataExpired;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.queue.QueueRefresh;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.queue.QueueRequest;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.queue.QueueStatus;

/** Factory for TunnelStream package objects. Internal use only. */
public class TunnelStreamFactory {

	/**
	 * Instantiates a new tunnel stream factory.
	 */
	private TunnelStreamFactory()
	{
		throw new AssertionError();
	}
	
    /**
     *  Internal use only. Creates a queue request message.
     *
     * @return the queue request
     */
    public static QueueRequest createQueueRequest()
    {
        return new QueueRequestImpl();
    }

    /**
     *  Internal use only. Creates a queue close message.
     *
     * @return the queue close
     */
    public static QueueClose createQueueClose()
    {
        return new QueueCloseImpl();
    }

    /**
     *  Internal use only. Creates a queue refresh message.
     *
     * @return the queue refresh
     */
    public static QueueRefresh createQueueRefresh()
    {
        return new QueueRefreshImpl();
    }

    /**
     *  Internal use only. Creates a queue status message.
     *
     * @return the queue status
     */
    public static QueueStatus createQueueStatus()
    {
        return new QueueStatusImpl();
    }
    
    /**
     *  Internal use only. Creates a queue data message.
     *
     * @return the queue data
     */
    public static QueueData createQueueData()
    {
        return new QueueDataImpl();
    }

    /**
     *  Internal use only. Creates a queue acknowledgement message.
     *
     * @return the queue ack
     */
    public static QueueAck createQueueAck()
    {
        return new QueueAckImpl();
    }
    
    /**
     *  Internal use only. Creates a queue dataExpired message.
     *
     * @return the queue data expired
     */
    public static QueueDataExpired createQueueDataExpired()
    {
        return new QueueDataExpiredImpl();
    }    
}
	
	

