/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

import com.refinitiv.ema.JUnitConfigVariables;
import com.refinitiv.ema.RetryRule;
import com.refinitiv.ema.access.unittest.requestrouting.ConsumerTestClient;
import com.refinitiv.ema.access.unittest.requestrouting.ConsumerTestOptions;
import com.refinitiv.ema.access.unittest.requestrouting.ProviderTestClient;
import com.refinitiv.ema.access.unittest.requestrouting.ProviderTestOptions;
import com.refinitiv.ema.rdm.EmaRdm;
import com.refinitiv.ema.unittest.TestUtilities;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.rdm.SymbolList;
import org.junit.After;
import org.junit.Rule;
import org.junit.Test;

import java.nio.ByteBuffer;
import java.util.*;

import static org.junit.Assert.*;

public class OmmEnhancedSymbolListTests
{
    public OmmEnhancedSymbolListTests()
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
    public void testRequestingSingleSymbolListItem()
    {
        /*
            Consumer application requests Symbol List item, receives Refresh for it.
            API automatically requests Market Price items provided in Symbol List Refresh, consumer gets Refreshes for them.
            Consumer successfully unregisters one of these Market Price items.
         */
        TestUtilities.printTestHead("testRequestingSingleSymbolListItem","");
        String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";
        OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);
        ProviderTestOptions providerTestOptions = new ProviderTestOptions();
        ProviderTestClient providerClient1 = new ProviderTestClient(providerTestOptions);
        providerTestOptions.itemGroupId = ByteBuffer.wrap("10".getBytes());

        // Provider_1 provides the DIRECT_FEED service name
        OmmProvider ommprovider = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_1"), providerClient1);

        assertNotNull(ommprovider);

        OmmConsumer consumer = null;
        ConsumerTestClient consumerClient = new ConsumerTestClient();

        try
        {
            consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_1"), consumerClient);

            ElementList payload = EmaFactory.createElementList();
            ElementEntry entry = EmaFactory.createElementEntry();

            ElementList eePayload = EmaFactory.createElementList();
            ElementEntry eeEntry = EmaFactory.createElementEntry();

            eeEntry.uintValue(":DataStreams", SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_STREAMS);
            eePayload.add(eeEntry);

            entry.elementList(":SymbolListBehaviors", eePayload);
            payload.add(entry);

            ReqMsg reqMsg = EmaFactory.createReqMsg().domainType(EmaRdm.MMT_SYMBOL_LIST).serviceName("DIRECT_FEED").name(".AV.N").payload(payload);

            Thread.sleep(1000);

            consumerClient.clearQueue();

            long itemHandle = consumer.registerClient(reqMsg, consumerClient);

            Thread.sleep(3000);

            assertEquals(4, providerClient1.queueSize());

            Msg message = providerClient1.popMessage();

            assertTrue(message instanceof ReqMsg);
            ReqMsg requestMsg = (ReqMsg)message;

            assertEquals("DIRECT_FEED", requestMsg.serviceName());
            assertEquals(".AV.N", requestMsg.name());

            HashSet<String> names = new HashSet<String>();
            for (int i = 0; i < 3; i++)
            {
                message = providerClient1.popMessage();
                requestMsg = (ReqMsg)message;
                assertEquals("DIRECT_FEED", requestMsg.serviceName());
                assertTrue("itemA".equals(requestMsg.name()) || "itemB".equals(requestMsg.name()) || "itemC".equals(requestMsg.name()));
                names.add(requestMsg.name());
            }
            assertEquals(names.size(), 3);

            assertEquals(4, consumerClient.queueSize());

            message = consumerClient.popMessage();
            assertTrue(message instanceof RefreshMsg);

            RefreshMsg refresh = (RefreshMsg)message;
            assertTrue(refresh.hasName());
            assertEquals(".AV.N", refresh.name());

            names.clear();
            for (int i = 0; i < 3; i++)
            {
                message = consumerClient.popMessage();
                assertTrue(message instanceof RefreshMsg);

                refresh = (RefreshMsg)message;
                assertTrue(refresh.hasName());
                assertTrue("itemA".equals(refresh.name()) || "itemB".equals(refresh.name()) || "itemC".equals(refresh.name()));
                names.add(refresh.name());
            }
            assertEquals(names.size(), 3);

            long itemBHandle = consumerClient.handleNameMap.get("itemB");
            consumer.unregister(itemBHandle);

            Thread.sleep(2000);

            assertEquals(1, providerClient1.queueSize());
            message = providerClient1.popMessage();
            assertTrue(message instanceof ReqMsg);
            requestMsg = (ReqMsg)message;
            assertTrue("itemB".equals(requestMsg.name()));

            consumer.unregister(itemHandle);

            Thread.sleep(1000);

            assertEquals(1, providerClient1.queueSize());
            message = providerClient1.popMessage();
            assertTrue(message instanceof ReqMsg);
            requestMsg = (ReqMsg)message;
            assertTrue(".AV.N".equals(requestMsg.name()));

            assertNotNull(consumer);
        }
        catch (OmmException excep)
        {
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
    public void testRequestingSymbolListItem_MarketPriceItemsRecovery()
    {
        /*
            Consumer application requests Symbol List item, receives Refresh for it.
            API automatically requests Market Price items provided in Symbol List Refresh, consumer gets Refreshes for them.
            Consumer successfully unregisters one of these Market Price items and the Symbol List item itself.
            Then Provider goes down and up again. The Consumer application successfully recovers the two remaining
            Market Price items, but not the Symbol List and Market Price items consumer unregistered.
         */
        TestUtilities.printTestHead("testRequestingSymbolListItem_MarketPriceItemsRecovery","");
        String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";
        OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);
        ProviderTestOptions providerTestOptions = new ProviderTestOptions();
        ProviderTestClient providerClient1 = new ProviderTestClient(providerTestOptions);
        providerTestOptions.itemGroupId = ByteBuffer.wrap("10".getBytes());

        // Provider_1 provides the DIRECT_FEED service name
        OmmProvider ommprovider = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_1"), providerClient1);

        assertNotNull(ommprovider);

        OmmConsumer consumer = null;
        ConsumerTestClient consumerClient = new ConsumerTestClient();

        try
        {
            consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_1"), consumerClient);

            ElementList payload = EmaFactory.createElementList();
            ElementEntry entry = EmaFactory.createElementEntry();

            ElementList eePayload = EmaFactory.createElementList();
            ElementEntry eeEntry = EmaFactory.createElementEntry();

            eeEntry.uintValue(":DataStreams", SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_STREAMS);
            eePayload.add(eeEntry);

            entry.elementList(":SymbolListBehaviors", eePayload);
            payload.add(entry);

            ReqMsg reqMsg = EmaFactory.createReqMsg().domainType(EmaRdm.MMT_SYMBOL_LIST).serviceName("DIRECT_FEED").name(".AV.N").payload(payload);

            Thread.sleep(1000);

            consumerClient.clearQueue();

            long itemHandle = consumer.registerClient(reqMsg, consumerClient);

            Thread.sleep(3000);

            assertEquals(4, providerClient1.queueSize());

            Msg message = providerClient1.popMessage();

            assertTrue(message instanceof ReqMsg);
            ReqMsg requestMsg = (ReqMsg)message;

            assertEquals("DIRECT_FEED", requestMsg.serviceName());
            assertEquals(".AV.N", requestMsg.name());

            HashSet<String> names = new HashSet<String>();
            for (int i = 0; i < 3; i++)
            {
                message = providerClient1.popMessage();
                requestMsg = (ReqMsg)message;
                assertEquals("DIRECT_FEED", requestMsg.serviceName());
                assertTrue("itemA".equals(requestMsg.name()) || "itemB".equals(requestMsg.name()) || "itemC".equals(requestMsg.name()));
                names.add(requestMsg.name());
            }
            assertEquals(names.size(), 3);

            assertEquals(4, consumerClient.queueSize());

            message = consumerClient.popMessage();
            assertTrue(message instanceof RefreshMsg);

            RefreshMsg refresh = (RefreshMsg)message;
            assertTrue(refresh.hasName());
            assertEquals(".AV.N", refresh.name());

            names.clear();
            for (int i = 0; i < 3; i++)
            {
                message = consumerClient.popMessage();
                assertTrue(message instanceof RefreshMsg);

                refresh = (RefreshMsg)message;
                assertTrue(refresh.hasName());
                assertTrue("itemA".equals(refresh.name()) || "itemB".equals(refresh.name()) || "itemC".equals(refresh.name()));
                names.add(refresh.name());
            }
            assertEquals(names.size(), 3);

            long itemBHandle = consumerClient.handleNameMap.get("itemB");
            consumer.unregister(itemBHandle);

            Thread.sleep(1000);

            assertEquals(1, providerClient1.queueSize());
            message = providerClient1.popMessage();
            assertTrue(message instanceof ReqMsg);
            requestMsg = (ReqMsg)message;
            assertTrue("itemB".equals(requestMsg.name()));

            consumer.unregister(itemHandle);

            ommprovider.uninitialize();

            providerClient1 = new ProviderTestClient(providerTestOptions);
            ommprovider = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_1"), providerClient1);

            Thread.sleep(8000);

            names.clear();
            int count = providerClient1.queueSize();
            for (int i = 0; i < count; i++)
            {
                message = providerClient1.popMessage();
                if (message instanceof ReqMsg)
                {
                    requestMsg = (ReqMsg)message;
                    if (requestMsg.hasName()
                            && ( requestMsg.name().equals("itemA") || requestMsg.name().equals("itemB")
                            || requestMsg.name().equals("itemC") || requestMsg.name().equals(".AV.N")) )
                    {
                        names.add(requestMsg.name());
                    }
                }
            }
            assertEquals(2, names.size());
            assertFalse(names.contains("itemB"));
            assertFalse(names.contains(".AV.N"));

            long itemAHandle = consumerClient.handleNameMap.get("itemA");
            consumer.unregister(itemAHandle);

            Thread.sleep(1000);

            assertEquals(1, providerClient1.queueSize());
            message = providerClient1.popMessage();
            assertTrue(message instanceof ReqMsg);
            requestMsg = (ReqMsg)message;
            assertTrue("itemA".equals(requestMsg.name()));

            assertNotNull(consumer);
        }
        catch (OmmException excep)
        {
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
    public void testRequestingSymbolListItem_SymbolListAndMarketPriceItemsRecovery()
    {
        /*
            Consumer application requests Symbol List item, receives Refresh for it.
            API automatically requests Market Price items provided in Symbol List Refresh, consumer gets Refreshes for them.
            Consumer successfully unregisters one of these Market Price items.
            Then Provider goes down and up again.
            The Consumer application successfully recovers all four items - the Symbol List item and the three
            Market Price items associated with it (the two Market Price items that were not unregistered before
            Provider restart should not be requested twice!).
         */
        TestUtilities.printTestHead("testRequestingSymbolListItem_SymbolListAndMarketPriceItemsRecovery","");
        String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";
        OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);
        ProviderTestOptions providerTestOptions = new ProviderTestOptions();
        ProviderTestClient providerClient1 = new ProviderTestClient(providerTestOptions);
        providerTestOptions.itemGroupId = ByteBuffer.wrap("10".getBytes());

        // Provider_1 provides the DIRECT_FEED service name
        OmmProvider ommprovider = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_1"), providerClient1);

        assertNotNull(ommprovider);

        OmmConsumer consumer = null;
        ConsumerTestClient consumerClient = new ConsumerTestClient();

        try
        {
            consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_1"), consumerClient);

            ElementList payload = EmaFactory.createElementList();
            ElementEntry entry = EmaFactory.createElementEntry();

            ElementList eePayload = EmaFactory.createElementList();
            ElementEntry eeEntry = EmaFactory.createElementEntry();

            eeEntry.uintValue(":DataStreams", SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_STREAMS);
            eePayload.add(eeEntry);

            entry.elementList(":SymbolListBehaviors", eePayload);
            payload.add(entry);

            ReqMsg reqMsg = EmaFactory.createReqMsg().domainType(EmaRdm.MMT_SYMBOL_LIST).serviceName("DIRECT_FEED").name(".AV.N").payload(payload);

            Thread.sleep(1000);

            consumerClient.clearQueue();

            long itemHandle = consumer.registerClient(reqMsg, consumerClient);

            Thread.sleep(3000);

            assertEquals(4, providerClient1.queueSize());

            Msg message = providerClient1.popMessage();

            assertTrue(message instanceof ReqMsg);
            ReqMsg requestMsg = (ReqMsg)message;

            assertEquals("DIRECT_FEED", requestMsg.serviceName());
            assertEquals(".AV.N", requestMsg.name());

            HashSet<String> names = new HashSet<String>();
            for (int i = 0; i < 3; i++)
            {
                message = providerClient1.popMessage();
                requestMsg = (ReqMsg)message;
                assertEquals("DIRECT_FEED", requestMsg.serviceName());
                assertTrue("itemA".equals(requestMsg.name()) || "itemB".equals(requestMsg.name()) || "itemC".equals(requestMsg.name()));
                names.add(requestMsg.name());
            }
            assertEquals(names.size(), 3);

            assertEquals(4, consumerClient.queueSize());

            message = consumerClient.popMessage();
            assertTrue(message instanceof RefreshMsg);

            RefreshMsg refresh = (RefreshMsg)message;
            assertTrue(refresh.hasName());
            assertEquals(".AV.N", refresh.name());

            names.clear();
            for (int i = 0; i < 3; i++)
            {
                message = consumerClient.popMessage();
                assertTrue(message instanceof RefreshMsg);

                refresh = (RefreshMsg)message;
                assertTrue(refresh.hasName());
                assertTrue("itemA".equals(refresh.name()) || "itemB".equals(refresh.name()) || "itemC".equals(refresh.name()));
                names.add(refresh.name());
            }
            assertEquals(names.size(), 3);

            long itemBHandle = consumerClient.handleNameMap.get("itemB");
            consumer.unregister(itemBHandle);

            Thread.sleep(1000);

            assertEquals(1, providerClient1.queueSize());
            message = providerClient1.popMessage();
            assertTrue(message instanceof ReqMsg);
            requestMsg = (ReqMsg)message;
            assertTrue("itemB".equals(requestMsg.name()));

            ommprovider.uninitialize();

            providerClient1 = new ProviderTestClient(providerTestOptions);
            ommprovider = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_1"), providerClient1);

            Thread.sleep(8000);

            names.clear();
            int count = consumerClient.queueSize();
            for (int i = 0; i < count; i++)
            {
                message = providerClient1.popMessage();
                if (message instanceof ReqMsg)
                {
                    requestMsg = (ReqMsg)message;
                    if (requestMsg.hasName()
                            && ( requestMsg.name().equals("itemA") || requestMsg.name().equals("itemB")
                            || requestMsg.name().equals("itemC") || requestMsg.name().equals(".AV.N")) )
                    {
                        names.add(requestMsg.name());
                    }
                }
            }
            assertEquals(4, names.size());

            long itemAHandle = consumerClient.handleNameMap.get("itemA");
            consumer.unregister(itemAHandle);

            Thread.sleep(1000);

            assertEquals(1, providerClient1.queueSize());
            message = providerClient1.popMessage();
            assertTrue(message instanceof ReqMsg);
            requestMsg = (ReqMsg)message;
            assertTrue("itemA".equals(requestMsg.name()));

            assertNotNull(consumer);
        }
        catch (OmmException excep)
        {
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
    public void testRequestingSymbolListItem_SymbolListAndMarketPriceItemsRecovery_PrivateStream()
    {
        /*
            Consumer requests symbol list item on private stream, receives Refresh for Symbol List with 3 Market Price items.
            Consumer unregisters one item. After that provider restarts. Consumer doesn't recover Symbol List item since it is private
            but recovers the remaining market price items
         */
        TestUtilities.printTestHead("testRequestingSymbolListItem_SymbolListAndMarketPriceItemsRecovery","");
        String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";
        OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);
        ProviderTestOptions providerTestOptions = new ProviderTestOptions();
        ProviderTestClient providerClient1 = new ProviderTestClient(providerTestOptions);
        providerTestOptions.itemGroupId = ByteBuffer.wrap("10".getBytes());

        // Provider_1 provides the DIRECT_FEED service name
        OmmProvider ommprovider = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_1"), providerClient1);

        assertNotNull(ommprovider);

        OmmConsumer consumer = null;
        ConsumerTestClient consumerClient = new ConsumerTestClient();

        try
        {
            consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_1"), consumerClient);

            ElementList payload = EmaFactory.createElementList();
            ElementEntry entry = EmaFactory.createElementEntry();

            ElementList eePayload = EmaFactory.createElementList();
            ElementEntry eeEntry = EmaFactory.createElementEntry();

            eeEntry.uintValue(":DataStreams", SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_STREAMS);
            eePayload.add(eeEntry);

            entry.elementList(":SymbolListBehaviors", eePayload);
            payload.add(entry);

            ReqMsg reqMsg = EmaFactory.createReqMsg().domainType(EmaRdm.MMT_SYMBOL_LIST).serviceName("DIRECT_FEED").name(".AV.N").privateStream(true).payload(payload);

            Thread.sleep(1000);

            consumerClient.clearQueue();

            long itemHandle = consumer.registerClient(reqMsg, consumerClient);

            Thread.sleep(2000);

            assertEquals(4, providerClient1.queueSize());

            Msg message = providerClient1.popMessage();

            assertTrue(message instanceof ReqMsg);
            ReqMsg requestMsg = (ReqMsg)message;

            assertEquals("DIRECT_FEED", requestMsg.serviceName());
            assertEquals(".AV.N", requestMsg.name());

            HashSet<String> names = new HashSet<String>();
            for (int i = 0; i < 3; i++)
            {
                message = providerClient1.popMessage();
                requestMsg = (ReqMsg)message;
                assertEquals("DIRECT_FEED", requestMsg.serviceName());
                assertTrue("itemA".equals(requestMsg.name()) || "itemB".equals(requestMsg.name()) || "itemC".equals(requestMsg.name()));
                names.add(requestMsg.name());
            }
            assertEquals(names.size(), 3);

            assertEquals(4, consumerClient.queueSize());

            message = consumerClient.popMessage();
            assertTrue(message instanceof RefreshMsg);

            RefreshMsg refresh = (RefreshMsg)message;
            assertTrue(refresh.hasName());
            assertEquals(".AV.N", refresh.name());

            names.clear();
            for (int i = 0; i < 3; i++)
            {
                message = consumerClient.popMessage();
                assertTrue(message instanceof RefreshMsg);

                refresh = (RefreshMsg)message;
                assertTrue(refresh.hasName());
                assertTrue("itemA".equals(refresh.name()) || "itemB".equals(refresh.name()) || "itemC".equals(refresh.name()));
                names.add(refresh.name());
            }
            assertEquals(names.size(), 3);

            long itemBHandle = consumerClient.handleNameMap.get("itemB");
            consumer.unregister(itemBHandle);

            Thread.sleep(1000);

            assertEquals(1, providerClient1.queueSize());
            message = providerClient1.popMessage();
            assertTrue(message instanceof ReqMsg);
            requestMsg = (ReqMsg)message;
            assertTrue("itemB".equals(requestMsg.name()));

            ommprovider.uninitialize();

            providerClient1 = new ProviderTestClient(providerTestOptions);
            ommprovider = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_1"), providerClient1);

            Thread.sleep(4000);

            names.clear();
            int count = consumerClient.queueSize();
            for (int i = 0; i < count; i++)
            {
                message = providerClient1.popMessage();
                if (message instanceof ReqMsg)
                {
                    requestMsg = (ReqMsg)message;
                    if (requestMsg.hasName()
                            && ( requestMsg.name().equals("itemA") || requestMsg.name().equals("itemB")
                            || requestMsg.name().equals("itemC") || requestMsg.name().equals(".AV.N")) )
                    {
                        names.add(requestMsg.name());
                    }
                }
            }
            assertEquals(2, names.size()); // private stream is not recovered

            long itemAHandle = consumerClient.handleNameMap.get("itemA");
            consumer.unregister(itemAHandle);

            Thread.sleep(1000);

            assertEquals(1, providerClient1.queueSize());
            message = providerClient1.popMessage();
            assertTrue(message instanceof ReqMsg);
            requestMsg = (ReqMsg)message;
            assertTrue("itemA".equals(requestMsg.name()));

            assertNotNull(consumer);
        }
        catch (OmmException excep)
        {
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
    public void testRequestingTwoSymbolListItems_SameItemListInRefresh()
    {
        /*
            Consumer sends two Symbol List requests for different items (service is the same).
            Market Price items are the same in Refreshes for both Symbol List items.
            Consumer application should receive Market Price item refreshes only for one set of Market Price items.
         */
        TestUtilities.printTestHead("testRequestingSingleSymbolListItem","");
        String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";
        OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);
        ProviderTestOptions providerTestOptions = new ProviderTestOptions();
        ProviderTestClient providerClient1 = new ProviderTestClient(providerTestOptions);
        providerTestOptions.itemGroupId = ByteBuffer.wrap("10".getBytes());

        // Provider_1 provides the DIRECT_FEED service name
        OmmProvider ommprovider = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_1"), providerClient1);

        assertNotNull(ommprovider);

        OmmConsumer consumer = null;
        ConsumerTestClient consumerClient = new ConsumerTestClient();

        try
        {
            consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_1"), consumerClient);

            ElementList payload = EmaFactory.createElementList();
            ElementEntry entry = EmaFactory.createElementEntry();

            ElementList eePayload = EmaFactory.createElementList();
            ElementEntry eeEntry = EmaFactory.createElementEntry();

            eeEntry.uintValue(":DataStreams", SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_STREAMS);
            eePayload.add(eeEntry);

            entry.elementList(":SymbolListBehaviors", eePayload);
            payload.add(entry);

            ReqMsg reqMsg = EmaFactory.createReqMsg().domainType(EmaRdm.MMT_SYMBOL_LIST).serviceName("DIRECT_FEED").name(".AV.N").payload(payload);

            Thread.sleep(1000);

            consumerClient.clearQueue();

            long itemHandle = consumer.registerClient(reqMsg, consumerClient);

            long itemHandle2 = consumer.registerClient(reqMsg.clear().name(".BV.N").domainType(EmaRdm.MMT_SYMBOL_LIST).serviceName("DIRECT_FEED").payload(payload), consumerClient);

            Thread.sleep(3000);

            assertEquals(5, providerClient1.queueSize());

            Msg message;
            ReqMsg requestMsg;

            HashSet<String> slNames = new HashSet<>();
            for (int i = 0; i < 2; i++)
            {
                message = providerClient1.popMessage();
                assertTrue(message instanceof ReqMsg);
                requestMsg = (ReqMsg)message;

                assertEquals("DIRECT_FEED", requestMsg.serviceName());
                assertTrue(".AV.N".equals(requestMsg.name()) || ".BV.N".equals(requestMsg.name()));

                slNames.add(requestMsg.name());
            }
            assertEquals(2, slNames.size());

            HashSet<String> names = new HashSet<String>();
            for (int i = 0; i < 3; i++)
            {
                message = providerClient1.popMessage();
                assertTrue(message instanceof ReqMsg);
                requestMsg = (ReqMsg)message;
                assertEquals("DIRECT_FEED", requestMsg.serviceName());
                assertTrue("itemA".equals(requestMsg.name()) || "itemB".equals(requestMsg.name()) || "itemC".equals(requestMsg.name()));
                names.add(requestMsg.name());
            }
            assertEquals(names.size(), 3);

            assertEquals(5, consumerClient.queueSize());

            slNames.clear();
            names.clear();
            RefreshMsg refresh;

            for (int i = 0; i< 5; i++)
            {
                message = consumerClient.popMessage();
                assertTrue(message instanceof RefreshMsg);

                refresh = (RefreshMsg)message;
                assertTrue(refresh.hasName());

                if (refresh.domainType() == DomainTypes.SYMBOL_LIST)
                {
                    assertTrue(".AV.N".equals(refresh.name()) || ".BV.N".equals(refresh.name()));
                    slNames.add(refresh.name());
                }
                else if (refresh.domainType() == DomainTypes.MARKET_PRICE)
                {
                    assertTrue("itemA".equals(refresh.name()) || "itemB".equals(refresh.name()) || "itemC".equals(refresh.name()));
                    names.add(refresh.name());
                }
            }

            assertEquals(names.size(), 3);
            assertEquals(slNames.size(), 2);

            assertNotNull(consumer);
        }
        catch (OmmException excep)
        {
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
    public void testRequestingTwoSymbolListItems_DifferentServiceNames()
    {
        /*
            Consumer requests two symbol list items from different services DIRECT_FEED and DIRECT_FEED1.
            Consumer application should automatically receive two sets of Market Price items for both Symbol List requests
            even though they have all characteristics (name, Qos) the same apart from the service.
        */
        TestUtilities.printTestHead("testRequestingSingleSymbolListItem","");
        String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";
        OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);
        ProviderTestOptions providerTestOptions = new ProviderTestOptions();
        ProviderTestClient providerClient1 = new ProviderTestClient(providerTestOptions);
        providerTestOptions.itemGroupId = ByteBuffer.wrap("10".getBytes());

        // Provider_1 provides the DIRECT_FEED service name
        OmmProvider ommprovider = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_8"), providerClient1);

        assertNotNull(ommprovider);

        OmmConsumer consumer = null;
        ConsumerTestClient consumerClient = new ConsumerTestClient();

        try
        {
            consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_1"), consumerClient);

            ElementList payload = EmaFactory.createElementList();
            ElementEntry entry = EmaFactory.createElementEntry();

            ElementList eePayload = EmaFactory.createElementList();
            ElementEntry eeEntry = EmaFactory.createElementEntry();

            eeEntry.uintValue(":DataStreams", SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_STREAMS);
            eePayload.add(eeEntry);

            entry.elementList(":SymbolListBehaviors", eePayload);
            payload.add(entry);

            ReqMsg reqMsg = EmaFactory.createReqMsg().domainType(EmaRdm.MMT_SYMBOL_LIST).serviceName("DIRECT_FEED").name(".AV.N").payload(payload);

            Thread.sleep(1000);

            consumerClient.clearQueue();

            long itemHandle = consumer.registerClient(reqMsg, consumerClient);
            long itemHandle2 = consumer.registerClient(reqMsg.clear().name(".BV.N").domainType(EmaRdm.MMT_SYMBOL_LIST).serviceName("DIRECT_FEED1").payload(payload), consumerClient);

            Thread.sleep(3000);

            assertEquals(8, providerClient1.queueSize()); // we should receive item requests for 3 MARKET_PRICE items for serviceId 1 and serviceId 2 + 2 symbol list requests

            Msg message;
            ReqMsg requestMsg;

            HashSet<String> slNames = new HashSet<>();
            int[] mpNames = new int[3];
            for (int i = 0; i < 8; i++)
            {
                message = providerClient1.popMessage();
                assertTrue(message instanceof ReqMsg);
                requestMsg = (ReqMsg)message;

                if (requestMsg.domainType() == DomainTypes.SYMBOL_LIST)
                {
                    assertTrue(".AV.N".equals(requestMsg.name()) || ".BV.N".equals(requestMsg.name()));
                    slNames.add(requestMsg.name());
                }
                else if (requestMsg.domainType() == DomainTypes.MARKET_PRICE)
                {
                    assertTrue("itemA".equals(requestMsg.name()) || "itemB".equals(requestMsg.name()) || "itemC".equals(requestMsg.name()));
                    switch (requestMsg.name())
                    {
                        case "itemA": mpNames[0] += 1; break;
                        case "itemB": mpNames[1] += 1; break;
                        case "itemC": mpNames[2] += 1; break;
                        default: assertFalse(true);
                    }
                }

                assertTrue("DIRECT_FEED".equals(requestMsg.serviceName()) || "DIRECT_FEED1".equals(requestMsg.serviceName()));
            }
            assertEquals(2, slNames.size());

            for (int i = 0; i < 3; i++) assertEquals(2, mpNames[i]);
            assertEquals(8, consumerClient.queueSize());

            for (int i = 0; i < 3; i++) mpNames[i] = 0;
            slNames.clear();

            RefreshMsg refresh;

            for (int i = 0; i< 8; i++)
            {
                message = consumerClient.popMessage();
                assertTrue(message instanceof RefreshMsg);

                refresh = (RefreshMsg)message;
                assertTrue(refresh.hasName());

                if (refresh.domainType() == DomainTypes.SYMBOL_LIST)
                {
                    assertTrue(".AV.N".equals(refresh.name()) || ".BV.N".equals(refresh.name()));
                    slNames.add(refresh.name());
                }
                else if (refresh.domainType() == DomainTypes.MARKET_PRICE)
                {
                    assertTrue("itemA".equals(refresh.name()) || "itemB".equals(refresh.name()) || "itemC".equals(refresh.name()));
                    switch (refresh.name())
                    {
                        case "itemA": mpNames[0] += 1; break;
                        case "itemB": mpNames[1] += 1; break;
                        case "itemC": mpNames[2] += 1; break;
                        default: assertFalse(true);
                    }
                }

                assertTrue("DIRECT_FEED".equals(refresh.serviceName()) || "DIRECT_FEED1".equals(refresh.serviceName()));
            }

            assertEquals(2, slNames.size());
            for (int i = 0; i < 3; i++) assertEquals(2, mpNames[i]);

            assertNotNull(consumer);
        }
        catch (OmmException excep)
        {
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
    public void testRequestingSymbolListItem_SymbolListAndMarketPriceItemsReceived_SendGenericMsgOnMarketPriceStream()
    {
        /*
            When Consumer requests Symbol List item and API automatically opens streams for Market Price items from
            Symbol List Refresh, the Consumer and Provider should be able to successfully send Generic messages on the
            Market Price streams.
         */
        TestUtilities.printTestHead("testRequestingSymbolListItem_SymbolListAndMarketPriceItemsReceived_SendGeneriAndPostMsgOnMarketPriceStream","");
        String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";
        OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);
        ProviderTestOptions providerTestOptions = new ProviderTestOptions();
        ProviderTestClient providerClient1 = new ProviderTestClient(providerTestOptions);
        providerTestOptions.itemGroupId = ByteBuffer.wrap("10".getBytes());

        // Provider_1 provides the DIRECT_FEED service name
        OmmProvider ommprovider = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_1"), providerClient1);

        assertNotNull(ommprovider);

        OmmConsumer consumer = null;
        ConsumerTestClient consumerClient = new ConsumerTestClient();

        try
        {
            consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_1"), consumerClient);

            ElementList payload = EmaFactory.createElementList();
            ElementEntry entry = EmaFactory.createElementEntry();

            ElementList eePayload = EmaFactory.createElementList();
            ElementEntry eeEntry = EmaFactory.createElementEntry();

            eeEntry.uintValue(":DataStreams", SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_STREAMS);
            eePayload.add(eeEntry);

            entry.elementList(":SymbolListBehaviors", eePayload);
            payload.add(entry);

            ReqMsg reqMsg = EmaFactory.createReqMsg().domainType(EmaRdm.MMT_SYMBOL_LIST).serviceName("DIRECT_FEED").name(".AV.N").payload(payload);

            Thread.sleep(1000);

            consumerClient.clearQueue();

            long itemHandle = consumer.registerClient(reqMsg, consumerClient);

            Thread.sleep(3000);

            assertEquals(4, providerClient1.queueSize());

            Msg message = providerClient1.popMessage();

            assertTrue(message instanceof ReqMsg);
            ReqMsg requestMsg = (ReqMsg)message;

            assertEquals("DIRECT_FEED", requestMsg.serviceName());
            assertEquals(".AV.N", requestMsg.name());

            int itemBStreamId = 0;

            HashSet<String> names = new HashSet<String>();
            for (int i = 0; i < 3; i++)
            {
                message = providerClient1.popMessage();
                requestMsg = (ReqMsg)message;
                assertEquals("DIRECT_FEED", requestMsg.serviceName());
                assertTrue("itemA".equals(requestMsg.name()) || "itemB".equals(requestMsg.name()) || "itemC".equals(requestMsg.name()));
                names.add(requestMsg.name());

                if ("itemB".equals(requestMsg.name())) itemBStreamId = requestMsg.streamId();
            }
            assertEquals(names.size(), 3);
            assertTrue(itemBStreamId != 0);

            assertEquals(4, consumerClient.queueSize());

            message = consumerClient.popMessage();
            assertTrue(message instanceof RefreshMsg);

            RefreshMsg refresh = (RefreshMsg)message;
            assertTrue(refresh.hasName());
            assertEquals(".AV.N", refresh.name());

            names.clear();
            for (int i = 0; i < 3; i++)
            {
                message = consumerClient.popMessage();
                assertTrue(message instanceof RefreshMsg);

                refresh = (RefreshMsg)message;
                assertTrue(refresh.hasName());
                assertTrue("itemA".equals(refresh.name()) || "itemB".equals(refresh.name()) || "itemC".equals(refresh.name()));
                names.add(refresh.name());
            }
            assertEquals(names.size(), 3);

            long itemBHandle = consumerClient.handleNameMap.get("itemB");
            consumer.submit(EmaFactory.createGenericMsg().name("itemB").complete(true), itemBHandle);

            Thread.sleep(1000);

            assertEquals(1, providerClient1.queueSize());

            message = providerClient1.popMessage();
            assertTrue(message instanceof GenericMsg);
            GenericMsg genericMsg = (GenericMsg)message;
            assertTrue("itemB".equals(genericMsg.name()));
            assertEquals(itemBStreamId, genericMsg.streamId());

            long provHandleA = providerClient1.retriveItemHandle("itemA");

            if (provHandleA != 0)
            {
                providerClient1.providerInstance().submit(EmaFactory.createGenericMsg().name("Prov_itemA").complete(true), provHandleA);
            }

            Thread.sleep(1000);

            assertEquals(1, consumerClient.queueSize());
            message = consumerClient.popMessage();
            assertTrue(message instanceof GenericMsg);

            assertNotNull(consumer);
        }
        catch (OmmException excep)
        {
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
    public void testRequestingSymbolListItem_SymbolListAndMarketPriceItemsRecovery_MPItemRequestTimesOutAndIsRecovered()
    {
        /*
            Consumer requests Symbol List item. API automatically requests the Market Price items from the Refresh message.
            Provider sends Refreshes only for two out of three Market Price items requested. The Consumer receives these Refreshes and
            closes the streams for all currently open items (except for item without Refresh since EMA still doesn't know
            anything about it). After some time the remaining item times out. The API tries to recover it by sending another request,
            Provider sends Refresh, Consumer application receives it.
         */
        TestUtilities.printTestHead("testRequestingSymbolListItem_SymbolListAndMarketPriceItemsRecovery_MPItemRequestTimesOutAndIsRecovered","");
        String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";
        OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);
        ProviderTestOptions providerTestOptions = new ProviderTestOptions();
        ProviderTestClient providerClient1 = new ProviderTestClient(providerTestOptions);
        providerTestOptions.itemGroupId = ByteBuffer.wrap("10".getBytes());

        providerTestOptions.sendItemRefreshMap = new HashMap<>();
        providerTestOptions.sendItemRefreshMap.put("itemB", false);

        // Provider_1 provides the DIRECT_FEED service name
        OmmProvider ommprovider = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_1"), providerClient1);

        assertNotNull(ommprovider);

        OmmConsumer consumer = null;
        ConsumerTestClient consumerClient = new ConsumerTestClient();

        try
        {
            consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_1_1"), consumerClient);

            ElementList payload = EmaFactory.createElementList();
            ElementEntry entry = EmaFactory.createElementEntry();

            ElementList eePayload = EmaFactory.createElementList();
            ElementEntry eeEntry = EmaFactory.createElementEntry();

            eeEntry.uintValue(":DataStreams", SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_STREAMS);
            eePayload.add(eeEntry);

            entry.elementList(":SymbolListBehaviors", eePayload);
            payload.add(entry);

            ReqMsg reqMsg = EmaFactory.createReqMsg().domainType(EmaRdm.MMT_SYMBOL_LIST).serviceName("DIRECT_FEED").name(".AV.N").payload(payload);

            Thread.sleep(1000);

            consumerClient.clearQueue();

            long itemHandle = consumer.registerClient(reqMsg, consumerClient);

            Thread.sleep(3000);

            assertEquals(4, providerClient1.queueSize());

            Msg message = providerClient1.popMessage();

            assertTrue(message instanceof ReqMsg);
            ReqMsg requestMsg = (ReqMsg)message;

            assertEquals("DIRECT_FEED", requestMsg.serviceName());
            assertEquals(".AV.N", requestMsg.name());

            HashSet<String> names = new HashSet<String>();
            for (int i = 0; i < 3; i++)
            {
                message = providerClient1.popMessage();
                requestMsg = (ReqMsg)message;
                assertEquals("DIRECT_FEED", requestMsg.serviceName());
                assertTrue("itemA".equals(requestMsg.name()) || "itemB".equals(requestMsg.name()) || "itemC".equals(requestMsg.name()));
                names.add(requestMsg.name());
            }
            assertEquals(names.size(), 3);

            assertEquals(3, consumerClient.queueSize());

            message = consumerClient.popMessage();
            assertTrue(message instanceof RefreshMsg);

            RefreshMsg refresh = (RefreshMsg)message;
            assertTrue(refresh.hasName());
            assertEquals(".AV.N", refresh.name());

            names.clear();
            for (int i = 0; i < 2; i++)
            {
                message = consumerClient.popMessage();
                assertTrue(message instanceof RefreshMsg);

                refresh = (RefreshMsg)message;
                assertTrue(refresh.hasName());
                assertTrue("itemA".equals(refresh.name()) || "itemB".equals(refresh.name()) || "itemC".equals(refresh.name()));
                names.add(refresh.name());
            }
            assertEquals(names.size(), 2);

            providerTestOptions.sendItemRefreshMap = null;

            long itemAHandle = consumerClient.handleNameMap.get("itemA");
            consumer.unregister(itemAHandle);

            long itemCHandle = consumerClient.handleNameMap.get("itemC");
            consumer.unregister(itemCHandle);

            consumer.unregister(itemHandle);

            Thread.sleep(8000);

            assertEquals(5, providerClient1.queueSize()); // Close msgs for .AV.N, itemA, itemC, Close for itemB since it has expired and ReqMsg for itemB since it was not closed by the application and we want to recover it
            int[] foundMsgs = new int[4];
            for (int i = 0; i < 5; i++)
            {
                message = providerClient1.popMessage();
                assertTrue(message instanceof ReqMsg);
                reqMsg = (ReqMsg)message;
                if (reqMsg.hasName())
                {
                    switch (reqMsg.name())
                    {
                        case ".AV.N": foundMsgs[0] += 1; break;
                        case "itemA": foundMsgs[1] += 1; break;
                        case "itemB": foundMsgs[2] += 1; break;
                        case "itemC": foundMsgs[3] += 1; break;
                        default: break;
                    }
                }
            }
            assertEquals(1, foundMsgs[0]);
            assertEquals(1, foundMsgs[1]);
            assertEquals(2, foundMsgs[2]);
            assertEquals(1, foundMsgs[3]);

            assertEquals(2, consumerClient.queueSize()); // Open/Suspect status for itemB since it timed out, Refresh for itemB.

            boolean statusFound = false;
            boolean refreshFound = false;

            for (int i = 0; i < 2; i++)
            {
                message = consumerClient.popMessage();
                assertTrue(message instanceof RefreshMsg || message instanceof StatusMsg);
                if (message instanceof RefreshMsg)
                {
                    refreshFound = true;
                }
                if (message instanceof StatusMsg)
                {
                    statusFound = true;
                }
            }

            assertTrue(refreshFound && statusFound);

            assertNotNull(consumer);
        }
        catch (OmmException excep)
        {
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
    public void testRequestingSymbolListItem_ProviderClosesMarketPriceItem_ConsumerShouldDoFine()
    {
        /*
            Consumer requests Symbol List item. API automatically requests the Market Price items from the Refresh message.
            Provider sends Refreshes for the three Market Price items requested automatically. The Consumer receives these Refreshes.
            The Provider sends Status CLOSED message for one of the Market Price items. Consumer gets Status CLOSED message,
            no error occurs.
         */
        TestUtilities.printTestHead("testRequestingSymbolListItem_SymbolListAndMarketPriceItemsRecovery_MPItemRequestTimesOutAndIsRecovered","");
        String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";
        OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);
        ProviderTestOptions providerTestOptions = new ProviderTestOptions();
        ProviderTestClient providerClient1 = new ProviderTestClient(providerTestOptions);
        providerTestOptions.itemGroupId = ByteBuffer.wrap("10".getBytes());

        // Provider_1 provides the DIRECT_FEED service name
        OmmProvider ommprovider = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_1"), providerClient1);

        assertNotNull(ommprovider);

        OmmConsumer consumer = null;
        ConsumerTestClient consumerClient = new ConsumerTestClient();

        try
        {
            consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_1"), consumerClient);

            ElementList payload = EmaFactory.createElementList();
            ElementEntry entry = EmaFactory.createElementEntry();

            ElementList eePayload = EmaFactory.createElementList();
            ElementEntry eeEntry = EmaFactory.createElementEntry();

            eeEntry.uintValue(":DataStreams", SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_STREAMS);
            eePayload.add(eeEntry);

            entry.elementList(":SymbolListBehaviors", eePayload);
            payload.add(entry);

            ReqMsg reqMsg = EmaFactory.createReqMsg().domainType(EmaRdm.MMT_SYMBOL_LIST).serviceName("DIRECT_FEED").name(".AV.N").payload(payload);

            Thread.sleep(1000);

            consumerClient.clearQueue();

            long itemHandle = consumer.registerClient(reqMsg, consumerClient);

            Thread.sleep(3000);

            assertEquals(4, providerClient1.queueSize());

            Msg message = providerClient1.popMessage();

            assertTrue(message instanceof ReqMsg);
            ReqMsg requestMsg = (ReqMsg)message;

            assertEquals("DIRECT_FEED", requestMsg.serviceName());
            assertEquals(".AV.N", requestMsg.name());

            HashSet<String> names = new HashSet<String>();
            for (int i = 0; i < 3; i++)
            {
                message = providerClient1.popMessage();
                requestMsg = (ReqMsg)message;
                assertEquals("DIRECT_FEED", requestMsg.serviceName());
                assertTrue("itemA".equals(requestMsg.name()) || "itemB".equals(requestMsg.name()) || "itemC".equals(requestMsg.name()));
                names.add(requestMsg.name());
            }
            assertEquals(names.size(), 3);

            assertEquals(4, consumerClient.queueSize());

            message = consumerClient.popMessage();
            assertTrue(message instanceof RefreshMsg);

            RefreshMsg refresh = (RefreshMsg)message;
            assertTrue(refresh.hasName());
            assertEquals(".AV.N", refresh.name());

            names.clear();
            int itemAStreamId = 0;
            for (int i = 0; i < 3; i++)
            {
                message = consumerClient.popMessage();
                assertTrue(message instanceof RefreshMsg);

                refresh = (RefreshMsg)message;
                assertTrue(refresh.hasName());
                assertTrue("itemA".equals(refresh.name()) || "itemB".equals(refresh.name()) || "itemC".equals(refresh.name()));
                names.add(refresh.name());
                if ("itemA".equals(refresh.name())) itemAStreamId = refresh.streamId();
            }
            assertEquals(names.size(), 3);

            long handle = providerClient1.retriveItemHandle("itemA");
            providerClient1.providerInstance().submit(EmaFactory.createStatusMsg().state(OmmState.StreamState.CLOSED, OmmState.DataState.SUSPECT), handle);

            Thread.sleep(1000);

            assertEquals(1, consumerClient.queueSize());

            message = consumerClient.popMessage();
            assertTrue(message instanceof StatusMsg);
            assertEquals(itemAStreamId, message.streamId());

            assertNotNull(consumer);
        }
        catch (OmmException excep)
        {
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
    public void testSymbolListRequest_UnregisterSymbolListBeforeMarketPriceRefreshesArrive()
    {
        /*
            Consumer requests Symbol List item. API automatically requests the Market Price items from the Refresh message.
            Provider sends Refresh message for Symbol List request, but desn't send Market Price Refreshes at once.
            Before getting Refreshes for Market Price items, Consumer unregisters Symbol List item.
            Then Provider sends Market Price Refreshes. Consumer should receive Refreshes for Market Price items.
         */
        TestUtilities.printTestHead("testSymbolListRequest_ReissueSymbolListAndMarketPriceRequests","");
        String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";
        OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);
        ProviderTestOptions providerTestOptions = new ProviderTestOptions();
        ProviderTestClient providerClient1 = new ProviderTestClient(providerTestOptions);
        providerTestOptions.itemGroupId = ByteBuffer.wrap("10".getBytes());

        // Provider_1 provides the DIRECT_FEED service name
        OmmProvider ommprovider = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_1"), providerClient1);

        assertNotNull(ommprovider);

        OmmConsumer consumer = null;
        ConsumerTestClient consumerClient = new ConsumerTestClient();

        try
        {
            consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_1"), consumerClient);

            ElementList payload = EmaFactory.createElementList();
            ElementEntry entry = EmaFactory.createElementEntry();

            ElementList eePayload = EmaFactory.createElementList();
            ElementEntry eeEntry = EmaFactory.createElementEntry();

            eeEntry.uintValue(":DataStreams", SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_STREAMS);
            eePayload.add(eeEntry);

            entry.elementList(":SymbolListBehaviors", eePayload);
            payload.add(entry);

            ReqMsg reqMsg = EmaFactory.createReqMsg().domainType(EmaRdm.MMT_SYMBOL_LIST).serviceName("DIRECT_FEED").name(".AV.N").payload(payload);

            Thread.sleep(1000);

            consumerClient.clearQueue();

            providerTestOptions.waitBeforeSendingItemRefresh = 5000;
            long itemHandle = consumer.registerClient(reqMsg, consumerClient);

            Thread.sleep(2000);
            Msg message;

            assertEquals(1, consumerClient.queueSize());

            message = consumerClient.popMessage();
            assertTrue(message instanceof RefreshMsg);

            RefreshMsg refresh = (RefreshMsg)message;
            assertTrue(refresh.hasName());
            assertEquals(".AV.N", refresh.name());

            consumer.unregister(itemHandle);

            Thread.sleep(7000);

            assertEquals(3, consumerClient.queueSize());
            HashSet<String> names = new HashSet<String>();
            for (int i = 0; i < 3; i++)
            {
                message = consumerClient.popMessage();
                assertTrue(message instanceof RefreshMsg);
                refresh = (RefreshMsg)message;
                assertTrue(refresh.hasName());
                assertTrue("itemA".equals(refresh.name()) || "itemB".equals(refresh.name()) || "itemC".equals(refresh.name()));
                names.add(refresh.name());
            }
            assertEquals(3, names.size());

            assertNotNull(consumer);
        }
        catch (OmmException excep)
        {
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
    public void testSymbolListRequest_ReissueSymbolListAndMarketPriceRequests()
    {
        /*
            Consumer requests Symbol List item. API automatically requests the Market Price items from the Refresh message.
            Provider sends Refreshes for the three Market Price items requested automatically. The Consumer receives these Refreshes.
            The Provider sends Status CLOSED message for one of the Market Price items. Consumer gets Status CLOSED message,
            no error occurs.
         */
        TestUtilities.printTestHead("testSymbolListRequest_ReissueSymbolListAndMarketPriceRequests","");
        String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";
        OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);
        ProviderTestOptions providerTestOptions = new ProviderTestOptions();
        ProviderTestClient providerClient1 = new ProviderTestClient(providerTestOptions);
        providerTestOptions.itemGroupId = ByteBuffer.wrap("10".getBytes());

        // Provider_1 provides the DIRECT_FEED service name
        OmmProvider ommprovider = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_1"), providerClient1);

        assertNotNull(ommprovider);

        OmmConsumer consumer = null;
        ConsumerTestClient consumerClient = new ConsumerTestClient();

        try
        {
            consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_1"), consumerClient);

            ElementList payload = EmaFactory.createElementList();
            ElementEntry entry = EmaFactory.createElementEntry();

            ElementList eePayload = EmaFactory.createElementList();
            ElementEntry eeEntry = EmaFactory.createElementEntry();

            eeEntry.uintValue(":DataStreams", SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_STREAMS);
            eePayload.add(eeEntry);

            entry.elementList(":SymbolListBehaviors", eePayload);
            payload.add(entry);

            ReqMsg reqMsg = EmaFactory.createReqMsg().domainType(EmaRdm.MMT_SYMBOL_LIST).serviceName("DIRECT_FEED").name(".AV.N").payload(payload);

            Thread.sleep(1000);

            consumerClient.clearQueue();

            long itemHandle = consumer.registerClient(reqMsg, consumerClient);

            Thread.sleep(3000);

            assertEquals(4, providerClient1.queueSize());

            Msg message = providerClient1.popMessage();

            assertTrue(message instanceof ReqMsg);
            ReqMsg requestMsg = (ReqMsg)message;

            assertEquals("DIRECT_FEED", requestMsg.serviceName());
            assertEquals(".AV.N", requestMsg.name());

            HashSet<String> names = new HashSet<String>();
            for (int i = 0; i < 3; i++)
            {
                message = providerClient1.popMessage();
                requestMsg = (ReqMsg)message;
                assertEquals("DIRECT_FEED", requestMsg.serviceName());
                assertTrue("itemA".equals(requestMsg.name()) || "itemB".equals(requestMsg.name()) || "itemC".equals(requestMsg.name()));
                names.add(requestMsg.name());
            }
            assertEquals(names.size(), 3);

            assertEquals(4, consumerClient.queueSize());

            message = consumerClient.popMessage();
            assertTrue(message instanceof RefreshMsg);

            RefreshMsg refresh = (RefreshMsg)message;
            assertTrue(refresh.hasName());
            assertEquals(".AV.N", refresh.name());

            names.clear();
            int itemAStreamId = 0;
            for (int i = 0; i < 3; i++)
            {
                message = consumerClient.popMessage();
                assertTrue(message instanceof RefreshMsg);

                refresh = (RefreshMsg)message;
                assertTrue(refresh.hasName());
                assertTrue("itemA".equals(refresh.name()) || "itemB".equals(refresh.name()) || "itemC".equals(refresh.name()));
                names.add(refresh.name());
                if ("itemA".equals(refresh.name())) itemAStreamId = refresh.streamId();
            }
            assertEquals(names.size(), 3);

            consumer.registerClient(reqMsg, consumerClient);

            Thread.sleep(2000);
            assertNotNull(consumer);
        }
        catch (OmmException excep)
        {
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

    // Modified tests from OmmConsumerTests that observe Enhanced Symbol List feature behavior for different MultiConnection/WSB/PH scenarios
    // ProviderDriven items requested on behalf of the user should be properly handled upon connection recovery/moving from one server to another

    @Test
    public void testRequestingSingleSymbolListItemWithRequestedConnectionDownWithDisableSessionEnhancedItemRecovery()
    {
        TestUtilities.printTestHead("testRequestingSingleSymbolListItemWithRequestedConnectionDownWithDisableSessionEnhancedItemRecovery","");

        String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";

        OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);

        ProviderTestOptions providerTestOptions = new ProviderTestOptions();

        ProviderTestClient providerClient1 = new ProviderTestClient(providerTestOptions);
        providerClient1.name = "Provider 1";

        providerTestOptions.itemGroupId = ByteBuffer.wrap("10".getBytes());

        // Provider_1 provides the DIRECT_FEED service name
        OmmProvider ommprovider = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_1"), providerClient1);

        assertNotNull(ommprovider);

        ProviderTestClient providerClient2 = new ProviderTestClient(providerTestOptions);
        providerClient2.name = "Provider 2";

        // Provider_1 provides the DIRECT_FEED service name
        OmmProvider ommprovider2 = EmaFactory.createOmmProvider(config.port("19004").providerName("Provider_1"), providerClient2);

        assertNotNull(ommprovider2);

        OmmConsumer consumer = null;
        ConsumerTestClient consumerClient = new ConsumerTestClient();

        try
        {
            consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_9"));

            Thread.sleep(1000);

            int channelQueueCount = 0;

            consumerClient.clearQueue();
            channelQueueCount = consumerClient.channelInfoSize();
            for (int i = 0; i < channelQueueCount; i++) consumerClient.popChannelInfo();

            providerClient1.clearQueue();

            ElementList payload = EmaFactory.createElementList();
            ElementEntry entry = EmaFactory.createElementEntry();

            ElementList eePayload = EmaFactory.createElementList();
            ElementEntry eeEntry = EmaFactory.createElementEntry();

            eeEntry.uintValue(":DataStreams", SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_STREAMS);
            eePayload.add(eeEntry);

            entry.elementList(":SymbolListBehaviors", eePayload);
            payload.add(entry);

            ReqMsg reqMsg = EmaFactory.createReqMsg().domainType(EmaRdm.MMT_SYMBOL_LIST).serviceName("DIRECT_FEED").name(".AV.N").payload(payload);

            Thread.sleep(1000);

            System.out.println(">>>> Consumer requests Symbol List item <<<<");
            long itemHandle = consumer.registerClient(reqMsg, consumerClient);

            Thread.sleep(1000);

            /* Checks provider that receives the item request. */
            Msg message;
            ReqMsg requestMsg = null;

            assertEquals(4, providerClient1.queueSize());
            for (int i = 0; i < 4; i++)
            {
                message = providerClient1.popMessage();
                assertTrue(message instanceof ReqMsg);
            }

            assertEquals(4, consumerClient.queueSize());
            for (int i = 0; i < 4; i++)
            {
                message = consumerClient.popMessage();
                assertTrue(message instanceof RefreshMsg);
            }

            Thread.sleep(1000);

            System.out.println(">>>> Provider 1 goes down <<<<");
            /* Force channel down on the first provider */
            ommprovider.uninitialize();

            Thread.sleep(10000);

            HashSet<Integer> statusStreamIds = new HashSet<>();
            HashSet<String> refreshNames = new HashSet<>();
            StatusMsg status;
            RefreshMsg refresh;

            assertEquals(8, consumerClient.queueSize());

            for (int i = 0; i < 8; i++)
            {
                message = consumerClient.popMessage();
                if (message instanceof StatusMsg)
                {
                    status = (StatusMsg)message;
                    statusStreamIds.add(status.streamId());
                    assertEquals(OmmState.DataState.SUSPECT, status.state().dataState());
                    if (status.streamId() < 0)
                    {
                        assertEquals(OmmState.StreamState.CLOSED, status.state().streamState());
                    }
                    else
                    {
                        assertEquals(OmmState.StreamState.OPEN, status.state().streamState());
                    }
                }
                else if (message instanceof RefreshMsg)
                {
                    refresh = (RefreshMsg)message;
                    assertTrue(refresh.hasName());
                    refreshNames.add(refresh.name());
                    assertTrue(".AV.N".equals(refresh.name())
                            || "itemA".equals(refresh.name()) || "itemB".equals(refresh.name()) || "itemC".equals(refresh.name()));
                }
            }

            assertEquals(4, statusStreamIds.size());
            assertEquals(4, refreshNames.size());

            assertEquals(4, providerClient2.queueSize());
            for (int i = 0; i < 4; i++)
            {
                message = providerClient2.popMessage();
                assertTrue(message instanceof ReqMsg);
                requestMsg = (ReqMsg)message;
                assertTrue(".AV.N".equals(requestMsg.name())
                        || "itemA".equals(requestMsg.name()) || "itemB".equals(requestMsg.name()) || "itemC".equals(requestMsg.name()));
            }

            System.out.println(">>>> Provider 1 goes up again <<<<");
            providerClient1.clearQueue();
            ommprovider = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_1"), providerClient1);

            Thread.sleep(3000);

            assertEquals(0, consumerClient.queueSize());
            assertEquals(0, providerClient1.queueSize());
        }
        catch(OmmException excep)
        {
            assertFalse(true);
        }
        catch (Exception e) {
            e.printStackTrace();
            assertFalse(true);
        }
        finally {
            System.out.println("Uninitializing...");

            consumer.uninitialize();
            ommprovider.uninitialize();
            ommprovider2.uninitialize();
        }
    }

    @Test
    public void testRequestingSingleSymbolListItemWithRequestedConnectionDownWithEnableSessionEnhancedItemRecovery()
    {
        TestUtilities.printTestHead("testRequestingSingleSymbolListItemWithRequestedConnectionDownWithEnableSessionEnhancedItemRecovery","");

        String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";

        OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);

        ProviderTestOptions providerTestOptions = new ProviderTestOptions();

        ProviderTestClient providerClient1 = new ProviderTestClient(providerTestOptions);

        providerTestOptions.itemGroupId = ByteBuffer.wrap("10".getBytes());

        // Provider_1 provides the DIRECT_FEED service name
        OmmProvider ommprovider = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_1"), providerClient1);
        providerClient1.name = "Provider 1";

        assertNotNull(ommprovider);

        ProviderTestClient providerClient2 = new ProviderTestClient(providerTestOptions);

        // Provider_1 provides the DIRECT_FEED service name
        OmmProvider ommprovider2 = EmaFactory.createOmmProvider(config.port("19004").providerName("Provider_1"), providerClient2);
        providerClient2.name = "Provider 2";

        assertNotNull(ommprovider2);

        OmmConsumer consumer = null;
        ConsumerTestClient consumerClient = new ConsumerTestClient();

        try
        {
            consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_12"));

            Thread.sleep(1000);

            int channelQueueCount = 0;

            consumerClient.clearQueue();
            channelQueueCount = consumerClient.channelInfoSize();
            for (int i = 0; i < channelQueueCount; i++) consumerClient.popChannelInfo();

            providerClient1.clearQueue();

            ElementList payload = EmaFactory.createElementList();
            ElementEntry entry = EmaFactory.createElementEntry();

            ElementList eePayload = EmaFactory.createElementList();
            ElementEntry eeEntry = EmaFactory.createElementEntry();

            eeEntry.uintValue(":DataStreams", SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_STREAMS);
            eePayload.add(eeEntry);

            entry.elementList(":SymbolListBehaviors", eePayload);
            payload.add(entry);

            ReqMsg reqMsg = EmaFactory.createReqMsg().domainType(EmaRdm.MMT_SYMBOL_LIST).serviceName("DIRECT_FEED").name(".AV.N").payload(payload);

            long itemHandle = consumer.registerClient(reqMsg, consumerClient);

            Thread.sleep(2000);

            /* Checks provider that receives the item request. */
            Msg message;
            ReqMsg requestMsg = null;

            assertEquals(4, providerClient1.queueSize());
            for (int i = 0; i < 4; i++)
            {
                message = providerClient1.popMessage();
                assertTrue(message instanceof ReqMsg);
            }

            assertEquals(4, consumerClient.queueSize());
            for (int i = 0; i < 4; i++)
            {
                message = consumerClient.popMessage();
                assertTrue(message instanceof RefreshMsg);
            }

            Thread.sleep(2000);

            System.out.println(">>>>>> Provider 1 goes down\n");
            ommprovider.uninitialize();

            Thread.sleep(10000);

            HashSet<Integer> statusStreamIds = new HashSet<>();
            HashSet<String> refreshNames = new HashSet<>();
            StatusMsg status;
            RefreshMsg refresh;

            assertEquals(8, consumerClient.queueSize());

            for (int i = 0; i < 8; i++)
            {
                message = consumerClient.popMessage();
                if (message instanceof StatusMsg)
                {
                    status = (StatusMsg)message;
                    statusStreamIds.add(status.streamId());
                    assertTrue(status.state().streamState() == OmmState.StreamState.CLOSED || status.state().streamState() == OmmState.StreamState.OPEN);
                    assertTrue(status.state().dataState() == OmmState.DataState.SUSPECT);
                }
                else if (message instanceof RefreshMsg)
                {
                    refresh = (RefreshMsg)message;
                    assertTrue(refresh.hasName());
                    refreshNames.add(refresh.name());
                    assertTrue(".AV.N".equals(refresh.name())
                            || "itemA".equals(refresh.name()) || "itemB".equals(refresh.name()) || "itemC".equals(refresh.name()));
                }
            }

            assertEquals(4, statusStreamIds.size());
            assertEquals(4, refreshNames.size());

            assertEquals(4, providerClient2.queueSize());
            for (int i = 0; i < 4; i++)
            {
                message = providerClient2.popMessage();
                assertTrue(message instanceof ReqMsg);
                requestMsg = (ReqMsg)message;
                assertTrue(".AV.N".equals(requestMsg.name())
                        || "itemA".equals(requestMsg.name()) || "itemB".equals(requestMsg.name()) || "itemC".equals(requestMsg.name()));
            }

            System.out.println(">>>> Provider 1 goes up again <<<<");
            providerClient1.clearQueue();
            ommprovider = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_1"), providerClient1);

            Thread.sleep(3000);

            assertEquals(0, consumerClient.queueSize());
            assertEquals(0, providerClient1.queueSize());
        }
        catch(OmmException excep)
        {
            assertFalse(true);
        }
        catch (Exception e) {
            e.printStackTrace();
            assertFalse(true);
        }
        finally {
            System.out.println("Uninitializing...");

            consumer.uninitialize();
            ommprovider.uninitialize();
            ommprovider2.uninitialize();
        }
    }

    @Test
    public void testSingleConnectWSBServiceBased_MovingFromOneGroupToAnother_SymbolListItem() /* Preferred host is not enabled for this case */
    {
        TestUtilities.printTestHead("testSingleConnectWSBServiceBased_MovingFromOneGroupToAnother_SymbolListItem","");

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
        providerClient.name = "Prov1";

        // Provider_7 provides the DIRECT_FEED and DIRECT_FEED1 service name for WSB channel(standby server)
        OmmProvider ommprovider2 = EmaFactory.createOmmProvider(config.port("19006").providerName("Provider_7"), providerClient2);
        assertNotNull(ommprovider2);
        providerClient2.name = "Prov2";

        /* WarmStandbyChannel_4_1 (preferred WSB channel) */
        // Provider_7 provides the DIRECT_FEED and DIRECT_FEED1 service name for WSB channel(starting server)
        OmmProvider ommprovider3 = EmaFactory.createOmmProvider(config.port("19007").providerName("Provider_8"), providerClient3);
        assertNotNull(ommprovider3);
        providerClient3.name = "Prov3";

        // Provider_7 provides the DIRECT_FEED and DIRECT_FEED1 service name for WSB channel(standby server)
        OmmProvider ommprovider4 = EmaFactory.createOmmProvider(config.port("19008").providerName("Provider_8"), providerClient4);
        assertNotNull(ommprovider4);
        providerClient4.name = "Prov4";

        OmmConsumer consumer = null;
        ConsumerTestOptions consumerOption = new ConsumerTestOptions();

        consumerOption.getChannelInformation = true;
        consumerOption.getSessionChannelInfo = false;
        ConsumerTestClient consumerClient = new ConsumerTestClient(consumerOption);

        try
        {
            ElementList payload = EmaFactory.createElementList();
            ElementEntry entry = EmaFactory.createElementEntry();

            ElementList eePayload = EmaFactory.createElementList();
            ElementEntry eeEntry = EmaFactory.createElementEntry();

            eeEntry.uintValue(":DataStreams", SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_STREAMS);
            eePayload.add(eeEntry);

            entry.elementList(":SymbolListBehaviors", eePayload);
            payload.add(entry);

            ReqMsg reqMsg = EmaFactory.createReqMsg().domainType(EmaRdm.MMT_SYMBOL_LIST).serviceName("DIRECT_FEED").name(".AV.N").payload(payload);

            //ReqMsg reqMsg = EmaFactory.createReqMsg().domainType(EmaRdm.MMT_MARKET_PRICE).serviceName("DIRECT_FEED").name(".AV.N");

            consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_24"));

            Thread.sleep(2500);

            int channelQueueCount = 0;

            consumerClient.clearQueue();
            channelQueueCount = consumerClient.channelInfoSize();
            for (int i = 0; i < channelQueueCount; i++) consumerClient.popChannelInfo();

            providerClient.clearQueue();
            providerClient2.clearQueue();

            long itemHandle = consumer.registerClient(reqMsg, consumerClient);
            Thread.sleep(2500);

            /* Receives one symbol list request and 3 Market Price item requests */
            //assertEquals(4, providerClient.queueSize());

            /*Msg message;
            boolean slFound = false;
            for (int i = 0; i < 4; i++)
            {
                message = providerClient.popMessage();
                assertTrue(message instanceof ReqMsg);
                if (message.domainType() == DomainTypes.SYMBOL_LIST) slFound = true;
            }
            assertTrue(slFound);

            slFound = false;
            assertEquals(4, providerClient2.queueSize());
            for (int i = 0; i < 4; i++)
            {
                message = providerClient2.popMessage();
                assertTrue(message instanceof ReqMsg);
                if (message.domainType() == DomainTypes.SYMBOL_LIST) slFound = true;
            }
            assertTrue(slFound);

            assertEquals(4, consumerClient.queueSize());
            for (int i = 0; i < 4; i++)
            {
                message = consumerClient.popMessage();
                // Checks the market price item refresh
                RefreshMsg refreshMsg = (RefreshMsg)message;
                assertTrue(refreshMsg.hasName());
                if (refreshMsg.domainType() == DomainTypes.SYMBOL_LIST)
                {
                    assertEquals(".AV.N", refreshMsg.name());
                }
                else if (refreshMsg.domainType() == DomainTypes.MARKET_PRICE)
                {
                    assertTrue("itemA".equals(refreshMsg.name()) || "itemB".equals(refreshMsg.name()) || "itemC".equals(refreshMsg.name()));
                }

                ChannelInformation channelInfo = consumerClient.popChannelInfo();
                assertEquals("Channel_6", channelInfo.channelName());
                assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
            } */

            System.out.println("Closing the providers from WarmStandbyChannel_3_1");
            /* Kill Provider1 and Provider2 to switch over to the WarmStandbyChannel_4_1 */

            ommprovider2.uninitialize();
            Thread.sleep(1500); // Wait 1 second to switch over to another server.

            StatusMsg status;
            RefreshMsg refresh;
            HashSet<String> refreshNames = new HashSet<>();

            /*assertEquals(8, consumerClient.queueSize());
            for (int i = 0; i < 8; i++)
            {
                message = consumerClient.popMessage();
                if (message instanceof StatusMsg)
                {
                    status = (StatusMsg)message;
                    assertEquals(OmmState.StreamState.OPEN, status.state().streamState());
                    assertEquals(OmmState.DataState.SUSPECT, status.state().dataState());

                    ChannelInformation channelInfo = consumerClient.popChannelInfo();
                    assertEquals("Channel_6", channelInfo.channelName());
                }
                else if (message instanceof RefreshMsg)
                {
                    refresh = (RefreshMsg)message;
                    refreshNames.add(refresh.name());

                    ChannelInformation channelInfo = consumerClient.popChannelInfo();
                    assertEquals("Channel_3", channelInfo.channelName());
                    assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
                }
            }
            assertEquals(4, refreshNames.size());*/

            ommprovider1.uninitialize();

            /* Waits for 3 seconds to move to  WarmStandbyChannel_4_1 */
            Thread.sleep(8000);

            /*slFound = false;
            refreshNames.clear();
            for (int i = 0; i < 8; i++)
            {
                message = consumerClient.popMessage();
                if (message instanceof StatusMsg)
                {
                    status = (StatusMsg)message;
                    assertEquals(OmmState.StreamState.OPEN, status.state().streamState());
                    assertEquals(OmmState.DataState.SUSPECT, status.state().dataState());

                    ChannelInformation channelInfo = consumerClient.popChannelInfo();
                    assertEquals("Channel_3", channelInfo.channelName());
                }
                else if (message instanceof RefreshMsg)
                {
                    refresh = (RefreshMsg)message;
                    refreshNames.add(refresh.name());

                    if (refresh.domainType() == DomainTypes.SYMBOL_LIST) slFound = true;

                    ChannelInformation channelInfo = consumerClient.popChannelInfo();
                    assertEquals("Channel_7", channelInfo.channelName());
                    assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
                }
            }
            assertEquals(4, refreshNames.size());
            assertTrue(slFound);*/
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
    public void testSingleConnectionChangingPreferredWSBGroupByIOCTLAndThenCallFallbackMethod_SymbolListItem()
    {
        TestUtilities.printTestHead("testSingleConnectionChangingPreferredWSBGroupByIOCTLAndThenCallFallbackMethod_SymbolListItem","");

        String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";

        OmmConsumer consumer = null;
        ConsumerTestOptions consumerOption = new ConsumerTestOptions();
        ProviderTestOptions providerTestOptions = new ProviderTestOptions();
        providerTestOptions.supportStandby = true;
        providerTestOptions.sendRefreshAttrib = true;
        providerTestOptions.itemGroupId = ByteBuffer.wrap("10".getBytes());

        ProviderTestClient providerClient_7 = new ProviderTestClient(providerTestOptions);
        ProviderTestClient providerClient_8 = new ProviderTestClient(providerTestOptions);
        ProviderTestClient providerClient_9 = new ProviderTestClient(providerTestOptions);
        ProviderTestClient providerClient_10 = new ProviderTestClient(providerTestOptions);

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
        OmmProvider ommprovider_7 = EmaFactory.createOmmProvider(config.port("19007").providerName("Provider_1"), providerClient_7);
        assertNotNull(ommprovider_7);
        providerClient_7.name = "Prov7";

        OmmProvider ommprovider_8 = EmaFactory.createOmmProvider(config.port("19008").providerName("Provider_1"), providerClient_8);
        assertNotNull(ommprovider_8);
        providerClient_8.name = "Prov8";

        //WSB-G2 (WarmStandbyChannel_5)
        OmmProvider ommprovider_9 = EmaFactory.createOmmProvider(config.port("19009").providerName("Provider_1"), providerClient_9);
        assertNotNull(ommprovider_9);
        providerClient_9.name = "Prov9";

        OmmProvider ommprovider_10 = EmaFactory.createOmmProvider(config.port("19010").providerName("Provider_1"), providerClient_10);
        assertNotNull(ommprovider_10);
        providerClient_10.name = "Prov10";

        try
        {
            ConsumerTestOptions options = new ConsumerTestOptions();
            options.getChannelInformation = true;

            ElementList payload = EmaFactory.createElementList();
            ElementEntry entry = EmaFactory.createElementEntry();

            ElementList eePayload = EmaFactory.createElementList();
            ElementEntry eeEntry = EmaFactory.createElementEntry();

            eeEntry.uintValue(":DataStreams", SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_STREAMS);
            eePayload.add(eeEntry);

            entry.elementList(":SymbolListBehaviors", eePayload);
            payload.add(entry);

            ReqMsg reqMsg = EmaFactory.createReqMsg().domainType(EmaRdm.MMT_SYMBOL_LIST).serviceName("DIRECT_FEED").name(".AV.N").payload(payload);

            consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_27"));

            Thread.sleep(2500);

            int channelQueueCount = 0;

            consumerClient.clearQueue();
            channelQueueCount = consumerClient.channelInfoSize();
            for (int i = 0; i < channelQueueCount; i++) consumerClient.popChannelInfo();

            providerClient_7.clearQueue();
            providerClient_8.clearQueue();
            providerClient_9.clearQueue();
            providerClient_10.clearQueue();

            String serviceName = "DIRECT_FEED";

            System.out.println(" >>> Registering Symbol List request <<< ");
            long itemHandle = consumer.registerClient(reqMsg, consumerClient);

            Thread.sleep(2500);

            assertEquals(4, providerClient_7.queueSize());
            assertEquals(1, providerClient_8.queueSize());


            Msg message;
            boolean slFound = false;
            for (int i = 0; i < 4; i++)
            {
                message = providerClient_7.popMessage();
                assertTrue(message instanceof ReqMsg);
                if (message.domainType() == DomainTypes.SYMBOL_LIST) slFound = true;
            }
            assertTrue(slFound);

            slFound = false;

            message = providerClient_8.popMessage();
            assertTrue(message.domainType() == DomainTypes.SYMBOL_LIST);

            assertEquals(4, consumerClient.queueSize());
            for (int i = 0; i < 4; i++)
            {
                message = consumerClient.popMessage();
                /* Checks the market price item refresh */
                RefreshMsg refreshMsg = (RefreshMsg)message;
                assertTrue(refreshMsg.hasName());
                if (refreshMsg.domainType() == DomainTypes.SYMBOL_LIST)
                {
                    assertEquals(".AV.N", refreshMsg.name());
                }
                else if (refreshMsg.domainType() == DomainTypes.MARKET_PRICE)
                {
                    assertTrue("itemA".equals(refreshMsg.name()) || "itemB".equals(refreshMsg.name()) || "itemC".equals(refreshMsg.name()));
                }

                ChannelInformation channelInfo = consumerClient.popChannelInfo();
                assertEquals("Channel_7", channelInfo.channelName());
                assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
            }

            // Enable the PH fallback process with the modifyIOCtl() method.
            PreferredHostOptions phOptions = EmaFactory.createPreferredHostOptions();
            phOptions.setPreferredHostEnabled(true);
            phOptions.setWsbChannelName("WarmStandbyChannel_5");

            System.out.println("\n >>> Modify PH options via IOCtl <<< ");
            consumer.modifyIOCtl(IOCtlCode.FALLBACK_PREFERRED_HOST_OPTIONS, phOptions);

            Thread.sleep(1000);

            System.out.println("\n >>> Calls fallbackPreferredHost() to fallback to WarmStandbyChannel_5 <<< ");
            consumer.fallbackPreferredHost();

            Thread.sleep(2000);

            StatusMsg status;
            RefreshMsg refresh;
            HashSet<String> refreshNames = new HashSet<>();

            assertEquals(8, consumerClient.queueSize());
            for (int i = 0; i < 8; i++)
            {
                message = consumerClient.popMessage();
                if (message instanceof StatusMsg)
                {
                    status = (StatusMsg)message;
                    assertTrue(OmmState.StreamState.CLOSED == status.state().streamState() || OmmState.StreamState.OPEN == status.state().streamState());
                    assertEquals(OmmState.DataState.SUSPECT, status.state().dataState());

                    ChannelInformation channelInfo = consumerClient.popChannelInfo();
                    assertEquals("Channel_7", channelInfo.channelName());
                }
                else if (message instanceof RefreshMsg)
                {
                    refresh = (RefreshMsg)message;
                    refreshNames.add(refresh.name());

                    ChannelInformation channelInfo = consumerClient.popChannelInfo();
                    assertEquals("Channel_9", channelInfo.channelName());
                    assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
                }
            }
            assertEquals(4, refreshNames.size());

            System.out.println("\n >>> Bring down S6 and S5 <<< ");

            ommprovider_10.uninitialize();
            ommprovider_9.uninitialize();

            Thread.sleep(15000);

            refreshNames.clear();
            assertEquals(8, consumerClient.queueSize());
        }
        catch(Exception excep)
        {
            System.out.println(excep);
        }
        finally
        {
            if (consumer != null) consumer.uninitialize();

            ommprovider_7.uninitialize();
            ommprovider_8.uninitialize();
        }
    }

    @Test
    public void testMultipleConnectionsWSBServiceBased_FallbackWithinGroupWhenPerServiceNameSet_ProviderSendsServiceDown_SymbolListItem()
    {
		/*
			Consumer has 2 WarmStandbyChannels WarmStandbyChannel_9 and WarmStandbyChannel_10, WarmStandbyChannel_10 is preferred.
			Also, fallbackWithinWSBGroup is set to true. At the beginning WarmStandbyChannel_10 servers are all down,
			WarmStandbyChannel_9 servers are up.
			Consumer eventually connects to WarmStandbyChannel_9 (Provider_3 (active), Provider_6 (standby)) servers.
			Consumer registers for TRI.N item, service is DIRECT_FEED1.
			Provider_3 gets Generic message with Active mode for DIRECT_FEED1 and Generic message with Standby mode for DIRECT_FEED,
			and a request message for the item TRI.N registered with DIRECT_FEED1 name. Sends full Refresh for the item.
			Provider_6 gets Generic message with Standby mode for DIRECT_FEED1 and Generic message with Active mode for DIRECT_FEED,
			and a request message for the item TRI.N registered with DIRECT_FEED1 name. Sends Refresh with empty payload for the item.
			Consumer gets the Refresh for TRI.N.
			Servers from WarmStandbyChannel_10 go online.
			Provider_3 (active server) sends Directory Update with Service DOWN for DIRECT_FEED1.
			Provider_6 gets Generic message with Active mode for DIRECT_FEED1
			Consumer triggers preferred host callback using preferredHostFallback method. Wait for 4 seconds.
			Provider_3 gets Generic message with Standby mode for DIRECT_FEED1
			Provider_6 gets nothing.
			Provider_3 sends Directory Update message with Service UP state for DIRECT_FEED1
			Provider_3 gets Item Request message for TRI.N with service DIRECT_FEED1.
			Consumer triggers preferred host fallback using preferredHostFallback method again. Waits for 4 seconds.
			Provider_3 and Provider_6 get no messages.
			Consumer registers for item ABC.N with service name DIRECT_FEED, registers for item DEF.N with service name DIRECT_FEED1.
			Both Provider_3 and Provider_6 get requests for these two items with the respective service names.
			Provider_3 sends Refreshes with empty payloads for them, while Provider_6 sends full Refreshes for these items.
		*/
        String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";

        OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);

        ProviderTestOptions providerTestOptions = new ProviderTestOptions();
        providerTestOptions.supportStandby = true;
        providerTestOptions.sendRefreshAttrib = true;
        providerTestOptions.itemGroupId = ByteBuffer.wrap("10".getBytes());

        ProviderTestClient providerClient_3 = new ProviderTestClient(providerTestOptions);
        providerClient_3.name = "Provider_3";
        ProviderTestClient providerClient_6 = new ProviderTestClient(providerTestOptions);
        providerClient_6.name = "Provider_6";
        ProviderTestClient providerClient_7 = new ProviderTestClient(providerTestOptions);
        providerClient_7.name = "Provider_7";
        ProviderTestClient providerClient_8 = new ProviderTestClient(providerTestOptions);
        providerClient_8.name = "Provider_8";
        ProviderTestClient providerClient_9 = new ProviderTestClient(providerTestOptions);
        providerClient_9.name = "Provider_9";

        /* WarmStandbyChannel_9*/
        // Provider_9 provides the DIRECT_FEED and DIRECT_FEED1 service name for WSB channel(starting server)
        OmmProvider ommprovider_3 = EmaFactory.createOmmProvider(config.port("19003").providerName("Provider_9"), providerClient_3);
        assertNotNull(ommprovider_3);

        // Provider_9 provides the DIRECT_FEED and DIRECT_FEED1 service name for WSB channel(standby server)
        OmmProvider ommprovider_6 = EmaFactory.createOmmProvider(config.port("19006").providerName("Provider_9"), providerClient_6);
        assertNotNull(ommprovider_6);

        /* WarmStandbyChannel_10 (preferred WSB channel, all connections are down at the beginning) */
        OmmProvider ommprovider_7 = null;

        // Provider_9 provides the DIRECT_FEED and DIRECT_FEED1 service name for WSB channel(starting server)
        OmmProvider ommprovider_8 = null;

        // Provider_9 provides the DIRECT_FEED and DIRECT_FEED1 service name for WSB channel(standby server)
        OmmProvider ommprovider_9 = null;

        OmmConsumer consumer = null;
        ConsumerTestOptions consumerOption = new ConsumerTestOptions();

        consumerOption.getChannelInformation = true;
        consumerOption.getSessionChannelInfo = false;
        ConsumerTestClient consumerClient = new ConsumerTestClient(consumerOption);

        try
        {
            consumer  = EmaFactory.createOmmConsumer(EmaFactory
                    .createOmmConsumerConfig(emaConfigFileLocation)
                    .consumerName("Consumer_28"), consumerClient);

            Thread.sleep(2000);

            int channelQueueCount = 0;

            consumerClient.clearQueue();
            while (consumerClient.popChannelInfo() != null) {}

            providerClient_3.clearQueue();
            providerClient_6.clearQueue();

            String serviceName = "DIRECT_FEED";
            String serviceName2 = "DIRECT_FEED1";
            long DIRECT_FEED_SERVICE_ID = 1; // This is for the DIRECT_FEED service
            long DIRECT_FEED1_SERVICE_ID = 2; // This is for the DIRECT_FEED1 service

            ElementList payload = EmaFactory.createElementList();
            ElementEntry entry = EmaFactory.createElementEntry();

            ElementList eePayload = EmaFactory.createElementList();
            ElementEntry eeEntry = EmaFactory.createElementEntry();

            eeEntry.uintValue(":DataStreams", SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_STREAMS);
            eePayload.add(eeEntry);

            entry.elementList(":SymbolListBehaviors", eePayload);
            payload.add(entry);

            ReqMsg reqMsg = EmaFactory.createReqMsg().domainType(EmaRdm.MMT_SYMBOL_LIST).serviceName("DIRECT_FEED").name(".AV.N").payload(payload);

            long itemHandle = consumer.registerClient(reqMsg, consumerClient);

            Thread.sleep(2000);

            Msg message;
            boolean slFound = false;

            assertEquals(1, providerClient_3.queueSize());
            message = providerClient_3.popMessage();
            assertTrue(message.domainType() == DomainTypes.SYMBOL_LIST);

            slFound = false;
            assertEquals(4, providerClient_6.queueSize());
            for (int i = 0; i < 4; i++)
            {
                message = providerClient_6.popMessage();
                assertTrue(message instanceof ReqMsg);
                if (message.domainType() == DomainTypes.SYMBOL_LIST) slFound = true;
            }
            assertTrue(slFound);

            assertEquals(4, consumerClient.queueSize());
            for (int i = 0; i < 4; i++)
            {
                message = consumerClient.popMessage();
                ChannelInformation channelInfo = consumerClient.popChannelInfo();
                /* Checks the market price item refresh */
                RefreshMsg refreshMsg = (RefreshMsg)message;
                assertTrue(refreshMsg.hasName());
                if (refreshMsg.domainType() == DomainTypes.SYMBOL_LIST)
                {
                    assertEquals(".AV.N", refreshMsg.name());
                }
                else if (refreshMsg.domainType() == DomainTypes.MARKET_PRICE)
                {
                    assertTrue("itemA".equals(refreshMsg.name()) || "itemB".equals(refreshMsg.name()) || "itemC".equals(refreshMsg.name()));
                }
                assertEquals("Channel_6", channelInfo.channelName());
                assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
            }

            Thread.sleep(1500);

            // WarmStandbyChannel_10 servers go up...
            ommprovider_7 = EmaFactory.createOmmProvider(config.port("19007").providerName("Provider_9"), providerClient_6);
            assertNotNull(ommprovider_7);

            // Provider_9 provides the DIRECT_FEED and DIRECT_FEED1 service name for WSB channel(starting server)
            ommprovider_8 = EmaFactory.createOmmProvider(config.port("19008").providerName("Provider_9"), providerClient_8);
            assertNotNull(ommprovider_8);

            // Provider_9 provides the DIRECT_FEED and DIRECT_FEED1 service name for WSB channel(standby server)
            ommprovider_9 = EmaFactory.createOmmProvider(config.port("19009").providerName("Provider_9"), providerClient_9);
            assertNotNull(ommprovider_9);

            Thread.sleep(3000);

            System.out.println("\n >>>>>>>>>>>>>>>>>>>>>>> Provider_6 sends ServiceDown... \n");
            // Send service down for the starting server of the first connection.
            ElementList serviceState = EmaFactory.createElementList();
            serviceState.add( EmaFactory.createElementEntry().uintValue( EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_DOWN ));
            serviceState.add( EmaFactory.createElementEntry().uintValue( EmaRdm.ENAME_ACCEPTING_REQS, 0 ));
            serviceState.add( EmaFactory.createElementEntry().state( EmaRdm.ENAME_STATUS, OmmState.StreamState.CLOSED_RECOVER, OmmState.DataState.SUSPECT, OmmState.StatusCode.NONE, ""));
            FilterList filterListEnc = EmaFactory.createFilterList();
            filterListEnc.add( EmaFactory.createFilterEntry().elementList( EmaRdm.SERVICE_STATE_ID, FilterEntry.FilterAction.SET, serviceState ) );
            Map mapEnc2 = EmaFactory.createMap();
            mapEnc2.add( EmaFactory.createMapEntry().keyUInt( DIRECT_FEED_SERVICE_ID, MapEntry.MapAction.UPDATE, filterListEnc ));
            UpdateMsg updateMsg = EmaFactory.createUpdateMsg();
            ommprovider_6.submit( updateMsg.domainType( EmaRdm.MMT_DIRECTORY ).
                    filter( EmaRdm.SERVICE_STATE_FILTER ).
                    payload( mapEnc2 ), 0);	// use 0 item handle to fan-out to all subscribers
            providerClient_6.processServiceDown(DIRECT_FEED_SERVICE_ID);

            // Waits for 2.5 seconds for Provider_6 to get Generic message
            Thread.sleep(2500);

            assertEquals(7, consumerClient.queueSize());
            for (int i = 0; i < 7; i++)
            {
                message = consumerClient.popMessage();
                ChannelInformation channelInfo = consumerClient.popChannelInfo();
                // Checks the market price item refresh
                if (message instanceof RefreshMsg)
                {
                    RefreshMsg refreshMsg = (RefreshMsg)message;
                    assertTrue(refreshMsg.hasName());
                    if (refreshMsg.domainType() == DomainTypes.SYMBOL_LIST)
                    {
                        assertEquals(".AV.N", refreshMsg.name());
                    }
                    else if (refreshMsg.domainType() == DomainTypes.MARKET_PRICE)
                    {
                        assertTrue("itemA".equals(refreshMsg.name()) || "itemB".equals(refreshMsg.name()) || "itemC".equals(refreshMsg.name()));
                    }
                    //assertEquals("Channel_3", channelInfo.channelName());
                    assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
                }

            }

            System.out.println("\n >>>>>>>>>>>>>>>>>>> Consumer triggers PreferredHost Fallback... \n");
            // Trigger fallback to preferred host
            consumer.fallbackPreferredHost();

            Thread.sleep(4000);

            assertEquals(2, consumerClient.queueSize()); // The fallback events
            for (int i = 0; i < 2; i++)
            {
                message = consumerClient.popMessage();
                assertTrue(message instanceof StatusMsg);
                ChannelInformation channelInfo = consumerClient.popChannelInfo();
            }

            System.out.println("\n >>>>>>>>>>>>>>>>>>>>>>>>>>>>>> Provider_6 sends ServiceUp for DIRECT_FEED... \n");
            // Send service down for the starting server of the first connection.
            serviceState.clear();
            serviceState.add( EmaFactory.createElementEntry().uintValue( EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_UP ));
            serviceState.add( EmaFactory.createElementEntry().uintValue( EmaRdm.ENAME_ACCEPTING_REQS, 1 ));
            serviceState.add( EmaFactory.createElementEntry().state( EmaRdm.ENAME_STATUS, OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, ""));
            filterListEnc.clear();
            filterListEnc.add( EmaFactory.createFilterEntry().elementList( EmaRdm.SERVICE_STATE_ID, FilterEntry.FilterAction.SET, serviceState ) );
            mapEnc2.clear();
            mapEnc2.add( EmaFactory.createMapEntry().keyUInt( DIRECT_FEED_SERVICE_ID, MapEntry.MapAction.UPDATE, filterListEnc ));
            updateMsg.clear();
            ommprovider_6.submit( updateMsg.domainType( EmaRdm.MMT_DIRECTORY ).
                    filter( EmaRdm.SERVICE_STATE_FILTER ).
                    payload( mapEnc2 ), 0);	// use 0 item handle to fan-out to all subscribers

            Thread.sleep(4000);

            assertEquals(0, consumerClient.queueSize());

            System.out.println("\n >>>>>>>>>>>>>>>>>>>>>>>>>>  Consumer triggers PreferredHost Fallback again... \n");
            // Trigger fallback to preferred host
            consumer.fallbackPreferredHost(); // nothing happens here at all

            Thread.sleep(4000);

            assertEquals(6, consumerClient.queueSize()); // The fallback events + new Refreshes
            for (int i = 0; i < 2; i++)
            {
                message = consumerClient.popMessage();
                assertTrue(message instanceof StatusMsg);
                ChannelInformation channelInfo = consumerClient.popChannelInfo();
            }

            for (int i = 0; i < 4; i++)
            {
                message = consumerClient.popMessage();
                /* Checks the market price item refresh */
                RefreshMsg refreshMsg = (RefreshMsg)message;
                assertTrue(refreshMsg.hasName());
                if (refreshMsg.domainType() == DomainTypes.SYMBOL_LIST)
                {
                    assertEquals(".AV.N", refreshMsg.name());
                }
                else if (refreshMsg.domainType() == DomainTypes.MARKET_PRICE)
                {
                    assertTrue("itemA".equals(refreshMsg.name()) || "itemB".equals(refreshMsg.name()) || "itemC".equals(refreshMsg.name()));
                }

                ChannelInformation channelInfo = consumerClient.popChannelInfo();
                assertEquals("Channel_6", channelInfo.channelName());
                assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
            }

            System.out.println("Shutting down...");
        }
        catch (Exception e) {

            e.printStackTrace();
            assertFalse(true);
        }
        finally
        {
            assertNotNull(consumer);

            System.out.println("Uninitializing...");
            consumer.uninitialize();
            ommprovider_3.uninitialize();
            ommprovider_6.uninitialize();
            if (ommprovider_7 != null) ommprovider_7.uninitialize();
            if (ommprovider_8 != null) ommprovider_8.uninitialize();
            if (ommprovider_9 != null) ommprovider_9.uninitialize();
        }

    }

    @Test
    public void testMultiConnectionSymbolListFallBackToPreferredWSBGroupWhenNonPreferredIsDownForLoginBasedWSB()
    {
        String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";

        OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);

        ProviderTestOptions providerTestOptions = new ProviderTestOptions();
        providerTestOptions.supportStandby = true;
        providerTestOptions.sendRefreshAttrib = true;
        providerTestOptions.itemGroupId = ByteBuffer.wrap("10".getBytes());

        ProviderTestClient providerClient_1 = new ProviderTestClient(new ProviderTestOptions());
        ProviderTestClient providerClient_3 = new ProviderTestClient(providerTestOptions);
        ProviderTestClient providerClient_6 = new ProviderTestClient(providerTestOptions);
        ProviderTestClient providerClient_7 = new ProviderTestClient(providerTestOptions);
        ProviderTestClient providerClient_8 = new ProviderTestClient(providerTestOptions);

        // Connection_1_1
        // Channel_1 (preferred Channel name)
        // Provider_9 provides the DIRECT_FEED(RT/TickByTick) and DIRECT_FEED1(RT/JustInTimeConflated) service names.
        OmmProvider ommprovider_1 = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_9"), providerClient_1);
        assertNotNull(ommprovider_1);
        providerClient_1.name = "Prov_1";

        // Connection_25
        // WarmStandbyChannel_1 (Preferred WSB Channel name). This is login based WSB channel.
        // Start only the standby server
        // Provider_9 provides the DIRECT_FEED and DIRECT_FEED1 service name for WSB channel(starting server)
        OmmProvider ommprovider_6 = EmaFactory.createOmmProvider(config.port("19006").providerName("Provider_9"), providerClient_6);
        assertNotNull(ommprovider_6);
        providerClient_6.name = "Prov_6";

        /* WarmStandbyChannel_2 (Non-preferred WSB Channel name). This is login based WSB channel. */
        // Provider_9 provides the DIRECT_FEED and DIRECT_FEED1 service name for WSB channel(starting server)
        OmmProvider ommprovider_7 = EmaFactory.createOmmProvider(config.port("19007").providerName("Provider_9"), providerClient_7);
        assertNotNull(ommprovider_7);
        providerClient_7.name = "Prov_7";

        // Provider_9 provides the DIRECT_FEED and DIRECT_FEED1 service name for WSB channel(standby server)
        OmmProvider ommprovider_8 = EmaFactory.createOmmProvider(config.port("19008").providerName("Provider_9"), providerClient_8);
        assertNotNull(ommprovider_8);
        providerClient_8.name = "Prov_8";

        OmmConsumer consumer = null;

        ConsumerTestOptions consumerOption = new ConsumerTestOptions();

        consumerOption.getChannelInformation = true;
        consumerOption.getSessionChannelInfo = false;
        ConsumerTestClient consumerClient = new ConsumerTestClient(consumerOption);

        OmmProvider ommprovider_3 = null;

        try
        {
            ConsumerTestOptions options = new ConsumerTestOptions();
            options.getChannelInformation = true;

            consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_31"), consumerClient);

            Thread.sleep(3000);

            consumerClient.clearQueue();
            int channelCount = consumerClient.channelInfoSize();
            for (int i = 0; i < channelCount; i++) consumerClient.popChannelInfo();

            providerClient_1.clearQueue();
            providerClient_3.clearQueue();
            providerClient_6.clearQueue();
            providerClient_7.clearQueue();
            providerClient_8.clearQueue();

            String serviceName = "DIRECT_FEED";
            String itemName = ".AV.N";

            ElementList payload = EmaFactory.createElementList();
            ElementEntry entry = EmaFactory.createElementEntry();

            ElementList eePayload = EmaFactory.createElementList();
            ElementEntry eeEntry = EmaFactory.createElementEntry();

            eeEntry.uintValue(":DataStreams", SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_STREAMS);
            eePayload.add(eeEntry);

            entry.elementList(":SymbolListBehaviors", eePayload);
            payload.add(entry);

            ReqMsg reqMsg = EmaFactory.createReqMsg().domainType(EmaRdm.MMT_SYMBOL_LIST).serviceName(serviceName).name(itemName).payload(payload);

            System.out.println(" >>>>>>> Requesting symbol list item <<<<<<<");
            long itemHandle = consumer.registerClient(reqMsg, consumerClient);

            /* Waits for refresh message from Connection_1_1 */
            Thread.sleep(1500);

            Msg message;
            ReqMsg requestMsg;

            assertEquals(4, providerClient_1.queueSize());
            HashSet<String> names = new HashSet<>();

            for (int i = 0; i < 4; i++)
            {
                message = providerClient_1.popMessage();
                assertTrue(message instanceof ReqMsg);
                requestMsg = (ReqMsg)message;
                if (".AV.N".equals(requestMsg.name()) || "itemA".equals(requestMsg.name()) || "itemB".equals(requestMsg.name()) || "itemC".equals(requestMsg.name()))
                    names.add(requestMsg.name());
            }
            assertEquals(4, names.size());

            assertEquals(4, consumerClient.queueSize());
            ChannelInformation chInfo;
            RefreshMsg refresh;

            names.clear();

            for (int i = 0; i < 4; i++)
            {
                message = consumerClient.popMessage();
                assertTrue(message instanceof RefreshMsg);
                assertTrue(message.hasName());
                refresh = (RefreshMsg)message;
                if (".AV.N".equals(refresh.name()) || "itemA".equals(refresh.name()) || "itemB".equals(refresh.name()) || "itemC".equals(refresh.name()))
                    names.add(refresh.name());
                chInfo = consumerClient.popChannelInfo();
                assertEquals("Channel_1", chInfo.channelName());
            }
            assertEquals(4, names.size());

            System.out.println(" >>>>>>> Uninitializing OmmProvider_1 19001<<<<<<<< ");
            /* Kills Provider1 to close Channel_1 */
            ommprovider_1.uninitialize();

            Thread.sleep(5000);

            int msgCount = consumerClient.queueSize();
            names.clear();
            StatusMsg status;
            int closedItemsCount = 0;

            for (int i = 0; i < msgCount; i++)
            {
                message = consumerClient.popMessage();
                chInfo = consumerClient.popChannelInfo();
                if (message instanceof RefreshMsg)
                {
                    assertTrue(message.hasName());
                    refresh = (RefreshMsg)message;
                    if (".AV.N".equals(refresh.name()) || "itemA".equals(refresh.name()) || "itemB".equals(refresh.name()) || "itemC".equals(refresh.name()))
                        names.add(refresh.name());
                    assertEquals("Channel_7", chInfo.channelName());
                }
                else if (message instanceof StatusMsg)
                {
                    status = (StatusMsg)message;
                    assertTrue("Channel_1".equals(chInfo.channelName()) || "Channel_7".equals(chInfo.channelName()));
                    if (status.state().statusText().contains("This individual item of the Symbol List will be recovered"))
                        closedItemsCount++;
                }
            }
            assertEquals(4, names.size());
            assertEquals(3, closedItemsCount);

            System.out.println(" >>>>>>>>> Uninitializing OmmProvider_7 19007 <<<<<<<<< ");
            /* Closes the starting server of WarmStandbyChannel_2 */
            ommprovider_7.uninitialize();

            Thread.sleep(3000);

            names.clear();
            closedItemsCount = 0;
            msgCount = consumerClient.queueSize();

            for (int i = 0; i < msgCount; i++)
            {
                message = consumerClient.popMessage();
                chInfo = consumerClient.popChannelInfo();
                if (message instanceof RefreshMsg)
                {
                    assertTrue(message.hasName());
                    refresh = (RefreshMsg)message;
                    if (".AV.N".equals(refresh.name()) || "itemA".equals(refresh.name()) || "itemB".equals(refresh.name()) || "itemC".equals(refresh.name()))
                        names.add(refresh.name());
                    assertEquals("Channel_8", chInfo.channelName());
                }
                else if (message instanceof StatusMsg)
                {
                    status = (StatusMsg)message;
                    assertTrue("Channel_7".equals(chInfo.channelName()) || "Channel_8".equals(chInfo.channelName()) || "Channel_1".equals(chInfo.channelName()));
                    if (status.state().statusText().contains("Individual item from Symbol List closed due to server change."))
                        closedItemsCount++;
                }
            }
            assertEquals(4, names.size());
            assertEquals(3, closedItemsCount);

            System.out.println(" >>>>>>>>>> OmmProvider 19003 goes up <<<<<<<<<<");
            // WarmStandbyChannel_1 (Preferred WSB Channel name).
            // Start the starting server to fallback to this preferred group.
            // Provider_9 provides the DIRECT_FEED and DIRECT_FEED1 service name for WSB channel(starting server)
            ommprovider_3 = EmaFactory.createOmmProvider(config.port("19003").providerName("Provider_9"), providerClient_3);
            assertNotNull(ommprovider_3);

            System.out.println("The starting server of the preferred WSB is started..");
            Thread.sleep(10000);

            names.clear();
            closedItemsCount = 0;
            msgCount = consumerClient.queueSize();
            for (int i = 0; i < msgCount; i++)
            {
                message = consumerClient.popMessage();
                chInfo = consumerClient.popChannelInfo();
                if (message instanceof RefreshMsg)
                {
                    assertTrue(message.hasName());
                    refresh = (RefreshMsg)message;
                    if (".AV.N".equals(refresh.name()) || "itemA".equals(refresh.name()) || "itemB".equals(refresh.name()) || "itemC".equals(refresh.name()))
                        names.add(refresh.name());
                    assertEquals("Channel_3", chInfo.channelName());
                }
                else if (message instanceof StatusMsg)
                {
                    status = (StatusMsg)message;
                    assertTrue("Channel_3".equals(chInfo.channelName()) || "Channel_7".equals(chInfo.channelName()) || "Channel_8".equals(chInfo.channelName()));
                    if (status.state().statusText().contains("Individual item from Symbol List closed"))
                        closedItemsCount++;
                }
            }
            assertEquals(4, names.size());
            assertEquals(3, closedItemsCount);

            consumer.unregister(itemHandle);

            Thread.sleep(4000);

        }
        catch (Exception ex)
        {
            ex.printStackTrace();
            assertFalse(true);
        }
        finally
        {
            assertNotNull(consumer);

            if (consumer != null) consumer.uninitialize();
            if (ommprovider_1 != null) ommprovider_1.uninitialize();
            if (ommprovider_3 != null) ommprovider_3.uninitialize();
            if (ommprovider_6 != null) ommprovider_6.uninitialize();
            if (ommprovider_8 != null) ommprovider_8.uninitialize();
        }
    }

    @Test
    public void testRequestingSingleSLItemWithRequestedConnectionDownWithDisableSessionEnhancedItemRecoveryAndChannelIsClosed()
    {
        TestUtilities.printTestHead("testRequestingSingleSLItemWithRequestedConnectionDownWithDisableSessionEnhancedItemRecoveryAndChannelIsClosed","");

        String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";

        OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);

        ProviderTestOptions providerTestOptions = new ProviderTestOptions();

        ProviderTestClient providerClient1 = new ProviderTestClient(providerTestOptions);

        providerTestOptions.itemGroupId = ByteBuffer.wrap("10".getBytes());

        // Provider_1 provides the DIRECT_FEED service name
        OmmProvider ommprovider_1 = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_1"), providerClient1);

        assertNotNull(ommprovider_1);

        ProviderTestClient providerClient2 = new ProviderTestClient(providerTestOptions);

        // Provider_1 provides the DIRECT_FEED service name
        OmmProvider ommprovider_4 = EmaFactory.createOmmProvider(config.port("19004").providerName("Provider_1"), providerClient2);

        assertNotNull(ommprovider_4);

        OmmConsumer consumer = null;
        ConsumerTestClient consumerClient = new ConsumerTestClient();

        try
        {
            consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_9"));

            Thread.sleep(2000);

            consumerClient.clearQueue();
            int count = consumerClient.channelInfoSize();
            for (int i = 0; i < count; i++) consumerClient.popChannelInfo();

            String serviceName = "DIRECT_FEED";
            String itemName = ".AV.N";

            ElementList payload = EmaFactory.createElementList();
            ElementEntry entry = EmaFactory.createElementEntry();

            ElementList eePayload = EmaFactory.createElementList();
            ElementEntry eeEntry = EmaFactory.createElementEntry();

            eeEntry.uintValue(":DataStreams", SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_STREAMS);
            eePayload.add(eeEntry);

            entry.elementList(":SymbolListBehaviors", eePayload);
            payload.add(entry);

            ReqMsg reqMsg = EmaFactory.createReqMsg().domainType(EmaRdm.MMT_SYMBOL_LIST).serviceName(serviceName).name(itemName).payload(payload);

            System.out.println(" >>>>>>> Requesting symbol list item <<<<<<<");
            long itemHandle = consumer.registerClient(reqMsg, consumerClient);

            Thread.sleep(2000);

            HashSet<String> itemNames = new HashSet<>();
            HashSet<Integer> itemStreamIds = new HashSet<>();
            count = consumerClient.queueSize();
            for (int i = 0; i < count; i++)
            {
                Msg message = consumerClient.popMessage();
                ChannelInformation chInfo = consumerClient.popChannelInfo();
                if (message instanceof RefreshMsg) {
                    RefreshMsg refresh = (RefreshMsg) message;
                    if (refresh.hasName()
                            && (refresh.name().equals(".AV.N")
                            || refresh.name().equals("itemA")
                            || refresh.name().equals("itemB")
                            || refresh.name().equals("itemC"))) {
                        if (refresh.name().equals(".AV.N")) assertEquals(5, refresh.streamId());
                        itemNames.add(refresh.name());
                        itemStreamIds.add(refresh.streamId());
                    }
                }
            }
            assertEquals(4, itemNames.size());


            Thread.sleep(1000);

            System.out.println(" >>>> Killing provider_1");
            /* Force channel down on the first provider of Connection_1*/
            ReqMsg recvReqMsg;
            ommprovider_1.uninitialize();

            /* Wait until the subscribed channel is closed */
            Thread.sleep(8000);

            int msgCount = consumerClient.queueSize();
            HashSet<String> names = new HashSet<>();
            StatusMsg status;
            int closedItemsCount = 0;

            for (int i = 0; i < msgCount; i++)
            {
                Msg message = consumerClient.popMessage();
                ChannelInformation chInfo = consumerClient.popChannelInfo();
                if (message instanceof RefreshMsg)
                {
                    assertTrue(message.hasName());
                    RefreshMsg refresh = (RefreshMsg)message;
                    if (".AV.N".equals(refresh.name()) || "itemA".equals(refresh.name()) || "itemB".equals(refresh.name()) || "itemC".equals(refresh.name()))
                        names.add(refresh.name());
                }
                else if (message instanceof StatusMsg)
                {
                    status = (StatusMsg)message;
                    if (status.state().statusText().contains("Individual item from Symbol List closed due to server change."))
                        closedItemsCount++;
                }
            }
            assertEquals(4, names.size());
            assertEquals(3, closedItemsCount);

            providerClient2.clearQueue();

            consumer.unregister(itemHandle);

            Thread.sleep(3000);

            assertTrue(providerClient2.queueSize() > 0); // provider_4 received close message
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
            ommprovider_4.uninitialize();
        }
    }

    @Test
    public void testRequestingSingleSLItemWSBConnectionRecovery()
    {
        /*
            Consumer has one WSB connection and requests symbol list item. API automatically requests Market Price items
            from Symbol List Refresh and the Consumer gets the respective Market Price refreshes.
            The active server goes down. StandBy server should send refreshes for SymbolList item and for
            Market Price items (Market Price items are recovered once - not twice). Active server goes up, StandBy server goes down.
            Active server sends refreshes for SymbolList item and for Market Price items (Market Price items are recovered once - not twice)
         */
        TestUtilities.printTestHead("testRequestingSingleSLItemWSBConnection","");

        String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";

        OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);

        ProviderTestOptions providerTestOptions = new ProviderTestOptions();

        providerTestOptions.itemGroupId = ByteBuffer.wrap("10".getBytes());
        providerTestOptions.supportStandby = true;
        providerTestOptions.supportStandbyMode = 0;
        providerTestOptions.sendRefreshAttrib = true;

        ProviderTestClient providerClient3 = new ProviderTestClient(providerTestOptions);

        // Provider_1 provides the DIRECT_FEED service name
        OmmProvider ommprovider_3 = EmaFactory.createOmmProvider(config.port("19003").providerName("Provider_9"), providerClient3);
        providerClient3.name = "Provider 3";

        assertNotNull(ommprovider_3);

        providerTestOptions.supportStandbyMode = 1;
        ProviderTestClient providerClient6 = new ProviderTestClient(providerTestOptions);

        // Provider_1 provides the DIRECT_FEED service name
        OmmProvider ommprovider_6 = EmaFactory.createOmmProvider(config.port("19006").providerName("Provider_9"), providerClient6);
        providerClient6.name = "Provider 6";

        assertNotNull(ommprovider_6);

        OmmConsumer consumer = null;
        ConsumerTestClient consumerClient = new ConsumerTestClient();

        try
        {
            consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_50"));

            String serviceName = "DIRECT_FEED";
            String itemName = ".AV.N";

            ElementList payload = EmaFactory.createElementList();
            ElementEntry entry = EmaFactory.createElementEntry();

            ElementList eePayload = EmaFactory.createElementList();
            ElementEntry eeEntry = EmaFactory.createElementEntry();

            eeEntry.uintValue(":DataStreams", SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_STREAMS);
            eePayload.add(eeEntry);

            entry.elementList(":SymbolListBehaviors", eePayload);
            payload.add(entry);

            ReqMsg reqMsg = EmaFactory.createReqMsg().domainType(EmaRdm.MMT_SYMBOL_LIST).serviceName(serviceName).name(itemName).payload(payload);

            Thread.sleep(1500);

            consumerClient.clearQueue();

            System.out.println(" >>>>>>> Requesting symbol list item <<<<<<<");
            long itemHandle = consumer.registerClient(reqMsg, consumerClient);

            Thread.sleep(1000);

            Msg message;

            assertEquals(4, consumerClient.queueSize());
            for (int i = 0; i < 4; i++)
            {
                message = consumerClient.popMessage();
                /* Checks the market price item refresh */
                RefreshMsg refreshMsg = (RefreshMsg)message;
                assertTrue(refreshMsg.hasName());
                if (refreshMsg.domainType() == DomainTypes.SYMBOL_LIST)
                {
                    assertEquals(".AV.N", refreshMsg.name());
                }
                else if (refreshMsg.domainType() == DomainTypes.MARKET_PRICE)
                {
                    assertTrue("itemA".equals(refreshMsg.name()) || "itemB".equals(refreshMsg.name()) || "itemC".equals(refreshMsg.name()));
                }
            }

            System.out.println(" >>>> Killing provider_3");
            /* Force channel down on the first provider */
            ommprovider_3.uninitialize();

            /* Wait until the subscribed channel is closed */
            Thread.sleep(8000);

            int refreshCount = 0;
            int statusCount = 0;

            int count = consumerClient.queueSize();
            for (int i = 0; i < count; i++)
            {
                message = consumerClient.popMessage();
                if (message instanceof RefreshMsg)
                {
                    // Checks the market price item refresh
                    RefreshMsg refreshMsg = (RefreshMsg)message;
                    assertTrue(refreshMsg.hasName());
                    if (refreshMsg.domainType() == DomainTypes.SYMBOL_LIST)
                    {
                        assertEquals(".AV.N", refreshMsg.name());
                    }
                    else if (refreshMsg.domainType() == DomainTypes.MARKET_PRICE)
                    {
                        assertTrue("itemA".equals(refreshMsg.name()) || "itemB".equals(refreshMsg.name()) || "itemC".equals(refreshMsg.name()));
                    }
                    refreshCount++;
                }
                else if (message instanceof StatusMsg)
                {
                    // Checks the market price item refresh
                    StatusMsg statusMsg = (StatusMsg)message;
                    assertTrue(statusMsg.domainType() == DomainTypes.MARKET_PRICE || statusMsg.domainType() == DomainTypes.SYMBOL_LIST);

                    statusCount++;
                }
            }

            assertEquals(4, statusCount);
            assertEquals(4, refreshCount);

            providerClient3 = new ProviderTestClient(providerTestOptions);

            System.out.println(" >>>> Provider_3 goes up again");
            // Provider_1 provides the DIRECT_FEED service name
            ommprovider_3 = EmaFactory.createOmmProvider(config.port("19003").providerName("Provider_9"), providerClient3);
            providerClient3.name = "Provider 3";

            Thread.sleep(2500);

            System.out.println(" >>>> Killing provider_6");
            ommprovider_6.uninitialize();

            Thread.sleep(8000);

            refreshCount = 0;
            statusCount = 0;

            count = consumerClient.queueSize();
            for (int i = 0; i < count; i++)
            {
                message = consumerClient.popMessage();
                if (message instanceof RefreshMsg)
                {
                    // Checks the market price item refresh
                    RefreshMsg refreshMsg = (RefreshMsg)message;
                    assertTrue(refreshMsg.hasName());
                    if (refreshMsg.domainType() == DomainTypes.SYMBOL_LIST)
                    {
                        assertEquals(".AV.N", refreshMsg.name());
                    }
                    else if (refreshMsg.domainType() == DomainTypes.MARKET_PRICE)
                    {
                        assertTrue("itemA".equals(refreshMsg.name()) || "itemB".equals(refreshMsg.name()) || "itemC".equals(refreshMsg.name()));
                    }
                    refreshCount++;
                }
                else if (message instanceof StatusMsg)
                {
                    // Checks the market price item refresh
                    StatusMsg statusMsg = (StatusMsg)message;
                    assertTrue(statusMsg.domainType() == DomainTypes.MARKET_PRICE || statusMsg.domainType() == DomainTypes.SYMBOL_LIST);

                    statusCount++;
                }
            }
            assertEquals(4, statusCount);
            assertEquals(4, refreshCount);

            providerClient3.clearQueue();

            consumer.unregister(itemHandle);

            Thread.sleep(2000);

            assertTrue(providerClient3.queueSize() > 0);
        }
        catch (OmmException excep)
        {
            System.out.println(">>>> Exception: \n" + excep.getMessage());
            assertFalse(true);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        finally {
            System.out.println("Uninitializing...");

            consumer.uninitialize();
            if (ommprovider_3 != null) ommprovider_3.uninitialize();
            if (ommprovider_6 != null) ommprovider_6.uninitialize();
        }
    }

    @Test
    public void testMultiConnectionSymbolListItemRecoveryBetweenConnectionListAndWSBGroups()
    {
        String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";

        OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);

        ProviderTestOptions providerTestOptions = new ProviderTestOptions();
        providerTestOptions.supportStandby = true;
        providerTestOptions.sendRefreshAttrib = true;
        providerTestOptions.itemGroupId = ByteBuffer.wrap("10".getBytes());

        ProviderTestClient providerClient = new ProviderTestClient(new ProviderTestOptions());
        ProviderTestClient providerClient_3 = new ProviderTestClient(providerTestOptions);
        providerClient_3.name = "Provider 3";
        ProviderTestClient providerClient_6 = new ProviderTestClient(providerTestOptions);
        providerClient_6.name = "Provider 6";
        ProviderTestClient providerClient_7 = new ProviderTestClient(providerTestOptions);
        providerClient_7.name = "Provider 7";
        ProviderTestClient providerClient_8 = new ProviderTestClient(providerTestOptions);
        providerClient_8.name = "Provider 8";

        OmmProvider ommprovider_3 = null;

        OmmProvider ommprovider_6 = null;

        OmmProvider ommprovider_7 = null;

        OmmProvider ommprovider_8 = null;

        OmmProvider ommprovider_1 = null;

        OmmProvider ommprovider_2 = null;

        ProviderTestClient providerClient_1 = new ProviderTestClient(providerTestOptions);
        providerClient_1.name = "Provider 1";
        ProviderTestClient providerClient_2 = new ProviderTestClient(providerTestOptions);
        providerClient_2.name = "Provider 2";

        System.out.println(">>>>>> Starting 19001 and 19002");

        ommprovider_1 = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_9"), providerClient_1);
        assertNotNull(ommprovider_1);

        ommprovider_2 = EmaFactory.createOmmProvider(config.port("19002").providerName("Provider_9"), providerClient_2);
        assertNotNull(ommprovider_2);

        OmmConsumer consumer = null;
        ConsumerTestOptions consumerOption = new ConsumerTestOptions();

        consumerOption.getChannelInformation = true;
        consumerOption.getSessionChannelInfo = false;
        ConsumerTestClient consumerClient = new ConsumerTestClient(consumerOption);

        try
        {
            ConsumerTestOptions options = new ConsumerTestOptions();
            options.getChannelInformation = true;

            consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_54"), consumerClient);

            Thread.sleep(10000);

            consumerClient.clearQueue();
            int count = consumerClient.channelInfoSize();
            for (int i = 0; i < count; i++) consumerClient.popChannelInfo();

            String serviceName = "DIRECT_FEED";
            String itemName = ".AV.N";

            ElementList payload = EmaFactory.createElementList();
            ElementEntry entry = EmaFactory.createElementEntry();

            ElementList eePayload = EmaFactory.createElementList();
            ElementEntry eeEntry = EmaFactory.createElementEntry();

            eeEntry.uintValue(":DataStreams", SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_STREAMS);
            eePayload.add(eeEntry);

            entry.elementList(":SymbolListBehaviors", eePayload);
            payload.add(entry);

            ReqMsg reqMsg = EmaFactory.createReqMsg().domainType(EmaRdm.MMT_SYMBOL_LIST).serviceName(serviceName).name(itemName).payload(payload);

            System.out.println(">>>>>>> Requesting symbol list...\n");
            long itemHandle = consumer.registerClient(reqMsg, consumerClient);

            Thread.sleep(4000);

            Msg message;
            ChannelInformation chInfo;

            HashSet<String> itemNames = new HashSet<>();
            HashSet<Integer> itemStreamIds = new HashSet<>();
            count = consumerClient.queueSize();
            for (int i = 0; i < count; i++)
            {
                message = consumerClient.popMessage();
                chInfo = consumerClient.popChannelInfo();
                if (chInfo.channelName().equals("Channel_1"))
                {
                    if (message instanceof RefreshMsg)
                    {
                        RefreshMsg refresh = (RefreshMsg)message;
                        if (refresh.hasName()
                                && (refresh.name().equals(".AV.N")
                                || refresh.name().equals("itemA")
                                || refresh.name().equals("itemB")
                                || refresh.name().equals("itemC")))
                        {
                            if (refresh.name().equals(".AV.N")) assertEquals(5, refresh.streamId());
                            itemNames.add(refresh.name());
                            itemStreamIds.add(refresh.streamId());
                        }
                    }
                }
            }
            assertEquals(4, itemNames.size());
            assertTrue(itemStreamIds.contains(-1) && itemStreamIds.contains(-2) && itemStreamIds.contains(-3));

            System.out.println(" >>>>>>> All WSB group providers go up");

            ommprovider_3 = EmaFactory.createOmmProvider(config.port("19003").providerName("Provider_9"), providerClient_3);
            assertNotNull(ommprovider_3);

            ommprovider_6 = EmaFactory.createOmmProvider(config.port("19006").providerName("Provider_9"), providerClient_6);
            assertNotNull(ommprovider_6);

            ommprovider_7 = EmaFactory.createOmmProvider(config.port("19007").providerName("Provider_9"), providerClient_7);
            assertNotNull(ommprovider_7);

            ommprovider_8 = EmaFactory.createOmmProvider(config.port("19008").providerName("Provider_9"), providerClient_8);
            assertNotNull(ommprovider_8);

            Thread.sleep(5000);

            System.out.println(" >>>>>>> Channel 1 goes down");
            ommprovider_1.uninitialize();

            Thread.sleep(9000);

            checkStatusAndRefreshMsgs(consumerClient, itemNames, itemStreamIds, "Channel_1", "Channel_3", "Channel_3");
            itemNames.clear();
            itemStreamIds.clear();

            System.out.println(" >>>>>>> Channel 2 goes down");
            ommprovider_2.uninitialize();

            Thread.sleep(9000);

            System.out.println(" >>>>>>>>>> Killing 19003 ");
            /* Kills Provider1 to close Channel_1 */
            ommprovider_3.uninitialize();
            Thread.sleep(9000);

            checkStatusAndRefreshMsgs(consumerClient, itemNames, itemStreamIds, "Channel_3", "Channel_6", null);
            assertTrue(itemStreamIds.contains(-7) && itemStreamIds.contains(-8) && itemStreamIds.contains(-9));
            itemNames.clear();
            itemStreamIds.clear();

            System.out.println(">>>>>>>>>>> Killing 19006");
            ommprovider_6.uninitialize();
            Thread.sleep(20000);

            checkStatusAndRefreshMsgs(consumerClient, itemNames, itemStreamIds, "Channel_6", "Channel_7", null);
            itemNames.clear();
            itemStreamIds.clear();

            System.out.println(">>>>>>>>>>>>> Killing 19007");
            ommprovider_7.uninitialize();

            Thread.sleep(8000);

            checkStatusAndRefreshMsgs(consumerClient, itemNames, itemStreamIds, "Channel_7", "Channel_8", null);
            itemNames.clear();
            itemStreamIds.clear();

            System.out.println(">>>>>>>>>>>>> Killing 19008");
            ommprovider_8.uninitialize();

            Thread.sleep(2000);

            System.out.println(">>>>>> Starting 19001 and 19002");

            ommprovider_1 = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_9"), providerClient_1);
            assertNotNull(ommprovider_1);

            ommprovider_2 = EmaFactory.createOmmProvider(config.port("19002").providerName("Provider_9"), providerClient_2);
            assertNotNull(ommprovider_2);

            Thread.sleep(15000);

            checkStatusAndRefreshMsgs(consumerClient, itemNames, itemStreamIds, "Channel_8", "Channel_2", null);
        }
        catch (Exception ex)
        {
            ex.printStackTrace();
            assertFalse(true);
        }
        finally
        {
            System.out.println(">>>>> Uninitializing...");
            assertNotNull(consumer);

            if (consumer != null) consumer.uninitialize();
            if (ommprovider_1 != null) ommprovider_1.uninitialize();
            if (ommprovider_2 != null) ommprovider_2.uninitialize();
            if (ommprovider_3 != null) ommprovider_3.uninitialize();
            if (ommprovider_6 != null) ommprovider_6.uninitialize();
            if (ommprovider_7 != null) ommprovider_7.uninitialize();
            if (ommprovider_8 != null) ommprovider_8.uninitialize();
        }
    }

    @Test
    public void testMultiConnectionSymbolListRequestsToDifferentServers()
    {
        String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";

        OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);

        ProviderTestOptions providerTestOptions = new ProviderTestOptions();
        providerTestOptions.supportStandby = true;
        providerTestOptions.sendRefreshAttrib = true;
        providerTestOptions.itemGroupId = ByteBuffer.wrap("10".getBytes());

        ProviderTestClient providerClient_1 = new ProviderTestClient(new ProviderTestOptions());
        ProviderTestClient providerClient_4 = new ProviderTestClient(providerTestOptions);
        ProviderTestClient providerClient_5 = new ProviderTestClient(providerTestOptions);

        // Connection_1_1
        // Channel_1 (preferred Channel name)
        OmmProvider ommprovider_1 = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_1"), providerClient_1);
        assertNotNull(ommprovider_1);

        // Connection_2
        // Channel_4, Channel_5
        OmmProvider ommprovider_4 = EmaFactory.createOmmProvider(config.port("19004").providerName("Provider_3"), providerClient_4);
        assertNotNull(ommprovider_4);

        OmmProvider ommprovider_5 = EmaFactory.createOmmProvider(config.port("19005").providerName("Provider_3"), providerClient_5);
        assertNotNull(ommprovider_5);


        OmmConsumer consumer = null;
        ConsumerTestOptions consumerOption = new ConsumerTestOptions();

        consumerOption.getChannelInformation = true;
        consumerOption.getSessionChannelInfo = false;
        ConsumerTestClient consumerClient = new ConsumerTestClient(consumerOption);

        try
        {
            ConsumerTestOptions options = new ConsumerTestOptions();
            options.getChannelInformation = true;

            consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_55"), consumerClient);

            Thread.sleep(2000);

            consumerClient.clearQueue();
            int count = consumerClient.channelInfoSize();
            for (int i = 0; i < count; i++) consumerClient.popChannelInfo();

            String serviceName = "DIRECT_FEED";
            String serviceName2 = "DIRECT_FEED_2";
            String itemName = ".AV.N";

            ElementList payload = EmaFactory.createElementList();
            ElementEntry entry = EmaFactory.createElementEntry();

            ElementList eePayload = EmaFactory.createElementList();
            ElementEntry eeEntry = EmaFactory.createElementEntry();

            eeEntry.uintValue(":DataStreams", SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_STREAMS);
            eePayload.add(eeEntry);

            entry.elementList(":SymbolListBehaviors", eePayload);
            payload.add(entry);

            ReqMsg reqMsg = EmaFactory.createReqMsg().domainType(EmaRdm.MMT_SYMBOL_LIST).serviceName(serviceName).name(itemName).payload(payload);
            ReqMsg reqMsg2 = EmaFactory.createReqMsg().domainType(EmaRdm.MMT_SYMBOL_LIST).serviceName(serviceName2).name(itemName).payload(payload);

            System.out.println(">>>>>>> Requesting items...\n");
            long itemHandle = consumer.registerClient(reqMsg, consumerClient);

            Thread.sleep(2500);

            Msg message;
            ChannelInformation chInfo;

            HashSet<String> itemNames = new HashSet<>();
            HashSet<Integer> itemStreamIds = new HashSet<>();
            count = consumerClient.queueSize();
            for (int i = 0; i < count; i++)
            {
                message = consumerClient.popMessage();
                chInfo = consumerClient.popChannelInfo();
                if (chInfo.channelName().equals("Channel_1"))
                {
                    if (message instanceof RefreshMsg)
                    {
                        RefreshMsg refresh = (RefreshMsg)message;
                        if (refresh.hasName()
                                && (refresh.name().equals(".AV.N")
                                || refresh.name().equals("itemA")
                                || refresh.name().equals("itemB")
                                || refresh.name().equals("itemC")))
                        {
                            if (refresh.name().equals(".AV.N")) assertEquals(5, refresh.streamId());
                            itemNames.add(refresh.name());
                            itemStreamIds.add(refresh.streamId());
                        }
                    }
                }
            }
            assertEquals(4, itemNames.size());
            assertTrue(itemStreamIds.contains(-1) && itemStreamIds.contains(-2) && itemStreamIds.contains(-3));

            itemNames.clear();
            itemStreamIds.clear();

            long itemHandle2 = consumer.registerClient(reqMsg2, consumerClient);

            Thread.sleep(2500);

            count = consumerClient.queueSize();
            for (int i = 0; i < count; i++)
            {
                message = consumerClient.popMessage();
                chInfo = consumerClient.popChannelInfo();
                if (chInfo.channelName().equals("Channel_4"))
                {
                    if (message instanceof RefreshMsg)
                    {
                        RefreshMsg refresh = (RefreshMsg)message;
                        if (refresh.hasName()
                                && (refresh.name().equals(".AV.N")
                                || refresh.name().equals("itemA")
                                || refresh.name().equals("itemB")
                                || refresh.name().equals("itemC")))
                        {
                            if (refresh.name().equals(".AV.N")) assertEquals(6, refresh.streamId());
                            itemNames.add(refresh.name());
                            itemStreamIds.add(refresh.streamId());
                        }
                    }
                }
            }
            assertEquals(4, itemNames.size());
            assertTrue(itemStreamIds.contains(-4) && itemStreamIds.contains(-5) && itemStreamIds.contains(-6));

            providerClient_1.clearQueue();
            providerClient_4.clearQueue();

            consumer.unregister(itemHandle);

            Thread.sleep(1500);

            assertTrue(providerClient_1.queueSize() > 0);

            consumer.unregister(itemHandle2);

            Thread.sleep(1500);

            assertTrue(providerClient_4.queueSize() > 0);
        }
        catch (Exception ex)
        {
            ex.printStackTrace();
            assertFalse(true);
        }
        finally
        {
            System.out.println(">>>>> Uninitializing...");
            assertNotNull(consumer);

            if (consumer != null) consumer.uninitialize();
            if (ommprovider_1 != null) ommprovider_1.uninitialize();
            if (ommprovider_4 != null) ommprovider_4.uninitialize();
            if (ommprovider_5 != null) ommprovider_5.uninitialize();
        }
    }

    @Test
    public void testWSBServiceBasedSymbolListRequestsToDifferentServers()
    {
        String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";

        OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);

        ProviderTestOptions providerTestOptions = new ProviderTestOptions();
        providerTestOptions.supportStandby = true;
        providerTestOptions.sendRefreshAttrib = true;
        providerTestOptions.itemGroupId = ByteBuffer.wrap("10".getBytes());

        ProviderTestClient providerClient_3 = new ProviderTestClient(providerTestOptions);
        ProviderTestClient providerClient_6 = new ProviderTestClient(providerTestOptions);


        // Connection_1_1
        // Channel_1 (preferred Channel name)
        // Provider_9 provides the DIRECT_FEED(RT/TickByTick) and DIRECT_FEED1(RT/JustInTimeConflated) service names.
        OmmProvider ommprovider_3 = EmaFactory.createOmmProvider(config.port("19003").providerName("Provider_1"), providerClient_3);
        assertNotNull(ommprovider_3);

        OmmProvider ommprovider_6 = EmaFactory.createOmmProvider(config.port("19006").providerName("Provider_3_1"), providerClient_6);
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

            consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_56_3"), consumerClient);

            Thread.sleep(2000);

            consumerClient.clearQueue();
            int count = consumerClient.channelInfoSize();
            for (int i = 0; i < count; i++) consumerClient.popChannelInfo();

            String serviceName = "DIRECT_FEED";
            String serviceName2 = "DIRECT_FEED_2";
            String itemName = ".AV.N";

            ElementList payload = EmaFactory.createElementList();
            ElementEntry entry = EmaFactory.createElementEntry();

            ElementList eePayload = EmaFactory.createElementList();
            ElementEntry eeEntry = EmaFactory.createElementEntry();

            eeEntry.uintValue(":DataStreams", SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_STREAMS);
            eePayload.add(eeEntry);

            entry.elementList(":SymbolListBehaviors", eePayload);
            payload.add(entry);

            ReqMsg reqMsg = EmaFactory.createReqMsg().domainType(EmaRdm.MMT_SYMBOL_LIST).serviceName(serviceName).name(itemName).payload(payload);
            ReqMsg reqMsg2 = EmaFactory.createReqMsg().domainType(EmaRdm.MMT_SYMBOL_LIST).serviceName(serviceName2).name(".BV.N").payload(payload);

            System.out.println(">>>>>>> Requesting items...\n");
            long itemHandle = consumer.registerClient(reqMsg, consumerClient);

            Thread.sleep(2000);

            long itemHandle2 = consumer.registerClient(reqMsg2, consumerClient);

            Thread.sleep(5000);

            Msg message;
            ChannelInformation chInfo;
            boolean avFound = false;
            boolean bvFound = false;
            HashSet<Integer> itemStreamIds = new HashSet<>();
            HashSet<String> itemNames = new HashSet<>();

            int msgcount = consumerClient.queueSize();
            for (int i = 0; i < msgcount; i++)
            {
                message = consumerClient.popMessage();
                chInfo = consumerClient.popChannelInfo();
                if (message instanceof RefreshMsg)
                {
                    RefreshMsg refresh = (RefreshMsg)message;
                    if (refresh.hasName())
                    {
                        if (refresh.name().equals(".AV.N"))
                        {
                            assertTrue("Channel_3".equals(chInfo.channelName()));
                            assertTrue("DIRECT_FEED".equals(refresh.serviceName()));
                            avFound = true;
                        }
                        else if (refresh.name().equals(".BV.N"))
                        {
                            assertTrue("Channel_6".equals(chInfo.channelName()));
                            assertTrue("DIRECT_FEED_2".equals(refresh.serviceName()));
                            bvFound = true;
                        }
                        else if (refresh.domainType() == DomainTypes.MARKET_PRICE)
                        {
                            itemNames.add(refresh.name());
                            itemStreamIds.add(refresh.streamId());
                        }
                    }
                }
            }
            assertTrue(avFound && bvFound);
            assertEquals(6, itemStreamIds.size());
            assertEquals(3, itemNames.size());
            assertTrue(itemStreamIds.contains(-1) && itemStreamIds.contains(-2) && itemStreamIds.contains(-3)
                    && itemStreamIds.contains(-4) && itemStreamIds.contains(-5) && itemStreamIds.contains(-6));
            assertTrue(itemNames.contains("itemA") && itemNames.contains("itemB") && itemNames.contains("itemC"));

            System.out.println("Send the DIRECT_FEED_2 service up state for ommprovider_3");
            ElementList serviceState = EmaFactory.createElementList();
            serviceState.add( EmaFactory.createElementEntry().uintValue( EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_UP ));
            serviceState.add( EmaFactory.createElementEntry().uintValue( EmaRdm.ENAME_ACCEPTING_REQS, 1 ));
            serviceState.add( EmaFactory.createElementEntry().state( EmaRdm.ENAME_STATUS, OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, ""));
            FilterList filterListEnc = EmaFactory.createFilterList();
            filterListEnc.add( EmaFactory.createFilterEntry().elementList( EmaRdm.SERVICE_STATE_ID, FilterEntry.FilterAction.SET, serviceState ) );
            Map mapEnc = EmaFactory.createMap();
            mapEnc.add( EmaFactory.createMapEntry().keyUInt( 14, MapEntry.MapAction.UPDATE, filterListEnc ));
            UpdateMsg updateMsg = EmaFactory.createUpdateMsg();
            ommprovider_3.submit( updateMsg.domainType( EmaRdm.MMT_DIRECTORY ).
                    filter( EmaRdm.SERVICE_STATE_FILTER ).
                    payload( mapEnc ), 0);	// use 0 item handle to fan-out to all subscribers


            Thread.sleep(5000);

            itemNames.clear();

            count = consumerClient.queueSize();
            int statusClosedCount = 0;
            for (int i = 0; i < count; i++)
            {
                message = consumerClient.popMessage();
                chInfo = consumerClient.popChannelInfo();
                if (message instanceof StatusMsg)
                {
                    StatusMsg status = (StatusMsg)message;
                    if (status.state().statusText().contains("Individual item from Symbol List closed due to server change."))
                    {
                        statusClosedCount++;
                        assertTrue(chInfo.channelName().equals("Channel_3"));
                    }
                }
            }
            assertEquals(3, statusClosedCount); //Check that we closed all provider-driven items.

            ommprovider_6.uninitialize();
        }
        catch (Exception ex)
        {
            ex.printStackTrace();
            assertFalse(true);
        }
        finally
        {
            System.out.println(">>>>> Uninitializing...");
            assertNotNull(consumer);

            if (consumer != null) consumer.uninitialize();
            if (ommprovider_3 != null) ommprovider_3.uninitialize();
            if (ommprovider_6 != null) ommprovider_6.uninitialize();
        }
    }

    @Test
    public void testRequestingSingleItemWithNonExistenceServiceNameAndAddTheService()
    {
        TestUtilities.printTestHead("testRequestingSingleItemWithNonExistenceServiceNameAndAddTheService","");

        String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";

        OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);

        ProviderTestOptions providerTestOptions = new ProviderTestOptions();

        ProviderTestClient providerClient_1 = new ProviderTestClient(providerTestOptions);

        // Provider_1 provides the DIRECT_FEED service name
        OmmProvider ommprovider_1 = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_1"), providerClient_1);
        assertNotNull(ommprovider_1);

        ProviderTestClient providerClient_4 = new ProviderTestClient(providerTestOptions);

        // Provider_1 provides the DIRECT_FEED service name
        OmmProvider ommprovider_4 = EmaFactory.createOmmProvider(config.port("19004").providerName("Provider_1"), providerClient_4);
        assertNotNull(ommprovider_4);

        OmmConsumer consumer = null;
        ConsumerTestClient consumerClient = new ConsumerTestClient();

        try
        {
            consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_9"));

            String itemName = ".AV.N";
            String itemName2 = ".BV.N";

            ElementList payload = EmaFactory.createElementList();
            ElementEntry entry = EmaFactory.createElementEntry();

            ElementList eePayload = EmaFactory.createElementList();
            ElementEntry eeEntry = EmaFactory.createElementEntry();

            eeEntry.uintValue(":DataStreams", SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_STREAMS);
            eePayload.add(eeEntry);

            entry.elementList(":SymbolListBehaviors", eePayload);
            payload.add(entry);

            ReqMsg reqMsg = EmaFactory.createReqMsg();

            long itemHandle1 = consumer.registerClient(reqMsg.clear().domainType(DomainTypes.SYMBOL_LIST).serviceName("DIRECT_FEED2").name(itemName).payload(payload), consumerClient);
            long itemHandle2 = consumer.registerClient(reqMsg.clear().domainType(DomainTypes.SYMBOL_LIST).serviceName("DIRECT_FEED2").name(itemName2).payload(payload), consumerClient);

            Thread.sleep(2000);

            /* Ensure that the provider doesn't receive any request message */
            assertEquals(0, providerClient_1.queueSize());
            assertEquals(2, consumerClient.queueSize());

            /* Provider send source directory update message to add the DIRECT_FEED2 service */
            OmmArray capablities = EmaFactory.createOmmArray();
            capablities.add(EmaFactory.createOmmArrayEntry().uintValue( EmaRdm.MMT_MARKET_PRICE));
            capablities.add(EmaFactory.createOmmArrayEntry().uintValue( EmaRdm.MMT_MARKET_BY_PRICE));
            capablities.add(EmaFactory.createOmmArrayEntry().uintValue( EmaRdm.MMT_SYMBOL_LIST));
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
            ommprovider_1.submit( updateMsg.domainType(EmaRdm.MMT_DIRECTORY).
                    filter( EmaRdm.SERVICE_INFO_FILTER | EmaRdm.SERVICE_STATE_FILTER).
                    payload(map), 0);

            Thread.sleep(3000);

            // Provider receives a request once the service is available
            assertEquals(5, providerClient_1.queueSize());

            Msg message = providerClient_1.popMessage();
            ReqMsg recvReqMsg = (ReqMsg)message;

            assertEquals(3, recvReqMsg.streamId());
            assertEquals(2, recvReqMsg.serviceId());
            assertEquals("DIRECT_FEED2", recvReqMsg.serviceName());
            assertEquals(".AV.N", recvReqMsg.name());

            message = providerClient_1.popMessage();
            recvReqMsg = (ReqMsg)message;

            assertEquals(4, recvReqMsg.streamId());
            assertEquals(2, recvReqMsg.serviceId());
            assertEquals("DIRECT_FEED2", recvReqMsg.serviceName());
            assertEquals(".BV.N", recvReqMsg.name());

            message = providerClient_1.popMessage();
            recvReqMsg = (ReqMsg)message;

            assertEquals(5, recvReqMsg.streamId());
            assertEquals(2, recvReqMsg.serviceId());
            assertEquals("DIRECT_FEED2", recvReqMsg.serviceName());
            assertEquals("itemA", recvReqMsg.name());

            message = providerClient_1.popMessage();
            recvReqMsg = (ReqMsg)message;

            assertEquals(6, recvReqMsg.streamId());
            assertEquals(2, recvReqMsg.serviceId());
            assertEquals("DIRECT_FEED2", recvReqMsg.serviceName());
            assertEquals("itemB", recvReqMsg.name());

            message = providerClient_1.popMessage();
            recvReqMsg = (ReqMsg)message;

            assertEquals(7, recvReqMsg.streamId());
            assertEquals(2, recvReqMsg.serviceId());
            assertEquals("DIRECT_FEED2", recvReqMsg.serviceName());
            assertEquals("itemC", recvReqMsg.name());

            Thread.sleep(3000);

            assertEquals(7, consumerClient.queueSize());

            for (int i = 0; i < 2; i++)
            {
                message = consumerClient.popMessage();
                assertTrue(message instanceof StatusMsg);

                StatusMsg status = (StatusMsg)message;
                assertTrue(status.state().statusText().contains("No matching service present."));
            }

            message = consumerClient.popMessage();
            RefreshMsg refreshMsg = (RefreshMsg)message;

            assertEquals("DIRECT_FEED2", refreshMsg.serviceName());
            assertEquals(".AV.N", refreshMsg.name());
            assertEquals(32768, refreshMsg.serviceId());
            assertEquals(OmmState.StreamState.OPEN, refreshMsg.state().streamState());
            assertEquals(OmmState.DataState.OK, refreshMsg.state().dataState());
            assertEquals(OmmState.StatusCode.NONE, refreshMsg.state().code());
            assertTrue(refreshMsg.solicited());
            assertTrue(refreshMsg.complete());

            message = consumerClient.popMessage();

            refreshMsg = (RefreshMsg)message;

            assertEquals("DIRECT_FEED2", refreshMsg.serviceName());
            assertEquals(".BV.N", refreshMsg.name());
            assertEquals(32768, refreshMsg.serviceId());
            assertEquals(OmmState.StreamState.OPEN, refreshMsg.state().streamState());
            assertEquals(OmmState.DataState.OK, refreshMsg.state().dataState());
            assertEquals(OmmState.StatusCode.NONE, refreshMsg.state().code());
            assertTrue(refreshMsg.solicited());
            assertTrue(refreshMsg.complete());

            for (int i = 0; i < 3; i++)
            {
                message = consumerClient.popMessage();

                refreshMsg = (RefreshMsg)message;
                assertEquals("DIRECT_FEED2", refreshMsg.serviceName());
                assertTrue("itemA".equals(refreshMsg.name()) || "itemB".equals(refreshMsg.name()) || "itemC".equals(refreshMsg.name()));
                assertEquals(32768, refreshMsg.serviceId());
                assertEquals(OmmState.StreamState.OPEN, refreshMsg.state().streamState());
                assertEquals(OmmState.DataState.OK, refreshMsg.state().dataState());
                assertEquals(OmmState.StatusCode.NONE, refreshMsg.state().code());
                assertTrue(refreshMsg.solicited());
                assertTrue(refreshMsg.complete());
            }

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
            ommprovider_1.uninitialize();
            ommprovider_4.uninitialize();
        }
    }

    @Test
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

            Thread.sleep(2000);

            consumerClient.clearQueue();
            int c = consumerClient.channelInfoSize();
            for (int i = 0; i < c; i++) consumerClient.popChannelInfo();

            String serviceName = "DIRECT_FEED";
            String itemName = ".AV.N";

            ElementList payload = EmaFactory.createElementList();
            ElementEntry entry = EmaFactory.createElementEntry();

            ElementList eePayload = EmaFactory.createElementList();
            ElementEntry eeEntry = EmaFactory.createElementEntry();

            eeEntry.uintValue(":DataStreams", SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_STREAMS);
            eePayload.add(eeEntry);

            entry.elementList(":SymbolListBehaviors", eePayload);
            payload.add(entry);

            ReqMsg reqMsg = EmaFactory.createReqMsg();
            long itemHandle = consumer.registerClient(reqMsg.name(itemName).serviceName(serviceName).domainType(DomainTypes.SYMBOL_LIST).payload(payload), consumerClient);

            Thread.sleep(2000);

            HashSet<String> itemNames = new HashSet<String>();
            HashSet<Integer> streamIds = new HashSet<Integer>();

            int count = consumerClient.queueSize();
            for (int i = 0; i < count; i++)
            {
                Msg message = consumerClient.popMessage();
                ChannelInformation chInfo = consumerClient.popChannelInfo();

                if (message instanceof RefreshMsg)
                {
                    RefreshMsg refresh = (RefreshMsg)message;
                    if (refresh.hasName()
                            && (refresh.name().equals(".AV.N")
                            || refresh.name().equals("itemA")
                            || refresh.name().equals("itemB")
                            || refresh.name().equals("itemC")))
                    {
                        if (refresh.name().equals(".AV.N")) assertEquals(5, refresh.streamId());
                        itemNames.add(refresh.name());
                        streamIds.add(refresh.streamId());
                    }
                    assertTrue("Channel_3".equals(chInfo.channelName()));
                }
            }
            assertTrue(itemNames.contains("itemA") && itemNames.contains("itemB") && itemNames.contains("itemC"));
            assertTrue(streamIds.contains(-1) && streamIds.contains(-2) && streamIds.contains(-3));

            Thread.sleep(3000);

            // Call the method to fallback but the preferred group is not up yet.
            System.out.println("\nCalls fallbackPreferredHost() to fallback to WarmStandbyChannel_2(Do nothing)");
            consumer.fallbackPreferredHost();

            Thread.sleep(3000);

            count = consumerClient.queueSize();
            // Checks for PH START and COMPLETE events
            for (int i = 0; i < count; i++)
            {
                Msg message = consumerClient.popMessage();
                ChannelInformation chInfo = consumerClient.popChannelInfo();
                assertTrue(message instanceof StatusMsg);
                StatusMsg status = (StatusMsg)message;
                assertTrue(status.state().statusText().contains("preferred host starting fallback")
                        || status.state().statusText().contains("preferred host complete"));
                assertEquals(DomainTypes.LOGIN, status.domainType());
                assertEquals(ChannelInformation.ChannelState.ACTIVE, chInfo.channelState());
                assertEquals("WarmStandbyChannel_2", chInfo.preferredHostInfo().getWsbChannelName());
            }

            // Start the preferred WSB group WSB-G1
            ommprovider3 = EmaFactory.createOmmProvider(config.port("19007").providerName("Provider_1"), providerClient3);

            ommprovider4 = EmaFactory.createOmmProvider(config.port("19008").providerName("Provider_1"), providerClient4);

            // The fallback should happen by the detection time interval
            System.out.println("Fallback by the detection time interval.");

            Thread.sleep(15000);

            count = consumerClient.queueSize();

            boolean phStartFound = false;
            boolean phFinishFound= false;

            int closedMarketPriceItemsCount = 0;
            HashSet<Integer> closedStreamIds = new HashSet<>();
            streamIds.clear();

            for (int i = 0; i < count; i++)
            {
                Msg message = consumerClient.popMessage();
                ChannelInformation channelInfo = consumerClient.popChannelInfo();

                if (message instanceof StatusMsg)
                {
                    StatusMsg status = (StatusMsg)message;
                    if (status.state().statusText().contains("preferred host starting fallback"))
                    {
                        phStartFound = true;
                        assertTrue("Channel_3".equals(channelInfo.channelName()));
                    }
                    else if (status.state().statusText().contains("preferred host complete"))
                    {
                        phFinishFound = true;
                        assertTrue("Channel_7".equals(channelInfo.channelName()));
                    }
                    else if (status.state().statusText().contains("Individual item from Symbol List closed due to server change."))
                    {
                        closedMarketPriceItemsCount++;
                        closedStreamIds.add(status.streamId());
                        assertTrue("Channel_3".equals(channelInfo.channelName()));
                    }
                }
                else if (message instanceof RefreshMsg)
                {
                    RefreshMsg refresh = (RefreshMsg)message;
                    streamIds.add(refresh.streamId());
                    assertTrue("Channel_7".equals(channelInfo.channelName()));
                }
            }
            assertTrue(phStartFound && phFinishFound);
            assertEquals(3, closedMarketPriceItemsCount);
            assertTrue(closedStreamIds.contains(-1) && closedStreamIds.contains(-2) && closedStreamIds.contains(-3));
            assertTrue(streamIds.contains(-4) && streamIds.contains(-5) && streamIds.contains(-6));

            Thread.sleep(10000);

            // Checks for PH START and COMPLETE events

            /* Checks login status messages */
            Msg message = consumerClient.popMessage();
            ChannelInformation channelInfo = consumerClient.popChannelInfo();
            StatusMsg statusMsg = (StatusMsg)message;

            assertEquals(1, statusMsg.streamId());
            assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
            assertEquals("Open / Ok / PreferredHostNoFallback / 'preferred host no fallback'", statusMsg.state().toString());
            assertTrue(statusMsg.hasMsgKey());
            assertEquals(DataType.DataTypes.NO_DATA, statusMsg.payload().dataType());
            channelInfo = consumerClient.popChannelInfo();
            assertEquals("Channel_7", channelInfo.channelName());
            assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
            assertEquals("WarmStandbyChannel_2", channelInfo.preferredHostInfo().getWsbChannelName());

            consumer.unregister(itemHandle);

            Thread.sleep(2000);
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

    @Test
    public void testSingleConnectionForWSBServiceBasedDetectionTimeIntervalAndHostDownToTestWithWSBGroupReconnectLogicWithEnablingFallbackWithInWSBGroup()
    {
        TestUtilities.printTestHead("testSingleConnectionForWSBServiceBasedDetectionTimeIntervalAndHostDownToTestWithWSBGroupReconnectLogicWithEnablingFallbackWithInWSBGroup","");

        String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";

        OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);

        ProviderTestOptions providerTestOptions = new ProviderTestOptions();
        providerTestOptions.supportStandby = true;
        providerTestOptions.sendRefreshAttrib = true;
        providerTestOptions.itemGroupId = ByteBuffer.wrap("10".getBytes());

        ProviderTestClient providerClient_3 = new ProviderTestClient(providerTestOptions);
        ProviderTestClient providerClient_6 = new ProviderTestClient(providerTestOptions);
        ProviderTestClient providerClient_7 = new ProviderTestClient(providerTestOptions);
        ProviderTestClient providerClient_8 = new ProviderTestClient(providerTestOptions);
        ProviderTestClient providerClient_9 = new ProviderTestClient(providerTestOptions);
        ProviderTestClient providerClient10 = new ProviderTestClient(providerTestOptions);

        //WSB-G0 (WarmStandbyChannel_3).
        // Provider_9 provides the DIRECT_FEED and DIRECT_FEED1 service name for WSB channel.
        OmmProvider ommprovider_3 = EmaFactory.createOmmProvider(config.port("19003").providerName("Provider_9"), providerClient_3);
        assertNotNull(ommprovider_3);

        OmmProvider ommprovider_6 = EmaFactory.createOmmProvider(config.port("19006").providerName("Provider_9"), providerClient_6);
        assertNotNull(ommprovider_6);

        //WSB-G1 (WarmStandbyChannel_4). This is preferred WSB group.
        // Provider_9 provides the DIRECT_FEED and DIRECT_FEED1 service name for WSB channel.
        OmmProvider ommprovider_7 = EmaFactory.createOmmProvider(config.port("19007").providerName("Provider_9"), providerClient_7);
        assertNotNull(ommprovider_7);

        OmmProvider ommprovider_8 = EmaFactory.createOmmProvider(config.port("19008").providerName("Provider_9"), providerClient_8);
        assertNotNull(ommprovider_8);

        //WSB-G2 (WarmStandbyChannel_7).
        // Provider_9 provides the DIRECT_FEED and DIRECT_FEED1 service name for WSB channel.
        OmmProvider ommprovider_9 = EmaFactory.createOmmProvider(config.port("19009").providerName("Provider_9"), providerClient_9);
        assertNotNull(ommprovider_9);

        OmmProvider ommprovider10 = EmaFactory.createOmmProvider(config.port("19010").providerName("Provider_9"), providerClient10);
        assertNotNull(ommprovider10);

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

            Thread.sleep(2000);

            consumerClient.clearQueue();
            while (consumerClient.popChannelInfo() != null) {}

            String serviceName = "DIRECT_FEED";
            String itemName = ".AV.N";

            String serviceName2 = "DIRECT_FEED1";
            String itemName2 = "BV.N";

            ElementList payload = EmaFactory.createElementList();
            ElementEntry entry = EmaFactory.createElementEntry();

            ElementList eePayload = EmaFactory.createElementList();
            ElementEntry eeEntry = EmaFactory.createElementEntry();

            eeEntry.uintValue(":DataStreams", SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_STREAMS);
            eePayload.add(eeEntry);

            entry.elementList(":SymbolListBehaviors", eePayload);
            payload.add(entry);

            ReqMsg reqMsg = EmaFactory.createReqMsg();

            long itemHandle = consumer.registerClient(reqMsg.name(itemName).serviceName(serviceName).domainType(EmaRdm.MMT_SYMBOL_LIST).payload(payload), consumerClient);

            Thread.sleep(500);

            long itemHandle2 = consumer.registerClient(reqMsg.clear().name(itemName2).serviceName(serviceName2).domainType(EmaRdm.MMT_SYMBOL_LIST).payload(payload), consumerClient);

            Thread.sleep(2000);

            consumerClient.clearQueue();
            while (consumerClient.popChannelInfo() != null) {}

            List<Integer> serviceIdList = new ArrayList<Integer>();
            serviceIdList.add(1);
            serviceIdList.add(2);

            System.out.println("Bring down all services for ommprovider_7");
            int iter = 0;
            for (Integer serviceId : serviceIdList)
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
                ommprovider_7.submit( updateMsg.domainType( EmaRdm.MMT_DIRECTORY ).
                        filter( EmaRdm.SERVICE_STATE_FILTER ).
                        payload( mapEnc ), 0);	// use 0 item handle to fan-out to all subscribers

                Thread.sleep(3000);

                HashSet<String> streamNames = new HashSet<String>();
                HashSet<Integer> closedStreamIds = new HashSet<Integer>();

                int count = consumerClient.queueSize();
                for (int i = 0; i < count; i++)
                {
                    Msg msg = consumerClient.popMessage();
                    ChannelInformation channelInfo = consumerClient.popChannelInfo();

                    if (msg instanceof StatusMsg)
                    {
                        StatusMsg status = (StatusMsg)msg;
                        if (status.state().statusText().contains("Individual item from Symbol List closed due to service down"))
                        {
                            closedStreamIds.add(status.streamId());
                            assertEquals("Channel_7", channelInfo.channelName());
                        }
                    }
                    else if (msg instanceof RefreshMsg)
                    {
                        RefreshMsg refresh = (RefreshMsg)msg;
                        assertEquals("Channel_8", channelInfo.channelName());
                        if (refresh.domainType() == DomainTypes.SYMBOL_LIST)
                        {
                            assertTrue(refresh.state().statusText().contains("Unsolicited Refresh Completed"));
                        }
                        streamNames.add(refresh.name());
                    }
                }
                assertTrue(streamNames.contains("itemA") && streamNames.contains("itemB") && streamNames.contains("itemC"));
                assertTrue(closedStreamIds.contains(-1 + iter) && closedStreamIds.contains(-2 + iter) && closedStreamIds.contains(-3 + iter));

                streamNames.clear();
                closedStreamIds.clear();

                iter = -3;
            }

            Thread.sleep(3000);

            System.out.println("\n>>>>> Bring down ommprovider_7 ");
            ommprovider_7.uninitialize();
            ommprovider_7 = null;
            Thread.sleep(3000);

            System.out.println(" >>>>> Bring down provider_8 ");
            ommprovider_8.uninitialize();
            ommprovider_8 = null;
            Thread.sleep(5000);

            HashSet<String> streamNames = new HashSet<String>();
            HashSet<Integer> closedStreamIds = new HashSet<Integer>();

            int count = consumerClient.queueSize();
            for (int i = 0; i < count; i++)
            {
                Msg msg = consumerClient.popMessage();
                ChannelInformation channelInfo = consumerClient.popChannelInfo();

                if (msg instanceof StatusMsg)
                {
                    StatusMsg status = (StatusMsg)msg;
                    if (status.state().statusText().contains("Individual item from Symbol List closed due to server change"))
                    {
                        closedStreamIds.add(status.streamId());
                        assertEquals("Channel_8", channelInfo.channelName());
                    }
                }
                else if (msg instanceof RefreshMsg)
                {
                    RefreshMsg refresh = (RefreshMsg)msg;
                    assertEquals("Channel_3", channelInfo.channelName());
                    if (refresh.domainType() == DomainTypes.SYMBOL_LIST)
                    {
                        assertTrue(refresh.state().statusText().contains("Refresh Completed"));
                    }
                    streamNames.add(refresh.name());
                }
            }
            assertTrue(streamNames.contains("itemA") && streamNames.contains("itemB") && streamNames.contains("itemC"));
            assertTrue(closedStreamIds.contains(-7) && closedStreamIds.contains(-8) && closedStreamIds.contains(-9)
                    && closedStreamIds.contains(-10) && closedStreamIds.contains(-11) && closedStreamIds.contains(-12));

            System.out.println(">>>>>> Bring up ommprovider_7 and ommprovider_8 for WSB-G1 again");
            ommprovider_7 = EmaFactory.createOmmProvider(config.port("19007").providerName("Provider_9"), providerClient_7);

            ommprovider_8 = EmaFactory.createOmmProvider(config.port("19008").providerName("Provider_9"), providerClient_8);

            Thread.sleep(10000);

            /* Check the fallback message from detection time internal which doesn't fallback to preferred group as
             * the PHFallBackWithInWSBGroup is set to true. */

            boolean phStartFound = false;
            boolean phFinishFound = false;

            count = consumerClient.queueSize();

            for (int i = 0; i < count; i++)
            {
                ChannelInformation channelInfo = consumerClient.popChannelInfo();
                Msg msg = consumerClient.popMessage();

                if (msg instanceof StatusMsg)
                {
                    StatusMsg status = (StatusMsg)msg;
                    if (status.state().statusText().contains("preferred host starting fallback"))
                    {
                        phStartFound = true;
                        assertEquals("Channel_3", channelInfo.channelName());
                        assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
                    }
                    else if (status.state().statusText().contains("preferred host complete"))
                    {
                        phFinishFound = true;
                        assertEquals("Channel_3", channelInfo.channelName());
                        assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
                    }
                }
            }
            assertTrue(phStartFound && phFinishFound);

            consumer.unregister(itemHandle);
            consumer.unregister(itemHandle2);

            Thread.sleep(3000);
        }
        catch(Exception exp)
        {
            System.out.println(exp.getMessage());
            assertFalse(true);
        }
        finally
        {
            consumer.uninitialize();

            ommprovider_3.uninitialize();
            ommprovider_6.uninitialize();

            if(ommprovider_7 != null)
                ommprovider_7.uninitialize();

            if(ommprovider_8 != null)
                ommprovider_8.uninitialize();

            ommprovider_9.uninitialize();
            ommprovider10.uninitialize();
        }
    }

    @Test
    public void testSingleConnectionForWSBServiceBasedServiceDeleted()
    {
        TestUtilities.printTestHead("testSingleConnectionForWSBServiceBasedServiceDeleted","");

        String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";

        OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);

        ProviderTestOptions providerTestOptions = new ProviderTestOptions();
        providerTestOptions.supportStandby = true;
        providerTestOptions.sendRefreshAttrib = true;
        providerTestOptions.itemGroupId = ByteBuffer.wrap("10".getBytes());

        ProviderTestClient providerClient_3 = new ProviderTestClient(providerTestOptions);
        ProviderTestClient providerClient_6 = new ProviderTestClient(providerTestOptions);
        ProviderTestClient providerClient_7 = new ProviderTestClient(providerTestOptions);
        ProviderTestClient providerClient_8 = new ProviderTestClient(providerTestOptions);
        ProviderTestClient providerClient_9 = new ProviderTestClient(providerTestOptions);
        ProviderTestClient providerClient10 = new ProviderTestClient(providerTestOptions);

        //WSB-G0 (WarmStandbyChannel_3).
        // Provider_9 provides the DIRECT_FEED and DIRECT_FEED1 service name for WSB channel.
        OmmProvider ommprovider_3 = EmaFactory.createOmmProvider(config.port("19003").providerName("Provider_9"), providerClient_3);
        assertNotNull(ommprovider_3);

        OmmProvider ommprovider_6 = EmaFactory.createOmmProvider(config.port("19006").providerName("Provider_9"), providerClient_6);
        assertNotNull(ommprovider_6);

        //WSB-G1 (WarmStandbyChannel_4). This is preferred WSB group.
        // Provider_9 provides the DIRECT_FEED and DIRECT_FEED1 service name for WSB channel.
        OmmProvider ommprovider_7 = EmaFactory.createOmmProvider(config.port("19007").providerName("Provider_9"), providerClient_7);
        assertNotNull(ommprovider_7);

        OmmProvider ommprovider_8 = EmaFactory.createOmmProvider(config.port("19008").providerName("Provider_9"), providerClient_8);
        assertNotNull(ommprovider_8);

        //WSB-G2 (WarmStandbyChannel_7).
        // Provider_9 provides the DIRECT_FEED and DIRECT_FEED1 service name for WSB channel.
        OmmProvider ommprovider_9 = EmaFactory.createOmmProvider(config.port("19009").providerName("Provider_9"), providerClient_9);
        assertNotNull(ommprovider_9);

        OmmProvider ommprovider10 = EmaFactory.createOmmProvider(config.port("19010").providerName("Provider_9"), providerClient10);
        assertNotNull(ommprovider10);

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

            Thread.sleep(2000);

            consumerClient.clearQueue();
            while (consumerClient.popChannelInfo() != null) {}

            String serviceName = "DIRECT_FEED";
            String itemName = ".AV.N";

            String serviceName2 = "DIRECT_FEED1";
            String itemName2 = "BV.N";

            ElementList payload = EmaFactory.createElementList();
            ElementEntry entry = EmaFactory.createElementEntry();

            ElementList eePayload = EmaFactory.createElementList();
            ElementEntry eeEntry = EmaFactory.createElementEntry();

            eeEntry.uintValue(":DataStreams", SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_STREAMS);
            eePayload.add(eeEntry);

            entry.elementList(":SymbolListBehaviors", eePayload);
            payload.add(entry);

            ReqMsg reqMsg = EmaFactory.createReqMsg();

            long itemHandle = consumer.registerClient(reqMsg.name(itemName).serviceName(serviceName).domainType(EmaRdm.MMT_SYMBOL_LIST).payload(payload), consumerClient);

            Thread.sleep(500);

            long itemHandle2 = consumer.registerClient(reqMsg.clear().name(itemName2).serviceName(serviceName2).domainType(EmaRdm.MMT_SYMBOL_LIST).payload(payload), consumerClient);

            Thread.sleep(2000);

            consumerClient.clearQueue();
            while (consumerClient.popChannelInfo() != null) {}

            List<Integer> serviceIdList = new ArrayList<Integer>();
            serviceIdList.add(1);
            serviceIdList.add(2);

            System.out.println("Bring down all services for ommprovider_7");
            int iter = 0;
            for (Integer serviceId : serviceIdList)
            {
                ElementList serviceState = EmaFactory.createElementList();
                serviceState.add( EmaFactory.createElementEntry().uintValue( EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_DOWN ));
                serviceState.add( EmaFactory.createElementEntry().uintValue( EmaRdm.ENAME_ACCEPTING_REQS, 0 ));
                serviceState.add( EmaFactory.createElementEntry().state( EmaRdm.ENAME_STATUS, OmmState.StreamState.OPEN, OmmState.DataState.SUSPECT, OmmState.StatusCode.NONE, ""));
                FilterList filterListEnc = EmaFactory.createFilterList();
                filterListEnc.add( EmaFactory.createFilterEntry().elementList( EmaRdm.SERVICE_STATE_ID, FilterEntry.FilterAction.SET, serviceState ) );
                Map mapEnc = EmaFactory.createMap();
                mapEnc.add( EmaFactory.createMapEntry().keyUInt( serviceId.intValue(), MapEntry.MapAction.DELETE, filterListEnc ));
                UpdateMsg updateMsg = EmaFactory.createUpdateMsg();
                ommprovider_7.submit( updateMsg.domainType( EmaRdm.MMT_DIRECTORY ).
                        filter( EmaRdm.SERVICE_STATE_FILTER ).
                        payload( mapEnc ), 0);	// use 0 item handle to fan-out to all subscribers

                Thread.sleep(3000);

                HashSet<String> streamNames = new HashSet<String>();
                HashSet<Integer> closedStreamIds = new HashSet<Integer>();

                int count = consumerClient.queueSize();
                for (int i = 0; i < count; i++)
                {
                    Msg msg = consumerClient.popMessage();
                    ChannelInformation channelInfo = consumerClient.popChannelInfo();

                    if (msg instanceof StatusMsg)
                    {
                        StatusMsg status = (StatusMsg)msg;
                        if (status.state().statusText().contains("Individual item from Symbol List closed due to service deleted"))
                        {
                            closedStreamIds.add(status.streamId());
                            assertEquals("Channel_7", channelInfo.channelName());
                        }
                    }
                    else if (msg instanceof RefreshMsg)
                    {
                        RefreshMsg refresh = (RefreshMsg)msg;
                        assertEquals("Channel_8", channelInfo.channelName());
                        if (refresh.domainType() == DomainTypes.SYMBOL_LIST)
                        {
                            assertTrue(refresh.state().statusText().contains("Unsolicited Refresh Completed"));
                        }
                        streamNames.add(refresh.name());
                    }
                }
                assertTrue(streamNames.contains("itemA") && streamNames.contains("itemB") && streamNames.contains("itemC"));
                assertTrue(closedStreamIds.contains(-1 + iter) && closedStreamIds.contains(-2 + iter) && closedStreamIds.contains(-3 + iter));

                streamNames.clear();
                closedStreamIds.clear();

                iter = -3;
            }

            Thread.sleep(3000);

            System.out.println("\n>>>>> Bring down ommprovider_7 ");
            ommprovider_7.uninitialize();
            ommprovider_7 = null;
            Thread.sleep(3000);

            System.out.println(" >>>>> Bring down provider_8 ");
            ommprovider_8.uninitialize();
            ommprovider_8 = null;
            Thread.sleep(5000);

            HashSet<String> streamNames = new HashSet<String>();
            HashSet<Integer> closedStreamIds = new HashSet<Integer>();

            int count = consumerClient.queueSize();
            for (int i = 0; i < count; i++)
            {
                Msg msg = consumerClient.popMessage();
                ChannelInformation channelInfo = consumerClient.popChannelInfo();

                if (msg instanceof StatusMsg)
                {
                    StatusMsg status = (StatusMsg)msg;
                    if (status.state().statusText().contains("Individual item from Symbol List closed due to server change"))
                    {
                        closedStreamIds.add(status.streamId());
                        assertEquals("Channel_8", channelInfo.channelName());
                    }
                }
                else if (msg instanceof RefreshMsg)
                {
                    RefreshMsg refresh = (RefreshMsg)msg;
                    assertEquals("Channel_3", channelInfo.channelName());
                    if (refresh.domainType() == DomainTypes.SYMBOL_LIST)
                    {
                        assertTrue(refresh.state().statusText().contains("Refresh Completed"));
                    }
                    streamNames.add(refresh.name());
                }
            }
            assertTrue(streamNames.contains("itemA") && streamNames.contains("itemB") && streamNames.contains("itemC"));
                                                                                                assertTrue(closedStreamIds.contains(-7) && closedStreamIds.contains(-8) && closedStreamIds.contains(-9)
                    && closedStreamIds.contains(-10) && closedStreamIds.contains(-11) && closedStreamIds.contains(-12));

            System.out.println(">>>>>> Bring up ommprovider_7 and ommprovider_8 for WSB-G1 again");
            ommprovider_7 = EmaFactory.createOmmProvider(config.port("19007").providerName("Provider_9"), providerClient_7);

            ommprovider_8 = EmaFactory.createOmmProvider(config.port("19008").providerName("Provider_9"), providerClient_8);

            Thread.sleep(10000);

            /* Check the fallback message from detection time internal which doesn't fallback to preferred group as
             * the PHFallBackWithInWSBGroup is set to true. */

            boolean phStartFound = false;
            boolean phFinishFound = false;

            count = consumerClient.queueSize();

            for (int i = 0; i < count; i++)
            {
                ChannelInformation channelInfo = consumerClient.popChannelInfo();
                Msg msg = consumerClient.popMessage();

                if (msg instanceof StatusMsg)
                {
                    StatusMsg status = (StatusMsg)msg;
                    if (status.state().statusText().contains("preferred host starting fallback"))
                    {
                        phStartFound = true;
                        assertEquals("Channel_3", channelInfo.channelName());
                        assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
                    }
                    else if (status.state().statusText().contains("preferred host complete"))
                    {
                        phFinishFound = true;
                        assertEquals("Channel_3", channelInfo.channelName());
                        assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
                    }
                }
            }
            assertTrue(phStartFound && phFinishFound);

            consumer.unregister(itemHandle);
            consumer.unregister(itemHandle2);

            Thread.sleep(3000);
        }
        catch(Exception exp)
        {
            System.out.println(exp.getMessage());
            assertFalse(true);
        }
        finally
        {
            consumer.uninitialize();

            ommprovider_3.uninitialize();
            ommprovider_6.uninitialize();

            if(ommprovider_7 != null)
                ommprovider_7.uninitialize();

            if(ommprovider_8 != null)
                ommprovider_8.uninitialize();

            ommprovider_9.uninitialize();
            ommprovider10.uninitialize();
        }
    }

    @Test
    public void testSingleConnectionForWSBServiceBasedServiceDown()
    {
        TestUtilities.printTestHead("testSingleConnectionForWSBServiceBasedServiceDown","");

        String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";

        OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);

        ProviderTestOptions providerTestOptions = new ProviderTestOptions();
        providerTestOptions.supportStandby = true;
        providerTestOptions.sendRefreshAttrib = true;
        providerTestOptions.itemGroupId = ByteBuffer.wrap("10".getBytes());

        ProviderTestClient providerClient_3 = new ProviderTestClient(providerTestOptions);
        ProviderTestClient providerClient_6 = new ProviderTestClient(providerTestOptions);
        ProviderTestClient providerClient_7 = new ProviderTestClient(providerTestOptions);
        ProviderTestClient providerClient_8 = new ProviderTestClient(providerTestOptions);
        ProviderTestClient providerClient_9 = new ProviderTestClient(providerTestOptions);
        ProviderTestClient providerClient10 = new ProviderTestClient(providerTestOptions);

        //WSB-G0 (WarmStandbyChannel_3).
        // Provider_9 provides the DIRECT_FEED and DIRECT_FEED1 service name for WSB channel.
        OmmProvider ommprovider_3 = EmaFactory.createOmmProvider(config.port("19003").providerName("Provider_9"), providerClient_3);
        assertNotNull(ommprovider_3);

        OmmProvider ommprovider_6 = EmaFactory.createOmmProvider(config.port("19006").providerName("Provider_9"), providerClient_6);
        assertNotNull(ommprovider_6);

        //WSB-G1 (WarmStandbyChannel_4). This is preferred WSB group.
        // Provider_9 provides the DIRECT_FEED and DIRECT_FEED1 service name for WSB channel.
        OmmProvider ommprovider_7 = EmaFactory.createOmmProvider(config.port("19007").providerName("Provider_9"), providerClient_7);
        assertNotNull(ommprovider_7);

        OmmProvider ommprovider_8 = EmaFactory.createOmmProvider(config.port("19008").providerName("Provider_9"), providerClient_8);
        assertNotNull(ommprovider_8);

        //WSB-G2 (WarmStandbyChannel_7).
        // Provider_9 provides the DIRECT_FEED and DIRECT_FEED1 service name for WSB channel.
        OmmProvider ommprovider_9 = EmaFactory.createOmmProvider(config.port("19009").providerName("Provider_9"), providerClient_9);
        assertNotNull(ommprovider_9);

        OmmProvider ommprovider10 = EmaFactory.createOmmProvider(config.port("19010").providerName("Provider_9"), providerClient10);
        assertNotNull(ommprovider10);

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

            Thread.sleep(2000);

            consumerClient.clearQueue();
            while (consumerClient.popChannelInfo() != null) {}

            String serviceName = "DIRECT_FEED";
            String itemName = ".AV.N";

            String serviceName2 = "DIRECT_FEED1";
            String itemName2 = "BV.N";

            ElementList payload = EmaFactory.createElementList();
            ElementEntry entry = EmaFactory.createElementEntry();

            ElementList eePayload = EmaFactory.createElementList();
            ElementEntry eeEntry = EmaFactory.createElementEntry();

            eeEntry.uintValue(":DataStreams", SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_STREAMS);
            eePayload.add(eeEntry);

            entry.elementList(":SymbolListBehaviors", eePayload);
            payload.add(entry);

            ReqMsg reqMsg = EmaFactory.createReqMsg();

            long itemHandle = consumer.registerClient(reqMsg.name(itemName).serviceName(serviceName).domainType(EmaRdm.MMT_SYMBOL_LIST).payload(payload), consumerClient);

            Thread.sleep(500);

            long itemHandle2 = consumer.registerClient(reqMsg.clear().name(itemName2).serviceName(serviceName2).domainType(EmaRdm.MMT_SYMBOL_LIST).payload(payload), consumerClient);

            Thread.sleep(2000);

            consumerClient.clearQueue();
            while (consumerClient.popChannelInfo() != null) {}

            List<Integer> serviceIdList = new ArrayList<Integer>();
            serviceIdList.add(1);
            serviceIdList.add(2);

            System.out.println("Bring down all services for ommprovider_7");
            int iter = 0;
            for (Integer serviceId : serviceIdList)
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
                ommprovider_7.submit( updateMsg.domainType( EmaRdm.MMT_DIRECTORY ).
                        filter( EmaRdm.SERVICE_STATE_FILTER ).
                        payload( mapEnc ), 0);	// use 0 item handle to fan-out to all subscribers

                Thread.sleep(3000);

                HashSet<String> streamNames = new HashSet<String>();
                HashSet<Integer> closedStreamIds = new HashSet<Integer>();

                int count = consumerClient.queueSize();
                for (int i = 0; i < count; i++)
                {
                    Msg msg = consumerClient.popMessage();
                    ChannelInformation channelInfo = consumerClient.popChannelInfo();

                    if (msg instanceof StatusMsg)
                    {
                        StatusMsg status = (StatusMsg)msg;
                        if (status.state().statusText().contains("Individual item from Symbol List closed due to service down"))
                        {
                            closedStreamIds.add(status.streamId());
                            assertEquals("Channel_7", channelInfo.channelName());
                        }
                    }
                    else if (msg instanceof RefreshMsg)
                    {
                        RefreshMsg refresh = (RefreshMsg)msg;
                        assertEquals("Channel_8", channelInfo.channelName());
                        if (refresh.domainType() == DomainTypes.SYMBOL_LIST)
                        {
                            assertTrue(refresh.state().statusText().contains("Unsolicited Refresh Completed"));
                        }
                        streamNames.add(refresh.name());
                    }
                }
                assertTrue(streamNames.contains("itemA") && streamNames.contains("itemB") && streamNames.contains("itemC"));
                assertTrue(closedStreamIds.contains(-1 + iter) && closedStreamIds.contains(-2 + iter) && closedStreamIds.contains(-3 + iter));

                streamNames.clear();
                closedStreamIds.clear();

                iter = -3;
            }

            Thread.sleep(3000);

            System.out.println("\n>>>>> Bring down ommprovider_7 ");
            ommprovider_7.uninitialize();
            ommprovider_7 = null;
            Thread.sleep(3000);

            System.out.println(" >>>>> Bring down provider_8 ");
            ommprovider_8.uninitialize();
            ommprovider_8 = null;
            Thread.sleep(5000);

            HashSet<String> streamNames = new HashSet<String>();
            HashSet<Integer> closedStreamIds = new HashSet<Integer>();

            int count = consumerClient.queueSize();
            for (int i = 0; i < count; i++)
            {
                Msg msg = consumerClient.popMessage();
                ChannelInformation channelInfo = consumerClient.popChannelInfo();

                if (msg instanceof StatusMsg)
                {
                    StatusMsg status = (StatusMsg)msg;
                    if (status.state().statusText().contains("Individual item from Symbol List closed due to server change"))
                    {
                        closedStreamIds.add(status.streamId());
                        assertEquals("Channel_8", channelInfo.channelName());
                    }
                }
                else if (msg instanceof RefreshMsg)
                {
                    RefreshMsg refresh = (RefreshMsg)msg;
                    assertEquals("Channel_3", channelInfo.channelName());
                    if (refresh.domainType() == DomainTypes.SYMBOL_LIST)
                    {
                        assertTrue(refresh.state().statusText().contains("Refresh Completed"));
                    }
                    streamNames.add(refresh.name());
                }
            }
            assertTrue(streamNames.contains("itemA") && streamNames.contains("itemB") && streamNames.contains("itemC"));
            assertTrue(closedStreamIds.contains(-7) && closedStreamIds.contains(-8) && closedStreamIds.contains(-9)
                    && closedStreamIds.contains(-10) && closedStreamIds.contains(-11) && closedStreamIds.contains(-12));

            System.out.println(">>>>>> Bring up ommprovider_7 and ommprovider_8 for WSB-G1 again");
            ommprovider_7 = EmaFactory.createOmmProvider(config.port("19007").providerName("Provider_9"), providerClient_7);

            ommprovider_8 = EmaFactory.createOmmProvider(config.port("19008").providerName("Provider_9"), providerClient_8);

            Thread.sleep(10000);

            /* Check the fallback message from detection time internal which doesn't fallback to preferred group as
             * the PHFallBackWithInWSBGroup is set to true. */

            boolean phStartFound = false;
            boolean phFinishFound = false;

            count = consumerClient.queueSize();

            for (int i = 0; i < count; i++)
            {
                ChannelInformation channelInfo = consumerClient.popChannelInfo();
                Msg msg = consumerClient.popMessage();

                if (msg instanceof StatusMsg)
                {
                    StatusMsg status = (StatusMsg)msg;
                    if (status.state().statusText().contains("preferred host starting fallback"))
                    {
                        phStartFound = true;
                        assertEquals("Channel_3", channelInfo.channelName());
                        assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
                    }
                    else if (status.state().statusText().contains("preferred host complete"))
                    {
                        phFinishFound = true;
                        assertEquals("Channel_3", channelInfo.channelName());
                        assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
                    }
                }
            }
            assertTrue(phStartFound && phFinishFound);

            consumer.unregister(itemHandle);
            consumer.unregister(itemHandle2);

            Thread.sleep(3000);
        }
        catch(Exception exp)
        {
            System.out.println(exp.getMessage());
            assertFalse(true);
        }
        finally
        {
            consumer.uninitialize();

            ommprovider_3.uninitialize();
            ommprovider_6.uninitialize();

            if(ommprovider_7 != null)
                ommprovider_7.uninitialize();

            if(ommprovider_8 != null)
                ommprovider_8.uninitialize();

            ommprovider_9.uninitialize();
            ommprovider10.uninitialize();
        }
    }

    @Test
    public void testMultiConnectionsItemRequestsWithServiceListNameButConcreteServicesAreNotAvaliableThenConcreteServiceIsAdded()
    {
        TestUtilities.printTestHead("testMultiConnectionsItemRequestsWithServiceListNameButConcreteServicesAreNotAvaliableThenConcreateServiceIsAdded","");

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
            ElementList payload = EmaFactory.createElementList();
            ElementEntry entry = EmaFactory.createElementEntry();

            ElementList eePayload = EmaFactory.createElementList();
            ElementEntry eeEntry = EmaFactory.createElementEntry();

            eeEntry.uintValue(":DataStreams", SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_STREAMS);
            eePayload.add(eeEntry);

            entry.elementList(":SymbolListBehaviors", eePayload);
            payload.add(entry);

            ServiceList serviceList = EmaFactory.createServiceList("SVG1");

            serviceList.concreteServiceList().add("UNKNOWN_SERVICE");
            serviceList.concreteServiceList().add("DIRECT_FEED2");

            consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_9").addServiceList(serviceList));

            Thread.sleep(2000);

            consumerClient.clearQueue();
            while (consumerClient.popChannelInfo() != null) {}

            ReqMsg reqMsg = EmaFactory.createReqMsg();

            long itemHandle = consumer.registerClient(reqMsg.clear().serviceListName("SVG1").name(".AV.N").domainType(DomainTypes.SYMBOL_LIST).payload(payload), consumerClient);

            Thread.sleep(2000);

            assertEquals(1, consumerClient.queueSize());

            Msg message = consumerClient.popMessage();

            StatusMsg statusMsg = (StatusMsg)message;

            assertEquals("SVG1", statusMsg.serviceName());
            assertEquals(".AV.N", statusMsg.name());
            assertEquals(OmmState.StreamState.OPEN, statusMsg.state().streamState());
            assertEquals(OmmState.DataState.SUSPECT, statusMsg.state().dataState());
            assertEquals(OmmState.StatusCode.NONE, statusMsg.state().statusCode());
            assertEquals("No matching service present.", statusMsg.state().statusText());

            /* Provider send source directory update message to add the DIRECT_FEED2 service */
            OmmArray capablities = EmaFactory.createOmmArray();
            capablities.add(EmaFactory.createOmmArrayEntry().uintValue( EmaRdm.MMT_MARKET_PRICE));
            capablities.add(EmaFactory.createOmmArrayEntry().uintValue( EmaRdm.MMT_MARKET_BY_PRICE));
            capablities.add(EmaFactory.createOmmArrayEntry().uintValue( EmaRdm.MMT_SYMBOL_LIST));
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

            Thread.sleep(5000);

            assertEquals(4, providerClient1.queueSize());

            message = providerClient1.popMessage();

            assertTrue(message instanceof ReqMsg);

            ReqMsg requestMsg = (ReqMsg)message;

            assertEquals("DIRECT_FEED2", requestMsg.serviceName());
            assertEquals(".AV.N", requestMsg.name());
            assertEquals(2, requestMsg.serviceId());

            for (int i = 0; i < 3; i++)
            {
                message = providerClient1.popMessage();
                assertTrue(message instanceof ReqMsg);
                requestMsg = (ReqMsg)message;
                assertEquals("DIRECT_FEED2", requestMsg.serviceName());
                assertTrue("itemA".equals(requestMsg.name()) || "itemB".equals(requestMsg.name()) || "itemC".equals(requestMsg.name()));
                assertEquals(2, requestMsg.serviceId());
            }

            assertEquals(4, consumerClient.queueSize());
            message = consumerClient.popMessage();

            RefreshMsg refreshMsg = (RefreshMsg)message;
            assertEquals("SVG1", refreshMsg.serviceName());
            assertEquals(".AV.N", refreshMsg.name());
            assertEquals(32767, refreshMsg.serviceId());
            assertEquals(OmmState.StreamState.OPEN, refreshMsg.state().streamState());
            assertEquals(OmmState.DataState.OK, refreshMsg.state().dataState());
            assertEquals(OmmState.StatusCode.NONE, refreshMsg.state().code());
            assertTrue(refreshMsg.solicited());

            for (int i = 0; i < 3; i++)
            {
                message = consumerClient.popMessage();
                assertTrue(message instanceof RefreshMsg);
                refreshMsg = (RefreshMsg)message;
                assertEquals("SVG1", refreshMsg.serviceName());
                assertTrue("itemA".equals(refreshMsg.name()) || "itemB".equals(refreshMsg.name()) || "itemC".equals(refreshMsg.name()));
                assertEquals(32767, refreshMsg.serviceId());
            }

            consumer.unregister(itemHandle);
        }
        catch (OmmException excep)
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
    public void testMultiConnectionItemGroupClosedRecoverableWithServiceList()
    {
        TestUtilities.printTestHead("testMultiConnectionItemGroupClosedRecoverableWithServiceList","");

        String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";

        OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);

        ProviderTestOptions providerTestOptions = new ProviderTestOptions();

        ProviderTestClient providerClient1 = new ProviderTestClient(providerTestOptions);

        // Provider_1 provides the DIRECT_FEED service name
        OmmProvider ommprovider = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_1"), providerClient1);

        assertNotNull(ommprovider);

        ProviderTestClient providerClient2 = new ProviderTestClient(providerTestOptions);

        // Provider_1 provides the DIRECT_FEED_2 service name
        OmmProvider ommprovider2 = EmaFactory.createOmmProvider(config.port("19004").providerName("Provider_3"), providerClient2);

        assertNotNull(ommprovider2);

        OmmConsumer consumer = null;
        ConsumerTestClient consumerClient = new ConsumerTestClient();

        try
        {
            ElementList payload = EmaFactory.createElementList();
            ElementEntry entry = EmaFactory.createElementEntry();

            ElementList eePayload = EmaFactory.createElementList();
            ElementEntry eeEntry = EmaFactory.createElementEntry();

            eeEntry.uintValue(":DataStreams", SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_STREAMS);
            eePayload.add(eeEntry);

            entry.elementList(":SymbolListBehaviors", eePayload);
            payload.add(entry);

            ServiceList serviceList = EmaFactory.createServiceList("SVG1");

            serviceList.concreteServiceList().add("DIRECT_FEED");
            serviceList.concreteServiceList().add("DIRECT_FEED_2");

            consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_9").addServiceList(serviceList));

            Thread.sleep(2000);

            consumerClient.clearQueue();
            while (consumerClient.popChannelInfo() != null) {}

            ConsumerSession<OmmConsumerClient> consumerSession = ((OmmConsumerImpl)consumer).consumerSession();
            int serviceId = consumerSession.serviceList("SVG1").serviceId();

            ReqMsg reqMsg = EmaFactory.createReqMsg();

            long itemHandle = consumer.registerClient(reqMsg.clear().serviceListName("SVG1").name(".AV.N").domainType(DomainTypes.SYMBOL_LIST).payload(payload), consumerClient);

            Thread.sleep(3000);

            assertEquals(4, consumerClient.queueSize());
            Msg message = consumerClient.popMessage();

            RefreshMsg refreshMsg = (RefreshMsg)message;
            assertEquals(".AV.N", refreshMsg.name());
            assertEquals(OmmState.StreamState.OPEN, refreshMsg.state().streamState());
            assertEquals(OmmState.DataState.OK, refreshMsg.state().dataState());
            assertEquals(OmmState.StatusCode.NONE, refreshMsg.state().code());
            assertTrue(refreshMsg.solicited());

            for (int i = 0; i < 3; i++)
            {
                message = consumerClient.popMessage();
                assertTrue(message instanceof RefreshMsg);
                refreshMsg = (RefreshMsg)message;
                assertTrue("itemA".equals(refreshMsg.name()) || "itemB".equals(refreshMsg.name()) || "itemC".equals(refreshMsg.name()));
            }

            /* Send item recoverable status from the first provider */
            Long providerItemHandle = providerClient1.retriveItemHandle(".AV.N");

            assertNotNull(providerItemHandle);

            ElementList serviceState = EmaFactory.createElementList();
            serviceState.add( EmaFactory.createElementEntry().uintValue( EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_DOWN ));
            serviceState.add( EmaFactory.createElementEntry().uintValue( EmaRdm.ENAME_ACCEPTING_REQS, 1 ));
            serviceState.add( EmaFactory.createElementEntry().state( EmaRdm.ENAME_STATUS, OmmState.StreamState.OPEN, OmmState.DataState.SUSPECT, OmmState.StatusCode.NONE));

            FilterList filterListEnc = EmaFactory.createFilterList();
            filterListEnc.add( EmaFactory.createFilterEntry().elementList( EmaRdm.SERVICE_STATE_ID, FilterEntry.FilterAction.UPDATE, serviceState ) );

            Map map = EmaFactory.createMap();
            map.add( EmaFactory.createMapEntry().keyUInt( 1, MapEntry.MapAction.UPDATE, filterListEnc ));

            UpdateMsg updateMsg = EmaFactory.createUpdateMsg();
            ommprovider.submit( updateMsg.domainType( EmaRdm.MMT_DIRECTORY ).
                    filter( EmaRdm.SERVICE_STATE_FILTER ).
                    payload( map ), 0);	// use 0 item handle to fan-out to all subscribers


            ommprovider.submit( EmaFactory.createStatusMsg().domainType( EmaRdm.MMT_SYMBOL_LIST )
                            .state(OmmState.StreamState.CLOSED_RECOVER, OmmState.DataState.SUSPECT, OmmState.StatusCode.NONE, "Item temporary closed")
                    , providerItemHandle.longValue());


            Thread.sleep(2000); // Wait until consumer receives the item closed recoverable status message.

            int count = consumerClient.queueSize();

            int closedItemsCount = 0;
            for (int i = 0; i < count; i++)
            {
                message = consumerClient.popMessage();
                if (message instanceof StatusMsg)
                {
                    StatusMsg status = (StatusMsg)message;
                    if (status.state().statusText().contains("This individual item of the Symbol List will be"))
                    {
                        closedItemsCount++;
                    }
                    else if (status.name().equals(".AV.N"))
                    {
                        assertEquals("SVG1", status.serviceName());
                        assertEquals(serviceId, status.serviceId());
                        assertEquals(OmmState.StreamState.OPEN, status.state().streamState());
                        assertEquals(OmmState.DataState.SUSPECT, status.state().dataState());
                        assertEquals(OmmState.StatusCode.NONE, status.state().statusCode());
                    }
                }
                else if (message instanceof RefreshMsg)
                {
                    RefreshMsg refresh = (RefreshMsg)message;
                    assertEquals("SVG1", refreshMsg.serviceName());
                    assertEquals(serviceId, refreshMsg.serviceId());
                    assertEquals(OmmState.StreamState.OPEN, refreshMsg.state().streamState());
                    assertEquals(OmmState.DataState.OK, refreshMsg.state().dataState());
                    assertEquals(OmmState.StatusCode.NONE, refreshMsg.state().code());

                    assertTrue(".AV.N".equals(refreshMsg.name())
                            || "itemA".equals(refreshMsg.name()) || "itemB".equals(refreshMsg.name()) || "itemC".equals(refreshMsg.name()));
                }
            }
            assertEquals(3, closedItemsCount);

            Thread.sleep(2000);

            consumer.unregister(itemHandle);

            Thread.sleep(1000);
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
    public void testMultiConnectionsForSingleItemWithWSBChannelsAndChannelDownReconnectingForLoginBased()
    {
        TestUtilities.printTestHead("testMultiConnectionsForSingleItemWithWSBChannelsAndChannelDownReconnectingForLoginBased","");

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
            consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_15"));

            Thread.sleep(2000);

            consumerClient.clearQueue();
            while (consumerClient.popChannelInfo() != null) {}

            ElementList payload = EmaFactory.createElementList();
            ElementEntry entry = EmaFactory.createElementEntry();

            ElementList eePayload = EmaFactory.createElementList();
            ElementEntry eeEntry = EmaFactory.createElementEntry();

            eeEntry.uintValue(":DataStreams", SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_STREAMS);
            eePayload.add(eeEntry);

            entry.elementList(":SymbolListBehaviors", eePayload);
            payload.add(entry);

            ReqMsg reqMsg = EmaFactory.createReqMsg();

            long itemHandle = consumer.registerClient(reqMsg.name(".AV.N").serviceName("DIRECT_FEED").payload(payload).domainType(EmaRdm.MMT_SYMBOL_LIST), consumerClient);

            Thread.sleep(4000);

            HashSet<String> names = new HashSet<>();
            HashSet<Integer> streamIds = new HashSet<>();

            int count = consumerClient.queueSize();
            for (int i = 0; i < count; i++)
            {
                Msg message = consumerClient.popMessage();
                ChannelInformation chInfo = consumerClient.popChannelInfo();
                if (message instanceof RefreshMsg)
                {
                    RefreshMsg refresh = (RefreshMsg)message;
                    assertTrue("Channel_3".equals(chInfo.channelName()));

                    if (refresh.hasName()
                            && (refresh.name().equals(".AV.N")
                            || refresh.name().equals("itemA")
                            || refresh.name().equals("itemB")
                            || refresh.name().equals("itemC")))
                    {
                        if (refresh.name().equals(".AV.N")) assertEquals(5, refresh.streamId());
                        names.add(refresh.name());
                    }
                }
            }
            assertEquals(4, names.size());

            /* Closes the active server for the first connection */
            System.out.println(" \n >>>> Kill provider 1\n");
            ommprovider1.uninitialize();

            Thread.sleep(4000);

            checkStatusAndRefreshMsgs(consumerClient, names, streamIds, "Channel_3", "Channel_6", null);

            /* Closes the new active server for the first connection */
            System.out.println(" \n>>>>> Kill provider 2");
            ommprovider2.uninitialize();

            Thread.sleep(3000);

            names.clear();
            streamIds.clear();

            checkStatusAndRefreshMsgs(consumerClient, names, streamIds, "Channel_6", "Channel_7", null);

            consumer.unregister(itemHandle);

            Thread.sleep(1000);
        }
        catch (OmmException ex)
        {
            assertFalse(true);
        } catch (InterruptedException e) {

            e.printStackTrace();
        }
        finally {
            System.out.println("Uninitilizing...");

            consumer.uninitialize();
            ommprovider1.uninitialize();
            ommprovider2.uninitialize();
            ommprovider3.uninitialize();
            ommprovider4.uninitialize();
        }
    }

    @Test
    public void testMultiConnectionForItemRecoveryWithPreferredHostWSBForLoginBasedEnabledByFallbackMethod()
    {
        TestUtilities.printTestHead("testMultiConnectionForItemRecoveryWithPreferredHostWSBForLoginBasedEnabledByFallbackMethod","");

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

        /* WarmStandbyChannel_5 (non-preferred WSB channel) */
        // Provider_5 provides the DIRECT_FEED service name for WSB channel(starting server)
        OmmProvider ommprovider_9 = EmaFactory.createOmmProvider(config.port("19009").providerName("Provider_5"), providerClient);
        assertNotNull(ommprovider_9);

        // Provider_5 provides the DIRECT_FEED service name for WSB channel(standby server)
        OmmProvider ommprovider_10 = EmaFactory.createOmmProvider(config.port("19010").providerName("Provider_5"), providerClient2);
        assertNotNull(ommprovider_10);


        /* WarmStandbyChannel_6 (non-preferred WSB channel) */
        // Provider_5 provides the DIRECT_FEED service name for WSB channel(starting server)
        OmmProvider ommprovider_11 = EmaFactory.createOmmProvider(config.port("19011").providerName("Provider_5"), providerClient3);

        assertNotNull(ommprovider_11);

        // Provider_5 provides the DIRECT_FEED service name for WSB channel(standby server)
        OmmProvider ommprovider_12 = EmaFactory.createOmmProvider(config.port("19012").providerName("Provider_5"), providerClient4);
        assertNotNull(ommprovider_12);

        OmmConsumer consumer = null;
        ConsumerTestOptions consumerOption = new ConsumerTestOptions();

        consumerOption.getChannelInformation = true;
        consumerOption.getSessionChannelInfo = false;
        ConsumerTestClient consumerClient = new ConsumerTestClient(consumerOption);

        OmmProvider ommprovider_3 = null;
        OmmProvider ommprovider_6 = null;
        OmmProvider ommprovider_7 = null;
        OmmProvider ommprovider_8 = null;

        try
        {
            ElementList payload = EmaFactory.createElementList();
            ElementEntry entry = EmaFactory.createElementEntry();

            ElementList eePayload = EmaFactory.createElementList();
            ElementEntry eeEntry = EmaFactory.createElementEntry();

            eeEntry.uintValue(":DataStreams", SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_STREAMS);
            eePayload.add(eeEntry);

            entry.elementList(":SymbolListBehaviors", eePayload);
            payload.add(entry);

            consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_18_1"));

            Thread.sleep(2000);

            consumerClient.clearQueue();
            while (consumerClient.popChannelInfo() != null) {}

            String serviceName = "DIRECT_FEED";
            String itemName = ".AV.N";

            ReqMsg reqMsg = EmaFactory.createReqMsg();

            long itemHandle = consumer.registerClient(reqMsg.name(itemName).serviceName(serviceName).domainType(EmaRdm.MMT_SYMBOL_LIST).payload(payload), consumerClient);

            Thread.sleep(4000);

            Thread.sleep(1000);

            /* Checks the item refresh message from the starting server of Connection_13 */
            Msg message = consumerClient.popMessage();
            /* Checks the market price item refresh */
            RefreshMsg refreshMsg = (RefreshMsg)message;

            assertEquals(5, refreshMsg.streamId());
            assertEquals(DomainTypes.SYMBOL_LIST, refreshMsg.domainType());
            assertEquals("Open / Ok / None / 'Refresh Completed'", refreshMsg.state().toString());
            assertTrue(refreshMsg.complete());
            assertTrue(refreshMsg.solicited());
            assertTrue(refreshMsg.hasName());
            assertEquals(itemName, refreshMsg.name());
            assertTrue(refreshMsg.hasServiceId());
            assertEquals(32767, refreshMsg.serviceId());
            assertTrue(refreshMsg.hasServiceName());
            assertEquals(serviceName, refreshMsg.serviceName());

            ChannelInformation channelInfo = consumerClient.popChannelInfo();
            assertEquals("Channel_9", channelInfo.channelName());
            assertEquals("Connection_13_1", channelInfo.sessionChannelName());
            assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());

            for (int i = 0; i < 3; i++)
            {
                message = consumerClient.popMessage();
                refreshMsg = (RefreshMsg)message;

                assertEquals(-1 - i, refreshMsg.streamId());
                assertEquals(DomainTypes.MARKET_PRICE, refreshMsg.domainType());
                assertEquals("Open / Ok / None / 'Refresh Completed'", refreshMsg.state().toString());
                assertTrue(refreshMsg.complete());
                assertTrue(refreshMsg.solicited());
                assertTrue(refreshMsg.hasName());
                assertTrue(refreshMsg.hasServiceId());
                assertEquals(32767, refreshMsg.serviceId());
                assertTrue(refreshMsg.hasServiceName());
                assertEquals(serviceName, refreshMsg.serviceName());

                channelInfo = consumerClient.popChannelInfo();
                assertEquals("Channel_9", channelInfo.channelName());
                assertEquals("Connection_13_1", channelInfo.sessionChannelName());
                assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
            }

            /* WarmStandbyChannel_1 (preferred WSB channel) */
            // Provider_5 provides the DIRECT_FEED service name for WSB channel(starting server)
            ommprovider_3 = EmaFactory.createOmmProvider(config.port("19003").providerName("Provider_5"), providerClient);

            // Provider_5 provides the DIRECT_FEED service name for WSB channel(standby server)
            ommprovider_6 = EmaFactory.createOmmProvider(config.port("19006").providerName("Provider_5"), providerClient2);

            /* WarmStandbyChannel_2 (preferred WSB channel) */
            // Provider_5 provides the DIRECT_FEED service name for WSB channel(starting server)
            ommprovider_7 = EmaFactory.createOmmProvider(config.port("19007").providerName("Provider_5"), providerClient);

            // Provider_5 provides the DIRECT_FEED service name for WSB channel(standby server)
            ommprovider_8 = EmaFactory.createOmmProvider(config.port("19008").providerName("Provider_5"), providerClient2);

            consumer.fallbackPreferredHost();
            Thread.sleep(4000);

            /* Checks the item status message from the starting server of Connection_13 when fallback process is trigger */
            System.out.println("consumerClient.queueSize() " + consumerClient.queueSize() );

            int count = consumerClient.queueSize();
            StatusMsg statusMsg;
            int refreshCount = 0;

            for (int i = 0; i < count; i++)
            {
                message = consumerClient.popMessage();
                if (message instanceof StatusMsg)
                {
                    statusMsg = (StatusMsg)message;
                    if (statusMsg.domainType() == DomainTypes.SYMBOL_LIST)
                    {
                        assertEquals(5, statusMsg.streamId());
                        assertTrue(statusMsg.hasState());
                        assertEquals("Open / Suspect / None / 'channel down.'", statusMsg.state().toString());
                        assertTrue(statusMsg.hasName());
                        assertEquals(".AV.N", statusMsg.name());
                        assertTrue(statusMsg.hasServiceId());
                        assertEquals(32767, statusMsg.serviceId());
                        assertTrue(statusMsg.hasServiceName());
                        assertEquals(serviceName, statusMsg.serviceName());
                        assertEquals(DataType.DataTypes.NO_DATA, statusMsg.payload().dataType());
                    }
                    else if (statusMsg.domainType() == DomainTypes.MARKET_PRICE)
                    {
                        assertTrue(statusMsg.streamId() < 0);
                        assertTrue(statusMsg.hasServiceId());
                        assertEquals(32767, statusMsg.serviceId());
                        assertTrue(statusMsg.hasServiceName());
                        assertEquals(serviceName, statusMsg.serviceName());
                        assertTrue(statusMsg.state().statusText().contains("Individual item from Symbol List closed due to server change"));
                    }

                    channelInfo = consumerClient.popChannelInfo();
                    assertEquals("Channel_9", channelInfo.channelName());
                    assertEquals("Connection_13_1", channelInfo.sessionChannelName());
                    assertEquals(ChannelInformation.ChannelState.INACTIVE, channelInfo.channelState());
                }
                else if (message instanceof RefreshMsg)
                {
                    refreshCount++;

                    refreshMsg = (RefreshMsg)message;
                    assertTrue(refreshMsg.hasServiceId());
                    assertEquals(32767, refreshMsg.serviceId());
                    assertTrue(refreshMsg.hasServiceName());
                    assertEquals(serviceName, refreshMsg.serviceName());

                    channelInfo = consumerClient.popChannelInfo();
                    assertEquals("Channel_3", channelInfo.channelName());
                    assertEquals("Connection_13_1", channelInfo.sessionChannelName());
                    assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
                }
            }
            assertEquals(4, refreshCount);

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
            ommprovider_9.uninitialize();
            ommprovider_10.uninitialize();
            ommprovider_11.uninitialize();
            ommprovider_12.uninitialize();

            if(ommprovider_3 != null)
                ommprovider_3.uninitialize();

            if(ommprovider_6 != null)
                ommprovider_6.uninitialize();

            if(ommprovider_7 != null)
                ommprovider_7.uninitialize();

            if(ommprovider_8 != null)
                ommprovider_8.uninitialize();
        }
    }

    @Test
    public void testMultiConnectionRequestToDifferentServices()
    {
        TestUtilities.printTestHead("testMultiConnectionItemGroupClosedRecoverableWithServiceList","");

        String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";

        OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);

        ProviderTestOptions providerTestOptions = new ProviderTestOptions();

        ProviderTestClient providerClient1 = new ProviderTestClient(providerTestOptions);

        // Provider_1 provides the DIRECT_FEED service name
        OmmProvider ommprovider = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_1"), providerClient1);

        assertNotNull(ommprovider);

        ProviderTestClient providerClient2 = new ProviderTestClient(providerTestOptions);

        // Provider_1 provides the DIRECT_FEED_2 service name
        OmmProvider ommprovider2 = EmaFactory.createOmmProvider(config.port("19004").providerName("Provider_3"), providerClient2);

        assertNotNull(ommprovider2);

        OmmConsumer consumer = null;
        ConsumerTestClient consumerClient = new ConsumerTestClient();

        try
        {
            ElementList payload = EmaFactory.createElementList();
            ElementEntry entry = EmaFactory.createElementEntry();

            ElementList eePayload = EmaFactory.createElementList();
            ElementEntry eeEntry = EmaFactory.createElementEntry();

            eeEntry.uintValue(":DataStreams", SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_STREAMS);
            eePayload.add(eeEntry);

            entry.elementList(":SymbolListBehaviors", eePayload);
            payload.add(entry);

            ServiceList serviceList = EmaFactory.createServiceList("SVG1");

            serviceList.concreteServiceList().add("DIRECT_FEED");
            serviceList.concreteServiceList().add("DIRECT_FEED_2");

            consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_9").addServiceList(serviceList));

            Thread.sleep(2000);

            consumerClient.clearQueue();
            while (consumerClient.popChannelInfo() != null) {}

            ReqMsg reqMsg = EmaFactory.createReqMsg();

            consumer.registerClient(reqMsg.clear().serviceName("DIRECT_FEED").name(".AV.N").domainType(DomainTypes.SYMBOL_LIST).payload(payload), consumerClient);

            Thread.sleep(3000);

            assertEquals(4, consumerClient.queueSize());

            HashSet<Integer> negativeStreamIds = new HashSet<>();
            negativeStreamIds.add(-1);
            negativeStreamIds.add(-2);
            negativeStreamIds.add(-3);

            for (int i = 0; i < 4; i++)
            {
                Msg message = consumerClient.popMessage();
                assertTrue(message instanceof RefreshMsg);
                RefreshMsg refresh = (RefreshMsg)message;

                if (refresh.domainType() == DomainTypes.SYMBOL_LIST) {
                    assertEquals(5, refresh.streamId());
                    assertEquals(".AV.N", refresh.name());
                }
                else if (refresh.domainType() == DomainTypes.MARKET_PRICE) assertTrue(negativeStreamIds.contains(refresh.streamId()));
            }

            Long dfItemAHandle = consumerClient.handleNameMap.get("itemA");
            consumerClient.handleNameMap.remove("itemA");

            ReqMsg reqMsg2 = EmaFactory.createReqMsg();

            long itemHandle2 = consumer.registerClient(reqMsg2.clear().serviceName("DIRECT_FEED_2").name(".AV.N").domainType(DomainTypes.SYMBOL_LIST).payload(payload), consumerClient);

            Thread.sleep(4000);

            negativeStreamIds.clear();
            negativeStreamIds.add(-4);
            negativeStreamIds.add(-5);
            negativeStreamIds.add(-6);

            for (int i = 0; i < 4; i++)
            {
                Msg message = consumerClient.popMessage();
                assertTrue(message instanceof RefreshMsg);
                RefreshMsg refresh = (RefreshMsg)message;

                if (refresh.domainType() == DomainTypes.SYMBOL_LIST) {
                    assertEquals(6, refresh.streamId());
                    assertEquals(".AV.N", refresh.name());
                }
                else if (refresh.domainType() == DomainTypes.MARKET_PRICE) assertTrue(negativeStreamIds.contains(refresh.streamId()));
            }

            Long df2ItemAHandle = consumerClient.handleNameMap.get("itemA");
            consumerClient.handleNameMap.remove("itemA");

            assertNotEquals(dfItemAHandle, df2ItemAHandle);

            consumer.unregister(dfItemAHandle);
            consumer.unregister(df2ItemAHandle);

            Thread.sleep(3000);

            assertEquals(2, ((OmmConsumerImpl) consumer).itemCallbackClient()._providerDrivenItemsByReactorChannel.size());
            ((OmmConsumerImpl) consumer).itemCallbackClient()._providerDrivenItemsByReactorChannel.values().forEach(v -> {
                assertTrue(!v.containsKey(-1));
                assertTrue(v.containsKey(-2));
                assertTrue(v.containsKey(-3));
            });

            Thread.sleep(2000);
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
    public void testSymbolListRecoveryReconnectWithChannelList()
    {
        TestUtilities.printTestHead("testSymbolListRecoveryReconnectWithChannelList","");

        String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";

        OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);

        ProviderTestOptions providerTestOptions = new ProviderTestOptions();

        ProviderTestClient providerClient1 = new ProviderTestClient(providerTestOptions);

        OmmProvider ommprovider_1 = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_1"), providerClient1);

        assertNotNull(ommprovider_1);

        ProviderTestClient providerClient2 = new ProviderTestClient(providerTestOptions);

        OmmProvider ommprovider_2 = EmaFactory.createOmmProvider(config.port("19002").providerName("Provider_1"), providerClient2);

        assertNotNull(ommprovider_2);

        ProviderTestClient providerClient3 = new ProviderTestClient(providerTestOptions);

        OmmProvider ommprovider_3 = EmaFactory.createOmmProvider(config.port("19003").providerName("Provider_1"), providerClient3);

        assertNotNull(ommprovider_3);

        OmmConsumer consumer = null;
        ConsumerTestClient consumerClient = new ConsumerTestClient();

        try
        {
            ElementList payload = EmaFactory.createElementList();
            ElementEntry entry = EmaFactory.createElementEntry();

            ElementList eePayload = EmaFactory.createElementList();
            ElementEntry eeEntry = EmaFactory.createElementEntry();

            eeEntry.uintValue(":DataStreams", SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_STREAMS);
            eePayload.add(eeEntry);

            entry.elementList(":SymbolListBehaviors", eePayload);
            payload.add(entry);

            consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_1_2"));

            Thread.sleep(2000);

            consumerClient.clearQueue();
            while (consumerClient.popChannelInfo() != null) {}

            ReqMsg reqMsg = EmaFactory.createReqMsg();

            consumer.registerClient(reqMsg.clear().serviceName("DIRECT_FEED").name(".AV.N").domainType(DomainTypes.SYMBOL_LIST).payload(payload), consumerClient);

            Thread.sleep(1500);

            HashSet<Integer> negativeStreamIds = new HashSet<>();
            negativeStreamIds.add(-1);
            negativeStreamIds.add(-2);
            negativeStreamIds.add(-3);

            for (int i = 0; i < 4; i++)
            {
                Msg message = consumerClient.popMessage();
                assertTrue(message instanceof RefreshMsg);
                RefreshMsg refresh = (RefreshMsg)message;

                if (refresh.domainType() == DomainTypes.SYMBOL_LIST) {
                    assertEquals(5, refresh.streamId());
                    assertEquals(".AV.N", refresh.name());
                }
                else if (refresh.domainType() == DomainTypes.MARKET_PRICE) assertTrue(negativeStreamIds.contains(refresh.streamId()));
            }

            System.out.println("\n >>> Killing provider_1 \n");

            ommprovider_1.uninitialize();

            Thread.sleep(10000);

            int count = consumerClient.queueSize();
            int statusClosedCount = 0;
            int refreshCount = 0;
            for (int i = 0; i < count; i++)
            {
                Msg message = consumerClient.popMessage();
                if (message instanceof StatusMsg)
                {
                    StatusMsg status = (StatusMsg)message;
                    if (status.state().statusText().contains("Individual item from Symbol List closed due to server change"))
                    {
                        assertTrue(status.streamId() < 0);
                        assertTrue(status.domainType() == DomainTypes.MARKET_PRICE);
                        statusClosedCount++;
                    }
                }
                else if (message instanceof RefreshMsg)
                {
                    RefreshMsg refresh = (RefreshMsg)message;
                    assertTrue(refresh.hasName());
                    assertTrue(refresh.name().equals(".AV.N")
                            || refresh.name().equals("itemA")
                            || refresh.name().equals("itemB")
                            || refresh.name().equals("itemC"));
                    refreshCount++;
                }
            }
            assertEquals(3, statusClosedCount);
            assertEquals(4, refreshCount);

            System.out.println("\n >>> Killing provider_2 \n");

            ommprovider_2.uninitialize();

            Thread.sleep(10000);

            count = consumerClient.queueSize();
            statusClosedCount = 0;
            refreshCount = 0;

            for (int i = 0; i < count; i++)
            {
                Msg message = consumerClient.popMessage();
                if (message instanceof StatusMsg)
                {
                    StatusMsg status = (StatusMsg)message;
                    if (status.state().statusText().contains("Individual item from Symbol List closed due to server change"))
                    {
                        assertTrue(status.streamId() < 0);
                        assertTrue(status.domainType() == DomainTypes.MARKET_PRICE);
                        statusClosedCount++;
                    }
                }
                else if (message instanceof RefreshMsg)
                {
                    RefreshMsg refresh = (RefreshMsg)message;
                    assertTrue(refresh.hasName());
                    assertTrue(refresh.name().equals(".AV.N")
                            || refresh.name().equals("itemA")
                            || refresh.name().equals("itemB")
                            || refresh.name().equals("itemC"));
                    refreshCount++;
                }
            }
            assertEquals(3, statusClosedCount);
            assertEquals(4, refreshCount);

            ommprovider_1 = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_1"), providerClient1);
            ommprovider_2 = EmaFactory.createOmmProvider(config.port("19002").providerName("Provider_1"), providerClient2);

            System.out.println("\n >>> Killing provider_3 \n");

            ommprovider_3.uninitialize();

            Thread.sleep(10000);

            count = consumerClient.queueSize();
            statusClosedCount = 0;
            refreshCount = 0;

            for (int i = 0; i < count; i++)
            {
                Msg message = consumerClient.popMessage();
                if (message instanceof StatusMsg)
                {
                    StatusMsg status = (StatusMsg)message;
                    if (status.state().statusText().contains("Individual item from Symbol List closed due to server change"))
                    {
                        assertTrue(status.streamId() < 0);
                        assertTrue(status.domainType() == DomainTypes.MARKET_PRICE);
                        statusClosedCount++;
                    }
                }
                else if (message instanceof RefreshMsg)
                {
                    RefreshMsg refresh = (RefreshMsg)message;
                    assertTrue(refresh.hasName());
                    assertTrue(refresh.name().equals(".AV.N")
                            || refresh.name().equals("itemA")
                            || refresh.name().equals("itemB")
                            || refresh.name().equals("itemC"));
                    refreshCount++;
                }
            }
            assertEquals(3, statusClosedCount);
            assertEquals(4, refreshCount);
        }
        catch(OmmException excep)
        {
            assertFalse(true);
        }
        catch (InterruptedException e) {
            e.printStackTrace();
        }
        finally {
            System.out.println("Uninitializing...");

            consumer.uninitialize();
            if (ommprovider_1 != null) ommprovider_1.uninitialize();
            if (ommprovider_2 != null) ommprovider_2.uninitialize();
            if (ommprovider_3 != null) ommprovider_3.uninitialize();
        }
    }

    @Test
    public void testSymbolListClearCacheMsgReceived()
    {
        TestUtilities.printTestHead("testSymbolListRecoveryReconnectWithChannelList","");

        String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";

        OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);

        ProviderTestOptions providerTestOptions = new ProviderTestOptions();

        ProviderTestClient providerClient1 = new ProviderTestClient(providerTestOptions);

        OmmProvider ommprovider_1 = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_1"), providerClient1);

        assertNotNull(ommprovider_1);

        OmmProvider ommprovider_2 = null;
        OmmProvider ommprovider_3 = null;

        OmmConsumer consumer = null;
        ConsumerTestClient consumerClient = new ConsumerTestClient();

        try
        {
            ElementList payload = EmaFactory.createElementList();
            ElementEntry entry = EmaFactory.createElementEntry();

            ElementList eePayload = EmaFactory.createElementList();
            ElementEntry eeEntry = EmaFactory.createElementEntry();

            eeEntry.uintValue(":DataStreams", SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_STREAMS);
            eePayload.add(eeEntry);

            entry.elementList(":SymbolListBehaviors", eePayload);
            payload.add(entry);

            consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_1_2"));

            Thread.sleep(2000);

            consumerClient.clearQueue();
            while (consumerClient.popChannelInfo() != null) {}

            ReqMsg reqMsg = EmaFactory.createReqMsg();

            consumer.registerClient(reqMsg.clear().serviceName("DIRECT_FEED").name(".AV.N").domainType(DomainTypes.SYMBOL_LIST).payload(payload), consumerClient);

            Thread.sleep(1500);

            HashSet<Integer> negativeStreamIds = new HashSet<>();
            negativeStreamIds.add(-1);
            negativeStreamIds.add(-2);
            negativeStreamIds.add(-3);

            for (int i = 0; i < 4; i++)
            {
                Msg message = consumerClient.popMessage();
                assertTrue(message instanceof RefreshMsg);
                RefreshMsg refresh = (RefreshMsg)message;

                if (refresh.domainType() == DomainTypes.SYMBOL_LIST) {
                    assertEquals(5, refresh.streamId());
                    assertEquals(".AV.N", refresh.name());
                }
                else if (refresh.domainType() == DomainTypes.MARKET_PRICE) assertTrue(negativeStreamIds.contains(refresh.streamId()));
            }

            StatusMsg refreshMsg = EmaFactory.createStatusMsg();
            ommprovider_1.submit( refreshMsg.domainType(EmaRdm.MMT_DIRECTORY).clearCache(true).state(OmmState.StreamState.OPEN, OmmState.DataState.OK), 0);	// use 0 item handle to fan-out to all subscribers

            Thread.sleep(5000);

            int count = consumerClient.queueSize();
            int statusClosedCount = 0;
            for (int i = 0; i < count; i++)
            {
                Msg message = consumerClient.popMessage();
                if (message instanceof StatusMsg)
                {
                    StatusMsg status = (StatusMsg)message;
                    if (status.state().statusText().contains("Individual item from Symbol List closed"))
                    {
                        assertTrue(status.streamId() < 0);
                        assertTrue(status.domainType() == DomainTypes.MARKET_PRICE);
                        statusClosedCount++;
                    }
                }
            }
            assertEquals(3, statusClosedCount);
        }
        catch(OmmException excep)
        {
            excep.printStackTrace();
            assertFalse(true);
        }
        catch (InterruptedException e) {
            e.printStackTrace();
        }
        finally {
            System.out.println("Uninitializing...");

            consumer.uninitialize();
            if (ommprovider_1 != null) ommprovider_1.uninitialize();
            if (ommprovider_2 != null) ommprovider_2.uninitialize();
            if (ommprovider_3 != null) ommprovider_3.uninitialize();
        }
    }

    @Test
    public void  testReconnectionInChannelSet()
    {
        TestUtilities.printTestHead("testReconnectionInChannelSet","");

        String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";

        OmmConsumer consumer = null;
        ConsumerTestOptions consumerOption = new ConsumerTestOptions();
        ProviderTestOptions providerTestOptions = new ProviderTestOptions();
        providerTestOptions.supportStandby = true;
        providerTestOptions.sendRefreshAttrib = true;
        providerTestOptions.itemGroupId = ByteBuffer.wrap("10".getBytes());

        ProviderTestClient providerClient_1 = new ProviderTestClient(providerTestOptions);
        ProviderTestClient providerClient_2 = new ProviderTestClient(providerTestOptions);
        ProviderTestClient providerClient_3 = new ProviderTestClient(providerTestOptions);

        OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);

        consumerOption.getChannelInformation = true;
        consumerOption.getSessionChannelInfo = false;
        ConsumerTestClient consumerClient = new ConsumerTestClient(consumerOption);


        // Channel_1
        OmmProvider ommprovider_1 = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_1"), providerClient_1);
        assertNotNull(ommprovider_1);

        // Channel_2 /* This is preferred host */
        OmmProvider ommprovider_2 = EmaFactory.createOmmProvider(config.port("19002").providerName("Provider_1"), providerClient_2);
        assertNotNull(ommprovider_2);

        OmmProvider ommprovider_3 = EmaFactory.createOmmProvider(config.port("19003").providerName("Provider_1"), providerClient_3);
        assertNotNull(ommprovider_3);

        try
        {
            ConsumerTestOptions options = new ConsumerTestOptions();
            options.getChannelInformation = true;

            ElementList payload = EmaFactory.createElementList();
            ElementEntry entry = EmaFactory.createElementEntry();

            ElementList eePayload = EmaFactory.createElementList();
            ElementEntry eeEntry = EmaFactory.createElementEntry();

            eeEntry.uintValue(":DataStreams", SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_STREAMS);
            eePayload.add(eeEntry);

            entry.elementList(":SymbolListBehaviors", eePayload);
            payload.add(entry);

            consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_1_3"), consumerClient);

            String serviceName = "DIRECT_FEED";
            String itemName = ".AV.N";

            ReqMsg reqMsg = EmaFactory.createReqMsg();
            long itemHandle = consumer.registerClient(reqMsg.name(itemName).serviceName(serviceName).domainType(DomainTypes.SYMBOL_LIST).payload(payload), consumerClient);

            Thread.sleep(3000);

            consumerClient.clearQueue();
            while (consumerClient.popChannelInfo() != null) {}

            System.out.println(">>>> Killing provider 1");
            ommprovider_1.uninitialize();

            Thread.sleep(6000);

            HashSet<String> itemNames = new HashSet<String>();
            HashSet<Integer> closedStreamIds = new HashSet<Integer>();

            int count = consumerClient.queueSize();
            for (int i = 0; i < count; i++)
            {
                Msg message = consumerClient.popMessage();
                ChannelInformation chInfo = consumerClient.popChannelInfo();

                if (message instanceof RefreshMsg)
                {
                    RefreshMsg refresh = (RefreshMsg)message;
                    if (refresh.hasName()
                            && (refresh.name().equals(".AV.N")
                            || refresh.name().equals("itemA")
                            || refresh.name().equals("itemB")
                            || refresh.name().equals("itemC")))
                    {
                        if (refresh.name().equals(".AV.N")) assertEquals(5, refresh.streamId());
                        itemNames.add(refresh.name());
                    }
                }
                else if (message instanceof StatusMsg)
                {
                    StatusMsg status = (StatusMsg)message;
                    assertTrue(status.hasState());
                    if (status.streamId() < 0)
                    {
                        assertTrue(status.state().statusText().contains("Individual item"));
                        assertEquals(OmmState.StreamState.CLOSED, status.state().streamState());
                        closedStreamIds.add(status.streamId());
                    }
                }
            }
            assertTrue(itemNames.contains("itemA") && itemNames.contains("itemB") && itemNames.contains("itemC"));
            assertTrue(closedStreamIds.contains(-1) && closedStreamIds.contains(-2) && closedStreamIds.contains(-3));
        }
        catch(Exception excep)
        {
            System.out.println(excep);
            assertTrue(false);
        }
        finally
        {
            System.out.println("Uninitializing...");
            if(consumer != null)
                consumer.uninitialize();

            if (ommprovider_1 != null) ommprovider_1.uninitialize();
            if (ommprovider_2 != null) ommprovider_2.uninitialize();
            if (ommprovider_3 != null) ommprovider_3.uninitialize();
        }
    }


    private void checkStatusAndRefreshMsgs(ConsumerTestClient consumerClient,
                                           HashSet<String> itemNames,
                                           HashSet<Integer> itemStreamIds,
                                           String closedChannel,
                                           String upChannel1,
                                           String upChannel2)
    {
        int closedSLStatusCount = 0;
        itemNames.clear();
        itemStreamIds.clear();
        HashSet<Integer> closedSLItemStreams = new HashSet<>();
        int count = consumerClient.queueSize();
        for (int i = 0; i < count; i++)
        {
            Msg message = consumerClient.popMessage();
            ChannelInformation chInfo = consumerClient.popChannelInfo();
            if (message instanceof StatusMsg)
            {
                if (chInfo.channelName().equals(closedChannel))
                {
                    StatusMsg statusMsg = (StatusMsg)message;
                    if (statusMsg.state().statusText().contains("Individual item from Symbol List closed due to server change."))
                    {
                        closedSLStatusCount++;
                        closedSLItemStreams.add(statusMsg.streamId());
                    }
                }
            }
            else if (message instanceof RefreshMsg)
            {
                RefreshMsg refresh = (RefreshMsg)message;
                if (chInfo != null)
                {
                    if (upChannel1 != null && upChannel2 != null)
                        assertTrue(upChannel1.equals(chInfo.channelName()) || upChannel2.equals(chInfo.channelName()));
                    else if (upChannel1 != null)
                        assertTrue(upChannel1.equals(chInfo.channelName()));
                    else if (upChannel2 != null)
                        assertTrue(upChannel2.equals(chInfo.channelName()));
                }

                if (refresh.hasName()
                        && (refresh.name().equals(".AV.N")
                        || refresh.name().equals("itemA")
                        || refresh.name().equals("itemB")
                        || refresh.name().equals("itemC")))
                {
                    if (refresh.name().equals(".AV.N")) assertEquals(5, refresh.streamId());
                    itemNames.add(refresh.name());
                    itemStreamIds.add(refresh.streamId());
                }
            }
        }
        assertEquals(3, closedSLStatusCount);
        assertEquals(3, closedSLItemStreams.size());
        assertEquals(4, itemNames.size());
    }
}