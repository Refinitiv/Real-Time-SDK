package com.thomsonreuters.upa.valueadd.reactor;

import java.util.HashMap;
import java.util.LinkedList;

import com.thomsonreuters.upa.codec.AckMsg;
import com.thomsonreuters.upa.codec.AckMsgFlags;
import com.thomsonreuters.upa.codec.CloseMsg;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.CopyMsgFlags;
import com.thomsonreuters.upa.codec.DataStates;
import com.thomsonreuters.upa.codec.DataTypes;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.Msg;
import com.thomsonreuters.upa.codec.MsgClasses;
import com.thomsonreuters.upa.codec.NakCodes;
import com.thomsonreuters.upa.codec.PostMsg;
import com.thomsonreuters.upa.codec.RequestMsg;
import com.thomsonreuters.upa.codec.State;
import com.thomsonreuters.upa.codec.StreamStates;
import com.thomsonreuters.upa.rdm.DomainTypes;
import com.thomsonreuters.upa.transport.IoctlCodes;
import com.thomsonreuters.upa.transport.TransportBuffer;
import com.thomsonreuters.upa.transport.TransportReturnCodes;
import com.thomsonreuters.upa.valueadd.common.VaNode;

/* Watchlist stream that handles basic stream management. */
class WlStream extends VaNode
{
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
    
    long _requestExpireTime;
    
    RequestMsg _requestMsg; // current request message associated with stream
                            // this is the request exchanged with the ADS or Provider
    
    boolean _multiPartRefreshPending; // flag to indicate a multi-part refresh is pending on the stream
    int _numSnapshotsPending; // number of snapshots pending on the stream
    
    /* waiting request list to handle cases where request could not be submitted with
     * directory stream up but there was a pending multi-part refresh or snapshot in progress */
    LinkedList<WlRequest> _waitingRequestList = new LinkedList<WlRequest>();
    
    // list of user requests associated with this stream (used for fanout)
    LinkedList<WlRequest> _userRequestList = new LinkedList<WlRequest>();

    // unsent message queue
    LinkedList<Msg> _unsentMsgQueue = new LinkedList<Msg>();
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
    
    WlStream()
    {
        _ackMsg = (AckMsg)CodecFactory.createMsg();
        _ackMsg.msgClass(MsgClasses.ACK);
        _closeMsg = (CloseMsg)CodecFactory.createMsg();
        _closeMsg.msgClass(MsgClasses.CLOSE);
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
    
    /* Returns whether or not a multi-part refresh is pending. */
    boolean multiPartRefreshPending()
    {
        return _multiPartRefreshPending;
    }

    /* Sets whether or not a multi-part refresh is pending. */
    void multiPartRefreshPending(boolean multiPartRefreshPending)
    {
        _multiPartRefreshPending = multiPartRefreshPending;
    }
    
    /* Returns number of snapshots pending. */
    int numSnapshotsPending()
    {
        return _numSnapshotsPending;
    }

    /* Sets number of snapshots pending. */
    void numSnapshotsPending(int numSnapshotsPending)
    {
        _numSnapshotsPending = numSnapshotsPending;
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
        
        // trigger dispatch if there are unsent messages to send
        if (!_unsentMsgQueue.isEmpty())
        {
            _watchlist.reactor().sendWatchlistDispatchNowEvent(_watchlist.reactorChannel());
        }
    }
    
    /* Handles channel down event. */
    void channelDown()
    {
        _channelUp = false;
    }
    
    /* Response received for this stream. */
    void responseReceived()
    {
        _requestPending = false;
    }
    
    /* Dispatch the stream. */
    int dispatch(ReactorErrorInfo errorInfo)
    {
        int ret = ReactorReturnCodes.SUCCESS;
        
        // if stream is CLOSED just return SUCCESS
        if (_state.streamState() == StreamStates.CLOSED)
        {
            return ret;
        }

        // send any unsent messages
        Msg msg;
        
        while ((msg = _unsentMsgQueue.poll()) != null)
        {
            // return msg to pool if not cached request message
            if (msg != _requestMsg)
            {
                _msgPool.add(msg);
            }
            
            // send message
            if ((ret = sendMsg(msg, _submitOptions, errorInfo)) < ReactorReturnCodes.SUCCESS)
            {
                return ret;
            }
        }

        return ret;
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
        if (_requestExpireTime <= currentTime && _requestPending)
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
            ret == TransportReturnCodes.NO_BUFFERS)
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
        
        // encode into buffer and send out
        if (isChannelUp()) // channel is up
        {
            if (msg.msgClass() != MsgClasses.CLOSE) // not a close message
            {
                // if post message, save user stream id and replace with stream id of stream
                int userStreamId = msg.streamId();
                if (msg.msgClass() == MsgClasses.POST)
                {
                    msg.streamId(_streamId);
                }
                
                ret =  encodeIntoBufferAndWrite(msg, submitOptions, errorInfo);
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
                        if (!_requestPending && !((RequestMsg)msg).checkNoRefresh())
                        {
                            if (startRequestTimer(errorInfo) != ReactorReturnCodes.SUCCESS)
                            {
                                return ReactorReturnCodes.FAILURE;
                            }
                        }
                    }
                    
                    // if post message, increment number of outstanding post messages and update post tables
                    if (msg.msgClass() == MsgClasses.POST)
                    {
                        // increment number of outstanding post messages
                        _watchlist.numOutstandingPosts(_watchlist.numOutstandingPosts() + 1);
                        
                        // reset post message stream id back to user stream id
                        // this is needed so ACK/NAK response fanout is sent with user stream id
                        msg.streamId(userStreamId);
                        
                        // update post tables
                        ret = updatePostTables((PostMsg)msg, errorInfo);
                    }
                }   
                else if (ret == ReactorReturnCodes.NO_BUFFERS || ret == TransportReturnCodes.WRITE_FLUSH_FAILED)
                {
                   // change WRITE_FLUSH_FAILED and NO_BUFFERS and not post message to SUCCESS
                    // post messages return NO_BUFFERS
                    if (ret == TransportReturnCodes.WRITE_FLUSH_FAILED ||
                        (ret == TransportReturnCodes.NO_BUFFERS && msg.msgClass() != MsgClasses.POST))
                    {
                        ret = ReactorReturnCodes.SUCCESS;
                    }
                    
                    // if returning SUCCESS, add to unsent message queue and trigger dispatch
                    if (ret == ReactorReturnCodes.SUCCESS)
                    {
                        // if post message, reset post message stream id back to user stream id
                        // this is needed so ACK/NAK response fanout is sent with user stream id
                        if (msg.msgClass() == MsgClasses.POST)
                        {
                            msg.streamId(userStreamId);
                        }
                        
                        // add to unsent message queue and trigger dispatch
                        addToUnsentMsgQueue(msg);
                        _watchlist.reactor().sendWatchlistDispatchNowEvent(_watchlist.reactorChannel());
                    }
                }
                else // error
                {
                    // if post message, reset post message stream id back to user stream id
                    if (msg.msgClass() == MsgClasses.POST)
                    {
                        msg.streamId(userStreamId);
                    }
                }
            }
            else // close message
            {
                ret = sendCloseMsg(msg, errorInfo);
            }
        }
        else // channel is not up, queue for sending when channel comes back up
        {
            addToUnsentMsgQueue(msg);
        }
        
        return ret;
    }
    
    /* Adds message to unsent message queue for sending later. */
    void addToUnsentMsgQueue(Msg msg)
    {
        // make copy of message
        Msg msgCopy = _msgPool.poll();
        if (msgCopy == null)
        {
            msgCopy = CodecFactory.createMsg();
        }
        msgCopy.clear();
        msg.copy(msgCopy, CopyMsgFlags.ALL_FLAGS);

        // queue for send on next dispatch
        _unsentMsgQueue.add(msgCopy);        
    }
    
    /* Update the applicable post tables after successfully sending a post message. */
    int updatePostTables(PostMsg postMsg, ReactorErrorInfo errorInfo)
    {
        // insert PostMsg into lookup tables id ACK required
        
        if (postMsg.checkAck())
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
        }

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
                                                                          "Cannot submit multi-part PostMsg with duplicate sequence number.");
                        }
                    }
                    else
                    {
                        if (!postMsg.checkHasSeqNum())
                        {
                            return _watchlist.reactor().populateErrorInfo(errorInfo,
                                                                          ReactorReturnCodes.INVALID_USAGE,
                                                                          "WlStream.handlePostSubmit",
                                                                          "Cannot submit multi-part PostMsg with no sequence number.");
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
        return _handler.callbackUser("WlStream.sendNak", _ackMsg, null, errorInfo);
    }
    
    /* Encodes a UPA message into buffer and writes to channel. */
    int encodeIntoBufferAndWrite(Msg msg, ReactorSubmitOptions submitOptions, ReactorErrorInfo errorInfo)
    {
        int ret = ReactorReturnCodes.SUCCESS;
        
        // lazily initialize channel info to get maxFragmentSize
        if (_reactorChannelInfo.channelInfo().maxFragmentSize() == 0)
        {
            _reactorChannel.info(_reactorChannelInfo, errorInfo);
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
            }
        } while (ret == ReactorReturnCodes.NO_BUFFERS || ret == TransportReturnCodes.WRITE_FLUSH_FAILED);
      
        return ret;
    }
    
    /* Closes the stream. */
    void close()
    {
        // set state to closed
        _state.clear();
        _state.streamState(StreamStates.CLOSED);
        _state.dataState(DataStates.SUSPECT);                
        // remove this stream from watchlist table
        _watchlist.streamIdtoWlStreamTable().remove(_streamId);        
        // return this stream to pool
        returnToPool();
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
        _requestPending = true;
        
        _requestExpireTime = (_watchlist.watchlistOptions().requestTimeout() * 1000000L) + System.nanoTime();
        
        return _watchlist.startWatchlistTimer(_requestExpireTime, this, errorInfo);
    }
    
    /* Clear state of watchlist stream for re-use. */
    void clear()
    {
        _streamId = 0;
        _domainType = 0;
        _state.clear();
        _requestPending = false;
        _channelUp = false;
        _handler  = null;
        _ackMsg.clear();
        _ackMsg.msgClass(MsgClasses.ACK);
        _closeMsg.clear();
        _closeMsg.msgClass(MsgClasses.CLOSE);
        _reactorChnlInfo.clear();
        // return any unsent messages back into message pool
        Msg msg = null;
        while ((msg = _unsentMsgQueue.poll()) != null)
        {
            if (msg != _requestMsg)
            {
                _msgPool.add(msg);
            }
        }
        _requestMsg = null;
        _itemAggregationKey = null;
        _requestExpireTime = 0;
        _multiPartRefreshPending = false;
        _numSnapshotsPending = 0;
        _eIter.clear();
        _reactorChannelInfo.clear();
        _submitOptions.clear();
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
        // return any waiting requests back to pool
        WlRequest wlRequest = null;
        while ((wlRequest = _waitingRequestList.poll()) != null)
        {
            wlRequest.returnToPool();
        }
        // return any user requests back to pool
        wlRequest = null;
        while ((wlRequest = _userRequestList.poll()) != null)
        {
            wlRequest.returnToPool();
        }
    }
}
