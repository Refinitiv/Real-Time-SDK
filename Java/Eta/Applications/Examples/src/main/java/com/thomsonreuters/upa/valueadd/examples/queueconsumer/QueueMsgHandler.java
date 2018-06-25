package com.thomsonreuters.upa.valueadd.examples.queueconsumer;


import java.nio.ByteBuffer;
import java.util.List;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataStates;
import com.thomsonreuters.upa.codec.DataTypes;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.MsgClasses;
import com.thomsonreuters.upa.codec.State;
import com.thomsonreuters.upa.codec.StreamStates;
import com.thomsonreuters.upa.rdm.ClassesOfService;
import com.thomsonreuters.upa.rdm.DomainTypes;
import com.thomsonreuters.upa.transport.TransportBuffer;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.queue.QueueAck;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.queue.QueueData;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.queue.QueueDataExpired;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.queue.QueueDataTimeoutCode;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.queue.QueueDataUndeliverableCode;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.queue.QueueRefresh;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.queue.QueueRequest;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.queue.QueueStatus;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.queue.QueueMsgFactory;
import com.thomsonreuters.upa.valueadd.reactor.TunnelStreamDefaultMsgCallback;
import com.thomsonreuters.upa.valueadd.reactor.TunnelStreamMsgEvent;
import com.thomsonreuters.upa.valueadd.reactor.TunnelStreamQueueMsgCallback;
import com.thomsonreuters.upa.valueadd.reactor.TunnelStreamQueueMsgEvent;
import com.thomsonreuters.upa.valueadd.reactor.ReactorCallbackReturnCodes;
import com.thomsonreuters.upa.valueadd.reactor.ReactorChannel;
import com.thomsonreuters.upa.valueadd.reactor.ReactorErrorInfo;
import com.thomsonreuters.upa.valueadd.reactor.ReactorFactory;
import com.thomsonreuters.upa.valueadd.reactor.ReactorReturnCodes;
import com.thomsonreuters.upa.valueadd.reactor.TunnelStreamStatusEvent;
import com.thomsonreuters.upa.valueadd.reactor.TunnelStreamOpenOptions;
import com.thomsonreuters.upa.valueadd.reactor.TunnelStreamStatusEventCallback;
import com.thomsonreuters.upa.valueadd.reactor.TunnelStreamSubmitOptions;

/*
 * This is the queue message handler for the UPA Value Add queue consumer application.
 * It handles any queue message delivery. It only supports one ReactorChannel/TunnelStream.
 */
class QueueMsgHandler implements TunnelStreamStatusEventCallback, TunnelStreamDefaultMsgCallback, TunnelStreamQueueMsgCallback
{
    final static int TUNNEL_STREAM_STREAM_ID = 1000;
    final static int QUEUE_MSG_STREAM_ID = 2000;
    final static int QUEUE_MSG_DOMAIN = 199;
    
    private int _serviceId;
    
    private String _sourceName;
    private List<String> _destNameList;
    private long _nextSubmitMsgTime;
    private static final int QUEUE_MSG_FREQUENCY = 2;
    private EncodeIterator _msgEncIter;
    private ReactorErrorInfo _errorInfo;
    private long _identifier;
    private TunnelStreamOpenOptions _tunnelStreamOpenOptions;
    private QueueRequest _queueRequest;
    private QueueData _queueData;
    ChannelInfo _chnlInfo;    
    private byte[] testMsg = new String("Hello World!").getBytes();
    private TunnelStreamSubmitOptions _tunnelStreamSubmitOptions;
    Buffer _testBuffer;
	boolean _tunnelAuth;
	int _tunnelDomain;
 
    QueueMsgHandler(String sourceName, List<String> destNames, boolean tunnelAuth, int tunnelDomain)
    {
        _sourceName = sourceName;
        _destNameList = destNames;
        _nextSubmitMsgTime = System.currentTimeMillis() + QUEUE_MSG_FREQUENCY * 1000;
        _msgEncIter = CodecFactory.createEncodeIterator();
        _tunnelStreamOpenOptions = ReactorFactory.createTunnelStreamOpenOptions();
        _errorInfo = ReactorFactory.createReactorErrorInfo();
        _queueRequest = QueueMsgFactory.createQueueRequest();
        _queueData = QueueMsgFactory.createQueueData();
        _tunnelStreamSubmitOptions = ReactorFactory.createTunnelStreamSubmitOptions();
        _testBuffer = CodecFactory.createBuffer();
        _testBuffer.data(ByteBuffer.allocate(1024));
		_tunnelAuth = tunnelAuth;
		_tunnelDomain = tunnelDomain;
    }
    
    /*
     * Used by the Consumer to open a tunnel stream once the Consumer's channel
     * is opened and the desired service identified.
     */
    int openStream(ChannelInfo chnlInfo, ReactorErrorInfo errorInfo)
    {
		int ret;
        if (_sourceName == null || chnlInfo.tunnelStreamOpenSent == true)
            return ReactorReturnCodes.SUCCESS;

        if(!hasCapability(chnlInfo.serviceInfo.info().capabilitiesList(), _tunnelDomain))
        {
            System.out.println("Domain (" + DomainTypes.toString(_tunnelDomain) + ") is not supported by the indicated provider\n");
            return ReactorReturnCodes.FAILURE;
        }

        _serviceId = chnlInfo.serviceInfo.serviceId();
        
        _tunnelStreamOpenOptions.clear();
        _tunnelStreamOpenOptions.classOfService().dataIntegrity().type(ClassesOfService.DataIntegrityTypes.RELIABLE);
        _tunnelStreamOpenOptions.classOfService().flowControl().type(ClassesOfService.FlowControlTypes.BIDIRECTIONAL);
        _tunnelStreamOpenOptions.classOfService().guarantee().type(ClassesOfService.GuaranteeTypes.PERSISTENT_QUEUE);
        _tunnelStreamOpenOptions.streamId(TUNNEL_STREAM_STREAM_ID);
        _tunnelStreamOpenOptions.domainType(_tunnelDomain);
        _tunnelStreamOpenOptions.serviceId(_serviceId);
        _tunnelStreamOpenOptions.queueMsgCallback(this);
        _tunnelStreamOpenOptions.statusEventCallback(this);
        _tunnelStreamOpenOptions.defaultMsgCallback(this);

		if (_tunnelAuth)
			_tunnelStreamOpenOptions.classOfService().authentication().type(ClassesOfService.AuthenticationTypes.OMM_LOGIN);
        
        if ((ret = chnlInfo.reactorChannel.openTunnelStream(_tunnelStreamOpenOptions, errorInfo)) != ReactorReturnCodes.SUCCESS)
		{
			System.out.println("ReactorChannel.openTunnelStream() failed: " + CodecReturnCodes.toString(ret)
					+ "(" + errorInfo.error().text() + ")");
		}
        
        chnlInfo.tunnelStreamOpenSent = true;
        _chnlInfo = chnlInfo;
    
		return ReactorReturnCodes.SUCCESS;
    }

    /*
     * Used by the Consumer to close any tunnel streams it opened
     * for the reactor channel.
     */
    int closeStreams(ChannelInfo chnlInfo, ReactorErrorInfo errorInfo)
    {
        int ret;

        if (chnlInfo.tunnelStream == null)
            return ReactorReturnCodes.SUCCESS;

        if ((ret =  chnlInfo.tunnelStream.close(true, errorInfo)) < ReactorReturnCodes.SUCCESS)
        {
            System.out.println("TunnelStream.close() failed with return code: " + ret);
            return ReactorCallbackReturnCodes.SUCCESS;
        }

        chnlInfo.isQueueStreamUp = false;
        
        return ReactorReturnCodes.SUCCESS;
    }

 
    /* Processes a queue message event. */
    @Override
    public int queueMsgCallback(TunnelStreamQueueMsgEvent event)
    {
        switch (event.queueMsg().rdmMsgType())
        {
            case DATA:
            {
                QueueData queueData = (QueueData)event.queueMsg();
                
                switch (queueData.containerType())
                {
                    case  DataTypes.OPAQUE:                    	                    	
                    {
                        System.out.print("Received a msg on stream " + queueData.streamId() +
                                " from " +  queueData.sourceName() +
                                " to " + queueData.destName() +
                                ", ID: " + queueData.identifier());
                        
                        if (queueData.checkPossibleDuplicate())
                            System.out.println("  (Message may be a duplicate of a previous message.)");

                        System.out.println("  Queue Depth: " + queueData.queueDepth());
                        
                        System.out.println("\n   Message is: " + queueData.encodedDataBody().toString() + "\n");
                        break;
                    }
                    default:
                        break;
                }                
                break;
            }
            case DATAEXPIRED:
            {
                QueueDataExpired queueDataExpired = (QueueDataExpired)event.queueMsg();
                
                switch (queueDataExpired.containerType())
                {
                    case  DataTypes.OPAQUE:                    	                    	
                    {
                        System.out.print("Received a msg on stream " + queueDataExpired.streamId() +
                                " from " +  queueDataExpired.sourceName() +
                                " to " + queueDataExpired.destName() +
                                ", ID: " + queueDataExpired.identifier());
                        System.out.println(" (Undeliverable Message with code: " + QueueDataUndeliverableCode.toString(queueDataExpired.undeliverableCode()) + ")");
                        
                        if (queueDataExpired.checkPossibleDuplicate())
                            System.out.println("  (Message may be a duplicate of a previous message.)");

                        System.out.println("  Queue Depth: " + queueDataExpired.queueDepth());
                        
                        System.out.println("\n   Message is: " + queueDataExpired.encodedDataBody().toString() + "\n");
                        break;
                    }
                    default:
                        break;
                }                
                break;
            }            
            case ACK:
                QueueAck queueAck = (QueueAck)event.queueMsg();
                System.out.println("Received persistence Ack for submitted message with ID: " + queueAck.identifier() + "\n");
                break;
            case REFRESH:
                QueueRefresh queueRefresh = (QueueRefresh)event.queueMsg();
                System.out.println("Received QueueRefresh on stream " + queueRefresh.streamId() + " for sourceName " + queueRefresh.sourceName() + " with " + queueRefresh.state()
                                   + "\n" + "  Queue Depth: " + queueRefresh.queueDepth() + "\n");
                if (queueRefresh.state().streamState() == StreamStates.OPEN &&
                    queueRefresh.state().dataState() == DataStates.OK)
                {
                    _chnlInfo.isQueueStreamUp = true;
                }
                break;
            case STATUS:
                QueueStatus queueStatus = (QueueStatus)event.queueMsg();
                if (queueStatus.checkHasState())
                {
                    System.out.println("Received QueueStatus on stream " + queueStatus.streamId() + " with " + queueStatus.state() + "\n");
                    if (queueStatus.state().streamState() == StreamStates.CLOSED ||
                        queueStatus.state().streamState() == StreamStates.CLOSED_RECOVER)
                    {
                        _chnlInfo.isQueueStreamUp = false;
                    }
                }
                else
                {
                    System.out.println("Received QueueStatus on stream " + queueStatus.streamId() + "\n");
                }
                break;
            default:
                break;
        }
        
        return ReactorCallbackReturnCodes.SUCCESS;
    }
        
    void sendQueueMsg(ReactorChannel reactorChannel)
    {
        long currentTime = System.currentTimeMillis();
        int ret; 
        if (_destNameList == null || _destNameList.isEmpty())
            return;

        if (currentTime < _nextSubmitMsgTime)
        {
            return;
        }
        
        _nextSubmitMsgTime = currentTime + QUEUE_MSG_FREQUENCY * 1000;
        
        for (int i = 0; i < _destNameList.size(); ++i)
        {
			if (_chnlInfo == null || _chnlInfo.tunnelStream == null || !_chnlInfo.isQueueStreamUp)
				continue;

            // get buffer to encode QueueData message into
            TransportBuffer buffer = _chnlInfo.tunnelStream.getBuffer(768, _errorInfo);
            if (buffer == null)
            {
                System.out.println("sendQueueMsg failed: Unable to get a buffer from TunnelStream <" + _errorInfo.error().text() + ">");
                return;                        
            }
            
            // initialize the QueueData encoding
            _queueData.clear();
            _queueData.streamId(QUEUE_MSG_STREAM_ID);
            _queueData.domainType(QUEUE_MSG_DOMAIN);
            _queueData.identifier(++_identifier);
            _queueData.sourceName().data(_sourceName);
            _queueData.destName().data(_destNameList.get(i));
            _queueData.timeout(QueueDataTimeoutCode.INFINITE);
            _queueData.containerType(DataTypes.OPAQUE);

            _testBuffer.data().clear();
            _testBuffer.data().put(testMsg);
            _queueData.encodedDataBody(_testBuffer);
            
            _msgEncIter.clear();
            _msgEncIter.setBufferAndRWFVersion(buffer, _chnlInfo.tunnelStream.classOfService().common().protocolMajorVersion(),
					_chnlInfo.tunnelStream.classOfService().common().protocolMinorVersion());
            if ((ret = _queueData.encode(_msgEncIter)) < ReactorReturnCodes.SUCCESS)
            {
                System.out.println("QueueData.encode() failed: " + CodecReturnCodes.toString(ret)
                                   + "(" + _errorInfo.error().text() + ")");
                return;                            
            }
                 
            // submit the encoded data buffer of QueueData to the tunnel stream
            _tunnelStreamSubmitOptions.clear();
            _tunnelStreamSubmitOptions.containerType(DataTypes.MSG);
            if ((ret = _chnlInfo.tunnelStream.submit(buffer, _tunnelStreamSubmitOptions, _errorInfo)) < ReactorReturnCodes.SUCCESS)
            {
                System.out.println("TunnelStream.submit() failed: " + CodecReturnCodes.toString(ret)
                        + "(" + _errorInfo.error().text() + ")");
                return;
            }
            
            System.out.println("Submitted message with ID " + _identifier + " to " + _destNameList.get(i) + ".\n");
        }      
    }    
    
 
    private boolean hasCapability(List<Long> capabilities, int gmCapability)
    {
        for (Long capability : capabilities)
        {
            if (capability.equals((long)gmCapability))
                return true;
        }
        return false;
    }

    /* Process a tunnel stream status event for the QueueMsgHandler. */
    @Override
    public int statusEventCallback(TunnelStreamStatusEvent event)
    {
        State state = event.state();
		int ret;
        
        System.out.println("Received TunnelStreamStatusEvent for Stream ID " + event.tunnelStream().streamId() + " with " + state + "\n");
        
		switch(state.streamState())
		{
			case StreamStates.OPEN:

				if (state.dataState() == DataStates.OK)
				{
					// Stream is open and ready for use.

					// Add it to our ChannelInfo
					_chnlInfo.tunnelStream = event.tunnelStream();
            
                    // open a queue message sub-stream for this tunnel stream
                    _queueRequest.clear();
        			_queueRequest.streamId(QUEUE_MSG_STREAM_ID);
        			_queueRequest.domainType(QUEUE_MSG_DOMAIN);
        			_queueRequest.sourceName().data(_sourceName);
                    
					if ((ret = _chnlInfo.tunnelStream.submit(_queueRequest, _errorInfo)) < ReactorReturnCodes.SUCCESS)
                    {
						 System.out.println("Failed to submit queue request:"
								 + CodecReturnCodes.toString(ret) + "(" + _errorInfo.error().text() + ")");
                    }
				}
				break;

			case StreamStates.CLOSED_RECOVER:
			case StreamStates.CLOSED:
			default:
				// For other stream states such as Closed & ClosedRecover, close the tunnel stream. 
				if ((ret = event.tunnelStream().close(true, _errorInfo)) < ReactorReturnCodes.SUCCESS)
				{
					System.out.println("Failed to close TunnelStream:"
							+ ReactorReturnCodes.toString(ret) + "(" + _errorInfo.error().text() + ")");
				}
				
				// Remove our tunnel information if the tunnel was open.
	            _chnlInfo.tunnelStreamOpenSent = false;
			    _chnlInfo.tunnelStream = null;
                _chnlInfo.isQueueStreamUp = false;
				break;
        }

		return ReactorCallbackReturnCodes.SUCCESS;
        
    }

    @Override
    public int defaultMsgCallback(TunnelStreamMsgEvent event)
    {
        System.out.printf("Received unhandled message in TunnelStream with stream ID %d, class %d(%s) and domainType %d(%s)\n\n",
                          event.msg().streamId(),
                          event.msg().msgClass(), MsgClasses.toString(event.msg().msgClass()),
                          event.msg().domainType(), DomainTypes.toString(event.msg().domainType()));

        return ReactorCallbackReturnCodes.SUCCESS;
    }
}
