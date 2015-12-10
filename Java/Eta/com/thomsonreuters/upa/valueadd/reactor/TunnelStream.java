package com.thomsonreuters.upa.valueadd.reactor;

import java.util.HashMap;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataTypes;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.GenericMsg;
import com.thomsonreuters.upa.codec.Msg;
import com.thomsonreuters.upa.codec.StreamStates;
import com.thomsonreuters.upa.codec.DataStates;
import com.thomsonreuters.upa.codec.StateCodes;
import com.thomsonreuters.upa.rdm.ClassesOfService;
import com.thomsonreuters.upa.rdm.DomainTypes;
import com.thomsonreuters.upa.transport.TransportBuffer;
import com.thomsonreuters.upa.tunnelstream.TunnelStreamHandler;
import com.thomsonreuters.upa.tunnelstream.TunnelBuffer;
import com.thomsonreuters.upa.tunnelstream.TunnelStreamListener;
import com.thomsonreuters.upa.tunnelstream.TunnelStreamTraceFlags;
import com.thomsonreuters.upa.tunnelstream.TunnelWriteBuffer;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.MsgBase;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginRequest;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.queue.QueueAck;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.queue.QueueData;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.queue.QueueDataExpired;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.queue.QueueDataFlags;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.queue.QueueMsg;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.queue.QueueMsgType;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.queue.QueueRequest;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.queue.QueueMsgFactory;
import com.thomsonreuters.upa.valueadd.reactor.ReactorChannel.State;

/**
 * Class to operate upon a TunnelStream. A tunnel stream is a private stream that supports
 * interactions with guaranteed messaging and other auxiliary services.
 *
 */
public class TunnelStream implements TunnelStreamListener
{
    ReactorChannel _reactorChannel;
    Reactor _reactor;
    TunnelStreamHandler _tunnelStreamHandler;
    QueueData _queueData;
    QueueDataExpired _queueDataExpired;
    QueueAck _queueAck;
    ReactorErrorInfo _errorInfo;
    HashMap<Integer,String> _streamIdtoQueueNameTable;
    Buffer _destName;
    Buffer _dataBuffer;
    int _streamId;
    int _domainType;
    int _serviceId;
    ClassOfService _classofService;
    // temporary GenericMsg and DecodeIterator for QueueData submit
    GenericMsg _genericMsg;
    DecodeIterator _dIter;
    EncodeIterator _eIter;
    Buffer _receiveBuffer;
	com.thomsonreuters.upa.codec.State _state;
    TunnelStreamDefaultMsgCallback _defaultMsgCallback;
    TunnelStreamStatusEventCallback _statusEventCallback;
    TunnelStreamQueueMsgCallback _queueMsgCallback;
    LoginRequest _authLoginRequest;
    String _name; // TunnelStream name
    int _guaranteedOutputBuffers;
	Object _userSpecObject;
	boolean _isProvider;
	
	// set to true for QueueMsg tracing
	boolean _enableQueueMsgTracing = false;
    
    public TunnelStream(ReactorChannel reactorChannel, TunnelStreamOpenOptions options)
    {
        this(reactorChannel);
        _streamId = options.streamId();
        _domainType = options.domainType();
        _serviceId = options.serviceId();
        _guaranteedOutputBuffers = options.guaranteedOutputBuffers();
        _classofService =  options.classOfService();
        _defaultMsgCallback = options.defaultMsgCallback();
        _statusEventCallback = options.statusEventCallback();
        _queueMsgCallback = options.queueMsgCallback();
        _authLoginRequest = options.authLoginRequest();
        if (_authLoginRequest == null && reactorChannel.role() != null)
        {
            _authLoginRequest = ((ConsumerRole)reactorChannel.role()).rdmLoginRequest();
        }
        _name = options.name();
        _userSpecObject = options.userSpecObject();
    }
    
    public TunnelStream(ReactorChannel reactorChannel, TunnelStreamRequestEvent event, TunnelStreamAcceptOptions options)
    {
        this(reactorChannel);
        _streamId = event.streamId();
        _domainType = event.domainType();
        _serviceId = event.serviceId();
        _guaranteedOutputBuffers = options.guaranteedOutputBuffers();
        _classofService = options.classOfService();
        _defaultMsgCallback = options.defaultMsgCallback();
        _statusEventCallback = options.statusEventCallback();
        _name = event.name();
        _userSpecObject = options.userSpecObject();
        _isProvider = true;
    }

    TunnelStream(ReactorChannel reactorChannel)
    {
        _reactorChannel = reactorChannel;
        _reactor = _reactorChannel.reactor();
        _queueData = QueueMsgFactory.createQueueData();
        _queueDataExpired = QueueMsgFactory.createQueueDataExpired();
        _queueAck = QueueMsgFactory.createQueueAck();
        _errorInfo = ReactorFactory.createReactorErrorInfo();
        _streamIdtoQueueNameTable = new HashMap<Integer,String>();
        _destName = CodecFactory.createBuffer();
        _dataBuffer = CodecFactory.createBuffer();
        _genericMsg = (GenericMsg)CodecFactory.createMsg();
        _dIter = CodecFactory.createDecodeIterator();
        _eIter = CodecFactory.createEncodeIterator();
        _receiveBuffer = CodecFactory.createBuffer();
        _state = CodecFactory.createState();
        _state.streamState(StreamStates.OPEN);
        _state.dataState(DataStates.SUSPECT);
        _state.code(StateCodes.NONE);
        _tunnelStreamHandler = reactorChannel.tunnelManager().createTunnelStream();
        _tunnelStreamHandler.listener(this);        
    }
    
    /**
     * Gets a buffer from the TunnelStream for writing a message.
     * 
     * @param size the size(in bytes) of the buffer to get
     * @param errorInfo error structure to be populated in the event of failure
     * 
     * @return the buffer for writing the message or
     *         null, if an error occurred (errorInfo will be populated with information)
     */
    public TransportBuffer getBuffer(int size, ReactorErrorInfo errorInfo)
    {
        TransportBuffer buffer = null;
        
        _reactor._reactorLock.lock();
        
        try
        {
            if (_tunnelStreamHandler != null)
            {
                return _tunnelStreamHandler.getBuffer(size, errorInfo.error());
            }
            else
            {
                _reactor.populateErrorInfo(errorInfo,
                                         ReactorReturnCodes.FAILURE,
                                         "TunnelStream.getBuffer",
                                         "No tunnel stream open for stream id " + _streamId);
            }
        }
        finally
        {
            _reactor._reactorLock.unlock();          
        }        

        return buffer;
    }
    
    /**
     * Returns an unwritten buffer to the TunnelStream.
     *
     * @param buffer the buffer to release
     * @param errorInfo error structure to be populated in the event of failure
     *
     * @return {@link ReactorReturnCodes} indicating success or failure
     */
    public int releaseBuffer(TransportBuffer buffer, ReactorErrorInfo errorInfo)
    {
        _reactor._reactorLock.lock();
        
        try
        {
            if (_tunnelStreamHandler != null)
            {
                return _tunnelStreamHandler.releaseBuffer((TunnelWriteBuffer)buffer, errorInfo.error());
            }
            else
            {
                _reactor.populateErrorInfo(errorInfo,
                                         ReactorReturnCodes.FAILURE,
                                         "TunnelStream.releaseBuffer",
                                         "No tunnel stream open for stream id " + _streamId);
            }
        }
        finally
        {
            _reactor._reactorLock.unlock();          
        }
        
        return ReactorReturnCodes.SUCCESS;
    }
    
    /**
     * Sends a buffer to the tunnel stream.
     * 
     * @param buffer the buffer to send
     * @param errorInfo error structure to be populated in the event of failure
     * 
     * @return {@link ReactorReturnCodes#SUCCESS}, if submit succeeded or
     * {@link ReactorReturnCodes#PERSISTENCE_FULL}, if the persistence file is full or
     * {@link ReactorReturnCodes#INVALID_ENCODING}, if the buffer encoding is invalid or
     * {@link ReactorReturnCodes#FAILURE}, if submit failed (refer to errorInfo for additional information)
     */
    public int submit(TransportBuffer buffer, TunnelStreamSubmitOptions options, ReactorErrorInfo errorInfo)
    {
        if (errorInfo == null)
            return ReactorReturnCodes.FAILURE;
        else if (buffer == null)
            return _reactor.populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                                               "TunnelStream.submit",
                                               "buffer cannot be null.");
        else if (options == null)
            return _reactor.populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                                               "TunnelStream.submit",
                                               "options cannot be null.");
        else if (_reactor.isShutdown())
            return _reactor.populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                                              "TunnelStream.submit",
                                              "Reactor is shutdown, submit aborted.");

        return handleBufferSubmit(_reactorChannel, buffer, options.containerType(), errorInfo);     
    }

    /**
     * Sends an RDM message to the tunnel stream.
     * 
     * @param rdmMsg the RDM message to send
     * @param errorInfo error structure to be populated in the event of failure
     * 
     * @return {@link ReactorReturnCodes#SUCCESS}, if submit succeeded or
     * {@link ReactorReturnCodes#PERSISTENCE_FULL}, if the persistence file is full or
     * {@link ReactorReturnCodes#FAILURE}, if submit failed (refer to errorInfo for additional information)
     */
    public int submit(MsgBase rdmMsg, ReactorErrorInfo errorInfo)
    {    
        if (errorInfo == null)
            return ReactorReturnCodes.FAILURE;
        else if (rdmMsg == null)
            return _reactor.populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                                               "TunnelStream.submit",
                                               "rdmMsg cannot be null.");
        else if (_reactor.isShutdown())
            return _reactor.populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                                              "TunnelStream.submit",
                                              "Reactor is shutdown, submit aborted.");

        if (_classofService.guarantee().type() != ClassesOfService.GuaranteeTypes.PERSISTENT_QUEUE ||
            rdmMsg.domainType() == DomainTypes.LOGIN ||
            rdmMsg.domainType() == DomainTypes.SOURCE ||
            rdmMsg.domainType() == DomainTypes.DICTIONARY ||
            rdmMsg.domainType() == DomainTypes.SYMBOL_LIST)
        {
            return handleRDMSubmit(_reactorChannel, rdmMsg, errorInfo);
        }
        else
        {
            return handleQueueMsgRDMSubmit(_reactorChannel, (QueueMsg)rdmMsg, errorInfo);
        }
    }
    
    private int handleRDMSubmit(ReactorChannel reactorChannel, MsgBase rdmMsg, ReactorErrorInfo errorInfo)
    {
        int ret = ReactorReturnCodes.SUCCESS;
        
        _reactor._reactorLock.lock();
        
        try
        {
            if (reactorChannel.state() == ReactorChannel.State.CLOSED)
            {
                ret = ReactorReturnCodes.FAILURE;
                return _reactor.populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                                         "TunnelStream.submit", "ReactorChannel is closed, aborting.");
            }
            
            int bufLength = _reactor.getMaxFragmentSize(reactorChannel, errorInfo);
            TransportBuffer buffer = getBuffer(bufLength, errorInfo);
            
            if (buffer != null)
            {
                _eIter.clear();
                _eIter.setBufferAndRWFVersion(buffer, _reactorChannel.majorVersion(), _reactorChannel.minorVersion());
                if ((ret = rdmMsg.encode(_eIter)) != CodecReturnCodes.SUCCESS)
                {
                    releaseBuffer(buffer, errorInfo);
                    return _reactor.populateErrorInfo(errorInfo,
                                                      ret,
                                                      "TunnelStream.submit",
                                                      "Unable to encode RDM Msg");
                }
                
                if ((ret = handleBufferSubmit(_reactorChannel,
                                              (TunnelWriteBuffer)buffer,
                                              DataTypes.MSG,
                                              errorInfo)) < ReactorReturnCodes.SUCCESS)
                {
                    releaseBuffer(buffer, errorInfo);
                    return _reactor.populateErrorInfo(errorInfo,
                                                      ret,
                                                      "TunnelStream.submit",
                                                      "TunnelStreamHandler.submit() failed");
                }
            }
            else // cannot get buffer
            {
                // send FLUSH event to worker
                if (!_reactor.sendWorkerEvent(WorkerEventTypes.FLUSH, reactorChannel))
                {
                    // _reactor.sendWorkerEvent() failed, send channel down
                    _reactor.sendWorkerEvent(WorkerEventTypes.CHANNEL_DOWN, reactorChannel);
                    reactorChannel.state(State.DOWN);
                    _reactor.sendAndHandleChannelEventCallback("TunnelStream.submit",
                                                          ReactorChannelEventTypes.CHANNEL_DOWN,
                                                          reactorChannel, errorInfo);
                    return _reactor.populateErrorInfo(errorInfo,
                                      ReactorReturnCodes.FAILURE,
                                      "TunnelStream.submit",
                                      "_reactor.sendWorkerEvent() failed");
                }
                
                _reactor.populateErrorInfo(errorInfo, ReactorReturnCodes.NO_BUFFERS,
                        "TunnelStream.submit", "channel out of buffers chnl="
                                + reactorChannel.channel().selectableChannel() + " errorId="
                                + errorInfo.error().errorId() + " errorText="
                                + errorInfo.error().text());
                
                return ReactorReturnCodes.NO_BUFFERS;
            }
        }
        finally
        {
            _reactor._reactorLock.unlock();          
        }
        
        return ReactorReturnCodes.SUCCESS;
    }
    
    private int handleQueueMsgRDMSubmit(ReactorChannel reactorChannel, QueueMsg queueMsg, ReactorErrorInfo errorInfo)
    {
        int ret = ReactorReturnCodes.SUCCESS;
        
        _reactor._reactorLock.lock();
        
        try
        {
            if (reactorChannel.state() == ReactorChannel.State.CLOSED)
            {
                ret = ReactorReturnCodes.FAILURE;
                return _reactor.populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                                         "TunnelStream.submit", "ReactorChannel is closed, aborting.");
            }
            
            switch (queueMsg.rdmMsgType())
            {
                case REQUEST:
                {
                    return openQueueMsgStream(reactorChannel, (QueueRequest)queueMsg, _serviceId, errorInfo);
                }
                case CLOSE:
                case REFRESH:
                case STATUS:
                case ACK:
                case DATA:
                {
                    int bufLength = (queueMsg.rdmMsgType() == QueueMsgType.DATA ? ((QueueData)queueMsg).encodedDataBody().length() + queueDataHdrBufSize((QueueData)queueMsg) : 128);
                    if (bufLength > _classofService.common().maxMsgSize())
                    {
                        bufLength = _classofService.common().maxMsgSize();
                    }
                    TransportBuffer buffer = getBuffer(bufLength, errorInfo);
                    
                    if (buffer != null)
                    {
                        _eIter.clear();
                        _eIter.setBufferAndRWFVersion(buffer, _reactorChannel.majorVersion(), _reactorChannel.minorVersion());
                        if ((ret = queueMsg.encode(_eIter)) != CodecReturnCodes.SUCCESS)
                        {
                            releaseBuffer(buffer, errorInfo);
                            return _reactor.populateErrorInfo(errorInfo,
                                                              ret,
                                                              "TunnelStream.submit",
                                                              "Unable to encode QueueMsg");
                        }
                        
                        if ((ret = handleBufferSubmit(_reactorChannel,
                                                      (TunnelWriteBuffer)buffer,
                                                      DataTypes.MSG,
                                                      errorInfo)) < ReactorReturnCodes.SUCCESS)
                        {
                            releaseBuffer(buffer, errorInfo);
                            return _reactor.populateErrorInfo(errorInfo,
                                                              ret,
                                                              "TunnelStream.submit",
                                                              "TunnelStreamHandler.submit() failed");
                        }
                    }
                    else // cannot get buffer
                    {
                        // send FLUSH event to worker
                        if (!_reactor.sendWorkerEvent(WorkerEventTypes.FLUSH, reactorChannel))
                        {
                            // _reactor.sendWorkerEvent() failed, send channel down
                            _reactor.sendWorkerEvent(WorkerEventTypes.CHANNEL_DOWN, reactorChannel);
                            reactorChannel.state(State.DOWN);
                            _reactor.sendAndHandleChannelEventCallback("TunnelStream.submit",
                                                                  ReactorChannelEventTypes.CHANNEL_DOWN,
                                                                  reactorChannel, errorInfo);
                            return _reactor.populateErrorInfo(errorInfo,
                                              ReactorReturnCodes.FAILURE,
                                              "TunnelStream.submit",
                                              "_reactor.sendWorkerEvent() failed");
                        }
                        
                        _reactor.populateErrorInfo(errorInfo, ReactorReturnCodes.NO_BUFFERS,
                                "TunnelStream.submit", "channel out of buffers chnl="
                                        + reactorChannel.channel().selectableChannel() + " errorId="
                                        + errorInfo.error().errorId() + " errorText="
                                        + errorInfo.error().text());
                        
                        return ReactorReturnCodes.NO_BUFFERS;
                    }
                    break;
                }
                default:
                {
                    return _reactor.populateErrorInfo(errorInfo,
                                                      ReactorReturnCodes.FAILURE,
                                                      "TunnelStream.submit",
                                                      "Unsupported QueueMsgType");                                    
                }
            }
        }
        finally
        {
            _reactor._reactorLock.unlock();          
        }
        
        return ReactorReturnCodes.SUCCESS;
    }
    
    int queueDataHdrBufSize(QueueData queueMsg)
    {
        return 128 + queueMsg.sourceName().length() + queueMsg.destName().length();
    }

    /**
     * Sends a message to the tunnel stream.
     * 
     * @param msg the message to send
     * @param errorInfo error structure to be populated in the event of failure
     * 
     * @return {@link ReactorReturnCodes#SUCCESS}, if submit succeeded or
     * {@link ReactorReturnCodes#FAILURE}, if submit failed (refer to errorInfo for additional information)
     */
    public int submit(Msg msg, ReactorErrorInfo errorInfo)
    {    
        if (errorInfo == null)
            return ReactorReturnCodes.FAILURE;
        else if (msg == null)
            return _reactor.populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                                               "TunnelStream.submit",
                                               "msg cannot be null.");
        else if (_reactor.isShutdown())
            return _reactor.populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                                              "TunnelStream.submit",
                                              "Reactor is shutdown, submit aborted.");
        
        return handleMsgSubmit(_reactorChannel, msg, errorInfo);
    }

    private int handleMsgSubmit(ReactorChannel reactorChannel, Msg msg, ReactorErrorInfo errorInfo)
    {
        int ret = ReactorReturnCodes.SUCCESS;
        
        _reactor._reactorLock.lock();
        
        try
        {
            if (reactorChannel.state() == ReactorChannel.State.CLOSED)
            {
                ret = ReactorReturnCodes.FAILURE;
                return _reactor.populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                                         "TunnelStream.submit", "ReactorChannel is closed, aborting.");
            }
            
            int bufLength = _reactor.getMaxFragmentSize(reactorChannel, errorInfo);
            TransportBuffer buffer = getBuffer(bufLength, errorInfo);
            
            if (buffer != null)
            {
                _eIter.clear();
                _eIter.setBufferAndRWFVersion(buffer, _reactorChannel.majorVersion(), _reactorChannel.minorVersion());
                if ((ret = msg.encode(_eIter)) != CodecReturnCodes.SUCCESS)
                {
                    releaseBuffer(buffer, errorInfo);
                    return _reactor.populateErrorInfo(errorInfo,
                                                      ret,
                                                      "TunnelStream.submit",
                                                      "Unable to encode Msg");
                }
                
                if ((ret = handleBufferSubmit(_reactorChannel,
                                              (TunnelWriteBuffer)buffer,
                                              DataTypes.MSG,
                                              errorInfo)) < ReactorReturnCodes.SUCCESS)
                {
                    releaseBuffer(buffer, errorInfo);
                    return _reactor.populateErrorInfo(errorInfo,
                                                      ret,
                                                      "TunnelStream.submit",
                                                      "TunnelStreamHandler.submit() failed");
                }
            }
            else // cannot get buffer
            {
                // send FLUSH event to worker
                if (!_reactor.sendWorkerEvent(WorkerEventTypes.FLUSH, reactorChannel))
                {
                    // _reactor.sendWorkerEvent() failed, send channel down
                    _reactor.sendWorkerEvent(WorkerEventTypes.CHANNEL_DOWN, reactorChannel);
                    reactorChannel.state(State.DOWN);
                    _reactor.sendAndHandleChannelEventCallback("TunnelStream.submit",
                                                          ReactorChannelEventTypes.CHANNEL_DOWN,
                                                          reactorChannel, errorInfo);
                    return _reactor.populateErrorInfo(errorInfo,
                                      ReactorReturnCodes.FAILURE,
                                      "TunnelStream.submit",
                                      "_reactor.sendWorkerEvent() failed");
                }
                
                _reactor.populateErrorInfo(errorInfo, ReactorReturnCodes.NO_BUFFERS,
                        "TunnelStream.submit", "channel out of buffers chnl="
                                + reactorChannel.channel().selectableChannel() + " errorId="
                                + errorInfo.error().errorId() + " errorText="
                                + errorInfo.error().text());
                
                return ReactorReturnCodes.NO_BUFFERS;
            }
        }
        finally
        {
            _reactor._reactorLock.unlock();          
        }
        
        return ReactorReturnCodes.SUCCESS;
    }

    private int handleBufferSubmit(ReactorChannel reactorChannel, TransportBuffer buffer, int containerType, ReactorErrorInfo errorInfo)
    {
        int ret = ReactorReturnCodes.SUCCESS;
        
        _reactor._reactorLock.lock();
        
        try
        {
            if (reactorChannel.state() == ReactorChannel.State.CLOSED)
            {
                ret = ReactorReturnCodes.FAILURE;
                return _reactor.populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                                         "TunnelStream.submit", "ReactorChannel is closed, aborting.");
            }
            
            if (_tunnelStreamHandler == null)
            {
                return _reactor.populateErrorInfo(errorInfo,
                                         ReactorReturnCodes.FAILURE,
                                         "TunnelStream.submit",
                                         "No tunnel stream open for stream id " + _streamId);
            }
            
            // directly submit encoded buffer to TunnelStreamHandler
            if ((ret = _tunnelStreamHandler.submit((TunnelWriteBuffer)buffer,
                                                   containerType,
                                                   errorInfo.error())) < ReactorReturnCodes.SUCCESS)
            {
                System.out.println("TunnelStreamHandler.submit() failed : " + CodecReturnCodes.toString(ret) + "(" + errorInfo.error().text() + ")");
                _tunnelStreamHandler.releaseBuffer((TunnelWriteBuffer)buffer, errorInfo.error());
                return ret;
            }
            
            // send a WorkerEvent to the Worker to immediately expire a timer
            if (reactorChannel.tunnelManager().needsDispatchNow())
            {
                if (!_reactor.sendDispatchNowEvent(reactorChannel))
                {
                    // _reactor.sendDispatchNowEvent() failed, send channel down
                    _reactor.sendWorkerEvent(WorkerEventTypes.CHANNEL_DOWN, reactorChannel);
                    reactorChannel.state(State.DOWN);
                    _reactor.sendAndHandleChannelEventCallback("TunnelStream.submit",
                                                          ReactorChannelEventTypes.CHANNEL_DOWN,
                                                          reactorChannel, errorInfo);
                    return _reactor.populateErrorInfo(errorInfo,
                                      ReactorReturnCodes.FAILURE,
                                      "TunnelStream.submit",
                                      "_reactor.sendDispatchNowEvent() failed");
                }
            }
            if (reactorChannel.tunnelManager().hasNextDispatchTime())
            {
                if (!_reactor.sendWorkerEvent(WorkerEventTypes.START_DISPATCH_TIMER, reactorChannel, reactorChannel.tunnelManager().nextDispatchTime()))
                {
                    // _reactor.sendWorkerEvent() failed, send channel down
                    _reactor.sendWorkerEvent(WorkerEventTypes.CHANNEL_DOWN, reactorChannel);
                    reactorChannel.state(State.DOWN);
                    _reactor.sendAndHandleChannelEventCallback("TunnelStream.dispatchChannel",
                                                          ReactorChannelEventTypes.CHANNEL_DOWN,
                                                          reactorChannel, errorInfo);
                    return _reactor.populateErrorInfo(errorInfo,
                                      ReactorReturnCodes.FAILURE,
                                      "TunnelStream.submit",
                                      "_reactor.sendWorkerEvent() failed");
                }
            }
        }
        finally
        {
            _reactor._reactorLock.unlock();          
        }        
        
        return ReactorReturnCodes.SUCCESS;            
    }

    /**
     * Closes the tunnel stream.
     * 
     * The finalStatusEvent argument indicates that the application wishes to receive a final
     * {@link TunnelStreamStatusEvent} when the closing of the tunnel stream to the remote end
     * is complete. If this is set to true, the tunnel stream will be cleaned up once the final
     * {@link TunnelStreamStatusEvent} event is provided to the application. This is indicated
     * by a non-open state.streamState on the event such as {@link StreamStates#CLOSED}
     * or {@link StreamStates#CLOSED_RECOVER}.
     * 
     * @param finalStatusEvent if true, a final {@link TunnelStreamStatusEvent} is provided to the
     *        application when the closing of the tunnel stream to the remote end is complete
     * @param errorInfo error structure to be populated in the event of failure
     * 
     * @return {@link ReactorReturnCodes} indicating success or failure
     */
    public int close(boolean finalStatusEvent, ReactorErrorInfo errorInfo)
    {   	
        _state.streamState(StreamStates.OPEN);
        _state.dataState(DataStates.SUSPECT);
        _state.code(StateCodes.NONE);

        // find and remove TunnelStream from table for this streamId
        if (_reactorChannel.streamIdtoTunnelStreamTable().containsKey(_streamId))
        {
            // close TunnelStreamHandler for this streamId
            _tunnelStreamHandler.close(finalStatusEvent, errorInfo.error());
 
            if (_reactorChannel.tunnelManager().needsDispatchNow())
            {
                if (!_reactor.sendDispatchNowEvent(_reactorChannel))
                {
                    // _reactor.sendDispatchNowEvent() failed, send channel down
                    _reactor.sendWorkerEvent(WorkerEventTypes.CHANNEL_DOWN, _reactorChannel);
                    _reactorChannel.state(State.DOWN);
                    return ReactorReturnCodes.SUCCESS;
                }
            }
        }
        
        return ReactorReturnCodes.SUCCESS;
    }

    int openQueueMsgStream(ReactorChannel reactorChannel, QueueRequest queueRequest, int serviceId, ReactorErrorInfo errorInfo)
    {
        int retval = ReactorReturnCodes.SUCCESS;

        if ((retval = _tunnelStreamHandler.openSubstream(queueRequest, errorInfo.error())) < ReactorReturnCodes.SUCCESS)
        {
            return _reactor.populateErrorInfo(errorInfo,
                                              retval,
                                              "TunnelStream.openQueueMsgStream",
                                              "tunnelStreamHandler.openSubstream() failed <" + errorInfo.error().text() + ">");
        }
        _streamIdtoQueueNameTable.put(queueRequest.streamId(), queueRequest.sourceName().toString());
        if (_enableQueueMsgTracing)
        {
            _tunnelStreamHandler.enableTrace(TunnelStreamTraceFlags.ACTIONS
                                 | TunnelStreamTraceFlags.MSGS
                                 | TunnelStreamTraceFlags.MSG_XML);
        }
        // send a WorkerEvent to the Worker to immediately expire a timer
        if (reactorChannel.tunnelManager().needsDispatchNow())
        {
            if (!_reactor.sendDispatchNowEvent(reactorChannel))
            {
                // _reactor.sendDispatchNowEvent() failed, send channel down
                _reactor.sendWorkerEvent(WorkerEventTypes.CHANNEL_DOWN, reactorChannel);
                reactorChannel.state(State.DOWN);
                _reactor.sendAndHandleChannelEventCallback("TunnelStream.openQueueMsgStream",
                                                      ReactorChannelEventTypes.CHANNEL_DOWN,
                                                      reactorChannel, errorInfo);
                return _reactor.populateErrorInfo(errorInfo,
                                  ReactorReturnCodes.FAILURE,
                                  "TunnelStream.openQueueMsgStream",
                                  "_reactor.sendDispatchNowEvent() failed");
            }
        }
        
        return ReactorReturnCodes.SUCCESS;
    }

    @Override
    public int queueMsgReceived(TunnelStreamHandler tunnelStreamHandler, TunnelBuffer buffer, Msg msg, int streamId)
    {
        int retval = ReactorCallbackReturnCodes.SUCCESS;
        String sourceName = _streamIdtoQueueNameTable.get(streamId);
        int flags = 0;
        
        if (buffer.isDuplicate())
        {
            flags |= QueueDataFlags.POSSIBLE_DUPLICATE;
        }
        
        if (!buffer.checkIsDeadLetter())
        {  
        	_queueData.clear();
            _queueData.sourceName().data(buffer.fromName().toString());
            _queueData.destName().data(sourceName);
            _queueData.streamId(streamId);
            _queueData.domainType(msg.domainType());
            _queueData.identifier(buffer.identifier());
            _queueData.containerType(buffer.containerType());
            _queueData.flags(flags);
            _queueData.queueDepth(buffer.queueDepth());
            _receiveBuffer.data(buffer.data(), buffer.position(), buffer.length());
            _queueData.encodedDataBody(_receiveBuffer);
            retval = _reactor.sendAndHandleQueueMsgCallback("TunnelStream.queueMsgReceived", _reactorChannel, this, buffer, msg, _queueData, _errorInfo);
        }
        else
        {
        	_queueDataExpired.clear();
            _queueDataExpired.undeliverableCode(buffer.expirationCode());
            _queueDataExpired.sourceName().data(sourceName);
            _queueDataExpired.destName().data(buffer.toName().toString());
            _queueDataExpired.streamId(streamId);
            _queueDataExpired.domainType(msg.domainType());
            _queueDataExpired.identifier(buffer.identifier());
            _queueDataExpired.containerType(buffer.containerType());
            _queueDataExpired.flags(flags);
            _queueDataExpired.queueDepth(buffer.queueDepth());
            _receiveBuffer.data(buffer.data(), buffer.position(), buffer.length());
            _queueDataExpired.encodedDataBody(_receiveBuffer);
            retval = _reactor.sendAndHandleQueueMsgCallback("TunnelStream.queueMsgReceived", _reactorChannel, this, buffer, msg, _queueDataExpired, _errorInfo);
        }
 
        if (retval == ReactorCallbackReturnCodes.RAISE)
            retval = _reactor.sendAndHandleTunnelStreamMsgCallback("TunnelStream.queueMsgReceived", _reactorChannel, this, buffer, msg, DataTypes.MSG, _errorInfo);
        
        return retval;
    }

    @Override
    public int queueMsgReceived(TunnelStreamHandler tunnelStreamHandler, QueueMsg queueMsg, Msg msg)
    {
        int retval = ReactorCallbackReturnCodes.SUCCESS;
        
        retval = _reactor.sendAndHandleQueueMsgCallback("TunnelStream.queueMsgReceived", _reactorChannel, this, null, msg, queueMsg, _errorInfo);

        if (retval == ReactorCallbackReturnCodes.RAISE)
            retval = _reactor.sendAndHandleTunnelStreamMsgCallback("TunnelStream.queueMsgReceived", _reactorChannel, this, null, msg, DataTypes.MSG, _errorInfo);
        
        return retval;
    }

    @Override
    public int queueMsgAcknowledged(TunnelStreamHandler tunnelStreamHandler, long identifier, Buffer fromName, Buffer toName, Msg msg, int streamId)
    {
        int retval = ReactorCallbackReturnCodes.SUCCESS;
        
        _queueAck.clear();
        _queueAck.identifier(identifier);
        _queueAck.streamId(streamId);
        _queueAck.domainType(msg.domainType());
        retval = _reactor.sendAndHandleQueueMsgCallback("TunnelStream.queueMsgAcknowledged", _reactorChannel, this, null, msg, _queueAck, _errorInfo);

        if (retval == ReactorCallbackReturnCodes.RAISE)
            retval = _reactor.sendAndHandleTunnelStreamMsgCallback("TunnelStream.queueMsgAcknowledged", _reactorChannel, this, null, msg, DataTypes.MSG, _errorInfo);
        
        return retval;
    }

    @Override
    public int queueMsgExpired(TunnelStreamHandler tunnelStreamHandler, TunnelBuffer buffer, Msg msg, int streamId)
    {
        int retval = ReactorCallbackReturnCodes.SUCCESS;
        
        String destName = buffer.toName().toString(); // use toName here since this is our own message
        String sourceName = _streamIdtoQueueNameTable.get(streamId);
        
        if (sourceName == null)
        {
            sourceName = buffer.fromName().toString();
        }

        _queueDataExpired.clear();
        _queueDataExpired.streamId(streamId);
        _queueDataExpired.identifier(buffer.identifier());
        _queueDataExpired.sourceName().data(sourceName);
        _queueDataExpired.destName().data(destName);
        _queueDataExpired.undeliverableCode(buffer.expirationCode());
        _dIter.clear();
        _dIter.setBufferAndRWFVersion(buffer, _reactorChannel.majorVersion(), _reactorChannel.minorVersion());
        // decode TunnelStream message
        _genericMsg.decode(_dIter);
        // decode substream message
        _genericMsg.decode(_dIter);
        _queueDataExpired.encodedDataBody(_genericMsg.encodedDataBody());
        _queueDataExpired.domainType(_genericMsg.domainType());
        _queueDataExpired.containerType(_genericMsg.containerType());
        retval = _reactor.sendAndHandleQueueMsgCallback("TunnelStream.queueMsgExpired", _reactorChannel, this, buffer, msg, _queueDataExpired, _errorInfo);

        if (retval == ReactorCallbackReturnCodes.RAISE)
            retval = _reactor.sendAndHandleTunnelStreamMsgCallback("TunnelStream.queueMsgExpired", _reactorChannel, this, buffer, msg, DataTypes.MSG, _errorInfo);
        
        return retval;
    }
    
    @Override
    public int msgReceived(TunnelStreamHandler tunnelStreamHandler, TunnelBuffer buffer, Msg msg, int containerType)
    {
        return _reactor.sendAndHandleTunnelStreamMsgCallback("TunnelStream.msgReceived", _reactorChannel, this, buffer, msg, containerType, _errorInfo);
    }

    /**
     * The stream id of the tunnel stream.
     * 
     * @return the stream id
     */
    public int streamId()
    {
        return _streamId;
    }
    
    /**
     * The domain type of the tunnel stream.
     * 
     * @return the domain type
     */
    public int domainType()
    {
        return _domainType;
    }
    
    /**
     * The service identifier of the tunnel stream.
     * 
     * @return the service id
     */
    public int serviceId()
    {
        return _serviceId;
    }

    /**
     * The class of service of the tunnel stream.
     * 
     * @return the ClassOfService
     * 
     * @see ClassOfService
     */
    public ClassOfService classOfService()
    {
        return _classofService;
    }
    
    /**
     * The number of guaranteed output buffers that will be available
     * for the tunnel stream.
     * 
     * @return the number of guaranteed output buffers
     */
    public int guaranteedOutputBuffers()
    {
        return _guaranteedOutputBuffers;
    }

    /**
     * The TunnelStreamStatusEventCallback of the TunnelStream. Handles stream events
     * for tunnel stream.
     * 
     * @return the tunnelStreamDefaultMsgCallback
     */
    public TunnelStreamStatusEventCallback statusEventCallback()
    {
        return _statusEventCallback;
    }

    /**
     * The TunnelStreamDefaultMsgCallback of the TunnelStream. Handles message events
     * for tunnel stream.
     * 
     * @return the tunnelStreamDefaultMsgCallback
     */
    public TunnelStreamDefaultMsgCallback defaultMsgCallback()
    {
        return _defaultMsgCallback;
    }
    
    /**
     * The QueueMsgCallback of the TunnelStream. Handles message events
     * for queue message streams.
     * 
     * @return the queueMsgCallback
     */
    public TunnelStreamQueueMsgCallback queueMsgCallback()
    {
        return _queueMsgCallback;
    }
    
    LoginRequest authLoginRequest()
    {
        return _authLoginRequest;
    }
    
    /**
     * The name of the TunnelStream.
     * 
     * @return the TunnelStream name
     */
    public String name()
    {
        return _name;
    }
    
    /**
     * A user specified object, possibly a closure. This information can be useful 
	 * for coupling this {@link TunnelStream} with other user created information, 
	 * such as a watch list associated with it.
     * @return the userSpecObject
     */
    public Object userSpecObject()
	{
		return _userSpecObject;
	}

    /**
     * The current known state of the tunnel stream, as indicated by the last
     * received response.
	 * @return the current state
     */
    public com.thomsonreuters.upa.codec.State state()
	{
		return _state;
	}
    
    /**
     * The internal handler associated with this tunnel stream.
     * 
     * @return the TunnelStreamHandler
     */
    public TunnelStreamHandler tunnelStreamHandler()
    {
        return _tunnelStreamHandler;
    }
    
    /**
     * The reactor channel associated with the tunnel stream.
     * 
     * @return the reactor channel
     */
    public ReactorChannel reactorChannel()
    {
        return _reactorChannel;
    }
    
    /**
     * Returns whether or not this is a provider tunnel stream.
     */
    public boolean isProvider()
    {
        return _isProvider;
    }
    
    /**
     * Returns whether or not XML tracing is enabled.
     */
    public boolean xmlTracing()
    {
        return _reactor._reactorOptions.xmlTracing();
    }
}
