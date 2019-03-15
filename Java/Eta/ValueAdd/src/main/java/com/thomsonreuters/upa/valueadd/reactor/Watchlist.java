package com.thomsonreuters.upa.valueadd.reactor;

import java.nio.ByteBuffer;
import java.util.HashMap;
import java.util.LinkedList;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.Codec;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.CopyMsgFlags;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.Msg;
import com.thomsonreuters.upa.codec.MsgClasses;
import com.thomsonreuters.upa.codec.MsgKey;
import com.thomsonreuters.upa.codec.MsgKeyFlags;
import com.thomsonreuters.upa.codec.RequestMsg;
import com.thomsonreuters.upa.codec.StreamStates;
import com.thomsonreuters.upa.rdm.DomainTypes;
import com.thomsonreuters.upa.valueadd.common.VaNode;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.MsgBase;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.dictionary.DictionaryMsg;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.dictionary.DictionaryMsgType;
import com.thomsonreuters.upa.valueadd.reactor.WlRequest.State;

/* The top-level watchlist class that routes both submitted and received message to the appropriate handlers. */
class Watchlist extends VaNode
{
    ReactorChannel _reactorChannel;
    Reactor _reactor;
    ConsumerRole _role;
    ConsumerWatchlistOptions _watchlistOptions;
    WlLoginHandler _loginHandler;
    WlDirectoryHandler _directoryHandler;
    WlItemHandler _itemHandler;
    int _nextStreamId; // used to give a unique stream id for all watchlist streams
    int _nextProviderStreamId; // used to give a unique provider stream id for all application streams used in symbollist stream
    HashMap<WlInteger,WlRequest> _streamIdtoWlRequestTable;
    HashMap<WlInteger,WlStream> _streamIdtoWlStreamTable;
    EncodeIterator _eIter = CodecFactory.createEncodeIterator();
    DecodeIterator _dIter = CodecFactory.createDecodeIterator();
    // list to track stream timeouts
    LinkedList<WlStream> _streamTimeoutInfoList = new LinkedList<WlStream>();
    
    int _numOutstandingPosts;
    
    Buffer _tempBuffer1 = CodecFactory.createBuffer();
    ByteBuffer _tempByteBuffer1;
    Buffer _tempBuffer2 = CodecFactory.createBuffer();
    ByteBuffer _tempByteBuffer2;
    Msg _tempMsg = CodecFactory.createMsg();
    
    WlInteger _tempWlInteger = ReactorFactory.createWlInteger();

    Watchlist(ReactorChannel reactorChannel, ConsumerRole consumerRole)
    {
        _reactorChannel = reactorChannel;
        _reactor = _reactorChannel.reactor();
        _role = consumerRole;
        _watchlistOptions = _role.watchlistOptions();
        if (_watchlistOptions.itemCountHint() > 0)
        {
            _streamIdtoWlRequestTable = new HashMap<WlInteger,WlRequest>(_watchlistOptions.itemCountHint() + 10, 1);
            _streamIdtoWlStreamTable = new HashMap<WlInteger,WlStream>(_watchlistOptions.itemCountHint() + 10, 1);
        }
        else
        {
            _streamIdtoWlRequestTable = new HashMap<WlInteger,WlRequest>();
            _streamIdtoWlStreamTable = new HashMap<WlInteger,WlStream>();
        }
        _loginHandler = new WlLoginHandler(this);
        _directoryHandler = new WlDirectoryHandler(this);
        _itemHandler = new WlItemHandler(this);
        _tempByteBuffer1 = ByteBuffer.allocate(8192);
        _tempBuffer1.data(_tempByteBuffer1);
        _tempByteBuffer2 = ByteBuffer.allocate(8192);
        _tempBuffer2.data(_tempByteBuffer2);
    }
    
    HashMap<WlInteger,WlStream> streamIdtoWlStreamTable()
    {
        return _streamIdtoWlStreamTable;
    }
    
    HashMap<WlInteger,WlRequest> streamIdtoWlRequestTable()
    {
        return _streamIdtoWlRequestTable;
    }
    
    /* Submit a Codec message to the watchlist. */
    int submitMsg(Msg msg, ReactorSubmitOptions submitOptions, ReactorErrorInfo errorInfo)
    {
        _tempWlInteger.value(msg.streamId());
        WlRequest wlRequest = _streamIdtoWlRequestTable.get(_tempWlInteger);
        boolean isReissue = false;

        if (msg.msgClass() == MsgClasses.REQUEST)
        {
            // make sure only either service name or service id is used in request
            if (submitOptions.serviceName() != null && ((RequestMsg)msg).msgKey().checkHasServiceId())
            {
                return _reactor.populateErrorInfo(errorInfo,
                                                  ReactorReturnCodes.INVALID_USAGE,
                                                  "Watchlist.submitMsg",
                                                  "Cannot submit request with both service name and service id specified.");                    
            }
            
            if (wlRequest == null)
            {
                // no stream association yet
                
                // make sure request does not have NO_REFRESH flag set
                if (((RequestMsg)msg).checkNoRefresh())
                {
                    return _reactor.populateErrorInfo(errorInfo,
                                                      ReactorReturnCodes.INVALID_USAGE,
                                                      "Watchlist.submitMsg",
                                                      "Cannot submit new item request to watchlist with NO_REFRESH flag set.");                    
                }

                // create watchlist request
                // use domain type to determine appropriate handler
                wlRequest = ReactorFactory.createWlRequest();
                switch (msg.domainType())
                {
                    case DomainTypes.LOGIN:
                        wlRequest.handler(_loginHandler);
                        break;
                    case DomainTypes.SOURCE:
                        wlRequest.handler(_directoryHandler);
                        break;
                    default: // all other domain types (including dictionary) handled by item handler
                        wlRequest.handler(_itemHandler);
                        break;
                }
            }
            else // stream already exists (this is a reissue)
            {
                // make sure reissue request isn't changing service name or service id
                int ret;
                if ((ret = validateReissue(wlRequest, (RequestMsg)msg, submitOptions, errorInfo)) < ReactorReturnCodes.SUCCESS)
                {
                    return ret;
                }
    
                // set reissue flag
                    isReissue = true;                    
            }
         
            // submit request
            int ret  = wlRequest.handler().submitRequest(wlRequest, (RequestMsg)msg, isReissue, submitOptions, errorInfo);
            // if successful, put in table and save necessary information
            if (ret >= ReactorReturnCodes.SUCCESS)
            {
                // save submitted request
                wlRequest.requestMsg().clear();
                
                // Don't need to copy the entire payload for the batch request as the items has been requested individually
                if (((RequestMsg)msg).checkHasBatch())
                	msg.copy(wlRequest.requestMsg(), CopyMsgFlags.ALL_FLAGS & (~CopyMsgFlags.DATA_BODY));
                else
                	msg.copy(wlRequest.requestMsg(), CopyMsgFlags.ALL_FLAGS);

                // add to watchlist request table if new request
                if (!isReissue)
                {
                    WlInteger wlInteger = ReactorFactory.createWlInteger();
                    wlInteger.value(msg.streamId());
                    wlRequest.tableKey(wlInteger);
                    _streamIdtoWlRequestTable.put(wlInteger, wlRequest);
                }
            }
            else // submit failed
            {
                // return WlRequest to pool if new request
                if (!isReissue)
                {
                    wlRequest.returnToPool();
                }
            }
            
            return ret;
        }
        else // not a request
        {
            // submit to appropriate handler if request stream already open, otherwise this is an error
            if (wlRequest != null)
            {
                // submit message
                return wlRequest.handler().submitMsg(wlRequest, msg, submitOptions, errorInfo);
            }
            else // error if not close
            {
                if (msg.msgClass() != MsgClasses.CLOSE)
                {
                    return _reactor.populateErrorInfo(errorInfo,
                                                      ReactorReturnCodes.INVALID_USAGE,
                                                      "Watchlist.submitMsg",
                                                      "Cannot submit message class " + MsgClasses.toString(msg.msgClass()) + " to watchlist before stream is opened with a REQUEST.");            
                }
                else
                {
                    return ReactorReturnCodes.SUCCESS;
                }
            }
        }
    }

    /* Submit an RDM message to the watchlist. */
    int submitMsg(MsgBase rdmMsg, ReactorSubmitOptions submitOptions, ReactorErrorInfo errorInfo)
    {
        // block user from submitting anything other than login, directory and dictionary domains
        if (rdmMsg.domainType() != DomainTypes.LOGIN &&
            rdmMsg.domainType() != DomainTypes.SOURCE &&
            rdmMsg.domainType() != DomainTypes.DICTIONARY)
        {
            return _reactor.populateErrorInfo(errorInfo,
                                              ReactorReturnCodes.INVALID_USAGE,
                                              "Watchlist.submitMsg",
                                              "Cannot submit domain type " + DomainTypes.toString(rdmMsg.domainType()) + " to watchlist as RDM message.");
        }
        
        // convert to Codec message
        _tempMsg.clear();
        convertRDMToCodecMsg(rdmMsg, _tempMsg);
        
        if (rdmMsg.domainType() == DomainTypes.DICTIONARY && ((DictionaryMsg)rdmMsg).rdmMsgType() == DictionaryMsgType.REQUEST
                && submitOptions.serviceName() != null)
        {
        	// DictionaryRequest.serviceId is not optional. If a service name was provided, let that take precedence over 
            // serviceId -- remove the serviceId from this message so that we don't get an error for having specified both.
        	_tempMsg.msgKey().flags(_tempMsg.msgKey().flags() & ~MsgKeyFlags.HAS_SERVICE_ID);
        }
        
        // submit with method that takes Codec message as argument
        return submitMsg(_tempMsg, submitOptions, errorInfo);
    }
    
    int validateReissue(WlRequest wlRequest, RequestMsg requestMsg, ReactorSubmitOptions submitOptions, ReactorErrorInfo errorInfo)
    {
        int ret = ReactorReturnCodes.SUCCESS;
        
        if (requestMsg.msgKey().checkHasServiceId())
        {
            // reissue request has service id
            if (!wlRequest.requestMsg().msgKey().checkHasServiceId() ||
                requestMsg.msgKey().serviceId() != wlRequest.requestMsg().msgKey().serviceId())
            {
                ret = ReactorReturnCodes.INVALID_USAGE;
            }
        }
        else // reissue request does not have service id
        {
            if (wlRequest.requestMsg().msgKey().checkHasServiceId())
            {
                ret = ReactorReturnCodes.INVALID_USAGE;
            }
            else if (submitOptions.serviceName() == null)
            {
                if (wlRequest.streamInfo().serviceName() != null)
                {
                    ret = ReactorReturnCodes.INVALID_USAGE;
                }                            
            }
            else
            {
                if (wlRequest.requestMsg().domainType() != DomainTypes.LOGIN && (wlRequest.streamInfo().serviceName() == null ||
                    !submitOptions.serviceName().equals(wlRequest.streamInfo().serviceName())))
                {
                    ret = ReactorReturnCodes.INVALID_USAGE;
                }                            
            }
        }
    
        if (ret < ReactorReturnCodes.SUCCESS)
        {
            ret = _reactor.populateErrorInfo(errorInfo,
                                             ReactorReturnCodes.INVALID_USAGE,
                                             "Watchlist.validateReissue",
                                             "Cannot change ServiceId or ServiceName on reissue.");
        }
        
        return ret;
    }

    /* Watchlist read message method. */
    int readMsg(WlStream wlStream, DecodeIterator dIter, Msg msg, ReactorErrorInfo errorInfo)
    {
        assert (wlStream != null);

        return wlStream.handler().readMsg(wlStream, dIter, msg, errorInfo);
    }
    
    int dispatch(ReactorErrorInfo errorInfo)
    {
        // dispatch all of the handlers
        int ret1 = _loginHandler.dispatch(errorInfo);
        int ret2 = _directoryHandler.dispatch(errorInfo);
        int ret3 = _itemHandler.dispatch(errorInfo);
        
        if (ret1 < ReactorReturnCodes.SUCCESS)
        {
            return ret1;
        }
        else if (ret2 < ReactorReturnCodes.SUCCESS)
        {
            return ret2;
        }
        else if (ret3 < ReactorReturnCodes.SUCCESS)
        {
            return ret3;
        }
        
        return ReactorReturnCodes.SUCCESS;
    }

    /* Handles watchlist timeout events. */
    int timeout(ReactorErrorInfo errorInfo)
    {
        int ret = ReactorReturnCodes.SUCCESS;
        
        // handle any stream timeouts
        WlStream wlStream = _streamTimeoutInfoList.poll();
        
        if  (wlStream != null)
        {
            ret = wlStream.timeout(errorInfo);
        }
        
        return ret;
    }

    /* Sets the reactor channel associated with the watchlist. */
    void reactorChannel(ReactorChannel reactorChannel)
    {
        _reactorChannel = reactorChannel;
        _reactor = _reactorChannel.reactor();
    }

    /* Returns the reactor channel associated with the watchlist. */
    ReactorChannel reactorChannel()
    {
        return _reactorChannel;
    }

    /* Returns the reactor associated with the watchlist. */
    Reactor reactor()
    {
        return _reactor;
    }

    /* Sets the consumer role associated with the watchlist. */
    void role(ConsumerRole role)
    {
        _role = role;
        _watchlistOptions = _role.watchlistOptions();
    }

    /* Returns the consumer role associated with the watchlist. */
    ConsumerRole role()
    {
        return _role;
    }

    /* Returns the options associated with the watchlist. */
    ConsumerWatchlistOptions watchlistOptions()
    {
        return _watchlistOptions;
    }
    
    int nextStreamId()
    {
        // handle rollover, watchlist stream id cannot be negative
        if (_nextStreamId == Integer.MAX_VALUE)
        {
            _nextStreamId = 0;
        }
        
        // keep attempting to get next stream id if already found in table
        int nextStreamId = ++_nextStreamId;
        _tempWlInteger.value(nextStreamId);
        while (_streamIdtoWlStreamTable.containsKey(_tempWlInteger))
        {
            nextStreamId = ++_nextStreamId;
            _tempWlInteger.value(nextStreamId);
        }

        return nextStreamId;
    }
    
    int nextProviderStreamId()
    {
        // handle rollover, provider stream id is negative
        if (_nextProviderStreamId == Integer.MAX_VALUE)
        {
            _nextProviderStreamId = 0;
        }
        
        // keep attempting to get next stream id if already found in table
        int nextProviderStreamId = ++_nextProviderStreamId;
        _tempWlInteger.value(-nextProviderStreamId);
        while (_streamIdtoWlRequestTable.containsKey(_tempWlInteger))
        {
            nextProviderStreamId = ++_nextProviderStreamId;
            _tempWlInteger.value(-nextProviderStreamId);
        }
        // negative
        return -nextProviderStreamId;
    }        

    /* Returns the login handler associated with the watchlist. */
    WlLoginHandler loginHandler()
    {
        return _loginHandler;
    }

    /* Returns the directory handler associated with the watchlist. */
    WlDirectoryHandler directoryHandler()
    {
        return _directoryHandler;
    }

    /* Returns the item handler associated with the watchlist. */
    WlItemHandler itemHandler()
    {
        return _itemHandler;
    }
    
    /* Returns the number of outstanding posts. */
    int numOutstandingPosts()
    {
        return _numOutstandingPosts;
    }

    /* Sets the number of outstanding posts. */
    void numOutstandingPosts(int numOutstandingPosts)
    {
        _numOutstandingPosts = numOutstandingPosts;
    }
    
    /* Handles channel down event. */
    void channelDown()
    {
        _loginHandler.channelDown();        
        _directoryHandler.deleteAllServices(true); // Delete all services (this will also trigger item status fanout)
    }

    /* Handles channel up event. */
    void channelUp(ReactorErrorInfo errorInfo)
    {
        _loginHandler.channelUp(errorInfo);
        _directoryHandler.channelUp(errorInfo);
        _itemHandler.channelUp(errorInfo);
    }
    
    void authenticationTimer(String authToken, ReactorErrorInfo errorInfo)
    {
    	// update the auth token and send it 
        _loginHandler.authenticationTimer(authToken, errorInfo);

    }
    
    /* Starts a watchlist timer. */
    int startWatchlistTimer(long expireTime, WlStream wlStream, ReactorErrorInfo errorInfo)
    {
        if (!_reactor.sendWorkerEvent(WorkerEventTypes.START_WATCHLIST_TIMER, _reactorChannel, expireTime))
        {
            // _reactor.sendWorkerEvent() failed, send channel down
            _reactor.sendWorkerEvent(WorkerEventTypes.CHANNEL_DOWN, _reactorChannel);
            _reactorChannel.state(com.thomsonreuters.upa.valueadd.reactor.ReactorChannel.State.DOWN);
            _reactor.sendAndHandleChannelEventCallback("Watchlist.startWatchlistTimer",
                                                  ReactorChannelEventTypes.CHANNEL_DOWN,
                                                  _reactorChannel, errorInfo);
            return _reactor.populateErrorInfo(errorInfo,
                              ReactorReturnCodes.FAILURE,
                              "Watchlist.startWatchlistTimer",
                              "_reactor.sendWorkerEvent() failed");
        }
        
        _streamTimeoutInfoList.add(wlStream);
    
        return ReactorReturnCodes.SUCCESS;
    }

    /* Converts an RDM message into a Codec message. */
    int convertRDMToCodecMsg(MsgBase rdmMsg, Msg msg)
    {
        int ret = CodecReturnCodes.SUCCESS;
        
        // clear temp buffer
        _tempBuffer1.clear();
        _tempByteBuffer1.clear();
        _tempBuffer1.data(_tempByteBuffer1);
        
        // encode RDM message into buffer
        _eIter.clear();
        _eIter.setBufferAndRWFVersion(_tempBuffer1, Codec.majorVersion(), Codec.minorVersion());
        
        ret = rdmMsg.encode(_eIter);
        
        while( ret == CodecReturnCodes.BUFFER_TOO_SMALL)
        {
        	_tempByteBuffer1 = ByteBuffer.allocate(_tempBuffer1.capacity() * 2 );
        	_tempBuffer1.clear();
        	_tempBuffer1.data(_tempByteBuffer1);
        	 _eIter.clear();
             _eIter.setBufferAndRWFVersion(_tempBuffer1, Codec.majorVersion(), Codec.minorVersion());
             ret = rdmMsg.encode(_eIter);
        }
        
        if (ret >= CodecReturnCodes.SUCCESS)
        {
            // decode encoded RDM message into Codec message
            _dIter.clear();
            _dIter.setBufferAndRWFVersion(_tempBuffer1, Codec.majorVersion(), Codec.minorVersion());
            ret = msg.decode(_dIter);
        }

        return ret;
    }
    
    /* Converts a Codec message into an RDM message. */
    int convertCodecToRDMMsg(Msg msg, MsgBase rdmMsg)
    {
        int ret = CodecReturnCodes.SUCCESS;
        
        // clear temp buffer
        _tempBuffer2.clear();
        _tempByteBuffer2.clear();
        _tempBuffer2.data(_tempByteBuffer2);
        
        // encode Codec message into buffer
        _eIter.clear();
        _eIter.setBufferAndRWFVersion(_tempBuffer2, Codec.majorVersion(), Codec.minorVersion());
        if ((ret = msg.encode(_eIter)) >= CodecReturnCodes.SUCCESS)
        {
            // decode encoded Codec message into RDM message
            _dIter.clear();
            _dIter.setBufferAndRWFVersion(_tempBuffer2, Codec.majorVersion(), Codec.minorVersion());
            _tempMsg.clear();
            _tempMsg.decode(_dIter);
            ret = rdmMsg.decode(_dIter, _tempMsg);
        }
        
        return ret;
    }

    /* Indicates whether a request should be recovered, based on the request's properties and
     * the received StreamState. */
    boolean isRequestRecoverable(WlRequest wlRequest, int streamState)
    {
        return
            (
             // A request is recoverable if:
             // - Request is not for a private stream
             !wlRequest.requestMsg().checkPrivateStream()

             // - And not a dictionary request, or is a dictionary request but didn't get full dictionary yet,
             && (wlRequest.requestMsg().domainType() != DomainTypes.DICTIONARY || wlRequest.state() != WlRequest.State.OPEN)

             // - and the StreamState is CLOSED_RECOVER and SingleOpen is enabled
             && loginHandler().supportSingleOpen() && streamState == StreamStates.CLOSED_RECOVER
            );
    }
    
    void closeWlRequest(WlRequest wlRequest)
    {
    	assert(wlRequest.state() != State.RETURN_TO_POOL);
        _tempWlInteger.value(wlRequest.requestMsg().streamId());
        WlRequest removedRequest = _streamIdtoWlRequestTable.remove(_tempWlInteger);
        assert (removedRequest == wlRequest); // There should a (non-null) WlRequest in the table, and it should be this same request.
    }
    
    /* Close the watchlist. */
    public void close()
    {
        clear();
        returnToPool();
    }

    /* Change service name to service id. */
    int changeServiceNameToID(MsgKey msgKey, String serviceName, ReactorErrorInfo errorInfo)
    {
        if (msgKey.checkHasServiceId())
        {
            return _reactor.populateErrorInfo(errorInfo,
                                              ReactorReturnCodes.INVALID_USAGE,
                                              "Watchlist.changeServiceNameToID",
                                              "Message submitted with both service name and service ID.");
        }
        
        int serviceId = _directoryHandler.serviceId(serviceName);
        if (serviceId < ReactorReturnCodes.SUCCESS)
        {
            return _reactor.populateErrorInfo(errorInfo,
                                              ReactorReturnCodes.INVALID_USAGE,
                                              "Watchlist.changeServiceNameToID",
                                              "Message submitted with unknown service name " + serviceName + ".");                    
        }
        else
        {
            msgKey.applyHasServiceId();
            msgKey.serviceId(serviceId);
        }

        return ReactorReturnCodes.SUCCESS;
    }
    
    /* Clear state of watchlist for re-use. */
    void clear()
    {
        _reactorChannel = null;
        _reactor = null;
        _role = null;
        _watchlistOptions = null;
        _nextStreamId = 0;
        _loginHandler.clear();
        _directoryHandler.clear();
        _itemHandler.clear();
        _eIter.clear();
        _dIter.clear();
        _tempMsg.clear();
        _tempWlInteger.clear();
        _streamTimeoutInfoList.clear();
        _streamIdtoWlRequestTable.clear();
        _streamIdtoWlStreamTable.clear();
    }
}
