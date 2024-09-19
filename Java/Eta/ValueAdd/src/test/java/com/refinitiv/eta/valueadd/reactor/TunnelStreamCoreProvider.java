///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019 LSEG. All rights reserved.     
///*|-----------------------------------------------------------------------------

package com.refinitiv.eta.valueadd.reactor;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.assertNotNull;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DataStates;
import com.refinitiv.eta.codec.DataTypes;
import com.refinitiv.eta.codec.GenericMsg;
import com.refinitiv.eta.codec.Msg;
import com.refinitiv.eta.codec.MsgClasses;
import com.refinitiv.eta.codec.RefreshMsg;
import com.refinitiv.eta.codec.RequestMsg;
import com.refinitiv.eta.codec.StateCodes;
import com.refinitiv.eta.codec.StreamStates;
import com.refinitiv.eta.transport.TransportBuffer;
import com.refinitiv.eta.transport.TransportReturnCodes;
import com.refinitiv.eta.valueadd.domainrep.rdm.queue.QueueMsg;
import com.refinitiv.eta.valueadd.reactor.TunnelStreamMsg.TunnelStreamAck;
import com.refinitiv.eta.valueadd.reactor.TunnelStreamMsg.TunnelStreamData;

public class TunnelStreamCoreProvider extends CoreComponent
{
    /* Needed for ClassOfService decode */
    ReactorErrorInfo _errorInfo;
    int MSG_BUF_LEN = 512;
    
    public TunnelStreamCoreProvider()
    {
        _errorInfo = ReactorFactory.createReactorErrorInfo();
    }
    
    void acceptTunnelStreamRequest(Consumer consumer, RequestMsg requestMsg, ClassOfService provClassOfService)
    {
        TransportBuffer buffer;
        RefreshMsg refreshMsg = (RefreshMsg)CodecFactory.createMsg();
        ClassOfService cos;

        assertTrue(requestMsg.checkStreaming());
        assertTrue(requestMsg.checkPrivateStream());
        assertTrue(requestMsg.checkQualifiedStream());;
        assertTrue(requestMsg.msgKey().checkHasFilter());
        assertTrue(requestMsg.msgKey().checkHasServiceId());
        assertEquals(DataTypes.FILTER_LIST, requestMsg.containerType());
        
        _dIter.clear();
        assertEquals(CodecReturnCodes.SUCCESS, _dIter.setBufferAndRWFVersion(requestMsg.encodedDataBody(), _channel.majorVersion(), _channel.minorVersion()));

        if (provClassOfService == null)
        {
            /* Echo consumer's class of service. */
            cos = ReactorFactory.createClassOfService();
            assertEquals(CodecReturnCodes.SUCCESS, cos.decode(consumer.reactorChannel(), requestMsg.encodedDataBody(), _errorInfo));
        }
        else
            cos = provClassOfService;

        cos.isServer(true);
        
        buffer = _channel.getBuffer(MSG_BUF_LEN, false, _error);
        assertNotNull(buffer);
        _eIter.clear();
        _eIter.setBufferAndRWFVersion(buffer, _channel.majorVersion(), _channel.minorVersion());
        
        /* Encode tunnel stream refresh. */
        refreshMsg.clear();
        refreshMsg.msgClass(MsgClasses.REFRESH);
        refreshMsg.applyPrivateStream();
        refreshMsg.applyQualifiedStream();
        refreshMsg.applySolicited();
        refreshMsg.applyRefreshComplete();
        refreshMsg.applyDoNotCache();
        refreshMsg.domainType(requestMsg.domainType());
        refreshMsg.streamId(requestMsg.streamId());
        refreshMsg.applyHasMsgKey();
        refreshMsg.msgKey().applyHasServiceId();
        refreshMsg.msgKey().serviceId(requestMsg.msgKey().serviceId());
        refreshMsg.msgKey().applyHasName();
        refreshMsg.msgKey().name(requestMsg.msgKey().name());
        refreshMsg.state().streamState(StreamStates.OPEN);
        refreshMsg.state().dataState(DataStates.OK);
        refreshMsg.state().code(StateCodes.NONE);
        refreshMsg.state().text().data("Successful open of TunnelStream" + requestMsg.msgKey().name());
        refreshMsg.containerType(DataTypes.FILTER_LIST);
        refreshMsg.msgKey().applyHasFilter();
        refreshMsg.msgKey().filter(cos.filterFlags());
        
        assertEquals(CodecReturnCodes.ENCODE_CONTAINER, refreshMsg.encodeInit(_eIter, 0));
        assertEquals(CodecReturnCodes.SUCCESS, cos.encode(_eIter));
        
        assertEquals(CodecReturnCodes.SUCCESS, refreshMsg.encodeComplete(_eIter, true));
        
        writeBuffer(buffer);
    }
    
    // returns substream Msg
    public Msg decodeQueueMsg(Msg msg, TunnelStreamMsg streamHeader,
            QueueMsg substreamHeader, AckRangeList ackRangeList, AckRangeList nakRangeList)
    {
        Msg subMsg = CodecFactory.createMsg();
        GenericMsg genericMsg = (GenericMsg)msg;

        assertTrue(msg.msgClass() == MsgClasses.GENERIC);

        _dIter.clear();
        assertEquals(CodecReturnCodes.SUCCESS, streamHeader.decode(_dIter, genericMsg, ackRangeList, nakRangeList));

        if (streamHeader.opCode() == TunnelStreamMsg.OpCodes.DATA
                || streamHeader.opCode() == TunnelStreamMsg.OpCodes.RETRANS)
        {
            _dIter.setBufferAndRWFVersion(genericMsg.encodedDataBody(), _channel.majorVersion(), _channel.minorVersion());
            assertEquals(subMsg.decode(_dIter), CodecReturnCodes.SUCCESS);
            assertEquals(CodecReturnCodes.SUCCESS, substreamHeader.decode(_dIter, subMsg));
        }       
        return subMsg;
    }

    public void submitTunnelStreamAck(TunnelStreamAck tunnelStreamAck)
    {
        submitTunnelStreamAck(tunnelStreamAck, null, null, 0);
    }
    
    public void submitTunnelStreamAck(TunnelStreamAck tunnelStreamAck, AckRangeList ackRangeList, AckRangeList nakRangeList, int actionOpCode)
    {
        int ret;
        int bufferSize = 256;
        
        do
        {
            TransportBuffer buffer = _channel.getBuffer(bufferSize, false, _error);
            assertNotNull(buffer);
            assertEquals(TransportReturnCodes.SUCCESS, _eIter.setBufferAndRWFVersion(buffer, _channel.majorVersion(), _channel.minorVersion()));
            ret = tunnelStreamAck.encodeAck(_eIter, ackRangeList, nakRangeList, actionOpCode);
            
            switch (ret)
            {
                case CodecReturnCodes.BUFFER_TOO_SMALL:
                    assertEquals(TransportReturnCodes.SUCCESS, _channel.releaseBuffer(buffer, _error));
                    bufferSize *= 2;
                    break;
                default:
                    assertEquals(CodecReturnCodes.SUCCESS, ret);
                    writeBuffer(buffer);
                    return;
            }
        } while (true);       
    }

    /* Helper to send a queueMsg or Codec msg in a TunnelStream. */
    private void submitTunnelStreamMsg(TunnelStreamData tunnelStreamData, QueueMsg queueMsg, Msg msg)
    {
        int ret;
        int bufferSize = 256;

        assert (queueMsg != null && msg == null || queueMsg == null && msg != null);
        
        do
        {
            TransportBuffer buffer = _channel.getBuffer(bufferSize, false, _error);
            assertNotNull(buffer);
            assertEquals(TransportReturnCodes.SUCCESS, _eIter.setBufferAndRWFVersion(buffer, _channel.majorVersion(), _channel.minorVersion()));
            ret = tunnelStreamData.encodeDataInit(_eIter);
            if (ret == CodecReturnCodes.SUCCESS)
                if (queueMsg != null)
                    ret = queueMsg.encode(_eIter);
                else
                    ret = msg.encode(_eIter);
            if (ret == CodecReturnCodes.SUCCESS)
                ret = tunnelStreamData.encodeDataComplete(_eIter);
            
            switch (ret)
            {
                case CodecReturnCodes.BUFFER_TOO_SMALL:
                    assertEquals(TransportReturnCodes.SUCCESS, _channel.releaseBuffer(buffer, _error));
                    bufferSize *= 2;
                    break;
                default:
                    assertEquals(CodecReturnCodes.SUCCESS, ret);
                    writeBuffer(buffer);
                    return;
            }
        } while (true);     
    }

    /* Submit a QueueMsg in a TunnelStream. */
    public void submitTunnelStreamQueueMsg(TunnelStreamData tunnelStreamData, QueueMsg queueMsg)
    {
        submitTunnelStreamMsg(tunnelStreamData, queueMsg, null);
    }

    /* Submit a Codec Msg in a TunnelStream. */
    public void submitTunnelStreamMsg(TunnelStreamData tunnelStreamData, Msg msg)
    {
        submitTunnelStreamMsg(tunnelStreamData, null, msg);
    }
    
    /* Submit a opaque buffer in a TunnelStream. */
    public void submitTunnelStreamOpaqueData(TunnelStreamData tunnelStreamData, Buffer opaqueBuffer)
    {
        int ret;
        int bufferSize = 256;

        do
        {
            TransportBuffer buffer = _channel.getBuffer(bufferSize, false, _error);
            assertNotNull(buffer);
            assertEquals(TransportReturnCodes.SUCCESS, _eIter.setBufferAndRWFVersion(buffer, _channel.majorVersion(), _channel.minorVersion()));
            ret = ((TunnelStreamMsgImpl)tunnelStreamData).encodeDataInitOpaque(_eIter);
            if (ret == CodecReturnCodes.SUCCESS)
            {
            	Buffer nonRWFBuffer = CodecFactory.createBuffer();
            	ret = _eIter.encodeNonRWFInit(nonRWFBuffer);
            	if (ret == CodecReturnCodes.SUCCESS)
            	{
            		if (opaqueBuffer.length() < nonRWFBuffer.length())
            		{
            			nonRWFBuffer.data().put(opaqueBuffer.data());
                		ret = _eIter.encodeNonRWFComplete(nonRWFBuffer, true);
            		}
            	}
            }
            if (ret == CodecReturnCodes.SUCCESS)
                ret = tunnelStreamData.encodeDataComplete(_eIter);
            
            switch (ret)
            {
                case CodecReturnCodes.BUFFER_TOO_SMALL:
                    assertEquals(TransportReturnCodes.SUCCESS, _channel.releaseBuffer(buffer, _error));
                    bufferSize *= 2;
                    break;
                default:
                    assertEquals(CodecReturnCodes.SUCCESS, ret);
                    writeBuffer(buffer);
                    return;
            }
        } while (true);     
    }
}
