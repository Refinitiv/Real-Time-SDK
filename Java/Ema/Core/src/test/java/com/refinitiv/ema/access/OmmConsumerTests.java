///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|             Copyright (C) 2024-2025 LSEG. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

import com.refinitiv.ema.JUnitConfigVariables;
import com.refinitiv.ema.RetryRule;
import com.refinitiv.ema.unittest.TestUtilities;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.rdm.Login.ServerTypes;
import com.refinitiv.eta.valueadd.reactor.ReactorFactory;

import junit.framework.TestCase;

import static org.junit.Assert.assertThrows;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import org.junit.*;
import org.junit.runners.MethodSorters;

import com.refinitiv.ema.access.ChannelInformation.ChannelState;
import com.refinitiv.ema.access.DataType.DataTypes;
import com.refinitiv.ema.access.OmmIProviderConfig.AdminControl;
import com.refinitiv.ema.access.unittest.requestrouting.ConsumerTestClient;
import com.refinitiv.ema.access.unittest.requestrouting.ConsumerTestOptions;
import com.refinitiv.ema.access.unittest.requestrouting.ProviderTestClient;
import com.refinitiv.ema.access.unittest.requestrouting.ProviderTestOptions;
import com.refinitiv.ema.rdm.EmaRdm;

@FixMethodOrder(MethodSorters.DEFAULT)
public class OmmConsumerTests extends TestCase
{
	public OmmConsumerTests()
	{
	}

	@Rule
	public RetryRule retryRule = new RetryRule(JUnitConfigVariables.TEST_RETRY_COUNT);

	@After
	public void tearDown()
	{
		try { Thread.sleep(JUnitConfigVariables.WAIT_AFTER_TEST); }
		catch (Exception e) { }
	}
	
	@Test
	public void testSingleItemRecoverFromRequestTimeout()
	{
		TestUtilities.printTestHead("testSingleItemRecoverFromRequestTimeout","");

		String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";
		
		OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);
		
		ProviderTestOptions providerTestOptions = new ProviderTestOptions();
		
		providerTestOptions.sendItemResponse = false;
		ProviderTestClient providerClient1 = new ProviderTestClient(providerTestOptions);
		
		// Provider_1 provides the DIRECT_FEED service name
		OmmProvider ommprovider = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_1"), providerClient1);
		
		assertNotNull(ommprovider);
		
		ProviderTestOptions providerTestOptions2 = new ProviderTestOptions();
		ProviderTestClient providerClient2 = new ProviderTestClient(providerTestOptions2);
		
		// Provider_1 provides the DIRECT_FEED service name
		OmmProvider ommprovider2 = EmaFactory.createOmmProvider(config.port("19004").providerName("Provider_1"), providerClient2);
		
		assertNotNull(ommprovider2);
		
		OmmConsumer consumer = null;
		ConsumerTestClient consumerClient = new ConsumerTestClient();
		
		try
		{
			/* The item request timeout is set to 3 seconds for Consumer_9 */
			consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_9"));
			
			ReqMsg reqMsg = EmaFactory.createReqMsg();
			
			long itemHandle1 = consumer.registerClient(reqMsg.clear().serviceName("DIRECT_FEED").name("LSEG.O"), consumerClient);
			
			Thread.sleep(8500);
			
			/* Ensure that the provider receives a request message but it doesn't send a response back. */
			assertEquals(2, providerClient1.queueSize());
			
			Msg message = providerClient1.popMessage();
			ReqMsg recvReqMsg = (ReqMsg)message;
			
			assertEquals(3, recvReqMsg.streamId());
			assertEquals(1, recvReqMsg.serviceId());
			assertEquals("DIRECT_FEED", recvReqMsg.serviceName());
			assertEquals("LSEG.O", recvReqMsg.name());
			
			assertEquals(2, consumerClient.queueSize());
			message = consumerClient.popMessage();
			
			StatusMsg statusMsg = (StatusMsg)message;
			
			assertEquals(5, statusMsg.streamId());
			assertEquals("DIRECT_FEED", statusMsg.serviceName());
			assertEquals("LSEG.O", statusMsg.name());
			assertEquals(OmmState.StreamState.OPEN, statusMsg.state().streamState());
			assertEquals(OmmState.DataState.SUSPECT, statusMsg.state().dataState());
			assertEquals(OmmState.StatusCode.NONE, statusMsg.state().statusCode());
			assertEquals("Request timeout", statusMsg.state().statusText());
			
			Thread.sleep(2000);
			assertEquals(1, providerClient1.queueSize());
				
			/* Provider receives a close message */
			message = providerClient1.popMessage();
			recvReqMsg = (ReqMsg)message;
			
			assertEquals(3, recvReqMsg.streamId());
			assertEquals(1, recvReqMsg.serviceId());
			assertEquals("DIRECT_FEED", recvReqMsg.serviceName());
			assertEquals("LSEG.O", recvReqMsg.name());
			
			/* The second provider receives the request message */
			assertEquals(1, providerClient2.queueSize());
			message = providerClient2.popMessage();
			recvReqMsg = (ReqMsg)message;
			
			assertEquals(3, recvReqMsg.streamId());
			assertEquals(1, recvReqMsg.serviceId());
			assertEquals("DIRECT_FEED", recvReqMsg.serviceName());
			assertEquals("LSEG.O", recvReqMsg.name());
			
			/* Receives the first refresh message from the second provider */
			assertEquals(1, consumerClient.queueSize());
			message = consumerClient.popMessage();
			
			RefreshMsg refreshMsg = (RefreshMsg)message;
			
			assertEquals("DIRECT_FEED", refreshMsg.serviceName());
			assertEquals("LSEG.O", refreshMsg.name());
			assertEquals(32767, refreshMsg.serviceId());
			assertEquals(OmmState.StreamState.OPEN, refreshMsg.state().streamState());
			assertEquals(OmmState.DataState.OK, refreshMsg.state().dataState());
			assertEquals(OmmState.StatusCode.NONE, refreshMsg.state().code());
			assertTrue(refreshMsg.solicited());
			assertTrue(refreshMsg.complete());

			consumer.unregister(itemHandle1);

			assertNotNull(consumer);
		}
		catch(OmmException excep)
		{
			assertFalse(true);
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
		finally {
			System.out.println("Uninitializing...");

			consumer.uninitialize();
			ommprovider.uninitialize();
			ommprovider2.uninitialize();
		}
	}
	
	@Test
	public void testRequestingSingleItemWithRequestedServiceDownWithOpenSuspectStatus()
	{
		TestUtilities.printTestHead("testRequestingSingleItemWithRequestedServiceDownWithOpenSuspectStatus","");

		String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";
		
		OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);
		
		ProviderTestOptions providerTestOptions = new ProviderTestOptions();
		
		ProviderTestClient providerClient1 = new ProviderTestClient(providerTestOptions);
		
		providerTestOptions.itemGroupId = ByteBuffer.wrap("10".getBytes());
		
		// Provider_1 provides the DIRECT_FEED service name
		OmmProvider ommprovider = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_1"), providerClient1);
		
		assertNotNull(ommprovider);
		
		ProviderTestClient providerClient2 = new ProviderTestClient(providerTestOptions);
		
		// Provider_1 provides the DIRECT_FEED service name
		OmmProvider ommprovider2 = EmaFactory.createOmmProvider(config.port("19004").providerName("Provider_1"), providerClient2);
		
		assertNotNull(ommprovider2);
		
		OmmConsumer consumer = null;
		ConsumerTestClient consumerClient = new ConsumerTestClient();
		
		try
		{			
			consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_9"));
			
			ReqMsg reqMsg = EmaFactory.createReqMsg();
			
			long itemHandle = consumer.registerClient(reqMsg.clear().serviceName("DIRECT_FEED").name("LSEG.O"), consumerClient);
					
			Thread.sleep(3000);
			
			assertEquals(1, providerClient1.queueSize());
			
			Msg message = providerClient1.popMessage();
			
			assertTrue(message instanceof ReqMsg);
			
			ReqMsg requestMsg = (ReqMsg)message;
			
			assertEquals("DIRECT_FEED", requestMsg.serviceName());
			assertEquals("LSEG.O", requestMsg.name());
			assertEquals(1, requestMsg.serviceId());
			
			Thread.sleep(2000);
			
			assertEquals(1, consumerClient.queueSize());
			
			message = consumerClient.popMessage();
			
			RefreshMsg refreshMsg = (RefreshMsg)message;
			
			assertEquals("DIRECT_FEED", refreshMsg.serviceName());
			assertEquals("LSEG.O", refreshMsg.name());
			assertEquals(OmmState.StreamState.OPEN, refreshMsg.state().streamState());
			assertEquals(OmmState.DataState.OK, refreshMsg.state().dataState());
			assertEquals(OmmState.StatusCode.NONE, refreshMsg.state().code());
			
			/* The first provider sends a source directory update message to change service state to DOWN with service's status */			
			ElementList serviceState = EmaFactory.createElementList();
			serviceState.add( EmaFactory.createElementEntry().uintValue( EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_DOWN ));
			serviceState.add( EmaFactory.createElementEntry().uintValue( EmaRdm.ENAME_ACCEPTING_REQS, 1 ));
			serviceState.add( EmaFactory.createElementEntry().state( EmaRdm.ENAME_STATUS, OmmState.StreamState.OPEN, OmmState.DataState.SUSPECT, OmmState.StatusCode.NONE));
			
			FilterList filterListEnc = EmaFactory.createFilterList();
			filterListEnc.add( EmaFactory.createFilterEntry().elementList( EmaRdm.SERVICE_STATE_ID, FilterEntry.FilterAction.SET, serviceState ) );
	      
			Map map = EmaFactory.createMap();
			map.add( EmaFactory.createMapEntry().keyUInt( 1, MapEntry.MapAction.UPDATE, filterListEnc ));
	      
			UpdateMsg updateMsg = EmaFactory.createUpdateMsg();
			ommprovider.submit( updateMsg.domainType( EmaRdm.MMT_DIRECTORY ).
					filter( EmaRdm.SERVICE_STATE_FILTER ).
					payload( map ), 0);	// use 0 item handle to fan-out to all subscribers
			
			
			Thread.sleep(2000); // Wait until consumer receives the item Open/Suspect status message from the VA Watchlist
			
			/* The second provider receives the request message */
			assertEquals(1, providerClient2.queueSize());
			message = providerClient2.popMessage();
			ReqMsg recvReqMsg = (ReqMsg)message;
			
			assertEquals(3, recvReqMsg.streamId());
			assertEquals(1, recvReqMsg.serviceId());
			assertEquals("DIRECT_FEED", recvReqMsg.serviceName());
			assertEquals("LSEG.O", recvReqMsg.name());
			
			assertEquals(2, consumerClient.queueSize());
			
			message = consumerClient.popMessage();
			
			StatusMsg statusMsg = (StatusMsg)message;
			
			assertEquals("DIRECT_FEED", statusMsg.serviceName());
			assertEquals("LSEG.O", statusMsg.name());
			assertEquals(OmmState.StreamState.OPEN, statusMsg.state().streamState());
			assertEquals(OmmState.DataState.SUSPECT, statusMsg.state().dataState());
			assertEquals(OmmState.StatusCode.NONE, statusMsg.state().statusCode());
			
			/* Receives refresh message from the second provider */
			assertEquals(1, consumerClient.queueSize());
			message = consumerClient.popMessage();
			
			refreshMsg = (RefreshMsg)message;
			
			assertEquals("DIRECT_FEED", refreshMsg.serviceName());
			assertEquals("LSEG.O", refreshMsg.name());
			assertEquals(32767, refreshMsg.serviceId());
			assertEquals(OmmState.StreamState.OPEN, refreshMsg.state().streamState());
			assertEquals(OmmState.DataState.OK, refreshMsg.state().dataState());
			assertEquals(OmmState.StatusCode.NONE, refreshMsg.state().code());
			assertTrue(refreshMsg.solicited());
			assertTrue(refreshMsg.complete());
			
			consumer.unregister(itemHandle);

			assertNotNull(consumer);
		}
		catch(OmmException excep)
		{
			assertFalse(true);
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
		finally {
			System.out.println("Uninitializing...");

			consumer.uninitialize();
			ommprovider.uninitialize();
			ommprovider2.uninitialize();
		}
	}
	
	@Test
	public void testRequestingSingleItemWithRequestedConnectionDownWithEnableSessionEnhancedItemRecovery()
	{
		TestUtilities.printTestHead("testRequestingSingleItemWithRequestedConnectionDownWithEnableSessionEnhancedItemRecovery","");

		String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";
		
		OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);
		
		ProviderTestOptions providerTestOptions = new ProviderTestOptions();
		
		ProviderTestClient providerClient1 = new ProviderTestClient(providerTestOptions);
		
		providerTestOptions.itemGroupId = ByteBuffer.wrap("10".getBytes());
		
		// Provider_1 provides the DIRECT_FEED service name
		OmmProvider ommprovider = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_1"), providerClient1);
		
		assertNotNull(ommprovider);
		
		ProviderTestClient providerClient2 = new ProviderTestClient(providerTestOptions);
		
		// Provider_1 provides the DIRECT_FEED service name
		OmmProvider ommprovider2 = EmaFactory.createOmmProvider(config.port("19004").providerName("Provider_1"), providerClient2);
		
		assertNotNull(ommprovider2);
		
		OmmConsumer consumer = null;
		ConsumerTestClient consumerClient = new ConsumerTestClient();
		
		try
		{			
			consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_12"));
			
			ReqMsg reqMsg = EmaFactory.createReqMsg();
			
			long itemHandle = consumer.registerClient(reqMsg.clear().serviceName("DIRECT_FEED").name("LSEG.O"), consumerClient);
					
			Thread.sleep(3000);
			
			/* Checks provider that receives the item request. */
			Msg message;
			ReqMsg requestMsg = null;			
			
			message = providerClient1.popMessage();
			assertTrue(message instanceof ReqMsg);
			requestMsg = (ReqMsg)message;
			
			assertEquals("DIRECT_FEED", requestMsg.serviceName());
			assertEquals("LSEG.O", requestMsg.name());
			assertEquals(1, requestMsg.serviceId());
			
			Thread.sleep(2000);
			
			assertEquals(1, consumerClient.queueSize());
			
			message = consumerClient.popMessage();
			
			RefreshMsg refreshMsg = (RefreshMsg)message;
			
			assertEquals("DIRECT_FEED", refreshMsg.serviceName());
			assertEquals("LSEG.O", refreshMsg.name());
			assertEquals(OmmState.StreamState.OPEN, refreshMsg.state().streamState());
			assertEquals(OmmState.DataState.OK, refreshMsg.state().dataState());
			assertEquals(OmmState.StatusCode.NONE, refreshMsg.state().code());
			
			/* Force channel down on provider */
			ReqMsg recvReqMsg;
			ommprovider.uninitialize();
			
			Thread.sleep(2000);
		
			/* The second provider receives the request message */
			assertEquals(1, providerClient2.queueSize());
			message = providerClient2.popMessage();
			recvReqMsg = (ReqMsg)message;
			
			assertEquals(3, recvReqMsg.streamId());
			assertEquals(1, recvReqMsg.serviceId());
			assertEquals("DIRECT_FEED", recvReqMsg.serviceName());
			assertEquals("LSEG.O", recvReqMsg.name());
			
			assertEquals(2, consumerClient.queueSize());
			
			message = consumerClient.popMessage();
			
			StatusMsg statusMsg = (StatusMsg)message;
			
			assertEquals("DIRECT_FEED", statusMsg.serviceName());
			assertEquals("LSEG.O", statusMsg.name());
			assertEquals(OmmState.StreamState.OPEN, statusMsg.state().streamState());
			assertEquals(OmmState.DataState.SUSPECT, statusMsg.state().dataState());
			assertEquals(OmmState.StatusCode.NONE, statusMsg.state().statusCode());
			
			/* Receives refresh message from the second provider */
			assertEquals(1, consumerClient.queueSize());
			message = consumerClient.popMessage();
			
			refreshMsg = (RefreshMsg)message;
			
			assertEquals("DIRECT_FEED", refreshMsg.serviceName());
			assertEquals("LSEG.O", refreshMsg.name());
			assertEquals(32767, refreshMsg.serviceId());
			assertEquals(OmmState.StreamState.OPEN, refreshMsg.state().streamState());
			assertEquals(OmmState.DataState.OK, refreshMsg.state().dataState());
			assertEquals(OmmState.StatusCode.NONE, refreshMsg.state().code());
			assertTrue(refreshMsg.solicited());
			assertTrue(refreshMsg.complete());
			
			consumer.unregister(itemHandle);
		}
		catch(OmmException excep)
		{
			assertFalse(true);
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
		finally {
			System.out.println("Uninitializing...");

			consumer.uninitialize();
			ommprovider.uninitialize();
			ommprovider2.uninitialize();
		}
	}
	
	@Test
	public void testRequestingSingleItemWithRequestedConnectionDownWithDisableSessionEnhancedItemRecovery()
	{
		TestUtilities.printTestHead("testRequestingSingleItemWithRequestedConnectionDownWithDisableSessionEnhancedItemRecovery","");

		String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";
		
		OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);
		
		ProviderTestOptions providerTestOptions = new ProviderTestOptions();
		
		ProviderTestClient providerClient1 = new ProviderTestClient(providerTestOptions);
		
		providerTestOptions.itemGroupId = ByteBuffer.wrap("10".getBytes());
		
		// Provider_1 provides the DIRECT_FEED service name
		OmmProvider ommprovider = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_1"), providerClient1);
		
		assertNotNull(ommprovider);
		
		ProviderTestClient providerClient2 = new ProviderTestClient(providerTestOptions);
		
		// Provider_1 provides the DIRECT_FEED service name
		OmmProvider ommprovider2 = EmaFactory.createOmmProvider(config.port("19004").providerName("Provider_1"), providerClient2);
		
		assertNotNull(ommprovider2);
		
		OmmConsumer consumer = null;
		ConsumerTestClient consumerClient = new ConsumerTestClient();
		
		try
		{			
			consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_9"));
			
			ReqMsg reqMsg = EmaFactory.createReqMsg();
			
			long itemHandle = consumer.registerClient(reqMsg.clear().serviceName("DIRECT_FEED").name("LSEG.O"), consumerClient);
					
			Thread.sleep(1000);
			
			/* Checks provider that receives the item request. */
			Msg message;
			ReqMsg requestMsg = null;			
			
			message = providerClient1.popMessage();
			assertTrue(message instanceof ReqMsg);
			requestMsg = (ReqMsg)message;
			
			assertEquals("DIRECT_FEED", requestMsg.serviceName());
			assertEquals("LSEG.O", requestMsg.name());
			assertEquals(1, requestMsg.serviceId());
			
			Thread.sleep(1000);
			
			assertEquals(1, consumerClient.queueSize());
			
			message = consumerClient.popMessage();
			
			RefreshMsg refreshMsg = (RefreshMsg)message;
			
			assertEquals("DIRECT_FEED", refreshMsg.serviceName());
			assertEquals("LSEG.O", refreshMsg.name());
			assertEquals(OmmState.StreamState.OPEN, refreshMsg.state().streamState());
			assertEquals(OmmState.DataState.OK, refreshMsg.state().dataState());
			assertEquals(OmmState.StatusCode.NONE, refreshMsg.state().code());
			
			/* Force channel down on the first provider */
			ReqMsg recvReqMsg;
			ommprovider.uninitialize();
			
			Thread.sleep(1000);
			
			ommprovider = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_1"), providerClient1);
			
			Thread.sleep(3000);
			
			/* The first provider receives the request message */
			assertEquals(1, providerClient1.queueSize());
			
			message = providerClient1.popMessage();
			recvReqMsg = (ReqMsg)message;
			
			assertEquals(1, recvReqMsg.streamId());
			assertEquals(EmaRdm.MMT_LOGIN, recvReqMsg.domainType());
			
			Thread.sleep(3000);
			assertEquals(1, providerClient1.queueSize());
			
			message = providerClient1.popMessage();
			recvReqMsg = (ReqMsg)message;
			
			assertEquals(4, recvReqMsg.streamId());
			assertEquals(1, recvReqMsg.serviceId());
			assertEquals("DIRECT_FEED", recvReqMsg.serviceName());
			assertEquals("LSEG.O", recvReqMsg.name());
			
			assertEquals(2, consumerClient.queueSize());
			
			message = consumerClient.popMessage();
			
			StatusMsg statusMsg = (StatusMsg)message;
			
			assertEquals("DIRECT_FEED", statusMsg.serviceName());
			assertEquals("LSEG.O", statusMsg.name());
			assertEquals(OmmState.StreamState.OPEN, statusMsg.state().streamState());
			assertEquals(OmmState.DataState.SUSPECT, statusMsg.state().dataState());
			assertEquals(OmmState.StatusCode.NONE, statusMsg.state().statusCode());
			
			/* Receives refresh message from the first provider */
			assertEquals(1, consumerClient.queueSize());
			message = consumerClient.popMessage();
			
			refreshMsg = (RefreshMsg)message;
			
			assertEquals("DIRECT_FEED", refreshMsg.serviceName());
			assertEquals("LSEG.O", refreshMsg.name());
			assertEquals(32767, refreshMsg.serviceId());
			assertEquals(OmmState.StreamState.OPEN, refreshMsg.state().streamState());
			assertEquals(OmmState.DataState.OK, refreshMsg.state().dataState());
			assertEquals(OmmState.StatusCode.NONE, refreshMsg.state().code());
			assertTrue(refreshMsg.solicited());
			assertTrue(refreshMsg.complete());
			
			consumer.unregister(itemHandle);
		}
		catch(OmmException excep)
		{
			assertFalse(true);
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
		finally {
			System.out.println("Uninitializing...");

			consumer.uninitialize();
			ommprovider.uninitialize();
			ommprovider2.uninitialize();
		}
	}
	
	@Test
	public void testRequestingSingleItemWithRequestedConnectionDownWithDisableSessionEnhancedItemRecoveryAndChannelIsClosed()
	{
		TestUtilities.printTestHead("testRequestingSingleItemWithRequestedConnectionDownWithDisableSessionEnhancedItemRecoveryAndChannelIsClosed","");

		String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";
		
		OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);
		
		ProviderTestOptions providerTestOptions = new ProviderTestOptions();
		
		ProviderTestClient providerClient1 = new ProviderTestClient(providerTestOptions);
		
		providerTestOptions.itemGroupId = ByteBuffer.wrap("10".getBytes());
		
		// Provider_1 provides the DIRECT_FEED service name
		OmmProvider ommprovider = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_1"), providerClient1);
		
		assertNotNull(ommprovider);
		
		ProviderTestClient providerClient2 = new ProviderTestClient(providerTestOptions);
		
		// Provider_1 provides the DIRECT_FEED service name
		OmmProvider ommprovider2 = EmaFactory.createOmmProvider(config.port("19004").providerName("Provider_1"), providerClient2);
		
		assertNotNull(ommprovider2);
		
		OmmConsumer consumer = null;
		ConsumerTestClient consumerClient = new ConsumerTestClient();
		
		try
		{			
			consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_9"));
			
			ReqMsg reqMsg = EmaFactory.createReqMsg();
			
			long itemHandle = consumer.registerClient(reqMsg.clear().serviceName("DIRECT_FEED").name("LSEG.O"), consumerClient);
					
			Thread.sleep(1000);
			
			/* Checks provider that receives the item request. */
			Msg message;
			ReqMsg requestMsg = null;			
			
			message = providerClient1.popMessage();
			assertTrue(message instanceof ReqMsg);
			requestMsg = (ReqMsg)message;
			
			assertEquals("DIRECT_FEED", requestMsg.serviceName());
			assertEquals("LSEG.O", requestMsg.name());
			assertEquals(1, requestMsg.serviceId());
			
			Thread.sleep(1000);
			
			assertEquals(1, consumerClient.queueSize());
			
			message = consumerClient.popMessage();
			
			RefreshMsg refreshMsg = (RefreshMsg)message;
			
			assertEquals("DIRECT_FEED", refreshMsg.serviceName());
			assertEquals("LSEG.O", refreshMsg.name());
			assertEquals(OmmState.StreamState.OPEN, refreshMsg.state().streamState());
			assertEquals(OmmState.DataState.OK, refreshMsg.state().dataState());
			assertEquals(OmmState.StatusCode.NONE, refreshMsg.state().code());
			
			/* Force channel down on the first provider */
			ReqMsg recvReqMsg;
			ommprovider.uninitialize();
			
			/* Wait until the subscribed channel is closed */
			Thread.sleep(8000);

			assertEquals(1, providerClient2.queueSize());
			
			message = providerClient2.popMessage();
			recvReqMsg = (ReqMsg)message;
			
			assertEquals(3, recvReqMsg.streamId());
			assertEquals(1, recvReqMsg.serviceId());
			assertEquals("DIRECT_FEED", recvReqMsg.serviceName());
			assertEquals("LSEG.O", recvReqMsg.name());
			
			assertEquals(2, consumerClient.queueSize());
			
			message = consumerClient.popMessage();
			
			StatusMsg statusMsg = (StatusMsg)message;
			
			assertEquals("DIRECT_FEED", statusMsg.serviceName());
			assertEquals("LSEG.O", statusMsg.name());
			assertEquals(OmmState.StreamState.OPEN, statusMsg.state().streamState());
			assertEquals(OmmState.DataState.SUSPECT, statusMsg.state().dataState());
			assertEquals(OmmState.StatusCode.NONE, statusMsg.state().statusCode());
			
			/* Receives refresh message from the second provider */
			assertEquals(1, consumerClient.queueSize());
			message = consumerClient.popMessage();
			
			refreshMsg = (RefreshMsg)message;
			
			assertEquals("DIRECT_FEED", refreshMsg.serviceName());
			assertEquals("LSEG.O", refreshMsg.name());
			assertEquals(32767, refreshMsg.serviceId());
			assertEquals(OmmState.StreamState.OPEN, refreshMsg.state().streamState());
			assertEquals(OmmState.DataState.OK, refreshMsg.state().dataState());
			assertEquals(OmmState.StatusCode.NONE, refreshMsg.state().code());
			assertTrue(refreshMsg.solicited());
			assertTrue(refreshMsg.complete());
			
			consumer.unregister(itemHandle);
		}
		catch(OmmException excep)
		{
			assertFalse(true);
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
		finally {
			System.out.println("Uninitializing...");

			consumer.uninitialize();
			ommprovider2.uninitialize();
		}
	}
	
	@Test
	public void testRequestingSingleItemReceivedCloseSuspectFromProvider()
	{
		TestUtilities.printTestHead("testRequestingSingleItemReceivedCloseSuspectFromProvider","");

		String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";
		
		OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);
		
		ProviderTestOptions providerTestOptions = new ProviderTestOptions();
		
		providerTestOptions.closeItemRequest = true;
		ProviderTestClient providerClient1 = new ProviderTestClient(providerTestOptions);
		
		// Provider_1 provides the DIRECT_FEED service name
		OmmProvider ommprovider = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_1"), providerClient1);
		
		assertNotNull(ommprovider);
		
		ProviderTestOptions providerTestOptions2 = new ProviderTestOptions();
		providerTestOptions2.closeItemRequest = true;
		ProviderTestClient providerClient2 = new ProviderTestClient(providerTestOptions2);
		
		// Provider_1 provides the DIRECT_FEED service name
		OmmProvider ommprovider2 = EmaFactory.createOmmProvider(config.port("19004").providerName("Provider_1"), providerClient2);
		
		assertNotNull(ommprovider2);
		
		OmmConsumer consumer = null;
		ConsumerTestOptions consumerTestOptions = new ConsumerTestOptions();
		consumerTestOptions.getChannelInformation = true;
		ConsumerTestClient consumerClient = new ConsumerTestClient(consumerTestOptions);
		
		try
		{
			/* The item request timeout is set to 3 seconds for Consumer_9 */
			consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_9"));
			
			ReqMsg reqMsg = EmaFactory.createReqMsg();
			
			consumer.registerClient(reqMsg.clear().serviceName("DIRECT_FEED").name("LSEG.O"), consumerClient);
			
			Thread.sleep(2000);
			
			/* Ensure that the provider receives a request message but it closes the item stream. */
			assertEquals(1, providerClient1.queueSize());
			
			Msg message = providerClient1.popMessage();
			ReqMsg recvReqMsg = (ReqMsg)message;
			
			assertEquals(3, recvReqMsg.streamId());
			assertEquals(1, recvReqMsg.serviceId());
			assertEquals("DIRECT_FEED", recvReqMsg.serviceName());
			assertEquals("LSEG.O", recvReqMsg.name());
			
			Thread.sleep(1000);
			
			/* Ensure that the first provider doesn't receive a close message */
			assertEquals(0, providerClient1.queueSize());
				
			/* Ensure that the second provider receive a request message but it closes the item stream */
			assertEquals(1, providerClient2.queueSize());
			
			message = providerClient2.popMessage();
			recvReqMsg = (ReqMsg)message;
			
			assertEquals(3, recvReqMsg.streamId());
			assertEquals(1, recvReqMsg.serviceId());
			assertEquals("DIRECT_FEED", recvReqMsg.serviceName());
			assertEquals("LSEG.O", recvReqMsg.name());
			
			assertEquals(2, consumerClient.queueSize());
			message = consumerClient.popMessage();
			
			StatusMsg statusMsg = (StatusMsg)message;
			
			assertEquals(5, statusMsg.streamId());
			assertEquals("DIRECT_FEED", statusMsg.serviceName());
			assertEquals("LSEG.O", statusMsg.name());
			assertEquals(OmmState.StreamState.OPEN, statusMsg.state().streamState());
			assertEquals(OmmState.DataState.SUSPECT, statusMsg.state().dataState());
			assertEquals(OmmState.StatusCode.NOT_AUTHORIZED, statusMsg.state().statusCode());
			assertEquals("Unauthorized access to the item.", statusMsg.state().statusText());
			
			message = consumerClient.popMessage();
			
			statusMsg = (StatusMsg)message;
			
			assertEquals(5, statusMsg.streamId());
			assertEquals("DIRECT_FEED", statusMsg.serviceName());
			assertEquals("LSEG.O", statusMsg.name());
			assertEquals(OmmState.StreamState.CLOSED, statusMsg.state().streamState());
			assertEquals(OmmState.DataState.SUSPECT, statusMsg.state().dataState());
			assertEquals(OmmState.StatusCode.NOT_AUTHORIZED, statusMsg.state().statusCode());
			assertEquals("Unauthorized access to the item.", statusMsg.state().statusText());
		}
		catch(OmmException excep)
		{
			assertFalse(true);
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
		finally {
			System.out.println("Uninitializing...");

			consumer.uninitialize();
			ommprovider.uninitialize();
			ommprovider2.uninitialize();
		}
	}
	
	@Test
	public void testRequestingSingleItemWithUnknownServiceNameAndServiceId()
	{
		TestUtilities.printTestHead("testRequestingSingleItemWithUnknownServiceNameAndServiceId","");

		String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";
		
		OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);
		
		ProviderTestOptions providerTestOptions = new ProviderTestOptions();
		
		ProviderTestClient providerClient1 = new ProviderTestClient(providerTestOptions);
		
		// Provider_1 provides the DIRECT_FEED service name
		OmmProvider ommprovider = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_1"), providerClient1);
		
		assertNotNull(ommprovider);
		
		ProviderTestClient providerClient2 = new ProviderTestClient(providerTestOptions);
		
		// Provider_1 provides the DIRECT_FEED service name
		OmmProvider ommprovider2 = EmaFactory.createOmmProvider(config.port("19004").providerName("Provider_1"), providerClient2);
		
		assertNotNull(ommprovider2);
		
		OmmConsumer consumer = null;
		ConsumerTestClient consumerClient = new ConsumerTestClient();
		
		try
		{
			consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_9"));
			
			ReqMsg reqMsg = EmaFactory.createReqMsg();
			
			long itemHandle1 = consumer.registerClient(reqMsg.clear().serviceName("UNKNOWN_SERVICE").name("LSEG.O"), consumerClient);
			
			Thread.sleep(2000);
			
			/* Ensure that the provider doesn't receive any request message */
			assertEquals(0, providerClient1.queueSize());
			
			assertEquals(1, consumerClient.queueSize());
			
			Msg message = consumerClient.popMessage();
			
			StatusMsg statusMsg = (StatusMsg)message;
			
			assertEquals("UNKNOWN_SERVICE", statusMsg.serviceName());
			assertEquals("LSEG.O", statusMsg.name());
			assertEquals(OmmState.StreamState.OPEN, statusMsg.state().streamState());
			assertEquals(OmmState.DataState.SUSPECT, statusMsg.state().dataState());
			assertEquals(OmmState.StatusCode.NONE, statusMsg.state().statusCode());
			assertEquals("No matching service present.", statusMsg.state().statusText());
			
			consumer.unregister(itemHandle1); // Users cancel this request.
			
			long itemHandle2 = consumer.registerClient(reqMsg.clear().serviceId(0).name("LSEG.O"), consumerClient);
			
			/* Ensure that the provider doesn't receive any request message */
			assertEquals(0, providerClient1.queueSize());
			
			Thread.sleep(2000);
			
			assertEquals(1, consumerClient.queueSize());
			
			message = consumerClient.popMessage();
			
			statusMsg = (StatusMsg)message;
			
			assertEquals(0, statusMsg.serviceId());
			assertEquals("LSEG.O", statusMsg.name());
			assertEquals(OmmState.StreamState.CLOSED, statusMsg.state().streamState());
			assertEquals(OmmState.DataState.SUSPECT, statusMsg.state().dataState());
			assertEquals(OmmState.StatusCode.NONE, statusMsg.state().statusCode());
			assertEquals("Service id of '0' is not found.", statusMsg.state().statusText());
			
			consumer.unregister(itemHandle2); // Users cancel this request.
		}
		catch(OmmException excep)
		{
			assertFalse(true);
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
		finally {
			System.out.println("Uninitializing...");

			consumer.uninitialize();
			ommprovider.uninitialize();
			ommprovider2.uninitialize();
		}
	}
	
	@Test
	public void testRequestingSingleItemWithNonExistenceServiceNameAndAddTheService()
	{
		TestUtilities.printTestHead("testRequestingSingleItemWithNonExistenceServiceNameAndAddTheService","");

		String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";
		
		OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);
		
		ProviderTestOptions providerTestOptions = new ProviderTestOptions();
		
		ProviderTestClient providerClient1 = new ProviderTestClient(providerTestOptions);
		
		// Provider_1 provides the DIRECT_FEED service name
		OmmProvider ommprovider = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_1"), providerClient1);
		
		assertNotNull(ommprovider);
		
		ProviderTestClient providerClient2 = new ProviderTestClient(providerTestOptions);
		
		// Provider_1 provides the DIRECT_FEED service name
		OmmProvider ommprovider2 = EmaFactory.createOmmProvider(config.port("19004").providerName("Provider_1"), providerClient2);
		
		assertNotNull(ommprovider2);
		
		OmmConsumer consumer = null;
		ConsumerTestClient consumerClient = new ConsumerTestClient();
		
		try
		{
			consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_9"));
			
			ReqMsg reqMsg = EmaFactory.createReqMsg();
			
			long itemHandle1 = consumer.registerClient(reqMsg.clear().serviceName("DIRECT_FEED2").name("LSEG.O"), consumerClient);
			
			long itemHandle2 = consumer.registerClient(reqMsg.clear().serviceName("DIRECT_FEED2").name("LSEG.L"), consumerClient);
			
			Thread.sleep(2000);
			
			/* Ensure that the provider doesn't receive any request message */
			assertEquals(0, providerClient1.queueSize());
			
			assertEquals(2, consumerClient.queueSize());
			
			Msg message = consumerClient.popMessage();
			
			StatusMsg statusMsg = (StatusMsg)message;
			
			assertEquals("DIRECT_FEED2", statusMsg.serviceName());
			assertEquals("LSEG.O", statusMsg.name());
			assertEquals(OmmState.StreamState.OPEN, statusMsg.state().streamState());
			assertEquals(OmmState.DataState.SUSPECT, statusMsg.state().dataState());
			assertEquals(OmmState.StatusCode.NONE, statusMsg.state().statusCode());
			assertEquals("No matching service present.", statusMsg.state().statusText());
			
			message = consumerClient.popMessage();
			
			statusMsg = (StatusMsg)message;
			
			assertEquals("DIRECT_FEED2", statusMsg.serviceName());
			assertEquals("LSEG.L", statusMsg.name());
			assertEquals(OmmState.StreamState.OPEN, statusMsg.state().streamState());
			assertEquals(OmmState.DataState.SUSPECT, statusMsg.state().dataState());
			assertEquals(OmmState.StatusCode.NONE, statusMsg.state().statusCode());
			assertEquals("No matching service present.", statusMsg.state().statusText());
			
			/* Provider send source directory update message to add the DIRECT_FEED2 service */
			OmmArray capablities = EmaFactory.createOmmArray();
			capablities.add(EmaFactory.createOmmArrayEntry().uintValue( EmaRdm.MMT_MARKET_PRICE));
			capablities.add(EmaFactory.createOmmArrayEntry().uintValue( EmaRdm.MMT_MARKET_BY_PRICE));
			OmmArray dictionaryUsed = EmaFactory.createOmmArray();
			dictionaryUsed.add(EmaFactory.createOmmArrayEntry().ascii( "RWFFld"));
			dictionaryUsed.add(EmaFactory.createOmmArrayEntry().ascii( "RWFEnum"));
	      
			ElementList serviceInfoId = EmaFactory.createElementList();    
	      
			serviceInfoId.add( EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_NAME, "DIRECT_FEED2"));     
			serviceInfoId.add( EmaFactory.createElementEntry().array(EmaRdm.ENAME_CAPABILITIES, capablities));         
			serviceInfoId.add( EmaFactory.createElementEntry().array(EmaRdm.ENAME_DICTIONARYS_USED, dictionaryUsed));

			ElementList serviceStateId = EmaFactory.createElementList();
			serviceStateId.add( EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_UP));
				
			FilterList filterList = EmaFactory.createFilterList();
			filterList.add( EmaFactory.createFilterEntry().elementList(EmaRdm.SERVICE_INFO_ID, FilterEntry.FilterAction.SET, serviceInfoId) );
			filterList.add( EmaFactory.createFilterEntry().elementList(EmaRdm.SERVICE_STATE_ID, FilterEntry.FilterAction.SET, serviceStateId));
	      
			Map map = EmaFactory.createMap();
			map.add( EmaFactory.createMapEntry().keyUInt(2, MapEntry.MapAction.ADD, filterList));
	      
			UpdateMsg updateMsg = EmaFactory.createUpdateMsg();
			ommprovider.submit( updateMsg.domainType(EmaRdm.MMT_DIRECTORY).
													filter( EmaRdm.SERVICE_INFO_FILTER | EmaRdm.SERVICE_STATE_FILTER).
													payload(map), 0);
			
			Thread.sleep(2000);
			
			/* Provider receives a request once the service is available */
			assertEquals(2, providerClient1.queueSize());
			
			message = providerClient1.popMessage();
			ReqMsg recvReqMsg = (ReqMsg)message;
			
			assertEquals(3, recvReqMsg.streamId());
			assertEquals(2, recvReqMsg.serviceId());
			assertEquals("DIRECT_FEED2", recvReqMsg.serviceName());
			assertEquals("LSEG.O", recvReqMsg.name());
			
			message = providerClient1.popMessage();
			recvReqMsg = (ReqMsg)message;
			
			assertEquals(4, recvReqMsg.streamId());
			assertEquals(2, recvReqMsg.serviceId());
			assertEquals("DIRECT_FEED2", recvReqMsg.serviceName());
			assertEquals("LSEG.L", recvReqMsg.name());
			
			Thread.sleep(2000);
			
			assertEquals(2, consumerClient.queueSize());
			message = consumerClient.popMessage();
			
			RefreshMsg refreshMsg = (RefreshMsg)message;
			
			assertEquals("DIRECT_FEED2", refreshMsg.serviceName());
			assertEquals("LSEG.O", refreshMsg.name());
			assertEquals(32768, refreshMsg.serviceId());
			assertEquals(OmmState.StreamState.OPEN, refreshMsg.state().streamState());
			assertEquals(OmmState.DataState.OK, refreshMsg.state().dataState());
			assertEquals(OmmState.StatusCode.NONE, refreshMsg.state().code());
			assertTrue(refreshMsg.solicited());
			assertTrue(refreshMsg.complete());
			
			message = consumerClient.popMessage();
			
			refreshMsg = (RefreshMsg)message;
			
			assertEquals("DIRECT_FEED2", refreshMsg.serviceName());
			assertEquals("LSEG.L", refreshMsg.name());
			assertEquals(32768, refreshMsg.serviceId());
			assertEquals(OmmState.StreamState.OPEN, refreshMsg.state().streamState());
			assertEquals(OmmState.DataState.OK, refreshMsg.state().dataState());
			assertEquals(OmmState.StatusCode.NONE, refreshMsg.state().code());
			assertTrue(refreshMsg.solicited());
			assertTrue(refreshMsg.complete());
			
			consumer.unregister(itemHandle1);
			consumer.unregister(itemHandle2);
		}
		catch(OmmException excep)
		{
			assertFalse(true);
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
		finally {
			System.out.println("Uninitializing...");

			consumer.uninitialize();
			ommprovider.uninitialize();
			ommprovider2.uninitialize();
		}
	}
	
	@Test
	public void testRequestingSingleItemAndRecoveringTheItemWithNoMatchingServiceStatus()
	{
		TestUtilities.printTestHead("testRequestingSingleItemAndRecoveringTheItemWithNoMatchingServiceStatus","");

		String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";
		
		OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);
		
		ProviderTestOptions providerTestOptions = new ProviderTestOptions();
		
		ProviderTestClient providerClient1 = new ProviderTestClient(providerTestOptions);
		
		// Provider_1 provides the DIRECT_FEED service name
		OmmProvider ommprovider = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_1"), providerClient1);
		
		assertNotNull(ommprovider);
		
		ProviderTestClient providerClient2 = new ProviderTestClient(providerTestOptions);
		
		// Provider_3 provides the DIRECT_FEED_2 service name
		OmmProvider ommprovider2 = EmaFactory.createOmmProvider(config.port("19004").providerName("Provider_3"), providerClient2);
		
		assertNotNull(ommprovider2);
		
		OmmConsumer consumer = null;
		ConsumerTestClient consumerClient = new ConsumerTestClient();
		
		try
		{
			consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_9"));
			
			ConsumerSession<OmmConsumerClient> consumerSession = ((OmmConsumerImpl)consumer).consumerSession();
			
			int serviceId = consumerSession.sessionDirectoryByName("DIRECT_FEED").service().serviceId();
			
			ReqMsg reqMsg = EmaFactory.createReqMsg();
			
			long itemHandle1 = consumer.registerClient(reqMsg.clear().serviceName("DIRECT_FEED").name("LSEG.O"), consumerClient);
			
			Thread.sleep(1000);
			
			/* Ensure that the provider receives the request message */
			assertEquals(1, providerClient1.queueSize());
			
			Msg message = providerClient1.popMessage();
			
			assertTrue(message instanceof ReqMsg);
			
			ReqMsg requestMsg = (ReqMsg)message;
			
			assertEquals( 3, requestMsg.streamId());
			assertEquals("DIRECT_FEED", requestMsg.serviceName());
			assertEquals("LSEG.O", requestMsg.name());
			assertEquals(1, requestMsg.serviceId());
			
			assertEquals(1, consumerClient.queueSize());
			
			message = consumerClient.popMessage();
			
			RefreshMsg refreshMsg = (RefreshMsg)message;
			
			assertEquals("DIRECT_FEED", refreshMsg.serviceName());
			assertEquals(serviceId, refreshMsg.serviceId());
			assertEquals("LSEG.O", refreshMsg.name());
			assertEquals(OmmState.StreamState.OPEN, refreshMsg.state().streamState());
			assertEquals(OmmState.DataState.OK, refreshMsg.state().dataState());
			assertEquals(OmmState.StatusCode.NONE, refreshMsg.state().code());
			
			/* The first server changes the service state to down and sends item closed recoverable status */
			ElementList serviceState = EmaFactory.createElementList();
			serviceState.add( EmaFactory.createElementEntry().uintValue( EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_DOWN ));
			serviceState.add( EmaFactory.createElementEntry().uintValue( EmaRdm.ENAME_ACCEPTING_REQS, 0 ));
			serviceState.add( EmaFactory.createElementEntry().state( EmaRdm.ENAME_STATUS, OmmState.StreamState.CLOSED_RECOVER, OmmState.DataState.SUSPECT, 
					OmmState.StatusCode.NONE, "Item temporary closed"));
			
			FilterList filterListEnc = EmaFactory.createFilterList();
			filterListEnc.add( EmaFactory.createFilterEntry().elementList( EmaRdm.SERVICE_STATE_ID, FilterEntry.FilterAction.UPDATE, serviceState ) );
	      
			Map map = EmaFactory.createMap();
			map.add( EmaFactory.createMapEntry().keyUInt( 1, MapEntry.MapAction.UPDATE, filterListEnc ));
	      
			UpdateMsg updateMsg = EmaFactory.createUpdateMsg();
			ommprovider.submit( updateMsg.domainType( EmaRdm.MMT_DIRECTORY ).
					filter( EmaRdm.SERVICE_STATE_FILTER ).
					payload( map ), 0);	// use 0 item handle to fan-out to all subscribers
			
			Thread.sleep(2000);
			
			assertEquals(2, consumerClient.queueSize());
			
			message = consumerClient.popMessage();
			StatusMsg statusMsg = (StatusMsg)message;
			
			assertEquals("DIRECT_FEED", statusMsg.serviceName());
			assertEquals(serviceId, statusMsg.serviceId());
			assertEquals("LSEG.O", statusMsg.name());
			assertEquals(OmmState.StreamState.OPEN, statusMsg.state().streamState());
			assertEquals(OmmState.DataState.SUSPECT, statusMsg.state().dataState());
			assertEquals(OmmState.StatusCode.NONE, statusMsg.state().statusCode());
			assertEquals("Item temporary closed", statusMsg.state().statusText());
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			
			assertEquals("DIRECT_FEED", statusMsg.serviceName());
			assertEquals(serviceId, statusMsg.serviceId());
			assertEquals("LSEG.O", statusMsg.name());
			assertEquals(OmmState.StreamState.OPEN, statusMsg.state().streamState());
			assertEquals(OmmState.DataState.SUSPECT, statusMsg.state().dataState());
			assertEquals(OmmState.StatusCode.NONE, statusMsg.state().statusCode());
			assertEquals("No matching service present.", statusMsg.state().statusText());
			
			
			/* The second provider send source directory update message to add the DIRECT_FEED service */
			OmmArray capablities = EmaFactory.createOmmArray();
			capablities.add(EmaFactory.createOmmArrayEntry().uintValue( EmaRdm.MMT_MARKET_PRICE));
			capablities.add(EmaFactory.createOmmArrayEntry().uintValue( EmaRdm.MMT_MARKET_BY_PRICE));
			OmmArray dictionaryUsed = EmaFactory.createOmmArray();
			dictionaryUsed.add(EmaFactory.createOmmArrayEntry().ascii( "RWFFld"));
			dictionaryUsed.add(EmaFactory.createOmmArrayEntry().ascii( "RWFEnum"));
			
			OmmArray qosList = EmaFactory.createOmmArray();
			qosList.add(EmaFactory.createOmmArrayEntry().qos(OmmQos.Timeliness.REALTIME, OmmQos.Rate.TICK_BY_TICK));
			qosList.add(EmaFactory.createOmmArrayEntry().qos(100, 100));
	      
			ElementList serviceInfoId = EmaFactory.createElementList();    
	      
			serviceInfoId.add( EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_NAME, "DIRECT_FEED"));     
			serviceInfoId.add( EmaFactory.createElementEntry().array(EmaRdm.ENAME_CAPABILITIES, capablities));         
			serviceInfoId.add( EmaFactory.createElementEntry().array(EmaRdm.ENAME_DICTIONARYS_USED, dictionaryUsed));
			serviceInfoId.add( EmaFactory.createElementEntry().array(EmaRdm.ENAME_QOS, qosList));
			serviceInfoId.add( EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_ITEM_LIST, "#.itemlist"));
			serviceInfoId.add( EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_SUPPS_QOS_RANGE, 0));

			ElementList serviceStateId = EmaFactory.createElementList();
			serviceStateId.add( EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_UP));
				
			FilterList filterList = EmaFactory.createFilterList();
			filterList.add( EmaFactory.createFilterEntry().elementList(EmaRdm.SERVICE_INFO_ID, FilterEntry.FilterAction.SET, serviceInfoId) );
			filterList.add( EmaFactory.createFilterEntry().elementList(EmaRdm.SERVICE_STATE_ID, FilterEntry.FilterAction.SET, serviceStateId));
	      
			map = EmaFactory.createMap();
			map.add( EmaFactory.createMapEntry().keyUInt(2, MapEntry.MapAction.ADD, filterList));
	      
			updateMsg = EmaFactory.createUpdateMsg();
			ommprovider2.submit( updateMsg.domainType(EmaRdm.MMT_DIRECTORY).
													filter( EmaRdm.SERVICE_INFO_FILTER | EmaRdm.SERVICE_STATE_FILTER).
													payload(map), 0);
			
			Thread.sleep(2000);
			
			/* The second provider receives a request once the service is available */
			assertEquals(1, providerClient2.queueSize());
			
			message = providerClient2.popMessage();
			ReqMsg recvReqMsg = (ReqMsg)message;
			
			assertEquals(3, recvReqMsg.streamId());
			assertEquals(2, recvReqMsg.serviceId());
			assertEquals("DIRECT_FEED", recvReqMsg.serviceName());
			assertEquals("LSEG.O", recvReqMsg.name());
			
			Thread.sleep(1000);
			
			assertEquals(1, consumerClient.queueSize());
			message = consumerClient.popMessage();
			
			refreshMsg = (RefreshMsg)message;
			
			assertEquals("DIRECT_FEED", refreshMsg.serviceName());
			assertEquals("LSEG.O", refreshMsg.name());
			assertEquals(serviceId, refreshMsg.serviceId());
			assertEquals(OmmState.StreamState.OPEN, refreshMsg.state().streamState());
			assertEquals(OmmState.DataState.OK, refreshMsg.state().dataState());
			assertEquals(OmmState.StatusCode.NONE, refreshMsg.state().code());
			assertTrue(refreshMsg.solicited());
			assertTrue(refreshMsg.complete());
			
			consumer.unregister(itemHandle1);
		}
		catch(OmmException excep)
		{
			System.out.println(excep);
			assertFalse(true);
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
		finally {
			System.out.println("Uninitializing...");

			consumer.uninitialize();
			ommprovider.uninitialize();
			ommprovider2.uninitialize();
		}
	}
	
	@Test
	public void testRequestingTheSameItemNameAndServiceName()
	{
		TestUtilities.printTestHead("testRequestingTheSameItemNameAndServiceName","");

		String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";
		
		OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);
		
		ProviderTestOptions providerTestOptions = new ProviderTestOptions();
		
		ProviderTestClient providerClient1 = new ProviderTestClient(providerTestOptions);
		
		// Provider_1 provides the DIRECT_FEED service name
		OmmProvider ommprovider = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_1"), providerClient1);
		
		assertNotNull(ommprovider);
		
		ProviderTestClient providerClient2 = new ProviderTestClient(providerTestOptions);
		
		// Provider_1 provides the DIRECT_FEED service name
		OmmProvider ommprovider2 = EmaFactory.createOmmProvider(config.port("19004").providerName("Provider_1"), providerClient2);
		
		assertNotNull(ommprovider2);
		
		OmmConsumer consumer = null;
		ConsumerTestClient consumerClient = new ConsumerTestClient();
		
		try
		{
			consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_9"));
			
			ReqMsg reqMsg = EmaFactory.createReqMsg();
			
			int serviceId = 32767;
			
			long itemHandle1 = consumer.registerClient(reqMsg.clear().serviceName("DIRECT_FEED").name("LSEG.O"), consumerClient);
			
			long itemHandle2 = consumer.registerClient(reqMsg.clear().serviceName("DIRECT_FEED").name("LSEG.O"), consumerClient);
			
			Thread.sleep(2000);
			
			assertEquals(1, providerClient1.queueSize());
			
			Msg message = providerClient1.popMessage();
			
			assertTrue(message instanceof ReqMsg);
			
			ReqMsg requestMsg = (ReqMsg)message;
			
			assertEquals("DIRECT_FEED", requestMsg.serviceName());
			assertEquals("LSEG.O", requestMsg.name());
			assertEquals(1, requestMsg.serviceId());
			
			Thread.sleep(2000);
			
			assertEquals(2, consumerClient.queueSize());
			
			message = consumerClient.popMessage();
			
			RefreshMsg refreshMsg = (RefreshMsg)message;
			
			assertEquals("DIRECT_FEED", refreshMsg.serviceName());
			assertEquals("LSEG.O", refreshMsg.name());
			assertEquals(serviceId, refreshMsg.serviceId());
			assertEquals(OmmState.StreamState.OPEN, refreshMsg.state().streamState());
			assertEquals(OmmState.DataState.OK, refreshMsg.state().dataState());
			assertEquals(OmmState.StatusCode.NONE, refreshMsg.state().code());
			
			message = consumerClient.popMessage();
			
			refreshMsg = (RefreshMsg)message;
			
			assertEquals("DIRECT_FEED", refreshMsg.serviceName());
			assertEquals("LSEG.O", refreshMsg.name());
			assertEquals(serviceId, refreshMsg.serviceId());
			assertEquals(OmmState.StreamState.OPEN, refreshMsg.state().streamState());
			assertEquals(OmmState.DataState.OK, refreshMsg.state().dataState());
			assertEquals(OmmState.StatusCode.NONE, refreshMsg.state().code());
			
			consumer.unregister(itemHandle1);
			consumer.unregister(itemHandle2);
		}
		catch(OmmException excep)
		{
			assertFalse(true);
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
		finally {
			System.out.println("Uninitializing...");

			consumer.uninitialize();
			ommprovider.uninitialize();
			ommprovider2.uninitialize();
		}
	}
	
	@Test
	public void testRequestingUnsupportedCapabilitiesByServiceNameAndServiceId()
	{
		TestUtilities.printTestHead("testRequestingUnsupportedCapabilitiesByServiceNameAndServiceId","");

		String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";
		
		OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);
		
		ProviderTestOptions providerTestOptions = new ProviderTestOptions();
		
		ProviderTestClient providerClient1 = new ProviderTestClient(providerTestOptions);
		
		// Provider_1 provides the DIRECT_FEED service name
		OmmProvider ommprovider = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_1"), providerClient1);
		
		assertNotNull(ommprovider);
		
		ProviderTestClient providerClient2 = new ProviderTestClient(providerTestOptions);
		
		// Provider_1 provides the DIRECT_FEED service name
		OmmProvider ommprovider2 = EmaFactory.createOmmProvider(config.port("19004").providerName("Provider_1"), providerClient2);
		
		assertNotNull(ommprovider2);
		
		OmmConsumer consumer = null;
		ConsumerTestClient consumerClient = new ConsumerTestClient();
		
		try
		{
			consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_9"));
			
			ReqMsg reqMsg = EmaFactory.createReqMsg();
			
			int domainType = 100;
			
			long itemHandle1 = consumer.registerClient(reqMsg.clear().serviceName("DIRECT_FEED").domainType(domainType).name("LSEG.O"), 
					consumerClient);
			
			Thread.sleep(1000);
			
			/* Ensure that the provider doesn't receive any request message */
			assertEquals(0, providerClient1.queueSize());
			
			assertEquals(1, consumerClient.queueSize());
			
			Msg message = consumerClient.popMessage();
			
			StatusMsg statusMsg = (StatusMsg)message;
			
			assertEquals("DIRECT_FEED", statusMsg.serviceName());
			assertEquals("LSEG.O", statusMsg.name());
			assertEquals(domainType, statusMsg.domainType());
			assertEquals(OmmState.StreamState.OPEN, statusMsg.state().streamState());
			assertEquals(OmmState.DataState.SUSPECT, statusMsg.state().dataState());
			assertEquals(OmmState.StatusCode.NONE, statusMsg.state().statusCode());
			assertEquals("Capability not supported", statusMsg.state().statusText());
			
			Thread.sleep(1000);
			
			consumer.unregister(itemHandle1);
			
			long itemHandle2 = consumer.registerClient(reqMsg.clear().serviceId(32767).domainType(domainType).name("LSEG.O"), 
					consumerClient);
			
			Thread.sleep(1000);
			
			/* Ensure that the provider doesn't receive any request message */
			assertEquals(0, providerClient1.queueSize());
			
			assertEquals(1, consumerClient.queueSize());
			
			message = consumerClient.popMessage();
			
			statusMsg = (StatusMsg)message;
			
			assertEquals("DIRECT_FEED", statusMsg.serviceName());
			assertEquals(32767, statusMsg.serviceId());
			assertEquals("LSEG.O", statusMsg.name());
			assertEquals(domainType, statusMsg.domainType());
			assertEquals(OmmState.StreamState.OPEN, statusMsg.state().streamState());
			assertEquals(OmmState.DataState.SUSPECT, statusMsg.state().dataState());
			assertEquals(OmmState.StatusCode.NONE, statusMsg.state().statusCode());
			assertEquals("Capability not supported", statusMsg.state().statusText());
			
			Thread.sleep(1000);
			
			consumer.unregister(itemHandle2);
		}
		catch(OmmException excep)
		{
			System.out.println(excep);
			assertFalse(true);
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
		finally {
			System.out.println("Uninitializing...");

			consumer.uninitialize();
			ommprovider.uninitialize();
			ommprovider2.uninitialize();
		}
	}
	
	@Test
	public void testRequestingWithSupportedCapabilitiesOnAnotherServer()
	{
		TestUtilities.printTestHead("testRequestingWithSupportedCapabilitiesOnAnotherServer","");

		String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";
		
		OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);
		
		ProviderTestOptions providerTestOptions = new ProviderTestOptions();
		
		ProviderTestClient providerClient1 = new ProviderTestClient(providerTestOptions);
		
		/* Source directory for this the first provider */
		OmmArray capablities = EmaFactory.createOmmArray();
		capablities.add(EmaFactory.createOmmArrayEntry().uintValue( EmaRdm.MMT_MARKET_BY_PRICE));
		OmmArray dictionaryUsed = EmaFactory.createOmmArray();
		dictionaryUsed.add(EmaFactory.createOmmArrayEntry().ascii( "RWFFld"));
		dictionaryUsed.add(EmaFactory.createOmmArrayEntry().ascii( "RWFEnum"));
      
		ElementList serviceInfoId = EmaFactory.createElementList();    
      
		serviceInfoId.add( EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_NAME, "DIRECT_FEED"));     
		serviceInfoId.add( EmaFactory.createElementEntry().array(EmaRdm.ENAME_CAPABILITIES, capablities));         
		serviceInfoId.add( EmaFactory.createElementEntry().array(EmaRdm.ENAME_DICTIONARYS_USED, dictionaryUsed));

		ElementList serviceStateId = EmaFactory.createElementList();
		serviceStateId.add( EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_UP));
			
		FilterList filterList = EmaFactory.createFilterList();
		filterList.add( EmaFactory.createFilterEntry().elementList(EmaRdm.SERVICE_INFO_ID, FilterEntry.FilterAction.SET, serviceInfoId) );
		filterList.add( EmaFactory.createFilterEntry().elementList(EmaRdm.SERVICE_STATE_ID, FilterEntry.FilterAction.SET, serviceStateId));
      
		Map map = EmaFactory.createMap();
		map.add( EmaFactory.createMapEntry().keyUInt(1, MapEntry.MapAction.ADD, filterList));
		
		providerTestOptions.sourceDirectoryPayload = map;
		
		// Provider_1 provides the DIRECT_FEED service name
		OmmProvider ommprovider = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_1")
				.adminControlDirectory(AdminControl.USER_CONTROL), providerClient1);
		
		assertNotNull(ommprovider);
		
		ProviderTestOptions providerTestOptions2 = new ProviderTestOptions();
		ProviderTestClient providerClient2 = new ProviderTestClient(providerTestOptions2);
		
		/* Source directory for this the first provider */
		OmmArray capablities2 = EmaFactory.createOmmArray();
		capablities2.add(EmaFactory.createOmmArrayEntry().uintValue( EmaRdm.MMT_MARKET_PRICE));
		capablities2.add(EmaFactory.createOmmArrayEntry().uintValue( EmaRdm.MMT_MARKET_BY_PRICE));
		OmmArray dictionaryUsed2 = EmaFactory.createOmmArray();
		dictionaryUsed2.add(EmaFactory.createOmmArrayEntry().ascii( "RWFFld"));
		dictionaryUsed2.add(EmaFactory.createOmmArrayEntry().ascii( "RWFEnum"));
      
		ElementList serviceInfoId2 = EmaFactory.createElementList();    
      
		serviceInfoId2.add( EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_NAME, "DIRECT_FEED"));     
		serviceInfoId2.add( EmaFactory.createElementEntry().array(EmaRdm.ENAME_CAPABILITIES, capablities2));         
		serviceInfoId2.add( EmaFactory.createElementEntry().array(EmaRdm.ENAME_DICTIONARYS_USED, dictionaryUsed2));

		ElementList serviceStateId2 = EmaFactory.createElementList();
		serviceStateId2.add( EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_UP));
			
		FilterList filterList2 = EmaFactory.createFilterList();
		filterList2.add( EmaFactory.createFilterEntry().elementList(EmaRdm.SERVICE_INFO_ID, FilterEntry.FilterAction.SET, serviceInfoId2) );
		filterList2.add( EmaFactory.createFilterEntry().elementList(EmaRdm.SERVICE_STATE_ID, FilterEntry.FilterAction.SET, serviceStateId2));
      
		Map map2 = EmaFactory.createMap();
		map2.add( EmaFactory.createMapEntry().keyUInt(1, MapEntry.MapAction.ADD, filterList2));
		
		providerTestOptions2.sourceDirectoryPayload = map2;
		
		// Provider_1 provides the DIRECT_FEED service name
		OmmProvider ommprovider2 = EmaFactory.createOmmProvider(config.port("19004").providerName("Provider_1")
				.adminControlDirectory(AdminControl.USER_CONTROL), providerClient2);
		
		assertNotNull(ommprovider2);
		
		OmmConsumer consumer = null;
		ConsumerTestClient consumerClient = new ConsumerTestClient();
		
		try
		{
			consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_9"));
			
			ReqMsg reqMsg = EmaFactory.createReqMsg();
			
			// The second provider provides the MARKET_PRICE domain 
			long itemHandle1 = consumer.registerClient(reqMsg.clear().serviceName("DIRECT_FEED").domainType(EmaRdm.MMT_MARKET_PRICE).name("LSEG.O"), 
					consumerClient);
			
			Thread.sleep(2000);
			
			/* Ensure that the first server doesn't receive the request message */
			assertEquals(0, providerClient1.queueSize());
			
			/* The second receives the request message as it provides the MARKET_PRICE domain type */
			assertEquals(1, providerClient2.queueSize());
			
			Msg message = providerClient2.popMessage();
			
			assertTrue(message instanceof ReqMsg);
			
			ReqMsg requestMsg = (ReqMsg)message;
			
			assertEquals("DIRECT_FEED", requestMsg.serviceName());
			assertEquals("LSEG.O", requestMsg.name());
			assertEquals(1, requestMsg.serviceId());
			
			Thread.sleep(2000);
			
			assertEquals(1, consumerClient.queueSize());
			
			message = consumerClient.popMessage();
			
			RefreshMsg refreshMsg = (RefreshMsg)message;
			
			assertEquals("DIRECT_FEED", refreshMsg.serviceName());
			assertEquals("LSEG.O", refreshMsg.name());
			assertEquals(32767, refreshMsg.serviceId());
			assertEquals(OmmState.StreamState.OPEN, refreshMsg.state().streamState());
			assertEquals(OmmState.DataState.OK, refreshMsg.state().dataState());
			assertEquals(OmmState.StatusCode.NONE, refreshMsg.state().code());
			
			Thread.sleep(1000);
			
			consumer.unregister(itemHandle1);
		}
		catch(OmmException excep)
		{
			System.out.println(excep);
			assertFalse(true);
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
		finally {
			System.out.println("Uninitializing...");

			consumer.uninitialize();
			ommprovider.uninitialize();
			ommprovider2.uninitialize();
		}
	}
	
	@Test
	public void testRequestingUnsupportedQos()
	{
		TestUtilities.printTestHead("testRequestingUnsupportedQos","");

		String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";
		
		OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);
		
		ProviderTestOptions providerTestOptions = new ProviderTestOptions();
		
		ProviderTestClient providerClient1 = new ProviderTestClient(providerTestOptions);
		
		// Provider_1 provides the DIRECT_FEED service name
		OmmProvider ommprovider = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_1"), providerClient1);
		
		assertNotNull(ommprovider);
		
		ProviderTestClient providerClient2 = new ProviderTestClient(providerTestOptions);
		
		// Provider_1 provides the DIRECT_FEED service name
		OmmProvider ommprovider2 = EmaFactory.createOmmProvider(config.port("19004").providerName("Provider_1"), providerClient2);
		
		assertNotNull(ommprovider2);
		
		OmmConsumer consumer = null;
		ConsumerTestClient consumerClient = new ConsumerTestClient();
		
		try
		{
			consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_9"));
			
			ReqMsg reqMsg = EmaFactory.createReqMsg();
			
			long itemHandle1 = consumer.registerClient(reqMsg.qos(OmmQos.Timeliness.INEXACT_DELAYED, OmmQos.Rate.JUST_IN_TIME_CONFLATED).serviceName("DIRECT_FEED").domainType(EmaRdm.MMT_MARKET_PRICE).name("LSEG.O"), 
					consumerClient);
			
			Thread.sleep(1000);
			
			/* Ensure that the provider doesn't receive any request message */
			assertEquals(0, providerClient1.queueSize());
			
			assertEquals(1, consumerClient.queueSize());
			
			Msg message = consumerClient.popMessage();
			
			StatusMsg statusMsg = (StatusMsg)message;
			
			assertEquals("DIRECT_FEED", statusMsg.serviceName());
			assertEquals("LSEG.O", statusMsg.name());
			assertEquals(OmmState.StreamState.OPEN, statusMsg.state().streamState());
			assertEquals(OmmState.DataState.SUSPECT, statusMsg.state().dataState());
			assertEquals(OmmState.StatusCode.NONE, statusMsg.state().statusCode());
			assertEquals("Service does not provide a matching QoS", statusMsg.state().statusText());
			
			Thread.sleep(1000);
			
			consumer.unregister(itemHandle1);
		}
		catch(OmmException excep)
		{
			assertFalse(true);
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
		finally {
			System.out.println("Uninitializing...");

			consumer.uninitialize();

			/* Ensure that there is no item status as the item is unregistered */
			assertEquals(0, consumerClient.queueSize());

			ommprovider.uninitialize();
			ommprovider2.uninitialize();
		}
	}
	
	@Test
	public void testRequestingUnsupportedQosByServiceNameAndServiceId()
	{
		TestUtilities.printTestHead("testRequestingUnsupportedQosByServiceNameAndServiceId","");

		String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";
		
		OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);
		
		ProviderTestOptions providerTestOptions = new ProviderTestOptions();
		
		ProviderTestClient providerClient1 = new ProviderTestClient(providerTestOptions);
		
		// Provider_1 provides the DIRECT_FEED service name
		OmmProvider ommprovider = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_1"), providerClient1);
		
		assertNotNull(ommprovider);
		
		ProviderTestClient providerClient2 = new ProviderTestClient(providerTestOptions);
		
		// Provider_1 provides the DIRECT_FEED service name
		OmmProvider ommprovider2 = EmaFactory.createOmmProvider(config.port("19004").providerName("Provider_1"), providerClient2);
		
		assertNotNull(ommprovider2);
		
		OmmConsumer consumer = null;
		ConsumerTestClient consumerClient = new ConsumerTestClient();
		
		try
		{
			consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_9"));
			
			ReqMsg reqMsg = EmaFactory.createReqMsg();
			
			long itemHandle1 = consumer.registerClient(reqMsg.qos(OmmQos.Timeliness.INEXACT_DELAYED, OmmQos.Rate.JUST_IN_TIME_CONFLATED).serviceName("DIRECT_FEED").domainType(EmaRdm.MMT_MARKET_PRICE).name("LSEG.O"), 
					consumerClient);
			
			Thread.sleep(1000);
			
			/* Ensure that the provider doesn't receive any request message */
			assertEquals(0, providerClient1.queueSize());
			
			assertEquals(1, consumerClient.queueSize());
			
			Msg message = consumerClient.popMessage();
			
			StatusMsg statusMsg = (StatusMsg)message;
			
			assertEquals("DIRECT_FEED", statusMsg.serviceName());
			assertEquals("LSEG.O", statusMsg.name());
			assertEquals(OmmState.StreamState.OPEN, statusMsg.state().streamState());
			assertEquals(OmmState.DataState.SUSPECT, statusMsg.state().dataState());
			assertEquals(OmmState.StatusCode.NONE, statusMsg.state().statusCode());
			assertEquals("Service does not provide a matching QoS", statusMsg.state().statusText());
			
			Thread.sleep(1000);
			
			consumer.unregister(itemHandle1);
			
			itemHandle1 = consumer.registerClient(reqMsg.qos(OmmQos.Timeliness.INEXACT_DELAYED, OmmQos.Rate.JUST_IN_TIME_CONFLATED).serviceId(32767).domainType(EmaRdm.MMT_MARKET_PRICE).name("LSEG.O"), 
					consumerClient);
			
			Thread.sleep(1000);
			
			/* Ensure that the provider doesn't receive any request message */
			assertEquals(0, providerClient1.queueSize());
			
			assertEquals(1, consumerClient.queueSize());
			
			message = consumerClient.popMessage();
			
			statusMsg = (StatusMsg)message;
			
			assertEquals("DIRECT_FEED", statusMsg.serviceName());
			assertEquals(32767, statusMsg.serviceId());
			assertEquals("LSEG.O", statusMsg.name());
			assertEquals(OmmState.StreamState.OPEN, statusMsg.state().streamState());
			assertEquals(OmmState.DataState.SUSPECT, statusMsg.state().dataState());
			assertEquals(OmmState.StatusCode.NONE, statusMsg.state().statusCode());
			assertEquals("Service does not provide a matching QoS", statusMsg.state().statusText());
		}
		catch(OmmException excep)
		{
			assertFalse(true);
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
		finally {
			System.out.println("Uninitializing...");

			consumer.uninitialize();
			ommprovider.uninitialize();
			ommprovider2.uninitialize();
		}
	}
	
	@Test
	public void testOmmConsumerItemRequestsWithServiceListNames() /* The request routing feature is not enabled for this test. */
	{
		TestUtilities.printTestHead("testOmmConsumerItemRequestsWithServiceListNames","");

		String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";
		
		OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);
		
		ProviderTestOptions providerTestOptions = new ProviderTestOptions();
		providerTestOptions.sendUpdateMessage = true;
		
		ProviderTestClient providerClient1 = new ProviderTestClient(providerTestOptions);
		
		// Provider_1 provides the DIRECT_FEED service name
		OmmProvider ommprovider = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_1"), providerClient1);
		
		assertNotNull(ommprovider);
		
		
		OmmConsumer consumer = null;
		ConsumerTestClient consumerClient = new ConsumerTestClient();
		
		try
		{
			ServiceList serviceList = EmaFactory.createServiceList("SVG1");
			
			serviceList.concreteServiceList().add("DIRECT_FEED");
			serviceList.concreteServiceList().add("DIRECT_FEED_2");
			
			consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_1").addServiceList(serviceList));
			
			ReqMsg reqMsg = EmaFactory.createReqMsg();
			
			long itemHandle = consumer.registerClient(reqMsg.clear().serviceListName("SVG1").name("LSEG.O"), consumerClient);
			
			Thread.sleep(2000);
			
			assertEquals(1, providerClient1.queueSize());
			
			Msg message = providerClient1.popMessage();
			
			assertTrue(message instanceof ReqMsg);
			
			ReqMsg requestMsg = (ReqMsg)message;
			
			assertEquals("DIRECT_FEED", requestMsg.serviceName());
			assertEquals("LSEG.O", requestMsg.name());
			assertEquals(1, requestMsg.serviceId());
			
			Thread.sleep(2000);
			
			assertEquals(2, consumerClient.queueSize());
			
			message = consumerClient.popMessage();
			
			RefreshMsg refreshMsg = (RefreshMsg)message;
			
			assertEquals("DIRECT_FEED", refreshMsg.serviceName());
			assertEquals(1, refreshMsg.serviceId());
			assertEquals("LSEG.O", refreshMsg.name());
			assertEquals(OmmState.StreamState.OPEN, refreshMsg.state().streamState());
			assertEquals(OmmState.DataState.OK, refreshMsg.state().dataState());
			assertEquals(OmmState.StatusCode.NONE, refreshMsg.state().code());
			
			message = consumerClient.popMessage();
			
			UpdateMsg updateMsg = (UpdateMsg)message;
			
			assertEquals("DIRECT_FEED", updateMsg.serviceName());
			assertEquals(1, updateMsg.serviceId());
			assertEquals("LSEG.O", updateMsg.name());
			
			consumer.unregister(itemHandle);
		}
		catch(OmmException excep)
		{
			System.out.println(excep);
			assertFalse(true);
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
		finally {
			System.out.println("Uninitializing...");

			consumer.uninitialize();
			ommprovider.uninitialize();
		}
	}
	
	@Test
	public void testSingleItemRecoverFromRequestTimeoutWithPrivateStream()
	{
		TestUtilities.printTestHead("testSingleItemRecoverFromRequestTimeoutWithPrivateStream","");

		String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";
		
		OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);
		
		ProviderTestOptions providerTestOptions = new ProviderTestOptions();
		
		providerTestOptions.sendItemResponse = false;
		ProviderTestClient providerClient1 = new ProviderTestClient(providerTestOptions);
		
		// Provider_1 provides the DIRECT_FEED service name
		OmmProvider ommprovider = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_1"), providerClient1);
		
		assertNotNull(ommprovider);
		
		ProviderTestOptions providerTestOptions2 = new ProviderTestOptions();
		ProviderTestClient providerClient2 = new ProviderTestClient(providerTestOptions2);
		
		// Provider_1 provides the DIRECT_FEED service name
		OmmProvider ommprovider2 = EmaFactory.createOmmProvider(config.port("19004").providerName("Provider_1"), providerClient2);
		
		assertNotNull(ommprovider2);
		
		OmmConsumer consumer = null;
		ConsumerTestClient consumerClient = new ConsumerTestClient();
		
		try
		{
			/* The item request timeout is set to 3 seconds for Consumer_9 */
			consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_9"));
			
			ReqMsg reqMsg = EmaFactory.createReqMsg();
			
			long itemHandle1 = consumer.registerClient(reqMsg.clear().serviceName("DIRECT_FEED").name("LSEG.O").privateStream(true), consumerClient);
			
			Thread.sleep(7000);
			
			/* Ensure that the provider receives a request message but it doesn't send a response back. */
			assertEquals(3, providerClient1.queueSize());
			
			Msg message = providerClient1.popMessage();
			ReqMsg recvReqMsg = (ReqMsg)message;
			
			assertEquals(3, recvReqMsg.streamId());
			assertEquals(1, recvReqMsg.serviceId());
			assertEquals("DIRECT_FEED", recvReqMsg.serviceName());
			assertEquals("LSEG.O", recvReqMsg.name());
			
			message = providerClient1.popMessage();
			recvReqMsg = (ReqMsg)message;
			
			/* This is close message from consumer */
			assertEquals(3, recvReqMsg.streamId());
			assertEquals(1, recvReqMsg.serviceId());
			assertEquals("DIRECT_FEED", recvReqMsg.serviceName());
			assertEquals("LSEG.O", recvReqMsg.name());
			
			message = providerClient1.popMessage();
			recvReqMsg = (ReqMsg)message;
			
			assertEquals(1, consumerClient.queueSize());
			message = consumerClient.popMessage();
			
			StatusMsg statusMsg = (StatusMsg)message;
			
			assertEquals(5, statusMsg.streamId());
			assertEquals("DIRECT_FEED", statusMsg.serviceName());
			assertEquals("LSEG.O", statusMsg.name());
			assertEquals(OmmState.StreamState.CLOSED_RECOVER, statusMsg.state().streamState());
			assertEquals(OmmState.DataState.SUSPECT, statusMsg.state().dataState());
			assertEquals(OmmState.StatusCode.NONE, statusMsg.state().statusCode());
			assertEquals("Request timeout", statusMsg.state().statusText());
			
			/* The second provider doesn't receive any request message */
			assertEquals(0, providerClient2.queueSize());

			consumer.unregister(itemHandle1);
		}
		catch(OmmException excep)
		{
			assertFalse(true);
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
		finally {
			System.out.println("Uninitializing...");

			consumer.uninitialize();
			ommprovider.uninitialize();
			ommprovider2.uninitialize();
		}
	}
	
	@Test
	public void testRequestingSingleItemWithRequestedServiceDownWithOpenSuspectStatusWithPrivateStream()
	{
		TestUtilities.printTestHead("testRequestingSingleItemWithRequestedServiceDownWithOpenSuspectStatusWithPrivateStream","");

		String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";
		
		OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);
		
		ProviderTestOptions providerTestOptions = new ProviderTestOptions();
		
		ProviderTestClient providerClient1 = new ProviderTestClient(providerTestOptions);
		
		providerTestOptions.itemGroupId = ByteBuffer.wrap("10".getBytes());
		
		// Provider_1 provides the DIRECT_FEED service name
		OmmProvider ommprovider = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_1"), providerClient1);
		
		assertNotNull(ommprovider);
		
		ProviderTestClient providerClient2 = new ProviderTestClient(providerTestOptions);
		
		// Provider_1 provides the DIRECT_FEED service name
		OmmProvider ommprovider2 = EmaFactory.createOmmProvider(config.port("19004").providerName("Provider_1"), providerClient2);
		
		assertNotNull(ommprovider2);
		
		OmmConsumer consumer = null;
		ConsumerTestClient consumerClient = new ConsumerTestClient();
		
		try
		{			
			consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_9"));
			
			ReqMsg reqMsg = EmaFactory.createReqMsg();
			
			long itemHandle = consumer.registerClient(reqMsg.clear().serviceName("DIRECT_FEED").name("LSEG.O").privateStream(true), consumerClient);
					
			Thread.sleep(3000);
			
			assertEquals(1, providerClient1.queueSize());
			
			Msg message = providerClient1.popMessage();
			
			assertTrue(message instanceof ReqMsg);
			
			ReqMsg requestMsg = (ReqMsg)message;
			
			assertEquals("DIRECT_FEED", requestMsg.serviceName());
			assertEquals("LSEG.O", requestMsg.name());
			assertEquals(1, requestMsg.serviceId());
			
			Thread.sleep(2000);
			
			assertEquals(1, consumerClient.queueSize());
			
			message = consumerClient.popMessage();
			
			RefreshMsg refreshMsg = (RefreshMsg)message;
			
			assertEquals("DIRECT_FEED", refreshMsg.serviceName());
			assertEquals("LSEG.O", refreshMsg.name());
			assertEquals(OmmState.StreamState.OPEN, refreshMsg.state().streamState());
			assertEquals(OmmState.DataState.OK, refreshMsg.state().dataState());
			assertEquals(OmmState.StatusCode.NONE, refreshMsg.state().code());
			
			/* The first provider sends a source directory update message to change service state to DOWN with service's status */			
			ElementList serviceState = EmaFactory.createElementList();
			serviceState.add( EmaFactory.createElementEntry().uintValue( EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_DOWN ));
			serviceState.add( EmaFactory.createElementEntry().uintValue( EmaRdm.ENAME_ACCEPTING_REQS, 1 ));
			serviceState.add( EmaFactory.createElementEntry().state( EmaRdm.ENAME_STATUS, OmmState.StreamState.OPEN, OmmState.DataState.SUSPECT, OmmState.StatusCode.NONE));
			
			FilterList filterListEnc = EmaFactory.createFilterList();
			filterListEnc.add( EmaFactory.createFilterEntry().elementList( EmaRdm.SERVICE_STATE_ID, FilterEntry.FilterAction.SET, serviceState ) );
	      
			Map map = EmaFactory.createMap();
			map.add( EmaFactory.createMapEntry().keyUInt( 1, MapEntry.MapAction.UPDATE, filterListEnc ));
	      
			UpdateMsg updateMsg = EmaFactory.createUpdateMsg();
			ommprovider.submit( updateMsg.domainType( EmaRdm.MMT_DIRECTORY ).
					filter( EmaRdm.SERVICE_STATE_FILTER ).
					payload( map ), 0);	// use 0 item handle to fan-out to all subscribers
			
			
			Thread.sleep(2000); // Wait until consumer receives the item Open/Suspect status message from the VA Watchlist
			
			/* Ensure that second provider doesn't receive the request message */
			assertEquals(0, providerClient2.queueSize());	
			assertEquals(1, consumerClient.queueSize());
			
			message = consumerClient.popMessage();
			
			StatusMsg statusMsg = (StatusMsg)message;
			
			assertEquals("DIRECT_FEED", statusMsg.serviceName());
			assertEquals("LSEG.O", statusMsg.name());
			assertEquals(OmmState.StreamState.OPEN, statusMsg.state().streamState());
			assertEquals(OmmState.DataState.SUSPECT, statusMsg.state().dataState());
			assertEquals(OmmState.StatusCode.NONE, statusMsg.state().statusCode());
			
			consumer.unregister(itemHandle);
		}
		catch(OmmException excep)
		{
			assertFalse(true);
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
		finally {
			System.out.println("Uninitializing...");

			consumer.uninitialize();
			ommprovider.uninitialize();
			ommprovider2.uninitialize();
		}
	}
	
	@Test
	public void testRequestingSingleItemWithRequestedConnectionDownWithDisableSessionEnhancedItemRecoveryWithPrivateStream()
	{
		TestUtilities.printTestHead("testRequestingSingleItemWithRequestedConnectionDownWithDisableSessionEnhancedItemRecoveryWithPrivateStream","");

		String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";
		
		OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);
		
		ProviderTestOptions providerTestOptions = new ProviderTestOptions();
		
		ProviderTestClient providerClient1 = new ProviderTestClient(providerTestOptions);
		
		providerTestOptions.itemGroupId = ByteBuffer.wrap("10".getBytes());
		
		// Provider_1 provides the DIRECT_FEED service name
		OmmProvider ommprovider = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_1"), providerClient1);
		
		assertNotNull(ommprovider);
		
		ProviderTestClient providerClient2 = new ProviderTestClient(providerTestOptions);
		
		// Provider_1 provides the DIRECT_FEED service name
		OmmProvider ommprovider2 = EmaFactory.createOmmProvider(config.port("19004").providerName("Provider_1"), providerClient2);
		
		assertNotNull(ommprovider2);
		
		OmmConsumer consumer = null;
		ConsumerTestClient consumerClient = new ConsumerTestClient();
		
		try
		{			
			consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_9"));
			
			ReqMsg reqMsg = EmaFactory.createReqMsg();
			
			long itemHandle = consumer.registerClient(reqMsg.clear().serviceName("DIRECT_FEED").name("LSEG.O").privateStream(true), consumerClient);
					
			Thread.sleep(1000);
			
			/* Checks provider that receives the item request. */
			Msg message;
			ReqMsg requestMsg = null;			
			
			message = providerClient1.popMessage();
			assertTrue(message instanceof ReqMsg);
			requestMsg = (ReqMsg)message;
			
			assertEquals("DIRECT_FEED", requestMsg.serviceName());
			assertEquals("LSEG.O", requestMsg.name());
			assertEquals(1, requestMsg.serviceId());
			
			Thread.sleep(1000);
			
			assertEquals(1, consumerClient.queueSize());
			
			message = consumerClient.popMessage();
			
			RefreshMsg refreshMsg = (RefreshMsg)message;
			
			assertEquals("DIRECT_FEED", refreshMsg.serviceName());
			assertEquals("LSEG.O", refreshMsg.name());
			assertEquals(OmmState.StreamState.OPEN, refreshMsg.state().streamState());
			assertEquals(OmmState.DataState.OK, refreshMsg.state().dataState());
			assertEquals(OmmState.StatusCode.NONE, refreshMsg.state().code());
			
			/* Force channel down on the first provider */
			ReqMsg recvReqMsg;
			ommprovider.uninitialize();
			
			Thread.sleep(1000);
			
			ommprovider = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_1"), providerClient1);
			
			Thread.sleep(2000);
			
			/* Ensures that the first provider receives the login request message  */
			assertEquals(1, providerClient1.queueSize());
			
			message = providerClient1.popMessage();
			recvReqMsg = (ReqMsg)message;
			
			assertEquals(1, recvReqMsg.streamId());
			assertEquals(EmaRdm.MMT_LOGIN, recvReqMsg.domainType());
			
			Thread.sleep(3000);
			
			/* Ensures that the first provider doesn't receive the request message */
			assertEquals(0, providerClient1.queueSize());
			
			assertEquals(1, consumerClient.queueSize());
			
			message = consumerClient.popMessage();
			
			StatusMsg statusMsg = (StatusMsg)message;
			
			assertEquals("DIRECT_FEED", statusMsg.serviceName());
			assertEquals("LSEG.O", statusMsg.name());
			assertEquals(OmmState.StreamState.CLOSED_RECOVER, statusMsg.state().streamState());
			assertEquals(OmmState.DataState.SUSPECT, statusMsg.state().dataState());
			assertEquals(OmmState.StatusCode.NONE, statusMsg.state().statusCode());
			assertEquals("channel down.", statusMsg.state().statusText());
			
			consumer.unregister(itemHandle);
		}
		catch(OmmException excep)
		{
			assertFalse(true);
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
		finally {
			System.out.println("Uninitializing...");

			consumer.uninitialize();
			ommprovider.uninitialize();
			ommprovider2.uninitialize();
		}
	}
	
	@Test
	public void testRequestingSingleItemWithRequestedConnectionDownWithEnableSessionEnhancedItemRecoveryWithPrivateStream()
	{
		TestUtilities.printTestHead("testRequestingSingleItemWithRequestedConnectionDownWithEnableSessionEnhancedItemRecoveryWithPrivateStream","");

		String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";
		
		OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);
		
		ProviderTestOptions providerTestOptions = new ProviderTestOptions();
		
		ProviderTestClient providerClient1 = new ProviderTestClient(providerTestOptions);
		
		providerTestOptions.itemGroupId = ByteBuffer.wrap("10".getBytes());
		
		// Provider_1 provides the DIRECT_FEED service name
		OmmProvider ommprovider = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_1"), providerClient1);
		
		assertNotNull(ommprovider);
		
		ProviderTestClient providerClient2 = new ProviderTestClient(providerTestOptions);
		
		// Provider_1 provides the DIRECT_FEED service name
		OmmProvider ommprovider2 = EmaFactory.createOmmProvider(config.port("19004").providerName("Provider_1"), providerClient2);
		
		assertNotNull(ommprovider2);
		
		OmmConsumer consumer = null;
		ConsumerTestClient consumerClient = new ConsumerTestClient();
		
		try
		{			
			consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_12"));
			
			ReqMsg reqMsg = EmaFactory.createReqMsg();
			
			long itemHandle = consumer.registerClient(reqMsg.clear().serviceName("DIRECT_FEED").name("LSEG.O").privateStream(true), consumerClient);
					
			Thread.sleep(3000);
			
			/* Checks provider that receives the item request. */
			Msg message;
			ReqMsg requestMsg = null;			
			
			message = providerClient1.popMessage();
			assertTrue(message instanceof ReqMsg);
			requestMsg = (ReqMsg)message;
			
			assertEquals("DIRECT_FEED", requestMsg.serviceName());
			assertEquals("LSEG.O", requestMsg.name());
			assertEquals(1, requestMsg.serviceId());
			
			Thread.sleep(2000);
			
			assertEquals(1, consumerClient.queueSize());
			
			message = consumerClient.popMessage();
			
			RefreshMsg refreshMsg = (RefreshMsg)message;
			
			assertEquals("DIRECT_FEED", refreshMsg.serviceName());
			assertEquals("LSEG.O", refreshMsg.name());
			assertEquals(OmmState.StreamState.OPEN, refreshMsg.state().streamState());
			assertEquals(OmmState.DataState.OK, refreshMsg.state().dataState());
			assertEquals(OmmState.StatusCode.NONE, refreshMsg.state().code());
			
			/* Force channel down on provider */
			ommprovider.uninitialize();
			
			Thread.sleep(2000);
		
			/* Ensure that the second provider doesn't receive the request message */
			assertEquals(0, providerClient2.queueSize());
			
			assertEquals(1, consumerClient.queueSize());
			
			message = consumerClient.popMessage();
			
			StatusMsg statusMsg = (StatusMsg)message;
			
			assertEquals("DIRECT_FEED", statusMsg.serviceName());
			assertEquals("LSEG.O", statusMsg.name());
			assertEquals(OmmState.StreamState.CLOSED_RECOVER, statusMsg.state().streamState());
			assertEquals(OmmState.DataState.SUSPECT, statusMsg.state().dataState());
			assertEquals(OmmState.StatusCode.NONE, statusMsg.state().statusCode());
			assertEquals("channel down.", statusMsg.state().statusText());
			
			consumer.unregister(itemHandle);
		}
		catch(OmmException excep)
		{
			assertFalse(true);
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
		finally {
			System.out.println("Uninitializing...");

			consumer.uninitialize();
			ommprovider.uninitialize();
			ommprovider2.uninitialize();
		}
	}
	
	@Test
	public void testRequestingSingleItemWithUnknownServiceNameAndServiceIdThenCloseConsumerSession()
	{
		TestUtilities.printTestHead("testRequestingSingleItemWithUnknownServiceNameAndServiceIdThenCloseConsumerSession","");

		String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";
		
		OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);
		
		ProviderTestOptions providerTestOptions = new ProviderTestOptions();
		
		ProviderTestClient providerClient1 = new ProviderTestClient(providerTestOptions);
		
		// Provider_1 provides the DIRECT_FEED service name
		OmmProvider ommprovider = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_1"), providerClient1);
		
		assertNotNull(ommprovider);
		
		ProviderTestClient providerClient2 = new ProviderTestClient(providerTestOptions);
		
		// Provider_1 provides the DIRECT_FEED service name
		OmmProvider ommprovider2 = EmaFactory.createOmmProvider(config.port("19004").providerName("Provider_1"), providerClient2);
		
		assertNotNull(ommprovider2);
		
		OmmConsumer consumer = null;
		ConsumerTestClient consumerClient = new ConsumerTestClient();
		
		try
		{
			ServiceList serviceList = EmaFactory.createServiceList("SVG1");
			
			serviceList.concreteServiceList().add("FEED_1");
			serviceList.concreteServiceList().add("FEED_2");
			
			consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_9").addServiceList(serviceList));
			
			ReqMsg reqMsg = EmaFactory.createReqMsg();
			
			consumer.registerClient(reqMsg.clear().serviceName("UNKNOWN_SERVICE").name("itemA"), consumerClient);
			
			consumer.registerClient(reqMsg.clear().serviceName("UNKNOWN_SERVICE").name("itemB"), consumerClient);
			
			consumer.registerClient(reqMsg.clear().serviceListName("SVG1").name("itemC"), consumerClient);
			
			Thread.sleep(2000);
			
			/* Ensure that the provider doesn't receive any request message */
			assertEquals(0, providerClient1.queueSize());
			
			assertEquals(3, consumerClient.queueSize());
			
			Msg message = consumerClient.popMessage();
			
			StatusMsg statusMsg = (StatusMsg)message;
			
			assertEquals("UNKNOWN_SERVICE", statusMsg.serviceName());
			assertEquals("itemA", statusMsg.name());
			assertEquals(OmmState.StreamState.OPEN, statusMsg.state().streamState());
			assertEquals(OmmState.DataState.SUSPECT, statusMsg.state().dataState());
			assertEquals(OmmState.StatusCode.NONE, statusMsg.state().statusCode());
			assertEquals("No matching service present.", statusMsg.state().statusText());
			
			message = consumerClient.popMessage();
			
			statusMsg = (StatusMsg)message;
			
			assertEquals("UNKNOWN_SERVICE", statusMsg.serviceName());
			assertEquals("itemB", statusMsg.name());
			assertEquals(OmmState.StreamState.OPEN, statusMsg.state().streamState());
			assertEquals(OmmState.DataState.SUSPECT, statusMsg.state().dataState());
			assertEquals(OmmState.StatusCode.NONE, statusMsg.state().statusCode());
			assertEquals("No matching service present.", statusMsg.state().statusText());
			
			message = consumerClient.popMessage();

			statusMsg = (StatusMsg)message;
			
			assertEquals("SVG1", statusMsg.serviceName());
			assertEquals("itemC", statusMsg.name());
			assertEquals(OmmState.StreamState.OPEN, statusMsg.state().streamState());
			assertEquals(OmmState.DataState.SUSPECT, statusMsg.state().dataState());
			assertEquals(OmmState.StatusCode.NONE, statusMsg.state().statusCode());
			assertEquals("No matching service present.", statusMsg.state().statusText());
			
			// Uninitialized OmmConsumer to get item closed status
			System.out.println("Uninitializing...");
			consumer.uninitialize();
			
			assertEquals(3, consumerClient.queueSize());
			
			message = consumerClient.popMessage();
			
			statusMsg = (StatusMsg)message;
			
			assertEquals("UNKNOWN_SERVICE", statusMsg.serviceName());
			assertEquals("itemA", statusMsg.name());
			assertEquals(OmmState.StreamState.CLOSED, statusMsg.state().streamState());
			assertEquals(OmmState.DataState.SUSPECT, statusMsg.state().dataState());
			assertEquals(OmmState.StatusCode.NONE, statusMsg.state().statusCode());
			assertEquals("Consumer session is closed.", statusMsg.state().statusText());
			
			message = consumerClient.popMessage();
			
			statusMsg = (StatusMsg)message;
			
			assertEquals("UNKNOWN_SERVICE", statusMsg.serviceName());
			assertEquals("itemB", statusMsg.name());
			assertEquals(OmmState.StreamState.CLOSED, statusMsg.state().streamState());
			assertEquals(OmmState.DataState.SUSPECT, statusMsg.state().dataState());
			assertEquals(OmmState.StatusCode.NONE, statusMsg.state().statusCode());
			assertEquals("Consumer session is closed.", statusMsg.state().statusText());
			
			message = consumerClient.popMessage();
			
			statusMsg = (StatusMsg)message;
			
			assertEquals("SVG1", statusMsg.serviceName());
			assertEquals("itemC", statusMsg.name());
			assertEquals(OmmState.StreamState.CLOSED, statusMsg.state().streamState());
			assertEquals(OmmState.DataState.SUSPECT, statusMsg.state().dataState());
			assertEquals(OmmState.StatusCode.NONE, statusMsg.state().statusCode());
			assertEquals("Consumer session is closed.", statusMsg.state().statusText());
			
		}
		catch(OmmException excep)
		{
			assertFalse(true);
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
		finally {
			ommprovider.uninitialize();
			ommprovider2.uninitialize();
		}
	}
	
	@Test
	public void testRequestingSingleItemWithRequestedConnectionDownWithDisableSessionEnhancedItemRecoveryThenCloseConsumerSession()
	{
		TestUtilities.printTestHead("testRequestingSingleItemWithRequestedConnectionDownWithDisableSessionEnhancedItemRecoveryThenCloseConsumerSession","");

		String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";
		
		OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);
		
		ProviderTestOptions providerTestOptions = new ProviderTestOptions();
		
		ProviderTestClient providerClient1 = new ProviderTestClient(providerTestOptions);
		
		providerTestOptions.itemGroupId = ByteBuffer.wrap("10".getBytes());
		
		// Provider_1 provides the DIRECT_FEED service name
		OmmProvider ommprovider = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_1"), providerClient1);
		
		assertNotNull(ommprovider);
		
		ProviderTestClient providerClient2 = new ProviderTestClient(providerTestOptions);
		
		// Provider_1 provides the DIRECT_FEED service name
		OmmProvider ommprovider2 = EmaFactory.createOmmProvider(config.port("19004").providerName("Provider_1"), providerClient2);
		
		assertNotNull(ommprovider2);
		
		OmmConsumer consumer = null;
		ConsumerTestClient consumerClient = new ConsumerTestClient();
		
		try
		{			
			ServiceList serviceList = EmaFactory.createServiceList("SVG1");
			
			serviceList.concreteServiceList().add("DIRECT_FEED");
			serviceList.concreteServiceList().add("FEED_2");
			
			consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_9").addServiceList(serviceList));
			
			
			ReqMsg reqMsg = EmaFactory.createReqMsg();
			
			consumer.registerClient(reqMsg.clear().serviceName("DIRECT_FEED").name("itemA"), consumerClient);
			
			consumer.registerClient(reqMsg.clear().serviceName("DIRECT_FEED").name("itemB"), consumerClient);
			
			consumer.registerClient(reqMsg.clear().serviceListName("SVG1").name("itemC"), consumerClient);
					
			Thread.sleep(2000);
			
			/* Checks provider that receives the item request. */
			Msg message;
			ReqMsg requestMsg = null;
			
			assertEquals(3, providerClient1.queueSize());
			
			message = providerClient1.popMessage();
			assertTrue(message instanceof ReqMsg);
			requestMsg = (ReqMsg)message;
			
			assertEquals("DIRECT_FEED", requestMsg.serviceName());
			assertEquals("itemA", requestMsg.name());
			assertEquals(1, requestMsg.serviceId());
			
			message = providerClient1.popMessage();
			assertTrue(message instanceof ReqMsg);
			requestMsg = (ReqMsg)message;
			
			assertEquals("DIRECT_FEED", requestMsg.serviceName());
			assertEquals("itemB", requestMsg.name());
			assertEquals(1, requestMsg.serviceId());
			
			message = providerClient1.popMessage();
			assertTrue(message instanceof ReqMsg);
			requestMsg = (ReqMsg)message;
			
			assertEquals("DIRECT_FEED", requestMsg.serviceName());
			assertEquals("itemC", requestMsg.name());
			assertEquals(1, requestMsg.serviceId());
			
			Thread.sleep(1000);
			
			assertEquals(3, consumerClient.queueSize());
			
			message = consumerClient.popMessage();
			
			RefreshMsg refreshMsg = (RefreshMsg)message;
			
			assertEquals("DIRECT_FEED", refreshMsg.serviceName());
			assertEquals("itemA", refreshMsg.name());
			assertEquals(OmmState.StreamState.OPEN, refreshMsg.state().streamState());
			assertEquals(OmmState.DataState.OK, refreshMsg.state().dataState());
			assertEquals(OmmState.StatusCode.NONE, refreshMsg.state().code());
			
			message = consumerClient.popMessage();
			
			refreshMsg = (RefreshMsg)message;
			
			assertEquals("DIRECT_FEED", refreshMsg.serviceName());
			assertEquals("itemB", refreshMsg.name());
			assertEquals(OmmState.StreamState.OPEN, refreshMsg.state().streamState());
			assertEquals(OmmState.DataState.OK, refreshMsg.state().dataState());
			assertEquals(OmmState.StatusCode.NONE, refreshMsg.state().code());
			
			message = consumerClient.popMessage();
			
			refreshMsg = (RefreshMsg)message;
			
			assertEquals("SVG1", refreshMsg.serviceName());
			assertEquals("itemC", refreshMsg.name());
			assertEquals(OmmState.StreamState.OPEN, refreshMsg.state().streamState());
			assertEquals(OmmState.DataState.OK, refreshMsg.state().dataState());
			assertEquals(OmmState.StatusCode.NONE, refreshMsg.state().code());
			
			/* Force channel down on the first provider */
			ommprovider.uninitialize();
			
			Thread.sleep(2000);
			
			System.out.println("Uninitializing...");
			consumer.uninitialize();
			
			StatusMsg statusMsg;
			
			assertEquals(6, consumerClient.queueSize());
			
			message = consumerClient.popMessage();
			
			statusMsg = (StatusMsg)message;
			
			assertEquals("DIRECT_FEED", statusMsg.serviceName());
			assertEquals("itemA", statusMsg.name());
			assertEquals(OmmState.StreamState.OPEN, statusMsg.state().streamState());
			assertEquals(OmmState.DataState.SUSPECT, statusMsg.state().dataState());
			assertEquals(OmmState.StatusCode.NONE, statusMsg.state().statusCode());
			assertEquals("channel down.", statusMsg.state().statusText());
			
			message = consumerClient.popMessage();
			
			statusMsg = (StatusMsg)message;
			
			assertEquals("DIRECT_FEED", statusMsg.serviceName());
			assertEquals("itemB", statusMsg.name());
			assertEquals(OmmState.StreamState.OPEN, statusMsg.state().streamState());
			assertEquals(OmmState.DataState.SUSPECT, statusMsg.state().dataState());
			assertEquals(OmmState.StatusCode.NONE, statusMsg.state().statusCode());
			assertEquals("channel down.", statusMsg.state().statusText());
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			
			assertEquals("SVG1", statusMsg.serviceName());
			assertEquals("itemC", statusMsg.name());
			assertEquals(OmmState.StreamState.OPEN, statusMsg.state().streamState());
			assertEquals(OmmState.DataState.SUSPECT, statusMsg.state().dataState());
			assertEquals(OmmState.StatusCode.NONE, statusMsg.state().statusCode());
			assertEquals("channel down.", statusMsg.state().statusText());
			
			message = consumerClient.popMessage();
			
			statusMsg = (StatusMsg)message;
			
			assertEquals("DIRECT_FEED", statusMsg.serviceName());
			assertEquals("itemA", statusMsg.name());
			assertEquals(OmmState.StreamState.CLOSED, statusMsg.state().streamState());
			assertEquals(OmmState.DataState.SUSPECT, statusMsg.state().dataState());
			assertEquals(OmmState.StatusCode.NONE, statusMsg.state().statusCode());
			assertEquals("Consumer session is closed.", statusMsg.state().statusText());
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			
			assertEquals("DIRECT_FEED", statusMsg.serviceName());
			assertEquals("itemB", statusMsg.name());
			assertEquals(OmmState.StreamState.CLOSED, statusMsg.state().streamState());
			assertEquals(OmmState.DataState.SUSPECT, statusMsg.state().dataState());
			assertEquals(OmmState.StatusCode.NONE, statusMsg.state().statusCode());
			assertEquals("Consumer session is closed.", statusMsg.state().statusText());
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			
			assertEquals("SVG1", statusMsg.serviceName());
			assertEquals("itemC", statusMsg.name());
			assertEquals(OmmState.StreamState.CLOSED, statusMsg.state().streamState());
			assertEquals(OmmState.DataState.SUSPECT, statusMsg.state().dataState());
			assertEquals(OmmState.StatusCode.NONE, statusMsg.state().statusCode());
			assertEquals("Consumer session is closed.", statusMsg.state().statusText());
			
		}
		catch(OmmException excep)
		{
			assertFalse(true);
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
		finally
		{
			ommprovider.uninitialize();
			ommprovider2.uninitialize();
		}
	}
	
	public void testRequestingTheSameItemNameAndServiceListName()
	{
		TestUtilities.printTestHead("testRequestingTheSameItemNameAndServiceListName","");

		String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";
		
		OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);
		
		ProviderTestOptions providerTestOptions = new ProviderTestOptions();
		
		ProviderTestClient providerClient1 = new ProviderTestClient(providerTestOptions);
		
		// Provider_1 provides the DIRECT_FEED service name
		OmmProvider ommprovider = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_1"), providerClient1);
		
		assertNotNull(ommprovider);
		
		ProviderTestClient providerClient2 = new ProviderTestClient(providerTestOptions);
		
		// Provider_3 provides the DIRECT_FEED_2 service name
		OmmProvider ommprovider2 = EmaFactory.createOmmProvider(config.port("19004").providerName("Provider_3"), providerClient2);
		
		assertNotNull(ommprovider2);
		
		OmmConsumer consumer = null;
		ConsumerTestClient consumerClient = new ConsumerTestClient();
		
		try
		{
			ServiceList serviceList = EmaFactory.createServiceList("SVG1");
			
			serviceList.concreteServiceList().add("DIRECT_FEED");
			serviceList.concreteServiceList().add("DIRECT_FEED_2");
			
			consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_9").addServiceList(serviceList));
			
			ReqMsg reqMsg = EmaFactory.createReqMsg();
			
			int serviceId = 32767;
			
			long itemHandle1 = consumer.registerClient(reqMsg.clear().serviceListName("SVG1").name("LSEG.O"), consumerClient);
			
			long itemHandle2 = consumer.registerClient(reqMsg.clear().serviceListName("SVG1").name("LSEG.O"), consumerClient);
			
			Thread.sleep(2000);
			
			assertEquals(1, providerClient1.queueSize());
			
			/* Ensure that the second provider doesn't receive the request */
			assertEquals(0, providerClient2.queueSize());
			
			Msg message = providerClient1.popMessage();
			
			assertTrue(message instanceof ReqMsg);
			
			ReqMsg requestMsg = (ReqMsg)message;
			
			assertEquals("DIRECT_FEED", requestMsg.serviceName());
			assertEquals("LSEG.O", requestMsg.name());
			assertEquals(1, requestMsg.serviceId());
			
			Thread.sleep(2000);
			
			assertEquals(2, consumerClient.queueSize());
			
			message = consumerClient.popMessage();
			
			RefreshMsg refreshMsg = (RefreshMsg)message;
			
			assertEquals("SVG1", refreshMsg.serviceName());
			assertEquals("LSEG.O", refreshMsg.name());
			assertEquals(serviceId, refreshMsg.serviceId());
			assertEquals(OmmState.StreamState.OPEN, refreshMsg.state().streamState());
			assertEquals(OmmState.DataState.OK, refreshMsg.state().dataState());
			assertEquals(OmmState.StatusCode.NONE, refreshMsg.state().code());
			assertTrue(refreshMsg.complete());
			
			message = consumerClient.popMessage();
			
			refreshMsg = (RefreshMsg)message;
			
			assertEquals("SVG1", refreshMsg.serviceName());
			assertEquals("LSEG.O", refreshMsg.name());
			assertEquals(serviceId, refreshMsg.serviceId());
			assertEquals(OmmState.StreamState.OPEN, refreshMsg.state().streamState());
			assertEquals(OmmState.DataState.OK, refreshMsg.state().dataState());
			assertEquals(OmmState.StatusCode.NONE, refreshMsg.state().code());
			assertTrue(refreshMsg.complete());
			
			consumer.unregister(itemHandle1);
			consumer.unregister(itemHandle2);
		}
		catch(OmmException excep)
		{
			assertFalse(true);
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
		finally {
			System.out.println("Uninitializing...");

			consumer.uninitialize();
			ommprovider.uninitialize();
			ommprovider2.uninitialize();
		}
	}
	
	@Test
	public void testRequestingSingleItemWithRequestedServiceListConnectionDownWithDisableSessionEnhancedItemRecovery()
	{
		TestUtilities.printTestHead("testRequestingSingleItemWithRequestedServiceListConnectionDownWithDisableSessionEnhancedItemRecovery","");

		String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";
		
		OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);
		
		ProviderTestOptions providerTestOptions = new ProviderTestOptions();
		
		ProviderTestClient providerClient1 = new ProviderTestClient(providerTestOptions);
		
		// Provider_5 provides the DIRECT_FEED and DIRECT_FEED_2 service names
		OmmProvider ommprovider = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_5"), providerClient1);
		
		assertNotNull(ommprovider);
		
		ProviderTestClient providerClient2 = new ProviderTestClient(providerTestOptions);
		
		// Provider_5 provides the DIRECT_FEED and DIRECT_FEED_2 service names
		OmmProvider ommprovider2 = EmaFactory.createOmmProvider(config.port("19004").providerName("Provider_5"), providerClient2);
		
		assertNotNull(ommprovider2);
		
		OmmConsumer consumer = null;
		ConsumerTestClient consumerClient = new ConsumerTestClient();
		
		try
		{	
			ServiceList serviceList = EmaFactory.createServiceList("SVG1");
			
			serviceList.concreteServiceList().add("DIRECT_FEED");
			serviceList.concreteServiceList().add("DIRECT_FEED_2");
			
			consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).addServiceList(serviceList) .consumerName("Consumer_9"));
			
			ReqMsg reqMsg = EmaFactory.createReqMsg();
			
			long itemHandle = consumer.registerClient(reqMsg.clear().serviceListName("SVG1").name("LSEG.O"), consumerClient);
					
			Thread.sleep(1000);
			
			/* Checks provider that receives the item request. */
			Msg message;
			ReqMsg requestMsg = null;			
			
			message = providerClient1.popMessage();
			assertTrue(message instanceof ReqMsg);
			requestMsg = (ReqMsg)message;
			
			assertEquals("DIRECT_FEED", requestMsg.serviceName());
			assertEquals("LSEG.O", requestMsg.name());
			assertEquals(1, requestMsg.serviceId());
			
			Thread.sleep(1000);
			
			assertEquals(1, consumerClient.queueSize());
			
			message = consumerClient.popMessage();
			
			RefreshMsg refreshMsg = (RefreshMsg)message;
			
			assertEquals("SVG1", refreshMsg.serviceName());
			assertEquals("LSEG.O", refreshMsg.name());
			assertEquals(32767, refreshMsg.serviceId());
			assertEquals(OmmState.StreamState.OPEN, refreshMsg.state().streamState());
			assertEquals(OmmState.DataState.OK, refreshMsg.state().dataState());
			assertEquals(OmmState.StatusCode.NONE, refreshMsg.state().code());
			
			/* Force channel down on the first provider */
			ReqMsg recvReqMsg;
			ommprovider.uninitialize();
			
			Thread.sleep(1000);
			
			ommprovider = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_1"), providerClient1);
			
			Thread.sleep(3000);
			
			/* The first provider receives the request message */
			assertEquals(1, providerClient1.queueSize());
			
			/* Ensure that the second provider doesn't receive any request message */
			assertEquals(0, providerClient2.queueSize());
			
			message = providerClient1.popMessage();
			recvReqMsg = (ReqMsg)message;
			
			assertEquals(1, recvReqMsg.streamId());
			assertEquals(EmaRdm.MMT_LOGIN, recvReqMsg.domainType());
			
			Thread.sleep(3000);
			assertEquals(1, providerClient1.queueSize());
			
			message = providerClient1.popMessage();
			recvReqMsg = (ReqMsg)message;
			
			assertEquals(4, recvReqMsg.streamId());
			assertEquals(1, recvReqMsg.serviceId());
			assertEquals("DIRECT_FEED", recvReqMsg.serviceName());
			assertEquals("LSEG.O", recvReqMsg.name());
			
			assertEquals(2, consumerClient.queueSize());
			
			message = consumerClient.popMessage();
			
			StatusMsg statusMsg = (StatusMsg)message;
			
			assertEquals("SVG1", statusMsg.serviceName());
			assertEquals("LSEG.O", statusMsg.name());
			assertEquals(OmmState.StreamState.OPEN, statusMsg.state().streamState());
			assertEquals(OmmState.DataState.SUSPECT, statusMsg.state().dataState());
			assertEquals(OmmState.StatusCode.NONE, statusMsg.state().statusCode());
			
			/* Receives refresh message from the first provider */
			assertEquals(1, consumerClient.queueSize());
			message = consumerClient.popMessage();
			
			refreshMsg = (RefreshMsg)message;
			
			assertEquals("SVG1", refreshMsg.serviceName());
			assertEquals("LSEG.O", refreshMsg.name());
			assertEquals(32767, refreshMsg.serviceId());
			assertEquals(OmmState.StreamState.OPEN, refreshMsg.state().streamState());
			assertEquals(OmmState.DataState.OK, refreshMsg.state().dataState());
			assertEquals(OmmState.StatusCode.NONE, refreshMsg.state().code());
			assertTrue(refreshMsg.solicited());
			assertTrue(refreshMsg.complete());
			
			consumer.unregister(itemHandle);
		}
		catch(OmmException excep)
		{
			assertFalse(true);
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
		finally {
			System.out.println("Uninitializing...");

			consumer.uninitialize();
			ommprovider.uninitialize();
			ommprovider2.uninitialize();
		}
	}
	
	@Test
	public void testGetSessionInformationFromOmmConsumer()
	{
		TestUtilities.printTestHead("testGetSessionInformationFromOmmConsumer","");

		String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";
		
		OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);
		
		ProviderTestOptions providerTestOptions = new ProviderTestOptions();
		
		ProviderTestClient providerClient1 = new ProviderTestClient(providerTestOptions);
		
		providerTestOptions.sendUpdateMessage = true;
		
		// Provider_1 provides the DIRECT_FEED service name
		OmmProvider ommprovider = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_1"), providerClient1);
		
		assertNotNull(ommprovider);
		
		ProviderTestClient providerClient2 = new ProviderTestClient(providerTestOptions);
		
		// Provider_3 provides the DIRECT_FEED_2 service name
		OmmProvider ommprovider2 = EmaFactory.createOmmProvider(config.port("19004").providerName("Provider_3"), providerClient2);
		
		assertNotNull(ommprovider2);
		
		OmmConsumer consumer = null;
		ConsumerTestOptions consumerTestOptions = new ConsumerTestOptions();
		
		consumerTestOptions.getChannelInformation = true;
		ConsumerTestClient consumerClient = new ConsumerTestClient(consumerTestOptions);
		
		try
		{
			consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_9"), consumerClient);
			
			ChannelInformation channelInformation = EmaFactory.createChannelInformation();
			
			OmmConsumer tempConsumer = consumer;
						
			Exception exception = assertThrows(OmmInvalidUsageException.class,  () -> tempConsumer.channelInformation(channelInformation));
			assertEquals("The request routing feature do not support the channelInformation method. The sessionChannelInfo() must be used instead.", exception.getMessage());
			
			List<ChannelInformation> channelList = new ArrayList<ChannelInformation>();
			consumer.sessionChannelInfo(channelList);
			
			assertEquals(2, channelList.size());
			ChannelInformation channelInfo = channelList.get(0);
			ChannelInformation channelInfo2 = channelList.get(1);
			
			assertEquals("Channel_1", channelInfo.channelName());
			assertEquals("Connection_1", channelInfo.sessionChannelName());
			assertEquals("not available for OmmConsumer connections", channelInfo.ipAddress());
			assertEquals(19001, channelInfo.port());
			assertEquals(ChannelState.ACTIVE, channelInfo.channelState());
			
			assertEquals("Channel_4", channelInfo2.channelName());
			assertEquals("Connection_2", channelInfo2.sessionChannelName());
			assertEquals(19004, channelInfo2.port());
			assertEquals(ChannelState.ACTIVE, channelInfo2.channelState());
			assertEquals("not available for OmmConsumer connections", channelInfo2.ipAddress());
			
			// Shutdown the first provider to remove the first session channel.
			ommprovider.uninitialize();
			
			// Wait to remove the session channel
			Thread.sleep(7000);
			
			// Gets channel info list again.
			consumer.sessionChannelInfo(channelList);
			
			assertEquals(1, channelList.size());
			
			channelInfo = channelList.get(0);
			assertEquals("Channel_4", channelInfo.channelName());
			assertEquals("Connection_2", channelInfo.sessionChannelName());
			assertEquals(19004, channelInfo.port());
			assertEquals(ChannelState.ACTIVE, channelInfo.channelState());
			assertEquals("not available for OmmConsumer connections", channelInfo.ipAddress());
		}
		catch(OmmException excep)
		{
			assertFalse(true);
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
		finally {
			System.out.println("Uninitializing...");

			consumer.uninitialize();
			ommprovider2.uninitialize();
		}
	}
	
	@Test
	public void testRequestingSingleItemWithServiceListReceivedCloseSuspectFromProvider()
	{
		TestUtilities.printTestHead("testRequestingSingleItemWithServiceListReceivedCloseSuspectFromProvider","");

		String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";
		
		OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);
		
		ProviderTestOptions providerTestOptions = new ProviderTestOptions();
		
		providerTestOptions.closeItemRequest = true;
		ProviderTestClient providerClient1 = new ProviderTestClient(providerTestOptions);
		
		// Provider_1 provides the DIRECT_FEED service name
		OmmProvider ommprovider = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_1"), providerClient1);
		
		assertNotNull(ommprovider);
		
		ProviderTestOptions providerTestOptions2 = new ProviderTestOptions();
		providerTestOptions2.closeItemRequest = true;
		ProviderTestClient providerClient2 = new ProviderTestClient(providerTestOptions2);
		
		// Provider_3 provides the DIRECT_FEED_2 service name
		OmmProvider ommprovider2 = EmaFactory.createOmmProvider(config.port("19004").providerName("Provider_3"), providerClient2);
		
		assertNotNull(ommprovider2);
		
		OmmConsumer consumer = null;
		ConsumerTestOptions consumerTestOptions = new ConsumerTestOptions();
		consumerTestOptions.getChannelInformation = true;
		ConsumerTestClient consumerClient = new ConsumerTestClient(consumerTestOptions);
		
		try
		{
			ServiceList serviceList = EmaFactory.createServiceList("SVG1");
			
			serviceList.concreteServiceList().add("DIRECT_FEED");
			serviceList.concreteServiceList().add("DIRECT_FEED_2");
			
			consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_9").addServiceList(serviceList));
			
			ReqMsg reqMsg = EmaFactory.createReqMsg();
			
			consumer.registerClient(reqMsg.clear().serviceListName("SVG1").name("LSEG.L"), consumerClient);
			
			Thread.sleep(2000);
			
			/* Ensure that the provider receives a request message but it closes the item stream. */
			assertEquals(1, providerClient1.queueSize());
			
			Msg message = providerClient1.popMessage();
			ReqMsg recvReqMsg = (ReqMsg)message;
			
			assertEquals(3, recvReqMsg.streamId());
			assertEquals(1, recvReqMsg.serviceId());
			assertEquals("DIRECT_FEED", recvReqMsg.serviceName());
			assertEquals("LSEG.L", recvReqMsg.name());
			
			Thread.sleep(1000);
			
			/* Ensure that the first provider doesn't receive a close message */
			assertEquals(0, providerClient1.queueSize());
				
			/* Ensure that the second provider receive a request message but it closes the item stream */
			assertEquals(1, providerClient2.queueSize());
			
			message = providerClient2.popMessage();
			recvReqMsg = (ReqMsg)message;
			
			assertEquals(3, recvReqMsg.streamId());
			assertEquals(1, recvReqMsg.serviceId());
			assertEquals("DIRECT_FEED_2", recvReqMsg.serviceName());
			assertEquals("LSEG.L", recvReqMsg.name());
			
			assertEquals(2, consumerClient.queueSize());
			message = consumerClient.popMessage();
			
			StatusMsg statusMsg = (StatusMsg)message;
			
			assertEquals(5, statusMsg.streamId());
			assertEquals("SVG1", statusMsg.serviceName());
			assertEquals("LSEG.L", statusMsg.name());
			assertEquals(OmmState.StreamState.OPEN, statusMsg.state().streamState());
			assertEquals(OmmState.DataState.SUSPECT, statusMsg.state().dataState());
			assertEquals(OmmState.StatusCode.NOT_AUTHORIZED, statusMsg.state().statusCode());
			assertEquals("Unauthorized access to the item.", statusMsg.state().statusText());
			
			message = consumerClient.popMessage();
			
			statusMsg = (StatusMsg)message;
			
			assertEquals(5, statusMsg.streamId());
			assertEquals("SVG1", statusMsg.serviceName());
			assertEquals("LSEG.L", statusMsg.name());
			assertEquals(OmmState.StreamState.CLOSED, statusMsg.state().streamState());
			assertEquals(OmmState.DataState.SUSPECT, statusMsg.state().dataState());
			assertEquals(OmmState.StatusCode.NOT_AUTHORIZED, statusMsg.state().statusCode());
			assertEquals("Unauthorized access to the item.", statusMsg.state().statusText());
		}
		catch(OmmException excep)
		{
			assertFalse(true);
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
		finally {
			System.out.println("Uninitializing...");

			consumer.uninitialize();
			ommprovider.uninitialize();
			ommprovider2.uninitialize();
		}
	}
	
	@Test
	public void testRequestingSingleItemWithNonExistenceServiceNameAndUnregisterRequestThenAddTheService()
	{
		TestUtilities.printTestHead("testRequestingSingleItemWithNonExistenceServiceNameAndUnregisterRequestThenAddTheService","");

		String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";
		
		OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);
		
		ProviderTestOptions providerTestOptions = new ProviderTestOptions();
		
		ProviderTestClient providerClient1 = new ProviderTestClient(providerTestOptions);
		
		// Provider_1 provides the DIRECT_FEED service name
		OmmProvider ommprovider = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_1"), providerClient1);
		
		assertNotNull(ommprovider);
		
		ProviderTestClient providerClient2 = new ProviderTestClient(providerTestOptions);
		
		// Provider_1 provides the DIRECT_FEED service name
		OmmProvider ommprovider2 = EmaFactory.createOmmProvider(config.port("19004").providerName("Provider_1"), providerClient2);
		
		assertNotNull(ommprovider2);
		
		OmmConsumer consumer = null;
		ConsumerTestClient consumerClient = new ConsumerTestClient();
		
		try
		{
			consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_9"));
			
			ReqMsg reqMsg = EmaFactory.createReqMsg();
			
			long itemHandle1 = consumer.registerClient(reqMsg.clear().serviceName("DIRECT_FEED2").name("LSEG.O"), consumerClient);
			
			Thread.sleep(2000);
			
			/* Ensure that the provider doesn't receive any request message */
			assertEquals(0, providerClient1.queueSize());
			
			assertEquals(1, consumerClient.queueSize());
			
			Msg message = consumerClient.popMessage();
			
			StatusMsg statusMsg = (StatusMsg)message;
			
			assertEquals("DIRECT_FEED2", statusMsg.serviceName());
			assertEquals("LSEG.O", statusMsg.name());
			assertEquals(OmmState.StreamState.OPEN, statusMsg.state().streamState());
			assertEquals(OmmState.DataState.SUSPECT, statusMsg.state().dataState());
			assertEquals(OmmState.StatusCode.NONE, statusMsg.state().statusCode());
			assertEquals("No matching service present.", statusMsg.state().statusText());
			
			/* Cancel the request to remove the item */
			consumer.unregister(itemHandle1);
			
			/* Provider send source directory update message to add the DIRECT_FEED2 service */
			OmmArray capablities = EmaFactory.createOmmArray();
			capablities.add(EmaFactory.createOmmArrayEntry().uintValue( EmaRdm.MMT_MARKET_PRICE));
			capablities.add(EmaFactory.createOmmArrayEntry().uintValue( EmaRdm.MMT_MARKET_BY_PRICE));
			OmmArray dictionaryUsed = EmaFactory.createOmmArray();
			dictionaryUsed.add(EmaFactory.createOmmArrayEntry().ascii( "RWFFld"));
			dictionaryUsed.add(EmaFactory.createOmmArrayEntry().ascii( "RWFEnum"));
	      
			ElementList serviceInfoId = EmaFactory.createElementList();    
	      
			serviceInfoId.add( EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_NAME, "DIRECT_FEED2"));     
			serviceInfoId.add( EmaFactory.createElementEntry().array(EmaRdm.ENAME_CAPABILITIES, capablities));         
			serviceInfoId.add( EmaFactory.createElementEntry().array(EmaRdm.ENAME_DICTIONARYS_USED, dictionaryUsed));

			ElementList serviceStateId = EmaFactory.createElementList();
			serviceStateId.add( EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_UP));
				
			FilterList filterList = EmaFactory.createFilterList();
			filterList.add( EmaFactory.createFilterEntry().elementList(EmaRdm.SERVICE_INFO_ID, FilterEntry.FilterAction.SET, serviceInfoId) );
			filterList.add( EmaFactory.createFilterEntry().elementList(EmaRdm.SERVICE_STATE_ID, FilterEntry.FilterAction.SET, serviceStateId));
	      
			Map map = EmaFactory.createMap();
			map.add( EmaFactory.createMapEntry().keyUInt(2, MapEntry.MapAction.ADD, filterList));
	      
			UpdateMsg updateMsg = EmaFactory.createUpdateMsg();
			ommprovider.submit( updateMsg.domainType(EmaRdm.MMT_DIRECTORY).
													filter( EmaRdm.SERVICE_INFO_FILTER | EmaRdm.SERVICE_STATE_FILTER).
													payload(map), 0);
			
			Thread.sleep(2000);
			
			/* There is no recover to the provider as the request is canceled */	
			assertEquals(0, providerClient1.queueSize());	
			assertEquals(0, consumerClient.queueSize());
		}
		catch(OmmException excep)
		{
			assertFalse(true);
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
		finally {
			System.out.println("Uninitializing...");

			consumer.uninitialize();
			ommprovider.uninitialize();
			ommprovider2.uninitialize();
		}
	}

	// change applicationId and applicationName, check that new values are
	// propagated to the login request message
	@Test
	public void testConsumerAppIdName() {
		TestUtilities.printTestHead("testConsumerAppIdName","");

		OmmConsumerConfig consumerConfig = EmaFactory.createOmmConsumerConfig();
		OmmConsumerConfigImpl config = (OmmConsumerConfigImpl) consumerConfig;

		// check that the default values are as expected
		String defaultAppId = "256";
		String defaultAppName = "ema";

		com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest loginReq = config.loginReq();

		assertNotNull(loginReq);
		assertTrue(loginReq.checkHasAttrib());
		assertTrue(loginReq.attrib().checkHasApplicationId());
		assertTrue(loginReq.attrib().checkHasApplicationName());

		String defaultAppIdCfg = loginReq.attrib().applicationId().toString();
		assertEquals(defaultAppId, defaultAppIdCfg);

		String defaultAppNameCfg = loginReq.attrib().applicationName().toString();
		assertEquals(defaultAppName, defaultAppNameCfg);

		// modify them via API calls
		String testAppId = "652";
		String testAppName = "test-app-name";

		config.applicationId(testAppId);
		config.applicationName(testAppName);

		String testAppIdCfg = loginReq.attrib().applicationId().toString();
		String testAppNameCfg = loginReq.attrib().applicationName().toString();

		assertEquals(testAppId, testAppIdCfg);
		assertEquals(testAppName, testAppNameCfg);
	}
	
	public static void checkFieldListFromRefreshMsg(FieldList fieldList)
	{
		Iterator<FieldEntry> it = fieldList.iterator();
		assertTrue(it.hasNext());
		FieldEntry fieldEntry = it.next();
		assertEquals(DataTypes.REAL, fieldEntry.loadType());
		assertEquals(22, fieldEntry.fieldId());
		assertTrue(it.hasNext());
		fieldEntry = it.next();
		assertEquals(DataTypes.REAL, fieldEntry.loadType());
		assertEquals(25, fieldEntry.fieldId());
		assertTrue(it.hasNext());
		fieldEntry = it.next();
		assertEquals(DataTypes.REAL, fieldEntry.loadType());
		assertEquals(30, fieldEntry.fieldId());
		assertTrue(it.hasNext());
		fieldEntry = it.next();
		assertEquals(DataTypes.REAL, fieldEntry.loadType());
		assertEquals(31, fieldEntry.fieldId());
		assertFalse(it.hasNext());
	}
	
	@Test
	public void testSingleItemWithWSBChannelsAndDownloadingDictionaryFromNetwork()
	{
        TestUtilities.printTestHead("testSingleItemWithWSBChannelsAndDownloadingDictionaryFromNetwork","");
		
		String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";
		
		OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);
		
		ProviderTestOptions providerTestOptions = new ProviderTestOptions();
		providerTestOptions.supportStandby = true;
		providerTestOptions.sendRefreshAttrib = true;
		
		ProviderTestClient providerClient = new ProviderTestClient(providerTestOptions);
		ProviderTestClient providerClient2 = new ProviderTestClient(providerTestOptions);
		ProviderTestClient providerClient3 = new ProviderTestClient(providerTestOptions);
		ProviderTestClient providerClient4 = new ProviderTestClient(providerTestOptions);
		
		/* WarmStandbyChannel_1 */
		// Provider_5 provides the DIRECT_FEED service name for WSB channel(starting server)
		OmmProvider ommprovider1 = EmaFactory.createOmmProvider(config.port("19003").providerName("Provider_5"), providerClient);
		
		assertNotNull(ommprovider1);
		
		// Provider_5 provides the DIRECT_FEED service name for WSB channel(standby server)
		OmmProvider ommprovider2 = EmaFactory.createOmmProvider(config.port("19006").providerName("Provider_5"), providerClient2);
		assertNotNull(ommprovider2);
		
		
		/* WarmStandbyChannel_2 */
		// Provider_5 provides the DIRECT_FEED service name for WSB channel(starting server)
		OmmProvider ommprovider3 = EmaFactory.createOmmProvider(config.port("19007").providerName("Provider_5"), providerClient3);
		
		assertNotNull(ommprovider3);
		
		// Provider_5 provides the DIRECT_FEED service name for WSB channel(standby server)
		OmmProvider ommprovider4 = EmaFactory.createOmmProvider(config.port("19008").providerName("Provider_5"), providerClient4);
		assertNotNull(ommprovider4);
		
		OmmConsumer consumer = null;
		ConsumerTestOptions consumerOption = new ConsumerTestOptions();
		
		consumerOption.getChannelInformation = true;
		consumerOption.getSessionChannelInfo = true;
		ConsumerTestClient consumerClient = new ConsumerTestClient(consumerOption);
		
		try
		{
			// Consumer_WSB_Channels is configured to download dictionary from network.
			consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_WSB_Channels"));
			
			ReqMsg reqMsg = EmaFactory.createReqMsg();
			
			long itemHandle = consumer.registerClient(reqMsg.name("IBM.N").serviceName("DIRECT_FEED"), consumerClient);
			
			Thread.sleep(4000);
			
			/* Receives one generic message and one request message of active server for the first connection */
			assertEquals(2, providerClient.queueSize());
			
			Msg message = providerClient.popMessage();
			
			/* GenericMsg for active server for the first connection */
			GenericMsg genericMsg = (GenericMsg)message;
			assertEquals(1, genericMsg.streamId());
			assertEquals(DomainTypes.LOGIN, genericMsg.domainType());
			assertEquals(EmaRdm.ENAME_CONS_CONN_STATUS, genericMsg.name());
			assertEquals(DataTypes.MAP, genericMsg.payload().dataType());
			
			Map map = genericMsg.payload().map();
			Iterator<MapEntry> mapIt = map.iterator();
			assertTrue(mapIt.hasNext());
			MapEntry mapEntry = mapIt.next();
			assertEquals(EmaRdm.ENAME_WARMSTANDBY_INFO, mapEntry.key().ascii().toString());
			assertEquals(DataTypes.ELEMENT_LIST, mapEntry.loadType());
			
			ElementList elementList = mapEntry.elementList();
			Iterator<ElementEntry> elementListIt = elementList.iterator();
			assertTrue(elementListIt.hasNext());
			ElementEntry elementEntry = elementListIt.next();
			assertEquals(EmaRdm.ENAME_WARMSTANDBY_MODE, elementEntry.name());
			assertEquals(ServerTypes.ACTIVE, elementEntry.uintValue());
			assertFalse(elementListIt.hasNext()); // End of ElementList
			
			assertFalse(mapIt.hasNext()); // End of Map
			
			/* Market price request message for active server for the first connection */
			message = providerClient.popMessage();
			ReqMsg recvReq = (ReqMsg)message;
			
			assertEquals(5, recvReq.streamId());
			assertEquals("DIRECT_FEED", recvReq.serviceName());
			assertEquals("IBM.N", recvReq.name());
			assertEquals(1, recvReq.serviceId());
			
			Thread.sleep(1000);
			
			/* Receives one generic message and one request message of standby server for the first connection */
			assertEquals(2, providerClient2.queueSize());
			/* GenericMsg for standby server for the first connection */
			message = providerClient2.popMessage();
			genericMsg = (GenericMsg)message;
			assertEquals(1, genericMsg.streamId());
			assertEquals(DomainTypes.LOGIN, genericMsg.domainType());
			assertEquals(EmaRdm.ENAME_CONS_CONN_STATUS, genericMsg.name());
			assertEquals(DataTypes.MAP, genericMsg.payload().dataType());
			
			map = genericMsg.payload().map();
			mapIt = map.iterator();
			assertTrue(mapIt.hasNext());
			mapEntry = mapIt.next();
			assertEquals(EmaRdm.ENAME_WARMSTANDBY_INFO, mapEntry.key().ascii().toString());
			assertEquals(DataTypes.ELEMENT_LIST, mapEntry.loadType());
			
			elementList = mapEntry.elementList();
			elementListIt = elementList.iterator();
			assertTrue(elementListIt.hasNext());
			elementEntry = elementListIt.next();
			assertEquals(EmaRdm.ENAME_WARMSTANDBY_MODE, elementEntry.name());
			assertEquals(ServerTypes.STANDBY, elementEntry.uintValue());
			assertFalse(elementListIt.hasNext()); // End of ElementList
			
			assertFalse(mapIt.hasNext()); // End of Map
			
			/* Market price request message for standby server for the first connection */
			message = providerClient2.popMessage();
			recvReq = (ReqMsg)message;
			
			assertEquals(5, recvReq.streamId());
			assertEquals("DIRECT_FEED", recvReq.serviceName());
			assertEquals("IBM.N", recvReq.name());
			assertEquals(1, recvReq.serviceId());
			
			/* Receives one refresh message from the active server of the first connection */
			assertEquals(1, consumerClient.queueSize());
			
			message = consumerClient.popMessage();
			
			assertTrue(message instanceof RefreshMsg);
			
			RefreshMsg refreshMsg = (RefreshMsg)message;
			
			assertEquals(5, refreshMsg.streamId());
			assertEquals(DomainTypes.MARKET_PRICE, refreshMsg.domainType());
			assertEquals("Open / Ok / None / 'Refresh Completed'", refreshMsg.state().toString());
			assertTrue(refreshMsg.solicited());
			assertTrue(refreshMsg.complete());
			assertTrue(refreshMsg.hasMsgKey());
			assertEquals(1, refreshMsg.serviceId());
			assertEquals("DIRECT_FEED", refreshMsg.serviceName());
			assertEquals("IBM.N", refreshMsg.name());
			assertEquals(DataTypes.FIELD_LIST, refreshMsg.payload().dataType());
			
			checkFieldListFromRefreshMsg(refreshMsg.payload().fieldList());
			
			assertEquals(1, consumerClient.channelInfoSize());
			ChannelInformation channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_3", channelInfo.channelName());
			
			
			/* Closes the active server for the first connection */
			ommprovider1.uninitialize();
			
			Thread.sleep(2000);
			
			/* Receives one generic message to promote the standby to active server for the first connection.*/
			assertEquals(1, providerClient2.queueSize());
			/* GenericMsg for standby server for the first connection */
			message = providerClient2.popMessage();
			genericMsg = (GenericMsg)message;
			assertEquals(1, genericMsg.streamId());
			assertEquals(DomainTypes.LOGIN, genericMsg.domainType());
			assertEquals(EmaRdm.ENAME_CONS_CONN_STATUS, genericMsg.name());
			assertEquals(DataTypes.MAP, genericMsg.payload().dataType());
			
			map = genericMsg.payload().map();
			mapIt = map.iterator();
			assertTrue(mapIt.hasNext());
			mapEntry = mapIt.next();
			assertEquals(EmaRdm.ENAME_WARMSTANDBY_INFO, mapEntry.key().ascii().toString());
			assertEquals(DataTypes.ELEMENT_LIST, mapEntry.loadType());
			
			elementList = mapEntry.elementList();
			elementListIt = elementList.iterator();
			assertTrue(elementListIt.hasNext());
			elementEntry = elementListIt.next();
			assertEquals(EmaRdm.ENAME_WARMSTANDBY_MODE, elementEntry.name());
			assertEquals(ServerTypes.ACTIVE, elementEntry.uintValue());
			assertFalse(elementListIt.hasNext()); // End of ElementList
			
			assertFalse(mapIt.hasNext()); // End of Map
			
			assertEquals(2, consumerClient.queueSize());
			
			message = consumerClient.popMessage();
			
			StatusMsg statusMsg = (StatusMsg)message;
			assertEquals(5, statusMsg.streamId());
			assertEquals("DIRECT_FEED", statusMsg.serviceName());
			assertEquals("IBM.N", statusMsg.name());
			assertEquals(OmmState.StreamState.OPEN, statusMsg.state().streamState());
			assertEquals(OmmState.DataState.SUSPECT, statusMsg.state().dataState());
			assertEquals(OmmState.StatusCode.NONE, statusMsg.state().statusCode());
			assertEquals("Service for this item was lost.", statusMsg.state().statusText());
			
			assertEquals(2, consumerClient.channelInfoSize());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_3", channelInfo.channelName());
			
			
			message = consumerClient.popMessage();
			refreshMsg = (RefreshMsg)message;
			
			assertEquals(5, refreshMsg.streamId());
			assertEquals(DomainTypes.MARKET_PRICE, refreshMsg.domainType());
			assertEquals("Open / Ok / None / 'Unsolicited Refresh Completed'", refreshMsg.state().toString());
			assertFalse(refreshMsg.solicited());
			assertTrue(refreshMsg.complete());
			assertTrue(refreshMsg.hasMsgKey());
			assertEquals(1, refreshMsg.serviceId());
			assertEquals("DIRECT_FEED", refreshMsg.serviceName());
			assertEquals("IBM.N", refreshMsg.name());
			assertEquals(DataTypes.FIELD_LIST, refreshMsg.payload().dataType());
			
			checkFieldListFromRefreshMsg(refreshMsg.payload().fieldList());
			
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_6", channelInfo.channelName());
			
			/* Closes the new active server for the first connection */
			ommprovider2.uninitialize();
			
			Thread.sleep(3000);
			
			assertEquals(2, providerClient3.queueSize());
			
			message = providerClient3.popMessage();
			/* GenericMsg for active server for the second connection */
			genericMsg = (GenericMsg)message;
			assertEquals(1, genericMsg.streamId());
			assertEquals(DomainTypes.LOGIN, genericMsg.domainType());
			assertEquals(EmaRdm.ENAME_CONS_CONN_STATUS, genericMsg.name());
			assertEquals(DataTypes.MAP, genericMsg.payload().dataType());
			
			map = genericMsg.payload().map();
			mapIt = map.iterator();
			assertTrue(mapIt.hasNext());
			mapEntry = mapIt.next();
			assertEquals(EmaRdm.ENAME_WARMSTANDBY_INFO, mapEntry.key().ascii().toString());
			assertEquals(DataTypes.ELEMENT_LIST, mapEntry.loadType());
			
			elementList = mapEntry.elementList();
			elementListIt = elementList.iterator();
			assertTrue(elementListIt.hasNext());
			elementEntry = elementListIt.next();
			assertEquals(EmaRdm.ENAME_WARMSTANDBY_MODE, elementEntry.name());
			assertEquals(ServerTypes.ACTIVE, elementEntry.uintValue());
			assertFalse(elementListIt.hasNext()); // End of ElementList
			
			assertFalse(mapIt.hasNext()); // End of Map
			
			/* Receives market price request message for active server for the second connection */
			message = providerClient3.popMessage();
			recvReq = (ReqMsg)message;
			
			assertEquals(6, recvReq.streamId());
			assertEquals("DIRECT_FEED", recvReq.serviceName());
			assertEquals("IBM.N", recvReq.name());
			assertEquals(1, recvReq.serviceId());
			
			assertEquals(2, providerClient4.queueSize());
			
			/* GenericMsg for standby server for the second connection */
			message = providerClient4.popMessage();
			genericMsg = (GenericMsg)message;
			assertEquals(1, genericMsg.streamId());
			assertEquals(DomainTypes.LOGIN, genericMsg.domainType());
			assertEquals(EmaRdm.ENAME_CONS_CONN_STATUS, genericMsg.name());
			assertEquals(DataTypes.MAP, genericMsg.payload().dataType());
			
			map = genericMsg.payload().map();
			mapIt = map.iterator();
			assertTrue(mapIt.hasNext());
			mapEntry = mapIt.next();
			assertEquals(EmaRdm.ENAME_WARMSTANDBY_INFO, mapEntry.key().ascii().toString());
			assertEquals(DataTypes.ELEMENT_LIST, mapEntry.loadType());
			
			elementList = mapEntry.elementList();
			elementListIt = elementList.iterator();
			assertTrue(elementListIt.hasNext());
			elementEntry = elementListIt.next();
			assertEquals(EmaRdm.ENAME_WARMSTANDBY_MODE, elementEntry.name());
			assertEquals(ServerTypes.STANDBY, elementEntry.uintValue());
			assertFalse(elementListIt.hasNext()); // End of ElementList
			assertFalse(mapIt.hasNext()); // End of Map
			
			/* Receives market price request message for standby server for the second connection */
			message = providerClient4.popMessage();
			recvReq = (ReqMsg)message;
			
			assertEquals(3, recvReq.streamId());
			assertEquals("DIRECT_FEED", recvReq.serviceName());
			assertEquals("IBM.N", recvReq.name());
			assertEquals(1, recvReq.serviceId());
			
			assertEquals(2, consumerClient.queueSize());
			
			/* Receives Status message as the first connection is down */
			message = consumerClient.popMessage();
			
			statusMsg = (StatusMsg)message;
			assertEquals(5, statusMsg.streamId());
			assertEquals("DIRECT_FEED", statusMsg.serviceName());
			assertEquals("IBM.N", statusMsg.name());
			assertEquals(OmmState.StreamState.OPEN, statusMsg.state().streamState());
			assertEquals(OmmState.DataState.SUSPECT, statusMsg.state().dataState());
			assertEquals(OmmState.StatusCode.NONE, statusMsg.state().statusCode());
			assertEquals("channel down.", statusMsg.state().statusText());
			
			assertEquals(2, consumerClient.channelInfoSize());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_6", channelInfo.channelName());
			
			message = consumerClient.popMessage();
			
			/* Receives Refresh message from the active server of the second connection */
			refreshMsg = (RefreshMsg)message;	
			assertEquals(5, refreshMsg.streamId());
			assertEquals(DomainTypes.MARKET_PRICE, refreshMsg.domainType());
			assertEquals("Open / Ok / None / 'Refresh Completed'", refreshMsg.state().toString());
			assertTrue(refreshMsg.solicited());
			assertTrue(refreshMsg.complete());
			assertTrue(refreshMsg.hasMsgKey());
			assertEquals(1, refreshMsg.serviceId());
			assertEquals("DIRECT_FEED", refreshMsg.serviceName());
			assertEquals("IBM.N", refreshMsg.name());
			assertEquals(DataTypes.FIELD_LIST, refreshMsg.payload().dataType());
			
			checkFieldListFromRefreshMsg(refreshMsg.payload().fieldList());
			
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_7", channelInfo.channelName());
			
			consumer.unregister(itemHandle);
		}
		catch(OmmException ex)
		{
			assertFalse(true);
		} catch (InterruptedException e) {

			e.printStackTrace();
		}
		finally
		{
			assertNotNull(consumer);
		
			System.out.println("Uninitilizing...");
		
			consumer.uninitialize();
			ommprovider3.uninitialize();
			ommprovider4.uninitialize();
		}
	}
	

	@Test
	public void testSingleConnectionFallBackToCurrentWSBGroupForServiceBasedTest()
	{
		TestUtilities.printTestHead("testSingleConnectionFallBackToCurrentWSBGroupForServiceBasedTest","");
		
		String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";
		
		OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);
		
		ProviderTestOptions providerTestOptions = new ProviderTestOptions();
		providerTestOptions.supportStandby = true;
		providerTestOptions.sendRefreshAttrib = true;
		providerTestOptions.itemGroupId = ByteBuffer.wrap("10".getBytes());
		
		ProviderTestClient providerClient = new ProviderTestClient(providerTestOptions);
		ProviderTestClient providerClient2 = new ProviderTestClient(providerTestOptions);
		ProviderTestClient providerClient3 = new ProviderTestClient(providerTestOptions);
		ProviderTestClient providerClient4 = new ProviderTestClient(providerTestOptions);
		
		/* WarmStandbyChannel_3 (preferred WSB channel) */
		// Provider_5 provides the DIRECT_FEED service name for WSB channel(starting server)
		OmmProvider ommprovider1 = EmaFactory.createOmmProvider(config.port("19003").providerName("Provider_5"), providerClient);
		
		// Provider_5 provides the DIRECT_FEED service name for WSB channel(standby server)
		OmmProvider ommprovider2 = EmaFactory.createOmmProvider(config.port("19006").providerName("Provider_5"), providerClient2);
		
		/* WarmStandbyChannel_7 (non preferred WSB channel) */
		// Provider_5 provides the DIRECT_FEED service name for WSB channel(starting server)
		OmmProvider ommprovider3 = EmaFactory.createOmmProvider(config.port("19007").providerName("Provider_5"), providerClient3);
		
		// Provider_5 provides the DIRECT_FEED service name for WSB channel(standby server)
		OmmProvider ommprovider4 = EmaFactory.createOmmProvider(config.port("19008").providerName("Provider_5"), providerClient4);
		
		OmmConsumer consumer = null;
		ConsumerTestOptions consumerOption = new ConsumerTestOptions();
		
		consumerOption.getChannelInformation = true;
		ConsumerTestClient consumerClient = new ConsumerTestClient(consumerOption);
		
		OmmProvider ommprovider5 = null;
		OmmProvider ommprovider6 = null;
		OmmProvider ommprovider7 = null;
		OmmProvider ommprovider8 = null;
		
		try
		{
			consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_23"), consumerClient);
			
			System.out.println("consumerClient.channelInfoSize() = "+ consumerClient.channelInfoSize());
			
			System.out.println("consumerClient.queueSize() = " + consumerClient.queueSize());
			
			String serviceName = "DIRECT_FEED";
			String itemName = "IBM.N";
			
			ReqMsg reqMsg = EmaFactory.createReqMsg();
			
			long itemHandle = consumer.registerClient(reqMsg.name(itemName).serviceName(serviceName), consumerClient);
			
			Thread.sleep(1000);
			
			/* Receives one generic message and one request message of active server for the first connection */
			assertEquals(2, providerClient.queueSize());
			
			Msg message = providerClient.popMessage();
			
			/* GenericMsg for active server for the first connection */
			GenericMsg genericMsg = (GenericMsg)message;
			assertEquals(2, genericMsg.streamId());
			assertEquals(DomainTypes.SOURCE, genericMsg.domainType());
			assertEquals(EmaRdm.ENAME_CONS_STATUS, genericMsg.name());
			assertEquals(DataTypes.MAP, genericMsg.payload().dataType());
			
			long serviceId = 1;
			
			Map map = genericMsg.payload().map();
			Iterator<MapEntry> mapIt = map.iterator();
			assertTrue(mapIt.hasNext());
			MapEntry mapEntry = mapIt.next();
			assertEquals(serviceId, mapEntry.key().uintValue());
			assertEquals(DataTypes.ELEMENT_LIST, mapEntry.loadType());
			
			ElementList elementList = mapEntry.elementList();
			Iterator<ElementEntry> elementListIt = elementList.iterator();
			assertTrue(elementListIt.hasNext());
			ElementEntry elementEntry = elementListIt.next();
			assertEquals(EmaRdm.ENAME_WARMSTANDBY_MODE, elementEntry.name());
			assertEquals(ServerTypes.ACTIVE, elementEntry.uintValue());
			assertFalse(elementListIt.hasNext()); // End of ElementList
			
			assertFalse(mapIt.hasNext()); // End of Map
			
			/* Market price request message for active server for the first connection */
			message = providerClient.popMessage();
			ReqMsg recvReq = (ReqMsg)message;
			
			assertEquals(3, recvReq.streamId());
			assertEquals("DIRECT_FEED", recvReq.serviceName());
			assertEquals("IBM.N", recvReq.name());
			assertEquals(1, recvReq.serviceId());
			
			Thread.sleep(1000);
			
			/* Checks the item refresh messages from the starting server of Connection_13 */
			message = consumerClient.popMessage();
			
			/* Checks login refresh message */
			RefreshMsg refreshMsg = (RefreshMsg)message;
			
			assertEquals(1, refreshMsg.streamId());
			assertEquals(DomainTypes.LOGIN, refreshMsg.domainType());
			assertEquals("Open / Ok / None / 'Login accepted'", refreshMsg.state().toString());
			assertTrue(refreshMsg.solicited());
			assertTrue(refreshMsg.complete());
			assertTrue(refreshMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, refreshMsg.payload().dataType());
			assertEquals(DataTypes.ELEMENT_LIST, refreshMsg.attrib().dataType());
			
			message = consumerClient.popMessage();
			
			/* Checks the market price item refresh */
			refreshMsg = (RefreshMsg)message;
			
			assertEquals(5, refreshMsg.streamId());
			assertEquals(DomainTypes.MARKET_PRICE, refreshMsg.domainType());
			assertEquals("Open / Ok / None / 'Refresh Completed'", refreshMsg.state().toString());
			assertTrue(refreshMsg.complete());
			assertTrue(refreshMsg.solicited());
			assertTrue(refreshMsg.hasName());
			assertEquals(itemName, refreshMsg.name());
			assertTrue(refreshMsg.hasServiceId());
			assertEquals(1, refreshMsg.serviceId());
			assertTrue(refreshMsg.hasServiceName());
			assertEquals(serviceName, refreshMsg.serviceName());
			assertEquals(DataTypes.FIELD_LIST, refreshMsg.payload().dataType());
			ChannelInformation channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_3", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			
			/* Receives one generic message and one request message of standby server for the first connection */
			assertEquals(2, providerClient2.queueSize());
			/* GenericMsg for standby server for the first connection */
			message = providerClient2.popMessage();
			genericMsg = (GenericMsg)message;
			assertEquals(2, genericMsg.streamId());
			assertEquals(DomainTypes.SOURCE, genericMsg.domainType());
			assertEquals(EmaRdm.ENAME_CONS_STATUS, genericMsg.name());
			assertEquals(DataTypes.MAP, genericMsg.payload().dataType());
			
			map = genericMsg.payload().map();
			mapIt = map.iterator();
			assertTrue(mapIt.hasNext());
			mapEntry = mapIt.next();
			assertEquals(serviceId, mapEntry.key().uintValue());
			assertEquals(DataTypes.ELEMENT_LIST, mapEntry.loadType());
			
			elementList = mapEntry.elementList();
			elementListIt = elementList.iterator();
			assertTrue(elementListIt.hasNext());
			elementEntry = elementListIt.next();
			assertEquals(EmaRdm.ENAME_WARMSTANDBY_MODE, elementEntry.name());
			assertEquals(ServerTypes.STANDBY, elementEntry.uintValue());
			assertFalse(elementListIt.hasNext()); // End of ElementList
			
			assertFalse(mapIt.hasNext()); // End of Map
			
			/* Market price request message for standby server for the first connection */
			message = providerClient2.popMessage();
			recvReq = (ReqMsg)message;
			
			assertEquals(3, recvReq.streamId());
			assertEquals("DIRECT_FEED", recvReq.serviceName());
			assertEquals("IBM.N", recvReq.name());
			assertEquals(1, recvReq.serviceId());

			Thread.sleep(1500);
			
			System.out.println("consumerClient.queueSize() " + consumerClient.queueSize() );
			
			/* There must not be any message as it falls back to the same WSB channel */
			assertTrue(consumerClient.queueSize() >= 2);
			
			// Checks for PH START and COMPLETE events
			message = consumerClient.popMessage();
			
			/* Checks login status messages */
			StatusMsg statusMsg = (StatusMsg)message;
			
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Ok / None / 'preferred host starting fallback'", statusMsg.state().toString());
			assertTrue(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			
			message = consumerClient.popMessage();
			
			/* Checks login refresh message */
			statusMsg = (StatusMsg)message;
			
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Ok / None / 'preferred host complete'", statusMsg.state().toString());
			assertTrue(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			
			consumer.unregister(itemHandle);
		}
		catch(OmmException ex)
		{
			assertFalse(true);
		} catch (InterruptedException e) {

			e.printStackTrace();
		}
		finally
		{
			assertNotNull(consumer);
		
			System.out.println("Uninitilizing...");
		
			consumer.uninitialize();
			ommprovider1.uninitialize();
			ommprovider2.uninitialize();
			ommprovider3.uninitialize();
			ommprovider4.uninitialize();
			
			if(ommprovider5 != null)
				ommprovider5.uninitialize();
			
			if(ommprovider6 != null)
				ommprovider6.uninitialize();
			
			if(ommprovider7 != null)
				ommprovider7.uninitialize();
			
			if(ommprovider8 != null)
				ommprovider8.uninitialize();
		}
	}

	@Test
	public void testSingleConnectWSBServiceBased_MovingFromOneGroupToAnother() /* Preferred host is not enabled for this case */
	{
		TestUtilities.printTestHead("testSingleConnectWSBServiceBased_MovingFromOneGroupToAnother","");
		
		String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";
		
		OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);
		
		ProviderTestOptions providerTestOptions = new ProviderTestOptions();
		providerTestOptions.supportStandby = true;
		providerTestOptions.sendRefreshAttrib = true;
		providerTestOptions.itemGroupId = ByteBuffer.wrap("10".getBytes());
		
		ProviderTestClient providerClient = new ProviderTestClient(providerTestOptions);
		ProviderTestClient providerClient2 = new ProviderTestClient(providerTestOptions);
		ProviderTestClient providerClient3 = new ProviderTestClient(providerTestOptions);
		ProviderTestClient providerClient4 = new ProviderTestClient(providerTestOptions);
		
		/* WarmStandbyChannel_3_1*/
		// Provider_7 provides the DIRECT_FEED and DIRECT_FEED1 service name for WSB channel(starting server)
		OmmProvider ommprovider1 = EmaFactory.createOmmProvider(config.port("19003").providerName("Provider_7"), providerClient);
		assertNotNull(ommprovider1);
		
		// Provider_7 provides the DIRECT_FEED and DIRECT_FEED1 service name for WSB channel(standby server)
		OmmProvider ommprovider2 = EmaFactory.createOmmProvider(config.port("19006").providerName("Provider_7"), providerClient2);
		assertNotNull(ommprovider2);
		
		/* WarmStandbyChannel_4_1 (preferred WSB channel) */
		// Provider_7 provides the DIRECT_FEED and DIRECT_FEED1 service name for WSB channel(starting server)
		OmmProvider ommprovider3 = EmaFactory.createOmmProvider(config.port("19007").providerName("Provider_8"), providerClient3);
		
		assertNotNull(ommprovider3);
		
		// Provider_7 provides the DIRECT_FEED and DIRECT_FEED1 service name for WSB channel(standby server)
		OmmProvider ommprovider4 = EmaFactory.createOmmProvider(config.port("19008").providerName("Provider_8"), providerClient4);
		assertNotNull(ommprovider4);
		
		OmmConsumer consumer = null;
		ConsumerTestOptions consumerOption = new ConsumerTestOptions();
		
		consumerOption.getChannelInformation = true;
		consumerOption.getSessionChannelInfo = false;
		ConsumerTestClient consumerClient = new ConsumerTestClient(consumerOption);
		
		try
		{
			consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_24"));
			
			String serviceName = "DIRECT_FEED";
			String itemName = "TRI.N";
			String serviceName2 = "DIRECT_FEED1";
			String itemName2 = "IBM.N";
			long DIRECT_FEED_SERVICE_ID = 1; // This is for the DIRECT_FEED service
			long DIRECT_FEED1_SERVICE_ID = 2; // This is for the DIRECT_FEED1 service
			
			ReqMsg reqMsg = EmaFactory.createReqMsg();

			long itemHandle2 = consumer.registerClient(reqMsg.name(itemName2).serviceName(serviceName2), consumerClient);
			long itemHandle = consumer.registerClient(reqMsg.name(itemName).serviceName(serviceName), consumerClient);

			Thread.sleep(2000);
			
			/* Receives two generic messages and two request messages of active server for the first connection */
			assertEquals(4, providerClient.queueSize());
			
			Msg message = providerClient.popMessage();
			
			/* GenericMsg of active server for the DIRECT_FEED service */
			GenericMsg genericMsg = (GenericMsg)message;
			assertEquals(2, genericMsg.streamId());
			assertEquals(DomainTypes.SOURCE, genericMsg.domainType());
			assertEquals(EmaRdm.ENAME_CONS_STATUS, genericMsg.name());
			assertEquals(DataTypes.MAP, genericMsg.payload().dataType());
			
			Map map = genericMsg.payload().map();
			Iterator<MapEntry> mapIt = map.iterator();
			assertTrue(mapIt.hasNext());
			MapEntry mapEntry = mapIt.next();
			assertEquals(DIRECT_FEED_SERVICE_ID, mapEntry.key().uintValue());
			assertEquals(DataTypes.ELEMENT_LIST, mapEntry.loadType());
			
			ElementList elementList = mapEntry.elementList();
			Iterator<ElementEntry> elementListIt = elementList.iterator();
			assertTrue(elementListIt.hasNext());
			ElementEntry elementEntry = elementListIt.next();
			assertEquals(EmaRdm.ENAME_WARMSTANDBY_MODE, elementEntry.name());
			assertEquals(ServerTypes.STANDBY, elementEntry.uintValue());
			assertFalse(elementListIt.hasNext()); // End of ElementList
			
			assertFalse(mapIt.hasNext()); // End of Map
			
			message = providerClient.popMessage();
			
			/* GenericMsg of active server for the DIRECT_FEED1 service */
			genericMsg = (GenericMsg)message;
			assertEquals(2, genericMsg.streamId());
			assertEquals(DomainTypes.SOURCE, genericMsg.domainType());
			assertEquals(EmaRdm.ENAME_CONS_STATUS, genericMsg.name());
			assertEquals(DataTypes.MAP, genericMsg.payload().dataType());
			
			map = genericMsg.payload().map();
			mapIt = map.iterator();
			assertTrue(mapIt.hasNext());
			mapEntry = mapIt.next();
			assertEquals(DIRECT_FEED1_SERVICE_ID, mapEntry.key().uintValue());
			assertEquals(DataTypes.ELEMENT_LIST, mapEntry.loadType());
			
			elementList = mapEntry.elementList();
			elementListIt = elementList.iterator();
			assertTrue(elementListIt.hasNext());
			elementEntry = elementListIt.next();
			assertEquals(EmaRdm.ENAME_WARMSTANDBY_MODE, elementEntry.name());
			assertEquals(ServerTypes.ACTIVE, elementEntry.uintValue());
			assertFalse(elementListIt.hasNext()); // End of ElementList
			
			assertFalse(mapIt.hasNext()); // End of Map
			
			/* Market price request message for active server for the first connection */
			message = providerClient.popMessage();
			ReqMsg recvReq = (ReqMsg)message;
			
			assertEquals(3, recvReq.streamId());
			assertEquals(serviceName2, recvReq.serviceName());
			assertEquals(itemName2, recvReq.name());
			assertEquals(2, recvReq.serviceId());
			
			message = providerClient.popMessage();
			recvReq = (ReqMsg)message;
			
			assertEquals(4, recvReq.streamId());
			assertEquals(serviceName, recvReq.serviceName());
			assertEquals(itemName, recvReq.name());
			assertEquals(1, recvReq.serviceId());
			
			Thread.sleep(1500);
			
			/* Checks the item refresh message from the starting server of WarmStandbyChannel_3_1 */
			message = consumerClient.popMessage();
			/* Checks the market price item refresh */
			RefreshMsg refreshMsg = (RefreshMsg)message;
			
			assertEquals(5, refreshMsg.streamId());
			assertEquals(DomainTypes.MARKET_PRICE, refreshMsg.domainType());
			assertEquals("Open / Ok / None / 'Refresh Completed'", refreshMsg.state().toString());
			assertTrue(refreshMsg.complete());
			assertTrue(refreshMsg.solicited());
			assertTrue(refreshMsg.hasName());
			assertEquals(itemName2, refreshMsg.name());
			assertTrue(refreshMsg.hasServiceId());
			assertEquals(DIRECT_FEED1_SERVICE_ID, refreshMsg.serviceId());
			assertTrue(refreshMsg.hasServiceName());
			assertEquals(serviceName2, refreshMsg.serviceName()); /* The active server provides "DIRECT_FEED1" as the active service */
			assertEquals(DataTypes.FIELD_LIST, refreshMsg.payload().dataType());
			ChannelInformation channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_3", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			
			/* Receives two generic messages and two request messages of standby server of WarmStandbyChannel_3_1 */
			assertEquals(4, providerClient2.queueSize());
			
			/* GenericMsg of standby server for the DIRECT_FEED service name */
			message = providerClient2.popMessage();
			genericMsg = (GenericMsg)message;
			assertEquals(2, genericMsg.streamId());
			assertEquals(DomainTypes.SOURCE, genericMsg.domainType());
			assertEquals(EmaRdm.ENAME_CONS_STATUS, genericMsg.name());
			assertEquals(DataTypes.MAP, genericMsg.payload().dataType());
			
			map = genericMsg.payload().map();
			mapIt = map.iterator();
			assertTrue(mapIt.hasNext());
			mapEntry = mapIt.next();
			assertEquals(DIRECT_FEED_SERVICE_ID, mapEntry.key().uintValue());
			assertEquals(DataTypes.ELEMENT_LIST, mapEntry.loadType());
			
			elementList = mapEntry.elementList();
			elementListIt = elementList.iterator();
			assertTrue(elementListIt.hasNext());
			elementEntry = elementListIt.next();
			assertEquals(EmaRdm.ENAME_WARMSTANDBY_MODE, elementEntry.name());
			assertEquals(ServerTypes.ACTIVE, elementEntry.uintValue());
			assertFalse(elementListIt.hasNext()); // End of ElementList
			
			assertFalse(mapIt.hasNext()); // End of Map
			
			/* GenericMsg of standby server for the DIRECT_FEED1 service name */
			message = providerClient2.popMessage();
			genericMsg = (GenericMsg)message;
			assertEquals(2, genericMsg.streamId());
			assertEquals(DomainTypes.SOURCE, genericMsg.domainType());
			assertEquals(EmaRdm.ENAME_CONS_STATUS, genericMsg.name());
			assertEquals(DataTypes.MAP, genericMsg.payload().dataType());
			
			map = genericMsg.payload().map();
			mapIt = map.iterator();
			assertTrue(mapIt.hasNext());
			mapEntry = mapIt.next();
			assertEquals(DIRECT_FEED1_SERVICE_ID, mapEntry.key().uintValue());
			assertEquals(DataTypes.ELEMENT_LIST, mapEntry.loadType());
			
			elementList = mapEntry.elementList();
			elementListIt = elementList.iterator();
			assertTrue(elementListIt.hasNext());
			elementEntry = elementListIt.next();
			assertEquals(EmaRdm.ENAME_WARMSTANDBY_MODE, elementEntry.name());
			assertEquals(ServerTypes.STANDBY, elementEntry.uintValue());
			assertFalse(elementListIt.hasNext()); // End of ElementList
			
			assertFalse(mapIt.hasNext()); // End of Map
			
			/* Market price request message for standby server for the first connection */
			message = providerClient2.popMessage();
			recvReq = (ReqMsg)message;
			
			assertEquals(3, recvReq.streamId());
			assertEquals(serviceName2, recvReq.serviceName());
			assertEquals(itemName2, recvReq.name());
			assertEquals(2, recvReq.serviceId());
			
			message = providerClient2.popMessage();
			recvReq = (ReqMsg)message;
			
			assertEquals(4, recvReq.streamId());
			assertEquals(serviceName, recvReq.serviceName());
			assertEquals(itemName, recvReq.name());
			assertEquals(1, recvReq.serviceId());
			
			Thread.sleep(1000);
			
			/* Checks the item refresh message from the standby server of WarmStandbyChannel_3_1 */
			message = consumerClient.popMessage();
			
			/* Checks the market price item refresh */
			refreshMsg = (RefreshMsg)message;
			
			assertEquals(6, refreshMsg.streamId());
			assertEquals(DomainTypes.MARKET_PRICE, refreshMsg.domainType());
			assertEquals("Open / Ok / None / 'Refresh Completed'", refreshMsg.state().toString());
			assertTrue(refreshMsg.complete());
			assertTrue(refreshMsg.solicited());
			assertTrue(refreshMsg.hasName());
			assertEquals(itemName, refreshMsg.name());
			assertTrue(refreshMsg.hasServiceId());
			assertEquals(DIRECT_FEED_SERVICE_ID, refreshMsg.serviceId());
			assertTrue(refreshMsg.hasServiceName());
			assertEquals(serviceName, refreshMsg.serviceName());  /* This standby server provides "DIRECT_FEED" as the active service */
			assertEquals(DataTypes.FIELD_LIST, refreshMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_6", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());

			System.out.println("Closing the providerd from WarmStandbyChannel_3_1");
			/* Kill Provider1 and Provider2 to switch over to the WarmStandbyChannel_4_1 */
			ommprovider1.uninitialize();
			Thread.sleep(1000); // Wait 1 second to switch over to the standby server.
			ommprovider2.uninitialize();
			
			/* Waits for 3 seconds to move to  WarmStandbyChannel_4_1 */
			Thread.sleep(2500);
			
			/* Checks the item status message when fallback process is trigger */
			System.out.println("consumerClient.queueSize() " + consumerClient.queueSize() );
			assertEquals(6, consumerClient.queueSize());
			
			message = consumerClient.popMessage();
			StatusMsg statusMsg = (StatusMsg)message;
			
			assertEquals(5, statusMsg.streamId());
			assertEquals(DomainTypes.MARKET_PRICE, statusMsg.domainType());
			assertTrue(statusMsg.hasState());
			assertEquals("Open / Suspect / None / 'Service for this item was lost.'", statusMsg.state().toString());
			assertTrue(statusMsg.hasName());
			assertEquals(itemName2, statusMsg.name());
			assertTrue(statusMsg.hasServiceId());
			assertEquals(2, statusMsg.serviceId());
			assertTrue(statusMsg.hasServiceName());
			assertEquals(serviceName2, statusMsg.serviceName());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_3", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.CLOSED, channelInfo.channelState());
			
			message = consumerClient.popMessage();
			refreshMsg = (RefreshMsg)message;
			
			assertEquals(5, refreshMsg.streamId());
			assertEquals(DomainTypes.MARKET_PRICE, refreshMsg.domainType());
			assertEquals("Open / Ok / None / 'Unsolicited Refresh Completed'", refreshMsg.state().toString());
			assertFalse(refreshMsg.solicited());
			assertTrue(refreshMsg.complete());
			assertTrue(refreshMsg.hasMsgKey());
			assertTrue(refreshMsg.hasName());
			assertEquals(itemName2, refreshMsg.name());
			assertTrue(refreshMsg.hasServiceId());
			assertTrue(refreshMsg.hasServiceName());
			assertEquals(serviceName2, refreshMsg.serviceName());
			assertEquals(DataTypes.FIELD_LIST, refreshMsg.payload().dataType());
			checkFieldListFromRefreshMsg(refreshMsg.payload().fieldList());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_6", channelInfo.channelName());
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			
			assertEquals(6, statusMsg.streamId());
			assertEquals(DomainTypes.MARKET_PRICE, statusMsg.domainType());
			assertTrue(statusMsg.hasState());
			assertEquals("Open / Suspect / None / 'channel down.'", statusMsg.state().toString());
			assertTrue(statusMsg.hasName());
			assertEquals(itemName, statusMsg.name());
			assertTrue(statusMsg.hasServiceId());
			assertEquals(1, statusMsg.serviceId());
			assertTrue(statusMsg.hasServiceName());
			assertEquals(serviceName, statusMsg.serviceName());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_6", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.CLOSED, channelInfo.channelState());
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			
			assertEquals(5, statusMsg.streamId());
			assertEquals(DomainTypes.MARKET_PRICE, statusMsg.domainType());
			assertTrue(statusMsg.hasState());
			assertEquals("Open / Suspect / None / 'channel down.'", statusMsg.state().toString());
			assertTrue(statusMsg.hasName());
			assertEquals(itemName2, statusMsg.name());
			assertTrue(statusMsg.hasServiceId());
			assertEquals(2, statusMsg.serviceId());
			assertTrue(statusMsg.hasServiceName());
			assertEquals(serviceName2, statusMsg.serviceName());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_6", channelInfo.channelName()); // Gets this status message when recovering with the standby channel.
			assertEquals(ChannelInformation.ChannelState.CLOSED, channelInfo.channelState());
			
			message = consumerClient.popMessage();
			
			/* Checks the market price item refresh */
			refreshMsg = (RefreshMsg)message;
			
			assertEquals(6, refreshMsg.streamId());
			assertEquals(DomainTypes.MARKET_PRICE, refreshMsg.domainType());
			assertEquals("Open / Ok / None / 'Refresh Completed'", refreshMsg.state().toString());
			assertTrue(refreshMsg.complete());
			assertTrue(refreshMsg.solicited());
			assertTrue(refreshMsg.hasName());
			assertEquals(itemName, refreshMsg.name());
			assertTrue(refreshMsg.hasServiceId());
			assertEquals(1, refreshMsg.serviceId());
			assertTrue(refreshMsg.hasServiceName());
			assertEquals(serviceName, refreshMsg.serviceName());
			assertEquals(DataTypes.FIELD_LIST, refreshMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_7", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			
			message = consumerClient.popMessage();
			refreshMsg = (RefreshMsg)message;
			
			assertEquals(5, refreshMsg.streamId());
			assertEquals(DomainTypes.MARKET_PRICE, refreshMsg.domainType());
			assertEquals("Open / Ok / None / 'Refresh Completed'", refreshMsg.state().toString());
			assertTrue(refreshMsg.complete());
			assertTrue(refreshMsg.solicited());
			assertTrue(refreshMsg.hasName());
			assertEquals(itemName2, refreshMsg.name());
			assertTrue(refreshMsg.hasServiceId());
			assertEquals(2, refreshMsg.serviceId());
			assertTrue(refreshMsg.hasServiceName());
			assertEquals(serviceName2, refreshMsg.serviceName());
			assertEquals(DataTypes.FIELD_LIST, refreshMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_8", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
		
			consumer.unregister(itemHandle);
			consumer.unregister(itemHandle2);
		}
		catch(OmmException ex)
		{
			assertFalse(true);
		} catch (InterruptedException e) {

			e.printStackTrace();
		}
		finally
		{
			assertNotNull(consumer);
		
			System.out.println("Uninitilizing...");
			consumer.uninitialize();
			ommprovider3.uninitialize();
			ommprovider4.uninitialize();
			
		}
		
	}
	
	
	@Test
	public void testSingleConnectionChangingPreferredWSBGroupByIOCTLAndThenCallFallbackMethod()
	{
		TestUtilities.printTestHead("testSingleConnectionChangingPreferredWSBGroupByIOCTLAndThenCallFallbackMethod","");
		
		String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";
		
		OmmConsumer consumer = null;
		ConsumerTestOptions consumerOption = new ConsumerTestOptions();
		ProviderTestOptions providerTestOptions = new ProviderTestOptions();
		providerTestOptions.supportStandby = true;
		providerTestOptions.sendRefreshAttrib = true;
		providerTestOptions.itemGroupId = ByteBuffer.wrap("10".getBytes());
		
		ProviderTestClient providerClient3 = new ProviderTestClient(providerTestOptions);
		ProviderTestClient providerClient4 = new ProviderTestClient(providerTestOptions);
		ProviderTestClient providerClient5 = new ProviderTestClient(providerTestOptions);
		ProviderTestClient providerClient6 = new ProviderTestClient(providerTestOptions);
		
		OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);
		
		consumerOption.getChannelInformation = true;
		consumerOption.getSessionChannelInfo = false;
		ConsumerTestClient consumerClient = new ConsumerTestClient(consumerOption);
		
		//WSB-G0 (Down at startup for the entire group) (WarmStandbyChannel_2)
		//OmmProvider ommprovider = EmaFactory.createOmmProvider(config.port("19003").providerName("Provider_1"), providerClient);
		//assertNotNull(ommprovider);
		
		//OmmProvider ommprovider2 = EmaFactory.createOmmProvider(config.port("19006").providerName("Provider_1"), providerClient2);
		//assertNotNull(ommprovider2);

		//WSB-G1 (WarmStandbyChannel_2)
		OmmProvider ommprovider3 = EmaFactory.createOmmProvider(config.port("19007").providerName("Provider_1"), providerClient3);
		assertNotNull(ommprovider3);
		
		OmmProvider ommprovider4 = EmaFactory.createOmmProvider(config.port("19008").providerName("Provider_1"), providerClient4);
		assertNotNull(ommprovider4);
		
		//WSB-G2 (WarmStandbyChannel_5)
		OmmProvider ommprovider5 = EmaFactory.createOmmProvider(config.port("19009").providerName("Provider_1"), providerClient5);
		assertNotNull(ommprovider5);
		
		OmmProvider ommprovider6 = EmaFactory.createOmmProvider(config.port("19010").providerName("Provider_1"), providerClient6);
		assertNotNull(ommprovider6);
		
		try
		{
			ConsumerTestOptions options = new ConsumerTestOptions();
			options.getChannelInformation = true;
		
			consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_27"));
			
			String serviceName = "DIRECT_FEED";
			String itemName = "TRI.N";
			
			ReqMsg reqMsg = EmaFactory.createReqMsg();
			long itemHandle = consumer.registerClient(reqMsg.name(itemName).serviceName(serviceName), consumerClient);
			
			Thread.sleep(1000);

			Msg message = consumerClient.popMessage();

			/* Checks the market price item refresh from the starting channel of WSB-G1 as WSB-G0 is down*/
			RefreshMsg refreshMsg = (RefreshMsg)message;
			
			assertEquals(5, refreshMsg.streamId());
			assertEquals(DomainTypes.MARKET_PRICE, refreshMsg.domainType());
			assertEquals("Open / Ok / None / 'Refresh Completed'", refreshMsg.state().toString());
			assertTrue(refreshMsg.complete());
			assertTrue(refreshMsg.solicited());
			assertTrue(refreshMsg.hasName());
			assertEquals(itemName, refreshMsg.name());
			assertTrue(refreshMsg.hasServiceId());
			assertTrue(refreshMsg.hasServiceName());
			assertEquals(serviceName, refreshMsg.serviceName());
			assertEquals(DataTypes.FIELD_LIST, refreshMsg.payload().dataType());
			ChannelInformation channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_7", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			assertEquals("WarmStandbyChannel_1", channelInfo.preferredHostInfo().getWsbChannelName());
			
			
			// Enable the PH fallback process with the modifyIOCtl() method.
			PreferredHostOptions phOptions = EmaFactory.createPreferredHostOptions();
			phOptions.setPreferredHostEnabled(true);
			phOptions.setWsbChannelName("WarmStandbyChannel_5");
			
			consumer.modifyIOCtl(IOCtlCode.FALLBACK_PREFERRED_HOST_OPTIONS, phOptions);
			
			Thread.sleep(1000);
			
			System.out.println("\nCalls fallbackPreferredHost() to fallback to WarmStandbyChannel_5");
			consumer.fallbackPreferredHost();
			
			Thread.sleep(2000);
			
			message = consumerClient.popMessage();
			StatusMsg statusMsg = (StatusMsg)message;
			assertEquals(5, statusMsg.streamId());
			assertEquals(DomainTypes.MARKET_PRICE, statusMsg.domainType());
			assertTrue(statusMsg.hasState());
			assertEquals("Open / Suspect / None / 'channel down.'", statusMsg.state().toString());
			assertTrue(statusMsg.hasName());
			assertEquals(itemName, statusMsg.name());
			assertTrue(statusMsg.hasServiceId());
			assertTrue(statusMsg.hasServiceName());
			assertEquals(serviceName, statusMsg.serviceName());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_7", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.INACTIVE, channelInfo.channelState());
			
			message = consumerClient.popMessage();

			/* Checks the market price item refresh from the starting channel of WSB-G2 after the fallback is trigger */
			refreshMsg = (RefreshMsg)message;
			
			assertEquals(5, refreshMsg.streamId());
			assertEquals(DomainTypes.MARKET_PRICE, refreshMsg.domainType());
			assertEquals("Open / Ok / None / 'Refresh Completed'", refreshMsg.state().toString());
			assertTrue(refreshMsg.complete());
			assertTrue(refreshMsg.solicited());
			assertTrue(refreshMsg.hasName());
			assertEquals(itemName, refreshMsg.name());
			assertTrue(refreshMsg.hasServiceId());
			assertTrue(refreshMsg.hasServiceName());
			assertEquals(serviceName, refreshMsg.serviceName());
			assertEquals(DataTypes.FIELD_LIST, refreshMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_9", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			assertEquals("WarmStandbyChannel_5", channelInfo.preferredHostInfo().getWsbChannelName());
			
			System.out.println("\nBring down S6 and S5");
			
			providerClient4.clearQueue(); // Clear the previous message in the queue to check only the last recovery events.
			
			ommprovider6.uninitialize();
			ommprovider5.uninitialize();
			
			Thread.sleep(10000);
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			assertEquals(5, statusMsg.streamId());
			assertEquals(DomainTypes.MARKET_PRICE, statusMsg.domainType());
			assertTrue(statusMsg.hasState());
			assertEquals("Open / Suspect / None / 'channel down.'", statusMsg.state().toString());
			assertTrue(statusMsg.hasName());
			assertEquals(itemName, statusMsg.name());
			assertTrue(statusMsg.hasServiceId());
			assertTrue(statusMsg.hasServiceName());
			assertEquals(serviceName, statusMsg.serviceName());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_9", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.CLOSED, channelInfo.channelState());
			
			message = consumerClient.popMessage();

			/* Checks the market price item refresh from the starting channel of WSB-G1 as the preferred group is down */
			refreshMsg = (RefreshMsg)message;
			
			assertEquals(5, refreshMsg.streamId());
			assertEquals(DomainTypes.MARKET_PRICE, refreshMsg.domainType());
			assertEquals("Open / Ok / None / 'Refresh Completed'", refreshMsg.state().toString());
			assertTrue(refreshMsg.complete());
			assertTrue(refreshMsg.solicited());
			assertTrue(refreshMsg.hasName());
			assertEquals(itemName, refreshMsg.name());
			assertTrue(refreshMsg.hasServiceId());
			assertTrue(refreshMsg.hasServiceName());
			assertEquals(serviceName, refreshMsg.serviceName());
			assertEquals(DataTypes.FIELD_LIST, refreshMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_7", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			assertEquals("WarmStandbyChannel_5", channelInfo.preferredHostInfo().getWsbChannelName());
			
			// Checks to ensure that the standby sever of WSB-G1 receives the item request message
			message = providerClient4.popMessage();
			
			GenericMsg genericMsg = (GenericMsg)message;
			assertEquals(1, genericMsg.streamId());
			assertEquals(DomainTypes.LOGIN, genericMsg.domainType());
			assertEquals(EmaRdm.ENAME_CONS_CONN_STATUS, genericMsg.name());
			assertEquals(DataTypes.MAP, genericMsg.payload().dataType());
			Map map = genericMsg.payload().map();
			Iterator<MapEntry> mapIt = map.iterator();
			assertTrue(mapIt.hasNext());
			MapEntry mapEntry = mapIt.next();
			assertEquals(EmaRdm.ENAME_WARMSTANDBY_INFO, mapEntry.key().ascii().toString());
			assertEquals(DataTypes.ELEMENT_LIST, mapEntry.loadType());
			
			ElementList elementList = mapEntry.elementList();
			Iterator<ElementEntry> elementListIt = elementList.iterator();
			assertTrue(elementListIt.hasNext());
			ElementEntry elementEntry = elementListIt.next();
			assertEquals(EmaRdm.ENAME_WARMSTANDBY_MODE, elementEntry.name());
			assertEquals(ServerTypes.STANDBY, elementEntry.uintValue());
			assertFalse(elementListIt.hasNext()); // End of ElementList
			assertFalse(mapIt.hasNext()); // End of Map
			
			// Checks to ensure that the standby sever of WSB-G1 receives the item request message
			message = providerClient4.popMessage();
			ReqMsg recvReq = (ReqMsg)message;
			
			assertEquals(3, recvReq.streamId());
			assertEquals("DIRECT_FEED", recvReq.serviceName());
			assertEquals(itemName, recvReq.name());
			assertEquals(1, recvReq.serviceId());
			
			
			consumer.unregister(itemHandle);
		}
		catch(Exception excep)
		{
			System.out.println(excep);
		}
		finally
		{
			if(consumer != null)
				consumer.uninitialize();
			
			ommprovider3.uninitialize();
			ommprovider4.uninitialize();
		}
	}

	
	@Test // ETA-RTSDK2.2.1-42303
	public void testSingleConnectionForPHWSBServiceBasedServiceDownAndServiceUpThenDetectionTimeIntervalIsTriggerToFallbackToPerServiceNameSet()
	{
		TestUtilities.printTestHead("testSingleConnectionForPHWSBServiceBasedServiceDownAndServiceUpThenDetectionTimeIntervalIsTriggerToFallbackToPerServiceNameSet","");
		
		String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";
		
		OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);
		
		ProviderTestOptions providerTestOptions = new ProviderTestOptions();
		providerTestOptions.supportStandby = true;
		providerTestOptions.sendRefreshAttrib = true;
		providerTestOptions.itemGroupId = ByteBuffer.wrap("10".getBytes());
		
		ProviderTestClient providerClient = new ProviderTestClient(providerTestOptions);
		ProviderTestClient providerClient2 = new ProviderTestClient(providerTestOptions);
		ProviderTestClient providerClient3 = new ProviderTestClient(providerTestOptions);
		ProviderTestClient providerClient4 = new ProviderTestClient(providerTestOptions);
		
		//WSB-G0 (WarmStandbyChannel_3_1). The per service name set is specified. 
		// Provider_9 provides the DIRECT_FEED and DIRECT_FEED1 service name for WSB channel.
		OmmProvider ommprovider = EmaFactory.createOmmProvider(config.port("19003").providerName("Provider_9"), providerClient);
		assertNotNull(ommprovider);
				
		OmmProvider ommprovider2 = EmaFactory.createOmmProvider(config.port("19006").providerName("Provider_9"), providerClient2);
		assertNotNull(ommprovider2);

		//WSB-G1 (WarmStandbyChannel_4_1). The per service name set is specified.
		// Provider_9 provides the DIRECT_FEED and DIRECT_FEED1 service name for WSB channel.
		OmmProvider ommprovider3 = EmaFactory.createOmmProvider(config.port("19007").providerName("Provider_9"), providerClient3);
		assertNotNull(ommprovider3);
				
		OmmProvider ommprovider4 = EmaFactory.createOmmProvider(config.port("19008").providerName("Provider_9"), providerClient4);
		assertNotNull(ommprovider4);
				
		
		OmmConsumer consumer = null;
		ConsumerTestOptions consumerOption = new ConsumerTestOptions();
		
		consumerOption.getChannelInformation = true;
		consumerOption.getSessionChannelInfo = false;
		ConsumerTestClient consumerClient = new ConsumerTestClient(consumerOption);
		
		try
		{
			ConsumerTestOptions options = new ConsumerTestOptions();
			options.getChannelInformation = true;
		
			consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_32"), consumerClient);
			
			String serviceName = "DIRECT_FEED";
			String itemName = "TRI.N";
			
			String serviceName2 = "DIRECT_FEED1";
			String itemName2 = "IBM.N";
			
			ReqMsg reqMsg = EmaFactory.createReqMsg();
			long itemHandle = consumer.registerClient(reqMsg.name(itemName).serviceName(serviceName), consumerClient);
			
			long itemHandle2 = consumer.registerClient(reqMsg.name(itemName2).serviceName(serviceName2), consumerClient);
			
			Thread.sleep(1000);
			
			Msg message = consumerClient.popMessage();
			
			/* Checks login refresh message */
			RefreshMsg refreshMsg = (RefreshMsg)message;
			assertEquals(1, refreshMsg.streamId());
			assertEquals(DomainTypes.LOGIN, refreshMsg.domainType());
			assertEquals("Open / Ok / None / 'Login accepted'", refreshMsg.state().toString());
			assertTrue(refreshMsg.solicited());
			assertTrue(refreshMsg.complete());
			assertTrue(refreshMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, refreshMsg.payload().dataType());
			assertEquals(DataTypes.ELEMENT_LIST, refreshMsg.attrib().dataType());
			ChannelInformation channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_7", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			
			/* Checks market price refresh message for TRI.N with the DIRECT_FEED service name */
			message = consumerClient.popMessage();
			refreshMsg = (RefreshMsg)message;
			
			assertEquals(5, refreshMsg.streamId());
			assertEquals(DomainTypes.MARKET_PRICE, refreshMsg.domainType());
			assertEquals("Open / Ok / None / 'Refresh Completed'", refreshMsg.state().toString());
			assertTrue(refreshMsg.complete());
			assertTrue(refreshMsg.solicited());
			assertTrue(refreshMsg.hasName());
			assertEquals(itemName, refreshMsg.name());
			assertTrue(refreshMsg.hasServiceId());
			assertTrue(refreshMsg.hasServiceName());
			assertEquals(serviceName, refreshMsg.serviceName());
			assertEquals(DataTypes.FIELD_LIST, refreshMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_7", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			
			/* Checks market price refresh message for IBM.N with the DIRECT_FEED1 service name */
			message = consumerClient.popMessage();
			refreshMsg = (RefreshMsg)message;
			
			assertEquals(6, refreshMsg.streamId());
			assertEquals(DomainTypes.MARKET_PRICE, refreshMsg.domainType());
			assertEquals("Open / Ok / None / 'Refresh Completed'", refreshMsg.state().toString());
			assertTrue(refreshMsg.complete());
			assertTrue(refreshMsg.solicited());
			assertTrue(refreshMsg.hasName());
			assertEquals(itemName2, refreshMsg.name());
			assertTrue(refreshMsg.hasServiceId());
			assertTrue(refreshMsg.hasServiceName());
			assertEquals(serviceName2, refreshMsg.serviceName());
			assertEquals(DataTypes.FIELD_LIST, refreshMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_8", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			
			{
			// Bring the DIRECT_FEED1(service ID 2) service from  ommprovider4.
			System.out.println("Send the DIRECT_FEED1 service down state for ommprovider4");
			ElementList serviceState = EmaFactory.createElementList();
			serviceState.add( EmaFactory.createElementEntry().uintValue( EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_DOWN ));
			serviceState.add( EmaFactory.createElementEntry().uintValue( EmaRdm.ENAME_ACCEPTING_REQS, 0 ));
			serviceState.add( EmaFactory.createElementEntry().state( EmaRdm.ENAME_STATUS, OmmState.StreamState.OPEN, OmmState.DataState.SUSPECT, OmmState.StatusCode.NONE, ""));
			FilterList filterListEnc = EmaFactory.createFilterList();
			filterListEnc.add( EmaFactory.createFilterEntry().elementList( EmaRdm.SERVICE_STATE_ID, FilterEntry.FilterAction.SET, serviceState ) );
			Map mapEnc = EmaFactory.createMap();
			mapEnc.add( EmaFactory.createMapEntry().keyUInt( 2, MapEntry.MapAction.UPDATE, filterListEnc ));
			UpdateMsg updateMsg = EmaFactory.createUpdateMsg();
			ommprovider4.submit( updateMsg.domainType( EmaRdm.MMT_DIRECTORY ).
					filter( EmaRdm.SERVICE_STATE_FILTER ).
					payload( mapEnc ), 0);	// use 0 item handle to fan-out to all subscribers
			}
			
			Thread.sleep(1000);
			
			/* Checks market price refresh message for IBM.N with the DIRECT_FEED1 service name from ommprovider3 as DIRECT_FEED1 of ommprovider4 is down. */
			message = consumerClient.popMessage();
			refreshMsg = (RefreshMsg)message;
			
			assertEquals(6, refreshMsg.streamId());
			assertEquals(DomainTypes.MARKET_PRICE, refreshMsg.domainType());
			assertEquals("Open / Ok / None / 'Unsolicited Refresh Completed'", refreshMsg.state().toString());
			assertTrue(refreshMsg.complete());
			assertFalse(refreshMsg.solicited());
			assertTrue(refreshMsg.hasName());
			assertEquals(itemName2, refreshMsg.name());
			assertTrue(refreshMsg.hasServiceId());
			assertTrue(refreshMsg.hasServiceName());
			assertEquals(serviceName2, refreshMsg.serviceName());
			assertEquals(DataTypes.FIELD_LIST, refreshMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_7", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			
			{
			// Bring the DIRECT_FEED(service ID 1) service down from ommprovider3.
			System.out.println("Send the DIRECT_FEED service down state for ommprovider3");
			ElementList serviceState2 = EmaFactory.createElementList();
			serviceState2.add( EmaFactory.createElementEntry().uintValue( EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_DOWN ));
			serviceState2.add( EmaFactory.createElementEntry().uintValue( EmaRdm.ENAME_ACCEPTING_REQS, 0 ));
			serviceState2.add( EmaFactory.createElementEntry().state( EmaRdm.ENAME_STATUS, OmmState.StreamState.OPEN, OmmState.DataState.SUSPECT, OmmState.StatusCode.NONE, ""));
			FilterList filterListEnc2 = EmaFactory.createFilterList();
			filterListEnc2.add( EmaFactory.createFilterEntry().elementList( EmaRdm.SERVICE_STATE_ID, FilterEntry.FilterAction.SET, serviceState2 ) );
			Map mapEnc2 = EmaFactory.createMap();
			mapEnc2.add( EmaFactory.createMapEntry().keyUInt( 1, MapEntry.MapAction.UPDATE, filterListEnc2 ));
			UpdateMsg updateMsg2 = EmaFactory.createUpdateMsg();
			ommprovider3.submit( updateMsg2.domainType( EmaRdm.MMT_DIRECTORY ).
					filter( EmaRdm.SERVICE_STATE_FILTER ).
					payload( mapEnc2 ), 0);	// use 0 item handle to fan-out to all subscribers
			}
			
			Thread.sleep(2000);
			
			/* Checks market price refresh message for TRI.N with the DIRECT_FEED service name from ommprovider4 as DIRECT_FEED of ommprovider3 is down. */
			message = consumerClient.popMessage();
			refreshMsg = (RefreshMsg)message;
			
			assertEquals(5, refreshMsg.streamId());
			assertEquals(DomainTypes.MARKET_PRICE, refreshMsg.domainType());
			assertEquals("Open / Ok / None / 'Unsolicited Refresh Completed'", refreshMsg.state().toString());
			assertTrue(refreshMsg.complete());
			assertFalse(refreshMsg.solicited());
			assertTrue(refreshMsg.hasName());
			assertEquals(itemName, refreshMsg.name());
			assertTrue(refreshMsg.hasServiceId());
			assertTrue(refreshMsg.hasServiceName());
			assertEquals(serviceName, refreshMsg.serviceName());
			assertEquals(DataTypes.FIELD_LIST, refreshMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_8", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			
			{
				// Bring the DIRECT_FEED1(service ID 2) service up from  ommprovider4.
				System.out.println("Send the DIRECT_FEED1 service up state for ommprovider4");
				ElementList serviceState = EmaFactory.createElementList();
				serviceState.add( EmaFactory.createElementEntry().uintValue( EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_UP ));
				serviceState.add( EmaFactory.createElementEntry().uintValue( EmaRdm.ENAME_ACCEPTING_REQS, 1 ));
				serviceState.add( EmaFactory.createElementEntry().state( EmaRdm.ENAME_STATUS, OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, ""));
				FilterList filterListEnc = EmaFactory.createFilterList();
				filterListEnc.add( EmaFactory.createFilterEntry().elementList( EmaRdm.SERVICE_STATE_ID, FilterEntry.FilterAction.SET, serviceState ) );
				Map mapEnc = EmaFactory.createMap();
				mapEnc.add( EmaFactory.createMapEntry().keyUInt( 2, MapEntry.MapAction.UPDATE, filterListEnc ));
				UpdateMsg updateMsg = EmaFactory.createUpdateMsg();
				ommprovider4.submit( updateMsg.domainType( EmaRdm.MMT_DIRECTORY ).
						filter( EmaRdm.SERVICE_STATE_FILTER ).
						payload( mapEnc ), 0);	// use 0 item handle to fan-out to all subscribers
			}
			
			Thread.sleep(1000);
			
			{
				// Bring the DIRECT_FEED(service ID 1) service up from ommprovider3.
				System.out.println("Send the DIRECT_FEED service up state for ommprovider3");
				ElementList serviceState2 = EmaFactory.createElementList();
				serviceState2.add( EmaFactory.createElementEntry().uintValue( EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_UP ));
				serviceState2.add( EmaFactory.createElementEntry().uintValue( EmaRdm.ENAME_ACCEPTING_REQS, 1 ));
				serviceState2.add( EmaFactory.createElementEntry().state( EmaRdm.ENAME_STATUS, OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, ""));
				FilterList filterListEnc2 = EmaFactory.createFilterList();
				filterListEnc2.add( EmaFactory.createFilterEntry().elementList( EmaRdm.SERVICE_STATE_ID, FilterEntry.FilterAction.SET, serviceState2 ) );
				Map mapEnc2 = EmaFactory.createMap();
				mapEnc2.add( EmaFactory.createMapEntry().keyUInt( 1, MapEntry.MapAction.UPDATE, filterListEnc2 ));
				UpdateMsg updateMsg2 = EmaFactory.createUpdateMsg();
				ommprovider3.submit( updateMsg2.domainType( EmaRdm.MMT_DIRECTORY ).
						filter( EmaRdm.SERVICE_STATE_FILTER ).
						payload( mapEnc2 ), 0);	// use 0 item handle to fan-out to all subscribers
			}
			
			System.out.println("\n\nWait for the fallback to trigger");
			Thread.sleep(10000);
			
			message = consumerClient.popMessage();
			
			/* Checks login status message to start the fallback process to fallback to the preferred service name set */
			StatusMsg statusMsg = (StatusMsg)message;
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Ok / None / 'preferred host starting fallback'", statusMsg.state().toString());
			assertTrue(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			assertEquals(DataTypes.NO_DATA, statusMsg.attrib().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_7", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Ok / None / 'preferred host complete'", statusMsg.state().toString());
			assertTrue(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			assertEquals(DataTypes.NO_DATA, statusMsg.attrib().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_7", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			
			/* Checks market price refresh message for TRI.N with the DIRECT_FEED service name from ommprovider3 again*/
			message = consumerClient.popMessage();
			refreshMsg = (RefreshMsg)message;
			
			assertEquals(5, refreshMsg.streamId());
			assertEquals(DomainTypes.MARKET_PRICE, refreshMsg.domainType());
			assertEquals("Open / Ok / None / 'Unsolicited Refresh Completed'", refreshMsg.state().toString());
			assertTrue(refreshMsg.complete());
			assertFalse(refreshMsg.solicited());
			assertTrue(refreshMsg.hasName());
			assertEquals(itemName, refreshMsg.name());
			assertTrue(refreshMsg.hasServiceId());
			assertTrue(refreshMsg.hasServiceName());
			assertEquals(serviceName, refreshMsg.serviceName());
			assertEquals(DataTypes.FIELD_LIST, refreshMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_7", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			
			/* Checks market price refresh message for IBM.N with the DIRECT_FEED1 service name from ommprovider4 again*/
			message = consumerClient.popMessage();
			refreshMsg = (RefreshMsg)message;
			
			assertEquals(6, refreshMsg.streamId());
			assertEquals(DomainTypes.MARKET_PRICE, refreshMsg.domainType());
			assertEquals("Open / Ok / None / 'Unsolicited Refresh Completed'", refreshMsg.state().toString());
			assertTrue(refreshMsg.complete());
			assertFalse(refreshMsg.solicited());
			assertTrue(refreshMsg.hasName());
			assertEquals(itemName2, refreshMsg.name());
			assertTrue(refreshMsg.hasServiceId());
			assertTrue(refreshMsg.hasServiceName());
			assertEquals(serviceName2, refreshMsg.serviceName());
			assertEquals(DataTypes.FIELD_LIST, refreshMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_8", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			
			// Bring down ommprovider4 and ommprovider3
			System.out.println("\n\nBring down ommprovider4\n");
			ommprovider4.uninitialize();
			ommprovider4 = null;
			Thread.sleep(2000);
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			assertEquals(6, statusMsg.streamId());
			assertEquals(DomainTypes.MARKET_PRICE, statusMsg.domainType());
			assertTrue(statusMsg.hasState());
			assertEquals("Open / Suspect / None / 'Service for this item was lost.'", statusMsg.state().toString());
			assertTrue(statusMsg.hasName());
			assertEquals(itemName2, statusMsg.name());
			assertTrue(statusMsg.hasServiceId());
			assertTrue(statusMsg.hasServiceName());
			assertEquals(serviceName2, statusMsg.serviceName());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_8", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.CLOSED, channelInfo.channelState());
			
			message = consumerClient.popMessage();
			refreshMsg = (RefreshMsg)message;
			
			assertEquals(6, refreshMsg.streamId());
			assertEquals(DomainTypes.MARKET_PRICE, refreshMsg.domainType());
			assertEquals("Open / Ok / None / 'Unsolicited Refresh Completed'", refreshMsg.state().toString());
			assertTrue(refreshMsg.complete());
			assertFalse(refreshMsg.solicited());
			assertTrue(refreshMsg.hasName());
			assertEquals(itemName2, refreshMsg.name());
			assertTrue(refreshMsg.hasServiceId());
			assertTrue(refreshMsg.hasServiceName());
			assertEquals(serviceName2, refreshMsg.serviceName());
			assertEquals(DataTypes.FIELD_LIST, refreshMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_7", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			
			System.out.println("\n\nBring down ommprovider3\n");
			ommprovider3.uninitialize();
			ommprovider3 = null;
			Thread.sleep(2000);
			
			// WSB-G1 is down
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Suspect / None / 'channel down'", statusMsg.state().toString());
			assertTrue(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			assertEquals(DataTypes.NO_DATA, statusMsg.attrib().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_7", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.CLOSED, channelInfo.channelState());
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Suspect / None / ''", statusMsg.state().toString());
			assertFalse(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			assertEquals(DataTypes.NO_DATA, statusMsg.attrib().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_7", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.CLOSED, channelInfo.channelState());
			
			// Receives the Open/Suspect status message for TRI.N as the channel is down.
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			assertEquals(5, statusMsg.streamId());
			assertEquals(DomainTypes.MARKET_PRICE, statusMsg.domainType());
			assertTrue(statusMsg.hasState());
			assertEquals("Open / Suspect / None / 'channel down.'", statusMsg.state().toString());
			assertTrue(statusMsg.hasName());
			assertEquals(itemName, statusMsg.name());
			assertTrue(statusMsg.hasServiceId());
			assertTrue(statusMsg.hasServiceName());
			assertEquals(serviceName, statusMsg.serviceName());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_7", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.CLOSED, channelInfo.channelState());
			
			// Receives the Open/Suspect status message for IBM.N as the channel is down.
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			assertEquals(6, statusMsg.streamId());
			assertEquals(DomainTypes.MARKET_PRICE, statusMsg.domainType());
			assertTrue(statusMsg.hasState());
			assertEquals("Open / Suspect / None / 'channel down.'", statusMsg.state().toString());
			assertTrue(statusMsg.hasName());
			assertEquals(itemName2, statusMsg.name());
			assertTrue(statusMsg.hasServiceId());
			assertTrue(statusMsg.hasServiceName());
			assertEquals(serviceName2, statusMsg.serviceName());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_7", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.CLOSED, channelInfo.channelState());

			Thread.sleep(5000);
			
			System.out.println("\n\nBring ommprovider3 and ommprovider4 up again\n");
			/* Bring  ommprovider3 up again */
			ommprovider3 = EmaFactory.createOmmProvider(config.port("19007").providerName("Provider_9"), providerClient3);
					
			/* Bring  ommprovider4 up again */
			ommprovider4 = EmaFactory.createOmmProvider(config.port("19008").providerName("Provider_9"), providerClient4);
			
			/* Checks login refresh message from the staring server of WSB-G0*/
			message = consumerClient.popMessage();
			refreshMsg = (RefreshMsg)message;
			assertEquals(1, refreshMsg.streamId());
			assertEquals(DomainTypes.LOGIN, refreshMsg.domainType());
			assertEquals("Open / Ok / None / 'Login accepted'", refreshMsg.state().toString());
			assertTrue(refreshMsg.solicited());
			assertTrue(refreshMsg.complete());
			assertTrue(refreshMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, refreshMsg.payload().dataType());
			assertEquals(DataTypes.ELEMENT_LIST, refreshMsg.attrib().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_3", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());

			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Ok / None / 'channel up'", statusMsg.state().toString());
			assertTrue(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			assertEquals(DataTypes.NO_DATA, statusMsg.attrib().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_3", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			
			/* Checks market price refresh message for IBM.N with the DIRECT_FEED1 service name */
			message = consumerClient.popMessage();
			refreshMsg = (RefreshMsg)message;
			
			assertEquals(6, refreshMsg.streamId());
			assertEquals(DomainTypes.MARKET_PRICE, refreshMsg.domainType());
			assertEquals("Open / Ok / None / 'Refresh Completed'", refreshMsg.state().toString());
			assertTrue(refreshMsg.complete());
			assertTrue(refreshMsg.solicited());
			assertTrue(refreshMsg.hasName());
			assertEquals(itemName2, refreshMsg.name());
			assertTrue(refreshMsg.hasServiceId());
			assertTrue(refreshMsg.hasServiceName());
			assertEquals(serviceName2, refreshMsg.serviceName());
			assertEquals(DataTypes.FIELD_LIST, refreshMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_3", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			
			/* Checks market price refresh message for TRI.N with the DIRECT_FEED service name */
			message = consumerClient.popMessage();
			refreshMsg = (RefreshMsg)message;
			
			assertEquals(5, refreshMsg.streamId());
			assertEquals(DomainTypes.MARKET_PRICE, refreshMsg.domainType());
			assertEquals("Open / Ok / None / 'Refresh Completed'", refreshMsg.state().toString());
			assertTrue(refreshMsg.complete());
			assertTrue(refreshMsg.solicited());
			assertTrue(refreshMsg.hasName());
			assertEquals(itemName, refreshMsg.name());
			assertTrue(refreshMsg.hasServiceId());
			assertTrue(refreshMsg.hasServiceName());
			assertEquals(serviceName, refreshMsg.serviceName());
			assertEquals(DataTypes.FIELD_LIST, refreshMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_6", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			
			/* Check the fallback message from detection time internal which doesn't fallback to preferred group as 
			 * the PHFallBackWithInWSBGroup is set to true. */
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Ok / None / 'preferred host starting fallback'", statusMsg.state().toString());
			assertTrue(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			assertEquals(DataTypes.NO_DATA, statusMsg.attrib().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_3", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Ok / None / 'preferred host complete'", statusMsg.state().toString());
			assertTrue(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			assertEquals(DataTypes.NO_DATA, statusMsg.attrib().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_3", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			
			consumer.unregister(itemHandle);
			consumer.unregister(itemHandle2);			
		}
		catch(Exception exp)
		{
			System.out.println("Exception message: " + exp.getMessage());
			System.out.println(exp.getStackTrace());
			Assert.assertFalse(true);
		}
		finally
		{
			consumer.uninitialize();
			
			ommprovider.uninitialize();
			ommprovider2.uninitialize();
			
			if(ommprovider3 != null)
				ommprovider3.uninitialize();
			
			if(ommprovider4 != null)
				ommprovider4.uninitialize();
		}
	}
	
	@Test // ETA-RTSDK2.2.1-42302
	public void testSingleConnectionForWSBServiceBasedDetectionTimeIntervalAndHostDownToTestWithWSBGroupReconnectLogicWithEnablingFallbackWithInWSBGroup()
	{
		TestUtilities.printTestHead("testSingleConnectionForWSBServiceBasedDetectionTimeIntervalAndHostDownToTestWithWSBGroupReconnectLogicWithEnablingFallbackWithInWSBGroup","");
		
		String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";
		
		OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);
		
		ProviderTestOptions providerTestOptions = new ProviderTestOptions();
		providerTestOptions.supportStandby = true;
		providerTestOptions.sendRefreshAttrib = true;
		providerTestOptions.itemGroupId = ByteBuffer.wrap("10".getBytes());
		
		ProviderTestClient providerClient = new ProviderTestClient(providerTestOptions);
		ProviderTestClient providerClient2 = new ProviderTestClient(providerTestOptions);
		ProviderTestClient providerClient3 = new ProviderTestClient(providerTestOptions);
		ProviderTestClient providerClient4 = new ProviderTestClient(providerTestOptions);
		ProviderTestClient providerClient5 = new ProviderTestClient(providerTestOptions);
		ProviderTestClient providerClient6 = new ProviderTestClient(providerTestOptions);
		
		//WSB-G0 (WarmStandbyChannel_3).
		// Provider_9 provides the DIRECT_FEED and DIRECT_FEED1 service name for WSB channel.
		OmmProvider ommprovider = EmaFactory.createOmmProvider(config.port("19003").providerName("Provider_9"), providerClient);
		assertNotNull(ommprovider);
				
		OmmProvider ommprovider2 = EmaFactory.createOmmProvider(config.port("19006").providerName("Provider_9"), providerClient2);
		assertNotNull(ommprovider2);

		//WSB-G1 (WarmStandbyChannel_4). This is preferred WSB group.
		// Provider_9 provides the DIRECT_FEED and DIRECT_FEED1 service name for WSB channel.
		OmmProvider ommprovider3 = EmaFactory.createOmmProvider(config.port("19007").providerName("Provider_9"), providerClient3);
		assertNotNull(ommprovider3);
				
		OmmProvider ommprovider4 = EmaFactory.createOmmProvider(config.port("19008").providerName("Provider_9"), providerClient4);
		assertNotNull(ommprovider4);
		
		//WSB-G2 (WarmStandbyChannel_7).
		// Provider_9 provides the DIRECT_FEED and DIRECT_FEED1 service name for WSB channel.
		OmmProvider ommprovider5 = EmaFactory.createOmmProvider(config.port("19009").providerName("Provider_9"), providerClient5);
		assertNotNull(ommprovider5);
						
		OmmProvider ommprovider6 = EmaFactory.createOmmProvider(config.port("19010").providerName("Provider_9"), providerClient6);
		assertNotNull(ommprovider6);
		
		OmmConsumer consumer = null;
		ConsumerTestOptions consumerOption = new ConsumerTestOptions();
		
		consumerOption.getChannelInformation = true;
		consumerOption.getSessionChannelInfo = false;
		ConsumerTestClient consumerClient = new ConsumerTestClient(consumerOption);
		
		try
		{
			ConsumerTestOptions options = new ConsumerTestOptions();
			options.getChannelInformation = true;
		
			consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_34"), consumerClient);
			
			String serviceName = "DIRECT_FEED";
			String itemName = "TRI.N";
			
			String serviceName2 = "DIRECT_FEED1";
			String itemName2 = "IBM.N";
			
			ReqMsg reqMsg = EmaFactory.createReqMsg();
			long itemHandle = consumer.registerClient(reqMsg.name(itemName).serviceName(serviceName), consumerClient);
			
			long itemHandle2 = consumer.registerClient(reqMsg.clear().name(itemName2).serviceName(serviceName2), consumerClient);
			
			Thread.sleep(2000);
			
			Msg message = consumerClient.popMessage();
			
			/* Checks login refresh message */
			RefreshMsg refreshMsg = (RefreshMsg)message;
			assertEquals(1, refreshMsg.streamId());
			assertEquals(DomainTypes.LOGIN, refreshMsg.domainType());
			assertEquals("Open / Ok / None / 'Login accepted'", refreshMsg.state().toString());
			assertTrue(refreshMsg.solicited());
			assertTrue(refreshMsg.complete());
			assertTrue(refreshMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, refreshMsg.payload().dataType());
			assertEquals(DataTypes.ELEMENT_LIST, refreshMsg.attrib().dataType());
			ChannelInformation channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_7", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			
			/* Checks market price refresh message for TRI.N with the DIRECT_FEED service name from ommprovider3 */
			message = consumerClient.popMessage();
			refreshMsg = (RefreshMsg)message;
			
			assertEquals(5, refreshMsg.streamId());
			assertEquals(DomainTypes.MARKET_PRICE, refreshMsg.domainType());
			assertEquals("Open / Ok / None / 'Refresh Completed'", refreshMsg.state().toString());
			assertTrue(refreshMsg.complete());
			assertTrue(refreshMsg.solicited());
			assertTrue(refreshMsg.hasName());
			assertEquals(itemName, refreshMsg.name());
			assertTrue(refreshMsg.hasServiceId());
			assertTrue(refreshMsg.hasServiceName());
			assertEquals(serviceName, refreshMsg.serviceName());
			assertEquals(DataTypes.FIELD_LIST, refreshMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_7", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			
			/* Checks market price refresh message for IBM.N with the DIRECT_FEED1 service name from ommprovider3 */
			message = consumerClient.popMessage();
			refreshMsg = (RefreshMsg)message;
			
			assertEquals(6, refreshMsg.streamId());
			assertEquals(DomainTypes.MARKET_PRICE, refreshMsg.domainType());
			assertEquals("Open / Ok / None / 'Refresh Completed'", refreshMsg.state().toString());
			assertTrue(refreshMsg.complete());
			assertTrue(refreshMsg.solicited());
			assertTrue(refreshMsg.hasName());
			assertEquals(itemName2, refreshMsg.name());
			assertTrue(refreshMsg.hasServiceId());
			assertTrue(refreshMsg.hasServiceName());
			assertEquals(serviceName2, refreshMsg.serviceName());
			assertEquals(DataTypes.FIELD_LIST, refreshMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_7", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			
			List<Integer> serviceIdList = new ArrayList<Integer>();
			serviceIdList.add(1);
			serviceIdList.add(2);
			
			System.out.println("Bring down all services for ommprovider3");
			for(Integer serviceId : serviceIdList)
			{
				ElementList serviceState = EmaFactory.createElementList();
				serviceState.add( EmaFactory.createElementEntry().uintValue( EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_DOWN ));
				serviceState.add( EmaFactory.createElementEntry().uintValue( EmaRdm.ENAME_ACCEPTING_REQS, 0 ));
				serviceState.add( EmaFactory.createElementEntry().state( EmaRdm.ENAME_STATUS, OmmState.StreamState.OPEN, OmmState.DataState.SUSPECT, OmmState.StatusCode.NONE, ""));
				FilterList filterListEnc = EmaFactory.createFilterList();
				filterListEnc.add( EmaFactory.createFilterEntry().elementList( EmaRdm.SERVICE_STATE_ID, FilterEntry.FilterAction.SET, serviceState ) );
				Map mapEnc = EmaFactory.createMap();
				mapEnc.add( EmaFactory.createMapEntry().keyUInt( serviceId.intValue(), MapEntry.MapAction.UPDATE, filterListEnc ));
				UpdateMsg updateMsg = EmaFactory.createUpdateMsg();
				ommprovider3.submit( updateMsg.domainType( EmaRdm.MMT_DIRECTORY ).
						filter( EmaRdm.SERVICE_STATE_FILTER ).
						payload( mapEnc ), 0);	// use 0 item handle to fan-out to all subscribers
			}
			
			Thread.sleep(3000);
			
			/* Checks market price refresh message for TRI.N with the DIRECT_FEED service name from ommprovider4 */
			message = consumerClient.popMessage();
			refreshMsg = (RefreshMsg)message;
			
			assertEquals(5, refreshMsg.streamId());
			assertEquals(DomainTypes.MARKET_PRICE, refreshMsg.domainType());
			assertEquals("Open / Ok / None / 'Unsolicited Refresh Completed'", refreshMsg.state().toString());
			assertTrue(refreshMsg.complete());
			assertFalse(refreshMsg.solicited());
			assertTrue(refreshMsg.hasName());
			assertEquals(itemName, refreshMsg.name());
			assertTrue(refreshMsg.hasServiceId());
			assertTrue(refreshMsg.hasServiceName());
			assertEquals(serviceName, refreshMsg.serviceName());
			assertEquals(DataTypes.FIELD_LIST, refreshMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_8", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			
			/* Checks market price refresh message for IBM.N with the DIRECT_FEED1 service name from ommprovider4 */
			message = consumerClient.popMessage();
			refreshMsg = (RefreshMsg)message;
			
			assertEquals(6, refreshMsg.streamId());
			assertEquals(DomainTypes.MARKET_PRICE, refreshMsg.domainType());
			assertEquals("Open / Ok / None / 'Unsolicited Refresh Completed'", refreshMsg.state().toString());
			assertTrue(refreshMsg.complete());
			assertFalse(refreshMsg.solicited());
			assertTrue(refreshMsg.hasName());
			assertEquals(itemName2, refreshMsg.name());
			assertTrue(refreshMsg.hasServiceId());
			assertTrue(refreshMsg.hasServiceName());
			assertEquals(serviceName2, refreshMsg.serviceName());
			assertEquals(DataTypes.FIELD_LIST, refreshMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_8", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			
			System.out.println("\nBring down ommprovider3 and ommprovider4");
			ommprovider3.uninitialize();
			ommprovider3 = null;
			Thread.sleep(1000);
			
			ommprovider4.uninitialize();
			ommprovider4 = null;
			Thread.sleep(5000);
			
			// WSB-G1 is down
			message = consumerClient.popMessage();
			StatusMsg statusMsg = (StatusMsg)message;
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Suspect / None / 'channel down'", statusMsg.state().toString());
			assertTrue(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			assertEquals(DataTypes.NO_DATA, statusMsg.attrib().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_8", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.CLOSED, channelInfo.channelState());
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Suspect / None / ''", statusMsg.state().toString());
			assertFalse(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			assertEquals(DataTypes.NO_DATA, statusMsg.attrib().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_8", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.CLOSED, channelInfo.channelState());
			
			// Receives the Open/Suspect status message for TRI.N as the channel is down.
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			assertEquals(5, statusMsg.streamId());
			assertEquals(DomainTypes.MARKET_PRICE, statusMsg.domainType());
			assertTrue(statusMsg.hasState());
			assertEquals("Open / Suspect / None / 'channel down.'", statusMsg.state().toString());
			assertTrue(statusMsg.hasName());
			assertEquals(itemName, statusMsg.name());
			assertTrue(statusMsg.hasServiceId());
			assertTrue(statusMsg.hasServiceName());
			assertEquals(serviceName, statusMsg.serviceName());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_8", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.CLOSED, channelInfo.channelState());
			
			// Receives the Open/Suspect status message for IBM.N as the channel is down.
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			assertEquals(6, statusMsg.streamId());
			assertEquals(DomainTypes.MARKET_PRICE, statusMsg.domainType());
			assertTrue(statusMsg.hasState());
			assertEquals("Open / Suspect / None / 'channel down.'", statusMsg.state().toString());
			assertTrue(statusMsg.hasName());
			assertEquals(itemName2, statusMsg.name());
			assertTrue(statusMsg.hasServiceId());
			assertTrue(statusMsg.hasServiceName());
			assertEquals(serviceName2, statusMsg.serviceName());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_8", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.CLOSED, channelInfo.channelState());
			
			System.out.println("Reactor reconnect to WSB-G1 and WSB-G0");
			
			/* Checks login refresh message from the staring server of WSB-G0*/
			message = consumerClient.popMessage();
			refreshMsg = (RefreshMsg)message;
			assertEquals(1, refreshMsg.streamId());
			assertEquals(DomainTypes.LOGIN, refreshMsg.domainType());
			assertEquals("Open / Ok / None / 'Login accepted'", refreshMsg.state().toString());
			assertTrue(refreshMsg.solicited());
			assertTrue(refreshMsg.complete());
			assertTrue(refreshMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, refreshMsg.payload().dataType());
			assertEquals(DataTypes.ELEMENT_LIST, refreshMsg.attrib().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_3", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());

			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Ok / None / 'channel up'", statusMsg.state().toString());
			assertTrue(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			assertEquals(DataTypes.NO_DATA, statusMsg.attrib().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_3", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			
			/* Checks market price refresh message for TRI.N with the DIRECT_FEED service name */
			message = consumerClient.popMessage();
			refreshMsg = (RefreshMsg)message;
			
			assertEquals(5, refreshMsg.streamId());
			assertEquals(DomainTypes.MARKET_PRICE, refreshMsg.domainType());
			assertEquals("Open / Ok / None / 'Refresh Completed'", refreshMsg.state().toString());
			assertTrue(refreshMsg.complete());
			assertTrue(refreshMsg.solicited());
			assertTrue(refreshMsg.hasName());
			assertEquals(itemName, refreshMsg.name());
			assertTrue(refreshMsg.hasServiceId());
			assertTrue(refreshMsg.hasServiceName());
			assertEquals(serviceName, refreshMsg.serviceName());
			assertEquals(DataTypes.FIELD_LIST, refreshMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_3", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			
			/* Checks market price refresh message for IBM.N with the DIRECT_FEED1 service name */
			message = consumerClient.popMessage();
			refreshMsg = (RefreshMsg)message;
			
			assertEquals(6, refreshMsg.streamId());
			assertEquals(DomainTypes.MARKET_PRICE, refreshMsg.domainType());
			assertEquals("Open / Ok / None / 'Refresh Completed'", refreshMsg.state().toString());
			assertTrue(refreshMsg.complete());
			assertTrue(refreshMsg.solicited());
			assertTrue(refreshMsg.hasName());
			assertEquals(itemName2, refreshMsg.name());
			assertTrue(refreshMsg.hasServiceId());
			assertTrue(refreshMsg.hasServiceName());
			assertEquals(serviceName2, refreshMsg.serviceName());
			assertEquals(DataTypes.FIELD_LIST, refreshMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_3", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			
			System.out.println("Bring up ommprovider3 and ommprovider4 for WSB-G1 again");
			ommprovider3 = EmaFactory.createOmmProvider(config.port("19007").providerName("Provider_9"), providerClient3);
					
			ommprovider4 = EmaFactory.createOmmProvider(config.port("19008").providerName("Provider_9"), providerClient4);
			
			Thread.sleep(3000);
			
			/* Check the fallback message from detection time internal which doesn't fallback to preferred group as 
			 * the PHFallBackWithInWSBGroup is set to true. */
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Ok / None / 'preferred host starting fallback'", statusMsg.state().toString());
			assertTrue(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			assertEquals(DataTypes.NO_DATA, statusMsg.attrib().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_3", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Ok / None / 'preferred host complete'", statusMsg.state().toString());
			assertTrue(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			assertEquals(DataTypes.NO_DATA, statusMsg.attrib().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_3", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			
			consumer.unregister(itemHandle);
			consumer.unregister(itemHandle2);
		}
		catch(Exception exp)
		{
			System.out.println(exp.getMessage());
			assertFalse(true);
		}
		finally
		{
			consumer.uninitialize();
			
			ommprovider.uninitialize();
			ommprovider2.uninitialize();
			
			if(ommprovider3 != null)
				ommprovider3.uninitialize();
			
			if(ommprovider4 != null)
				ommprovider4.uninitialize();
			
			ommprovider5.uninitialize();
			ommprovider6.uninitialize();
		}
	}

	
	@Test // ETA-RTSDK2.2.1-42310
	public void testSingleConnectionForWSBServiceBasedDisablePHThenEnablePHWithIOCtlCallWithPreferredServiceListSpecified()
	{
		TestUtilities.printTestHead("testSingleConnectionForWSBServiceBasedDisablePHThenEnablePHWithIOCtlCallWithPreferredServiceListSpecified","");
		
		String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";
		
		OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);
		
		ProviderTestOptions providerTestOptions = new ProviderTestOptions();
		providerTestOptions.supportStandby = true;
		providerTestOptions.sendRefreshAttrib = true;
		providerTestOptions.itemGroupId = ByteBuffer.wrap("10".getBytes());
		
		ProviderTestClient providerClient = new ProviderTestClient(providerTestOptions);
		ProviderTestClient providerClient2 = new ProviderTestClient(providerTestOptions);
		ProviderTestClient providerClient3 = new ProviderTestClient(providerTestOptions);
		ProviderTestClient providerClient4 = new ProviderTestClient(providerTestOptions);
		
		//WSB-G0 (WarmStandbyChannel_3).
		// Provider_9 provides the DIRECT_FEED and DIRECT_FEED1 service name for WSB channel.
		OmmProvider ommprovider = EmaFactory.createOmmProvider(config.port("19003").providerName("Provider_9"), providerClient);
		assertNotNull(ommprovider);
				
		OmmProvider ommprovider2 = EmaFactory.createOmmProvider(config.port("19006").providerName("Provider_9"), providerClient2);
		assertNotNull(ommprovider2);

		//WSB-G1 (WarmStandbyChannel_4). This is preferred WSB group.
		// Provider_9 provides the DIRECT_FEED and DIRECT_FEED1 service name for WSB channel.
		OmmProvider ommprovider3 = EmaFactory.createOmmProvider(config.port("19007").providerName("Provider_9"), providerClient3);
		assertNotNull(ommprovider3);
				
		OmmProvider ommprovider4 = EmaFactory.createOmmProvider(config.port("19008").providerName("Provider_9"), providerClient4);
		assertNotNull(ommprovider4);

		OmmConsumer consumer = null;
		ConsumerTestOptions consumerOption = new ConsumerTestOptions();
		
		consumerOption.getChannelInformation = true;
		consumerOption.getSessionChannelInfo = false;
		ConsumerTestClient consumerClient = new ConsumerTestClient(consumerOption);
		
		try
		{
			ConsumerTestOptions options = new ConsumerTestOptions();
			options.getChannelInformation = true;
		
			consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_35"), consumerClient);
			
			String serviceName = "DIRECT_FEED";
			int serviceId = 1;
			String itemName = "TRI.N";
			
			String serviceName2 = "DIRECT_FEED1";
			int serviceId2 = 2;
			String itemName2 = "IBM.N";
			
			ReqMsg reqMsg = EmaFactory.createReqMsg();
			long itemHandle = consumer.registerClient(reqMsg.name(itemName).serviceName(serviceName), consumerClient);
			
			long itemHandle2 = consumer.registerClient(reqMsg.clear().name(itemName2).serviceName(serviceName2), consumerClient);
			
			Thread.sleep(2000);
			
			Msg message = consumerClient.popMessage();
			
			/* Checks login refresh message */
			RefreshMsg refreshMsg = (RefreshMsg)message;
			assertEquals(1, refreshMsg.streamId());
			assertEquals(DomainTypes.LOGIN, refreshMsg.domainType());
			assertEquals("Open / Ok / None / 'Login accepted'", refreshMsg.state().toString());
			assertTrue(refreshMsg.solicited());
			assertTrue(refreshMsg.complete());
			assertTrue(refreshMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, refreshMsg.payload().dataType());
			assertEquals(DataTypes.ELEMENT_LIST, refreshMsg.attrib().dataType());
			ChannelInformation channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_3", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			
			/* Checks market price refresh message for TRI.N with the DIRECT_FEED service name from ommprovider */
			message = consumerClient.popMessage();
			refreshMsg = (RefreshMsg)message;
			
			assertEquals(5, refreshMsg.streamId());
			assertEquals(DomainTypes.MARKET_PRICE, refreshMsg.domainType());
			assertEquals("Open / Ok / None / 'Refresh Completed'", refreshMsg.state().toString());
			assertTrue(refreshMsg.complete());
			assertTrue(refreshMsg.solicited());
			assertTrue(refreshMsg.hasName());
			assertEquals(itemName, refreshMsg.name());
			assertTrue(refreshMsg.hasServiceId());
			assertEquals(serviceId, refreshMsg.serviceId());
			assertTrue(refreshMsg.hasServiceName());
			assertEquals(serviceName, refreshMsg.serviceName());
			assertEquals(DataTypes.FIELD_LIST, refreshMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_3", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			
			/* Checks market price refresh message for IBM.N with the DIRECT_FEED1 service name from ommprovider */
			message = consumerClient.popMessage();
			refreshMsg = (RefreshMsg)message;
			
			assertEquals(6, refreshMsg.streamId());
			assertEquals(DomainTypes.MARKET_PRICE, refreshMsg.domainType());
			assertEquals("Open / Ok / None / 'Refresh Completed'", refreshMsg.state().toString());
			assertTrue(refreshMsg.complete());
			assertTrue(refreshMsg.solicited());
			assertTrue(refreshMsg.hasName());
			assertEquals(itemName2, refreshMsg.name());
			assertTrue(refreshMsg.hasServiceId());
			assertEquals(serviceId2, refreshMsg.serviceId());
			assertTrue(refreshMsg.hasServiceName());
			assertEquals(serviceName2, refreshMsg.serviceName());
			assertEquals(DataTypes.FIELD_LIST, refreshMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_3", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			
			Thread.sleep(3000);
			
			System.out.println("\nStop ommprovider to let the ommprovide2 provide the service instead");
			ommprovider.uninitialize();
			ommprovider = null;
			
			Thread.sleep(5000);
			
			message = consumerClient.popMessage();
			StatusMsg statusMsg = (StatusMsg)message;
			
			assertEquals(5, statusMsg.streamId());
			assertEquals(DomainTypes.MARKET_PRICE, statusMsg.domainType());
			assertTrue(statusMsg.hasState());
			assertEquals("Open / Suspect / None / 'Service for this item was lost.'", statusMsg.state().toString());
			assertTrue(statusMsg.hasName());
			assertEquals(itemName, statusMsg.name());
			assertTrue(statusMsg.hasServiceId());
			assertEquals(serviceId, statusMsg.serviceId());
			assertTrue(statusMsg.hasServiceName());
			assertEquals(serviceName, statusMsg.serviceName());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_3", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.CLOSED, channelInfo.channelState());
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			
			assertEquals(6, statusMsg.streamId());
			assertEquals(DomainTypes.MARKET_PRICE, statusMsg.domainType());
			assertTrue(statusMsg.hasState());
			assertEquals("Open / Suspect / None / 'Service for this item was lost.'", statusMsg.state().toString());
			assertTrue(statusMsg.hasName());
			assertEquals(itemName2, statusMsg.name());
			assertTrue(statusMsg.hasServiceId());
			assertEquals(serviceId2, statusMsg.serviceId());
			assertTrue(statusMsg.hasServiceName());
			assertEquals(serviceName2, statusMsg.serviceName());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_3", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.CLOSED, channelInfo.channelState());
			
			/* Checks unsolicited refresh message from the standby server of WarmStandbyChannel_3 */
			message = consumerClient.popMessage();
			refreshMsg = (RefreshMsg)message;
			
			assertEquals(5, refreshMsg.streamId());
			assertEquals(DomainTypes.MARKET_PRICE, refreshMsg.domainType());
			assertEquals("Open / Ok / None / 'Unsolicited Refresh Completed'", refreshMsg.state().toString());
			assertTrue(refreshMsg.complete());
			assertFalse(refreshMsg.solicited());
			assertTrue(refreshMsg.hasName());
			assertEquals(itemName, refreshMsg.name());
			assertTrue(refreshMsg.hasServiceId());
			assertEquals(serviceId, refreshMsg.serviceId());
			assertTrue(refreshMsg.hasServiceName());
			assertEquals(serviceName, refreshMsg.serviceName());
			assertEquals(DataTypes.FIELD_LIST, refreshMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_6", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			
			/* Checks market price refresh message for IBM.N with the DIRECT_FEED1 service name from ommprovider2 again*/
			message = consumerClient.popMessage();
			refreshMsg = (RefreshMsg)message;
			
			assertEquals(6, refreshMsg.streamId());
			assertEquals(DomainTypes.MARKET_PRICE, refreshMsg.domainType());
			assertEquals("Open / Ok / None / 'Unsolicited Refresh Completed'", refreshMsg.state().toString());
			assertTrue(refreshMsg.complete());
			assertFalse(refreshMsg.solicited());
			assertTrue(refreshMsg.hasName());
			assertEquals(itemName2, refreshMsg.name());
			assertTrue(refreshMsg.hasServiceId());
			assertEquals(serviceId2, refreshMsg.serviceId());
			assertTrue(refreshMsg.hasServiceName());
			assertEquals(serviceName2, refreshMsg.serviceName());
			assertEquals(DataTypes.FIELD_LIST, refreshMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_6", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			
			System.out.println("\nCall modify IOCTL to enable the PH feature to fallback to WarmStandbyChannel_4.");
			
			// Enable the PH fallback process with the modifyIOCtl() method.
			PreferredHostOptions phOptions = EmaFactory.createPreferredHostOptions();
			phOptions.setPreferredHostEnabled(true);
			phOptions.setWsbChannelName("WarmStandbyChannel_4");
			phOptions.setDetectionTimeInterval(3);
			
			consumer.modifyIOCtl(IOCtlCode.FALLBACK_PREFERRED_HOST_OPTIONS, phOptions);
			
			Thread.sleep(8000);
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Ok / None / 'preferred host starting fallback'", statusMsg.state().toString());
			assertTrue(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			assertEquals(DataTypes.NO_DATA, statusMsg.attrib().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_6", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Suspect / None / 'channel down'", statusMsg.state().toString());
			assertTrue(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			assertEquals(DataTypes.NO_DATA, statusMsg.attrib().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_6", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.INACTIVE, channelInfo.channelState());
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Suspect / None / ''", statusMsg.state().toString());
			assertFalse(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			assertEquals(DataTypes.NO_DATA, statusMsg.attrib().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_6", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.INACTIVE, channelInfo.channelState());
			
			// Receives the Open/Suspect status message for TRI.N as the channel is down.
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			assertEquals(5, statusMsg.streamId());
			assertEquals(DomainTypes.MARKET_PRICE, statusMsg.domainType());
			assertTrue(statusMsg.hasState());
			assertEquals("Open / Suspect / None / 'channel down.'", statusMsg.state().toString());
			assertTrue(statusMsg.hasName());
			assertEquals(itemName, statusMsg.name());
			assertTrue(statusMsg.hasServiceId());
			assertEquals(serviceId, statusMsg.serviceId());
			assertTrue(statusMsg.hasServiceName());
			assertEquals(serviceName, statusMsg.serviceName());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_6", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.INACTIVE, channelInfo.channelState());
			
			// Receives the Open/Suspect status message for IBM.N as the channel is down.
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			assertEquals(6, statusMsg.streamId());
			assertEquals(DomainTypes.MARKET_PRICE, statusMsg.domainType());
			assertTrue(statusMsg.hasState());
			assertEquals("Open / Suspect / None / 'channel down.'", statusMsg.state().toString());
			assertTrue(statusMsg.hasName());
			assertEquals(itemName2, statusMsg.name());
			assertTrue(statusMsg.hasServiceId());
			assertEquals(serviceId2, statusMsg.serviceId());
			assertTrue(statusMsg.hasServiceName());
			assertEquals(serviceName2, statusMsg.serviceName());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_6", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.INACTIVE, channelInfo.channelState());
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Suspect / None / 'preferred host complete'", statusMsg.state().toString());
			assertTrue(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			assertEquals(DataTypes.NO_DATA, statusMsg.attrib().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_7", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			
			message = consumerClient.popMessage();
			
			/* Checks login refresh message */
			refreshMsg = (RefreshMsg)message;
			assertEquals(1, refreshMsg.streamId());
			assertEquals(DomainTypes.LOGIN, refreshMsg.domainType());
			assertEquals("Open / Ok / None / 'Login accepted'", refreshMsg.state().toString());
			assertTrue(refreshMsg.solicited());
			assertTrue(refreshMsg.complete());
			assertTrue(refreshMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, refreshMsg.payload().dataType());
			assertEquals(DataTypes.ELEMENT_LIST, refreshMsg.attrib().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_7", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Ok / None / 'channel up'", statusMsg.state().toString());
			assertTrue(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			assertEquals(DataTypes.NO_DATA, statusMsg.attrib().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_7", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			
			/* Checks market price refresh message for TRI.N with the DIRECT_FEED service name from ommprovider3 */
			message = consumerClient.popMessage();
			refreshMsg = (RefreshMsg)message;
			
			assertEquals(5, refreshMsg.streamId());
			assertEquals(DomainTypes.MARKET_PRICE, refreshMsg.domainType());
			assertEquals("Open / Ok / None / 'Refresh Completed'", refreshMsg.state().toString());
			assertTrue(refreshMsg.complete());
			assertTrue(refreshMsg.solicited());
			assertTrue(refreshMsg.hasName());
			assertEquals(itemName, refreshMsg.name());
			assertTrue(refreshMsg.hasServiceId());
			assertEquals(serviceId, refreshMsg.serviceId());
			assertTrue(refreshMsg.hasServiceName());
			assertEquals(serviceName, refreshMsg.serviceName());
			assertEquals(DataTypes.FIELD_LIST, refreshMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_7", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			
			/* Checks market price refresh message for IBM.N with the DIRECT_FEED1 service name from ommprovider3 */
			message = consumerClient.popMessage();
			refreshMsg = (RefreshMsg)message;
			
			assertEquals(6, refreshMsg.streamId());
			assertEquals(DomainTypes.MARKET_PRICE, refreshMsg.domainType());
			assertEquals("Open / Ok / None / 'Refresh Completed'", refreshMsg.state().toString());
			assertTrue(refreshMsg.complete());
			assertTrue(refreshMsg.solicited());
			assertTrue(refreshMsg.hasName());
			assertEquals(itemName2, refreshMsg.name());
			assertTrue(refreshMsg.hasServiceId());
			assertEquals(serviceId2, refreshMsg.serviceId());
			assertTrue(refreshMsg.hasServiceName());
			assertEquals(serviceName2, refreshMsg.serviceName());
			assertEquals(DataTypes.FIELD_LIST, refreshMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_7", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			
			/* Do nothing as it is connected to the preferred WSB group */
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Ok / None / 'preferred host starting fallback'", statusMsg.state().toString());
			assertTrue(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			assertEquals(DataTypes.NO_DATA, statusMsg.attrib().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_7", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Ok / None / 'preferred host complete'", statusMsg.state().toString());
			assertTrue(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			assertEquals(DataTypes.NO_DATA, statusMsg.attrib().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_7", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			
			consumer.unregister(itemHandle);
			consumer.unregister(itemHandle2);
		}
		catch(Exception exp)
		{
			System.out.println(exp.getMessage());
			assertFalse(true);
		}
		finally
		{
			consumer.uninitialize();
			
			if(ommprovider != null)
				ommprovider.uninitialize();
			
			ommprovider2.uninitialize();
			
			if(ommprovider3 != null)
				ommprovider3.uninitialize();
			
			if(ommprovider4 != null)
				ommprovider4.uninitialize();
		}
	}
	
	public void testSingleConnectionForWSBServiceBasedEnablePHThenChangeDetectionTimeIntervalByIOCtlMethod()
	{
		TestUtilities.printTestHead("testSingleConnectionForWSBServiceBasedEnablePHThenChangeDetectionTimeIntervalByIOCtlMethod","");
		
		// The PHDetectionTimeInterval is set to 12 seconds in the configuration file.
		// The objective of this test case is to use the modifyIOCtl() method to cancel the existing timer and set a new one to fallback.
		
		String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";
		
		OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);
		
		ProviderTestOptions providerTestOptions = new ProviderTestOptions();
		providerTestOptions.supportStandby = true;
		providerTestOptions.sendRefreshAttrib = true;
		providerTestOptions.itemGroupId = ByteBuffer.wrap("10".getBytes());
		
		ProviderTestClient providerClient = new ProviderTestClient(providerTestOptions);
		ProviderTestClient providerClient2 = new ProviderTestClient(providerTestOptions);
		ProviderTestClient providerClient3 = new ProviderTestClient(providerTestOptions);
		ProviderTestClient providerClient4 = new ProviderTestClient(providerTestOptions);
		
		//WSB-G0 (WarmStandbyChannel_3).
		// Provider_9 provides the DIRECT_FEED and DIRECT_FEED1 service name for WSB channel.
		OmmProvider ommprovider = EmaFactory.createOmmProvider(config.port("19003").providerName("Provider_9"), providerClient);
		assertNotNull(ommprovider);
				
		OmmProvider ommprovider2 = EmaFactory.createOmmProvider(config.port("19006").providerName("Provider_9"), providerClient2);
		assertNotNull(ommprovider2);

		//WSB-G1 (WarmStandbyChannel_4). This is preferred WSB group.
		// Provider_9 provides the DIRECT_FEED and DIRECT_FEED1 service name for WSB channel.
		OmmProvider ommprovider3 = EmaFactory.createOmmProvider(config.port("19007").providerName("Provider_9"), providerClient3);
		assertNotNull(ommprovider3);
				
		OmmProvider ommprovider4 = EmaFactory.createOmmProvider(config.port("19008").providerName("Provider_9"), providerClient4);
		assertNotNull(ommprovider4);

		OmmConsumer consumer = null;
		ConsumerTestOptions consumerOption = new ConsumerTestOptions();
		
		consumerOption.getChannelInformation = true;
		consumerOption.getSessionChannelInfo = false;
		ConsumerTestClient consumerClient = new ConsumerTestClient(consumerOption);
		
		try
		{
			ConsumerTestOptions options = new ConsumerTestOptions();
			options.getChannelInformation = true;
		
			consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_36"), consumerClient);
			
			String serviceName = "DIRECT_FEED";
			int serviceId = 1;
			String itemName = "TRI.N";
			
			String serviceName2 = "DIRECT_FEED1";
			int serviceId2 = 2;
			String itemName2 = "IBM.N";
			
			ReqMsg reqMsg = EmaFactory.createReqMsg();
			long itemHandle = consumer.registerClient(reqMsg.name(itemName).serviceName(serviceName), consumerClient);
			
			long itemHandle2 = consumer.registerClient(reqMsg.clear().name(itemName2).serviceName(serviceName2), consumerClient);
			
			Thread.sleep(2000);
			
			Msg message = consumerClient.popMessage();
			
			/* Checks login refresh message */
			RefreshMsg refreshMsg = (RefreshMsg)message;
			assertEquals(1, refreshMsg.streamId());
			assertEquals(DomainTypes.LOGIN, refreshMsg.domainType());
			assertEquals("Open / Ok / None / 'Login accepted'", refreshMsg.state().toString());
			assertTrue(refreshMsg.solicited());
			assertTrue(refreshMsg.complete());
			assertTrue(refreshMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, refreshMsg.payload().dataType());
			assertEquals(DataTypes.ELEMENT_LIST, refreshMsg.attrib().dataType());
			ChannelInformation channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_3", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			
			/* Checks market price refresh message for TRI.N with the DIRECT_FEED service name from ommprovider */
			message = consumerClient.popMessage();
			refreshMsg = (RefreshMsg)message;
			
			assertEquals(5, refreshMsg.streamId());
			assertEquals(DomainTypes.MARKET_PRICE, refreshMsg.domainType());
			assertEquals("Open / Ok / None / 'Refresh Completed'", refreshMsg.state().toString());
			assertTrue(refreshMsg.complete());
			assertTrue(refreshMsg.solicited());
			assertTrue(refreshMsg.hasName());
			assertEquals(itemName, refreshMsg.name());
			assertTrue(refreshMsg.hasServiceId());
			assertEquals(serviceId, refreshMsg.serviceId());
			assertTrue(refreshMsg.hasServiceName());
			assertEquals(serviceName, refreshMsg.serviceName());
			assertEquals(DataTypes.FIELD_LIST, refreshMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_3", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			
			/* Checks market price refresh message for IBM.N with the DIRECT_FEED1 service name from ommprovider */
			message = consumerClient.popMessage();
			refreshMsg = (RefreshMsg)message;
			
			assertEquals(6, refreshMsg.streamId());
			assertEquals(DomainTypes.MARKET_PRICE, refreshMsg.domainType());
			assertEquals("Open / Ok / None / 'Refresh Completed'", refreshMsg.state().toString());
			assertTrue(refreshMsg.complete());
			assertTrue(refreshMsg.solicited());
			assertTrue(refreshMsg.hasName());
			assertEquals(itemName2, refreshMsg.name());
			assertTrue(refreshMsg.hasServiceId());
			assertEquals(serviceId2, refreshMsg.serviceId());
			assertTrue(refreshMsg.hasServiceName());
			assertEquals(serviceName2, refreshMsg.serviceName());
			assertEquals(DataTypes.FIELD_LIST, refreshMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_3", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			
			Thread.sleep(3000);
			
			System.out.println("\nCall modify IOCTL to change the detection time interval to fallback to WarmStandbyChannel_4.");
			
			// Enable the PH fallback process with the modifyIOCtl() method.
			PreferredHostOptions phOptions = EmaFactory.createPreferredHostOptions();
			phOptions.setPreferredHostEnabled(true);
			phOptions.setWsbChannelName("WarmStandbyChannel_4");
			phOptions.setDetectionTimeInterval(5);
			
			consumer.modifyIOCtl(IOCtlCode.FALLBACK_PREFERRED_HOST_OPTIONS, phOptions);
			
			Thread.sleep(6000);
			
			message = consumerClient.popMessage();
			StatusMsg statusMsg = (StatusMsg)message;
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Ok / None / 'preferred host starting fallback'", statusMsg.state().toString());
			assertTrue(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			assertEquals(DataTypes.NO_DATA, statusMsg.attrib().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_3", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Suspect / None / ''", statusMsg.state().toString());
			assertFalse(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			assertEquals(DataTypes.NO_DATA, statusMsg.attrib().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_6", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.INACTIVE, channelInfo.channelState());
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Suspect / None / 'channel down'", statusMsg.state().toString());
			assertTrue(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			assertEquals(DataTypes.NO_DATA, statusMsg.attrib().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_3", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.INACTIVE, channelInfo.channelState());
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Suspect / None / ''", statusMsg.state().toString());
			assertFalse(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			assertEquals(DataTypes.NO_DATA, statusMsg.attrib().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_3", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.INACTIVE, channelInfo.channelState());
			
			// Receives the Open/Suspect status message for TRI.N as the channel is down.
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			assertEquals(5, statusMsg.streamId());
			assertEquals(DomainTypes.MARKET_PRICE, statusMsg.domainType());
			assertTrue(statusMsg.hasState());
			assertEquals("Open / Suspect / None / 'channel down.'", statusMsg.state().toString());
			assertTrue(statusMsg.hasName());
			assertEquals(itemName, statusMsg.name());
			assertTrue(statusMsg.hasServiceId());
			assertEquals(serviceId, statusMsg.serviceId());
			assertTrue(statusMsg.hasServiceName());
			assertEquals(serviceName, statusMsg.serviceName());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_3", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.INACTIVE, channelInfo.channelState());
			
			// Receives the Open/Suspect status message for IBM.N as the channel is down.
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			assertEquals(6, statusMsg.streamId());
			assertEquals(DomainTypes.MARKET_PRICE, statusMsg.domainType());
			assertTrue(statusMsg.hasState());
			assertEquals("Open / Suspect / None / 'channel down.'", statusMsg.state().toString());
			assertTrue(statusMsg.hasName());
			assertEquals(itemName2, statusMsg.name());
			assertTrue(statusMsg.hasServiceId());
			assertEquals(serviceId2, statusMsg.serviceId());
			assertTrue(statusMsg.hasServiceName());
			assertEquals(serviceName2, statusMsg.serviceName());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_3", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.INACTIVE, channelInfo.channelState());
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Suspect / None / 'preferred host complete'", statusMsg.state().toString());
			assertTrue(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			assertEquals(DataTypes.NO_DATA, statusMsg.attrib().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_7", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			
			message = consumerClient.popMessage();
			
			/* Checks login refresh message */
			refreshMsg = (RefreshMsg)message;
			assertEquals(1, refreshMsg.streamId());
			assertEquals(DomainTypes.LOGIN, refreshMsg.domainType());
			assertEquals("Open / Ok / None / 'Login accepted'", refreshMsg.state().toString());
			assertTrue(refreshMsg.solicited());
			assertTrue(refreshMsg.complete());
			assertTrue(refreshMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, refreshMsg.payload().dataType());
			assertEquals(DataTypes.ELEMENT_LIST, refreshMsg.attrib().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_7", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Ok / None / 'channel up'", statusMsg.state().toString());
			assertTrue(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			assertEquals(DataTypes.NO_DATA, statusMsg.attrib().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_7", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			
			/* Checks market price refresh message for TRI.N with the DIRECT_FEED service name from ommprovider3 */
			message = consumerClient.popMessage();
			refreshMsg = (RefreshMsg)message;
			
			assertEquals(5, refreshMsg.streamId());
			assertEquals(DomainTypes.MARKET_PRICE, refreshMsg.domainType());
			assertEquals("Open / Ok / None / 'Refresh Completed'", refreshMsg.state().toString());
			assertTrue(refreshMsg.complete());
			assertTrue(refreshMsg.solicited());
			assertTrue(refreshMsg.hasName());
			assertEquals(itemName, refreshMsg.name());
			assertTrue(refreshMsg.hasServiceId());
			assertEquals(serviceId, refreshMsg.serviceId());
			assertTrue(refreshMsg.hasServiceName());
			assertEquals(serviceName, refreshMsg.serviceName());
			assertEquals(DataTypes.FIELD_LIST, refreshMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_7", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			
			/* Checks market price refresh message for IBM.N with the DIRECT_FEED1 service name from ommprovider3 */
			message = consumerClient.popMessage();
			refreshMsg = (RefreshMsg)message;
			
			assertEquals(6, refreshMsg.streamId());
			assertEquals(DomainTypes.MARKET_PRICE, refreshMsg.domainType());
			assertEquals("Open / Ok / None / 'Refresh Completed'", refreshMsg.state().toString());
			assertTrue(refreshMsg.complete());
			assertTrue(refreshMsg.solicited());
			assertTrue(refreshMsg.hasName());
			assertEquals(itemName2, refreshMsg.name());
			assertTrue(refreshMsg.hasServiceId());
			assertEquals(serviceId2, refreshMsg.serviceId());
			assertTrue(refreshMsg.hasServiceName());
			assertEquals(serviceName2, refreshMsg.serviceName());
			assertEquals(DataTypes.FIELD_LIST, refreshMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_7", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			
			Thread.sleep(2000);
			
			/* Ensure that there is timer trigger from the detection time interval of the EMA configure file. */
			assertEquals(0,  consumerClient.queueSize());
			
			consumer.unregister(itemHandle);
			consumer.unregister(itemHandle2);
		}
		catch(Exception exp)
		{
			System.out.println(exp.getMessage());
			assertFalse(true);
		}
		finally
		{
			consumer.uninitialize();
			
			if(ommprovider != null)
				ommprovider.uninitialize();
			
			ommprovider2.uninitialize();
			
			if(ommprovider3 != null)
				ommprovider3.uninitialize();
			
			if(ommprovider4 != null)
				ommprovider4.uninitialize();
		}
	}
	
	@Test // ETA-RTSDK2.2.1-42211
	public void testSingleConnectionLoginBasedFallBackByMethodAndDetectionTimeInterval()
	{
		TestUtilities.printTestHead("testSingleConnectionLoginBasedFallBackByMethodAndDetectionTimeInterval","");
		
		String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";
		
		OmmConsumer consumer = null;
		ConsumerTestOptions consumerOption = new ConsumerTestOptions();
		ProviderTestOptions providerTestOptions = new ProviderTestOptions();
		providerTestOptions.supportStandby = true;
		providerTestOptions.sendRefreshAttrib = true;
		providerTestOptions.itemGroupId = ByteBuffer.wrap("10".getBytes());
		
		ProviderTestClient providerClient = new ProviderTestClient(providerTestOptions);
		ProviderTestClient providerClient2 = new ProviderTestClient(providerTestOptions);
		ProviderTestClient providerClient3 = new ProviderTestClient(providerTestOptions);
		ProviderTestClient providerClient4 = new ProviderTestClient(providerTestOptions);
		ProviderTestClient providerClient5 = new ProviderTestClient(providerTestOptions);
		ProviderTestClient providerClient6 = new ProviderTestClient(providerTestOptions);
		
		OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);
		
		consumerOption.getChannelInformation = true;
		consumerOption.getSessionChannelInfo = false;
		ConsumerTestClient consumerClient = new ConsumerTestClient(consumerOption);
		
		//WSB-G0 (Down at startup for the entire group) (WarmStandbyChannel_1)
		OmmProvider ommprovider = EmaFactory.createOmmProvider(config.port("19003").providerName("Provider_1"), providerClient);
		assertNotNull(ommprovider);
		
		OmmProvider ommprovider2 = EmaFactory.createOmmProvider(config.port("19006").providerName("Provider_1"), providerClient2);
		assertNotNull(ommprovider2);

		//WSB-G1 (WarmStandbyChannel_2) // This is preferred WSB group but it is not up at the beginning.
		OmmProvider ommprovider3 = null;
		
		OmmProvider ommprovider4 = null;
		
		//WSB-G2 (WarmStandbyChannel_5)
		OmmProvider ommprovider5 = EmaFactory.createOmmProvider(config.port("19009").providerName("Provider_1"), providerClient5);
		assertNotNull(ommprovider5);
		
		OmmProvider ommprovider6 = EmaFactory.createOmmProvider(config.port("19010").providerName("Provider_1"), providerClient6);
		assertNotNull(ommprovider6);
		
		try
		{
			ConsumerTestOptions options = new ConsumerTestOptions();
			options.getChannelInformation = true;
		
			consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_37"), consumerClient);
			
			String serviceName = "DIRECT_FEED";
			String itemName = "TRI.N";
			
			ReqMsg reqMsg = EmaFactory.createReqMsg();
			long itemHandle = consumer.registerClient(reqMsg.name(itemName).serviceName(serviceName), consumerClient);
			
			Thread.sleep(1000);

			Msg message = consumerClient.popMessage();
			StatusMsg statusMsg = (StatusMsg)message;

			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Suspect / None / 'channel down'", statusMsg.state().toString());
			ChannelInformation channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_7", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.INITIALIZING, channelInfo.channelState());
			
			message = consumerClient.popMessage();
			RefreshMsg refreshMsg = (RefreshMsg)message;
						
			assertEquals(1, refreshMsg.streamId());
			assertEquals(DomainTypes.LOGIN, refreshMsg.domainType());
			assertEquals("Open / Ok / None / 'Login accepted'", refreshMsg.state().toString());
			assertTrue(refreshMsg.solicited());
			assertTrue(refreshMsg.complete());
			assertTrue(refreshMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, refreshMsg.payload().dataType());
			assertEquals(DataTypes.ELEMENT_LIST, refreshMsg.attrib().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_3", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			assertEquals("WarmStandbyChannel_2", channelInfo.preferredHostInfo().getWsbChannelName());

			//Checks the market price item refresh from the starting channel of WSB-G0 as WSB-G1 is down
			message = consumerClient.popMessage();
			refreshMsg = (RefreshMsg)message;
			
			assertEquals(5, refreshMsg.streamId());
			assertEquals(DomainTypes.MARKET_PRICE, refreshMsg.domainType());
			assertEquals("Open / Ok / None / 'Refresh Completed'", refreshMsg.state().toString());
			assertTrue(refreshMsg.complete());
			assertTrue(refreshMsg.solicited());
			assertTrue(refreshMsg.hasName());
			assertEquals(itemName, refreshMsg.name());
			assertTrue(refreshMsg.hasServiceId());
			assertTrue(refreshMsg.hasServiceName());
			assertEquals(serviceName, refreshMsg.serviceName());
			assertEquals(DataTypes.FIELD_LIST, refreshMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_3", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			assertEquals("WarmStandbyChannel_2", channelInfo.preferredHostInfo().getWsbChannelName());
			
			Thread.sleep(3000);
			
			message = consumerClient.popMessage();
			/* Checks login status messages */
			statusMsg = (StatusMsg)message;
			
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Ok / None / 'channel up'", statusMsg.state().toString());
			assertTrue(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_6", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			assertEquals("WarmStandbyChannel_2", channelInfo.preferredHostInfo().getWsbChannelName());
			
			// Call the method to fallback but the preferred group is not up yet.
			System.out.println("\nCalls fallbackPreferredHost() to fallback to WarmStandbyChannel_2(Do nothing)");
			consumer.fallbackPreferredHost();
			
			Thread.sleep(3000);
			
			// Checks for PH START and COMPLETE events
			message = consumerClient.popMessage();
			
			/* Checks login status messages */
			statusMsg = (StatusMsg)message;
			
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Ok / None / 'preferred host starting fallback'", statusMsg.state().toString());
			assertTrue(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_6", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			assertEquals("WarmStandbyChannel_2", channelInfo.preferredHostInfo().getWsbChannelName());
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Ok / None / 'preferred host complete'", statusMsg.state().toString());
			assertTrue(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_6", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			assertEquals("WarmStandbyChannel_2", channelInfo.preferredHostInfo().getWsbChannelName());
			
			// Start the preferred WSB group WSB-G1
			ommprovider3 = EmaFactory.createOmmProvider(config.port("19007").providerName("Provider_1"), providerClient3);
			
			ommprovider4 = EmaFactory.createOmmProvider(config.port("19008").providerName("Provider_1"), providerClient4);
			
			// The fallback should happen by the detection time interval
			System.out.println("Fallback by the detection time interval.");
			
			Thread.sleep(5000);
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Ok / None / 'preferred host starting fallback'", statusMsg.state().toString());
			assertTrue(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			assertEquals(DataTypes.NO_DATA, statusMsg.attrib().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_3", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Suspect / None / ''", statusMsg.state().toString());
			assertFalse(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			assertEquals(DataTypes.NO_DATA, statusMsg.attrib().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_6", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.INACTIVE, channelInfo.channelState());
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Suspect / None / 'channel down'", statusMsg.state().toString());
			assertTrue(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			assertEquals(DataTypes.NO_DATA, statusMsg.attrib().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_3", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.INACTIVE, channelInfo.channelState());
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Suspect / None / ''", statusMsg.state().toString());
			assertFalse(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			assertEquals(DataTypes.NO_DATA, statusMsg.attrib().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_3", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.INACTIVE, channelInfo.channelState());
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			assertEquals(5, statusMsg.streamId());
			assertEquals(DomainTypes.MARKET_PRICE, statusMsg.domainType());
			assertTrue(statusMsg.hasState());
			assertEquals("Open / Suspect / None / 'channel down.'", statusMsg.state().toString());
			assertTrue(statusMsg.hasName());
			assertEquals(itemName, statusMsg.name());
			assertTrue(statusMsg.hasServiceId());
			assertTrue(statusMsg.hasServiceName());
			assertEquals(serviceName, statusMsg.serviceName());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_3", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.INACTIVE, channelInfo.channelState());
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Suspect / None / 'preferred host complete'", statusMsg.state().toString());
			assertTrue(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_7", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			assertEquals("WarmStandbyChannel_2", channelInfo.preferredHostInfo().getWsbChannelName());
			
			message = consumerClient.popMessage();
			refreshMsg = (RefreshMsg)message;
						
			assertEquals(1, refreshMsg.streamId());
			assertEquals(DomainTypes.LOGIN, refreshMsg.domainType());
			assertEquals("Open / Ok / None / 'Login accepted'", refreshMsg.state().toString());
			assertTrue(refreshMsg.solicited());
			assertTrue(refreshMsg.complete());
			assertTrue(refreshMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, refreshMsg.payload().dataType());
			assertEquals(DataTypes.ELEMENT_LIST, refreshMsg.attrib().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_7", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			
			message = consumerClient.popMessage();
			/* Checks login status messages */
			statusMsg = (StatusMsg)message;
			
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Ok / None / 'channel up'", statusMsg.state().toString());
			assertTrue(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_7", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			assertEquals("WarmStandbyChannel_2", channelInfo.preferredHostInfo().getWsbChannelName());
			
			message = consumerClient.popMessage();

			//Checks the market price item refresh from the starting channel of WSB-G1 after the fallback is trigger 
			refreshMsg = (RefreshMsg)message;
			
			assertEquals(5, refreshMsg.streamId());
			assertEquals(DomainTypes.MARKET_PRICE, refreshMsg.domainType());
			assertEquals("Open / Ok / None / 'Refresh Completed'", refreshMsg.state().toString());
			assertTrue(refreshMsg.complete());
			assertTrue(refreshMsg.solicited());
			assertTrue(refreshMsg.hasName());
			assertEquals(itemName, refreshMsg.name());
			assertTrue(refreshMsg.hasServiceId());
			assertTrue(refreshMsg.hasServiceName());
			assertEquals(serviceName, refreshMsg.serviceName());
			assertEquals(DataTypes.FIELD_LIST, refreshMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_7", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			assertEquals("WarmStandbyChannel_2", channelInfo.preferredHostInfo().getWsbChannelName());
			
			Thread.sleep(10000);
			
			// Checks for PH START and COMPLETE events
			message = consumerClient.popMessage();
			
			/* Checks login status messages */
			statusMsg = (StatusMsg)message;
			
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Ok / None / 'preferred host starting fallback'", statusMsg.state().toString());
			assertTrue(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_7", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			assertEquals("WarmStandbyChannel_2", channelInfo.preferredHostInfo().getWsbChannelName());
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Ok / None / 'preferred host complete'", statusMsg.state().toString());
			assertTrue(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_7", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			assertEquals("WarmStandbyChannel_2", channelInfo.preferredHostInfo().getWsbChannelName());
			
			consumer.unregister(itemHandle);
		}
		catch(Exception excep)
		{
			System.out.println(excep);
		}
		finally
		{
			if(consumer != null)
				consumer.uninitialize();
			
			ommprovider.uninitialize();
			ommprovider2.uninitialize();
			
			if (ommprovider3 != null)
				ommprovider3.uninitialize();
			
			if(ommprovider4 != null)
				ommprovider4.uninitialize();
			
			ommprovider5.uninitialize();
			ommprovider6.uninitialize();
		}
	}


	@Test // ETA-RTSDK2.2.1-42305
	public void testServiceBasedAPISwitchToWSBGroupUponDetectionInterval()
	{
		TestUtilities.printTestHead("testServiceBasedAPISwitchToWSBGroupUponDetectionInterval","");
		
		String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";

		OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);

		ProviderTestOptions providerTestOptions = new ProviderTestOptions();
		providerTestOptions.supportStandby = true;
		providerTestOptions.sendRefreshAttrib = true;
		providerTestOptions.itemGroupId = ByteBuffer.wrap("10".getBytes());

		ProviderTestClient providerClient_1 = new ProviderTestClient(providerTestOptions);
		providerClient_1.name = "Provider_1";
		ProviderTestClient providerClient_3 = new ProviderTestClient(providerTestOptions);
		providerClient_3.name = "Provider_3";
		ProviderTestClient providerClient_6 = new ProviderTestClient(providerTestOptions);
		providerClient_6.name = "Provider_6";

		// Provider_9 provides the DIRECT_FEED and DIRECT_FEED1 service name for WSB channel.
		OmmProvider ommprovider_1 = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_9"), providerClient_1);
		assertNotNull(ommprovider_1);

		//WSB-G0 (WarmStandbyChannel_3).
		OmmProvider ommprovider_3 = EmaFactory.createOmmProvider(config.port("19003").providerName("Provider_9"), providerClient_3);
		assertNotNull(ommprovider_3);

		// Provider_9 provides the DIRECT_FEED and DIRECT_FEED1 service name for WSB channel.
		OmmProvider ommprovider_6 = EmaFactory.createOmmProvider(config.port("19006").providerName("Provider_9"), providerClient_6);
		assertNotNull(ommprovider_6);

		OmmConsumer consumer = null;
		ConsumerTestOptions consumerOption = new ConsumerTestOptions();

		consumerOption.getChannelInformation = true;
		consumerOption.getSessionChannelInfo = false;
		ConsumerTestClient consumerClient = new ConsumerTestClient(consumerOption);

		try
		{
			ConsumerTestOptions options = new ConsumerTestOptions();
			options.getChannelInformation = true;

			ServiceList serviceList = EmaFactory.createServiceList("SVG1");

			serviceList.concreteServiceList().add("DIRECT_FEED");
			serviceList.concreteServiceList().add("DIRECT_FEED1");

			consumer = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation)
					.consumerName("Consumer_38").addServiceList(serviceList), consumerClient);

			Thread.sleep(2000);

			ReqMsg reqMsg = EmaFactory.createReqMsg();

			long itemHandle = consumer.registerClient(reqMsg.name("IBM.N").serviceName("DIRECT_FEED"), consumerClient);
			long itemHandle2 = consumer.registerClient(reqMsg.clear().name("TRI.N").serviceName("DIRECT_FEED1"), consumerClient);

			Thread.sleep(2000);

			System.out.println("\n>>>>>>>>>>> WSB Group services go down <<<<<<<<<<");

			System.out.println("Send the DIRECT_FEED service down state for ommprovider_3");
			ElementList serviceState2 = EmaFactory.createElementList();
			serviceState2.add( EmaFactory.createElementEntry().uintValue( EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_DOWN ));
			serviceState2.add( EmaFactory.createElementEntry().uintValue( EmaRdm.ENAME_ACCEPTING_REQS, 0 ));
			serviceState2.add( EmaFactory.createElementEntry().state( EmaRdm.ENAME_STATUS, OmmState.StreamState.OPEN, OmmState.DataState.SUSPECT, OmmState.StatusCode.NONE, ""));
			FilterList filterListEnc2 = EmaFactory.createFilterList();
			filterListEnc2.add( EmaFactory.createFilterEntry().elementList( EmaRdm.SERVICE_STATE_ID, FilterEntry.FilterAction.SET, serviceState2 ) );
			Map mapEnc2 = EmaFactory.createMap();
			mapEnc2.add( EmaFactory.createMapEntry().keyUInt( 1, MapEntry.MapAction.UPDATE, filterListEnc2 ));
			UpdateMsg updateMsg2 = EmaFactory.createUpdateMsg();
			ommprovider_3.submit( updateMsg2.domainType( EmaRdm.MMT_DIRECTORY ).
					filter( EmaRdm.SERVICE_STATE_FILTER ).
					payload( mapEnc2 ), 0);	// use 0 item handle to fan-out to all subscribers

			System.out.println("Send the DIRECT_FEED service down state for ommprovider_3");
			serviceState2 = EmaFactory.createElementList();
			serviceState2.add( EmaFactory.createElementEntry().uintValue( EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_DOWN ));
			serviceState2.add( EmaFactory.createElementEntry().uintValue( EmaRdm.ENAME_ACCEPTING_REQS, 0 ));
			serviceState2.add( EmaFactory.createElementEntry().state( EmaRdm.ENAME_STATUS, OmmState.StreamState.OPEN, OmmState.DataState.SUSPECT, OmmState.StatusCode.NONE, ""));
			filterListEnc2 = EmaFactory.createFilterList();
			filterListEnc2.add( EmaFactory.createFilterEntry().elementList( EmaRdm.SERVICE_STATE_ID, FilterEntry.FilterAction.SET, serviceState2 ) );
			mapEnc2 = EmaFactory.createMap();
			mapEnc2.add( EmaFactory.createMapEntry().keyUInt( 2, MapEntry.MapAction.UPDATE, filterListEnc2 ));
			updateMsg2 = EmaFactory.createUpdateMsg();
			ommprovider_3.submit( updateMsg2.domainType( EmaRdm.MMT_DIRECTORY ).
					filter( EmaRdm.SERVICE_STATE_FILTER ).
					payload( mapEnc2 ), 0);	// use 0 item handle to fan-out to all subscribers

			System.out.println("Send the DIRECT_FEED service down state for ommprovider_6");
			serviceState2 = EmaFactory.createElementList();
			serviceState2.add( EmaFactory.createElementEntry().uintValue( EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_DOWN ));
			serviceState2.add( EmaFactory.createElementEntry().uintValue( EmaRdm.ENAME_ACCEPTING_REQS, 0 ));
			serviceState2.add( EmaFactory.createElementEntry().state( EmaRdm.ENAME_STATUS, OmmState.StreamState.OPEN, OmmState.DataState.SUSPECT, OmmState.StatusCode.NONE, ""));
			filterListEnc2 = EmaFactory.createFilterList();
			filterListEnc2.add( EmaFactory.createFilterEntry().elementList( EmaRdm.SERVICE_STATE_ID, FilterEntry.FilterAction.SET, serviceState2 ) );
			mapEnc2 = EmaFactory.createMap();
			mapEnc2.add( EmaFactory.createMapEntry().keyUInt( 1, MapEntry.MapAction.UPDATE, filterListEnc2 ));
			updateMsg2 = EmaFactory.createUpdateMsg();
			ommprovider_6.submit( updateMsg2.domainType( EmaRdm.MMT_DIRECTORY ).
					filter( EmaRdm.SERVICE_STATE_FILTER ).
					payload( mapEnc2 ), 0);	// use 0 item handle to fan-out to all subscribers

			System.out.println("Send the DIRECT_FEED service down state for ommprovider_6");
			serviceState2 = EmaFactory.createElementList();
			serviceState2.add( EmaFactory.createElementEntry().uintValue( EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_DOWN ));
			serviceState2.add( EmaFactory.createElementEntry().uintValue( EmaRdm.ENAME_ACCEPTING_REQS, 0 ));
			serviceState2.add( EmaFactory.createElementEntry().state( EmaRdm.ENAME_STATUS, OmmState.StreamState.OPEN, OmmState.DataState.SUSPECT, OmmState.StatusCode.NONE, ""));
			filterListEnc2 = EmaFactory.createFilterList();
			filterListEnc2.add( EmaFactory.createFilterEntry().elementList( EmaRdm.SERVICE_STATE_ID, FilterEntry.FilterAction.SET, serviceState2 ) );
			mapEnc2 = EmaFactory.createMap();
			mapEnc2.add( EmaFactory.createMapEntry().keyUInt( 2, MapEntry.MapAction.UPDATE, filterListEnc2 ));
			updateMsg2 = EmaFactory.createUpdateMsg();
			ommprovider_6.submit( updateMsg2.domainType( EmaRdm.MMT_DIRECTORY ).
					filter( EmaRdm.SERVICE_STATE_FILTER ).
					payload( mapEnc2 ), 0);	// use 0 item handle to fan-out to all subscribers

			Thread.sleep(5000);

			System.out.println("\n>>>>>>>>>>> WSB Group goes down!!! <<<<<<<<<<");

			ommprovider_3.uninitialize();
			ommprovider_6.uninitialize();

			Thread.sleep(10000);

			System.out.println("\n>>>>>>>>>>> WSB Group goes up <<<<<<<<<<");
			providerClient_3 = new ProviderTestClient(providerTestOptions);
			providerClient_3.name = "Provider_3";
			providerClient_6 = new ProviderTestClient(providerTestOptions);
			providerClient_6.name = "Provider_6";

			ommprovider_3 = EmaFactory.createOmmProvider(config.port("19003").providerName("Provider_9"), providerClient_3);
			assertNotNull(ommprovider_3);

			ommprovider_6 = EmaFactory.createOmmProvider(config.port("19006").providerName("Provider_9"), providerClient_6);
			assertNotNull(ommprovider_6);

			Thread.sleep(3000);

			System.out.println("\n>>>>>>>>>>> Starting 100s sleep! <<<<<<<<<<");

			Thread.sleep(100000);

			System.out.println(">>>>>>>>>>>>>> Sleep finished!");

			int preferredHostStartedEventCount = 0;
			int preferredHostCompletedEventCount = 0;

			int count = consumerClient.queueSize();
			for (int i = 0; i < count; i++)
			{
				Msg msg = consumerClient.popMessage();
				if (msg instanceof StatusMsg)
				{
					StatusMsg statusMsg = (StatusMsg)msg;
					OmmState state = statusMsg.state();
					if (state.statusText().contains("preferred host complete")) preferredHostCompletedEventCount++;
					if (state.statusText().contains("preferred host starting fallback")) preferredHostStartedEventCount++;
				}
			}

			assertTrue(preferredHostStartedEventCount > 2);
			assertTrue(preferredHostCompletedEventCount > 2);

			for (int i = 0; i < consumerClient.channelInfoSize(); i++) consumerClient.popChannelInfo();

			FieldList fieldList = EmaFactory.createFieldList();

			fieldList.clear();
			fieldList.add(EmaFactory.createFieldEntry().real(22, 3991, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			fieldList.add(EmaFactory.createFieldEntry().real(30, 10, OmmReal.MagnitudeType.EXPONENT_0));

			ommprovider_3.submit( EmaFactory.createUpdateMsg().payload( fieldList ), providerClient_3.retriveItemHandle("IBM.N") );
			ommprovider_3.submit( EmaFactory.createUpdateMsg().payload( fieldList ), providerClient_3.retriveItemHandle("TRI.N") );

			ommprovider_6.submit( EmaFactory.createUpdateMsg().payload( fieldList ), providerClient_6.retriveItemHandle("IBM.N") );
			ommprovider_6.submit( EmaFactory.createUpdateMsg().payload( fieldList ), providerClient_6.retriveItemHandle("TRI.N") );

			Thread.sleep(3000);

			assertEquals(2, consumerClient.queueSize());

			boolean foundTRI = false;
			boolean foundIBM = false;

			for (int i = 0; i < 2; i++)
			{
				Msg message = consumerClient.popMessage();
				UpdateMsg update = (UpdateMsg)message;

				if (update.name().equals("TRI.N"))
				{
					foundTRI = true;
				}
				if (update.name().equals("IBM.N"))
				{
					foundIBM = true;
				}

				assertTrue(update.name().equals("TRI.N") && update.serviceName().equals("DIRECT_FEED1")
						|| update.name().equals("IBM.N") && update.serviceName().equals("DIRECT_FEED"));
			}

			assertTrue(foundIBM);
			assertTrue(foundTRI);
		}
		catch (Exception e)
		{
			e.printStackTrace();
			assertFalse(true);
		}
		finally
		{
			System.out.println("\nUninitializing...");
			
			if(consumer != null)
				consumer.uninitialize();

			ommprovider_1.uninitialize();
			ommprovider_3.uninitialize();
			ommprovider_6.uninitialize();
		}
	}
	
	@Test // ETA-RTSDK2.2.1-42109
	public void  testSingleConnectionFallbackToPreferredChannelOnChannelListUponDetectionInterval()
	{
		TestUtilities.printTestHead("testSingleConnectionFallbackToPreferredChannelOnChannelListUponDetectionInterval","");
		
		String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";
		
		OmmConsumer consumer = null;
		ConsumerTestOptions consumerOption = new ConsumerTestOptions();
		ProviderTestOptions providerTestOptions = new ProviderTestOptions();
		providerTestOptions.supportStandby = true;
		providerTestOptions.sendRefreshAttrib = true;
		providerTestOptions.itemGroupId = ByteBuffer.wrap("10".getBytes());
		
		ProviderTestClient providerClient = new ProviderTestClient(providerTestOptions);
		ProviderTestClient providerClient2 = new ProviderTestClient(providerTestOptions);
		ProviderTestClient providerClient3 = new ProviderTestClient(providerTestOptions);
		
		OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);
		
		consumerOption.getChannelInformation = true;
		consumerOption.getSessionChannelInfo = false;
		ConsumerTestClient consumerClient = new ConsumerTestClient(consumerOption);
		

		// Channel_1
		OmmProvider ommprovider = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_1"), providerClient);
		assertNotNull(ommprovider);
		
		// Channel_2 /* This is preferred host */
		OmmProvider ommprovider2 = null; // EmaFactory.createOmmProvider(config.port("19002").providerName("Provider_1"), providerClient2);
		//assertNotNull(ommprovider2);
		
		// Channel_3
		OmmProvider ommprovider3 = EmaFactory.createOmmProvider(config.port("19003").providerName("Provider_1"), providerClient3);
		assertNotNull(ommprovider3);
		
		try
		{
			ConsumerTestOptions options = new ConsumerTestOptions();
			options.getChannelInformation = true;
		
			consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_40"), consumerClient);
			
			String serviceName = "DIRECT_FEED";
			String itemName = "TRI.N";
			
			ReqMsg reqMsg = EmaFactory.createReqMsg();
			long itemHandle = consumer.registerClient(reqMsg.name(itemName).serviceName(serviceName), consumerClient);
			
			Thread.sleep(2000);

			Msg message = consumerClient.popMessage();
			StatusMsg statusMsg = (StatusMsg)message;

			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Suspect / None / 'channel down'", statusMsg.state().toString());
			ChannelInformation channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_2", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.INITIALIZING, channelInfo.channelState());
			
			message = consumerClient.popMessage();
			RefreshMsg refreshMsg = (RefreshMsg)message;
						
			assertEquals(1, refreshMsg.streamId());
			assertEquals(DomainTypes.LOGIN, refreshMsg.domainType());
			assertEquals("Open / Ok / None / 'Login accepted'", refreshMsg.state().toString());
			assertTrue(refreshMsg.solicited());
			assertTrue(refreshMsg.complete());
			assertTrue(refreshMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, refreshMsg.payload().dataType());
			assertEquals(DataTypes.ELEMENT_LIST, refreshMsg.attrib().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_1", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			assertEquals("Channel_2", channelInfo.preferredHostInfo().getChannelName());

			//Checks the market price item refresh from the starting channel of WSB-G0 as WSB-G1 is down
			message = consumerClient.popMessage();
			refreshMsg = (RefreshMsg)message;
			
			assertEquals(5, refreshMsg.streamId());
			assertEquals(DomainTypes.MARKET_PRICE, refreshMsg.domainType());
			assertEquals("Open / Ok / None / 'Refresh Completed'", refreshMsg.state().toString());
			assertTrue(refreshMsg.complete());
			assertTrue(refreshMsg.solicited());
			assertTrue(refreshMsg.hasName());
			assertEquals(itemName, refreshMsg.name());
			assertTrue(refreshMsg.hasServiceId());
			assertTrue(refreshMsg.hasServiceName());
			assertEquals(serviceName, refreshMsg.serviceName());
			assertEquals(DataTypes.FIELD_LIST, refreshMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_1", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			assertEquals("Channel_2", channelInfo.preferredHostInfo().getChannelName());
			
			Thread.sleep(6000);
			
			// Checks for PH START and COMPLETE events
			message = consumerClient.popMessage();
			
			/* Checks login status messages */
			statusMsg = (StatusMsg)message;
			
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Ok / None / 'preferred host starting fallback'", statusMsg.state().toString());
			assertTrue(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_1", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			assertEquals("Channel_2", channelInfo.preferredHostInfo().getChannelName());
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Ok / None / 'preferred host complete'", statusMsg.state().toString());
			assertTrue(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_1", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			assertEquals("Channel_2", channelInfo.preferredHostInfo().getChannelName());
			
			System.out.println("Bring up the preferred channel.");
			
			// Start the provider for Channel_2 */
			ommprovider2 = EmaFactory.createOmmProvider(config.port("19002").providerName("Provider_1"), providerClient2);
			
			// The fallback should happen by the detection time interval
			System.out.println("Fallback by the detection time interval to connect to the preferred channel.");
			
			Thread.sleep(8000);
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Ok / None / 'preferred host starting fallback'", statusMsg.state().toString());
			assertTrue(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			assertEquals(DataTypes.NO_DATA, statusMsg.attrib().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_1", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			assertEquals("Channel_2", channelInfo.preferredHostInfo().getChannelName());
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Suspect / None / ''", statusMsg.state().toString());
			assertFalse(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			assertEquals(DataTypes.NO_DATA, statusMsg.attrib().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_1", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.INACTIVE, channelInfo.channelState());

			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			assertEquals(5, statusMsg.streamId());
			assertEquals(DomainTypes.MARKET_PRICE, statusMsg.domainType());
			assertTrue(statusMsg.hasState());
			assertEquals("Open / Suspect / None / 'channel down.'", statusMsg.state().toString());
			assertTrue(statusMsg.hasName());
			assertEquals(itemName, statusMsg.name());
			assertTrue(statusMsg.hasServiceId());
			assertTrue(statusMsg.hasServiceName());
			assertEquals(serviceName, statusMsg.serviceName());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_1", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.INACTIVE, channelInfo.channelState());
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Suspect / None / 'preferred host complete'", statusMsg.state().toString());
			assertTrue(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_2", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			assertEquals("Channel_2", channelInfo.preferredHostInfo().getChannelName());
			
			message = consumerClient.popMessage();
			refreshMsg = (RefreshMsg)message;
						
			assertEquals(1, refreshMsg.streamId());
			assertEquals(DomainTypes.LOGIN, refreshMsg.domainType());
			assertEquals("Open / Ok / None / 'Login accepted'", refreshMsg.state().toString());
			assertTrue(refreshMsg.solicited());
			assertTrue(refreshMsg.complete());
			assertTrue(refreshMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, refreshMsg.payload().dataType());
			assertEquals(DataTypes.ELEMENT_LIST, refreshMsg.attrib().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_2", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			assertEquals("Channel_2", channelInfo.preferredHostInfo().getChannelName());
			
			message = consumerClient.popMessage();
			/* Checks login status messages */
			statusMsg = (StatusMsg)message;
			
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Ok / None / 'channel up'", statusMsg.state().toString());
			assertTrue(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_2", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			assertEquals("Channel_2", channelInfo.preferredHostInfo().getChannelName());
			
			message = consumerClient.popMessage();

			//Checks the market price item refresh from the starting channel of WSB-G1 after the fallback is trigger 
			refreshMsg = (RefreshMsg)message;
			
			assertEquals(5, refreshMsg.streamId());
			assertEquals(DomainTypes.MARKET_PRICE, refreshMsg.domainType());
			assertEquals("Open / Ok / None / 'Refresh Completed'", refreshMsg.state().toString());
			assertTrue(refreshMsg.complete());
			assertTrue(refreshMsg.solicited());
			assertTrue(refreshMsg.hasName());
			assertEquals(itemName, refreshMsg.name());
			assertTrue(refreshMsg.hasServiceId());
			assertTrue(refreshMsg.hasServiceName());
			assertEquals(serviceName, refreshMsg.serviceName());
			assertEquals(DataTypes.FIELD_LIST, refreshMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_2", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			assertEquals("Channel_2", channelInfo.preferredHostInfo().getChannelName());
			
			Thread.sleep(10000);
			
			// Checks for PH START and COMPLETE events
			message = consumerClient.popMessage();
			
			/* Checks login status messages */
			statusMsg = (StatusMsg)message;
			
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Ok / None / 'preferred host starting fallback'", statusMsg.state().toString());
			assertTrue(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_2", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			assertEquals("Channel_2", channelInfo.preferredHostInfo().getChannelName());
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Ok / None / 'preferred host complete'", statusMsg.state().toString());
			assertTrue(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_2", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			assertEquals("Channel_2", channelInfo.preferredHostInfo().getChannelName());
			
			consumer.unregister(itemHandle);
		}
		catch(Exception excep)
		{
			System.out.println(excep);
			assertTrue(false);
		}
		finally
		{
			if(consumer != null)
				consumer.uninitialize();
			
			ommprovider.uninitialize();
			
			if(ommprovider2 != null)
				ommprovider2.uninitialize();
			
			ommprovider3.uninitialize();
		}
	}
	
	public void  testSingleConnectionEnabledPHForLoginBasedMovingFromWSBGroupToChannelListAndBackToWSBGroupWithDetectionTimeInterval()
	{
		/* This test case is used to ensure that the warm standby and preferred host feature can move back and forth between WSB and channel list multiple times. */
		
		TestUtilities.printTestHead("testSingleConnectionEnabledPHForLoginBasedMovingFromWSBGroupToChannelListAndBackToWSBGroupWithDetectionTimeInterval","");
		
		String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";

		OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);

		ProviderTestOptions providerTestOptions = new ProviderTestOptions();
		providerTestOptions.supportStandby = true;
		providerTestOptions.sendRefreshAttrib = true;
		providerTestOptions.itemGroupId = ByteBuffer.wrap("10".getBytes());

		ProviderTestClient providerClient_1 = new ProviderTestClient(providerTestOptions);
		providerClient_1.name = "Provider_1";
		ProviderTestClient providerClient_3 = new ProviderTestClient(providerTestOptions);
		providerClient_3.name = "Provider_3";
		ProviderTestClient providerClient_6 = new ProviderTestClient(providerTestOptions);
		providerClient_6.name = "Provider_6";

		// Provider_9 provides the DIRECT_FEED and DIRECT_FEED1 service name for channel.
		// Channel_1 
		OmmProvider ommprovider_1 = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_9"), providerClient_1);
		assertNotNull(ommprovider_1);

		//WSB-G0 (WarmStandbyChannel_1). The Channel_3 is the staring server.
		OmmProvider ommprovider_3 = EmaFactory.createOmmProvider(config.port("19003").providerName("Provider_9"), providerClient_3);
		assertNotNull(ommprovider_3);

		// Provider_9 provides the DIRECT_FEED and DIRECT_FEED1 service name for WSB channel. The The Channel_6 is the standby server.
		OmmProvider ommprovider_6 = EmaFactory.createOmmProvider(config.port("19006").providerName("Provider_9"), providerClient_6);
		assertNotNull(ommprovider_6);

		OmmConsumer ommconsumer = null;
		ConsumerTestOptions consumerOption = new ConsumerTestOptions();

		consumerOption.getChannelInformation = true;
		consumerOption.getSessionChannelInfo = false;
		ConsumerTestClient consumerClient = new ConsumerTestClient(consumerOption);
		
		try
		{
			ConsumerTestOptions options = new ConsumerTestOptions();
			options.getChannelInformation = true;

			ommconsumer = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_42"), consumerClient);
			
			Msg message = consumerClient.popMessage();
			RefreshMsg refreshMsg = (RefreshMsg)message;
						
			assertEquals(1, refreshMsg.streamId());
			assertEquals(DomainTypes.LOGIN, refreshMsg.domainType());
			assertEquals("Open / Ok / None / 'Login accepted'", refreshMsg.state().toString());
			assertTrue(refreshMsg.solicited());
			assertTrue(refreshMsg.complete());
			assertTrue(refreshMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, refreshMsg.payload().dataType());
			assertEquals(DataTypes.ELEMENT_LIST, refreshMsg.attrib().dataType());
			ChannelInformation channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_3", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			assertEquals("Channel_1", channelInfo.preferredHostInfo().getChannelName());
			assertEquals("WarmStandbyChannel_1", channelInfo.preferredHostInfo().getWsbChannelName());
			
			Thread.sleep(10000);
			
			// Checks for PH START and COMPLETE events
			message = consumerClient.popMessage();
			
			/* Checks login status messages */
			StatusMsg statusMsg = (StatusMsg)message;
			
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Ok / None / 'preferred host starting fallback'", statusMsg.state().toString());
			assertTrue(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_3", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			assertEquals("Channel_1", channelInfo.preferredHostInfo().getChannelName());
			assertEquals("WarmStandbyChannel_1", channelInfo.preferredHostInfo().getWsbChannelName());
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Ok / None / 'preferred host complete'", statusMsg.state().toString());
			assertTrue(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_3", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			assertEquals("Channel_1", channelInfo.preferredHostInfo().getChannelName());
			assertEquals("WarmStandbyChannel_1", channelInfo.preferredHostInfo().getWsbChannelName());
			
			System.out.println("\nBring down the WSB-G0 to connect with Channel_1");
			ommprovider_3.uninitialize(); ommprovider_3 = null;
			ommprovider_6.uninitialize(); ommprovider_6 = null;
			
			Thread.sleep(12000);
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Suspect / None / 'channel down'", statusMsg.state().toString());
			assertTrue(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			assertEquals(DataTypes.NO_DATA, statusMsg.attrib().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_6", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.CLOSED, channelInfo.channelState());
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Suspect / None / ''", statusMsg.state().toString());
			assertFalse(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			assertEquals(DataTypes.NO_DATA, statusMsg.attrib().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_6", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.CLOSED, channelInfo.channelState());
			
			message = consumerClient.popMessage();
			refreshMsg = (RefreshMsg)message;
						
			assertEquals(1, refreshMsg.streamId());
			assertEquals(DomainTypes.LOGIN, refreshMsg.domainType());
			assertEquals("Open / Ok / None / 'Login accepted'", refreshMsg.state().toString());
			assertTrue(refreshMsg.solicited());
			assertTrue(refreshMsg.complete());
			assertTrue(refreshMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, refreshMsg.payload().dataType());
			assertEquals(DataTypes.ELEMENT_LIST, refreshMsg.attrib().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_1", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			assertEquals("Channel_1", channelInfo.preferredHostInfo().getChannelName());
			assertEquals("WarmStandbyChannel_1", channelInfo.preferredHostInfo().getWsbChannelName());
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Ok / None / 'channel up'", statusMsg.state().toString());
			assertTrue(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			assertEquals(DataTypes.NO_DATA, statusMsg.attrib().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_1", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			assertEquals("Channel_1", channelInfo.preferredHostInfo().getChannelName());
			assertEquals("WarmStandbyChannel_1", channelInfo.preferredHostInfo().getWsbChannelName());
			
			// Checks for PH START and COMPLETE events
			message = consumerClient.popMessage();
			
			/* Checks login status messages */
			statusMsg = (StatusMsg)message;
			
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Ok / None / 'preferred host starting fallback'", statusMsg.state().toString());
			assertTrue(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_1", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			assertEquals("Channel_1", channelInfo.preferredHostInfo().getChannelName());
			assertEquals("WarmStandbyChannel_1", channelInfo.preferredHostInfo().getWsbChannelName());
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Ok / None / 'preferred host complete'", statusMsg.state().toString());
			assertTrue(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_1", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			assertEquals("Channel_1", channelInfo.preferredHostInfo().getChannelName());
			assertEquals("WarmStandbyChannel_1", channelInfo.preferredHostInfo().getWsbChannelName());
			
			System.out.println("\nBring up the WSB-G0 again");
			ommprovider_3 = EmaFactory.createOmmProvider(config.port("19003").providerName("Provider_9"), providerClient_3);
			ommprovider_6 = EmaFactory.createOmmProvider(config.port("19006").providerName("Provider_9"), providerClient_6);
			
			Thread.sleep(7000);
			
			message = consumerClient.popMessage();
			
			/* Checks login status messages */
			statusMsg = (StatusMsg)message;
			
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Ok / None / 'preferred host starting fallback'", statusMsg.state().toString());
			assertTrue(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_1", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			assertEquals("Channel_1", channelInfo.preferredHostInfo().getChannelName());
			assertEquals("WarmStandbyChannel_1", channelInfo.preferredHostInfo().getWsbChannelName());
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Suspect / None / 'channel down'", statusMsg.state().toString());
			assertTrue(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			assertEquals(DataTypes.NO_DATA, statusMsg.attrib().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_1", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.INACTIVE, channelInfo.channelState());
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Suspect / None / ''", statusMsg.state().toString());
			assertFalse(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			assertEquals(DataTypes.NO_DATA, statusMsg.attrib().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_1", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.INACTIVE, channelInfo.channelState());
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Suspect / None / 'preferred host complete'", statusMsg.state().toString());
			assertTrue(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_3", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			assertEquals("Channel_1", channelInfo.preferredHostInfo().getChannelName());
			assertEquals("WarmStandbyChannel_1", channelInfo.preferredHostInfo().getWsbChannelName());
			
			message = consumerClient.popMessage();
			refreshMsg = (RefreshMsg)message;
						
			assertEquals(1, refreshMsg.streamId());
			assertEquals(DomainTypes.LOGIN, refreshMsg.domainType());
			assertEquals("Open / Ok / None / 'Login accepted'", refreshMsg.state().toString());
			assertTrue(refreshMsg.solicited());
			assertTrue(refreshMsg.complete());
			assertTrue(refreshMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, refreshMsg.payload().dataType());
			assertEquals(DataTypes.ELEMENT_LIST, refreshMsg.attrib().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_3", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			assertEquals("Channel_1", channelInfo.preferredHostInfo().getChannelName());
			assertEquals("WarmStandbyChannel_1", channelInfo.preferredHostInfo().getWsbChannelName());
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Ok / None / 'channel up'", statusMsg.state().toString());
			assertTrue(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			assertEquals(DataTypes.NO_DATA, statusMsg.attrib().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_3", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			assertEquals("Channel_1", channelInfo.preferredHostInfo().getChannelName());
			assertEquals("WarmStandbyChannel_1", channelInfo.preferredHostInfo().getWsbChannelName());
			
			System.out.println("\nBrind down the starting server of WSB-GO");
			ommprovider_3.uninitialize(); ommprovider_3 = null;
			Thread.sleep(5000);
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Ok / None / 'preferred host starting fallback'", statusMsg.state().toString());
			assertTrue(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			assertEquals(DataTypes.NO_DATA, statusMsg.attrib().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_6", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			assertEquals("Channel_1", channelInfo.preferredHostInfo().getChannelName());
			assertEquals("WarmStandbyChannel_1", channelInfo.preferredHostInfo().getWsbChannelName());
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Ok / None / 'preferred host complete'", statusMsg.state().toString());
			assertTrue(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_6", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			assertEquals("Channel_1", channelInfo.preferredHostInfo().getChannelName());
			assertEquals("WarmStandbyChannel_1", channelInfo.preferredHostInfo().getWsbChannelName());
			
			System.out.println("\nBrind down the standby server of WSB-GO to connect with Channel_1");
			ommprovider_6.uninitialize(); ommprovider_6 = null;
			
			Thread.sleep(10000);
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Suspect / None / 'channel down'", statusMsg.state().toString());
			assertTrue(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			assertEquals(DataTypes.NO_DATA, statusMsg.attrib().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_6", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.CLOSED, channelInfo.channelState());
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Suspect / None / ''", statusMsg.state().toString());
			assertFalse(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			assertEquals(DataTypes.NO_DATA, statusMsg.attrib().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_6", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.CLOSED, channelInfo.channelState());
			
			/* Able to reconnect to Channel_1 */
			message = consumerClient.popMessage();
			refreshMsg = (RefreshMsg)message;
						
			assertEquals(1, refreshMsg.streamId());
			assertEquals(DomainTypes.LOGIN, refreshMsg.domainType());
			assertEquals("Open / Ok / None / 'Login accepted'", refreshMsg.state().toString());
			assertTrue(refreshMsg.solicited());
			assertTrue(refreshMsg.complete());
			assertTrue(refreshMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, refreshMsg.payload().dataType());
			assertEquals(DataTypes.ELEMENT_LIST, refreshMsg.attrib().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_1", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			assertEquals("Channel_1", channelInfo.preferredHostInfo().getChannelName());
			assertEquals("WarmStandbyChannel_1", channelInfo.preferredHostInfo().getWsbChannelName());
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Ok / None / 'channel up'", statusMsg.state().toString());
			assertTrue(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			assertEquals(DataTypes.NO_DATA, statusMsg.attrib().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_1", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			assertEquals("Channel_1", channelInfo.preferredHostInfo().getChannelName());
			assertEquals("WarmStandbyChannel_1", channelInfo.preferredHostInfo().getWsbChannelName());
			
			/* Fallback to PH doesn't do anything as the preferred WSB group is still down. */
			// Checks for PH START and COMPLETE events
			message = consumerClient.popMessage();
			
			/* Checks login status messages */
			statusMsg = (StatusMsg)message;
			
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Ok / None / 'preferred host starting fallback'", statusMsg.state().toString());
			assertTrue(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_1", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			assertEquals("Channel_1", channelInfo.preferredHostInfo().getChannelName());
			assertEquals("WarmStandbyChannel_1", channelInfo.preferredHostInfo().getWsbChannelName());
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Ok / None / 'preferred host complete'", statusMsg.state().toString());
			assertTrue(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_1", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			assertEquals("Channel_1", channelInfo.preferredHostInfo().getChannelName());
			assertEquals("WarmStandbyChannel_1", channelInfo.preferredHostInfo().getWsbChannelName());
			
			System.out.println("\nBring up the WSB-G0 again for second time");
			ommprovider_3 = EmaFactory.createOmmProvider(config.port("19003").providerName("Provider_9"), providerClient_3);
			ommprovider_6 = EmaFactory.createOmmProvider(config.port("19006").providerName("Provider_9"), providerClient_6);
			
			Thread.sleep(10000);
			
			/* Fallback to the preferred WSB group */
			message = consumerClient.popMessage();
			
			/* Checks login status messages */
			statusMsg = (StatusMsg)message;
			
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Ok / None / 'preferred host starting fallback'", statusMsg.state().toString());
			assertTrue(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_1", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			assertEquals("Channel_1", channelInfo.preferredHostInfo().getChannelName());
			assertEquals("WarmStandbyChannel_1", channelInfo.preferredHostInfo().getWsbChannelName());
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Suspect / None / 'channel down'", statusMsg.state().toString());
			assertTrue(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			assertEquals(DataTypes.NO_DATA, statusMsg.attrib().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_1", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.INACTIVE, channelInfo.channelState());
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Suspect / None / ''", statusMsg.state().toString());
			assertFalse(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			assertEquals(DataTypes.NO_DATA, statusMsg.attrib().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_1", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.INACTIVE, channelInfo.channelState());
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Suspect / None / 'preferred host complete'", statusMsg.state().toString());
			assertTrue(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_3", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			assertEquals("Channel_1", channelInfo.preferredHostInfo().getChannelName());
			assertEquals("WarmStandbyChannel_1", channelInfo.preferredHostInfo().getWsbChannelName());
			
			message = consumerClient.popMessage();
			refreshMsg = (RefreshMsg)message;
						
			assertEquals(1, refreshMsg.streamId());
			assertEquals(DomainTypes.LOGIN, refreshMsg.domainType());
			assertEquals("Open / Ok / None / 'Login accepted'", refreshMsg.state().toString());
			assertTrue(refreshMsg.solicited());
			assertTrue(refreshMsg.complete());
			assertTrue(refreshMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, refreshMsg.payload().dataType());
			assertEquals(DataTypes.ELEMENT_LIST, refreshMsg.attrib().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_3", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			assertEquals("Channel_1", channelInfo.preferredHostInfo().getChannelName());
			assertEquals("WarmStandbyChannel_1", channelInfo.preferredHostInfo().getWsbChannelName());
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Ok / None / 'channel up'", statusMsg.state().toString());
			assertTrue(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			assertEquals(DataTypes.NO_DATA, statusMsg.attrib().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_3", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			assertEquals("Channel_1", channelInfo.preferredHostInfo().getChannelName());
			assertEquals("WarmStandbyChannel_1", channelInfo.preferredHostInfo().getWsbChannelName());
			
			System.out.println("\nBrind down the standby server of WSB-GO");
			ommprovider_6.uninitialize(); ommprovider_6 = null;
			Thread.sleep(5000);
			
			// There is no fallback as the staring service is still up.
			// Checks for PH START and COMPLETE events
			message = consumerClient.popMessage();
			
			/* Checks login status messages */
			statusMsg = (StatusMsg)message;
			
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Ok / None / 'preferred host starting fallback'", statusMsg.state().toString());
			assertTrue(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_3", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			assertEquals("Channel_1", channelInfo.preferredHostInfo().getChannelName());
			assertEquals("WarmStandbyChannel_1", channelInfo.preferredHostInfo().getWsbChannelName());
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Ok / None / 'preferred host complete'", statusMsg.state().toString());
			assertTrue(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_3", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			assertEquals("Channel_1", channelInfo.preferredHostInfo().getChannelName());
			assertEquals("WarmStandbyChannel_1", channelInfo.preferredHostInfo().getWsbChannelName());
			
			System.out.println("\nBrind down the starting server of WSB-GO to connect with Channel_1");
			ommprovider_3.uninitialize(); ommprovider_3 = null;
			
			Thread.sleep(10000);
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Suspect / None / 'channel down'", statusMsg.state().toString());
			assertTrue(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			assertEquals(DataTypes.NO_DATA, statusMsg.attrib().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_3", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.CLOSED, channelInfo.channelState());
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Suspect / None / ''", statusMsg.state().toString());
			assertFalse(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			assertEquals(DataTypes.NO_DATA, statusMsg.attrib().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_3", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.CLOSED, channelInfo.channelState());
			
			/* Able to reconnect to Channel_1 */
			message = consumerClient.popMessage();
			refreshMsg = (RefreshMsg)message;
						
			assertEquals(1, refreshMsg.streamId());
			assertEquals(DomainTypes.LOGIN, refreshMsg.domainType());
			assertEquals("Open / Ok / None / 'Login accepted'", refreshMsg.state().toString());
			assertTrue(refreshMsg.solicited());
			assertTrue(refreshMsg.complete());
			assertTrue(refreshMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, refreshMsg.payload().dataType());
			assertEquals(DataTypes.ELEMENT_LIST, refreshMsg.attrib().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_1", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			assertEquals("Channel_1", channelInfo.preferredHostInfo().getChannelName());
			assertEquals("WarmStandbyChannel_1", channelInfo.preferredHostInfo().getWsbChannelName());
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Ok / None / 'channel up'", statusMsg.state().toString());
			assertTrue(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			assertEquals(DataTypes.NO_DATA, statusMsg.attrib().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_1", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			assertEquals("Channel_1", channelInfo.preferredHostInfo().getChannelName());
			assertEquals("WarmStandbyChannel_1", channelInfo.preferredHostInfo().getWsbChannelName());
			
			/* Fallback to PH doesn't do anything as the preferred WSB group is still down. */
			// Checks for PH START and COMPLETE events
			message = consumerClient.popMessage();
			
			/* Checks login status messages */
			statusMsg = (StatusMsg)message;
			
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Ok / None / 'preferred host starting fallback'", statusMsg.state().toString());
			assertTrue(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_1", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			assertEquals("Channel_1", channelInfo.preferredHostInfo().getChannelName());
			assertEquals("WarmStandbyChannel_1", channelInfo.preferredHostInfo().getWsbChannelName());
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Ok / None / 'preferred host complete'", statusMsg.state().toString());
			assertTrue(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_1", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			assertEquals("Channel_1", channelInfo.preferredHostInfo().getChannelName());
			assertEquals("WarmStandbyChannel_1", channelInfo.preferredHostInfo().getWsbChannelName());
			
			System.out.println("\nBring up the WSB-G0 again for third time");
			ommprovider_3 = EmaFactory.createOmmProvider(config.port("19003").providerName("Provider_9"), providerClient_3);
			ommprovider_6 = EmaFactory.createOmmProvider(config.port("19006").providerName("Provider_9"), providerClient_6);
			Thread.sleep(10000);
			
			/* Fallback to the preferred WSB group */
			message = consumerClient.popMessage();
			
			/* Checks login status messages */
			statusMsg = (StatusMsg)message;
			
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Ok / None / 'preferred host starting fallback'", statusMsg.state().toString());
			assertTrue(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_1", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			assertEquals("Channel_1", channelInfo.preferredHostInfo().getChannelName());
			assertEquals("WarmStandbyChannel_1", channelInfo.preferredHostInfo().getWsbChannelName());
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Suspect / None / 'channel down'", statusMsg.state().toString());
			assertTrue(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			assertEquals(DataTypes.NO_DATA, statusMsg.attrib().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_1", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.INACTIVE, channelInfo.channelState());
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Suspect / None / ''", statusMsg.state().toString());
			assertFalse(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			assertEquals(DataTypes.NO_DATA, statusMsg.attrib().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_1", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.INACTIVE, channelInfo.channelState());
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Suspect / None / 'preferred host complete'", statusMsg.state().toString());
			assertTrue(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_3", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			assertEquals("Channel_1", channelInfo.preferredHostInfo().getChannelName());
			assertEquals("WarmStandbyChannel_1", channelInfo.preferredHostInfo().getWsbChannelName());
			
			message = consumerClient.popMessage();
			refreshMsg = (RefreshMsg)message;
						
			assertEquals(1, refreshMsg.streamId());
			assertEquals(DomainTypes.LOGIN, refreshMsg.domainType());
			assertEquals("Open / Ok / None / 'Login accepted'", refreshMsg.state().toString());
			assertTrue(refreshMsg.solicited());
			assertTrue(refreshMsg.complete());
			assertTrue(refreshMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, refreshMsg.payload().dataType());
			assertEquals(DataTypes.ELEMENT_LIST, refreshMsg.attrib().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_3", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			assertEquals("Channel_1", channelInfo.preferredHostInfo().getChannelName());
			assertEquals("WarmStandbyChannel_1", channelInfo.preferredHostInfo().getWsbChannelName());
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Ok / None / 'channel up'", statusMsg.state().toString());
			assertTrue(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			assertEquals(DataTypes.NO_DATA, statusMsg.attrib().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_3", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			assertEquals("Channel_1", channelInfo.preferredHostInfo().getChannelName());
			assertEquals("WarmStandbyChannel_1", channelInfo.preferredHostInfo().getWsbChannelName());
			
			System.out.println("\nBring down all servers and wait for OmmConsuemr to reconnect");
			ommprovider_1.uninitialize(); ommprovider_1 = null;
			ommprovider_3.uninitialize(); ommprovider_3 = null;
			ommprovider_6.uninitialize(); ommprovider_6 = null;
			
			Thread.sleep(15000);
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Suspect / None / 'channel down'", statusMsg.state().toString());
			assertTrue(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			assertEquals(DataTypes.NO_DATA, statusMsg.attrib().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_6", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.CLOSED, channelInfo.channelState());
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Suspect / None / ''", statusMsg.state().toString());
			assertFalse(statusMsg.hasMsgKey());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			assertEquals(DataTypes.NO_DATA, statusMsg.attrib().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_6", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.CLOSED, channelInfo.channelState());
		}
		catch(Exception exp)
		{
			System.out.println("Exception message: " + exp.getMessage());
			assertTrue(false);
		}
		finally
		{
			ommconsumer.uninitialize();
			
			if(ommprovider_1 != null)
				ommprovider_1.uninitialize();
			
			if(ommprovider_3 != null)
				ommprovider_3.uninitialize();
			
			if(ommprovider_6 != null)
				ommprovider_6.uninitialize();
		}
	}
}
