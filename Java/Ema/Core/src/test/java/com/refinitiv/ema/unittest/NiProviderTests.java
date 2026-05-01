/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025-2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.unittest;

import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.transport.Transport;

import org.junit.runners.MethodSorters;

import com.refinitiv.ema.JUnitConfigVariables;
import com.refinitiv.ema.NiConsumerHelper;
import com.refinitiv.ema.NiConsumerHelperConfig;
import com.refinitiv.ema.access.ChannelInformation;
import com.refinitiv.ema.access.DataType;
import com.refinitiv.ema.access.ElementEntry;
import com.refinitiv.ema.access.DataType.DataTypes;
import com.refinitiv.ema.access.ElementList;
import com.refinitiv.ema.access.EmaFactory;
import com.refinitiv.ema.access.FieldList;
import com.refinitiv.ema.access.FilterEntry;
import com.refinitiv.ema.access.FilterList;
import com.refinitiv.ema.access.GenericMsg;
import com.refinitiv.ema.access.Map;
import com.refinitiv.ema.access.MapEntry;
import com.refinitiv.ema.access.Msg;
import com.refinitiv.ema.access.OmmArray;
import com.refinitiv.ema.access.OmmInvalidHandleException;
import com.refinitiv.ema.access.OmmInvalidUsageException;
import com.refinitiv.ema.access.OmmNiProviderConfig;
import com.refinitiv.ema.access.OmmProvider;
import com.refinitiv.ema.access.OmmProviderClient;
import com.refinitiv.ema.access.OmmProviderEvent;
import com.refinitiv.ema.access.OmmReal;
import com.refinitiv.ema.access.OmmState;
import com.refinitiv.ema.access.OmmState.DataState;
import com.refinitiv.ema.access.OmmState.StreamState;
import com.refinitiv.ema.access.PackedMsg;
import com.refinitiv.ema.access.PostMsg;
import com.refinitiv.ema.access.RefreshMsg;
import com.refinitiv.ema.access.ReqMsg;
import com.refinitiv.ema.access.StatusMsg;
import com.refinitiv.ema.access.UpdateMsg;
import com.refinitiv.ema.rdm.EmaRdm;

import static org.junit.Assert.*;

import java.nio.ByteBuffer;
import java.util.ArrayDeque;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.concurrent.locks.ReentrantLock;

import org.junit.After;
import org.junit.Before;
import org.junit.FixMethodOrder;
import org.junit.Test;

@FixMethodOrder(MethodSorters.DEFAULT)
public class NiProviderTests
{
	
	public class NiProviderTestClient implements OmmProviderClient 
	{
		private ArrayDeque<Msg> _messageQueue = new ArrayDeque<Msg>();
		
		private ArrayDeque<ChannelInformation> _channelInfoQueue = new ArrayDeque<>();
		
		private ArrayDeque<List<ChannelInformation>> _sessionChannelInfoQueue = new ArrayDeque<>();
		
		private ReentrantLock _userLock = new java.util.concurrent.locks.ReentrantLock();
		
		private HashSet<Long> _handles = new HashSet<Long>();
		
		private OmmProvider _provider;
		
		com.refinitiv.eta.json.converter.JsonConverter jsonConverter = null;
		
		public NiProviderTestClient()
		{
		}
		
		public void provider(OmmProvider provider)
		{
			_provider = provider;
		}
		
		public int queueSize()
		{
			_userLock.lock();
			try
			{
				return _messageQueue.size();
			}
			finally
			{
				_userLock.unlock();
			}
		}
		
		public Msg popMessage()
		{
			_userLock.lock();
			try
			{
				return _messageQueue.removeFirst();
			}
			finally
			{
				_userLock.unlock();
			}
		}
		
		public void clearQueue() 
		{
			_userLock.lock();
			try
			{
				_messageQueue.clear();
			}
			finally
			{
				_userLock.unlock();
			}
			
		}
		
		public int channelInfoSize()
		{
			_userLock.lock();
			try
			{
				return _channelInfoQueue.size();
			}
			finally
			{
				_userLock.unlock();
			}
		}
		
		public ChannelInformation popChannelInfo()
		{
			_userLock.lock();
			try
			{
				return _channelInfoQueue.removeFirst();
			}
			finally
			{
				_userLock.unlock();
			}
		}
		
		public int sessionChannelInfoSize()
		{
			_userLock.lock();
			try
			{
				return _sessionChannelInfoQueue.size();
			}
			finally
			{
				_userLock.unlock();
			}
		}
		
		public List<ChannelInformation> popSessionChannelInfo()
		{
			_userLock.lock();
			try
			{
				return _sessionChannelInfoQueue.removeFirst();
			}
			finally
			{
				_userLock.unlock();
			}
		}
		
		public void unregisterAllHandles()
		{
			for(Long handle : _handles)
			{
				_provider.unregister(handle);
			}
			
			_handles.clear();
		}

		@Override
		public void onRefreshMsg(RefreshMsg refreshMsg, OmmProviderEvent providerEvent) 
		{
			_userLock.lock();
			
			try
			{
				_handles.add(providerEvent.handle());
				
				RefreshMsg cloneMsg = EmaFactory.createRefreshMsg(1280);
				
				refreshMsg.copy(cloneMsg);
				System.out.println("\n>>>>>>>>>>>>>> Consumer received refresh message in domain " + cloneMsg.domainType()
						+ (cloneMsg.hasServiceName() ? ", serviceName: " + refreshMsg.serviceName() : ""));

				_messageQueue.add(cloneMsg);
			}
			finally
			{
				_userLock.unlock();
			}
			
		}

		@Override
		public void onStatusMsg(StatusMsg statusMsg, OmmProviderEvent providerEvent)
		{
			
			_userLock.lock();
			
			try
			{
				StatusMsg cloneMsg = EmaFactory.createStatusMsg(1280);
				
				statusMsg.copy(cloneMsg);
				
				System.out.println(cloneMsg);
			
				_messageQueue.add(cloneMsg);
				
			}
			finally
			{
				_userLock.unlock();
			}
		}

		@Override
		public void onGenericMsg(GenericMsg genericMsg, OmmProviderEvent providerEvent) 
		{
			_userLock.lock();
			
			try
			{
				GenericMsg cloneMsg = EmaFactory.createGenericMsg(1280);
				
				genericMsg.copy(cloneMsg);
				
				System.out.println(cloneMsg);
			
				_messageQueue.add(cloneMsg);
				
			}
			finally
			{
				_userLock.unlock();
			}
			
		}
		
		

		@Override
		public void onAllMsg(Msg msg, OmmProviderEvent providerEvent) {
			// Do nothing
		}
		@Override
		public void onPostMsg(PostMsg postMsg, OmmProviderEvent providerEvent) {}
		@Override
		public void onReqMsg(ReqMsg reqMsg, OmmProviderEvent providerEvent) {}
		@Override
		public void onReissue(ReqMsg reqMsg, OmmProviderEvent providerEvent) {}
		@Override
		public void onClose(ReqMsg reqMsg, OmmProviderEvent providerEvent) {}

	}
	
	private com.refinitiv.eta.transport.Error etaError = com.refinitiv.eta.transport.TransportFactory.createError();
	private com.refinitiv.eta.transport.InitArgs etaInitArgs = com.refinitiv.eta.transport.TransportFactory.createInitArgs();
	com.refinitiv.eta.codec.EncodeIterator etaEncodeIter = com.refinitiv.eta.codec.CodecFactory.createEncodeIterator();
	private com.refinitiv.eta.codec.DecodeIterator etaDecodeIter = com.refinitiv.eta.codec.CodecFactory.createDecodeIterator();
	
	private NiConsumerHelper niConsumer1 = null;
	private NiConsumerHelper niConsumer2 = null;
	private NiConsumerHelper niConsumer3 = null;
	private NiConsumerHelper niConsumer4 = null;
	private OmmProvider provider = null;
	private OmmNiProviderConfig niProvConfig = null;
	
	private String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmNiProviderTests/EmaNiProvTestConfig.xml";
	
	public NiProviderTests()
	{

	}
	
	private com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsg convertToLoginMsg(com.refinitiv.eta.codec.Msg message)
	{
		com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsg etaLoginMsg = com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgFactory.createMsg();
		etaLoginMsg.rdmMsgType(com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType.REQUEST);
		com.refinitiv.eta.codec.Msg decodeMsg = com.refinitiv.eta.codec.CodecFactory.createMsg(); 
		decodeMsg.clear();
		etaEncodeIter.clear();
		etaDecodeIter.clear();
		com.refinitiv.eta.codec.Buffer buff = com.refinitiv.eta.codec.CodecFactory.createBuffer();
		ByteBuffer byteBuffer = ByteBuffer.allocate(1500);
		buff.data(byteBuffer);
		
		etaEncodeIter.setBufferAndRWFVersion(buff, com.refinitiv.eta.codec.Codec.majorVersion(), com.refinitiv.eta.codec.Codec.minorVersion());
		assertEquals(message.encode(etaEncodeIter), com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS);
		
		etaDecodeIter.setBufferAndRWFVersion(buff, com.refinitiv.eta.codec.Codec.majorVersion(), com.refinitiv.eta.codec.Codec.minorVersion());
		assertEquals(decodeMsg.decode(etaDecodeIter), com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS);
		
		assertEquals(etaLoginMsg.decode(etaDecodeIter, decodeMsg), com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS);
		
		return etaLoginMsg;
	}
	
	private com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsg convertToDirectoryRefreshMsg(com.refinitiv.eta.codec.Msg message)
	{
		com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsg etaDirectoryMsg = com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgFactory.createMsg();
		etaDirectoryMsg.rdmMsgType(com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType.REFRESH);
		com.refinitiv.eta.codec.Msg decodeMsg = com.refinitiv.eta.codec.CodecFactory.createMsg(); 
		etaEncodeIter.clear();
		etaDecodeIter.clear();
		com.refinitiv.eta.codec.Buffer buff = com.refinitiv.eta.codec.CodecFactory.createBuffer();
		ByteBuffer byteBuffer = ByteBuffer.allocate(1500);
		buff.data(byteBuffer);
		
		etaEncodeIter.setBufferAndRWFVersion(buff, com.refinitiv.eta.codec.Codec.majorVersion(), com.refinitiv.eta.codec.Codec.minorVersion());
		assertEquals(message.encode(etaEncodeIter), com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS);		
		
		etaDecodeIter.setBufferAndRWFVersion(buff, com.refinitiv.eta.codec.Codec.majorVersion(), com.refinitiv.eta.codec.Codec.minorVersion());
		assertEquals(decodeMsg.decode(etaDecodeIter), com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS);
		assertEquals(etaDirectoryMsg.decode(etaDecodeIter, decodeMsg), com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS);
		
		return etaDirectoryMsg;
	}
	
	private com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsg convertToDirectoryUpdateMsg(com.refinitiv.eta.codec.Msg message)
	{
		com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsg etaDirectoryMsg = com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgFactory.createMsg();
		etaDirectoryMsg.rdmMsgType(com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType.UPDATE);
		com.refinitiv.eta.codec.Msg decodeMsg = com.refinitiv.eta.codec.CodecFactory.createMsg(); 
		etaEncodeIter.clear();
		etaDecodeIter.clear();
		com.refinitiv.eta.codec.Buffer buff = com.refinitiv.eta.codec.CodecFactory.createBuffer();
		ByteBuffer byteBuffer = ByteBuffer.allocate(1500);
		buff.data(byteBuffer);
		
		etaEncodeIter.setBufferAndRWFVersion(buff, com.refinitiv.eta.codec.Codec.majorVersion(), com.refinitiv.eta.codec.Codec.minorVersion());
		assertEquals(message.encode(etaEncodeIter), com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS);		
		
		etaDecodeIter.setBufferAndRWFVersion(buff, com.refinitiv.eta.codec.Codec.majorVersion(), com.refinitiv.eta.codec.Codec.minorVersion());
		assertEquals(decodeMsg.decode(etaDecodeIter), com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS);
		assertEquals(etaDirectoryMsg.decode(etaDecodeIter, decodeMsg), com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS);
		
		return etaDirectoryMsg;
	}
	
	
	@Before
	public void SetUp()
	{
		etaInitArgs.globalLocking(true);
		
		Transport.initialize(etaInitArgs, etaError);
		
		niConsumer1 = null;
		niConsumer2 = null;
		niConsumer3 = null;
		niConsumer4 = null;

		provider = null;
		niProvConfig = null;
	}
	
	@After
	public void TearDown()
	{
		if(niConsumer1 != null)
		{
			niConsumer1.shutdown();
			niConsumer1 = null;
		}
		
		if(niConsumer2 != null)
		{
			niConsumer2.shutdown();
			niConsumer2 = null;
		}
		
		if(niConsumer3 != null)
		{
			niConsumer3.shutdown();
			niConsumer3 = null;
		}
		
		if(niConsumer4 != null)
		{
			niConsumer4.shutdown();
			niConsumer4 = null;
		}
		
		if(provider != null)
		{
			provider.uninitialize();
			provider = null;
		}
		
		Transport.uninitialize();
		
		try { Thread.sleep(JUnitConfigVariables.WAIT_AFTER_TEST); }
		catch (Exception e) { }
	}
	
	@Test
	public void NiProviderSingleConnectionTest()
	{
		TestUtilities.printTestHead("NiProviderSingleConnectionTest","");

		NiConsumerHelperConfig niConsConfig = new NiConsumerHelperConfig();
		long itemHandle = 10;
		
		niConsConfig.port = "19001";
		try
		{
			niProvConfig = EmaFactory.createOmmNiProviderConfig(emaConfigFileLocation);
			niConsumer1 = new NiConsumerHelper(niConsConfig);
			
			provider = EmaFactory.createOmmProvider(niProvConfig);
			assertNotNull(provider);
			
			// sleep to make sure that the consumer gets everything
			Thread.sleep(100);
			
			assertEquals(2, niConsumer1.getMsgCount());
			
			com.refinitiv.eta.codec.Msg etaMsg = niConsumer1.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REQUEST);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.LOGIN);
			
			com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsg etaLoginMsg = convertToLoginMsg(etaMsg);
			
			assertEquals(etaLoginMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType.REQUEST);
			com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest etaLoginRequest = (com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest)etaLoginMsg;
			assertEquals(etaLoginRequest.role(), com.refinitiv.eta.rdm.Login.RoleTypes.PROV);
			
			etaMsg = niConsumer1.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.SOURCE);
			
			com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsg etaDirectoryMsg = convertToDirectoryRefreshMsg(etaMsg);
			assertEquals(etaDirectoryMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType.REFRESH);
			com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh etaDirectoryRefresh = (com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh)etaDirectoryMsg;
			assertEquals(etaDirectoryRefresh.serviceList().size(), 2);
			
			FieldList fieldList = EmaFactory.createFieldList();
			fieldList.add( EmaFactory.createFieldEntry().real(22, 3990, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(25, 3994, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(30, 9,  OmmReal.MagnitudeType.EXPONENT_0));
			fieldList.add( EmaFactory.createFieldEntry().real(31, 19, OmmReal.MagnitudeType.EXPONENT_0));
			
			provider.submit( EmaFactory.createRefreshMsg().serviceName("NI_PUB_1").name("IBM.N")
					.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
					.payload(fieldList).complete(true), itemHandle);
			
			Thread.sleep(500);
			
			assertEquals(1, niConsumer1.getMsgCount());
			
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			provider.submit( EmaFactory.createStatusMsg().serviceName("NI_PUB_1").name("IBM.N")
					.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
					, itemHandle);
			
			Thread.sleep(500);
			
			assertEquals(1, niConsumer1.getMsgCount());
			
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.STATUS);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			provider.submit( EmaFactory.createGenericMsg().name("IBM.N").payload(fieldList).complete(true), itemHandle);
			
			Thread.sleep(500);
			
			assertEquals(1, niConsumer1.getMsgCount());
			
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.GENERIC);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			provider.submit( EmaFactory.createUpdateMsg().name("IBM.N").payload(fieldList), itemHandle);
			
			Thread.sleep(500);
			
			assertEquals(1, niConsumer1.getMsgCount());
			
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.UPDATE);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			
			
		} catch(Exception e)
		{
			e.printStackTrace();
			assertTrue( false);
		}
		
	}
	
	@Test
	public void NiProviderSingleConnectionPackedMsgTest()
	{
		TestUtilities.printTestHead("NiProviderSingleConnectionPackedMsgTest","");

		NiConsumerHelperConfig niConsConfig1 = new NiConsumerHelperConfig();

		long itemHandle = 10;
		
		niConsConfig1.port = "19001";

		try
		{
			niProvConfig = EmaFactory.createOmmNiProviderConfig(emaConfigFileLocation);

			niConsumer1 = new NiConsumerHelper(niConsConfig1);
			
			provider = EmaFactory.createOmmProvider(niProvConfig);
			assertNotNull(provider);
			
			// sleep to make sure that the consumer gets everything
			Thread.sleep(100);
			
			assertEquals(2, niConsumer1.getMsgCount());
			
			com.refinitiv.eta.codec.Msg etaMsg = niConsumer1.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REQUEST);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.LOGIN);
			
			com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsg etaLoginMsg = convertToLoginMsg(etaMsg);
			
			assertEquals(etaLoginMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType.REQUEST);
			com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest etaLoginRequest = (com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest)etaLoginMsg;
			assertEquals(etaLoginRequest.role(), com.refinitiv.eta.rdm.Login.RoleTypes.PROV);
			
			etaMsg = niConsumer1.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.SOURCE);
			
			com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsg etaDirectoryMsg = convertToDirectoryRefreshMsg(etaMsg);
			assertEquals(etaDirectoryMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType.REFRESH);
			com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh etaDirectoryRefresh = (com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh)etaDirectoryMsg;
			assertEquals(etaDirectoryRefresh.serviceList().size(), 2);
			
			FieldList fieldList = EmaFactory.createFieldList();
			fieldList.add( EmaFactory.createFieldEntry().real(22, 3990, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(25, 3994, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(30, 9,  OmmReal.MagnitudeType.EXPONENT_0));
			fieldList.add( EmaFactory.createFieldEntry().real(31, 19, OmmReal.MagnitudeType.EXPONENT_0));
			
			
			PackedMsg messagePack = EmaFactory.createPackedMsg(provider);
			
			messagePack.initBuffer();
			
			messagePack.addMsg(EmaFactory.createRefreshMsg().serviceName("NI_PUB_1").name("IBM.N")
					.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
					.payload(fieldList).complete(true), itemHandle);
			
			messagePack.addMsg(EmaFactory.createStatusMsg().serviceName("NI_PUB_1").name("IBM.N")
					.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
					, itemHandle);
			
			messagePack.addMsg(EmaFactory.createGenericMsg().name("IBM.N").payload(fieldList).complete(true), itemHandle);
			
			messagePack.addMsg( EmaFactory.createUpdateMsg().name("IBM.N").payload(fieldList), itemHandle);
			
			provider.submit(messagePack);
			
			Thread.sleep(500);
			
			assertEquals(4, niConsumer1.getMsgCount());
			
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.STATUS);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.GENERIC);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.UPDATE);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
						
		} catch(Exception e)
		{
			e.printStackTrace();
			assertTrue( false);
		}
		
	}
	
	@Test
	public void NiProviderSingleConnectionPackedMsgBuffTooSmallTest()
	{
		TestUtilities.printTestHead("NiProviderSingleConnectionPackedMsgBuffTooSmallTest","");

		NiConsumerHelperConfig niConsConfig1 = new NiConsumerHelperConfig();

		long itemHandle = 10;
		
		niConsConfig1.port = "19001";

		try
		{
			niProvConfig = EmaFactory.createOmmNiProviderConfig(emaConfigFileLocation);
			niConsumer1 = new NiConsumerHelper(niConsConfig1);
			
			provider = EmaFactory.createOmmProvider(niProvConfig);
			assertNotNull(provider);
			
			// sleep to make sure that the consumer gets everything
			Thread.sleep(100);
			
			assertEquals(2, niConsumer1.getMsgCount());
			
			com.refinitiv.eta.codec.Msg etaMsg = niConsumer1.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REQUEST);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.LOGIN);
			
			com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsg etaLoginMsg = convertToLoginMsg(etaMsg);
			
			assertEquals(etaLoginMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType.REQUEST);
			com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest etaLoginRequest = (com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest)etaLoginMsg;
			assertEquals(etaLoginRequest.role(), com.refinitiv.eta.rdm.Login.RoleTypes.PROV);
			
			etaMsg = niConsumer1.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.SOURCE);
			
			com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsg etaDirectoryMsg = convertToDirectoryRefreshMsg(etaMsg);
			assertEquals(etaDirectoryMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType.REFRESH);
			com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh etaDirectoryRefresh = (com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh)etaDirectoryMsg;
			assertEquals(etaDirectoryRefresh.serviceList().size(), 2);
			
			FieldList fieldList = EmaFactory.createFieldList();
			fieldList.add( EmaFactory.createFieldEntry().real(22, 3990, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(25, 3994, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(30, 9,  OmmReal.MagnitudeType.EXPONENT_0));
			fieldList.add( EmaFactory.createFieldEntry().real(31, 19, OmmReal.MagnitudeType.EXPONENT_0));
			
			PackedMsg messagePack = EmaFactory.createPackedMsg(provider);
			
			messagePack.initBuffer(200);
			
			messagePack.addMsg(EmaFactory.createRefreshMsg().serviceName("NI_PUB_1").name("IBM.N")
					.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
					.payload(fieldList).complete(true), itemHandle);
			
			messagePack.addMsg( EmaFactory.createUpdateMsg().id(1).name("IBM.N").payload(fieldList), itemHandle);
	
			messagePack.addMsg( EmaFactory.createUpdateMsg().id(2).name("IBM.N").payload(fieldList), itemHandle);

			try
			{
				messagePack.addMsg( EmaFactory.createUpdateMsg().id(3).name("IBM.N").payload(fieldList), itemHandle);
				assertFalse(true);
			}
			catch(OmmInvalidUsageException e)
			{
				assertEquals(e.errorCode(), OmmInvalidUsageException.ErrorCode.BUFFER_TOO_SMALL);
				
			}

			provider.submit(messagePack);
			
			Thread.sleep(500);
			
			assertEquals(3, niConsumer1.getMsgCount());
			
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
				
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.UPDATE);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			assertEquals(etaMsg.msgKey().identifier(), 1);
			
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.UPDATE);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			assertEquals(etaMsg.msgKey().identifier(), 2);

		} catch(Exception e)
		{
			e.printStackTrace();
			assertTrue( false);
		}
		
	}
	
	@Test
	public void NiProviderSingleConnectionSubmitBadDirectoryNameTest()
	{
		TestUtilities.printTestHead("NiProviderSingleConnectionTest","");

		NiConsumerHelperConfig niConsConfig = new NiConsumerHelperConfig();
		long itemHandle = 10;
		
		niConsConfig.port = "19001";
		try
		{
			niProvConfig = EmaFactory.createOmmNiProviderConfig(emaConfigFileLocation);
			niConsumer1 = new NiConsumerHelper(niConsConfig);
			
			provider = EmaFactory.createOmmProvider(niProvConfig);
			assertNotNull(provider);
			
			// sleep to make sure that the consumer gets everything
			Thread.sleep(100);
			
			assertEquals(2, niConsumer1.getMsgCount());
			
			com.refinitiv.eta.codec.Msg etaMsg = niConsumer1.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REQUEST);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.LOGIN);
			
			com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsg etaLoginMsg = convertToLoginMsg(etaMsg);
			
			assertEquals(etaLoginMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType.REQUEST);
			com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest etaLoginRequest = (com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest)etaLoginMsg;
			assertEquals(etaLoginRequest.role(), com.refinitiv.eta.rdm.Login.RoleTypes.PROV);
			
			etaMsg = niConsumer1.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.SOURCE);
			
			com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsg etaDirectoryMsg = convertToDirectoryRefreshMsg(etaMsg);
			assertEquals(etaDirectoryMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType.REFRESH);
			com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh etaDirectoryRefresh = (com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh)etaDirectoryMsg;
			assertEquals(etaDirectoryRefresh.serviceList().size(), 2);
			
			FieldList fieldList = EmaFactory.createFieldList();
			fieldList.add( EmaFactory.createFieldEntry().real(22, 3990, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(25, 3994, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(30, 9,  OmmReal.MagnitudeType.EXPONENT_0));
			fieldList.add( EmaFactory.createFieldEntry().real(31, 19, OmmReal.MagnitudeType.EXPONENT_0));
			
			try
			{
				provider.submit( EmaFactory.createRefreshMsg().serviceName("BAD_SERVICE").name("IBM.N")
						.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
						.payload(fieldList).complete(true), itemHandle);
				assertTrue(false);
			} catch (OmmInvalidUsageException except)
			{
				System.out.println("Exception thrown on bad serviceName for refresh msg.");
			}
			
			try
			{
				provider.submit( EmaFactory.createStatusMsg().serviceName("BAD_SERVICE").name("IBM.N")
						.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
						.payload(fieldList), itemHandle);
				assertTrue(false);
			} catch (OmmInvalidUsageException except)
			{
				System.out.println("Exception thrown on bad serviceName for status msg.");
			}
			
			try
			{
				provider.submit( EmaFactory.createUpdateMsg().serviceName("BAD_SERVICE").name("IBM.N")
						.payload(fieldList), itemHandle);
				assertTrue(false);
			} catch (OmmInvalidUsageException except)
			{
				System.out.println("Exception thrown on bad serviceName for update msg.");
			}
			
			assertEquals(0, niConsumer1.getMsgCount());
			
		} catch(Exception e)
		{
			e.printStackTrace();
			assertTrue( false);
		}
		
	}
	
	@Test
	public void NiProviderSingleConnectionSubmitBadHandleTest()
	{
		TestUtilities.printTestHead("NiProviderSingleConnectionTest","");

		NiConsumerHelperConfig niConsConfig = new NiConsumerHelperConfig();
		long itemHandle = 10;
		long badHandle = 11;
		
		niConsConfig.port = "19001";
		try
		{
			niProvConfig = EmaFactory.createOmmNiProviderConfig(emaConfigFileLocation);
			niConsumer1 = new NiConsumerHelper(niConsConfig);
			
			provider = EmaFactory.createOmmProvider(niProvConfig);
			assertNotNull(provider);
			
			// sleep to make sure that the consumer gets everything
			Thread.sleep(100);
			
			assertEquals(2, niConsumer1.getMsgCount());
			
			com.refinitiv.eta.codec.Msg etaMsg = niConsumer1.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REQUEST);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.LOGIN);
			
			com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsg etaLoginMsg = convertToLoginMsg(etaMsg);
			
			assertEquals(etaLoginMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType.REQUEST);
			com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest etaLoginRequest = (com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest)etaLoginMsg;
			assertEquals(etaLoginRequest.role(), com.refinitiv.eta.rdm.Login.RoleTypes.PROV);
			
			etaMsg = niConsumer1.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.SOURCE);
			
			com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsg etaDirectoryMsg = convertToDirectoryRefreshMsg(etaMsg);
			assertEquals(etaDirectoryMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType.REFRESH);
			com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh etaDirectoryRefresh = (com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh)etaDirectoryMsg;
			assertEquals(etaDirectoryRefresh.serviceList().size(), 2);
			
			FieldList fieldList = EmaFactory.createFieldList();
			fieldList.add( EmaFactory.createFieldEntry().real(22, 3990, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(25, 3994, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(30, 9,  OmmReal.MagnitudeType.EXPONENT_0));
			fieldList.add( EmaFactory.createFieldEntry().real(31, 19, OmmReal.MagnitudeType.EXPONENT_0));
			
			try
			{
				provider.submit( EmaFactory.createGenericMsg().name("IBM.N")
						.payload(fieldList).complete(true), badHandle);
				assertTrue(false);
			} catch (OmmInvalidHandleException except)
			{
				System.out.println("Exception thrown on bad handle on generic msg.");
			}
			
			// Add in the item.
			provider.submit( EmaFactory.createRefreshMsg().serviceName("NI_PUB_1").name("IBM.N")
					.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
					.payload(fieldList).complete(true), itemHandle);
			
			Thread.sleep(500);
			
			assertEquals(1, niConsumer1.getMsgCount());
			
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			try
			{
				provider.submit( EmaFactory.createGenericMsg().name("IBM.N")
						.payload(fieldList).complete(true), badHandle);
				assertTrue(false);
			} catch (OmmInvalidHandleException except)
			{
				System.out.println("Exception thrown on bad handle on generic msg after adding a handle.");
			}
			
		} catch(Exception e)
		{
			e.printStackTrace();
			assertTrue( false);
		}
		
	}
		
	// RecoverUserSubmitSourceDirectory on, RemoveItemsOnDisconnect on)
	@Test
	public void NiProviderSingleConnectionDirectoryTestRecoverUserOnRemoveItemsOn()
	{
		TestUtilities.printTestHead("NiProviderSingleConnectionDirectoryTestRecoverUserOnRemoveItemsOn","");

		NiConsumerHelperConfig niConsConfig = new NiConsumerHelperConfig();
		long itemHandle = 10;
		
		niConsConfig.port = "19001";
		try
		{
			niProvConfig = EmaFactory.createOmmNiProviderConfig(emaConfigFileLocation);
			niConsumer1 = new NiConsumerHelper(niConsConfig);
			
			niProvConfig.adminControlDirectory(OmmNiProviderConfig.AdminControl.USER_CONTROL);
			
			provider = EmaFactory.createOmmProvider(niProvConfig);
			assertNotNull(provider);
			
			// sleep to make sure that the consumer gets everything
			Thread.sleep(100);
			
			assertEquals(1, niConsumer1.getMsgCount());
			
			com.refinitiv.eta.codec.Msg etaMsg = niConsumer1.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REQUEST);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.LOGIN);
			
			com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsg etaLoginMsg = convertToLoginMsg(etaMsg);
			
			assertEquals(etaLoginMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType.REQUEST);
			com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest etaLoginRequest = (com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest)etaLoginMsg;
			assertEquals(etaLoginRequest.role(), com.refinitiv.eta.rdm.Login.RoleTypes.PROV);
			
			FieldList fieldList = EmaFactory.createFieldList();
			fieldList.add( EmaFactory.createFieldEntry().real(22, 3990, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(25, 3994, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(30, 9,  OmmReal.MagnitudeType.EXPONENT_0));
			fieldList.add( EmaFactory.createFieldEntry().real(31, 19, OmmReal.MagnitudeType.EXPONENT_0));
			
			// Attempt to submit a refresh with NI_PUB_1 before submitting the directory.
			try
			{
				provider.submit( EmaFactory.createRefreshMsg().serviceName("NI_PUB_1").name("IBM.N")
						.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
						.payload(fieldList).complete(true), itemHandle);
				assertTrue(false);
			} catch (OmmInvalidUsageException except)
			{
				System.out.println("Exception thrown on not available NI_PUB_1 service name for refresh msg.");
			}
			
			long sourceDirectoryHandle = 1;
			
            OmmArray capablities = EmaFactory.createOmmArray();
            capablities.add(EmaFactory.createOmmArrayEntry().uintValue( EmaRdm.MMT_MARKET_PRICE));
            capablities.add(EmaFactory.createOmmArrayEntry().uintValue( EmaRdm.MMT_MARKET_BY_PRICE));
            OmmArray dictionaryUsed = EmaFactory.createOmmArray();
            dictionaryUsed.add(EmaFactory.createOmmArrayEntry().ascii( "RWFFld"));
            dictionaryUsed.add(EmaFactory.createOmmArrayEntry().ascii( "RWFEnum"));
            
            ElementList serviceInfoId = EmaFactory.createElementList();    
            
            serviceInfoId.add( EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_NAME, "NI_PUB_1"));     
            serviceInfoId.add( EmaFactory.createElementEntry().array(EmaRdm.ENAME_CAPABILITIES, capablities));         
            serviceInfoId.add( EmaFactory.createElementEntry().array(EmaRdm.ENAME_DICTIONARYS_USED, dictionaryUsed));

            ElementList serviceStateId = EmaFactory.createElementList();
            serviceStateId.add( EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_UP));
				
            FilterList filterList = EmaFactory.createFilterList();
            filterList.add( EmaFactory.createFilterEntry().elementList(EmaRdm.SERVICE_INFO_ID, FilterEntry.FilterAction.SET, serviceInfoId) );
            filterList.add( EmaFactory.createFilterEntry().elementList(EmaRdm.SERVICE_STATE_ID, FilterEntry.FilterAction.SET, serviceStateId));
            
            Map map = EmaFactory.createMap();
            map.add( EmaFactory.createMapEntry().keyUInt(2, MapEntry.MapAction.ADD, filterList));
            
            RefreshMsg refreshMsg = EmaFactory.createRefreshMsg();
            provider.submit( refreshMsg.domainType(EmaRdm.MMT_DIRECTORY).filter( EmaRdm.SERVICE_INFO_FILTER | EmaRdm.SERVICE_STATE_FILTER).payload(map)
                    ,sourceDirectoryHandle );
            
            Thread.sleep(500);
			
			assertEquals(1, niConsumer1.getMsgCount());
			
			etaMsg = niConsumer1.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.SOURCE);
			
			com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsg etaDirectoryMsg = convertToDirectoryRefreshMsg(etaMsg);
			assertEquals(etaDirectoryMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType.REFRESH);
			com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh etaDirectoryRefresh = (com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh)etaDirectoryMsg;
			assertEquals(etaDirectoryRefresh.serviceList().size(), 1);
			com.refinitiv.eta.valueadd.domainrep.rdm.directory.Service service = etaDirectoryRefresh.serviceList().get(0);
			assertEquals(service.info().serviceName().toString(), "NI_PUB_1");
			assertEquals(service.state().serviceState(), EmaRdm.SERVICE_UP);
			
			provider.submit( EmaFactory.createRefreshMsg().serviceName("NI_PUB_1").name("IBM.N")
					.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
					.payload(fieldList).complete(true), itemHandle);
			
			Thread.sleep(500);
			
			assertEquals(1, niConsumer1.getMsgCount());
			
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			provider.submit( EmaFactory.createStatusMsg().serviceName("NI_PUB_1").name("IBM.N")
					.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
					, itemHandle);
			
			Thread.sleep(500);
			
			assertEquals(1, niConsumer1.getMsgCount());
			
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.STATUS);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			provider.submit( EmaFactory.createGenericMsg().name("IBM.N").payload(fieldList).complete(true), itemHandle);
			
			Thread.sleep(500);
			
			assertEquals(1, niConsumer1.getMsgCount());
			
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.GENERIC);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			provider.submit( EmaFactory.createUpdateMsg().name("IBM.N").payload(fieldList), itemHandle);
			
			Thread.sleep(500);
			
			assertEquals(1, niConsumer1.getMsgCount());
			
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.UPDATE);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			niConsumer1.closeChannel();
			
			Thread.sleep(100);
			
			// This should fail because the channel is down.
			try
			{
				provider.submit( EmaFactory.createRefreshMsg().serviceName("NI_PUB_1").name("IBM.N")
						.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
						.payload(fieldList).complete(true), itemHandle);
				assertTrue(false);
			} catch (OmmInvalidUsageException except)
			{
				System.out.println("Exception thrown on submit attempt with channel down.");
			}
			
			Thread.sleep(1500);
			
			// Channel's backup, check to see if the cached messages are sent correctly.
			assertEquals(2, niConsumer1.getMsgCount());
			
			etaMsg = niConsumer1.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REQUEST);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.LOGIN);
			
			etaLoginMsg = convertToLoginMsg(etaMsg);
			
			assertEquals(etaLoginMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType.REQUEST);
			etaLoginRequest = (com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest)etaLoginMsg;
			assertEquals(etaLoginRequest.role(), com.refinitiv.eta.rdm.Login.RoleTypes.PROV);
			
			etaMsg = niConsumer1.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.SOURCE);
			
			etaDirectoryMsg = convertToDirectoryRefreshMsg(etaMsg);
			assertEquals(etaDirectoryMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType.REFRESH);
			etaDirectoryRefresh = (com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh)etaDirectoryMsg;
			assertEquals(etaDirectoryRefresh.serviceList().size(), 1);
			service = etaDirectoryRefresh.serviceList().get(0);
			assertEquals(service.info().serviceName().toString(), "NI_PUB_1");
			assertEquals(service.state().serviceState(), EmaRdm.SERVICE_UP);
			
			provider.submit( EmaFactory.createRefreshMsg().serviceName("NI_PUB_1").name("IBM.N")
					.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
					.payload(fieldList).complete(true), itemHandle);
			
			Thread.sleep(500);
			
			assertEquals(1, niConsumer1.getMsgCount());
			
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
		} catch(Exception e)
		{
			e.printStackTrace();
			assertTrue( false);
		}
		
	}
	
	// RecoverUserSubmitSourceDirectory off, RemoveItemsOnDisconnect on)
	@Test
	public void NiProviderSingleConnectionDirectoryTestRecoverUserOffRemoveItemsOn()
	{
		TestUtilities.printTestHead("NiProviderSingleConnectionDirectoryTestRecoverUserOffRemoveItemsOn","");

		NiConsumerHelperConfig niConsConfig = new NiConsumerHelperConfig();
		long itemHandle = 10;
		
		niConsConfig.port = "19001";
		try
		{
			niProvConfig = EmaFactory.createOmmNiProviderConfig(emaConfigFileLocation);
			niProvConfig.providerName("SingleProvider_RecoverUserSubmitSourceDirectoryOff");
			niConsumer1 = new NiConsumerHelper(niConsConfig);
			
			niProvConfig.adminControlDirectory(OmmNiProviderConfig.AdminControl.USER_CONTROL);
			
			provider = EmaFactory.createOmmProvider(niProvConfig);
			assertNotNull(provider);
			
			// sleep to make sure that the consumer gets everything
			Thread.sleep(100);
			
			assertEquals(1, niConsumer1.getMsgCount());
			
			com.refinitiv.eta.codec.Msg etaMsg = niConsumer1.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REQUEST);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.LOGIN);
			
			com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsg etaLoginMsg = convertToLoginMsg(etaMsg);
			
			assertEquals(etaLoginMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType.REQUEST);
			com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest etaLoginRequest = (com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest)etaLoginMsg;
			assertEquals(etaLoginRequest.role(), com.refinitiv.eta.rdm.Login.RoleTypes.PROV);
			
			FieldList fieldList = EmaFactory.createFieldList();
			fieldList.add( EmaFactory.createFieldEntry().real(22, 3990, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(25, 3994, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(30, 9,  OmmReal.MagnitudeType.EXPONENT_0));
			fieldList.add( EmaFactory.createFieldEntry().real(31, 19, OmmReal.MagnitudeType.EXPONENT_0));
			
			// Attempt to submit a refresh with NI_PUB_1 before submitting the directory.
			try
			{
				provider.submit( EmaFactory.createRefreshMsg().serviceName("NI_PUB_1").name("IBM.N")
						.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
						.payload(fieldList).complete(true), itemHandle);
				assertTrue(false);
			} catch (OmmInvalidUsageException except)
			{
				System.out.println("Exception thrown on not available NI_PUB_1 service name for refresh msg.");
			}
			
			long sourceDirectoryHandle = 1;
			
            OmmArray capablities = EmaFactory.createOmmArray();
            capablities.add(EmaFactory.createOmmArrayEntry().uintValue( EmaRdm.MMT_MARKET_PRICE));
            capablities.add(EmaFactory.createOmmArrayEntry().uintValue( EmaRdm.MMT_MARKET_BY_PRICE));
            OmmArray dictionaryUsed = EmaFactory.createOmmArray();
            dictionaryUsed.add(EmaFactory.createOmmArrayEntry().ascii( "RWFFld"));
            dictionaryUsed.add(EmaFactory.createOmmArrayEntry().ascii( "RWFEnum"));
            
            ElementList serviceInfoId = EmaFactory.createElementList();    
            
            serviceInfoId.add( EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_NAME, "NI_PUB_1"));     
            serviceInfoId.add( EmaFactory.createElementEntry().array(EmaRdm.ENAME_CAPABILITIES, capablities));         
            serviceInfoId.add( EmaFactory.createElementEntry().array(EmaRdm.ENAME_DICTIONARYS_USED, dictionaryUsed));

            ElementList serviceStateId = EmaFactory.createElementList();
            serviceStateId.add( EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_UP));
				
            FilterList filterList = EmaFactory.createFilterList();
            filterList.add( EmaFactory.createFilterEntry().elementList(EmaRdm.SERVICE_INFO_ID, FilterEntry.FilterAction.SET, serviceInfoId) );
            filterList.add( EmaFactory.createFilterEntry().elementList(EmaRdm.SERVICE_STATE_ID, FilterEntry.FilterAction.SET, serviceStateId));
            
            Map map = EmaFactory.createMap();
            map.add( EmaFactory.createMapEntry().keyUInt(2, MapEntry.MapAction.ADD, filterList));
            
            RefreshMsg refreshMsg = EmaFactory.createRefreshMsg();
            provider.submit( refreshMsg.domainType(EmaRdm.MMT_DIRECTORY).filter( EmaRdm.SERVICE_INFO_FILTER | EmaRdm.SERVICE_STATE_FILTER).payload(map)
                    ,sourceDirectoryHandle );
            
            Thread.sleep(500);
			
			assertEquals(1, niConsumer1.getMsgCount());
			
			etaMsg = niConsumer1.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.SOURCE);
			
			com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsg etaDirectoryMsg = convertToDirectoryRefreshMsg(etaMsg);
			assertEquals(etaDirectoryMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType.REFRESH);
			com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh etaDirectoryRefresh = (com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh)etaDirectoryMsg;
			assertEquals(etaDirectoryRefresh.serviceList().size(), 1);
			com.refinitiv.eta.valueadd.domainrep.rdm.directory.Service service = etaDirectoryRefresh.serviceList().get(0);
			assertEquals(service.info().serviceName().toString(), "NI_PUB_1");
			assertEquals(service.state().serviceState(), EmaRdm.SERVICE_UP);
			
			provider.submit( EmaFactory.createRefreshMsg().serviceName("NI_PUB_1").name("IBM.N")
					.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
					.payload(fieldList).complete(true), itemHandle);
			
			Thread.sleep(500);
			
			assertEquals(1, niConsumer1.getMsgCount());
			
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			provider.submit( EmaFactory.createStatusMsg().serviceName("NI_PUB_1").name("IBM.N")
					.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
					, itemHandle);
			
			Thread.sleep(500);
			
			assertEquals(1, niConsumer1.getMsgCount());
			
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.STATUS);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			provider.submit( EmaFactory.createGenericMsg().name("IBM.N").payload(fieldList).complete(true), itemHandle);
			
			Thread.sleep(500);
			
			assertEquals(1, niConsumer1.getMsgCount());
			
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.GENERIC);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			provider.submit( EmaFactory.createUpdateMsg().name("IBM.N").payload(fieldList), itemHandle);
			
			Thread.sleep(500);
			
			assertEquals(1, niConsumer1.getMsgCount());
			
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.UPDATE);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			niConsumer1.closeChannel();
			
			Thread.sleep(100);
			
			// This should fail because the channel is down.
			try
			{
				provider.submit( EmaFactory.createRefreshMsg().serviceName("NI_PUB_1").name("IBM.N")
						.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
						.payload(fieldList).complete(true), itemHandle);
				assertTrue(false);
			} catch (OmmInvalidUsageException except)
			{
				System.out.println("Exception thrown on submit attempt with channel down.");
			}
			
			Thread.sleep(1500);
			
			// Channel's backup, check to see if the cached messages are sent correctly.
			assertEquals(1, niConsumer1.getMsgCount());
			
			etaMsg = niConsumer1.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REQUEST);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.LOGIN);
			
			etaLoginMsg = convertToLoginMsg(etaMsg);
			
			assertEquals(etaLoginMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType.REQUEST);
			etaLoginRequest = (com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest)etaLoginMsg;
			assertEquals(etaLoginRequest.role(), com.refinitiv.eta.rdm.Login.RoleTypes.PROV);
			
			// This should fail because there are no services available.
			try
			{
				provider.submit( EmaFactory.createRefreshMsg().serviceName("NI_PUB_1").name("IBM.N")
						.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
						.payload(fieldList).complete(true), itemHandle);
				assertTrue(false);
			} catch (OmmInvalidUsageException except)
			{
				System.out.println("Exception thrown on submit attempt with channel down.");
			}
			
		} catch(Exception e)
		{
			e.printStackTrace();
			assertTrue( false);
		}
		
	}
	
	@Test
	public void NiProviderSingleConnectionDirectoryUpdateTestRecover()
	{
		TestUtilities.printTestHead("NiProviderTwoConnectionSessionDirectoryTestRecover","");

		NiConsumerHelperConfig niConsConfig1 = new NiConsumerHelperConfig();

		long itemHandle = 10;
		long itemHandle_2 = 12;
		
		niConsConfig1.port = "19001";

		try
		{
			niProvConfig = EmaFactory.createOmmNiProviderConfig(emaConfigFileLocation);
			niProvConfig.adminControlDirectory(OmmNiProviderConfig.AdminControl.USER_CONTROL);
			niConsumer1 = new NiConsumerHelper(niConsConfig1);
			
			provider = EmaFactory.createOmmProvider(niProvConfig);
			assertNotNull(provider);
			
			// sleep to make sure that the consumer gets everything
			Thread.sleep(100);
			
			assertEquals(1, niConsumer1.getMsgCount());
			
			com.refinitiv.eta.codec.Msg etaMsg = niConsumer1.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REQUEST);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.LOGIN);
			
			com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsg etaLoginMsg = convertToLoginMsg(etaMsg);
			
			assertEquals(etaLoginMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType.REQUEST);
			com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest etaLoginRequest = (com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest)etaLoginMsg;
			assertEquals(etaLoginRequest.role(), com.refinitiv.eta.rdm.Login.RoleTypes.PROV);
			
			
		FieldList fieldList = EmaFactory.createFieldList();
			fieldList.add( EmaFactory.createFieldEntry().real(22, 3990, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(25, 3994, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(30, 9,  OmmReal.MagnitudeType.EXPONENT_0));
			fieldList.add( EmaFactory.createFieldEntry().real(31, 19, OmmReal.MagnitudeType.EXPONENT_0));
			
			// Attempt to submit a refresh with NI_PUB_1 before submitting the directory.
			try
			{
				provider.submit( EmaFactory.createRefreshMsg().serviceName("NI_PUB_1").name("IBM.N")
						.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
						.payload(fieldList).complete(true), itemHandle);
				assertTrue(false);
			} catch (OmmInvalidUsageException except)
			{
				System.out.println("Exception thrown on not available NI_PUB_1 service name for refresh msg.");
			}
			
			long sourceDirectoryHandle = 1;
			
            OmmArray capablities = EmaFactory.createOmmArray();
            capablities.add(EmaFactory.createOmmArrayEntry().uintValue( EmaRdm.MMT_MARKET_PRICE));
            capablities.add(EmaFactory.createOmmArrayEntry().uintValue( EmaRdm.MMT_MARKET_BY_PRICE));
            OmmArray dictionaryUsed = EmaFactory.createOmmArray();
            dictionaryUsed.add(EmaFactory.createOmmArrayEntry().ascii( "RWFFld"));
            dictionaryUsed.add(EmaFactory.createOmmArrayEntry().ascii( "RWFEnum"));
            
            ElementList serviceInfoId = EmaFactory.createElementList();    
            
            serviceInfoId.add( EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_NAME, "NI_PUB_1"));     
            serviceInfoId.add( EmaFactory.createElementEntry().array(EmaRdm.ENAME_CAPABILITIES, capablities));         
            serviceInfoId.add( EmaFactory.createElementEntry().array(EmaRdm.ENAME_DICTIONARYS_USED, dictionaryUsed));

            ElementList serviceStateId = EmaFactory.createElementList();
            serviceStateId.add( EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_UP));
				
            FilterList filterList = EmaFactory.createFilterList();
            filterList.add( EmaFactory.createFilterEntry().elementList(EmaRdm.SERVICE_INFO_ID, FilterEntry.FilterAction.SET, serviceInfoId) );
            filterList.add( EmaFactory.createFilterEntry().elementList(EmaRdm.SERVICE_STATE_ID, FilterEntry.FilterAction.SET, serviceStateId));
            
            Map map = EmaFactory.createMap();
            map.add( EmaFactory.createMapEntry().keyUInt(2, MapEntry.MapAction.ADD, filterList));
            
            capablities.clear();
            capablities.add(EmaFactory.createOmmArrayEntry().uintValue( EmaRdm.MMT_MARKET_PRICE));
            capablities.add(EmaFactory.createOmmArrayEntry().uintValue( EmaRdm.MMT_MARKET_BY_PRICE));
            dictionaryUsed.clear();
            dictionaryUsed.add(EmaFactory.createOmmArrayEntry().ascii( "RWFFld"));
            dictionaryUsed.add(EmaFactory.createOmmArrayEntry().ascii( "RWFEnum"));
            
            serviceInfoId.clear();
            
            serviceInfoId.add( EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_NAME, "NI_PUB_2"));     
            serviceInfoId.add( EmaFactory.createElementEntry().array(EmaRdm.ENAME_CAPABILITIES, capablities));         
            serviceInfoId.add( EmaFactory.createElementEntry().array(EmaRdm.ENAME_DICTIONARYS_USED, dictionaryUsed));

            serviceStateId.clear();
            serviceStateId.add( EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_UP));
				
            filterList.clear();
            filterList.add( EmaFactory.createFilterEntry().elementList(EmaRdm.SERVICE_INFO_ID, FilterEntry.FilterAction.SET, serviceInfoId) );
            filterList.add( EmaFactory.createFilterEntry().elementList(EmaRdm.SERVICE_STATE_ID, FilterEntry.FilterAction.SET, serviceStateId));
            
            map.add( EmaFactory.createMapEntry().keyUInt(3, MapEntry.MapAction.ADD, filterList));
            
            RefreshMsg refreshMsg = EmaFactory.createRefreshMsg();
            provider.submit( refreshMsg.domainType(EmaRdm.MMT_DIRECTORY).filter( EmaRdm.SERVICE_INFO_FILTER | EmaRdm.SERVICE_STATE_FILTER).payload(map)
                    ,sourceDirectoryHandle );
            
            Thread.sleep(500);
            
            assertEquals(1, niConsumer1.getMsgCount());
			
			etaMsg = niConsumer1.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.SOURCE);
			
			com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsg etaDirectoryMsg = convertToDirectoryRefreshMsg(etaMsg);
			assertEquals(etaDirectoryMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType.REFRESH);
			com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh etaDirectoryRefresh = (com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh)etaDirectoryMsg;
			assertEquals(etaDirectoryRefresh.serviceList().size(), 2);
			com.refinitiv.eta.valueadd.domainrep.rdm.directory.Service service = etaDirectoryRefresh.serviceList().get(0);
			assertEquals(service.info().serviceName().toString(), "NI_PUB_1");
			assertEquals(service.state().serviceState(), EmaRdm.SERVICE_UP);
			service = etaDirectoryRefresh.serviceList().get(1);
			assertEquals(service.info().serviceName().toString(), "NI_PUB_2");
			assertEquals(service.state().serviceState(), EmaRdm.SERVICE_UP);
			
			provider.submit( EmaFactory.createRefreshMsg().serviceName("NI_PUB_1").name("IBM.N")
					.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
					.payload(fieldList).complete(true), itemHandle);
			
			Thread.sleep(100);
			
			assertEquals(1, niConsumer1.getMsgCount());
			
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			provider.submit( EmaFactory.createRefreshMsg().serviceName("NI_PUB_2").name("TEST_2")
					.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
					.payload(fieldList).complete(true), itemHandle);
			
			Thread.sleep(100);
			
			assertEquals(1, niConsumer1.getMsgCount());
			
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.msgKey().name().toString(), "TEST_2");
			
			// Send update with delete action
			map.clear();
			filterList.clear();
            map.add( EmaFactory.createMapEntry().keyUInt(2, MapEntry.MapAction.DELETE, filterList));
            
            UpdateMsg updateMsg = EmaFactory.createUpdateMsg();
            provider.submit( updateMsg.domainType(EmaRdm.MMT_DIRECTORY).filter( EmaRdm.SERVICE_STATE_FILTER).payload(map)
                    ,sourceDirectoryHandle );
            
            Thread.sleep(500);
            
            assertEquals(1, niConsumer1.getMsgCount());
			
			etaMsg = niConsumer1.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.UPDATE);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.SOURCE);
			
			etaDirectoryMsg = convertToDirectoryUpdateMsg(etaMsg);
			assertEquals(etaDirectoryMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType.UPDATE);
			com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryUpdate etaDirectoryUpdate = (com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryUpdate)etaDirectoryMsg;
			assertEquals(etaDirectoryUpdate.serviceList().size(), 1);
			service = etaDirectoryUpdate.serviceList().get(0);
			assertEquals(service.serviceId(), 2);
			assertEquals(service.action(), com.refinitiv.eta.codec.MapEntryActions.DELETE);
			
			try
			{
				provider.submit( EmaFactory.createRefreshMsg().serviceName("NI_PUB_1").name("TEST_3")
						.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
						.payload(fieldList).complete(true), 20);
				assertTrue(false);
			} catch (OmmInvalidUsageException except)
			{
				System.out.println("Exception thrown on just deleted NI_PUB_1 service name for refresh msg.");
			}

			niConsumer1.closeChannel();
			Thread.sleep(150);
			
			try
			{
				provider.submit( EmaFactory.createRefreshMsg().serviceName("NI_PUB_1").name("IBM.N")
						.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
						.payload(fieldList).complete(true), itemHandle);
				assertTrue(false);
			} catch (OmmInvalidUsageException except)
			{
				System.out.println("Exception thrown on not available NI_PUB_1 service name for refresh msg.");
			}
			
			Thread.sleep(1500);
			
			assertEquals(2, niConsumer1.getMsgCount());
			
			etaMsg = niConsumer1.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REQUEST);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.LOGIN);
			
			etaLoginMsg = convertToLoginMsg(etaMsg);
			
			assertEquals(etaLoginMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType.REQUEST);
			etaLoginRequest = (com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest)etaLoginMsg;
			assertEquals(etaLoginRequest.role(), com.refinitiv.eta.rdm.Login.RoleTypes.PROV);
			
			etaMsg = niConsumer1.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.SOURCE);
			
			etaDirectoryMsg = convertToDirectoryRefreshMsg(etaMsg);
			assertEquals(etaDirectoryMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType.REFRESH);
			etaDirectoryRefresh = (com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh)etaDirectoryMsg;
			assertEquals(etaDirectoryRefresh.serviceList().size(), 1);
			service = etaDirectoryRefresh.serviceList().get(0);
			assertEquals(service.info().serviceName().toString(), "NI_PUB_2");
			assertEquals(service.state().serviceState(), EmaRdm.SERVICE_UP);
			
			
			provider.submit( EmaFactory.createRefreshMsg().serviceName("NI_PUB_2").name("TEST_2")
					.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
					.payload(fieldList).complete(true), itemHandle_2);
			
			Thread.sleep(500);
			
			assertEquals(1, niConsumer1.getMsgCount());
			
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.msgKey().name().toString(), "TEST_2");
			
			
			// Send update with delete action on other service
			map.clear();
			filterList.clear();
	        map.add( EmaFactory.createMapEntry().keyUInt(3, MapEntry.MapAction.DELETE, filterList));
	        
	        updateMsg = EmaFactory.createUpdateMsg();
	        provider.submit( updateMsg.domainType(EmaRdm.MMT_DIRECTORY).filter( EmaRdm.SERVICE_STATE_FILTER).payload(map)
	                ,sourceDirectoryHandle );
	        
	        Thread.sleep(500);
	        
	        assertEquals(1, niConsumer1.getMsgCount());
			
			etaMsg = niConsumer1.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.UPDATE);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.SOURCE);
			
			etaDirectoryMsg = convertToDirectoryUpdateMsg(etaMsg);
			assertEquals(etaDirectoryMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType.UPDATE);
			etaDirectoryUpdate = (com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryUpdate)etaDirectoryMsg;
			assertEquals(etaDirectoryUpdate.serviceList().size(), 1);
			service = etaDirectoryUpdate.serviceList().get(0);
			assertEquals(service.serviceId(), 3);
			assertEquals(service.action(), com.refinitiv.eta.codec.MapEntryActions.DELETE);
			
			// Send a new message on NI_PUB_2, should thrown an exception
			try
			{
				provider.submit( EmaFactory.createRefreshMsg().serviceName("NI_PUB_2").name("TEST_3")
						.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
						.payload(fieldList).complete(true), 30);
				assertTrue(false);
			} catch (OmmInvalidUsageException except)
			{
				System.out.println("Exception thrown on just deleted NI_PUB_2 service name for refresh msg.");
			}
	
			// Shutdown both consumers
			niConsumer1.closeChannel();
			Thread.sleep(1500);
			
			// Consumer 1 will only get the login message
			assertEquals(1, niConsumer1.getMsgCount());
			
			etaMsg = niConsumer1.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REQUEST);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.LOGIN);
			
			etaLoginMsg = convertToLoginMsg(etaMsg);
			
			assertEquals(etaLoginMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType.REQUEST);
			etaLoginRequest = (com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest)etaLoginMsg;
			assertEquals(etaLoginRequest.role(), com.refinitiv.eta.rdm.Login.RoleTypes.PROV);
			
			Thread.sleep(500);
			
		} catch(Exception e)
		{
			e.printStackTrace();
			assertTrue( false);
		}
	}
	
	@Test
	public void NiProviderTwoConnectionSessionTest()
	{
		TestUtilities.printTestHead("NiProviderTwoConnectionSessionTest","");

		NiConsumerHelperConfig niConsConfig1 = new NiConsumerHelperConfig();
		NiConsumerHelperConfig niConsConfig2 = new NiConsumerHelperConfig();
		NiProviderTestClient loginClient = new NiProviderTestClient();

		long itemHandle = 10;
		
		niConsConfig1.port = "19001";
		niConsConfig2.port = "19002";

		try
		{
			niProvConfig = EmaFactory.createOmmNiProviderConfig(emaConfigFileLocation);
			niProvConfig.providerName("Provider_Session_1");
			niConsumer1 = new NiConsumerHelper(niConsConfig1);
			niConsumer2 = new NiConsumerHelper(niConsConfig2);
			
			provider = EmaFactory.createOmmProvider(niProvConfig, loginClient);
			assertNotNull(provider);
			
			// sleep to make sure that the consumer gets everything
			Thread.sleep(100);
			
			// sleep to make sure that the consumer gets everything
			Thread.sleep(100);
			
			assertEquals(2, niConsumer1.getMsgCount());
			
			com.refinitiv.eta.codec.Msg etaMsg = niConsumer1.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REQUEST);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.LOGIN);
			
			com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsg etaLoginMsg = convertToLoginMsg(etaMsg);
			
			assertEquals(etaLoginMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType.REQUEST);
			com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest etaLoginRequest = (com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest)etaLoginMsg;
			assertEquals(etaLoginRequest.role(), com.refinitiv.eta.rdm.Login.RoleTypes.PROV);
			
			etaMsg = niConsumer1.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.SOURCE);
			
			com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsg etaDirectoryMsg = convertToDirectoryRefreshMsg(etaMsg);
			assertEquals(etaDirectoryMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType.REFRESH);
			com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh etaDirectoryRefresh = (com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh)etaDirectoryMsg;
			assertEquals(etaDirectoryRefresh.serviceList().size(), 2);
			
			assertEquals(2, niConsumer2.getMsgCount());
			
			etaMsg = niConsumer2.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REQUEST);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.LOGIN);
			
			etaLoginMsg = convertToLoginMsg(etaMsg);
			
			assertEquals(etaLoginMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType.REQUEST);
			etaLoginRequest = (com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest)etaLoginMsg;
			assertEquals(etaLoginRequest.role(), com.refinitiv.eta.rdm.Login.RoleTypes.PROV);
			
			etaMsg = niConsumer2.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.SOURCE);
			
			etaDirectoryMsg = convertToDirectoryRefreshMsg(etaMsg);
			assertEquals(etaDirectoryMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType.REFRESH);
			etaDirectoryRefresh = (com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh)etaDirectoryMsg;
			assertEquals(etaDirectoryRefresh.serviceList().size(), 2);
			
			// Two status messages of OPEN/SUSPECT for the channels getting established, then a refresh message
			assertEquals(3, loginClient.queueSize());
			
			Msg emaMsg = loginClient.popMessage();
			assertEquals(DataTypes.STATUS_MSG, emaMsg.dataType());
			
			StatusMsg emaStatus = (StatusMsg)emaMsg;
			assertEquals(StreamState.OPEN, emaStatus.state().streamState());
			assertEquals(DataState.SUSPECT, emaStatus.state().dataState());
			
			emaMsg = loginClient.popMessage();
			assertEquals(DataTypes.STATUS_MSG, emaMsg.dataType());
			
			emaStatus = (StatusMsg)emaMsg;
			assertEquals(StreamState.OPEN, emaStatus.state().streamState());
			assertEquals(DataState.SUSPECT, emaStatus.state().dataState());
			
			emaMsg = loginClient.popMessage();
			assertEquals(DataTypes.REFRESH_MSG, emaMsg.dataType());
			
			RefreshMsg emaRefresh = (RefreshMsg)emaMsg;
			assertEquals(StreamState.OPEN, emaRefresh.state().streamState());
			assertEquals(DataState.OK, emaRefresh.state().dataState());
			assertEquals(DomainTypes.LOGIN, emaRefresh.domainType());
            assertTrue(emaRefresh.solicited());
            assertTrue(emaRefresh.complete());
            assertTrue(emaRefresh.hasMsgKey());
            assertEquals(DataType.DataTypes.NO_DATA, emaRefresh.payload().dataType());
            assertEquals(DataType.DataTypes.ELEMENT_LIST, emaRefresh.attrib().dataType());

            ElementList elementList = emaRefresh.attrib().elementList();
            for(ElementEntry element : elementList)
            {
            	System.out.println("element.name: " + element.name());
                switch(element.name())
                {
                    case EmaRdm.ENAME_SUPPORT_PROVIDER_DICTIONARY_DOWNLOAD:
                    {
                        assertEquals(1, element.uintValue());
                        break;
                    }
                    case EmaRdm.ENAME_APP_ID:
                    {
                        assertEquals("100", element.ascii().toString());
                        break;
                    }
                    case EmaRdm.ENAME_APP_NAME:
                    {
                        assertEquals("NiConsumer", element.ascii().toString());
                        break;
                    }
                    default:
                    {
                    	assertFalse(true);
                    }
                }
            }
			
			
			FieldList fieldList = EmaFactory.createFieldList();
			fieldList.add( EmaFactory.createFieldEntry().real(22, 3990, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(25, 3994, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(30, 9,  OmmReal.MagnitudeType.EXPONENT_0));
			fieldList.add( EmaFactory.createFieldEntry().real(31, 19, OmmReal.MagnitudeType.EXPONENT_0));
			
			
			List<ChannelInformation> chnlInfo = new ArrayList<ChannelInformation>();

			// Check to see if all the channels are connected
			provider.sessionChannelInfo(chnlInfo);
			
			assertEquals(2, chnlInfo.size());
			
			assertEquals(chnlInfo.get(0).channelName(), "Channel_1");
			assertEquals(chnlInfo.get(1).channelName(), "Channel_2");

			provider.submit( EmaFactory.createRefreshMsg().serviceName("NI_PUB_1").name("IBM.N")
					.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
					.payload(fieldList).complete(true), itemHandle);
			
			Thread.sleep(500);
			
			assertEquals(1, niConsumer1.getMsgCount());
			
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			assertEquals(1, niConsumer2.getMsgCount());
			
			etaMsg = niConsumer2.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			provider.submit( EmaFactory.createStatusMsg().serviceName("NI_PUB_1").name("IBM.N")
					.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
					, itemHandle);
			
			Thread.sleep(500);
			
			assertEquals(1, niConsumer1.getMsgCount());
			
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.STATUS);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			assertEquals(1, niConsumer2.getMsgCount());
			
			etaMsg = niConsumer2.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.STATUS);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			
			provider.submit( EmaFactory.createGenericMsg().name("IBM.N").payload(fieldList).complete(true), itemHandle);
			
			Thread.sleep(500);
			
			assertEquals(1, niConsumer1.getMsgCount());
			
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.GENERIC);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			assertEquals(1, niConsumer2.getMsgCount());
			
			etaMsg = niConsumer2.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.GENERIC);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			provider.submit( EmaFactory.createUpdateMsg().name("IBM.N").payload(fieldList), itemHandle);
			
			Thread.sleep(500);
			
			assertEquals(1, niConsumer1.getMsgCount());
			
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.UPDATE);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			assertEquals(1, niConsumer2.getMsgCount());
			
			etaMsg = niConsumer2.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.UPDATE);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");			
		} catch(Exception e)
		{
			e.printStackTrace();
			assertTrue( false);
		}
		
	}
	
	@Test
	public void NiProviderTwoConnectionSessionSubmitDuringFullReconnectTest()
	{
		TestUtilities.printTestHead("NiProviderTwoConnectionSessionTest","");

		NiConsumerHelperConfig niConsConfig1 = new NiConsumerHelperConfig();
		NiConsumerHelperConfig niConsConfig2 = new NiConsumerHelperConfig();
		NiProviderTestClient loginClient = new NiProviderTestClient();

		long itemHandle = 10;
		
		niConsConfig1.port = "19001";
		niConsConfig2.port = "19002";

		try
		{
			niProvConfig = EmaFactory.createOmmNiProviderConfig(emaConfigFileLocation);
			niProvConfig.providerName("Provider_Session_1");
			niConsumer1 = new NiConsumerHelper(niConsConfig1);
			niConsumer2 = new NiConsumerHelper(niConsConfig2);
			
			provider = EmaFactory.createOmmProvider(niProvConfig, loginClient);
			assertNotNull(provider);
			
			// sleep to make sure that the consumer gets everything
			Thread.sleep(100);
			
			// sleep to make sure that the consumer gets everything
			Thread.sleep(100);
			
			assertEquals(2, niConsumer1.getMsgCount());
			
			com.refinitiv.eta.codec.Msg etaMsg = niConsumer1.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REQUEST);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.LOGIN);
			
			com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsg etaLoginMsg = convertToLoginMsg(etaMsg);
			
			assertEquals(etaLoginMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType.REQUEST);
			com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest etaLoginRequest = (com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest)etaLoginMsg;
			assertEquals(etaLoginRequest.role(), com.refinitiv.eta.rdm.Login.RoleTypes.PROV);
			
			etaMsg = niConsumer1.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.SOURCE);
			
			com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsg etaDirectoryMsg = convertToDirectoryRefreshMsg(etaMsg);
			assertEquals(etaDirectoryMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType.REFRESH);
			com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh etaDirectoryRefresh = (com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh)etaDirectoryMsg;
			assertEquals(etaDirectoryRefresh.serviceList().size(), 2);
			
			assertEquals(2, niConsumer2.getMsgCount());
			
			etaMsg = niConsumer2.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REQUEST);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.LOGIN);
			
			etaLoginMsg = convertToLoginMsg(etaMsg);
			
			assertEquals(etaLoginMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType.REQUEST);
			etaLoginRequest = (com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest)etaLoginMsg;
			assertEquals(etaLoginRequest.role(), com.refinitiv.eta.rdm.Login.RoleTypes.PROV);
			
			etaMsg = niConsumer2.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.SOURCE);
			
			etaDirectoryMsg = convertToDirectoryRefreshMsg(etaMsg);
			assertEquals(etaDirectoryMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType.REFRESH);
			etaDirectoryRefresh = (com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh)etaDirectoryMsg;
			assertEquals(etaDirectoryRefresh.serviceList().size(), 2);
			
			// Two status messages of OPEN/SUSPECT for the channels getting established, then a refresh message
			assertEquals(3, loginClient.queueSize());
			
			Msg emaMsg = loginClient.popMessage();
			assertEquals(DataTypes.STATUS_MSG, emaMsg.dataType());
			
			StatusMsg emaStatus = (StatusMsg)emaMsg;
			assertEquals(StreamState.OPEN, emaStatus.state().streamState());
			assertEquals(DataState.SUSPECT, emaStatus.state().dataState());
			
			emaMsg = loginClient.popMessage();
			assertEquals(DataTypes.STATUS_MSG, emaMsg.dataType());
			
			emaStatus = (StatusMsg)emaMsg;
			assertEquals(StreamState.OPEN, emaStatus.state().streamState());
			assertEquals(DataState.SUSPECT, emaStatus.state().dataState());
			
			emaMsg = loginClient.popMessage();
			assertEquals(DataTypes.REFRESH_MSG, emaMsg.dataType());
			
			RefreshMsg emaRefresh = (RefreshMsg)emaMsg;
			assertEquals(StreamState.OPEN, emaRefresh.state().streamState());
			assertEquals(DataState.OK, emaRefresh.state().dataState());
			assertEquals(DomainTypes.LOGIN, emaRefresh.domainType());
            assertTrue(emaRefresh.solicited());
            assertTrue(emaRefresh.complete());
            assertTrue(emaRefresh.hasMsgKey());
            assertEquals(DataType.DataTypes.NO_DATA, emaRefresh.payload().dataType());
            assertEquals(DataType.DataTypes.ELEMENT_LIST, emaRefresh.attrib().dataType());

            ElementList elementList = emaRefresh.attrib().elementList();
            for(ElementEntry element : elementList)
            {
            	System.out.println("element.name: " + element.name());
                switch(element.name())
                {
                    case EmaRdm.ENAME_SUPPORT_PROVIDER_DICTIONARY_DOWNLOAD:
                    {
                        assertEquals(1, element.uintValue());
                        break;
                    }
                    case EmaRdm.ENAME_APP_ID:
                    {
                        assertEquals("100", element.ascii().toString());
                        break;
                    }
                    case EmaRdm.ENAME_APP_NAME:
                    {
                        assertEquals("NiConsumer", element.ascii().toString());
                        break;
                    }
                    default:
                    {
                    	assertFalse(true);
                    }
                }
            }
			
			
			FieldList fieldList = EmaFactory.createFieldList();
			fieldList.add( EmaFactory.createFieldEntry().real(22, 3990, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(25, 3994, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(30, 9,  OmmReal.MagnitudeType.EXPONENT_0));
			fieldList.add( EmaFactory.createFieldEntry().real(31, 19, OmmReal.MagnitudeType.EXPONENT_0));
			
			
			List<ChannelInformation> chnlInfo = new ArrayList<ChannelInformation>();

			// Check to see if all the channels are connected
			provider.sessionChannelInfo(chnlInfo);
			
			assertEquals(2, chnlInfo.size());
			
			assertEquals(chnlInfo.get(0).channelName(), "Channel_1");
			assertEquals(chnlInfo.get(1).channelName(), "Channel_2");
			
			// Close both channels
			niConsumer1.closeChannel();
			niConsumer2.closeChannel();
			
			Thread.sleep(500);
			
			// One status message of OPEN/OK, then a status message of OPEN/SUSPECT 
			assertEquals(2, loginClient.queueSize());
			
			emaMsg = loginClient.popMessage();
			assertEquals(DataTypes.STATUS_MSG, emaMsg.dataType());
			
			emaStatus = (StatusMsg)emaMsg;
			assertEquals(StreamState.OPEN, emaStatus.state().streamState());
			assertEquals(DataState.OK, emaStatus.state().dataState());
			
			emaMsg = loginClient.popMessage();
			assertEquals(DataTypes.STATUS_MSG, emaMsg.dataType());
			
			emaStatus = (StatusMsg)emaMsg;
			assertEquals(StreamState.OPEN, emaStatus.state().streamState());
			assertEquals(DataState.SUSPECT, emaStatus.state().dataState());
			
			try
			{
				provider.submit( EmaFactory.createRefreshMsg().serviceName("NI_PUB_1").name("IBM.N")
						.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
						.payload(fieldList).complete(true), itemHandle);
				assertTrue(false);
			}catch(OmmInvalidUsageException ommExcept)
			{
				assertEquals(OmmInvalidUsageException.ErrorCode.NO_ACTIVE_CHANNEL, ommExcept.errorCode());
			}
			
			try
			{
				provider.submit( EmaFactory.createStatusMsg().serviceName("NI_PUB_1").name("IBM.N")
						.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
						, itemHandle);
				assertTrue(false);
			}catch(OmmInvalidUsageException ommExcept)
			{
				assertEquals(OmmInvalidUsageException.ErrorCode.NO_ACTIVE_CHANNEL, ommExcept.errorCode());
			}			
			
		
			try
			{
				provider.submit( EmaFactory.createGenericMsg().name("IBM.N").payload(fieldList).complete(true), itemHandle);

				assertTrue(false);
			}catch(OmmInvalidUsageException ommExcept)
			{
				assertEquals(OmmInvalidUsageException.ErrorCode.NO_ACTIVE_CHANNEL, ommExcept.errorCode());
			}	
			
			try
			{
				provider.submit( EmaFactory.createUpdateMsg().name("IBM.N").payload(fieldList), itemHandle);

				assertTrue(false);
			}catch(OmmInvalidUsageException ommExcept)
			{
				assertEquals(OmmInvalidUsageException.ErrorCode.NO_ACTIVE_CHANNEL, ommExcept.errorCode());
			}				
		} catch(Exception e)
		{
			e.printStackTrace();
			assertTrue( false);
		}
		
	}
	
	@Test
	public void NiProviderTwoConnectionSessionSubmitBadDirectoryTest()
	{
		TestUtilities.printTestHead("NiProviderSingleConnectionTest","");

		NiConsumerHelperConfig niConsConfig1 = new NiConsumerHelperConfig();
		NiConsumerHelperConfig niConsConfig2 = new NiConsumerHelperConfig();

		long itemHandle = 10;
		
		niConsConfig1.port = "19001";
		niConsConfig2.port = "19002";

		try
		{
			niProvConfig = EmaFactory.createOmmNiProviderConfig(emaConfigFileLocation);
			niProvConfig.providerName("Provider_Session_1");
			niConsumer1 = new NiConsumerHelper(niConsConfig1);
			niConsumer2 = new NiConsumerHelper(niConsConfig2);
			
			provider = EmaFactory.createOmmProvider(niProvConfig);
			assertNotNull(provider);
			
			// sleep to make sure that the consumer gets everything
			Thread.sleep(100);
			
			assertEquals(2, niConsumer1.getMsgCount());
			
			com.refinitiv.eta.codec.Msg etaMsg = niConsumer1.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REQUEST);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.LOGIN);
			
			com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsg etaLoginMsg = convertToLoginMsg(etaMsg);
			
			assertEquals(etaLoginMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType.REQUEST);
			com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest etaLoginRequest = (com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest)etaLoginMsg;
			assertEquals(etaLoginRequest.role(), com.refinitiv.eta.rdm.Login.RoleTypes.PROV);
			
			etaMsg = niConsumer1.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.SOURCE);
			
			com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsg etaDirectoryMsg = convertToDirectoryRefreshMsg(etaMsg);
			assertEquals(etaDirectoryMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType.REFRESH);
			com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh etaDirectoryRefresh = (com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh)etaDirectoryMsg;
			assertEquals(etaDirectoryRefresh.serviceList().size(), 2);
			
			assertEquals(2, niConsumer2.getMsgCount());
			
			etaMsg = niConsumer2.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REQUEST);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.LOGIN);
			
			etaLoginMsg = convertToLoginMsg(etaMsg);
			
			assertEquals(etaLoginMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType.REQUEST);
			etaLoginRequest = (com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest)etaLoginMsg;
			assertEquals(etaLoginRequest.role(), com.refinitiv.eta.rdm.Login.RoleTypes.PROV);
			
			etaMsg = niConsumer2.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.SOURCE);
			
			etaDirectoryMsg = convertToDirectoryRefreshMsg(etaMsg);
			assertEquals(etaDirectoryMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType.REFRESH);
			etaDirectoryRefresh = (com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh)etaDirectoryMsg;
			assertEquals(etaDirectoryRefresh.serviceList().size(), 2);
			
			FieldList fieldList = EmaFactory.createFieldList();
			fieldList.add( EmaFactory.createFieldEntry().real(22, 3990, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(25, 3994, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(30, 9,  OmmReal.MagnitudeType.EXPONENT_0));
			fieldList.add( EmaFactory.createFieldEntry().real(31, 19, OmmReal.MagnitudeType.EXPONENT_0));
			
			try
			{
				provider.submit( EmaFactory.createRefreshMsg().serviceName("BAD_SERVICE").name("IBM.N")
						.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
						.payload(fieldList).complete(true), itemHandle);
				assertTrue(false);
			} catch (OmmInvalidUsageException except)
			{
				System.out.println("Exception thrown on bad serviceName for refresh msg.");
			}
			
			try
			{
				provider.submit( EmaFactory.createStatusMsg().serviceName("BAD_SERVICE").name("IBM.N")
						.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
						.payload(fieldList), itemHandle);
				assertTrue(false);
			} catch (OmmInvalidUsageException except)
			{
				System.out.println("Exception thrown on bad serviceName for status msg.");
			}
			
			try
			{
				provider.submit( EmaFactory.createUpdateMsg().serviceName("BAD_SERVICE").name("IBM.N")
						.payload(fieldList), itemHandle);
				assertTrue(false);
			} catch (OmmInvalidUsageException except)
			{
				System.out.println("Exception thrown on bad serviceName for update msg.");
			}
			
			assertEquals(0, niConsumer1.getMsgCount());
			assertEquals(0, niConsumer2.getMsgCount());

			
		} catch(Exception e)
		{
			e.printStackTrace();
			assertTrue( false);
		}
		
	}
	
	@Test
	public void NiProviderTwoConnectionSessionSubmitBadHandleTest()
	{
		TestUtilities.printTestHead("NiProviderSingleConnectionTest","");

		NiConsumerHelperConfig niConsConfig1 = new NiConsumerHelperConfig();
		NiConsumerHelperConfig niConsConfig2 = new NiConsumerHelperConfig();

		long itemHandle = 10;
		long badHandle = 11;
		
		niConsConfig1.port = "19001";
		niConsConfig2.port = "19002";

		try
		{
			niProvConfig = EmaFactory.createOmmNiProviderConfig(emaConfigFileLocation);
			niProvConfig.providerName("Provider_Session_1");
			niConsumer1 = new NiConsumerHelper(niConsConfig1);
			niConsumer2 = new NiConsumerHelper(niConsConfig2);
			
			provider = EmaFactory.createOmmProvider(niProvConfig);
			assertNotNull(provider);
			
			// sleep to make sure that the consumer gets everything
			Thread.sleep(100);
			
			assertEquals(2, niConsumer1.getMsgCount());
			
			com.refinitiv.eta.codec.Msg etaMsg = niConsumer1.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REQUEST);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.LOGIN);
			
			com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsg etaLoginMsg = convertToLoginMsg(etaMsg);
			
			assertEquals(etaLoginMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType.REQUEST);
			com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest etaLoginRequest = (com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest)etaLoginMsg;
			assertEquals(etaLoginRequest.role(), com.refinitiv.eta.rdm.Login.RoleTypes.PROV);
			
			etaMsg = niConsumer1.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.SOURCE);
			
			com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsg etaDirectoryMsg = convertToDirectoryRefreshMsg(etaMsg);
			assertEquals(etaDirectoryMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType.REFRESH);
			com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh etaDirectoryRefresh = (com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh)etaDirectoryMsg;
			assertEquals(etaDirectoryRefresh.serviceList().size(), 2);
			
			assertEquals(2, niConsumer2.getMsgCount());
			
			etaMsg = niConsumer2.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REQUEST);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.LOGIN);
			
			etaLoginMsg = convertToLoginMsg(etaMsg);
			
			assertEquals(etaLoginMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType.REQUEST);
			etaLoginRequest = (com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest)etaLoginMsg;
			assertEquals(etaLoginRequest.role(), com.refinitiv.eta.rdm.Login.RoleTypes.PROV);
			
			etaMsg = niConsumer2.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.SOURCE);
			
			etaDirectoryMsg = convertToDirectoryRefreshMsg(etaMsg);
			assertEquals(etaDirectoryMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType.REFRESH);
			etaDirectoryRefresh = (com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh)etaDirectoryMsg;
			assertEquals(etaDirectoryRefresh.serviceList().size(), 2);
			
			FieldList fieldList = EmaFactory.createFieldList();
			fieldList.add( EmaFactory.createFieldEntry().real(22, 3990, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(25, 3994, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(30, 9,  OmmReal.MagnitudeType.EXPONENT_0));
			fieldList.add( EmaFactory.createFieldEntry().real(31, 19, OmmReal.MagnitudeType.EXPONENT_0));
			
			try
			{
				provider.submit( EmaFactory.createGenericMsg().name("IBM.N")
						.payload(fieldList).complete(true), badHandle);
				assertTrue(false);
			} catch (OmmInvalidHandleException except)
			{
				System.out.println("Exception thrown on bad handle on generic msg.");
			}
			
			// Add in the item.
			provider.submit( EmaFactory.createRefreshMsg().serviceName("NI_PUB_1").name("IBM.N")
					.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
					.payload(fieldList).complete(true), itemHandle);
			
			Thread.sleep(500);
			
			assertEquals(1, niConsumer1.getMsgCount());
			
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			assertEquals(1, niConsumer2.getMsgCount());
			
			etaMsg = niConsumer2.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			try
			{
				provider.submit( EmaFactory.createGenericMsg().name("IBM.N")
						.payload(fieldList).complete(true), badHandle);
				assertTrue(false);
			} catch (OmmInvalidHandleException except)
			{
				System.out.println("Exception thrown on bad handle on generic msg after adding a handle.");
			}
			
			assertEquals(0, niConsumer1.getMsgCount());
			assertEquals(0, niConsumer2.getMsgCount());
			
		} catch(Exception e)
		{
			e.printStackTrace();
			assertTrue( false);
		}
		
	}
	

	@Test
	public void NiProviderTwoConnectionSessionSecondLoginRejectTest()
	{
		TestUtilities.printTestHead("NiProviderTwoConnectionSessionFirstLoginRejectTest","");
	
		NiConsumerHelperConfig niConsConfig1 = new NiConsumerHelperConfig();
		NiConsumerHelperConfig niConsConfig2 = new NiConsumerHelperConfig();
		
		NiProviderTestClient loginClient = new NiProviderTestClient();
	
		long itemHandle = 10;
		
		niConsConfig1.port = "19001";
		niConsConfig2.port = "19002";
		niConsConfig2.acceptLogin = false;
	
		try
		{
			niProvConfig = EmaFactory.createOmmNiProviderConfig(emaConfigFileLocation);
			niProvConfig.providerName("Provider_Session_1");
			niConsumer1 = new NiConsumerHelper(niConsConfig1);
			niConsumer2 = new NiConsumerHelper(niConsConfig2);
			
			provider = EmaFactory.createOmmProvider(niProvConfig, loginClient);
			assertNotNull(provider);
			
			// sleep to make sure that the consumer gets everything
			Thread.sleep(100);
			
			assertEquals(2, niConsumer1.getMsgCount());
			
			com.refinitiv.eta.codec.Msg etaMsg = niConsumer1.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REQUEST);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.LOGIN);
			
			com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsg etaLoginMsg = convertToLoginMsg(etaMsg);
			
			assertEquals(etaLoginMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType.REQUEST);
			com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest etaLoginRequest = (com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest)etaLoginMsg;
			assertEquals(etaLoginRequest.role(), com.refinitiv.eta.rdm.Login.RoleTypes.PROV);
			
			etaMsg = niConsumer1.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.SOURCE);
			
			com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsg etaDirectoryMsg = convertToDirectoryRefreshMsg(etaMsg);
			assertEquals(etaDirectoryMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType.REFRESH);
			com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh etaDirectoryRefresh = (com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh)etaDirectoryMsg;
			assertEquals(etaDirectoryRefresh.serviceList().size(), 2);
			
			assertEquals(1, niConsumer2.getMsgCount());
			
			etaMsg = niConsumer2.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REQUEST);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.LOGIN);
			
			etaLoginMsg = convertToLoginMsg(etaMsg);
			
			assertEquals(etaLoginMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType.REQUEST);
			etaLoginRequest = (com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest)etaLoginMsg;
			assertEquals(etaLoginRequest.role(), com.refinitiv.eta.rdm.Login.RoleTypes.PROV);
		
			// Three status messages of OPEN/SUSPECT for the channels getting established, then a refresh message
			assertEquals(4, loginClient.queueSize());
			
			Msg emaMsg = loginClient.popMessage();
			assertEquals(DataTypes.STATUS_MSG, emaMsg.dataType());

			StatusMsg emaStatus = (StatusMsg)emaMsg;
			assertEquals(StreamState.OPEN, emaStatus.state().streamState());
			assertEquals(DataState.SUSPECT, emaStatus.state().dataState());
			
			emaMsg = loginClient.popMessage();
			assertEquals(DataTypes.STATUS_MSG, emaMsg.dataType());

			emaStatus = (StatusMsg)emaMsg;
			assertEquals(StreamState.OPEN, emaStatus.state().streamState());
			assertEquals(DataState.SUSPECT, emaStatus.state().dataState());
			
			emaMsg = loginClient.popMessage();
			assertEquals(DataTypes.STATUS_MSG, emaMsg.dataType());
			
			emaStatus = (StatusMsg)emaMsg;
			assertEquals(StreamState.OPEN, emaStatus.state().streamState());
			assertEquals(DataState.SUSPECT, emaStatus.state().dataState());
			
			emaMsg = loginClient.popMessage();
			assertEquals(DataTypes.REFRESH_MSG, emaMsg.dataType());
			
			RefreshMsg emaRefresh = (RefreshMsg)emaMsg;
			assertEquals(StreamState.OPEN, emaRefresh.state().streamState());
			assertEquals(DataState.OK, emaRefresh.state().dataState());
			assertEquals(DomainTypes.LOGIN, emaRefresh.domainType());
            assertTrue(emaRefresh.solicited());
            assertTrue(emaRefresh.complete());
            assertTrue(emaRefresh.hasMsgKey());
            assertEquals(DataType.DataTypes.NO_DATA, emaRefresh.payload().dataType());
            assertEquals(DataType.DataTypes.ELEMENT_LIST, emaRefresh.attrib().dataType());

            ElementList elementList = emaRefresh.attrib().elementList();
            for(ElementEntry element : elementList)
            {
            	System.out.println("element.name: " + element.name());
                switch(element.name())
                {
                    case EmaRdm.ENAME_SUPPORT_PROVIDER_DICTIONARY_DOWNLOAD:
                    {
                        assertEquals(1, element.uintValue());
                        break;
                    }
                    case EmaRdm.ENAME_APP_ID:
                    {
                        assertEquals("100", element.ascii().toString());
                        break;
                    }
                    case EmaRdm.ENAME_APP_NAME:
                    {
                        assertEquals("NiConsumer", element.ascii().toString());
                        break;
                    }
                    default:
                    {
                    	assertFalse(true);
                    }
                }
            }
			
			FieldList fieldList = EmaFactory.createFieldList();
			fieldList.add( EmaFactory.createFieldEntry().real(22, 3990, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(25, 3994, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(30, 9,  OmmReal.MagnitudeType.EXPONENT_0));
			fieldList.add( EmaFactory.createFieldEntry().real(31, 19, OmmReal.MagnitudeType.EXPONENT_0));
			
			List<ChannelInformation> chnlInfo = new ArrayList<ChannelInformation>();
	
			// Check to see if all the channels are connected
			provider.sessionChannelInfo(chnlInfo);
			
			assertEquals(1, chnlInfo.size());
			
			assertEquals(chnlInfo.get(0).channelName(), "Channel_1");
	
			provider.submit( EmaFactory.createRefreshMsg().serviceName("NI_PUB_1").name("IBM.N")
					.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
					.payload(fieldList).complete(true), itemHandle);
			
			Thread.sleep(500);
			
			assertEquals(1, niConsumer1.getMsgCount());
					
		} catch(Exception e)
		{
			e.printStackTrace();
			assertTrue( false);
		}
		
	}
	
	@Test
	public void NiProviderTwoConnectionSessionFirstLoginRejectTest()
	{
		TestUtilities.printTestHead("NiProviderTwoConnectionOneLoginRejectSessionTest","");
	
		NiConsumerHelperConfig niConsConfig1 = new NiConsumerHelperConfig();
		NiConsumerHelperConfig niConsConfig2 = new NiConsumerHelperConfig();
		
		NiProviderTestClient loginClient = new NiProviderTestClient();
	
		long itemHandle = 10;
		
		niConsConfig1.port = "19001";
		niConsConfig2.port = "19002";
		niConsConfig1.acceptLogin = false;
	
		try
		{
			niProvConfig = EmaFactory.createOmmNiProviderConfig(emaConfigFileLocation);
			niProvConfig.providerName("Provider_Session_1");
			niConsumer1 = new NiConsumerHelper(niConsConfig1);
			niConsumer2 = new NiConsumerHelper(niConsConfig2);
			
			provider = EmaFactory.createOmmProvider(niProvConfig, loginClient);
			assertNotNull(provider);
			
			// sleep to make sure that the consumer gets everything
			Thread.sleep(100);
			
			assertEquals(1, niConsumer1.getMsgCount());
			
			com.refinitiv.eta.codec.Msg etaMsg = niConsumer1.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REQUEST);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.LOGIN);
			
			com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsg etaLoginMsg = convertToLoginMsg(etaMsg);
			
			assertEquals(etaLoginMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType.REQUEST);
			com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest etaLoginRequest = (com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest)etaLoginMsg;
			assertEquals(etaLoginRequest.role(), com.refinitiv.eta.rdm.Login.RoleTypes.PROV);
			
			assertEquals(2, niConsumer2.getMsgCount());
			
			etaMsg = niConsumer2.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REQUEST);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.LOGIN);
			
			etaLoginMsg = convertToLoginMsg(etaMsg);
			
			assertEquals(etaLoginMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType.REQUEST);
			etaLoginRequest = (com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest)etaLoginMsg;
			assertEquals(etaLoginRequest.role(), com.refinitiv.eta.rdm.Login.RoleTypes.PROV);
			
			etaMsg = niConsumer2.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.SOURCE);
			
			com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsg etaDirectoryMsg = convertToDirectoryRefreshMsg(etaMsg);
			assertEquals(etaDirectoryMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType.REFRESH);
			com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh etaDirectoryRefresh = (com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh)etaDirectoryMsg;
			assertEquals(etaDirectoryRefresh.serviceList().size(), 2);
		
			// Three status messages of OPEN/SUSPECT for the channels getting established, then a refresh message
			assertEquals(4, loginClient.queueSize());
			
			Msg emaMsg = loginClient.popMessage();
			assertEquals(DataTypes.STATUS_MSG, emaMsg.dataType());

			StatusMsg emaStatus = (StatusMsg)emaMsg;
			assertEquals(StreamState.OPEN, emaStatus.state().streamState());
			assertEquals(DataState.SUSPECT, emaStatus.state().dataState());
			
			emaMsg = loginClient.popMessage();
			assertEquals(DataTypes.STATUS_MSG, emaMsg.dataType());

			emaStatus = (StatusMsg)emaMsg;
			assertEquals(StreamState.OPEN, emaStatus.state().streamState());
			assertEquals(DataState.SUSPECT, emaStatus.state().dataState());
			
			emaMsg = loginClient.popMessage();
			assertEquals(DataTypes.STATUS_MSG, emaMsg.dataType());
			
			emaStatus = (StatusMsg)emaMsg;
			assertEquals(StreamState.OPEN, emaStatus.state().streamState());
			assertEquals(DataState.SUSPECT, emaStatus.state().dataState());
			
			emaMsg = loginClient.popMessage();
			assertEquals(DataTypes.REFRESH_MSG, emaMsg.dataType());
			
			RefreshMsg emaRefresh = (RefreshMsg)emaMsg;
			assertEquals(StreamState.OPEN, emaRefresh.state().streamState());
			assertEquals(DataState.OK, emaRefresh.state().dataState());
			assertEquals(DomainTypes.LOGIN, emaRefresh.domainType());
            assertTrue(emaRefresh.solicited());
            assertTrue(emaRefresh.complete());
            assertTrue(emaRefresh.hasMsgKey());
            assertEquals(DataType.DataTypes.NO_DATA, emaRefresh.payload().dataType());
            assertEquals(DataType.DataTypes.ELEMENT_LIST, emaRefresh.attrib().dataType());

            ElementList elementList = emaRefresh.attrib().elementList();
            for(ElementEntry element : elementList)
            {
            	System.out.println("element.name: " + element.name());
                switch(element.name())
                {
                    case EmaRdm.ENAME_SUPPORT_PROVIDER_DICTIONARY_DOWNLOAD:
                    {
                        assertEquals(1, element.uintValue());
                        break;
                    }
                    case EmaRdm.ENAME_APP_ID:
                    {
                        assertEquals("100", element.ascii().toString());
                        break;
                    }
                    case EmaRdm.ENAME_APP_NAME:
                    {
                        assertEquals("NiConsumer", element.ascii().toString());
                        break;
                    }
                    default:
                    {
                    	assertFalse(true);
                    }
                }
            }
			
			FieldList fieldList = EmaFactory.createFieldList();
			fieldList.add( EmaFactory.createFieldEntry().real(22, 3990, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(25, 3994, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(30, 9,  OmmReal.MagnitudeType.EXPONENT_0));
			fieldList.add( EmaFactory.createFieldEntry().real(31, 19, OmmReal.MagnitudeType.EXPONENT_0));
			
			List<ChannelInformation> chnlInfo = new ArrayList<ChannelInformation>();
	
			// Check to see if all the channels are connected
			provider.sessionChannelInfo(chnlInfo);
			
			assertEquals(1, chnlInfo.size());
			
			assertEquals(chnlInfo.get(0).channelName(), "Channel_2");
	
			provider.submit( EmaFactory.createRefreshMsg().serviceName("NI_PUB_1").name("IBM.N")
					.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
					.payload(fieldList).complete(true), itemHandle);
			
			Thread.sleep(500);
			
			assertEquals(1, niConsumer2.getMsgCount());
					
		} catch(Exception e)
		{
			e.printStackTrace();
			assertTrue( false);
		}
		
	}


	@Test
	public void NiProviderTwoConnectionBothLoginRejectSessionTest()
	{
		TestUtilities.printTestHead("NiProviderTwoConnectionBothLoginRejectSessionTest","");
	
		NiConsumerHelperConfig niConsConfig1 = new NiConsumerHelperConfig();
		NiConsumerHelperConfig niConsConfig2 = new NiConsumerHelperConfig();
		
		NiProviderTestClient loginClient = new NiProviderTestClient();
	
		niConsConfig1.port = "19001";
		niConsConfig1.acceptLogin = false;
		niConsConfig2.port = "19002";
		niConsConfig2.acceptLogin = false;
	
		try
		{
			niProvConfig = EmaFactory.createOmmNiProviderConfig(emaConfigFileLocation);
			niProvConfig.providerName("Provider_Session_1");
			niConsumer1 = new NiConsumerHelper(niConsConfig1);
			niConsumer2 = new NiConsumerHelper(niConsConfig2);
			
			provider = EmaFactory.createOmmProvider(niProvConfig, loginClient);
			
			assertTrue("NiProv did not error out with both logins denied", false);
			
		} 
		catch(OmmInvalidUsageException excep)
		{
			System.out.println("All NiProv logins denied");
		}	
		catch(Exception e)
		{
			e.printStackTrace();
			assertTrue( false);
		}
		
		assertEquals(1, niConsumer1.getMsgCount());
		
		com.refinitiv.eta.codec.Msg etaMsg = niConsumer1.getMessage();
		
		assertNotNull(etaMsg);
		assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REQUEST);
		assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.LOGIN);
		
		com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsg etaLoginMsg = convertToLoginMsg(etaMsg);
		
		assertEquals(etaLoginMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType.REQUEST);
		com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest etaLoginRequest = (com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest)etaLoginMsg;
		assertEquals(etaLoginRequest.role(), com.refinitiv.eta.rdm.Login.RoleTypes.PROV);
		
		assertEquals(1, niConsumer2.getMsgCount());
		
		etaMsg = niConsumer2.getMessage();
		
		assertNotNull(etaMsg);
		assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REQUEST);
		assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.LOGIN);

		try
		{
			// Three status messages of OPEN/SUSPECT for the channels getting established, then a status message of CLOSED/SUSPECT
			assertEquals(4, loginClient.queueSize());
			
			Msg emaMsg = loginClient.popMessage();
			assertEquals(DataTypes.STATUS_MSG, emaMsg.dataType());
	
			StatusMsg emaStatus = (StatusMsg)emaMsg;
			assertEquals(StreamState.OPEN, emaStatus.state().streamState());
			assertEquals(DataState.SUSPECT, emaStatus.state().dataState());
			
			emaMsg = loginClient.popMessage();
			assertEquals(DataTypes.STATUS_MSG, emaMsg.dataType());
	
			emaStatus = (StatusMsg)emaMsg;
			assertEquals(StreamState.OPEN, emaStatus.state().streamState());
			assertEquals(DataState.SUSPECT, emaStatus.state().dataState());
			
			emaMsg = loginClient.popMessage();
			assertEquals(DataTypes.STATUS_MSG, emaMsg.dataType());
			
			emaStatus = (StatusMsg)emaMsg;
			assertEquals(StreamState.OPEN, emaStatus.state().streamState());
			assertEquals(DataState.SUSPECT, emaStatus.state().dataState());
			
			
			emaMsg = loginClient.popMessage();
			assertEquals(DataTypes.STATUS_MSG, emaMsg.dataType());
			
			emaStatus = (StatusMsg)emaMsg;
			assertEquals(StreamState.CLOSED, emaStatus.state().streamState());
			assertEquals(DataState.SUSPECT, emaStatus.state().dataState());
		}
		catch(Exception e)
		{
			e.printStackTrace();
			assertTrue( false);
		}
 
		
	}
	
	@Test
	public void NiProviderTwoConnectionSessionPackedMsgTest()
	{
		TestUtilities.printTestHead("NiProviderTwoConnectionPackedMsgSessionTest","");

		NiConsumerHelperConfig niConsConfig1 = new NiConsumerHelperConfig();
		NiConsumerHelperConfig niConsConfig2 = new NiConsumerHelperConfig();

		long itemHandle = 10;
		
		niConsConfig1.port = "19001";
		niConsConfig2.port = "19002";

		try
		{
			niProvConfig = EmaFactory.createOmmNiProviderConfig(emaConfigFileLocation);
			niProvConfig.providerName("Provider_Session_1");
			niConsumer1 = new NiConsumerHelper(niConsConfig1);
			niConsumer2 = new NiConsumerHelper(niConsConfig2);
			
			provider = EmaFactory.createOmmProvider(niProvConfig);
			assertNotNull(provider);
			
			// sleep to make sure that the consumer gets everything
			Thread.sleep(100);
			
			assertEquals(2, niConsumer1.getMsgCount());
			
			com.refinitiv.eta.codec.Msg etaMsg = niConsumer1.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REQUEST);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.LOGIN);
			
			com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsg etaLoginMsg = convertToLoginMsg(etaMsg);
			
			assertEquals(etaLoginMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType.REQUEST);
			com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest etaLoginRequest = (com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest)etaLoginMsg;
			assertEquals(etaLoginRequest.role(), com.refinitiv.eta.rdm.Login.RoleTypes.PROV);
			
			etaMsg = niConsumer1.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.SOURCE);
			
			com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsg etaDirectoryMsg = convertToDirectoryRefreshMsg(etaMsg);
			assertEquals(etaDirectoryMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType.REFRESH);
			com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh etaDirectoryRefresh = (com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh)etaDirectoryMsg;
			assertEquals(etaDirectoryRefresh.serviceList().size(), 2);
			
			assertEquals(2, niConsumer2.getMsgCount());
			
			etaMsg = niConsumer2.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REQUEST);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.LOGIN);
			
			etaLoginMsg = convertToLoginMsg(etaMsg);
			
			assertEquals(etaLoginMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType.REQUEST);
			etaLoginRequest = (com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest)etaLoginMsg;
			assertEquals(etaLoginRequest.role(), com.refinitiv.eta.rdm.Login.RoleTypes.PROV);
			
			etaMsg = niConsumer2.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.SOURCE);
			
			etaDirectoryMsg = convertToDirectoryRefreshMsg(etaMsg);
			assertEquals(etaDirectoryMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType.REFRESH);
			etaDirectoryRefresh = (com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh)etaDirectoryMsg;
			assertEquals(etaDirectoryRefresh.serviceList().size(), 2);
			
			FieldList fieldList = EmaFactory.createFieldList();
			fieldList.add( EmaFactory.createFieldEntry().real(22, 3990, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(25, 3994, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(30, 9,  OmmReal.MagnitudeType.EXPONENT_0));
			fieldList.add( EmaFactory.createFieldEntry().real(31, 19, OmmReal.MagnitudeType.EXPONENT_0));
			
			
			List<ChannelInformation> chnlInfo = new ArrayList<ChannelInformation>();

			// Check to see if all the channels are connected
			provider.sessionChannelInfo(chnlInfo);
			
			assertEquals(2, chnlInfo.size());
			
			assertEquals(chnlInfo.get(0).channelName(), "Channel_1");
			assertEquals(chnlInfo.get(1).channelName(), "Channel_2");

			PackedMsg messagePack = EmaFactory.createPackedMsg(provider);
			
			messagePack.initBuffer();
			
			messagePack.addMsg(EmaFactory.createRefreshMsg().serviceName("NI_PUB_1").name("IBM.N")
					.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
					.payload(fieldList).complete(true), itemHandle);
			
			messagePack.addMsg(EmaFactory.createStatusMsg().serviceName("NI_PUB_1").name("IBM.N")
					.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
					, itemHandle);
			
			messagePack.addMsg(EmaFactory.createGenericMsg().name("IBM.N").payload(fieldList).complete(true), itemHandle);
			
			messagePack.addMsg( EmaFactory.createUpdateMsg().name("IBM.N").payload(fieldList), itemHandle);
			
			provider.submit(messagePack);
			
			Thread.sleep(500);
			
			assertEquals(4, niConsumer1.getMsgCount());
			
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.STATUS);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.GENERIC);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.UPDATE);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			assertEquals(4, niConsumer2.getMsgCount());
			
			etaMsg = niConsumer2.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			etaMsg = niConsumer2.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.STATUS);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			etaMsg = niConsumer2.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.GENERIC);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");

			etaMsg = niConsumer2.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.UPDATE);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");				
		} catch(Exception e)
		{
			e.printStackTrace();
			assertTrue( false);
		}
		
	}
	
	@Test
	public void NiProviderTwoConnectionSessionPackedMsgSubmitDuringFullReconnectTest()
	{
		TestUtilities.printTestHead("NiProviderTwoConnectionPackedMsgSessionTest","");

		NiConsumerHelperConfig niConsConfig1 = new NiConsumerHelperConfig();
		NiConsumerHelperConfig niConsConfig2 = new NiConsumerHelperConfig();

		long itemHandle = 10;
		
		niConsConfig1.port = "19001";
		niConsConfig2.port = "19002";

		try
		{
			niProvConfig = EmaFactory.createOmmNiProviderConfig(emaConfigFileLocation);
			niProvConfig.providerName("Provider_Session_1");
			niConsumer1 = new NiConsumerHelper(niConsConfig1);
			niConsumer2 = new NiConsumerHelper(niConsConfig2);
			
			provider = EmaFactory.createOmmProvider(niProvConfig);
			assertNotNull(provider);
			
			// sleep to make sure that the consumer gets everything
			Thread.sleep(100);
			
			assertEquals(2, niConsumer1.getMsgCount());
			
			com.refinitiv.eta.codec.Msg etaMsg = niConsumer1.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REQUEST);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.LOGIN);
			
			com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsg etaLoginMsg = convertToLoginMsg(etaMsg);
			
			assertEquals(etaLoginMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType.REQUEST);
			com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest etaLoginRequest = (com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest)etaLoginMsg;
			assertEquals(etaLoginRequest.role(), com.refinitiv.eta.rdm.Login.RoleTypes.PROV);
			
			etaMsg = niConsumer1.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.SOURCE);
			
			com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsg etaDirectoryMsg = convertToDirectoryRefreshMsg(etaMsg);
			assertEquals(etaDirectoryMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType.REFRESH);
			com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh etaDirectoryRefresh = (com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh)etaDirectoryMsg;
			assertEquals(etaDirectoryRefresh.serviceList().size(), 2);
			
			assertEquals(2, niConsumer2.getMsgCount());
			
			etaMsg = niConsumer2.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REQUEST);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.LOGIN);
			
			etaLoginMsg = convertToLoginMsg(etaMsg);
			
			assertEquals(etaLoginMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType.REQUEST);
			etaLoginRequest = (com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest)etaLoginMsg;
			assertEquals(etaLoginRequest.role(), com.refinitiv.eta.rdm.Login.RoleTypes.PROV);
			
			etaMsg = niConsumer2.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.SOURCE);
			
			etaDirectoryMsg = convertToDirectoryRefreshMsg(etaMsg);
			assertEquals(etaDirectoryMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType.REFRESH);
			etaDirectoryRefresh = (com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh)etaDirectoryMsg;
			assertEquals(etaDirectoryRefresh.serviceList().size(), 2);
			
			FieldList fieldList = EmaFactory.createFieldList();
			fieldList.add( EmaFactory.createFieldEntry().real(22, 3990, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(25, 3994, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(30, 9,  OmmReal.MagnitudeType.EXPONENT_0));
			fieldList.add( EmaFactory.createFieldEntry().real(31, 19, OmmReal.MagnitudeType.EXPONENT_0));
			
			
			List<ChannelInformation> chnlInfo = new ArrayList<ChannelInformation>();

			// Check to see if all the channels are connected
			provider.sessionChannelInfo(chnlInfo);
			
			assertEquals(2, chnlInfo.size());
			
			assertEquals(chnlInfo.get(0).channelName(), "Channel_1");
			assertEquals(chnlInfo.get(1).channelName(), "Channel_2");

			PackedMsg messagePack = EmaFactory.createPackedMsg(provider);
			
			messagePack.initBuffer();
			
			messagePack.addMsg(EmaFactory.createRefreshMsg().serviceName("NI_PUB_1").name("IBM.N")
					.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
					.payload(fieldList).complete(true), itemHandle);
			
			messagePack.addMsg(EmaFactory.createStatusMsg().serviceName("NI_PUB_1").name("IBM.N")
					.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
					, itemHandle);
			
			messagePack.addMsg(EmaFactory.createGenericMsg().name("IBM.N").payload(fieldList).complete(true), itemHandle);
			
			messagePack.addMsg( EmaFactory.createUpdateMsg().name("IBM.N").payload(fieldList), itemHandle);
			
			// Close both channels
			niConsumer1.closeChannel();
			niConsumer2.closeChannel();
			
			Thread.sleep(500);
			
			try
			{
				provider.submit(messagePack);
				assertTrue(false);
			} catch(OmmInvalidUsageException ommExcept)
			{
				assertEquals(OmmInvalidUsageException.ErrorCode.NO_ACTIVE_CHANNEL, ommExcept.errorCode());
			}
		} catch(Exception e)
		{
			e.printStackTrace();
			assertTrue( false);
		}
		
	}
	
	@Test
	public void NiProviderTwoConnectionSessionPackedMsgPackDuringFullReconnectTest()
	{
		TestUtilities.printTestHead("NiProviderTwoConnectionPackedMsgSessionTest","");

		NiConsumerHelperConfig niConsConfig1 = new NiConsumerHelperConfig();
		NiConsumerHelperConfig niConsConfig2 = new NiConsumerHelperConfig();

		long itemHandle = 10;
		
		niConsConfig1.port = "19001";
		niConsConfig2.port = "19002";

		try
		{
			niProvConfig = EmaFactory.createOmmNiProviderConfig(emaConfigFileLocation);
			niProvConfig.providerName("Provider_Session_1");
			niConsumer1 = new NiConsumerHelper(niConsConfig1);
			niConsumer2 = new NiConsumerHelper(niConsConfig2);
			
			provider = EmaFactory.createOmmProvider(niProvConfig);
			assertNotNull(provider);
			
			// sleep to make sure that the consumer gets everything
			Thread.sleep(100);
			
			assertEquals(2, niConsumer1.getMsgCount());
			
			com.refinitiv.eta.codec.Msg etaMsg = niConsumer1.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REQUEST);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.LOGIN);
			
			com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsg etaLoginMsg = convertToLoginMsg(etaMsg);
			
			assertEquals(etaLoginMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType.REQUEST);
			com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest etaLoginRequest = (com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest)etaLoginMsg;
			assertEquals(etaLoginRequest.role(), com.refinitiv.eta.rdm.Login.RoleTypes.PROV);
			
			etaMsg = niConsumer1.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.SOURCE);
			
			com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsg etaDirectoryMsg = convertToDirectoryRefreshMsg(etaMsg);
			assertEquals(etaDirectoryMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType.REFRESH);
			com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh etaDirectoryRefresh = (com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh)etaDirectoryMsg;
			assertEquals(etaDirectoryRefresh.serviceList().size(), 2);
			
			assertEquals(2, niConsumer2.getMsgCount());
			
			etaMsg = niConsumer2.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REQUEST);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.LOGIN);
			
			etaLoginMsg = convertToLoginMsg(etaMsg);
			
			assertEquals(etaLoginMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType.REQUEST);
			etaLoginRequest = (com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest)etaLoginMsg;
			assertEquals(etaLoginRequest.role(), com.refinitiv.eta.rdm.Login.RoleTypes.PROV);
			
			etaMsg = niConsumer2.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.SOURCE);
			
			etaDirectoryMsg = convertToDirectoryRefreshMsg(etaMsg);
			assertEquals(etaDirectoryMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType.REFRESH);
			etaDirectoryRefresh = (com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh)etaDirectoryMsg;
			assertEquals(etaDirectoryRefresh.serviceList().size(), 2);
			
			FieldList fieldList = EmaFactory.createFieldList();
			fieldList.add( EmaFactory.createFieldEntry().real(22, 3990, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(25, 3994, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(30, 9,  OmmReal.MagnitudeType.EXPONENT_0));
			fieldList.add( EmaFactory.createFieldEntry().real(31, 19, OmmReal.MagnitudeType.EXPONENT_0));
			
			
			List<ChannelInformation> chnlInfo = new ArrayList<ChannelInformation>();

			// Check to see if all the channels are connected
			provider.sessionChannelInfo(chnlInfo);
			
			assertEquals(2, chnlInfo.size());
			
			assertEquals(chnlInfo.get(0).channelName(), "Channel_1");
			assertEquals(chnlInfo.get(1).channelName(), "Channel_2");

			PackedMsg messagePack = EmaFactory.createPackedMsg(provider);
			
			messagePack.initBuffer();
			
			// Close both channels
			niConsumer1.closeChannel();
			niConsumer2.closeChannel();
			
			Thread.sleep(500);
			
			try
			{
				messagePack.addMsg(EmaFactory.createRefreshMsg().serviceName("NI_PUB_1").name("IBM.N")
						.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
						.payload(fieldList).complete(true), itemHandle);
				assertTrue(false);
			} catch(OmmInvalidUsageException ommExcept)
			{
				assertEquals(OmmInvalidUsageException.ErrorCode.NO_ACTIVE_CHANNEL, ommExcept.errorCode());
			}
			
			
		} catch(Exception e)
		{
			e.printStackTrace();
			assertTrue( false);
		}
		
	}
	
	@Test
	public void NiProviderTwoConnectionSessionPackedMsgBuffTooSmallTest()
	{
		TestUtilities.printTestHead("NiProviderTwoConnectionSessionPackedMsgBuffTooSmallTest","");

		NiConsumerHelperConfig niConsConfig1 = new NiConsumerHelperConfig();
		NiConsumerHelperConfig niConsConfig2 = new NiConsumerHelperConfig();

		long itemHandle = 10;
		
		niConsConfig1.port = "19001";
		niConsConfig2.port = "19002";

		try
		{
			niProvConfig = EmaFactory.createOmmNiProviderConfig(emaConfigFileLocation);
			niProvConfig.providerName("Provider_Session_1");
			niConsumer1 = new NiConsumerHelper(niConsConfig1);
			niConsumer2 = new NiConsumerHelper(niConsConfig2);
			
			provider = EmaFactory.createOmmProvider(niProvConfig);
			assertNotNull(provider);
			
			// sleep to make sure that the consumer gets everything
			Thread.sleep(100);
			
			assertEquals(2, niConsumer1.getMsgCount());
			
			com.refinitiv.eta.codec.Msg etaMsg = niConsumer1.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REQUEST);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.LOGIN);
			
			com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsg etaLoginMsg = convertToLoginMsg(etaMsg);
			
			assertEquals(etaLoginMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType.REQUEST);
			com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest etaLoginRequest = (com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest)etaLoginMsg;
			assertEquals(etaLoginRequest.role(), com.refinitiv.eta.rdm.Login.RoleTypes.PROV);
			
			etaMsg = niConsumer1.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.SOURCE);
			
			com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsg etaDirectoryMsg = convertToDirectoryRefreshMsg(etaMsg);
			assertEquals(etaDirectoryMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType.REFRESH);
			com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh etaDirectoryRefresh = (com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh)etaDirectoryMsg;
			assertEquals(etaDirectoryRefresh.serviceList().size(), 2);
			
			assertEquals(2, niConsumer2.getMsgCount());
			
			etaMsg = niConsumer2.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REQUEST);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.LOGIN);
			
			etaLoginMsg = convertToLoginMsg(etaMsg);
			
			assertEquals(etaLoginMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType.REQUEST);
			etaLoginRequest = (com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest)etaLoginMsg;
			assertEquals(etaLoginRequest.role(), com.refinitiv.eta.rdm.Login.RoleTypes.PROV);
			
			etaMsg = niConsumer2.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.SOURCE);
			
			etaDirectoryMsg = convertToDirectoryRefreshMsg(etaMsg);
			assertEquals(etaDirectoryMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType.REFRESH);
			etaDirectoryRefresh = (com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh)etaDirectoryMsg;
			assertEquals(etaDirectoryRefresh.serviceList().size(), 2);
			
			FieldList fieldList = EmaFactory.createFieldList();
			fieldList.add( EmaFactory.createFieldEntry().real(22, 3990, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(25, 3994, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(30, 9,  OmmReal.MagnitudeType.EXPONENT_0));
			fieldList.add( EmaFactory.createFieldEntry().real(31, 19, OmmReal.MagnitudeType.EXPONENT_0));
			
			
			List<ChannelInformation> chnlInfo = new ArrayList<ChannelInformation>();

			// Check to see if all the channels are connected
			provider.sessionChannelInfo(chnlInfo);
			
			assertEquals(2, chnlInfo.size());
			
			assertEquals(chnlInfo.get(0).channelName(), "Channel_1");
			assertEquals(chnlInfo.get(1).channelName(), "Channel_2");

			PackedMsg messagePack = EmaFactory.createPackedMsg(provider);
			
			messagePack.initBuffer(200);
			
			messagePack.addMsg(EmaFactory.createRefreshMsg().serviceName("NI_PUB_1").name("IBM.N")
					.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
					.payload(fieldList).complete(true), itemHandle);
			
			messagePack.addMsg( EmaFactory.createUpdateMsg().id(1).name("IBM.N").payload(fieldList), itemHandle);
	
			messagePack.addMsg( EmaFactory.createUpdateMsg().id(2).name("IBM.N").payload(fieldList), itemHandle);

			try
			{
				messagePack.addMsg( EmaFactory.createUpdateMsg().id(3).name("IBM.N").payload(fieldList), itemHandle);
				assertFalse(true);
			}
			catch(OmmInvalidUsageException e)
			{
				assertEquals(e.errorCode(), OmmInvalidUsageException.ErrorCode.BUFFER_TOO_SMALL);
				
			}

			provider.submit(messagePack);
			
			Thread.sleep(500);
			
			assertEquals(3, niConsumer1.getMsgCount());
			
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
				
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.UPDATE);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			assertEquals(etaMsg.msgKey().identifier(), 1);
			
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.UPDATE);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			assertEquals(etaMsg.msgKey().identifier(), 2);

			assertEquals(3, niConsumer2.getMsgCount());
			
			etaMsg = niConsumer2.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			etaMsg = niConsumer2.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.UPDATE);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");		
			assertEquals(etaMsg.msgKey().identifier(), 1);
			
			etaMsg = niConsumer2.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.UPDATE);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			assertEquals(etaMsg.msgKey().identifier(), 2);
		} catch(Exception e)
		{
			e.printStackTrace();
			assertTrue( false);
		}
		
	}
	
	@Test
	public void NiProviderTwoConnectionSessionReconnectTest()
	{
		TestUtilities.printTestHead("NiProviderTwoConnectionSessionTest","");

		NiConsumerHelperConfig niConsConfig1 = new NiConsumerHelperConfig();
		NiConsumerHelperConfig niConsConfig2 = new NiConsumerHelperConfig();

		long itemHandle = 10;
		
		niConsConfig1.port = "19001";
		niConsConfig2.port = "19002";

		try
		{
			niProvConfig = EmaFactory.createOmmNiProviderConfig(emaConfigFileLocation);
			niProvConfig.providerName("Provider_Session_1");
			niConsumer1 = new NiConsumerHelper(niConsConfig1);
			niConsumer2 = new NiConsumerHelper(niConsConfig2);
			
			provider = EmaFactory.createOmmProvider(niProvConfig);
			assertNotNull(provider);
			
			// sleep to make sure that the consumer gets everything
			Thread.sleep(100);
			
			assertEquals(2, niConsumer1.getMsgCount());
			
			com.refinitiv.eta.codec.Msg etaMsg = niConsumer1.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REQUEST);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.LOGIN);
			
			com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsg etaLoginMsg = convertToLoginMsg(etaMsg);
			
			assertEquals(etaLoginMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType.REQUEST);
			com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest etaLoginRequest = (com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest)etaLoginMsg;
			assertEquals(etaLoginRequest.role(), com.refinitiv.eta.rdm.Login.RoleTypes.PROV);
			
			etaMsg = niConsumer1.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.SOURCE);
			
			com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsg etaDirectoryMsg = convertToDirectoryRefreshMsg(etaMsg);
			assertEquals(etaDirectoryMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType.REFRESH);
			com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh etaDirectoryRefresh = (com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh)etaDirectoryMsg;
			assertEquals(etaDirectoryRefresh.serviceList().size(), 2);
			
			assertEquals(2, niConsumer2.getMsgCount());
			
			etaMsg = niConsumer2.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REQUEST);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.LOGIN);
			
			etaLoginMsg = convertToLoginMsg(etaMsg);
			
			assertEquals(etaLoginMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType.REQUEST);
			etaLoginRequest = (com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest)etaLoginMsg;
			assertEquals(etaLoginRequest.role(), com.refinitiv.eta.rdm.Login.RoleTypes.PROV);
			
			etaMsg = niConsumer2.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.SOURCE);
			
			etaDirectoryMsg = convertToDirectoryRefreshMsg(etaMsg);
			assertEquals(etaDirectoryMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType.REFRESH);
			etaDirectoryRefresh = (com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh)etaDirectoryMsg;
			assertEquals(etaDirectoryRefresh.serviceList().size(), 2);
			
			FieldList fieldList = EmaFactory.createFieldList();
			fieldList.add( EmaFactory.createFieldEntry().real(22, 3990, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(25, 3994, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(30, 9,  OmmReal.MagnitudeType.EXPONENT_0));
			fieldList.add( EmaFactory.createFieldEntry().real(31, 19, OmmReal.MagnitudeType.EXPONENT_0));
			
			
			List<ChannelInformation> chnlInfo = new ArrayList<ChannelInformation>();

			// Check to see if all the channels are connected
			provider.sessionChannelInfo(chnlInfo);
			
			assertEquals(2, chnlInfo.size());
			
			assertEquals(chnlInfo.get(0).channelName(), "Channel_1");
			assertEquals(chnlInfo.get(1).channelName(), "Channel_2");


			provider.submit( EmaFactory.createRefreshMsg().serviceName("NI_PUB_1").name("IBM.N")
					.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
					.payload(fieldList).complete(true), itemHandle);
			
			Thread.sleep(500);
			
			assertEquals(1, niConsumer1.getMsgCount());
			
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			assertEquals(1, niConsumer2.getMsgCount());
			
			etaMsg = niConsumer2.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			provider.submit( EmaFactory.createStatusMsg().serviceName("NI_PUB_1").name("IBM.N")
					.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
					, itemHandle);
			
			Thread.sleep(500);
			
			assertEquals(1, niConsumer1.getMsgCount());
			
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.STATUS);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			assertEquals(1, niConsumer2.getMsgCount());
			
			etaMsg = niConsumer2.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.STATUS);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			
			provider.submit( EmaFactory.createGenericMsg().name("IBM.N").payload(fieldList).complete(true), itemHandle);
			
			Thread.sleep(500);
			
			assertEquals(1, niConsumer1.getMsgCount());
			
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.GENERIC);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			assertEquals(1, niConsumer2.getMsgCount());
			
			etaMsg = niConsumer2.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.GENERIC);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			provider.submit( EmaFactory.createUpdateMsg().name("IBM.N").payload(fieldList), itemHandle);
			
			Thread.sleep(500);
			
			assertEquals(1, niConsumer1.getMsgCount());
			
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.UPDATE);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			assertEquals(1, niConsumer2.getMsgCount());
			
			etaMsg = niConsumer2.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.UPDATE);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");		
			
			// Shutdown niConsumer 2.
			niConsumer2.closeChannel();		
			Thread.sleep(150);
			
			// Check to see if all the channels are connected
			provider.sessionChannelInfo(chnlInfo);
			
			assertEquals(2, chnlInfo.size());
			
			assertEquals(chnlInfo.get(0).channelName(), "Channel_1");
			
			// Submit all messsage types and see that only consumer 1 gets it.
			provider.submit( EmaFactory.createRefreshMsg().serviceName("NI_PUB_1").name("IBM.N")
					.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
					.payload(fieldList).complete(true), itemHandle);
			
			provider.submit( EmaFactory.createStatusMsg().serviceName("NI_PUB_1").name("IBM.N")
					.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
					, itemHandle);
			
			provider.submit( EmaFactory.createGenericMsg().name("IBM.N").payload(fieldList).complete(true), itemHandle);

			provider.submit( EmaFactory.createUpdateMsg().name("IBM.N").payload(fieldList), itemHandle);
			
			Thread.sleep(500);
			
			assertEquals(4, niConsumer1.getMsgCount());
			
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
					
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.STATUS);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");

			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.GENERIC);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.UPDATE);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			assertEquals(0, niConsumer1.getMsgCount());

			// give time to reconnect
			Thread.sleep(2000);
			
			assertEquals(0, niConsumer1.getMsgCount());
			
			assertEquals(2, niConsumer2.getMsgCount());
			
			etaMsg = niConsumer2.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REQUEST);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.LOGIN);
			
			etaLoginMsg = convertToLoginMsg(etaMsg);
			
			assertEquals(etaLoginMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType.REQUEST);
			etaLoginRequest = (com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest)etaLoginMsg;
			assertEquals(etaLoginRequest.role(), com.refinitiv.eta.rdm.Login.RoleTypes.PROV);
			
			etaMsg = niConsumer2.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.SOURCE);
			
			etaDirectoryMsg = convertToDirectoryRefreshMsg(etaMsg);
			assertEquals(etaDirectoryMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType.REFRESH);
			etaDirectoryRefresh = (com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh)etaDirectoryMsg;
			assertEquals(etaDirectoryRefresh.serviceList().size(), 2);
			
			// Shutdown both consumers
			niConsumer1.closeChannel();		
			niConsumer2.closeChannel();		
			Thread.sleep(150);
			
			// Attempt to submit
			try
			{
				provider.submit( EmaFactory.createRefreshMsg().serviceName("NI_PUB_1").name("IBM.N")
					.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
					.payload(fieldList).complete(true), itemHandle);
				
				assertFalse(true);
			} catch(OmmInvalidUsageException except)
			{
				System.out.println("Got exception while attempting to submit on closed channel. Text: " + except.getMessage());
				assertTrue(true);
			}
			
			
		} catch(Exception e)
		{
			e.printStackTrace();
			assertTrue( false);
		}
		
	}
	
	@Test
	public void NiProviderTwoConnectionSessionReconnectPackDuringReconnectTest()
	{
		TestUtilities.printTestHead("NiProviderTwoConnectionReconnectPackDuringReconnectSessionTest","");

		NiConsumerHelperConfig niConsConfig1 = new NiConsumerHelperConfig();
		NiConsumerHelperConfig niConsConfig2 = new NiConsumerHelperConfig();

		long itemHandle = 10;
		
		niConsConfig1.port = "19001";
		niConsConfig2.port = "19002";

		try
		{
			niProvConfig = EmaFactory.createOmmNiProviderConfig(emaConfigFileLocation);
			niProvConfig.providerName("Provider_Session_1");
			niConsumer1 = new NiConsumerHelper(niConsConfig1);
			niConsumer2 = new NiConsumerHelper(niConsConfig2);
			
			provider = EmaFactory.createOmmProvider(niProvConfig);
			assertNotNull(provider);
			
			// sleep to make sure that the consumer gets everything
			Thread.sleep(100);
			
			assertEquals(2, niConsumer1.getMsgCount());
			
			com.refinitiv.eta.codec.Msg etaMsg = niConsumer1.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REQUEST);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.LOGIN);
			
			com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsg etaLoginMsg = convertToLoginMsg(etaMsg);
			
			assertEquals(etaLoginMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType.REQUEST);
			com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest etaLoginRequest = (com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest)etaLoginMsg;
			assertEquals(etaLoginRequest.role(), com.refinitiv.eta.rdm.Login.RoleTypes.PROV);
			
			etaMsg = niConsumer1.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.SOURCE);
			
			com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsg etaDirectoryMsg = convertToDirectoryRefreshMsg(etaMsg);
			assertEquals(etaDirectoryMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType.REFRESH);
			com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh etaDirectoryRefresh = (com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh)etaDirectoryMsg;
			assertEquals(etaDirectoryRefresh.serviceList().size(), 2);
			
			assertEquals(2, niConsumer2.getMsgCount());
			
			etaMsg = niConsumer2.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REQUEST);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.LOGIN);
			
			etaLoginMsg = convertToLoginMsg(etaMsg);
			
			assertEquals(etaLoginMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType.REQUEST);
			etaLoginRequest = (com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest)etaLoginMsg;
			assertEquals(etaLoginRequest.role(), com.refinitiv.eta.rdm.Login.RoleTypes.PROV);
			
			etaMsg = niConsumer2.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.SOURCE);
			
			etaDirectoryMsg = convertToDirectoryRefreshMsg(etaMsg);
			assertEquals(etaDirectoryMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType.REFRESH);
			etaDirectoryRefresh = (com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh)etaDirectoryMsg;
			assertEquals(etaDirectoryRefresh.serviceList().size(), 2);
			
			FieldList fieldList = EmaFactory.createFieldList();
			fieldList.add( EmaFactory.createFieldEntry().real(22, 3990, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(25, 3994, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(30, 9,  OmmReal.MagnitudeType.EXPONENT_0));
			fieldList.add( EmaFactory.createFieldEntry().real(31, 19, OmmReal.MagnitudeType.EXPONENT_0));
			
			
			List<ChannelInformation> chnlInfo = new ArrayList<ChannelInformation>();

			// Check to see if all the channels are connected
			provider.sessionChannelInfo(chnlInfo);
			
			assertEquals(2, chnlInfo.size());
			
			assertEquals(chnlInfo.get(0).channelName(), "Channel_1");
			assertEquals(chnlInfo.get(1).channelName(), "Channel_2");


			
			PackedMsg messagePack = EmaFactory.createPackedMsg(provider);
			
			messagePack.initBuffer();
			
			messagePack.addMsg(EmaFactory.createRefreshMsg().serviceName("NI_PUB_1").name("IBM.N")
					.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
					.payload(fieldList).complete(true), itemHandle);
			
			messagePack.addMsg(EmaFactory.createStatusMsg().serviceName("NI_PUB_1").name("IBM.N")
					.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
					, itemHandle);
			
			messagePack.addMsg(EmaFactory.createGenericMsg().name("IBM.N").payload(fieldList).complete(true), itemHandle);
			
			messagePack.addMsg( EmaFactory.createUpdateMsg().name("IBM.N").payload(fieldList), itemHandle);
			
			provider.submit(messagePack);
			
			Thread.sleep(500);
			
			assertEquals(4, niConsumer1.getMsgCount());
			
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.STATUS);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.GENERIC);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.UPDATE);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			assertEquals(4, niConsumer2.getMsgCount());
			
			etaMsg = niConsumer2.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			etaMsg = niConsumer2.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.STATUS);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			etaMsg = niConsumer2.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.GENERIC);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");

			etaMsg = niConsumer2.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.UPDATE);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");		
			
			// Test 1: kill the connection during the pack, send while channel still down.
			messagePack.initBuffer();
			
			messagePack.addMsg(EmaFactory.createRefreshMsg().serviceName("NI_PUB_1").name("IBM.N")
					.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
					.payload(fieldList).complete(true), itemHandle);
			
			messagePack.addMsg(EmaFactory.createStatusMsg().serviceName("NI_PUB_1").name("IBM.N")
					.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
					, itemHandle);
			
			// Shutdown niConsumer 2.
			niConsumer2.closeChannel();		
			Thread.sleep(150);
			
			messagePack.addMsg(EmaFactory.createGenericMsg().name("IBM.N").payload(fieldList).complete(true), itemHandle);
			
			messagePack.addMsg( EmaFactory.createUpdateMsg().name("IBM.N").payload(fieldList), itemHandle);
			
			provider.submit(messagePack);
			
			Thread.sleep(100);
			
			assertEquals(4, niConsumer1.getMsgCount());
			
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.STATUS);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.GENERIC);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.UPDATE);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			assertEquals(0, niConsumer2.getMsgCount());
			
			// give time to reconnect
			Thread.sleep(2000);
			
			assertEquals(0, niConsumer1.getMsgCount());
			
			assertEquals(2, niConsumer2.getMsgCount());
			
			etaMsg = niConsumer2.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REQUEST);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.LOGIN);
			
			etaLoginMsg = convertToLoginMsg(etaMsg);
			
			assertEquals(etaLoginMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType.REQUEST);
			etaLoginRequest = (com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest)etaLoginMsg;
			assertEquals(etaLoginRequest.role(), com.refinitiv.eta.rdm.Login.RoleTypes.PROV);
			
			etaMsg = niConsumer2.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.SOURCE);
			
			etaDirectoryMsg = convertToDirectoryRefreshMsg(etaMsg);
			assertEquals(etaDirectoryMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType.REFRESH);
			etaDirectoryRefresh = (com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh)etaDirectoryMsg;
			assertEquals(etaDirectoryRefresh.serviceList().size(), 2);			
		
			
		} catch(Exception e)
		{
			e.printStackTrace();
			assertTrue( false);
		}
		
	}
	
	@Test
	public void NiProviderTwoConnectionSessionReconnectPackAfterReconnectTest()
	{
		TestUtilities.printTestHead("NiProviderTwoConnectionReconnectPackAfterReconnectSessionTest","");

		NiConsumerHelperConfig niConsConfig1 = new NiConsumerHelperConfig();
		NiConsumerHelperConfig niConsConfig2 = new NiConsumerHelperConfig();

		long itemHandle = 10;
		
		niConsConfig1.port = "19001";
		niConsConfig2.port = "19002";

		try
		{
			niProvConfig = EmaFactory.createOmmNiProviderConfig(emaConfigFileLocation);
			niProvConfig.providerName("Provider_Session_1");
			niConsumer1 = new NiConsumerHelper(niConsConfig1);
			niConsumer2 = new NiConsumerHelper(niConsConfig2);
			
			provider = EmaFactory.createOmmProvider(niProvConfig);
			assertNotNull(provider);
			
			// sleep to make sure that the consumer gets everything
			Thread.sleep(100);
			
			assertEquals(2, niConsumer1.getMsgCount());
			
			com.refinitiv.eta.codec.Msg etaMsg = niConsumer1.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REQUEST);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.LOGIN);
			
			com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsg etaLoginMsg = convertToLoginMsg(etaMsg);
			
			assertEquals(etaLoginMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType.REQUEST);
			com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest etaLoginRequest = (com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest)etaLoginMsg;
			assertEquals(etaLoginRequest.role(), com.refinitiv.eta.rdm.Login.RoleTypes.PROV);
			
			etaMsg = niConsumer1.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.SOURCE);
			
			com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsg etaDirectoryMsg = convertToDirectoryRefreshMsg(etaMsg);
			assertEquals(etaDirectoryMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType.REFRESH);
			com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh etaDirectoryRefresh = (com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh)etaDirectoryMsg;
			assertEquals(etaDirectoryRefresh.serviceList().size(), 2);
			
			assertEquals(2, niConsumer2.getMsgCount());
			
			etaMsg = niConsumer2.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REQUEST);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.LOGIN);
			
			etaLoginMsg = convertToLoginMsg(etaMsg);
			
			assertEquals(etaLoginMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType.REQUEST);
			etaLoginRequest = (com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest)etaLoginMsg;
			assertEquals(etaLoginRequest.role(), com.refinitiv.eta.rdm.Login.RoleTypes.PROV);
			
			etaMsg = niConsumer2.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.SOURCE);
			
			etaDirectoryMsg = convertToDirectoryRefreshMsg(etaMsg);
			assertEquals(etaDirectoryMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType.REFRESH);
			etaDirectoryRefresh = (com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh)etaDirectoryMsg;
			assertEquals(etaDirectoryRefresh.serviceList().size(), 2);
			
			FieldList fieldList = EmaFactory.createFieldList();
			fieldList.add( EmaFactory.createFieldEntry().real(22, 3990, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(25, 3994, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(30, 9,  OmmReal.MagnitudeType.EXPONENT_0));
			fieldList.add( EmaFactory.createFieldEntry().real(31, 19, OmmReal.MagnitudeType.EXPONENT_0));
			
			
			List<ChannelInformation> chnlInfo = new ArrayList<ChannelInformation>();

			// Check to see if all the channels are connected
			provider.sessionChannelInfo(chnlInfo);
			
			assertEquals(2, chnlInfo.size());
			
			assertEquals(chnlInfo.get(0).channelName(), "Channel_1");
			assertEquals(chnlInfo.get(1).channelName(), "Channel_2");


			
			PackedMsg messagePack = EmaFactory.createPackedMsg(provider);
			
			messagePack.initBuffer();
			
			messagePack.addMsg(EmaFactory.createRefreshMsg().serviceName("NI_PUB_1").name("IBM.N")
					.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
					.payload(fieldList).complete(true), itemHandle);
			
			messagePack.addMsg(EmaFactory.createStatusMsg().serviceName("NI_PUB_1").name("IBM.N")
					.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
					, itemHandle);
			
			messagePack.addMsg(EmaFactory.createGenericMsg().name("IBM.N").payload(fieldList).complete(true), itemHandle);
			
			messagePack.addMsg( EmaFactory.createUpdateMsg().name("IBM.N").payload(fieldList), itemHandle);
			
			provider.submit(messagePack);
			
			Thread.sleep(500);
			
			assertEquals(4, niConsumer1.getMsgCount());
			
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.STATUS);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.GENERIC);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.UPDATE);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			assertEquals(4, niConsumer2.getMsgCount());
			
			etaMsg = niConsumer2.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			etaMsg = niConsumer2.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.STATUS);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			etaMsg = niConsumer2.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.GENERIC);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");

			etaMsg = niConsumer2.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.UPDATE);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");		
			
			// Attempt to pack after reconnect
			messagePack.initBuffer();
			
			messagePack.addMsg(EmaFactory.createRefreshMsg().serviceName("NI_PUB_1").name("IBM.N")
					.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
					.payload(fieldList).complete(true), itemHandle);
			
			messagePack.addMsg(EmaFactory.createStatusMsg().serviceName("NI_PUB_1").name("IBM.N")
					.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
					, itemHandle);
			
			// Shutdown niConsumer 2.
			niConsumer2.closeChannel();		
			// give time to reconnect
			Thread.sleep(2000);
			
			assertEquals(0, niConsumer1.getMsgCount());
			
			assertEquals(2, niConsumer2.getMsgCount());
			
			etaMsg = niConsumer2.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REQUEST);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.LOGIN);
			
			etaLoginMsg = convertToLoginMsg(etaMsg);
			
			assertEquals(etaLoginMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType.REQUEST);
			etaLoginRequest = (com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest)etaLoginMsg;
			assertEquals(etaLoginRequest.role(), com.refinitiv.eta.rdm.Login.RoleTypes.PROV);
			
			etaMsg = niConsumer2.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.SOURCE);
			
			etaDirectoryMsg = convertToDirectoryRefreshMsg(etaMsg);
			assertEquals(etaDirectoryMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType.REFRESH);
			etaDirectoryRefresh = (com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh)etaDirectoryMsg;
			assertEquals(etaDirectoryRefresh.serviceList().size(), 2);
			
			messagePack.addMsg(EmaFactory.createGenericMsg().name("IBM.N").payload(fieldList).complete(true), itemHandle);
			
			messagePack.addMsg( EmaFactory.createUpdateMsg().name("IBM.N").payload(fieldList), itemHandle);
			
			provider.submit(messagePack);
			
			Thread.sleep(100);
			
			assertEquals(4, niConsumer1.getMsgCount());
			
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.STATUS);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.GENERIC);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.UPDATE);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			assertEquals(0, niConsumer2.getMsgCount());
			
			
			
			// Shutdown both consumers
			niConsumer1.closeChannel();		
			niConsumer2.closeChannel();		
			Thread.sleep(150);
			
			// Attempt to submit
			try
			{
				provider.submit( EmaFactory.createRefreshMsg().serviceName("NI_PUB_1").name("IBM.N")
					.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
					.payload(fieldList).complete(true), itemHandle);
				
				assertFalse(true);
			} catch(OmmInvalidUsageException except)
			{
				System.out.println("Got exception while attempting to submit on closed channel. Text: " + except.getMessage());
				assertTrue(true);
			}
			
			
		} catch(Exception e)
		{
			e.printStackTrace();
			assertTrue( false);
		}
		
	}
	
	@Test
	public void NiProviderTwoConnectionSessionReconnectPackSendAfterReconnectTest()
	{
		TestUtilities.printTestHead("NiProviderTwoConnectionReconnectSendAfterReconnectSessionTest","");

		NiConsumerHelperConfig niConsConfig1 = new NiConsumerHelperConfig();
		NiConsumerHelperConfig niConsConfig2 = new NiConsumerHelperConfig();

		long itemHandle = 10;
		
		niConsConfig1.port = "19001";
		niConsConfig2.port = "19002";

		try
		{
			niProvConfig = EmaFactory.createOmmNiProviderConfig(emaConfigFileLocation);
			niProvConfig.providerName("Provider_Session_1");
			niConsumer1 = new NiConsumerHelper(niConsConfig1);
			niConsumer2 = new NiConsumerHelper(niConsConfig2);
			
			provider = EmaFactory.createOmmProvider(niProvConfig);
			assertNotNull(provider);
			
			// sleep to make sure that the consumer gets everything
			Thread.sleep(100);
			
			assertEquals(2, niConsumer1.getMsgCount());
			
			com.refinitiv.eta.codec.Msg etaMsg = niConsumer1.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REQUEST);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.LOGIN);
			
			com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsg etaLoginMsg = convertToLoginMsg(etaMsg);
			
			assertEquals(etaLoginMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType.REQUEST);
			com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest etaLoginRequest = (com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest)etaLoginMsg;
			assertEquals(etaLoginRequest.role(), com.refinitiv.eta.rdm.Login.RoleTypes.PROV);
			
			etaMsg = niConsumer1.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.SOURCE);
			
			com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsg etaDirectoryMsg = convertToDirectoryRefreshMsg(etaMsg);
			assertEquals(etaDirectoryMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType.REFRESH);
			com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh etaDirectoryRefresh = (com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh)etaDirectoryMsg;
			assertEquals(etaDirectoryRefresh.serviceList().size(), 2);
			
			assertEquals(2, niConsumer2.getMsgCount());
			
			etaMsg = niConsumer2.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REQUEST);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.LOGIN);
			
			etaLoginMsg = convertToLoginMsg(etaMsg);
			
			assertEquals(etaLoginMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType.REQUEST);
			etaLoginRequest = (com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest)etaLoginMsg;
			assertEquals(etaLoginRequest.role(), com.refinitiv.eta.rdm.Login.RoleTypes.PROV);
			
			etaMsg = niConsumer2.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.SOURCE);
			
			etaDirectoryMsg = convertToDirectoryRefreshMsg(etaMsg);
			assertEquals(etaDirectoryMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType.REFRESH);
			etaDirectoryRefresh = (com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh)etaDirectoryMsg;
			assertEquals(etaDirectoryRefresh.serviceList().size(), 2);
			
			FieldList fieldList = EmaFactory.createFieldList();
			fieldList.add( EmaFactory.createFieldEntry().real(22, 3990, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(25, 3994, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(30, 9,  OmmReal.MagnitudeType.EXPONENT_0));
			fieldList.add( EmaFactory.createFieldEntry().real(31, 19, OmmReal.MagnitudeType.EXPONENT_0));
			
			
			List<ChannelInformation> chnlInfo = new ArrayList<ChannelInformation>();

			// Check to see if all the channels are connected
			provider.sessionChannelInfo(chnlInfo);
			
			assertEquals(2, chnlInfo.size());
			
			assertEquals(chnlInfo.get(0).channelName(), "Channel_1");
			assertEquals(chnlInfo.get(1).channelName(), "Channel_2");


			
			PackedMsg messagePack = EmaFactory.createPackedMsg(provider);
			
			messagePack.initBuffer();
			
			messagePack.addMsg(EmaFactory.createRefreshMsg().serviceName("NI_PUB_1").name("IBM.N")
					.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
					.payload(fieldList).complete(true), itemHandle);
			
			messagePack.addMsg(EmaFactory.createStatusMsg().serviceName("NI_PUB_1").name("IBM.N")
					.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
					, itemHandle);
			
			messagePack.addMsg(EmaFactory.createGenericMsg().name("IBM.N").payload(fieldList).complete(true), itemHandle);
			
			messagePack.addMsg( EmaFactory.createUpdateMsg().name("IBM.N").payload(fieldList), itemHandle);
			
			provider.submit(messagePack);
			
			Thread.sleep(500);
			
			assertEquals(4, niConsumer1.getMsgCount());
			
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.STATUS);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.GENERIC);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.UPDATE);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			assertEquals(4, niConsumer2.getMsgCount());
			
			etaMsg = niConsumer2.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			etaMsg = niConsumer2.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.STATUS);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			etaMsg = niConsumer2.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.GENERIC);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");

			etaMsg = niConsumer2.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.UPDATE);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");		
			
			// Attempt to pack after reconnect
			messagePack.initBuffer();
			
			messagePack.addMsg(EmaFactory.createRefreshMsg().serviceName("NI_PUB_1").name("IBM.N")
					.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
					.payload(fieldList).complete(true), itemHandle);
			
			messagePack.addMsg(EmaFactory.createStatusMsg().serviceName("NI_PUB_1").name("IBM.N")
					.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
					, itemHandle);
			
			messagePack.addMsg(EmaFactory.createGenericMsg().name("IBM.N").payload(fieldList).complete(true), itemHandle);
			
			messagePack.addMsg( EmaFactory.createUpdateMsg().name("IBM.N").payload(fieldList), itemHandle);
			
			// Shutdown niConsumer 2.
			niConsumer2.closeChannel();		
			// give time to reconnect
			Thread.sleep(2000);
			
			assertEquals(0, niConsumer1.getMsgCount());
			
			assertEquals(2, niConsumer2.getMsgCount());
			
			etaMsg = niConsumer2.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REQUEST);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.LOGIN);
			
			etaLoginMsg = convertToLoginMsg(etaMsg);
			
			assertEquals(etaLoginMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType.REQUEST);
			etaLoginRequest = (com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest)etaLoginMsg;
			assertEquals(etaLoginRequest.role(), com.refinitiv.eta.rdm.Login.RoleTypes.PROV);
			
			etaMsg = niConsumer2.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.SOURCE);
			
			etaDirectoryMsg = convertToDirectoryRefreshMsg(etaMsg);
			assertEquals(etaDirectoryMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType.REFRESH);
			etaDirectoryRefresh = (com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh)etaDirectoryMsg;
			assertEquals(etaDirectoryRefresh.serviceList().size(), 2);
			
			provider.submit(messagePack);
			
			Thread.sleep(100);
			
			assertEquals(4, niConsumer1.getMsgCount());
			
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.STATUS);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.GENERIC);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.UPDATE);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			assertEquals(0, niConsumer2.getMsgCount());
			
			
		} catch(Exception e)
		{
			e.printStackTrace();
			assertTrue( false);
		}
		
	}
	
	@Test
	public void NiProviderTwoConnectionSessionGetPackWithOneConnectionTest()
	{
		TestUtilities.printTestHead("NiProviderTwoConnectionGetPackWithOneConnectionSessionTest","");

		NiConsumerHelperConfig niConsConfig1 = new NiConsumerHelperConfig();
		NiConsumerHelperConfig niConsConfig2 = new NiConsumerHelperConfig();

		long itemHandle = 10;
		
		niConsConfig1.port = "19001";
		niConsConfig2.port = "19002";

		try
		{
			niProvConfig = EmaFactory.createOmmNiProviderConfig(emaConfigFileLocation);
			niProvConfig.providerName("Provider_Session_1");
			niConsumer1 = new NiConsumerHelper(niConsConfig1);
			niConsumer2 = new NiConsumerHelper(niConsConfig2);
			
			provider = EmaFactory.createOmmProvider(niProvConfig);
			assertNotNull(provider);
			
			// sleep to make sure that the consumer gets everything
			Thread.sleep(100);
			
			assertEquals(2, niConsumer1.getMsgCount());
			
			com.refinitiv.eta.codec.Msg etaMsg = niConsumer1.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REQUEST);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.LOGIN);
			
			com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsg etaLoginMsg = convertToLoginMsg(etaMsg);
			
			assertEquals(etaLoginMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType.REQUEST);
			com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest etaLoginRequest = (com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest)etaLoginMsg;
			assertEquals(etaLoginRequest.role(), com.refinitiv.eta.rdm.Login.RoleTypes.PROV);
			
			etaMsg = niConsumer1.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.SOURCE);
			
			com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsg etaDirectoryMsg = convertToDirectoryRefreshMsg(etaMsg);
			assertEquals(etaDirectoryMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType.REFRESH);
			com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh etaDirectoryRefresh = (com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh)etaDirectoryMsg;
			assertEquals(etaDirectoryRefresh.serviceList().size(), 2);
			
			assertEquals(2, niConsumer2.getMsgCount());
			
			etaMsg = niConsumer2.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REQUEST);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.LOGIN);
			
			etaLoginMsg = convertToLoginMsg(etaMsg);
			
			assertEquals(etaLoginMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType.REQUEST);
			etaLoginRequest = (com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest)etaLoginMsg;
			assertEquals(etaLoginRequest.role(), com.refinitiv.eta.rdm.Login.RoleTypes.PROV);
			
			etaMsg = niConsumer2.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.SOURCE);
			
			etaDirectoryMsg = convertToDirectoryRefreshMsg(etaMsg);
			assertEquals(etaDirectoryMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType.REFRESH);
			etaDirectoryRefresh = (com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh)etaDirectoryMsg;
			assertEquals(etaDirectoryRefresh.serviceList().size(), 2);
			
			FieldList fieldList = EmaFactory.createFieldList();
			fieldList.add( EmaFactory.createFieldEntry().real(22, 3990, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(25, 3994, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(30, 9,  OmmReal.MagnitudeType.EXPONENT_0));
			fieldList.add( EmaFactory.createFieldEntry().real(31, 19, OmmReal.MagnitudeType.EXPONENT_0));
			
			
			List<ChannelInformation> chnlInfo = new ArrayList<ChannelInformation>();

			// Check to see if all the channels are connected
			provider.sessionChannelInfo(chnlInfo);
			
			assertEquals(2, chnlInfo.size());
			
			assertEquals(chnlInfo.get(0).channelName(), "Channel_1");
			assertEquals(chnlInfo.get(1).channelName(), "Channel_2");


			
			PackedMsg messagePack = EmaFactory.createPackedMsg(provider);
			
			messagePack.initBuffer();
			
			messagePack.addMsg(EmaFactory.createRefreshMsg().serviceName("NI_PUB_1").name("IBM.N")
					.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
					.payload(fieldList).complete(true), itemHandle);
			
			messagePack.addMsg(EmaFactory.createStatusMsg().serviceName("NI_PUB_1").name("IBM.N")
					.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
					, itemHandle);
			
			messagePack.addMsg(EmaFactory.createGenericMsg().name("IBM.N").payload(fieldList).complete(true), itemHandle);
			
			messagePack.addMsg( EmaFactory.createUpdateMsg().name("IBM.N").payload(fieldList), itemHandle);
			
			provider.submit(messagePack);
			
			Thread.sleep(500);
			
			assertEquals(4, niConsumer1.getMsgCount());
			
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.STATUS);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.GENERIC);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.UPDATE);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			assertEquals(4, niConsumer2.getMsgCount());
			
			etaMsg = niConsumer2.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			etaMsg = niConsumer2.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.STATUS);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			etaMsg = niConsumer2.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.GENERIC);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");

			etaMsg = niConsumer2.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.UPDATE);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");		
			
			// Shutdown niConsumer 2.
			niConsumer2.closeChannel();		
			// give time to reconnect
			Thread.sleep(150);
			
			// Attempt to pack after reconnect
			messagePack.initBuffer();
			
			messagePack.addMsg(EmaFactory.createRefreshMsg().serviceName("NI_PUB_1").name("IBM.N")
					.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
					.payload(fieldList).complete(true), itemHandle);
			
			messagePack.addMsg(EmaFactory.createStatusMsg().serviceName("NI_PUB_1").name("IBM.N")
					.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
					, itemHandle);
			
			messagePack.addMsg(EmaFactory.createGenericMsg().name("IBM.N").payload(fieldList).complete(true), itemHandle);
			
			messagePack.addMsg( EmaFactory.createUpdateMsg().name("IBM.N").payload(fieldList), itemHandle);
			
			Thread.sleep(2000);
			
			assertEquals(0, niConsumer1.getMsgCount());
			
			assertEquals(2, niConsumer2.getMsgCount());
			
			etaMsg = niConsumer2.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REQUEST);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.LOGIN);
			
			etaLoginMsg = convertToLoginMsg(etaMsg);
			
			assertEquals(etaLoginMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType.REQUEST);
			etaLoginRequest = (com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest)etaLoginMsg;
			assertEquals(etaLoginRequest.role(), com.refinitiv.eta.rdm.Login.RoleTypes.PROV);
			
			etaMsg = niConsumer2.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.SOURCE);
			
			etaDirectoryMsg = convertToDirectoryRefreshMsg(etaMsg);
			assertEquals(etaDirectoryMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType.REFRESH);
			etaDirectoryRefresh = (com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh)etaDirectoryMsg;
			assertEquals(etaDirectoryRefresh.serviceList().size(), 2);
			
			provider.submit(messagePack);
			
			Thread.sleep(100);
			
			assertEquals(4, niConsumer1.getMsgCount());
			
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.STATUS);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.GENERIC);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.UPDATE);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			assertEquals(0, niConsumer2.getMsgCount());
			
			
		} catch(Exception e)
		{
			e.printStackTrace();
			assertTrue( false);
		}
		
	}
	
	@Test
	public void NiProviderTwoConnectionSessionChannelSetReconnectionTest()
	{
		TestUtilities.printTestHead("NiProviderTwoConnectionChannelSetSessionTest","");

		NiConsumerHelperConfig niConsConfig1 = new NiConsumerHelperConfig();
		NiConsumerHelperConfig niConsConfig2 = new NiConsumerHelperConfig();
		NiConsumerHelperConfig niConsConfig3 = new NiConsumerHelperConfig();
		NiConsumerHelperConfig niConsConfig4 = new NiConsumerHelperConfig();

		long itemHandle = 10;
		
		niConsConfig1.port = "19001";
		niConsConfig2.port = "19002";
		niConsConfig3.port = "19003";
		niConsConfig4.port = "19004";

		try
		{
			niProvConfig = EmaFactory.createOmmNiProviderConfig(emaConfigFileLocation);
			niProvConfig.providerName("Provider_Session_ChannelSet");
			niConsumer1 = new NiConsumerHelper(niConsConfig1);
			niConsumer2 = new NiConsumerHelper(niConsConfig2);
			niConsumer3 = new NiConsumerHelper(niConsConfig3);
			niConsumer4 = new NiConsumerHelper(niConsConfig4);
			
			provider = EmaFactory.createOmmProvider(niProvConfig);
			assertNotNull(provider);
			
			// sleep to make sure that the consumer gets everything
			Thread.sleep(100);
			
			assertEquals(2, niConsumer1.getMsgCount());
			
			com.refinitiv.eta.codec.Msg etaMsg = niConsumer1.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REQUEST);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.LOGIN);
			
			com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsg etaLoginMsg = convertToLoginMsg(etaMsg);
			
			assertEquals(etaLoginMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType.REQUEST);
			com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest etaLoginRequest = (com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest)etaLoginMsg;
			assertEquals(etaLoginRequest.role(), com.refinitiv.eta.rdm.Login.RoleTypes.PROV);
			
			etaMsg = niConsumer1.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.SOURCE);
			
			com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsg etaDirectoryMsg = convertToDirectoryRefreshMsg(etaMsg);
			assertEquals(etaDirectoryMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType.REFRESH);
			com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh etaDirectoryRefresh = (com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh)etaDirectoryMsg;
			assertEquals(etaDirectoryRefresh.serviceList().size(), 2);
			
			assertEquals(0, niConsumer2.getMsgCount());

			assertEquals(2, niConsumer3.getMsgCount());
			
			etaMsg = niConsumer3.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REQUEST);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.LOGIN);
			
			etaLoginMsg = convertToLoginMsg(etaMsg);
			
			assertEquals(etaLoginMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType.REQUEST);
			etaLoginRequest = (com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest)etaLoginMsg;
			assertEquals(etaLoginRequest.role(), com.refinitiv.eta.rdm.Login.RoleTypes.PROV);
			
			etaMsg = niConsumer3.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.SOURCE);
			
			etaDirectoryMsg = convertToDirectoryRefreshMsg(etaMsg);
			assertEquals(etaDirectoryMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType.REFRESH);
			etaDirectoryRefresh = (com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh)etaDirectoryMsg;
			assertEquals(etaDirectoryRefresh.serviceList().size(), 2);
			
			assertEquals(0, niConsumer4.getMsgCount());

			
			FieldList fieldList = EmaFactory.createFieldList();
			fieldList.add( EmaFactory.createFieldEntry().real(22, 3990, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(25, 3994, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(30, 9,  OmmReal.MagnitudeType.EXPONENT_0));
			fieldList.add( EmaFactory.createFieldEntry().real(31, 19, OmmReal.MagnitudeType.EXPONENT_0));
			
			
			List<ChannelInformation> chnlInfo = new ArrayList<ChannelInformation>();

			// Check to see if all the channels are connected
			provider.sessionChannelInfo(chnlInfo);
			
			assertEquals(2, chnlInfo.size());
			
			assertEquals(chnlInfo.get(0).channelName(), "Channel_1");
			assertEquals(chnlInfo.get(1).channelName(), "Channel_3");

			provider.submit( EmaFactory.createRefreshMsg().serviceName("NI_PUB_1").name("IBM.N")
					.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
					.payload(fieldList).complete(true), itemHandle);
			
			Thread.sleep(500);
			
			assertEquals(1, niConsumer1.getMsgCount());
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			assertEquals(0, niConsumer2.getMsgCount());
			
			assertEquals(1, niConsumer3.getMsgCount());
			etaMsg = niConsumer3.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			assertEquals(0, niConsumer4.getMsgCount());

			// Shutdown niConsumer 2.
			niConsumer3.closeChannel();		
			Thread.sleep(150);
			
			// Check to see if all the channels are connected
			provider.sessionChannelInfo(chnlInfo);
			
			assertEquals(2, chnlInfo.size());
			
			assertEquals(chnlInfo.get(0).channelName(), "Channel_1");
			
			provider.submit( EmaFactory.createRefreshMsg().serviceName("NI_PUB_1").name("IBM.N")
					.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
					.payload(fieldList).complete(true), itemHandle);
			
			Thread.sleep(100);
			
			assertEquals(1, niConsumer1.getMsgCount());
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			assertEquals(0, niConsumer2.getMsgCount());
			
			assertEquals(0, niConsumer3.getMsgCount());
			
			assertEquals(0, niConsumer4.getMsgCount());
			
			// give time to reconnect
			Thread.sleep(2000);
			
			// Check to see if all the channels are connected
			provider.sessionChannelInfo(chnlInfo);
			
			assertEquals(2, chnlInfo.size());
			
			assertEquals(chnlInfo.get(0).channelName(), "Channel_1");
			assertEquals(chnlInfo.get(1).channelName(), "Channel_4");
			
			assertEquals(0, niConsumer1.getMsgCount());
			
			assertEquals(0, niConsumer2.getMsgCount());
			
			assertEquals(0, niConsumer3.getMsgCount());
			
			assertEquals(2, niConsumer4.getMsgCount());

			etaMsg = niConsumer4.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REQUEST);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.LOGIN);
			
			etaLoginMsg = convertToLoginMsg(etaMsg);
			
			assertEquals(etaLoginMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType.REQUEST);
			etaLoginRequest = (com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest)etaLoginMsg;
			assertEquals(etaLoginRequest.role(), com.refinitiv.eta.rdm.Login.RoleTypes.PROV);
			
			etaMsg = niConsumer4.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.SOURCE);
			
			etaDirectoryMsg = convertToDirectoryRefreshMsg(etaMsg);
			assertEquals(etaDirectoryMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType.REFRESH);
			etaDirectoryRefresh = (com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh)etaDirectoryMsg;
			assertEquals(etaDirectoryRefresh.serviceList().size(), 2);
			
			provider.submit( EmaFactory.createRefreshMsg().serviceName("NI_PUB_1").name("IBM.N")
					.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
					.payload(fieldList).complete(true), itemHandle);
			
			Thread.sleep(100);
			
			assertEquals(1, niConsumer1.getMsgCount());
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			assertEquals(0, niConsumer2.getMsgCount());
			
			assertEquals(0, niConsumer3.getMsgCount());

			assertEquals(1, niConsumer4.getMsgCount());
			etaMsg = niConsumer4.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");

			// Shutdown Channel 1
			niConsumer1.closeChannel();		
			Thread.sleep(150);
			
			// Check to see if all the channels are connected
			provider.sessionChannelInfo(chnlInfo);
			
			assertEquals(2, chnlInfo.size());
			
			assertEquals(chnlInfo.get(1).channelName(), "Channel_4");
			
			provider.submit( EmaFactory.createRefreshMsg().serviceName("NI_PUB_1").name("IBM.N")
					.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
					.payload(fieldList).complete(true), itemHandle);
			
			Thread.sleep(100);
			
			assertEquals(0, niConsumer1.getMsgCount());
			
			assertEquals(0, niConsumer2.getMsgCount());
			
			assertEquals(0, niConsumer3.getMsgCount());
			
			assertEquals(1, niConsumer4.getMsgCount());
			
			etaMsg = niConsumer4.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			// give time to reconnect
			Thread.sleep(2000);
			
			// Check to see if all the channels are connected
			provider.sessionChannelInfo(chnlInfo);
			
			assertEquals(2, chnlInfo.size());
			
			assertEquals(chnlInfo.get(0).channelName(), "Channel_2");
			assertEquals(chnlInfo.get(1).channelName(), "Channel_4");
			
			assertEquals(0, niConsumer1.getMsgCount());
			
			assertEquals(2, niConsumer2.getMsgCount());
			
			assertEquals(0, niConsumer3.getMsgCount());
			
			assertEquals(0, niConsumer4.getMsgCount());

			etaMsg = niConsumer2.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REQUEST);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.LOGIN);
			
			etaLoginMsg = convertToLoginMsg(etaMsg);
			
			assertEquals(etaLoginMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType.REQUEST);
			etaLoginRequest = (com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest)etaLoginMsg;
			assertEquals(etaLoginRequest.role(), com.refinitiv.eta.rdm.Login.RoleTypes.PROV);
			
			etaMsg = niConsumer2.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.SOURCE);
			
			etaDirectoryMsg = convertToDirectoryRefreshMsg(etaMsg);
			assertEquals(etaDirectoryMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType.REFRESH);
			etaDirectoryRefresh = (com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh)etaDirectoryMsg;
			assertEquals(etaDirectoryRefresh.serviceList().size(), 2);
			
			provider.submit( EmaFactory.createRefreshMsg().serviceName("NI_PUB_1").name("IBM.N")
					.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
					.payload(fieldList).complete(true), itemHandle);
			
			Thread.sleep(100);
			
			assertEquals(0, niConsumer1.getMsgCount());
			
			assertEquals(1, niConsumer2.getMsgCount());
			etaMsg = niConsumer2.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			assertEquals(0, niConsumer3.getMsgCount());

			assertEquals(1, niConsumer4.getMsgCount());
			etaMsg = niConsumer4.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");

		} catch(Exception e)
		{
			e.printStackTrace();
			assertTrue( false);
		}
		
	}
	
	@Test
	public void NiProviderTwoConnectionSessionDictionaryRequestTest()
	{
		TestUtilities.printTestHead("NiProviderTwoConnectionSessionDictionaryRequestTest","");

		NiConsumerHelperConfig niConsConfig1 = new NiConsumerHelperConfig();
		NiConsumerHelperConfig niConsConfig2 = new NiConsumerHelperConfig();
		
		niConsConfig1.port = "19001";
		niConsConfig2.port = "19002";

		try
		{
			niProvConfig = EmaFactory.createOmmNiProviderConfig(emaConfigFileLocation);
			niProvConfig.providerName("Provider_Session_1");
			niConsumer1 = new NiConsumerHelper(niConsConfig1);
			niConsumer2 = new NiConsumerHelper(niConsConfig2);
			
			provider = EmaFactory.createOmmProvider(niProvConfig);
			assertNotNull(provider);
			
			// sleep to make sure that the consumer gets everything
			Thread.sleep(100);
			
			assertEquals(2, niConsumer1.getMsgCount());
			
			com.refinitiv.eta.codec.Msg etaMsg = niConsumer1.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REQUEST);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.LOGIN);
			
			com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsg etaLoginMsg = convertToLoginMsg(etaMsg);
			
			assertEquals(etaLoginMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType.REQUEST);
			com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest etaLoginRequest = (com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest)etaLoginMsg;
			assertEquals(etaLoginRequest.role(), com.refinitiv.eta.rdm.Login.RoleTypes.PROV);
			
			etaMsg = niConsumer1.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.SOURCE);
			
			com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsg etaDirectoryMsg = convertToDirectoryRefreshMsg(etaMsg);
			assertEquals(etaDirectoryMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType.REFRESH);
			com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh etaDirectoryRefresh = (com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh)etaDirectoryMsg;
			assertEquals(etaDirectoryRefresh.serviceList().size(), 2);
			
			assertEquals(2, niConsumer2.getMsgCount());
			
			etaMsg = niConsumer2.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REQUEST);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.LOGIN);
			
			etaLoginMsg = convertToLoginMsg(etaMsg);
			
			assertEquals(etaLoginMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType.REQUEST);
			etaLoginRequest = (com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest)etaLoginMsg;
			assertEquals(etaLoginRequest.role(), com.refinitiv.eta.rdm.Login.RoleTypes.PROV);
			
			etaMsg = niConsumer2.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.SOURCE);
			
			etaDirectoryMsg = convertToDirectoryRefreshMsg(etaMsg);
			assertEquals(etaDirectoryMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType.REFRESH);
			etaDirectoryRefresh = (com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh)etaDirectoryMsg;
			assertEquals(etaDirectoryRefresh.serviceList().size(), 2);
			
			FieldList fieldList = EmaFactory.createFieldList();
			fieldList.add( EmaFactory.createFieldEntry().real(22, 3990, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(25, 3994, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(30, 9,  OmmReal.MagnitudeType.EXPONENT_0));
			fieldList.add( EmaFactory.createFieldEntry().real(31, 19, OmmReal.MagnitudeType.EXPONENT_0));
			
			
			List<ChannelInformation> chnlInfo = new ArrayList<ChannelInformation>();

			// Check to see if all the channels are connected
			provider.sessionChannelInfo(chnlInfo);
			
			assertEquals(2, chnlInfo.size());
			
			assertEquals(chnlInfo.get(0).channelName(), "Channel_1");
			assertEquals(chnlInfo.get(1).channelName(), "Channel_2");


			NiProviderTestClient niProviderClient = new NiProviderTestClient();
			niProviderClient.provider(provider);
			
			provider.registerClient(EmaFactory.createReqMsg().name("RWFFld").filter(EmaRdm.DICTIONARY_NORMAL)
					.serviceName("TEST_NI_PUB").domainType(EmaRdm.MMT_DICTIONARY), niProviderClient);
			
			Thread.sleep(100);
			
			assertEquals(1, niConsumer1.getMsgCount());
			
			etaMsg = niConsumer1.getMessage();
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REQUEST);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.DICTIONARY);
			
			assertEquals(0, niConsumer2.getMsgCount());
			
			assertEquals(1, niProviderClient.queueSize());
			
			Msg dictMsg = niProviderClient.popMessage();
			assertEquals(DataTypes.REFRESH_MSG, dictMsg.dataType());
			
			RefreshMsg dictRefresh = (RefreshMsg)dictMsg;
			
			assertEquals(dictRefresh.name(), "RWFFld");
			assertEquals(dictRefresh.domainType(), com.refinitiv.eta.rdm.DomainTypes.DICTIONARY);
			assertTrue(dictRefresh.complete());
			
			niProviderClient.unregisterAllHandles();
			Thread.sleep(100);
			
			assertEquals(1, niConsumer1.getMsgCount());
			
			etaMsg = niConsumer1.getMessage();
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.CLOSE);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.DICTIONARY);
			
			assertEquals(0, niConsumer2.getMsgCount());
			
		} catch(Exception e)
		{
			e.printStackTrace();
			assertTrue( false);
		}
		
	}
	
	@Test
	public void NiProviderTwoConnectionSessionLoginDictionaryDownloadMergeTest()
	{
		TestUtilities.printTestHead("NiProviderTwoConnectionSessionDictionaryRequestDictionaryDownloadTest","");

		NiConsumerHelperConfig niConsConfig1 = new NiConsumerHelperConfig();
		NiConsumerHelperConfig niConsConfig2 = new NiConsumerHelperConfig();
		
		NiProviderTestClient loginClient = new NiProviderTestClient();
		
		niConsConfig1.port = "19001";
		niConsConfig2.port = "19002";

		// First test, both connections have ProviderDictionaryDownload turned on.
		try
		{
			niProvConfig = EmaFactory.createOmmNiProviderConfig(emaConfigFileLocation);
			niProvConfig.providerName("Provider_Session_1");
			niConsumer1 = new NiConsumerHelper(niConsConfig1);
			niConsumer2 = new NiConsumerHelper(niConsConfig2);
			
			provider = EmaFactory.createOmmProvider(niProvConfig, loginClient);
			assertNotNull(provider);
			
			// sleep to make sure that the consumer gets everything
			Thread.sleep(100);
			
			assertEquals(2, niConsumer1.getMsgCount());
			
			com.refinitiv.eta.codec.Msg etaMsg = niConsumer1.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REQUEST);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.LOGIN);
			
			com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsg etaLoginMsg = convertToLoginMsg(etaMsg);
			
			assertEquals(etaLoginMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType.REQUEST);
			com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest etaLoginRequest = (com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest)etaLoginMsg;
			assertEquals(etaLoginRequest.role(), com.refinitiv.eta.rdm.Login.RoleTypes.PROV);
			
			etaMsg = niConsumer1.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.SOURCE);
			
			com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsg etaDirectoryMsg = convertToDirectoryRefreshMsg(etaMsg);
			assertEquals(etaDirectoryMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType.REFRESH);
			com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh etaDirectoryRefresh = (com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh)etaDirectoryMsg;
			assertEquals(etaDirectoryRefresh.serviceList().size(), 2);
			
			assertEquals(2, niConsumer2.getMsgCount());
			
			etaMsg = niConsumer2.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REQUEST);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.LOGIN);
			
			etaLoginMsg = convertToLoginMsg(etaMsg);
			
			assertEquals(etaLoginMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType.REQUEST);
			etaLoginRequest = (com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest)etaLoginMsg;
			assertEquals(etaLoginRequest.role(), com.refinitiv.eta.rdm.Login.RoleTypes.PROV);
			
			etaMsg = niConsumer2.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.SOURCE);
			
			etaDirectoryMsg = convertToDirectoryRefreshMsg(etaMsg);
			assertEquals(etaDirectoryMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType.REFRESH);
			etaDirectoryRefresh = (com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh)etaDirectoryMsg;
			assertEquals(etaDirectoryRefresh.serviceList().size(), 2);
			
			FieldList fieldList = EmaFactory.createFieldList();
			fieldList.add( EmaFactory.createFieldEntry().real(22, 3990, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(25, 3994, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(30, 9,  OmmReal.MagnitudeType.EXPONENT_0));
			fieldList.add( EmaFactory.createFieldEntry().real(31, 19, OmmReal.MagnitudeType.EXPONENT_0));
			
			// Two status messages of OPEN/SUSPECT for the channels getting established, then a refresh message
			assertEquals(3, loginClient.queueSize());
			
			Msg emaMsg = loginClient.popMessage();
			assertEquals(DataTypes.STATUS_MSG, emaMsg.dataType());
			
			StatusMsg emaStatus = (StatusMsg)emaMsg;
			assertEquals(StreamState.OPEN, emaStatus.state().streamState());
			assertEquals(DataState.SUSPECT, emaStatus.state().dataState());
			
			emaMsg = loginClient.popMessage();
			assertEquals(DataTypes.STATUS_MSG, emaMsg.dataType());
			
			emaStatus = (StatusMsg)emaMsg;
			assertEquals(StreamState.OPEN, emaStatus.state().streamState());
			assertEquals(DataState.SUSPECT, emaStatus.state().dataState());
			
			emaMsg = loginClient.popMessage();
			assertEquals(DataTypes.REFRESH_MSG, emaMsg.dataType());
			
			RefreshMsg emaRefresh = (RefreshMsg)emaMsg;
			assertEquals(StreamState.OPEN, emaRefresh.state().streamState());
			assertEquals(DataState.OK, emaRefresh.state().dataState());
			assertEquals(DomainTypes.LOGIN, emaRefresh.domainType());
            assertTrue(emaRefresh.solicited());
            assertTrue(emaRefresh.complete());
            assertTrue(emaRefresh.hasMsgKey());
            assertEquals(DataType.DataTypes.NO_DATA, emaRefresh.payload().dataType());
            assertEquals(DataType.DataTypes.ELEMENT_LIST, emaRefresh.attrib().dataType());

            ElementList elementList = emaRefresh.attrib().elementList();
            for(ElementEntry element : elementList)
            {
            	System.out.println("element.name: " + element.name());
                switch(element.name())
                {
                    case EmaRdm.ENAME_SUPPORT_PROVIDER_DICTIONARY_DOWNLOAD:
                    {
                        assertEquals(1, element.uintValue());
                        break;
                    }
                    case EmaRdm.ENAME_APP_ID:
                    {
                        assertEquals("100", element.ascii().toString());
                        break;
                    }
                    case EmaRdm.ENAME_APP_NAME:
                    {
                        assertEquals("NiConsumer", element.ascii().toString());
                        break;
                    }
                    default:
                    {
                    	assertFalse(true);
                    }
                }
            }
			
			
			List<ChannelInformation> chnlInfo = new ArrayList<ChannelInformation>();

			// Check to see if all the channels are connected
			provider.sessionChannelInfo(chnlInfo);
			
			assertEquals(2, chnlInfo.size());
			
			assertEquals(chnlInfo.get(0).channelName(), "Channel_1");
			assertEquals(chnlInfo.get(1).channelName(), "Channel_2");


			NiProviderTestClient niProviderClient = new NiProviderTestClient();
			niProviderClient.provider(provider);
			
			provider.registerClient(EmaFactory.createReqMsg().name("RWFFld").filter(EmaRdm.DICTIONARY_NORMAL)
					.serviceName("TEST_NI_PUB").domainType(EmaRdm.MMT_DICTIONARY), niProviderClient);
			
			Thread.sleep(100);
			
			assertEquals(1, niConsumer1.getMsgCount());
			
			etaMsg = niConsumer1.getMessage();
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REQUEST);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.DICTIONARY);
			
			assertEquals(0, niConsumer2.getMsgCount());
			
			assertEquals(1, niProviderClient.queueSize());
			
			Msg dictMsg = niProviderClient.popMessage();
			assertEquals(DataTypes.REFRESH_MSG, dictMsg.dataType());
			
			RefreshMsg dictRefresh = (RefreshMsg)dictMsg;
			
			assertEquals(dictRefresh.name(), "RWFFld");
			assertEquals(dictRefresh.domainType(), com.refinitiv.eta.rdm.DomainTypes.DICTIONARY);
			assertTrue(dictRefresh.complete());
			
			niProviderClient.unregisterAllHandles();
			Thread.sleep(100);
			
			assertEquals(1, niConsumer1.getMsgCount());
			
			etaMsg = niConsumer1.getMessage();
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.CLOSE);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.DICTIONARY);
			
			assertEquals(0, niConsumer2.getMsgCount());
			
		} catch(Exception e)
		{
			e.printStackTrace();
			assertTrue( false);
		}
		finally
		{
			provider.uninitialize();
			provider = null;
			
			niConsumer1.shutdown();
			niConsumer1 = null;
			niConsumer2.shutdown();
			niConsumer2 = null;
		}
		
		assertEquals(0, loginClient.queueSize());
		
		niConsConfig2.loginRefresh.features().supportProviderDictionaryDownload(0);

		// Second test, only connection one has supportProviderDictionaryDownload turned on.
		
		try
		{
			niProvConfig = EmaFactory.createOmmNiProviderConfig(emaConfigFileLocation);
			niProvConfig.providerName("Provider_Session_1");
			niConsumer1 = new NiConsumerHelper(niConsConfig1);
			niConsumer2 = new NiConsumerHelper(niConsConfig2);
			
			provider = EmaFactory.createOmmProvider(niProvConfig, loginClient);
			assertNotNull(provider);
			
			// sleep to make sure that the consumer gets everything
			Thread.sleep(100);
			
			assertEquals(2, niConsumer1.getMsgCount());
			
			com.refinitiv.eta.codec.Msg etaMsg = niConsumer1.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REQUEST);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.LOGIN);
			
			com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsg etaLoginMsg = convertToLoginMsg(etaMsg);
			
			assertEquals(etaLoginMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType.REQUEST);
			com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest etaLoginRequest = (com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest)etaLoginMsg;
			assertEquals(etaLoginRequest.role(), com.refinitiv.eta.rdm.Login.RoleTypes.PROV);
			
			etaMsg = niConsumer1.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.SOURCE);
			
			com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsg etaDirectoryMsg = convertToDirectoryRefreshMsg(etaMsg);
			assertEquals(etaDirectoryMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType.REFRESH);
			com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh etaDirectoryRefresh = (com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh)etaDirectoryMsg;
			assertEquals(etaDirectoryRefresh.serviceList().size(), 2);
			
			assertEquals(2, niConsumer2.getMsgCount());
			
			etaMsg = niConsumer2.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REQUEST);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.LOGIN);
			
			etaLoginMsg = convertToLoginMsg(etaMsg);
			
			assertEquals(etaLoginMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType.REQUEST);
			etaLoginRequest = (com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest)etaLoginMsg;
			assertEquals(etaLoginRequest.role(), com.refinitiv.eta.rdm.Login.RoleTypes.PROV);
			
			etaMsg = niConsumer2.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.SOURCE);
			
			etaDirectoryMsg = convertToDirectoryRefreshMsg(etaMsg);
			assertEquals(etaDirectoryMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType.REFRESH);
			etaDirectoryRefresh = (com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh)etaDirectoryMsg;
			assertEquals(etaDirectoryRefresh.serviceList().size(), 2);
			
			FieldList fieldList = EmaFactory.createFieldList();
			fieldList.add( EmaFactory.createFieldEntry().real(22, 3990, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(25, 3994, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(30, 9,  OmmReal.MagnitudeType.EXPONENT_0));
			fieldList.add( EmaFactory.createFieldEntry().real(31, 19, OmmReal.MagnitudeType.EXPONENT_0));
			
			// Two status messages of OPEN/SUSPECT for the channels getting established, then a refresh message
			assertEquals(3, loginClient.queueSize());
			
			Msg emaMsg = loginClient.popMessage();
			assertEquals(DataTypes.STATUS_MSG, emaMsg.dataType());
			
			StatusMsg emaStatus = (StatusMsg)emaMsg;
			assertEquals(StreamState.OPEN, emaStatus.state().streamState());
			assertEquals(DataState.SUSPECT, emaStatus.state().dataState());
			
			emaMsg = loginClient.popMessage();
			assertEquals(DataTypes.STATUS_MSG, emaMsg.dataType());
			
			emaStatus = (StatusMsg)emaMsg;
			assertEquals(StreamState.OPEN, emaStatus.state().streamState());
			assertEquals(DataState.SUSPECT, emaStatus.state().dataState());
			
			emaMsg = loginClient.popMessage();
			assertEquals(DataTypes.REFRESH_MSG, emaMsg.dataType());
			
			RefreshMsg emaRefresh = (RefreshMsg)emaMsg;
			assertEquals(StreamState.OPEN, emaRefresh.state().streamState());
			assertEquals(DataState.OK, emaRefresh.state().dataState());
			assertEquals(DomainTypes.LOGIN, emaRefresh.domainType());
            assertTrue(emaRefresh.solicited());
            assertTrue(emaRefresh.complete());
            assertTrue(emaRefresh.hasMsgKey());
            assertEquals(DataType.DataTypes.NO_DATA, emaRefresh.payload().dataType());
            assertEquals(DataType.DataTypes.ELEMENT_LIST, emaRefresh.attrib().dataType());

            ElementList elementList = emaRefresh.attrib().elementList();
            for(ElementEntry element : elementList)
            {
            	System.out.println("element.name: " + element.name());
                switch(element.name())
                {
                    case EmaRdm.ENAME_SUPPORT_PROVIDER_DICTIONARY_DOWNLOAD:
                    {
                        assertEquals(1, element.uintValue());
                        break;
                    }
                    case EmaRdm.ENAME_APP_ID:
                    {
                        assertEquals("100", element.ascii().toString());
                        break;
                    }
                    case EmaRdm.ENAME_APP_NAME:
                    {
                        assertEquals("NiConsumer", element.ascii().toString());
                        break;
                    }
                    default:
                    {
                    	assertFalse(true);
                    }
                }
            }
			
			
			List<ChannelInformation> chnlInfo = new ArrayList<ChannelInformation>();

			// Check to see if all the channels are connected
			provider.sessionChannelInfo(chnlInfo);
			
			assertEquals(2, chnlInfo.size());
			
			assertEquals(chnlInfo.get(0).channelName(), "Channel_1");
			assertEquals(chnlInfo.get(1).channelName(), "Channel_2");


			NiProviderTestClient niProviderClient = new NiProviderTestClient();
			niProviderClient.provider(provider);
			
			provider.registerClient(EmaFactory.createReqMsg().name("RWFFld").filter(EmaRdm.DICTIONARY_NORMAL)
					.serviceName("TEST_NI_PUB").domainType(EmaRdm.MMT_DICTIONARY), niProviderClient);
			
			Thread.sleep(100);
			
			assertEquals(1, niConsumer1.getMsgCount());
			
			etaMsg = niConsumer1.getMessage();
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REQUEST);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.DICTIONARY);
			
			assertEquals(0, niConsumer2.getMsgCount());
			
			assertEquals(1, niProviderClient.queueSize());
			
			Msg dictMsg = niProviderClient.popMessage();
			assertEquals(DataTypes.REFRESH_MSG, dictMsg.dataType());
			
			RefreshMsg dictRefresh = (RefreshMsg)dictMsg;
			
			assertEquals(dictRefresh.name(), "RWFFld");
			assertEquals(dictRefresh.domainType(), com.refinitiv.eta.rdm.DomainTypes.DICTIONARY);
			assertTrue(dictRefresh.complete());
			
			niProviderClient.unregisterAllHandles();
			Thread.sleep(100);
			
			assertEquals(1, niConsumer1.getMsgCount());
			
			etaMsg = niConsumer1.getMessage();
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.CLOSE);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.DICTIONARY);
			
			assertEquals(0, niConsumer2.getMsgCount());
		
		} catch(Exception e)
		{
			e.printStackTrace();
			assertTrue( false);
		}
		finally
		{
			provider.uninitialize();
			provider = null;
			
			niConsumer1.shutdown();
			niConsumer1 = null;
			niConsumer2.shutdown();
			niConsumer2 = null;
		}
		
		assertEquals(0, loginClient.queueSize());
		
		niConsConfig1.loginRefresh.features().supportProviderDictionaryDownload(0);
		niConsConfig2.loginRefresh.features().supportProviderDictionaryDownload(1);

		// Third test, only connection twp has supportProviderDictionaryDownload turned on.
		try
		{
			niProvConfig = EmaFactory.createOmmNiProviderConfig(emaConfigFileLocation);
			niProvConfig.providerName("Provider_Session_1");
			
			niConsumer1 = new NiConsumerHelper(niConsConfig1);
			niConsumer2 = new NiConsumerHelper(niConsConfig2);
			
			provider = EmaFactory.createOmmProvider(niProvConfig, loginClient);
			assertNotNull(provider);
			
			// sleep to make sure that the consumer gets everything
			Thread.sleep(100);
			
			assertEquals(2, niConsumer1.getMsgCount());
			
			com.refinitiv.eta.codec.Msg etaMsg = niConsumer1.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REQUEST);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.LOGIN);
			
			com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsg etaLoginMsg = convertToLoginMsg(etaMsg);
			
			assertEquals(etaLoginMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType.REQUEST);
			com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest etaLoginRequest = (com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest)etaLoginMsg;
			assertEquals(etaLoginRequest.role(), com.refinitiv.eta.rdm.Login.RoleTypes.PROV);
			
			etaMsg = niConsumer1.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.SOURCE);
			
			com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsg etaDirectoryMsg = convertToDirectoryRefreshMsg(etaMsg);
			assertEquals(etaDirectoryMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType.REFRESH);
			com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh etaDirectoryRefresh = (com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh)etaDirectoryMsg;
			assertEquals(etaDirectoryRefresh.serviceList().size(), 2);
			
			assertEquals(2, niConsumer2.getMsgCount());
			
			etaMsg = niConsumer2.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REQUEST);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.LOGIN);
			
			etaLoginMsg = convertToLoginMsg(etaMsg);
			
			assertEquals(etaLoginMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType.REQUEST);
			etaLoginRequest = (com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest)etaLoginMsg;
			assertEquals(etaLoginRequest.role(), com.refinitiv.eta.rdm.Login.RoleTypes.PROV);
			
			etaMsg = niConsumer2.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.SOURCE);
			
			etaDirectoryMsg = convertToDirectoryRefreshMsg(etaMsg);
			assertEquals(etaDirectoryMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType.REFRESH);
			etaDirectoryRefresh = (com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh)etaDirectoryMsg;
			assertEquals(etaDirectoryRefresh.serviceList().size(), 2);
			
			FieldList fieldList = EmaFactory.createFieldList();
			fieldList.add( EmaFactory.createFieldEntry().real(22, 3990, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(25, 3994, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(30, 9,  OmmReal.MagnitudeType.EXPONENT_0));
			fieldList.add( EmaFactory.createFieldEntry().real(31, 19, OmmReal.MagnitudeType.EXPONENT_0));
			
			// Two status messages of OPEN/SUSPECT for the channels getting established, then a refresh message
			assertEquals(3, loginClient.queueSize());
			
			Msg emaMsg = loginClient.popMessage();
			assertEquals(DataTypes.STATUS_MSG, emaMsg.dataType());
			
			StatusMsg emaStatus = (StatusMsg)emaMsg;
			assertEquals(StreamState.OPEN, emaStatus.state().streamState());
			assertEquals(DataState.SUSPECT, emaStatus.state().dataState());
			
			emaMsg = loginClient.popMessage();
			assertEquals(DataTypes.STATUS_MSG, emaMsg.dataType());
			
			emaStatus = (StatusMsg)emaMsg;
			assertEquals(StreamState.OPEN, emaStatus.state().streamState());
			assertEquals(DataState.SUSPECT, emaStatus.state().dataState());
			
			emaMsg = loginClient.popMessage();
			assertEquals(DataTypes.REFRESH_MSG, emaMsg.dataType());
			
			RefreshMsg emaRefresh = (RefreshMsg)emaMsg;
			assertEquals(StreamState.OPEN, emaRefresh.state().streamState());
			assertEquals(DataState.OK, emaRefresh.state().dataState());
			assertEquals(DomainTypes.LOGIN, emaRefresh.domainType());
            assertTrue(emaRefresh.solicited());
            assertTrue(emaRefresh.complete());
            assertTrue(emaRefresh.hasMsgKey());
            assertEquals(DataType.DataTypes.NO_DATA, emaRefresh.payload().dataType());
            assertEquals(DataType.DataTypes.ELEMENT_LIST, emaRefresh.attrib().dataType());

            ElementList elementList = emaRefresh.attrib().elementList();
            for(ElementEntry element : elementList)
            {
            	System.out.println("element.name: " + element.name());
                switch(element.name())
                {
                    case EmaRdm.ENAME_SUPPORT_PROVIDER_DICTIONARY_DOWNLOAD:
                    {
                        assertEquals(1, element.uintValue());
                        break;
                    }
                    case EmaRdm.ENAME_APP_ID:
                    {
                        assertEquals("100", element.ascii().toString());
                        break;
                    }
                    case EmaRdm.ENAME_APP_NAME:
                    {
                        assertEquals("NiConsumer", element.ascii().toString());
                        break;
                    }
                    default:
                    {
                    	assertFalse(true);
                    }
                }
            }
			
			
			List<ChannelInformation> chnlInfo = new ArrayList<ChannelInformation>();

			// Check to see if all the channels are connected
			provider.sessionChannelInfo(chnlInfo);
			
			assertEquals(2, chnlInfo.size());
			
			assertEquals(chnlInfo.get(0).channelName(), "Channel_1");
			assertEquals(chnlInfo.get(1).channelName(), "Channel_2");


			NiProviderTestClient niProviderClient = new NiProviderTestClient();
			niProviderClient.provider(provider);
			
			provider.registerClient(EmaFactory.createReqMsg().name("RWFFld").filter(EmaRdm.DICTIONARY_NORMAL)
					.serviceName("TEST_NI_PUB").domainType(EmaRdm.MMT_DICTIONARY), niProviderClient);
			
			Thread.sleep(100);
			
			assertEquals(0, niConsumer1.getMsgCount());
			
			assertEquals(1, niConsumer2.getMsgCount());
			etaMsg = niConsumer2.getMessage();
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REQUEST);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.DICTIONARY);
			
			assertEquals(1, niProviderClient.queueSize());
			
			Msg dictMsg = niProviderClient.popMessage();
			assertEquals(DataTypes.REFRESH_MSG, dictMsg.dataType());
			
			RefreshMsg dictRefresh = (RefreshMsg)dictMsg;
			
			assertEquals(dictRefresh.name(), "RWFFld");
			assertEquals(dictRefresh.domainType(), com.refinitiv.eta.rdm.DomainTypes.DICTIONARY);
			assertTrue(dictRefresh.complete());
			
			niProviderClient.unregisterAllHandles();
			Thread.sleep(100);
			
			assertEquals(0, niConsumer1.getMsgCount());
			
			assertEquals(1, niConsumer2.getMsgCount());
			
			etaMsg = niConsumer2.getMessage();
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.CLOSE);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.DICTIONARY);			
		} catch(Exception e)
		{
			e.printStackTrace();
			assertTrue( false);
		}
		finally
		{
			provider.uninitialize();
			provider = null;
			
			niConsumer1.shutdown();
			niConsumer1 = null;
			niConsumer2.shutdown();
			niConsumer2 = null;
		}
		
		assertEquals(0, loginClient.queueSize());
		
		niConsConfig1.loginRefresh.features().supportProviderDictionaryDownload(0);
		niConsConfig2.loginRefresh.features().supportProviderDictionaryDownload(0);

		// Fourth test, neither connection has supportProviderDictionaryDownload turned on.
		
		try
		{
			niProvConfig = EmaFactory.createOmmNiProviderConfig(emaConfigFileLocation);
			niProvConfig.providerName("Provider_Session_1");
			
			niConsumer1 = new NiConsumerHelper(niConsConfig1);
			niConsumer2 = new NiConsumerHelper(niConsConfig2);
			
			provider = EmaFactory.createOmmProvider(niProvConfig, loginClient);
			assertNotNull(provider);
			
			// sleep to make sure that the consumer gets everything
			Thread.sleep(100);
			
			assertEquals(2, niConsumer1.getMsgCount());
			
			com.refinitiv.eta.codec.Msg etaMsg = niConsumer1.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REQUEST);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.LOGIN);
			
			com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsg etaLoginMsg = convertToLoginMsg(etaMsg);
			
			assertEquals(etaLoginMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType.REQUEST);
			com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest etaLoginRequest = (com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest)etaLoginMsg;
			assertEquals(etaLoginRequest.role(), com.refinitiv.eta.rdm.Login.RoleTypes.PROV);
			
			etaMsg = niConsumer1.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.SOURCE);
			
			com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsg etaDirectoryMsg = convertToDirectoryRefreshMsg(etaMsg);
			assertEquals(etaDirectoryMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType.REFRESH);
			com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh etaDirectoryRefresh = (com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh)etaDirectoryMsg;
			assertEquals(etaDirectoryRefresh.serviceList().size(), 2);
			
			assertEquals(2, niConsumer2.getMsgCount());
			
			etaMsg = niConsumer2.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REQUEST);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.LOGIN);
			
			etaLoginMsg = convertToLoginMsg(etaMsg);
			
			assertEquals(etaLoginMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType.REQUEST);
			etaLoginRequest = (com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest)etaLoginMsg;
			assertEquals(etaLoginRequest.role(), com.refinitiv.eta.rdm.Login.RoleTypes.PROV);
			
			etaMsg = niConsumer2.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.SOURCE);
			
			etaDirectoryMsg = convertToDirectoryRefreshMsg(etaMsg);
			assertEquals(etaDirectoryMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType.REFRESH);
			etaDirectoryRefresh = (com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh)etaDirectoryMsg;
			assertEquals(etaDirectoryRefresh.serviceList().size(), 2);
			
			FieldList fieldList = EmaFactory.createFieldList();
			fieldList.add( EmaFactory.createFieldEntry().real(22, 3990, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(25, 3994, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(30, 9,  OmmReal.MagnitudeType.EXPONENT_0));
			fieldList.add( EmaFactory.createFieldEntry().real(31, 19, OmmReal.MagnitudeType.EXPONENT_0));
			
			// Two status messages of OPEN/SUSPECT for the channels getting established, then a refresh message
			assertEquals(3, loginClient.queueSize());
			
			Msg emaMsg = loginClient.popMessage();
			assertEquals(DataTypes.STATUS_MSG, emaMsg.dataType());
			
			StatusMsg emaStatus = (StatusMsg)emaMsg;
			assertEquals(StreamState.OPEN, emaStatus.state().streamState());
			assertEquals(DataState.SUSPECT, emaStatus.state().dataState());
			
			emaMsg = loginClient.popMessage();
			assertEquals(DataTypes.STATUS_MSG, emaMsg.dataType());
			
			emaStatus = (StatusMsg)emaMsg;
			assertEquals(StreamState.OPEN, emaStatus.state().streamState());
			assertEquals(DataState.SUSPECT, emaStatus.state().dataState());
			
			emaMsg = loginClient.popMessage();
			assertEquals(DataTypes.REFRESH_MSG, emaMsg.dataType());
			
			RefreshMsg emaRefresh = (RefreshMsg)emaMsg;
			assertEquals(StreamState.OPEN, emaRefresh.state().streamState());
			assertEquals(DataState.OK, emaRefresh.state().dataState());
			assertEquals(DomainTypes.LOGIN, emaRefresh.domainType());
            assertTrue(emaRefresh.solicited());
            assertTrue(emaRefresh.complete());
            assertTrue(emaRefresh.hasMsgKey());
            assertEquals(DataType.DataTypes.NO_DATA, emaRefresh.payload().dataType());
            assertEquals(DataType.DataTypes.ELEMENT_LIST, emaRefresh.attrib().dataType());

            ElementList elementList = emaRefresh.attrib().elementList();
            for(ElementEntry element : elementList)
            {
            	System.out.println("element.name: " + element.name());
                switch(element.name())
                {
                    case EmaRdm.ENAME_SUPPORT_PROVIDER_DICTIONARY_DOWNLOAD:
                    {
                        assertEquals(0, element.uintValue());
                        break;
                    }
                    case EmaRdm.ENAME_APP_ID:
                    {
                        assertEquals("100", element.ascii().toString());
                        break;
                    }
                    case EmaRdm.ENAME_APP_NAME:
                    {
                        assertEquals("NiConsumer", element.ascii().toString());
                        break;
                    }
                    default:
                    {
                    	assertFalse(true);
                    }
                }
            }
			
			
			List<ChannelInformation> chnlInfo = new ArrayList<ChannelInformation>();

			// Check to see if all the channels are connected
			provider.sessionChannelInfo(chnlInfo);
			
			assertEquals(2, chnlInfo.size());
			
			assertEquals(chnlInfo.get(0).channelName(), "Channel_1");
			assertEquals(chnlInfo.get(1).channelName(), "Channel_2");


			NiProviderTestClient niProviderClient = new NiProviderTestClient();
			niProviderClient.provider(provider);
			
			try
			{
				provider.registerClient(EmaFactory.createReqMsg().name("RWFFld").filter(EmaRdm.DICTIONARY_NORMAL)
					.serviceName("TEST_NI_PUB").domainType(EmaRdm.MMT_DICTIONARY), niProviderClient);
				assertFalse(true);
			}
			catch(OmmInvalidUsageException e)
			{
				assertEquals(OmmInvalidUsageException.ErrorCode.INVALID_OPERATION, e.errorCode());
			}
			
			Thread.sleep(100);
			
			assertEquals(0, niConsumer1.getMsgCount());
			assertEquals(0, niConsumer2.getMsgCount());
		} catch(Exception e)
		{
			e.printStackTrace();
			assertTrue( false);
		}
	}
	
	@Test
	public void NiProviderTwoConnectionSessionDirectoryTestRecover()
	{
		TestUtilities.printTestHead("NiProviderTwoConnectionSessionDirectoryTestRecover","");

		NiConsumerHelperConfig niConsConfig1 = new NiConsumerHelperConfig();
		NiConsumerHelperConfig niConsConfig2 = new NiConsumerHelperConfig();

		long itemHandle = 10;
		
		niConsConfig1.port = "19001";
		niConsConfig2.port = "19002";

		try
		{
			niProvConfig = EmaFactory.createOmmNiProviderConfig(emaConfigFileLocation);
			niProvConfig.providerName("Provider_Session_1");
			niProvConfig.adminControlDirectory(OmmNiProviderConfig.AdminControl.USER_CONTROL);
			niConsumer1 = new NiConsumerHelper(niConsConfig1);
			niConsumer2 = new NiConsumerHelper(niConsConfig2);
			
			provider = EmaFactory.createOmmProvider(niProvConfig);
			assertNotNull(provider);
			
			// sleep to make sure that the consumer gets everything
			Thread.sleep(100);
			
			assertEquals(1, niConsumer1.getMsgCount());
			
			com.refinitiv.eta.codec.Msg etaMsg = niConsumer1.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REQUEST);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.LOGIN);
			
			com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsg etaLoginMsg = convertToLoginMsg(etaMsg);
			
			assertEquals(etaLoginMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType.REQUEST);
			com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest etaLoginRequest = (com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest)etaLoginMsg;
			assertEquals(etaLoginRequest.role(), com.refinitiv.eta.rdm.Login.RoleTypes.PROV);
			
			List<ChannelInformation> chnlInfo = new ArrayList<ChannelInformation>();

			// Check to see if all the channels are connected
			provider.sessionChannelInfo(chnlInfo);
			
			assertEquals(2, chnlInfo.size());
			
			assertEquals(chnlInfo.get(0).channelName(), "Channel_1");
			assertEquals(chnlInfo.get(1).channelName(), "Channel_2");
			
			assertEquals(1, niConsumer2.getMsgCount());
			
			etaMsg = niConsumer2.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REQUEST);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.LOGIN);
			
			etaLoginMsg = convertToLoginMsg(etaMsg);
			
			assertEquals(etaLoginMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType.REQUEST);
			etaLoginRequest = (com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest)etaLoginMsg;
			assertEquals(etaLoginRequest.role(), com.refinitiv.eta.rdm.Login.RoleTypes.PROV);
			
			FieldList fieldList = EmaFactory.createFieldList();
			fieldList.add( EmaFactory.createFieldEntry().real(22, 3990, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(25, 3994, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(30, 9,  OmmReal.MagnitudeType.EXPONENT_0));
			fieldList.add( EmaFactory.createFieldEntry().real(31, 19, OmmReal.MagnitudeType.EXPONENT_0));
			
			// Attempt to submit a refresh with NI_PUB_1 before submitting the directory.
			try
			{
				provider.submit( EmaFactory.createRefreshMsg().serviceName("NI_PUB_1").name("IBM.N")
						.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
						.payload(fieldList).complete(true), itemHandle);
				assertTrue(false);
			} catch (OmmInvalidUsageException except)
			{
				System.out.println("Exception thrown on not available NI_PUB_1 service name for refresh msg.");
			}
			
			long sourceDirectoryHandle = 1;
			
            OmmArray capablities = EmaFactory.createOmmArray();
            capablities.add(EmaFactory.createOmmArrayEntry().uintValue( EmaRdm.MMT_MARKET_PRICE));
            capablities.add(EmaFactory.createOmmArrayEntry().uintValue( EmaRdm.MMT_MARKET_BY_PRICE));
            OmmArray dictionaryUsed = EmaFactory.createOmmArray();
            dictionaryUsed.add(EmaFactory.createOmmArrayEntry().ascii( "RWFFld"));
            dictionaryUsed.add(EmaFactory.createOmmArrayEntry().ascii( "RWFEnum"));
            
            ElementList serviceInfoId = EmaFactory.createElementList();    
            
            serviceInfoId.add( EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_NAME, "NI_PUB_1"));     
            serviceInfoId.add( EmaFactory.createElementEntry().array(EmaRdm.ENAME_CAPABILITIES, capablities));         
            serviceInfoId.add( EmaFactory.createElementEntry().array(EmaRdm.ENAME_DICTIONARYS_USED, dictionaryUsed));

            ElementList serviceStateId = EmaFactory.createElementList();
            serviceStateId.add( EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_UP));
				
            FilterList filterList = EmaFactory.createFilterList();
            filterList.add( EmaFactory.createFilterEntry().elementList(EmaRdm.SERVICE_INFO_ID, FilterEntry.FilterAction.SET, serviceInfoId) );
            filterList.add( EmaFactory.createFilterEntry().elementList(EmaRdm.SERVICE_STATE_ID, FilterEntry.FilterAction.SET, serviceStateId));
            
            Map map = EmaFactory.createMap();
            map.add( EmaFactory.createMapEntry().keyUInt(2, MapEntry.MapAction.ADD, filterList));
            
            RefreshMsg refreshMsg = EmaFactory.createRefreshMsg();
            provider.submit( refreshMsg.domainType(EmaRdm.MMT_DIRECTORY).filter( EmaRdm.SERVICE_INFO_FILTER | EmaRdm.SERVICE_STATE_FILTER).payload(map)
                    ,sourceDirectoryHandle );
            
            Thread.sleep(500);
            
            assertEquals(1, niConsumer1.getMsgCount());
			
			etaMsg = niConsumer1.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.SOURCE);
			
			com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsg etaDirectoryMsg = convertToDirectoryRefreshMsg(etaMsg);
			assertEquals(etaDirectoryMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType.REFRESH);
			com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh etaDirectoryRefresh = (com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh)etaDirectoryMsg;
			assertEquals(etaDirectoryRefresh.serviceList().size(), 1);
			com.refinitiv.eta.valueadd.domainrep.rdm.directory.Service service = etaDirectoryRefresh.serviceList().get(0);
			assertEquals(service.info().serviceName().toString(), "NI_PUB_1");
			assertEquals(service.state().serviceState(), EmaRdm.SERVICE_UP);
			
			 assertEquals(1, niConsumer2.getMsgCount());
				
			etaMsg = niConsumer2.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.SOURCE);
			
			etaDirectoryMsg = convertToDirectoryRefreshMsg(etaMsg);
			assertEquals(etaDirectoryMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType.REFRESH);
			etaDirectoryRefresh = (com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh)etaDirectoryMsg;
			assertEquals(etaDirectoryRefresh.serviceList().size(), 1);
			service = etaDirectoryRefresh.serviceList().get(0);
			assertEquals(service.info().serviceName().toString(), "NI_PUB_1");
			assertEquals(service.state().serviceState(), EmaRdm.SERVICE_UP);
			
			provider.submit( EmaFactory.createRefreshMsg().serviceName("NI_PUB_1").name("IBM.N")
					.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
					.payload(fieldList).complete(true), itemHandle);
			
			Thread.sleep(100);
			
			assertEquals(1, niConsumer1.getMsgCount());
			
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			assertEquals(1, niConsumer2.getMsgCount());
			
			etaMsg = niConsumer2.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			

			// Shutdown both consumers
			niConsumer1.closeChannel();
			Thread.sleep(150);
			
			provider.submit( EmaFactory.createRefreshMsg().serviceName("NI_PUB_1").name("IBM.N")
					.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
					.payload(fieldList).complete(true), itemHandle);
			
			Thread.sleep(100);
			
			assertEquals(0, niConsumer1.getMsgCount());
			
			assertEquals(1, niConsumer2.getMsgCount());
			etaMsg = niConsumer2.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			Thread.sleep(1500);
			
			assertEquals(2, niConsumer1.getMsgCount());
			
			etaMsg = niConsumer1.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REQUEST);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.LOGIN);
			
			etaLoginMsg = convertToLoginMsg(etaMsg);
			
			assertEquals(etaLoginMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType.REQUEST);
			etaLoginRequest = (com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest)etaLoginMsg;
			assertEquals(etaLoginRequest.role(), com.refinitiv.eta.rdm.Login.RoleTypes.PROV);
			
			etaMsg = niConsumer1.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.SOURCE);
			
			etaDirectoryMsg = convertToDirectoryRefreshMsg(etaMsg);
			assertEquals(etaDirectoryMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType.REFRESH);
			etaDirectoryRefresh = (com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh)etaDirectoryMsg;
			assertEquals(etaDirectoryRefresh.serviceList().size(), 1);
			service = etaDirectoryRefresh.serviceList().get(0);
			assertEquals(service.info().serviceName().toString(), "NI_PUB_1");
			assertEquals(service.state().serviceState(), EmaRdm.SERVICE_UP);
			
			assertEquals(0, niConsumer2.getMsgCount());
	
			niConsumer1.closeChannel();
			niConsumer2.closeChannel();
			Thread.sleep(150);
			
			try
			{
				provider.submit( EmaFactory.createRefreshMsg().serviceName("NI_PUB_1").name("IBM.N")
						.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
						.payload(fieldList).complete(true), itemHandle);
				assertTrue(false);
			} catch (OmmInvalidUsageException except)
			{
				System.out.println("Exception thrown on not available NI_PUB_1 service name for refresh msg.");
			}
			
			Thread.sleep(1500);
			
			assertEquals(2, niConsumer1.getMsgCount());
			
			etaMsg = niConsumer1.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REQUEST);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.LOGIN);
			
			etaLoginMsg = convertToLoginMsg(etaMsg);
			
			assertEquals(etaLoginMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType.REQUEST);
			etaLoginRequest = (com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest)etaLoginMsg;
			assertEquals(etaLoginRequest.role(), com.refinitiv.eta.rdm.Login.RoleTypes.PROV);
			
			etaMsg = niConsumer1.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.SOURCE);
			
			etaDirectoryMsg = convertToDirectoryRefreshMsg(etaMsg);
			assertEquals(etaDirectoryMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType.REFRESH);
			etaDirectoryRefresh = (com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh)etaDirectoryMsg;
			assertEquals(etaDirectoryRefresh.serviceList().size(), 1);
			service = etaDirectoryRefresh.serviceList().get(0);
			assertEquals(service.info().serviceName().toString(), "NI_PUB_1");
			assertEquals(service.state().serviceState(), EmaRdm.SERVICE_UP);
			
			
			assertEquals(2, niConsumer2.getMsgCount());
			
			etaMsg = niConsumer2.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REQUEST);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.LOGIN);
			
			etaLoginMsg = convertToLoginMsg(etaMsg);
			
			assertEquals(etaLoginMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType.REQUEST);
			etaLoginRequest = (com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest)etaLoginMsg;
			assertEquals(etaLoginRequest.role(), com.refinitiv.eta.rdm.Login.RoleTypes.PROV);
			
			etaMsg = niConsumer2.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.SOURCE);
			
			etaDirectoryMsg = convertToDirectoryRefreshMsg(etaMsg);
			assertEquals(etaDirectoryMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType.REFRESH);
			etaDirectoryRefresh = (com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh)etaDirectoryMsg;
			assertEquals(etaDirectoryRefresh.serviceList().size(), 1);
			service = etaDirectoryRefresh.serviceList().get(0);
			assertEquals(service.info().serviceName().toString(), "NI_PUB_1");
			assertEquals(service.state().serviceState(), EmaRdm.SERVICE_UP);
			
			provider.submit( EmaFactory.createRefreshMsg().serviceName("NI_PUB_1").name("IBM.N")
					.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
					.payload(fieldList).complete(true), itemHandle);
			
			Thread.sleep(500);
			
			assertEquals(1, niConsumer1.getMsgCount());
			
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			assertEquals(1, niConsumer2.getMsgCount());
			
			etaMsg = niConsumer2.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
		} catch(Exception e)
		{
			e.printStackTrace();
			assertTrue( false);
		}
		
	}
	
	@Test
	public void NiProviderTwoConnectionSessionDirectoryUpdateTestRecover()
	{
		TestUtilities.printTestHead("NiProviderTwoConnectionSessionDirectoryTestRecover","");

		NiConsumerHelperConfig niConsConfig1 = new NiConsumerHelperConfig();
		NiConsumerHelperConfig niConsConfig2 = new NiConsumerHelperConfig();

		long itemHandle = 10;
		long itemHandle_2 = 12;
		
		niConsConfig1.port = "19001";
		niConsConfig2.port = "19002";

		try
		{
			niProvConfig = EmaFactory.createOmmNiProviderConfig(emaConfigFileLocation);
			niProvConfig.providerName("Provider_Session_1");
			niProvConfig.adminControlDirectory(OmmNiProviderConfig.AdminControl.USER_CONTROL);
			niConsumer1 = new NiConsumerHelper(niConsConfig1);
			niConsumer2 = new NiConsumerHelper(niConsConfig2);
			
			provider = EmaFactory.createOmmProvider(niProvConfig);
			assertNotNull(provider);
			
			// sleep to make sure that the consumer gets everything
			Thread.sleep(100);
			
			assertEquals(1, niConsumer1.getMsgCount());
			
			com.refinitiv.eta.codec.Msg etaMsg = niConsumer1.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REQUEST);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.LOGIN);
			
			com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsg etaLoginMsg = convertToLoginMsg(etaMsg);
			
			assertEquals(etaLoginMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType.REQUEST);
			com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest etaLoginRequest = (com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest)etaLoginMsg;
			assertEquals(etaLoginRequest.role(), com.refinitiv.eta.rdm.Login.RoleTypes.PROV);
			
			List<ChannelInformation> chnlInfo = new ArrayList<ChannelInformation>();

			// Check to see if all the channels are connected
			provider.sessionChannelInfo(chnlInfo);
			
			assertEquals(2, chnlInfo.size());
			
			assertEquals(chnlInfo.get(0).channelName(), "Channel_1");
			assertEquals(chnlInfo.get(1).channelName(), "Channel_2");
			
			assertEquals(1, niConsumer2.getMsgCount());
			
			etaMsg = niConsumer2.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REQUEST);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.LOGIN);
			
			etaLoginMsg = convertToLoginMsg(etaMsg);
			
			assertEquals(etaLoginMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType.REQUEST);
			etaLoginRequest = (com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest)etaLoginMsg;
			assertEquals(etaLoginRequest.role(), com.refinitiv.eta.rdm.Login.RoleTypes.PROV);
			
			FieldList fieldList = EmaFactory.createFieldList();
			fieldList.add( EmaFactory.createFieldEntry().real(22, 3990, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(25, 3994, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add( EmaFactory.createFieldEntry().real(30, 9,  OmmReal.MagnitudeType.EXPONENT_0));
			fieldList.add( EmaFactory.createFieldEntry().real(31, 19, OmmReal.MagnitudeType.EXPONENT_0));
			
			// Attempt to submit a refresh with NI_PUB_1 before submitting the directory.
			try
			{
				provider.submit( EmaFactory.createRefreshMsg().serviceName("NI_PUB_1").name("IBM.N")
						.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
						.payload(fieldList).complete(true), itemHandle);
				assertTrue(false);
			} catch (OmmInvalidUsageException except)
			{
				System.out.println("Exception thrown on not available NI_PUB_1 service name for refresh msg.");
			}
			
			long sourceDirectoryHandle = 1;
			
            OmmArray capablities = EmaFactory.createOmmArray();
            capablities.add(EmaFactory.createOmmArrayEntry().uintValue( EmaRdm.MMT_MARKET_PRICE));
            capablities.add(EmaFactory.createOmmArrayEntry().uintValue( EmaRdm.MMT_MARKET_BY_PRICE));
            OmmArray dictionaryUsed = EmaFactory.createOmmArray();
            dictionaryUsed.add(EmaFactory.createOmmArrayEntry().ascii( "RWFFld"));
            dictionaryUsed.add(EmaFactory.createOmmArrayEntry().ascii( "RWFEnum"));
            
            ElementList serviceInfoId = EmaFactory.createElementList();    
            
            serviceInfoId.add( EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_NAME, "NI_PUB_1"));     
            serviceInfoId.add( EmaFactory.createElementEntry().array(EmaRdm.ENAME_CAPABILITIES, capablities));         
            serviceInfoId.add( EmaFactory.createElementEntry().array(EmaRdm.ENAME_DICTIONARYS_USED, dictionaryUsed));

            ElementList serviceStateId = EmaFactory.createElementList();
            serviceStateId.add( EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_UP));
				
            FilterList filterList = EmaFactory.createFilterList();
            filterList.add( EmaFactory.createFilterEntry().elementList(EmaRdm.SERVICE_INFO_ID, FilterEntry.FilterAction.SET, serviceInfoId) );
            filterList.add( EmaFactory.createFilterEntry().elementList(EmaRdm.SERVICE_STATE_ID, FilterEntry.FilterAction.SET, serviceStateId));
            
            Map map = EmaFactory.createMap();
            map.add( EmaFactory.createMapEntry().keyUInt(2, MapEntry.MapAction.ADD, filterList));
            
            capablities.clear();
            capablities.add(EmaFactory.createOmmArrayEntry().uintValue( EmaRdm.MMT_MARKET_PRICE));
            capablities.add(EmaFactory.createOmmArrayEntry().uintValue( EmaRdm.MMT_MARKET_BY_PRICE));
            dictionaryUsed.clear();
            dictionaryUsed.add(EmaFactory.createOmmArrayEntry().ascii( "RWFFld"));
            dictionaryUsed.add(EmaFactory.createOmmArrayEntry().ascii( "RWFEnum"));
            
            serviceInfoId.clear();
            
            serviceInfoId.add( EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_NAME, "NI_PUB_2"));     
            serviceInfoId.add( EmaFactory.createElementEntry().array(EmaRdm.ENAME_CAPABILITIES, capablities));         
            serviceInfoId.add( EmaFactory.createElementEntry().array(EmaRdm.ENAME_DICTIONARYS_USED, dictionaryUsed));

            serviceStateId.clear();
            serviceStateId.add( EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_UP));
				
            filterList.clear();
            filterList.add( EmaFactory.createFilterEntry().elementList(EmaRdm.SERVICE_INFO_ID, FilterEntry.FilterAction.SET, serviceInfoId) );
            filterList.add( EmaFactory.createFilterEntry().elementList(EmaRdm.SERVICE_STATE_ID, FilterEntry.FilterAction.SET, serviceStateId));
            
            map.add( EmaFactory.createMapEntry().keyUInt(3, MapEntry.MapAction.ADD, filterList));
            
            RefreshMsg refreshMsg = EmaFactory.createRefreshMsg();
            provider.submit( refreshMsg.domainType(EmaRdm.MMT_DIRECTORY).filter( EmaRdm.SERVICE_INFO_FILTER | EmaRdm.SERVICE_STATE_FILTER).payload(map)
                    ,sourceDirectoryHandle );
            
            Thread.sleep(500);
            
            assertEquals(1, niConsumer1.getMsgCount());
			
			etaMsg = niConsumer1.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.SOURCE);
			
			com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsg etaDirectoryMsg = convertToDirectoryRefreshMsg(etaMsg);
			assertEquals(etaDirectoryMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType.REFRESH);
			com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh etaDirectoryRefresh = (com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh)etaDirectoryMsg;
			assertEquals(etaDirectoryRefresh.serviceList().size(), 2);
			com.refinitiv.eta.valueadd.domainrep.rdm.directory.Service service = etaDirectoryRefresh.serviceList().get(0);
			assertEquals(service.info().serviceName().toString(), "NI_PUB_1");
			assertEquals(service.state().serviceState(), EmaRdm.SERVICE_UP);
			service = etaDirectoryRefresh.serviceList().get(1);
			assertEquals(service.info().serviceName().toString(), "NI_PUB_2");
			assertEquals(service.state().serviceState(), EmaRdm.SERVICE_UP);
			
			assertEquals(1, niConsumer2.getMsgCount());
				
			etaMsg = niConsumer2.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.SOURCE);
			
			etaDirectoryMsg = convertToDirectoryRefreshMsg(etaMsg);
			assertEquals(etaDirectoryMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType.REFRESH);
			etaDirectoryRefresh = (com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh)etaDirectoryMsg;
			assertEquals(etaDirectoryRefresh.serviceList().size(), 2);
			service = etaDirectoryRefresh.serviceList().get(0);
			assertEquals(service.info().serviceName().toString(), "NI_PUB_1");
			assertEquals(service.state().serviceState(), EmaRdm.SERVICE_UP);
			service = etaDirectoryRefresh.serviceList().get(1);
			assertEquals(service.info().serviceName().toString(), "NI_PUB_2");
			assertEquals(service.state().serviceState(), EmaRdm.SERVICE_UP);
			
			provider.submit( EmaFactory.createRefreshMsg().serviceName("NI_PUB_1").name("IBM.N")
					.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
					.payload(fieldList).complete(true), itemHandle);
			
			Thread.sleep(100);
			
			assertEquals(1, niConsumer1.getMsgCount());
			
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			assertEquals(1, niConsumer2.getMsgCount());
			
			etaMsg = niConsumer2.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.msgKey().name().toString(), "IBM.N");
			
			provider.submit( EmaFactory.createRefreshMsg().serviceName("NI_PUB_2").name("TEST_2")
					.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
					.payload(fieldList).complete(true), itemHandle);
			
			Thread.sleep(100);
			
			assertEquals(1, niConsumer1.getMsgCount());
			
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.msgKey().name().toString(), "TEST_2");
			
			assertEquals(1, niConsumer2.getMsgCount());
			
			etaMsg = niConsumer2.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.msgKey().name().toString(), "TEST_2");
			
			// Send update with delete action
			map.clear();
			filterList.clear();
            map.add( EmaFactory.createMapEntry().keyUInt(2, MapEntry.MapAction.DELETE, filterList));
            
            UpdateMsg updateMsg = EmaFactory.createUpdateMsg();
            provider.submit( updateMsg.domainType(EmaRdm.MMT_DIRECTORY).filter( EmaRdm.SERVICE_STATE_FILTER).payload(map)
                    ,sourceDirectoryHandle );
            
            Thread.sleep(500);
            
            assertEquals(1, niConsumer1.getMsgCount());
			
			etaMsg = niConsumer1.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.UPDATE);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.SOURCE);
			
			etaDirectoryMsg = convertToDirectoryUpdateMsg(etaMsg);
			assertEquals(etaDirectoryMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType.UPDATE);
			com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryUpdate etaDirectoryUpdate = (com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryUpdate)etaDirectoryMsg;
			assertEquals(etaDirectoryUpdate.serviceList().size(), 1);
			service = etaDirectoryUpdate.serviceList().get(0);
			assertEquals(service.serviceId(), 2);
			assertEquals(service.action(), com.refinitiv.eta.codec.MapEntryActions.DELETE);
			
			 assertEquals(1, niConsumer2.getMsgCount());
				
			etaMsg = niConsumer2.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.UPDATE);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.SOURCE);
			
			etaDirectoryMsg = convertToDirectoryUpdateMsg(etaMsg);
			assertEquals(etaDirectoryMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType.UPDATE);
			etaDirectoryUpdate = (com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryUpdate)etaDirectoryMsg;
			assertEquals(etaDirectoryUpdate.serviceList().size(), 1);
			service = etaDirectoryUpdate.serviceList().get(0);
			assertEquals(service.serviceId(), 2);
			assertEquals(service.action(), com.refinitiv.eta.codec.MapEntryActions.DELETE);
			
			try
			{
				provider.submit( EmaFactory.createRefreshMsg().serviceName("NI_PUB_1").name("TEST_3")
						.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
						.payload(fieldList).complete(true), 20);
				assertTrue(false);
			} catch (OmmInvalidUsageException except)
			{
				System.out.println("Exception thrown on just deleted NI_PUB_1 service name for refresh msg.");
			}

			// Shutdown both consumers
			niConsumer1.closeChannel();
			Thread.sleep(150);
			
			provider.submit( EmaFactory.createRefreshMsg().serviceName("NI_PUB_2").name("TEST_2")
					.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
					.payload(fieldList).complete(true), itemHandle_2);
			
			Thread.sleep(100);
			
			assertEquals(0, niConsumer1.getMsgCount());
			
			assertEquals(1, niConsumer2.getMsgCount());
			etaMsg = niConsumer2.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.msgKey().name().toString(), "TEST_2");
			
			Thread.sleep(1500);
			
			assertEquals(2, niConsumer1.getMsgCount());
			
			etaMsg = niConsumer1.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REQUEST);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.LOGIN);
			
			etaLoginMsg = convertToLoginMsg(etaMsg);
			
			assertEquals(etaLoginMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType.REQUEST);
			etaLoginRequest = (com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest)etaLoginMsg;
			assertEquals(etaLoginRequest.role(), com.refinitiv.eta.rdm.Login.RoleTypes.PROV);
			
			etaMsg = niConsumer1.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.SOURCE);
			
			etaDirectoryMsg = convertToDirectoryRefreshMsg(etaMsg);
			assertEquals(etaDirectoryMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType.REFRESH);
			etaDirectoryRefresh = (com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh)etaDirectoryMsg;
			assertEquals(etaDirectoryRefresh.serviceList().size(), 1);
			service = etaDirectoryRefresh.serviceList().get(0);
			assertEquals(service.info().serviceName().toString(), "NI_PUB_2");
			assertEquals(service.state().serviceState(), EmaRdm.SERVICE_UP);
			
			assertEquals(0, niConsumer2.getMsgCount());
	
			niConsumer1.closeChannel();
			niConsumer2.closeChannel();
			Thread.sleep(150);
			
			try
			{
				provider.submit( EmaFactory.createRefreshMsg().serviceName("NI_PUB_1").name("IBM.N")
						.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
						.payload(fieldList).complete(true), itemHandle);
				assertTrue(false);
			} catch (OmmInvalidUsageException except)
			{
				System.out.println("Exception thrown on not available NI_PUB_1 service name for refresh msg.");
			}
			
			Thread.sleep(1500);
			
			assertEquals(2, niConsumer1.getMsgCount());
			
			etaMsg = niConsumer1.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REQUEST);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.LOGIN);
			
			etaLoginMsg = convertToLoginMsg(etaMsg);
			
			assertEquals(etaLoginMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType.REQUEST);
			etaLoginRequest = (com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest)etaLoginMsg;
			assertEquals(etaLoginRequest.role(), com.refinitiv.eta.rdm.Login.RoleTypes.PROV);
			
			etaMsg = niConsumer1.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.SOURCE);
			
			etaDirectoryMsg = convertToDirectoryRefreshMsg(etaMsg);
			assertEquals(etaDirectoryMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType.REFRESH);
			etaDirectoryRefresh = (com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh)etaDirectoryMsg;
			assertEquals(etaDirectoryRefresh.serviceList().size(), 1);
			service = etaDirectoryRefresh.serviceList().get(0);
			assertEquals(service.info().serviceName().toString(), "NI_PUB_2");
			assertEquals(service.state().serviceState(), EmaRdm.SERVICE_UP);
			
			
			assertEquals(2, niConsumer2.getMsgCount());
			
			etaMsg = niConsumer2.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REQUEST);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.LOGIN);
			
			etaLoginMsg = convertToLoginMsg(etaMsg);
			
			assertEquals(etaLoginMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType.REQUEST);
			etaLoginRequest = (com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest)etaLoginMsg;
			assertEquals(etaLoginRequest.role(), com.refinitiv.eta.rdm.Login.RoleTypes.PROV);
			
			etaMsg = niConsumer2.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.SOURCE);
			
			etaDirectoryMsg = convertToDirectoryRefreshMsg(etaMsg);
			assertEquals(etaDirectoryMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType.REFRESH);
			etaDirectoryRefresh = (com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh)etaDirectoryMsg;
			assertEquals(etaDirectoryRefresh.serviceList().size(), 1);
			service = etaDirectoryRefresh.serviceList().get(0);
			assertEquals(service.info().serviceName().toString(), "NI_PUB_2");
			assertEquals(service.state().serviceState(), EmaRdm.SERVICE_UP);
			
			provider.submit( EmaFactory.createRefreshMsg().serviceName("NI_PUB_2").name("TEST_2")
					.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
					.payload(fieldList).complete(true), itemHandle_2);
			
			Thread.sleep(500);
			
			assertEquals(1, niConsumer1.getMsgCount());
			
			etaMsg = niConsumer1.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.msgKey().name().toString(), "TEST_2");
			
			assertEquals(1, niConsumer2.getMsgCount());
			
			etaMsg = niConsumer2.getMessage();
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REFRESH);
			assertEquals(etaMsg.msgKey().name().toString(), "TEST_2");
			
			// Now close the other service and see what happens
			
			// Send update with delete action
			map.clear();
			filterList.clear();
	        map.add( EmaFactory.createMapEntry().keyUInt(3, MapEntry.MapAction.DELETE, filterList));
	        
	        updateMsg = EmaFactory.createUpdateMsg();
	        provider.submit( updateMsg.domainType(EmaRdm.MMT_DIRECTORY).filter( EmaRdm.SERVICE_STATE_FILTER).payload(map)
	                ,sourceDirectoryHandle );
	        
	        Thread.sleep(500);
	        
	        assertEquals(1, niConsumer1.getMsgCount());
			
			etaMsg = niConsumer1.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.UPDATE);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.SOURCE);
			
			etaDirectoryMsg = convertToDirectoryUpdateMsg(etaMsg);
			assertEquals(etaDirectoryMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType.UPDATE);
			etaDirectoryUpdate = (com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryUpdate)etaDirectoryMsg;
			assertEquals(etaDirectoryUpdate.serviceList().size(), 1);
			service = etaDirectoryUpdate.serviceList().get(0);
			assertEquals(service.serviceId(), 3);
			assertEquals(service.action(), com.refinitiv.eta.codec.MapEntryActions.DELETE);
			
			 assertEquals(1, niConsumer2.getMsgCount());
				
			etaMsg = niConsumer2.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.UPDATE);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.SOURCE);
			
			etaDirectoryMsg = convertToDirectoryUpdateMsg(etaMsg);
			assertEquals(etaDirectoryMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType.UPDATE);
			etaDirectoryUpdate = (com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryUpdate)etaDirectoryMsg;
			assertEquals(etaDirectoryUpdate.serviceList().size(), 1);
			service = etaDirectoryUpdate.serviceList().get(0);
			assertEquals(service.serviceId(), 3);
			assertEquals(service.action(), com.refinitiv.eta.codec.MapEntryActions.DELETE);
			
			// Send a new message on NI_PUB_2, should thrown an exception
			try
			{
				provider.submit( EmaFactory.createRefreshMsg().serviceName("NI_PUB_2").name("TEST_3")
						.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
						.payload(fieldList).complete(true), 30);
				assertTrue(false);
			} catch (OmmInvalidUsageException except)
			{
				System.out.println("Exception thrown on just deleted NI_PUB_2 service name for refresh msg.");
			}
	
			// Shutdown both consumers
			niConsumer1.closeChannel();
			Thread.sleep(1500);
			
			// Consumer 1 will only get the login message
			assertEquals(1, niConsumer1.getMsgCount());
			
			etaMsg = niConsumer1.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REQUEST);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.LOGIN);
			
			etaLoginMsg = convertToLoginMsg(etaMsg);
			
			assertEquals(etaLoginMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType.REQUEST);
			etaLoginRequest = (com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest)etaLoginMsg;
			assertEquals(etaLoginRequest.role(), com.refinitiv.eta.rdm.Login.RoleTypes.PROV);
			

			assertEquals(0, niConsumer2.getMsgCount());
	
			niConsumer1.closeChannel();
			niConsumer2.closeChannel();
			Thread.sleep(150);
			
			try
			{
				provider.submit( EmaFactory.createRefreshMsg().serviceName("NI_PUB_1").name("IBM.N")
						.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "UnSolicited Refresh Completed")
						.payload(fieldList).complete(true), itemHandle);
				assertTrue(false);
			} catch (OmmInvalidUsageException except)
			{
				System.out.println("Exception thrown on not available NI_PUB_1 service name for refresh msg.");
			}
			
			Thread.sleep(1500);
			
			// Consumer 1 will only get the login message
			assertEquals(1, niConsumer1.getMsgCount());
			
			etaMsg = niConsumer1.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REQUEST);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.LOGIN);
			
			etaLoginMsg = convertToLoginMsg(etaMsg);
			
			assertEquals(etaLoginMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType.REQUEST);
			etaLoginRequest = (com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest)etaLoginMsg;
			assertEquals(etaLoginRequest.role(), com.refinitiv.eta.rdm.Login.RoleTypes.PROV);
			
			// Consumer 2 will only get the login message
			assertEquals(1, niConsumer2.getMsgCount());
			
			etaMsg = niConsumer2.getMessage();
			
			assertNotNull(etaMsg);
			assertEquals(etaMsg.msgClass(), com.refinitiv.eta.codec.MsgClasses.REQUEST);
			assertEquals(etaMsg.domainType(), com.refinitiv.eta.rdm.DomainTypes.LOGIN);
			
			etaLoginMsg = convertToLoginMsg(etaMsg);
			
			assertEquals(etaLoginMsg.rdmMsgType(), com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType.REQUEST);
			etaLoginRequest = (com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest)etaLoginMsg;
			assertEquals(etaLoginRequest.role(), com.refinitiv.eta.rdm.Login.RoleTypes.PROV);
			
			Thread.sleep(500);
			
		} catch(Exception e)
		{
			e.printStackTrace();
			assertTrue( false);
		}
	}
}


