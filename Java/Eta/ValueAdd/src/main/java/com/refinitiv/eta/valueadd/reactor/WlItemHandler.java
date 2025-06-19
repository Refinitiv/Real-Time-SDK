/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

import java.nio.ByteBuffer;
import java.util.ArrayDeque;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedHashMap;
import java.util.LinkedHashSet;
import java.util.LinkedList;
import java.util.Map;

import com.refinitiv.eta.codec.AckMsg;
import com.refinitiv.eta.codec.AckMsgFlags;
import com.refinitiv.eta.codec.Array;
import com.refinitiv.eta.codec.ArrayEntry;
import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CloseMsg;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.CopyMsgFlags;
import com.refinitiv.eta.codec.DataStates;
import com.refinitiv.eta.codec.DataTypes;
import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.ElementEntry;
import com.refinitiv.eta.codec.ElementList;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.GenericMsg;
import com.refinitiv.eta.codec.GenericMsgFlags;
import com.refinitiv.eta.codec.Int;
import com.refinitiv.eta.codec.MapEntry;
import com.refinitiv.eta.codec.MapEntryActions;
import com.refinitiv.eta.codec.Msg;
import com.refinitiv.eta.codec.MsgClasses;
import com.refinitiv.eta.codec.MsgKey;
import com.refinitiv.eta.codec.MsgKeyFlags;
import com.refinitiv.eta.codec.PostMsg;
import com.refinitiv.eta.codec.Qos;
import com.refinitiv.eta.codec.QosRates;
import com.refinitiv.eta.codec.QosTimeliness;
import com.refinitiv.eta.codec.RefreshMsg;
import com.refinitiv.eta.codec.RefreshMsgFlags;
import com.refinitiv.eta.codec.RequestMsg;
import com.refinitiv.eta.codec.RequestMsgFlags;
import com.refinitiv.eta.codec.StatusMsg;
import com.refinitiv.eta.codec.StatusMsgFlags;
import com.refinitiv.eta.codec.StreamStates;
import com.refinitiv.eta.codec.UInt;
import com.refinitiv.eta.codec.UpdateMsg;
import com.refinitiv.eta.codec.UpdateMsgFlags;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.rdm.ElementNames;
import com.refinitiv.eta.rdm.InstrumentNameTypes;
import com.refinitiv.eta.rdm.SymbolList;
import com.refinitiv.eta.rdm.ViewTypes;
import com.refinitiv.eta.transport.ChannelState;
import com.refinitiv.eta.valueadd.domainrep.rdm.MsgBase;
import com.refinitiv.eta.valueadd.domainrep.rdm.dictionary.DictionaryMsg;
import com.refinitiv.eta.valueadd.domainrep.rdm.dictionary.DictionaryMsgFactory;
import com.refinitiv.eta.valueadd.domainrep.rdm.dictionary.DictionaryMsgType;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.Service;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.Service.ServiceGroup;
import com.refinitiv.eta.valueadd.reactor.WlRequest.State;
import com.refinitiv.eta.valueadd.reactor.WlStream.RefreshStates;

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
    LinkedHashMap<WlInteger,LinkedHashSet<WlRequest>> _pendingRequestByIdTable = new LinkedHashMap<WlInteger,LinkedHashSet<WlRequest>>();
    LinkedHashMap<String,LinkedHashSet<WlRequest>> _pendingRequestByNameTable = new LinkedHashMap<String,LinkedHashSet<WlRequest>>();
    // pool of pending request lists (to avoid GC)
    LinkedList<LinkedHashSet<WlRequest>> _pendingRequestListPool = new LinkedList<LinkedHashSet<WlRequest>>();
    
    ReactorErrorInfo _errorInfo = ReactorFactory.createReactorErrorInfo();
    ReactorSubmitOptions _submitOptions = ReactorFactory.createReactorSubmitOptions();
    
    // list of streams
    LinkedHashSet<WlStream> _streamList = new LinkedHashSet<WlStream>();
    
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
    LinkedHashMap<WlInteger, StatusMsg> _statusMsgDispatchList = new LinkedHashMap<WlInteger, StatusMsg>();
    
    // List of streams with pending messages to send
    LinkedList<WlStream> _pendingSendMsgList = new LinkedList<WlStream>();
    
    // List of user requests to re-submit upon dispatch that had request timeout
    LinkedList<WlRequest> _requestTimeoutList = new LinkedList<WlRequest>();
    
    // RDM dictionary message for callback
    DictionaryMsg _rdmDictionaryMsg = DictionaryMsgFactory.createMsg();

    // in case of close recover, the streamId list of user streams 
    LinkedHashSet<WlInteger> _userStreamIdListToRecover = new LinkedHashSet<WlInteger>();
  
	// table that maps item provider request aggregation key to application
	// requests for symbol list data stream
	HashMap<WlItemAggregationKey, RequestMsg> _providerRequestTable = new HashMap<WlItemAggregationKey, RequestMsg>();
	RequestMsg _requestMsg = (RequestMsg) CodecFactory.createMsg();
	WlItemAggregationKey _symbolListRequestKey = ReactorFactory
			.createWlItemAggregationKey();
	com.refinitiv.eta.codec.Map _map = CodecFactory.createMap();
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
	Int _fieldId = CodecFactory.createInt();
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

    // Set whenever a message is being fanned out on a stream. If all requests are closed,
    // this prevents repooling the stream too soon.
    WlStream _currentFanoutStream;
    boolean _hasPendingViewRequest = false;
    // Points to the state of a message being fanned out on a stream. 
    // Set to null if recovery is not needed.
    com.refinitiv.eta.codec.State _msgState;
		
    WlItemHandler(Watchlist watchlist)
    {
        _watchlist = watchlist;
        _defaultQos.clear();
        _defaultQos.timeliness(QosTimeliness.REALTIME);
        _defaultQos.rate(QosRates.TICK_BY_TICK);
        _statusMsg.msgClass(MsgClasses.STATUS);
        _wlViewHandler = new WlViewHandler(watchlist);
        _itemAggregationKeytoWlStreamTable = new LinkedHashMap<WlItemAggregationKey,WlStream>(_watchlist.role().watchlistOptions().itemCountHint() + 10, 1);
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
               			if ( (ret = handleViews(wlRequest, errorInfo)) < ReactorReturnCodes.SUCCESS)
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

					if ( requestMsg.checkPause() && requestMsg.checkStreaming())
						wlStream.numPausedRequestsCount(1);
                    
                    // send message to stream
                    if (sendNow)
                    {
                        // send now
                        ret = wlStream.sendMsgOutOfLoop(_tempItemAggregationRequest, submitOptions, _errorInfo);
                        wlStream.refreshState( _tempItemAggregationRequest.checkHasView() ? WlStream.RefreshStates.REFRESH_VIEW_PENDING :
                        						WlStream.RefreshStates.REFRESH_PENDING);
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
                        
                        // update request state to PENDING_REFRESH
                        wlRequest.state(WlRequest.State.PENDING_REFRESH);
                        
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
                    
                    /* Add request to stream if the stream is not waiting for a refresh or the stream is streaming but it it not waiting for multi-part and view pending */
                    if( !wlStream.requestPending() || wlStream.refreshState() == WlStream.RefreshStates.REFRESH_NOT_REQUIRED ||
                    	( wlStream.requestMsg().checkStreaming() && wlStream.refreshState() != RefreshStates.REFRESH_COMPLETE_PENDING && 
                    	  wlStream.refreshState() != RefreshStates.REFRESH_VIEW_PENDING )
                    	)
                    {                   	
                        if (requestMsg.checkHasView())
                        {   
                            if ( (ret = handleViews(wlRequest, errorInfo)) < ReactorReturnCodes.SUCCESS)
                                return ret;
                        }
                		
                        // add request to stream
                        wlStream.userRequestList().add(wlRequest);

                        if ( wlRequest.requestMsg().checkPause() && wlRequest.requestMsg().checkStreaming())
                        	wlStream.numPausedRequestsCount(wlStream.numPausedRequestsCount() + 1);
                                                
                        // update request state to PENDING_REFRESH
                        wlRequest.state(WlRequest.State.PENDING_REFRESH);
    
                        // retrieve request from stream
                        RequestMsg streamRequestMsg = wlStream.requestMsg();
                        
                        // increment number of snapshots pending
                        if (!requestMsg.checkStreaming())
                        {
                            // Join the existing request if the current request haven't received a refresh yet. 
                            if( wlStream.refreshState() == WlStream.RefreshStates.REFRESH_PENDING)
                            	return ret;
                        }
                        else
                        {
	                        // Update priority for streaming requests only
	                        if (requestMsg.checkHasPriority())
	                        {
	                            // use priorityClass of request if greater than existing one
	                            if (requestMsg.priority().priorityClass() > streamRequestMsg.priority().priorityClass())
	                            {
	                                streamRequestMsg.priority().priorityClass(requestMsg.priority().priorityClass());
	                            }
	                            
	                            // add priorityCount to that of existing one
	                            if (!streamRequestMsg.checkStreaming())
	                            	streamRequestMsg.priority().count(requestMsg.priority().count());
	                            else
	                            	streamRequestMsg.priority().count(streamRequestMsg.priority().count() + requestMsg.priority().count());
	                        }
	                        else // request has no priority, assume default of 1/1
	                        {
	                            streamRequestMsg.applyHasPriority();
	                            if (!streamRequestMsg.checkStreaming())
	                            	streamRequestMsg.priority().count(1);
	                            else
	                            	streamRequestMsg.priority().count(streamRequestMsg.priority().count() + 1);
	                        }

	                        if (!streamRequestMsg.checkStreaming())
	                            streamRequestMsg.applyStreaming();
                        }

                        if (sendNow)
                        {
                            // increment number of outstanding requests if not dictionary domain and a request isn't currently pending
                            if (requestMsg.domainType() != DomainTypes.DICTIONARY && !wlStream.requestPending() && !requestMsg.checkNoRefresh())
                            {
                                wlService.numOutstandingRequests(wlService.numOutstandingRequests() + 1);
                            }

                            ret = wlStream.sendMsgOutOfLoop(streamRequestMsg, submitOptions, _errorInfo);
                            wlStream.refreshState(streamRequestMsg.checkHasView() ? WlStream.RefreshStates.REFRESH_VIEW_PENDING : WlStream.RefreshStates.REFRESH_PENDING );
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
                    else
                    {  
                    	// WlStream is waiting a response for snapshot request and the user's request is snapshot without view
                    	if( (!wlStream.requestMsg().checkStreaming() && (wlStream.refreshState() == WlStream.RefreshStates.REFRESH_PENDING )) &&
                    		!requestMsg.checkStreaming() && !requestMsg.checkHasView() )
                    	{
                    	    // join the snapshot request to the existing snapshot stream
                    		wlRequest.state(State.PENDING_REFRESH);
                            wlStream.userRequestList().add(wlRequest);
                    	}
                    	else
                    	{
                    	    // currently in the middle of snapshot, multi-part refresh or pending view
                    	    wlRequest.state(State.PENDING_REQUEST);
                    	    wlStream.waitingRequestList().add(wlRequest);
                    	}
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
            	_tempWlInteger.value(requestMsg.streamId());
            	if (_userStreamIdListToRecover.contains(_tempWlInteger))
            	{
            		queueStatusForDispatch(requestMsg.streamId(), requestMsg.domainType(), errorInfo.error().text(), requestMsg.checkPrivateStream());
            	}
            }

            if (!requestMsg.checkPrivateStream() && _watchlist.loginHandler().supportSingleOpen())
            {
            	// wlRequest._requestMsg may not be set yet.  However, if submitOptions.serviceName is not set,
            	// addToPendingRequestTable uses the serviceId from wlRequst._requestMsg.msgKey so just
            	// set it here
            	if (submitOptions.serviceName() == null)
            		wlRequest.requestMsg().msgKey().serviceId(requestMsg.msgKey().serviceId());
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
    	Buffer itemName = CodecFactory.createBuffer();
    	Buffer encodedDataBody = null;
    	ArrayDeque<String> itemNames = new ArrayDeque<String>();
    	int retDecodeVal = CodecReturnCodes.SUCCESS;
    	
    	// Make sure requestMsg does not have item name set on MsgKey
    	if (requestMsg.msgKey().checkHasName())
    	{
            return _watchlist.reactor().populateErrorInfo(errorInfo,
            		ReactorReturnCodes.FAILURE,
                    "WlItemHandler.processBatchRequest",
                    "Requested batch has name in message key.");
    	}
    	
		if (requestMsg.domainType() == DomainTypes.SYMBOL_LIST)
		{
			if (( ret = extractSymbolListFromMsg(wlRequest, requestMsg, errorInfo)) < ReactorReturnCodes.SUCCESS)
				return ret;
		}
		
		_dIterBatch.clear();
		_dIterBatch.setBufferAndRWFVersion(requestMsg.encodedDataBody(), _watchlist.reactorChannel().majorVersion(),
    			_watchlist.reactorChannel().minorVersion());
		
	    wlRequest.streamInfo().serviceName(submitOptions.serviceName());
	    wlRequest.streamInfo().userSpecObject(submitOptions.requestMsgOptions().userSpecObj());
	    
    	WlInteger wlInteger = ReactorFactory.createWlInteger();
        wlInteger.value(requestMsg.streamId());
    	_watchlist.streamIdtoWlRequestTable().put(wlInteger, wlRequest);

    	if (requestMsg.containerType() == DataTypes.ELEMENT_LIST)
    	{
        	if ((retDecodeVal = elementList.decode(_dIterBatch, null)) <= CodecReturnCodes.FAILURE)
        	{
                return _watchlist.reactor().populateErrorInfo(errorInfo,
                        ReactorReturnCodes.FAILURE,
                        "WlItemHandler.handleBatchRequest",
                        "ElementList.decode() failure.");
        	}
        	
        	int entryCount = 0;
        	int itemListDataSize = 0;
        	
        	// check element list for itemList
    		while ((retDecodeVal = elementEntry.decode(_dIterBatch)) != CodecReturnCodes.END_OF_CONTAINER)
    		{
    			if (retDecodeVal <= CodecReturnCodes.FAILURE)
    			{
    	            return _watchlist.reactor().populateErrorInfo(errorInfo,
    	            		ReactorReturnCodes.FAILURE,
    	                    "WlItemHandler.handleBatchRequest",
    	                    "ElementEntry.decode() failure.");
    			}
    			
    			entryCount++;
    			
    			if (elementEntry.name().toString().contains(ElementNames.BATCH_ITEM_LIST.toString()))
				{
    				itemListDataSize = ElementNames.BATCH_ITEM_LIST.length();
    				itemListDataSize += elementEntry.encodedData().length();
    				
					foundBatch = true;
					
					if ((retDecodeVal = batchArray.decode(_dIterBatch)) <= CodecReturnCodes.FAILURE)
					{
			            return _watchlist.reactor().populateErrorInfo(errorInfo,
			            		ReactorReturnCodes.FAILURE,
			                    "WlItemHandler.handleBatchRequest",
			                    "Array.decode() failure.");
					}
					
					while ((retDecodeVal = batchArrayEntry.decode(_dIterBatch)) != CodecReturnCodes.END_OF_CONTAINER)
					{
						if ((retDecodeVal = itemName.decode(_dIterBatch)) == CodecReturnCodes.SUCCESS)
						{
							itemNames.add(itemName.toString());
						}
						else
						{
							return _watchlist.reactor().populateErrorInfo(errorInfo,
				            		ReactorReturnCodes.FAILURE,
				                    "WlItemHandler.handleBatchRequest",
				                    "Invalid BLANK_DATA while decoding :ItemList -- " + CodecReturnCodes.toString(ret));
						}
					}
				}
    			
    			// Found the batch item list and others entry
    			if(foundBatch && (entryCount > 1))
    			{
    				// Encodes a separate ElementList for Element names which is not itemList:
    	    			Buffer encodedBuffer = CodecFactory.createBuffer();
    	    			encodedBuffer.data(ByteBuffer.allocate(requestMsg.encodedDataBody().length() - itemListDataSize));
    	    	        EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
    	    	        ElementEntry elementEntryEnc = CodecFactory.createElementEntry();
    	    	        ElementList elementListEnc = CodecFactory.createElementList();
    	    	        DecodeIterator decodeIter = CodecFactory.createDecodeIterator();
    	    	        
    	    	        encodeIter.clear();
    	    	        encodeIter.setBufferAndRWFVersion(encodedBuffer, _watchlist.reactorChannel().majorVersion(), _watchlist.reactorChannel().minorVersion());
    	    	        
    	    	        elementListEnc.clear();
    	    	        
    	    	        // Copies attributes from the original ElementList
    	    	        elementListEnc.flags(elementList.flags());
    	    	        if (elementList.checkHasInfo())
    	    	        	elementListEnc.elementListNum(elementList.elementListNum());
    	    	        
    	    	        if (elementList.checkHasSetData())
    	    	        	elementListEnc.encodedSetData(elementList.encodedSetData());
    	    	        
    	    	        if(elementList.checkHasSetId())
    	    	        	elementListEnc.setId(elementList.setId());
    	    	        
    	    	        if ((ret = elementListEnc.encodeInit(encodeIter, null, 0)) <= CodecReturnCodes.FAILURE)
    	    	        {
    	    	        	return _watchlist.reactor().populateErrorInfo(errorInfo,
    	                            ReactorReturnCodes.FAILURE,
    	                            "WlItemHandler.handleBatchRequest",
    	                            "ElementList.encodeInit() failure.");
    	    	        }
    	    	        
    	    	        decodeIter.clear();
    	    	        decodeIter.setBufferAndRWFVersion(requestMsg.encodedDataBody(), _watchlist.reactorChannel().majorVersion(),
    	    	    			_watchlist.reactorChannel().minorVersion());
    	    			
    	    			if ((ret = elementList.decode(decodeIter, null)) <= CodecReturnCodes.FAILURE)
    	            	{
    	                    return _watchlist.reactor().populateErrorInfo(errorInfo,
    	                            ReactorReturnCodes.FAILURE,
    	                            "WlItemHandler.handleBatchRequest",
    	                            "ElementList.decode() failure.");
    	            	}
    	            	
    	        		while ((ret = elementEntry.decode(decodeIter)) != CodecReturnCodes.END_OF_CONTAINER)
    	        		{
    	        			if (ret <= CodecReturnCodes.FAILURE)
    	        			{
    	        	            return _watchlist.reactor().populateErrorInfo(errorInfo,
    	        	            		ReactorReturnCodes.FAILURE,
    	        	                    "WlItemHandler.handleBatchRequest",
    	        	                    "ElementEntry.decode() failure.");
    	        			}
    	        			
    	        			if (!elementEntry.name().toString().contains(ElementNames.BATCH_ITEM_LIST.toString()))
    	    				{
    	        				elementEntryEnc.clear();
    	        				elementEntryEnc.name(elementEntry.name());
    	        				elementEntryEnc.dataType(elementEntry.dataType());
    	        				elementEntryEnc.encodedData(elementEntry.encodedData());
    	        				
    	        				ret = elementEntryEnc.encode(encodeIter);
    	        				if (ret <= CodecReturnCodes.FAILURE)
    	            			{
    	            	            return _watchlist.reactor().populateErrorInfo(errorInfo,
    	            	            		ReactorReturnCodes.FAILURE,
    	            	                    "WlItemHandler.handleBatchRequest",
    	            	                    "ElementEntry.encode() failure.");
    	            			}
    	    				}
    	        		}
    	        		
    	        		if ((ret = elementListEnc.encodeComplete(encodeIter, true)) <= CodecReturnCodes.FAILURE)
    	    	        {
    	    	        	return _watchlist.reactor().populateErrorInfo(errorInfo,
    	                            ReactorReturnCodes.FAILURE,
    	                            "WlItemHandler.handleBatchRequest",
    	                            "ElementList.encodeComplete() failure.");
    	    	        }
    	        		
    	        		encodedDataBody = encodeIter.buffer();
    	    		
    	    		break; // Breaks from the ElementList of the request message
    			}
    		}
    		
    		if(!foundBatch)
    		{
    			return _watchlist.reactor().populateErrorInfo(errorInfo,
                        ReactorReturnCodes.FAILURE,
                        "WlItemHandler.handleBatchRequest",
                        ":ItemList not found.");
    		}
    	}
    	else
    	{
            return _watchlist.reactor().populateErrorInfo(errorInfo,
            		ReactorReturnCodes.FAILURE,
                    "WlItemHandler.handleBatchRequest",
                    "Unexpected container type or decoding error.");
    	}
    	
		// found itemList, thus a batch. Make individual item requests from array of requests
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
		
		HashMap<Integer, WlRequest> wlRequestList = new HashMap<Integer, WlRequest>(itemNames.size());
		HashMap<Integer, RequestMsg> requestMsgList = new HashMap<Integer, RequestMsg>(itemNames.size());
		int possibleStreamId = currentStreamId;
		
		while(!itemNames.isEmpty()) 
		{
			itemName.data(itemNames.remove());

			_tempWlInteger.value(possibleStreamId);
			if (_watchlist.streamIdtoWlRequestTable().get(_tempWlInteger) != null)
			{
				while (!wlRequestList.isEmpty())
				{
	            	WlRequest removeWlRequest = wlRequestList.remove(currentStreamId);
	            	repoolWlRequest(removeWlRequest);
	            	requestMsgList.remove(currentStreamId);
	            	currentStreamId++;
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
        	

        	// Remove batch flag and do not copy the encoded data body from the batch request
        	requestMsg.copy(newRequestMsg, CopyMsgFlags.ALL_FLAGS & (~CopyMsgFlags.DATA_BODY));
        	newRequestMsg.flags(newRequestMsg.flags() & ~RequestMsgFlags.HAS_BATCH);
        	newRequestMsg.streamId(possibleStreamId);
        	
        	newRequestMsg.msgClass(MsgClasses.REQUEST);
        	
        	// Set msgKey item name
        	newRequestMsg.applyMsgKeyInUpdates();
        	newRequestMsg.msgKey().applyHasName();
    		newRequestMsg.msgKey().name(itemName);
    		
    		// Set encoded data body which does not have the :ItemList entry name
    		if ( encodedDataBody != null ) 
    		{
    			newRequestMsg.encodedDataBody(encodedDataBody);
    		}
    		else
    		{
    			// Unset the container type when there is no encoded databody
    			newRequestMsg.containerType(DataTypes.NO_DATA);
    		}

        	if ((ret = newRequestMsg.copy(newWlRequest.requestMsg(), CopyMsgFlags.ALL_FLAGS)) <= CodecReturnCodes.FAILURE)
        	{
        		return _watchlist.reactor().populateErrorInfo(errorInfo,
                        ReactorReturnCodes.FAILURE,
                        "WlItemHandler.handleBatchRequest",
                        "RequestMsg.copy() failure.");
        	}
        	
        	wlRequestList.put(possibleStreamId, newWlRequest);
        	requestMsgList.put(possibleStreamId, newRequestMsg);
    	
            possibleStreamId++;
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
		
		/* Requests created. Make a request for the batch stream so it can be acknowledged. */
		StatusMsg statusMsg = _statusMsgPool.poll();
		if (statusMsg == null)
		{
			statusMsg = (StatusMsg)CodecFactory.createMsg();
		}
        
		statusMsg.clear();
		
        statusMsg.domainType(requestMsg.domainType());
		statusMsg.msgClass(MsgClasses.STATUS);
		statusMsg.streamId(originalStreamId);
		statusMsg.applyHasState();
	    statusMsg.state().streamState(StreamStates.CLOSED);
		statusMsg.state().dataState(DataStates.OK);
		Buffer statusText = CodecFactory.createBuffer();
		statusText.data("Stream closed for batch");
		statusMsg.state().text(statusText);
		
		wlInteger = ReactorFactory.createWlInteger();
		wlInteger.value(originalStreamId);
		_statusMsgDispatchList.put(wlInteger, statusMsg);
		
        if (_statusMsgDispatchList.size() == 1)
        {
            // trigger dispatch only for first add to list
            _watchlist.reactor().sendWatchlistDispatchNowEvent(_watchlist.reactorChannel());
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
                    "WlItemHandler.handleReissue",
                    "Reissue request not allowed on an unopen stream.");
            return ret;
        }
    	wlRequest._reissue_hasChange = (!requestMsg.checkNoRefresh() && !requestMsg.checkPause());
    	wlRequest._reissue_hasViewChange = false;
                
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
            boolean repooled = false;
                      
            WlRequest tempWlRequest = ReactorFactory.createWlRequest();
            
    		if (requestMsg.checkHasView())  // has viewFlag
    		{    			
    			// for re-issue, in case incoming request does not have view data, re-use the cached one
    			if (requestMsg.encodedDataBody().data() == null )
    			{
    				requestMsg.encodedDataBody(wlRequest.requestMsg().encodedDataBody());
    				requestMsg.containerType(wlRequest.requestMsg().containerType());
    			}
    			
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
    			if ( (ret = handleViews(wlRequest, errorInfo)) < ReactorReturnCodes.SUCCESS)
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
            	repoolWlRequest(tempWlRequest);
            	repooled = true;
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
            
	            if (wlRequest.stream().state().streamState() == StreamStates.OPEN)
	            {
	                // handle reissue only if not in the middle of multi-part refresh
	                if ( wlRequest.stream().refreshState() != WlStream.RefreshStates.REFRESH_COMPLETE_PENDING )
	                {                   
	                    // send message to stream
	            		if( wlRequest._reissue_hasChange || wlRequest._reissue_hasViewChange)
	            		{
	            			if(wlRequest._reissue_hasViewChange) wlRequest.stream()._pendingViewChange = true;
	            			
	            			ret = wlRequest.stream().sendMsgOutOfLoop(streamRequestMsg, submitOptions, _errorInfo);
	              			
	            			// Check if we repooled already, making the view null and already destroyed
	            			if (!repooled && oldView!= null &&  wlRequest._reissue_hasViewChange)
	            			{
	               				// this stems from when no_refresh is set on request, but later still get refresh callback from Provider,
	               				// need this flag to send fan out to all
	              				 if (requestMsg.checkNoRefresh()) wlRequest.stream().refreshState(WlStream.RefreshStates.REFRESH_VIEW_PENDING);
	            				_wlViewHandler.destroyView(oldView);
	            			} 
	            			
	            		
	              			// update request state to PENDING_REFRESH if refresh is desired
	              			if (!requestMsg.checkNoRefresh())
	              			{
	              				wlRequest.state(WlRequest.State.PENDING_REFRESH);
	              			}
	            		}
	            		            	
	            		if (streamRequestMsg.checkNoRefresh())
	                    	streamRequestMsg.flags(streamRequestMsg.flags() & ~RequestMsgFlags.NO_REFRESH);
	            		
	            		if (_hasPendingViewRequest)
	            		{
	            			wlRequest.stream().waitingRequestList().add(wlRequest);
	            			_hasPendingViewRequest = false;
	            		}
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
	                                                              "WlItemHandler.handleReissue",
	                                                              "Reissue requests must occur while stream state is known as open.");           	
	            }

        }
        else // streaming flag has changed
        {
            // streaming flag cannot be changed for a reissue
            ret = _watchlist.reactor().populateErrorInfo(errorInfo,
                                                          ReactorReturnCodes.FAILURE,
                                                          "WlItemHandler.handleReissue",
                                                          "Reissue requests may not alter streaming flag.");
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
        requestMsg.msgKey().copy(_tempItemAggregationKey.msgKey());
        if (submitOptions.serviceName() != null)
        {
            // set service id in item aggregation key if requested by service name
            int serviceId = _watchlist.directoryHandler().serviceId(submitOptions.serviceName());
            _tempItemAggregationKey.msgKey().applyHasServiceId();
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
    	LinkedHashSet<WlRequest> pendingRequestList = null;
    	if (submitOptions.serviceName() != null)
    	{
    		pendingRequestList = _pendingRequestByNameTable.get(submitOptions.serviceName());
    	}
    	else
    	{
    		_tempWlInteger.value(wlRequest.requestMsg().msgKey().serviceId());
    		pendingRequestList = _pendingRequestByIdTable.get(_tempWlInteger);
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
    			pendingRequestList = new LinkedHashSet<WlRequest>();
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
    			WlInteger wlInteger = ReactorFactory.createWlInteger();
    			wlInteger.value(wlRequest.requestMsg().msgKey().serviceId());
    			_pendingRequestByIdTable.put(wlInteger, pendingRequestList);
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
        wlStream.refreshState(WlStream.RefreshStates.REFRESH_NOT_REQUIRED);        
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
                	if (wlStream.requestPending() && wlStream.wlService() != null)
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
                    boolean resetServiceId = false;
                    
                    // replace service id if message submitted with service name
                    if (submitOptions.serviceName() != null)
                    {
                        if (!((GenericMsg)msg).checkHasMsgKey())
                        {
                            return _watchlist.reactor().populateErrorInfo(errorInfo,
                                                                          ReactorReturnCodes.INVALID_USAGE,
                                                                          "WlItemHandler.submitMsg",
                                                                          "Generic message submitted with service name but no message key.");
                            
                        }
                        
                        if ((ret = _watchlist.changeServiceNameToID(((GenericMsg)msg).msgKey(), submitOptions.serviceName(), errorInfo)) < ReactorReturnCodes.SUCCESS)
                        {
                            return ret;
                        }
                        
                        // set resetServiceId flag
                        resetServiceId = true;
                    }
                    
                 
                    // replace stream id with aggregated stream id
                    msg.streamId(wlRequest.stream().streamId());
                    
                    // send message
                    ret = wlRequest.stream().sendMsgOutOfLoop(msg, submitOptions, errorInfo);
                    
                    // reset service id if necessary
                    if (resetServiceId)
                    {
                        msg.msgKey().flags(msg.msgKey().flags() & ~MsgKeyFlags.HAS_SERVICE_ID);
                        msg.msgKey().serviceId(0);
                    }              
                    
                    // return if send message not successful
                    if (ret < ReactorReturnCodes.SUCCESS)
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
    	
        Iterator<WlRequest> wlRequestIter = wlStream.waitingRequestList().iterator();
        while (wlRequestIter.hasNext())
        {
        	WlRequest wlRequestInList = wlRequestIter.next();
        	
        	if (wlRequestInList.requestMsg().streamId() != wlRequest.requestMsg().streamId())
                continue;
            else
            {
            	wlRequestIter.remove();
                
                // close watchlist request
                closeWlRequest(wlRequest);
                repoolWlRequest(wlRequest);
                return ret;
            }
        }
        
        wlRequestIter = wlStream.userRequestList().iterator();
        while (wlRequestIter.hasNext())
        {
        	WlRequest wlRequestInList = wlRequestIter.next();
            
            if (wlRequestInList.requestMsg().streamId() != wlRequest.requestMsg().streamId())
                continue;
            else
            {
            	wlRequestIter.remove();

            	wlStream.refreshState(WlStream.RefreshStates.REFRESH_NOT_REQUIRED);
            	
       			if (wlRequest.requestMsg().checkPause())
    			{
       				wlStream.numPausedRequestsCount(wlStream.numPausedRequestsCount() - 1);
    			}
       			
       			if (wlRequest.requestMsg().checkHasView() &&  wlStream._requestsWithViewCount > 0)
       			{
       				removeRequestView(wlStream, wlRequest, errorInfo);
       				wlStream._pendingViewChange = true;
       			}
       			else if (wlStream._requestsWithViewCount > 0 )
       				wlStream._pendingViewChange = true;
                
       			if (wlStream.state().streamState() == StreamStates.OPEN)
       			{
                    // Stream is open; need to change priority or close it.
	                if (wlStream.userRequestList().size() == 0)
	                {                        
                        closeWlStream(wlRequest.stream());
                        
                        msg.copy(_closeMsg, CopyMsgFlags.NONE);
	
                        _closeMsg.streamId(wlRequest.stream().streamId());
	            
	                    if ((ret = wlRequest.stream().sendMsgOutOfLoop(_closeMsg, submitOptions, errorInfo)) < ReactorReturnCodes.SUCCESS)
	                    {
	                        return ret;
	                    }

	                    // If inside dispatch reading a message on this stream,
                        // don't repool it yet. The fanout will still be accessing it, e.g. iterating over its userRequestList.
                        // If not in dispatch, however, it's safe to repool it now.
	                    if (wlStream == _currentFanoutStream)
	                        _currentFanoutStream = null;                          
	                    else
	                        wlRequest.stream().returnToPool();
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
	                    wlRequest.stream().requestMsg().flags(wlStream.requestMsg().flags() | RequestMsgFlags.NO_REFRESH);            		   
	                    wlRequest.stream().sendMsgOutOfLoop(wlRequest.stream().requestMsg(), submitOptions, errorInfo);
	                    wlRequest.stream().requestMsg().flags(wlStream.requestMsg().flags() & ~RequestMsgFlags.NO_REFRESH);
	                }
       			}
                
                // close watchlist request
                closeWlRequest(wlRequest);
                repoolWlRequest(wlRequest);
                break;
            }
        }

        _requestTimeoutList.remove(wlRequest);

        return ret;
    }

    private int removeUserRequestFromClosedStream(WlRequest wlRequest)
    {
        int ret = ReactorReturnCodes.SUCCESS;

        // remove from _userStreamIdListToRecover list
        _tempWlInteger.value(wlRequest.requestMsg().streamId());
        _userStreamIdListToRecover.remove(_tempWlInteger);

        // remove from _statusMsgDispatchList list
        _tempWlInteger.value(wlRequest.requestMsg().streamId());
        StatusMsg statusMsg = _statusMsgDispatchList.remove(_tempWlInteger);
        if (statusMsg != null)
        	_statusMsgPool.add(statusMsg);

        Iterator<Map.Entry<WlInteger, LinkedHashSet<WlRequest>>> I = _pendingRequestByIdTable.entrySet().iterator();
        while (I.hasNext())
        {
        	Map.Entry<WlInteger, LinkedHashSet<WlRequest>> entry = I.next();
        	LinkedHashSet<WlRequest> pendingRequests = entry.getValue();
        	pendingRequests.remove(wlRequest);

        	if (pendingRequests.isEmpty())
        	{
        		entry.getKey().returnToPool();
        		I.remove();
        		_pendingRequestListPool.add(pendingRequests);
        	}
        }
        
        // remove from _pendingRequestByNameTable
        Iterator<Map.Entry<String, LinkedHashSet<WlRequest>>> J = _pendingRequestByNameTable.entrySet().iterator();
        while (J.hasNext())
        {
	        Map.Entry<String, LinkedHashSet<WlRequest>> entry = J.next();
	        LinkedHashSet<WlRequest> pendingRequests = entry.getValue();
        	pendingRequests.remove(wlRequest);

        	if (pendingRequests.isEmpty())
        	{
        		J.remove();
        		_pendingRequestListPool.add(pendingRequests);
        	}
        }
        
        closeWlRequest(wlRequest);
        repoolWlRequest(wlRequest);

        return ret;        
    }

    /* Handles a post message. */
    int handlePost(WlRequest wlRequest, Msg msg, ReactorSubmitOptions submitOptions, ReactorErrorInfo errorInfo)
    {
        int ret = ReactorReturnCodes.SUCCESS;
        WlStream wlStream =  wlRequest.stream();
        if (_watchlist.numOutstandingPosts() < _watchlist.watchlistOptions().maxOutstandingPosts())
        {
            boolean resetServiceId = false;
            
            // validate post submit
            if ((ret = wlStream.validatePostSubmit((PostMsg)msg, errorInfo)) != ReactorReturnCodes.SUCCESS)
            {
                return ret;
            }
            
            // replace service id if message submitted with service name
            if (submitOptions.serviceName() != null)
            {
                if (!((PostMsg)msg).checkHasMsgKey())
                {
                    return _watchlist.reactor().populateErrorInfo(errorInfo,
                                                                  ReactorReturnCodes.INVALID_USAGE,
                                                                  "WlItemHandler.handlePost",
                                                                  "Post message submitted with service name but no message key.");
                    
                }
                
                if ((ret = _watchlist.changeServiceNameToID(((PostMsg)msg).msgKey(), submitOptions.serviceName(), errorInfo)) < ReactorReturnCodes.SUCCESS)
                {
                    return ret;
                }
                
                // set resetServiceId flag
                resetServiceId = true;
            }

            // send message
            // no need to replace stream id for post message here - that's done inside sendMsg()
            int userStreamId = msg.streamId();
            msg.streamId(wlStream._streamId);
            ret = wlStream.sendMsgOutOfLoop(msg, submitOptions, errorInfo);
            msg.streamId(userStreamId);

            // reset service id if checkAck() return false
            if( resetServiceId && (!((PostMsg) msg).checkAck() || ( ret < ReactorReturnCodes.SUCCESS)))
            {
                ((PostMsg) msg).msgKey().flags(((PostMsg) msg).msgKey().flags() & ~MsgKeyFlags.HAS_SERVICE_ID);
                ((PostMsg) msg).msgKey().serviceId(0);
                resetServiceId = false;
            }
            if (ret < ReactorReturnCodes.SUCCESS)
                return ret;
            else
            {
            	  if (((PostMsg)msg).checkAck())
                  {
                      // increment number of outstanding post messages
                      _watchlist.numOutstandingPosts(_watchlist.numOutstandingPosts() + 1);
                      
                      // update post tables
                      ret = wlStream.updatePostTables((PostMsg)msg, errorInfo);

                      // reset service id if necessary
                      if (resetServiceId)
                      {
                          ((PostMsg) msg).msgKey().flags(((PostMsg) msg).msgKey().flags() & ~MsgKeyFlags.HAS_SERVICE_ID);
                          ((PostMsg) msg).msgKey().serviceId(0);
                      }
                  }
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
        WlInteger wlInteger = ReactorFactory.createWlInteger();
        wlInteger.value(streamId);
        _statusMsgDispatchList.put(wlInteger, statusMsg);
        
        if (_statusMsgDispatchList.size() == 1)
        {
            // trigger dispatch only for first add to list
            _watchlist.reactor().sendWatchlistDispatchNowEvent(_watchlist.reactorChannel());
        }
    }

    /* Disassociate WlStream from its parent service (and item group in that service). */
    void removeWlStreamFromService(WlStream wlStream)
    {
        removeStreamFromItemGroup(wlStream);
        wlStream.wlService().streamList().remove(wlStream);
        wlStream.wlService(null);
    }

    /* Remove the WlStream from associated watchlist tables and repool it. */
    void closeWlStream(WlStream wlStream)
    {
        // Remove stream from aggregation table.
        if (wlStream.itemAggregationKey() != null)
        {
            _itemAggregationKeytoWlStreamTable.remove(wlStream.itemAggregationKey());
            wlStream.itemAggregationKey().returnToPool();
            wlStream.itemAggregationKey(null);
        }

    	
    	_pendingSendMsgList.remove(wlStream);
    	_streamList.remove(wlStream);

    	if (wlStream.wlService() != null)
            removeWlStreamFromService(wlStream);
        
        wlStream.close();
    }
    
	@Override
    public int readMsg(WlStream wlStream, DecodeIterator dIter, Msg msg, boolean wsbSendCloseRecover, ReactorErrorInfo errorInfo)
    {
        int ret = ReactorReturnCodes.SUCCESS;
        WlService wlService = wlStream.wlService();
        
        _currentFanoutStream = wlStream;
        _msgState = null;
        
        switch (msg.msgClass())
        {
            case MsgClasses.REFRESH:
            {
                RefreshMsg refreshMsg = (RefreshMsg)msg;

                _msgState = refreshMsg.state();
                readRefreshMsg(wlStream, refreshMsg, errorInfo);
                break;
            }

            case MsgClasses.STATUS:
            {
                StatusMsg statusMsg = (StatusMsg)msg;

                if (statusMsg.checkHasState())
                	_msgState = statusMsg.state();

                readStatusMsg(wlStream, statusMsg, errorInfo);

                break;
            }

            case MsgClasses.UPDATE:
                ret =  readUpdateMsg(wlStream, msg, errorInfo);
                break;
            case MsgClasses.GENERIC:
                ret =  readGenericMsg(wlStream, msg, errorInfo);
                break;
            case MsgClasses.ACK:
                ret =  readAckMsg(wlStream, msg, errorInfo);
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
        	// All requests were closed inside callback; it is now safe to repool the WlStream.
        	wlStream.returnToPool();
        }
        else if (_msgState != null)
        {
            // If _msgState is still set, stream is closed and may need to be recovered
            
            assert(_msgState.streamState() != StreamStates.OPEN);
            wlStream.userRequestList().addAll(wlStream.waitingRequestList());
            wlStream.waitingRequestList().clear();

            closeWlStream(wlStream);

            WlRequest usrRequest = null;
            for (usrRequest = wlStream.userRequestList().poll(); usrRequest != null; usrRequest = wlStream.userRequestList().poll())
            {
                msg.streamId(usrRequest.requestMsg().streamId());
                msg.domainType(usrRequest.requestMsg().domainType());

                if ( _watchlist.isRequestRecoverable(usrRequest, _msgState.streamState()) && !wsbSendCloseRecover)
                {
                    int origDataState = _msgState.dataState();

                    WlInteger wlInteger = ReactorFactory.createWlInteger();
                    wlInteger.value(usrRequest.requestMsg().streamId());
                    _userStreamIdListToRecover.add(wlInteger);
                    usrRequest.state(State.PENDING_REQUEST);
                    
                    _msgState.streamState(StreamStates.OPEN);
                    _msgState.dataState(DataStates.SUSPECT);

                    _submitOptions.serviceName(usrRequest.streamInfo().serviceName());
                    _submitOptions.requestMsgOptions().userSpecObj(usrRequest.streamInfo().userSpecObject());  		
                    addToPendingRequestTable(usrRequest, _submitOptions);

                    if ((ret = callbackUser("WlItemHandler.readMsg", msg, null, usrRequest, errorInfo)) < ReactorCallbackReturnCodes.SUCCESS)
                        break;

                    // Restore original state on message before next fanout.
                    _msgState.streamState(StreamStates.CLOSED_RECOVER);
                    _msgState.dataState(origDataState);
                }
                else
                {
                    closeWlRequest(usrRequest);

                    if ((ret = callbackUser("WlItemHandler.readMsg", msg, null, usrRequest, errorInfo)) < ReactorCallbackReturnCodes.SUCCESS)
                        break;
                    
                    repoolWlRequest(usrRequest);
                }

            }

            if (ret < ReactorCallbackReturnCodes.SUCCESS)
            {
                wlStream.returnToPool();
                _currentFanoutStream = null;
                return ret;
            }

            wlStream.returnToPool();

            if (wlService != null)
            	serviceAdded(wlService);
        }
    
        /* send next request in service's waiting request list */
        if (wlService != null && wlService.waitingRequestList().size() > 0 && msg.msgClass() != MsgClasses.REFRESH)
        {
            WlRequest waitingRequest = wlService.waitingRequestList().poll();
            _submitOptions.serviceName(waitingRequest.streamInfo().serviceName());
            _submitOptions.requestMsgOptions().userSpecObj(waitingRequest.streamInfo().userSpecObject());
            ret = handleRequest(waitingRequest, waitingRequest.requestMsg(), _submitOptions, true, _errorInfo);
        }

        _currentFanoutStream = null;
        return ret;
    }

    /* Reads a refresh message. */
    int readRefreshMsg(WlStream wlStream, RefreshMsg msg, ReactorErrorInfo errorInfo)
    {
        int ret = ReactorReturnCodes.SUCCESS;
        _snapshotViewClosed = false;
        int currentViewCount = 0;
                
        boolean isRefreshComplete = msg.checkRefreshComplete();
        
        boolean fanoutViewPendingRefresh =( (wlStream.refreshState() == WlStream.RefreshStates.REFRESH_VIEW_PENDING) && 
        									(wlStream.aggregateView() != null && wlStream.aggregateView().elemCount() != wlStream._requestsWithViewCount) );
        boolean solicitedRefresh = (wlStream.refreshState() == WlStream.RefreshStates.REFRESH_PENDING ||
        		wlStream.refreshState() == WlStream.RefreshStates.REFRESH_COMPLETE_PENDING); 

        WlService wlService = wlStream.wlService();
        
        // notify stream that response received if solicited
        if (msg.checkSolicited())
        {
            wlStream.responseReceived();
        }

        if (msg.domainType() == DomainTypes.SYMBOL_LIST)
        {
        	handleSymbolList(wlStream, msg, errorInfo);  
        }
        
        // set state from refresh message
        msg.state().copy(wlStream.state());

        // If message isn't open or non-streaming, let recovery code handle it.
        if (msg.state().streamState() != StreamStates.OPEN && msg.state().streamState() != StreamStates.NON_STREAMING)
            return ReactorReturnCodes.SUCCESS;

        int listSize = wlStream.userRequestList().size();
        
        // decrement number of outstanding requests on service when the request has not been removed by the user
        if (isRefreshComplete && (listSize != 0) )
        {
            wlService.numOutstandingRequests(wlService.numOutstandingRequests() - 1);   
        }
        
        int numRequestsProcessed = 0;
        // fanout refresh message to user requests associated with the stream
        for (int i = 0; i < wlStream.userRequestList().size(); i++)
        {            	
            if (numRequestsProcessed >= listSize) break;
            numRequestsProcessed++;
            WlRequest wlRequest = wlStream.userRequestList().get(i);
            
            wlRequest.handlePendingViewFanout(fanoutViewPendingRefresh);

            // only fanout if refresh is desired and refresh is unsolicited or to those whose state is awaiting refresh
            if (!wlRequest.requestMsg().checkNoRefresh() &&
                    (!msg.checkSolicited() ||
                     wlRequest.solicitedRefreshNeededForView(solicitedRefresh) ||
                     wlRequest.state() == WlRequest.State.PENDING_REFRESH ||
                     wlRequest.state() == WlRequest.State.PENDING_COMPLETE_REFRESH) ||
                     fanoutViewPendingRefresh)
            {
                // check refresh complete flag and change state of user request accordingly
                if (isRefreshComplete)
                {
                	wlStream.refreshState(WlStream.RefreshStates.REFRESH_NOT_REQUIRED);

                    // Remove item from existing group, if present.
                    removeStreamFromItemGroup(wlStream);
                                                                  
                    WlItemGroup wlItemGroup = wlService.itemGroupTableGet(msg.groupId());
                    // Add group Id as new itemGroup in the WlService's itemGroupTable
                    if (wlItemGroup == null)
                    {
                        Buffer groupId = CodecFactory.createBuffer();
                        groupId.data(ByteBuffer.allocate( msg.groupId().length()));
                        msg.groupId().copy(groupId);
                        wlItemGroup = ReactorFactory.createWlItemGroup();
                        wlItemGroup.groupId(groupId);
                        wlItemGroup.wlService(wlService);
                        wlService.itemGroupTablePut(groupId, wlItemGroup);
                    }

                    addStreamToItemGroup(wlItemGroup, wlStream);


                    if (wlRequest.requestMsg().checkStreaming() &&
                            msg.state().streamState() != StreamStates.NON_STREAMING)
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
                        if( wlRequest.requestMsg().checkStreaming() && wlRequest.requestMsg().checkPause())
                        {
                            wlRequest.stream().numPausedRequestsCount(wlRequest.stream().numPausedRequestsCount() -1);
                        }
                    }
                }
                else if (wlRequest.state() == WlRequest.State.PENDING_REFRESH) // multi-part refresh
                {
                	wlRequest.state(WlRequest.State.PENDING_COMPLETE_REFRESH);
                	
                    // set multi-part refresh pending flag
                    wlStream.refreshState(WlStream.RefreshStates.REFRESH_COMPLETE_PENDING);

                    // start another request timer for each part of multi-part refresh
                    wlStream.startRequestTimer(errorInfo);
                }

                // update stream id in message to that of user request
                msg.streamId(wlRequest.requestMsg().streamId());

                // For snapshot requests, change OPEN state to NON-STREAMING.
                int tmpStreamState = msg.state().streamState();
                if (!wlRequest.requestMsg().checkStreaming() && tmpStreamState == StreamStates.OPEN)
                    msg.state().streamState(StreamStates.NON_STREAMING);

                // if snapshot request or NON_STREAMING and the refresh complete is received, close watchlist request and stream if necessary
                if ( (!wlRequest.requestMsg().checkStreaming() ||
                        msg.state().streamState() == StreamStates.NON_STREAMING) && isRefreshComplete)
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
                    closeWlRequest(wlRequest);
                    
                    // if no more requests in stream, close stream
                    boolean allRequestsClosed = (wlStream.userRequestList().size() == 0 &&   
                        wlStream.waitingRequestList().size() == 0);
                    
                    wlStream.refreshState(WlStream.RefreshStates.REFRESH_NOT_REQUIRED);
                    
                    // If there are no requests pending refresh completion, the stream can be closed at this point.
                    if (allRequestsClosed)
                        closeWlStream(wlStream);
                    
                    ret = callbackUser("WlItemHandler.readRefreshMsg", msg, null, wlRequest, errorInfo);
                    
                    wlRequest.returnToPool();

                    if (ret < ReactorCallbackReturnCodes.SUCCESS)
                    {
                        // break out of loop for error
                        break;
                    }

                    if (allRequestsClosed)
                    {
                        // Stream and all its requests were closed. Unset _currentFanoutStream so that the WlStream is repooled.
                        _currentFanoutStream = null;
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
                
                msg.state().streamState(tmpStreamState);
            }
        }

        if (_currentFanoutStream != null)
        {
            /* if no longer waiting for snapshot, send requests in waiting request list */
            if (wlStream.waitingRequestList().size() > 0 &&
                    (wlStream.refreshState() != WlStream.RefreshStates.REFRESH_COMPLETE_PENDING))
            {
                WlRequest waitingRequest = null;

                while( (wlStream.refreshState() < WlStream.RefreshStates.REFRESH_PENDING) && (waitingRequest = wlStream.waitingRequestList().poll()) != null) 
                {
                    _submitOptions.serviceName(waitingRequest.streamInfo().serviceName());
                    _submitOptions.requestMsgOptions().userSpecObj(waitingRequest.streamInfo().userSpecObject());

                    if (waitingRequest._reissue_hasViewChange)
                    	ret = handleReissue(waitingRequest, waitingRequest.requestMsg(), _submitOptions, errorInfo); 
                    else
                    	ret = handleRequest(waitingRequest, waitingRequest.requestMsg(), _submitOptions, true, errorInfo);                    
           			if (ret < ReactorReturnCodes.SUCCESS) return ret;
                }
            }

            if (_snapshotViewClosed)
            {
                _snapshotViewClosed = false;

                if ( currentViewCount > 0 && wlStream._requestsWithViewCount == currentViewCount && wlStream.userRequestList().size()  > 0 && (_wlViewHandler.resorted() ||
                            !_wlViewHandler.commitedViewsContainsAggregateView(wlStream._aggregateView)) && wlStream._requestsWithViewCount == wlStream._userRequestList.size())
                { 
                    wlStream.requestMsg().flags(wlStream.requestMsg().flags() | RequestMsgFlags.NO_REFRESH);            		 
                    wlStream.sendMsgOutOfLoop(wlStream.requestMsg(), _submitOptions, errorInfo);
                    wlStream.requestMsg().flags(wlStream.requestMsg().flags() & ~RequestMsgFlags.NO_REFRESH);
                    if (_wlViewHandler.resorted()) _wlViewHandler.resorted(false);
                }
            }           
        }

        /* send next request in service's waiting request list */
        if (wlService != null && wlService.waitingRequestList().size() > 0 && isRefreshComplete)
        {
            WlRequest waitingRequest = wlService.waitingRequestList().poll();
            _submitOptions.serviceName(waitingRequest.streamInfo().serviceName());
            _submitOptions.requestMsgOptions().userSpecObj(waitingRequest.streamInfo().userSpecObject());

            ret = handleRequest(waitingRequest, waitingRequest.requestMsg(), _submitOptions, true, errorInfo);
            if (ret < ReactorReturnCodes.SUCCESS) return ret;
        }
        
        // No recovery needed, so set _msgState to null.
        _msgState = null;
        
        return ret;
    }

    /* Reads a update message. */
    int readUpdateMsg(WlStream wlStream, Msg msg, ReactorErrorInfo errorInfo)
    {
        int ret = ReactorReturnCodes.SUCCESS;
        if (msg.domainType() == DomainTypes.SYMBOL_LIST)
        {
        	handleSymbolList(wlStream, msg, errorInfo); 
        }
                
        // fanout update message to user requests associated with the stream
        for (int i = 0; i < wlStream.userRequestList().size(); i++)
        {
            WlRequest wlRequest = wlStream.userRequestList().get(i);
                        
            // only fanout to those whose state is OPEN or REFRESH_COMPLETE_PENDING
            if ( wlRequest.state() == WlRequest.State.PENDING_COMPLETE_REFRESH ||
            	 wlRequest.state() == WlRequest.State.OPEN)
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
    int readStatusMsg(WlStream wlStream, Msg msg, ReactorErrorInfo errorInfo)
    {
    	if ( wlStream.wlService() != null && wlStream.requestPending())
    		wlStream.wlService().numOutstandingRequests(wlStream.wlService().numOutstandingRequests() - 1); 

    	// notify stream that response received
        wlStream.responseReceived();

        // set state from status message
        if (((StatusMsg)msg).checkHasState()) 
        	((StatusMsg)msg).state().copy(wlStream.state());

        // Fanout if open; otherwise, recovery code will do it.
        if (!((StatusMsg)msg).checkHasState() || ((StatusMsg)msg).state().streamState() == StreamStates.OPEN)
        {
            // fanout status message to user requests associated with the stream
            for (int i = 0; i < wlStream.userRequestList().size(); i++)
            {
                WlRequest wlRequest = wlStream.userRequestList().get(i);

                // update stream id in message to that of user request
                msg.streamId(wlRequest.requestMsg().streamId());

                // callback user
                _tempWlInteger.value(msg.streamId());

                if (callbackUser("WlItemHandler.readStatusMsg", msg, null, _watchlist.streamIdtoWlRequestTable().get(_tempWlInteger), errorInfo) < ReactorCallbackReturnCodes.SUCCESS)
                {
                    // break out of loop for error
                    break;
                }
            }

            // No recovery needed, so set _msgState to null.
            _msgState = null;
        }

        return 0;
    }

    /* Reads a generic message. */
    int readGenericMsg(WlStream wlStream, Msg msg, ReactorErrorInfo errorInfo)
    {
        int ret = ReactorReturnCodes.SUCCESS;
        
        // fanout generic message to user requests associated with the stream
        for (int i = 0; i < wlStream.userRequestList().size(); i++)
        {
            WlRequest wlRequest = wlStream.userRequestList().get(i);
            
            // only fanout to those whose state is PENDING_REFRESH or OPEN
            if (wlRequest.state() == WlRequest.State.PENDING_REFRESH ||
            	wlRequest.state() == WlRequest.State.PENDING_COMPLETE_REFRESH ||
                wlRequest.state() == WlRequest.State.OPEN)
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
    int readAckMsg(WlStream wlStream, Msg msg, ReactorErrorInfo errorInfo)
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
        
        // send any queued status messages to the user
        StatusMsg statusMsg = null;
        Iterator<Map.Entry<WlInteger, StatusMsg>> statusMsgIter = _statusMsgDispatchList.entrySet().iterator();
        while (statusMsgIter.hasNext())
        {
            Map.Entry<WlInteger, StatusMsg> entry = statusMsgIter.next(); // Get an entry and use it to access both key and value
            statusMsg = entry.getValue();
            _tempWlInteger.value(statusMsg.streamId());
            WlRequest wlRequest = _watchlist.streamIdtoWlRequestTable().get(_tempWlInteger);
            boolean requestClosed = (statusMsg.checkHasState() && statusMsg.state().streamState() != StreamStates.OPEN);
            
            if (requestClosed)
            	closeWlRequest(wlRequest);
            
            ret = callbackUser("WlItemHandler.dispatch", statusMsg, null, wlRequest, errorInfo);
            
            if (requestClosed)
            	repoolWlRequest(wlRequest);
            
            // return StatusMsg to pool
            _statusMsgPool.add(statusMsg);

            // return WlInteger to pool
            entry.getKey().returnToPool();
            
            statusMsgIter.remove();
            
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
        int loopCount = _pendingSendMsgList.size();
        while((wlStream = _pendingSendMsgList.poll()) != null)
        {
            if ((ret = wlStream.sendMsgOnLoop(wlStream.requestMsg(), _submitOptions, _errorInfo)) < ReactorReturnCodes.SUCCESS)
            {
            	/* No buffers means that the request was re-queued, so we can end the loop here */
            	if(ret == ReactorReturnCodes.NO_BUFFERS)
            	{
            		return ReactorReturnCodes.SUCCESS;
            	}
            	else
            	{
            		return ret;
            	}
            }
            else
            {
            	loopCount--;
            	if(loopCount == 0)
            	{
            		return ReactorReturnCodes.SUCCESS;
            	}
            }
        }
        
        return ret;
    }
    
    /* Handles login stream open event. */
    int loginStreamOpen(ReactorErrorInfo errorInfo)
    {
        return ReactorReturnCodes.SUCCESS;
    }

    /* Handles login stream closed event. 
     * If state is null, presumes it was closed by a CloseMsg. */
    int loginStreamClosed(com.refinitiv.eta.codec.State state)
    {
        _statusMsg.clear();
        _statusMsg.applyHasState();
        _statusMsg.msgClass(MsgClasses.STATUS);
        
        if (state != null)
        {
            _statusMsg.state().streamState(state.streamState());
            _statusMsg.state().dataState(state.dataState());
        }
        else
        {
            // Closed via CloseMsg.
            _statusMsg.state().streamState(StreamStates.CLOSED);
            _statusMsg.state().dataState(DataStates.SUSPECT);
        }
        
        
        _statusMsg.state().text().data("Login stream was closed.");
        fanoutToAllStreams(_statusMsg); 

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
        fanoutToAllStreams(msg);
                   
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
    	     	   	   
   	   for (Map.Entry<String, LinkedHashSet<WlRequest>> entry : _pendingRequestByNameTable.entrySet())
   	   {
   		LinkedHashSet<WlRequest> pendingRequestList  = entry.getValue();
   	   	   for (WlRequest usrRequest : pendingRequestList)
       	   { 
   	   		   usrRequest.requestMsg().applyPause();
       	   }
   	   }
   
   	   for (Map.Entry<WlInteger, LinkedHashSet<WlRequest>> entry : _pendingRequestByIdTable.entrySet())
   	   {
   		LinkedHashSet<WlRequest> pendingRequestList  = entry.getValue();
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
        	     	   	   
    	for (Map.Entry<String, LinkedHashSet<WlRequest>> entry : _pendingRequestByNameTable.entrySet())
    	{
    		LinkedHashSet<WlRequest> pendingRequestList  = entry.getValue();
    		for (WlRequest usrRequest : pendingRequestList)
    		{ 
    			usrRequest.requestMsg().flags(usrRequest.requestMsg().flags() & ~RequestMsgFlags.PAUSE);
           	}
    	}
       
    	for (Map.Entry<WlInteger, LinkedHashSet<WlRequest>> entry : _pendingRequestByIdTable.entrySet())
    	{
    		LinkedHashSet<WlRequest> pendingRequestList  = entry.getValue();
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
		_streamList.clear();
    }
    
    /* Handles service added event. */
    int serviceAdded(WlService wlService)
    {
        int ret = ReactorReturnCodes.SUCCESS;
        LinkedHashSet<WlRequest> pendingRequestList = null;
        
        // handle any pending requests
        // retrieve matching requests based on service id or service name
        _tempWlInteger.value(wlService.rdmService().serviceId());
        pendingRequestList = _pendingRequestByIdTable.remove(_tempWlInteger);
        if (pendingRequestList == null)
        {
            pendingRequestList = _pendingRequestByNameTable.remove(wlService.rdmService().info().serviceName().toString());
        }
        
        // handle request
        if (pendingRequestList != null)
        {
            WlRequest wlRequest = null;
            Iterator<WlRequest> wlRequestIter = pendingRequestList.iterator();
            while(wlRequestIter.hasNext())
            {
            	wlRequest = wlRequestIter.next();
                _submitOptions.serviceName(wlRequest.streamInfo().serviceName());
                _submitOptions.requestMsgOptions().userSpecObj(wlRequest.streamInfo().userSpecObject());
                
          		if (wlRequest.requestMsg().checkNoRefresh())
          			wlRequest.requestMsg().flags(wlRequest.requestMsg().flags() & ~RequestMsgFlags.NO_REFRESH);
          		
                if ((ret = handleRequest(wlRequest, wlRequest.requestMsg(), _submitOptions, false, _errorInfo)) < ReactorReturnCodes.SUCCESS)
                {
                    wlRequestIter.remove();
                    return ret;
                }
                
                wlRequestIter.remove();
            }
            _pendingRequestListPool.add(pendingRequestList);
        }
        
        // call sendMsg on all streams in pending stream send list
        WlStream wlStream = null;
        int loopCount = _pendingSendMsgList.size();
        while((wlStream = _pendingSendMsgList.poll()) != null)
        {
        	if ((ret = wlStream.sendMsgOnLoop(wlStream.requestMsg(), _submitOptions, _errorInfo)) < ReactorReturnCodes.SUCCESS)
            {
            	/* No buffers means that the request was re-queued, so we can end the loop here */
            	if(ret == ReactorReturnCodes.NO_BUFFERS)
            	{
            		return ReactorReturnCodes.SUCCESS;
            	}
            	else
            	{
            		return ret;
            	}
            }
            else
            {
            	loopCount--;
            	if(loopCount == 0)
            	{
            		return ReactorReturnCodes.SUCCESS;
            	}
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
    			
        		ret = readMsg(wlService.streamList().get(streamCount), null, statusMsg, false, _errorInfo);
	            
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
						
						ret = readMsg(wlService.itemGroupTableGet(serviceGroup.group()).openStreamList().get(i), null, statusMsg, false, _errorInfo);
    	            
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
    int serviceDeleted(WlService wlService, boolean channelIsDown)
    {
        String stateText;
        boolean forceClose = false;
        boolean sendChannelDownText = channelIsDown;
        
        int statusFlags = 0;

        if(_watchlist.reactor().reactorHandlesWarmStandby(_watchlist.reactorChannel()))
        {
        	ReactorWarmStandbyHandler wsbHandler = _watchlist.reactorChannel().warmStandByHandlerImpl;
        	
        	if(wsbHandler.currentWarmStandbyGroupImpl().warmStandbyMode() == ReactorWarmStandbyMode.LOGIN_BASED)
        	{
        		/* Check to see if this service is also in the next active, if it exists */
        		if(wsbHandler.nextActiveReactorChannel() != null && wsbHandler.nextActiveReactorChannel().channel() != null 
        				&& wsbHandler.nextActiveReactorChannel().channel().state() == ChannelState.ACTIVE)
        		{
    				statusFlags = WlStreamStatusFlags.SEND_STATUS;
    				sendChannelDownText = false;

        			if(wsbHandler.nextActiveReactorChannel().watchlist()._directoryHandler._serviceCache._servicesByIdTable.get(wlService._tableKey) == null)
        			{
                		ReactorWSBService wsbService = _watchlist.reactorChannel().warmStandByHandlerImpl.currentWarmStandbyGroupImpl()._perServiceById.get(wlService.tableKey());
                		
                		if(wsbService == null || (wsbService.channels.size() == 0))
                		{
                			// Don't have the service in any channels, force close the item 
                			forceClose = true;
                		}
        			}
        		}		
        	}
        	else
        	{
        		boolean hasService = false;
        		ReactorWSBService wsbService = _watchlist.reactorChannel().warmStandByHandlerImpl.currentWarmStandbyGroupImpl()._perServiceById.get(wlService.tableKey());
        		if(wsbService != null)
        		{
	        		boolean containsChannel = wsbService.channels.contains(_watchlist.reactorChannel());
	        		// We haven't removed the channel from the list yet, so we need to check if there are additional channels
	        		if(wsbService != null && ((!containsChannel && wsbService.channels.size() > 0) || (containsChannel && wsbService.channels.size() > 1)))
	        		{
	        			hasService = true;
	        			sendChannelDownText = false;
	        		}
        		}
        		
        		// Checks whether there is a channel down for others channel.
        		ReactorChannel processChannel;
        		boolean hasChannelDown = false;
        		for (int i = 0; i < wsbHandler.channelList().size(); i++)
        		{
        			processChannel = wsbHandler.channelList().get(i);

        			if (_watchlist.reactorChannel() != null && processChannel != _watchlist.reactorChannel())
        			{
        				if(processChannel.state() != ReactorChannel.State.READY || processChannel.state() != ReactorChannel.State.UP)
        				{
        					hasChannelDown = true;
        					break;
        				}
        			}
        		}

        		if(!hasChannelDown && hasService == false)
        		{
        			forceClose = true;
        			sendChannelDownText = false;
    				statusFlags = WlStreamStatusFlags.SEND_STATUS;
        		}
        		
        	}
        }
        
        // For item recovery there is no functional difference between losing
        // all services and the channel being down. The text is just changed
        // if the cause was actually the channel going down.
        if (sendChannelDownText)
            stateText = "channel down.";
        else
            stateText = "Service for this item was lost.";

        _statusMsg.clear();
        _statusMsg.msgClass(MsgClasses.STATUS);
        _statusMsg.applyHasState();
        _statusMsg.state().streamState(StreamStates.CLOSED_RECOVER);
        _statusMsg.state().dataState(DataStates.SUSPECT);
        _statusMsg.msgClass(MsgClasses.STATUS);
        _statusMsg.state().text().data(stateText);
        
        WlStream wlStream;
        while ((wlStream = wlService.streamList().peek()) != null)
        {
            int ret;
            removeWlStreamFromService(wlStream);
            _statusMsg.domainType(wlStream.domainType());
            if ((ret = readMsg(wlStream, null, _statusMsg, forceClose, _errorInfo)) != ReactorReturnCodes.SUCCESS)
                return ret;
        }  
        
        for (WlRequest wlRequest = wlService.waitingRequestList().poll(); wlRequest != null; wlRequest = wlService.waitingRequestList().poll())
        {
            int ret;
            boolean isRecoverable = _watchlist.isRequestRecoverable(wlRequest, StreamStates.CLOSED_RECOVER) && !forceClose;

            _statusMsg.streamId(wlRequest.requestMsg().streamId());
            _statusMsg.domainType(wlRequest.requestMsg().domainType());
            
            wlRequest.statusFlags(statusFlags);

            if (isRecoverable)
            {
                _statusMsg.state().streamState(StreamStates.OPEN);
                _submitOptions.serviceName(wlRequest.streamInfo().serviceName());
                _submitOptions.requestMsgOptions().userSpecObj(wlRequest.streamInfo().userSpecObject());
                addToPendingRequestTable(wlRequest, _submitOptions);
            }
            else
            {
            	closeWlRequest(wlRequest);
                _statusMsg.state().streamState(StreamStates.CLOSED_RECOVER);
            }

            if ((ret = callbackUser("WlItemHandler.serviceDeleted", _statusMsg, null, wlRequest, _errorInfo)) < ReactorReturnCodes.SUCCESS)
                return ret;
            
            if (!isRecoverable)
            	repoolWlRequest(wlRequest);
        }

        return ReactorReturnCodes.SUCCESS;        	
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
                                                                       wlRequest,
                                                                       errorInfo);
        }
        else // dictionary domain
        {

            /* Decode to a DictionaryMsg. */

            _rdmDictionaryMsg.clear();
            switch(msg.msgClass())
            {
                case MsgClasses.REFRESH:
                    _rdmDictionaryMsg.rdmMsgType(DictionaryMsgType.REFRESH);
                    break;
                case MsgClasses.STATUS:
                    _rdmDictionaryMsg.rdmMsgType(DictionaryMsgType.STATUS);
                    break;
                default:
                    return _watchlist.reactor().populateErrorInfo(errorInfo,
                            ReactorReturnCodes.FAILURE, "ItemHandler.callbackUser",
                            "Unknown message class for dictionary: < " + MsgClasses.toString(msg.msgClass()) + ">");

            }

            _dIter.clear();
            if (msg.encodedDataBody().data() != null)
                _dIter.setBufferAndRWFVersion(msg.encodedDataBody(), _watchlist.reactorChannel().majorVersion(),
                        _watchlist.reactorChannel().minorVersion());

            if ((ret = _rdmDictionaryMsg.decode(_dIter, msg)) < CodecReturnCodes.SUCCESS)
            {
                return _watchlist.reactor().populateErrorInfo(errorInfo,
                        ret, "ItemHandler.callbackUser",
                        "DictionaryMsg.decode() failed: < " + CodecReturnCodes.toString(ret) + ">");
            }

            ret = _watchlist.reactor().sendAndHandleDictionaryMsgCallback(location,
                                                                          _watchlist.reactorChannel(),
                                                                          null,
                                                                          msg,
                                                                          _rdmDictionaryMsg,
                                                                          wlRequest,
                                                                          errorInfo);

            if (ret == ReactorCallbackReturnCodes.RAISE)
            {
                ret = _watchlist.reactor().sendAndHandleDefaultMsgCallback(location,
                                                                           _watchlist.reactorChannel(),
                                                                           null,
                                                                           msg,
                                                                           wlRequest,
                                                                           errorInfo);
            }
        }

        /* If the watchlist added the MsgKey, remove it in case subsqeuent requests on this 
           stream did not ask for MsgKeys in responses. */
        msg.flags(msg.flags() & ~msgFlagsToReset);

        return ret;
    }
    
    @Override
    public void addPendingRequest(WlStream wlStream)
    {
    	//if retVal is no buffer when calling stream.sentMsg(), will add the unsent request into _pendingSendMsgList
    	if (wlStream != null)
    	{
    		if (!_pendingSendMsgList.contains(wlStream))
    			_pendingSendMsgList.addFirst(wlStream);
    	}
    	//if the msg passed to stream.sentMsg() call is a request msg which has _pendingViewChange as true and 
    	//has _pendingViewRefresh as true, eta should not send this request out, 
    	//eta will put it into the waitingRequestList for sending it out after receiving refresh.
    	else
    		_hasPendingViewRequest = true;
    }
    
    void fanoutToAllStreams(Msg msg)
    {
		Iterator<WlStream> streamListIterator = _streamList.iterator();
		while (streamListIterator.hasNext())
		{
			WlStream wlStream = streamListIterator.next();
			streamListIterator.remove();

			readMsg(wlStream, null, msg, false, _errorInfo);
		}
    }    
    
    @Override
    public int requestTimeout(WlStream wlStream, ReactorErrorInfo errorInfo)
    {
        closeWlStream(wlStream);

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

        wlStream.returnToPool();
        
        // trigger dispatch
        _watchlist.reactor().sendWatchlistDispatchNowEvent(_watchlist.reactorChannel());
        
        return ReactorReturnCodes.SUCCESS;
    }

    /* Remove a WlStream from its item group, if any. */
    void removeStreamFromItemGroup(WlStream wlStream)
    {
        // Remove stream from its item group.
        WlItemGroup wlItemGroup = wlStream.itemGroup();
        if (wlItemGroup != null)
        {
            wlItemGroup.openStreamList().remove(wlStream);
            _tempWlInteger.value(wlStream.streamId());
            wlItemGroup.streamIdToItemGroupTable().remove(_tempWlInteger);
            wlStream.groupTableKey().returnToPool();
            wlStream.groupTableKey(null);
            // If no streams left in group's stream list, remove item group from table
            if (wlItemGroup.openStreamList().isEmpty())
                wlStream.wlService().itemGroupTableRemove(wlItemGroup.groupId());
            wlStream.itemGroup(null);
        }
    }

    /* Add a WlStream to an item group. */
    void addStreamToItemGroup(WlItemGroup wlItemGroup, WlStream wlStream)
    {
        _tempWlInteger.value(wlStream.streamId());
        if (!wlItemGroup.streamIdToItemGroupTable().containsKey(_tempWlInteger))
        {
            wlStream.itemGroup(wlItemGroup);
            wlItemGroup.openStreamList().add(wlStream);

            WlInteger wlInteger = ReactorFactory.createWlInteger();
            wlInteger.value(wlStream.streamId());
            wlStream.groupTableKey(wlInteger);
            wlItemGroup.streamIdToItemGroupTable().put(wlInteger, wlStream);
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
        _hasPendingViewRequest = false;
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

	private int handleSymbolList(WlStream wlStream, Msg msg, ReactorErrorInfo errorInfo) 
	{
		for (int i = 0; i < wlStream.userRequestList().size(); i++)
		{
			WlRequest wlRequest = wlStream.userRequestList().get(i);
			if (!wlRequest.hasBehaviour() || wlRequest.symbolListFlags() == 0 )
				continue;
			int ret = ReactorReturnCodes.SUCCESS;
			int serviceId = 0;
			if (wlRequest.requestMsg().msgKey().checkHasServiceId())
			{
				// retrieve service id from request
				serviceId = wlRequest.requestMsg().msgKey().serviceId();
			}
			else if (wlRequest.streamInfo().serviceName() != null)
			{
				// lookup service id by service name
				serviceId = _watchlist.directoryHandler().service(wlRequest.streamInfo().serviceName()).rdmService().serviceId();
			}
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

			_dIter.clear();
			_dIter.setBufferAndRWFVersion(msg.encodedDataBody(), _watchlist.reactorChannel().majorVersion(), _watchlist
					.reactorChannel().minorVersion());
			_map.clear();
			_mapEntry.clear();
			_mapKey.clear();

			switch (msg.msgClass()) 
			{
			case MsgClasses.UPDATE:
			case MsgClasses.REFRESH:
				ret = _map.decode(_dIter);
				if (ret != CodecReturnCodes.SUCCESS) 
				{
					_watchlist.reactor().populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
							"ItemHandler.handleSymbolList", "DecodeMap() failed: < "
									+ CodecReturnCodes.toString(ret) + ">");
					return ret;
				}
				while ((ret = _mapEntry.decode(_dIter, _mapKey)) != CodecReturnCodes.END_OF_CONTAINER) 
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
						    repoolWlRequest(newWlRequest);
						}
						break;
					default:
						break;
					}// switch mapEntry
				}// while mapEntry
				break;
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
						ReactorReturnCodes.FAILURE, "ItemHandler.extractViewFromMsg",
						"ElementEntry.decode() failed: < " + CodecReturnCodes.toString(ret) + ">");
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
										if (_fieldId.toLong() < Short.MIN_VALUE || _fieldId.toLong() > Short.MAX_VALUE)
										{
											_watchlist.reactor().populateErrorInfo(errorInfo,
													ReactorReturnCodes.FAILURE, "ItemHandler.extractViewFromMsg",
													"Field id in view request is outside the valid ID range <" + _fieldId + ">");
												return CodecReturnCodes.FAILURE;												
										}	
										wlRequest._viewFieldIdList.add((int) _fieldId.toLong());
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
											if (_fieldId.toLong() < Short.MIN_VALUE || _fieldId.toLong() > Short.MAX_VALUE)
											{
												_watchlist.reactor().populateErrorInfo(errorInfo,
														ReactorReturnCodes.FAILURE, "ItemHandler.extractViewFromMsg",
														"Field id in view request is outside the valid ID range <" + _fieldId + ">");
													return CodecReturnCodes.FAILURE;												
											}		
											wlRequest._viewFieldIdList.add((int)_fieldId.toLong());
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
	
	private int handleViews(WlRequest wlRequest, ReactorErrorInfo errorInfo)
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
	
    /* Returns a WlRequest and its members to appropriate object pools. */
	public void repoolWlRequest(WlRequest wlRequest)
	{
		if(wlRequest.view() != null)
        {
            switch(wlRequest.view().viewType())
            {		
                case ViewTypes.FIELD_ID_LIST:
                    _wlViewHandler._viewFieldIdListPool.add(wlRequest._viewFieldIdList);
                    wlRequest._viewFieldIdList = null;
                    break;
                case ViewTypes.ELEMENT_NAME_LIST:				
                    _wlViewHandler._viewElementNameListPool.add(wlRequest._viewElementNameList);		
                    wlRequest._viewElementNameList = null;
                    break;
                default:
                    break;
            }
            
            _wlViewHandler.destroyView(wlRequest._view);
        }

        wlRequest.returnToPool();
	}	
	
    void closeWlRequest(WlRequest wlRequest)
    {
        if (wlRequest.providerDriven())
        {
            _symbolListRequestKey.clear();
            _symbolListRequestKey.msgKey(wlRequest.requestMsg().msgKey());
            _symbolListRequestKey.msgKey().serviceId(wlRequest.requestMsg().msgKey().serviceId());
            _symbolListRequestKey.domainType(wlRequest.requestMsg().domainType());
            _symbolListRequestKey.qos(wlRequest.requestMsg().qos());
            _providerRequestTable.remove(_symbolListRequestKey);
        }    	

        _watchlist.closeWlRequest(wlRequest);
    }
	
} 

