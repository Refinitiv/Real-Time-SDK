/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024-2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access.unittest.requestrouting;

import java.util.*;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.locks.ReentrantLock;

import com.refinitiv.ema.JUnitConfigVariables;
import com.refinitiv.ema.access.*;
import com.refinitiv.ema.rdm.EmaRdm;

import static org.junit.Assert.assertTrue;

public class ConsumerTestClient implements OmmConsumerClient
{
	private final ArrayBlockingQueue<Msg> _messageQueue = new ArrayBlockingQueue<Msg>(200);
	
	private final ArrayBlockingQueue<ChannelInformation> _channelInfoQueue = new ArrayBlockingQueue<>(200);
	
	private final ArrayBlockingQueue<List<ChannelInformation>> _sessionChannelInfoQueue = new ArrayBlockingQueue<>(200);

	private final Queue<WarmStandbyChangeEventInfo> _wsbChangeEventInfoQueue = new ArrayBlockingQueue<>(200);

	private final ReentrantLock _userLock = new java.util.concurrent.locks.ReentrantLock();
	
	private final ConsumerTestOptions _consumerTestOptions;
	
	private final HashSet<Long> _handles = new HashSet<>();
	
	private long _postId = 0;
	
	private OmmConsumer _consumer;
	
	public ConsumerTestClient(ConsumerTestOptions options)
	{
		_consumerTestOptions = options;
	}
	
	public ConsumerTestClient()
	{
		_consumerTestOptions = new ConsumerTestOptions();
	}
	
	public void consumer(OmmConsumer consumer)
	{
		_consumer = consumer;
	}

	public long poll_timeout = JUnitConfigVariables.MSGQUEUE_POLL_TIMEOUT_MS;

	public TimeUnit poll_time_unit = TimeUnit.MILLISECONDS;
	
	public int queueSize()
	{
		return _messageQueue.size();
	}
	
	public Msg popMessage()
	{
		try
		{
			return _messageQueue.poll(poll_timeout, poll_time_unit);
		}
		catch (Exception e)
		{
			assertTrue(false);
			return null;
		}
	}

	public Msg popMessage(long timeoutMs)
	{
		try
		{
			return _messageQueue.poll(timeoutMs, poll_time_unit);
		}
		catch (Exception e)
		{
			assertTrue(false);
			return null;
		}
	}
	
	public void clearQueue() 
	{
		_messageQueue.clear();
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
			return _channelInfoQueue.poll();
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
			return _sessionChannelInfoQueue.poll();
		}
		finally
		{
			_userLock.unlock();
		}
	}

	public int wsbChangeEventInfoSize()
	{
		_userLock.lock();
		try
		{
			return _wsbChangeEventInfoQueue.size();
		}
		finally
		{
			_userLock.unlock();
		}
	}

	public WarmStandbyChangeEventInfo popWsbChangeEventInfo()
	{
		_userLock.lock();
		try
		{
			return _wsbChangeEventInfoQueue.poll();
		}
		finally
		{
			_userLock.unlock();
		}
	}

	private void addChannelAndSessionInfo(OmmConsumerEvent consumerEvent)
	{
		if(_consumerTestOptions.getChannelInformation)
		{
			ChannelInformation channelInfo = new ChannelInformationTest(consumerEvent.channelInformation());
			
			_channelInfoQueue.add(channelInfo);
			
			System.out.println(consumerEvent.channelInformation());
		}
		
		if(_consumerTestOptions.getSessionChannelInfo)
		{
			List<ChannelInformation> sessionInfo = new ArrayList<>();
			
			consumerEvent.sessionChannelInfo(sessionInfo);
			
			_sessionChannelInfoQueue.add(sessionInfo);
		}
	}

	private void addWsbChangeEventInfo(OmmConsumerEvent consumerEvent)
	{
		if(_consumerTestOptions.getWsbChangeEventInfo)
		{
			WarmStandbyChangeEventInfo wsbChangeEventInfo =  consumerEvent.warmStandbyChangeEventInfo();

			_wsbChangeEventInfoQueue.offer(wsbChangeEventInfo);

			System.out.println(wsbChangeEventInfo);
		}
	}

	public void unregisterAllHandles()
	{
		for(Long handle : _handles)
		{
			_consumer.unregister(handle);
		}
		
		_handles.clear();
	}

	public HashMap<String, Long> handleNameMap = new HashMap<>();

	@Override
	public void onRefreshMsg(RefreshMsg refreshMsg, OmmConsumerEvent consumerEvent) 
	{
		_userLock.lock();
		
		try
		{
			_handles.add(consumerEvent.handle());
			if (refreshMsg.hasName()) handleNameMap.put(refreshMsg.name(), consumerEvent.handle());

			RefreshMsg cloneMsg = EmaFactory.createRefreshMsg(1280);
			
			refreshMsg.copy(cloneMsg);
			System.out.println("\n>>>>>>>>>>>>>> Consumer received refresh message in domain " + cloneMsg.domainType()
					+ (cloneMsg.hasServiceName() ? ", serviceName: " + refreshMsg.serviceName() : ""));

			if(refreshMsg.domainType() == EmaRdm.MMT_DICTIONARY)
			{
				if(_consumerTestOptions.dumpDictionaryRefreshMsg)
					System.out.println(cloneMsg);
			}
			else
			{
				if(_consumerTestOptions.submitPostOnLoginRefresh)
				{
					if(refreshMsg.domainType() == EmaRdm.MMT_LOGIN && 
							refreshMsg.state().streamState() == OmmState.StreamState.OPEN &&
							refreshMsg.state().dataState() == OmmState.DataState.OK)
					{
						/* Submit a PostMsg to the login stream */
						PostMsg postMsg = EmaFactory.createPostMsg();
						UpdateMsg nestedUpdateMsg = EmaFactory.createUpdateMsg();
						FieldList nestedFieldList = EmaFactory.createFieldList();
	
						nestedFieldList.add(EmaFactory.createFieldEntry().real(22, 34, OmmReal.MagnitudeType.EXPONENT_POS_1));
						nestedFieldList.add(EmaFactory.createFieldEntry().real(25, 35, OmmReal.MagnitudeType.EXPONENT_POS_1));
						nestedFieldList.add(EmaFactory.createFieldEntry().time(18, 11, 29, 30));
						nestedFieldList.add(EmaFactory.createFieldEntry().enumValue(37, 3));
						
						nestedUpdateMsg.payload(nestedFieldList );
							
						_consumer.submit( postMsg.postId( ++_postId ).serviceName("DIRECT_FEED")
																	.name( "IBM.N" ).solicitAck( false ).complete(true)
																	.payload(nestedUpdateMsg), consumerEvent.handle() );
					}
				}
				
				System.out.println(cloneMsg);
			}
	
			_messageQueue.add(cloneMsg);
			
			addChannelAndSessionInfo(consumerEvent);
		}
		finally
		{
			_userLock.unlock();
		}
		
	}

	@Override
	public void onUpdateMsg(UpdateMsg updateMsg, OmmConsumerEvent consumerEvent) 
	{
		_userLock.lock();
		
		try
		{
			UpdateMsg cloneMsg = EmaFactory.createUpdateMsg(1280);
			
			updateMsg.copy(cloneMsg);
			
			System.out.println(cloneMsg);
			
			_messageQueue.add(cloneMsg);
			
			addChannelAndSessionInfo(consumerEvent);
		}
		finally
		{
			_userLock.unlock();
		}
		
	}

	@Override
	public void onStatusMsg(StatusMsg statusMsg, OmmConsumerEvent consumerEvent)
	{
		
		_userLock.lock();
		
		try
		{
			StatusMsg cloneMsg = EmaFactory.createStatusMsg(1280);
			
			statusMsg.copy(cloneMsg);
			
			System.out.println(cloneMsg);
		
			_messageQueue.add(cloneMsg);
			
			addChannelAndSessionInfo(consumerEvent);
			if (statusMsg.state().statusCode() == OmmState.StatusCode.WSB_CHANGE_ACTIVE_COMPLETE)
			{
				addWsbChangeEventInfo(consumerEvent);
			}
		}
		finally
		{
			_userLock.unlock();
		}
	}

	@Override
	public void onGenericMsg(GenericMsg genericMsg, OmmConsumerEvent consumerEvent) 
	{
		_userLock.lock();
		
		try
		{
			GenericMsg cloneMsg = EmaFactory.createGenericMsg(1280);
			
			genericMsg.copy(cloneMsg);
			
			System.out.println(cloneMsg);
		
			_messageQueue.add(cloneMsg);
			
			addChannelAndSessionInfo(consumerEvent);
		}
		finally
		{
			_userLock.unlock();
		}
		
	}

	@Override
	public void onAckMsg(AckMsg ackMsg, OmmConsumerEvent consumerEvent) 
	{
		_userLock.lock();
		
		try
		{
			AckMsg cloneMsg = EmaFactory.createAckMsg(1280);
			
			ackMsg.copy(cloneMsg);
		
			_messageQueue.add(cloneMsg);
			
			addChannelAndSessionInfo(consumerEvent);
		}
		finally
		{
			_userLock.unlock();
		}
	}

	@Override
	public void onAllMsg(Msg msg, OmmConsumerEvent consumerEvent) {
		// Do nothing
	}

}
