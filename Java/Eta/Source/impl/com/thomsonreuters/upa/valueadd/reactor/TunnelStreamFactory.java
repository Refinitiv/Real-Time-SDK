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

	private TunnelStreamFactory()
	{
		throw new AssertionError();
	}
	
    /** Internal use only. Creates a queue request message. */
    public static QueueRequest createQueueRequest()
    {
        return new QueueRequestImpl();
    }

    /** Internal use only. Creates a queue close message. */
    public static QueueClose createQueueClose()
    {
        return new QueueCloseImpl();
    }

    /** Internal use only. Creates a queue refresh message. */
    public static QueueRefresh createQueueRefresh()
    {
        return new QueueRefreshImpl();
    }

    /** Internal use only. Creates a queue status message. */
    public static QueueStatus createQueueStatus()
    {
        return new QueueStatusImpl();
    }
    
    /** Internal use only. Creates a queue data message. */
    public static QueueData createQueueData()
    {
        return new QueueDataImpl();
    }

    /** Internal use only. Creates a queue acknowledgement message. */
    public static QueueAck createQueueAck()
    {
        return new QueueAckImpl();
    }
    
    /** Internal use only. Creates a queue dataExpired message. */
    public static QueueDataExpired createQueueDataExpired()
    {
        return new QueueDataExpiredImpl();
    }    
}
	
	

