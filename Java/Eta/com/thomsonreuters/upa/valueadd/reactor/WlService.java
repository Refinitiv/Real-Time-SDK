package com.thomsonreuters.upa.valueadd.reactor;

import java.util.LinkedList;

import com.thomsonreuters.upa.valueadd.common.VaNode;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryMsgFactory;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.Service;

/* Watchlist service that contains necessary information regarding the service. */
public class WlService extends VaNode
{
    Service _rdmService = DirectoryMsgFactory.createService();
    long _numOutstandingRequests; // number of outstanding requests for service
    
    /* waiting request list to handle cases where request could not be submitted with
     * directory stream up but the open window wasn't open */
    LinkedList<WlRequest> _waitingRequestList = new LinkedList<WlRequest>();
    
    /* watchlist streams associated with this service */
    LinkedList<WlStream> _streamList = new LinkedList<WlStream>();    
    
    /* Returns the RDM service. Use to get or set RDM service. */
    Service rdmService()
    {
        return _rdmService;
    }

    /* Returns the number of outstanding requests. */
    long numOutstandingRequests()
    {
        return _numOutstandingRequests;
    }

    /* Sets the number of outstanding requests. */
    void numOutstandingRequests(long numOutstandingRequests)
    {
        _numOutstandingRequests = numOutstandingRequests;
    }
    
    /* Returns the waiting request list. Handle cases where request could not be
     * submitted with directory stream up but the open window wasn't open. */
    LinkedList<WlRequest> waitingRequestList()
    {
        return _waitingRequestList;
    }
    
    /* Returns the list of streams associated with this service. */
    LinkedList<WlStream> streamList()
    {
        return _streamList;
    }

    /* Clears the object for re-use. */
    void clear()
    {
        _numOutstandingRequests = 0;
        WlRequest wlRequest = null;
        while ((wlRequest = _waitingRequestList.poll()) != null)
        {
            wlRequest.returnToPool();
        }
        WlStream wlStream = null;
        while ((wlStream = _streamList.poll()) != null)
        {
            wlStream.returnToPool();
        }
    }    
}
