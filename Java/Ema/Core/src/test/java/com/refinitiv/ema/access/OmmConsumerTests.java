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
import junit.framework.TestCase;

import static org.junit.Assert.assertThrows;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.List;

import org.junit.*;
import org.junit.runners.MethodSorters;

import com.refinitiv.ema.access.ChannelInformation.ChannelState;
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
}
