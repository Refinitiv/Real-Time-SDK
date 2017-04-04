package com.thomsonreuters.upa.valueadd.reactor;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.Map;

import com.thomsonreuters.upa.codec.AckMsg;
import com.thomsonreuters.upa.codec.AckMsgFlags;
import com.thomsonreuters.upa.codec.Array;
import com.thomsonreuters.upa.codec.ArrayEntry;
import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CloseMsg;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.CopyMsgFlags;
import com.thomsonreuters.upa.codec.DataStates;
import com.thomsonreuters.upa.codec.DataTypes;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.ElementEntry;
import com.thomsonreuters.upa.codec.ElementList;
import com.thomsonreuters.upa.codec.GenericMsg;
import com.thomsonreuters.upa.codec.GenericMsgFlags;
import com.thomsonreuters.upa.codec.MapEntry;
import com.thomsonreuters.upa.codec.MapEntryActions;
import com.thomsonreuters.upa.codec.Msg;
import com.thomsonreuters.upa.codec.MsgClasses;
import com.thomsonreuters.upa.codec.MsgKey;
import com.thomsonreuters.upa.codec.PostMsg;
import com.thomsonreuters.upa.codec.Qos;
import com.thomsonreuters.upa.codec.QosRates;
import com.thomsonreuters.upa.codec.QosTimeliness;
import com.thomsonreuters.upa.codec.RefreshMsg;
import com.thomsonreuters.upa.codec.RefreshMsgFlags;
import com.thomsonreuters.upa.codec.RequestMsg;
import com.thomsonreuters.upa.codec.RequestMsgFlags;
import com.thomsonreuters.upa.codec.StatusMsg;
import com.thomsonreuters.upa.codec.StatusMsgFlags;
import com.thomsonreuters.upa.codec.StreamStates;
import com.thomsonreuters.upa.codec.UInt;
import com.thomsonreuters.upa.codec.UpdateMsg;
import com.thomsonreuters.upa.codec.UpdateMsgFlags;
import com.thomsonreuters.upa.rdm.DomainTypes;
import com.thomsonreuters.upa.rdm.ElementNames;
import com.thomsonreuters.upa.rdm.InstrumentNameTypes;
import com.thomsonreuters.upa.rdm.SymbolList;
import com.thomsonreuters.upa.rdm.ViewTypes;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.MsgBase;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.dictionary.DictionaryMsg;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.dictionary.DictionaryMsgFactory;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.dictionary.DictionaryMsgType;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.Service;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.Service.ServiceGroup;
import com.thomsonreuters.upa.valueadd.reactor.WlRequest.State;

class WlItemHandler implements WlHandler
{
    // non private stream copy flags
    int defaultCopyFlags = CopyMsgFlags.ALL_FLAGS & ~CopyMsgFlags.EXTENDED_HEADER & ~CopyMsgFlags.DATA_BODY;
    
    // private stream copy flags
    int privateStreamCopyFlags = CopyMsgFlags.ALL_FLAGS;
    
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
    HashMap<WlItemAggregationKey,WlStream> _itemAggregationKeytoWlStreamTable;
    
    WlItemAggregationKey _tempItemAggregationKey = ReactorFactory.createWlItemAggregationKey();
    RequestMsg _tempItemAggregationRequest = (RequestMsg)CodecFactory.createMsg();
    
    Qos _defaultQos = CodecFactory.createQos();
    Qos _tempMatchedQos = CodecFactory.createQos();
    
    StatusMsg _statusMsg = (StatusMsg)CodecFactory.createMsg();

	CloseMsg _closeMsg = (CloseMsg)CodecFactory.createMsg();

    // pool of StatusMsgs
    LinkedList<StatusMsg> _statusMsgPool = new LinkedList<StatusMsg>();
    
    // List of StatusMsgs to send when dispatch is called
    LinkedList<StatusMsg> _statusMsgDispatchList = new LinkedList<StatusMsg>();
    
    // List of streams with pending messages to send
    LinkedList<WlStream> _pendingSendMsgList = new LinkedList<WlStream>();
    
    // List of user requests to re-submit upon dispatch that had request timeout
    LinkedList<WlRequest> _requestTimeoutList = new LinkedList<WlRequest>();
    
    // RDM dictionary message for callback
    DictionaryMsg _rdmDictionaryMsg = DictionaryMsgFactory.createMsg();

    // in case of close recover, the streamId list of user streams 
    LinkedList<Integer> _userStreamIdListToRecover = new LinkedList<Integer>();
  
	// table that maps item provider request aggregation key to application
	// requests for symbol list data stream
	HashMap<WlItemAggregationKey, RequestMsg> _providerRequestTable = new HashMap<WlItemAggregationKey, RequestMsg>();
	RequestMsg _requestMsg = (RequestMsg) CodecFactory.createMsg();
	WlItemAggregationKey _symbolListRequestKey = ReactorFactory
			.createWlItemAggregationKey();
	com.thomsonreuters.upa.codec.Map _map = CodecFactory.createMap();
	MapEntry _mapEntry = CodecFactory.createMapEntry();
	Buffer _mapKey = CodecFactory.createBuffer();
	
	ElementList _elementList = CodecFactory.createElementList();
	ElementList _behaviourElementList = CodecFactory.createElementList();
	ElementEntry _elementEntry = CodecFactory.createElementEntry();
	ElementEntry _behaviourEntry = CodecFactory.createElementEntry();
	UInt _dataStreamFlag = CodecFactory.createUInt();
	DecodeIterator _dIter = CodecFactory.createDecodeIterator();
	DecodeIterator _dIterBatch = CodecFactory.createDecodeIterator();

	WlInteger _tempWlInteger = ReactorFactory.createWlInteger();

	UInt _viewType = CodecFactory.createUInt();
	boolean _hasViewType;
	Buffer _viewDataElement = CodecFactory.createBuffer();
	boolean _viewDataFound;
	UInt _fieldId = CodecFactory.createUInt();
	final int VIEW_ACTION_SET = 1;
	final int VIEW_ACTION_MAINTAIN = 2;
	final int VIEW_ACTION_NONE = 3;
    int _viewElemCount;	
	WlViewHandler _wlViewHandler;			
   	Array _viewArray = CodecFactory.createArray();
	ArrayEntry _viewArrayEntry = CodecFactory.createArrayEntry();
	Buffer _elementName = CodecFactory.createBuffer();
	Buffer _viewElemList = CodecFactory.createBuffer();
	boolean _snapshotViewClosed;
	// Denotes if the watchlist stream had already called handleClose
	//     due to snapshot stream close on a wlStream with no other requests
	boolean _snapshotStreamClosed;

    WlStream _currentFanoutStream;
		
    WlItemHandler(Watchlist watchlist)
    {
        _watchlist = watchlist;
        _defaultQos.clear();
        _defaultQos.timeliness(QosTimeliness.REALTIME);
        _defaultQos.rate(QosRates.TICK_BY_TICK);
        _statusMsg.msgClass(MsgClasses.STATUS);
        _wlViewHandler = new WlViewHandler(watchlist);
        _itemAggregationKeytoWlStreamTable = new HashMap<WlItemAggregationKey,WlStream>(_watchlist.role().watchlistOptions().itemCountHint() + 10, 1);
    }
    
    @Override
    public int submitRequest(WlRequest wlRequest, RequestMsg requestMsg, boolean isReissue, ReactorSubmitOptions submitOptions, ReactorErrorInfo errorInfo)
    {
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

            if (requestMsg.checkHasBatch())
            {
                
            	// handle batch request
            	return handleBatchRequest(wlRequest, requestMsg, submitOptions, errorInfo);
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
        
		if (requestMsg.domainType() == DomainTypes.SYMBOL_LIST)
		{
			if (( ret = extractSymbolListFromMsg(wlRequest, requestMsg, errorInfo)) < ReactorReturnCodes.SUCCESS)
				return ret;
		}
		
		if ( requestMsg.checkHasView())
		{
			if ( (ret = extractViewFromMsg(wlRequest, requestMsg, errorInfo)) < ReactorReturnCodes.SUCCESS)
            return ret;
		}
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
            wlRequest.serviceId(wlService.rdmService().serviceId());
            // next check if window is open for service (don't check if dictionary domain)
            if (requestMsg.domainType() == DomainTypes.DICTIONARY || isWindowOpen(wlService))
            {
                // find aggregation stream
                WlStream wlStream = null;
                if ((wlStream = findItemAggregationStream(requestMsg, _tempMatchedQos, submitOptions)) == null)
                {
                    // item cannot be aggregated, create new stream
                    wlStream = createNewStream(requestMsg);
                    wlRequest.stream(wlStream);
                    // add to fanout list                        
                    wlStream.userRequestList().add(wlRequest);
                    
               		if ( requestMsg.checkHasView() && requestMsg.domainType() != DomainTypes.SYMBOL_LIST)
            		{ 
               			if ( (ret = handleViews(wlRequest, false, errorInfo)) < ReactorReturnCodes.SUCCESS)
               				return ret;			
            		}
            	                    
                    WlItemAggregationKey itemAggregationKey = null;
                    
                    // copy temporary item aggregation key from findItemAggregationStream() into new one if not private stream
                    if (!requestMsg.checkPrivateStream())
                    {
                        itemAggregationKey = ReactorFactory.createWlItemAggregationKey();
                        _tempItemAggregationKey.copy(itemAggregationKey);
                    }                    
                    // update temporary item aggregation request and send
                    _tempItemAggregationRequest.clear();
                    if (!requestMsg.checkPrivateStream())
                    {
                        requestMsg.copy(_tempItemAggregationRequest, defaultCopyFlags);
                        _tempItemAggregationRequest.containerType(DataTypes.NO_DATA);
                    }
                    else
                    {
                        requestMsg.copy(_tempItemAggregationRequest, privateStreamCopyFlags);
                    	if ( requestMsg.checkHasView())
                    	{
                    		_tempItemAggregationRequest.containerType(DataTypes.ELEMENT_LIST);                    	
                        	_tempItemAggregationRequest.applyHasView();
                        	_tempItemAggregationRequest.encodedDataBody(_viewElemList);
                    	}
                    }
                    // stream id is that of watchlist stream
                    _tempItemAggregationRequest.streamId(wlStream.streamId());
                    // Service id is that of item aggregation key
                    _tempItemAggregationRequest.msgKey().applyHasServiceId();
                    _tempItemAggregationRequest.msgKey().serviceId(_tempItemAggregationKey.msgKey().serviceId());
                    // qos is that of item aggregation key if not dictionary domain
                    if (requestMsg.domainType() != DomainTypes.DICTIONARY)
                    {
                        _tempItemAggregationRequest.applyHasQos();
                        _tempItemAggregationKey.qos().copy(_tempItemAggregationRequest.qos());
                        // clear worst qos flag
                        _tempItemAggregationRequest.flags(_tempItemAggregationRequest.flags() & ~RequestMsgFlags.HAS_WORST_QOS);
                    }
                    // priority is that of request or 1/1 if not present
                    if (requestMsg.checkHasPriority())
                    {
                        _tempItemAggregationRequest.applyHasPriority();
                        _tempItemAggregationRequest.priority().priorityClass(requestMsg.priority().priorityClass());
                        _tempItemAggregationRequest.priority().count(requestMsg.priority().count());
                    }
                    else if (!requestMsg.checkPrivateStream()) // request has no priority, use default of 1/1 if not private stream
                    {
                        _tempItemAggregationRequest.applyHasPriority();
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
                    if (requestMsg.domainType() != DomainTypes.DICTIONARY && !requestMsg.checkNoRefresh())
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
                        WlInteger wlInteger = ReactorFactory.createWlInteger();
                        wlInteger.value(wlStream.streamId());
                        wlStream.tableKey(wlInteger);
                        _watchlist.streamIdtoWlStreamTable().put(wlInteger, wlStream);

                        // add to _itemAggregationKeytoWlStreamTable if not private stream
                        if (!requestMsg.checkPrivateStream())
                        {
                            _itemAggregationKeytoWlStreamTable.put(itemAggregationKey, wlStream);
                        }
                        
                        // associate stream and item aggregation key if not private stream
                        if (!requestMsg.checkPrivateStream())
                        {
                            wlStream.itemAggregationKey(itemAggregationKey);
                        }
                        
                        // associate stream and service
                        wlStream.wlService(wlService);
                        
                        // add stream to service's stream list
                        wlService.streamList().add(wlStream);
                        
                        // update request state to REFRESH_PENDING
                        wlRequest.state(WlRequest.State.REFRESH_PENDING);
                        
                        if ( wlRequest.requestMsg().checkPause() && wlRequest.requestMsg().checkStreaming())
                        	wlStream.numPausedRequestsCount(1);
                        
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
                    
                    // set stream associated with request
                    wlRequest.stream(wlStream);
                    
                    /* add request to stream only if not in the middle of snapshot or
                       multi-part refresh and not in pending view refresh, and snapshot view with no request pending */
                    if ((((wlStream.numSnapshotsPending() == 0 || (wlStream.numSnapshotsPending() > 0 && !requestMsg.checkStreaming())) || wlRequest.state() == WlRequest.State.PENDING_REQUEST ) &&
                        !wlStream.multiPartRefreshPending() ) && !wlStream._pendingViewRefresh  && !(requestMsg.checkHasView() && !requestMsg.checkStreaming() && wlStream.requestPending()) )
                    {                   	
                		if ( requestMsg.checkHasView())
                 		{                   			
                			if ( (ret = handleViews(wlRequest, false, errorInfo)) < ReactorReturnCodes.SUCCESS)
                	            return ret;
                 		}
                        // add request to stream
                        wlStream.userRequestList().add(wlRequest);

                        if ( wlRequest.requestMsg().checkPause() && wlRequest.requestMsg().checkStreaming())
                        	wlStream.numPausedRequestsCount(wlStream.numPausedRequestsCount() +1);
                                                
                        // update request state to REFRESH_PENDING
                        wlRequest.state(WlRequest.State.REFRESH_PENDING);
    
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
                            // use priorityClass of request if greater than existing one
                            if (requestMsg.priority().priorityClass() > streamRequestMsg.priority().priorityClass())
                            {
                                streamRequestMsg.priority().priorityClass(requestMsg.priority().priorityClass());
                            }
                            
                            // add priorityCount to that of existing one
                            if (requestMsg.checkStreaming() && !streamRequestMsg.checkStreaming())
                            	streamRequestMsg.priority().count(requestMsg.priority().count());
                            else if (requestMsg.checkStreaming())
                            	streamRequestMsg.priority().count(streamRequestMsg.priority().count() + requestMsg.priority().count());
                        }
                        else // request has no priority, assume default of 1/1
                        {
                            streamRequestMsg.applyHasPriority();
                            if (requestMsg.checkStreaming() && !streamRequestMsg.checkStreaming())
                            	streamRequestMsg.priority().count(1);
                            else if (requestMsg.checkStreaming())
                            	streamRequestMsg.priority().count(streamRequestMsg.priority().count() + 1);
                        }

                        if (requestMsg.checkStreaming() && !streamRequestMsg.checkStreaming())
                        	streamRequestMsg.applyStreaming();
                        
                        // send message to stream if streaming or it is a snapshot with no request pending
                        if (sendNow && (requestMsg.checkStreaming() 
                        		|| (!requestMsg.checkStreaming() && !wlStream.requestPending()) ))
                        {
                            // increment number of outstanding requests if not dictionary domain and a request isn't currently pending
                            if (requestMsg.domainType() != DomainTypes.DICTIONARY && !wlStream.requestPending() && !requestMsg.checkNoRefresh())
                            {
                                wlService.numOutstandingRequests(wlService.numOutstandingRequests() + 1);
                            }
                            
                            ret = wlStream.sendMsg(streamRequestMsg, submitOptions, _errorInfo);
                        }                        
                        else // if not sendNow, add stream to pending send message list if not already there
                        {
                            if (!_pendingSendMsgList.contains(wlStream))
                            {
                                _pendingSendMsgList.add(wlStream);
 
                                // increment number of outstanding requests if not dictionary domain and a request isn't currently pending
                                if (requestMsg.domainType() != DomainTypes.DICTIONARY && !wlStream.requestPending() && !requestMsg.checkNoRefresh())
                                {
                                    wlService.numOutstandingRequests(wlService.numOutstandingRequests() + 1);
                                }
                            }
                            else // stream is pending, if this request has no pause flag set while the pending stream has it set, remove pause flag from pending stream
                            {
                            	RequestMsg pendingRequestMsg = _pendingSendMsgList.get(_pendingSendMsgList.indexOf(wlStream)).requestMsg();
                            	if (pendingRequestMsg.checkPause() && !requestMsg.checkPause())
                            	{
                            		pendingRequestMsg.flags(pendingRequestMsg.flags() & ~RequestMsgFlags.PAUSE);
                            	}
                            }
                        }
                    }
                    else // currently in the middle of snapshot or multi-part refresh
                    {
                        // cannot open item at this time, add to waiting request list
    
                        // add to waiting request list for stream
                        wlRequest.streamInfo().serviceName(submitOptions.serviceName());
                        wlRequest.streamInfo().userSpecObject(submitOptions.requestMsgOptions().userSpecObj());
                        wlStream.waitingRequestList().add(wlRequest);
                    }
                }
            }
            else // service window not open
            {
                // add to waiting request list for service            	
                wlRequest.streamInfo().serviceName(submitOptions.serviceName());
                wlRequest.streamInfo().userSpecObject(submitOptions.requestMsgOptions().userSpecObj());
                wlService.waitingRequestList().add(wlRequest);         	
            }
        }
        else // cannot open item at this time, add to pending request table if not private stream
        {
            // save stream info
            wlRequest.streamInfo().serviceName(submitOptions.serviceName());
            wlRequest.streamInfo().userSpecObject(submitOptions.requestMsgOptions().userSpecObj());
            
            if (wlService == null)
            {
                errorInfo.error().text("Service not available");
            }

            // queue status message to send on next dispatch call
            if (_userStreamIdListToRecover.size() == 0 || sendNow)                        
            	queueStatusForDispatch(requestMsg.streamId(), requestMsg.domainType(), errorInfo.error().text(), requestMsg.checkPrivateStream());
            else
            {            	
            	// only queue status in case of close recover
            	if (_userStreamIdListToRecover.contains(requestMsg.streamId()))
            	{
            		queueStatusForDispatch(requestMsg.streamId(), requestMsg.domainType(), errorInfo.error().text(), requestMsg.checkPrivateStream());
            	}
            }

            if (!requestMsg.checkPrivateStream() && _watchlist.loginHandler().supportSingleOpen())
            {
                addToPendingRequestTable(wlRequest, submitOptions);
            }
        }
        return ret;
    }
    
    int handleBatchRequest(WlRequest wlRequest, RequestMsg requestMsg, ReactorSubmitOptions submitOptions, ReactorErrorInfo errorInfo)
    {
    	int ret = ReactorReturnCodes.SUCCESS;
    	
    	ElementList elementList = CodecFactory.createElementList();
    	ElementEntry elementEntry = CodecFactory.createElementEntry();
    	Array batchArray = CodecFactory.createArray();
    	ArrayEntry batchArrayEntry = CodecFactory.createArrayEntry();
    	boolean foundBatch = false;
    	int originalStreamId = requestMsg.streamId();
    	int currentStreamId = requestMsg.streamId();
    	RequestMsg tempRequestMsg = (RequestMsg)CodecFactory.createMsg();
    	requestMsg.copy(tempRequestMsg, CopyMsgFlags.ALL_FLAGS);
    	
    	// Make sure requestMsg does not have item name set on MsgKey
    	if (tempRequestMsg.msgKey().checkHasName())
    	{
            return _watchlist.reactor().populateErrorInfo(errorInfo,
            		ReactorReturnCodes.FAILURE,
                    "WlItemHandler.processBatchRequest",
                    "Requested batch has name in message key.");
    	}
    	
		if (tempRequestMsg.domainType() == DomainTypes.SYMBOL_LIST)
		{
			if (( ret = extractSymbolListFromMsg(wlRequest, requestMsg, errorInfo)) < ReactorReturnCodes.SUCCESS)
				return ret;
		}
		
		_dIterBatch.clear();
		_dIterBatch.setBufferAndRWFVersion(tempRequestMsg.encodedDataBody(), _watchlist.reactorChannel().majorVersion(),
    			_watchlist.reactorChannel().minorVersion());
		
	    wlRequest.streamInfo().serviceName(submitOptions.serviceName());
	    wlRequest.streamInfo().userSpecObject(submitOptions.requestMsgOptions().userSpecObj());

    	if (tempRequestMsg.containerType() == DataTypes.ELEMENT_LIST)
    	{
        	if ((ret = elementList.decode(_dIterBatch, null)) <= CodecReturnCodes.FAILURE)
        	{
                return _watchlist.reactor().populateErrorInfo(errorInfo,
                        ReactorReturnCodes.FAILURE,
                        "WlItemHandler.handleBatchRequest",
                        "ElementList.decode() failure.");
        	}
        	
        	// check element list for itemList
    		while ((ret = elementEntry.decode(_dIterBatch)) != CodecReturnCodes.END_OF_CONTAINER)
    		{
    			if (ret <= CodecReturnCodes.FAILURE)
    			{
    	            return _watchlist.reactor().populateErrorInfo(errorInfo,
    	            		ReactorReturnCodes.FAILURE,
    	                    "WlItemHandler.handleBatchRequest",
    	                    "ElementEntry.decode() failure.");
    			}
    			Buffer itemListName = CodecFactory.createBuffer();
    			itemListName.data(":ItemList");
    			if (elementEntry.name().toString().contains(itemListName.toString()))
				{
					foundBatch = true;
					break;
				}
    		}
    	}
    	else
    	{
            return _watchlist.reactor().populateErrorInfo(errorInfo,
            		ReactorReturnCodes.FAILURE,
                    "WlItemHandler.handleBatchRequest",
                    "Unexpected container type or decoding error.");
    	}
    	
    	WlInteger wlInteger = ReactorFactory.createWlInteger();
        wlInteger.value(requestMsg.streamId());
    	_watchlist.streamIdtoWlRequestTable().put(wlInteger, wlRequest);
	

    	
		// found itemList, thus a batch. Make individual item requests from array of requests
		if (foundBatch)
		{	
			/* Start at stream ID after batch request. */
			currentStreamId++;
			
			_tempWlInteger.value(currentStreamId);
			if (_watchlist.streamIdtoWlRequestTable().get(_tempWlInteger) != null)
			{
	            return _watchlist.reactor().populateErrorInfo(errorInfo,
	            		ReactorReturnCodes.FAILURE,
	                    "WlItemHandler.handleBatchRequest",
	                    "Item in batch has same ID as existing stream.");
			}
			
			if ((ret = batchArray.decode(_dIterBatch)) <= CodecReturnCodes.FAILURE)
			{
	            return _watchlist.reactor().populateErrorInfo(errorInfo,
	            		ReactorReturnCodes.FAILURE,
	                    "WlItemHandler.handleBatchRequest",
	                    "Array.decode() failure.");
			}
			
			HashMap<Integer, WlRequest> wlRequestList = new HashMap<Integer, WlRequest>();
			HashMap<Integer, RequestMsg> requestMsgList = new HashMap<Integer, RequestMsg>();
			int possibleStreamId = currentStreamId;
			int retDecodeVal = CodecReturnCodes.SUCCESS;
			
			while ((retDecodeVal = batchArrayEntry.decode(_dIterBatch)) != CodecReturnCodes.END_OF_CONTAINER)
			{
				if ((retDecodeVal = tempRequestMsg.msgKey().name().decode(_dIterBatch)) == CodecReturnCodes.SUCCESS)
				{
					_tempWlInteger.value(possibleStreamId);
					if (_watchlist.streamIdtoWlRequestTable().get(_tempWlInteger) != null)
					{
						while (!wlRequestList.isEmpty())
						{
			            	WlRequest removeWlRequest = wlRequestList.remove(currentStreamId);
			            	removeWlRequest.state(State.RETURN_TO_POOL);
			            	removeWlRequest.returnToPool();
			            	requestMsgList.remove(currentStreamId);
			            	currentStreamId++;
			            	putWlRequestViewListBackToPool(removeWlRequest);
						}
			            return _watchlist.reactor().populateErrorInfo(errorInfo,
			            		ReactorReturnCodes.FAILURE,
			                    "WlItemHandler.handleBatchRequest",
			                    "Item in batch has same ID as existing stream.");
					}
					
					WlRequest newWlRequest = ReactorFactory.createWlRequest();
					RequestMsg newRequestMsg = (RequestMsg) CodecFactory.createMsg();

	            	// Create item list request and new watchlist request based off old watchlist request
	            	newWlRequest.handler(wlRequest.handler());
	            	newWlRequest.stream(wlRequest.stream());
	            	

	            	// Remove batch flag
	            	tempRequestMsg.copy(newRequestMsg, CopyMsgFlags.ALL_FLAGS);
	            	newRequestMsg.flags(newRequestMsg.flags() & ~RequestMsgFlags.HAS_BATCH);
	            	newRequestMsg.streamId(possibleStreamId);
	            	
	            	newRequestMsg.msgClass(MsgClasses.REQUEST);
	            	
	            	// Set msgKey item name
	            	newRequestMsg.applyMsgKeyInUpdates();
	            	newRequestMsg.msgKey().applyHasName();
            		newRequestMsg.msgKey().name(batchArrayEntry.encodedData());

	            	newRequestMsg.copy(newWlRequest.requestMsg(), CopyMsgFlags.ALL_FLAGS);
	            	
	            	wlRequestList.put(possibleStreamId, newWlRequest);
	            	requestMsgList.put(possibleStreamId, newRequestMsg);
            	
                    possibleStreamId++;
				}
				else
				{
		            return _watchlist.reactor().populateErrorInfo(errorInfo,
		            		ReactorReturnCodes.FAILURE,
		                    "WlItemHandler.handleBatchRequest",
		                    "Invalid BLANK_DATA while decoding :ItemList -- " + CodecReturnCodes.toString(ret));
				}
			}
			
			while (!wlRequestList.isEmpty())
			{
	            
            	// Add watchlist request to request table
                wlInteger = ReactorFactory.createWlInteger();
                wlInteger.value(currentStreamId);
                _watchlist.streamIdtoWlRequestTable().put(wlInteger, wlRequestList.get(currentStreamId));
                
            	ret = handleRequest(wlRequestList.get(currentStreamId), requestMsgList.get(currentStreamId), submitOptions, true, errorInfo);
            	if (ret <= ReactorReturnCodes.FAILURE)
            	{
		            return ret;
            	}
            	
            	wlRequestList.remove(currentStreamId);
            	requestMsgList.remove(currentStreamId);
            	
                currentStreamId++;
			}
			
			if (retDecodeVal == CodecReturnCodes.END_OF_CONTAINER)
			{
				/* Requests created. Make a request for the batch stream so it can be acknowledged. */
    			StatusMsg statusMsg = _statusMsgPool.poll();
    			if (statusMsg == null)
    			{
    				statusMsg = (StatusMsg)CodecFactory.createMsg();
    			}
	            
    			statusMsg.clear();
    			
	            statusMsg.domainType(tempRequestMsg.domainType());
    			statusMsg.msgClass(MsgClasses.STATUS);
    			statusMsg.streamId(originalStreamId);
    			statusMsg.applyHasState();
    		    statusMsg.state().streamState(StreamStates.CLOSED);
	    		statusMsg.state().dataState(DataStates.OK);
	    		Buffer statusText = CodecFactory.createBuffer();
	    		statusText.data("Stream closed for batch");
	    		statusMsg.state().text(statusText);

	    		_statusMsgDispatchList.add(statusMsg);
	    		
	            if (_statusMsgDispatchList.size() == 1)
	            {
	                // trigger dispatch only for first add to list
	                _watchlist.reactor().sendWatchlistDispatchNowEvent(_watchlist.reactorChannel());
	            }
	            
			}
		}
		else
		{
            return _watchlist.reactor().populateErrorInfo(errorInfo,
                    ReactorReturnCodes.FAILURE,
                    "WlItemHandler.handleBatchRequest",
                    ":ItemList not found.");
		}

    	return ret;
    }

    /* Handles a user reissue. */
    int handleReissue(WlRequest wlRequest, RequestMsg requestMsg, ReactorSubmitOptions submitOptions, ReactorErrorInfo errorInfo)
    { 	
        int ret = ReactorReturnCodes.SUCCESS;
        
        if (wlRequest.stream() == null ) 
        {
            ret = _watchlist.reactor().populateErrorInfo(errorInfo,
                    ReactorReturnCodes.FAILURE,
                    "WlItemHandler.handleRequest",
                    "Reissue not allowed on an unopen stream.");
            return ret;
        }
    	wlRequest._reissue_hasChange = false;
    	wlRequest._reissue_hasViewChange = false;
    	
        if (requestMsg.checkNoRefresh())
        {
        	wlRequest._reissue_hasChange = false;
        }
                
		if (requestMsg.domainType() == DomainTypes.SYMBOL_LIST)
		{
			if (( ret = extractSymbolListFromMsg(wlRequest, requestMsg, errorInfo)) < ReactorReturnCodes.SUCCESS)
				return ret;
        	wlRequest._reissue_hasChange = true;
		}

            /* handle reissue only if streaming flag has not changed
         * (it's an error if the streaming flag is changed) */
        if (requestMsg.checkStreaming() == wlRequest.requestMsg().checkStreaming())
        {
            if ( requestMsg.checkStreaming())
            {
            	if (requestMsg.checkPause() &&  !wlRequest.requestMsg().checkPause())
            	{
            		wlRequest.stream().numPausedRequestsCount(wlRequest.stream().numPausedRequestsCount() +1);
                  	if(wlRequest.stream().numPausedRequestsCount() == wlRequest.stream()._userRequestList.size())  	  
                  		wlRequest._reissue_hasChange= true;
            	}
            	if (!requestMsg.checkPause() && wlRequest.requestMsg().checkPause())
            	{
            		wlRequest.stream().numPausedRequestsCount(wlRequest.stream().numPausedRequestsCount() -1);
            		wlRequest._reissue_hasChange = true;
            	}        	
            }
            // retrieve request from stream
            RequestMsg streamRequestMsg = wlRequest.stream().requestMsg();

            boolean removeOldView = true;
            boolean effectiveViewChange = true;
                      
            WlRequest tempWlRequest = ReactorFactory.createWlRequest();
            
    		if (requestMsg.checkHasView())  // has viewFlag
    		{    			
    			// for re-issue, in case incoming request does not have view data, re-use the cached one
    			if (requestMsg.encodedDataBody().data() == null ) requestMsg.encodedDataBody(wlRequest.requestMsg().encodedDataBody());
    			
    			if ( (ret = extractViewFromMsg(tempWlRequest, requestMsg, errorInfo)) < ReactorReturnCodes.SUCCESS)
    			{
    				
    				if (!_hasViewType && tempWlRequest._viewElemCount == 0)
//    				if (tempWlRequest.viewType() != ViewTypes.FIELD_ID_LIST &&  tempWlRequest.viewType() != ViewTypes.ELEMENT_NAME_LIST)
    				{
    					// no view type and empty view content, use the old view type and view field list, do not remove old view, still send out request
    					removeOldView = false;
    					// same view needs to go out on the wire again
    					wlRequest._reissue_hasViewChange = true;
    				}
//    				else if (!_viewDataFound)
//    				{
//    				  // valid but empty fieldId list and empty elementyNameList, use this new empty view 	
//    				}
    				else
    					return ret;
    			}
    			    			
            	if(wlRequest.viewElemCount() > 0 )
            	{
            		if ( wlRequest.viewType() != tempWlRequest.viewType() )
            		{
                        ret = _watchlist.reactor().populateErrorInfo(errorInfo,
                                ReactorReturnCodes.FAILURE,
                                "WlItemHandler.handleReissue",
                                "Requested view type does not match existing stream.");
                        return ret;
            		}	        
            		 // for only one view, aggView is that view and might be removed later
            		if(_wlViewHandler.aggregateViewContainsView(wlRequest.stream()._aggregateView, tempWlRequest) && streamRequestMsg.checkHasView())         	
            		effectiveViewChange = false;
            	}     			
    		}
            WlView oldView = null; 
            if (wlRequest.viewElemCount() > 0 && removeOldView)
            {
            	// has old view and can be removed
            	oldView = removeRequestView(wlRequest.stream(), wlRequest, errorInfo);
            	wlRequest._reissue_hasViewChange = true;
            }                            	
                    
            if (requestMsg.checkHasView())
            {
            	extractViewFromMsg(wlRequest, requestMsg, errorInfo);  	  
    			if ( (ret = handleViews(wlRequest, false, errorInfo)) < ReactorReturnCodes.SUCCESS)
    	            return ret;
            	wlRequest._reissue_hasViewChange = true;
            	
            	// if only one view re-issue, the original aggView could be removed above, this check hence becomes no-op
            	if(_wlViewHandler.aggregateViewContainsNewViews(wlRequest.stream()._aggregateView) && streamRequestMsg.checkHasView()) 
            		wlRequest._reissue_hasViewChange = false;				
            	
            	if(!streamRequestMsg.checkHasView() && wlRequest.stream()._requestsWithViewCount != wlRequest.stream()._userRequestList.size())
            		wlRequest._reissue_hasViewChange = false;            
            	
            	if (!effectiveViewChange) wlRequest._reissue_hasViewChange = false;
            }
                        
            if ( tempWlRequest != null) 
            {
                tempWlRequest.state(State.RETURN_TO_POOL);
            	tempWlRequest.returnToPool();
            	putWlRequestViewListBackToPool(tempWlRequest);
            }
        
            // User requested no refresh flag, so temporarily for this message, set it and turn it off after send
            if (requestMsg.checkNoRefresh())
            	streamRequestMsg.applyNoRefresh();
            
            // update priority only if present on reissue request
            if (requestMsg.checkHasPriority())
            {
                if (!wlRequest.requestMsg().checkHasPriority())
                {
                    // Apply 1,1 priority if not currently present.
                    wlRequest.requestMsg().applyHasPriority();
                    wlRequest.requestMsg().priority().priorityClass(1);
                    wlRequest.requestMsg().priority().count(1);
                }
                
                // update priorityClass only if changed
                if (requestMsg.priority().priorityClass() != wlRequest.requestMsg().priority().priorityClass())
                {
                    // use priorityClass of reissue request if private stream or greater than existing one 
                    if (streamRequestMsg.checkPrivateStream() || requestMsg.priority().priorityClass() > streamRequestMsg.priority().priorityClass())
                    {
                        streamRequestMsg.priority().priorityClass(requestMsg.priority().priorityClass());
                        wlRequest._reissue_hasChange = true;
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
                    wlRequest._reissue_hasChange = true;
                }
            }
            
            // if dictionary domain, update MsgKey filter if changed
            if (requestMsg.domainType() == DomainTypes.DICTIONARY)
            {
                if (requestMsg.msgKey().filter() != streamRequestMsg.msgKey().filter())
                {
                    streamRequestMsg.msgKey().filter(requestMsg.msgKey().filter());
                    wlRequest._reissue_hasChange = true;
                } 
            }
            
            // send reissue if stream is open
            if (wlRequest.stream().state().streamState() == StreamStates.OPEN)
            {
                // handle reissue only if not in the middle of snapshot or multi-part refresh
                if (wlRequest.stream().numSnapshotsPending() == 0 &&
                    !wlRequest.stream().multiPartRefreshPending())
                {                   
                    // send message to stream
            		if( wlRequest._reissue_hasChange || wlRequest._reissue_hasViewChange)
            		{
            			if(wlRequest._reissue_hasViewChange) wlRequest.stream()._pendingViewChange = true;
            			
            			ret = wlRequest.stream().sendMsg(streamRequestMsg, submitOptions, _errorInfo);
              			if ( oldView!= null &&  wlRequest._reissue_hasViewChange)
            			{
               				// this stems from when no_refresh is set on request, but later still get refresh callback from Provider,
               				// need this flag to send fan out to all
              				 if (requestMsg.checkNoRefresh()) wlRequest.stream()._pendingViewRefresh = true;
            				_wlViewHandler.destroyView(oldView);
            			} 
            		
              			// update request state to REFRESH_PENDING if refresh is desired
              			if (!requestMsg.checkNoRefresh())
              			{
              				wlRequest.state(WlRequest.State.REFRESH_PENDING);
              			}
            		}
            		            	
            		if (streamRequestMsg.checkNoRefresh())
                    	streamRequestMsg.flags(streamRequestMsg.flags() & ~RequestMsgFlags.NO_REFRESH);
                }
                else
                {
                    // add to waiting request list
                	if(wlRequest._reissue_hasChange)
                		wlRequest.stream().waitingRequestList().add(wlRequest);
                }
            }
            else
            {
                ret = _watchlist.reactor().populateErrorInfo(errorInfo,
                                                              ReactorReturnCodes.FAILURE,
                                                              "WlItemHandler.handleRequest",
                                                              "Request reissue while stream state is known as open.");           	
            }             
        }
        else // streaming flag has changed
        {
            // streaming flag cannot be changed for a reissue
            ret = _watchlist.reactor().populateErrorInfo(errorInfo,
                                                          ReactorReturnCodes.FAILURE,
                                                          "WlItemHandler.handleRequest",
                                                          "Request reissue may not alter streaming flag.");
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
                            if (service.info().qosList().size() > 0)
                            {
                                service.info().bestQos().copy(matchedQos);
                            } 
                            else
                            {
                                matchedQos.rate(QosRates.TICK_BY_TICK);
                                matchedQos.timeliness(QosTimeliness.REALTIME);
                            }
                            
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
        WlStream wlStream = null;
        
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
        
        // item can be aggregated only for non private stream
        if (!requestMsg.checkPrivateStream()) 
        {
            wlStream = _itemAggregationKeytoWlStreamTable.get(_tempItemAggregationKey);
        }
        
        return wlStream;
    }

    /* Adds a user request to the pending request table. */
    void addToPendingRequestTable(WlRequest wlRequest, ReactorSubmitOptions submitOptions)
    {
        // set WlStream to null when starting over
        wlRequest.stream(null);
        
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
        wlStream._pendingViewChange = false;
        wlStream._requestsWithViewCount = 0;
        wlStream._pendingViewRefresh = false;        
        return wlStream;
    }

    /* Determines if a service is up and accepting requests. */
    boolean isServiceUpAndAcceptingRequests(Service service)
    {
        return (_watchlist.reactorChannel().state() == ReactorChannel.State.UP ||
        		_watchlist.reactorChannel().state() == ReactorChannel.State.READY ) && service.checkHasState() &&
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
            	
            	if (wlStream != null)
            	{
                	if (wlStream.requestPending())
                		wlStream.wlService().numOutstandingRequests(wlStream.wlService().numOutstandingRequests() - 1); 
                	
            	    ret = removeUserRequestFromOpenStream(wlRequest, msg, wlStream, submitOptions, errorInfo);
            	}
            	else
            	{
                    ret = removeUserRequestFromClosedStream(wlRequest);
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

    private int removeUserRequestFromOpenStream(WlRequest wlRequest, Msg msg, WlStream wlStream, ReactorSubmitOptions submitOptions, ReactorErrorInfo errorInfo)
    {
        int ret = ReactorReturnCodes.SUCCESS;
        
        for (int i = 0; i < wlStream.userRequestList().size(); i++)
        {
            WlRequest wlRequestInList = wlRequest.stream().userRequestList().get(i);
        
            if (wlRequestInList.requestMsg().streamId() != wlRequest.requestMsg().streamId())
                continue;
            else
            {
                wlRequest.stream().userRequestList().remove(i);
       			if ( wlRequest.requestMsg().checkPause())
    			{
    				wlRequest.stream().numPausedRequestsCount(wlRequest.stream().numPausedRequestsCount() - 1);
    			}
                
                if (wlRequest.stream().userRequestList().size() == 0 )
                {                                                  
                    if (wlRequest.stream().itemAggregationKey() != null)
                    {
                        _itemAggregationKeytoWlStreamTable.remove(wlRequest.stream().itemAggregationKey());
                        wlRequest.stream().itemAggregationKey().returnToPool();
                    }

                    msg.streamId(wlRequest.stream().streamId());
            
                    if ((ret = wlRequest.stream().sendMsg(msg, submitOptions, errorInfo)) < ReactorReturnCodes.SUCCESS)
                    {
                        return ret;
                    }

                    // If inside dispatch and fanning out to this stream, 
                    // indicate that it should be closed.
                    // Otherwise, safe to just close it.
                    if (wlStream == _currentFanoutStream)
                        _currentFanoutStream = null;
                    else
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
                    if (wlRequest.requestMsg().checkHasView() &&  wlStream._requestsWithViewCount > 0)
                    {
                    	removeRequestView(wlStream, wlRequest, errorInfo);
                    	wlStream._pendingViewChange = true;
                    }
                    else if (wlStream._requestsWithViewCount > 0 )
                    	wlStream._pendingViewChange = true;
                    	
                    // resend  
                    wlRequest.stream().requestMsg().flags(wlStream.requestMsg().flags() | RequestMsgFlags.NO_REFRESH);            		   
                    wlRequest.stream().sendMsg(wlRequest.stream().requestMsg(), submitOptions, errorInfo);
                    wlRequest.stream().requestMsg().flags(wlStream.requestMsg().flags() & ~RequestMsgFlags.NO_REFRESH);
                }                       
                
                // close watchlist request
                _watchlist.closeWlRequest(wlRequest);
                break;
            }
        }
        
        return ret;
    }
    
    private int removeUserRequestFromClosedStream(WlRequest wlRequest)
    {
        int ret = ReactorReturnCodes.SUCCESS;
        
        // remove from _userStreamIdListToRecover list
        for (int i = 0; i < _userStreamIdListToRecover.size(); i++)
        {
            int listStreamId = _userStreamIdListToRecover.get(i);
        
            if (listStreamId == wlRequest.requestMsg().streamId())
            {
                _userStreamIdListToRecover.remove(i);
                break;
            }
        }
        
        // remove from _statusMsgDispatchList list
        for (int i = 0; i < _statusMsgDispatchList.size(); i++)
        {
            StatusMsg statusMsg = _statusMsgDispatchList.get(i);
        
            if (statusMsg.streamId() == wlRequest.requestMsg().streamId())
            {
                _statusMsgDispatchList.remove(i);
                _statusMsgPool.add(statusMsg);
                break;
            }
        }        

        // remove from _pendingRequestByIdTable
        for (int intKey : _pendingRequestByIdTable.keySet())
        {
            LinkedList<WlRequest> wlRequestList = _pendingRequestByIdTable.remove(intKey);
            
            for (int i = 0; i < wlRequestList.size(); i++)
            {
                WlRequest wlRequestInList = wlRequestList.get(i);
                
                if (wlRequestInList.requestMsg().streamId() == wlRequest.requestMsg().streamId())
                {
                    wlRequestList.remove(i);
                    _pendingRequestByIdTable.put(intKey, wlRequestList);
                    break;
                }
                else
                {
                    _pendingRequestListPool.add(wlRequestList);
                }
            }
        }
        
        // remove from _pendingRequestByNameTable
        for (String stringKey : _pendingRequestByNameTable.keySet())
        {
            LinkedList<WlRequest> wlRequestList = _pendingRequestByNameTable.remove(stringKey);
            
            for (int i = 0; i < wlRequestList.size(); i++)
            {
                WlRequest wlRequestInList = wlRequestList.get(i);
                
                if (wlRequestInList.requestMsg().streamId() == wlRequest.requestMsg().streamId())
                {
                    wlRequestList.remove(i);
                    if (wlRequestList.size() > 0)
                    {
                        _pendingRequestByNameTable.put(stringKey, wlRequestList);
                    }
                    break;
                }
                else
                {
                    _pendingRequestListPool.add(wlRequestList);                    
                }
            }
        }
        
        _watchlist.closeWlRequest(wlRequest);
        
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
    int sendStatus(int streamId, int domainType, String text, boolean privateStream)
    {
        // populate StatusMsg
        _statusMsg.streamId(streamId);
        _statusMsg.domainType(domainType);
        _statusMsg.applyHasState();

        if (!privateStream && (_watchlist.loginHandler().supportSingleOpen() || _watchlist.loginHandler().supportAllowSuspectData()))
        {
            _statusMsg.state().streamState(StreamStates.OPEN);
        }
        else
        {
            _statusMsg.state().streamState(StreamStates.CLOSED_RECOVER);
        }
        _statusMsg.state().dataState(DataStates.SUSPECT);
        _statusMsg.state().text().data(text);
  
        // callback user
        _tempWlInteger.value(_statusMsg.streamId());
        return callbackUser("WlItemHandler.sendStatus", _statusMsg, null, _watchlist.streamIdtoWlRequestTable().get(_tempWlInteger), _errorInfo);
    }

    /* Queues a status message for sending on dispatch. */
    void queueStatusForDispatch(int streamId, int domainType, String text, boolean privateStream)
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
                        
        if (!privateStream && _watchlist.loginHandler().supportSingleOpen())
        {
            statusMsg.state().streamState(StreamStates.OPEN);
        }
        else
        {
            statusMsg.state().streamState(StreamStates.CLOSED_RECOVER);
        }
        statusMsg.state().dataState(DataStates.SUSPECT);
        statusMsg.state().text().data(text);
        
        // add StatusMsg to dispatch list and trigger dispatch
        _statusMsgDispatchList.add(statusMsg);
        
        if (_statusMsgDispatchList.size() == 1)
        {
            // trigger dispatch only for first add to list
            _watchlist.reactor().sendWatchlistDispatchNowEvent(_watchlist.reactorChannel());
        }
    }
    
    void handleStateTransition(WlStream wlStream, Msg msg)
    {
        // handle any state transition
        switch (wlStream.state().streamState())
        { 
            case StreamStates.CLOSED:
                handleClose(wlStream, msg);
                break;
            case StreamStates.CLOSED_RECOVER:
                if (_watchlist.loginHandler().supportSingleOpen())
                {                       
                    WlService wlService = wlStream.wlService();
                    handleCloseRecover(wlStream, msg);
                    serviceAdded(wlService);
                }
                else 
                    handleCloseRecoverStatusMsg(wlStream, msg);                 
                break;
            case StreamStates.REDIRECTED:
                handleRedirected(wlStream, msg);
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
    
    @Override
    public int readMsg(WlStream wlStream, DecodeIterator dIter, Msg msg, ReactorErrorInfo errorInfo)
    {
        int ret = ReactorReturnCodes.SUCCESS;

        _currentFanoutStream = wlStream;
        
        switch (msg.msgClass())
        {
            case MsgClasses.REFRESH:
                // if dictionary domain, create RDM dictionary message
                if (msg.domainType() == DomainTypes.DICTIONARY)
                {
                    _rdmDictionaryMsg.clear();
                    _rdmDictionaryMsg.rdmMsgType(DictionaryMsgType.REFRESH);
                    if ((ret = _rdmDictionaryMsg.decode(dIter, msg)) > CodecReturnCodes.SUCCESS)
                    {
                        _currentFanoutStream = null;
                        return ret;
                    }
                }                    
                ret = readRefreshMsg(wlStream, dIter, msg, errorInfo);
                if (ret == ReactorReturnCodes.SUCCESS && !_snapshotStreamClosed)
                {
                    handleStateTransition(wlStream, msg);
                }

                break;
            case MsgClasses.STATUS:
                // if dictionary domain, create RDM dictionary message
                if (msg.domainType() == DomainTypes.DICTIONARY)
                {
                    _rdmDictionaryMsg.clear();
                    _rdmDictionaryMsg.rdmMsgType(DictionaryMsgType.STATUS);
                    if ((ret = _rdmDictionaryMsg.decode(dIter, msg)) > CodecReturnCodes.SUCCESS)
                    {
                        _currentFanoutStream = null;
                        return ret;
                    }
                }                    
                ret =  readStatusMsg(wlStream, dIter, msg, errorInfo);
                if (ret == ReactorReturnCodes.SUCCESS)
                {
                    handleStateTransition(wlStream, msg);
                }
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

        if (_currentFanoutStream == null)
        {
            // All requests for this stream were closed within callback, close stream now.
            wlStream.close();
            return ret;
        }

    
        /* send next request in service's waiting request list */
        if (wlStream.wlService().waitingRequestList().size() > 0 && msg.msgClass() != MsgClasses.REFRESH)
        {
            WlRequest waitingRequest = wlStream.wlService().waitingRequestList().poll();
            _submitOptions.serviceName(waitingRequest.streamInfo().serviceName());
            _submitOptions.requestMsgOptions().userSpecObj(waitingRequest.streamInfo().userSpecObject());
            ret = handleRequest(waitingRequest, waitingRequest.requestMsg(), _submitOptions, true, _errorInfo);
        }        

        _currentFanoutStream = null;
        return ret;
    }

    /* Reads a refresh message. */
    int readRefreshMsg(WlStream wlStream, DecodeIterator dIter, Msg msg, ReactorErrorInfo errorInfo)
    {
        int ret = ReactorReturnCodes.SUCCESS;
        _snapshotViewClosed = false;
        _snapshotStreamClosed = false;
        int currentViewCount = 0;
        
        // have to flip the pendingViewRefresh after callback
        boolean needtoSetPendingViewRefreshFlagOff = false;
        
        boolean isRefreshComplete = ((RefreshMsg)msg).checkRefreshComplete();
        
        // notify stream that response received if solicited
        if (((RefreshMsg)msg).checkSolicited())
        {
            wlStream.responseReceived();
        }
        if( wlStream._requestsWithViewCount  > 0 && wlStream._pendingViewRefresh)
        {
        	needtoSetPendingViewRefreshFlagOff = true;
        }
        
        // set state from refresh message
        ((RefreshMsg)msg).state().copy(wlStream.state());
        
        // decrement number of outstanding requests on service
        if (isRefreshComplete)
        {
            wlStream.wlService().numOutstandingRequests(wlStream.wlService().numOutstandingRequests() - 1);   
        }
        
        // only process stream state of open here
        if ((wlStream.state().streamState() == StreamStates.OPEN && wlStream.state().dataState() == DataStates.OK) ||
        	wlStream.state().streamState() == StreamStates.NON_STREAMING)
        {
        	int listSize = wlStream.userRequestList().size();
        	int numRequestsProcessed = 0;
            // fanout refresh message to user requests associated with the stream
            for (int i = 0; i < wlStream.userRequestList().size(); i++)
            {            	
            	if (numRequestsProcessed >= listSize) break;
            	numRequestsProcessed++;
            	WlRequest wlRequest = wlStream.userRequestList().get(i);
                
                // only fanout if refresh is desired and refresh is unsolicited or to those whose state is awaiting refresh
                if (!wlRequest.requestMsg().checkNoRefresh() &&
                    (!((RefreshMsg)msg).checkSolicited() ||
                    wlRequest.state() == WlRequest.State.REFRESH_PENDING ||
                    wlRequest.state() == WlRequest.State.REFRESH_COMPLETE_PENDING ) ||
                    wlStream._pendingViewRefresh)
                {
                    // check refresh complete flag and change state of user request accordingly
                    if (isRefreshComplete)
                    {
                        // reset multi-part refresh pending flag
                        wlStream.multiPartRefreshPending(false);
                                                                      
                        WlItemGroup wlItemGroup = wlStream.wlService().itemGroupTableGet(((RefreshMsg)msg).groupId());
                        // Add group Id as new itemGroup in the WlService's itemGroupTable
                        if (wlItemGroup == null)
                        {
                        	Buffer groupId = CodecFactory.createBuffer();
                        	groupId.data(ByteBuffer.allocate( ((RefreshMsg)msg).groupId().length()));
                            ((RefreshMsg)msg).groupId().copy(groupId);
                        	wlItemGroup = ReactorFactory.createWlItemGroup();
                        	wlItemGroup.groupId(groupId);
                        	wlItemGroup.wlService(wlStream.wlService());
                            wlStream.wlService().itemGroupTablePut(groupId, wlItemGroup);
							wlStream.groupId(groupId);
                        	wlItemGroup.openStreamList().add(wlStream);
                        	WlInteger wlInteger = ReactorFactory.createWlInteger();
                            wlInteger.value(wlStream.streamId());
                            wlStream.groupTableKey(wlInteger);
                        	wlItemGroup.streamIdToItemGroupTable().put(wlInteger, wlStream);
                        }
                        else
                        {
                        	// Add group Id to current itemGroup in the WlService's itemGroupTable
                            _tempWlInteger.value(wlStream.streamId());
                            if (!wlItemGroup.streamIdToItemGroupTable().containsKey(_tempWlInteger))
                            {
								wlStream.groupId(wlItemGroup.groupId());

								wlItemGroup.openStreamList().add(wlStream);

								WlInteger wlInteger = ReactorFactory.createWlInteger();
                                wlInteger.value(wlStream.streamId());
								wlStream.groupTableKey(wlInteger);
                        		wlItemGroup.streamIdToItemGroupTable().put(wlInteger, wlStream);
                            }
                        }

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
                      
                            if( wlRequest.requestMsg().checkStreaming() && wlRequest.requestMsg().checkPause())
                            {
                            	wlRequest.stream().numPausedRequestsCount(wlRequest.stream().numPausedRequestsCount() -1);
                            }
                        }
                    }
                    else if (wlRequest.state() == WlRequest.State.REFRESH_PENDING) // multi-part refresh
                    {
                        // change user request state to REFRESH_COMPLETE_PENDING
                        wlRequest.state(WlRequest.State.REFRESH_COMPLETE_PENDING);
                        
                        // set multi-part refresh pending flag
                        wlStream.multiPartRefreshPending(true);
                        
                        // start another request timer for each part of multi-part refresh
                        wlStream.startRequestTimer(errorInfo);
                    }

                    // update stream id in message to that of user request
                    msg.streamId(wlRequest.requestMsg().streamId());
                    
                    // For snapshot requests, change OPEN state to NON-STREAMING.
                    int tmpStreamState = ((RefreshMsg)msg).state().streamState();
                    if (!wlRequest.requestMsg().checkStreaming() && tmpStreamState == StreamStates.OPEN)
                        ((RefreshMsg)msg).state().streamState(StreamStates.NON_STREAMING);
                                                           
                    // For streaming requests, change NON_STREAMING state to OPEN, and user request state to OPEN.
                    if (wlRequest.requestMsg().checkStreaming() && tmpStreamState == StreamStates.NON_STREAMING)
                    {
                        ((RefreshMsg)msg).state().streamState(StreamStates.OPEN);
                        wlRequest.state(WlRequest.State.OPEN);
                    }
                    
                    // if snapshot request or NON_STREAMING, close watchlist request and stream if necessary
                    if (!wlRequest.requestMsg().checkStreaming() ||
                        ((RefreshMsg)msg).state().streamState() == StreamStates.NON_STREAMING)
                    {
                        // remove from list
                        if (!(msg.domainType() == DomainTypes.SYMBOL_LIST 
                                && (wlRequest.symbolListFlags() & SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_SNAPSHOTS)  > 0 ))
                        {
                            wlStream.userRequestList().remove(i--);
                            if (wlStream._requestsWithViewCount > 0) 
                            {
                                if (wlRequest.requestMsg().checkHasView())
                                    removeRequestView(wlStream, wlRequest, errorInfo);     
                                wlStream._pendingViewChange = true;
                            	_snapshotViewClosed = true;
                            	// save the current view count as callbackuser() and waiting list below can potentially add views 
                            	currentViewCount = wlStream._requestsWithViewCount;
                            }
                            _submitOptions.serviceName(wlRequest.streamInfo().serviceName());
                            _submitOptions.requestMsgOptions().userSpecObj(wlRequest.streamInfo().userSpecObject());
                            _tempWlInteger.value(wlRequest.requestMsg().streamId());
                            _watchlist.streamIdtoWlRequestTable().remove(_tempWlInteger);
                        }
                        
                        if (!wlRequest.requestMsg().checkStreaming() ||
                                ((RefreshMsg)msg).state().streamState() == StreamStates.NON_STREAMING)
                        {   
                            // if no more requests in stream, close stream
                            if (wlStream.userRequestList().size() == 0 &&   
                                wlStream.waitingRequestList().size() == 0 &&
                                wlStream.wlService().waitingRequestList().size() == 0)
                            {                            
                                handleClose(wlStream, msg);
                                _pendingSendMsgList.remove(wlStream);
                                _snapshotStreamClosed = true;
                            }
                        }
                        
                        if ((ret = callbackUser("WlItemHandler.readRefreshMsg", msg, null, wlRequest, errorInfo)) < ReactorCallbackReturnCodes.SUCCESS)
                        {
                            // break out of loop for error
                            break;
                        }
                    }
                    else
                    {
                        _tempWlInteger.value(msg.streamId());
                        if ((ret = callbackUser("WlItemHandler.readRefreshMsg", msg, null, _watchlist.streamIdtoWlRequestTable().get(_tempWlInteger), errorInfo)) < ReactorCallbackReturnCodes.SUCCESS)
                        {
                            // break out of loop for error
                            break;
                        }
                    }
                    
                    // if snapshot request or NON_STREAMING, close watchlist request and stream if necessary
                    if (!wlRequest.requestMsg().checkStreaming() ||
                        ((RefreshMsg)msg).state().streamState() == StreamStates.NON_STREAMING)
                    {
                        // remove from list
                        if (!(msg.domainType() == DomainTypes.SYMBOL_LIST 
                                && (wlRequest.symbolListFlags() & SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_SNAPSHOTS)  > 0 ))
                        {
                            _watchlist.closeWlRequest(wlRequest);
                        }
                    }
                }
            }

            if(needtoSetPendingViewRefreshFlagOff) wlStream._pendingViewRefresh = false;
            
            
            /* if longer waiting for snapshot or multi-part refresh,
               send requests in waiting request list */
            if (wlStream.waitingRequestList().size() > 0 &&
                wlStream.numSnapshotsPending() == 0 &&
                !wlStream.multiPartRefreshPending())
            {
                WlRequest waitingRequest = null;
                                                
                while(!wlStream._pendingViewRefresh && (waitingRequest = wlStream.waitingRequestList().poll()) != null) 
                {
                    _submitOptions.serviceName(waitingRequest.streamInfo().serviceName());
                    _submitOptions.requestMsgOptions().userSpecObj(waitingRequest.streamInfo().userSpecObject());

                    ret = handleRequest(waitingRequest, waitingRequest.requestMsg(), _submitOptions, true, errorInfo);                    
           			if (ret < ReactorReturnCodes.SUCCESS) return ret;
                }
            }
           /* send next request in service's waiting request list */
            if (wlStream.wlService().waitingRequestList().size() > 0 && isRefreshComplete)
            {
                WlRequest waitingRequest = wlStream.wlService().waitingRequestList().poll();
                _submitOptions.serviceName(waitingRequest.streamInfo().serviceName());
                _submitOptions.requestMsgOptions().userSpecObj(waitingRequest.streamInfo().userSpecObject());

                ret = handleRequest(waitingRequest, waitingRequest.requestMsg(), _submitOptions, true, errorInfo);
            	if (ret < ReactorReturnCodes.SUCCESS) return ret;
            }
            
            if (_snapshotViewClosed)
            {
            	_snapshotViewClosed = false;

            	if ( currentViewCount > 0 && wlStream._requestsWithViewCount == currentViewCount && wlStream.userRequestList().size()  > 0 && (_wlViewHandler.resorted() ||
            	        !_wlViewHandler.commitedViewsContainsAggregateView(wlStream._aggregateView)) && wlStream._requestsWithViewCount == wlStream._userRequestList.size())
            	{ 
            		wlStream.requestMsg().flags(wlStream.requestMsg().flags() | RequestMsgFlags.NO_REFRESH);            		 
            		wlStream.sendMsg(wlStream.requestMsg(), _submitOptions, errorInfo);
                	wlStream.requestMsg().flags(wlStream.requestMsg().flags() & ~RequestMsgFlags.NO_REFRESH);
                	if (_wlViewHandler.resorted()) _wlViewHandler.resorted(false);
            	}
            }           
            
        }
        if (msg.domainType() == DomainTypes.SYMBOL_LIST)
        {
        	handleSymbolList(wlStream, msg, dIter, errorInfo);  
        }
        
        return ret;
    }

    /* Reads a update message. */
    int readUpdateMsg(WlStream wlStream, DecodeIterator dIter, Msg msg, ReactorErrorInfo errorInfo)
    {
        int ret = ReactorReturnCodes.SUCCESS;
        if (msg.domainType() == DomainTypes.SYMBOL_LIST)
        {
        	handleSymbolList(wlStream, msg, dIter, errorInfo); 
        }
                
        // fanout update message to user requests associated with the stream
        for (int i = 0; i < wlStream.userRequestList().size(); i++)
        {
            WlRequest wlRequest = wlStream.userRequestList().get(i);
                        
            // only fanout to those whose state is OPEN or REFRESH_COMPLETE_PENDING
            if (wlRequest.state() == WlRequest.State.OPEN ||
                wlRequest.state() == WlRequest.State.REFRESH_COMPLETE_PENDING)
            {
                // update stream id in message to that of user request
                msg.streamId(wlRequest.requestMsg().streamId());
                
                // callback user
                _tempWlInteger.value(msg.streamId());
                
                if ((ret = callbackUser("WlItemHandler.readUpdateMsg", msg, null, _watchlist.streamIdtoWlRequestTable().get(_tempWlInteger), errorInfo)) < ReactorCallbackReturnCodes.SUCCESS)
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
    	if ( wlStream.requestPending())
    		wlStream.wlService().numOutstandingRequests(wlStream.wlService().numOutstandingRequests() - 1); 

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
                _tempWlInteger.value(msg.streamId());
                if ((ret = callbackUser("WlItemHandler.readGenericMsg", msg, null, _watchlist.streamIdtoWlRequestTable().get(_tempWlInteger), errorInfo)) < ReactorCallbackReturnCodes.SUCCESS)
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
            _tempWlInteger.value(msg.streamId());
            ret = callbackUser("WlItemHandler.readAckMsg", msg, null, _watchlist.streamIdtoWlRequestTable().get(_tempWlInteger), errorInfo);
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
            MsgBase rdmMsg = null;
            if (statusMsg.domainType() == DomainTypes.DICTIONARY)
            {
                _rdmDictionaryMsg.clear();
                _rdmDictionaryMsg.rdmMsgType(DictionaryMsgType.STATUS);
                _watchlist.convertCodecToRDMMsg(statusMsg, _rdmDictionaryMsg);
                rdmMsg = _rdmDictionaryMsg;            
            }
            
            _tempWlInteger.value(statusMsg.streamId());
            ret = callbackUser("WlItemHandler.dispatch", statusMsg, rdmMsg, _watchlist.streamIdtoWlRequestTable().get(_tempWlInteger), errorInfo);
            
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
    int loginStreamClosed(Msg msg)
    {
    	boolean singleOpen = _watchlist.loginHandler().supportSingleOpen();
    	if (singleOpen && _watchlist.loginHandler().wlStream().state().streamState() == StreamStates.CLOSED_RECOVER)
    		handleCloseRecover(msg);
    	else
    	{
	        _statusMsg.clear();
	        _statusMsg.applyHasState();
	        _statusMsg.msgClass(MsgClasses.STATUS);
	        _statusMsg.state().text().data("Login stream was closed.");
	        handleClose(_statusMsg); 
    	}

        return ReactorReturnCodes.SUCCESS;
    }

    /* Handles directory stream open event. */
    int directoryStreamOpen()
    {
        _directoryStreamOpen = true;
        
        return ReactorReturnCodes.SUCCESS;
    }

    /* Handles directory stream closed event. */
    int directoryStreamClosed(Msg msg)
    {
        _directoryStreamOpen = false;
 
    	boolean singleOpen = _watchlist.loginHandler().supportSingleOpen();
    	if (singleOpen && _watchlist.directoryHandler()._stream.state().streamState() == StreamStates.CLOSED_RECOVER)
    		handleCloseRecover(msg);
    	else
    		handleClose(msg);
                   
        return ReactorReturnCodes.SUCCESS;
    }
    
    /* Handles pause all event. */
    int pauseAll()
    {
   	   for (WlStream wlStream : _streamList)
   	   {    	 
   	       	LinkedList<WlRequest> requestList = wlStream.userRequestList();
   	   	   
   	    	for (WlRequest usrRequest : requestList)
   	    	{        
   	    		usrRequest.requestMsg().applyPause();
   	    	}
   	    	wlStream.numPausedRequestsCount(requestList.size());
   	   }
    	     	   	   
   	   for (Map.Entry<String, LinkedList<WlRequest>> entry : _pendingRequestByNameTable.entrySet())
   	   {
   		   LinkedList<WlRequest> pendingRequestList  = (LinkedList<WlRequest>)entry.getValue();
   	   	   for (WlRequest usrRequest : pendingRequestList)
       	   { 
   	   		   usrRequest.requestMsg().applyPause();
       	   }
   	   }
   
   	   for (Map.Entry<Integer, LinkedList<WlRequest>> entry : _pendingRequestByIdTable.entrySet())
   	   {
  		   LinkedList<WlRequest> pendingRequestList  = (LinkedList<WlRequest>)entry.getValue();
   	   	   for (WlRequest usrRequest : pendingRequestList)
       	   { 
   	   		   usrRequest.requestMsg().applyPause();
       	   } 
   	   } 	   
         return ReactorReturnCodes.SUCCESS;
    }

    /* Handles resume all event. */
    int resumeAll()
    {
    	for (WlStream wlStream : _streamList)
    	{    	 
    		LinkedList<WlRequest> requestList = wlStream.userRequestList();
    		
    		if (wlStream.numPausedRequestsCount() > 0) 
    		{
    			for (WlRequest usrRequest : requestList)
    			{        
    				usrRequest.requestMsg().flags(usrRequest.requestMsg().flags() & ~RequestMsgFlags.PAUSE);
       	      	}
    			wlStream.numPausedRequestsCount(0);
       	 	}
    	}
        	     	   	   
    	for (Map.Entry<String, LinkedList<WlRequest>> entry : _pendingRequestByNameTable.entrySet())
    	{
    		LinkedList<WlRequest> pendingRequestList  = (LinkedList<WlRequest>)entry.getValue();
    		for (WlRequest usrRequest : pendingRequestList)
    		{ 
    			usrRequest.requestMsg().flags(usrRequest.requestMsg().flags() & ~RequestMsgFlags.PAUSE);
           	}
    	}
       
    	for (Map.Entry<Integer, LinkedList<WlRequest>> entry : _pendingRequestByIdTable.entrySet())
    	{
    		LinkedList<WlRequest> pendingRequestList  = (LinkedList<WlRequest>)entry.getValue();
    		for (WlRequest usrRequest : pendingRequestList)
    		{ 
    			usrRequest.requestMsg().flags(usrRequest.requestMsg().flags() & ~RequestMsgFlags.PAUSE);
    		}	 
    	} 	   
    	return ReactorReturnCodes.SUCCESS;
    }

    /* Handles channel up event. */
    void channelUp(ReactorErrorInfo errorInfo)
    {
   	   for (WlStream wlStream = _streamList.poll(); wlStream != null; wlStream = _streamList.poll())
   	   {    	   
   		   wlStream.channelUp();
   	   }
    }

    /* Handles channel down event. */
    void channelDown()
    {    	
    	boolean singleOpen = _watchlist.loginHandler().supportSingleOpen();
    	_statusMsg.clear();
		_statusMsg.applyHasState();
		_statusMsg.msgClass(MsgClasses.STATUS);
		_statusMsg.state().text().data("channel down.");
    	if (singleOpen) 
    		handleCloseRecover(_statusMsg);
    	else
    		handleCloseRecoverStatusMsg(_statusMsg);
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
                
          		if (wlRequest.requestMsg().checkNoRefresh())
          			wlRequest.requestMsg().flags(wlRequest.requestMsg().flags() & ~RequestMsgFlags.NO_REFRESH);
                                
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
        
        _userStreamIdListToRecover.clear();
                
        return ret;
    }
    
    /* Handles service updated event. */
    int serviceUpdated(WlService wlService, boolean containsServiceStateUpdate)
    {
    	int ret = ReactorReturnCodes.SUCCESS;
    	_errorInfo.clear();
		
    	// Check if streamList is empty or there are no userRequests and call serviceAdded in these cases,
    	// 	but we want to return out serviceUpdated afterwards, ignoring further processing in these cases
    	if (wlService.streamList().isEmpty())
    	{
    		ret = serviceAdded(wlService);
    		
			// We do not cache groupStateList
			wlService.rdmService().groupStateList().clear();
    		return ret;
    	}
    	else
		{
    		boolean userRequestExists = false;
			for (int streamCount = 0; streamCount < wlService.streamList().size(); streamCount++)
			{
				if (wlService.streamList().get(streamCount).userRequestList().size() != 0)
				{
					userRequestExists = true;
					break;
				}
			}
			if (!userRequestExists)
			{
				ret = serviceAdded(wlService);
				
				// We do not cache groupStateList
				wlService.rdmService().groupStateList().clear();
				return ret;
			}
		}
    	
    	ret = serviceAdded(wlService);
		if (ret < ReactorReturnCodes.SUCCESS)
		{
			return ret;
		}

    	// Check if service state is available for processing
    	if (containsServiceStateUpdate && wlService.rdmService().checkHasState() && wlService.rdmService().state().checkHasStatus())
    	{
    		/* Fanout Status */
    		for (int streamCount = 0; streamCount < wlService.streamList().size(); streamCount++)
    		{
    			int streamListSizeBefore = wlService.streamList().size();
    			StatusMsg statusMsg = _statusMsgPool.poll();
    			if (statusMsg == null)
    			{
    				statusMsg = (StatusMsg)CodecFactory.createMsg();
    			}
	            
    			statusMsg.clear();
    			statusMsg.domainType(wlService.streamList().get(streamCount).requestMsg().domainType());
    			statusMsg.msgClass(MsgClasses.STATUS);
    			statusMsg.streamId(wlService.streamList().get(streamCount).requestMsg().streamId());
        		statusMsg.applyHasState();
        		wlService.rdmService().state().status().copy(statusMsg.state());
    			
        		ret = readMsg(wlService.streamList().get(streamCount), null, statusMsg, _errorInfo);
	            
    			int streamListSizeAfter = wlService.streamList().size();
    			streamCount = streamCount - (streamListSizeBefore - streamListSizeAfter);
    			
    			// return StatusMsg to pool
    			_statusMsgPool.add(statusMsg);
	            
    			if (ret < ReactorReturnCodes.SUCCESS)
    			{
    				return ret;
    			}
    		}
    	}
    	
    	
		// Check if group states are available for processing
		for (int groupIndex = 0; groupIndex < wlService.rdmService().groupStateList().size(); ++groupIndex)
		{
			ServiceGroup serviceGroup = wlService.rdmService().groupStateList().get(groupIndex);
			if (serviceGroup.checkHasStatus())
			{
				/*	Fanout Status. */
				WlItemGroup wlItemGroup = wlService.itemGroupTableGet(serviceGroup.group());
				if (wlItemGroup != null)
				{
					for (int i = 0; i < wlItemGroup.openStreamList().size(); i++)
					{
						StatusMsg statusMsg = _statusMsgPool.poll();
						if (statusMsg == null)
						{
							statusMsg = (StatusMsg)CodecFactory.createMsg();
						}
    	            
						statusMsg.clear();
						statusMsg.domainType(wlService.itemGroupTableGet(serviceGroup.group()).openStreamList().get(i).domainType());
						statusMsg.msgClass(MsgClasses.STATUS);
						statusMsg.streamId(wlService.itemGroupTableGet(serviceGroup.group()).openStreamList().get(i).streamId());
						statusMsg.applyHasState(); 
						statusMsg.applyHasGroupId();
						statusMsg.groupId(wlItemGroup.groupId());
						serviceGroup.status().copy(statusMsg.state());
						
						ret = readMsg(wlService.itemGroupTableGet(serviceGroup.group()).openStreamList().get(i), null, statusMsg, _errorInfo);
    	            
						// return StatusMsg to pool
						_statusMsgPool.add(statusMsg);
    	            
						if (ret < ReactorReturnCodes.SUCCESS)
						{
							return ret;
						}
					}
				}	
			}
			if (serviceGroup.checkHasMergedToGroup())
			{
				WlItemGroup wlItemGroup = wlService.itemGroupTableRemove(serviceGroup.group());
				WlItemGroup newItemGroup = wlService.itemGroupTableGet(serviceGroup.mergedToGroup());
				if (wlItemGroup != null)
				{
					if (newItemGroup != null)
					{
						newItemGroup.openStreamList().addAll(wlItemGroup.openStreamList());
						newItemGroup.streamIdToItemGroupTable().putAll(wlItemGroup.streamIdToItemGroupTable());
					}
					else
					{
						Buffer groupId = CodecFactory.createBuffer();
						groupId.data(ByteBuffer.allocate(serviceGroup.mergedToGroup().length()));
						serviceGroup.mergedToGroup().copy(groupId);
						wlItemGroup.groupId(groupId);
						wlService.itemGroupTablePut(wlItemGroup.groupId(), wlItemGroup);
					}
				}
			}
		}
		// We do not cache groupStateList. After processing, clear.
		wlService.rdmService().groupStateList().clear();
    	
        return ret;
    }
        
    /* Handles service deleted event. */
    int serviceDeleted(WlService wlService, Msg msg)
    {
    	boolean singleOpen = _watchlist.loginHandler().supportSingleOpen();
    	_statusMsg.clear();
		_statusMsg.applyHasState();
		_statusMsg.msgClass(MsgClasses.STATUS);
		_statusMsg.state().text().data("Service for this item was lost.");
    	if (singleOpen) 
    		handleCloseRecover(wlService, _statusMsg);	
    	else 
    		handleCloseRecoverStatusMsg(_statusMsg);
        return ReactorReturnCodes.SUCCESS;        	
    }
    
    /* Handles all service deleted event. */
    void allServicesDeleted(Msg msg)
    {
       	boolean singleOpen = _watchlist.loginHandler().supportSingleOpen();
    	_statusMsg.clear();
		_statusMsg.applyHasState();
		_statusMsg.msgClass(MsgClasses.STATUS);
		_statusMsg.state().text().data("Service for this item was lost.");
    	if (singleOpen) 
    		handleCloseRecover(_statusMsg);
    	else 
    		handleCloseRecoverStatusMsg(_statusMsg);
    }

    /* Shallow-copies a message key for use with forwarding to a request. Also applies a service ID if available. */
    private void copyRequestKeyReferencesToMsg(WlRequest wlRequest, Msg destMsg)
    {
        MsgKey destKey = destMsg.msgKey();
        MsgKey srcKey = wlRequest.requestMsg().msgKey();

        destKey.flags(srcKey.flags());
        destKey.nameType(srcKey.nameType());
        destKey.name(srcKey.name());
        destKey.filter(srcKey.filter());
        destKey.identifier(srcKey.identifier());
        destKey.attribContainerType(srcKey.attribContainerType());
        destKey.encodedAttrib(srcKey.encodedAttrib());

        if (wlRequest.hasServiceId())
        {
            /* Request may have requested its service by name but we know the ID, so set it */
            destKey.applyHasServiceId();
            destKey.serviceId((int)wlRequest.serviceId());
        }
        else  /* Request may have requested its service by ID. */
            destKey.serviceId(srcKey.serviceId());
    }
               
    @Override
    public int callbackUser(String location, Msg msg, MsgBase rdmMsg, WlRequest wlRequest, ReactorErrorInfo errorInfo)
    {
        int ret = ReactorReturnCodes.SUCCESS;
        int msgFlagsToReset = 0;

        /* Check if we need to add a MsgKey to the message. */
        if (wlRequest != null && (
                    wlRequest.requestMsg().checkMsgKeyInUpdates() /* MsgKey requested in responses. */
                    || wlRequest.providerDriven() && !wlRequest.initialResponseReceived() /* Initial response to a provider-driven stream */))
        {
            /* Message needs to have a key. If it does not, copy it from the request. */
            switch (msg.msgClass())
            {
                case MsgClasses.UPDATE:
                    if (!((UpdateMsg)msg).checkHasMsgKey())
                    {
                        ((UpdateMsg)msg).applyHasMsgKey();
                        copyRequestKeyReferencesToMsg(wlRequest, msg);
                        msgFlagsToReset = UpdateMsgFlags.HAS_MSG_KEY;
                    }
                    break;

                case MsgClasses.REFRESH:
                    if (!((RefreshMsg)msg).checkHasMsgKey())
                    {
                        ((RefreshMsg)msg).applyHasMsgKey();
                        copyRequestKeyReferencesToMsg(wlRequest, msg);
                        msgFlagsToReset = RefreshMsgFlags.HAS_MSG_KEY;
                    }
                    
                    if (((RefreshMsg)msg).state().isFinal())
                        wlRequest.unsetServiceId();
                    break;

                case MsgClasses.STATUS:
                    if (!((StatusMsg)msg).checkHasMsgKey())
                    {
                        ((StatusMsg)msg).applyHasMsgKey();
                        copyRequestKeyReferencesToMsg(wlRequest, msg);
                        msgFlagsToReset = StatusMsgFlags.HAS_MSG_KEY;
                    }

                    if (((StatusMsg)msg).checkHasState() && ((StatusMsg)msg).state().isFinal())
                        wlRequest.unsetServiceId();
                    break;
                    
                case MsgClasses.GENERIC:
                    if (!((GenericMsg)msg).checkHasMsgKey())
                    {
                        ((GenericMsg)msg).applyHasMsgKey();
                        copyRequestKeyReferencesToMsg(wlRequest, msg);
                        msgFlagsToReset = GenericMsgFlags.HAS_MSG_KEY;
                    }
                    break;

                case MsgClasses.ACK:
                    if (!((AckMsg)msg).checkHasMsgKey())
                    {
                        ((AckMsg)msg).applyHasMsgKey();
                        copyRequestKeyReferencesToMsg(wlRequest, msg);
                        msgFlagsToReset = AckMsgFlags.HAS_MSG_KEY;
                    }
                    break;

                default:
                    /* Do nothing for classes that do not have MsgKeys. */
                    break;
            }

            wlRequest.initialResponseReceived(true);
        }

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
            if (rdmMsg == null)
            {
                rdmMsg = _rdmDictionaryMsg;
            }
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

        /* If the watchlist added the MsgKey, remove it in case subsqeuent requests on this 
           stream did not ask for MsgKeys in responses. */
        msg.flags(msg.flags() & ~msgFlagsToReset);

        // close out user request here if necessary
        if (wlRequest != null && (wlRequest.requestMsg().checkPrivateStream() || !_watchlist.loginHandler().supportSingleOpen() || (wlRequest.requestMsg().checkHasBatch())))
        {
            // close watchlist request if close of close recover status message
            if (msg.msgClass() == MsgClasses.STATUS && ((StatusMsg)msg).checkHasState() && ((StatusMsg)msg).state().streamState() != StreamStates.OPEN)
            {
                _watchlist.closeWlRequest(wlRequest);
            }
        }       

        return ret;
    }
    
    void handleCloseRecover(Msg msg)
    {
  	   for (WlStream wlStream = _streamList.poll(); wlStream != null; wlStream = _streamList.poll())
  	   {    	   
  		   wlStream.channelDown();
  		   wlStream.wlService().streamList().remove(wlStream);
           handleCloseRecover(wlStream, msg);
  	   }
    }
  	       
    void handleCloseRecover(WlStream wlStream, Msg msg)
    {       	
    	LinkedList<WlRequest> requestList = wlStream.userRequestList();		
  	   	LinkedList<WlRequest> waitingList = wlStream.wlService().waitingRequestList();
  	   	
  	   	for ( WlRequest usrRequest = waitingList.poll(); usrRequest != null; usrRequest = waitingList.poll()) 
  	   	{
  	   		usrRequest.state(State.PENDING_REQUEST);
  	   		if ( usrRequest.viewElemCount()>0) 
  	   		{
  	   			usrRequest.view().state(WlView.State.NEW);
  	   		    usrRequest.viewAction(VIEW_ACTION_SET);
  	   		}
            if (!usrRequest.requestMsg().checkPrivateStream())
            {
  	   		    addToPendingRequestTable(usrRequest, _submitOptions);
            }
            
  		   msg.streamId(usrRequest.requestMsg().streamId());
  		   msg.domainType(usrRequest.requestMsg().domainType());
  		   if (((StatusMsg)msg).checkHasState())
  		   {		                    		
  			   if (!usrRequest.requestMsg().checkPrivateStream())
  			   {    			
  				   ((StatusMsg)msg).state().streamState(StreamStates.OPEN);    			
  			   }
  			   else // private stream
  			   {
  				   ((StatusMsg)msg).state().streamState(StreamStates.CLOSED_RECOVER);    		    
  			   }
  			   ((StatusMsg)msg).state().dataState(DataStates.SUSPECT);
  		   }	
  	        _tempWlInteger.value(msg.streamId());
  		   if ((callbackUser("WlItemHandler.handleClose", msg, null, _watchlist.streamIdtoWlRequestTable().get(_tempWlInteger), _errorInfo)) < ReactorCallbackReturnCodes.SUCCESS)
  		   {
  			   System.out.println(" WlItemHandler handleCloseRecover callbackUser failed for stream " + usrRequest.requestMsg().streamId() );
  		   }
  	   	} 	  
    	
    	WlRequest usrRequest = null;
    	for (usrRequest = requestList.poll(); usrRequest != null; usrRequest = requestList.poll())
    	{
    		_userStreamIdListToRecover.add(usrRequest.requestMsg().streamId());
         	
    		usrRequest.state(State.PENDING_REQUEST);
 	   		if ( usrRequest.viewElemCount() > 0) 
  	   		{
  	   			usrRequest.view().state(WlView.State.NEW);
  	   			usrRequest.viewAction(VIEW_ACTION_SET);
  	   		}
    		_submitOptions.serviceName(usrRequest.streamInfo().serviceName());
    		_submitOptions.requestMsgOptions().userSpecObj(usrRequest.streamInfo().userSpecObject());  		

    		msg.streamId(usrRequest.requestMsg().streamId());
    		msg.domainType(usrRequest._requestMsg.domainType());
    		
    		if (((StatusMsg)msg).checkHasState())
    		{		                    		
        		if (!usrRequest.requestMsg().checkPrivateStream())
        		{    			
        			((StatusMsg)msg).state().streamState(StreamStates.OPEN);    			
        		}
        		else // private stream
        		{
        		    ((StatusMsg)msg).state().streamState(StreamStates.CLOSED_RECOVER);    		    
        		}
           		((StatusMsg)msg).state().dataState(DataStates.SUSPECT);
    		}

    		MsgBase rdmMsg = null;
    		if (wlStream.domainType() == DomainTypes.DICTIONARY)
    		{
                _rdmDictionaryMsg.clear();
                _rdmDictionaryMsg.rdmMsgType(DictionaryMsgType.STATUS);
                _watchlist.convertCodecToRDMMsg(msg, _rdmDictionaryMsg);
                rdmMsg = _rdmDictionaryMsg;
    		}
    		
            if (wlStream.domainType() != DomainTypes.DICTIONARY && !usrRequest.requestMsg().checkPrivateStream())
            {
                addToPendingRequestTable(usrRequest, _submitOptions);
            }

            if (wlStream.itemAggregationKey() != null)
            {
       	   	    _itemAggregationKeytoWlStreamTable.remove(wlStream.itemAggregationKey());
       	   	    wlStream.itemAggregationKey().returnToPool();
            }

    		if (usrRequest.providerDriven() && (((StatusMsg)msg).checkHasState()) && ((StatusMsg)msg).state().streamState() != StreamStates.OPEN)
    		{
      			_symbolListRequestKey.clear();
    			_symbolListRequestKey.msgKey(usrRequest.requestMsg().msgKey());
			    _symbolListRequestKey.msgKey().serviceId(wlStream.wlService().rdmService().serviceId());
			    _symbolListRequestKey.domainType(usrRequest.requestMsg().domainType());
			    _symbolListRequestKey.qos(usrRequest.requestMsg().qos());
			    _providerRequestTable.remove(_symbolListRequestKey);
    		}    	
            
            
            if (((StatusMsg)msg).checkHasGroupId())
            	wlStream.wlService().itemGroupTableGet(((StatusMsg)msg).groupId()).openStreamList().remove(wlStream);
            if (wlStream.wlService() != null)
            	wlStream.wlService().streamList().remove(wlStream);
        	wlStream.close();
        	
            _tempWlInteger.value(msg.streamId());
    		if ((callbackUser("WlItemHandler.handleCloseRecover", msg, rdmMsg, _watchlist.streamIdtoWlRequestTable().get(_tempWlInteger), _errorInfo)) < ReactorCallbackReturnCodes.SUCCESS)
    		{
    			System.out.println(" WlItemHandler handleCloseRecover callbackUser failed for stream " + usrRequest.requestMsg().streamId() );
    		}    
    	} 
    }    	

    void handleCloseRecover(WlService wlService, Msg msg)
    {
   	   for (WlStream wlStream = wlService.streamList().poll(); wlStream != null; wlStream = wlService.streamList().poll())
  	   {
  		   wlStream.channelDown();
 		   LinkedList<WlRequest> requestList = wlStream.userRequestList();
 	   
 		   if (requestList.size() == 0 ) continue;
 		   else 
 			   handleCloseRecover(wlStream, msg);
 			
 		  removeItemGroupTableStream(wlStream);
 		   _streamList.remove(wlStream); 		   
  	   } 	
    }    
                            
    void handleClose(Msg msg)
    {    	
 	   for (WlStream wlStream = _streamList.poll();  wlStream != null; wlStream = _streamList.poll())
 	   {    	 
 		   handleClose(wlStream, msg);
 	   }
  	   
 	   for (Map.Entry<String, LinkedList<WlRequest>> entry : _pendingRequestByNameTable.entrySet())
 	   {
 		   LinkedList<WlRequest> pendingRequestList  = (LinkedList<WlRequest>)entry.getValue();
 	   	   for (WlRequest usrRequest = pendingRequestList.poll(); usrRequest!= null; usrRequest = pendingRequestList.poll())
     	   { 
               // close watchlist request
               _watchlist.closeWlRequest(usrRequest);
     	   }
    	   _pendingRequestListPool.add(pendingRequestList); 
 	   }
 
 	   for (Map.Entry<Integer, LinkedList<WlRequest>> entry : _pendingRequestByIdTable.entrySet())
 	   {
 		   LinkedList<WlRequest> pendingRequestList  = (LinkedList<WlRequest>)entry.getValue();
 	   	   for (WlRequest usrRequest = pendingRequestList.poll(); usrRequest!= null; usrRequest = pendingRequestList.poll())
     	   { 
               // close watchlist request
               _watchlist.closeWlRequest(usrRequest);
     	   }
    	   _pendingRequestListPool.add(pendingRequestList); 
 	   } 	   
    }
    
    void handleClose(WlStream wlStream, Msg msg)
    {
    	LinkedList<WlRequest> requestList = wlStream.userRequestList();
     	   
    	WlRequest usrRequest = null;

        if (wlStream.itemAggregationKey() != null)
        {
            _itemAggregationKeytoWlStreamTable.remove(wlStream.itemAggregationKey());
            wlStream.itemAggregationKey().returnToPool();
        }

    	_streamList.remove(wlStream);
    	removeItemGroupTableStream(wlStream);
        if (wlStream.wlService() != null)
        	wlStream.wlService().streamList().remove(wlStream);
    	wlStream.close();
    	    	    	
    	for (usrRequest = requestList.poll(); usrRequest!= null; usrRequest = requestList.poll())
    	{    	 
    		msg.streamId(usrRequest.requestMsg().streamId());
    		msg.domainType(usrRequest.requestMsg().domainType());
    		((StatusMsg)msg).state().streamState(StreamStates.CLOSED);
    		((StatusMsg)msg).state().dataState(DataStates.SUSPECT);    	
            _tempWlInteger.value(msg.streamId());
    		if ((callbackUser("WlItemHandler.handleClose", msg, null, _watchlist.streamIdtoWlRequestTable().get(_tempWlInteger), _errorInfo)) < ReactorCallbackReturnCodes.SUCCESS)
    		{
    			System.out.println(" WlItemHandler handleClose callbackUser failed for stream " + usrRequest.requestMsg().streamId() );
    		}
    		
    		if (usrRequest.providerDriven())
    		{
      			_symbolListRequestKey.clear();
    			_symbolListRequestKey.msgKey(usrRequest.requestMsg().msgKey());
			    _symbolListRequestKey.msgKey().serviceId(wlStream.wlService().rdmService().serviceId());
			    _symbolListRequestKey.domainType(usrRequest.requestMsg().domainType());
			    _symbolListRequestKey.qos(usrRequest.requestMsg().qos());
			    _providerRequestTable.remove(_symbolListRequestKey);
    		}    		

            _watchlist.closeWlRequest(usrRequest);
    	}             
    }
    
    void handleCloseRecoverStatusMsg(Msg msg)
    {
  	   for (WlStream wlStream = _streamList.poll(); wlStream != null; wlStream = _streamList.poll())
  	   {    	   
  		   wlStream.channelDown();
           handleCloseRecoverStatusMsg(wlStream, msg);
  	   }
    }    
    
    void handleCloseRecoverStatusMsg(WlStream wlStream, Msg msg)
    {
    	LinkedList<WlRequest> requestList = wlStream.userRequestList();
     	   
    	WlRequest usrRequest = null;
    	
        if (wlStream.itemAggregationKey() != null)
        {
           	_itemAggregationKeytoWlStreamTable.remove(wlStream.itemAggregationKey());
        	wlStream.itemAggregationKey().returnToPool();
        }
        
    	removeItemGroupTableStream(wlStream);
    	_streamList.remove(wlStream);
    	wlStream.close();
    	
  	   	LinkedList<WlRequest> waitingList = wlStream.wlService().waitingRequestList();
  	   	
  	   	for (usrRequest = waitingList.poll(); usrRequest != null; usrRequest = waitingList.poll()) 
  	   	{
  		   msg.streamId(usrRequest.requestMsg().streamId());
  		   msg.domainType(usrRequest.requestMsg().domainType());
  		   ((StatusMsg)msg).state().streamState(StreamStates.CLOSED_RECOVER);
  		   ((StatusMsg)msg).state().dataState(DataStates.SUSPECT);    	
  	        _tempWlInteger.value(msg.streamId());
  		   if ((callbackUser("WlItemHandler.handleClose", msg, null, _watchlist.streamIdtoWlRequestTable().get(_tempWlInteger), _errorInfo)) < ReactorCallbackReturnCodes.SUCCESS)
  		   {
  			   System.out.println(" WlItemHandler handleCloseRecover callbackUser failed for stream " + usrRequest.requestMsg().streamId() );
  		   }
  	   	} 	  
    	    	    	    	
    	for (usrRequest = requestList.poll(); usrRequest!= null; usrRequest = requestList.poll())
    	{    
    		msg.streamId(usrRequest.requestMsg().streamId());
    		msg.domainType(usrRequest._requestMsg.domainType());
    		((StatusMsg)msg).state().streamState(StreamStates.CLOSED_RECOVER);
    		((StatusMsg)msg).state().dataState(DataStates.SUSPECT);

            _tempWlInteger.value(msg.streamId());
    		if ((callbackUser("WlItemHandler.handleCloseRecoverStatusMsg", msg, null, _watchlist.streamIdtoWlRequestTable().get(_tempWlInteger), _errorInfo)) < ReactorCallbackReturnCodes.SUCCESS)
    		{
    			System.out.println(" WlItemHandler handleClose callbackUser failed for stream " + usrRequest.requestMsg().streamId() );
    		}
    		
    		if (usrRequest.providerDriven())
    		{
      			_symbolListRequestKey.clear();
    			_symbolListRequestKey.msgKey(usrRequest.requestMsg().msgKey());
    		    _symbolListRequestKey.msgKey().serviceId(wlStream.wlService().rdmService().serviceId());
    		    _symbolListRequestKey.domainType(usrRequest.requestMsg().domainType());
    		    _symbolListRequestKey.qos(usrRequest.requestMsg().qos());
    		    _providerRequestTable.remove(_symbolListRequestKey);
    		}    	
            _watchlist.closeWlRequest(usrRequest);
    	}                 	
    }
                                               
    void handleOpenStatus(WlStream wlStream, Msg msg)
    {
    	boolean singleOpen = _watchlist.loginHandler().supportSingleOpen();
    	boolean allowSuspect = _watchlist.loginHandler().supportAllowSuspectData();
    	
    	if (singleOpen || allowSuspect)
    	{
        	LinkedList<WlRequest> requestList = wlStream.userRequestList();
      	   
        	for (WlRequest usrRequest : requestList)
        	{        		
        	    usrRequest.state(WlRequest.State.REFRESH_PENDING);
                msg.streamId(usrRequest.requestMsg().streamId());
                      
                _tempWlInteger.value(msg.streamId());
        		if ((callbackUser("WlItemHandler.handleOpenStatus", msg, null, _watchlist.streamIdtoWlRequestTable().get(_tempWlInteger), _errorInfo)) < ReactorCallbackReturnCodes.SUCCESS)
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
			for (WlRequest usrRequest : requestList)
			{
				msg.streamId(usrRequest.requestMsg().streamId());
				
		        _tempWlInteger.value(msg.streamId());
				if ((callbackUser("WlItemHandler.handleSuspect", msg, null, _watchlist.streamIdtoWlRequestTable().get(_tempWlInteger), _errorInfo)) < ReactorCallbackReturnCodes.SUCCESS)
				{
					System.out.println(" WlItemHandler handleStatus callbackUser failed for stream " + usrRequest.requestMsg().streamId() );
				}
			}
		}
		else // data is SUSPECT
		{
			_closeMsg.clear();
			_closeMsg.msgClass(MsgClasses.CLOSE);
			_closeMsg.streamId(wlStream.streamId());
			_closeMsg.domainType(requestList.get(0).requestMsg().domainType());
			_closeMsg.containerType(DataTypes.NO_DATA); 

			ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
			ReactorErrorInfo errorInfo = ReactorFactory.createReactorErrorInfo();
			if ((wlStream.sendMsg(_closeMsg, submitOptions, errorInfo)) < ReactorReturnCodes.SUCCESS)
			{
				System.out.println(" WlItemHandler sendCloseMsg for upstream failed for stream " + wlStream.streamId() );
			}
			
			for (WlRequest usrRequest = requestList.poll(); usrRequest != null; usrRequest = requestList.poll())
			{
	    		msg.streamId(usrRequest.requestMsg().streamId());
	    		((StatusMsg)msg).domainType(usrRequest._requestMsg.domainType());
	    		((StatusMsg)msg).state().streamState(StreamStates.CLOSED_RECOVER);
										
	            _tempWlInteger.value(msg.streamId());
				if ((callbackUser("WlItemHandler handleStatus", msg, null, _watchlist.streamIdtoWlRequestTable().get(_tempWlInteger), _errorInfo)) < ReactorCallbackReturnCodes.SUCCESS)
				{
        			System.out.println(" WlItemHandler handleStatus callbackUser failed for stream " + usrRequest.requestMsg().streamId() );
				}
                _watchlist.closeWlRequest(usrRequest);
			}			
			removeItemGroupTableStream(wlStream);
			_streamList.remove(wlStream);
			wlStream.close(); 
		}
	}
    
    void handleRedirected(WlStream wlStream, Msg msg)
    {
    	LinkedList<WlRequest> requestList = wlStream.userRequestList();
  	   
    	WlRequest usrRequest = null;
    	
        if (wlStream.itemAggregationKey() != null)
        {
            _itemAggregationKeytoWlStreamTable.remove(wlStream.itemAggregationKey());
            wlStream.itemAggregationKey().returnToPool();
        }

    	_streamList.remove(wlStream);
    	wlStream.wlService().streamList().remove(wlStream);
    	removeItemGroupTableStream(wlStream);
    	wlStream.close();
    	    	    	
    	for (usrRequest = requestList.poll(); usrRequest!= null; usrRequest = requestList.poll())
    	{    	 
    		msg.streamId(usrRequest.requestMsg().streamId());
    		msg.domainType(usrRequest._requestMsg.domainType());
            _tempWlInteger.value(msg.streamId());
    		if ((callbackUser("WlItemHandler.handleClose", msg, null, _watchlist.streamIdtoWlRequestTable().get(_tempWlInteger), _errorInfo)) < ReactorCallbackReturnCodes.SUCCESS)
    		{
    			System.out.println(" WlItemHandler handleClose callbackUser failed for stream " + usrRequest.requestMsg().streamId() );
    		}
            _watchlist.closeWlRequest(usrRequest);
    	}  
    }
        
    
    @Override
    public int requestTimeout(WlStream wlStream, ReactorErrorInfo errorInfo)
    {
    	// remove stream from service
    	 wlStream.wlService().streamList().remove(wlStream);
    	 
        // close stream and update item aggregation table
        if (wlStream.itemAggregationKey() != null)
        {
            _itemAggregationKeytoWlStreamTable.remove(wlStream.itemAggregationKey());
            wlStream.itemAggregationKey().returnToPool();
        }
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
            
            sendStatus(usrRequest.requestMsg().streamId(), usrRequest.requestMsg().domainType(), "Request timeout", usrRequest.requestMsg().checkPrivateStream());
        }
        
        // trigger dispatch
        _watchlist.reactor().sendWatchlistDispatchNowEvent(_watchlist.reactorChannel());
        
        return ReactorReturnCodes.SUCCESS;
    }
    
 	// Remove stream from itemGroupTable openStreamList
    void removeItemGroupTableStream(WlStream wlStream)
    {
    	WlItemGroup wlItemGroup = wlStream.wlService().itemGroupTableGet(wlStream.groupId());
    	if (wlItemGroup != null)
    	{
    		wlItemGroup.openStreamList().remove(wlStream);
    		_tempWlInteger.value(wlStream.streamId());
    		wlItemGroup.streamIdToItemGroupTable().remove(_tempWlInteger);
    		wlStream.groupTableKey().returnToPool();
    		// If no streams left in group's stream list, remove item group from table
    		if (wlItemGroup.openStreamList().isEmpty())
    			wlStream.wlService().itemGroupTableRemove(wlStream.groupId());
    	}
    }
    
    /* Clear state of watchlist item handler for re-use. */
    void clear()
    {
        // this handler is still associated with same watchlist so don't set watchlist to null
        _directoryStreamOpen = false;
        _tempItemAggregationRequest.clear();
        _itemAggregationKeytoWlStreamTable.clear();
		_providerRequestTable.clear();
		_streamList.clear();
		_pendingRequestByIdTable.clear();
		_pendingRequestByNameTable.clear();
        _statusMsgDispatchList.clear();
        _pendingSendMsgList.clear();
        _userStreamIdListToRecover.clear();
        _currentFanoutStream = null;
    }
    
	private int extractSymbolListFromMsg(WlRequest wlRequest, RequestMsg requestMsg, ReactorErrorInfo errorInfo)
	{
		_elementList.clear();
		_elementEntry.clear();
		_behaviourElementList.clear();
		_behaviourEntry.clear();
		_dataStreamFlag.clear();
		
		if (requestMsg.containerType() != DataTypes.ELEMENT_LIST)
			return CodecReturnCodes.SUCCESS; // nothing to extract


		_dIter.clear();
		_dIter.setBufferAndRWFVersion(requestMsg.encodedDataBody(), _watchlist.reactorChannel().majorVersion(),
    			_watchlist.reactorChannel().minorVersion());

		int ret = _elementList.decode(_dIter, null);		
		if (ret != CodecReturnCodes.SUCCESS)
		{
			_watchlist.reactor().populateErrorInfo(errorInfo,
					ReactorReturnCodes.FAILURE, "ItemHandler.extractSymbolListFromMsg",
					"DecodeElementList() failed: < " + CodecReturnCodes.toString(ret) + ">");
			return ret;
		}

		while ((ret = _elementEntry.decode(_dIter)) != CodecReturnCodes.END_OF_CONTAINER)
		{
			if (ret != CodecReturnCodes.SUCCESS)
			{
				_watchlist.reactor().populateErrorInfo(errorInfo,
						ReactorReturnCodes.FAILURE, "ItemHandler.extractSymbolListFromMsg",
						"DecodeMapEntry() failed: < " + CodecReturnCodes.toString(ret) + ">");
				return ret;
			}

			if (_elementEntry.name().equals(SymbolList.ElementNames.SYMBOL_LIST_BEHAVIORS)) 
			{
				wlRequest.hasBehaviour(true);
				
				if (_elementEntry.dataType() != DataTypes.ELEMENT_LIST) 
				{
					// Nothing to extract
					return CodecReturnCodes.SUCCESS;
				}

				ret = _behaviourElementList.decode(_dIter, null);
				if (ret == CodecReturnCodes.SUCCESS)
				{
					while ((ret = _behaviourEntry.decode(_dIter)) != CodecReturnCodes.END_OF_CONTAINER) 
					{
						if (ret < CodecReturnCodes.SUCCESS)
						{
							_watchlist.reactor().populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
											"ItemHandler.extractSymbollistFromMsg",
											"Error decoding element entry in RequestMsg.");
							return ret;
						}
						else
						{
							if (_behaviourEntry.name().equals(SymbolList.ElementNames.SYMBOL_LIST_DATA_STREAMS))
							{
								if (_behaviourEntry.dataType() != DataTypes.UINT)
								{
									_watchlist.reactor().populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
											"ItemHandler.extractSymbollistFromMsg",
											"Error decoding Symbol List Data Streams -- Element has wrong data type.");	
									return ret;								
								}

								ret = _dataStreamFlag.decode(_dIter);
								if (ret != CodecReturnCodes.SUCCESS)
								{
									_watchlist.reactor().populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
										"ItemHandler.extractSymbollistFromMsg",
										"Error decoding Symbol List Data Streams");
									return ret;
								}
	
								if (_dataStreamFlag.toBigInteger().intValue() < SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_NAMES_ONLY || _dataStreamFlag.toBigInteger().intValue() > SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_SNAPSHOTS )
								{
									_watchlist.reactor().populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
											"ItemHandler.extractSymbollistFromMsg",
											"Invalid symbol list request flags.");
									return CodecReturnCodes.FAILURE;
								}
																
								wlRequest.symbolListFlags(_dataStreamFlag.toBigInteger().intValue());
							}
						}
					}
				}
			}
		}

		return CodecReturnCodes.SUCCESS;
	}

	private int handleSymbolList(WlStream wlStream, Msg msg, DecodeIterator dIter, ReactorErrorInfo errorInfo) 
	{
		for (int i = 0; i < wlStream.userRequestList().size(); i++)
		{
			WlRequest wlRequest = wlStream.userRequestList().get(i);
			if (!wlRequest.hasBehaviour() || wlRequest.symbolListFlags() == 0 )
				continue;
			int ret = ReactorReturnCodes.SUCCESS;
			int serviceId = wlRequest.requestMsg().msgKey().serviceId();
			WlService wlService = _watchlist.directoryHandler().service(serviceId);
			if (wlService == null)
			{
				_watchlist.reactor().populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
								"ItemHandler.handleSymbolList",
								"Service for Symbol List stream is missing name. Cannot create data streams.");
				return CodecReturnCodes.FAILURE;
			}

			if (msg.containerType() != DataTypes.MAP)
				continue;
			_requestMsg.clear();
			_requestMsg.msgClass(MsgClasses.REQUEST);
			_requestMsg.domainType(DomainTypes.MARKET_PRICE);
			_requestMsg.containerType(DataTypes.NO_DATA);
			_requestMsg.applyHasQos();
			
			if ((wlRequest.symbolListFlags() & SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_STREAMS)  > 0 )
			{
				_requestMsg.applyStreaming();
			}
			_requestMsg.msgKey().applyHasNameType();
			_requestMsg.msgKey().nameType(InstrumentNameTypes.RIC);
			// not batch
			_requestMsg.msgKey().applyHasName();

			Qos itemQos;
			if (wlService.rdmService().info().qosList().size() > 0)
			{
				itemQos = wlService.rdmService().info().bestQos();
				itemQos.copy(_requestMsg.qos());
			} 
			else
			{
				_requestMsg.qos().rate(QosRates.TICK_BY_TICK);
				_requestMsg.qos().timeliness(QosTimeliness.REALTIME);
			}

			dIter.clear();
			dIter.setBufferAndRWFVersion(msg.encodedDataBody(), _watchlist.reactorChannel().majorVersion(), _watchlist
					.reactorChannel().minorVersion());
			_map.clear();
			_mapEntry.clear();
			_mapKey.clear();

			switch (msg.msgClass()) 
			{
			case MsgClasses.UPDATE:
			case MsgClasses.REFRESH:
				ret = _map.decode(dIter);
				if (ret != CodecReturnCodes.SUCCESS) 
				{
					_watchlist.reactor().populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
							"ItemHandler.handleSymbolList", "DecodeMap() failed: < "
									+ CodecReturnCodes.toString(ret) + ">");
					return ret;
				}
				while ((ret = _mapEntry.decode(dIter, _mapKey)) != CodecReturnCodes.END_OF_CONTAINER) 
				{
					if (ret != CodecReturnCodes.SUCCESS) 
					{
						_watchlist.reactor().populateErrorInfo(errorInfo,
								ReactorReturnCodes.FAILURE, "ItemHandler.handleSymbolList",
								"DecodeMapEntry() failed: < " + CodecReturnCodes.toString(ret) + ">");
						return ret;
					}

					switch (_mapEntry.action()) 
					{
					case MapEntryActions.ADD:
					case MapEntryActions.UPDATE:
						_requestMsg.msgKey().name(_mapKey);
						_symbolListRequestKey.clear();
						_symbolListRequestKey.msgKey(_requestMsg.msgKey());
						_symbolListRequestKey.msgKey().serviceId(serviceId);
						_symbolListRequestKey.domainType(_requestMsg.domainType());
						_symbolListRequestKey.qos(_requestMsg.qos());
						
						if (_providerRequestTable.containsKey(_symbolListRequestKey))
							continue;
						int providerProvideStreamId = _watchlist.nextProviderStreamId();
						_requestMsg.streamId(providerProvideStreamId);
						WlRequest newWlRequest = ReactorFactory.createWlRequest();
						newWlRequest.providerDriven(true);
						newWlRequest.streamInfo().serviceName(wlRequest.streamInfo().serviceName());
						_submitOptions.serviceName(wlRequest.streamInfo().serviceName());
						_submitOptions.requestMsgOptions().userSpecObj(wlRequest.streamInfo().userSpecObject());
						ret = handleRequest(newWlRequest, _requestMsg, _submitOptions, true, errorInfo);

						if (ret >= ReactorReturnCodes.SUCCESS) 
						{
							newWlRequest.requestMsg().clear();
							_requestMsg.copy(newWlRequest.requestMsg(), CopyMsgFlags.ALL_FLAGS);
							WlInteger wlInteger = ReactorFactory.createWlInteger();
					        wlInteger.value(providerProvideStreamId);
					        newWlRequest.handler(this);
					        newWlRequest.tableKey(wlInteger);
							_watchlist.streamIdtoWlRequestTable().put(wlInteger, newWlRequest);
							if (_requestMsg.checkStreaming())
								_providerRequestTable.put(_symbolListRequestKey, newWlRequest.requestMsg());
						} 
						else // submit failed
						{
						     newWlRequest.state(State.RETURN_TO_POOL);
							newWlRequest.returnToPool();
							
							
							
							
							
							
						}
					}// switch mapEntry
				}// while mapEntry
			default:
				return CodecReturnCodes.SUCCESS;
			}// switch msgClass
		}// wlStream loop
		return CodecReturnCodes.SUCCESS;
	}    
    	
	private int extractViewFromMsg(WlRequest wlRequest, RequestMsg requestMsg, ReactorErrorInfo errorInfo)
	{		
		wlRequest.viewElemCount(0);
		_viewDataFound = false;		
	    _viewElemCount = 0;	    
		_elementList.clear();
		_elementEntry.clear();
		_hasViewType = false;
		_viewType.clear();
		_viewDataElement.clear();
		_viewElemList = requestMsg.encodedDataBody();
		_dIter.clear();
		_dIter.setBufferAndRWFVersion(requestMsg.encodedDataBody(), _watchlist.reactorChannel().majorVersion(), 
				_watchlist.reactorChannel().minorVersion());
				
		if (requestMsg.containerType() != DataTypes.ELEMENT_LIST)
		{
			_watchlist.reactor().populateErrorInfo(errorInfo,
					ReactorReturnCodes.FAILURE, "ItemHandler.extractViewFromMsg",
					"Unexpected container type <" + requestMsg.containerType() + ">");
			return CodecReturnCodes.FAILURE;		
		}
     
        int ret;
        
    	if ( _elementList.decode(_dIter, null) != CodecReturnCodes.SUCCESS )
    	{
			_watchlist.reactor().populateErrorInfo(errorInfo,
					ReactorReturnCodes.FAILURE, "ItemHandler.extractViewFromMsg",
					"Unexpected container type <" + requestMsg.containerType() + ">");
			return CodecReturnCodes.FAILURE;    		
    		
    	}
    	
		while ((ret = _elementEntry.decode(_dIter)) != CodecReturnCodes.END_OF_CONTAINER)
		{
			if (ret != CodecReturnCodes.SUCCESS)
			{
				_watchlist.reactor().populateErrorInfo(errorInfo,
						ReactorReturnCodes.FAILURE, "ItemHandler.extractSymbolListFromMsg",
						"DecodeMapEntry() failed: < " + CodecReturnCodes.toString(ret) + ">");
				return ret;
			}
			else
			{
				if (_elementEntry.name().equals(ElementNames.VIEW_TYPE) &&
					_elementEntry.dataType() == DataTypes.UINT) 
				{
					_viewType.decode(_dIter);
					_hasViewType = true;
				}
			
				if (_elementEntry.name().equals(ElementNames.VIEW_DATA) &&
					_elementEntry.dataType() == DataTypes.ARRAY) 
				{
					_viewDataElement = _elementEntry.encodedData();
					_viewDataFound = true;
				}
			}
			
		} // while
					
		int viewType = _viewType.toBigInteger().intValue();		
		wlRequest.viewType(viewType);
	    wlRequest.viewAction(VIEW_ACTION_SET);
	    
		if (viewType == ViewTypes.FIELD_ID_LIST || viewType == ViewTypes.ELEMENT_NAME_LIST || !_hasViewType)			
		{
			if(requestMsg.domainType() == DomainTypes.SYMBOL_LIST) return CodecReturnCodes.SUCCESS;
			if (!_viewDataFound)
			{
				_watchlist.reactor().populateErrorInfo(errorInfo,
						ReactorReturnCodes.FAILURE, "ItemHandler.extractViewFromMsg",
						":ViewData element not found <" + CodecReturnCodes.INCOMPLETE_DATA + ">");
				return CodecReturnCodes.FAILURE;
			}
			else
			{
				_dIter.clear();
				_dIter.setBufferAndRWFVersion(_viewDataElement, _watchlist.reactorChannel().majorVersion(), 
						_watchlist.reactorChannel().minorVersion());
				
				switch (viewType)
				{
					case ViewTypes.FIELD_ID_LIST:
					{
						_viewArray.clear();
						if ((ret = _viewArray.decode(_dIter)) == CodecReturnCodes.SUCCESS)
						{
							if (_viewArray.primitiveType() != DataTypes.INT)
							{
								_watchlist.reactor().populateErrorInfo(errorInfo,
									ReactorReturnCodes.FAILURE, "ItemHandler.extractViewFromMsg",
									"Unexpected primitive type in array  <" + _viewArray.primitiveType() + ">");
								return CodecReturnCodes.FAILURE;							
							}	
			
							if (wlRequest._viewFieldIdList == null)	 										
								wlRequest._viewFieldIdList = _wlViewHandler._viewFieldIdListPool.poll();
							if (wlRequest._viewFieldIdList == null) wlRequest._viewFieldIdList  = new ArrayList<Integer>();
							else 
								wlRequest._viewFieldIdList.clear();;
								
							while ((ret = _viewArrayEntry.decode(_dIter)) != CodecReturnCodes.END_OF_CONTAINER)
							{								
								if (ret < CodecReturnCodes.SUCCESS)
								{
									_watchlist.reactor().populateErrorInfo(errorInfo,
										ReactorReturnCodes.FAILURE, "ItemHandler.extractViewFromMsg",
										"Error decoding array entry   <" + ret + ">");
									return ret;										
								}
								else
								{								
									if ((ret = _fieldId.decode(_dIter)) == CodecReturnCodes.SUCCESS)
									{
										if (_fieldId.toBigInteger().intValue() < -32768 || _fieldId.toBigInteger().intValue() > 32767)
										{
											_watchlist.reactor().populateErrorInfo(errorInfo,
													ReactorReturnCodes.FAILURE, "ItemHandler.extractViewFromMsg",
													"Field id in view request is outside the valid ID range <" + _fieldId + ">");
												return CodecReturnCodes.FAILURE;												
										}	
										wlRequest._viewFieldIdList.add(_fieldId.toBigInteger().intValue());
										_viewElemCount++;
									}
									else
									{
										_watchlist.reactor().populateErrorInfo(errorInfo,
												ReactorReturnCodes.FAILURE, "ItemHandler.extractViewFromMsg",
												"Invalid BLANK_DATA or incomplete data while decoding :ViewData <" + ret + ">");
										return ret;												
									}
								}								
							}// while
							wlRequest.viewElemCount(_viewElemCount);
						}
						else
						{
							_watchlist.reactor().populateErrorInfo(errorInfo,
									ReactorReturnCodes.FAILURE, "ItemHandler.extractViewFromMsg",
									"Error decoding array  <" + ret + ">");
								return ret;														
						}
						break;
					}// case
					case ViewTypes.ELEMENT_NAME_LIST:
					{
						if ((ret = _viewArray.decode(_dIter)) == CodecReturnCodes.SUCCESS)
						{
							if (!(_viewArray.primitiveType() == DataTypes.ASCII_STRING ||
									_viewArray.primitiveType() == DataTypes.UTF8_STRING ||
											_viewArray.primitiveType() == DataTypes.RMTES_STRING))
							{
								_watchlist.reactor().populateErrorInfo(errorInfo,
									ReactorReturnCodes.FAILURE, "ItemHandler.extractViewFromMsg",
									"Unexpected primitive type in array  <" + _viewArray.primitiveType() + ">");
								return CodecReturnCodes.FAILURE;							
							}
						
							if (wlRequest._viewElementNameList == null)	 	
								wlRequest._viewElementNameList = _wlViewHandler._viewElementNameListPool.poll();
							if (wlRequest._viewElementNameList == null) wlRequest._viewElementNameList = new ArrayList<String>();
							else
								wlRequest._viewElementNameList.clear();
	
							while ((ret = _viewArrayEntry.decode(_dIter)) != CodecReturnCodes.END_OF_CONTAINER)
							{
								if (ret < CodecReturnCodes.SUCCESS)
								{
									_watchlist.reactor().populateErrorInfo(errorInfo,
										ReactorReturnCodes.FAILURE, "ItemHandler.extractViewFromMsg",
										"Error decoding array entry   <" + ret + ">");
									return ret;										
								}
								else
								{						           
									if (_elementName.decode(_dIter) == CodecReturnCodes.SUCCESS)
									{
										wlRequest._viewElementNameList.add(_elementName.toString());
										_viewElemCount++;
									}
									else
									{
										_watchlist.reactor().populateErrorInfo(errorInfo,
												ReactorReturnCodes.FAILURE, "ItemHandler.extractViewFromMsg",
												"Invalid BLANK_DATA or incomplete data while decoding :ViewData <" + ret + ">");
										return ret;																					
									}
								}								
							}// while			
							wlRequest.viewElemCount(_viewElemCount);
						}
						else
						{
							_watchlist.reactor().populateErrorInfo(errorInfo,
									ReactorReturnCodes.FAILURE, "ItemHandler.extractViewFromMsg",
									"Error decoding array  <" + ret + ">");
							return ret;														
						}

						break;
					}
					default:
					{
						// non-existent viewType, but still move on, infer from primitive type 
						_viewArray.clear();
						if ((ret = _viewArray.decode(_dIter)) == CodecReturnCodes.SUCCESS)
						{
							if (_viewArray.primitiveType() == DataTypes.INT)
							{
								if (wlRequest._viewFieldIdList == null)								
									wlRequest._viewFieldIdList  = _wlViewHandler._viewFieldIdListPool.poll();
								if (wlRequest._viewFieldIdList == null) wlRequest._viewFieldIdList = new ArrayList<Integer>();
								else 
									wlRequest._viewFieldIdList.clear();
									
								while ((ret = _viewArrayEntry.decode(_dIter)) != CodecReturnCodes.END_OF_CONTAINER)
								{								
									if (ret < CodecReturnCodes.SUCCESS)
									{
										_watchlist.reactor().populateErrorInfo(errorInfo,
											ReactorReturnCodes.FAILURE, "ItemHandler.extractViewFromMsg",
											"Error decoding array entry   <" + ret + ">");
										return ret;										
									}
									else
									{								
										if ((ret = _fieldId.decode(_dIter)) == CodecReturnCodes.SUCCESS)
										{
											if (_fieldId.toBigInteger().intValue() < -32768 || _fieldId.toBigInteger().intValue() > 32767)
											{
												_watchlist.reactor().populateErrorInfo(errorInfo,
														ReactorReturnCodes.FAILURE, "ItemHandler.extractViewFromMsg",
														"Field id in view request is outside the valid ID range <" + _fieldId + ">");
													return CodecReturnCodes.FAILURE;												
											}		
											wlRequest._viewFieldIdList.add(_fieldId.toBigInteger().intValue());
											_viewElemCount++;
										}
										else
										{
											_watchlist.reactor().populateErrorInfo(errorInfo,
													ReactorReturnCodes.FAILURE, "ItemHandler.extractViewFromMsg",
													"Invalid BLANK_DATA or incomplete data while decoding :ViewData <" + ret + ">");
											return ret;												
										}
									}								
								}// while
								wlRequest.viewElemCount(_viewElemCount);
								return CodecReturnCodes.FAILURE;
							}												
							else if (_viewArray.primitiveType() == DataTypes.ASCII_STRING ||
									_viewArray.primitiveType() == DataTypes.UTF8_STRING ||
											_viewArray.primitiveType() == DataTypes.RMTES_STRING)
							{
								if (wlRequest._viewElementNameList == null)	 	
									wlRequest._viewElementNameList = _wlViewHandler._viewElementNameListPool.poll();
								if (wlRequest._viewElementNameList == null) wlRequest._viewElementNameList = new ArrayList<String>();
								
								else
									wlRequest._viewElementNameList.clear();
	
								while ((ret = _viewArrayEntry.decode(_dIter)) != CodecReturnCodes.END_OF_CONTAINER)
								{
									if (ret < CodecReturnCodes.SUCCESS)
									{
										_watchlist.reactor().populateErrorInfo(errorInfo,
												ReactorReturnCodes.FAILURE, "ItemHandler.extractViewFromMsg",
												"Error decoding array entry   <" + ret + ">");
										return ret;										
									}
									else
									{						           
										if (_elementName.decode(_dIter) == CodecReturnCodes.SUCCESS)
										{
											wlRequest._viewElementNameList.add(_elementName.toString());
											_viewElemCount++;
										}
										else
										{
											_watchlist.reactor().populateErrorInfo(errorInfo,
												ReactorReturnCodes.FAILURE, "ItemHandler.extractViewFromMsg",
												"Invalid BLANK_DATA or incomplete data while decoding :ViewData <" + ret + ">");
											return ret;																					
										}
									}								
								}// while			
								wlRequest.viewElemCount(_viewElemCount);
								return CodecReturnCodes.FAILURE;
							}
							else
							{
								_watchlist.reactor().populateErrorInfo(errorInfo,
									ReactorReturnCodes.FAILURE, "ItemHandler.extractViewFromMsg",
									"Error decoding array  <" + ret + ">");
								return ret;														
							}
						}
						else
						{							
							_watchlist.reactor().populateErrorInfo(errorInfo,
									ReactorReturnCodes.FAILURE, "ItemHandler.extractViewFromMsg",
									"Error decoding array  <" + ret + ">");
							return ret;														
						}
					
					}
				}// switch
			}// viewDataFound	
		}// if field_id_list or element_name_list
		else
		{
			_watchlist.reactor().populateErrorInfo(errorInfo,
					ReactorReturnCodes.FAILURE, "ItemHandler.extractViewFromMsg",
					":Invalid :ViewType or :ViewType not found  <" + viewType + ">");
			return CodecReturnCodes.FAILURE;
		}
		return CodecReturnCodes.SUCCESS;
	}	
	
	private int handleViews(WlRequest wlRequest, boolean isReissue, ReactorErrorInfo errorInfo)
	{
		switch(wlRequest.viewAction())
		{
			case VIEW_ACTION_SET:
			{							
				WlView view = _wlViewHandler.viewCreate(wlRequest.viewFieldIdList(), wlRequest.viewElementNameList(), wlRequest.viewElemCount(), wlRequest.viewType(), errorInfo);
				if ( view == null ) return CodecReturnCodes.FAILURE;
				wlRequest.view(view);
				break;
			}
			case VIEW_ACTION_MAINTAIN:
				break;
			case VIEW_ACTION_NONE:					
				break;
			default:
			{
				_watchlist.reactor().populateErrorInfo(errorInfo,
						ReactorReturnCodes.FAILURE, "ItemHandler",
						"Invalid View Action  <" + wlRequest.viewAction() + ">");
				return CodecReturnCodes.FAILURE;
			}
		}					
		return addRequestView(wlRequest, errorInfo);
	}
	
	private int addRequestView(WlRequest wlRequest, ReactorErrorInfo errorInfo)
	{
		WlStream wlStream = wlRequest.stream();
		if ( wlRequest.view() != null && wlStream.aggregateView() != null && 
				wlRequest.view().viewType() != wlStream.aggregateView().viewType())
		{
			_watchlist.reactor().populateErrorInfo(errorInfo,
					ReactorReturnCodes.FAILURE, "ItemHandler",
					"ViewType mismatch, cannot be aggregated.");
			return CodecReturnCodes.FAILURE;
		}
		
		wlStream._requestsWithViewCount++;
		
		wlStream._pendingViewChange = true;		
		if ( wlStream.aggregateView() == null) 
		{
			WlView aggView = _wlViewHandler.aggregateViewCreate(wlRequest.view(), errorInfo);
			if ( aggView == null ) return CodecReturnCodes.FAILURE;
			wlStream.aggregateView(aggView);
		}	
		else
		{
			if(_wlViewHandler.aggregateViewAdd(wlStream.aggregateView(), wlRequest.view(), errorInfo) < CodecReturnCodes.SUCCESS)
			{
				return CodecReturnCodes.FAILURE;
			}			
		}		
		return CodecReturnCodes.SUCCESS;		
	}
	
	private WlView removeRequestView(WlStream wlStream, WlRequest wlRequest, ReactorErrorInfo errorInfo)
	{
		_wlViewHandler.removeRequestView(wlStream, wlRequest, errorInfo);
		wlStream._pendingViewChange = true;	
		return wlRequest.view();
	}			
	
	public void putWlRequestViewListBackToPool(WlRequest wlRequest)
	{
		if(wlRequest.view() == null) return;
 
		switch(wlRequest.view().viewType())
		{		
			case ViewTypes.FIELD_ID_LIST:
				_wlViewHandler._viewFieldIdListPool.add(wlRequest._viewFieldIdList);
				break;
			case ViewTypes.ELEMENT_NAME_LIST:				
				_wlViewHandler._viewElementNameListPool.add(wlRequest._viewElementNameList);				
				break;
		}
		wlRequest.view().returnToPool();
	}	
	
	
} 

