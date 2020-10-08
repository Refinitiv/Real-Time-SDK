///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.refinitiv.eta.valueadd.reactor;
import static org.junit.Assert.fail;
import static org.junit.Assert.assertEquals;

import java.nio.ByteBuffer;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.CopyMsgFlags;
import com.refinitiv.eta.codec.RequestMsg;
import com.refinitiv.eta.valueadd.domainrep.rdm.dictionary.DictionaryMsgFactory;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgFactory;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgFactory;
import com.refinitiv.eta.valueadd.domainrep.rdm.queue.QueueAck;
import com.refinitiv.eta.valueadd.domainrep.rdm.queue.QueueData;
import com.refinitiv.eta.valueadd.domainrep.rdm.queue.QueueDataExpired;
import com.refinitiv.eta.valueadd.domainrep.rdm.queue.QueueMsg;
import com.refinitiv.eta.valueadd.domainrep.rdm.queue.QueueRefresh;
import com.refinitiv.eta.valueadd.domainrep.rdm.queue.QueueRequest;
import com.refinitiv.eta.valueadd.domainrep.rdm.queue.QueueStatus;

/** Represents an event received when a component calls dispatch on its reactor.
 * The event is fully copied, including such data as messages received in Reactor events. 
*/
public class TestReactorEvent
{
    TestReactorEventTypes _type;
    ReactorEvent _event;
    TunnelStreamRequestEvent _tunnelStreamRequestEvent;
    long _nanoTime = System.nanoTime();
        
    /** Returns the type of the ComponentEvent. */
    public TestReactorEventTypes type()
    {
        return _type;
    }
    
    /** Returns the associated ReactorEvent. */
    public ReactorEvent reactorEvent()
    {
        return _event;
    }
    
    /** Returns the associated TunnelStreamRequestEvent. */
    public TunnelStreamRequestEvent tunnelStreamRequestEvent()
    {
        return _tunnelStreamRequestEvent;
    }
    
    /** Returns time at which the event was received, as taken by System.nanoTime() */
    public long nanoTime()
    {
        return _nanoTime;
    }
    
    /** Allocates memory in destination buffer and performs buffer copy. */
    private void createBufferCopy(Buffer srcBuffer, Buffer destBuffer)
    {
        destBuffer.data(ByteBuffer.allocateDirect(srcBuffer.length()));
        assertEquals(CodecReturnCodes.SUCCESS, srcBuffer.copy(destBuffer.data()));
    }
    
    /** Construct an event based on its type */
    public TestReactorEvent(TestReactorEventTypes type, ReactorEvent event)
    {
        _type = type;
        
        /* Copy event, based on which type it is. */
        switch(type)
        {
        
        case AUTH_TOKEN_EVENT:
        {
        	_event = new ReactorAuthTokenEvent();
        	
        	((ReactorAuthTokenEvent)_event)._reactorAuthTokenInfo = ((ReactorAuthTokenEvent)event).reactorAuthTokenInfo();
        	break;
        }
        
        case SERVICE_DISC_ENDPOINT:
        {
        	_event = new ReactorServiceEndpointEvent();

        	((ReactorServiceEndpointEvent)_event)._reactorServiceEndpointInfoList = ((ReactorServiceEndpointEvent)event).serviceEndpointInfo();
        	break;
        }
        
        case CHANNEL_EVENT:
        {
            _event = new ReactorChannelEvent();
            ((ReactorChannelEvent)_event).eventType(((ReactorChannelEvent)event).eventType());
            break;
        }
            
        case MSG:
        {
            _event = new ReactorMsgEvent();
            TestUtil.copyMsgEvent((ReactorMsgEvent)event, (ReactorMsgEvent)_event);
            break;
        }
        
        case LOGIN_MSG:
        {
            _event = new RDMLoginMsgEvent();
            RDMLoginMsgEvent loginMsgEvent = (RDMLoginMsgEvent)_event; 
            RDMLoginMsgEvent otherLoginMsgEvent = (RDMLoginMsgEvent)event;
            
            TestUtil.copyMsgEvent((ReactorMsgEvent)event, (ReactorMsgEvent)_event);
            
            if (otherLoginMsgEvent.rdmLoginMsg() != null)
            {
                loginMsgEvent.rdmLoginMsg(LoginMsgFactory.createMsg());
                TestUtil.copyLoginMsg(otherLoginMsgEvent.rdmLoginMsg(), loginMsgEvent.rdmLoginMsg());
            }
            break;
        }
        
        case DIRECTORY_MSG:
        {
            _event = new RDMDirectoryMsgEvent();
            RDMDirectoryMsgEvent directoryMsgEvent = (RDMDirectoryMsgEvent)_event; 
            RDMDirectoryMsgEvent otherDirectoryMsgEvent = (RDMDirectoryMsgEvent)event;
            
            TestUtil.copyMsgEvent((ReactorMsgEvent)event, (ReactorMsgEvent)_event);
            
            if (otherDirectoryMsgEvent.rdmDirectoryMsg() != null)
            {
                directoryMsgEvent.rdmDirectoryMsg(DirectoryMsgFactory.createMsg());
                TestUtil.copyDirectoryMsg(otherDirectoryMsgEvent.rdmDirectoryMsg(), directoryMsgEvent.rdmDirectoryMsg());
            }
            break;
        }
        
        case DICTIONARY_MSG:
        {
            _event = new RDMDictionaryMsgEvent();
            RDMDictionaryMsgEvent dictionaryMsgEvent = (RDMDictionaryMsgEvent)_event; 
            RDMDictionaryMsgEvent otherDictionaryMsgEvent = (RDMDictionaryMsgEvent)event;
            
            TestUtil.copyMsgEvent((ReactorMsgEvent)event, (ReactorMsgEvent)_event);
            
            if (otherDictionaryMsgEvent.rdmDictionaryMsg() != null)
            {
                dictionaryMsgEvent.rdmDictionaryMsg(DictionaryMsgFactory.createMsg());
                TestUtil.copyDictionaryMsg(otherDictionaryMsgEvent.rdmDictionaryMsg(), dictionaryMsgEvent.rdmDictionaryMsg());
            }
            break;
        }
        
        case TUNNEL_STREAM_STATUS:
        {
            _event = new TunnelStreamStatusEvent();
            TunnelStreamStatusEvent statusEvent = (TunnelStreamStatusEvent)_event;
            TunnelStreamStatusEvent otherStatusEvent = (TunnelStreamStatusEvent)event;
            
            otherStatusEvent.state().copy(statusEvent.state());
            statusEvent.tunnelStream(otherStatusEvent.tunnelStream());
            
            if (otherStatusEvent.authInfo() != null)
            {
                statusEvent.authInfo(new TunnelStreamAuthInfo());
                if (otherStatusEvent.authInfo().loginMsg() != null)
                {
                    statusEvent.authInfo().loginMsg(LoginMsgFactory.createMsg());
                    TestUtil.copyLoginMsg(otherStatusEvent.authInfo().loginMsg(), statusEvent.authInfo().loginMsg());
                }
            }
            
            break;
        }
        
        case TUNNEL_STREAM_MSG:
        {
            _event = new TunnelStreamMsgEvent();
            
            TunnelStreamMsgEvent msgEvent = (TunnelStreamMsgEvent)_event;
            TunnelStreamMsgEvent otherMsgEvent = (TunnelStreamMsgEvent)event;
                
            msgEvent.tunnelStream(otherMsgEvent.tunnelStream());
            msgEvent.containerType(otherMsgEvent.containerType());
            TestUtil.copyMsgEvent(otherMsgEvent, msgEvent);
            break;
        }
        
        case TUNNEL_STREAM_QUEUE_MSG:
        {
            _event = new TunnelStreamQueueMsgEvent();
            
            TunnelStreamQueueMsgEvent queueMsgEvent = (TunnelStreamQueueMsgEvent)_event;
            TunnelStreamQueueMsgEvent otherQueueMsgEvent = (TunnelStreamQueueMsgEvent)event;
            
            queueMsgEvent.tunnelStream(otherQueueMsgEvent.tunnelStream());
            TestUtil.copyMsgEvent(otherQueueMsgEvent, queueMsgEvent);

            /* Copy QueueMsg elements according to message type. */
            switch(otherQueueMsgEvent.queueMsg().rdmMsgType())
            {
                case REQUEST:
                {
                    QueueRequestImpl queueRequest = new QueueRequestImpl();
                    QueueRequest otherQueueRequest = (QueueRequest)otherQueueMsgEvent.queueMsg();
                    createBufferCopy(otherQueueRequest.sourceName(), queueRequest.sourceName());
                    queueMsgEvent.queueMsg(queueRequest);
                    break;
                }
                
                case REFRESH:
                {
                    QueueRefreshImpl queueRefresh = new QueueRefreshImpl();
                    QueueRefresh otherQueueRefresh = (QueueRefresh)otherQueueMsgEvent.queueMsg();
                    queueRefresh.queueDepth(otherQueueRefresh.queueDepth());
                    createBufferCopy(otherQueueRefresh.sourceName(), queueRefresh.sourceName());
                    otherQueueRefresh.state().copy(queueRefresh.state());
                    queueMsgEvent.queueMsg(queueRefresh);
                    break;
                }
                case STATUS:
                {
                    QueueStatusImpl queueStatus = new QueueStatusImpl();
                    QueueStatus otherQueueStatus = (QueueStatus)otherQueueMsgEvent.queueMsg();
                    
                    queueStatus.flags(otherQueueStatus.flags());
                    if (otherQueueStatus.checkHasState())
                    {
                        queueStatus.applyHasState();
                        otherQueueStatus.state().copy(queueStatus.state());
                    }
                    queueMsgEvent.queueMsg(queueStatus);
                    break;
                }
                case DATA:
                {
                    QueueDataImpl queueData = new QueueDataImpl();
                    QueueData otherQueueData = (QueueData)otherQueueMsgEvent.queueMsg();
                    
                    queueData.containerType(otherQueueData.containerType());
                    createBufferCopy(otherQueueData.destName(), queueData.destName());
                    if (otherQueueData.encodedDataBody().length() > 0)
                    {
                       createBufferCopy(otherQueueData.encodedDataBody(), queueData.encodedDataBody());
                    }
                    queueData.flags(otherQueueData.flags());
                    queueData.identifier(otherQueueData.identifier());
                    queueData.queueDepth(otherQueueData.queueDepth());
                    createBufferCopy(otherQueueData.sourceName(), queueData.sourceName());
                    queueData.timeout(otherQueueData.timeout());

                    queueMsgEvent.queueMsg(queueData);
                    break;
                }
                case DATAEXPIRED:
                {
                    QueueDataExpiredImpl queueDataExpired = new QueueDataExpiredImpl();
                    QueueDataExpired otherQueueDataExpired = (QueueDataExpired)otherQueueMsgEvent.queueMsg();
                    
                    queueDataExpired.containerType(otherQueueDataExpired.containerType());
                    createBufferCopy(otherQueueDataExpired.destName(), queueDataExpired.destName());
                    if (otherQueueDataExpired.encodedDataBody().length() > 0)
                    {
                       createBufferCopy(otherQueueDataExpired.encodedDataBody(), queueDataExpired.encodedDataBody());
                    }
                    queueDataExpired.flags(otherQueueDataExpired.flags());
                    queueDataExpired.identifier(otherQueueDataExpired.identifier());
                    queueDataExpired.queueDepth(otherQueueDataExpired.queueDepth());
                    createBufferCopy(otherQueueDataExpired.sourceName(), queueDataExpired.sourceName());
                    queueDataExpired.undeliverableCode(otherQueueDataExpired.undeliverableCode());

                    queueMsgEvent.queueMsg(queueDataExpired);
                    break;
                }
                case ACK:
                {
                    QueueAckImpl queueAck = new QueueAckImpl();
                    QueueAck otherQueueAck = (QueueAck)otherQueueMsgEvent.queueMsg();
                    
                    createBufferCopy(otherQueueAck.destName(), queueAck.destName());
                    queueAck.identifier(otherQueueAck.identifier());
                    createBufferCopy(otherQueueAck.sourceName(), queueAck.sourceName());
                    queueMsgEvent.queueMsg(queueAck);
                    break;
                }
                default:
                    fail("Unknown QueueMsg type");
            }
            
            /* Copy general QueueMsg members. */
            QueueMsg queueMsg = queueMsgEvent.queueMsg();
            QueueMsg otherQueueMsg = otherQueueMsgEvent.queueMsg();
            queueMsg.streamId(otherQueueMsg.streamId());
            queueMsg.domainType(otherQueueMsg.domainType());
            queueMsg.serviceId(otherQueueMsg.serviceId());
            break;
        }
        
        default:
            fail("Unknown ComponentEvent type.");
            break;
            
        }
        
        TestUtil.copyErrorInfo(event.errorInfo(), _event.errorInfo());
        _event.reactorChannel(event.reactorChannel());
    }
    
    /** Construct a tunnel stream request event. */
    public TestReactorEvent(TunnelStreamRequestEvent tunnelStreamRequestEvent)
    {
        _type = TestReactorEventTypes.TUNNEL_STREAM_REQUEST;
        _tunnelStreamRequestEvent = new TunnelStreamRequestEvent();

        /* Copy the event. */
        _tunnelStreamRequestEvent.reactorChannel(tunnelStreamRequestEvent.reactorChannel());
        _tunnelStreamRequestEvent.domainType(tunnelStreamRequestEvent.domainType());
        _tunnelStreamRequestEvent.streamId(tunnelStreamRequestEvent.streamId());
        _tunnelStreamRequestEvent.serviceId(tunnelStreamRequestEvent.serviceId());
        _tunnelStreamRequestEvent.name(tunnelStreamRequestEvent.name());
        
        _tunnelStreamRequestEvent.msg((RequestMsg)CodecFactory.createMsg());
        tunnelStreamRequestEvent.msg().copy(_tunnelStreamRequestEvent.msg(), CopyMsgFlags.ALL_FLAGS);
        
        if (tunnelStreamRequestEvent.errorInfo() != null)
        {
            _tunnelStreamRequestEvent.errorInfo(ReactorFactory.createReactorErrorInfo());
            TestUtil.copyErrorInfo(tunnelStreamRequestEvent.errorInfo(), _tunnelStreamRequestEvent.errorInfo());   
        }
        
        _tunnelStreamRequestEvent.classOfServiceFilter(tunnelStreamRequestEvent.classOfServiceFilter());
        tunnelStreamRequestEvent.classOfService().copy(_tunnelStreamRequestEvent.classOfService());
    }
}
