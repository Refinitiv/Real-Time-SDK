/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

import java.nio.ByteBuffer;
import java.util.HashMap;
import java.util.LinkedList;

import com.refinitiv.eta.codec.AckMsg;
import com.refinitiv.eta.codec.AckMsgFlags;
import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CloseMsg;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.CopyMsgFlags;
import com.refinitiv.eta.codec.DataStates;
import com.refinitiv.eta.codec.DataTypes;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.Msg;
import com.refinitiv.eta.codec.MsgClasses;
import com.refinitiv.eta.codec.NakCodes;
import com.refinitiv.eta.codec.PostMsg;
import com.refinitiv.eta.codec.RequestMsg;
import com.refinitiv.eta.codec.RequestMsgFlags;
import com.refinitiv.eta.codec.State;
import com.refinitiv.eta.codec.StreamStates;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.transport.ChannelState;
import com.refinitiv.eta.transport.IoctlCodes;
import com.refinitiv.eta.transport.TransportBuffer;
import com.refinitiv.eta.transport.TransportReturnCodes;
import com.refinitiv.eta.valueadd.common.VaNode;

/* Watchlist stream that handles basic stream management. */
class WlStream extends VaNode
{
    /* The state of the watch-list stream. */
	class RefreshStates
    {		
		// No refresh needed.
		static final int REFRESH_NOT_REQUIRED = 0;
		
		// Currently waiting for a refresh.
		static final int REFRESH_PENDING = 1;
		
		// Currently waiting for a view refresh
		static final int REFRESH_VIEW_PENDING = 2; 
		
		// Received partial refresh, need the rest
		static final int REFRESH_COMPLETE_PENDING = 3;
    }
	
    Watchlist _watchlist;
    WlHandler _handler; // handler associated with stream
    ReactorChannel _reactorChannel;
    Reactor _reactor;

    int _streamId;
    int _domainType;
    boolean _requestPending;
    boolean _channelUp;
    
    // item aggregation key associated with the stream
    WlItemAggregationKey _itemAggregationKey;
    
    // service associated with the stream
    WlService _wlService;
    
    // group id associated with the stream
    WlItemGroup _itemGroup;
    
    long _requestExpireTime;
    
    RequestMsg _requestMsg; // current request message associated with stream
                            // this is the request exchanged with the ADS or Provider
    
    // The refresh state defined in RefreshStates
    int _refreshState;
    
    /* waiting request list to handle cases where request could not be submitted with
     * directory stream up but there was a pending multi-part refresh or snapshot in progress */
    LinkedList<WlRequest> _waitingRequestList = new LinkedList<WlRequest>();
    
    // list of user requests associated with this stream (used for fanout)
    LinkedList<WlRequest> _userRequestList = new LinkedList<WlRequest>();

    // unsent message queue
    // pool of messages
    LinkedList<Msg> _msgPool = new LinkedList<Msg>();
    
    State _state = CodecFactory.createState();
    EncodeIterator _eIter = CodecFactory.createEncodeIterator();
    ReactorChannelInfo _reactorChannelInfo = ReactorFactory.createReactorChannelInfo();
    ReactorSubmitOptions _submitOptions = ReactorFactory.createReactorSubmitOptions();
    
    // outer table is indexed by post id and inner table is indexed by sequence number
    HashMap<Long,HashMap<Long,PostMsg>> _postIdToMsgTable = new HashMap<Long,HashMap<Long,PostMsg>>();
    // pool of sequence number to post message hash tables
    LinkedList<HashMap<Long,PostMsg>> _postMsgHashMapPool = new LinkedList<HashMap<Long,PostMsg>>();
    // pool of post messages
    LinkedList<PostMsg> _postMsgPool = new LinkedList<PostMsg>();
    
    // list to track post ACK timeouts
    LinkedList<WlPostTimeoutInfo> _postTimeoutInfoList = new LinkedList<WlPostTimeoutInfo>();

    AckMsg _ackMsg;
    CloseMsg _closeMsg;
    
    ReactorChannelInfo _reactorChnlInfo = ReactorFactory.createReactorChannelInfo();
    
    WlInteger _tableKey, _groupTableKey;
    
    int _requestsPausedCount;
    boolean _paused;
    
    WlView _aggregateView;
    int _requestsWithViewCount;
    boolean _pendingViewChange;  
    boolean _viewSubsetContained;
    Buffer _viewBuffer = CodecFactory.createBuffer();
    ByteBuffer _viewByteBuffer = ByteBuffer.allocateDirect(2048);
        
    /* This is used to keep track whether this object has been added the watchlist's timeout list
     * in oder to remove it from the list before returning back to its pool.
     */
    boolean _addedToTimeOutList;

    WlStream()
    {
        _ackMsg = (AckMsg)CodecFactory.createMsg();
        _ackMsg.msgClass(MsgClasses.ACK);
        _closeMsg = (CloseMsg)CodecFactory.createMsg();
        _closeMsg.msgClass(MsgClasses.CLOSE);
        _addedToTimeOutList = false;
    }
    
    /* Returns the state of the watchlist stream. */
    State state()
    {
        return _state;
    }

    /* Handler associated with stream. */
    WlHandler handler()
    {
        return _handler;
    }
    
    /* Set the handler associated with stream. */
    void handler(WlHandler handler)
    {
        _handler = handler;
    }
    
    /* Set the watchlist associated with stream. */
    void watchlist(Watchlist watchlist)
    {
        _watchlist = watchlist;
        _reactorChannel = _watchlist.reactorChannel();
        _reactor = _watchlist.reactor();
    }
    
    /* Returns the stream id of the stream. */
    int streamId()
    {
        return _streamId;
    }
    
    /* Set the id of the stream. */
    void streamId(int streamId)
    {
        _streamId = streamId;
    }
    
    /* Returns the domain type of the stream. */
    int domainType()
    {
        return _domainType;
    }
    
    /* Set the domain type of the stream. */
    void domainType(int domainType)
    {
        _domainType = domainType;
    }
    
    /* Returns the request message of the stream. */
    RequestMsg requestMsg()
    {
        return _requestMsg;
    }
    
    /* Sets the request message of the stream. */
    int requestMsg(RequestMsg requestMsg)
    {
        if (_requestMsg != null)
        {
            _requestMsg.clear();
        }
        else
        {
            _requestMsg = (RequestMsg)CodecFactory.createMsg();
        }
        _requestMsg.msgClass(MsgClasses.REQUEST);
        return requestMsg.copy(_requestMsg, CopyMsgFlags.ALL_FLAGS);
    }
    
    /* Returns the current refresh state of this stream */
    int refreshState()
    {
        return _refreshState;
    }

    /* Sets the refresh state of this stream defined in RefreshStates */
    void refreshState(int refreshState)
    {
    	_refreshState = refreshState;
    }
    
    /* Returns the service associated with this stream. */
    WlService wlService()
    {
        return _wlService;
    }
    
    /* Sets the service associated with this stream. */
    void wlService(WlService wlService)
    {
       _wlService = wlService;
    }
    
    /* Returns the group id associated with this stream. */
    WlItemGroup itemGroup()
    {
    	return _itemGroup;
    }
    
    /* Sets the group id associated with this stream. */
    void itemGroup(WlItemGroup itemGroup)
    {
    	_itemGroup = itemGroup;
    }
    
    /* Returns the waiting request list. Handle cases where request could not be
     * submitted with directory stream up but there was a pending multi-part refresh
     * or snapshot in progress. */
    LinkedList<WlRequest> waitingRequestList()
    {
        return _waitingRequestList;
    }
    
    /* Returns list of user requests associated with this stream (used for fanout). */
    LinkedList<WlRequest> userRequestList()
    {
        return _userRequestList;
    }
    
    /* Returns item aggregation key associated with this stream. */
    WlItemAggregationKey itemAggregationKey()
    {
        return _itemAggregationKey;
    }
    
    /* Sets item aggregation key associated with this stream. */
    void itemAggregationKey(WlItemAggregationKey itemAggregationKey)
    {
        _itemAggregationKey = itemAggregationKey;
    }
    
    /* Handles channel up event. */
    void channelUp()
    {
        _channelUp = true;  
    }
    
    /* Handles channel down event. */
    void channelDown()
    {
        _refreshState = RefreshStates.REFRESH_NOT_REQUIRED;
        _requestPending = false;
        _channelUp = false;
    }
    
    /* Response received for this stream. */
    void responseReceived()
    {
    	_requestPending = false;
    }
    
    /* Handles a timeout for the stream. */
    public int timeout(ReactorErrorInfo errorInfo)
    {
        int ret = ReactorReturnCodes.SUCCESS;
        
        // if stream is CLOSED just return SUCCESS
        if (_state.streamState() == StreamStates.CLOSED)
        {
            return ret;
        }
        
        long currentTime = System.nanoTime();

        // handle request timeout
        // if request pending, resend request
        if (_requestExpireTime <= currentTime &&  _requestPending)
        {
            // encode into buffer and send out
            if (isChannelUp())
            {
                // send close message first
                _closeMsg.streamId(_streamId);
                _closeMsg.domainType(_domainType);
                if ((ret = sendCloseMsg(_closeMsg, errorInfo)) >= ReactorReturnCodes.SUCCESS)
                {
                    // reset request pending flag
                	_requestPending = false;
                    
                    // notify handler of request timeout
                    ret = _handler.requestTimeout(this, errorInfo);
                }
            }
        }
        
        // change WRITE_FLUSH_FAILED and NO_BUFFERS to SUCCESS
        if (ret == TransportReturnCodes.WRITE_FLUSH_FAILED ||
            ret == TransportReturnCodes.NO_BUFFERS )
        {
            ret = ReactorReturnCodes.SUCCESS;
        }
        
        // change FAILURE when channel state is not ACTIVE to SUCCESS
        if (ret == TransportReturnCodes.FAILURE && 
        	_reactorChannel.channel().state() != ChannelState.ACTIVE)
        {
            ret = ReactorReturnCodes.SUCCESS;
        }
        
        // handle any post ACK timeouts
        if (_postTimeoutInfoList.size() > 0)
        {
            while (_postTimeoutInfoList.peek() != null && _postTimeoutInfoList.peek().timeout() <= currentTime)
            {
            	WlPostTimeoutInfo postTimeoutInfo = _postTimeoutInfoList.poll();
                
                // NAK if ACK requested and not already ACKed
                
                // retrieve postSeqNumToMsgTable from postIdToMsgTable by post id
                HashMap<Long,PostMsg> postSeqNumToMsgTable = _postIdToMsgTable.get(postTimeoutInfo.postMsg().postId());
                if (postSeqNumToMsgTable != null)
                {
                    // retrieve PostMsg from postSeqNumToMsgTable by sequence number
                    // it will still be in table if it hasn't been ACKed
                    long seqNum = postTimeoutInfo.postMsg().checkHasSeqNum() ? postTimeoutInfo.postMsg().seqNum() : 0;
                    PostMsg postMsg = postSeqNumToMsgTable.get(seqNum);
                    if (postMsg != null)
                    {
                        // PostMsg hasn't been ACKed, send NAK to user if ACK requested
                        if (postTimeoutInfo.postMsg().checkAck())
                        {
                            ret = sendNak(postTimeoutInfo.postMsg(), errorInfo);
                        }
                        
                        // decrement number of outstanding post messages
                        _watchlist.numOutstandingPosts(_watchlist.numOutstandingPosts() - 1);
    
                        // remove PostMsg from postSeqNumToMsgTable and add back to pool
                        _postMsgPool.add(postSeqNumToMsgTable.remove(postMsg.seqNum()));
                        /* if there are no more entries in postSeqNumToMsgTable,
                           remove postSeqNumToMsgTable from postIdToMsgTable and add back to pool */
                        if (postSeqNumToMsgTable.size() == 0)
                        {
                            _postMsgHashMapPool.add(_postIdToMsgTable.remove(postTimeoutInfo.postMsg().postId()));
                        }
                    }
                }
                
                postTimeoutInfo.returnToPool();
            }
        }
        
        return ret;
    }
    
    /* Sends a message to the stream. */
    int sendMsg(Msg msg, ReactorSubmitOptions submitOptions, ReactorErrorInfo errorInfo)
    {
        int ret = ReactorReturnCodes.SUCCESS;

		if (msg.msgClass() == MsgClasses.REQUEST)
		{
			if (_requestsWithViewCount > 0 && _requestsWithViewCount == _userRequestList.size() &&
					_watchlist._loginHandler._loginRefresh.features().checkHasSupportViewRequests() &&
					_watchlist._loginHandler._loginRefresh.features().supportViewRequests() == 1)
			{
				if (_pendingViewChange && (_refreshState != RefreshStates.REFRESH_VIEW_PENDING))
				{
					_viewSubsetContained = false;
					if (_aggregateView.viewHandler().aggregateViewContainsNewViews(_aggregateView))
						_viewSubsetContained = true;
					_aggregateView.viewHandler().aggregateViewMerge(_aggregateView);

					msg.flags(msg.flags() | RequestMsgFlags.HAS_VIEW);

					_viewByteBuffer.clear();
					_viewBuffer.data(_viewByteBuffer);
					_eIter.clear();
					_eIter.setBufferAndRWFVersion(_viewBuffer, _reactorChannel.majorVersion(), _reactorChannel.minorVersion());
					_aggregateView.viewHandler().encodeViewRequest(_eIter, _aggregateView);
					msg.containerType(DataTypes.ELEMENT_LIST);
					msg.encodedDataBody(_viewBuffer);
				}
				else
				{
					// until viewRefresh is applied		
					// add to waiting request list
					handler().addPendingRequest(null);

					return ReactorReturnCodes.SUCCESS;
				}
			}
			else
			{
				removeViewFromMsg(msg);
			}
		}

        // encode into buffer and send out
        if (isChannelUp()) // channel is up
        {
	        if (msg.msgClass() != MsgClasses.CLOSE) // not a close message
	        {
	            if (msg.domainType() != DomainTypes.LOGIN  && 
	            		msg.msgClass() == MsgClasses.REQUEST && _userRequestList.size()  > 0)
	            {
	            	if (_requestsPausedCount == _userRequestList.size() 
	            			&& _watchlist._loginHandler._loginRefresh.checkHasFeatures() 
	            			&& _watchlist._loginHandler._loginRefresh.features().checkHasSupportOptimizedPauseResume() 
	            			&& _watchlist._loginHandler._loginRefresh.features().supportOptimizedPauseResume() == 1)
	            	{	
	            		((RequestMsg)msg).applyPause();
	            		_paused = true;
	            	}
	            	else
	            	{
	            		if (_paused)
	            		{
	            			((RequestMsg)msg).flags(((RequestMsg)msg).flags() & ~RequestMsgFlags.PAUSE);
	                        _paused = false;
	            		}                	
	            	}
	            }
	                     
	            ret = encodeIntoBufferAndWrite(msg, submitOptions, errorInfo);	
	            	
	            if (ret >= ReactorReturnCodes.SUCCESS)
	            {
	                // start request timer if request and refresh expected and refresh not already pending
	                if (msg.msgClass() == MsgClasses.REQUEST)
	                {
	                    // copy submitted request message to cached request message if not done already
	                    if (msg != _requestMsg)
	                    {
	                        if ((ret = requestMsg((RequestMsg)msg)) < ReactorReturnCodes.SUCCESS)
	                        {
	                            return ret;
	                        }
	                    }
	                    
	                    // start timer if request is not already pending
	                    if ( !_requestPending && !((RequestMsg)msg).checkNoRefresh())
	                    {
	                        if (startRequestTimer(errorInfo) != ReactorReturnCodes.SUCCESS)
	                        {
	                            return ReactorReturnCodes.FAILURE;
	                        }
	                    }
	                    if ( _pendingViewChange)
	                    {
	                    	// no need to send refresh back on other app streams already received
	                    	_pendingViewChange = false;                        	
	                    	if (!((RequestMsg)msg).checkNoRefresh() && !_viewSubsetContained)
	                    		_refreshState = RefreshStates.REFRESH_VIEW_PENDING;
	                                        
	                        if (_aggregateView != null && 	(msg.flags() & RequestMsgFlags.HAS_VIEW) > 0)
	                        	_aggregateView.viewHandler().aggregateViewCommit(_aggregateView);
	
	                        if (_aggregateView != null && _requestsWithViewCount == 0 )
	                        {
	                        	_aggregateView.viewHandler().aggregateViewDestroy(_aggregateView);
	                        	_aggregateView = null;
	                        }                    		
	                	}                        
	                }
	            } 
	            else if (ret == ReactorReturnCodes.NO_BUFFERS && msg.msgClass() != MsgClasses.POST)
	            {
	            	  if (msg.msgClass() == MsgClasses.REQUEST)
	            	  {
	            		  if (msg != requestMsg())
								requestMsg((RequestMsg)msg);
		            		handler().addPendingRequest(this);
	            	  }
	                 return ReactorReturnCodes.SUCCESS;
	            }
            }
            else // close message
            {
                ret = sendCloseMsg(msg, errorInfo);
            }
        }
        else // channel is not up, it means transport channel is gone, should not send out anything.
        {
        	return ReactorReturnCodes.SUCCESS;
        }
        
        return ret;
    }

    private void removeViewFromMsg(Msg msg)
    {
    	/* Clears the view flag and the payload only when there is a view for this item stream. But the reissue request doesn't have a view flag to clear the view */
    	if( (msg.flags() & RequestMsgFlags.HAS_VIEW) != 0 )
    	{
    		msg.flags(msg.flags() & ~RequestMsgFlags.HAS_VIEW);
    		_viewBuffer.clear();
    		msg.encodedDataBody(_viewBuffer);
    		msg.containerType(DataTypes.NO_DATA);
    		if(_aggregateView != null && _aggregateView.viewHandler() != null)
    		{
    			_aggregateView.viewHandler().aggregateViewUncommit(_aggregateView);
    		}
    		_pendingViewChange = true;
    		_viewSubsetContained = false;
    	}
    }

    /* Update the applicable post tables after successfully sending a post message. */
    int updatePostTables(PostMsg postMsg, ReactorErrorInfo errorInfo)
    {
        // need to make copy of PostMsg before adding to tables
        PostMsg postMsgCopy = _postMsgPool.poll();
        if (postMsgCopy == null)
        {
            postMsgCopy = (PostMsg)CodecFactory.createMsg();
        }
        postMsgCopy.clear();
        postMsgCopy.msgClass(MsgClasses.POST);
        postMsg.copy(postMsgCopy, CopyMsgFlags.ALL_FLAGS);
        
        // retrieve postSeqNumToMsgTable from postIdToMsgTable
        HashMap<Long,PostMsg> postSeqNumToMsgTable = _postIdToMsgTable.get(postMsg.postId());
        
        // create new postSeqNumToMsgTable if post id isn't already in use
        if (postSeqNumToMsgTable == null)
        {
            HashMap<Long,PostMsg> postMsgHashMap;
            if ((postMsgHashMap = _postMsgHashMapPool.poll()) == null)
            {
                postMsgHashMap = new HashMap<Long,PostMsg>();
            }
            
            // set postSeqNumToMsgTable to newly created postMsgHashMap
            postSeqNumToMsgTable = postMsgHashMap;
            
            // insert postSeqNumToMsgTable into postIdToMsgTable by post id
            _postIdToMsgTable.put(postMsg.postId(), postSeqNumToMsgTable);            
        }
        
        // insert PostMsg into postSeqNumToMsgTable by sequence number
        long seqNum = postMsg.checkHasSeqNum() ? postMsg.seqNum() : 0;
        
        postSeqNumToMsgTable.put(seqNum, postMsgCopy);

        // always start post ACK timer even for multi-part since each part gets ACK
        /* this has the dual purpose of aging out entries in the _postIdToMsgTable
           and NAKing when there's no response to the post message */
        long postExpireTime = (_watchlist.watchlistOptions().postAckTimeout() * 1000000L) + System.nanoTime();
        if (_watchlist.startWatchlistTimer(postExpireTime, this, errorInfo) != ReactorReturnCodes.SUCCESS)
        {
            return ReactorReturnCodes.FAILURE;
        }
        
        // insert PostMsg into timeout list
        WlPostTimeoutInfo postTimeoutInfo = ReactorFactory.createWlPostTimeoutInfo();                        
        postTimeoutInfo.timeout(postExpireTime);            
        postTimeoutInfo.postMsg(postMsg);
        
        _postTimeoutInfoList.add(postTimeoutInfo);

        return ReactorReturnCodes.SUCCESS;        
    }

    /* Validates a post message submit. */
    int validatePostSubmit(PostMsg postMsg, ReactorErrorInfo errorInfo)
    {
        if (_domainType != DomainTypes.LOGIN || postMsg.checkHasMsgKey())
        {
            // make sure post message domain isn't an administrative domain
            if (postMsg.domainType() == DomainTypes.LOGIN ||
                postMsg.domainType() == DomainTypes.SOURCE ||
                postMsg.domainType() == DomainTypes.DICTIONARY)
            {
                // cannot submit post message with login domain type
                return _watchlist.reactor().populateErrorInfo(errorInfo,
                                                              ReactorReturnCodes.INVALID_USAGE,
                                                              "WlStream.submitMsg",
                                                              "Cannot submit PostMsg with administrative domain type.");
            }
            
            // check post id and sequence number if ACK required 
            if (postMsg.checkAck())
            {
                // make sure post message has post id if ACK required
                if (!postMsg.checkHasPostId())
                {
                    // cannot submit post requiring ack with no post id
                    return _watchlist.reactor().populateErrorInfo(errorInfo,
                                                                  ReactorReturnCodes.INVALID_USAGE,
                                                                  "WlStream.submitMsg",
                                                                  "Cannot submit PostMsg requiring ack with no post id.");
                }
                
                // make sure multi-part post message has sequence number
                if (!postMsg.checkPostComplete() && !postMsg.checkHasSeqNum())
                {
                    // cannot submit multi-part post message with no sequence number
                    return _watchlist.reactor().populateErrorInfo(errorInfo,
                                                                  ReactorReturnCodes.INVALID_USAGE,
                                                                  "WlStream.submitMsg",
                                                                  "Cannot submit multi-part PostMsg with no sequence number.");                   
                }
                
                // make sure multi-part message is valid if post id is already in use
                HashMap<Long,PostMsg> postSeqNumToMsgTable = _postIdToMsgTable.get(postMsg.postId());
                if (postSeqNumToMsgTable != null)
                {
                    // check for multi-part post and return error if sequence number invalid
                    if (postMsg.checkHasSeqNum())
                    {
                        if (postSeqNumToMsgTable.containsKey(postMsg.seqNum()))
                        {
                            return _watchlist.reactor().populateErrorInfo(errorInfo,
                                                                          ReactorReturnCodes.INVALID_USAGE,
                                                                          "WlStream.handlePostSubmit",
                                                                          "Cannot submit multi-part PostMsg with duplicate sequence number " + postMsg.seqNum() + ", postId = " + postMsg.postId());
                        }
                    }
                    else
                    {
                        if (!postMsg.checkHasSeqNum())
                        {
                            return _watchlist.reactor().populateErrorInfo(errorInfo,
                                                                          ReactorReturnCodes.INVALID_USAGE,
                                                                          "WlStream.handlePostSubmit",
                                                                          "Cannot submit PostMsg with duplicate postId of " + postMsg.postId());
                        }
                    }
                }
            }
        }
        else
        {
            // cannot submit post with no MsgKey
            return _watchlist.reactor().populateErrorInfo(errorInfo,
                                                          ReactorReturnCodes.INVALID_USAGE,
                                                          "WlStream.submitMsg",
                                                          "Cannot submit PostMsg with no MsgKey.");
        }

        return ReactorReturnCodes.SUCCESS;        
    }

    /* Handles a post ACK message. */
    boolean handlePostAck(Msg msg)
    {
        // return false if post id was already removed from table, true if still in table
        boolean ret = false;
        
        // retrieve postSeqNumToMsgTable from postIdToMsgTable by ACK id which is the same as post id
        HashMap<Long,PostMsg> postSeqNumToMsgTable = _postIdToMsgTable.get(((AckMsg)msg).ackId());
        if (postSeqNumToMsgTable != null)
        {
            // decrement number of outstanding post messages
            _watchlist.numOutstandingPosts(_watchlist.numOutstandingPosts() - 1);
            
            // remove PostMsg from postSeqNumToMsgTable and add back to pool
            long seqNum = ((AckMsg)msg).checkHasSeqNum() ? ((AckMsg)msg).seqNum() : 0;
            PostMsg postMsg = postSeqNumToMsgTable.get(seqNum);
            if (postMsg != null)
            {
                // remove PostMsg from postSeqNumToMsgTable and add back to pool
                _postMsgPool.add(postSeqNumToMsgTable.remove(postMsg.seqNum()));

            	/* if there are no more entries in postSeqNumToMsgTable,
                   remove postSeqNumToMsgTable from postIdToMsgTable and add back to pool */
                if (postSeqNumToMsgTable.size() == 0)
                {
                    _postMsgHashMapPool.add(_postIdToMsgTable.remove(((AckMsg)msg).ackId()));
                }
                
                // set message's stream id to that of post message for fanout
                msg.streamId(postMsg.streamId());
                
                ret = true;
            }
        }
        return ret;
    }
    
    /* Sends a NAK message to the application. */
    int sendNak(PostMsg postMsg, ReactorErrorInfo errorInfo)
    {
        _ackMsg.streamId(postMsg.streamId());
        _ackMsg.domainType(postMsg.domainType());
        _ackMsg.containerType(DataTypes.NO_DATA);
        _ackMsg.flags(AckMsgFlags.NONE);
        _ackMsg.applyHasNakCode();
        _ackMsg.nakCode(NakCodes.NO_RESPONSE);
        _ackMsg.applyHasText();
        _ackMsg.text().data("No Ack received for PostMsg with postId = " + postMsg.postId());
        _ackMsg.ackId(postMsg.postId());
        _ackMsg.seqNum(postMsg.seqNum());

        if (postMsg.checkHasSeqNum())
            _ackMsg.applyHasSeqNum();
         
        // call back with NAK message
        WlInteger tempWlInteger = ReactorFactory.createWlInteger();
        tempWlInteger.value(_ackMsg.streamId());
        
        // call back item handler with NAK message
        return _watchlist.itemHandler().callbackUser("WlStream.sendNak", _ackMsg, null, _watchlist.streamIdtoWlRequestTable().get(tempWlInteger), errorInfo);
    }
    
    /* Encodes a ETA message into buffer and writes to channel. */
    int encodeIntoBufferAndWrite(Msg msg, ReactorSubmitOptions submitOptions, ReactorErrorInfo errorInfo)
    {
        int ret = ReactorReturnCodes.SUCCESS;
        
        // lazily initialize channel info to get maxFragmentSize
        if (_reactorChannelInfo.channelInfo().maxFragmentSize() == 0)
        {
            if ((ret = _reactorChannel.info(_reactorChannelInfo, errorInfo)) < ReactorReturnCodes.SUCCESS)
            {
            	if (ret == ReactorReturnCodes.FAILURE && _reactorChannel.channel().state() != ChannelState.ACTIVE)
            		return ReactorReturnCodes.SUCCESS;
            	else
            		return ret;
            }
        }
        
        TransportBuffer buffer = _reactorChannel.getBuffer(_reactorChannelInfo.channelInfo().maxFragmentSize(), false, errorInfo);
        
        if (buffer != null)
        {
            _eIter.clear();
            _eIter.setBufferAndRWFVersion(buffer, _reactorChannel.majorVersion(), _reactorChannel.minorVersion());
            
            if ((ret = msg.encode(_eIter)) < CodecReturnCodes.SUCCESS)
            {
                return ret;
            }
            
            ret = _reactor.submitChannel(_reactorChannel, buffer, submitOptions, errorInfo);
            switch (ret)
            {
                case TransportReturnCodes.SUCCESS:
                case TransportReturnCodes.WRITE_FLUSH_FAILED:
                    // don't release buffer here
                    break;
                case TransportReturnCodes.WRITE_CALL_AGAIN:
                    // call submit again until it passes
                    while (ret == TransportReturnCodes.WRITE_CALL_AGAIN)
                    {
                        ret = _reactor.submitChannel(_reactorChannel, buffer, submitOptions, errorInfo);
                        try
                        {
                            Thread.sleep(1);
                        }
                        catch (InterruptedException e) { }
                    }
                    break;
                default:
                    _reactorChannel.releaseBuffer(buffer, errorInfo);
                    break;
            }
        }
        else
        {
            return _reactor.populateErrorInfo(errorInfo,
                                              ReactorReturnCodes.NO_BUFFERS,
                                              "Watchlist.encodeIntoBufferAndQueue",
                                              "channel out of buffers errorId="
                                                     + errorInfo.error().errorId() + " errorText="
                                                     + errorInfo.error().text());
        }

        return ret;
    }
    
    /* Sends close message for the stream. */
    int sendCloseMsg(Msg msg, ReactorErrorInfo errorInfo)
    {
        int ret = ReactorReturnCodes.SUCCESS;

        // if close message cannot be sent keep trying until close message is sent
        do
        {
            ret = encodeIntoBufferAndWrite(msg, _submitOptions, errorInfo);
            if (ret == ReactorReturnCodes.NO_BUFFERS || ret == TransportReturnCodes.WRITE_FLUSH_FAILED)
            {
                // increase buffers to move things along
                int ret2;
                _reactorChnlInfo.clear();
                if ((ret2 = _reactorChannel.info(_reactorChnlInfo, errorInfo)) >= ReactorReturnCodes.SUCCESS)
                {
                    int newNumberOfBuffers = _reactorChnlInfo.channelInfo().guaranteedOutputBuffers() + 2;
                    if ((ret2 =_reactorChannel.ioctl(IoctlCodes.NUM_GUARANTEED_BUFFERS, newNumberOfBuffers, errorInfo)) < ReactorReturnCodes.SUCCESS)
                    {
                        return ret2;
                    }
                }
                else
                    return ret2;
            }
        } while (ret == ReactorReturnCodes.NO_BUFFERS || ret == TransportReturnCodes.WRITE_FLUSH_FAILED);
      
        return ret;
    }
    
    /* Closes the stream. */
    void close()
    {
        assert(!inPool());
        // set state to closed
        _state.clear();
        _state.streamState(StreamStates.CLOSED);
        _state.dataState(DataStates.SUSPECT);                
        // remove this stream from watchlist table
        if (_tableKey != null)
        {
            _watchlist.streamIdtoWlStreamTable().remove(_tableKey);
            _tableKey.returnToPool();
            _tableKey = null;
        }
        _requestsPausedCount = 0;
        _paused = false;
    }
    
    /* Is the channel up? */
    boolean isChannelUp()
    {
        if (_reactorChannel.state() == ReactorChannel.State.UP ||
            _reactorChannel.state() == ReactorChannel.State.READY)
        {
            _channelUp = true;
        }
        else
        {
            _channelUp = false;
        }
        
        return _channelUp;
    }
    
    int startRequestTimer(ReactorErrorInfo errorInfo)
    {
        int ret = ReactorReturnCodes.SUCCESS;

        _requestPending = true;
        
        if (_watchlist.watchlistOptions().requestTimeout() > 0)
        {
            _requestExpireTime = (_watchlist.watchlistOptions().requestTimeout() * 1000000L) + System.nanoTime();
        
            ret = _watchlist.startWatchlistTimer(_requestExpireTime, this, errorInfo);
        }
        
        return ret;
    }
    
    void tableKey(WlInteger tableKey)
    {
        _tableKey = tableKey;
    }
    
    WlInteger tableKey()
    {
        return _tableKey;
    }
    
    void groupTableKey(WlInteger groupTableKey)
    {
        _groupTableKey = groupTableKey;
    }
    
    WlInteger groupTableKey()
    {
        return _groupTableKey;
    }
    
    boolean requestPending()
    {
        return _requestPending;
    }
    
    int numPausedRequestsCount()
    {
        return _requestsPausedCount;
    }

    /* Sets number of paused request count */
    void numPausedRequestsCount(int numPausedRequestCount)
    {
    	_requestsPausedCount = numPausedRequestCount;
    }    
        
    /* Clear state of watchlist stream for re-use. */
    void clear()
    {
        _streamId = 0;
        _domainType = 0;
        _state.clear();
        _refreshState = RefreshStates.REFRESH_NOT_REQUIRED;
        _channelUp = false;
        _requestPending = false;
        _handler  = null;
        _ackMsg.clear();
        _ackMsg.msgClass(MsgClasses.ACK);
        _closeMsg.clear();
        _closeMsg.msgClass(MsgClasses.CLOSE);
        _reactorChnlInfo.clear();
        _requestsPausedCount = 0;
        _paused = false;
        _requestMsg = null;
        _itemAggregationKey = null;
        _requestExpireTime = 0;
        _eIter.clear();
        _reactorChannelInfo.clear();
        _submitOptions.clear();
        _itemGroup = null;
        _tableKey = null;
        _groupTableKey = null;
        _wlService = null;
        // return any WlPostTimeoutInfo back to pool
        WlPostTimeoutInfo postTimeoutInfo = null;
        while ((postTimeoutInfo = _postTimeoutInfoList.poll()) != null)
        {
            postTimeoutInfo.returnToPool();
        }
        _postTimeoutInfoList.clear();
        // return any PostMsgs back to pool
        for (HashMap<Long,PostMsg> postMsgHashMap : _postIdToMsgTable.values())
        {
            for (PostMsg postMsg : postMsgHashMap.values())
            {
                _postMsgPool.add(postMsg);
            }
            postMsgHashMap.clear();
            _postMsgHashMapPool.add(postMsgHashMap);
        }
        _postIdToMsgTable.clear();
        _waitingRequestList.clear();
        _userRequestList.clear();
        if (_aggregateView != null) 
    	{
        	_aggregateView.clear();
        	_aggregateView.returnToPool();
        	_aggregateView = null;
    	}
        _requestsWithViewCount = 0;
        _reactorChannel = null;
        _reactor = null;
        _addedToTimeOutList = false;
    }
    
	WlView aggregateView()
	{
		return _aggregateView;
	}

	public void aggregateView(WlView aggregateView)
	{
		_aggregateView = aggregateView;
	}
      
	@Override
	public void returnToPool()
	{
		assert(!inPool());
		
		// return any WlPostTimeoutInfo back to pool
		WlPostTimeoutInfo postTimeoutInfo = null;
		while ((postTimeoutInfo = _postTimeoutInfoList.poll()) != null)
		{
			postTimeoutInfo.returnToPool();
		}
		_postTimeoutInfoList.clear();
        
		if (_aggregateView != null)
		{
        		_aggregateView.clear();
        		_aggregateView.returnToPool();
        		_aggregateView = null;
        }

		if(_addedToTimeOutList)
		{
			_addedToTimeOutList = false;
			_watchlist._streamTimeoutInfoList.remove(this);
		}
        
		_handler  = null;
		_requestMsg = null;
		_itemAggregationKey = null;
		_reactorChannel = null;
		_reactor = null;
		_itemGroup = null;
		_tableKey = null;
		_groupTableKey = null;
		_wlService = null;
		
		super.returnToPool();
	}
}
