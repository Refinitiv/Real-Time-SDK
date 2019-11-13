///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.upa.valueadd.reactor;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.GenericMsg;
import com.thomsonreuters.upa.codec.Msg;
import com.thomsonreuters.upa.codec.MsgClasses;
import com.thomsonreuters.upa.rdm.DomainTypes;
import com.thomsonreuters.upa.transport.Channel;
import com.thomsonreuters.upa.transport.TransportBuffer;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryMsg;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryMsgFactory;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryMsgType;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginMsg;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginMsgFactory;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginMsgType;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.queue.QueueAck;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.queue.QueueClose;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.queue.QueueData;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.queue.QueueDataExpired;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.queue.QueueRefresh;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.queue.QueueRequest;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.queue.QueueStatus;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.fail;

public class ReadEvent
{
    TransportBuffer _buffer;
    Channel _channel;
    DecodeIterator _dIter = CodecFactory.createDecodeIterator();
    int _lastReadRet;
    
    ReadEvent(TransportBuffer buffer, Channel channel, int lastReadRet)
    {
        if (buffer != null)
        {
            _buffer = new CopiedTransportBuffer(buffer);
            _dIter = CodecFactory.createDecodeIterator();
        }
        else
            _buffer = null;
        
        _channel = channel;
        _lastReadRet = lastReadRet; 
    }
    
    /** The readRetVal of the last call to channel.read. */
    int lastReadRet()
    {
        return _lastReadRet;
    }
    
	/** Decodes the buffer of this event to a message and returns it. */
    Msg msg()
    {
        Msg msg = CodecFactory.createMsg();
		_dIter.clear();
        assertEquals(CodecReturnCodes.SUCCESS, _dIter.setBufferAndRWFVersion(_buffer, _channel.majorVersion(), _channel.minorVersion()));
        assertEquals(CodecReturnCodes.SUCCESS, msg.decode(_dIter));
        return msg;
    }
    
	/** Decodes the buffer of this event to a LoginMsg and returns it. */
    LoginMsg loginMsg()
    {
        Msg msg = msg();
        LoginMsg loginMsg = LoginMsgFactory.createMsg();
                
		assertEquals(DomainTypes.LOGIN, msg.domainType());
        switch(msg.msgClass())
        {
            case MsgClasses.REQUEST:
                loginMsg.rdmMsgType(LoginMsgType.REQUEST);
                assertEquals(CodecReturnCodes.SUCCESS, loginMsg.decode(_dIter, msg));
                break;
                
            case MsgClasses.REFRESH:
                loginMsg.rdmMsgType(LoginMsgType.REFRESH);
                assertEquals(CodecReturnCodes.SUCCESS, loginMsg.decode(_dIter, msg));
                break;
                
            case MsgClasses.STATUS:
                loginMsg.rdmMsgType(LoginMsgType.STATUS);
                assertEquals(CodecReturnCodes.SUCCESS, loginMsg.decode(_dIter, msg));
                break;
                
            case MsgClasses.GENERIC:
                loginMsg.rdmMsgType(LoginMsgType.CONSUMER_CONNECTION_STATUS);
                assertEquals(CodecReturnCodes.SUCCESS, loginMsg.decode(_dIter, msg));
                break;
            default:
                fail("Unknown loginMsg type.");
        }
        
        return loginMsg;
    }

	/** Decodes the buffer of this event to a DirectoryMsg and returns it. */
    public DirectoryMsg directoryMsg()
    {
        Msg msg = msg();
        DirectoryMsg directoryMsg = DirectoryMsgFactory.createMsg();
                
		assertEquals(DomainTypes.SOURCE, msg.domainType());
        switch(msg.msgClass())
        {
            case MsgClasses.REQUEST:
                directoryMsg.rdmMsgType(DirectoryMsgType.REQUEST);
                assertEquals(CodecReturnCodes.SUCCESS, directoryMsg.decode(_dIter, msg));
                break;
                
            case MsgClasses.REFRESH:
                directoryMsg.rdmMsgType(DirectoryMsgType.REFRESH);
                assertEquals(CodecReturnCodes.SUCCESS, directoryMsg.decode(_dIter, msg));
                break;

            case MsgClasses.UPDATE:
                directoryMsg.rdmMsgType(DirectoryMsgType.UPDATE);
                assertEquals(CodecReturnCodes.SUCCESS, directoryMsg.decode(_dIter, msg));
                break;
                
            case MsgClasses.STATUS:
                directoryMsg.rdmMsgType(DirectoryMsgType.STATUS);
                assertEquals(CodecReturnCodes.SUCCESS, directoryMsg.decode(_dIter, msg));
                break;
                
            case MsgClasses.GENERIC:
                directoryMsg.rdmMsgType(DirectoryMsgType.CONSUMER_STATUS);
                assertEquals(CodecReturnCodes.SUCCESS, directoryMsg.decode(_dIter, msg));
                break;
            default:
                fail("Unknown directoryMsg type.");
        }
        
        return directoryMsg;
    }

	/** Decodes the given message to a TunnelStream msg and returns it. */
    public TunnelStreamMsg tunnelMsg(Msg msg, AckRangeList ackRangeList, AckRangeList nakRangeList)
    {
        TunnelStreamMsg tunnelMsg = new TunnelStreamMsgImpl();
        assertEquals(MsgClasses.GENERIC, msg.msgClass());
        assertEquals(CodecReturnCodes.SUCCESS, tunnelMsg.decode(_dIter, (GenericMsg)msg, ackRangeList, nakRangeList));
        return tunnelMsg;
    }
    
	/** Decodes a buffer to a QueueRequest. */
    public QueueRequest queueRequest(Buffer buffer)
    {
        QueueRequest queueRequest = TunnelStreamFactory.createQueueRequest();
        Msg msg = decodeMsg(buffer);
        assertEquals(CodecReturnCodes.SUCCESS, queueRequest.decode(_dIter, msg));
        return queueRequest;
    }

	/** Decodes a buffer to a QueueRefresh. */
    public QueueRefresh queueRefresh(Buffer buffer)
    {
        QueueRefresh queueRefresh = TunnelStreamFactory.createQueueRefresh();
        Msg msg = decodeMsg(buffer);
        assertEquals(CodecReturnCodes.SUCCESS, queueRefresh.decode(_dIter, msg));
        return queueRefresh;
    }

	/** Decodes a buffer to a QueueStatus. */
    public QueueStatus queueStatus(Buffer buffer)
    {
        QueueStatus queueStatus = TunnelStreamFactory.createQueueStatus();
        Msg msg = decodeMsg(buffer);
        assertEquals(CodecReturnCodes.SUCCESS, queueStatus.decode(_dIter, msg));
        return queueStatus;
    }

	/** Decodes a buffer to a QueueData. */
    public QueueData queueData(Buffer buffer)
    {
        QueueData queueData = TunnelStreamFactory.createQueueData();
        Msg msg = decodeMsg(buffer);
        assertEquals(CodecReturnCodes.SUCCESS, queueData.decode(_dIter, msg));
        return queueData;
    }

	/** Decodes a buffer to a QueueDataExpired. */
    public QueueDataExpired queueDataExpired(Buffer buffer)
    {
        QueueDataExpired queueDataExpired = TunnelStreamFactory.createQueueDataExpired();
        Msg msg = decodeMsg(buffer);
        assertEquals(CodecReturnCodes.SUCCESS, queueDataExpired.decode(_dIter, msg));
        return queueDataExpired;
    }

	/** Decodes a buffer to a QueueAck. */
    public QueueAck queueAck(Buffer buffer)
    {
        QueueAck queueAck = TunnelStreamFactory.createQueueAck();
        Msg msg = decodeMsg(buffer);
        assertEquals(CodecReturnCodes.SUCCESS, queueAck.decode(_dIter, msg));
        return queueAck;
    }
    
    /** Decodes a buffer to a QueueClose. */
    public QueueClose queueClose(Buffer buffer)
    {
        QueueClose queueClose = TunnelStreamFactory.createQueueClose();
        Msg msg = decodeMsg(buffer);
        assertEquals(CodecReturnCodes.SUCCESS, queueClose.decode(_dIter, msg));
        return queueClose;
    }

	/* Decodes a buffer to a msg. */
    public Msg decodeMsg(Buffer buffer)
    {
        Msg msg = CodecFactory.createMsg();
		_dIter.clear();
        assertEquals(CodecReturnCodes.SUCCESS, _dIter.setBufferAndRWFVersion(buffer, _channel.majorVersion(), _channel.minorVersion()));
        assertEquals(CodecReturnCodes.SUCCESS, msg.decode(_dIter));
        return msg;
    }
}
