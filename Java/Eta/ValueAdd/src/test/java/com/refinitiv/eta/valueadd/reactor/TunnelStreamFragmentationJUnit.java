/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

import static org.junit.Assert.*;

import java.nio.ByteBuffer;

import org.junit.Test;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.Codec;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DataStates;
import com.refinitiv.eta.codec.DataTypes;
import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.ElementEntry;
import com.refinitiv.eta.codec.ElementList;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.FilterEntry;
import com.refinitiv.eta.codec.FilterEntryActions;
import com.refinitiv.eta.codec.FilterList;
import com.refinitiv.eta.codec.GenericMsg;
import com.refinitiv.eta.codec.Msg;
import com.refinitiv.eta.codec.MsgClasses;
import com.refinitiv.eta.codec.RequestMsg;
import com.refinitiv.eta.codec.StateCodes;
import com.refinitiv.eta.codec.StatusMsg;
import com.refinitiv.eta.codec.StreamStates;
import com.refinitiv.eta.codec.UInt;
import com.refinitiv.eta.rdm.ClassesOfService;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.transport.TransportBuffer;
import com.refinitiv.eta.valueadd.reactor.TunnelStreamMsg.TunnelStreamData;
import com.refinitiv.eta.rdm.ClassesOfService.AuthenticationTypes;
import com.refinitiv.eta.rdm.ClassesOfService.DataIntegrityTypes;
import com.refinitiv.eta.rdm.ClassesOfService.FlowControlTypes;
import com.refinitiv.eta.rdm.ClassesOfService.GuaranteeTypes;


public class TunnelStreamFragmentationJUnit 
{
    ReactorErrorInfo _errorInfo = ReactorFactory.createReactorErrorInfo();
    TunnelStreamSubmitOptions _tsSubmitOpts = ReactorFactory.createTunnelStreamSubmitOptions();

    /* Tests the big buffer pool for tunnel stream fragmentation. */
    @Test
    public void bigBufferPoolTest()
    {
    	int count = 500, poolCount = 0;
    	TunnelStreamBigBufferPool bufferPool = new TunnelStreamBigBufferPool(6144, count);
    	ReactorErrorInfo errorInfo = ReactorFactory.createReactorErrorInfo();
    	TunnelStreamBigBuffer buffers[] = new TunnelStreamBigBuffer[count];
    	
    	// get buffer and validate initial attributes
    	for (int i = 1; i < count + 1; i++)
    	{
    		TunnelStreamBigBuffer buffer = bufferPool.getBuffer(6144 * i - 100, errorInfo);
    		assertTrue(buffer != null);
    		assertTrue(buffer.isBigBuffer());
    		assertTrue(buffer.data() != null);
    		assertEquals(buffer.data().limit(), 6144 * i - 100);
    		assertEquals(buffer.dataStartPosition(), 0);
    		assertEquals(buffer.length(), 6144 * i - 100);
    		assertEquals(buffer.capacity(), 6144 * i - 100);
    		assertFalse(buffer.fragmentationInProgress());
    		assertEquals(0, buffer._totalMsgLength);
    		assertEquals(0, buffer._bytesRemainingToSend);
    		assertEquals(0, buffer._lastFragmentId);
    		assertEquals(0, buffer._messageId);
    		assertEquals(0, buffer._containerType);
    		buffers[i - 1] = buffer;
    	}
    	
    	// attempt to get another big buffer - this should fail since already retrieved max number specified in count
    	assertTrue(bufferPool.getBuffer(6144, errorInfo) == null);
    	
    	// add content to buffers and validate changes
    	for (int i = 0; i < count; i++)
    	{
    		String content = "Content for buffer number " + i;
    		buffers[i].data().put(content.getBytes());
    		assertEquals(buffers[i].dataStartPosition(), 0);
    		assertEquals(buffers[i].length(), content.length());
    		assertEquals(buffers[i].capacity(), (6144 * (i + 1)) - 100);
    		// validate copy
    		ByteBuffer byteBuffer = ByteBuffer.allocate(buffers[i].length());
    		buffers[i].copy(byteBuffer);
    		for (int j = 0; j < buffers[i].length(); j++)
    		{
    			assertTrue(byteBuffer.get(j) == buffers[i].data().get(j));
    		}
    		// save write progress and validate changes
    		buffers[i].saveWriteProgress(12345, 1234, 3, 4, 5);
    		assertTrue(buffers[i].fragmentationInProgress());
    		assertEquals(12345, buffers[i]._totalMsgLength);
    		assertEquals(1234, buffers[i]._bytesRemainingToSend);
    		assertEquals(3, buffers[i]._lastFragmentId);
    		assertEquals(4, buffers[i]._messageId);
    		assertEquals(5, buffers[i]._containerType);
    		// release buffer
    		bufferPool.releaseBuffer(buffers[i]);
    	}
    	
    	// count the number of pool buffers and validate it's the same as the number retrieved
    	for (int i = 0; i < bufferPool.NUM_POOLS; i++)
    	{
    		if (bufferPool._pools[i] != null)
    		{
    			poolCount += bufferPool._pools[i].count();
    		}
    	}
    	assertEquals(poolCount, count);

    	// REPEAT
    	poolCount = 0;
    	
    	// get buffer and validate initial attributes
    	for (int i = 1; i < count + 1; i++)
    	{
    		TunnelStreamBigBuffer buffer = bufferPool.getBuffer(6144 * i - 100, errorInfo);
    		assertTrue(buffer != null);
    		assertTrue(buffer.isBigBuffer());
    		assertTrue(buffer.data() != null);
    		assertEquals(buffer.data().limit(), 6144 * i - 100);
    		assertEquals(buffer.dataStartPosition(), 0);
    		assertEquals(buffer.length(), 6144 * i - 100);
    		assertEquals(buffer.capacity(), 6144 * i - 100);
    		assertFalse(buffer.fragmentationInProgress());
    		assertEquals(0, buffer._totalMsgLength);
    		assertEquals(0, buffer._bytesRemainingToSend);
    		assertEquals(0, buffer._lastFragmentId);
    		assertEquals(0, buffer._messageId);
    		assertEquals(0, buffer._containerType);
    		buffers[i - 1] = buffer;
    	}
    	
    	// attempt to get another big buffer - this should fail since already retrieved max number specified in count
    	assertTrue(bufferPool.getBuffer(6144, errorInfo) == null);
    	
    	// add content to buffers and validate changes
    	for (int i = 0; i < count; i++)
    	{
    		String content = "Content for buffer number " + i;
    		buffers[i].data().put(content.getBytes());
    		assertEquals(buffers[i].dataStartPosition(), 0);
    		assertEquals(buffers[i].length(), content.length());
    		assertEquals(buffers[i].capacity(), (6144 * (i + 1)) - 100);
    		// validate copy
    		ByteBuffer byteBuffer = ByteBuffer.allocate(buffers[i].length());
    		buffers[i].copy(byteBuffer);
    		for (int j = 0; j < buffers[i].length(); j++)
    		{
    			assertTrue(byteBuffer.get(j) == buffers[i].data().get(j));
    		}
    		// save write progress and validate changes
    		buffers[i].saveWriteProgress(54321, 5432, 5, 4, 3);
    		assertTrue(buffers[i].fragmentationInProgress());
    		assertEquals(54321, buffers[i]._totalMsgLength);
    		assertEquals(5432, buffers[i]._bytesRemainingToSend);
    		assertEquals(5, buffers[i]._lastFragmentId);
    		assertEquals(4, buffers[i]._messageId);
    		assertEquals(3, buffers[i]._containerType);
    		// release buffer
    		bufferPool.releaseBuffer(buffers[i]);
    	}
    	
    	// count the number of pool buffers and validate it's the same as the number retrieved
    	for (int i = 0; i < bufferPool.NUM_POOLS; i++)
    	{
    		if (bufferPool._pools[i] != null)
    		{
    			poolCount += bufferPool._pools[i].count();
    		}
    	}
    	assertEquals(poolCount, count);

    	// REPEAT AGAIN
    	poolCount = 0;
    	
    	// get buffer and validate initial attributes
    	for (int i = 1; i < count + 1; i++)
    	{
    		TunnelStreamBigBuffer buffer = bufferPool.getBuffer(6144 * i - 100, errorInfo);
    		assertTrue(buffer != null);
    		assertTrue(buffer.isBigBuffer());
    		assertTrue(buffer.data() != null);
    		assertEquals(buffer.data().limit(), 6144 * i - 100);
    		assertEquals(buffer.dataStartPosition(), 0);
    		assertEquals(buffer.length(), 6144 * i - 100);
    		assertEquals(buffer.capacity(), 6144 * i - 100);
    		assertFalse(buffer.fragmentationInProgress());
    		assertEquals(0, buffer._totalMsgLength);
    		assertEquals(0, buffer._bytesRemainingToSend);
    		assertEquals(0, buffer._lastFragmentId);
    		assertEquals(0, buffer._messageId);
    		assertEquals(0, buffer._containerType);
    		buffers[i - 1] = buffer;
    	}
    	
    	// attempt to get another big buffer - this should fail since already retrieved max number specified in count
    	assertTrue(bufferPool.getBuffer(6144, errorInfo) == null);
    	
    	// add content to buffers and validate changes
    	for (int i = 0; i < count; i++)
    	{
    		String content = "Content for buffer number " + i;
    		buffers[i].data().put(content.getBytes());
    		assertEquals(buffers[i].dataStartPosition(), 0);
    		assertEquals(buffers[i].length(), content.length());
    		assertEquals(buffers[i].capacity(), (6144 * (i + 1)) - 100);
    		// validate copy
    		ByteBuffer byteBuffer = ByteBuffer.allocate(buffers[i].length());
    		buffers[i].copy(byteBuffer);
    		for (int j = 0; j < buffers[i].length(); j++)
    		{
    			assertTrue(byteBuffer.get(j) == buffers[i].data().get(j));
    		}
    		buffers[i].saveWriteProgress(9999, 888, 77, 66, 55);
    		assertTrue(buffers[i].fragmentationInProgress());
    		assertEquals(9999, buffers[i]._totalMsgLength);
    		assertEquals(888, buffers[i]._bytesRemainingToSend);
    		assertEquals(77, buffers[i]._lastFragmentId);
    		assertEquals(66, buffers[i]._messageId);
    		assertEquals(55, buffers[i]._containerType);
    		// release buffer
    		bufferPool.releaseBuffer(buffers[i]);
    	}
    	
    	// count the number of pool buffers and validate it's the same as the number retrieved
    	for (int i = 0; i < bufferPool.NUM_POOLS; i++)
    	{
    		if (bufferPool._pools[i] != null)
    		{
    			poolCount += bufferPool._pools[i].count();
    		}
    	}
    	assertEquals(poolCount, count);
    }
    
    /* Tests a successful tunnel stream fragmentation handshake. */
    @Test
    public void tunnelStreamFragmentHandshakeTest()
    {
        /* Test opening a TunnelStream and exchanging a couple messages (consumer to prov, then prov to consumer). */
       
        TunnelStream consTunnelStream;
        TunnelStream provTunnelStream;
        
        /* Setup some sample data. */
        String sampleString = "MARKET preDicTion"; 
        ByteBuffer sampleData = ByteBuffer.allocateDirect(sampleString.length());
        sampleData.put(sampleString.getBytes());
               
        /* Create reactors. */
        TestReactor consumerReactor = new TestReactor();
        TestReactor providerReactor = new TestReactor();
                
        /* Create consumer. */
        Consumer consumer = new Consumer(consumerReactor);
        ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
        consumerRole.initDefaultRDMLoginRequest();
        consumerRole.initDefaultRDMDirectoryRequest();
        consumerRole.channelEventCallback(consumer);
        consumerRole.loginMsgCallback(consumer);
        consumerRole.directoryMsgCallback(consumer);
        consumerRole.dictionaryMsgCallback(consumer);
        consumerRole.defaultMsgCallback(consumer);
       
  
        /* Create provider. */
        TunnelStreamProvider provider = new TunnelStreamProvider(providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.reactorRole();
        providerRole.channelEventCallback(provider);
        providerRole.loginMsgCallback(provider);
        providerRole.directoryMsgCallback(provider);
        providerRole.dictionaryMsgCallback(provider);
        providerRole.defaultMsgCallback(provider);
        providerRole.tunnelStreamListenerCallback(provider);

        /* Connect the consumer and provider. Setup login & directory streams automatically. */
        ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
        opts.setupDefaultLoginStream(true);
        opts.setupDefaultDirectoryStream(true);
        provider.bind(opts);
        TestReactor.openSession(consumer, provider, opts);
        
        /* Open a TunnelStream. */
        TunnelStreamOpenOptions tsOpenOpts = ReactorFactory.createTunnelStreamOpenOptions();
        tsOpenOpts.name("Tunnel1");
        tsOpenOpts.statusEventCallback(consumer);
        tsOpenOpts.queueMsgCallback(consumer);
        tsOpenOpts.defaultMsgCallback(consumer);
        tsOpenOpts.classOfService().dataIntegrity().type(DataIntegrityTypes.RELIABLE);
        tsOpenOpts.streamId(5);
        tsOpenOpts.serviceId(Provider.defaultService().serviceId());
        tsOpenOpts.domainType(DomainTypes.SYSTEM);
        tsOpenOpts.userSpecObject(consumer);     
        tsOpenOpts.classOfService().authentication().type(AuthenticationTypes.OMM_LOGIN);

        Consumer.OpenedTunnelStreamInfo openedTsInfo = consumer.openTunnelStream(provider, tsOpenOpts);
        consTunnelStream = openedTsInfo.consumerTunnelStream();
        provTunnelStream = openedTsInfo.providerTunnelStream();
        
        /* Test tunnel stream accessors */
        assertEquals(5, consTunnelStream.streamId());
        assertEquals(5, provTunnelStream.streamId());
        assertEquals(DomainTypes.SYSTEM, consTunnelStream.domainType());
        assertEquals(DomainTypes.SYSTEM, provTunnelStream.domainType());
        assertEquals(Provider.defaultService().serviceId(), consTunnelStream.serviceId());
        assertEquals(Provider.defaultService().serviceId(), provTunnelStream.serviceId());
        assertEquals(consumer.reactorChannel(), consTunnelStream.reactorChannel());
        assertEquals(provider.reactorChannel(), provTunnelStream.reactorChannel());
        assertEquals(false, consTunnelStream.isProvider());
        assertEquals(true, provTunnelStream.isProvider());
        assertEquals("Tunnel1", consTunnelStream.name());
        assertEquals("Tunnel1", provTunnelStream.name());
        assertEquals(consumer, consTunnelStream.userSpecObject());
        assertEquals(null, provTunnelStream.userSpecObject());
        assertEquals(StreamStates.OPEN, consTunnelStream.state().streamState());
        assertEquals(DataStates.OK, consTunnelStream.state().dataState());
        assertEquals(StateCodes.NONE, consTunnelStream.state().code());
        assertEquals(StreamStates.OPEN, provTunnelStream.state().streamState());
        assertEquals(DataStates.OK, provTunnelStream.state().dataState());
        assertEquals(StateCodes.NONE, provTunnelStream.state().code());

        /* Test consumer tunnel stream class of service values */
        assertEquals(Codec.protocolType(), consTunnelStream.classOfService().common().protocolType());
        assertEquals(Codec.majorVersion(), consTunnelStream.classOfService().common().protocolMajorVersion());
        assertEquals(Codec.minorVersion(), consTunnelStream.classOfService().common().protocolMinorVersion());
        assertEquals(614400, consTunnelStream.classOfService().common().maxMsgSize());
        assertEquals(6144, consTunnelStream.classOfService().common().maxFragmentSize());
        assertEquals(AuthenticationTypes.OMM_LOGIN, consTunnelStream.classOfService().authentication().type());
 
        assertEquals(FlowControlTypes.BIDIRECTIONAL, consTunnelStream.classOfService().flowControl().type());
        assertEquals(TunnelStream.DEFAULT_RECV_WINDOW, consTunnelStream.classOfService().flowControl().sendWindowSize());
        assertEquals(TunnelStream.DEFAULT_RECV_WINDOW, consTunnelStream.classOfService().flowControl().recvWindowSize());
        assertEquals(DataIntegrityTypes.RELIABLE, consTunnelStream.classOfService().dataIntegrity().type());
        assertEquals(GuaranteeTypes.NONE, consTunnelStream.classOfService().guarantee().type());

        /* Test provider tunnel stream class of service values */
        assertEquals(Codec.protocolType(), provTunnelStream.classOfService().common().protocolType());
        assertEquals(Codec.majorVersion(), provTunnelStream.classOfService().common().protocolMajorVersion());
        assertEquals(Codec.minorVersion(), provTunnelStream.classOfService().common().protocolMinorVersion());
        assertEquals(614400, provTunnelStream.classOfService().common().maxMsgSize());
        assertEquals(6144, provTunnelStream.classOfService().common().maxFragmentSize());
        assertEquals(AuthenticationTypes.OMM_LOGIN, provTunnelStream.classOfService().authentication().type());
        assertEquals(FlowControlTypes.BIDIRECTIONAL, provTunnelStream.classOfService().flowControl().type());
        assertEquals(CosCommon.DEFAULT_MAX_FRAGMENT_SIZE, provTunnelStream.classOfService().flowControl().sendWindowSize());
        assertEquals(TunnelStream.DEFAULT_RECV_WINDOW, provTunnelStream.classOfService().flowControl().recvWindowSize());
        assertEquals(DataIntegrityTypes.RELIABLE, provTunnelStream.classOfService().dataIntegrity().type());
        assertEquals(GuaranteeTypes.NONE, provTunnelStream.classOfService().guarantee().type());
      
        consumerReactor.dispatch(0);
        
        /* Provider Reactor internally responds to FIN (no message). */
        providerReactor.dispatch(0);        
 
        TestReactorComponent.closeSession(consumer, provider);
        consumerReactor.close();
        providerReactor.close();
    }
    
    /* Tests a tunnel stream fragmentation handshake reject. */
    @Test
    public void tunnelStreamFragmentHandshakeRejectTest()
    {
        /* Test invalid tunnel stream open request with bad tunnel stream version.
         * Provider should send back StatusMsg with no MsgKey. */
        
        /* Test sends and receives messages at the ReactorChannel level not the
         * TunnelStream level. It's easier to test invalid tunnel stream version
         * this way versus with direct usage of tunnel streams. */ 
        
        ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
        TestReactorEvent event;
        ReactorMsgEvent msgEvent;
        Msg msg = CodecFactory.createMsg();
        RequestMsg requestMsg = (RequestMsg)msg;
        StatusMsg receivedStatusMsg;
        
        /* Create reactors. */
        TestReactor consumerReactor = new TestReactor();
        TestReactor providerReactor = new TestReactor();
                
        /* Create consumer. */
        Consumer consumer = new Consumer(consumerReactor);
        ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
        consumerRole.initDefaultRDMLoginRequest();
        consumerRole.initDefaultRDMDirectoryRequest();
        consumerRole.channelEventCallback(consumer);
        consumerRole.loginMsgCallback(consumer);
        consumerRole.directoryMsgCallback(consumer);
        consumerRole.dictionaryMsgCallback(consumer);
        consumerRole.defaultMsgCallback(consumer);
        consumerRole.watchlistOptions().channelOpenCallback(consumer);
        
        /* Create provider. */
        Provider provider = new Provider(providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.reactorRole();
        providerRole.channelEventCallback(provider);
        providerRole.loginMsgCallback(provider);
        providerRole.directoryMsgCallback(provider);
        providerRole.dictionaryMsgCallback(provider);
        providerRole.defaultMsgCallback(provider);
        
        /* Connect the consumer and provider. Setup login & directory streams automatically. */
        ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
        opts.setupDefaultLoginStream(true);
        opts.setupDefaultDirectoryStream(true);
        provider.bind(opts);
        TestReactor.openSession(consumer, provider, opts);
        
        /* Consumer sends invalid tunnel stream open request with bad tunnel stream version. */
        requestMsg.clear();
        requestMsg.msgClass(MsgClasses.REQUEST);
        requestMsg.streamId(1000);
        requestMsg.domainType(DomainTypes.SYSTEM);
        requestMsg.containerType(DataTypes.FILTER_LIST);
        requestMsg.applyPrivateStream();
        requestMsg.applyQualifiedStream();
        requestMsg.applyStreaming();
        requestMsg.msgKey().applyHasName();
        requestMsg.msgKey().name().data("VAConsumer");
        requestMsg.msgKey().applyHasServiceId();
        requestMsg.msgKey().serviceId(1);
        requestMsg.msgKey().applyHasFilter();
        TunnelStreamOpenOptions tsOpenOpts = ReactorFactory.createTunnelStreamOpenOptions();
        tsOpenOpts.classOfService().dataIntegrity().type(DataIntegrityTypes.RELIABLE);
        requestMsg.msgKey().filter(tsOpenOpts.classOfService().filterFlags());
        EncodeIterator encIter = CodecFactory.createEncodeIterator();
        Buffer buffer = CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(256));
        encIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        assertTrue(requestMsg.encodeInit(encIter, 0) >= CodecReturnCodes.SUCCESS);
        
        // encode class of service as payload
        ElementList elemList = CodecFactory.createElementList();
        ElementEntry elemEntry = CodecFactory.createElementEntry();
        FilterList filterList = CodecFactory.createFilterList();
        FilterEntry filterEntry = CodecFactory.createFilterEntry();
        UInt tempUInt = CodecFactory.createUInt();

        filterList.clear();
        filterList.containerType(DataTypes.ELEMENT_LIST);
        
        assertTrue(filterList.encodeInit(encIter) >= CodecReturnCodes.SUCCESS);
        
        // encode common properties
        filterEntry.clear();
        filterEntry.action(FilterEntryActions.SET);
        filterEntry.applyHasContainerType();
        filterEntry.containerType(DataTypes.ELEMENT_LIST);
        filterEntry.id(ClassesOfService.FilterIds.COMMON_PROPERTIES);

        assertTrue(filterEntry.encodeInit(encIter, 0) >= CodecReturnCodes.SUCCESS);

        elemList.clear();
        elemList.applyHasStandardData();
        assertTrue(elemList.encodeInit(encIter, null, 0) >= CodecReturnCodes.SUCCESS);

        elemEntry.clear();
        elemEntry.name(ClassesOfService.ElementNames.PROTOCOL_TYPE);
        elemEntry.dataType(DataTypes.UINT);
        tempUInt.value(tsOpenOpts.classOfService().common().protocolType());
        assertTrue(elemEntry.encode(encIter, tempUInt) >= CodecReturnCodes.SUCCESS);

        elemEntry.clear();
        elemEntry.name(ClassesOfService.ElementNames.PROTOCOL_MAJOR_VERSION);
        elemEntry.dataType(DataTypes.UINT);
        tempUInt.value(tsOpenOpts.classOfService().common().protocolMajorVersion());
        assertTrue(elemEntry.encode(encIter, tempUInt) >= CodecReturnCodes.SUCCESS);

        elemEntry.clear();
        elemEntry.name(ClassesOfService.ElementNames.PROTOCOL_MINOR_VERSION);
        elemEntry.dataType(DataTypes.UINT);
        tempUInt.value(tsOpenOpts.classOfService().common().protocolMinorVersion());
        assertTrue(elemEntry.encode(encIter, tempUInt) >= CodecReturnCodes.SUCCESS);

        elemEntry.clear();
        elemEntry.name(ClassesOfService.ElementNames.STREAM_VERSION);
        elemEntry.dataType(DataTypes.UINT);
        tempUInt.value(999); // invalid tunnel stream version 
        assertTrue(elemEntry.encode(encIter, tempUInt) >= CodecReturnCodes.SUCCESS);
        assertTrue(elemList.encodeComplete(encIter, true) >= CodecReturnCodes.SUCCESS);
        assertTrue(filterEntry.encodeComplete(encIter, true) >= CodecReturnCodes.SUCCESS);
                
        // encode data integrity
        filterEntry.clear();
        filterEntry.action(FilterEntryActions.SET);
        filterEntry.applyHasContainerType();
        filterEntry.containerType(DataTypes.ELEMENT_LIST);
        filterEntry.id(ClassesOfService.FilterIds.DATA_INTEGRITY);

        assertTrue(filterEntry.encodeInit(encIter, 0) >= CodecReturnCodes.SUCCESS);

        elemList.clear();
        elemList.applyHasStandardData();
        assertTrue(elemList.encodeInit(encIter, null, 0) >= CodecReturnCodes.SUCCESS);
        
        elemEntry.clear();
        elemEntry.name(ClassesOfService.ElementNames.TYPE);
        elemEntry.dataType(DataTypes.UINT);
        tempUInt.value(tsOpenOpts.classOfService().dataIntegrity().type());
        assertTrue(elemEntry.encode(encIter, tempUInt) >= CodecReturnCodes.SUCCESS);

        assertTrue(elemList.encodeComplete(encIter, true) >= CodecReturnCodes.SUCCESS);

        assertTrue(filterEntry.encodeComplete(encIter, true) >= CodecReturnCodes.SUCCESS);
        
        assertTrue(filterList.encodeComplete(encIter, true) >= CodecReturnCodes.SUCCESS);
        assertTrue(requestMsg.encodeComplete(encIter, true) >= CodecReturnCodes.SUCCESS);
        assertTrue(consumer.submitAndDispatch(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
        
        /* Provider receives tunnel stream request with invalid tunnel stream version.
         * It subsequently sends StatusMsg to close tunnel stream. */
        providerReactor.dispatch(1);
        event = providerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
        ReactorChannelEvent channelEvent = (ReactorChannelEvent)event.reactorEvent();
        assertEquals(ReactorChannelEventTypes.WARNING, channelEvent.eventType());
                
        /* Consumer receives Close StatusMsg with no MsgKey. */
        consumerReactor.dispatch(1);
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertNotNull(msgEvent.transportBuffer());
        assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());
        
        receivedStatusMsg = (StatusMsg)msgEvent.msg();
        assertFalse(receivedStatusMsg.checkHasMsgKey());
        assertEquals(DomainTypes.SYSTEM, receivedStatusMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedStatusMsg.containerType());
        assertTrue(receivedStatusMsg.checkClearCache());
        assertTrue(receivedStatusMsg.checkPrivateStream());
        assertTrue(receivedStatusMsg.checkQualifiedStream());
        assertTrue(receivedStatusMsg.checkHasState());
        assertEquals(StreamStates.CLOSED, receivedStatusMsg.state().streamState());
        assertEquals(DataStates.SUSPECT, receivedStatusMsg.state().dataState());
                
        TestReactorComponent.closeSession(consumer, provider);
    }
    
    /* Tests writing of a big buffer to a tunnel stream. */
    @Test
    public void writeBigBufferTest()
    {
        /* Test opening a TunnelStream and exchanging a big buffer. */
        
        TestReactorEvent event;
        TunnelStreamStatusEvent tsStatusEvent;
        TunnelStreamMsgEvent tsMsgEvent;
        TunnelStream consTunnelStream;
        TunnelStream provTunnelStream;
        TransportBuffer buffer;
        
        //TestReactor.enableReactorXmlTracing();
        
        /* Create reactors. */
        TestReactor consumerReactor = new TestReactor();
        TestReactor providerReactor = new TestReactor();
                       
        /* Create consumer. */
        Consumer consumer = new Consumer(consumerReactor);
        ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
        consumerRole.initDefaultRDMLoginRequest();
        consumerRole.initDefaultRDMDirectoryRequest();
        consumerRole.channelEventCallback(consumer);
        consumerRole.loginMsgCallback(consumer);
        consumerRole.directoryMsgCallback(consumer);
        consumerRole.dictionaryMsgCallback(consumer);
        consumerRole.defaultMsgCallback(consumer);
        
        /* Create provider. */
        TunnelStreamProvider provider = new TunnelStreamProvider(providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.reactorRole();
        providerRole.channelEventCallback(provider);
        providerRole.loginMsgCallback(provider);
        providerRole.directoryMsgCallback(provider);
        providerRole.dictionaryMsgCallback(provider);
        providerRole.defaultMsgCallback(provider);
        providerRole.tunnelStreamListenerCallback(provider);

        /* Connect the consumer and provider. Setup login & directory streams automatically. */
        ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
        opts.setupDefaultLoginStream(true);
        opts.setupDefaultDirectoryStream(true);
        provider.bind(opts);
        TestReactor.openSession(consumer, provider, opts);
        
        /* Open a TunnelStream. */
        TunnelStreamOpenOptions tsOpenOpts = ReactorFactory.createTunnelStreamOpenOptions();
        tsOpenOpts.name("Tunnel1");
        tsOpenOpts.statusEventCallback(consumer);
        tsOpenOpts.queueMsgCallback(consumer);
        tsOpenOpts.defaultMsgCallback(consumer);
        tsOpenOpts.classOfService().dataIntegrity().type(DataIntegrityTypes.RELIABLE);
        tsOpenOpts.streamId(5);
        tsOpenOpts.serviceId(Provider.defaultService().serviceId());
        tsOpenOpts.domainType(DomainTypes.SYSTEM);
        tsOpenOpts.userSpecObject(consumer);     

        Consumer.OpenedTunnelStreamInfo openedTsInfo = consumer.openTunnelStream(provider, tsOpenOpts);
        consTunnelStream = openedTsInfo.consumerTunnelStream();
        provTunnelStream = openedTsInfo.providerTunnelStream();
      
        /* Consumer sends a big opaque buffer of 25500 bytes to the provider. */
        _tsSubmitOpts.clear();
        _tsSubmitOpts.containerType(DataTypes.OPAQUE);
        assertNotNull(buffer = consTunnelStream.getBuffer(25500, _errorInfo));
        byte b = 0;
        for (int i = 0; i < 25500; i++)
        {
        	if (b == 256)
        	{
        		b = 0;
        	}
        	buffer.data().put(b++);
        }
        assertEquals(ReactorReturnCodes.SUCCESS, consTunnelStream.submit(buffer, _tsSubmitOpts, _errorInfo));
        consumerReactor.dispatch(0);
        
        providerReactor.dispatch(0);
        consumerReactor.dispatch(0);
      
        providerReactor.dispatch(0);
        consumerReactor.dispatch(0);

        /* Provider receives the buffer. */
        providerReactor.dispatch(1);
        event = providerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.TUNNEL_STREAM_MSG, event.type());
        tsMsgEvent = (TunnelStreamMsgEvent)event.reactorEvent();
        assertEquals(DataTypes.OPAQUE, tsMsgEvent.containerType());
        assertNotNull(buffer = tsMsgEvent.transportBuffer());
        // verify buffer length and contents  
        assertEquals(25500, buffer.length());
        b = 0;
        for (int i = 0; i < 25500; i++)
        {
        	if (b == 256)
        	{
        		b = 0;
        	}            
        	assertEquals(buffer.data().get(i), b++);
        }

        /* Provider closes the tunnel stream, starting FIN/ACK teardown */
        provTunnelStream.close(false, _errorInfo);
        providerReactor.dispatch(0);
        
        /* Consumer receives the open/suspect event */
        consumerReactor.dispatch(1);
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.TUNNEL_STREAM_STATUS, event.type());
        tsStatusEvent = (TunnelStreamStatusEvent)event.reactorEvent();
        assertEquals(null, tsStatusEvent.authInfo());
        assertEquals(consumer.reactorChannel(), tsStatusEvent.reactorChannel());
        assertEquals(StreamStates.OPEN, tsStatusEvent.state().streamState());
        assertEquals(DataStates.SUSPECT, tsStatusEvent.state().dataState());
        assertEquals(StateCodes.NONE, tsStatusEvent.state().code());
        assertNotNull(tsStatusEvent.tunnelStream());
        consumerReactor.dispatch(0);
        
        /* Provider Reactor internally responds to FIN (no message). */
        providerReactor.dispatch(0);
        
        /* Consumer receives the close event */
        consumerReactor.dispatch(1);
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.TUNNEL_STREAM_STATUS, event.type());
        tsStatusEvent = (TunnelStreamStatusEvent)event.reactorEvent();
        assertEquals(null, tsStatusEvent.authInfo());
        assertEquals(consumer.reactorChannel(), tsStatusEvent.reactorChannel());
        assertEquals(StreamStates.CLOSED, tsStatusEvent.state().streamState());
        assertEquals(DataStates.SUSPECT, tsStatusEvent.state().dataState());
        assertEquals(StateCodes.NONE, tsStatusEvent.state().code());
        assertNotNull(tsStatusEvent.tunnelStream());
        consumerReactor.dispatch(0);
        
        TestReactorComponent.closeSession(consumer, provider);
        consumerReactor.close();
        providerReactor.close();
    }

    /* Tests writing of a big message to a tunnel stream. */
    @Test
    public void writeBigMessageTest()
    {
    	/* Test opening a TunnelStream and exchanging a big message. */
        
        TestReactorEvent event;
        TunnelStreamMsgEvent tsMsgEvent;
        TunnelStream consTunnelStream;
        Buffer buffer;
        TransportBuffer receiveBuffer;
        
        /* Create reactors. */
        TestReactor consumerReactor = new TestReactor();
        TestReactor providerReactor = new TestReactor();
                
        /* Create consumer. */
        Consumer consumer = new Consumer(consumerReactor);
        ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
        consumerRole.initDefaultRDMLoginRequest();
        consumerRole.initDefaultRDMDirectoryRequest();
        consumerRole.channelEventCallback(consumer);
        consumerRole.loginMsgCallback(consumer);
        consumerRole.directoryMsgCallback(consumer);
        consumerRole.dictionaryMsgCallback(consumer);
        consumerRole.defaultMsgCallback(consumer);
        
        /* Create provider. */
        TunnelStreamProvider provider = new TunnelStreamProvider(providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.reactorRole();
        providerRole.channelEventCallback(provider);
        providerRole.loginMsgCallback(provider);
        providerRole.directoryMsgCallback(provider);
        providerRole.dictionaryMsgCallback(provider);
        providerRole.defaultMsgCallback(provider);
        providerRole.tunnelStreamListenerCallback(provider);

        /* Connect the consumer and provider. Setup login & directory streams automatically. */
        ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
        opts.setupDefaultLoginStream(true);
        opts.setupDefaultDirectoryStream(true);
        provider.bind(opts);
        TestReactor.openSession(consumer, provider, opts);
        
        /* Open a TunnelStream. */
        TunnelStreamOpenOptions tsOpenOpts = ReactorFactory.createTunnelStreamOpenOptions();
        tsOpenOpts.name("Tunnel1");
        tsOpenOpts.statusEventCallback(consumer);
        tsOpenOpts.queueMsgCallback(consumer);
        tsOpenOpts.defaultMsgCallback(consumer);
        tsOpenOpts.classOfService().dataIntegrity().type(DataIntegrityTypes.RELIABLE);
        tsOpenOpts.streamId(5);
        tsOpenOpts.serviceId(Provider.defaultService().serviceId());
        tsOpenOpts.domainType(DomainTypes.SYSTEM);
        tsOpenOpts.userSpecObject(consumer);     

        Consumer.OpenedTunnelStreamInfo openedTsInfo = consumer.openTunnelStream(provider, tsOpenOpts);
        consTunnelStream = openedTsInfo.consumerTunnelStream();
      
        /* Consumer sends a big message with big encoded data to the provider. */
        GenericMsg genericMsg = (GenericMsg)CodecFactory.createMsg();
        genericMsg.msgClass(MsgClasses.GENERIC);
        genericMsg.domainType(DomainTypes.SYSTEM);
        genericMsg.containerType(DataTypes.OPAQUE);
        genericMsg.applyMessageComplete();
        buffer = CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(25500));
        
        byte b = 0;
        for (int i = 0; i < 25500; i++)
        {
        	if (b == 256)
        	{
        		b = 0;
        	}
        	buffer.data().put(b++);
        }
        genericMsg.encodedDataBody(buffer);
        assertEquals(ReactorReturnCodes.SUCCESS, consTunnelStream.submit(genericMsg, _errorInfo));
        
        consumerReactor.dispatch(0);
        providerReactor.dispatch(0);
        consumerReactor.dispatch(0);
        providerReactor.dispatch(0);
        consumerReactor.dispatch(0);

        /* Provider receives the buffer. */
        providerReactor.dispatch(1);
        event = providerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.TUNNEL_STREAM_MSG, event.type());
        tsMsgEvent = (TunnelStreamMsgEvent)event.reactorEvent();
        assertEquals(DataTypes.MSG, tsMsgEvent.containerType());
        assertNotNull(receiveBuffer = tsMsgEvent.transportBuffer());

        // verify receiveBuffer length and contents
        int headerLength = receiveBuffer.data().getShort(0) + 2;
        assertEquals(25500 + headerLength, receiveBuffer.length());
        b = 0;
  
        for (int i = headerLength; i < 25500 + headerLength; i++)
        {
        	if (b == 256)
        	{
        		b = 0;
        	}            
        	assertEquals(receiveBuffer.data().get(i), b++);
        }
        
        // verify message is correct
        Msg msg = tsMsgEvent.msg();
        assertNotNull(msg);
        assertEquals(25500, msg.encodedDataBody().length());
        b = 0;
        for (int i = 0; i < 25500; i++)
        {
        	if (b == 256)
        	{
        		b = 0;
        	}            
        	assertEquals(msg.encodedDataBody().data().get(i), b++);
        }
        
        TestReactorComponent.closeSession(consumer, provider);
        consumerReactor.close();
        providerReactor.close();
    }
    
    /* Tests writing of a 6130 byte big buffer to a tunnel stream. */
    @Test
    public void writeMessage6130Test()
    {
    	/* Test opening a TunnelStream and exchanging a big message. */
        
        TestReactorEvent event;
        TunnelStreamMsgEvent tsMsgEvent;
        TunnelStream consTunnelStream;
        Buffer buffer;
        TransportBuffer receiveBuffer;
        
        /* Create reactors. */
        TestReactor consumerReactor = new TestReactor();
        TestReactor providerReactor = new TestReactor();
                
        /* Create consumer. */
        Consumer consumer = new Consumer(consumerReactor);
        ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
        consumerRole.initDefaultRDMLoginRequest();
        consumerRole.initDefaultRDMDirectoryRequest();
        consumerRole.channelEventCallback(consumer);
        consumerRole.loginMsgCallback(consumer);
        consumerRole.directoryMsgCallback(consumer);
        consumerRole.dictionaryMsgCallback(consumer);
        consumerRole.defaultMsgCallback(consumer);
        
        /* Create provider. */
        TunnelStreamProvider provider = new TunnelStreamProvider(providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.reactorRole();
        providerRole.channelEventCallback(provider);
        providerRole.loginMsgCallback(provider);
        providerRole.directoryMsgCallback(provider);
        providerRole.dictionaryMsgCallback(provider);
        providerRole.defaultMsgCallback(provider);
        providerRole.tunnelStreamListenerCallback(provider);

        /* Connect the consumer and provider. Setup login & directory streams automatically. */
        ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
        opts.setupDefaultLoginStream(true);
        opts.setupDefaultDirectoryStream(true);
        provider.bind(opts);
        TestReactor.openSession(consumer, provider, opts);
        
        /* Open a TunnelStream. */
        TunnelStreamOpenOptions tsOpenOpts = ReactorFactory.createTunnelStreamOpenOptions();
        tsOpenOpts.name("Tunnel1");
        tsOpenOpts.statusEventCallback(consumer);
        tsOpenOpts.queueMsgCallback(consumer);
        tsOpenOpts.defaultMsgCallback(consumer);
        tsOpenOpts.classOfService().dataIntegrity().type(DataIntegrityTypes.RELIABLE);
        tsOpenOpts.streamId(5);
        tsOpenOpts.serviceId(Provider.defaultService().serviceId());
        tsOpenOpts.domainType(DomainTypes.SYSTEM);
        tsOpenOpts.userSpecObject(consumer);     

        Consumer.OpenedTunnelStreamInfo openedTsInfo = consumer.openTunnelStream(provider, tsOpenOpts);
        consTunnelStream = openedTsInfo.consumerTunnelStream();
      
        /* Consumer sends a big message with big encoded data to the provider. */
        GenericMsg genericMsg = (GenericMsg)CodecFactory.createMsg();
        genericMsg.msgClass(MsgClasses.GENERIC);
        genericMsg.domainType(DomainTypes.SYSTEM);
        genericMsg.containerType(DataTypes.OPAQUE);
        genericMsg.applyMessageComplete();
        buffer = CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(6130));
        
        byte b = 0;
        for (int i = 0; i < 6130; i++)
        {
        	if (b == 256)
        	{
        		b = 0;
        	}
        	buffer.data().put(b++);
        }
        genericMsg.encodedDataBody(buffer);
        assertEquals(ReactorReturnCodes.SUCCESS, consTunnelStream.submit(genericMsg, _errorInfo));
        
        consumerReactor.dispatch(0);

        /* Provider receives the buffer. */
        providerReactor.dispatch(1);
        event = providerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.TUNNEL_STREAM_MSG, event.type());
        tsMsgEvent = (TunnelStreamMsgEvent)event.reactorEvent();
        assertEquals(DataTypes.MSG, tsMsgEvent.containerType());
        assertNotNull(receiveBuffer = tsMsgEvent.transportBuffer());

        // verify receiveBuffer length and contents
        int headerLength = receiveBuffer.data().getShort(0) + 2;
        assertEquals(6130 + headerLength, receiveBuffer.length());
        b = 0;
  
        for (int i = headerLength; i < 6130 + headerLength; i++)
        {
        	if (b == 256)
        	{
        		b = 0;
        	}            
        	assertEquals(receiveBuffer.data().get(i), b++);
        }
        
        // verify message is correct
        Msg msg = tsMsgEvent.msg();
        assertNotNull(msg);
        assertEquals(6130, msg.encodedDataBody().length());
        b = 0;
        for (int i = 0; i < 6130; i++)
        {
        	if (b == 256)
        	{
        		b = 0;
        	}            
        	assertEquals(msg.encodedDataBody().data().get(i), b++);
        }
        
        TestReactorComponent.closeSession(consumer, provider);
        consumerReactor.close();
        providerReactor.close();
    }

    /* Tests writing of multiple big buffers to a tunnel stream. */
    @Test
    public void writeBigBufferFragmentTest()
    {
        /* Test fragmenting buffers. */
        
        TestReactorEvent event;
        TunnelStreamStatusEvent tsStatusEvent;
        TunnelStream consTunnelStream;
        TunnelStream provTunnelStream;
        TransportBuffer buffer;
        
        /* Create reactors. */
        TestReactor consumerReactor = new TestReactor();
        TestReactor providerReactor = new TestReactor();
                
        /* Create consumer. */
        Consumer consumer = new Consumer(consumerReactor);
        ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
        consumerRole.initDefaultRDMLoginRequest();
        consumerRole.initDefaultRDMDirectoryRequest();
        consumerRole.channelEventCallback(consumer);
        consumerRole.loginMsgCallback(consumer);
        consumerRole.directoryMsgCallback(consumer);
        consumerRole.dictionaryMsgCallback(consumer);
        consumerRole.defaultMsgCallback(consumer);
        
        /* Create provider. */
        TunnelStreamProvider provider = new TunnelStreamProvider(providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.reactorRole();
        providerRole.channelEventCallback(provider);
        providerRole.loginMsgCallback(provider);
        providerRole.directoryMsgCallback(provider);
        providerRole.dictionaryMsgCallback(provider);
        providerRole.defaultMsgCallback(provider);
        providerRole.tunnelStreamListenerCallback(provider);

        /* Connect the consumer and provider. Setup login & directory streams automatically. */
        ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
        opts.setupDefaultLoginStream(true);
        opts.setupDefaultDirectoryStream(true);
        provider.bind(opts);
        TestReactor.openSession(consumer, provider, opts);
        
        /* Open a TunnelStream. */
        TunnelStreamOpenOptions tsOpenOpts = ReactorFactory.createTunnelStreamOpenOptions();
        tsOpenOpts.name("Tunnel1");
        tsOpenOpts.statusEventCallback(consumer);
        tsOpenOpts.queueMsgCallback(consumer);
        tsOpenOpts.defaultMsgCallback(consumer);
        tsOpenOpts.classOfService().dataIntegrity().type(DataIntegrityTypes.RELIABLE);
        tsOpenOpts.streamId(5);
        tsOpenOpts.serviceId(Provider.defaultService().serviceId());
        tsOpenOpts.domainType(DomainTypes.SYSTEM);
        tsOpenOpts.userSpecObject(consumer);     

        Consumer.OpenedTunnelStreamInfo openedTsInfo = consumer.openTunnelStream(provider, tsOpenOpts);
        consTunnelStream = openedTsInfo.consumerTunnelStream();
        provTunnelStream = openedTsInfo.providerTunnelStream();
      
        /* Consumer sends two big opaque buffers of 25500 bytes to the provider. */
        _tsSubmitOpts.clear();
        _tsSubmitOpts.containerType(DataTypes.XML);
        assertNotNull(buffer = consTunnelStream.getBuffer(25500, _errorInfo));
        for (int i = 0, b = 0; i < 25500; i++)
        {
        	if (b == 256)
        	{
        		b = 0;
        	}
        	buffer.data().put((byte)b++);
        }
        assertEquals(ReactorReturnCodes.SUCCESS, consTunnelStream.submit(buffer, _tsSubmitOpts, _errorInfo));
        assertNotNull(buffer = consTunnelStream.getBuffer(25500, _errorInfo));
        for (int i = 0, b = 0; i < 25500; i++)
        {
        	if (b == 256)
        	{
        		b = 0;
        	}
        	buffer.data().put((byte)b++);
        }
        assertEquals(ReactorReturnCodes.SUCCESS, consTunnelStream.submit(buffer, _tsSubmitOpts, _errorInfo));
        
        AckRangeList ackRangeList = new AckRangeList();
        AckRangeList nakRangeList = new AckRangeList();
        int numberOfFragments = 25500/consTunnelStream._classOfService.common().maxFragmentSize() + 1;
        int fragmentNumber = 0;
        int messageId = 1;
        
        // check _outboundTransmitList of tunnel stream and make sure fragments are there and in correct order
        for(TunnelStreamBuffer tunnelStreamBuffer = consTunnelStream._outboundTransmitList.start(TunnelStreamBuffer.RETRANS_LINK); 
	            tunnelStreamBuffer != null;
	            tunnelStreamBuffer = consTunnelStream._outboundTransmitList.forth(TunnelStreamBuffer.RETRANS_LINK))
	    {
	        DecodeIterator decodeIterator = CodecFactory.createDecodeIterator();
	        Msg msg = CodecFactory.createMsg();
	        assertEquals(CodecReturnCodes.SUCCESS, decodeIterator.setBufferAndRWFVersion(tunnelStreamBuffer, Codec.majorVersion(), Codec.minorVersion()));
	        assertEquals(CodecReturnCodes.SUCCESS, msg.decode(decodeIterator));
	        TunnelStreamMsg tunnelMsg = new TunnelStreamMsgImpl();
	        assertEquals(MsgClasses.GENERIC, msg.msgClass());
	        assertEquals(DataTypes.OPAQUE, msg.containerType());
	        assertEquals(CodecReturnCodes.SUCCESS, tunnelMsg.decode(decodeIterator, (GenericMsg)msg, ackRangeList, nakRangeList));
	        assertEquals(TunnelStreamMsg.OpCodes.DATA, tunnelMsg.opCode());
	        TunnelStreamMsg.TunnelStreamData dataHeader = (TunnelStreamMsg.TunnelStreamData)tunnelMsg; 
	        assertEquals(1, dataHeader.dataMsgFlag());
	        assertEquals(25500, dataHeader.totalMsgLength());
	        fragmentNumber++;
	        if (fragmentNumber < numberOfFragments)
	        {
	        	assertFalse(((GenericMsg)msg).checkMessageComplete());
	        }
	        else if (fragmentNumber > numberOfFragments)
	        {
	        	fragmentNumber = 1;
	        	messageId++;
	        }
	        else // fragmentNumber == numberOfFragments
	        {
	        	assertTrue(((GenericMsg)msg).checkMessageComplete());
	        }
	        assertEquals(fragmentNumber, dataHeader.fragmentNumber());
	        assertEquals(messageId, dataHeader.messageId());
	        assertEquals(DataTypes.XML, dataHeader.containerType());
	        int bufferLength, headerLength;
	        headerLength = tunnelStreamBuffer.tunnelStreamHeaderLen();
	        if (fragmentNumber < numberOfFragments)
	        {
	        	bufferLength = 6144 + headerLength;
	        }
	        else
	        {
	        	bufferLength = 924 + headerLength;
	        }
	        int tunnelStreamBufferLength = tunnelStreamBuffer.length();
	        assertEquals(bufferLength, tunnelStreamBufferLength);
	        byte b = 0;
	        for (int i = headerLength; i < bufferLength; i++)
	        {
	        	assertEquals(tunnelStreamBuffer.data().get(i), b++);
	        }
	    }
        consTunnelStream._outboundTransmitList.clear();
        
        /* Provider sends two big opaque buffers of 25500 bytes to the consumer. */
        _tsSubmitOpts.clear();
        _tsSubmitOpts.containerType(DataTypes.XML);
        assertNotNull(buffer = provTunnelStream.getBuffer(25500, _errorInfo));
        for (int i = 0, b = 0; i < 25500; i++)
        {
        	if (b == 256)
        	{
        		b = 0;
        	}
        	buffer.data().put((byte)b++);
        }
        assertEquals(ReactorReturnCodes.SUCCESS, provTunnelStream.submit(buffer, _tsSubmitOpts, _errorInfo));
        assertNotNull(buffer = provTunnelStream.getBuffer(25500, _errorInfo));
        for (int i = 0, b = 0; i < 25500; i++)
        {
        	if (b == 256)
        	{
        		b = 0;
        	}
        	buffer.data().put((byte)b++);
        }
        assertEquals(ReactorReturnCodes.SUCCESS, provTunnelStream.submit(buffer, _tsSubmitOpts, _errorInfo));
        
        numberOfFragments = 25500/provTunnelStream._classOfService.common().maxFragmentSize() + 1;
        fragmentNumber = 0;
        messageId = 1;
        
        // check _outboundTransmitList of tunnel stream and make sure fragments are there and in correct order
        for(TunnelStreamBuffer tunnelStreamBuffer = provTunnelStream._outboundTransmitList.start(TunnelStreamBuffer.RETRANS_LINK); 
	            tunnelStreamBuffer != null;
	            tunnelStreamBuffer = provTunnelStream._outboundTransmitList.forth(TunnelStreamBuffer.RETRANS_LINK))
	    {
	        DecodeIterator decodeIterator = CodecFactory.createDecodeIterator();
	        Msg msg = CodecFactory.createMsg();
	        assertEquals(CodecReturnCodes.SUCCESS, decodeIterator.setBufferAndRWFVersion(tunnelStreamBuffer, Codec.majorVersion(), Codec.minorVersion()));
	        assertEquals(CodecReturnCodes.SUCCESS, msg.decode(decodeIterator));
	        TunnelStreamMsg tunnelMsg = new TunnelStreamMsgImpl();
	        assertEquals(MsgClasses.GENERIC, msg.msgClass());
	        assertEquals(DataTypes.OPAQUE, msg.containerType());
	        assertEquals(CodecReturnCodes.SUCCESS, tunnelMsg.decode(decodeIterator, (GenericMsg)msg, ackRangeList, nakRangeList));
	        assertEquals(TunnelStreamMsg.OpCodes.DATA, tunnelMsg.opCode());
	        TunnelStreamMsg.TunnelStreamData dataHeader = (TunnelStreamMsg.TunnelStreamData)tunnelMsg; 
	        assertEquals(1, dataHeader.dataMsgFlag());
	        assertEquals(25500, dataHeader.totalMsgLength());
	        fragmentNumber++;
	        if (fragmentNumber < numberOfFragments)
	        {
	        	assertFalse(((GenericMsg)msg).checkMessageComplete());
	        }
	        else if (fragmentNumber > numberOfFragments)
	        {
	        	fragmentNumber = 1;
	        	messageId++;
	        }
	        else // fragmentNumber == numberOfFragments
	        {
	        	assertTrue(((GenericMsg)msg).checkMessageComplete());
	        }
	        assertEquals(fragmentNumber, dataHeader.fragmentNumber());
	        assertEquals(messageId, dataHeader.messageId());
	        assertEquals(DataTypes.XML, dataHeader.containerType());
	        int bufferLength, headerLength;
	        headerLength = tunnelStreamBuffer.tunnelStreamHeaderLen();
	        if (fragmentNumber < numberOfFragments)
	        {
	        	bufferLength = 6144 + headerLength;
	        }
	        else
	        {
	        	bufferLength = 924 + headerLength;
	        }
	        int tunnelStreamBufferLength = tunnelStreamBuffer.length();
	        assertEquals(bufferLength, tunnelStreamBufferLength);
	        byte b = 0;
	        for (int i = headerLength; i < bufferLength; i++)
	        {
	        	assertEquals(tunnelStreamBuffer.data().get(i), b++);
	        }
	    }
        provTunnelStream._outboundTransmitList.clear();

        /* Provider closes the tunnel stream, starting FIN/ACK teardown */
        provTunnelStream.close(false, _errorInfo);
        providerReactor.dispatch(0);
        
        /* Consumer receives the open/suspect event */
        consumerReactor.dispatch(1);
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.TUNNEL_STREAM_STATUS, event.type());
        tsStatusEvent = (TunnelStreamStatusEvent)event.reactorEvent();
        assertEquals(null, tsStatusEvent.authInfo());
        assertEquals(consumer.reactorChannel(), tsStatusEvent.reactorChannel());
        assertEquals(StreamStates.OPEN, tsStatusEvent.state().streamState());
        assertEquals(DataStates.SUSPECT, tsStatusEvent.state().dataState());
        assertEquals(StateCodes.NONE, tsStatusEvent.state().code());
        assertNotNull(tsStatusEvent.tunnelStream());
        consumerReactor.dispatch(0);
        
        /* Provider Reactor internally responds to FIN (no message). */
        providerReactor.dispatch(0);
        
        /* Consumer receives the close event */
        consumerReactor.dispatch(1);
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.TUNNEL_STREAM_STATUS, event.type());
        tsStatusEvent = (TunnelStreamStatusEvent)event.reactorEvent();
        assertEquals(null, tsStatusEvent.authInfo());
        assertEquals(consumer.reactorChannel(), tsStatusEvent.reactorChannel());
        assertEquals(StreamStates.CLOSED, tsStatusEvent.state().streamState());
        assertEquals(DataStates.SUSPECT, tsStatusEvent.state().dataState());
        assertEquals(StateCodes.NONE, tsStatusEvent.state().code());
        assertNotNull(tsStatusEvent.tunnelStream());
        consumerReactor.dispatch(0);
        
        TestReactorComponent.closeSession(consumer, provider);
        consumerReactor.close();
        providerReactor.close();
    }

    /* Tests writing of multiple big messages to a tunnel stream. */
    @Test
    public void writeBigMessageFragmentTest()
    {
    	/* Test fragmenting messages. */
        
        TestReactorEvent event;
        TunnelStreamStatusEvent tsStatusEvent;
        TunnelStream consTunnelStream;
        TunnelStream provTunnelStream;
        Buffer buffer;
        
        /* Create reactors. */
        TestReactor consumerReactor = new TestReactor();
        TestReactor providerReactor = new TestReactor();
                
        /* Create consumer. */
        Consumer consumer = new Consumer(consumerReactor);
        ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
        consumerRole.initDefaultRDMLoginRequest();
        consumerRole.initDefaultRDMDirectoryRequest();
        consumerRole.channelEventCallback(consumer);
        consumerRole.loginMsgCallback(consumer);
        consumerRole.directoryMsgCallback(consumer);
        consumerRole.dictionaryMsgCallback(consumer);
        consumerRole.defaultMsgCallback(consumer);
        
        /* Create provider. */
        TunnelStreamProvider provider = new TunnelStreamProvider(providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.reactorRole();
        providerRole.channelEventCallback(provider);
        providerRole.loginMsgCallback(provider);
        providerRole.directoryMsgCallback(provider);
        providerRole.dictionaryMsgCallback(provider);
        providerRole.defaultMsgCallback(provider);
        providerRole.tunnelStreamListenerCallback(provider);

        /* Connect the consumer and provider. Setup login & directory streams automatically. */
        ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
        opts.setupDefaultLoginStream(true);
        opts.setupDefaultDirectoryStream(true);
        provider.bind(opts);
        TestReactor.openSession(consumer, provider, opts);
        
        /* Open a TunnelStream. */
        TunnelStreamOpenOptions tsOpenOpts = ReactorFactory.createTunnelStreamOpenOptions();
        tsOpenOpts.name("Tunnel1");
        tsOpenOpts.statusEventCallback(consumer);
        tsOpenOpts.queueMsgCallback(consumer);
        tsOpenOpts.defaultMsgCallback(consumer);
        tsOpenOpts.classOfService().dataIntegrity().type(DataIntegrityTypes.RELIABLE);
        tsOpenOpts.streamId(5);
        tsOpenOpts.serviceId(Provider.defaultService().serviceId());
        tsOpenOpts.domainType(DomainTypes.SYSTEM);
        tsOpenOpts.userSpecObject(consumer);     

        Consumer.OpenedTunnelStreamInfo openedTsInfo = consumer.openTunnelStream(provider, tsOpenOpts);
        consTunnelStream = openedTsInfo.consumerTunnelStream();
        provTunnelStream = openedTsInfo.providerTunnelStream();
      
        /* Consumer sends 2 big messages with big encoded data to the provider. */
        GenericMsg genericMsg = (GenericMsg)CodecFactory.createMsg();
        genericMsg.msgClass(MsgClasses.GENERIC);
        genericMsg.domainType(DomainTypes.SYSTEM);
        genericMsg.containerType(DataTypes.OPAQUE);
        genericMsg.applyMessageComplete();
        buffer = CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(25500));
        for (int i = 0, b = 0; i < 25500; i++)
        {
        	if (b == 256)
        	{
        		b = 0;
        	}
        	buffer.data().put((byte)b++);
        }
        genericMsg.encodedDataBody(buffer);
        assertEquals(ReactorReturnCodes.SUCCESS, consTunnelStream.submit(genericMsg, _errorInfo));
        GenericMsg genericMsg2 = (GenericMsg)CodecFactory.createMsg();
        genericMsg2.msgClass(MsgClasses.GENERIC);
        genericMsg2.domainType(DomainTypes.SYSTEM);
        genericMsg2.containerType(DataTypes.OPAQUE);
        genericMsg2.applyMessageComplete();
        Buffer buffer2 = CodecFactory.createBuffer();
        buffer2.data(ByteBuffer.allocate(25500));
        for (int i = 0, b = 0; i < 25500; i++)
        {
        	if (b == 256)
        	{
        		b = 0;
        	}
        	buffer2.data().put(i, (byte)b++);
        }
        genericMsg2.encodedDataBody(buffer2);
        assertEquals(ReactorReturnCodes.SUCCESS, consTunnelStream.submit(genericMsg2, _errorInfo));
        
        AckRangeList ackRangeList = new AckRangeList();
        AckRangeList nakRangeList = new AckRangeList();
        int numberOfFragments = 25500/consTunnelStream._classOfService.common().maxFragmentSize() + 1;
        int fragmentNumber = 0;
        int messageId = 1;
        int msgHeaderLength = 0;
        byte b = 0;
        
        // check _outboundTransmitList of tunnel stream and make sure fragments are there and in correct order
        for(TunnelStreamBuffer tunnelStreamBuffer = consTunnelStream._outboundTransmitList.start(TunnelStreamBuffer.RETRANS_LINK); 
	            tunnelStreamBuffer != null;
	            tunnelStreamBuffer = consTunnelStream._outboundTransmitList.forth(TunnelStreamBuffer.RETRANS_LINK))
	    {
	        DecodeIterator decodeIterator = CodecFactory.createDecodeIterator();
	        Msg msg = CodecFactory.createMsg();
	        assertEquals(CodecReturnCodes.SUCCESS, decodeIterator.setBufferAndRWFVersion(tunnelStreamBuffer, Codec.majorVersion(), Codec.minorVersion()));
	        assertEquals(CodecReturnCodes.SUCCESS, msg.decode(decodeIterator));
	        TunnelStreamMsg tunnelMsg = new TunnelStreamMsgImpl();
	        assertEquals(MsgClasses.GENERIC, msg.msgClass());
	        assertEquals(DataTypes.OPAQUE, msg.containerType());
	        assertEquals(CodecReturnCodes.SUCCESS, tunnelMsg.decode(decodeIterator, (GenericMsg)msg, ackRangeList, nakRangeList));
	        assertEquals(TunnelStreamMsg.OpCodes.DATA, tunnelMsg.opCode());
	        TunnelStreamMsg.TunnelStreamData dataHeader = (TunnelStreamMsg.TunnelStreamData)tunnelMsg; 
	        assertEquals(1, dataHeader.dataMsgFlag());
	        int headerLength = tunnelStreamBuffer.tunnelStreamHeaderLen();
	        fragmentNumber++;
	        if (fragmentNumber == 1)
	        {
	        	msgHeaderLength = tunnelStreamBuffer.data().getShort(headerLength) + 2;
	        }
	        assertEquals(25500 + msgHeaderLength, dataHeader.totalMsgLength());
	        if (fragmentNumber < numberOfFragments)
	        {
	        	assertFalse(((GenericMsg)msg).checkMessageComplete());
	        }
	        else if (fragmentNumber > numberOfFragments)
	        {
	        	fragmentNumber = 1;
	        	messageId++;
	        }
	        else // fragmentNumber == numberOfFragments
	        {
	        	assertTrue(((GenericMsg)msg).checkMessageComplete());
	        }
	        assertEquals(fragmentNumber, dataHeader.fragmentNumber());
	        assertEquals(messageId, dataHeader.messageId());
	        assertEquals(DataTypes.MSG, dataHeader.containerType());
	        int bufferLength, extraHeaderLength;
        	if (fragmentNumber == 1)
        	{
        		extraHeaderLength = msgHeaderLength;
        		b = 0;
        	}
        	else
        	{
        		extraHeaderLength = 0;
        	}
	        if (fragmentNumber < numberOfFragments)
	        {
	        	bufferLength = 6144 + headerLength;
	        }
	        else
	        {
	        	bufferLength = 924 + headerLength + msgHeaderLength;
	        }
	        int tunnelStreamBufferLength = tunnelStreamBuffer.length();
	        assertEquals(bufferLength, tunnelStreamBufferLength);
	        for (int i = headerLength + extraHeaderLength; i < bufferLength; i++)
	        {
	        	byte readByte = tunnelStreamBuffer.data().get(i);
	        	assertEquals(readByte, b++);
	        }
	    }
        consTunnelStream._outboundTransmitList.clear();

        /* Provider sends 2 big messages with big encoded data to the consumer. */
        GenericMsg genericMsg3 = (GenericMsg)CodecFactory.createMsg();
        genericMsg3.msgClass(MsgClasses.GENERIC);
        genericMsg3.domainType(DomainTypes.SYSTEM);
        genericMsg3.containerType(DataTypes.OPAQUE);
        genericMsg3.applyMessageComplete();
        Buffer buffer3 = CodecFactory.createBuffer();
        buffer3.data(ByteBuffer.allocate(25500));
        b = 0;
        for (int i = 0; i < 25500; i++)
        {
        	if (b == 256)
        	{
        		b = 0;
        	}
        	buffer3.data().put(i, b++);
        }
        genericMsg3.encodedDataBody(buffer3);
        assertEquals(ReactorReturnCodes.SUCCESS, provTunnelStream.submit(genericMsg3, _errorInfo));
        GenericMsg genericMsg4 = (GenericMsg)CodecFactory.createMsg();
        genericMsg4.msgClass(MsgClasses.GENERIC);
        genericMsg4.domainType(DomainTypes.SYSTEM);
        genericMsg4.containerType(DataTypes.OPAQUE);
        genericMsg4.applyMessageComplete();
        Buffer buffer4 = CodecFactory.createBuffer();
        buffer4.data(ByteBuffer.allocate(25500));
        b = 0;
        for (int i = 0; i < 25500; i++)
        {
        	if (b == 256)
        	{
        		b = 0;
        	}
        	buffer4.data().put(i, b++);
        }
        genericMsg4.encodedDataBody(buffer4);
        assertEquals(ReactorReturnCodes.SUCCESS, provTunnelStream.submit(genericMsg4, _errorInfo));
        
        ackRangeList = new AckRangeList();
        nakRangeList = new AckRangeList();
        numberOfFragments = 25500/provTunnelStream._classOfService.common().maxFragmentSize() + 1;
        fragmentNumber = 0;
        messageId = 1;
        msgHeaderLength = 0;
        b = 0;
        
        // check _outboundTransmitList of tunnel stream and make sure fragments are there and in correct order
        for(TunnelStreamBuffer tunnelStreamBuffer = provTunnelStream._outboundTransmitList.start(TunnelStreamBuffer.RETRANS_LINK); 
	            tunnelStreamBuffer != null;
	            tunnelStreamBuffer = provTunnelStream._outboundTransmitList.forth(TunnelStreamBuffer.RETRANS_LINK))
	    {
	        DecodeIterator decodeIterator = CodecFactory.createDecodeIterator();
	        Msg msg = CodecFactory.createMsg();
	        assertEquals(CodecReturnCodes.SUCCESS, decodeIterator.setBufferAndRWFVersion(tunnelStreamBuffer, Codec.majorVersion(), Codec.minorVersion()));
	        assertEquals(CodecReturnCodes.SUCCESS, msg.decode(decodeIterator));
	        TunnelStreamMsg tunnelMsg = new TunnelStreamMsgImpl();
	        assertEquals(MsgClasses.GENERIC, msg.msgClass());
	        assertEquals(DataTypes.OPAQUE, msg.containerType());
	        assertEquals(CodecReturnCodes.SUCCESS, tunnelMsg.decode(decodeIterator, (GenericMsg)msg, ackRangeList, nakRangeList));
	        assertEquals(TunnelStreamMsg.OpCodes.DATA, tunnelMsg.opCode());
	        TunnelStreamMsg.TunnelStreamData dataHeader = (TunnelStreamMsg.TunnelStreamData)tunnelMsg; 
	        assertEquals(1, dataHeader.dataMsgFlag());
	        int headerLength = tunnelStreamBuffer.tunnelStreamHeaderLen();
	        fragmentNumber++;
	        if (fragmentNumber == 1)
	        {
	        	msgHeaderLength = tunnelStreamBuffer.data().getShort(headerLength) + 2;
	        }
	        assertEquals(25500 + msgHeaderLength, dataHeader.totalMsgLength());
	        if (fragmentNumber < numberOfFragments)
	        {
	        	assertFalse(((GenericMsg)msg).checkMessageComplete());
	        }
	        else if (fragmentNumber > numberOfFragments)
	        {
	        	fragmentNumber = 1;
	        	messageId++;
	        }
	        else // fragmentNumber == numberOfFragments
	        {
	        	assertTrue(((GenericMsg)msg).checkMessageComplete());
	        }
	        assertEquals(fragmentNumber, dataHeader.fragmentNumber());
	        assertEquals(messageId, dataHeader.messageId());
	        assertEquals(DataTypes.MSG, dataHeader.containerType());
	        int bufferLength, extraHeaderLength;
        	if (fragmentNumber == 1)
        	{
        		extraHeaderLength = msgHeaderLength;
        		b = 0;
        	}
        	else
        	{
        		extraHeaderLength = 0;
        	}
	        if (fragmentNumber < numberOfFragments)
	        {
	        	bufferLength = 6144 + headerLength;
	        }
	        else
	        {
	        	bufferLength = 924 + headerLength + msgHeaderLength;
	        }
	        int tunnelStreamBufferLength = tunnelStreamBuffer.length();
	        assertEquals(bufferLength, tunnelStreamBufferLength);
	        for (int i = headerLength + extraHeaderLength; i < bufferLength; i++)
	        {
	        	byte readByte = tunnelStreamBuffer.data().get(i);
	        	assertEquals(readByte, b++);
	        }
	    }
        provTunnelStream._outboundTransmitList.clear();

        /* Provider closes the tunnel stream, starting FIN/ACK teardown */
        provTunnelStream.close(false, _errorInfo);
        providerReactor.dispatch(0);
        
        /* Consumer receives the open/suspect event */
        consumerReactor.dispatch(1);
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.TUNNEL_STREAM_STATUS, event.type());
        tsStatusEvent = (TunnelStreamStatusEvent)event.reactorEvent();
        assertEquals(null, tsStatusEvent.authInfo());
        assertEquals(consumer.reactorChannel(), tsStatusEvent.reactorChannel());
        assertEquals(StreamStates.OPEN, tsStatusEvent.state().streamState());
        assertEquals(DataStates.SUSPECT, tsStatusEvent.state().dataState());
        assertEquals(StateCodes.NONE, tsStatusEvent.state().code());
        assertNotNull(tsStatusEvent.tunnelStream());
        consumerReactor.dispatch(0);
        
        /* Provider Reactor internally responds to FIN (no message). */
        providerReactor.dispatch(0);
        
        /* Consumer receives the close event */
        consumerReactor.dispatch(1);
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.TUNNEL_STREAM_STATUS, event.type());
        tsStatusEvent = (TunnelStreamStatusEvent)event.reactorEvent();
        assertEquals(null, tsStatusEvent.authInfo());
        assertEquals(consumer.reactorChannel(), tsStatusEvent.reactorChannel());
        assertEquals(StreamStates.CLOSED, tsStatusEvent.state().streamState());
        assertEquals(DataStates.SUSPECT, tsStatusEvent.state().dataState());
        assertEquals(StateCodes.NONE, tsStatusEvent.state().code());
        assertNotNull(tsStatusEvent.tunnelStream());
        consumerReactor.dispatch(0);
        
        TestReactorComponent.closeSession(consumer, provider);
        consumerReactor.close();
        providerReactor.close();
    }
    
    /* Tests writing of multiple 6144 byte buffers to a tunnel stream. */
    @Test
    public void writeBigBufferSmallMsgFragmentTest()
    {
        /* Test fragmenting buffers. */
        
        TestReactorEvent event;
        TunnelStreamStatusEvent tsStatusEvent;
        TunnelStream consTunnelStream;
        TunnelStream provTunnelStream;
        TransportBuffer buffer;
        
        /* Create reactors. */
        TestReactor consumerReactor = new TestReactor();
        TestReactor providerReactor = new TestReactor();
                
        /* Create consumer. */
        Consumer consumer = new Consumer(consumerReactor);
        ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
        consumerRole.initDefaultRDMLoginRequest();
        consumerRole.initDefaultRDMDirectoryRequest();
        consumerRole.channelEventCallback(consumer);
        consumerRole.loginMsgCallback(consumer);
        consumerRole.directoryMsgCallback(consumer);
        consumerRole.dictionaryMsgCallback(consumer);
        consumerRole.defaultMsgCallback(consumer);
        
        /* Create provider. */
        TunnelStreamProvider provider = new TunnelStreamProvider(providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.reactorRole();
        providerRole.channelEventCallback(provider);
        providerRole.loginMsgCallback(provider);
        providerRole.directoryMsgCallback(provider);
        providerRole.dictionaryMsgCallback(provider);
        providerRole.defaultMsgCallback(provider);
        providerRole.tunnelStreamListenerCallback(provider);

        /* Connect the consumer and provider. Setup login & directory streams automatically. */
        ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
        opts.setupDefaultLoginStream(true);
        opts.setupDefaultDirectoryStream(true);
        provider.bind(opts);
        TestReactor.openSession(consumer, provider, opts);
        
        /* Open a TunnelStream. */
        TunnelStreamOpenOptions tsOpenOpts = ReactorFactory.createTunnelStreamOpenOptions();
        tsOpenOpts.name("Tunnel1");
        tsOpenOpts.statusEventCallback(consumer);
        tsOpenOpts.queueMsgCallback(consumer);
        tsOpenOpts.defaultMsgCallback(consumer);
        tsOpenOpts.classOfService().dataIntegrity().type(DataIntegrityTypes.RELIABLE);
        tsOpenOpts.streamId(5);
        tsOpenOpts.serviceId(Provider.defaultService().serviceId());
        tsOpenOpts.domainType(DomainTypes.SYSTEM);
        tsOpenOpts.userSpecObject(consumer);     

        Consumer.OpenedTunnelStreamInfo openedTsInfo = consumer.openTunnelStream(provider, tsOpenOpts);
        consTunnelStream = openedTsInfo.consumerTunnelStream();
        provTunnelStream = openedTsInfo.providerTunnelStream();
      
        /* Consumer sends two small opaque buffers of 6144 bytes to the provider. */
        _tsSubmitOpts.clear();
        _tsSubmitOpts.containerType(DataTypes.XML);
        assertNotNull(buffer = consTunnelStream.getBuffer(25500, _errorInfo));
        for (int i = 0, b = 0; i < 6144; i++)
        {
        	if (b == 256)
        	{
        		b = 0;
        	}
        	buffer.data().put((byte)b++);
        }
        assertEquals(ReactorReturnCodes.SUCCESS, consTunnelStream.submit(buffer, _tsSubmitOpts, _errorInfo));
        assertNotNull(buffer = consTunnelStream.getBuffer(25500, _errorInfo));
        for (int i = 0, b = 0; i < 6144; i++)
        {
        	if (b == 256)
        	{
        		b = 0;
        	}
        	buffer.data().put((byte)b++);
        }
        assertEquals(ReactorReturnCodes.SUCCESS, consTunnelStream.submit(buffer, _tsSubmitOpts, _errorInfo));
        
        AckRangeList ackRangeList = new AckRangeList();
        AckRangeList nakRangeList = new AckRangeList();
        
        // check _outboundTransmitList of tunnel stream and make sure fragments are there and in correct order
        for(TunnelStreamBuffer tunnelStreamBuffer = consTunnelStream._outboundTransmitList.start(TunnelStreamBuffer.RETRANS_LINK); 
	            tunnelStreamBuffer != null;
	            tunnelStreamBuffer = consTunnelStream._outboundTransmitList.forth(TunnelStreamBuffer.RETRANS_LINK))
	    {
	        DecodeIterator decodeIterator = CodecFactory.createDecodeIterator();
	        Msg msg = CodecFactory.createMsg();
	        assertEquals(CodecReturnCodes.SUCCESS, decodeIterator.setBufferAndRWFVersion(tunnelStreamBuffer, Codec.majorVersion(), Codec.minorVersion()));
	        assertEquals(CodecReturnCodes.SUCCESS, msg.decode(decodeIterator));
	        TunnelStreamMsg tunnelMsg = new TunnelStreamMsgImpl();
	        assertEquals(MsgClasses.GENERIC, msg.msgClass());
	        assertEquals(DataTypes.XML, msg.containerType());
	        assertEquals(CodecReturnCodes.SUCCESS, tunnelMsg.decode(decodeIterator, (GenericMsg)msg, ackRangeList, nakRangeList));
	        assertEquals(TunnelStreamMsg.OpCodes.DATA, tunnelMsg.opCode());
	        TunnelStreamMsg.TunnelStreamData dataHeader = (TunnelStreamMsg.TunnelStreamData)tunnelMsg; 
	        assertEquals(0, dataHeader.dataMsgFlag());
	        assertEquals(0, dataHeader.totalMsgLength());
	        assertTrue(((GenericMsg)msg).checkMessageComplete());
	        assertEquals(0, dataHeader.fragmentNumber());
	        assertEquals(0, dataHeader.messageId());
	        assertEquals(0, dataHeader.containerType());
	        int bufferLength, headerLength;
	        headerLength = tunnelStreamBuffer.tunnelStreamHeaderLen();
        	bufferLength = 6144 + headerLength;
	        int tunnelStreamBufferLength = tunnelStreamBuffer.length();
	        assertEquals(bufferLength, tunnelStreamBufferLength);
	        byte b = 0;
	        for (int i = headerLength; i < bufferLength; i++)
	        {
	        	assertEquals(tunnelStreamBuffer.data().get(i), b++);
	        }
	    }
        consTunnelStream._outboundTransmitList.clear();
        
        /* Provider sends two big opaque buffers of 6144 bytes to the consumer. */
        _tsSubmitOpts.clear();
        _tsSubmitOpts.containerType(DataTypes.XML);
        assertNotNull(buffer = provTunnelStream.getBuffer(25500, _errorInfo));
        for (int i = 0, b = 0; i < 6144; i++)
        {
        	if (b == 256)
        	{
        		b = 0;
        	}
        	buffer.data().put((byte)b++);
        }
        assertEquals(ReactorReturnCodes.SUCCESS, provTunnelStream.submit(buffer, _tsSubmitOpts, _errorInfo));
        assertNotNull(buffer = provTunnelStream.getBuffer(25500, _errorInfo));
        for (int i = 0, b = 0; i < 6144; i++)
        {
        	if (b == 256)
        	{
        		b = 0;
        	}
        	buffer.data().put((byte)b++);
        }
        assertEquals(ReactorReturnCodes.SUCCESS, provTunnelStream.submit(buffer, _tsSubmitOpts, _errorInfo));
        
        // check _outboundTransmitList of tunnel stream and make sure fragments are there and in correct order
        for(TunnelStreamBuffer tunnelStreamBuffer = provTunnelStream._outboundTransmitList.start(TunnelStreamBuffer.RETRANS_LINK); 
	            tunnelStreamBuffer != null;
	            tunnelStreamBuffer = provTunnelStream._outboundTransmitList.forth(TunnelStreamBuffer.RETRANS_LINK))
	    {
	        DecodeIterator decodeIterator = CodecFactory.createDecodeIterator();
	        Msg msg = CodecFactory.createMsg();
	        assertEquals(CodecReturnCodes.SUCCESS, decodeIterator.setBufferAndRWFVersion(tunnelStreamBuffer, Codec.majorVersion(), Codec.minorVersion()));
	        assertEquals(CodecReturnCodes.SUCCESS, msg.decode(decodeIterator));
	        TunnelStreamMsg tunnelMsg = new TunnelStreamMsgImpl();
	        assertEquals(MsgClasses.GENERIC, msg.msgClass());
	        assertEquals(DataTypes.XML, msg.containerType());
	        assertEquals(CodecReturnCodes.SUCCESS, tunnelMsg.decode(decodeIterator, (GenericMsg)msg, ackRangeList, nakRangeList));
	        assertEquals(TunnelStreamMsg.OpCodes.DATA, tunnelMsg.opCode());
	        TunnelStreamMsg.TunnelStreamData dataHeader = (TunnelStreamMsg.TunnelStreamData)tunnelMsg; 
	        assertEquals(0, dataHeader.dataMsgFlag());
	        assertEquals(0, dataHeader.totalMsgLength());
	        assertTrue(((GenericMsg)msg).checkMessageComplete());
	        assertEquals(0, dataHeader.fragmentNumber());
	        assertEquals(0, dataHeader.messageId());
	        assertEquals(0, dataHeader.containerType());
	        int bufferLength, headerLength;
	        headerLength = tunnelStreamBuffer.tunnelStreamHeaderLen();
	        bufferLength = 6144 + headerLength;
	        int tunnelStreamBufferLength = tunnelStreamBuffer.length();
	        assertEquals(bufferLength, tunnelStreamBufferLength);
	        byte b = 0;
	        for (int i = headerLength; i < bufferLength; i++)
	        {
	        	assertEquals(tunnelStreamBuffer.data().get(i), b++);
	        }
	    }
        provTunnelStream._outboundTransmitList.clear();

        /* Provider closes the tunnel stream, starting FIN/ACK teardown */
        provTunnelStream.close(false, _errorInfo);
        providerReactor.dispatch(0);
        
        /* Consumer receives the open/suspect event */
        consumerReactor.dispatch(1);
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.TUNNEL_STREAM_STATUS, event.type());
        tsStatusEvent = (TunnelStreamStatusEvent)event.reactorEvent();
        assertEquals(null, tsStatusEvent.authInfo());
        assertEquals(consumer.reactorChannel(), tsStatusEvent.reactorChannel());
        assertEquals(StreamStates.OPEN, tsStatusEvent.state().streamState());
        assertEquals(DataStates.SUSPECT, tsStatusEvent.state().dataState());
        assertEquals(StateCodes.NONE, tsStatusEvent.state().code());
        assertNotNull(tsStatusEvent.tunnelStream());
        consumerReactor.dispatch(0);
        
        /* Provider Reactor internally responds to FIN (no message). */
        providerReactor.dispatch(0);
        
        /* Consumer receives the close event */
        consumerReactor.dispatch(1);
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.TUNNEL_STREAM_STATUS, event.type());
        tsStatusEvent = (TunnelStreamStatusEvent)event.reactorEvent();
        assertEquals(null, tsStatusEvent.authInfo());
        assertEquals(consumer.reactorChannel(), tsStatusEvent.reactorChannel());
        assertEquals(StreamStates.CLOSED, tsStatusEvent.state().streamState());
        assertEquals(DataStates.SUSPECT, tsStatusEvent.state().dataState());
        assertEquals(StateCodes.NONE, tsStatusEvent.state().code());
        assertNotNull(tsStatusEvent.tunnelStream());
        consumerReactor.dispatch(0);
        
        TestReactorComponent.closeSession(consumer, provider);
        consumerReactor.close();
        providerReactor.close();
    }

    /* Tests writing of big buffers to a tunnel stream that runs out of internal buffers
     * and that the tunnel stream internally tracks progress and recovers. */
    @Test
    public void writeBigBufferSaveProgressTest()
    {
        /* Test if saving a big buffer's progress works. */
        
        TestReactorEvent event;
        TunnelStreamStatusEvent tsStatusEvent;
        TunnelStream consTunnelStream;
        TunnelStream provTunnelStream;
        TransportBuffer buffer;
        
        /* Create reactors. */
        TestReactor consumerReactor = new TestReactor();
        TestReactor providerReactor = new TestReactor();
                
        /* Create consumer. */
        Consumer consumer = new Consumer(consumerReactor);
        ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
        consumerRole.initDefaultRDMLoginRequest();
        consumerRole.initDefaultRDMDirectoryRequest();
        consumerRole.channelEventCallback(consumer);
        consumerRole.loginMsgCallback(consumer);
        consumerRole.directoryMsgCallback(consumer);
        consumerRole.dictionaryMsgCallback(consumer);
        consumerRole.defaultMsgCallback(consumer);
        
        /* Create provider. */
        TunnelStreamProvider provider = new TunnelStreamProvider(providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.reactorRole();
        providerRole.channelEventCallback(provider);
        providerRole.loginMsgCallback(provider);
        providerRole.directoryMsgCallback(provider);
        providerRole.dictionaryMsgCallback(provider);
        providerRole.defaultMsgCallback(provider);
        providerRole.tunnelStreamListenerCallback(provider);

        /* Connect the consumer and provider. Setup login & directory streams automatically. */
        ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
        opts.setupDefaultLoginStream(true);
        opts.setupDefaultDirectoryStream(true);
        provider.bind(opts);
        TestReactor.openSession(consumer, provider, opts);
        
        /* Open a TunnelStream. */
        TunnelStreamOpenOptions tsOpenOpts = ReactorFactory.createTunnelStreamOpenOptions();
        tsOpenOpts.name("Tunnel1");
        tsOpenOpts.statusEventCallback(consumer);
        tsOpenOpts.queueMsgCallback(consumer);
        tsOpenOpts.defaultMsgCallback(consumer);
        tsOpenOpts.classOfService().dataIntegrity().type(DataIntegrityTypes.RELIABLE);
        tsOpenOpts.streamId(5);
        tsOpenOpts.serviceId(Provider.defaultService().serviceId());
        tsOpenOpts.domainType(DomainTypes.SYSTEM);
        tsOpenOpts.userSpecObject(consumer);     

        Consumer.OpenedTunnelStreamInfo openedTsInfo = consumer.openTunnelStream(provider, tsOpenOpts);
        consTunnelStream = openedTsInfo.consumerTunnelStream();
        provTunnelStream = openedTsInfo.providerTunnelStream();
        
        /* Set up tunnel stream so it can only fragment 2 buffers before running out. */
        SlicedBufferPool originalSlicedBufferPool = consTunnelStream._bufferPool;
        SlicedBufferPool modifiedSlicedBufferPool = new TestSlicedBufferPool(consTunnelStream._classOfService.common().maxFragmentSize(), consTunnelStream.guaranteedOutputBuffers(), 2);
        consTunnelStream._bufferPool = modifiedSlicedBufferPool;
      
        /* Consumer sends two big opaque buffers of 25500 bytes to the provider. */
        _tsSubmitOpts.clear();
        _tsSubmitOpts.containerType(DataTypes.XML);
        assertNotNull(buffer = consTunnelStream.getBuffer(25500, _errorInfo));
        for (int i = 0, b = 0; i < 25500; i++)
        {
        	if (b == 256)
        	{
        		b = 0;
        	}
        	buffer.data().put((byte)b++);
        }
        assertEquals(ReactorReturnCodes.SUCCESS, consTunnelStream.submit(buffer, _tsSubmitOpts, _errorInfo));
        assertNotNull(buffer = consTunnelStream.getBuffer(25500, _errorInfo));
        for (int i = 0, b = 0; i < 25500; i++)
        {
        	if (b == 256)
        	{
        		b = 0;
        	}
        	buffer.data().put((byte)b++);
        }
        assertEquals(ReactorReturnCodes.SUCCESS, consTunnelStream.submit(buffer, _tsSubmitOpts, _errorInfo));
        
        AckRangeList ackRangeList = new AckRangeList();
        AckRangeList nakRangeList = new AckRangeList();
        int numberOfFragments = 25500/consTunnelStream._classOfService.common().maxFragmentSize() + 1;
        int fragmentNumber = 0;
        int messageId = 1;
        byte b = 0;
        
        // check _outboundTransmitList of tunnel stream and make sure fragments are there and in correct order
        for(TunnelStreamBuffer tunnelStreamBuffer = consTunnelStream._outboundTransmitList.start(TunnelStreamBuffer.RETRANS_LINK); 
	            tunnelStreamBuffer != null;
	            tunnelStreamBuffer = consTunnelStream._outboundTransmitList.forth(TunnelStreamBuffer.RETRANS_LINK))
	    {
	        DecodeIterator decodeIterator = CodecFactory.createDecodeIterator();
	        Msg msg = CodecFactory.createMsg();
	        assertEquals(CodecReturnCodes.SUCCESS, decodeIterator.setBufferAndRWFVersion(tunnelStreamBuffer, Codec.majorVersion(), Codec.minorVersion()));
	        assertEquals(CodecReturnCodes.SUCCESS, msg.decode(decodeIterator));
	        TunnelStreamMsg tunnelMsg = new TunnelStreamMsgImpl();
	        assertEquals(MsgClasses.GENERIC, msg.msgClass());
	        assertEquals(DataTypes.OPAQUE, msg.containerType());
	        assertEquals(CodecReturnCodes.SUCCESS, tunnelMsg.decode(decodeIterator, (GenericMsg)msg, ackRangeList, nakRangeList));
	        assertEquals(TunnelStreamMsg.OpCodes.DATA, tunnelMsg.opCode());
	        TunnelStreamMsg.TunnelStreamData dataHeader = (TunnelStreamMsg.TunnelStreamData)tunnelMsg; 
	        assertEquals(1, dataHeader.dataMsgFlag());
	        assertEquals(25500, dataHeader.totalMsgLength());
	        fragmentNumber++;
	        if (fragmentNumber < numberOfFragments)
	        {
	        	assertFalse(((GenericMsg)msg).checkMessageComplete());
	        }
	        else if (fragmentNumber > numberOfFragments)
	        {
	        	fragmentNumber = 1;
	        	messageId++;
	        }
	        else // fragmentNumber == numberOfFragments
	        {
	        	assertTrue(((GenericMsg)msg).checkMessageComplete());
	        }
	        assertEquals(fragmentNumber, dataHeader.fragmentNumber());
	        assertEquals(messageId, dataHeader.messageId());
	        assertEquals(DataTypes.XML, dataHeader.containerType());
	        int bufferLength, headerLength;
	        headerLength = tunnelStreamBuffer.tunnelStreamHeaderLen();
	        if (fragmentNumber < numberOfFragments)
	        {
	        	bufferLength = 6144 + headerLength;
	        }
	        else
	        {
	        	bufferLength = 924 + headerLength;
	        }
	        int tunnelStreamBufferLength = tunnelStreamBuffer.length();
	        assertEquals(bufferLength, tunnelStreamBufferLength);
	        for (int i = headerLength; i < bufferLength; i++)
	        {
	        	assertEquals(tunnelStreamBuffer.data().get(i), b++);
	        }
	    }
        consTunnelStream._outboundTransmitList.clear();
        
        // reset sliced buffer pool and call consumer dispatch to get pending big buffers out
        consTunnelStream._bufferPool = originalSlicedBufferPool;
        consTunnelStream._jUnitSkipHandleTransmit = true;
        consumerReactor.dispatch(0);
        assertEquals(8, consTunnelStream._outboundTransmitList.count());

        // check _outboundTransmitList of tunnel stream and make sure fragments are there and in correct order
        for(TunnelStreamBuffer tunnelStreamBuffer = consTunnelStream._outboundTransmitList.start(TunnelStreamBuffer.RETRANS_LINK); 
	            tunnelStreamBuffer != null;
	            tunnelStreamBuffer = consTunnelStream._outboundTransmitList.forth(TunnelStreamBuffer.RETRANS_LINK))
	    {
	        DecodeIterator decodeIterator = CodecFactory.createDecodeIterator();
	        Msg msg = CodecFactory.createMsg();
	        assertEquals(CodecReturnCodes.SUCCESS, decodeIterator.setBufferAndRWFVersion(tunnelStreamBuffer, Codec.majorVersion(), Codec.minorVersion()));
	        assertEquals(CodecReturnCodes.SUCCESS, msg.decode(decodeIterator));
	        TunnelStreamMsg tunnelMsg = new TunnelStreamMsgImpl();
	        assertEquals(MsgClasses.GENERIC, msg.msgClass());
	        assertEquals(DataTypes.OPAQUE, msg.containerType());
	        assertEquals(CodecReturnCodes.SUCCESS, tunnelMsg.decode(decodeIterator, (GenericMsg)msg, ackRangeList, nakRangeList));
	        assertEquals(TunnelStreamMsg.OpCodes.DATA, tunnelMsg.opCode());
	        TunnelStreamMsg.TunnelStreamData dataHeader = (TunnelStreamMsg.TunnelStreamData)tunnelMsg; 
	        assertEquals(1, dataHeader.dataMsgFlag());
	        assertEquals(25500, dataHeader.totalMsgLength());
	        fragmentNumber++;	        
	        if (fragmentNumber < numberOfFragments)
	        {
	        	assertFalse(((GenericMsg)msg).checkMessageComplete());
	        }
	        else if (fragmentNumber > numberOfFragments)
	        {
	        	fragmentNumber = 1;
	        	b = 0;
	        	messageId++;
	        }
	        else // fragmentNumber == numberOfFragments
	        {
	        	assertTrue(((GenericMsg)msg).checkMessageComplete());
	        }
	        assertEquals(fragmentNumber, dataHeader.fragmentNumber());
	        assertEquals(messageId, dataHeader.messageId());
	        assertEquals(DataTypes.XML, dataHeader.containerType());
	        int bufferLength, headerLength;
	        headerLength = tunnelStreamBuffer.tunnelStreamHeaderLen();
	        if (fragmentNumber < numberOfFragments)
	        {
	        	bufferLength = 6144 + headerLength;
	        }
	        else
	        {
	        	bufferLength = 924 + headerLength;
	        }
	        int tunnelStreamBufferLength = tunnelStreamBuffer.length();
	        assertEquals(bufferLength, tunnelStreamBufferLength);
	        for (int i = headerLength; i < bufferLength; i++)
	        {
	        	byte data = tunnelStreamBuffer.data().get(i);
	        	assertEquals(data, b++);
	        }
	    }
        consTunnelStream._outboundTransmitList.clear();
        consTunnelStream._jUnitSkipHandleTransmit = false;

        /* Provider closes the tunnel stream, starting FIN/ACK teardown */
        provTunnelStream.close(false, _errorInfo);
        providerReactor.dispatch(0);
        
        /* Consumer receives the open/suspect event */
        consumerReactor.dispatch(1);
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.TUNNEL_STREAM_STATUS, event.type());
        tsStatusEvent = (TunnelStreamStatusEvent)event.reactorEvent();
        assertEquals(null, tsStatusEvent.authInfo());
        assertEquals(consumer.reactorChannel(), tsStatusEvent.reactorChannel());
        assertEquals(StreamStates.OPEN, tsStatusEvent.state().streamState());
        assertEquals(DataStates.SUSPECT, tsStatusEvent.state().dataState());
        assertEquals(StateCodes.NONE, tsStatusEvent.state().code());
        assertNotNull(tsStatusEvent.tunnelStream());
        consumerReactor.dispatch(0);
        
        /* Provider Reactor internally responds to FIN (no message). */
        providerReactor.dispatch(0);
        
        /* Consumer receives the close event */
        consumerReactor.dispatch(1);
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.TUNNEL_STREAM_STATUS, event.type());
        tsStatusEvent = (TunnelStreamStatusEvent)event.reactorEvent();
        assertEquals(null, tsStatusEvent.authInfo());
        assertEquals(consumer.reactorChannel(), tsStatusEvent.reactorChannel());
        assertEquals(StreamStates.CLOSED, tsStatusEvent.state().streamState());
        assertEquals(DataStates.SUSPECT, tsStatusEvent.state().dataState());
        assertEquals(StateCodes.NONE, tsStatusEvent.state().code());
        assertNotNull(tsStatusEvent.tunnelStream());
        consumerReactor.dispatch(0);
        
        TestReactorComponent.closeSession(consumer, provider);
        consumerReactor.close();
        providerReactor.close();
    }
    
    /* Used for testing progress of big buffer writes. numberBuffersToReturn constructor 
     * argument indicates how many buffers are returned before returning null. */
    class TestSlicedBufferPool extends SlicedBufferPool
    {
    	int _numberBuffersToReturn;
    	int _numberOfCalls;
    	
		TestSlicedBufferPool(int maxMsgSize, int numBuffers, int numberBuffersToReturn)
		{
			this(maxMsgSize, numBuffers);
			_numberBuffersToReturn = numberBuffersToReturn;
		}
		
		TestSlicedBufferPool(int maxMsgSize, int numBuffers)
		{
			super(maxMsgSize, numBuffers);
		}
		
		void getBufferSlice(TunnelStreamBuffer bufferImpl, int length, boolean isForUser)
		{
			if (_numberOfCalls++ < _numberBuffersToReturn)
			{
				super.getBufferSlice(bufferImpl, length, isForUser);
			}
			else
			{
				bufferImpl.data(null);
			}
		}
    }
    
    /* Tests receiving of interleaved fragmented messages from a tunnel stream. */
    @Test
    public void receiveInterleavedMessageFragmentsTest()
    {

        /* Test receiving two messages with interleaved message fragments. */
        
        ReadEvent readEvent;
        Msg provMsg = CodecFactory.createMsg();
        RequestMsg recvRequestMsg;
        TestReactorEvent event;
        int provTunnelStreamId;
        TunnelStreamStatusEvent tsStatusEvent;
        TunnelStreamMsgEvent tsMsgEvent;
        int responseTimeout = 2;

        /* Create reactor, consumer, and provider. */
        TestReactor consumerReactor = new TestReactor();
        Consumer consumer = new Consumer(consumerReactor);
        TunnelStreamCoreProvider provider = new TunnelStreamCoreProvider();
        ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();

        CoreComponent.openSession(consumer, provider, opts, false);

        /* Open a TunnelStream. */
        TunnelStreamOpenOptions tsOpenOpts = ReactorFactory.createTunnelStreamOpenOptions();
        tsOpenOpts.name("Tunnel1");
        tsOpenOpts.statusEventCallback(consumer);
        tsOpenOpts.queueMsgCallback(consumer);
        tsOpenOpts.defaultMsgCallback(consumer);
        tsOpenOpts.classOfService().dataIntegrity().type(DataIntegrityTypes.RELIABLE);
        tsOpenOpts.streamId(5);
        tsOpenOpts.serviceId(Provider.defaultService().serviceId());
        tsOpenOpts.domainType(DomainTypes.SYSTEM);
        tsOpenOpts.userSpecObject(consumer);     
        tsOpenOpts.responseTimeout(responseTimeout);

        assertEquals(ReactorReturnCodes.SUCCESS, consumer.reactorChannel().openTunnelStream(tsOpenOpts, _errorInfo));
        consumerReactor.dispatch(0);
        
        /* Provider accepts. */
        provider.dispatch(1);
        readEvent = provider.pollEvent();
        provMsg = readEvent.msg();
        assertEquals(MsgClasses.REQUEST, provMsg.msgClass());
        recvRequestMsg = (RequestMsg)provMsg;
        assertEquals(tsOpenOpts.domainType(), recvRequestMsg.domainType());
        assertTrue(recvRequestMsg.checkStreaming());
        assertTrue(recvRequestMsg.checkPrivateStream());
        assertTrue(recvRequestMsg.checkQualifiedStream());;
        assertTrue(recvRequestMsg.msgKey().checkHasFilter());
        assertTrue(recvRequestMsg.msgKey().checkHasServiceId());
        assertEquals(DataTypes.FILTER_LIST, recvRequestMsg.containerType());
        provTunnelStreamId = recvRequestMsg.streamId();
        provider.acceptTunnelStreamRequest(consumer, recvRequestMsg, null);
        
        /* Consumer receives response. */
        consumerReactor.dispatch(1);
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.TUNNEL_STREAM_STATUS, event.type());
        tsStatusEvent = (TunnelStreamStatusEvent)event.reactorEvent();
        assertEquals(StreamStates.OPEN, tsStatusEvent.state().streamState());
        assertEquals(DataStates.OK, tsStatusEvent.state().dataState());
        
        /* Provider sends two messages with interleaved message fragments. */
        TunnelStreamMsg tunnelStreamMsg = new TunnelStreamMsgImpl();
        TunnelStreamData tunnelStreamData = (TunnelStreamData)tunnelStreamMsg;
        
        // message 1 fragment 1
        tunnelStreamData.clearData();
        tunnelStreamMsg.streamId(provTunnelStreamId);
        tunnelStreamMsg.domainType(DataTypes.OPAQUE);
        tunnelStreamData.seqNum(1);
        tunnelStreamData.dataMsgFlag(1);
        tunnelStreamData.totalMsgLength(16);
        tunnelStreamData.fragmentNumber(1);
        tunnelStreamData.messageId(1);
        tunnelStreamData.containerType(DataTypes.OPAQUE);
        Buffer buffer = CodecFactory.createBuffer();
        buffer.data("abcdefg");
        provider.submitTunnelStreamOpaqueData(tunnelStreamData, buffer);
        
        // send message 1 fragment 1 again to make sure overwrite of previous re-assembly works
        tunnelStreamData.clearData();
        tunnelStreamMsg.streamId(provTunnelStreamId);
        tunnelStreamMsg.domainType(DataTypes.OPAQUE);
        tunnelStreamData.seqNum(2);
        tunnelStreamData.dataMsgFlag(1);
        tunnelStreamData.totalMsgLength(16);
        tunnelStreamData.fragmentNumber(1);
        tunnelStreamData.messageId(1);
        tunnelStreamData.containerType(DataTypes.OPAQUE);
        buffer.data("abcdefg");
        provider.submitTunnelStreamOpaqueData(tunnelStreamData, buffer);

        // message 2 fragment 1
        tunnelStreamData.clearData();
        tunnelStreamMsg.streamId(provTunnelStreamId);
        tunnelStreamMsg.domainType(DataTypes.OPAQUE);
        tunnelStreamData.seqNum(3);
        tunnelStreamData.dataMsgFlag(1);
        tunnelStreamData.totalMsgLength(10);
        tunnelStreamData.fragmentNumber(1);
        tunnelStreamData.messageId(2);
        tunnelStreamData.containerType(DataTypes.OPAQUE);
        buffer.data("01234");
        provider.submitTunnelStreamOpaqueData(tunnelStreamData, buffer);

        // message 1 fragment 2
        tunnelStreamData.clearData();
        tunnelStreamMsg.streamId(provTunnelStreamId);
        tunnelStreamMsg.domainType(DataTypes.OPAQUE);
        tunnelStreamData.seqNum(4);
        tunnelStreamData.dataMsgFlag(1);
        tunnelStreamData.totalMsgLength(16);
        tunnelStreamData.fragmentNumber(2);
        tunnelStreamData.messageId(1);
        tunnelStreamData.containerType(DataTypes.OPAQUE);
        buffer.data("hijklmnop");
        provider.submitTunnelStreamOpaqueData(tunnelStreamData, buffer);

        // message 2 fragment 2
        tunnelStreamData.clearData();
        tunnelStreamMsg.streamId(provTunnelStreamId);
        tunnelStreamMsg.domainType(DataTypes.OPAQUE);
        tunnelStreamData.seqNum(5);
        tunnelStreamData.dataMsgFlag(1);
        tunnelStreamData.totalMsgLength(10);
        tunnelStreamData.fragmentNumber(2);
        tunnelStreamData.messageId(2);
        tunnelStreamData.containerType(DataTypes.OPAQUE);
        buffer.data("56789");
        provider.submitTunnelStreamOpaqueData(tunnelStreamData, buffer);
        
        /* Consumer receives 2 messages with correct re-assembled data. */
        consumerReactor.dispatch(2);
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.TUNNEL_STREAM_MSG, event.type());
        tsMsgEvent = (TunnelStreamMsgEvent)event.reactorEvent();
        assertEquals(DataTypes.OPAQUE, tsMsgEvent.containerType());
        ByteBuffer byteBuffer = tsMsgEvent.transportBuffer().data();
        String byteBufferString = new String(byteBuffer.array());
        assertTrue(byteBufferString.equals("abcdefghijklmnop"));
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.TUNNEL_STREAM_MSG, event.type());
        tsMsgEvent = (TunnelStreamMsgEvent)event.reactorEvent();
        assertEquals(DataTypes.OPAQUE, tsMsgEvent.containerType());
        byteBuffer = tsMsgEvent.transportBuffer().data();
        byteBufferString = new String(byteBuffer.array());
        assertTrue(byteBufferString.equals("0123456789"));

        assertEquals(ReactorReturnCodes.SUCCESS, tsStatusEvent.tunnelStream().close(false, _errorInfo));
        provider.close();
        consumer.close();
        consumerReactor.close();
    }
}
