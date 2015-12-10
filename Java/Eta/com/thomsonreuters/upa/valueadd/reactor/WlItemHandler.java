package com.thomsonreuters.upa.valueadd.reactor;

import java.util.HashMap;
import java.util.LinkedList;
import java.util.Map;

import com.thomsonreuters.upa.codec.CloseMsg;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.CopyMsgFlags;
import com.thomsonreuters.upa.codec.DataStates;
import com.thomsonreuters.upa.codec.DataTypes;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.GenericMsg;
import com.thomsonreuters.upa.codec.Msg;
import com.thomsonreuters.upa.codec.MsgClasses;
import com.thomsonreuters.upa.codec.PostMsg;
import com.thomsonreuters.upa.codec.Qos;
import com.thomsonreuters.upa.codec.QosTimeliness;
import com.thomsonreuters.upa.codec.QosRates;
import com.thomsonreuters.upa.codec.RefreshMsg;
import com.thomsonreuters.upa.codec.RequestMsg;
import com.thomsonreuters.upa.codec.RequestMsgFlags;
import com.thomsonreuters.upa.codec.StatusMsg;
import com.thomsonreuters.upa.codec.StreamStates;
import com.thomsonreuters.upa.codec.UpdateMsg;
import com.thomsonreuters.upa.rdm.DomainTypes;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.MsgBase;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.dictionary.DictionaryMsg;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.Service;
import com.thomsonreuters.upa.valueadd.reactor.WlRequest.State;

public class WlItemHandler implements WlHandler
{
    Watchlist _watchlist;
    boolean _directoryStreamOpen;
    
    // used for requests that are submitted when directory stream is not up
    // two tables are required - one is indexed by service id and one is indexed by service name
    HashMap<Integer,LinkedList<WlRequest>> _pendingRequestByIdTable = new HashMap<Integer,LinkedList<WlRequest>>();
    HashMap<String,LinkedList<WlRequest>> _pendingRequestByNameTable = new HashMap<String,LinkedList<WlRequest>>();
    // pool of pending request lists (to avoid GC)
    LinkedList<LinkedList<WlRequest>> _pendingRequestListPool = new LinkedList<LinkedList<WlRequest>>();
    
    ReactorErrorInfo _errorInfo = ReactorFactory.createReactorErrorInfo();
    ReactorSubmitOptions _submitOptions = ReactorFactory.createReactorSubmitOptions();
    
    // list of streams
    LinkedList<WlStream> _streamList = new LinkedList<WlStream>();
    
    // table that maps item aggregation key to streams
    HashMap<WlItemAggregationKey,WlStream> _itemAggregationKeytoWlStreamTable = new HashMap<WlItemAggregationKey,WlStream>();
    
    WlItemAggregationKey _tempItemAggregationKey = ReactorFactory.createWlItemAggregationKey();
    RequestMsg _tempItemAggregationRequest = (RequestMsg)CodecFactory.createMsg();
    
    Qos _defaultQos = CodecFactory.createQos();
    Qos _tempMatchedQos = CodecFactory.createQos();
    
    StatusMsg _statusMsg = (StatusMsg)CodecFactory.createMsg();

    UpdateMsg _updateMsg = (UpdateMsg)CodecFactory.createMsg();

    // pool of StatusMsgs
    LinkedList<StatusMsg> _statusMsgPool = new LinkedList<StatusMsg>();
    
    // List of StatusMsgs to send when dispatch is called
    LinkedList<StatusMsg> _statusMsgDispatchList = new LinkedList<StatusMsg>();
    
    // List of streams with pending messages to send
    LinkedList<WlStream> _pendingSendMsgList = new LinkedList<WlStream>();
    
    // List of user requests to re-submit upon dispatch that had request timeout
    LinkedList<WlRequest> _requestTimeoutList = new LinkedList<WlRequest>();    

    WlItemHandler(Watchlist watchlist)
    {
        _watchlist = watchlist;
        _defaultQos.clear();
        _defaultQos.timeliness(QosTimeliness.REALTIME);
        _defaultQos.rate(QosRates.TICK_BY_TICK);
        _statusMsg.msgClass(MsgClasses.STATUS);
        _updateMsg.msgClass(MsgClasses.UPDATE);
    }
    
    @Override
    public int submitRequest(WlRequest wlRequest, RequestMsg requestMsg, boolean isReissue, ReactorSubmitOptions submitOptions, ReactorErrorInfo errorInfo)
    {
    	 if (!_watchlist.loginHandler().userLoginStreamProvided())
    	 {
             return _watchlist.reactor().populateErrorInfo(errorInfo,
                     CodecReturnCodes.INVALID_DATA,
                     "WlItemHandler.submitRequest",
                     "Application login information has to be provided first before an item request");    		 
    	 }
    	
    	
        if (!isReissue)
        {
            // validate Qos
            if (requestMsg.checkHasQos() && requestMsg.qos() != null &&
                (requestMsg.qos().timeliness() == QosTimeliness.UNSPECIFIED ||
                requestMsg.qos().rate() == QosRates.UNSPECIFIED ||
                requestMsg.qos().timeliness() > QosTimeliness.DELAYED ||
                requestMsg.qos().rate() > QosRates.TIME_CONFLATED))
            {
                return _watchlist.reactor().populateErrorInfo(errorInfo,
                                                              CodecReturnCodes.INVALID_DATA,
                                                              "WlItemHandler.submitRequest",
                                                              "Request has invalid QoS (Timeliness: " + requestMsg.qos().timeliness() + 
                                                              ", Rate: " + requestMsg.qos().rate() + ").");
            }
            
            // validate WorstQos
            if (requestMsg.checkHasWorstQos() && requestMsg.worstQos() != null &&
                (requestMsg.worstQos().timeliness() == QosTimeliness.UNSPECIFIED ||
                requestMsg.worstQos().rate() == QosRates.UNSPECIFIED ||
                requestMsg.worstQos().timeliness() > QosTimeliness.DELAYED ||
                requestMsg.worstQos().rate() > QosRates.TIME_CONFLATED))
            {
                return _watchlist.reactor().populateErrorInfo(errorInfo,
                                                              CodecReturnCodes.INVALID_DATA,
                                                              "WlItemHandler.submitRequest",
                                                              "Request has invalid worst QoS (Timeliness: " + requestMsg.worstQos().timeliness() + 
                                                              ", Rate: " + requestMsg.worstQos().rate() + ").");
            }

            // handle request
            return handleRequest(wlRequest, requestMsg, submitOptions, true, errorInfo);
        }
        else
        {
            // handle reissue
            return handleReissue(wlRequest, requestMsg, submitOptions, errorInfo);
        }
    }

    /* Handles a user request. */
    int handleRequest(WlRequest wlRequest, RequestMsg requestMsg, ReactorSubmitOptions submitOptions, boolean sendNow, ReactorErrorInfo errorInfo)
    {
        int ret = ReactorReturnCodes.SUCCESS;

        _tempMatchedQos.clear();
        
        // retrieve service for service id/name
        WlService wlService = null;
        if (submitOptions.serviceName() != null)
        {
            wlService = _watchlist.directoryHandler().service(submitOptions.serviceName());
        }
        else
        {
            if (requestMsg.msgKey().checkHasServiceId())
            {
                wlService = _watchlist.directoryHandler().service(requestMsg.msgKey().serviceId());
            }
        }

        // first determine if item can be opened at this time
        Qos staticQos = (wlRequest.hasStaticQos() ? wlRequest.matchedQos() : null);
        if (wlService != null && canItemBeOpened(requestMsg, submitOptions, _tempMatchedQos, staticQos, wlService.rdmService(), errorInfo))
        {
            // next check if window is open for service (don't check if dictionary domain)
            if (requestMsg.domainType() == DomainTypes.DICTIONARY || isWindowOpen(wlService))
            {
                // find aggregation stream
                WlStream wlStream = null;
                if ((wlStream = findItemAggregationStream(requestMsg, _tempMatchedQos, submitOptions)) == null)
                {
                    // item cannot be aggregated, create new stream
                    wlStream = createNewStream(requestMsg);
                    
                    // copy temporary item aggregation key from findItemAggregationStream() into new one
                    WlItemAggregationKey itemAggregationKey = ReactorFactory.createWlItemAggregationKey();
                    _tempItemAggregationKey.copy(itemAggregationKey);
                    
                    // update temporary item aggregation request and send
                    _tempItemAggregationRequest.clear();
                    requestMsg.copy(_tempItemAggregationRequest, CopyMsgFlags.ALL_FLAGS);
                    // stream id is that of watchlist stream
                    _tempItemAggregationRequest.streamId(wlStream.streamId());
                    // Service id is that of item aggregation key
                    _tempItemAggregationRequest.msgKey().applyHasServiceId();
                    _tempItemAggregationRequest.msgKey().serviceId(itemAggregationKey.msgKey().serviceId());
                    // qos is that of item aggregation key if not dictionary domain
                    if (requestMsg.domainType() != DomainTypes.DICTIONARY)
                    {
                        _tempItemAggregationRequest.applyHasQos();
                        itemAggregationKey.qos().copy(_tempItemAggregationRequest.qos());
                        // clear worst qos flag
                        _tempItemAggregationRequest.flags(_tempItemAggregationRequest.flags() & ~RequestMsgFlags.HAS_WORST_QOS);
                    }
                    // priority is that of request or 1/1 if not present
                    _tempItemAggregationRequest.applyHasPriority();
                    if (requestMsg.checkHasPriority())
                    {
                        _tempItemAggregationRequest.priority().priorityClass(requestMsg.priority().priorityClass());
                        _tempItemAggregationRequest.priority().count(requestMsg.priority().count());
                    }
                    else // request has no priority, use default of 1/1
                    {
                        _tempItemAggregationRequest.priority().priorityClass(1);
                        _tempItemAggregationRequest.priority().count(1);
                    }
                    // reset MSG_KEY_IN_UPDATES flag if set
                    if (_tempItemAggregationRequest.checkMsgKeyInUpdates())
                    {
                        _tempItemAggregationRequest.flags(_tempItemAggregationRequest.flags() & ~RequestMsgFlags.MSG_KEY_IN_UPDATES);
                    }
    
                    // increment number of snapshots pending
                    if (!requestMsg.checkStreaming())
                    {
                        wlStream.numSnapshotsPending(wlStream.numSnapshotsPending() + 1);
                    }
                    
                    // send message to stream
                    if (sendNow)
                    {
                        // send now
                        ret = wlStream.sendMsg(_tempItemAggregationRequest, submitOptions, _errorInfo);
                    }
                    else // don't send now, just set request message on stream
                    {
                        ret = wlStream.requestMsg(_tempItemAggregationRequest);
                    }
                    
                    // increment number of outstanding requests if not dictionary domain
                    if (requestMsg.domainType() != DomainTypes.DICTIONARY)
                    {
                        wlService.numOutstandingRequests(wlService.numOutstandingRequests() + 1);
                    }
    
                    // update tables and lists if send successful
                    if (ret >= ReactorReturnCodes.SUCCESS)
                    {
                        // save stream info
                        wlRequest.streamInfo().serviceName(submitOptions.serviceName());
                        wlRequest.streamInfo().userSpecObject(submitOptions.requestMsgOptions().userSpecObj());
                        
                        // save matched Qos if not dictionary
                        if (requestMsg.domainType() != DomainTypes.DICTIONARY)
                        {
                            _tempMatchedQos.copy(wlRequest.matchedQos());
                        }
    
                        // add to list of streams
                        _streamList.add(wlStream);
                        
                        // add stream to watchlist table
                        _watchlist.streamIdtoWlStreamTable().put(wlStream.streamId(), wlStream);
                        
                        // add to _itemAggregationKeytoWlStreamTable
                        _itemAggregationKeytoWlStreamTable.put(itemAggregationKey, wlStream);
                        
                        // associate stream and item aggregation key
                        wlStream.itemAggregationKey(itemAggregationKey);
                        
                        // associate stream and service
                        wlStream.wlService(wlService);
                        
                        // add stream to service's stream list
                        wlService.streamList().add(wlStream);
                        
                        // update request state to REFRESH_PENDING
                        wlRequest.state(WlRequest.State.REFRESH_PENDING);
                        
                        // add to fanout list
                        wlStream.userRequestList().add(wlRequest);
                        
                        // set stream associated with request
                        wlRequest.stream(wlStream);
                        
                        // associate stream and service
                        wlStream.wlService(wlService);                    
    
                        // if not sendNow, add stream to pending send message list
                        if (!sendNow)
                        {
                            _pendingSendMsgList.add(wlStream);
                        }
                    }
                    else
                    {
                        // return watchlist stream to pool
                        wlStream.returnToPool();
                    }
                }
                else // item can be aggregated
                {
                    // save stream info
                    wlRequest.streamInfo().serviceName(submitOptions.serviceName());
                    wlRequest.streamInfo().userSpecObject(submitOptions.requestMsgOptions().userSpecObj());
    
                    // save matched Qos if not dictionary
                    if (requestMsg.domainType() != DomainTypes.DICTIONARY)
                    {
                        _tempMatchedQos.copy(wlRequest.matchedQos());
                    }
    
                    /* add request to stream only if not in the middle of snapshot or
                       multi-part refresh */
                    if (wlStream.numSnapshotsPending() == 0 &&
                        !wlStream.multiPartRefreshPending())
                    {
                        // add request to stream
                        wlStream.userRequestList().add(wlRequest);
                    
                        // update request state to REFRESH_PENDING
                        wlRequest.state(WlRequest.State.REFRESH_PENDING);
    
                        // set stream associated with request
                        wlRequest.stream(wlStream);
    
                        // retrieve request from stream
                        RequestMsg streamRequestMsg = wlStream.requestMsg();
                        
                        // increment number of snapshots pending
                        if (!requestMsg.checkStreaming())
                        {
                            wlStream.numSnapshotsPending(wlStream.numSnapshotsPending() + 1);
                        }
    
                        // update priority
                        if (requestMsg.checkHasPriority())
                        {
                            // request has priority
                            
                            // use priorityClass of request if greater than existing one
                            if (requestMsg.priority().priorityClass() > streamRequestMsg.priority().priorityClass())
                            {
                                streamRequestMsg.priority().priorityClass(requestMsg.priority().priorityClass());
                            }
                            
                            // add priorityCount to that of existing one
                            streamRequestMsg.priority().count(streamRequestMsg.priority().count() +
                                                              requestMsg.priority().count());
                        }
                        else // request has no priority, assume default of 1/1
                        {
                            streamRequestMsg.priority().count(streamRequestMsg.priority().count() + 1);
                        }
                        
                        // send message to stream
                        if (sendNow)
                        {
                            ret = wlStream.sendMsg(streamRequestMsg, submitOptions, _errorInfo);
                        }
                        
                        // increment number of outstanding requests if not dictionary domain
                        if (requestMsg.domainType() != DomainTypes.DICTIONARY)
                        {
                            wlService.numOutstandingRequests(wlService.numOutstandingRequests() + 1);
                        }
                    }
                    else // currently in the middle of snapshot or multi-part refresh
                    {
                        // cannot open item at this time, add to waiting request list
    
                        // add to waiting request list for stream
                        wlStream.waitingRequestList().add(wlRequest);
                    }
                }
            }
            else // service window not open
            {
                // add to waiting request list for service
                wlService.waitingRequestList().add(wlRequest);
            }
        }
        else // cannot open item at this time, add to pending request table if not DICTIONARY domain
        {
            if (requestMsg.domainType() != DomainTypes.DICTIONARY)
            {
                // save stream info
                wlRequest.streamInfo().serviceName(submitOptions.serviceName());
                wlRequest.streamInfo().userSpecObject(submitOptions.requestMsgOptions().userSpecObj());

                if ( _watchlist.loginHandler().supportSingleOpen() ||  _watchlist.loginHandler().supportAllowSuspectData())
                	addToPendingRequestTable(wlRequest, submitOptions);
                
                if (wlService == null)
                {
                    errorInfo.error().text("Service not available");
                }

                // queue status message to send on next dispatch call
                queueStatusForDispatch(requestMsg.streamId(), requestMsg.domainType(), errorInfo.error().text());
            }
            else
            {
                // DICTIONARY domain cannot be recovered
                ret = _watchlist.reactor().populateErrorInfo(errorInfo,
                                                              ReactorReturnCodes.FAILURE,
                                                              "WlItemHandler.handleRequest",
                                                              "No service available for this dictionary request.");
            }
        }

        return ret;
    }
    
    /* Handles a user reissue. */
    int handleReissue(WlRequest wlRequest, RequestMsg requestMsg, ReactorSubmitOptions submitOptions, ReactorErrorInfo errorInfo)
    {
        int ret = ReactorReturnCodes.SUCCESS;

        /* handle reissue only if streaming flag has not changed
         * (it's an error if the streaming flag is changed) */
        if (requestMsg.checkStreaming() == wlRequest.requestMsg().checkStreaming())
        {
            // retrieve request from stream
            RequestMsg streamRequestMsg = wlRequest.stream().requestMsg();
            
            // update priority only if present on reissue request
            if (requestMsg.checkHasPriority())
            {
                // update priorityClass only if changed
                if (requestMsg.priority().priorityClass() != wlRequest.requestMsg().priority().priorityClass())
                {
                    // use priorityClass of reissue request if greater than existing one
                    if (requestMsg.priority().priorityClass() > streamRequestMsg.priority().priorityClass())
                    {
                        streamRequestMsg.priority().priorityClass(requestMsg.priority().priorityClass());
                    }
                }
                
                // update priorityCount only if changed
                if (requestMsg.priority().count() != wlRequest.requestMsg().priority().count())
                {
                    // get difference between reissue request priority count and request's previous priority count 
                    int priorityCountDiff = requestMsg.priority().count() - wlRequest.requestMsg().priority().count();
                    
                    // add priorityCount difference to that of existing one
                    streamRequestMsg.priority().count(streamRequestMsg.priority().count() +
                                                      priorityCountDiff);
                }
            }
            
            // if dictionary domain, update MsgKey filter if changed
            if (requestMsg.domainType() == DomainTypes.DICTIONARY)
            {
                if (requestMsg.msgKey().filter() != streamRequestMsg.msgKey().filter())
                {
                    streamRequestMsg.msgKey().filter(requestMsg.msgKey().filter());
                }
            }

            // send reissue if stream is open
            if (wlRequest.stream().state().streamState() == StreamStates.OPEN)
            {
                // handle reissue only if not in the middle of snapshot or multi-part refresh
                if (wlRequest.stream().numSnapshotsPending() == 0 &&
                    !wlRequest.stream().multiPartRefreshPending())
                {
                    // update request state to REFRESH_PENDING if refresh is desired
                    if (!requestMsg.checkNoRefresh())
                    {
                        wlRequest.state(WlRequest.State.REFRESH_PENDING);
                    }
                    
                    // send message to stream
                    ret = wlRequest.stream().sendMsg(streamRequestMsg, submitOptions, _errorInfo);
                }
                else
                {
                    // add to waiting request list
                    wlRequest.stream().waitingRequestList().add(wlRequest);
                }
            }
        }
        else // streaming flag has changed
        {
            // streaming flag cannot be changed for a reissue
            ret = _watchlist.reactor().populateErrorInfo(errorInfo,
                                                          ReactorReturnCodes.FAILURE,
                                                          "WlItemHandler.handleRequest",
                                                          "Streaming flag cannot be changed for a reissue.");
        }

        return ret;
    }
    
    /* Determines if an item can be opened. */
    boolean canItemBeOpened(RequestMsg requestMsg, ReactorSubmitOptions submitOptions, Qos matchedQos, Qos staticQos, Service service, ReactorErrorInfo errorInfo)
    {
        boolean ret = false;
        
        if (isServiceUpAndAcceptingRequests(service))
        {
            if (isCapabilitySupported(requestMsg.domainType(), service))
            {
                // check Qos if not DICTIONARY domain
                if (requestMsg.domainType() != DomainTypes.DICTIONARY)
                {
                    if (staticQos == null)
                    {
                        // no static qos specified, find matching Qos
                        if (requestMsg.checkHasQos())
                        {
                            Qos worstQos = null;
                            Qos qos = requestMsg.qos();
                            if (requestMsg.checkHasWorstQos())
                            {
                                worstQos = requestMsg.worstQos();
                            }
                            if (isQosSupported(qos, worstQos, service, matchedQos))
                            {
                                // qos and/or qos range is supported
                                ret = true;
                            }
                            else
                            {
                                errorInfo.error().text("Service does not provide a matching QoS");                
                            }                    
                        }
                        else // no Qos specified
                        {
                            // best effort qos is supported
                            service.info().bestQos().copy(matchedQos);
                            
                            ret = true;
                        }
                    }
                    else // static qos specified
                    {
                        // use static qos for matching qos
                        if (isQosSupported(staticQos, null, service, matchedQos))
                        {
                            // qos and/or qos range is supported
                            ret = true;
                        }
                        else
                        {
                            errorInfo.error().text("Service does not provide a matching QoS");                
                        }
                    }
                }
                else // DICTIONARY domain
                {
                    // DICTIONARY domain supported since capability supported
                    ret = true;
                }
            }
            else
            {
                errorInfo.error().text("Capability not supported");                
            }                    
        }
        else
        {
            errorInfo.error().text("Service not up");                
        }
        
        return ret;
    }

    /* Finds an item aggregation stream for a user request. */
    WlStream findItemAggregationStream(RequestMsg requestMsg, Qos matchedQos, ReactorSubmitOptions submitOptions)
    {
        // determine if a new stream is needed or if existing stream can be used
        _tempItemAggregationKey.clear();
        _tempItemAggregationKey.msgKey(requestMsg.msgKey());
        if (submitOptions.serviceName() != null)
        {
            // set service id in item aggregation key if requested by service name
            int serviceId = _watchlist.directoryHandler().serviceId(submitOptions.serviceName());
            _tempItemAggregationKey.msgKey().serviceId(serviceId);
        }
        _tempItemAggregationKey.domainType(requestMsg.domainType());
        _tempItemAggregationKey.qos(matchedQos);
        
        return _itemAggregationKeytoWlStreamTable.get(_tempItemAggregationKey);
    }

    /* Adds a user request to the pending request table. */
    void addToPendingRequestTable(WlRequest wlRequest, ReactorSubmitOptions submitOptions)
    {
        // add only if single open is true, otherwise user must perform recovery themselves
    	// retrieve pending request list for this service id/name if one exists
    	LinkedList<WlRequest> pendingRequestList = null;
    	if (submitOptions.serviceName() != null)
    	{
    		pendingRequestList = _pendingRequestByNameTable.get(submitOptions.serviceName());
    	}
    	else
    	{
    		pendingRequestList = _pendingRequestByIdTable.get(wlRequest.requestMsg().msgKey().serviceId());
    	}

    	// add to pending request list
    	if (pendingRequestList != null)
    	{
    		// pending request list exists, just add to existing list
    		pendingRequestList.add(wlRequest);
    	}
    	else // pending request list doesn't exist
    	{
    		// create a pending request list
    		pendingRequestList = _pendingRequestListPool.poll();
    		if (pendingRequestList == null)
    		{
    			pendingRequestList = new LinkedList<WlRequest>();
    		}
                
    		// add pending request to list
    		pendingRequestList.add(wlRequest);
                
    		// add pending request list to table
    		if (submitOptions.serviceName() != null)
    		{
    			_pendingRequestByNameTable.put(submitOptions.serviceName(), pendingRequestList);
    		}
    		else
    		{
    			_pendingRequestByIdTable.put(wlRequest.requestMsg().msgKey().serviceId(), pendingRequestList);
    		}
            
    	}
    }

    /* Creates a new stream. */
    WlStream createNewStream(RequestMsg requestMsg)
    {
        // create stream
        WlStream wlStream = ReactorFactory.createWlStream();
        wlStream.handler(this);
        wlStream.watchlist(_watchlist);
        wlStream.streamId(_watchlist.nextStreamId());
        wlStream.domainType(requestMsg.domainType());
        
        return wlStream;
    }

    /* Determines if a service is up and accepting requests. */
    boolean isServiceUpAndAcceptingRequests(Service service)
    {
        return service.checkHasState() &&
               (!service.state().checkHasAcceptingRequests() || service.state().acceptingRequests() == 1) &&
                service.state().serviceState() == 1;
    }

    /* Determines if a service supports a capability. */
    boolean isCapabilitySupported(int domainType, Service service)
    {
        boolean ret = false;
        
        if (service.checkHasInfo())
        {
            for (int i = 0; i < service.info().capabilitiesList().size(); i++)
            {
                if(service.info().capabilitiesList().get(i) == domainType)
                {
                    ret = true;
                    break;
                }
            }
        }
        
        return ret;
    }

    /* Determines if a service supports a qos or qos range. */
    boolean isQosSupported(Qos qos, Qos worstQos, Service service, Qos matchedQos)
    {
        boolean ret = false;
        
        if (service.checkHasInfo())
        {
            if (service.info().checkHasQos())
            {
                // service has qos
                for (int i = 0; i < service.info().qosList().size(); i++)
                {
                    Qos serviceQos = service.info().qosList().get(i);
                    if (worstQos == null)
                    {
                        // no worst qos, determine if request qos supported by service
                        if (serviceQos.equals(qos))
                        {
                            ret = true;
                            serviceQos.copy(matchedQos);
                            break;
                        }
                    }
                    else // worstQos specified
                    {
                        if (serviceQos.isInRange(qos, worstQos))
                        {
                            if (serviceQos.isBetter(matchedQos))
                            {
                                ret = true;
                                serviceQos.copy(matchedQos);
                            }
                        }
                    }
                }
            }
            else // service has no qos
            {
                // determine if qos matches default of Realtime, Tick-By-Tick
                if (worstQos == null)
                {
                    ret = _defaultQos.equals(qos);
                }
                else // worstQos specified
                {
                    ret = _defaultQos.isInRange(qos, worstQos);
                }
                
                if (ret == true)
                {
                    _defaultQos.copy(matchedQos);
                }
            }
        }
        
        // set isDynamic flag to that of requested qos before returning
        if (ret == true)
        {
            matchedQos.dynamic(qos.isDynamic());
        }
        
        return ret;
    }
    
    /* Determines if a service's window is open for another request. */
    boolean isWindowOpen(WlService wlService)
    {
        boolean ret = true;
        
        if (_watchlist.watchlistOptions().obeyOpenWindow() &&
            wlService.rdmService().checkHasLoad() && wlService.rdmService().load().checkHasOpenWindow())
        {
            long openWindow = wlService.rdmService().load().openWindow();
            if (openWindow == 0 || // open window of 0 means window is not open
                wlService.numOutstandingRequests() == openWindow)
            {
                ret = false;
            }
        }

        return ret;
    }

    @Override
    public int submitMsg(WlRequest wlRequest, Msg msg, ReactorSubmitOptions submitOptions, ReactorErrorInfo errorInfo)
    {
        int ret = ReactorReturnCodes.SUCCESS;
        
        switch (msg.msgClass())
        {
            case MsgClasses.CLOSE:
       	
            	WlStream wlStream = wlRequest.stream();
            	if ( wlStream == null) break;
            	
            	for (int i = 0; i < wlStream.userRequestList().size(); i++)
            	{
            		WlRequest wlRequestInList = wlRequest.stream().userRequestList().get(i);
                
            		if (wlRequestInList.requestMsg().streamId() != wlRequest.requestMsg().streamId())
            			continue;
            		else
            		{
            			wlRequest.stream().userRequestList().remove(i);
            			
            			if (wlRequest.stream().userRequestList().size() == 0 )
            			{            				            				
            				msg.streamId(wlRequest.stream().streamId());
            		
                            if ((ret = wlRequest.stream().sendMsg(msg, submitOptions, errorInfo)) < ReactorReturnCodes.SUCCESS)
                            {
                                return ret;
                            }

                            wlRequest.stream().close();                            
            			}
            			else
            			{
            	            // update priority
                            // reduce stream priority count by that in user request being closed
                            int streamPriorityCount =  wlRequest.stream().requestMsg().checkHasPriority() ? 
                            		wlRequest.stream().requestMsg().priority().count() : 1;
                            int userRequestPriorityCount = wlRequest.requestMsg().checkHasPriority() ? 
                            		wlRequest.requestMsg().priority().count() : 1;
                            wlRequest.stream().requestMsg().priority().count(streamPriorityCount - userRequestPriorityCount);                            
                            // resend 
                            wlRequest.stream().sendMsg(wlRequest.stream().requestMsg(), submitOptions, errorInfo);
            			}               		
            			
            			wlRequest.returnToPool();
            			break;
            		}
            	}
            	break;
            case MsgClasses.POST:
                if (_watchlist.loginHandler().supportPost())
                {                	
                    if (wlRequest.state() == WlRequest.State.OPEN)
                    {
                        return handlePost(wlRequest, msg, submitOptions, errorInfo);
                    }
                    else
                    { 
                        // cannot submit post when stream is not open
                        return _watchlist.reactor().populateErrorInfo(errorInfo,
                                                                      ReactorReturnCodes.INVALID_USAGE,
                                                                      "WlItemHandler.submitMsg",
                                                                      "Cannot submit PostMsg when stream not in open state.");
                    }
                }
                else
                {
                    return _watchlist.reactor().populateErrorInfo(errorInfo,
                                                                  ReactorReturnCodes.INVALID_USAGE,
                                                                  "WlItemHandler.submitMsg",
                                                                  "Posting not supported by provider");
                }
            case MsgClasses.GENERIC:
                if (wlRequest.state() == WlRequest.State.OPEN)
                {
                    // replace service id if message submitted with service name 
                    if (submitOptions.serviceName() != null &&
                        ((GenericMsg)msg).checkHasMsgKey() &&
                        ((GenericMsg)msg).msgKey().checkHasServiceId())
                    {
                        int serviceId = _watchlist.directoryHandler().serviceId(submitOptions.serviceName());
                        ((GenericMsg)msg).msgKey().serviceId(serviceId);
                    }
                    
                    // replace stream id with aggregated stream id
                    msg.streamId(wlRequest.stream().streamId());
                    
                    // send message
                    if ((ret = wlRequest.stream().sendMsg(msg, submitOptions, errorInfo)) < ReactorReturnCodes.SUCCESS)
                    {
                        return ret;
                    }
                }
                else
                {
                    // cannot submit generic message when stream is not open
                    return _watchlist.reactor().populateErrorInfo(errorInfo,
                                                                  ReactorReturnCodes.INVALID_USAGE,
                                                                  "WlItemHandler.submitMsg",
                                                                  "Cannot submit GenericMsg when stream not in open state.");
                }
                break;
            default:
                ret = _watchlist.reactor().populateErrorInfo(errorInfo,
                                                              ReactorReturnCodes.FAILURE,
                                                              "WlItemHandler.submitMsg",
                                                              "Invalid message class (" + msg.msgClass() + ") submitted to Watchlist item handler");
                break;
        }

        return ret;
    }

    /* Handles a post message. */
    int handlePost(WlRequest wlRequest, Msg msg, ReactorSubmitOptions submitOptions, ReactorErrorInfo errorInfo)
    {
        int ret = ReactorReturnCodes.SUCCESS;
        
        if (_watchlist.numOutstandingPosts() < _watchlist.watchlistOptions().maxOutstandingPosts())
        {
            // validate post submit
            if ((ret = wlRequest.stream().validatePostSubmit((PostMsg)msg, errorInfo)) != ReactorReturnCodes.SUCCESS)
            {
                return ret;
            }
            
            // replace service id if message submitted with service name 
            if (submitOptions.serviceName() != null && ((PostMsg)msg).msgKey().checkHasServiceId())
            {
                int serviceId = _watchlist.directoryHandler().serviceId(submitOptions.serviceName());
                ((PostMsg)msg).msgKey().serviceId(serviceId);
            }
            
            // send message
            // no need to replace stream id for post message here - that's done inside sendMsg()
            if ((ret = wlRequest.stream().sendMsg(msg, submitOptions, errorInfo)) < ReactorReturnCodes.SUCCESS)
            {
                return ret;
            }            
        }
        else
        {
            ret = _watchlist.reactor().populateErrorInfo(errorInfo,
                                                         ReactorReturnCodes.INVALID_USAGE,
                                                         "WlItemHandler.handlePost",
                                                         "maxOutstandingPosts limit reached.");
        }
        
        return ret;
    }

    /* Sends status message to a user. */
    int sendStatus(int streamId, int domainType, String text)
    {
        // populate StatusMsg
        _statusMsg.streamId(streamId);
        _statusMsg.domainType(domainType);
        _statusMsg.applyHasState();

        if (_watchlist.loginHandler().supportSingleOpen() || _watchlist.loginHandler().supportAllowSuspectData())
        {
            _statusMsg.state().streamState(StreamStates.OPEN);
            _statusMsg.state().dataState(DataStates.SUSPECT);
        }
        else
        {
            _statusMsg.state().streamState(StreamStates.CLOSED_RECOVER);
            _statusMsg.state().dataState(DataStates.SUSPECT);
        }
        _statusMsg.state().text().data(text);
  
        // callback user
        return callbackUser("WlItemHandler.sendStatus", _statusMsg, null, _errorInfo);
    }

    /* Queues a status message for sending on dispatch. */
    void queueStatusForDispatch(int streamId, int domainType, String text)
    {
        // get StatusMsg from pool
        StatusMsg statusMsg = _statusMsgPool.poll();
        if (statusMsg == null)
        {
            statusMsg = (StatusMsg)CodecFactory.createMsg();
        }
        
        // populate StatusMsg
        statusMsg.clear();
        statusMsg.msgClass(MsgClasses.STATUS);
        statusMsg.streamId(streamId);
        statusMsg.domainType(domainType);
        statusMsg.applyHasState();  
                        
        if ( _watchlist.loginHandler().supportSingleOpen() ||  _watchlist.loginHandler().supportAllowSuspectData())
        {
            statusMsg.state().streamState(StreamStates.OPEN);
            statusMsg.state().dataState(DataStates.SUSPECT);
        }
        else
        {
            statusMsg.state().streamState(StreamStates.CLOSED_RECOVER);
            statusMsg.state().dataState(DataStates.SUSPECT);
        }
        statusMsg.state().text().data(text);
        
        // add StatusMsg to dispatch list and trigger dispatch
        _statusMsgDispatchList.add(statusMsg);
        
        if (_statusMsgDispatchList.size() == 1)
        {
            // trigger dispatch only for first add to list
            _watchlist.reactor().sendWatchlistDispatchNowEvent(_watchlist.reactorChannel());
        }
    }
    
    @Override
    public int readMsg(WlStream wlStream, DecodeIterator dIter, Msg msg, ReactorErrorInfo errorInfo)
    {
        int ret = ReactorReturnCodes.SUCCESS;
        
        switch (msg.msgClass())
        {
            case MsgClasses.REFRESH:
                ret = readRefreshMsg(wlStream, dIter, msg, errorInfo);
                break;
            case MsgClasses.STATUS:
                ret =  readStatusMsg(wlStream, dIter, msg, errorInfo);
                break;
            case MsgClasses.UPDATE:
                ret =  readUpdateMsg(wlStream, dIter, msg, errorInfo);
                break;
            case MsgClasses.GENERIC:
                ret =  readGenericMsg(wlStream, dIter, msg, errorInfo);
                break;
            case MsgClasses.ACK:
                ret =  readAckMsg(wlStream, dIter, msg, errorInfo);
                break;
            default:
                ret = _watchlist.reactor().populateErrorInfo(errorInfo,
                                                              ReactorReturnCodes.FAILURE,
                                                              "WlItemHandler.readMsg",
                                                              "Invalid message class (" + msg.msgClass() + ") received by Watchlist directory handler");
                break;
        }
        
        // handle any state transition
        if (ret == ReactorReturnCodes.SUCCESS)
        {
            switch (wlStream.state().streamState())
            { 
                case StreamStates.CLOSED:
                	handleClose(wlStream, "item stream closed.");
                case StreamStates.CLOSED_RECOVER:
                	if (_watchlist.loginHandler().supportSingleOpen())
                		handleCloseRecover(wlStream, "item stream closed_recover.");
                	else 
                		handleClose(wlStream, "item stream closed");
                	break;
                case StreamStates.REDIRECTED:
                	handleClose(wlStream, "item stream redirected");
                    break;
                case StreamStates.OPEN:
                	if (msg.msgClass() == MsgClasses.STATUS
                		|| (msg.msgClass() == MsgClasses.REFRESH && wlStream.state().dataState() == DataStates.SUSPECT))
                    handleOpenStatus(wlStream, msg); 
                    break;
                default:
                    break;
            }
        }

        return ret;
    }

    /* Reads a refresh message. */
    int readRefreshMsg(WlStream wlStream, DecodeIterator dIter, Msg msg, ReactorErrorInfo errorInfo)
    {
        int ret = ReactorReturnCodes.SUCCESS;
        boolean isRefreshComplete = ((RefreshMsg)msg).checkRefreshComplete();
        
        // notify stream that response received if solicited
        if (((RefreshMsg)msg).checkSolicited())
        {
            wlStream.responseReceived();
        }
        
        // set state from refresh message
        ((RefreshMsg)msg).state().copy(wlStream.state());
        
        // decrement number of outstanding requests on service
        if (isRefreshComplete)
        {
            wlStream.wlService().numOutstandingRequests(wlStream.wlService().numOutstandingRequests() - 1);
        }

        // only process stream state of open here
        if (wlStream.state().streamState() == StreamStates.OPEN && wlStream.state().dataState() == DataStates.OK)
        {
            // fanout refresh message to user requests associated with the stream
            for (int i = 0; i < wlStream.userRequestList().size(); i++)
            {
                WlRequest wlRequest = wlStream.userRequestList().get(i);
                
                // only fanout if refresh is desired and refresh is unsolicited or to those whose state is awaiting refresh
                if (!wlRequest.requestMsg().checkNoRefresh() &&
                    (!((RefreshMsg)msg).checkSolicited() ||
                    wlRequest.state() == WlRequest.State.REFRESH_PENDING ||
                    wlRequest.state() == WlRequest.State.REFRESH_COMPLETE_PENDING))
                {
                    // update stream id in message to that of user request
                    msg.streamId(wlRequest.requestMsg().streamId());
                    
                    // callback user
                    if ((ret = callbackUser("WlItemHandler.readRefreshMsg", msg, null, errorInfo)) < ReactorCallbackReturnCodes.SUCCESS)
                    {
                        // break out of loop for error
                        break;
                    }
                    
                    // check refresh complete flag and change state of user request accordingly
                    if (isRefreshComplete)
                    {
                        // reset multi-part refresh pending flag
                        wlStream.multiPartRefreshPending(false);

                        if (wlRequest.requestMsg().checkStreaming() &&
                            ((RefreshMsg)msg).state().streamState() != StreamStates.NON_STREAMING)
                        {
                            // not snapshot or NON_STREAMING
                            
                            // change user request state to OPEN
                            wlRequest.state(WlRequest.State.OPEN);
                            
                            // set hasStaticQos flag if not dictionary and request's matched Qos.isDynamic is false
                            if (msg.domainType() != DomainTypes.DICTIONARY &&
                                !wlRequest.hasStaticQos() &&   
                                !wlRequest.matchedQos().isDynamic())
                            {
                                wlRequest.hasStaticQos(true);
                            }
                        }
                        else // snapshot request or NON_STREAMING
                        {
                            // if snapshot request, decrement number of snapshots pending
                            if (!wlRequest.requestMsg().checkStreaming())
                            {
                                wlStream.numSnapshotsPending(wlStream.numSnapshotsPending() - 1);
                            }
                            
                            // remove from list and return to pool
                            wlStream.userRequestList().remove(i);
                            wlRequest.returnToPool();
                            
                            // if no more requests in stream, close stream
                            if (wlStream.userRequestList().size() == 0 &&
                                wlStream.waitingRequestList().size() == 0 &&
                                wlStream.wlService().waitingRequestList().size() == 0)
                            {
                                wlStream.close();
                            }
                        }
                    }
                    else // multi-part refresh
                    {
                        // change user request state to REFRESH_COMPLETE_PENDING
                        wlRequest.state(WlRequest.State.REFRESH_COMPLETE_PENDING);
                        
                        // set multi-part refresh pending flag
                        wlStream.multiPartRefreshPending(true);
                        
                        // start another request timer for each part of multi-part refresh
                        wlStream.startRequestTimer(errorInfo);
                    }
                }
            }
            
            /* if longer waiting for snapshot or multi-part refresh,
               send requests in waiting request list */
            if (wlStream.waitingRequestList().size() > 0 &&
                wlStream.numSnapshotsPending() == 0 &&
                !wlStream.multiPartRefreshPending())
            {
                WlRequest waitingRequest = null;
                while((waitingRequest = wlStream.waitingRequestList().poll()) != null)
                {
                    _submitOptions.serviceName(waitingRequest.streamInfo().serviceName());
                    _submitOptions.requestMsgOptions().userSpecObj(waitingRequest.streamInfo().userSpecObject());
                    ret = handleRequest(waitingRequest, waitingRequest.requestMsg(), _submitOptions, true, _errorInfo);
                }
            }
            
            /* send next request in service's waiting request list */
            if (wlStream.wlService().waitingRequestList().size() > 0 && isRefreshComplete)
            {
                WlRequest waitingRequest = wlStream.wlService().waitingRequestList().poll();
                _submitOptions.serviceName(waitingRequest.streamInfo().serviceName());
                _submitOptions.requestMsgOptions().userSpecObj(waitingRequest.streamInfo().userSpecObject());
                ret = handleRequest(waitingRequest, waitingRequest.requestMsg(), _submitOptions, true, _errorInfo);
            }
        }
        
        return ret;
    }

    /* Reads a update message. */
    int readUpdateMsg(WlStream wlStream, DecodeIterator dIter, Msg msg, ReactorErrorInfo errorInfo)
    {
        int ret = ReactorReturnCodes.SUCCESS;
        
        // fanout update message to user requests associated with the stream
        for (int i = 0; i < wlStream.userRequestList().size(); i++)
        {
            Msg callbackMsg = msg;
            WlRequest wlRequest = wlStream.userRequestList().get(i);
            
            // only fanout to those whose state is OPEN or REFRESH_COMPLETE_PENDING
            if (wlRequest.state() == WlRequest.State.OPEN ||
                wlRequest.state() == WlRequest.State.REFRESH_COMPLETE_PENDING)
            {
                // update stream id in message to that of user request
                msg.streamId(wlRequest.requestMsg().streamId());
                
                // populate MsgKey if requested and update message doesn't have it
                if (wlRequest.requestMsg().checkMsgKeyInUpdates() &&
                    !((UpdateMsg)msg).checkHasMsgKey())
                {
                    _updateMsg.clear();
                    ((UpdateMsg)msg).copy(_updateMsg, CopyMsgFlags.ALL_FLAGS);
                    _updateMsg.applyHasMsgKey();
                    wlRequest.requestMsg().msgKey().copy(_updateMsg.msgKey());
                    callbackMsg = _updateMsg;
                }
                
                // callback user
                if ((ret = callbackUser("WlItemHandler.readUpdateMsg", callbackMsg, null, errorInfo)) < ReactorCallbackReturnCodes.SUCCESS)
                {
                    // break out of loop for error
                    break;
                }
            }
        }

        return ret;
    }

    /* Reads a status message. */
    int readStatusMsg(WlStream wlStream, DecodeIterator dIter, Msg msg, ReactorErrorInfo errorInfo)
    {
        // notify stream that response received
        wlStream.responseReceived();

        // set state from status message
        if (((StatusMsg)msg).checkHasState()) 
        	((StatusMsg)msg).state().copy(wlStream.state());

        return 0;
    }

    /* Reads a generic message. */
    int readGenericMsg(WlStream wlStream, DecodeIterator dIter, Msg msg, ReactorErrorInfo errorInfo)
    {
        int ret = ReactorReturnCodes.SUCCESS;
        
        // fanout generic message to user requests associated with the stream
        for (int i = 0; i < wlStream.userRequestList().size(); i++)
        {
            WlRequest wlRequest = wlStream.userRequestList().get(i);
            
            // only fanout to those whose state is OPEN, REFRESH_COMPLETE_PENDING or REFRESH_PENDING
            if (wlRequest.state() == WlRequest.State.OPEN ||
                wlRequest.state() == WlRequest.State.REFRESH_COMPLETE_PENDING ||
                wlRequest.state() == WlRequest.State.REFRESH_PENDING)
            {
                // update stream id in message to that of user request
                msg.streamId(wlRequest.requestMsg().streamId());
                
                // callback user
                if ((ret = callbackUser("WlItemHandler.readGenericMsg", msg, null, errorInfo)) < ReactorCallbackReturnCodes.SUCCESS)
                {
                    // break out of loop for error
                    break;
                }
            }
        }

        return ret;
    }

    /* Reads an Ack message. */
    int readAckMsg(WlStream wlStream, DecodeIterator dIter, Msg msg, ReactorErrorInfo errorInfo)
    {
        int ret = ReactorCallbackReturnCodes.SUCCESS;
        
        // handle the post Ack
        if (wlStream.handlePostAck(msg))
        {
            // call back user if ACK was processed
            ret = callbackUser("WlItemHandler.readAckMsg", msg, null, errorInfo);
        }
        
        return ret;
    }
    
    /* Dispatch all streams for the handler. */
    int dispatch(ReactorErrorInfo errorInfo)
    {
        int ret = ReactorReturnCodes.SUCCESS;
        
        // dispatch streams
        for (int i = 0; i < _streamList.size(); i++)
        {
            WlStream wlStream = _streamList.get(i);
            if ((ret = wlStream.dispatch(errorInfo)) < ReactorReturnCodes.SUCCESS)
            {
                return ret;
            }
        }
        
        // send any queued status messages to the user
        StatusMsg statusMsg = null;
        while ((statusMsg = _statusMsgDispatchList.poll()) != null)
        {
            // callback user
            ret = callbackUser("WlItemHandler.dispatch", statusMsg, null, errorInfo);
            
            // return StatusMsg to pool
            _statusMsgPool.add(statusMsg);
            
            if (ret < ReactorReturnCodes.SUCCESS)
            {
                return ret;
            }
        }
        
        // re-submit user requests that had request timeout
        WlRequest wlRequest = null;
        while ((wlRequest = _requestTimeoutList.poll()) != null)
        {
            _submitOptions.serviceName(wlRequest.streamInfo().serviceName());
            _submitOptions.requestMsgOptions().userSpecObj(wlRequest.streamInfo().userSpecObject());
            if ((ret = handleRequest(wlRequest, wlRequest.requestMsg(), _submitOptions, false, errorInfo)) < ReactorReturnCodes.SUCCESS)
            {
                return ret;
            }
        }
        
        // call sendMsg on all streams in pending stream send list
        WlStream wlStream = null;
        while((wlStream = _pendingSendMsgList.poll()) != null)
        {
            if ((ret = wlStream.sendMsg(wlStream.requestMsg(), _submitOptions, _errorInfo)) < ReactorReturnCodes.SUCCESS)
            {
                return ret;
            }
        }
        
        return ret;
    }
    
    /* Handles login stream open event. */
    int loginStreamOpen(ReactorErrorInfo errorInfo)
    {
        // TODO handle login stream open event
        return ReactorReturnCodes.SUCCESS;
    }

    /* Handles login stream closed event. */
    int loginStreamClosed()
    {
    	boolean singleOpen = _watchlist.loginHandler().supportSingleOpen();
    	if (singleOpen && _watchlist.loginHandler().wlStream().state().streamState() == StreamStates.CLOSED_RECOVER)
    		handleCloseRecover("login stream closed.");
    	else
    		handleClose("login stream closed"); 
        return ReactorReturnCodes.SUCCESS;
    }

    /* Handles directory stream open event. */
    int directoryStreamOpen()
    {
        _directoryStreamOpen = true;
        
        return ReactorReturnCodes.SUCCESS;
    }

    /* Handles directory stream closed event. */
    int directoryStreamClosed()
    {
        _directoryStreamOpen = false;
 
    	boolean singleOpen = _watchlist.loginHandler().supportSingleOpen();
    	if (singleOpen && _watchlist.directoryHandler()._stream.state().streamState() == StreamStates.CLOSED_RECOVER)
    		handleCloseRecover("directory stream closed");
    	else
    		handleClose("directory stream closed");
                   
        return ReactorReturnCodes.SUCCESS;
    }
    
    /* Handles pause all event. */
    int pauseAll()
    {
        // TODO handle pause all event
        return ReactorReturnCodes.SUCCESS;
    }

    /* Handles resume all event. */
    int resumeAll()
    {
        // TODO handle resume all event
        return ReactorReturnCodes.SUCCESS;
    }

    /* Handles channel up event. */
    void channelUp(ReactorErrorInfo errorInfo)
    {
   	   for (WlStream wlStream = _streamList.poll(); wlStream != null;)
   	   {    	   
   		   wlStream.channelUp();
   	   }
    }

    /* Handles channel down event. */
    void channelDown()
    {
    	boolean singleOpen = _watchlist.loginHandler().supportSingleOpen();
    	if (singleOpen) 
    		handleCloseRecover("channel down.");
    	else
    		handleClose("channel down");
    }
    
    /* Handles service added event. */
    int serviceAdded(WlService wlService)
    {
        int ret = ReactorReturnCodes.SUCCESS;
        LinkedList<WlRequest> pendingRequestList = null;
        
        // handle any pending requests
        // retrieve matching requests based on service id or service name
        pendingRequestList = _pendingRequestByIdTable.remove(wlService.rdmService().serviceId());
        if (pendingRequestList == null)
        {
            pendingRequestList = _pendingRequestByNameTable.remove(wlService.rdmService().info().serviceName().toString());
        }
        
        // handle request
        if (pendingRequestList != null)
        {
            WlRequest wlRequest = null;
            while((wlRequest = pendingRequestList.poll()) != null)
            {
                _submitOptions.serviceName(wlRequest.streamInfo().serviceName());
                _submitOptions.requestMsgOptions().userSpecObj(wlRequest.streamInfo().userSpecObject());
                if ((ret = handleRequest(wlRequest, wlRequest.requestMsg(), _submitOptions, false, _errorInfo)) < ReactorReturnCodes.SUCCESS)
                {
                    return ret;
                }
            }
            _pendingRequestListPool.add(pendingRequestList);
        }
        
        // call sendMsg on all streams in pending stream send list
        WlStream wlStream = null;
        while((wlStream = _pendingSendMsgList.poll()) != null)
        {
            if ((ret = wlStream.sendMsg(wlStream.requestMsg(), _submitOptions, _errorInfo)) < ReactorReturnCodes.SUCCESS)
            {
                return ret;
            }
        }
        
        return ret;
    }
    
    /* Handles service updated event. */
    int serviceUpdated(WlService wlService)
    {
        // TODO Auto-generated method stub
        return ReactorReturnCodes.SUCCESS;
    }
        
    /* Handles service deleted event. */
    int serviceDeleted(WlService wlService)
    {
    	// serviceName can be null string
    	boolean singleOpen = _watchlist.loginHandler().supportSingleOpen();
    	if (singleOpen) 
    		handleCloseRecover(wlService, "service deleted.");	
    	else 
    		handleClose(wlService, "service deleted.");
        return ReactorReturnCodes.SUCCESS;        	
    }
    
    /* Handles all service deleted event. */
    void allServicesDeleted()
    {
       	boolean singleOpen = _watchlist.loginHandler().supportSingleOpen();
    	if (singleOpen) 
    		handleCloseRecover("all service deleted.");
    	else 
    		handleClose("all service deleted");
    }
    
    @Override
    public int callbackUser(String location, Msg msg, MsgBase rdmMsg, ReactorErrorInfo errorInfo)
    {
        int ret = ReactorReturnCodes.SUCCESS;
        WlRequest wlRequest = _watchlist.streamIdtoWlRequestTable().get(msg.streamId());
        
        if (msg.domainType() != DomainTypes.DICTIONARY)
        {
            ret = _watchlist.reactor().sendAndHandleDefaultMsgCallback(location,
                                                                       _watchlist.reactorChannel(),
                                                                       null,
                                                                       msg,
                                                                       (wlRequest != null ? wlRequest.streamInfo() : null),
                                                                       errorInfo);
        }
        else // dictionary domain
        {
            ret = _watchlist.reactor().sendAndHandleDictionaryMsgCallback(location,
                                                                          _watchlist.reactorChannel(),
                                                                          null,
                                                                          msg,
                                                                          (DictionaryMsg)rdmMsg,
                                                                          (wlRequest != null ? wlRequest.streamInfo() : null),
                                                                          errorInfo);

            if (ret == ReactorCallbackReturnCodes.RAISE)
            {
                ret = _watchlist.reactor().sendAndHandleDefaultMsgCallback(location,
                                                                           _watchlist.reactorChannel(),
                                                                           null,
                                                                           msg,
                                                                           (wlRequest != null ? wlRequest.streamInfo() : null),
                                                                           errorInfo);
            }
        }
        return ret;
    }
    
    void handleCloseRecover(String text)
    {
  	   for (WlStream wlStream = _streamList.poll(); wlStream != null; wlStream = _streamList.poll())
  	   {    	   
  		   wlStream.channelDown();
           handleCloseRecover(wlStream, text);
  	   }
    }
  	       
    void handleCloseRecover(WlStream wlStream, String text)
    {
    	LinkedList<WlRequest> requestList = wlStream.userRequestList();
 	   
    	WlRequest usrRequest = null;
    	for (usrRequest = requestList.poll(); usrRequest != null; usrRequest = requestList.poll())
    	{
    	   	_itemAggregationKeytoWlStreamTable.remove(wlStream.itemAggregationKey());
        	wlStream.itemAggregationKey().returnToPool();
        	wlStream.close();
        	
    		usrRequest.state(State.PENDING_REQUEST);
    		_submitOptions.serviceName(usrRequest.streamInfo().serviceName());
    		_submitOptions.requestMsgOptions().userSpecObj(usrRequest.streamInfo().userSpecObject());
    		
    		addToPendingRequestTable(usrRequest, _submitOptions);
    		_statusMsg.applyHasState();
    		_statusMsg.streamId(usrRequest.requestMsg().streamId());
    		_statusMsg.domainType(usrRequest._requestMsg.domainType());
    		_statusMsg.state().streamState(StreamStates.OPEN);
    		_statusMsg.state().dataState(DataStates.SUSPECT);    		
    		_statusMsg.state().text().data(text);
    		if ((callbackUser("WlItemHandler.channelDown", _statusMsg, null, _errorInfo)) < ReactorCallbackReturnCodes.SUCCESS)
    		{
    			// break;
    			System.out.println(" WlItemHandler handleCloseRecover callbackUser failed for stream " + usrRequest.requestMsg().streamId() );
    		}    
    	} 			 
    }    	
           
    void handleCloseRecover(WlService wlService, String text)
    {
 	   for (int i = 0; i < wlService.streamList().size(); i++)
  	   {    
 		   WlStream wlStream = wlService.streamList().get(i);
    	     	   
  		   wlStream.channelDown();
 		   LinkedList<WlRequest> requestList = wlStream.userRequestList();
 	   
 		   if (requestList.size() == 0 ) continue;
 		   else 
 			   handleCloseRecover(wlStream, text);
 			
 		   _streamList.remove(wlStream); 		   
  	   } 	
 	   
 	   LinkedList<WlRequest> waitingList = wlService.waitingRequestList();
 	   for ( WlRequest usrRequest = waitingList.poll(); usrRequest != null; usrRequest = waitingList.poll()) 
 	   {
 	   		usrRequest.state(State.PENDING_REQUEST);
 	   		addToPendingRequestTable(usrRequest, _submitOptions);
 	   } 	   	   	   
    }    
                            
    void handleClose(String text)
    {    	
 	   for (WlStream wlStream = _streamList.poll();  wlStream != null; wlStream = _streamList.poll())
 	   {    	 
 		   handleClose(wlStream, text);
 	   }
  	   
 	   for (Map.Entry<String, LinkedList<WlRequest>> entry : _pendingRequestByNameTable.entrySet())
 	   {
 		   LinkedList<WlRequest> pendingRequestList  = (LinkedList<WlRequest>)entry.getValue();
 	   	   for (WlRequest usrRequest = pendingRequestList.poll(); usrRequest!= null; usrRequest = pendingRequestList.poll())
     	   { 
     		   usrRequest.returnToPool();    		 		  
     	   }
    	   _pendingRequestListPool.add(pendingRequestList); 
 	   }
 
 	   for (Map.Entry<Integer, LinkedList<WlRequest>> entry : _pendingRequestByIdTable.entrySet())
 	   {
 		   LinkedList<WlRequest> pendingRequestList  = (LinkedList<WlRequest>)entry.getValue();
 	   	   for (WlRequest usrRequest = pendingRequestList.poll(); usrRequest!= null; usrRequest = pendingRequestList.poll())
     	   { 
     		   usrRequest.returnToPool();    		 		  
     	   }
    	   _pendingRequestListPool.add(pendingRequestList); 
 	   } 	   
    }
    
    void handleClose(WlStream wlStream, String text)
    {
    	LinkedList<WlRequest> requestList = wlStream.userRequestList();
     	   
    	WlRequest usrRequest = null;
       	_itemAggregationKeytoWlStreamTable.remove(wlStream.itemAggregationKey());
    	wlStream.itemAggregationKey().returnToPool();

    	_streamList.remove(wlStream);
    	wlStream.close();
    	    	    	
    	for (usrRequest = requestList.poll(); usrRequest!= null; usrRequest = requestList.poll())
    	{    	 
    		_statusMsg.applyHasState();
    		_statusMsg.streamId(usrRequest.requestMsg().streamId());
    		_statusMsg.domainType(usrRequest._requestMsg.domainType());
    		_statusMsg.state().streamState(StreamStates.CLOSED);
    		_statusMsg.state().dataState(DataStates.SUSPECT);
    		_statusMsg.state().text().data(text);
    		if ((callbackUser("WlItemHandler.channelDown", _statusMsg, null, _errorInfo)) < ReactorCallbackReturnCodes.SUCCESS)
    		{
    			// break;
    			System.out.println(" WlItemHandler handleClose callbackUser failed for stream " + usrRequest.requestMsg().streamId() );
    		}
    		usrRequest.returnToPool();     		     		  
    	}             
    }
    
    void handleClose(WlService wlService, String text)
    {
 	   for (int i = 0; i < wlService.streamList().size(); i++)
  	   {    
 		   WlStream wlStream = wlService.streamList().get(i);
    	     	   
 		   LinkedList<WlRequest> requestList = wlStream.userRequestList();
 	   
 		   if (requestList.size() == 0 ) continue;
 		   else
 		   {
 			   handleClose(wlStream, text);
 		   }
 		   _streamList.remove(wlStream);
  	   } 	
 	   for ( WlRequest usrRequest = wlService.waitingRequestList().poll(); usrRequest != null; usrRequest = wlService.waitingRequestList().poll()) 
 	   {
 	   		usrRequest.returnToPool();
 	   } 	
    }        
    
    void handleOpenStatus(WlStream wlStream, Msg msg)
    {
    	boolean singleOpen = _watchlist.loginHandler().supportSingleOpen();
    	boolean allowSuspect = _watchlist.loginHandler().supportAllowSuspectData();
    	
    	if (singleOpen || allowSuspect)
    	{
        	LinkedList<WlRequest> requestList = wlStream.userRequestList();
      	   
        	for (WlRequest usrRequest = requestList.poll(); usrRequest != null; usrRequest = requestList.poll())
        	{        		
                msg.streamId(usrRequest.requestMsg().streamId());
        		if ((callbackUser("WlItemHandler handleStatus", msg, null, _errorInfo)) < ReactorCallbackReturnCodes.SUCCESS)
        		{
        			// break;
        			System.out.println(" WlItemHandler handleStatus callbackUser failed for stream " + usrRequest.requestMsg().streamId() );
        		}
        	}
    	}
    	else // (!singleOpen && ! allowSuspect)
    	{  
    		handleSuspect(wlStream, msg);    		     	
    	}    	
    }
        
    void handleSuspect(WlStream wlStream, Msg msg) 
    {
		LinkedList<WlRequest> requestList = wlStream.userRequestList();
	   
		int dataState = wlStream.state().dataState();
	
		if (dataState != DataStates.SUSPECT)
		{
			for (WlRequest usrRequest = requestList.poll(); usrRequest != null; usrRequest = requestList.poll())
			{
				msg.streamId(usrRequest.requestMsg().streamId());
				
				if ((callbackUser("WlItemHandler handleStatus", msg, null, _errorInfo)) < ReactorCallbackReturnCodes.SUCCESS)
				{
					System.out.println(" WlItemHandler handleStatus callbackUser failed for stream " + usrRequest.requestMsg().streamId() );
				}
			}
		}
		else // data is SUSPECT
		{
			for (WlRequest usrRequest = requestList.poll(); usrRequest != null; usrRequest = requestList.poll())
			{
				CloseMsg closeMsg = (CloseMsg)CodecFactory.createMsg();
				closeMsg.clear();
				closeMsg.msgClass(MsgClasses.CLOSE);
				closeMsg.streamId(usrRequest.requestMsg().streamId());
				closeMsg.domainType(usrRequest.requestMsg().domainType());
				closeMsg.containerType(DataTypes.NO_DATA); 
				ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
				ReactorErrorInfo errorInfo = ReactorFactory.createReactorErrorInfo();
				if ((wlStream.sendMsg(closeMsg, submitOptions, errorInfo)) < ReactorReturnCodes.SUCCESS)
				{
					System.out.println(" WlItemHandler sendCloseMsg for upstream failed for stream " + usrRequest.requestMsg().streamId() );
				}
				_statusMsg.applyHasState();
				_statusMsg.streamId(usrRequest.requestMsg().streamId()); 	   		
				_statusMsg.state().streamState(StreamStates.CLOSED_RECOVER);
				_statusMsg.state().dataState(dataState);
				_statusMsg.state().text().data("stream to closed_recover");
						
				if ((callbackUser("WlItemHandler handleStatus", _statusMsg, null, _errorInfo)) < ReactorCallbackReturnCodes.SUCCESS)
				{
        			// break;
        			System.out.println(" WlItemHandler handleStatus callbackUser failed for stream " + usrRequest.requestMsg().streamId() );
				}
				usrRequest.returnToPool();
			}			
			_streamList.remove(wlStream);
			wlStream.close(); 
		}
	}
    
    @Override
    public int requestTimeout(WlStream wlStream, ReactorErrorInfo errorInfo)
    {
        // close stream and update item aggregation table
        _itemAggregationKeytoWlStreamTable.remove(wlStream.itemAggregationKey());
        wlStream.itemAggregationKey().returnToPool();
        wlStream.close();

        // fanout status to user and add requests to request timeout list
        LinkedList<WlRequest> requestList = wlStream.userRequestList();
        WlRequest usrRequest = null;
        for (usrRequest = requestList.poll(); usrRequest != null; usrRequest = requestList.poll())
        {
            usrRequest.state(State.PENDING_REQUEST);
            
            // add to request timeout list only if single open supported
            if (_watchlist.loginHandler().supportSingleOpen())
            {
                _requestTimeoutList.add(usrRequest);
            }
            
            sendStatus(usrRequest.requestMsg().streamId(), usrRequest.requestMsg().domainType(), "Request timeout");
        }
        
        // trigger dispatch
        _watchlist.reactor().sendWatchlistDispatchNowEvent(_watchlist.reactorChannel());
        
        return ReactorReturnCodes.SUCCESS;
    }
    
    /* Clear state of watchlist item handler for re-use. */
    void clear()
    {
        // this handler is still associated with same watchlist so don't set watchlist to null
        _directoryStreamOpen = false;
        _tempItemAggregationRequest.clear();
    }
}
