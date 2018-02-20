package com.thomsonreuters.upa.valueadd.examples.consumer;

import java.nio.ByteBuffer;
import java.util.List;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataDictionary;
import com.thomsonreuters.upa.codec.DataStates;
import com.thomsonreuters.upa.codec.DataTypes;
import com.thomsonreuters.upa.codec.DateTime;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.DictionaryEntry;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.FieldEntry;
import com.thomsonreuters.upa.codec.FieldList;
import com.thomsonreuters.upa.codec.Int;
import com.thomsonreuters.upa.codec.MsgClasses;
import com.thomsonreuters.upa.codec.Real;
import com.thomsonreuters.upa.codec.RealHints;
import com.thomsonreuters.upa.codec.State;
import com.thomsonreuters.upa.codec.StreamStates;
import com.thomsonreuters.upa.codec.UInt;
import com.thomsonreuters.upa.codec.Enum;
import com.thomsonreuters.upa.rdm.ClassesOfService;
import com.thomsonreuters.upa.rdm.DomainTypes;
import com.thomsonreuters.upa.transport.TransportBuffer;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.queue.QueueAck;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.queue.QueueData;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.queue.QueueDataExpired;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.queue.QueueDataUndeliverableCode;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.queue.QueueRefresh;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.queue.QueueRequest;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.queue.QueueStatus;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.queue.QueueDataTimeoutCode;
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

/**
 * This is the queue message handler for the UPA Value Add consumer application.
 * It handles any queue message delivery. It only supports one ReactorChannel/TunnelStream.
 */
public class QueueMsgHandler implements TunnelStreamStatusEventCallback, TunnelStreamDefaultMsgCallback, TunnelStreamQueueMsgCallback
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
    private DecodeIterator _msgDecIter;
    private Int _msgInt;
    private UInt _msgUInt;
    private Real _msgReal; 
    private Buffer _msgStr;
    private DateTime _msgDateTime;
    private Enum _msgEnum;
    private ReactorErrorInfo _errorInfo;
    private long _identifier;
    private TunnelStreamOpenOptions _tunnelStreamOpenOptions;
    private QueueRequest _queueRequest;
    private QueueData _queueData;
    ChannelInfo _chnlInfo;
    private DataDictionary _dictionary;
    private FieldList _fieldList;
    private FieldEntry _fieldEntry;
    private Buffer _ackBuffer;
    private Buffer _payloadBuffer;
    private TunnelStreamSubmitOptions _tunnelStreamSubmitOptions;
    private boolean _finalStatusEvent;
	private boolean _tunnelAuth;
	private int _tunnelDomain;
    
    public QueueMsgHandler(String sourceName, List<String> destNames, DataDictionary dictionary, boolean tunnelAuth, int tunnelDomain)
    {
        _sourceName = sourceName;
        _destNameList = destNames;
        _dictionary = dictionary;
        _nextSubmitMsgTime = System.currentTimeMillis() + QUEUE_MSG_FREQUENCY * 1000;
        _msgEncIter = CodecFactory.createEncodeIterator();
        _msgDecIter = CodecFactory.createDecodeIterator();
        _msgInt = CodecFactory.createInt();
        _msgUInt = CodecFactory.createUInt();
        _msgEnum = CodecFactory.createEnum();
        _msgReal = CodecFactory.createReal();
        _msgStr = CodecFactory.createBuffer();
        _msgDateTime = CodecFactory.createDateTime();
        _tunnelStreamOpenOptions = ReactorFactory.createTunnelStreamOpenOptions();
        _errorInfo = ReactorFactory.createReactorErrorInfo();
        _queueRequest = QueueMsgFactory.createQueueRequest();
        _queueData = QueueMsgFactory.createQueueData();
        _fieldList = CodecFactory.createFieldList();
        _fieldEntry = CodecFactory.createFieldEntry();
        _ackBuffer = CodecFactory.createBuffer();
        _ackBuffer.data(ByteBuffer.allocate(1024));
        _payloadBuffer = CodecFactory.createBuffer();
        _payloadBuffer.data(ByteBuffer.allocate(1024));
        _tunnelStreamSubmitOptions = ReactorFactory.createTunnelStreamSubmitOptions();
        _finalStatusEvent = true;
		_tunnelAuth = tunnelAuth;
		_tunnelDomain = tunnelDomain;
    }
    
    /*
     * Used by the Consumer to open a tunnel stream once the Consumer's channel
     * is opened and the desired service identified.
     */
    public int openStream(ChannelInfo chnlInfo, ReactorErrorInfo errorInfo)
    {
        int ret;
        if (_sourceName == null || chnlInfo.tunnelStreamOpenSent == true)
            return ReactorReturnCodes.SUCCESS;

        if(!hasCapability(chnlInfo.qServiceInfo.info().capabilitiesList(), _tunnelDomain))
        {
            System.out.println("Domain (" + DomainTypes.toString(_tunnelDomain) + ") is not supported by the indicated provider\n");
            return ReactorReturnCodes.FAILURE;
        }

        _serviceId = chnlInfo.qServiceInfo.serviceId();
        
        _tunnelStreamOpenOptions.clear();
        _tunnelStreamOpenOptions.name("QueueMsgTunnelStream");
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
    public int closeStreams(ChannelInfo chnlInfo, boolean finalStatusEvent, ReactorErrorInfo errorInfo)
    {
        int ret;
        _finalStatusEvent = finalStatusEvent;

        if (chnlInfo.tunnelStream == null)
            return ReactorReturnCodes.SUCCESS;
        
        if ((ret =  chnlInfo.tunnelStream.close(finalStatusEvent, _errorInfo)) < ReactorReturnCodes.SUCCESS)
        {
            System.out.println("TunnelStream.close() failed with return code: " + ret);
            return ReactorCallbackReturnCodes.SUCCESS;
        }
        
        return ReactorReturnCodes.SUCCESS;
    }


    /*
     * Encode and send a queue FIX message through the ReactorChannel.
     * This message will contain a field list as its payload.
     */
    public void sendQueueMsg(ReactorChannel reactorChannel)
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
            _queueData.containerType(DataTypes.FIELD_LIST);
            
            _msgEncIter.clear();
            _msgEncIter.setBufferAndRWFVersion(buffer, _chnlInfo.tunnelStream.classOfService().common().protocolMajorVersion(),
					_chnlInfo.tunnelStream.classOfService().common().protocolMinorVersion());
            if ((ret = _queueData.encodeInit(_msgEncIter)) < ReactorReturnCodes.SUCCESS)
            {
                System.out.println("QueueData.encodeInit() failed: " + CodecReturnCodes.toString(ret)
                                   + "(" + _errorInfo.error().text() + ")");
                return;                            
            }
            
            //// Start Content Encoding ////
            if (createNewOrder(_msgEncIter) < ReactorReturnCodes.SUCCESS)
            {
                System.out.println("Failed to encode FIX order");
                return;
            }
            //// End Content Encoding ////

            // complete the QueueData encoding
            if ((ret = _queueData.encodeComplete(_msgEncIter, true)) < ReactorReturnCodes.SUCCESS)
            {
                System.out.println("QueueData.encodeComplete() failed: " + CodecReturnCodes.toString(ret)
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
                _chnlInfo.tunnelStream.releaseBuffer(buffer, _errorInfo);
                return;
            }
            
            System.out.println("Submitted message with ID " + _identifier + " to " + _destNameList.get(i) + ".\n");
        }      
    }
        
       
    /* Processes a queue message event. */
    @Override
    public int queueMsgCallback(TunnelStreamQueueMsgEvent event)
    {
        int ret;
        
        switch (event.queueMsg().rdmMsgType())
        {
            case DATA:
            {
                boolean sendAck = false;
                QueueData queueData = (QueueData)event.queueMsg();
                
                switch (queueData.containerType())
                {
                    case  DataTypes.FIELD_LIST:
                    {
                        System.out.println("Received Msg on stream " + queueData.streamId() +
                                         " from " +  queueData.sourceName() +
                                         " to " + queueData.destName() +
                                         ", ID: " + queueData.identifier());                                      
                     
                        if (queueData.checkPossibleDuplicate())
                            System.out.println("  (Message may be a duplicate of a previous message.)");

                        System.out.println("  Queue Depth: " + queueData.queueDepth());
                        
                        // Received a buffer; decode it.
                        _msgDecIter.clear();
						_msgDecIter.setBufferAndRWFVersion(queueData.encodedDataBody(), _chnlInfo.tunnelStream.classOfService().common().protocolMajorVersion(),
								_chnlInfo.tunnelStream.classOfService().common().protocolMinorVersion());
    
                        _fieldList.clear();
                        ret = _fieldList.decode(_msgDecIter, null);
                        if (ret != CodecReturnCodes.SUCCESS)
                        {
                            System.out.println("\nfieldList.decode() failed: " + ret);
                            return ReactorCallbackReturnCodes.SUCCESS;
                        }
            
                        _fieldEntry.clear();
                        while ((ret = _fieldEntry.decode(_msgDecIter)) != CodecReturnCodes.END_OF_CONTAINER)
                        {
                            if (ret != CodecReturnCodes.SUCCESS)
                            {
                                System.out.println("\nfieldEntry.decode() failed: " + ret);
                                return ReactorCallbackReturnCodes.SUCCESS;
                            }
            
                            DictionaryEntry dictionaryEntry;
                            if ((dictionaryEntry = _dictionary.entry(_fieldEntry.fieldId())) == null)
                            {
                                System.out.println("No dictionaryEntry for fieldId: " + _fieldEntry.fieldId());
                                return ReactorCallbackReturnCodes.SUCCESS;
                            }
                            
                            int type = dictionaryEntry.rwfType();
                                                       
                            if ( type == DataTypes.ASCII_STRING)
                            {
                                ret = _msgStr.decode(_msgDecIter);
                                if (ret != CodecReturnCodes.SUCCESS)
                                {
                                    System.out.println("\nmsgStr.decode() failed: " + ret);
                                    return ReactorCallbackReturnCodes.SUCCESS;
                                }
                                
                                if (_fieldEntry.fieldId() == 35 && _msgStr.toString().equals("D"))
                                {   
                                    sendAck = true;
                                }
                                
                                System.out.println("Received message, fieldID = " + _fieldEntry.fieldId() + " value = " + _msgStr);                                 
                            }
                            else if ( type == DataTypes.INT)
                            {               
                                ret = _msgInt.decode(_msgDecIter);
                                if (ret != CodecReturnCodes.SUCCESS)
                                {
                                    System.out.println("\nmsgInt.decode() failed: " + ret);
                                    return ReactorCallbackReturnCodes.SUCCESS;
                                }            
                                System.out.println("Received message, fieldID = " + _fieldEntry.fieldId() + " value = " + _msgInt);                                 
                            }
                            else if ( type == DataTypes.ENUM)
                            {               
                                ret = _msgEnum.decode(_msgDecIter);
                                if (ret != CodecReturnCodes.SUCCESS)
                                {
                                    System.out.println("\nmsgEnum.decode() failed: " + ret);
                                    return ReactorCallbackReturnCodes.SUCCESS;
                                }            
                                System.out.println("Received message, fieldID = " + _fieldEntry.fieldId() + " value = " + _msgEnum);                                    
                            }
                            else if ( type == DataTypes.REAL)
                            {               
                                ret = _msgReal.decode(_msgDecIter);
                                if (ret != CodecReturnCodes.SUCCESS)
                                {
                                    System.out.println("\nmsgReal.decode() failed: " + ret);
                                    return ReactorCallbackReturnCodes.SUCCESS;
                                }            
                                System.out.println("Received message, fieldID = " + _fieldEntry.fieldId() + " value = " + _msgReal);                                    
                            }  
                            else if ( type == DataTypes.DATETIME) 
                            {               
                                ret = _msgDateTime.decode(_msgDecIter);
                                if (ret != CodecReturnCodes.SUCCESS)
                                {
                                    System.out.println("\nmsgDateTime.decode() failed: " + ret);
                                    return ReactorCallbackReturnCodes.SUCCESS;
                                }            
                                System.out.println("Received message, fieldID = " + _fieldEntry.fieldId() + " value = " + _msgDateTime);                                    
                            } 
                            
                        }
    
                        System.out.println();
                
                        // send acknowledgment message to remote user
                        if (sendAck)
                        {
                            ackQueueMsg(queueData.sourceName().toString(), queueData.streamId(), event.tunnelStream().streamId(), _msgInt);
                        }
                        break;
                    }
                    default:
                        break;
                }                
                break;
            }
            case DATAEXPIRED:
            {
                @SuppressWarnings("unused")
                boolean sendAck = false;
                QueueDataExpired queueDataExpired = (QueueDataExpired)event.queueMsg();
                
                switch (queueDataExpired.containerType())
                {
                    case  DataTypes.FIELD_LIST:
                    {
                        System.out.print("Received Msg on stream " + queueDataExpired.streamId() +
                                         " from " +  queueDataExpired.sourceName() +
                                         " to " + queueDataExpired.destName() +
                                         ", ID: " + queueDataExpired.identifier()); 
                        System.out.println(" (Undeliverable Message with code: " + QueueDataUndeliverableCode.toString(queueDataExpired.undeliverableCode()) + ")");
                                          
                        if (queueDataExpired.checkPossibleDuplicate())
                            System.out.println("  (Message may be a duplicate of a previous message.)");

                        System.out.println("  Queue Depth: " + queueDataExpired.queueDepth());
                        
                        // Received a buffer; decode it.
                        _msgDecIter.clear();
						_msgDecIter.setBufferAndRWFVersion(queueDataExpired.encodedDataBody(), _chnlInfo.tunnelStream.classOfService().common().protocolMajorVersion(),
								_chnlInfo.tunnelStream.classOfService().common().protocolMinorVersion());
    
                        _fieldList.clear();
                        ret = _fieldList.decode(_msgDecIter, null);
                        if (ret != CodecReturnCodes.SUCCESS)
                        {
                            System.out.println("\nfieldList.decode() failed: " + ret);
                            return ReactorCallbackReturnCodes.SUCCESS;
                        }
            
                        _fieldEntry.clear();
                        while ((ret = _fieldEntry.decode(_msgDecIter)) != CodecReturnCodes.END_OF_CONTAINER)
                        {
                            if (ret != CodecReturnCodes.SUCCESS)
                            {
                                System.out.println("\nfieldEntry.decode() failed: " + ret);
                                return ReactorCallbackReturnCodes.SUCCESS;
                            }
            
                            DictionaryEntry dictionaryEntry;
                            if ((dictionaryEntry = _dictionary.entry(_fieldEntry.fieldId())) == null)
                            {
                                System.out.println("No dictionaryEntry for fieldId: " + _fieldEntry.fieldId());
                                return ReactorCallbackReturnCodes.SUCCESS;
                            }
                            
                            int type = dictionaryEntry.rwfType();
                                                       
                            if ( type == DataTypes.ASCII_STRING)
                            {
                                ret = _msgStr.decode(_msgDecIter);
                                if (ret != CodecReturnCodes.SUCCESS)
                                {
                                    System.out.println("\nmsgStr.decode() failed: " + ret);
                                    return ReactorCallbackReturnCodes.SUCCESS;
                                }
                                
                                if (_fieldEntry.fieldId() == 35 && _msgStr.toString().equals("D"))
                                {   
                                    sendAck = true;
                                }
                                
                                System.out.println("Received message, fieldID = " + _fieldEntry.fieldId() + " value = " + _msgStr);                                 
                            }
                            else if ( type == DataTypes.INT)
                            {               
                                ret = _msgInt.decode(_msgDecIter);
                                if (ret != CodecReturnCodes.SUCCESS)
                                {
                                    System.out.println("\nmsgInt.decode() failed: " + ret);
                                    return ReactorCallbackReturnCodes.SUCCESS;
                                }            
                                System.out.println("Received message, fieldID = " + _fieldEntry.fieldId() + " value = " + _msgInt);                                 
                            }
                            else if ( type == DataTypes.ENUM)
                            {               
                                ret = _msgEnum.decode(_msgDecIter);
                                if (ret != CodecReturnCodes.SUCCESS)
                                {
                                    System.out.println("\nmsgEnum.decode() failed: " + ret);
                                    return ReactorCallbackReturnCodes.SUCCESS;
                                }            
                                System.out.println("Received message, fieldID = " + _fieldEntry.fieldId() + " value = " + _msgEnum);                                    
                            }
                            else if ( type == DataTypes.REAL)
                            {               
                                ret = _msgReal.decode(_msgDecIter);
                                if (ret != CodecReturnCodes.SUCCESS)
                                {
                                    System.out.println("\nmsgReal.decode() failed: " + ret);
                                    return ReactorCallbackReturnCodes.SUCCESS;
                                }            
                                System.out.println("Received message, fieldID = " + _fieldEntry.fieldId() + " value = " + _msgReal);                                    
                            }  
                            else if ( type == DataTypes.DATETIME) 
                            {               
                                ret = _msgDateTime.decode(_msgDecIter);
                                if (ret != CodecReturnCodes.SUCCESS)
                                {
                                    System.out.println("\nmsgDateTime.decode() failed: " + ret);
                                    return ReactorCallbackReturnCodes.SUCCESS;
                                }            
                                System.out.println("Received message, fieldID = " + _fieldEntry.fieldId() + " value = " + _msgDateTime);                                    
                            } 
                            
                        }
    
                        System.out.println();
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
    
    /*
     * Encode and send a queue message ACK through the ReactorChannel.
     * This message will contain an field list as its payload.
     */
    private int ackQueueMsg(String destName, int streamId, int tunnelStreamId, Int msgNumber)
    {
        int ret = ReactorCallbackReturnCodes.SUCCESS;
        
        if (_chnlInfo == null || _chnlInfo.tunnelStream == null || !_chnlInfo.isQueueStreamUp)
            return ret;
        
        // encode content into ackBuffer
        _ackBuffer.data().clear();
        _msgEncIter.clear();
		_msgEncIter.setBufferAndRWFVersion(_ackBuffer, _chnlInfo.tunnelStream.classOfService().common().protocolMajorVersion(),
				_chnlInfo.tunnelStream.classOfService().common().protocolMinorVersion());
        if ((ret = createExecutionReport(_msgEncIter)) < ReactorReturnCodes.SUCCESS)
        {
            System.out.println("Failed to encode FIX execution report");
            return ret;
        }
        
        // initialize the QueueData with the ackBuffer set as the encodedDataBody
        _queueData.clear();
        _queueData.streamId(streamId);
        _queueData.domainType(QUEUE_MSG_DOMAIN);
        _queueData.identifier(++_identifier);
        _queueData.sourceName().data(_sourceName);
        _queueData.destName().data(destName);
        _queueData.timeout(QueueDataTimeoutCode.INFINITE);
        _queueData.containerType(DataTypes.FIELD_LIST);
        _queueData.encodedDataBody(_ackBuffer);
        
        // submit QueueData message with the encodedDataBody to the tunnel stream
        if ((ret = _chnlInfo.tunnelStream.submit(_queueData, _errorInfo)) < ReactorReturnCodes.SUCCESS)
        {
            System.out.println("TunnelStream.submit() failed: " + CodecReturnCodes.toString(ret)
                    + "(" + _errorInfo.error().text() + ")");
            return ret;
        }
        
        System.out.println("Submitted ACK message with ID " + _identifier + " to " + destName + ".\n");
        
        return ret;
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

                if (state.dataState() == DataStates.OK && _chnlInfo.tunnelStream == null)
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
                         System.out.println("Failed to submit queue request to TunnelStream:"
                                 + ReactorReturnCodes.toString(ret) + "(" + _errorInfo.error().text() + ")");
                    }
                }
                break;

            case StreamStates.CLOSED_RECOVER:
            case StreamStates.CLOSED:
            default:
            	
                // For other stream states such as Closed & ClosedRecover, close the tunnel stream. 
                if ((ret = event.tunnelStream().close(_finalStatusEvent, _errorInfo)) < ReactorReturnCodes.SUCCESS)
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
    
    private int createNewOrder(EncodeIterator eIter)
    {
        int ret = CodecReturnCodes.SUCCESS;
        _fieldList.clear();
        _fieldEntry.clear();        
        _fieldList.applyHasStandardData();
        _fieldList.applyHasInfo();
               
        if ((ret = _fieldList.encodeInit(eIter, null, 0)) < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }
        
        //encode fields
        _fieldEntry.clear();

        _fieldEntry.fieldId(35);// MsgType
        _fieldEntry.dataType(DataTypes.BUFFER);
        _msgStr.data("D");   // D for new single order 
            
        if ((ret = _fieldEntry.encode(eIter, _msgStr)) < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }
 
        _fieldEntry.fieldId(11); // ClOrderId    -- client order id  
        _fieldEntry.dataType(DataTypes.BUFFER);
        _msgStr.data("100000020998");
            
        if ((ret = _fieldEntry.encode(eIter, _msgStr)) < CodecReturnCodes.SUCCESS)
        {
            return ret;          
        }        

        _fieldEntry.fieldId(1); // Account    
        _fieldEntry.dataType(DataTypes.BUFFER);
        _msgStr.data("D6789-3456");
            
        if ((ret = _fieldEntry.encode(eIter, _msgStr)) < CodecReturnCodes.SUCCESS)
        {
            return ret;          
        }         
                 
        _fieldEntry.fieldId(21); // Handle instruction
        _fieldEntry.dataType(DataTypes.ENUM);
        _msgEnum.value(1); // 1 = automated (2) semi automated 3 manual 
            
        if ((ret = _fieldEntry.encode(eIter, _msgEnum)) < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }                
        
        _fieldEntry.fieldId(55); // Symbol
        _fieldEntry.dataType(DataTypes.BUFFER);
        _msgStr.data("TRI"); // 1 for Buy 
            
        if ((ret = _fieldEntry.encode(eIter, _msgStr)) < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }                     
                
        _fieldEntry.fieldId(54); // Side
        _fieldEntry.dataType(DataTypes.ENUM);
        _msgEnum.value(1); // 1 for Buy 
            
        if ((ret = _fieldEntry.encode(eIter, _msgEnum)) < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }                        
        
        _fieldEntry.fieldId(60); // TransactionTime
        _fieldEntry.dataType(DataTypes.DATETIME);
        _msgDateTime.value(System.currentTimeMillis()); 
            
        if ((ret = _fieldEntry.encode(eIter, _msgDateTime)) < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }                
        
        _fieldEntry.fieldId(38); // OrderQty
        _fieldEntry.dataType(DataTypes.UINT);        
        _msgUInt.value(1000);  
            
        if ((ret = _fieldEntry.encode(eIter, _msgUInt)) < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }                        
        
        _fieldEntry.fieldId(40); // OrdType
        _fieldEntry.dataType(DataTypes.ENUM);
        _msgEnum.value(2); // 2 for limit Order 
            
        if ((ret = _fieldEntry.encode(eIter, _msgEnum)) < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }            
        
        _fieldEntry.fieldId(44); // Price
        _fieldEntry.dataType(DataTypes.REAL);
        _msgReal.value(3835, RealHints.EXPONENT_2);  
            
        if ((ret = _fieldEntry.encode(eIter, _msgReal)) < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }                     
        if ((ret = _fieldList.encodeComplete(eIter, true)) != CodecReturnCodes.SUCCESS)
        {
            return ret;
        }        
        
        return ret;
    }    
    
    
    private int createExecutionReport(EncodeIterator eIter)
    {
        int ret = CodecReturnCodes.SUCCESS;
        _fieldList.clear();
        _fieldEntry.clear();        
        _fieldList.applyHasStandardData();
        _fieldList.applyHasInfo();
               
        if ((ret = _fieldList.encodeInit(eIter, null, 0)) < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }
        
        //encode fields
        _fieldEntry.clear();

        _fieldEntry.fieldId(35);// MsgType
        _fieldEntry.dataType(DataTypes.BUFFER);
        _msgStr.data("8");   // 8 for execution report  
            
        if ((ret = _fieldEntry.encode(eIter, _msgStr)) < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }

        _fieldEntry.fieldId(37); // OrderId      
        _fieldEntry.dataType(DataTypes.BUFFER);
        _msgStr.data("BATS-3456789-98765");
            
        if ((ret = _fieldEntry.encode(eIter, _msgStr)) < CodecReturnCodes.SUCCESS)
        {
            return ret;          
        }        
                
        _fieldEntry.fieldId(11); // ClOrderId    -- client order id  
        _fieldEntry.dataType(DataTypes.BUFFER);
        _msgStr.data("100000020998");
            
        if ((ret = _fieldEntry.encode(eIter, _msgStr)) < CodecReturnCodes.SUCCESS)
        {
            return ret;          
        }   
        
        _fieldEntry.fieldId(17); // ExecID    -- execution id 
        _fieldEntry.dataType(DataTypes.BUFFER);
        _msgStr.data("7654689076");
            
        if ((ret = _fieldEntry.encode(eIter, _msgStr)) < CodecReturnCodes.SUCCESS)
        {
            return ret;          
        }                         
        
        _fieldEntry.fieldId(150); // ExecType    
        _fieldEntry.dataType(DataTypes.ENUM);
        _msgEnum.value(0);  // 0 for New, 1 for Partial fill 2 for fill 3 for Done for day 
            
        if ((ret = _fieldEntry.encode(eIter, _msgEnum)) < CodecReturnCodes.SUCCESS)
        {
            return ret;          
        } 
        
        _fieldEntry.fieldId(39); // Order Status    
        _fieldEntry.dataType(DataTypes.ENUM);
        _msgEnum.value(0);  // 0 for New, 1 for Partial fill 2 for fill 3 for Done for day 
            
        if ((ret = _fieldEntry.encode(eIter, _msgEnum)) < CodecReturnCodes.SUCCESS)
        {
            return ret;          
        }         
         
        _fieldEntry.fieldId(55); // Symbol
        _fieldEntry.dataType(DataTypes.BUFFER);
        _msgStr.data("TRI"); // 1 for Buy 
            
        if ((ret = _fieldEntry.encode(eIter, _msgStr)) < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }                     
                
        _fieldEntry.fieldId(54); // Side
        _fieldEntry.dataType(DataTypes.ENUM);
        _msgEnum.value(1); // 1 for Buy 
            
        if ((ret = _fieldEntry.encode(eIter, _msgEnum)) < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }                        
                          
        _fieldEntry.fieldId(151); // LeavesQty
        _fieldEntry.dataType(DataTypes.UINT);        
        _msgUInt.value(1000);  
             
        if ((ret = _fieldEntry.encode(eIter, _msgUInt)) < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }     
        
        _fieldEntry.fieldId(14); // CumQty   --- executed qty 
        _fieldEntry.dataType(DataTypes.UINT);        
        _msgUInt.value(0); 
                    
        if ((ret = _fieldEntry.encode(eIter, _msgUInt)) < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }             
                           
        _fieldEntry.fieldId(6); // AvgPx   average price filled
        _fieldEntry.dataType(DataTypes.REAL);
        _msgReal.value(0, RealHints.EXPONENT_2); 
            
        if ((ret = _fieldEntry.encode(eIter, _msgReal)) < CodecReturnCodes.SUCCESS)
        {
            return ret;
        }                     
        if ((ret = _fieldList.encodeComplete(eIter, true)) != CodecReturnCodes.SUCCESS)
        {
            return ret;
        }        
        
        return ret;
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
