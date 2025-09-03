package com.refinitiv.ema.access;

import com.refinitiv.ema.JUnitConfigVariables;
import com.refinitiv.ema.RetryRule;
import com.refinitiv.ema.access.DataType.DataTypes;
import com.refinitiv.ema.access.unittest.requestrouting.ConsumerTestClient;
import com.refinitiv.ema.access.unittest.requestrouting.ConsumerTestOptions;
import com.refinitiv.ema.access.unittest.requestrouting.ProviderTestClient;
import com.refinitiv.ema.access.unittest.requestrouting.ProviderTestOptions;
import com.refinitiv.ema.rdm.EmaRdm;
import com.refinitiv.ema.unittest.TestUtilities;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.rdm.Login;
import com.refinitiv.eta.rdm.Login.ServerTypes;

import org.junit.After;
import org.junit.Assert;
import org.junit.Rule;
import org.junit.Test;

import java.io.ByteArrayOutputStream;
import java.io.PrintStream;
import java.nio.ByteBuffer;
import java.util.Arrays;
import java.util.HashSet;
import java.util.Iterator;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.atomic.AtomicReference;
import java.util.logging.Logger;
import java.util.logging.SimpleFormatter;
import java.util.logging.StreamHandler;

import static org.junit.Assert.*;

public class MultiConnectionsTests {

    @Rule
    public RetryRule retryRule = new RetryRule(JUnitConfigVariables.TEST_RETRY_COUNT);

    @After
    public void tearDown()
    {
        try { Thread.sleep(JUnitConfigVariables.WAIT_AFTER_TEST); }
        catch (Exception e) { }
    }

    @Test
    public void testMultiConnectionsLoginReqTimeout()
    {
        TestUtilities.printTestHead("testMultiConnectionsLoginReqTimeout","");

        String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";

        ConsumerTestOptions options = new ConsumerTestOptions();

        options.getChannelInformation = true;

        ConsumerTestClient consumerClient = new ConsumerTestClient(options);

        try
        {
            Exception exception = assertThrows(OmmInvalidUsageException.class,  () ->
                    EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_9"), consumerClient));

            assertEquals(4, consumerClient.queueSize()); // Receives status message for login stream state
            assertEquals(4, consumerClient.channelInfoSize());

            Msg message = consumerClient.popMessage();

            StatusMsg statusMsg = (StatusMsg)message;

            assertEquals(1, statusMsg.streamId());
            assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
            assertEquals("Open / Suspect / None / 'session channel down reconnecting'", statusMsg.state().toString());

            ChannelInformation channelInfo = consumerClient.popChannelInfo();
            assertEquals("Channel_1", channelInfo.channelName());
            assertEquals("Connection_1", channelInfo.sessionChannelName());
            assertEquals(ChannelInformation.ChannelState.INITIALIZING, channelInfo.channelState());

            message = consumerClient.popMessage();

            statusMsg = (StatusMsg)message;

            assertEquals(1, statusMsg.streamId());
            assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
            assertEquals("Open / Suspect / None / 'session channel down reconnecting'", statusMsg.state().toString());

            channelInfo = consumerClient.popChannelInfo();
            assertEquals("Channel_4", channelInfo.channelName());
            assertEquals("Connection_2", channelInfo.sessionChannelName());
            assertEquals(ChannelInformation.ChannelState.INITIALIZING, channelInfo.channelState());

            message = consumerClient.popMessage();

            statusMsg = (StatusMsg)message;

            assertEquals(1, statusMsg.streamId());
            assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
            assertEquals("Open / Suspect / None / 'session channel down reconnecting'", statusMsg.state().toString());

            channelInfo = consumerClient.popChannelInfo();
            assertEquals("Channel_2", channelInfo.channelName());
            assertEquals("Connection_1", channelInfo.sessionChannelName());
            assertEquals(ChannelInformation.ChannelState.INITIALIZING, channelInfo.channelState());

            message = consumerClient.popMessage();

            statusMsg = (StatusMsg)message;

            assertEquals(1, statusMsg.streamId());
            assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
            assertEquals("Open / Suspect / None / 'session channel down reconnecting'", statusMsg.state().toString());

            channelInfo = consumerClient.popChannelInfo();
            assertEquals("Channel_5", channelInfo.channelName());
            assertEquals("Connection_2", channelInfo.sessionChannelName());
            assertEquals(ChannelInformation.ChannelState.INITIALIZING, channelInfo.channelState());

            assertEquals("login failed (timed out after waiting 5000 milliseconds) for Connection_1, Connection_2", exception.getLocalizedMessage());
        }
        catch(OmmException ex)
        {
            System.out.println(ex);
            assertFalse(true);
        }
    }

    @Test
    public void testMultiConnectionsCloseAConnection()
    {
        TestUtilities.printTestHead("testMultiConnectionsCloseAConnection","");

        String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";

        OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);

        ProviderTestOptions providerTestOptions = new ProviderTestOptions();

        ProviderTestClient providerClient = new ProviderTestClient(providerTestOptions);

        OmmProvider ommprovider = EmaFactory.createOmmProvider(config.port("19001"), providerClient);

        assertNotNull(ommprovider);

        ConsumerTestOptions options = new ConsumerTestOptions();

        options.getChannelInformation = true;

        ConsumerTestClient consumerClient = new ConsumerTestClient(options);

        OmmConsumer consumer = null;
        try
        {
            Msg message;
            StatusMsg statusMsg;
            RefreshMsg refreshMsg;
            ChannelInformation channelInfo;

            // The Consumer_9 makes two connections.
            consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_9"), consumerClient);

            OmmConsumerImpl ommConsumerImpl = (OmmConsumerImpl)consumer;

            assertEquals(4, consumerClient.queueSize()); // Receives status message for login stream state
            assertEquals(4, consumerClient.channelInfoSize());

            for (int i = 0; i < 4; i++)
            {
                message = consumerClient.popMessage();
                channelInfo = consumerClient.popChannelInfo();

                if (message instanceof StatusMsg)
                {
                    statusMsg = (StatusMsg)message;
                    if (statusMsg.state().toString().equals("Open / Suspect / None / 'session channel up'"))
                    {
                        assertEquals("Channel_1", channelInfo.channelName());
                        assertEquals("Connection_1", channelInfo.sessionChannelName());
                        assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
                    }
                    else if (statusMsg.state().toString().equals("Open / Suspect / None / 'session channel down reconnecting'"))
                    {
                        assertTrue("Channel_5".equals(channelInfo.channelName()) || "Channel_4".equals(channelInfo.channelName()));
                        assertEquals("Connection_2", channelInfo.sessionChannelName());
                        assertEquals(ChannelInformation.ChannelState.INITIALIZING, channelInfo.channelState());
                    }
                    else
                    {
                        assertFalse("Unknown message status found: " + statusMsg.state().toString(),true);
                    }
                }
                else if (message instanceof RefreshMsg)
                {
                    refreshMsg = (RefreshMsg)message;
                    assertEquals(1, refreshMsg.streamId());
                    assertEquals(DomainTypes.LOGIN, refreshMsg.domainType());
                    assertEquals("Open / Ok / None / 'Login accepted'", refreshMsg.state().toString());
                }
                else
                {
                    assertFalse("Unexpected message type encountered: " + message.getClass(),true);
                }
            }

            // Ensure that only one connect left
            assertEquals(1, ommConsumerImpl.consumerSession().sessionChannelList().size());

            assertEquals("Connection_1", ommConsumerImpl.consumerSession().sessionChannelList().get(0).sessionChannelConfig().name);
            assertNotNull(consumer);
        }
        catch (Exception ex)
        {
            System.out.println(ex);
            assertFalse(true);
        }
        finally
        {
            consumer.uninitialize();
            ommprovider.uninitialize();
        }
    }

    @Test
    public void testMultiConnectionsForLoginStreamOk()
    {
        TestUtilities.printTestHead("testMultiConnectionsForLoginStreamOk","");

        String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";

        OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);

        ProviderTestOptions providerTestOptions = new ProviderTestOptions();

        ProviderTestClient providerClient = new ProviderTestClient(providerTestOptions);

        OmmProvider ommprovider = EmaFactory.createOmmProvider(config.port("19001"), providerClient);

        assertNotNull(ommprovider);

        OmmProvider ommprovider2 = EmaFactory.createOmmProvider(config.port("19004"), providerClient);

        assertNotNull(ommprovider2);

        OmmConsumer consumer = null;
        ConsumerTestOptions options = new ConsumerTestOptions();

        options.getChannelInformation = true;

        ConsumerTestClient consumerClient = new ConsumerTestClient(options);

        try
        {
            consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_9"), consumerClient);

            OmmConsumerImpl ommConsumerImpl = (OmmConsumerImpl)consumer;

            assertEquals(3, consumerClient.queueSize()); // Ensure that the callback receives only one login message
            assertEquals(3, consumerClient.channelInfoSize());

            String channelName = ommConsumerImpl.consumerSession().sessionChannelList().get(0).sessionChannelConfig().name;

            String expectedFirstChannelName = "Channel_1";
            String expectedFirstConnectionName = "Connection_1";
            String expectedSecondChannelName = "Channel_4";
            String expectedSecondConnectionName = "Connection_2";

            if (!channelName.equals("Connection_1"))
            {
                expectedFirstChannelName = expectedSecondChannelName;
                expectedFirstConnectionName = expectedSecondConnectionName;
                expectedSecondChannelName = "Channel_1";
                expectedSecondConnectionName = "Connection_1";
            }

            Msg message = consumerClient.popMessage();
            StatusMsg statusMsg = (StatusMsg)message;
            ChannelInformation channelInfo = null;

            for (int i = 0; i < 2; i++)
            {
                if (message instanceof StatusMsg)
                {
                    assertEquals(1, statusMsg.streamId());
                    assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
                    assertEquals("Open / Suspect / None / 'session channel up'", statusMsg.state().toString());
                    channelInfo = consumerClient.popChannelInfo();
                    assertTrue(expectedFirstChannelName.equals(channelInfo.channelName()) || expectedSecondChannelName.equals(channelInfo.channelName()));
                    assertTrue(expectedFirstConnectionName.equals(channelInfo.sessionChannelName())
                            || expectedSecondConnectionName.equals(channelInfo.sessionChannelName()));
                    assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
                }
                else if (message instanceof RefreshMsg)
                {
                    RefreshMsg refreshMsg = (RefreshMsg)message;

                    channelInfo = consumerClient.popChannelInfo();
                    assertEquals(expectedFirstChannelName, channelInfo.channelName());
                    assertEquals(expectedFirstChannelName, channelInfo.sessionChannelName());
                    assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());

                    assertEquals(1, refreshMsg.streamId());
                    assertEquals(DomainTypes.LOGIN, refreshMsg.domainType());
                    assertEquals("Open / Ok / None / 'Login accepted'", refreshMsg.state().toString());
                    assertTrue(refreshMsg.solicited());
                    assertTrue(refreshMsg.complete());
                    assertTrue(refreshMsg.hasMsgKey());
                    assertEquals(DataType.DataTypes.NO_DATA, refreshMsg.payload().dataType());
                    assertEquals(DataType.DataTypes.ELEMENT_LIST, refreshMsg.attrib().dataType());

                    ElementList elementList = refreshMsg.attrib().elementList();

                    for(ElementEntry element : elementList)
                    {
                        switch(element.name())
                        {
                            case EmaRdm.ENAME_SINGLE_OPEN:
                            {
                                assertEquals(1, element.uintValue());
                                break;
                            }
                            case EmaRdm.ENAME_ALLOW_SUSPECT_DATA:
                            {
                                assertEquals(1, element.uintValue());
                                break;
                            }
                            case EmaRdm.ENAME_SUPPORT_BATCH:
                            {
                                assertEquals(1, element.uintValue());
                                break;
                            }
                        }
                    }
                }
                else
                {
                    assertFalse(true);
                }
            }

            assertNotNull(consumer);
        }
        catch(OmmException ex)
        {
            assertFalse(true);
        }
        finally
        {
            consumer.uninitialize();
            ommprovider.uninitialize();
            ommprovider2.uninitialize();
        }
    }

    @Test
    public void testMultiConnectionsForLoginStreamViaRegisterClient()
    {
        TestUtilities.printTestHead("testMultiConnectionsForLoginStreamViaRegisterClient","");

        String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";

        OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);

        ProviderTestOptions providerTestOptions = new ProviderTestOptions();

        ProviderTestClient providerClient = new ProviderTestClient(providerTestOptions);

        OmmProvider ommprovider = EmaFactory.createOmmProvider(config.port("19001"), providerClient);

        assertNotNull(ommprovider);

        OmmProvider ommprovider2 = EmaFactory.createOmmProvider(config.port("19004"), providerClient);

        assertNotNull(ommprovider2);

        OmmConsumer consumer = null;
        ConsumerTestClient consumerClient = new ConsumerTestClient();

        try
        {
            consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_9"));

            ReqMsg reqMsg = EmaFactory.createReqMsg();

            long loginHandle = consumer.registerClient(reqMsg.domainType(EmaRdm.MMT_LOGIN), consumerClient);

            /* Waits until OmmConsumer receives the refresh message */
            while (consumerClient.queueSize() == 0) {}

            assertEquals(1, consumerClient.queueSize()); // Ensure that the callback receives only one login message

            Msg message = consumerClient.popMessage();

            assertTrue(message instanceof RefreshMsg);

            RefreshMsg refreshMsg = (RefreshMsg)message;

            assertEquals(1, refreshMsg.streamId());
            assertEquals(DomainTypes.LOGIN, refreshMsg.domainType());
            assertEquals("Open / Ok / None / 'Login accepted'", refreshMsg.state().toString());
            assertTrue(refreshMsg.solicited());
            assertTrue(refreshMsg.complete());
            assertTrue(refreshMsg.hasMsgKey());
            assertEquals(DataType.DataTypes.NO_DATA, refreshMsg.payload().dataType());
            assertEquals(DataType.DataTypes.ELEMENT_LIST, refreshMsg.attrib().dataType());

            ElementList elementList = refreshMsg.attrib().elementList();

            for(ElementEntry element : elementList)
            {
                switch(element.name())
                {
                    case EmaRdm.ENAME_SINGLE_OPEN:
                    {
                        assertEquals(1, element.uintValue());
                        break;
                    }
                    case EmaRdm.ENAME_ALLOW_SUSPECT_DATA:
                    {
                        assertEquals(1, element.uintValue());
                        break;
                    }
                    case EmaRdm.ENAME_SUPPORT_BATCH:
                    {
                        assertEquals(1, element.uintValue());
                        break;
                    }
                }
            }

            consumer.unregister(loginHandle);

            assertNotNull(consumer);
        }
        catch(OmmException ex)
        {
            assertFalse(true);
        }
        finally
        {
            consumer.uninitialize();
            ommprovider.uninitialize();
            ommprovider2.uninitialize();
        }
    }

    @Test
    public void testMultiConnectionsForLoginStreamViaRegisterClientWithWSBChannel()
    {

        TestUtilities.printTestHead("testMultiConnectionsForLoginStreamViaRegisterClientWithWSBChannel","");

        String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";

        OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);

        ProviderTestOptions providerTestOptions = new ProviderTestOptions();
        providerTestOptions.supportStandby = false;
        providerTestOptions.sendRefreshAttrib = true;

        ProviderTestOptions providerTestOptions2 = new ProviderTestOptions();
        providerTestOptions2.supportStandby = true;
        providerTestOptions2.sendRefreshAttrib = true;

        ProviderTestClient providerClient = new ProviderTestClient(providerTestOptions);
        ProviderTestClient providerClient2 = new ProviderTestClient(providerTestOptions2);

        // Provider_1 provides the DIRECT_FEED service name
        OmmProvider ommprovider = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_1"), providerClient);

        assertNotNull(ommprovider);

        // Provider_5 provides the DIRECT_FEED service name for WSB channel(starting server)
        OmmProvider ommprovider2 = EmaFactory.createOmmProvider(config.port("19003").providerName("Provider_5"), providerClient2);

        assertNotNull(ommprovider2);

        // Provider_5 provides the DIRECT_FEED service name for WSB channel(standby server)
        OmmProvider ommprovider3 = EmaFactory.createOmmProvider(config.port("19006").providerName("Provider_5"), providerClient2);
        assertNotNull(ommprovider3);

        OmmConsumer consumer = null;
        ConsumerTestClient consumerClient = new ConsumerTestClient();

        try
        {
            consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_10"));

            ReqMsg reqMsg = EmaFactory.createReqMsg();

            long loginHandle = consumer.registerClient(reqMsg.domainType(EmaRdm.MMT_LOGIN), consumerClient);

            /* Waits until OmmConsumer receives the refresh message */
            while (consumerClient.queueSize() == 0) {}

            assertEquals(1, consumerClient.queueSize()); // Ensure that the callback receives only one login message

            Msg message = consumerClient.popMessage();

            assertTrue(message instanceof RefreshMsg);

            RefreshMsg refreshMsg = (RefreshMsg)message;

            //assertEquals(1, refreshMsg.streamId());
            assertEquals(DomainTypes.LOGIN, refreshMsg.domainType());
            assertEquals("Open / Ok / None / 'Login accepted'", refreshMsg.state().toString());
            assertTrue(refreshMsg.solicited());
            assertTrue(refreshMsg.complete());
            assertTrue(refreshMsg.hasMsgKey());
            assertEquals(DataType.DataTypes.NO_DATA, refreshMsg.payload().dataType());
            assertEquals(DataType.DataTypes.ELEMENT_LIST, refreshMsg.attrib().dataType());

            ElementList elementList = refreshMsg.attrib().elementList();

            for(ElementEntry element : elementList)
            {
                switch(element.name())
                {
                    case EmaRdm.ENAME_SINGLE_OPEN:
                    {
                        assertEquals(1, element.uintValue());
                        break;
                    }
                    case EmaRdm.ENAME_ALLOW_SUSPECT_DATA:
                    {
                        assertEquals(1, element.uintValue());
                        break;
                    }
                    case EmaRdm.ENAME_SUPPORT_BATCH:
                    {
                        assertEquals(1, element.uintValue());
                        break;
                    }
                }
            }

            Thread.sleep(3000);

            consumer.unregister(loginHandle);

            assertNotNull(consumer);
        }
        catch(OmmException ex)
        {
            assertFalse(true);
        } catch (InterruptedException e) {

            e.printStackTrace();
        }
        finally
        {
            consumer.uninitialize();
            ommprovider.uninitialize();
            ommprovider2.uninitialize();
            ommprovider3.uninitialize();
        }
    }

    @Test
    public void testMultiConnectionsForLoginStreamDenyOnlyOneSession()
    {
        TestUtilities.printTestHead("testMultiConnectionsForLoginStreamDenyOnlyOneSession","");

        String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";

        OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);

        ProviderTestOptions providerTestOptions1 = new ProviderTestOptions();

        providerTestOptions1.acceptLoginRequest = true;

        ProviderTestClient providerClient = new ProviderTestClient(providerTestOptions1);

        OmmProvider ommprovider = EmaFactory.createOmmProvider(config.port("19001"), providerClient);

        assertNotNull(ommprovider);

        ProviderTestOptions providerTestOptions2 = new ProviderTestOptions();

        providerTestOptions2.acceptLoginRequest = false;

        ProviderTestClient providerClient2 = new ProviderTestClient(providerTestOptions2);

        OmmProvider ommprovider2 = EmaFactory.createOmmProvider(config.port("19004"), providerClient2);

        assertNotNull(ommprovider2);

        OmmConsumer consumer = null;
        ConsumerTestClient consumerClient = new ConsumerTestClient();

        try
        {
            consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_9"), consumerClient);

            Thread.sleep(3000);

            assertEquals(4, consumerClient.queueSize()); // Ensure that the callback receives only one login message

            int channelUpStatus = 0;
            int deniedStatus = 0;
            int loginAcceptedStatus = 0;

            for (int i = 0; i < 4; i++)
            {
                Msg message = consumerClient.popMessage();

                if (message instanceof StatusMsg)
                {
                    StatusMsg statusMsg = (StatusMsg)message;
                    if (statusMsg.state().toString().equals("Open / Suspect / None / 'session channel up'"))
                    {
                        assertEquals(1, statusMsg.streamId());
                        assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
                        channelUpStatus++;
                    }
                    else if (statusMsg.state().toString().equals("Open / Suspect / Not entitled / 'Login denied'"))
                    {
                        assertEquals(1, statusMsg.streamId());
                        assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
                        deniedStatus++;
                    }
                    else
                    {
                        assertFalse(true);
                    }
                }
                else if (message instanceof RefreshMsg)
                {
                    RefreshMsg refreshMsg = (RefreshMsg)message;

                    assertEquals(1, refreshMsg.streamId());
                    assertEquals(DomainTypes.LOGIN, refreshMsg.domainType());
                    assertEquals("Open / Ok / None / 'Login accepted'", refreshMsg.state().toString());
                    assertTrue(refreshMsg.solicited());
                    assertTrue(refreshMsg.complete());
                    assertTrue(refreshMsg.hasMsgKey());
                    assertEquals(DataType.DataTypes.NO_DATA, refreshMsg.payload().dataType());
                    assertEquals(DataType.DataTypes.ELEMENT_LIST, refreshMsg.attrib().dataType());

                    ElementList elementList = refreshMsg.attrib().elementList();

                    for(ElementEntry element : elementList)
                    {
                        switch(element.name())
                        {
                            case EmaRdm.ENAME_SINGLE_OPEN:
                            {
                                assertEquals(1, element.uintValue());
                                break;
                            }
                            case EmaRdm.ENAME_ALLOW_SUSPECT_DATA:
                            {
                                assertEquals(1, element.uintValue());
                                break;
                            }
                            case EmaRdm.ENAME_SUPPORT_BATCH:
                            {
                                assertEquals(1, element.uintValue());
                                break;
                            }
                        }
                    }

                    loginAcceptedStatus++;
                }
            }

            assertEquals(1, loginAcceptedStatus);
            assertEquals(1, deniedStatus);
            assertEquals(2, channelUpStatus);

            assertNotNull(consumer);
        }
        catch (OmmException ex)
        {
            Assert.fail(ex.getMessage());

        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        finally
        {
            consumer.uninitialize();
            ommprovider.uninitialize();
            ommprovider2.uninitialize();
        }
    }

    @Test
    public void testMultiConnectionsForLoginStreamRespAfterOmmConsumerCreation()
    {
        TestUtilities.printTestHead("testMultiConnectionsForLoginStreamRespAfterOmmConsumerCreation","");

        String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";

        OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);

        ProviderTestOptions providerTestOptions1 = new ProviderTestOptions();

        providerTestOptions1.acceptLoginRequest = true;

        ProviderTestClient providerClient = new ProviderTestClient(providerTestOptions1);

        OmmProvider ommprovider = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_1"), providerClient);

        assertNotNull(ommprovider);

        ProviderTestOptions providerTestOptions2 = new ProviderTestOptions();

        providerTestOptions2.acceptLoginRequest = true;
        providerTestOptions2.sendLoginResponseInMiliSecond = 2000;

        ProviderTestClient providerClient2 = new ProviderTestClient(providerTestOptions2);

        OmmProvider ommprovider2 = EmaFactory.createOmmProvider(config.port("19004").providerName("Provider_1"), providerClient2);

        assertNotNull(ommprovider2);

        OmmConsumer consumer = null;
        ConsumerTestClient consumerClient = new ConsumerTestClient();

        try
        {
            consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_9"), consumerClient);

            assertEquals(3, consumerClient.queueSize()); // Ensure that the callback receives only one login message

            int statusCount = 0;
            int refreshCount = 0;

            for (int i = 0; i < 3; i++)
            {
                Msg message = consumerClient.popMessage();

                if (message instanceof StatusMsg)
                {
                    StatusMsg statusMsg = (StatusMsg)message;

                    assertEquals(1, statusMsg.streamId());
                    assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
                    assertEquals("Open / Suspect / None / 'session channel up'", statusMsg.state().toString());

                    statusCount++;
                }
                else if (message instanceof RefreshMsg)
                {
                    RefreshMsg refreshMsg = (RefreshMsg)message;

                    assertEquals(1, refreshMsg.streamId());
                    assertEquals(DomainTypes.LOGIN, refreshMsg.domainType());
                    assertEquals("Open / Ok / None / 'Login accepted'", refreshMsg.state().toString());
                    assertTrue(refreshMsg.solicited());
                    assertTrue(refreshMsg.complete());
                    assertTrue(refreshMsg.hasMsgKey());
                    assertEquals(DataType.DataTypes.NO_DATA, refreshMsg.payload().dataType());
                    assertEquals(DataType.DataTypes.ELEMENT_LIST, refreshMsg.attrib().dataType());

                    ElementList elementList = refreshMsg.attrib().elementList();

                    for(ElementEntry element : elementList)
                    {
                        switch(element.name())
                        {
                            case EmaRdm.ENAME_SINGLE_OPEN:
                            {
                                assertEquals(1, element.uintValue());
                                break;
                            }
                            case EmaRdm.ENAME_ALLOW_SUSPECT_DATA:
                            {
                                assertEquals(1, element.uintValue());
                                break;
                            }
                            case EmaRdm.ENAME_SUPPORT_BATCH:
                            {
                                assertEquals(1, element.uintValue());
                                break;
                            }
                        }
                    }

                    refreshCount++;
                }
            }

            assertEquals(2, statusCount);
            assertEquals(1, refreshCount);

            // Wait to receive login response from the second provider.
            Thread.sleep(3000);

            assertEquals(0, consumerClient.queueSize()); // Ensure that there is no more message

            assertNotNull(consumer);
        }
        catch(OmmException ex)
        {
            Assert.fail(ex.getMessage());

        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        finally
        {
            consumer.uninitialize();
            ommprovider.uninitialize();
            ommprovider2.uninitialize();
        }
    }

    @Test
    public void testMultiConnectionsForLoginStreamAndForceLogoutFromProvider()
    {
        String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";

        OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);

        ProviderTestOptions providerTestOptions1 = new ProviderTestOptions();

        providerTestOptions1.acceptLoginRequest = true;

        ProviderTestClient providerClient = new ProviderTestClient(providerTestOptions1);

        // Provider_1 provides the DIRECT_FEED service name
        OmmProvider ommprovider = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_1"), providerClient);

        assertNotNull(ommprovider);

        ProviderTestOptions providerTestOptions2 = new ProviderTestOptions();

        providerTestOptions2.acceptLoginRequest = true;

        ProviderTestClient providerClient2 = new ProviderTestClient(providerTestOptions2);

        // Provider_3 provides the DIRECT_FEED_2 service name
        OmmProvider ommprovider2 = EmaFactory.createOmmProvider(config.port("19004").providerName("Provider_3"), providerClient2);

        assertNotNull(ommprovider2);

        OmmConsumer consumer = null;

        ConsumerTestOptions options = new ConsumerTestOptions();
        options.getChannelInformation = true;

        ConsumerTestClient consumerClient = new ConsumerTestClient(options);

        try
        {
            consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_9"), consumerClient);

            assertEquals(3, consumerClient.queueSize()); // Ensure that the callback receives only one login message
            assertEquals(3, consumerClient.channelInfoSize());

            int statusCount = 0;
            int refreshCount = 0;

            for (int i = 0; i < 3; i++)
            {
                Msg msg = consumerClient.popMessage();

                if (msg instanceof StatusMsg)
                {
                    StatusMsg statusMsg = (StatusMsg)msg;

                    assertEquals(1, statusMsg.streamId());
                    assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
                    assertEquals("Open / Suspect / None / 'session channel up'", statusMsg.state().toString());

                    statusCount++;
                }
                else if (msg instanceof RefreshMsg)
                {
                    RefreshMsg refreshMsg = (RefreshMsg)msg;

                    assertEquals(1, refreshMsg.streamId());
                    assertEquals(DomainTypes.LOGIN, refreshMsg.domainType());
                    assertEquals("Open / Ok / None / 'Login accepted'", refreshMsg.state().toString());
                    assertTrue(refreshMsg.solicited());
                    assertTrue(refreshMsg.complete());
                    assertTrue(refreshMsg.hasMsgKey());
                    assertEquals(DataType.DataTypes.NO_DATA, refreshMsg.payload().dataType());
                    assertEquals(DataType.DataTypes.ELEMENT_LIST, refreshMsg.attrib().dataType());

                    ElementList elementList = refreshMsg.attrib().elementList();

                    for (ElementEntry element : elementList)
                    {
                        switch(element.name())
                        {
                            case EmaRdm.ENAME_SINGLE_OPEN:
                            {
                                assertEquals(1, element.uintValue());
                                break;
                            }
                            case EmaRdm.ENAME_ALLOW_SUSPECT_DATA:
                            {
                                assertEquals(1, element.uintValue());
                                break;
                            }
                            case EmaRdm.ENAME_SUPPORT_BATCH:
                            {
                                assertEquals(1, element.uintValue());
                                break;
                            }
                        }
                    }

                    refreshCount++;
                }
            }

            assertEquals(2, statusCount);
            assertEquals(1, refreshCount);

            // Wait to receive login response from the second provider.
            Thread.sleep(3000);

            providerClient.forceLogout();

            providerClient2.forceLogout();

            Thread.sleep(2000);

            assertEquals(2, consumerClient.queueSize());

            Msg message = consumerClient.popMessage();

            StatusMsg statusMsg = (StatusMsg)message;

            assertEquals(1, statusMsg.streamId());
            assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
            assertEquals("Open / Ok / Not entitled / 'Force logout'", statusMsg.state().toString());

            message = consumerClient.popMessage();

            statusMsg = (StatusMsg)message;

            assertEquals(1, statusMsg.streamId());
            assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
            assertEquals("Closed / Suspect / Not entitled / 'Force logout'", statusMsg.state().toString());

            assertNotNull(consumer);
        }
        catch (OmmException ex)
        {
            Assert.fail(ex.getMessage());

        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        finally
        {
            consumer.uninitialize();
            ommprovider.uninitialize();
            ommprovider2.uninitialize();
        }
    }

    @Test
    public void testMultiConnectionsForDirectoryStreamWithSameServiceNameButDiffrentQoS()
    {
        TestUtilities.printTestHead("testMultiConnectionsForDirectoryStreamWithSameServiceNameButDiffrentQoS","");

        String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";

        OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);

        /* Source directory refresh for the first server */
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

        Map map = EmaFactory.createMap();
        map.add( EmaFactory.createMapEntry().keyUInt(1, MapEntry.MapAction.ADD, filterList));

        ProviderTestOptions providerTestOptions = new ProviderTestOptions();

        ProviderTestClient providerClient = new ProviderTestClient(providerTestOptions);
        providerTestOptions.sourceDirectoryPayload = map;


        OmmProvider ommprovider = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_1")
                .adminControlDirectory(OmmIProviderConfig.AdminControl.USER_CONTROL), providerClient);

        assertNotNull(ommprovider);

        /* Source directory refresh for the second server with different QoS*/
        qosList.clear();
        qosList.add(EmaFactory.createOmmArrayEntry().qos(500, 500));

        serviceInfoId.clear();
        serviceInfoId.add( EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_NAME, "DIRECT_FEED"));
        serviceInfoId.add( EmaFactory.createElementEntry().array(EmaRdm.ENAME_CAPABILITIES, capablities));
        serviceInfoId.add( EmaFactory.createElementEntry().array(EmaRdm.ENAME_DICTIONARYS_USED, dictionaryUsed));
        serviceInfoId.add( EmaFactory.createElementEntry().array(EmaRdm.ENAME_QOS, qosList));
        serviceInfoId.add( EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_ITEM_LIST, "#.itemlist"));
        serviceInfoId.add( EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_SUPPS_QOS_RANGE, 0));

        filterList.clear();
        filterList.add( EmaFactory.createFilterEntry().elementList(EmaRdm.SERVICE_INFO_ID, FilterEntry.FilterAction.SET, serviceInfoId) );
        filterList.add( EmaFactory.createFilterEntry().elementList(EmaRdm.SERVICE_STATE_ID, FilterEntry.FilterAction.SET, serviceStateId));

        Map map2 = EmaFactory.createMap();
        map2.add( EmaFactory.createMapEntry().keyUInt(1, MapEntry.MapAction.ADD, filterList));

        ProviderTestOptions providerTestOptions2 = new ProviderTestOptions();

        providerTestOptions2.sourceDirectoryPayload = map2;

        /* The second provider doesn't provide the source directory refresh within the DirectoryRequestTimeout value */
        ProviderTestClient providerClient2 = new ProviderTestClient(providerTestOptions2);

        OmmProvider ommprovider2 = EmaFactory.createOmmProvider(config.port("19004").providerName("Provider_1")
                .adminControlDirectory(OmmIProviderConfig.AdminControl.USER_CONTROL), providerClient2);

        assertNotNull(ommprovider2);

        OmmConsumer consumer = null;

        ConsumerTestOptions consumerTestOptions = new ConsumerTestOptions();

        consumerTestOptions.getChannelInformation = false;

        ConsumerTestClient consumerClient = new ConsumerTestClient(consumerTestOptions);

        try
        {
            consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_9"), consumerClient);

            assertEquals(4, consumerClient.queueSize()); // Ensure that the callback receives only one login message

            int statusCount = 0;
            int refreshCount = 0;

            for (int i = 0; i < 3; i++)
            {
                Msg msg = consumerClient.popMessage();

                if (msg instanceof StatusMsg)
                {
                    StatusMsg statusMsg = (StatusMsg)msg;

                    assertEquals(1, statusMsg.streamId());
                    assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
                    assertEquals("Open / Suspect / None / 'session channel up'", statusMsg.state().toString());

                    statusCount++;
                }
                else if (msg instanceof RefreshMsg)
                {
                    RefreshMsg refreshMsg = (RefreshMsg)msg;

                    assertEquals(1, refreshMsg.streamId());
                    assertEquals(DomainTypes.LOGIN, refreshMsg.domainType());
                    assertEquals("Open / Ok / None / 'Login accepted'", refreshMsg.state().toString());
                    assertTrue(refreshMsg.solicited());
                    assertTrue(refreshMsg.complete());
                    assertTrue(refreshMsg.hasMsgKey());
                    assertEquals(DataType.DataTypes.NO_DATA, refreshMsg.payload().dataType());
                    assertEquals(DataType.DataTypes.ELEMENT_LIST, refreshMsg.attrib().dataType());

                    ElementList elementList = refreshMsg.attrib().elementList();

                    for (ElementEntry element : elementList)
                    {
                        switch(element.name())
                        {
                            case EmaRdm.ENAME_SINGLE_OPEN:
                            {
                                assertEquals(1, element.uintValue());
                                break;
                            }
                            case EmaRdm.ENAME_ALLOW_SUSPECT_DATA:
                            {
                                assertEquals(1, element.uintValue());
                                break;
                            }
                            case EmaRdm.ENAME_SUPPORT_BATCH:
                            {
                                assertEquals(1, element.uintValue());
                                break;
                            }
                        }
                    }

                    refreshCount++;
                }
            }

            assertEquals(2, statusCount);
            assertEquals(1, refreshCount);

            ReqMsg reqMsg = EmaFactory.createReqMsg();

            long directoryHandle = consumer.registerClient(reqMsg.domainType(EmaRdm.MMT_DIRECTORY), consumerClient);

            Thread.sleep(2000);

            assertEquals(2, consumerClient.queueSize());

            Msg message = consumerClient.popMessage();

            StatusMsg statusMsg = (StatusMsg)message;

            assertEquals(1, statusMsg.streamId());
            assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
            assertEquals("Open / Ok / None / 'session channel closed'", statusMsg.state().toString());

            message = consumerClient.popMessage();

            assertTrue(message instanceof RefreshMsg);

            RefreshMsg refreshMsg = (RefreshMsg)message;

            assertEquals(2, refreshMsg.streamId());
            assertEquals(DomainTypes.SOURCE, refreshMsg.domainType());
            assertEquals("Open / Ok / None / ''", refreshMsg.state().toString());
            assertTrue(refreshMsg.solicited());
            assertTrue(refreshMsg.complete());
            assertTrue(refreshMsg.hasMsgKey());
            assertEquals(63, refreshMsg.filter());
            assertEquals(DataType.DataTypes.MAP, refreshMsg.payload().dataType());

            Map payload = refreshMsg.payload().map();

            assertEquals(1, payload.size());

            Iterator<MapEntry> mapIt =  payload.iterator();

            assertTrue(mapIt.hasNext());

            MapEntry mapEntry = mapIt.next();

            long serviceId = 32767;

            assertEquals(serviceId, mapEntry.key().uintValue());

            assertEquals(DataType.DataTypes.FILTER_LIST, mapEntry.loadType());

            consumer.unregister(directoryHandle);

            assertNotNull(consumer);
        }
        catch(OmmException ex)
        {
            assertFalse(true);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        finally
        {
            System.out.println("Uninitializing...");

            consumer.uninitialize();
            ommprovider.uninitialize();
            ommprovider2.uninitialize();
        }
    }

    @Test
    public void testMultiConnectionsForDirectoryStreamViaRegisterClientSameService()
    {
        TestUtilities.printTestHead("testMultiConnectionsForDirectoryStreamViaRegisterClientSameService","");

        String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";

        OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);

        ProviderTestOptions providerTestOptions = new ProviderTestOptions();

        ProviderTestClient providerClient = new ProviderTestClient(providerTestOptions);

        // Provider_1 provides the DIRECT_FEED service name
        OmmProvider ommprovider = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_1"), providerClient);

        assertNotNull(ommprovider);

        // Provider_1 provides the DIRECT_FEED service name
        OmmProvider ommprovider2 = EmaFactory.createOmmProvider(config.port("19004").providerName("Provider_1"), providerClient);

        assertNotNull(ommprovider2);

        OmmConsumer consumer = null;

        ConsumerTestOptions consumerTestOptions = new ConsumerTestOptions();

        consumerTestOptions.getChannelInformation = false;

        ConsumerTestClient consumerClient = new ConsumerTestClient(consumerTestOptions);

        try
        {
            consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_9"));

            ReqMsg reqMsg = EmaFactory.createReqMsg();

            long directoryHandle = consumer.registerClient(reqMsg.domainType(EmaRdm.MMT_DIRECTORY), consumerClient);

            /* Waits until OmmConsumer receives the refresh message */
            while(consumerClient.queueSize() == 0) {}

            assertEquals(1, consumerClient.queueSize()); // Ensure that the callback receives only one directory message

            Msg message = consumerClient.popMessage();

            assertTrue(message instanceof RefreshMsg);

            RefreshMsg refreshMsg = (RefreshMsg)message;

            assertEquals(2, refreshMsg.streamId());
            assertEquals(DomainTypes.SOURCE, refreshMsg.domainType());
            assertEquals("Open / Ok / None / ''", refreshMsg.state().toString());
            assertTrue(refreshMsg.solicited());
            assertTrue(refreshMsg.complete());
            assertTrue(refreshMsg.hasMsgKey());
            assertEquals(63, refreshMsg.filter());
            assertEquals(DataType.DataTypes.MAP, refreshMsg.payload().dataType());

            Map payload = refreshMsg.payload().map();

            assertEquals(1, payload.size());

            Iterator<MapEntry> mapIt =  payload.iterator();

            assertTrue(mapIt.hasNext());

            MapEntry mapEntry = mapIt.next();

            long serviceId = 32767;

            assertEquals(serviceId, mapEntry.key().uintValue());

            assertEquals(DataType.DataTypes.FILTER_LIST, mapEntry.loadType());

            FilterList filterList = mapEntry.filterList();

            Iterator<FilterEntry> filterIt =  filterList.iterator();

            assertTrue(filterIt.hasNext());

            FilterEntry filterEntry = filterIt.next();

            assertEquals(FilterEntry.FilterAction.SET, filterEntry.action());
            assertEquals(EmaRdm.SERVICE_INFO_FILTER, filterEntry.filterId());
            assertEquals(DataType.DataTypes.ELEMENT_LIST, filterEntry.loadType());

            // Checks INFO filter
            ElementList elementList = filterEntry.elementList();

            Iterator<ElementEntry> elIt = elementList.iterator();
            assertTrue(elIt.hasNext());
            ElementEntry elementEntry = elIt.next();

            assertEquals(EmaRdm.ENAME_NAME, elementEntry.name());
            assertEquals("DIRECT_FEED", elementEntry.ascii().ascii());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_VENDOR, elementEntry.name());
            assertEquals("company name", elementEntry.ascii().ascii());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_IS_SOURCE, elementEntry.name());
            assertEquals(0, elementEntry.uintValue());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_CAPABILITIES, elementEntry.name());
            assertEquals(DataType.DataTypes.ARRAY, elementEntry.loadType());
            OmmArray ommArray = elementEntry.array();
            Iterator<OmmArrayEntry> arrayIt = ommArray.iterator();
            assertTrue(arrayIt.hasNext());
            OmmArrayEntry arrayEntry = arrayIt.next();

            assertEquals(DataType.DataTypes.UINT, arrayEntry.loadType());
            assertEquals(6, arrayEntry.uintValue());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals(7, arrayEntry.uintValue());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals(8, arrayEntry.uintValue());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals(5, arrayEntry.uintValue());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals(10, arrayEntry.uintValue());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals(200, arrayEntry.uintValue());
            assertFalse(arrayIt.hasNext());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_DICTIONARYS_PROVIDED, elementEntry.name());
            assertEquals(DataType.DataTypes.ARRAY, elementEntry.loadType());
            ommArray = elementEntry.array();
            arrayIt = ommArray.iterator();
            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();

            assertEquals(DataType.DataTypes.ASCII, arrayEntry.loadType());
            assertEquals("RWFFld", arrayEntry.ascii().ascii());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals("RWFEnum", arrayEntry.ascii().ascii());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_DICTIONARYS_USED, elementEntry.name());
            assertEquals(DataType.DataTypes.ARRAY, elementEntry.loadType());
            ommArray = elementEntry.array();
            arrayIt = ommArray.iterator();
            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();

            assertEquals(DataType.DataTypes.ASCII, arrayEntry.loadType());
            assertEquals("RWFFld", arrayEntry.ascii().ascii());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals("RWFEnum", arrayEntry.ascii().ascii());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_QOS, elementEntry.name());
            assertEquals(DataType.DataTypes.ARRAY, elementEntry.loadType());
            ommArray = elementEntry.array();
            arrayIt = ommArray.iterator();
            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();

            assertEquals(DataType.DataTypes.QOS, arrayEntry.loadType());
            assertEquals("RealTime/TickByTick", arrayEntry.qos().toString());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals("Timeliness: 100/Rate: 100", arrayEntry.qos().toString());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_SUPPS_QOS_RANGE, elementEntry.name());
            assertEquals(DataType.DataTypes.UINT, elementEntry.loadType());
            assertEquals(0, elementEntry.uintValue());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_ITEM_LIST, elementEntry.name());
            assertEquals(DataType.DataTypes.ASCII, elementEntry.loadType());
            assertEquals("#.itemlist", elementEntry.ascii().ascii());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_SUPPS_OOB_SNAPSHOTS, elementEntry.name());
            assertEquals(DataType.DataTypes.UINT, elementEntry.loadType());
            assertEquals(0, elementEntry.uintValue());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_ACCEPTING_CONS_STATUS, elementEntry.name());
            assertEquals(DataType.DataTypes.UINT, elementEntry.loadType());
            assertEquals(0, elementEntry.uintValue());

            assertFalse(elIt.hasNext());

            assertTrue(filterIt.hasNext());

            filterEntry = filterIt.next();

            assertEquals(FilterEntry.FilterAction.SET, filterEntry.action());
            assertEquals(EmaRdm.SERVICE_STATE_FILTER, filterEntry.filterId());
            assertEquals(DataType.DataTypes.ELEMENT_LIST, filterEntry.loadType());

            elementList = filterEntry.elementList();

            elIt = elementList.iterator();
            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();

            assertEquals(EmaRdm.ENAME_SVC_STATE, elementEntry.name());
            assertEquals(1, elementEntry.uintValue());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_ACCEPTING_REQS, elementEntry.name());
            assertEquals(1, elementEntry.uintValue());

            assertFalse(filterIt.hasNext());

            assertFalse(mapIt.hasNext());

            consumer.unregister(directoryHandle);

            assertNotNull(consumer);
        }
        catch(OmmException ex)
        {
            assertFalse(true);
        }
        finally
        {
            System.out.println("Uninitializing...");

            consumer.uninitialize();
            ommprovider.uninitialize();
            ommprovider2.uninitialize();
        }
    }

    @Test
    public void testMultiConnectionsForDirectoryStreamViaRegisterClientSameServiceWithRequestFilters()
    {
        TestUtilities.printTestHead("testMultiConnectionsForDirectoryStreamViaRegisterClientSameServiceWithRequestFilters","");

        String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";

        OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);

        ProviderTestOptions providerTestOptions = new ProviderTestOptions();

        ProviderTestClient providerClient = new ProviderTestClient(providerTestOptions);

        // Provider_1 provides the DIRECT_FEED service name
        OmmProvider ommprovider = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_1"), providerClient);

        assertNotNull(ommprovider);

        // Provider_1 provides the DIRECT_FEED service name
        OmmProvider ommprovider2 = EmaFactory.createOmmProvider(config.port("19004").providerName("Provider_1"), providerClient);

        assertNotNull(ommprovider2);

        OmmConsumer consumer = null;
        ConsumerTestClient consumerClient = new ConsumerTestClient();

        try
        {
            consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_9"));

            ReqMsg reqMsg = EmaFactory.createReqMsg();

            // Request info filter only
            long directoryHandle = consumer.registerClient(reqMsg.domainType(EmaRdm.MMT_DIRECTORY).filter(1), consumerClient);

            /* Waits until OmmConsumer receives the refresh message */
            while(consumerClient.queueSize() == 0) {}

            assertEquals(1, consumerClient.queueSize()); // Ensure that the callback receives only one directory message

            Msg message = consumerClient.popMessage();

            assertTrue(message instanceof RefreshMsg);

            RefreshMsg refreshMsg = (RefreshMsg)message;

            assertEquals(2, refreshMsg.streamId());
            assertEquals(DomainTypes.SOURCE, refreshMsg.domainType());
            assertEquals("Open / Ok / None / ''", refreshMsg.state().toString());
            assertTrue(refreshMsg.solicited());
            assertTrue(refreshMsg.complete());
            assertTrue(refreshMsg.hasMsgKey());
            assertEquals(1, refreshMsg.filter());
            assertEquals(DataType.DataTypes.MAP, refreshMsg.payload().dataType());

            Map payload = refreshMsg.payload().map();

            assertEquals(1, payload.size());

            Iterator<MapEntry> mapIt =  payload.iterator();

            assertTrue(mapIt.hasNext());

            MapEntry mapEntry = mapIt.next();

            long serviceId = 32767;

            assertEquals(serviceId, mapEntry.key().uintValue());

            assertEquals(DataType.DataTypes.FILTER_LIST, mapEntry.loadType());

            FilterList filterList = mapEntry.filterList();

            Iterator<FilterEntry> filterIt =  filterList.iterator();

            assertTrue(filterIt.hasNext());

            FilterEntry filterEntry = filterIt.next();

            assertEquals(FilterEntry.FilterAction.SET, filterEntry.action());
            assertEquals(EmaRdm.SERVICE_INFO_FILTER, filterEntry.filterId());
            assertEquals(DataType.DataTypes.ELEMENT_LIST, filterEntry.loadType());

            // Checks INFO filter
            ElementList elementList = filterEntry.elementList();

            Iterator<ElementEntry> elIt = elementList.iterator();
            assertTrue(elIt.hasNext());
            ElementEntry elementEntry = elIt.next();

            assertEquals(EmaRdm.ENAME_NAME, elementEntry.name());
            assertEquals("DIRECT_FEED", elementEntry.ascii().ascii());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_VENDOR, elementEntry.name());
            assertEquals("company name", elementEntry.ascii().ascii());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_IS_SOURCE, elementEntry.name());
            assertEquals(0, elementEntry.uintValue());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_CAPABILITIES, elementEntry.name());
            assertEquals(DataType.DataTypes.ARRAY, elementEntry.loadType());
            OmmArray ommArray = elementEntry.array();
            Iterator<OmmArrayEntry> arrayIt = ommArray.iterator();
            assertTrue(arrayIt.hasNext());
            OmmArrayEntry arrayEntry = arrayIt.next();

            assertEquals(DataType.DataTypes.UINT, arrayEntry.loadType());
            assertEquals(6, arrayEntry.uintValue());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals(7, arrayEntry.uintValue());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals(8, arrayEntry.uintValue());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals(5, arrayEntry.uintValue());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals(10, arrayEntry.uintValue());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals(200, arrayEntry.uintValue());
            assertFalse(arrayIt.hasNext());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_DICTIONARYS_PROVIDED, elementEntry.name());
            assertEquals(DataType.DataTypes.ARRAY, elementEntry.loadType());
            ommArray = elementEntry.array();
            arrayIt = ommArray.iterator();
            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();

            assertEquals(DataType.DataTypes.ASCII, arrayEntry.loadType());
            assertEquals("RWFFld", arrayEntry.ascii().ascii());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals("RWFEnum", arrayEntry.ascii().ascii());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_DICTIONARYS_USED, elementEntry.name());
            assertEquals(DataType.DataTypes.ARRAY, elementEntry.loadType());
            ommArray = elementEntry.array();
            arrayIt = ommArray.iterator();
            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();

            assertEquals(DataType.DataTypes.ASCII, arrayEntry.loadType());
            assertEquals("RWFFld", arrayEntry.ascii().ascii());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals("RWFEnum", arrayEntry.ascii().ascii());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_QOS, elementEntry.name());
            assertEquals(DataType.DataTypes.ARRAY, elementEntry.loadType());
            ommArray = elementEntry.array();
            arrayIt = ommArray.iterator();
            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();

            assertEquals(DataType.DataTypes.QOS, arrayEntry.loadType());
            assertEquals("RealTime/TickByTick", arrayEntry.qos().toString());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals("Timeliness: 100/Rate: 100", arrayEntry.qos().toString());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_SUPPS_QOS_RANGE, elementEntry.name());
            assertEquals(DataType.DataTypes.UINT, elementEntry.loadType());
            assertEquals(0, elementEntry.uintValue());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_ITEM_LIST, elementEntry.name());
            assertEquals(DataType.DataTypes.ASCII, elementEntry.loadType());
            assertEquals("#.itemlist", elementEntry.ascii().ascii());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_SUPPS_OOB_SNAPSHOTS, elementEntry.name());
            assertEquals(DataType.DataTypes.UINT, elementEntry.loadType());
            assertEquals(0, elementEntry.uintValue());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_ACCEPTING_CONS_STATUS, elementEntry.name());
            assertEquals(DataType.DataTypes.UINT, elementEntry.loadType());
            assertEquals(0, elementEntry.uintValue());

            assertFalse(elIt.hasNext());

            assertFalse(filterIt.hasNext());

            long directoryHandle2 = consumer.registerClient(reqMsg.domainType(EmaRdm.MMT_DIRECTORY).filter(3), consumerClient);

            /* Waits until OmmConsumer receives the refresh message */
            while(consumerClient.queueSize() == 0) {}

            assertEquals(1, consumerClient.queueSize()); // Ensure that the callback receives only one directory message

            message = consumerClient.popMessage();

            assertTrue(message instanceof RefreshMsg);

            refreshMsg = (RefreshMsg)message;

            assertEquals(2, refreshMsg.streamId());
            assertEquals(DomainTypes.SOURCE, refreshMsg.domainType());
            assertEquals("Open / Ok / None / ''", refreshMsg.state().toString());
            assertTrue(refreshMsg.solicited());
            assertTrue(refreshMsg.complete());
            assertTrue(refreshMsg.hasMsgKey());
            assertEquals(3, refreshMsg.filter());
            assertEquals(DataType.DataTypes.MAP, refreshMsg.payload().dataType());

            payload = refreshMsg.payload().map();

            assertEquals(1, payload.size());

            mapIt =  payload.iterator();

            assertTrue(mapIt.hasNext());

            mapEntry = mapIt.next();

            serviceId = 32767;

            assertEquals(serviceId, mapEntry.key().uintValue());

            assertEquals(DataType.DataTypes.FILTER_LIST, mapEntry.loadType());

            filterList = mapEntry.filterList();

            filterIt =  filterList.iterator();

            assertTrue(filterIt.hasNext());

            filterEntry = filterIt.next();

            assertEquals(FilterEntry.FilterAction.SET, filterEntry.action());
            assertEquals(EmaRdm.SERVICE_INFO_FILTER, filterEntry.filterId());
            assertEquals(DataType.DataTypes.ELEMENT_LIST, filterEntry.loadType());

            // Checks INFO filter
            elementList = filterEntry.elementList();

            elIt = elementList.iterator();
            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();

            assertEquals(EmaRdm.ENAME_NAME, elementEntry.name());
            assertEquals("DIRECT_FEED", elementEntry.ascii().ascii());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_VENDOR, elementEntry.name());
            assertEquals("company name", elementEntry.ascii().ascii());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_IS_SOURCE, elementEntry.name());
            assertEquals(0, elementEntry.uintValue());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_CAPABILITIES, elementEntry.name());
            assertEquals(DataType.DataTypes.ARRAY, elementEntry.loadType());
            ommArray = elementEntry.array();
            arrayIt = ommArray.iterator();
            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();

            assertEquals(DataType.DataTypes.UINT, arrayEntry.loadType());
            assertEquals(6, arrayEntry.uintValue());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals(7, arrayEntry.uintValue());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals(8, arrayEntry.uintValue());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals(5, arrayEntry.uintValue());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals(10, arrayEntry.uintValue());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals(200, arrayEntry.uintValue());
            assertFalse(arrayIt.hasNext());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_DICTIONARYS_PROVIDED, elementEntry.name());
            assertEquals(DataType.DataTypes.ARRAY, elementEntry.loadType());
            ommArray = elementEntry.array();
            arrayIt = ommArray.iterator();
            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();

            assertEquals(DataType.DataTypes.ASCII, arrayEntry.loadType());
            assertEquals("RWFFld", arrayEntry.ascii().ascii());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals("RWFEnum", arrayEntry.ascii().ascii());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_DICTIONARYS_USED, elementEntry.name());
            assertEquals(DataType.DataTypes.ARRAY, elementEntry.loadType());
            ommArray = elementEntry.array();
            arrayIt = ommArray.iterator();
            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();

            assertEquals(DataType.DataTypes.ASCII, arrayEntry.loadType());
            assertEquals("RWFFld", arrayEntry.ascii().ascii());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals("RWFEnum", arrayEntry.ascii().ascii());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_QOS, elementEntry.name());
            assertEquals(DataType.DataTypes.ARRAY, elementEntry.loadType());
            ommArray = elementEntry.array();
            arrayIt = ommArray.iterator();
            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();

            assertEquals(DataType.DataTypes.QOS, arrayEntry.loadType());
            assertEquals("RealTime/TickByTick", arrayEntry.qos().toString());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals("Timeliness: 100/Rate: 100", arrayEntry.qos().toString());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_SUPPS_QOS_RANGE, elementEntry.name());
            assertEquals(DataType.DataTypes.UINT, elementEntry.loadType());
            assertEquals(0, elementEntry.uintValue());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_ITEM_LIST, elementEntry.name());
            assertEquals(DataType.DataTypes.ASCII, elementEntry.loadType());
            assertEquals("#.itemlist", elementEntry.ascii().ascii());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_SUPPS_OOB_SNAPSHOTS, elementEntry.name());
            assertEquals(DataType.DataTypes.UINT, elementEntry.loadType());
            assertEquals(0, elementEntry.uintValue());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_ACCEPTING_CONS_STATUS, elementEntry.name());
            assertEquals(DataType.DataTypes.UINT, elementEntry.loadType());
            assertEquals(0, elementEntry.uintValue());

            assertFalse(elIt.hasNext());

            assertTrue(filterIt.hasNext());

            filterEntry = filterIt.next();

            assertEquals(FilterEntry.FilterAction.SET, filterEntry.action());
            assertEquals(EmaRdm.SERVICE_STATE_FILTER, filterEntry.filterId());
            assertEquals(DataType.DataTypes.ELEMENT_LIST, filterEntry.loadType());

            elementList = filterEntry.elementList();

            elIt = elementList.iterator();
            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();

            assertEquals(EmaRdm.ENAME_SVC_STATE, elementEntry.name());
            assertEquals(1, elementEntry.uintValue());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_ACCEPTING_REQS, elementEntry.name());
            assertEquals(1, elementEntry.uintValue());

            assertFalse(filterIt.hasNext());

            consumer.unregister(directoryHandle);
            consumer.unregister(directoryHandle2);

            assertNotNull(consumer);
        }
        catch(OmmException ex)
        {
            assertFalse(true);
        }
        finally
        {
            System.out.println("Uninitializing...");

            consumer.uninitialize();
            ommprovider.uninitialize();
            ommprovider2.uninitialize();
        }
    }

    @Test
    public void testMultiConnectionsForDirectoryStreamViaRegisterClientDiffService()
    {
        TestUtilities.printTestHead("testMultiConnectionsForDirectoryStreamViaRegisterClientDiffService","");

        String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";

        OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);

        ProviderTestOptions providerTestOptions = new ProviderTestOptions();

        ProviderTestClient providerClient = new ProviderTestClient(providerTestOptions);

        // Provider_1 provides the DIRECT_FEED service name
        OmmProvider ommprovider = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_1"), providerClient);

        assertNotNull(ommprovider);

        // Provider_3 provides the DIRECT_FEED_2 service name
        OmmProvider ommprovider2 = EmaFactory.createOmmProvider(config.port("19004").providerName("Provider_3"), providerClient);

        assertNotNull(ommprovider2);

        OmmConsumer consumer = null;
        ConsumerTestClient consumerClient = new ConsumerTestClient();

        try
        {
            consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_9"));

            ReqMsg reqMsg = EmaFactory.createReqMsg();

            long directoryHandle = consumer.registerClient(reqMsg.domainType(EmaRdm.MMT_DIRECTORY), consumerClient);

            ConsumerSession<OmmConsumerClient> consumerSession = ((OmmConsumerImpl)consumer).consumerSession();

            /* Waits until OmmConsumer receives the refresh message */
            while(consumerClient.queueSize() == 0) {}

            assertEquals(1, consumerClient.queueSize()); // Ensure that the callback receives only one directory message

            Msg message = consumerClient.popMessage();

            assertTrue(message instanceof RefreshMsg);

            RefreshMsg refreshMsg = (RefreshMsg)message;

            assertEquals(2, refreshMsg.streamId());
            assertEquals(DomainTypes.SOURCE, refreshMsg.domainType());
            assertEquals("Open / Ok / None / ''", refreshMsg.state().toString());
            assertTrue(refreshMsg.solicited());
            assertTrue(refreshMsg.complete());
            assertTrue(refreshMsg.hasMsgKey());
            assertEquals(63, refreshMsg.filter());
            assertEquals(DataType.DataTypes.MAP, refreshMsg.payload().dataType());

            Map payload = refreshMsg.payload().map();

            assertEquals(2, payload.size());

            Iterator<MapEntry> mapIt =  payload.iterator();

            assertTrue(mapIt.hasNext());

            MapEntry mapEntry = mapIt.next();

            /* Service Id of the first service */
            int serviceId = (int)mapEntry.key().uintValue();

            String serviceName = consumerSession.sessionDirectoryById(serviceId).serviceName();

            assertEquals(serviceId, mapEntry.key().uintValue());

            assertEquals(DataType.DataTypes.FILTER_LIST, mapEntry.loadType());

            FilterList filterList = mapEntry.filterList();

            Iterator<FilterEntry> filterIt =  filterList.iterator();

            assertTrue(filterIt.hasNext());

            FilterEntry filterEntry = filterIt.next();

            assertEquals(FilterEntry.FilterAction.SET, filterEntry.action());
            assertEquals(EmaRdm.SERVICE_INFO_FILTER, filterEntry.filterId());
            assertEquals(DataType.DataTypes.ELEMENT_LIST, filterEntry.loadType());

            // Checks INFO filter
            ElementList elementList = filterEntry.elementList();

            Iterator<ElementEntry> elIt = elementList.iterator();
            assertTrue(elIt.hasNext());
            ElementEntry elementEntry = elIt.next();

            assertEquals(EmaRdm.ENAME_NAME, elementEntry.name());
            assertEquals(serviceName, elementEntry.ascii().ascii());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_VENDOR, elementEntry.name());
            assertEquals("company name", elementEntry.ascii().ascii());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_IS_SOURCE, elementEntry.name());
            assertEquals(0, elementEntry.uintValue());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_CAPABILITIES, elementEntry.name());
            assertEquals(DataType.DataTypes.ARRAY, elementEntry.loadType());
            OmmArray ommArray = elementEntry.array();
            Iterator<OmmArrayEntry> arrayIt = ommArray.iterator();
            assertTrue(arrayIt.hasNext());
            OmmArrayEntry arrayEntry = arrayIt.next();

            assertEquals(DataType.DataTypes.UINT, arrayEntry.loadType());
            assertEquals(6, arrayEntry.uintValue());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals(7, arrayEntry.uintValue());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals(8, arrayEntry.uintValue());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals(5, arrayEntry.uintValue());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals(10, arrayEntry.uintValue());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals(200, arrayEntry.uintValue());
            assertFalse(arrayIt.hasNext());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_DICTIONARYS_PROVIDED, elementEntry.name());
            assertEquals(DataType.DataTypes.ARRAY, elementEntry.loadType());
            ommArray = elementEntry.array();
            arrayIt = ommArray.iterator();
            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();

            assertEquals(DataType.DataTypes.ASCII, arrayEntry.loadType());
            assertEquals("RWFFld", arrayEntry.ascii().ascii());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals("RWFEnum", arrayEntry.ascii().ascii());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_DICTIONARYS_USED, elementEntry.name());
            assertEquals(DataType.DataTypes.ARRAY, elementEntry.loadType());
            ommArray = elementEntry.array();
            arrayIt = ommArray.iterator();
            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();

            assertEquals(DataType.DataTypes.ASCII, arrayEntry.loadType());
            assertEquals("RWFFld", arrayEntry.ascii().ascii());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals("RWFEnum", arrayEntry.ascii().ascii());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_QOS, elementEntry.name());
            assertEquals(DataType.DataTypes.ARRAY, elementEntry.loadType());
            ommArray = elementEntry.array();
            arrayIt = ommArray.iterator();
            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();

            assertEquals(DataType.DataTypes.QOS, arrayEntry.loadType());
            assertEquals("RealTime/TickByTick", arrayEntry.qos().toString());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals("Timeliness: 100/Rate: 100", arrayEntry.qos().toString());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_SUPPS_QOS_RANGE, elementEntry.name());
            assertEquals(DataType.DataTypes.UINT, elementEntry.loadType());
            assertEquals(0, elementEntry.uintValue());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_ITEM_LIST, elementEntry.name());
            assertEquals(DataType.DataTypes.ASCII, elementEntry.loadType());
            assertEquals("#.itemlist", elementEntry.ascii().ascii());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_SUPPS_OOB_SNAPSHOTS, elementEntry.name());
            assertEquals(DataType.DataTypes.UINT, elementEntry.loadType());
            assertEquals(0, elementEntry.uintValue());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_ACCEPTING_CONS_STATUS, elementEntry.name());
            assertEquals(DataType.DataTypes.UINT, elementEntry.loadType());
            assertEquals(0, elementEntry.uintValue());

            assertFalse(elIt.hasNext());

            assertTrue(filterIt.hasNext());

            filterEntry = filterIt.next();

            assertEquals(FilterEntry.FilterAction.SET, filterEntry.action());
            assertEquals(EmaRdm.SERVICE_STATE_FILTER, filterEntry.filterId());
            assertEquals(DataType.DataTypes.ELEMENT_LIST, filterEntry.loadType());

            elementList = filterEntry.elementList();

            elIt = elementList.iterator();
            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();

            assertEquals(EmaRdm.ENAME_SVC_STATE, elementEntry.name());
            assertEquals(1, elementEntry.uintValue());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_ACCEPTING_REQS, elementEntry.name());
            assertEquals(1, elementEntry.uintValue());

            assertFalse(filterIt.hasNext());

            assertTrue(mapIt.hasNext());

            mapEntry = mapIt.next();

            serviceId = (int)mapEntry.key().uintValue();

            serviceName = consumerSession.sessionDirectoryById(serviceId).serviceName();

            assertEquals(DataType.DataTypes.FILTER_LIST, mapEntry.loadType());

            filterList = mapEntry.filterList();

            filterIt =  filterList.iterator();

            assertTrue(filterIt.hasNext());

            filterEntry = filterIt.next();

            assertEquals(FilterEntry.FilterAction.SET, filterEntry.action());
            assertEquals(EmaRdm.SERVICE_INFO_FILTER, filterEntry.filterId());
            assertEquals(DataType.DataTypes.ELEMENT_LIST, filterEntry.loadType());

            // Checks INFO filter
            elementList = filterEntry.elementList();

            elIt = elementList.iterator();
            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();

            assertEquals(EmaRdm.ENAME_NAME, elementEntry.name());
            assertEquals(serviceName, elementEntry.ascii().ascii());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_VENDOR, elementEntry.name());
            assertEquals("company name", elementEntry.ascii().ascii());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_IS_SOURCE, elementEntry.name());
            assertEquals(0, elementEntry.uintValue());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_CAPABILITIES, elementEntry.name());
            assertEquals(DataType.DataTypes.ARRAY, elementEntry.loadType());
            ommArray = elementEntry.array();
            arrayIt = ommArray.iterator();
            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();

            assertEquals(DataType.DataTypes.UINT, arrayEntry.loadType());
            assertEquals(6, arrayEntry.uintValue());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals(7, arrayEntry.uintValue());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals(8, arrayEntry.uintValue());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals(5, arrayEntry.uintValue());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals(10, arrayEntry.uintValue());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals(200, arrayEntry.uintValue());
            assertFalse(arrayIt.hasNext());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_DICTIONARYS_PROVIDED, elementEntry.name());
            assertEquals(DataType.DataTypes.ARRAY, elementEntry.loadType());
            ommArray = elementEntry.array();
            arrayIt = ommArray.iterator();
            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();

            assertEquals(DataType.DataTypes.ASCII, arrayEntry.loadType());
            assertEquals("RWFFld", arrayEntry.ascii().ascii());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals("RWFEnum", arrayEntry.ascii().ascii());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_DICTIONARYS_USED, elementEntry.name());
            assertEquals(DataType.DataTypes.ARRAY, elementEntry.loadType());
            ommArray = elementEntry.array();
            arrayIt = ommArray.iterator();
            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();

            assertEquals(DataType.DataTypes.ASCII, arrayEntry.loadType());
            assertEquals("RWFFld", arrayEntry.ascii().ascii());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals("RWFEnum", arrayEntry.ascii().ascii());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_QOS, elementEntry.name());
            assertEquals(DataType.DataTypes.ARRAY, elementEntry.loadType());
            ommArray = elementEntry.array();
            arrayIt = ommArray.iterator();
            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();

            assertEquals(DataType.DataTypes.QOS, arrayEntry.loadType());
            assertEquals("RealTime/TickByTick", arrayEntry.qos().toString());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals("Timeliness: 100/Rate: 100", arrayEntry.qos().toString());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_SUPPS_QOS_RANGE, elementEntry.name());
            assertEquals(DataType.DataTypes.UINT, elementEntry.loadType());
            assertEquals(0, elementEntry.uintValue());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_ITEM_LIST, elementEntry.name());
            assertEquals(DataType.DataTypes.ASCII, elementEntry.loadType());
            assertEquals("#.itemlist", elementEntry.ascii().ascii());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_SUPPS_OOB_SNAPSHOTS, elementEntry.name());
            assertEquals(DataType.DataTypes.UINT, elementEntry.loadType());
            assertEquals(0, elementEntry.uintValue());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_ACCEPTING_CONS_STATUS, elementEntry.name());
            assertEquals(DataType.DataTypes.UINT, elementEntry.loadType());
            assertEquals(0, elementEntry.uintValue());

            assertFalse(elIt.hasNext());

            assertTrue(filterIt.hasNext());

            filterEntry = filterIt.next();

            assertEquals(FilterEntry.FilterAction.SET, filterEntry.action());
            assertEquals(EmaRdm.SERVICE_STATE_FILTER, filterEntry.filterId());
            assertEquals(DataType.DataTypes.ELEMENT_LIST, filterEntry.loadType());

            elementList = filterEntry.elementList();

            elIt = elementList.iterator();
            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();

            assertEquals(EmaRdm.ENAME_SVC_STATE, elementEntry.name());
            assertEquals(1, elementEntry.uintValue());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_ACCEPTING_REQS, elementEntry.name());
            assertEquals(1, elementEntry.uintValue());

            assertFalse(filterIt.hasNext());

            consumer.unregister(directoryHandle);

            assertNotNull(consumer);
        }
        catch(OmmException ex)
        {
            assertFalse(true);
        }
        finally
        {
            System.out.println("Uninitializing...");

            consumer.uninitialize();
            ommprovider.uninitialize();
            ommprovider2.uninitialize();
        }
    }

    @Test
    public void testMultiConnectionsForDirectoryStreamViaRegisterClientByServiceName()
    {
        TestUtilities.printTestHead("testMultiConnectionsForDirectoryStreamViaRegisterClientByServiceName","");

        String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";

        OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);

        ProviderTestOptions providerTestOptions = new ProviderTestOptions();

        ProviderTestClient providerClient = new ProviderTestClient(providerTestOptions);

        // Provider_1 provides the DIRECT_FEED service name
        OmmProvider ommprovider = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_1"), providerClient);

        assertNotNull(ommprovider);

        // Provider_3 provides the DIRECT_FEED_2 service name
        OmmProvider ommprovider2 = EmaFactory.createOmmProvider(config.port("19004").providerName("Provider_3"), providerClient);

        assertNotNull(ommprovider2);

        OmmConsumer consumer = null;
        ConsumerTestClient consumerClient = new ConsumerTestClient();

        try
        {
            consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_9"));

            ReqMsg reqMsg = EmaFactory.createReqMsg();

            long directoryHandle = consumer.registerClient(reqMsg.domainType(EmaRdm.MMT_DIRECTORY).serviceName("DIRECT_FEED_2"), consumerClient);

            /* Waits until OmmConsumer receives the refresh message */
            while(consumerClient.queueSize() == 0) {}

            assertEquals(1, consumerClient.queueSize()); // Ensure that the callback receives only one directory message

            Msg message = consumerClient.popMessage();

            assertTrue(message instanceof RefreshMsg);

            RefreshMsg refreshMsg = (RefreshMsg)message;

            assertEquals(2, refreshMsg.streamId());
            assertEquals(DomainTypes.SOURCE, refreshMsg.domainType());
            assertEquals("Open / Ok / None / ''", refreshMsg.state().toString());
            assertTrue(refreshMsg.solicited());
            assertTrue(refreshMsg.complete());
            assertTrue(refreshMsg.hasMsgKey());
            assertEquals(63, refreshMsg.filter());
            assertEquals(DataType.DataTypes.MAP, refreshMsg.payload().dataType());

            Map payload = refreshMsg.payload().map();

            assertEquals(1, payload.size());

            Iterator<MapEntry> mapIt =  payload.iterator();

            assertTrue(mapIt.hasNext());

            MapEntry mapEntry = mapIt.next();

            /* This depends on the first source directory message from provider */
            assertTrue(mapEntry.key().uintValue() == 32767 || mapEntry.key().uintValue() == 32768);

            assertEquals(DataType.DataTypes.FILTER_LIST, mapEntry.loadType());

            FilterList filterList = mapEntry.filterList();

            Iterator<FilterEntry> filterIt =  filterList.iterator();

            assertTrue(filterIt.hasNext());

            FilterEntry filterEntry = filterIt.next();

            assertEquals(FilterEntry.FilterAction.SET, filterEntry.action());
            assertEquals(EmaRdm.SERVICE_INFO_FILTER, filterEntry.filterId());
            assertEquals(DataType.DataTypes.ELEMENT_LIST, filterEntry.loadType());

            // Checks INFO filter
            ElementList elementList = filterEntry.elementList();

            Iterator<ElementEntry> elIt = elementList.iterator();
            assertTrue(elIt.hasNext());
            ElementEntry elementEntry = elIt.next();

            assertEquals(EmaRdm.ENAME_NAME, elementEntry.name());
            assertEquals("DIRECT_FEED_2", elementEntry.ascii().ascii());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_VENDOR, elementEntry.name());
            assertEquals("company name", elementEntry.ascii().ascii());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_IS_SOURCE, elementEntry.name());
            assertEquals(0, elementEntry.uintValue());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_CAPABILITIES, elementEntry.name());
            assertEquals(DataType.DataTypes.ARRAY, elementEntry.loadType());
            OmmArray ommArray = elementEntry.array();
            Iterator<OmmArrayEntry> arrayIt = ommArray.iterator();
            assertTrue(arrayIt.hasNext());
            OmmArrayEntry arrayEntry = arrayIt.next();

            assertEquals(DataType.DataTypes.UINT, arrayEntry.loadType());
            assertEquals(6, arrayEntry.uintValue());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals(7, arrayEntry.uintValue());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals(8, arrayEntry.uintValue());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals(5, arrayEntry.uintValue());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals(10, arrayEntry.uintValue());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals(200, arrayEntry.uintValue());
            assertFalse(arrayIt.hasNext());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_DICTIONARYS_PROVIDED, elementEntry.name());
            assertEquals(DataType.DataTypes.ARRAY, elementEntry.loadType());
            ommArray = elementEntry.array();
            arrayIt = ommArray.iterator();
            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();

            assertEquals(DataType.DataTypes.ASCII, arrayEntry.loadType());
            assertEquals("RWFFld", arrayEntry.ascii().ascii());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals("RWFEnum", arrayEntry.ascii().ascii());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_DICTIONARYS_USED, elementEntry.name());
            assertEquals(DataType.DataTypes.ARRAY, elementEntry.loadType());
            ommArray = elementEntry.array();
            arrayIt = ommArray.iterator();
            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();

            assertEquals(DataType.DataTypes.ASCII, arrayEntry.loadType());
            assertEquals("RWFFld", arrayEntry.ascii().ascii());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals("RWFEnum", arrayEntry.ascii().ascii());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_QOS, elementEntry.name());
            assertEquals(DataType.DataTypes.ARRAY, elementEntry.loadType());
            ommArray = elementEntry.array();
            arrayIt = ommArray.iterator();
            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();

            assertEquals(DataType.DataTypes.QOS, arrayEntry.loadType());
            assertEquals("RealTime/TickByTick", arrayEntry.qos().toString());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals("Timeliness: 100/Rate: 100", arrayEntry.qos().toString());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_SUPPS_QOS_RANGE, elementEntry.name());
            assertEquals(DataType.DataTypes.UINT, elementEntry.loadType());
            assertEquals(0, elementEntry.uintValue());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_ITEM_LIST, elementEntry.name());
            assertEquals(DataType.DataTypes.ASCII, elementEntry.loadType());
            assertEquals("#.itemlist", elementEntry.ascii().ascii());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_SUPPS_OOB_SNAPSHOTS, elementEntry.name());
            assertEquals(DataType.DataTypes.UINT, elementEntry.loadType());
            assertEquals(0, elementEntry.uintValue());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_ACCEPTING_CONS_STATUS, elementEntry.name());
            assertEquals(DataType.DataTypes.UINT, elementEntry.loadType());
            assertEquals(0, elementEntry.uintValue());

            assertFalse(elIt.hasNext());

            assertTrue(filterIt.hasNext());
            filterEntry = filterIt.next();

            assertEquals(FilterEntry.FilterAction.SET, filterEntry.action());
            assertEquals(EmaRdm.SERVICE_STATE_FILTER, filterEntry.filterId());
            assertEquals(DataType.DataTypes.ELEMENT_LIST, filterEntry.loadType());

            elementList = filterEntry.elementList();

            elIt = elementList.iterator();
            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();

            assertEquals(EmaRdm.ENAME_SVC_STATE, elementEntry.name());
            assertEquals(1, elementEntry.uintValue());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_ACCEPTING_REQS, elementEntry.name());
            assertEquals(1, elementEntry.uintValue());

            assertFalse(filterIt.hasNext());
            assertFalse(mapIt.hasNext());

            consumer.unregister(directoryHandle);

            assertNotNull(consumer);
        }
        catch(OmmException ex)
        {
            assertFalse(true);
        }
        finally
        {
            System.out.println("Uninitializing...");

            consumer.uninitialize();
            ommprovider.uninitialize();
            ommprovider2.uninitialize();
        }
    }

    @Test
    public void testMultiConnectionsForDirectoryStreamViaRegisterClientByServiceId()
    {
        TestUtilities.printTestHead("testMultiConnectionsForDirectoryStreamViaRegisterClientByServiceId","");

        String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";

        OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);

        ProviderTestOptions providerTestOptions = new ProviderTestOptions();

        ProviderTestClient providerClient = new ProviderTestClient(providerTestOptions);

        // Provider_1 provides the DIRECT_FEED service name
        OmmProvider ommprovider = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_1"), providerClient);

        assertNotNull(ommprovider);

        // Provider_3 provides the DIRECT_FEED_2 service name
        OmmProvider ommprovider2 = EmaFactory.createOmmProvider(config.port("19004").providerName("Provider_3"), providerClient);

        assertNotNull(ommprovider2);

        OmmConsumer consumer = null;
        ConsumerTestClient consumerClient = new ConsumerTestClient();

        try
        {
            consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_9"));

            ReqMsg reqMsg = EmaFactory.createReqMsg();

            long directoryHandle = consumer.registerClient(reqMsg.domainType(EmaRdm.MMT_DIRECTORY).serviceId(32767), consumerClient);

            /* Waits until OmmConsumer receives the refresh message */
            while(consumerClient.queueSize() == 0) {}

            assertEquals(1, consumerClient.queueSize()); // Ensure that the callback receives only one directory message

            Msg message = consumerClient.popMessage();

            assertTrue(message instanceof RefreshMsg);

            RefreshMsg refreshMsg = (RefreshMsg)message;

            assertEquals(2, refreshMsg.streamId());
            assertEquals(DomainTypes.SOURCE, refreshMsg.domainType());
            assertEquals("Open / Ok / None / ''", refreshMsg.state().toString());
            assertTrue(refreshMsg.solicited());
            assertTrue(refreshMsg.complete());
            assertTrue(refreshMsg.hasMsgKey());
            assertEquals(63, refreshMsg.filter());
            assertEquals(DataType.DataTypes.MAP, refreshMsg.payload().dataType());

            Map payload = refreshMsg.payload().map();

            assertEquals(1, payload.size());

            Iterator<MapEntry> mapIt =  payload.iterator();

            assertTrue(mapIt.hasNext());

            MapEntry mapEntry = mapIt.next();

            assertEquals(32767, mapEntry.key().uintValue());

            assertEquals(DataType.DataTypes.FILTER_LIST, mapEntry.loadType());

            FilterList filterList = mapEntry.filterList();

            Iterator<FilterEntry> filterIt =  filterList.iterator();

            assertTrue(filterIt.hasNext());

            FilterEntry filterEntry = filterIt.next();

            assertEquals(FilterEntry.FilterAction.SET, filterEntry.action());
            assertEquals(EmaRdm.SERVICE_INFO_FILTER, filterEntry.filterId());
            assertEquals(DataType.DataTypes.ELEMENT_LIST, filterEntry.loadType());

            // Checks INFO filter
            ElementList elementList = filterEntry.elementList();

            Iterator<ElementEntry> elIt = elementList.iterator();
            assertTrue(elIt.hasNext());
            ElementEntry elementEntry = elIt.next();

            assertEquals(EmaRdm.ENAME_NAME, elementEntry.name());

            /* This depends on the first source directory message from provider */
            assertTrue(elementEntry.ascii().ascii().equals("DIRECT_FEED_2") || elementEntry.ascii().ascii().equals("DIRECT_FEED"));

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_VENDOR, elementEntry.name());
            assertEquals("company name", elementEntry.ascii().ascii());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_IS_SOURCE, elementEntry.name());
            assertEquals(0, elementEntry.uintValue());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_CAPABILITIES, elementEntry.name());
            assertEquals(DataType.DataTypes.ARRAY, elementEntry.loadType());
            OmmArray ommArray = elementEntry.array();
            Iterator<OmmArrayEntry> arrayIt = ommArray.iterator();
            assertTrue(arrayIt.hasNext());
            OmmArrayEntry arrayEntry = arrayIt.next();

            assertEquals(DataType.DataTypes.UINT, arrayEntry.loadType());
            assertEquals(6, arrayEntry.uintValue());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals(7, arrayEntry.uintValue());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals(8, arrayEntry.uintValue());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals(5, arrayEntry.uintValue());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals(10, arrayEntry.uintValue());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals(200, arrayEntry.uintValue());
            assertFalse(arrayIt.hasNext());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_DICTIONARYS_PROVIDED, elementEntry.name());
            assertEquals(DataType.DataTypes.ARRAY, elementEntry.loadType());
            ommArray = elementEntry.array();
            arrayIt = ommArray.iterator();
            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();

            assertEquals(DataType.DataTypes.ASCII, arrayEntry.loadType());
            assertEquals("RWFFld", arrayEntry.ascii().ascii());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals("RWFEnum", arrayEntry.ascii().ascii());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_DICTIONARYS_USED, elementEntry.name());
            assertEquals(DataType.DataTypes.ARRAY, elementEntry.loadType());
            ommArray = elementEntry.array();
            arrayIt = ommArray.iterator();
            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();

            assertEquals(DataType.DataTypes.ASCII, arrayEntry.loadType());
            assertEquals("RWFFld", arrayEntry.ascii().ascii());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals("RWFEnum", arrayEntry.ascii().ascii());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_QOS, elementEntry.name());
            assertEquals(DataType.DataTypes.ARRAY, elementEntry.loadType());
            ommArray = elementEntry.array();
            arrayIt = ommArray.iterator();
            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();

            assertEquals(DataType.DataTypes.QOS, arrayEntry.loadType());
            assertEquals("RealTime/TickByTick", arrayEntry.qos().toString());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals("Timeliness: 100/Rate: 100", arrayEntry.qos().toString());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_SUPPS_QOS_RANGE, elementEntry.name());
            assertEquals(DataType.DataTypes.UINT, elementEntry.loadType());
            assertEquals(0, elementEntry.uintValue());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_ITEM_LIST, elementEntry.name());
            assertEquals(DataType.DataTypes.ASCII, elementEntry.loadType());
            assertEquals("#.itemlist", elementEntry.ascii().ascii());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_SUPPS_OOB_SNAPSHOTS, elementEntry.name());
            assertEquals(DataType.DataTypes.UINT, elementEntry.loadType());
            assertEquals(0, elementEntry.uintValue());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_ACCEPTING_CONS_STATUS, elementEntry.name());
            assertEquals(DataType.DataTypes.UINT, elementEntry.loadType());
            assertEquals(0, elementEntry.uintValue());

            assertFalse(elIt.hasNext());

            assertTrue(filterIt.hasNext());
            filterEntry = filterIt.next();

            assertEquals(FilterEntry.FilterAction.SET, filterEntry.action());
            assertEquals(EmaRdm.SERVICE_STATE_FILTER, filterEntry.filterId());
            assertEquals(DataType.DataTypes.ELEMENT_LIST, filterEntry.loadType());

            elementList = filterEntry.elementList();

            elIt = elementList.iterator();
            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();

            assertEquals(EmaRdm.ENAME_SVC_STATE, elementEntry.name());
            assertEquals(1, elementEntry.uintValue());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_ACCEPTING_REQS, elementEntry.name());
            assertEquals(1, elementEntry.uintValue());

            assertFalse(filterIt.hasNext());
            assertFalse(mapIt.hasNext());

            consumer.unregister(directoryHandle);

            assertNotNull(consumer);
        }
        catch(OmmException ex)
        {
            assertFalse(true);
        }
        finally
        {
            System.out.println("Uninitializing...");

            consumer.uninitialize();
            ommprovider.uninitialize();
            ommprovider2.uninitialize();
        }
    }

    @Test
    public void testMultiConnectionsForDirectoryStreamViaRegisterClientWithUnknowServiceName()
    {
        TestUtilities.printTestHead("testMultiConnectionsForDirectoryStreamViaRegisterClientWithUnknowServiceName","");

        String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";

        OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);

        ProviderTestOptions providerTestOptions = new ProviderTestOptions();

        ProviderTestClient providerClient = new ProviderTestClient(providerTestOptions);

        // Provider_1 provides the DIRECT_FEED service name
        OmmProvider ommprovider = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_1"), providerClient);

        assertNotNull(ommprovider);

        // Provider_3 provides the DIRECT_FEED_2 service name
        OmmProvider ommprovider2 = EmaFactory.createOmmProvider(config.port("19004").providerName("Provider_3"), providerClient);

        assertNotNull(ommprovider2);

        OmmConsumer consumer = null;
        ConsumerTestClient consumerClient = new ConsumerTestClient();

        try
        {
            consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_9"));

            ReqMsg reqMsg = EmaFactory.createReqMsg();

            long directoryHandle = consumer.registerClient(reqMsg.domainType(EmaRdm.MMT_DIRECTORY).serviceName("UNKNOWN_SERVICE"), consumerClient);

            /* Waits until OmmConsumer receives the refresh message */
            while(consumerClient.queueSize() == 0) {}

            assertEquals(1, consumerClient.queueSize()); // Ensure that the callback receives only one directory message

            Msg message = consumerClient.popMessage();

            assertTrue(message instanceof RefreshMsg);

            RefreshMsg refreshMsg = (RefreshMsg)message;

            assertEquals(2, refreshMsg.streamId());
            assertEquals(DomainTypes.SOURCE, refreshMsg.domainType());
            assertEquals("Open / Ok / None / ''", refreshMsg.state().toString());
            assertTrue(refreshMsg.solicited());
            assertTrue(refreshMsg.complete());
            assertTrue(refreshMsg.hasMsgKey());
            assertEquals("UNKNOWN_SERVICE", refreshMsg.serviceName());
            assertEquals(63, refreshMsg.filter());
            assertEquals(DataType.DataTypes.MAP, refreshMsg.payload().dataType());

            assertEquals(0, refreshMsg.payload().map().size());

            consumer.unregister(directoryHandle);

            assertNotNull(consumer);
        }
        catch(OmmException ex)
        {
            assertFalse(true);
        }
        finally
        {
            System.out.println("Uninitializing...");

            consumer.uninitialize();
            ommprovider.uninitialize();
            ommprovider2.uninitialize();
        }
    }

    @Test
    public void testMultiConnectionsForDirectoryStreamViaRegisterClientWithUnknowServiceId()
    {
        TestUtilities.printTestHead("testMultiConnectionsForDirectoryStreamViaRegisterClientWithUnknowServiceId","");

        String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";

        OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);

        ProviderTestOptions providerTestOptions = new ProviderTestOptions();

        ProviderTestClient providerClient = new ProviderTestClient(providerTestOptions);

        // Provider_1 provides the DIRECT_FEED service name
        OmmProvider ommprovider = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_1"), providerClient);

        assertNotNull(ommprovider);

        // Provider_3 provides the DIRECT_FEED_2 service name
        OmmProvider ommprovider2 = EmaFactory.createOmmProvider(config.port("19004").providerName("Provider_3"), providerClient);

        assertNotNull(ommprovider2);

        OmmConsumer consumer = null;
        ConsumerTestClient consumerClient = new ConsumerTestClient();

        try
        {
            consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_9"));

            ReqMsg reqMsg = EmaFactory.createReqMsg();

            long directoryHandle = consumer.registerClient(reqMsg.domainType(EmaRdm.MMT_DIRECTORY).serviceId(55555), consumerClient);

            /* Waits until OmmConsumer receives the refresh message */
            while(consumerClient.queueSize() == 0) {}

            assertEquals(1, consumerClient.queueSize()); // Ensure that the callback receives only one directory message

            Msg message = consumerClient.popMessage();

            assertTrue(message instanceof RefreshMsg);

            RefreshMsg refreshMsg = (RefreshMsg)message;

            assertEquals(2, refreshMsg.streamId());
            assertEquals(DomainTypes.SOURCE, refreshMsg.domainType());
            assertEquals("Open / Ok / None / ''", refreshMsg.state().toString());
            assertTrue(refreshMsg.solicited());
            assertTrue(refreshMsg.complete());
            assertTrue(refreshMsg.hasMsgKey());
            assertTrue(refreshMsg.hasMsgKey());
            assertEquals(55555, refreshMsg.serviceId());
            assertEquals(63, refreshMsg.filter());
            assertEquals(DataType.DataTypes.MAP, refreshMsg.payload().dataType());

            assertEquals(0, refreshMsg.payload().map().size());

            consumer.unregister(directoryHandle);

            assertNotNull(consumer);
        }
        catch(OmmException ex)
        {
            assertFalse(true);
        }
        finally
        {
            System.out.println("Uninitializing...");

            consumer.uninitialize();
            ommprovider.uninitialize();
            ommprovider2.uninitialize();
        }
    }

    @Test
    public void testMultiConnectionsForDirectoryStreamViaRegisterClientWithServiceDeletion()
    {
        TestUtilities.printTestHead("testMultiConnectionsForDirectoryStreamViaRegisterClientWithServiceDeletion","");

        String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";

        OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);

        ProviderTestOptions providerTestOptions = new ProviderTestOptions();

        ProviderTestClient providerClient = new ProviderTestClient(providerTestOptions);

        // Provider_1 provides the DIRECT_FEED service name
        OmmProvider ommprovider = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_1"), providerClient);

        assertNotNull(ommprovider);

        // Provider_3 provides the DIRECT_FEED_2 service name
        OmmProvider ommprovider2 = EmaFactory.createOmmProvider(config.port("19004").providerName("Provider_3"), providerClient);

        assertNotNull(ommprovider2);

        OmmConsumer consumer = null;
        ConsumerTestClient consumerClient = new ConsumerTestClient();

        try
        {
            consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_9"));

            ReqMsg reqMsg = EmaFactory.createReqMsg();

            long directoryHandle = consumer.registerClient(reqMsg.domainType(EmaRdm.MMT_DIRECTORY), consumerClient);

            long directoryHandle2 = consumer.registerClient(reqMsg.domainType(EmaRdm.MMT_DIRECTORY).serviceName("DIRECT_FEED_2"), consumerClient);

            /* Waits until OmmConsumer receives the refresh message */
            while(consumerClient.queueSize() == 0) {}

            Thread.sleep(2000);

            assertEquals(2, consumerClient.queueSize()); // Ensure that the callback receives only one directory message

            Msg message = consumerClient.popMessage();

            assertTrue(message instanceof RefreshMsg);

            RefreshMsg refreshMsg = (RefreshMsg)message;

            assertEquals(2, refreshMsg.streamId());
            assertEquals(DomainTypes.SOURCE, refreshMsg.domainType());
            assertEquals("Open / Ok / None / ''", refreshMsg.state().toString());
            assertTrue(refreshMsg.solicited());
            assertTrue(refreshMsg.complete());
            assertTrue(refreshMsg.hasMsgKey());
            assertEquals(DataType.DataTypes.MAP, refreshMsg.payload().dataType());

            while(consumerClient.queueSize() == 0) {}

            message = consumerClient.popMessage();

            assertTrue(message instanceof RefreshMsg);

            refreshMsg = (RefreshMsg)message;

            {
                FilterList filterList = EmaFactory.createFilterList();
                Map map = EmaFactory.createMap();
                map.add( EmaFactory.createMapEntry().keyUInt( 1, MapEntry.MapAction.DELETE, filterList));

                UpdateMsg updateMsg = EmaFactory.createUpdateMsg();
                ommprovider.submit( updateMsg.domainType( EmaRdm.MMT_DIRECTORY ).
                        payload( map ), 0);	// use 0 item handle to fanout to all subscribers

                map.clear();
                map.add( EmaFactory.createMapEntry().keyUInt( 1, MapEntry.MapAction.DELETE, filterList));
                ommprovider2.submit( updateMsg.clear().domainType( EmaRdm.MMT_DIRECTORY ).
                        payload( map ), 0);	// use 0 item handle to fanout to all subscribers
            }

            while(consumerClient.queueSize() != 3) {}

            int id32767Count = 0;
            int id32768Count = 0;

            message = consumerClient.popMessage();

            assertTrue(message instanceof UpdateMsg);

            UpdateMsg updateMsg = (UpdateMsg)message;
            assertEquals(2, updateMsg.streamId());
            assertEquals(DomainTypes.SOURCE, updateMsg.domainType());
            assertEquals(DataType.DataTypes.MAP, updateMsg.payload().dataType());

            Map payload = updateMsg.payload().map();

            assertEquals(1, payload.size());

            Iterator<MapEntry> mapIt =  payload.iterator();

            assertTrue(mapIt.hasNext());

            MapEntry mapEntry = mapIt.next();

            if (mapEntry.key().uintValue() == 32767) id32767Count++;
            if (mapEntry.key().uintValue() == 32768) id32768Count++;

            //assertEquals(32767, mapEntry.key().uintValue());

            assertEquals(DataType.DataTypes.NO_DATA, mapEntry.loadType());

            message = consumerClient.popMessage();

            assertTrue(message instanceof UpdateMsg);

            updateMsg = (UpdateMsg)message;
            assertEquals(2, updateMsg.streamId());
            assertEquals(DomainTypes.SOURCE, updateMsg.domainType());
            assertEquals(DataType.DataTypes.MAP, updateMsg.payload().dataType());

            payload = updateMsg.payload().map();

            assertEquals(1, payload.size());

            mapIt =  payload.iterator();

            assertTrue(mapIt.hasNext());

            mapEntry = mapIt.next();

            //assertEquals(32768, mapEntry.key().uintValue());
            if (mapEntry.key().uintValue() == 32767) id32767Count++;
            if (mapEntry.key().uintValue() == 32768) id32768Count++;

            assertEquals(DataType.DataTypes.NO_DATA, mapEntry.loadType());

            message = consumerClient.popMessage();

            assertTrue(message instanceof UpdateMsg);

            updateMsg = (UpdateMsg)message;
            assertEquals(2, updateMsg.streamId());
            assertEquals(DomainTypes.SOURCE, updateMsg.domainType());
            assertEquals(DataType.DataTypes.MAP, updateMsg.payload().dataType());

            payload = updateMsg.payload().map();

            assertEquals(1, payload.size());

            mapIt =  payload.iterator();

            assertTrue(mapIt.hasNext());

            mapEntry = mapIt.next();

            //assertEquals(32768, mapEntry.key().uintValue());
            if (mapEntry.key().uintValue() == 32767) id32767Count++;
            if (mapEntry.key().uintValue() == 32768) id32768Count++;

            assertEquals(DataType.DataTypes.NO_DATA, mapEntry.loadType());

            assertTrue(id32768Count == 2 && id32767Count == 1 || id32768Count == 1 && id32767Count == 2);


            consumer.unregister(directoryHandle);
            consumer.unregister(directoryHandle2);

            assertNotNull(consumer);
        }
        catch(OmmException ex)
        {
            assertFalse(true);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        finally
        {
            System.out.println("Uninitializing...");

            consumer.uninitialize();
            ommprovider.uninitialize();
            ommprovider2.uninitialize();
        }
    }

    @Test
    public void testMultiConnectionsForDirectoryStreamViaRegisterClientSameServiceWithStateChange()
    {
        TestUtilities.printTestHead("testMultiConnectionsForDirectoryStreamViaRegisterClientSameServiceWithStateChange","");

        String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";

        OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);

        ProviderTestOptions providerTestOptions = new ProviderTestOptions();

        ProviderTestClient providerClient = new ProviderTestClient(providerTestOptions);

        // Provider_1 provides the DIRECT_FEED service name
        OmmProvider ommprovider = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_1"), providerClient);

        assertNotNull(ommprovider);

        // Provider_1 provides the DIRECT_FEED service name
        OmmProvider ommprovider2 = EmaFactory.createOmmProvider(config.port("19004").providerName("Provider_1"), providerClient);

        assertNotNull(ommprovider2);

        OmmConsumer consumer = null;
        ConsumerTestClient consumerClient = new ConsumerTestClient();

        try
        {
            consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_9"));

            ReqMsg reqMsg = EmaFactory.createReqMsg();

            // Request info filter only
            long directoryHandle = consumer.registerClient(reqMsg.domainType(EmaRdm.MMT_DIRECTORY).filter(63), consumerClient);

            /* Waits until OmmConsumer receives the refresh message */
            while(consumerClient.queueSize() == 0) {}

            assertEquals(1, consumerClient.queueSize()); // Ensure that the callback receives only one directory message

            Msg message = consumerClient.popMessage();

            assertTrue(message instanceof RefreshMsg);

            RefreshMsg refreshMsg = (RefreshMsg)message;

            assertEquals(2, refreshMsg.streamId());
            assertEquals(DomainTypes.SOURCE, refreshMsg.domainType());
            assertEquals("Open / Ok / None / ''", refreshMsg.state().toString());
            assertTrue(refreshMsg.solicited());
            assertTrue(refreshMsg.complete());
            assertTrue(refreshMsg.hasMsgKey());
            assertEquals(63, refreshMsg.filter());
            assertEquals(DataType.DataTypes.MAP, refreshMsg.payload().dataType());

            Map payload = refreshMsg.payload().map();

            assertEquals(1, payload.size());

            Iterator<MapEntry> mapIt =  payload.iterator();

            assertTrue(mapIt.hasNext());

            MapEntry mapEntry = mapIt.next();

            long serviceId = 32767;

            assertEquals(serviceId, mapEntry.key().uintValue());

            assertEquals(DataType.DataTypes.FILTER_LIST, mapEntry.loadType());

            FilterList filterList = mapEntry.filterList();

            Iterator<FilterEntry> filterIt =  filterList.iterator();

            assertTrue(filterIt.hasNext());

            FilterEntry filterEntry = filterIt.next();

            assertEquals(FilterEntry.FilterAction.SET, filterEntry.action());
            assertEquals(EmaRdm.SERVICE_INFO_FILTER, filterEntry.filterId());
            assertEquals(DataType.DataTypes.ELEMENT_LIST, filterEntry.loadType());

            // Checks INFO filter
            ElementList elementList = filterEntry.elementList();

            Iterator<ElementEntry> elIt = elementList.iterator();
            assertTrue(elIt.hasNext());
            ElementEntry elementEntry = elIt.next();

            assertEquals(EmaRdm.ENAME_NAME, elementEntry.name());
            assertEquals("DIRECT_FEED", elementEntry.ascii().ascii());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_VENDOR, elementEntry.name());
            assertEquals("company name", elementEntry.ascii().ascii());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_IS_SOURCE, elementEntry.name());
            assertEquals(0, elementEntry.uintValue());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_CAPABILITIES, elementEntry.name());
            assertEquals(DataType.DataTypes.ARRAY, elementEntry.loadType());
            OmmArray ommArray = elementEntry.array();
            Iterator<OmmArrayEntry> arrayIt = ommArray.iterator();
            assertTrue(arrayIt.hasNext());
            OmmArrayEntry arrayEntry = arrayIt.next();

            assertEquals(DataType.DataTypes.UINT, arrayEntry.loadType());
            assertEquals(6, arrayEntry.uintValue());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals(7, arrayEntry.uintValue());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals(8, arrayEntry.uintValue());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals(5, arrayEntry.uintValue());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals(10, arrayEntry.uintValue());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals(200, arrayEntry.uintValue());
            assertFalse(arrayIt.hasNext());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_DICTIONARYS_PROVIDED, elementEntry.name());
            assertEquals(DataType.DataTypes.ARRAY, elementEntry.loadType());
            ommArray = elementEntry.array();
            arrayIt = ommArray.iterator();
            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();

            assertEquals(DataType.DataTypes.ASCII, arrayEntry.loadType());
            assertEquals("RWFFld", arrayEntry.ascii().ascii());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals("RWFEnum", arrayEntry.ascii().ascii());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_DICTIONARYS_USED, elementEntry.name());
            assertEquals(DataType.DataTypes.ARRAY, elementEntry.loadType());
            ommArray = elementEntry.array();
            arrayIt = ommArray.iterator();
            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();

            assertEquals(DataType.DataTypes.ASCII, arrayEntry.loadType());
            assertEquals("RWFFld", arrayEntry.ascii().ascii());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals("RWFEnum", arrayEntry.ascii().ascii());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_QOS, elementEntry.name());
            assertEquals(DataType.DataTypes.ARRAY, elementEntry.loadType());
            ommArray = elementEntry.array();
            arrayIt = ommArray.iterator();
            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();

            assertEquals(DataType.DataTypes.QOS, arrayEntry.loadType());
            assertEquals("RealTime/TickByTick", arrayEntry.qos().toString());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals("Timeliness: 100/Rate: 100", arrayEntry.qos().toString());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_SUPPS_QOS_RANGE, elementEntry.name());
            assertEquals(DataType.DataTypes.UINT, elementEntry.loadType());
            assertEquals(0, elementEntry.uintValue());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_ITEM_LIST, elementEntry.name());
            assertEquals(DataType.DataTypes.ASCII, elementEntry.loadType());
            assertEquals("#.itemlist", elementEntry.ascii().ascii());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_SUPPS_OOB_SNAPSHOTS, elementEntry.name());
            assertEquals(DataType.DataTypes.UINT, elementEntry.loadType());
            assertEquals(0, elementEntry.uintValue());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_ACCEPTING_CONS_STATUS, elementEntry.name());
            assertEquals(DataType.DataTypes.UINT, elementEntry.loadType());
            assertEquals(0, elementEntry.uintValue());

            assertFalse(elIt.hasNext());

            assertTrue(filterIt.hasNext());

            filterEntry = filterIt.next();

            assertEquals(FilterEntry.FilterAction.SET, filterEntry.action());
            assertEquals(EmaRdm.SERVICE_STATE_FILTER, filterEntry.filterId());
            assertEquals(DataType.DataTypes.ELEMENT_LIST, filterEntry.loadType());

            elementList = filterEntry.elementList();

            elIt = elementList.iterator();
            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();

            assertEquals(EmaRdm.ENAME_SVC_STATE, elementEntry.name());
            assertEquals(1, elementEntry.uintValue());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_ACCEPTING_REQS, elementEntry.name());
            assertEquals(1, elementEntry.uintValue());

            assertFalse(filterIt.hasNext());

            assertFalse(mapIt.hasNext());

            Thread.sleep(3000);

            // Change service's state to down from one provider
            ElementList serviceState = EmaFactory.createElementList();
            serviceState.add( EmaFactory.createElementEntry().uintValue( EmaRdm.ENAME_SVC_STATE, 0 ));

            FilterList filterListEnc = EmaFactory.createFilterList();
            filterListEnc.add( EmaFactory.createFilterEntry().elementList( EmaRdm.SERVICE_STATE_ID, FilterEntry.FilterAction.UPDATE, serviceState ) );

            Map map = EmaFactory.createMap();
            map.add( EmaFactory.createMapEntry().keyUInt( 1, MapEntry.MapAction.UPDATE, filterListEnc ));

            UpdateMsg updateMsg = EmaFactory.createUpdateMsg();
            ommprovider.submit( updateMsg.domainType( EmaRdm.MMT_DIRECTORY ).
                    filter( EmaRdm.SERVICE_STATE_FILTER ).
                    payload( map ), 0);	// use 0 item handle to fan-out to all subscribers

            Thread.sleep(5000);

            /* No message in the queue as the service state is still up on another provider */
            assertEquals(0, consumerClient.queueSize());

            ommprovider2.submit( updateMsg.clear().domainType( EmaRdm.MMT_DIRECTORY ).
                    filter( EmaRdm.SERVICE_STATE_FILTER ).
                    payload( map ), 0);	// use 0 item handle to fan-out to all subscribers

            Thread.sleep(5000);

            assertEquals(1, consumerClient.queueSize()); // Receives source directory update message to notify that the service state is down.

            message = consumerClient.popMessage();

            assertTrue(message instanceof UpdateMsg);

            updateMsg = (UpdateMsg)message;

            assertEquals(2, updateMsg.streamId());
            assertEquals(DomainTypes.SOURCE, updateMsg.domainType());
            assertTrue(updateMsg.hasMsgKey());
            assertEquals(63, updateMsg.filter());
            assertEquals(DataType.DataTypes.MAP, updateMsg.payload().dataType());

            payload = updateMsg.payload().map();

            assertEquals(1, payload.size());

            mapIt =  payload.iterator();

            assertTrue(mapIt.hasNext());

            mapEntry = mapIt.next();

            assertEquals(serviceId, mapEntry.key().uintValue());

            assertEquals(DataType.DataTypes.FILTER_LIST, mapEntry.loadType());

            filterList = mapEntry.filterList();

            filterIt =  filterList.iterator();

            assertTrue(filterIt.hasNext());

            filterEntry = filterIt.next();

            assertEquals(FilterEntry.FilterAction.SET, filterEntry.action());
            assertEquals(EmaRdm.SERVICE_INFO_FILTER, filterEntry.filterId());
            assertEquals(DataType.DataTypes.ELEMENT_LIST, filterEntry.loadType());

            // Checks INFO filter
            elementList = filterEntry.elementList();

            elIt = elementList.iterator();
            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();

            assertEquals(EmaRdm.ENAME_NAME, elementEntry.name());
            assertEquals("DIRECT_FEED", elementEntry.ascii().ascii());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_VENDOR, elementEntry.name());
            assertEquals("company name", elementEntry.ascii().ascii());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_IS_SOURCE, elementEntry.name());
            assertEquals(0, elementEntry.uintValue());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_CAPABILITIES, elementEntry.name());
            assertEquals(DataType.DataTypes.ARRAY, elementEntry.loadType());
            ommArray = elementEntry.array();
            arrayIt = ommArray.iterator();
            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();

            assertEquals(DataType.DataTypes.UINT, arrayEntry.loadType());
            assertEquals(6, arrayEntry.uintValue());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals(7, arrayEntry.uintValue());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals(8, arrayEntry.uintValue());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals(5, arrayEntry.uintValue());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals(10, arrayEntry.uintValue());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals(200, arrayEntry.uintValue());
            assertFalse(arrayIt.hasNext());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_DICTIONARYS_PROVIDED, elementEntry.name());
            assertEquals(DataType.DataTypes.ARRAY, elementEntry.loadType());
            ommArray = elementEntry.array();
            arrayIt = ommArray.iterator();
            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();

            assertEquals(DataType.DataTypes.ASCII, arrayEntry.loadType());
            assertEquals("RWFFld", arrayEntry.ascii().ascii());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals("RWFEnum", arrayEntry.ascii().ascii());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_DICTIONARYS_USED, elementEntry.name());
            assertEquals(DataType.DataTypes.ARRAY, elementEntry.loadType());
            ommArray = elementEntry.array();
            arrayIt = ommArray.iterator();
            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();

            assertEquals(DataType.DataTypes.ASCII, arrayEntry.loadType());
            assertEquals("RWFFld", arrayEntry.ascii().ascii());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals("RWFEnum", arrayEntry.ascii().ascii());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_QOS, elementEntry.name());
            assertEquals(DataType.DataTypes.ARRAY, elementEntry.loadType());
            ommArray = elementEntry.array();
            arrayIt = ommArray.iterator();
            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();

            assertEquals(DataType.DataTypes.QOS, arrayEntry.loadType());
            assertEquals("RealTime/TickByTick", arrayEntry.qos().toString());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals("Timeliness: 100/Rate: 100", arrayEntry.qos().toString());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_SUPPS_QOS_RANGE, elementEntry.name());
            assertEquals(DataType.DataTypes.UINT, elementEntry.loadType());
            assertEquals(0, elementEntry.uintValue());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_ITEM_LIST, elementEntry.name());
            assertEquals(DataType.DataTypes.ASCII, elementEntry.loadType());
            assertEquals("#.itemlist", elementEntry.ascii().ascii());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_SUPPS_OOB_SNAPSHOTS, elementEntry.name());
            assertEquals(DataType.DataTypes.UINT, elementEntry.loadType());
            assertEquals(0, elementEntry.uintValue());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_ACCEPTING_CONS_STATUS, elementEntry.name());
            assertEquals(DataType.DataTypes.UINT, elementEntry.loadType());
            assertEquals(0, elementEntry.uintValue());

            assertFalse(elIt.hasNext());

            assertTrue(filterIt.hasNext());

            filterEntry = filterIt.next();

            assertEquals(FilterEntry.FilterAction.SET, filterEntry.action());
            assertEquals(EmaRdm.SERVICE_STATE_FILTER, filterEntry.filterId());
            assertEquals(DataType.DataTypes.ELEMENT_LIST, filterEntry.loadType());

            elementList = filterEntry.elementList();

            elIt = elementList.iterator();
            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();

            assertEquals(EmaRdm.ENAME_SVC_STATE, elementEntry.name());
            assertEquals(0, elementEntry.uintValue());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_ACCEPTING_REQS, elementEntry.name());
            assertEquals(1, elementEntry.uintValue());

            assertFalse(filterIt.hasNext());

            assertFalse(mapIt.hasNext());

            consumer.unregister(directoryHandle);

            assertNotNull(consumer);
        }
        catch(OmmException ex)
        {
            assertFalse(true);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        finally
        {
            System.out.println("Uninitializing...");

            consumer.uninitialize();
            ommprovider.uninitialize();
            ommprovider2.uninitialize();
        }
    }

    @Test
    public void testMultiConnectionsForDirectoryStreamViaRegisterClientSameServiceWithAcceptingRequestsChange()
    {
        TestUtilities.printTestHead("testMultiConnectionsForDirectoryStreamViaRegisterClientSameServiceWithAcceptingRequestsChange","");

        String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";

        OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);

        ProviderTestOptions providerTestOptions = new ProviderTestOptions();

        ProviderTestClient providerClient = new ProviderTestClient(providerTestOptions);

        // Provider_1 provides the DIRECT_FEED service name
        OmmProvider ommprovider = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_1"), providerClient);

        assertNotNull(ommprovider);

        // Provider_1 provides the DIRECT_FEED service name
        OmmProvider ommprovider2 = EmaFactory.createOmmProvider(config.port("19004").providerName("Provider_1"), providerClient);

        assertNotNull(ommprovider2);

        OmmConsumer consumer = null;
        ConsumerTestClient consumerClient = new ConsumerTestClient();

        try
        {
            consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_9"));

            ReqMsg reqMsg = EmaFactory.createReqMsg();

            // Request info filter only
            long directoryHandle = consumer.registerClient(reqMsg.domainType(EmaRdm.MMT_DIRECTORY).filter(63), consumerClient);

            /* Waits until OmmConsumer receives the refresh message */
            while (consumerClient.queueSize() == 0) {}

            assertEquals(1, consumerClient.queueSize()); // Ensure that the callback receives only one directory message

            Msg message = consumerClient.popMessage();

            assertTrue(message instanceof RefreshMsg);

            RefreshMsg refreshMsg = (RefreshMsg)message;

            assertEquals(2, refreshMsg.streamId());
            assertEquals(DomainTypes.SOURCE, refreshMsg.domainType());
            assertEquals("Open / Ok / None / ''", refreshMsg.state().toString());
            assertTrue(refreshMsg.solicited());
            assertTrue(refreshMsg.complete());
            assertTrue(refreshMsg.hasMsgKey());
            assertEquals(63, refreshMsg.filter());
            assertEquals(DataType.DataTypes.MAP, refreshMsg.payload().dataType());

            Map payload = refreshMsg.payload().map();

            assertEquals(1, payload.size());

            Iterator<MapEntry> mapIt =  payload.iterator();

            assertTrue(mapIt.hasNext());

            MapEntry mapEntry = mapIt.next();

            long serviceId = 32767;

            assertEquals(serviceId, mapEntry.key().uintValue());

            assertEquals(DataType.DataTypes.FILTER_LIST, mapEntry.loadType());

            FilterList filterList = mapEntry.filterList();

            Iterator<FilterEntry> filterIt =  filterList.iterator();

            assertTrue(filterIt.hasNext());

            FilterEntry filterEntry = filterIt.next();

            assertEquals(FilterEntry.FilterAction.SET, filterEntry.action());
            assertEquals(EmaRdm.SERVICE_INFO_FILTER, filterEntry.filterId());
            assertEquals(DataType.DataTypes.ELEMENT_LIST, filterEntry.loadType());

            // Checks INFO filter
            ElementList elementList = filterEntry.elementList();

            Iterator<ElementEntry> elIt = elementList.iterator();
            assertTrue(elIt.hasNext());
            ElementEntry elementEntry = elIt.next();

            assertEquals(EmaRdm.ENAME_NAME, elementEntry.name());
            assertEquals("DIRECT_FEED", elementEntry.ascii().ascii());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_VENDOR, elementEntry.name());
            assertEquals("company name", elementEntry.ascii().ascii());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_IS_SOURCE, elementEntry.name());
            assertEquals(0, elementEntry.uintValue());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_CAPABILITIES, elementEntry.name());
            assertEquals(DataType.DataTypes.ARRAY, elementEntry.loadType());
            OmmArray ommArray = elementEntry.array();
            Iterator<OmmArrayEntry> arrayIt = ommArray.iterator();
            assertTrue(arrayIt.hasNext());
            OmmArrayEntry arrayEntry = arrayIt.next();

            assertEquals(DataType.DataTypes.UINT, arrayEntry.loadType());
            assertEquals(6, arrayEntry.uintValue());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals(7, arrayEntry.uintValue());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals(8, arrayEntry.uintValue());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals(5, arrayEntry.uintValue());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals(10, arrayEntry.uintValue());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals(200, arrayEntry.uintValue());
            assertFalse(arrayIt.hasNext());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_DICTIONARYS_PROVIDED, elementEntry.name());
            assertEquals(DataType.DataTypes.ARRAY, elementEntry.loadType());
            ommArray = elementEntry.array();
            arrayIt = ommArray.iterator();
            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();

            assertEquals(DataType.DataTypes.ASCII, arrayEntry.loadType());
            assertEquals("RWFFld", arrayEntry.ascii().ascii());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals("RWFEnum", arrayEntry.ascii().ascii());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_DICTIONARYS_USED, elementEntry.name());
            assertEquals(DataType.DataTypes.ARRAY, elementEntry.loadType());
            ommArray = elementEntry.array();
            arrayIt = ommArray.iterator();
            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();

            assertEquals(DataType.DataTypes.ASCII, arrayEntry.loadType());
            assertEquals("RWFFld", arrayEntry.ascii().ascii());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals("RWFEnum", arrayEntry.ascii().ascii());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_QOS, elementEntry.name());
            assertEquals(DataType.DataTypes.ARRAY, elementEntry.loadType());
            ommArray = elementEntry.array();
            arrayIt = ommArray.iterator();
            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();

            assertEquals(DataType.DataTypes.QOS, arrayEntry.loadType());
            assertEquals("RealTime/TickByTick", arrayEntry.qos().toString());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals("Timeliness: 100/Rate: 100", arrayEntry.qos().toString());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_SUPPS_QOS_RANGE, elementEntry.name());
            assertEquals(DataType.DataTypes.UINT, elementEntry.loadType());
            assertEquals(0, elementEntry.uintValue());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_ITEM_LIST, elementEntry.name());
            assertEquals(DataType.DataTypes.ASCII, elementEntry.loadType());
            assertEquals("#.itemlist", elementEntry.ascii().ascii());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_SUPPS_OOB_SNAPSHOTS, elementEntry.name());
            assertEquals(DataType.DataTypes.UINT, elementEntry.loadType());
            assertEquals(0, elementEntry.uintValue());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_ACCEPTING_CONS_STATUS, elementEntry.name());
            assertEquals(DataType.DataTypes.UINT, elementEntry.loadType());
            assertEquals(0, elementEntry.uintValue());

            assertFalse(elIt.hasNext());

            assertTrue(filterIt.hasNext());

            filterEntry = filterIt.next();

            assertEquals(FilterEntry.FilterAction.SET, filterEntry.action());
            assertEquals(EmaRdm.SERVICE_STATE_FILTER, filterEntry.filterId());
            assertEquals(DataType.DataTypes.ELEMENT_LIST, filterEntry.loadType());

            elementList = filterEntry.elementList();

            elIt = elementList.iterator();
            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();

            assertEquals(EmaRdm.ENAME_SVC_STATE, elementEntry.name());
            assertEquals(1, elementEntry.uintValue());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_ACCEPTING_REQS, elementEntry.name());
            assertEquals(1, elementEntry.uintValue());

            assertFalse(filterIt.hasNext());

            assertFalse(mapIt.hasNext());

            Thread.sleep(3000);

            // Change service's state to down from one provider
            ElementList serviceState = EmaFactory.createElementList();
            serviceState.add( EmaFactory.createElementEntry().uintValue( EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_UP ));
            serviceState.add( EmaFactory.createElementEntry().uintValue( EmaRdm.ENAME_ACCEPTING_REQS, 0 ));

            FilterList filterListEnc = EmaFactory.createFilterList();
            filterListEnc.add( EmaFactory.createFilterEntry().elementList( EmaRdm.SERVICE_STATE_ID, FilterEntry.FilterAction.UPDATE, serviceState ) );

            Map map = EmaFactory.createMap();
            map.add( EmaFactory.createMapEntry().keyUInt( 1, MapEntry.MapAction.UPDATE, filterListEnc ));

            UpdateMsg updateMsg = EmaFactory.createUpdateMsg();
            ommprovider.submit( updateMsg.domainType( EmaRdm.MMT_DIRECTORY ).
                    filter( EmaRdm.SERVICE_STATE_FILTER ).
                    payload( map ), 0);	// use 0 item handle to fan-out to all subscribers

            Thread.sleep(5000);

            /* No message in the queue as the service state is still up on another provider */
            assertEquals(0, consumerClient.queueSize());

            ommprovider2.submit( updateMsg.domainType( EmaRdm.MMT_DIRECTORY ).
                    filter( EmaRdm.SERVICE_STATE_FILTER ).
                    payload( map ), 0);	// use 0 item handle to fan-out to all subscribers

            Thread.sleep(5000);

            assertEquals(1, consumerClient.queueSize()); // Receives source directory update message to notify that the service state is down.

            message = consumerClient.popMessage();

            assertTrue(message instanceof UpdateMsg);

            updateMsg = (UpdateMsg)message;

            assertEquals(2, updateMsg.streamId());
            assertEquals(DomainTypes.SOURCE, updateMsg.domainType());
            assertTrue(updateMsg.hasMsgKey());
            assertEquals(63, updateMsg.filter());
            assertEquals(DataType.DataTypes.MAP, updateMsg.payload().dataType());

            payload = updateMsg.payload().map();

            assertEquals(1, payload.size());

            mapIt =  payload.iterator();

            assertTrue(mapIt.hasNext());

            mapEntry = mapIt.next();

            assertEquals(serviceId, mapEntry.key().uintValue());

            assertEquals(DataType.DataTypes.FILTER_LIST, mapEntry.loadType());

            filterList = mapEntry.filterList();

            filterIt =  filterList.iterator();

            assertTrue(filterIt.hasNext());

            filterEntry = filterIt.next();

            assertEquals(FilterEntry.FilterAction.SET, filterEntry.action());
            assertEquals(EmaRdm.SERVICE_INFO_FILTER, filterEntry.filterId());
            assertEquals(DataType.DataTypes.ELEMENT_LIST, filterEntry.loadType());

            // Checks INFO filter
            elementList = filterEntry.elementList();

            elIt = elementList.iterator();
            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();

            assertEquals(EmaRdm.ENAME_NAME, elementEntry.name());
            assertEquals("DIRECT_FEED", elementEntry.ascii().ascii());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_VENDOR, elementEntry.name());
            assertEquals("company name", elementEntry.ascii().ascii());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_IS_SOURCE, elementEntry.name());
            assertEquals(0, elementEntry.uintValue());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_CAPABILITIES, elementEntry.name());
            assertEquals(DataType.DataTypes.ARRAY, elementEntry.loadType());
            ommArray = elementEntry.array();
            arrayIt = ommArray.iterator();
            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();

            assertEquals(DataType.DataTypes.UINT, arrayEntry.loadType());
            assertEquals(6, arrayEntry.uintValue());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals(7, arrayEntry.uintValue());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals(8, arrayEntry.uintValue());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals(5, arrayEntry.uintValue());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals(10, arrayEntry.uintValue());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals(200, arrayEntry.uintValue());
            assertFalse(arrayIt.hasNext());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_DICTIONARYS_PROVIDED, elementEntry.name());
            assertEquals(DataType.DataTypes.ARRAY, elementEntry.loadType());
            ommArray = elementEntry.array();
            arrayIt = ommArray.iterator();
            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();

            assertEquals(DataType.DataTypes.ASCII, arrayEntry.loadType());
            assertEquals("RWFFld", arrayEntry.ascii().ascii());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals("RWFEnum", arrayEntry.ascii().ascii());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_DICTIONARYS_USED, elementEntry.name());
            assertEquals(DataType.DataTypes.ARRAY, elementEntry.loadType());
            ommArray = elementEntry.array();
            arrayIt = ommArray.iterator();
            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();

            assertEquals(DataType.DataTypes.ASCII, arrayEntry.loadType());
            assertEquals("RWFFld", arrayEntry.ascii().ascii());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals("RWFEnum", arrayEntry.ascii().ascii());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_QOS, elementEntry.name());
            assertEquals(DataType.DataTypes.ARRAY, elementEntry.loadType());
            ommArray = elementEntry.array();
            arrayIt = ommArray.iterator();
            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();

            assertEquals(DataType.DataTypes.QOS, arrayEntry.loadType());
            assertEquals("RealTime/TickByTick", arrayEntry.qos().toString());

            assertTrue(arrayIt.hasNext());
            arrayEntry = arrayIt.next();
            assertEquals("Timeliness: 100/Rate: 100", arrayEntry.qos().toString());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_SUPPS_QOS_RANGE, elementEntry.name());
            assertEquals(DataType.DataTypes.UINT, elementEntry.loadType());
            assertEquals(0, elementEntry.uintValue());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_ITEM_LIST, elementEntry.name());
            assertEquals(DataType.DataTypes.ASCII, elementEntry.loadType());
            assertEquals("#.itemlist", elementEntry.ascii().ascii());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_SUPPS_OOB_SNAPSHOTS, elementEntry.name());
            assertEquals(DataType.DataTypes.UINT, elementEntry.loadType());
            assertEquals(0, elementEntry.uintValue());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_ACCEPTING_CONS_STATUS, elementEntry.name());
            assertEquals(DataType.DataTypes.UINT, elementEntry.loadType());
            assertEquals(0, elementEntry.uintValue());

            assertFalse(elIt.hasNext());

            assertTrue(filterIt.hasNext());

            filterEntry = filterIt.next();

            assertEquals(FilterEntry.FilterAction.SET, filterEntry.action());
            assertEquals(EmaRdm.SERVICE_STATE_FILTER, filterEntry.filterId());
            assertEquals(DataType.DataTypes.ELEMENT_LIST, filterEntry.loadType());

            elementList = filterEntry.elementList();

            elIt = elementList.iterator();
            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();

            assertEquals(EmaRdm.ENAME_SVC_STATE, elementEntry.name());
            assertEquals(1, elementEntry.uintValue());

            assertTrue(elIt.hasNext());
            elementEntry = elIt.next();
            assertEquals(EmaRdm.ENAME_ACCEPTING_REQS, elementEntry.name());
            assertEquals(0, elementEntry.uintValue());

            assertFalse(filterIt.hasNext());

            assertFalse(mapIt.hasNext());

            consumer.unregister(directoryHandle);

            assertNotNull(consumer);
        }
        catch(OmmException ex)
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
    public void testMultiConnectionsReceiveDirectoryResponseOnlyOneConnection()
    {
        TestUtilities.printTestHead("testMultiConnectionsReceiveDirectoryResponseOnlyOneConnection","");

        String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";

        OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);

        /* Source directory refresh */
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

        ProviderTestOptions providerTestOptions = new ProviderTestOptions();

        ProviderTestClient providerClient = new ProviderTestClient(providerTestOptions);
        providerTestOptions.sourceDirectoryPayload = map;


        OmmProvider ommprovider = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_1")
                .adminControlDirectory(OmmIProviderConfig.AdminControl.USER_CONTROL), providerClient);

        assertNotNull(ommprovider);

        ProviderTestOptions providerTestOptions2 = new ProviderTestOptions();

        /* The second provider doesn't provide the source directory refresh within the DirectoryRequestTimeout value */
        ProviderTestClient providerClient2 = new ProviderTestClient(providerTestOptions2);
        OmmProvider ommprovider2 = EmaFactory.createOmmProvider(config.port("19004").providerName("Provider_1")
                .adminControlDirectory(OmmIProviderConfig.AdminControl.USER_CONTROL), providerClient2);

        assertNotNull(ommprovider2);

        OmmConsumer consumer = null;

        ConsumerTestOptions consumerTestOptions = new ConsumerTestOptions();
        consumerTestOptions.dumpDictionaryRefreshMsg = false;

        ConsumerTestClient consumerClient = new ConsumerTestClient(consumerTestOptions);

        try
        {
            consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_9"));

            ReqMsg reqMsg = EmaFactory.createReqMsg();

            // Request info filter only
            long directoryHandle = consumer.registerClient(reqMsg.domainType(EmaRdm.MMT_DIRECTORY).filter(63), consumerClient);

            Thread.sleep(1000);

            assertEquals(1, consumerClient.queueSize()); // Ensure that the callback receives only one directory message

            Msg message = consumerClient.popMessage();

            assertTrue(message instanceof RefreshMsg);

            RefreshMsg refreshMsg = (RefreshMsg)message;

            assertEquals(2, refreshMsg.streamId());
            assertEquals(DomainTypes.SOURCE, refreshMsg.domainType());
            assertEquals("Open / Ok / None / ''", refreshMsg.state().toString());
            assertTrue(refreshMsg.solicited());
            assertTrue(refreshMsg.complete());
            assertTrue(refreshMsg.hasMsgKey());
            assertEquals(63, refreshMsg.filter());
            assertEquals(DataType.DataTypes.MAP, refreshMsg.payload().dataType());

            consumer.unregister(directoryHandle);

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
    public void testMultiConnectionsSingleItemRequestByServiceId()
    {
        TestUtilities.printTestHead("testMultiConnectionsSingleItemRequestByServiceId","");

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
        ConsumerTestClient consumerClient = new ConsumerTestClient();

        try
        {
            consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_9"));

            ReqMsg reqMsg = EmaFactory.createReqMsg();

            // Request info filter only
            consumer.registerClient(reqMsg.domainType(EmaRdm.MMT_DIRECTORY).filter(63), consumerClient);

            /* Waits until OmmConsumer receives the refresh message */
            while(consumerClient.queueSize() == 0) {}

            assertEquals(1, consumerClient.queueSize()); // Ensure that the callback receives only one directory message

            Msg message = consumerClient.popMessage();

            assertTrue(message instanceof RefreshMsg);

            RefreshMsg refreshMsg = (RefreshMsg)message;

            assertEquals(2, refreshMsg.streamId());
            assertEquals(DomainTypes.SOURCE, refreshMsg.domainType());
            assertEquals("Open / Ok / None / ''", refreshMsg.state().toString());
            assertTrue(refreshMsg.solicited());
            assertTrue(refreshMsg.complete());
            assertTrue(refreshMsg.hasMsgKey());
            assertEquals(63, refreshMsg.filter());
            assertEquals(DataType.DataTypes.MAP, refreshMsg.payload().dataType());

            Map payload = refreshMsg.payload().map();

            assertEquals(2, payload.size());

            Iterator<MapEntry> mapIt =  payload.iterator();

            assertTrue(mapIt.hasNext());

            MapEntry mapEntry = mapIt.next();

            long serviceId = mapEntry.key().uintValue();

            assertEquals(DataType.DataTypes.FILTER_LIST, mapEntry.loadType());

            FilterList filterList = mapEntry.filterList();

            Iterator<FilterEntry> filterIt =  filterList.iterator();

            assertTrue(filterIt.hasNext());

            FilterEntry filterEntry = filterIt.next();

            assertEquals(FilterEntry.FilterAction.SET, filterEntry.action());
            assertEquals(EmaRdm.SERVICE_INFO_FILTER, filterEntry.filterId());
            assertEquals(DataType.DataTypes.ELEMENT_LIST, filterEntry.loadType());

            // Checks INFO filter
            ElementList elementList = filterEntry.elementList();

            Iterator<ElementEntry> elIt = elementList.iterator();
            assertTrue(elIt.hasNext());
            ElementEntry elementEntry = elIt.next();

            assertEquals(EmaRdm.ENAME_NAME, elementEntry.name());
            String serviceName = elementEntry.ascii().ascii();

            int direct_feed_serviceId;
            int direct_feed2_serviceId;

            if(serviceName.equals("DIRECT_FEED"))
            {
                direct_feed_serviceId = (int)serviceId;
                direct_feed2_serviceId = direct_feed_serviceId + 1;
            }
            else
            {
                direct_feed2_serviceId = (int)serviceId;
                direct_feed_serviceId = direct_feed2_serviceId + 1;
            }


            long itemHandle = consumer.registerClient(reqMsg.clear().serviceId(direct_feed_serviceId).name("LSEG.O"), consumerClient);

            Thread.sleep(2000);

            assertEquals(1, providerClient1.queueSize());

            message = providerClient1.popMessage();

            assertTrue(message instanceof ReqMsg);

            ReqMsg requestMsg = (ReqMsg)message;

            assertEquals("DIRECT_FEED", requestMsg.serviceName());
            assertEquals("LSEG.O", requestMsg.name());
            assertEquals(1, requestMsg.serviceId());

            Thread.sleep(2000);

            assertEquals(2, consumerClient.queueSize());

            message = consumerClient.popMessage();

            refreshMsg = (RefreshMsg)message;

            assertEquals("DIRECT_FEED", refreshMsg.serviceName());
            assertEquals("LSEG.O", refreshMsg.name());
            assertEquals(direct_feed_serviceId, refreshMsg.serviceId());

            message = consumerClient.popMessage();

            UpdateMsg updateMsg = (UpdateMsg)message;

            assertEquals("DIRECT_FEED", updateMsg.serviceName());
            assertEquals("LSEG.O", updateMsg.name());
            assertEquals(direct_feed_serviceId, updateMsg.serviceId());

            long itemHandle2 = consumer.registerClient(reqMsg.clear().serviceId(direct_feed2_serviceId).name("LSEG.O"), consumerClient);

            Thread.sleep(2000);

            assertEquals(1, providerClient2.queueSize());

            message = providerClient2.popMessage();

            assertTrue(message instanceof ReqMsg);

            requestMsg = (ReqMsg)message;

            assertEquals("DIRECT_FEED_2", requestMsg.serviceName());
            assertEquals("LSEG.O", requestMsg.name());
            assertEquals(1, requestMsg.serviceId());

            Thread.sleep(2000);

            assertEquals(2, consumerClient.queueSize());

            message = consumerClient.popMessage();

            refreshMsg = (RefreshMsg)message;

            assertEquals("DIRECT_FEED_2", refreshMsg.serviceName());
            assertEquals("LSEG.O", refreshMsg.name());
            assertEquals(direct_feed2_serviceId, refreshMsg.serviceId());

            message = consumerClient.popMessage();

            updateMsg = (UpdateMsg)message;

            assertEquals("DIRECT_FEED_2", updateMsg.serviceName());
            assertEquals("LSEG.O", updateMsg.name());
            assertEquals(direct_feed2_serviceId, updateMsg.serviceId());

            consumer.unregister(itemHandle);
            consumer.unregister(itemHandle2);

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
    public void testMultiConnectionsForLoadingDictionaryFromNetworkAndSubscribeDictioanryStream()
    {
        TestUtilities.printTestHead("testMultiConnectionsForLoadingDictionaryFromNetworkAndSubscribeDictioanryStream","");

        String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";

        OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);

        ProviderTestOptions providerTestOptions = new ProviderTestOptions();

        ProviderTestClient providerClient = new ProviderTestClient(providerTestOptions);

        // Provider_1 provides the DIRECT_FEED service name
        OmmProvider ommprovider = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_1")
                .adminControlDictionary(OmmIProviderConfig.AdminControl.USER_CONTROL), providerClient);

        assertNotNull(ommprovider);

        // Provider_1 provides the DIRECT_FEED service name
        OmmProvider ommprovider2 = EmaFactory.createOmmProvider(config.port("19004").providerName("Provider_1")
                .adminControlDictionary(OmmIProviderConfig.AdminControl.USER_CONTROL), providerClient);

        assertNotNull(ommprovider2);

        OmmConsumer consumer = null;

        ConsumerTestOptions consumerTestOptions = new ConsumerTestOptions();

        consumerTestOptions.dumpDictionaryRefreshMsg = false;

        ConsumerTestClient consumerClient = new ConsumerTestClient(consumerTestOptions);

        try
        {
            /* Consumer_11 to download dictionary from network when the first service available. */
            consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_11"));

            /* Provider receives two dictionary request from OmmConsumer */
            assertEquals(2, providerClient.queueSize());

            Msg message = providerClient.popMessage();

            assertTrue(message instanceof ReqMsg);

            ReqMsg requestMsg = (ReqMsg)message;

            assertEquals("DIRECT_FEED", requestMsg.serviceName());
            assertEquals("RWFFld", requestMsg.name());
            assertEquals(1, requestMsg.serviceId());
            assertEquals(EmaRdm.DICTIONARY_NORMAL, requestMsg.filter());

            message = providerClient.popMessage();

            assertTrue(message instanceof ReqMsg);

            requestMsg = (ReqMsg)message;

            assertEquals("DIRECT_FEED", requestMsg.serviceName());
            assertEquals("RWFEnum", requestMsg.name());
            assertEquals(1, requestMsg.serviceId());
            assertEquals(EmaRdm.DICTIONARY_NORMAL, requestMsg.filter());

            ReqMsg reqMsg = EmaFactory.createReqMsg();

            long rwfFldHandle = consumer.registerClient(reqMsg.domainType(EmaRdm.MMT_DICTIONARY).name("RWFFld").
                    filter(EmaRdm.DICTIONARY_NORMAL), consumerClient);

            long rwfEnumHandle = consumer.registerClient(reqMsg.domainType(EmaRdm.MMT_DICTIONARY).name("RWFEnum").
                    filter(EmaRdm.DICTIONARY_NORMAL), consumerClient);

            Thread.sleep(2000);

            assertEquals(3, consumerClient.queueSize());

            message = consumerClient.popMessage();

            RefreshMsg refreshMsg = (RefreshMsg)message;

            /* Receives first partial refresh from EMA */
            assertEquals("RWFFld", refreshMsg.name());
            assertFalse(refreshMsg.complete());
            assertTrue(refreshMsg.clearCache());

            message = consumerClient.popMessage();

            refreshMsg = (RefreshMsg)message;

            /* Receives complete refresh from EMA */
            assertEquals("RWFEnum", refreshMsg.name());
            assertTrue(refreshMsg.complete());
            assertTrue(refreshMsg.clearCache());

            message = consumerClient.popMessage();

            refreshMsg = (RefreshMsg)message;

            /* Receives second partial refresh from EMA */
            assertEquals("RWFFld", refreshMsg.name());
            assertTrue(refreshMsg.complete());
            assertFalse(refreshMsg.clearCache());

            consumer.unregister(rwfFldHandle);
            consumer.unregister(rwfEnumHandle);

            assertNotNull(consumer);
        }
        catch(OmmException ex)
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
    public void testMultiConnectionsForLoadingDictionaryFromNetworkAndSubscribeDictioanryStreamByServiceName()
    {
        TestUtilities.printTestHead("testMultiConnectionsForLoadingDictionaryFromNetworkAndSubscribeDictioanryStreamByServiceName","");

        String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";

        OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);

        ProviderTestOptions providerTestOptions = new ProviderTestOptions();

        ProviderTestClient providerClient = new ProviderTestClient(providerTestOptions);

        // Provider_1 provides the DIRECT_FEED service name
        OmmProvider ommprovider = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_1")
                .adminControlDictionary(OmmIProviderConfig.AdminControl.USER_CONTROL), providerClient);

        assertNotNull(ommprovider);

        // Provider_1 provides the DIRECT_FEED service name
        OmmProvider ommprovider2 = EmaFactory.createOmmProvider(config.port("19004").providerName("Provider_1")
                .adminControlDictionary(OmmIProviderConfig.AdminControl.USER_CONTROL), providerClient);

        assertNotNull(ommprovider2);

        OmmConsumer consumer = null;

        ConsumerTestOptions consumerTestOptions = new ConsumerTestOptions();

        consumerTestOptions.dumpDictionaryRefreshMsg = false;

        ConsumerTestClient consumerClient = new ConsumerTestClient(consumerTestOptions);

        try
        {
            /* Consumer_11 to download dictionary from network when the first service available. */
            consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_11"));

            /* Provider receives two dictionary request from OmmConsumer */
            assertEquals(2, providerClient.queueSize());

            Msg message = providerClient.popMessage();

            assertTrue(message instanceof ReqMsg);

            ReqMsg requestMsg = (ReqMsg)message;

            assertEquals("DIRECT_FEED", requestMsg.serviceName());
            assertEquals("RWFFld", requestMsg.name());
            assertEquals(1, requestMsg.serviceId());
            assertEquals(EmaRdm.DICTIONARY_NORMAL, requestMsg.filter());

            message = providerClient.popMessage();

            assertTrue(message instanceof ReqMsg);

            requestMsg = (ReqMsg)message;

            assertEquals("DIRECT_FEED", requestMsg.serviceName());
            assertEquals("RWFEnum", requestMsg.name());
            assertEquals(1, requestMsg.serviceId());
            assertEquals(EmaRdm.DICTIONARY_NORMAL, requestMsg.filter());

            ReqMsg reqMsg = EmaFactory.createReqMsg();

            long rwfFldHandle = consumer.registerClient(reqMsg.domainType(EmaRdm.MMT_DICTIONARY).name("RWFFld").serviceName("DIRECT_FEED").
                    filter(EmaRdm.DICTIONARY_NORMAL), consumerClient);

            long rwfEnumHandle = consumer.registerClient(reqMsg.clear().domainType(EmaRdm.MMT_DICTIONARY).name("RWFEnum").serviceName("DIRECT_FEED").
                    filter(EmaRdm.DICTIONARY_NORMAL), consumerClient);

            Thread.sleep(5000);

            assertEquals(2, consumerClient.queueSize());

            message = consumerClient.popMessage();

            RefreshMsg refreshMsg = (RefreshMsg)message;

            /* Receives complete refresh from EMA */
            assertEquals("RWFFld", refreshMsg.name());
            assertTrue(refreshMsg.complete());
            assertTrue(refreshMsg.clearCache());

            message = consumerClient.popMessage();

            refreshMsg = (RefreshMsg)message;

            /* Receives complete refresh from EMA */
            assertEquals("RWFEnum", refreshMsg.name());
            assertTrue(refreshMsg.complete());
            assertTrue(refreshMsg.clearCache());

            consumer.unregister(rwfFldHandle);
            consumer.unregister(rwfEnumHandle);

            assertNotNull(consumer);
        }
        catch(OmmException ex)
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
    public void testMultiConnectionsWithSymbollistRequest()
    {
        TestUtilities.printTestHead("testMultiConnectionsWithSymbollistRequest","");

        String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";

        OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);

        ProviderTestOptions providerTestOptions = new ProviderTestOptions();

        ProviderTestClient providerClient1 = new ProviderTestClient(providerTestOptions);

        providerTestOptions.sendUpdateMessage = false;

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

            ReqMsg reqMsg = EmaFactory.createReqMsg();

            long itemHandle = consumer.registerClient(reqMsg.clear().serviceName("DIRECT_FEED_2").domainType(EmaRdm.MMT_SYMBOL_LIST).name(".AV.N"), consumerClient);

            Thread.sleep(2000);

            ConsumerSession<OmmConsumerClient> consumerSession = ((OmmConsumerImpl)consumer).consumerSession();

            int serviceId = consumerSession.sessionDirectoryByName("DIRECT_FEED_2").service().serviceId();

            assertEquals(0, providerClient1.queueSize());

            assertEquals(1, providerClient2.queueSize());

            Msg message = providerClient2.popMessage();

            assertTrue(message instanceof ReqMsg);

            ReqMsg requestMsg = (ReqMsg)message;

            assertEquals("DIRECT_FEED_2", requestMsg.serviceName());
            assertEquals(".AV.N", requestMsg.name());
            assertEquals(1, requestMsg.serviceId());
            assertEquals(EmaRdm.MMT_SYMBOL_LIST, requestMsg.domainType());

            Thread.sleep(2000);

            assertEquals(1, consumerClient.queueSize());

            message = consumerClient.popMessage();

            RefreshMsg refreshMsg = (RefreshMsg)message;

            assertEquals("DIRECT_FEED_2", refreshMsg.serviceName());
            assertEquals(".AV.N", refreshMsg.name());
            assertEquals(serviceId, refreshMsg.serviceId());
            assertEquals(DataType.DataTypes.MAP, refreshMsg.payload().dataType());

            Map map = refreshMsg.payload().map();

            assertTrue(map.hasTotalCountHint());
            assertEquals(3, map.totalCountHint());
            assertEquals(DataType.DataTypes.FIELD_LIST,map.summaryData().dataType());

            Iterator<MapEntry> it = map.iterator();

            assertTrue(it.hasNext());
            MapEntry entry = it.next();
            assertEquals("6974 656D 41                                    itemA", entry.key().buffer().toString());

            assertTrue(it.hasNext());
            entry = it.next();
            assertEquals("6974 656D 42                                    itemB", entry.key().buffer().toString());

            assertTrue(it.hasNext());
            entry = it.next();
            assertEquals("6974 656D 43                                    itemC", entry.key().buffer().toString());

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
    public void testMultiConnectionsSingleItemRequestByServiceName()
    {
        TestUtilities.printTestHead("testMultiConnectionsSingleItemRequestByServiceName","");

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
        ConsumerTestClient consumerClient = new ConsumerTestClient();

        try
        {
            consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_9"));

            ReqMsg reqMsg = EmaFactory.createReqMsg();

            // Request info filter only
            consumer.registerClient(reqMsg.domainType(EmaRdm.MMT_DIRECTORY).filter(63), consumerClient);

            /* Waits until OmmConsumer receives the refresh message */
            while(consumerClient.queueSize() == 0) {}

            assertEquals(1, consumerClient.queueSize()); // Ensure that the callback receives only one directory message

            Msg message = consumerClient.popMessage();

            assertTrue(message instanceof RefreshMsg);

            RefreshMsg refreshMsg = (RefreshMsg)message;

            assertEquals(2, refreshMsg.streamId());
            assertEquals(DomainTypes.SOURCE, refreshMsg.domainType());
            assertEquals("Open / Ok / None / ''", refreshMsg.state().toString());
            assertTrue(refreshMsg.solicited());
            assertTrue(refreshMsg.complete());
            assertTrue(refreshMsg.hasMsgKey());
            assertEquals(63, refreshMsg.filter());
            assertEquals(DataType.DataTypes.MAP, refreshMsg.payload().dataType());

            Map payload = refreshMsg.payload().map();

            assertEquals(2, payload.size());

            Iterator<MapEntry> mapIt =  payload.iterator();

            assertTrue(mapIt.hasNext());

            MapEntry mapEntry = mapIt.next();

            long serviceId = mapEntry.key().uintValue();

            assertEquals(DataType.DataTypes.FILTER_LIST, mapEntry.loadType());

            FilterList filterList = mapEntry.filterList();

            Iterator<FilterEntry> filterIt =  filterList.iterator();

            assertTrue(filterIt.hasNext());

            FilterEntry filterEntry = filterIt.next();

            assertEquals(FilterEntry.FilterAction.SET, filterEntry.action());
            assertEquals(EmaRdm.SERVICE_INFO_FILTER, filterEntry.filterId());
            assertEquals(DataType.DataTypes.ELEMENT_LIST, filterEntry.loadType());

            // Checks INFO filter
            ElementList elementList = filterEntry.elementList();

            Iterator<ElementEntry> elIt = elementList.iterator();
            assertTrue(elIt.hasNext());
            ElementEntry elementEntry = elIt.next();

            assertEquals(EmaRdm.ENAME_NAME, elementEntry.name());
            String serviceName = elementEntry.ascii().ascii();

            int direct_feed_serviceId;
            int direct_feed2_serviceId;

            if(serviceName.equals("DIRECT_FEED"))
            {
                direct_feed_serviceId = (int)serviceId;
                direct_feed2_serviceId = direct_feed_serviceId + 1;
            }
            else
            {
                direct_feed2_serviceId = (int)serviceId;
                direct_feed_serviceId = direct_feed2_serviceId + 1;
            }


            long itemHandle = consumer.registerClient(reqMsg.clear().serviceName("DIRECT_FEED").name("LSEG.O"), consumerClient);

            Thread.sleep(2000);

            assertEquals(1, providerClient1.queueSize());

            message = providerClient1.popMessage();

            assertTrue(message instanceof ReqMsg);

            ReqMsg requestMsg = (ReqMsg)message;

            assertEquals("DIRECT_FEED", requestMsg.serviceName());
            assertEquals("LSEG.O", requestMsg.name());
            assertEquals(1, requestMsg.serviceId());

            Thread.sleep(2000);

            assertEquals(2, consumerClient.queueSize());

            message = consumerClient.popMessage();

            refreshMsg = (RefreshMsg)message;

            assertEquals("DIRECT_FEED", refreshMsg.serviceName());
            assertEquals("LSEG.O", refreshMsg.name());
            assertEquals(direct_feed_serviceId, refreshMsg.serviceId());

            message = consumerClient.popMessage();

            UpdateMsg updateMsg = (UpdateMsg)message;

            assertEquals("DIRECT_FEED", updateMsg.serviceName());
            assertEquals("LSEG.O", updateMsg.name());
            assertEquals(direct_feed_serviceId, updateMsg.serviceId());

            long itemHandle2 = consumer.registerClient(reqMsg.clear().serviceName("DIRECT_FEED_2").name("LSEG.O"), consumerClient);

            Thread.sleep(2000);

            assertEquals(1, providerClient2.queueSize());

            message = providerClient2.popMessage();

            assertTrue(message instanceof ReqMsg);

            requestMsg = (ReqMsg)message;

            assertEquals("DIRECT_FEED_2", requestMsg.serviceName());
            assertEquals("LSEG.O", requestMsg.name());
            assertEquals(1, requestMsg.serviceId());

            Thread.sleep(2000);

            assertEquals(2, consumerClient.queueSize());

            message = consumerClient.popMessage();

            refreshMsg = (RefreshMsg)message;

            assertEquals("DIRECT_FEED_2", refreshMsg.serviceName());
            assertEquals("LSEG.O", refreshMsg.name());
            assertEquals(direct_feed2_serviceId, refreshMsg.serviceId());

            message = consumerClient.popMessage();

            updateMsg = (UpdateMsg)message;

            assertEquals("DIRECT_FEED_2", updateMsg.serviceName());
            assertEquals("LSEG.O", updateMsg.name());
            assertEquals(direct_feed2_serviceId, updateMsg.serviceId());

            consumer.unregister(itemHandle);
            consumer.unregister(itemHandle2);

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
    public void testMultiConnectionsBatchItemRequestByServiceName()
    {
        TestUtilities.printTestHead("testMultiConnectionsBatchItemRequestByServiceName","");

        String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";

        OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);

        ProviderTestOptions providerTestOptions = new ProviderTestOptions();

        ProviderTestClient providerClient1 = new ProviderTestClient(providerTestOptions);

        providerTestOptions.sendUpdateMessage = false;

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

            consumerClient.consumer(consumer);

            ElementList batch = EmaFactory.createElementList();
            OmmArray array = EmaFactory.createOmmArray();

            array.add(EmaFactory.createOmmArrayEntry().ascii("itemA"));
            array.add(EmaFactory.createOmmArrayEntry().ascii("itemB"));
            array.add(EmaFactory.createOmmArrayEntry().ascii("itemC"));

            batch.add(EmaFactory.createElementEntry().array(EmaRdm.ENAME_BATCH_ITEM_LIST, array));

            ReqMsg reqMsg = EmaFactory.createReqMsg();

            consumer.registerClient(reqMsg.clear().serviceName("DIRECT_FEED").payload(batch), consumerClient);

            Thread.sleep(2000);

            assertEquals(3, providerClient1.queueSize());

            Msg message = providerClient1.popMessage();

            assertTrue(message instanceof ReqMsg);

            ReqMsg requestMsg = (ReqMsg)message;

            assertEquals(3, requestMsg.streamId());
            assertEquals("DIRECT_FEED", requestMsg.serviceName());
            assertEquals("itemA", requestMsg.name());
            assertEquals(1, requestMsg.serviceId());

            message = providerClient1.popMessage();

            assertTrue(message instanceof ReqMsg);

            requestMsg = (ReqMsg)message;

            assertEquals(4, requestMsg.streamId());
            assertEquals("DIRECT_FEED", requestMsg.serviceName());
            assertEquals("itemB", requestMsg.name());
            assertEquals(1, requestMsg.serviceId());

            message = providerClient1.popMessage();

            assertTrue(message instanceof ReqMsg);

            requestMsg = (ReqMsg)message;

            assertEquals(5, requestMsg.streamId());
            assertEquals("DIRECT_FEED", requestMsg.serviceName());
            assertEquals("itemC", requestMsg.name());
            assertEquals(1, requestMsg.serviceId());

            Thread.sleep(2000);

            assertEquals(4, consumerClient.queueSize());

            message = consumerClient.popMessage();

            assertTrue(message instanceof StatusMsg);

            StatusMsg statusMsg = (StatusMsg)message;

            assertEquals(5, statusMsg.streamId());
            assertFalse(statusMsg.hasName());
            assertEquals("DIRECT_FEED", statusMsg.serviceName());
            assertEquals(32767, statusMsg.serviceId());
            assertEquals(OmmState.StreamState.CLOSED, statusMsg.state().streamState());
            assertEquals(OmmState.DataState.OK, statusMsg.state().dataState());
            assertEquals(OmmState.StatusCode.NONE, statusMsg.state().statusCode());
            assertEquals("Stream closed for batch", statusMsg.state().statusText());

            message = consumerClient.popMessage();

            assertTrue(message instanceof RefreshMsg);

            RefreshMsg refreshMsg = (RefreshMsg)message;

            assertEquals(6, refreshMsg.streamId());
            assertEquals("DIRECT_FEED", refreshMsg.serviceName());
            assertEquals("itemA", refreshMsg.name());
            assertEquals(32767, refreshMsg.serviceId());
            assertEquals(OmmState.StreamState.OPEN, refreshMsg.state().streamState());
            assertEquals(OmmState.DataState.OK, refreshMsg.state().dataState());
            assertEquals(OmmState.StatusCode.NONE, refreshMsg.state().code());
            assertTrue(refreshMsg.solicited());
            assertTrue(refreshMsg.complete());

            message = consumerClient.popMessage();

            assertTrue(message instanceof RefreshMsg);

            refreshMsg = (RefreshMsg)message;

            assertEquals(7, refreshMsg.streamId());
            assertEquals("DIRECT_FEED", refreshMsg.serviceName());
            assertEquals("itemB", refreshMsg.name());
            assertEquals(32767, refreshMsg.serviceId());
            assertEquals(OmmState.StreamState.OPEN, refreshMsg.state().streamState());
            assertEquals(OmmState.DataState.OK, refreshMsg.state().dataState());
            assertEquals(OmmState.StatusCode.NONE, refreshMsg.state().code());
            assertTrue(refreshMsg.solicited());
            assertTrue(refreshMsg.complete());

            message = consumerClient.popMessage();

            assertTrue(message instanceof RefreshMsg);

            refreshMsg = (RefreshMsg)message;

            assertEquals(8, refreshMsg.streamId());
            assertEquals("DIRECT_FEED", refreshMsg.serviceName());
            assertEquals("itemC", refreshMsg.name());
            assertEquals(32767, refreshMsg.serviceId());
            assertEquals(OmmState.StreamState.OPEN, refreshMsg.state().streamState());
            assertEquals(OmmState.DataState.OK, refreshMsg.state().dataState());
            assertEquals(OmmState.StatusCode.NONE, refreshMsg.state().code());
            assertTrue(refreshMsg.solicited());
            assertTrue(refreshMsg.complete());

            /* Closes the first provider to force recovering items */
            ommprovider.uninitialize();

            Thread.sleep(2000);

            assertEquals(3, providerClient2.queueSize());

            Thread.sleep(1000);

            message = providerClient2.popMessage();

            assertTrue(message instanceof ReqMsg);

            requestMsg = (ReqMsg)message;

            assertEquals(3, requestMsg.streamId());
            assertEquals("DIRECT_FEED", requestMsg.serviceName());
            assertEquals("itemA", requestMsg.name());
            assertEquals(1, requestMsg.serviceId());

            message = providerClient2.popMessage();

            assertTrue(message instanceof ReqMsg);

            requestMsg = (ReqMsg)message;

            assertEquals(4, requestMsg.streamId());
            assertEquals("DIRECT_FEED", requestMsg.serviceName());
            assertEquals("itemB", requestMsg.name());
            assertEquals(1, requestMsg.serviceId());

            message = providerClient2.popMessage();

            assertTrue(message instanceof ReqMsg);

            requestMsg = (ReqMsg)message;

            assertEquals(5, requestMsg.streamId());
            assertEquals("DIRECT_FEED", requestMsg.serviceName());
            assertEquals("itemC", requestMsg.name());
            assertEquals(1, requestMsg.serviceId());

            assertEquals(6, consumerClient.queueSize());

            message = consumerClient.popMessage();

            statusMsg = (StatusMsg)message;
            assertEquals(6, statusMsg.streamId());
            assertEquals("DIRECT_FEED", statusMsg.serviceName());
            assertEquals("itemA", statusMsg.name());
            assertEquals(OmmState.StreamState.OPEN, statusMsg.state().streamState());
            assertEquals(OmmState.DataState.SUSPECT, statusMsg.state().dataState());
            assertEquals(OmmState.StatusCode.NONE, statusMsg.state().statusCode());
            assertEquals("channel down.", statusMsg.state().statusText());

            message = consumerClient.popMessage();

            statusMsg = (StatusMsg)message;
            assertEquals(7, statusMsg.streamId());
            assertEquals("DIRECT_FEED", statusMsg.serviceName());
            assertEquals("itemB", statusMsg.name());
            assertEquals(OmmState.StreamState.OPEN, statusMsg.state().streamState());
            assertEquals(OmmState.DataState.SUSPECT, statusMsg.state().dataState());
            assertEquals(OmmState.StatusCode.NONE, statusMsg.state().statusCode());
            assertEquals("channel down.", statusMsg.state().statusText());

            message = consumerClient.popMessage();

            statusMsg = (StatusMsg)message;
            assertEquals(8, statusMsg.streamId());
            assertEquals("DIRECT_FEED", statusMsg.serviceName());
            assertEquals("itemC", statusMsg.name());
            assertEquals(OmmState.StreamState.OPEN, statusMsg.state().streamState());
            assertEquals(OmmState.DataState.SUSPECT, statusMsg.state().dataState());
            assertEquals(OmmState.StatusCode.NONE, statusMsg.state().statusCode());
            assertEquals("channel down.", statusMsg.state().statusText());

            message = consumerClient.popMessage();

            assertTrue(message instanceof RefreshMsg);

            refreshMsg = (RefreshMsg)message;

            assertEquals(6, refreshMsg.streamId());
            assertEquals("DIRECT_FEED", refreshMsg.serviceName());
            assertEquals("itemA", refreshMsg.name());
            assertEquals(32767, refreshMsg.serviceId());
            assertEquals(OmmState.StreamState.OPEN, refreshMsg.state().streamState());
            assertEquals(OmmState.DataState.OK, refreshMsg.state().dataState());
            assertEquals(OmmState.StatusCode.NONE, refreshMsg.state().code());
            assertTrue(refreshMsg.solicited());
            assertTrue(refreshMsg.complete());

            message = consumerClient.popMessage();

            assertTrue(message instanceof RefreshMsg);

            refreshMsg = (RefreshMsg)message;

            assertEquals(7, refreshMsg.streamId());
            assertEquals("DIRECT_FEED", refreshMsg.serviceName());
            assertEquals("itemB", refreshMsg.name());
            assertEquals(32767, refreshMsg.serviceId());
            assertEquals(OmmState.StreamState.OPEN, refreshMsg.state().streamState());
            assertEquals(OmmState.DataState.OK, refreshMsg.state().dataState());
            assertEquals(OmmState.StatusCode.NONE, refreshMsg.state().code());
            assertTrue(refreshMsg.solicited());
            assertTrue(refreshMsg.complete());

            message = consumerClient.popMessage();

            assertTrue(message instanceof RefreshMsg);

            refreshMsg = (RefreshMsg)message;

            assertEquals(8, refreshMsg.streamId());
            assertEquals("DIRECT_FEED", refreshMsg.serviceName());
            assertEquals("itemC", refreshMsg.name());
            assertEquals(32767, refreshMsg.serviceId());
            assertEquals(OmmState.StreamState.OPEN, refreshMsg.state().streamState());
            assertEquals(OmmState.DataState.OK, refreshMsg.state().dataState());
            assertEquals(OmmState.StatusCode.NONE, refreshMsg.state().code());
            assertTrue(refreshMsg.solicited());
            assertTrue(refreshMsg.complete());

            consumerClient.unregisterAllHandles();

            Thread.sleep(1000);

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
    public void testMultiConnectionsBatchItemRequestByServiceList()
    {
        TestUtilities.printTestHead("testMultiConnectionsBatchItemRequestByServiceList","");

        String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";

        OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);

        ProviderTestOptions providerTestOptions = new ProviderTestOptions();

        ProviderTestClient providerClient1 = new ProviderTestClient(providerTestOptions);

        providerTestOptions.sendUpdateMessage = false;

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

            consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).addServiceList(serviceList).consumerName("Consumer_9"));

            ElementList batch = EmaFactory.createElementList();
            OmmArray array = EmaFactory.createOmmArray();

            array.add(EmaFactory.createOmmArrayEntry().ascii("itemA"));
            array.add(EmaFactory.createOmmArrayEntry().ascii("itemB"));
            array.add(EmaFactory.createOmmArrayEntry().ascii("itemC"));

            batch.add(EmaFactory.createElementEntry().array(EmaRdm.ENAME_BATCH_ITEM_LIST, array));

            ReqMsg reqMsg = EmaFactory.createReqMsg();

            consumer.registerClient(reqMsg.serviceListName("SVG1") .payload(batch), consumerClient);

            Thread.sleep(2000);

            ProviderTestClient providerClient = providerClient1.queueSize() > 0 ? providerClient1 : providerClient2;

            assertEquals(3, providerClient.queueSize());

            Msg message = providerClient.popMessage();

            assertTrue(message instanceof ReqMsg);

            ReqMsg requestMsg = (ReqMsg)message;

            assertEquals(3, requestMsg.streamId());
            assertEquals("DIRECT_FEED", requestMsg.serviceName());
            assertEquals("itemA", requestMsg.name());
            assertEquals(1, requestMsg.serviceId());

            message = providerClient.popMessage();

            assertTrue(message instanceof ReqMsg);

            requestMsg = (ReqMsg)message;

            assertEquals(4, requestMsg.streamId());
            assertEquals("DIRECT_FEED", requestMsg.serviceName());
            assertEquals("itemB", requestMsg.name());
            assertEquals(1, requestMsg.serviceId());

            message = providerClient.popMessage();

            assertTrue(message instanceof ReqMsg);

            requestMsg = (ReqMsg)message;

            assertEquals(5, requestMsg.streamId());
            assertEquals("DIRECT_FEED", requestMsg.serviceName());
            assertEquals("itemC", requestMsg.name());
            assertEquals(1, requestMsg.serviceId());

            Thread.sleep(2000);

            assertEquals(4, consumerClient.queueSize());

            message = consumerClient.popMessage();

            assertTrue(message instanceof StatusMsg);

            StatusMsg statusMsg = (StatusMsg)message;

            assertFalse(statusMsg.hasName());
            assertEquals("SVG1", statusMsg.serviceName());
            assertEquals(32767, statusMsg.serviceId());
            assertEquals(OmmState.StreamState.CLOSED, statusMsg.state().streamState());
            assertEquals(OmmState.DataState.OK, statusMsg.state().dataState());
            assertEquals(OmmState.StatusCode.NONE, statusMsg.state().statusCode());
            assertEquals("Stream closed for batch", statusMsg.state().statusText());

            message = consumerClient.popMessage();

            assertTrue(message instanceof RefreshMsg);

            RefreshMsg refreshMsg = (RefreshMsg)message;

            assertEquals("SVG1", refreshMsg.serviceName());
            assertEquals("itemA", refreshMsg.name());
            assertEquals(32767, refreshMsg.serviceId());
            assertEquals(OmmState.StreamState.OPEN, refreshMsg.state().streamState());
            assertEquals(OmmState.DataState.OK, refreshMsg.state().dataState());
            assertEquals(OmmState.StatusCode.NONE, refreshMsg.state().code());
            assertTrue(refreshMsg.solicited());
            assertTrue(refreshMsg.complete());

            message = consumerClient.popMessage();

            assertTrue(message instanceof RefreshMsg);

            refreshMsg = (RefreshMsg)message;

            assertEquals("SVG1", refreshMsg.serviceName());
            assertEquals("itemB", refreshMsg.name());
            assertEquals(32767, refreshMsg.serviceId());
            assertEquals(OmmState.StreamState.OPEN, refreshMsg.state().streamState());
            assertEquals(OmmState.DataState.OK, refreshMsg.state().dataState());
            assertEquals(OmmState.StatusCode.NONE, refreshMsg.state().code());
            assertTrue(refreshMsg.solicited());
            assertTrue(refreshMsg.complete());

            message = consumerClient.popMessage();

            assertTrue(message instanceof RefreshMsg);

            refreshMsg = (RefreshMsg)message;

            assertEquals("SVG1", refreshMsg.serviceName());
            assertEquals("itemC", refreshMsg.name());
            assertEquals(32767, refreshMsg.serviceId());
            assertEquals(OmmState.StreamState.OPEN, refreshMsg.state().streamState());
            assertEquals(OmmState.DataState.OK, refreshMsg.state().dataState());
            assertEquals(OmmState.StatusCode.NONE, refreshMsg.state().code());
            assertTrue(refreshMsg.solicited());
            assertTrue(refreshMsg.complete());

            assertNotNull(consumer);
        }
        catch (OmmException excep)
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
    public void testMultiConnectionsSingleItemRequestWithGroupCloseRecoverable()
    {
        TestUtilities.printTestHead("testMultiConnectionsSingleItemRequestWithGroupCloseRecoverable","");

        String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";

        OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);

        ProviderTestOptions providerTestOptions = new ProviderTestOptions();

        providerTestOptions.itemGroupId = ByteBuffer.wrap("10".getBytes());

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

            long itemHandle = consumer.registerClient(reqMsg.clear().serviceName("DIRECT_FEED").name("LSEG.O"), consumerClient);

            Thread.sleep(2000);

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
            assertEquals(serviceId, refreshMsg.serviceId());
            assertEquals(OmmState.StreamState.OPEN, refreshMsg.state().streamState());
            assertEquals(OmmState.DataState.OK, refreshMsg.state().dataState());
            assertEquals(OmmState.StatusCode.NONE, refreshMsg.state().code());

            /* Send item group recoverable status from the first provider */
            ElementList serviceGroupId = EmaFactory.createElementList();
            serviceGroupId.add( EmaFactory.createElementEntry().buffer( EmaRdm.ENAME_GROUP, providerTestOptions.itemGroupId ));
            serviceGroupId.add( EmaFactory.createElementEntry().state( EmaRdm.ENAME_STATUS,
                    OmmState.StreamState.CLOSED_RECOVER, OmmState.DataState.SUSPECT, OmmState.StatusCode.NONE, "Group Status Msg" ) );

            FilterList filterList = EmaFactory.createFilterList();
            filterList.add( EmaFactory.createFilterEntry().elementList( EmaRdm.SERVICE_GROUP_ID, FilterEntry.FilterAction.SET, serviceGroupId ) );

            Map map = EmaFactory.createMap();
            map.add( EmaFactory.createMapEntry().keyUInt(1, MapEntry.MapAction.UPDATE, filterList));

            ommprovider.submit( EmaFactory.createUpdateMsg().domainType( EmaRdm.MMT_DIRECTORY ).
                    filter( EmaRdm.SERVICE_GROUP_FILTER ).
                    payload( map ), 0);	// use 0 item handle to fanout to all subscribers

            Thread.sleep(2000); // Wait until consumer receives the group status message.


            assertEquals(2, consumerClient.queueSize());

            message = consumerClient.popMessage();

            StatusMsg statusMsg = (StatusMsg)message;

            assertEquals("DIRECT_FEED", statusMsg.serviceName());
            assertEquals("LSEG.O", statusMsg.name());
            assertEquals(serviceId, statusMsg.serviceId());
            assertEquals(OmmState.StreamState.OPEN, statusMsg.state().streamState());
            assertEquals(OmmState.DataState.SUSPECT, statusMsg.state().dataState());
            assertEquals(OmmState.StatusCode.NONE, statusMsg.state().statusCode());

            message = consumerClient.popMessage();

            refreshMsg = (RefreshMsg)message;

            assertEquals("DIRECT_FEED", refreshMsg.serviceName());
            assertEquals("LSEG.O", refreshMsg.name());
            assertEquals(serviceId, refreshMsg.serviceId());
            assertEquals(OmmState.StreamState.OPEN, refreshMsg.state().streamState());
            assertEquals(OmmState.DataState.OK, refreshMsg.state().dataState());
            assertEquals(OmmState.StatusCode.NONE, refreshMsg.state().code());

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
    public void testMultiConnectionsSingleItemRequestWithItemCloseRecoverable()
    {
        TestUtilities.printTestHead("testMultiConnectionsSingleItemRequestWithItemCloseRecoverable","");

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

            long itemHandle = consumer.registerClient(reqMsg.clear().serviceName("DIRECT_FEED").name("LSEG.O"), consumerClient);

            Thread.sleep(2000);

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
            assertEquals(serviceId, refreshMsg.serviceId());
            assertEquals(OmmState.StreamState.OPEN, refreshMsg.state().streamState());
            assertEquals(OmmState.DataState.OK, refreshMsg.state().dataState());
            assertEquals(OmmState.StatusCode.NONE, refreshMsg.state().code());

            /* Send item recoverable status from the first provider */

            Long providerItemHandle = providerClient1.retriveItemHandle("LSEG.O");

            assertNotNull(providerItemHandle);

            ommprovider.submit( EmaFactory.createStatusMsg().domainType( EmaRdm.MMT_MARKET_PRICE )
                            .state(OmmState.StreamState.CLOSED_RECOVER, OmmState.DataState.SUSPECT, OmmState.StatusCode.NONE, "Item temporary closed")
                    , providerItemHandle.longValue());

            Thread.sleep(2000); // Wait until consumer receives the item closed recoverable status message.

            assertEquals(2, consumerClient.queueSize());

            message = consumerClient.popMessage();

            StatusMsg statusMsg = (StatusMsg)message;

            assertEquals("DIRECT_FEED", statusMsg.serviceName());
            assertEquals("LSEG.O", statusMsg.name());
            assertEquals(serviceId, statusMsg.serviceId());
            assertEquals(OmmState.StreamState.OPEN, statusMsg.state().streamState());
            assertEquals(OmmState.DataState.SUSPECT, statusMsg.state().dataState());
            assertEquals(OmmState.StatusCode.NONE, statusMsg.state().statusCode());

            message = consumerClient.popMessage();

            refreshMsg = (RefreshMsg)message;

            assertEquals("DIRECT_FEED", refreshMsg.serviceName());
            assertEquals("LSEG.O", refreshMsg.name());
            assertEquals(serviceId, refreshMsg.serviceId());
            assertEquals(OmmState.StreamState.OPEN, refreshMsg.state().streamState());
            assertEquals(OmmState.DataState.OK, refreshMsg.state().dataState());
            assertEquals(OmmState.StatusCode.NONE, refreshMsg.state().code());

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
    public void testMultiConnectionsSingleItemRequestUnsubscribeItemWhenTheItemIsBeingRecovered()
    {
        TestUtilities.printTestHead("testMultiConnectionsSingleItemRequestUnsubscribeItemWhenTheItemIsBeingRecovered","");

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

            long itemHandle = consumer.registerClient(reqMsg.clear().serviceName("DIRECT_FEED").name("LSEG.O"), consumerClient);

            Thread.sleep(2000);

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
            assertEquals(serviceId, refreshMsg.serviceId());
            assertEquals(OmmState.StreamState.OPEN, refreshMsg.state().streamState());
            assertEquals(OmmState.DataState.OK, refreshMsg.state().dataState());
            assertEquals(OmmState.StatusCode.NONE, refreshMsg.state().code());

            /* Send source directory update to stop accepting item requests for both providers */
            ElementList serviceState = EmaFactory.createElementList();
            serviceState.add( EmaFactory.createElementEntry().uintValue( EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_UP ));
            serviceState.add( EmaFactory.createElementEntry().uintValue( EmaRdm.ENAME_ACCEPTING_REQS, 0 ));

            FilterList filterListEnc = EmaFactory.createFilterList();
            filterListEnc.add( EmaFactory.createFilterEntry().elementList( EmaRdm.SERVICE_STATE_ID, FilterEntry.FilterAction.UPDATE, serviceState ) );

            Map map = EmaFactory.createMap();
            map.add( EmaFactory.createMapEntry().keyUInt( 1, MapEntry.MapAction.UPDATE, filterListEnc ));

            UpdateMsg updateMsg = EmaFactory.createUpdateMsg();
            ommprovider.submit( updateMsg.domainType( EmaRdm.MMT_DIRECTORY ).
                    filter( EmaRdm.SERVICE_STATE_FILTER ).
                    payload( map ), 0);	// use 0 item handle to fan-out to all subscribers

            serviceState.clear();
            serviceState.add( EmaFactory.createElementEntry().uintValue( EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_UP ));
            serviceState.add( EmaFactory.createElementEntry().uintValue( EmaRdm.ENAME_ACCEPTING_REQS, 0 ));

            filterListEnc.clear();
            filterListEnc.add( EmaFactory.createFilterEntry().elementList( EmaRdm.SERVICE_STATE_ID, FilterEntry.FilterAction.UPDATE, serviceState ) );

            map.clear();
            map.add( EmaFactory.createMapEntry().keyUInt( 1, MapEntry.MapAction.UPDATE, filterListEnc ));

            ommprovider2.submit( updateMsg.clear().domainType( EmaRdm.MMT_DIRECTORY ).
                    filter( EmaRdm.SERVICE_STATE_FILTER ).
                    payload( map ), 0);	// use 0 item handle to fan-out to all subscribers

            Thread.sleep(2000);

            Long providerItemHandle = providerClient1.retriveItemHandle("LSEG.O");

            assertNotNull(providerItemHandle);

            ommprovider.submit( EmaFactory.createStatusMsg().domainType( EmaRdm.MMT_MARKET_PRICE )
                            .state(OmmState.StreamState.CLOSED_RECOVER, OmmState.DataState.SUSPECT, OmmState.StatusCode.NONE, "Item temporary closed")
                    , providerItemHandle.longValue());

            Thread.sleep(2000); // Wait until consumer receives the item closed recoverable status message.

            assertEquals(2, consumerClient.queueSize());

            message = consumerClient.popMessage();

            StatusMsg statusMsg = (StatusMsg)message;

            assertEquals("DIRECT_FEED", statusMsg.serviceName());
            assertEquals("LSEG.O", statusMsg.name());
            assertEquals(serviceId, statusMsg.serviceId());
            assertEquals(OmmState.StreamState.OPEN, statusMsg.state().streamState());
            assertEquals(OmmState.DataState.SUSPECT, statusMsg.state().dataState());
            assertEquals(OmmState.StatusCode.NONE, statusMsg.state().statusCode());
            assertEquals("Item temporary closed", statusMsg.state().statusText());

            message = consumerClient.popMessage();

            statusMsg = (StatusMsg)message;

            assertEquals("DIRECT_FEED", statusMsg.serviceName());
            assertEquals("LSEG.O", statusMsg.name());
            assertEquals(serviceId, statusMsg.serviceId());
            assertEquals(OmmState.StreamState.OPEN, statusMsg.state().streamState());
            assertEquals(OmmState.DataState.SUSPECT, statusMsg.state().dataState());
            assertEquals(OmmState.StatusCode.NONE, statusMsg.state().statusCode());
            assertEquals("No matching service present.", statusMsg.state().statusText());

            /* Unsubscribe item when it is being recovered. */
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
    public void testMultiConnectionsItemClosedRecoverableAndWaitForServiceToAcceptRequests()
    {
        TestUtilities.printTestHead("testMultiConnectionsItemClosedRecoverableAndWaitForServiceToAcceptRequests","");

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

            long itemHandle = consumer.registerClient(reqMsg.clear().serviceName("DIRECT_FEED").name("LSEG.O"), consumerClient);

            Thread.sleep(1000);

            assertEquals(1, providerClient1.queueSize());

            Msg message = providerClient1.popMessage();

            assertTrue(message instanceof ReqMsg);

            ReqMsg requestMsg = (ReqMsg)message;

            assertEquals("DIRECT_FEED", requestMsg.serviceName());
            assertEquals("LSEG.O", requestMsg.name());
            assertEquals(1, requestMsg.serviceId());

            Thread.sleep(1000);

            assertEquals(1, consumerClient.queueSize());

            message = consumerClient.popMessage();

            RefreshMsg refreshMsg = (RefreshMsg)message;

            assertEquals("DIRECT_FEED", refreshMsg.serviceName());
            assertEquals("LSEG.O", refreshMsg.name());
            assertEquals(serviceId, refreshMsg.serviceId());
            assertEquals(OmmState.StreamState.OPEN, refreshMsg.state().streamState());
            assertEquals(OmmState.DataState.OK, refreshMsg.state().dataState());
            assertEquals(OmmState.StatusCode.NONE, refreshMsg.state().code());

            /* Send source directory update to stop accepting item requests for both providers */
            ElementList serviceState = EmaFactory.createElementList();
            serviceState.add( EmaFactory.createElementEntry().uintValue( EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_UP ));
            serviceState.add( EmaFactory.createElementEntry().uintValue( EmaRdm.ENAME_ACCEPTING_REQS, 0 ));

            FilterList filterListEnc = EmaFactory.createFilterList();
            filterListEnc.add( EmaFactory.createFilterEntry().elementList( EmaRdm.SERVICE_STATE_ID, FilterEntry.FilterAction.UPDATE, serviceState ) );

            Map map = EmaFactory.createMap();
            map.add( EmaFactory.createMapEntry().keyUInt( 1, MapEntry.MapAction.UPDATE, filterListEnc ));

            UpdateMsg updateMsg = EmaFactory.createUpdateMsg();
            ommprovider.submit( updateMsg.domainType( EmaRdm.MMT_DIRECTORY ).
                    filter( EmaRdm.SERVICE_STATE_FILTER ).
                    payload( map ), 0);	// use 0 item handle to fan-out to all subscribers

            serviceState.clear();
            serviceState.add( EmaFactory.createElementEntry().uintValue( EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_UP ));
            serviceState.add( EmaFactory.createElementEntry().uintValue( EmaRdm.ENAME_ACCEPTING_REQS, 0 ));

            filterListEnc.clear();
            filterListEnc.add( EmaFactory.createFilterEntry().elementList( EmaRdm.SERVICE_STATE_ID, FilterEntry.FilterAction.UPDATE, serviceState ) );

            map.clear();
            map.add( EmaFactory.createMapEntry().keyUInt( 1, MapEntry.MapAction.UPDATE, filterListEnc ));

            ommprovider2.submit( updateMsg.clear().domainType( EmaRdm.MMT_DIRECTORY ).
                    filter( EmaRdm.SERVICE_STATE_FILTER ).
                    payload( map ), 0);	// use 0 item handle to fan-out to all subscribers

            Thread.sleep(1000);

            Long providerItemHandle = providerClient1.retriveItemHandle("LSEG.O");

            assertNotNull(providerItemHandle);

            ommprovider.submit( EmaFactory.createStatusMsg().domainType( EmaRdm.MMT_MARKET_PRICE )
                            .state(OmmState.StreamState.CLOSED_RECOVER, OmmState.DataState.SUSPECT, OmmState.StatusCode.NONE, "Item temporary closed")
                    , providerItemHandle.longValue());

            Thread.sleep(1000); // Wait until consumer receives the item closed recoverable status message.

            assertEquals(2, consumerClient.queueSize());

            message = consumerClient.popMessage();

            StatusMsg statusMsg = (StatusMsg)message;

            assertEquals("DIRECT_FEED", statusMsg.serviceName());
            assertEquals("LSEG.O", statusMsg.name());
            assertEquals(serviceId, statusMsg.serviceId());
            assertEquals(OmmState.StreamState.OPEN, statusMsg.state().streamState());
            assertEquals(OmmState.DataState.SUSPECT, statusMsg.state().dataState());
            assertEquals(OmmState.StatusCode.NONE, statusMsg.state().statusCode());
            assertEquals("Item temporary closed", statusMsg.state().statusText());

            message = consumerClient.popMessage();

            statusMsg = (StatusMsg)message;

            assertEquals("DIRECT_FEED", statusMsg.serviceName());
            assertEquals("LSEG.O", statusMsg.name());
            assertEquals(serviceId, statusMsg.serviceId());
            assertEquals(OmmState.StreamState.OPEN, statusMsg.state().streamState());
            assertEquals(OmmState.DataState.SUSPECT, statusMsg.state().dataState());
            assertEquals(OmmState.StatusCode.NONE, statusMsg.state().statusCode());
            assertEquals("No matching service present.", statusMsg.state().statusText());

            Thread.sleep(1000);

            /* Send source directory update for the second provider to accept requests */
            serviceState.clear();
            serviceState.add( EmaFactory.createElementEntry().uintValue( EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_UP ));
            serviceState.add( EmaFactory.createElementEntry().uintValue( EmaRdm.ENAME_ACCEPTING_REQS, 1 ));

            filterListEnc.clear();
            filterListEnc.add( EmaFactory.createFilterEntry().elementList( EmaRdm.SERVICE_STATE_ID, FilterEntry.FilterAction.UPDATE, serviceState ) );

            map.clear();
            map.add( EmaFactory.createMapEntry().keyUInt( 1, MapEntry.MapAction.UPDATE, filterListEnc ));

            ommprovider2.submit( updateMsg.clear().domainType( EmaRdm.MMT_DIRECTORY ).
                    filter( EmaRdm.SERVICE_STATE_FILTER ).
                    payload( map ), 0);	// use 0 item handle to fan-out to all subscribers

            Thread.sleep(2000);

            assertEquals(1, consumerClient.queueSize());

            message = consumerClient.popMessage();

            refreshMsg = (RefreshMsg)message;

            assertEquals("DIRECT_FEED", refreshMsg.serviceName());
            assertEquals("LSEG.O", refreshMsg.name());
            assertEquals(serviceId, refreshMsg.serviceId());
            assertEquals(OmmState.StreamState.OPEN, refreshMsg.state().streamState());
            assertEquals(OmmState.DataState.OK, refreshMsg.state().dataState());
            assertEquals(OmmState.StatusCode.NONE, refreshMsg.state().code());

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
    public void testMultiConnectionsItemRequestsWithServiceListNames()
    {
        TestUtilities.printTestHead("testMultiConnectionsItemRequestsWithServiceListNames","");

        String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";

        OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);

        ProviderTestOptions providerTestOptions = new ProviderTestOptions();
        providerTestOptions.sendUpdateMessage = true;

        ProviderTestClient providerClient1 = new ProviderTestClient(providerTestOptions);

        // Provider_1 provides the DIRECT_FEED service name
        OmmProvider ommprovider = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_1"), providerClient1);

        assertNotNull(ommprovider);

        ProviderTestClient providerClient2 = new ProviderTestClient(providerTestOptions);

        // Provider_1 provides the DIRECT_FEED2 service name
        OmmProvider ommprovider2 = EmaFactory.createOmmProvider(config.port("19004").providerName("Provider_3"), providerClient2);

        assertNotNull(ommprovider2);

        OmmConsumer consumer = null;
        ConsumerTestClient consumerClient = new ConsumerTestClient();

        try
        {
            ServiceList serviceList = EmaFactory.createServiceList("SVG1");

            serviceList.concreteServiceList().add("DIRECT_FEED");
            serviceList.concreteServiceList().add("DIRECT_FEED_2");

            ServiceList serviceList2 = EmaFactory.createServiceList("SVG2");

            serviceList2.concreteServiceList().add("DIRECT_FEED_2");
            serviceList2.concreteServiceList().add("DIRECT_FEED");

            consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_9").addServiceList(serviceList)
                    .addServiceList(serviceList2));

            ConsumerSession<OmmConsumerClient> consumerSession = ((OmmConsumerImpl)consumer).consumerSession();

            int serviceId = consumerSession.serviceList("SVG1").serviceId();
            int serviceId2 = consumerSession.serviceList("SVG2").serviceId();

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

            assertEquals("SVG1", refreshMsg.serviceName());
            assertEquals(serviceId, refreshMsg.serviceId());
            assertEquals("LSEG.O", refreshMsg.name());
            assertEquals(OmmState.StreamState.OPEN, refreshMsg.state().streamState());
            assertEquals(OmmState.DataState.OK, refreshMsg.state().dataState());
            assertEquals(OmmState.StatusCode.NONE, refreshMsg.state().code());

            message = consumerClient.popMessage();

            UpdateMsg updateMsg = (UpdateMsg)message;

            assertEquals("SVG1", updateMsg.serviceName());
            assertEquals(serviceId, updateMsg.serviceId());
            assertEquals("LSEG.O", updateMsg.name());

            long itemHandle2 = consumer.registerClient(reqMsg.clear().serviceListName("SVG2").name("LSEG2.O"), consumerClient);

            Thread.sleep(2000);

            assertEquals(1, providerClient2.queueSize());

            message = providerClient2.popMessage();

            assertTrue(message instanceof ReqMsg);

            requestMsg = (ReqMsg)message;

            assertEquals("DIRECT_FEED_2", requestMsg.serviceName());
            assertEquals("LSEG2.O", requestMsg.name());
            assertEquals(1, requestMsg.serviceId());

            Thread.sleep(2000);

            assertEquals(2, consumerClient.queueSize());

            message = consumerClient.popMessage();

            refreshMsg = (RefreshMsg)message;

            assertEquals("SVG2", refreshMsg.serviceName());
            assertEquals(serviceId2, refreshMsg.serviceId());
            assertEquals("LSEG2.O", refreshMsg.name());
            assertEquals(OmmState.StreamState.OPEN, refreshMsg.state().streamState());
            assertEquals(OmmState.DataState.OK, refreshMsg.state().dataState());
            assertEquals(OmmState.StatusCode.NONE, refreshMsg.state().code());

            message = consumerClient.popMessage();

            updateMsg = (UpdateMsg)message;

            assertEquals("SVG2", updateMsg.serviceName());
            assertEquals(serviceId2, updateMsg.serviceId());
            assertEquals("LSEG2.O", updateMsg.name());

            consumer.unregister(itemHandle);
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
    public void testMultiConnectionsItemRequestsWithServiceListNamesWithAUnknownServiceInTheList()
    {
        TestUtilities.printTestHead("testMultiConnectionsItemRequestsWithServiceListNamesWithAUnknownServiceInTheList","");

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

            serviceList.concreteServiceList().add("UNKNOWN_SERVICE");
            serviceList.concreteServiceList().add("DIRECT_FEED");

            consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_9").addServiceList(serviceList));

            ConsumerSession<OmmConsumerClient> consumerSession = ((OmmConsumerImpl)consumer).consumerSession();

            int serviceId = consumerSession.serviceList("SVG1").serviceId();

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

            assertEquals(1, consumerClient.queueSize());

            message = consumerClient.popMessage();

            RefreshMsg refreshMsg = (RefreshMsg)message;

            assertEquals("SVG1", refreshMsg.serviceName());
            assertEquals(serviceId, refreshMsg.serviceId());
            assertEquals("LSEG.O", refreshMsg.name());
            assertEquals(OmmState.StreamState.OPEN, refreshMsg.state().streamState());
            assertEquals(OmmState.DataState.OK, refreshMsg.state().dataState());
            assertEquals(OmmState.StatusCode.NONE, refreshMsg.state().code());

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
    public void testMultiConnectionsItemRequestsWithServiceListNameButConcreteServicesAreNotAvaliableThenConcreateServiceIsAdded()
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
            ServiceList serviceList = EmaFactory.createServiceList("SVG1");

            serviceList.concreteServiceList().add("UNKNOWN_SERVICE");
            serviceList.concreteServiceList().add("DIRECT_FEED2");

            consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_9").addServiceList(serviceList));

            ReqMsg reqMsg = EmaFactory.createReqMsg();

            long itemHandle = consumer.registerClient(reqMsg.clear().serviceListName("SVG1").name("LSEG.O"), consumerClient);

            Thread.sleep(2000);

            assertEquals(1, consumerClient.queueSize());

            Msg message = consumerClient.popMessage();

            StatusMsg statusMsg = (StatusMsg)message;

            assertEquals("SVG1", statusMsg.serviceName());
            assertEquals("LSEG.O", statusMsg.name());
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

            assertEquals(1, providerClient1.queueSize());

            message = providerClient1.popMessage();

            assertTrue(message instanceof ReqMsg);

            ReqMsg requestMsg = (ReqMsg)message;

            assertEquals("DIRECT_FEED2", requestMsg.serviceName());
            assertEquals("LSEG.O", requestMsg.name());
            assertEquals(2, requestMsg.serviceId());

            assertEquals(1, consumerClient.queueSize());
            message = consumerClient.popMessage();

            RefreshMsg refreshMsg = (RefreshMsg)message;
            assertEquals("SVG1", refreshMsg.serviceName());
            assertEquals("LSEG.O", refreshMsg.name());
            assertEquals(32767, refreshMsg.serviceId());
            assertEquals(OmmState.StreamState.OPEN, refreshMsg.state().streamState());
            assertEquals(OmmState.DataState.OK, refreshMsg.state().dataState());
            assertEquals(OmmState.StatusCode.NONE, refreshMsg.state().code());
            assertTrue(refreshMsg.solicited());

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
    public void testMultiConnectionItemGroupClosedReacoverableWithServiceList()
    {
        TestUtilities.printTestHead("testMultiConnectionItemGroupClosedReacoverableWithServiceList","");

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
            ServiceList serviceList = EmaFactory.createServiceList("SVG1");

            serviceList.concreteServiceList().add("DIRECT_FEED");
            serviceList.concreteServiceList().add("DIRECT_FEED_2");

            consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_9").addServiceList(serviceList));

            ConsumerSession<OmmConsumerClient> consumerSession = ((OmmConsumerImpl)consumer).consumerSession();
            int serviceId = consumerSession.serviceList("SVG1").serviceId();

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

            assertEquals(1, consumerClient.queueSize());

            message = consumerClient.popMessage();

            RefreshMsg refreshMsg = (RefreshMsg)message;

            assertEquals("SVG1", refreshMsg.serviceName());
            assertEquals("LSEG.O", refreshMsg.name());
            assertEquals(serviceId, refreshMsg.serviceId());
            assertEquals(OmmState.StreamState.OPEN, refreshMsg.state().streamState());
            assertEquals(OmmState.DataState.OK, refreshMsg.state().dataState());
            assertEquals(OmmState.StatusCode.NONE, refreshMsg.state().code());

            /* Send item recoverable status from the first provider */
            Long providerItemHandle = providerClient1.retriveItemHandle("LSEG.O");

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


            ommprovider.submit( EmaFactory.createStatusMsg().domainType( EmaRdm.MMT_MARKET_PRICE )
                            .state(OmmState.StreamState.CLOSED_RECOVER, OmmState.DataState.SUSPECT, OmmState.StatusCode.NONE, "Item temporary closed")
                    , providerItemHandle.longValue());


            Thread.sleep(2000); // Wait until consumer receives the item closed recoverable status message.

            /* The second provider receive a request message for the DIRECT_FEED_2 */
            assertEquals(1, providerClient2.queueSize());

            message = providerClient2.popMessage();

            assertTrue(message instanceof ReqMsg);

            requestMsg = (ReqMsg)message;

            assertEquals("DIRECT_FEED_2", requestMsg.serviceName());
            assertEquals("LSEG.O", requestMsg.name());
            assertEquals(1, requestMsg.serviceId());

            assertEquals(2, consumerClient.queueSize());

            message = consumerClient.popMessage();

            StatusMsg statusMsg = (StatusMsg)message;

            assertEquals("SVG1", statusMsg.serviceName());
            assertEquals("LSEG.O", statusMsg.name());
            assertEquals(serviceId, refreshMsg.serviceId());
            assertEquals(OmmState.StreamState.OPEN, statusMsg.state().streamState());
            assertEquals(OmmState.DataState.SUSPECT, statusMsg.state().dataState());
            assertEquals(OmmState.StatusCode.NONE, statusMsg.state().statusCode());

            message = consumerClient.popMessage();

            refreshMsg = (RefreshMsg)message;

            assertEquals("SVG1", refreshMsg.serviceName());
            assertEquals("LSEG.O", refreshMsg.name());
            assertEquals(serviceId, refreshMsg.serviceId());
            assertEquals(OmmState.StreamState.OPEN, refreshMsg.state().streamState());
            assertEquals(OmmState.DataState.OK, refreshMsg.state().dataState());
            assertEquals(OmmState.StatusCode.NONE, refreshMsg.state().code());

            Thread.sleep(2000);

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
    public void testMultiConnectionGroupClosedReacoverableStatusWithServiceList()
    {
        TestUtilities.printTestHead("testMultiConnectionGroupClosedReacoverableStatusWithServiceList","");

        String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";

        OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);

        ProviderTestOptions providerTestOptions = new ProviderTestOptions();
        providerTestOptions.itemGroupId = ByteBuffer.wrap("10".getBytes());

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
            ServiceList serviceList = EmaFactory.createServiceList("SVG1");

            serviceList.concreteServiceList().add("DIRECT_FEED");
            serviceList.concreteServiceList().add("DIRECT_FEED_2");

            consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_9").addServiceList(serviceList));
            ConsumerSession<OmmConsumerClient> consumerSession = ((OmmConsumerImpl)consumer).consumerSession();
            int serviceId = consumerSession.serviceList("SVG1").serviceId();

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

            assertEquals(1, consumerClient.queueSize());

            message = consumerClient.popMessage();

            RefreshMsg refreshMsg = (RefreshMsg)message;

            assertEquals("SVG1", refreshMsg.serviceName());
            assertEquals(serviceId, refreshMsg.serviceId());
            assertEquals("LSEG.O", refreshMsg.name());
            assertEquals(OmmState.StreamState.OPEN, refreshMsg.state().streamState());
            assertEquals(OmmState.DataState.OK, refreshMsg.state().dataState());
            assertEquals(OmmState.StatusCode.NONE, refreshMsg.state().code());

            /* Send item recoverable status from the first provider */
            Long providerItemHandle = providerClient1.retriveItemHandle("LSEG.O");

            assertNotNull(providerItemHandle);

            ElementList serviceGroupId = EmaFactory.createElementList();
            serviceGroupId.add( EmaFactory.createElementEntry().buffer( EmaRdm.ENAME_GROUP, providerTestOptions.itemGroupId ));
            serviceGroupId.add( EmaFactory.createElementEntry().state( EmaRdm.ENAME_STATUS,
                    OmmState.StreamState.CLOSED_RECOVER, OmmState.DataState.SUSPECT, OmmState.StatusCode.NONE, "Group Status Msg" ) );

            FilterList filterList = EmaFactory.createFilterList();
            filterList.add( EmaFactory.createFilterEntry().elementList( EmaRdm.SERVICE_GROUP_ID, FilterEntry.FilterAction.SET, serviceGroupId ) );

            Map map = EmaFactory.createMap();
            map.add( EmaFactory.createMapEntry().keyUInt(1, MapEntry.MapAction.UPDATE, filterList));

            ommprovider.submit( EmaFactory.createUpdateMsg().domainType( EmaRdm.MMT_DIRECTORY ).
                    filter( EmaRdm.SERVICE_GROUP_FILTER ).
                    payload( map ), 0);	// use 0 item handle to fanout to all subscribers

            Thread.sleep(2000); // Wait until consumer receives the item closed recoverable status message.

            /* The second provider receive a request message for the DIRECT_FEED_2 */
            assertEquals(1, providerClient2.queueSize());

            message = providerClient2.popMessage();

            assertTrue(message instanceof ReqMsg);

            requestMsg = (ReqMsg)message;

            assertEquals("DIRECT_FEED_2", requestMsg.serviceName());
            assertEquals("LSEG.O", requestMsg.name());
            assertEquals(1, requestMsg.serviceId());

            assertEquals(2, consumerClient.queueSize());

            message = consumerClient.popMessage();

            StatusMsg statusMsg = (StatusMsg)message;

            assertEquals("SVG1", statusMsg.serviceName());
            assertEquals(serviceId, refreshMsg.serviceId());
            assertEquals("LSEG.O", statusMsg.name());
            assertEquals(OmmState.StreamState.OPEN, statusMsg.state().streamState());
            assertEquals(OmmState.DataState.SUSPECT, statusMsg.state().dataState());
            assertEquals(OmmState.StatusCode.NONE, statusMsg.state().statusCode());

            message = consumerClient.popMessage();

            refreshMsg = (RefreshMsg)message;

            assertEquals("SVG1", refreshMsg.serviceName());
            assertEquals(serviceId, refreshMsg.serviceId());
            assertEquals("LSEG.O", refreshMsg.name());
            assertEquals(OmmState.StreamState.OPEN, refreshMsg.state().streamState());
            assertEquals(OmmState.DataState.OK, refreshMsg.state().dataState());
            assertEquals(OmmState.StatusCode.NONE, refreshMsg.state().code());

            Thread.sleep(2000);

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
    public void testMultiConnectionGroupOpenSuspectStatusWithServiceList()
    {
        TestUtilities.printTestHead("testMultiConnectionGroupOpenSuspectStatusWithServiceList","");

        String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";

        OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);

        ProviderTestOptions providerTestOptions = new ProviderTestOptions();
        providerTestOptions.itemGroupId = ByteBuffer.wrap("10".getBytes());

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
            ServiceList serviceList = EmaFactory.createServiceList("SVG1");

            serviceList.concreteServiceList().add("DIRECT_FEED");
            serviceList.concreteServiceList().add("DIRECT_FEED_2");

            consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_9").addServiceList(serviceList));
            ConsumerSession<OmmConsumerClient> consumerSession = ((OmmConsumerImpl)consumer).consumerSession();
            int serviceId = consumerSession.serviceList("SVG1").serviceId();

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

            assertEquals(1, consumerClient.queueSize());

            message = consumerClient.popMessage();

            RefreshMsg refreshMsg = (RefreshMsg)message;

            assertEquals("SVG1", refreshMsg.serviceName());
            assertEquals(serviceId, refreshMsg.serviceId());
            assertEquals("LSEG.O", refreshMsg.name());
            assertEquals(OmmState.StreamState.OPEN, refreshMsg.state().streamState());
            assertEquals(OmmState.DataState.OK, refreshMsg.state().dataState());
            assertEquals(OmmState.StatusCode.NONE, refreshMsg.state().code());

            /* Send item recoverable status from the first provider */
            Long providerItemHandle = providerClient1.retriveItemHandle("LSEG.O");

            assertNotNull(providerItemHandle);

            ElementList serviceGroupId = EmaFactory.createElementList();
            serviceGroupId.add( EmaFactory.createElementEntry().buffer( EmaRdm.ENAME_GROUP, providerTestOptions.itemGroupId ));
            serviceGroupId.add( EmaFactory.createElementEntry().state( EmaRdm.ENAME_STATUS,
                    OmmState.StreamState.OPEN, OmmState.DataState.SUSPECT, OmmState.StatusCode.NONE, "Group Status Msg" ) );

            FilterList filterList = EmaFactory.createFilterList();
            filterList.add( EmaFactory.createFilterEntry().elementList( EmaRdm.SERVICE_GROUP_ID, FilterEntry.FilterAction.SET, serviceGroupId ) );

            Map map = EmaFactory.createMap();
            map.add( EmaFactory.createMapEntry().keyUInt(1, MapEntry.MapAction.UPDATE, filterList));

            ommprovider.submit( EmaFactory.createUpdateMsg().domainType( EmaRdm.MMT_DIRECTORY ).
                    filter( EmaRdm.SERVICE_GROUP_FILTER ).
                    payload( map ), 0);	// use 0 item handle to fanout to all subscribers

            Thread.sleep(2000); // Wait until consumer receives the item closed recoverable status message.

            /* The second provider receive a request message for the DIRECT_FEED_2 */
            assertEquals(1, providerClient2.queueSize());

            message = providerClient2.popMessage();

            assertTrue(message instanceof ReqMsg);

            requestMsg = (ReqMsg)message;

            assertEquals("DIRECT_FEED_2", requestMsg.serviceName());
            assertEquals("LSEG.O", requestMsg.name());
            assertEquals(1, requestMsg.serviceId());

            assertEquals(2, consumerClient.queueSize());

            message = consumerClient.popMessage();

            StatusMsg statusMsg = (StatusMsg)message;

            assertEquals("SVG1", statusMsg.serviceName());
            assertEquals(serviceId, statusMsg.serviceId());
            assertEquals("LSEG.O", statusMsg.name());
            assertEquals(OmmState.StreamState.OPEN, statusMsg.state().streamState());
            assertEquals(OmmState.DataState.SUSPECT, statusMsg.state().dataState());
            assertEquals(OmmState.StatusCode.NONE, statusMsg.state().statusCode());

            message = consumerClient.popMessage();

            refreshMsg = (RefreshMsg)message;

            assertEquals("SVG1", refreshMsg.serviceName());
            assertEquals(serviceId, refreshMsg.serviceId());
            assertEquals("LSEG.O", refreshMsg.name());
            assertEquals(OmmState.StreamState.OPEN, refreshMsg.state().streamState());
            assertEquals(OmmState.DataState.OK, refreshMsg.state().dataState());
            assertEquals(OmmState.StatusCode.NONE, refreshMsg.state().code());

            Thread.sleep(2000);

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
    public void testMultiConnectionsOnStreamPostingWithServiceNameAndId()
    {
        TestUtilities.printTestHead("testMultiConnectionsOnStreamPostingWithServiceNameAndId","");

        String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";

        OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);

        ProviderTestOptions providerTestOptions = new ProviderTestOptions();
        providerTestOptions.sendRefreshAttrib = true;
        providerTestOptions.supportOMMPosting = true;

        ProviderTestClient providerClient1 = new ProviderTestClient(providerTestOptions);

        // Provider_1 provides the DIRECT_FEED service name
        OmmProvider ommprovider = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_1"), providerClient1);

        assertNotNull(ommprovider);

        ProviderTestClient providerClient2 = new ProviderTestClient(providerTestOptions);

        // Provider_3 provides the DIRECT_FEED_2 service name
        OmmProvider ommprovider2 = EmaFactory.createOmmProvider(config.port("19002").providerName("Provider_3"), providerClient2);

        assertNotNull(ommprovider2);

        OmmConsumer consumer = null;
        ConsumerTestClient consumerClient = new ConsumerTestClient();

        try
        {
            consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_9"));

            ReqMsg reqMsg = EmaFactory.createReqMsg();


            long itemHandle = consumer.registerClient(reqMsg.clear().serviceName("DIRECT_FEED").name("LSEG.O"), consumerClient);

            Thread.sleep(1000);

            assertEquals(1, providerClient1.queueSize());

            Msg message = providerClient1.popMessage();

            assertTrue(message instanceof ReqMsg);

            ReqMsg requestMsg = (ReqMsg)message;

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

            /* Submit a PostMsg to the first item stream */
            PostMsg postMsg = EmaFactory.createPostMsg();
            UpdateMsg nestedUpdateMsg = EmaFactory.createUpdateMsg();
            FieldList nestedFieldList = EmaFactory.createFieldList();

            nestedFieldList.add(EmaFactory.createFieldEntry().real(22, 34, OmmReal.MagnitudeType.EXPONENT_POS_1));
            nestedFieldList.add(EmaFactory.createFieldEntry().real(25, 35, OmmReal.MagnitudeType.EXPONENT_POS_1));
            nestedFieldList.add(EmaFactory.createFieldEntry().time(18, 11, 29, 30));
            nestedFieldList.add(EmaFactory.createFieldEntry().enumValue(37, 3));

            nestedUpdateMsg.payload(nestedFieldList );

            int postId = 0;

            consumer.submit( postMsg.postId( ++postId ).serviceName("DIRECT_FEED")
                    .name( "IBM.N" ).solicitAck( false ).complete(true)
                    .payload(nestedUpdateMsg), itemHandle );

            Thread.sleep(1000);

            /* Checks to ensure that the provider receives the PostMsg */
            assertEquals(1, providerClient1.queueSize());

            message = providerClient1.popMessage();

            assertTrue(message instanceof PostMsg);

            PostMsg recvPostMsg = (PostMsg)message;

            assertEquals("DIRECT_FEED", recvPostMsg.serviceName());
            assertEquals("IBM.N", recvPostMsg.name());
            assertEquals(1, recvPostMsg.postId());
            assertEquals(true, recvPostMsg.complete());
            assertEquals(DataType.DataTypes.UPDATE_MSG, recvPostMsg.payload().dataType());

            UpdateMsg updateMsg = recvPostMsg.payload().updateMsg();
            assertEquals(DataType.DataTypes.FIELD_LIST, updateMsg.payload().dataType());

            /* Ensure there is no more message from provider as the Ack flag is not set */
            assertEquals(0, consumerClient.queueSize());

            /* Submit another PostMsg which requires Ack to the second provider. */
            consumer.submit( postMsg.clear().postId( ++postId ).serviceId(32767)
                    .name( "IBM.N" ).solicitAck( true ).complete(true)
                    .payload(nestedUpdateMsg), itemHandle );

            Thread.sleep(1000);

            /* Checks to ensure that the provider receives the PostMsg */
            assertEquals(1, providerClient1.queueSize());

            message = providerClient1.popMessage();

            assertTrue(message instanceof PostMsg);

            recvPostMsg = (PostMsg)message;

            assertEquals("DIRECT_FEED", recvPostMsg.serviceName());
            assertEquals("IBM.N", recvPostMsg.name());
            assertEquals(2, recvPostMsg.postId());
            assertEquals(true, recvPostMsg.complete());
            assertEquals(DataType.DataTypes.UPDATE_MSG, recvPostMsg.payload().dataType());

            updateMsg = recvPostMsg.payload().updateMsg();
            assertEquals(DataType.DataTypes.FIELD_LIST, updateMsg.payload().dataType());

            Thread.sleep(2000);
            /* Ensure that the client side receives ACK message from provider */
            assertEquals(1, consumerClient.queueSize());

            message = consumerClient.popMessage();

            AckMsg ackMessage = (AckMsg)message;

            assertEquals("DIRECT_FEED", ackMessage.serviceName());
            assertEquals(32767, ackMessage.serviceId());
            assertEquals("IBM.N", ackMessage.name());
            assertEquals(2, ackMessage.ackId());

            consumer.unregister(itemHandle);
        }
        catch(OmmException excep)
        {
            System.out.println("Catch OmmException: " + excep.toString());
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
    public void testMultiConnectionsOnStreamPostingWithUnKnownServiceNameAndId()
    {
        TestUtilities.printTestHead("testMultiConnectionsOnStreamPostingWithUnKnownServiceNameAndId","");

        String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";

        OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);

        ProviderTestOptions providerTestOptions = new ProviderTestOptions();
        providerTestOptions.sendRefreshAttrib = true;
        providerTestOptions.supportOMMPosting = true;

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

            ReqMsg reqMsg = EmaFactory.createReqMsg();


            long itemHandle = consumer.registerClient(reqMsg.clear().serviceName("DIRECT_FEED").name("LSEG.O"), consumerClient);

            Thread.sleep(1000);

            assertEquals(1, providerClient1.queueSize());

            Msg message = providerClient1.popMessage();

            assertTrue(message instanceof ReqMsg);

            ReqMsg requestMsg = (ReqMsg)message;

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

            /* Submit a PostMsg to the first item stream */
            PostMsg postMsg = EmaFactory.createPostMsg();
            UpdateMsg nestedUpdateMsg = EmaFactory.createUpdateMsg();
            FieldList nestedFieldList = EmaFactory.createFieldList();

            nestedFieldList.add(EmaFactory.createFieldEntry().real(22, 34, OmmReal.MagnitudeType.EXPONENT_POS_1));
            nestedFieldList.add(EmaFactory.createFieldEntry().real(25, 35, OmmReal.MagnitudeType.EXPONENT_POS_1));
            nestedFieldList.add(EmaFactory.createFieldEntry().time(18, 11, 29, 30));
            nestedFieldList.add(EmaFactory.createFieldEntry().enumValue(37, 3));

            nestedUpdateMsg.payload(nestedFieldList );

            OmmConsumer tempConsumer = consumer;

            /* This is invalid usage as the DIRECT_FEED_2 service name doesn't exist on the provider of this item stream but the second provider. */
            Exception exception = assertThrows(OmmInvalidUsageException.class,  () -> tempConsumer.submit( postMsg.postId( 1 ).serviceName("DIRECT_FEED_2")
                    .name( "IBM.N" ).solicitAck( false ).complete(true)
                    .payload(nestedUpdateMsg), itemHandle ));

            assertEquals("Failed to submit PostMsg on item stream. Reason: ReactorReturnCodes.INVALID_USAGE. Error text: "
                    + "Message submitted with unknown service name DIRECT_FEED_2", exception.getLocalizedMessage());

            System.out.println("Services: \n" + "   DIRECT_FEED, id = "
                    + consumerSession.sessionDirectoryByName("DIRECT_FEED").service().serviceId() + ", DIRECT_FEED_2, id = "
                    + consumerSession.sessionDirectoryByName("DIRECT_FEED_2").service().serviceId());

            int serviceId = consumerSession.sessionDirectoryByName("DIRECT_FEED_2").service().serviceId();

            /* This is invalid usage as the service Id doesn't exist on the provider of this item stream but the second provider */
            exception = assertThrows(OmmInvalidUsageException.class,  () -> tempConsumer.submit( postMsg.clear().postId( 2 ).serviceId(serviceId)
                    .name( "IBM.N" ).solicitAck( true ).complete(true)
                    .payload(nestedUpdateMsg), itemHandle ));

            assertEquals("Failed to submit PostMsg on item stream. Reason: ReactorReturnCodes.INVALID_USAGE. Error text: "
                    + "Message submitted with unknown service Id " + serviceId, exception.getLocalizedMessage());

            /* This is invalid usage as the UNKNOWN_FEED service name doesn't exist in any providers */
            exception = assertThrows(OmmInvalidUsageException.class,  () -> tempConsumer.submit( postMsg.postId( 1 ).serviceName("UNKNOWN_FEED")
                    .name( "IBM.N" ).solicitAck( false ).complete(true)
                    .payload(nestedUpdateMsg), itemHandle ));

            assertEquals("Failed to submit PostMsg on item stream. Reason: ReactorReturnCodes.INVALID_USAGE. Error text: "
                    + "Message submitted with unknown service name UNKNOWN_FEED", exception.getLocalizedMessage());


            /* This is invalid usage as the 65535 service Id doesn't exist in any providers */
            exception = assertThrows(OmmInvalidUsageException.class,  () -> tempConsumer.submit( postMsg.clear().postId( 2 ).serviceId(65535)
                    .name( "IBM.N" ).solicitAck( true ).complete(true)
                    .payload(nestedUpdateMsg), itemHandle ));

            assertEquals("Failed to submit PostMsg on item stream. Reason: ReactorReturnCodes.INVALID_USAGE. Error text: "
                    + "Message submitted with unknown service Id 65535", exception.getLocalizedMessage());

            consumer.unregister(itemHandle);
        }
        catch (OmmException excep)
        {
            System.out.println("Catch OmmException: " + excep.toString());
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
    public void testMultiConnectionsOffStreamPostingWithServiceNameAndId()
    {
        TestUtilities.printTestHead("testMultiConnectionsOffStreamPostingWithServiceNameAndId","");

        String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";

        OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);

        ProviderTestOptions providerTestOptions = new ProviderTestOptions();
        providerTestOptions.sendRefreshAttrib = true;
        providerTestOptions.supportOMMPosting = true;

        ProviderTestClient providerClient1 = new ProviderTestClient(providerTestOptions);

        // Provider_1 provides the DIRECT_FEED service name
        OmmProvider ommprovider = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_1"), providerClient1);

        assertNotNull(ommprovider);

        ProviderTestOptions providerTestOptions2 = new ProviderTestOptions();
        providerTestOptions2.sendRefreshAttrib = true;
        providerTestOptions2.supportOMMPosting = true;

        ProviderTestClient providerClient2 = new ProviderTestClient(providerTestOptions2);

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


            long loginHandle = consumer.registerClient(reqMsg.domainType( EmaRdm.MMT_LOGIN ), consumerClient);

            Thread.sleep(1000);

            /* Ensure there is no more message from provider as the Ack flag is not set */
            assertEquals(1, consumerClient.queueSize());

            /* This is login refresh message message */
            Msg message = consumerClient.popMessage();

            /* Submit a PostMsg to the login stream */
            PostMsg postMsg = EmaFactory.createPostMsg();
            UpdateMsg nestedUpdateMsg = EmaFactory.createUpdateMsg();
            FieldList nestedFieldList = EmaFactory.createFieldList();

            nestedFieldList.add(EmaFactory.createFieldEntry().real(22, 34, OmmReal.MagnitudeType.EXPONENT_POS_1));
            nestedFieldList.add(EmaFactory.createFieldEntry().real(25, 35, OmmReal.MagnitudeType.EXPONENT_POS_1));
            nestedFieldList.add(EmaFactory.createFieldEntry().time(18, 11, 29, 30));
            nestedFieldList.add(EmaFactory.createFieldEntry().enumValue(37, 3));

            nestedUpdateMsg.payload(nestedFieldList );

            int postId = 0;

            consumer.submit( postMsg.postId( ++postId ).serviceName("DIRECT_FEED")
                    .name( "IBM.N" ).solicitAck( false ).complete(true)
                    .payload(nestedUpdateMsg), loginHandle );

            Thread.sleep(1000);

            /* Checks to ensure that the provider receives the PostMsg */
            assertEquals(1, providerClient1.queueSize());

            message = providerClient1.popMessage();

            assertTrue(message instanceof PostMsg);

            PostMsg recvPostMsg = (PostMsg)message;

            assertEquals(1, recvPostMsg.streamId());
            assertEquals("DIRECT_FEED", recvPostMsg.serviceName());
            assertEquals("IBM.N", recvPostMsg.name());
            assertEquals(1, recvPostMsg.postId());
            assertEquals(true, recvPostMsg.complete());
            assertEquals(DataType.DataTypes.UPDATE_MSG, recvPostMsg.payload().dataType());

            UpdateMsg updateMsg = recvPostMsg.payload().updateMsg();
            assertEquals(DataType.DataTypes.FIELD_LIST, updateMsg.payload().dataType());

            /* Submit another PostMsg which requires Ack to the second provider. */
            consumer.submit( postMsg.clear().postId( ++postId ).serviceId(serviceId)
                    .name( "IBM.N" ).solicitAck( true ).complete(true)
                    .payload(nestedUpdateMsg), loginHandle );

            Thread.sleep(2000);

            /* Checks to ensure that the provider receives the PostMsg */
            assertEquals(1, providerClient1.queueSize());

            message = providerClient1.popMessage();

            assertTrue(message instanceof PostMsg);

            recvPostMsg = (PostMsg)message;

            assertEquals("DIRECT_FEED", recvPostMsg.serviceName());
            assertEquals("IBM.N", recvPostMsg.name());
            assertEquals(2, recvPostMsg.postId());
            assertEquals(true, recvPostMsg.complete());
            assertEquals(DataType.DataTypes.UPDATE_MSG, recvPostMsg.payload().dataType());

            updateMsg = recvPostMsg.payload().updateMsg();
            assertEquals(DataType.DataTypes.FIELD_LIST, updateMsg.payload().dataType());

            Thread.sleep(2000);
            /* Ensure that the client side receives ACK message from provider */
            assertEquals(1, consumerClient.queueSize());

            message = consumerClient.popMessage();

            AckMsg ackMessage = (AckMsg)message;

            assertTrue(ackMessage.hasServiceName());
            assertEquals("DIRECT_FEED", ackMessage.serviceName());
            assertEquals(serviceId, ackMessage.serviceId());
            assertEquals("IBM.N", ackMessage.name());
            assertEquals(2, ackMessage.ackId());

            consumer.unregister(loginHandle);
        }
        catch(OmmException excep)
        {
            System.out.println("Catch OmmException: " + excep.toString());
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
    public void testMultiConnectionsSubmittingGenericMsgWithoutSpecifyingServiceId()
    {
        TestUtilities.printTestHead("testMultiConnectionsSubmittingGenericMsgWithoutSpecifyingServiceId","");

        String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";

        OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);

        ProviderTestOptions providerTestOptions = new ProviderTestOptions();
        providerTestOptions.sendGenericMessage = true;

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

            ReqMsg reqMsg = EmaFactory.createReqMsg();


            long itemHandle = consumer.registerClient(reqMsg.clear().serviceName("DIRECT_FEED").name("LSEG.O"), consumerClient);

            Thread.sleep(1000);

            assertEquals(1, providerClient1.queueSize());

            Msg message = providerClient1.popMessage();

            assertTrue(message instanceof ReqMsg);

            ReqMsg requestMsg = (ReqMsg)message;

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

            /* Submit a GenericMsg to the first item stream */
            GenericMsg genericMsg = EmaFactory.createGenericMsg();
            UpdateMsg nestedUpdateMsg = EmaFactory.createUpdateMsg();
            FieldList nestedFieldList = EmaFactory.createFieldList();

            nestedFieldList.add(EmaFactory.createFieldEntry().real(22, 34, OmmReal.MagnitudeType.EXPONENT_POS_1));
            nestedFieldList.add(EmaFactory.createFieldEntry().real(25, 35, OmmReal.MagnitudeType.EXPONENT_POS_1));
            nestedFieldList.add(EmaFactory.createFieldEntry().time(18, 11, 29, 30));
            nestedFieldList.add(EmaFactory.createFieldEntry().enumValue(37, 3));

            nestedUpdateMsg.payload(nestedFieldList );

            consumer.submit( genericMsg.name("genericMsg").domainType(200).complete(true).payload(nestedUpdateMsg), itemHandle );

            Thread.sleep(1000);

            /* Checks to ensure that the provider receives the GenericMsg */
            assertEquals(1, providerClient1.queueSize());

            message = providerClient1.popMessage();

            assertTrue(message instanceof GenericMsg);

            GenericMsg recvGenericMsg = (GenericMsg)message;

            assertEquals("genericMsg", recvGenericMsg.name());
            assertEquals(200, recvGenericMsg.domainType());
            assertFalse(recvGenericMsg.hasServiceId());
            assertEquals(true, recvGenericMsg.complete());
            assertEquals(DataType.DataTypes.UPDATE_MSG, recvGenericMsg.payload().dataType());

            UpdateMsg updateMsg = recvGenericMsg.payload().updateMsg();
            assertEquals(DataType.DataTypes.FIELD_LIST, updateMsg.payload().dataType());

            /* Ensure there is no more message from provider */
            assertEquals(1, consumerClient.queueSize());

            message = consumerClient.popMessage();

            recvGenericMsg = (GenericMsg)message;
            assertEquals("genericMsg", recvGenericMsg.name());
            assertEquals(200, recvGenericMsg.domainType());
            assertFalse(recvGenericMsg.hasServiceId());
            assertEquals(true, recvGenericMsg.complete());
            assertEquals(DataType.DataTypes.NO_DATA, recvGenericMsg.payload().dataType());

            consumer.unregister(itemHandle);
        }
        catch(OmmException excep)
        {
            System.out.println("Catch OmmException: " + excep.toString());
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
    public void testMultiConnectionsSubmittingGenericMsgWithServiceIdAndUnknownServiceId()
    {
        TestUtilities.printTestHead("testMultiConnectionsSubmittingGenericMsgWithServiceIdAndUnknownServiceId","");

        String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";

        OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);

        ProviderTestOptions providerTestOptions = new ProviderTestOptions();
        providerTestOptions.sendGenericMessage = true;

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

            ReqMsg reqMsg = EmaFactory.createReqMsg();

            ConsumerSession<OmmConsumerClient> consumerSession = ((OmmConsumerImpl)consumer).consumerSession();


            long itemHandle = consumer.registerClient(reqMsg.clear().serviceName("DIRECT_FEED").name("LSEG.O"), consumerClient);

            Thread.sleep(1000);

            assertEquals(1, providerClient1.queueSize());

            Msg message = providerClient1.popMessage();

            assertTrue(message instanceof ReqMsg);

            ReqMsg requestMsg = (ReqMsg)message;

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

            /* Submit a GenericMsg to the first item stream */
            GenericMsg genericMsg = EmaFactory.createGenericMsg();
            UpdateMsg nestedUpdateMsg = EmaFactory.createUpdateMsg();
            FieldList nestedFieldList = EmaFactory.createFieldList();

            nestedFieldList.add(EmaFactory.createFieldEntry().real(22, 34, OmmReal.MagnitudeType.EXPONENT_POS_1));
            nestedFieldList.add(EmaFactory.createFieldEntry().real(25, 35, OmmReal.MagnitudeType.EXPONENT_POS_1));
            nestedFieldList.add(EmaFactory.createFieldEntry().time(18, 11, 29, 30));
            nestedFieldList.add(EmaFactory.createFieldEntry().enumValue(37, 3));

            nestedUpdateMsg.payload(nestedFieldList );

            consumer.submit( genericMsg.name("genericMsg").domainType(200).serviceId(consumerSession.sessionDirectoryByName("DIRECT_FEED").service().serviceId())
                    .complete(true).payload(nestedUpdateMsg), itemHandle );

            Thread.sleep(1000);

            /* Checks to ensure that the provider receives the GenericMsg */
            assertEquals(1, providerClient1.queueSize());

            message = providerClient1.popMessage();

            assertTrue(message instanceof GenericMsg);

            GenericMsg recvGenericMsg = (GenericMsg)message;

            assertEquals("genericMsg", recvGenericMsg.name());
            assertEquals(200, recvGenericMsg.domainType());
            assertEquals(1, recvGenericMsg.serviceId());
            assertEquals(true, recvGenericMsg.complete());
            assertEquals(DataType.DataTypes.UPDATE_MSG, recvGenericMsg.payload().dataType());

            UpdateMsg updateMsg = recvGenericMsg.payload().updateMsg();
            assertEquals(DataType.DataTypes.FIELD_LIST, updateMsg.payload().dataType());

            Thread.sleep(1000);

            /* Ensure that Consumer receives a generic message from provider */
            assertEquals(1, consumerClient.queueSize());

            message = consumerClient.popMessage();

            assertTrue(message instanceof GenericMsg);

            recvGenericMsg = (GenericMsg)message;

            assertEquals("genericMsg", recvGenericMsg.name());
            assertEquals(200, recvGenericMsg.domainType());
            assertEquals(consumerSession.sessionDirectoryByName("DIRECT_FEED").service().serviceId(), recvGenericMsg.serviceId());
            assertEquals(true, recvGenericMsg.complete());
            assertEquals(DataType.DataTypes.NO_DATA, recvGenericMsg.payload().dataType());

            /* Submit unknown service Id which there is no translation */
            consumer.submit( genericMsg.name("genericMsg2").domainType(205).serviceId(555)
                    .complete(true).payload(nestedUpdateMsg), itemHandle );

            Thread.sleep(1000);

            assertEquals(1, providerClient1.queueSize());

            message = providerClient1.popMessage();

            assertTrue(message instanceof GenericMsg);

            recvGenericMsg = (GenericMsg)message;

            assertEquals("genericMsg2", recvGenericMsg.name());
            assertEquals(205, recvGenericMsg.domainType());
            assertEquals(555, recvGenericMsg.serviceId());
            assertEquals(true, recvGenericMsg.complete());
            assertEquals(DataType.DataTypes.UPDATE_MSG, recvGenericMsg.payload().dataType());

            updateMsg = recvGenericMsg.payload().updateMsg();
            assertEquals(DataType.DataTypes.FIELD_LIST, updateMsg.payload().dataType());

            Thread.sleep(1000);

            /* Ensure that Consumer receives a generic message from provider */
            assertEquals(1, consumerClient.queueSize());

            message = consumerClient.popMessage();

            assertTrue(message instanceof GenericMsg);

            recvGenericMsg = (GenericMsg)message;

            assertEquals("genericMsg2", recvGenericMsg.name());
            assertEquals(205, recvGenericMsg.domainType());
            assertFalse(recvGenericMsg.hasServiceId());
            assertEquals(true, recvGenericMsg.complete());
            assertEquals(DataType.DataTypes.NO_DATA, recvGenericMsg.payload().dataType());


            consumer.unregister(itemHandle);
        }
        catch(OmmException excep)
        {
            System.out.println("Catch OmmException: " + excep.toString());
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
    public void testMultiConnectionsSubmittingGenericMsgWithoutSpecifyingServiceIdViaLoginDomain()
    {
        TestUtilities.printTestHead("testMultiConnectionsSubmittingGenericMsgWithoutSpecifyingServiceIdViaLoginDomain","");

        String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";

        OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);

        ProviderTestOptions providerTestOptions = new ProviderTestOptions();
        providerTestOptions.sendGenericMessage = true;

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

            ReqMsg reqMsg = EmaFactory.createReqMsg();

            long loginHandle = consumer.registerClient(reqMsg.domainType( EmaRdm.MMT_LOGIN ), consumerClient);

            /* Submit a GenericMsg to the first item stream */
            GenericMsg genericMsg = EmaFactory.createGenericMsg();
            UpdateMsg nestedUpdateMsg = EmaFactory.createUpdateMsg();
            FieldList nestedFieldList = EmaFactory.createFieldList();

            nestedFieldList.add(EmaFactory.createFieldEntry().real(22, 34, OmmReal.MagnitudeType.EXPONENT_POS_1));
            nestedFieldList.add(EmaFactory.createFieldEntry().real(25, 35, OmmReal.MagnitudeType.EXPONENT_POS_1));
            nestedFieldList.add(EmaFactory.createFieldEntry().time(18, 11, 29, 30));
            nestedFieldList.add(EmaFactory.createFieldEntry().enumValue(37, 3));

            nestedUpdateMsg.payload(nestedFieldList );

            consumer.submit( genericMsg.name("genericMsg").domainType(200).complete(true).payload(nestedUpdateMsg), loginHandle );

            Thread.sleep(1000);

            /* Checks to ensure that the first provider receives the GenericMsg */
            assertEquals(1, providerClient1.queueSize());

            Msg message = providerClient1.popMessage();

            assertTrue(message instanceof GenericMsg);

            GenericMsg recvGenericMsg = (GenericMsg)message;

            assertEquals(1, recvGenericMsg.streamId());
            assertEquals("genericMsg", recvGenericMsg.name());
            assertEquals(200, recvGenericMsg.domainType());
            assertFalse(recvGenericMsg.hasServiceId());
            assertEquals(true, recvGenericMsg.complete());
            assertEquals(DataType.DataTypes.UPDATE_MSG, recvGenericMsg.payload().dataType());

            UpdateMsg updateMsg = recvGenericMsg.payload().updateMsg();
            assertEquals(DataType.DataTypes.FIELD_LIST, updateMsg.payload().dataType());

            /* Checks to ensure that the second provider receives the GenericMsg */
            assertEquals(1, providerClient2.queueSize());

            message = providerClient2.popMessage();

            assertTrue(message instanceof GenericMsg);

            recvGenericMsg = (GenericMsg)message;

            assertEquals(1, recvGenericMsg.streamId());
            assertEquals("genericMsg", recvGenericMsg.name());
            assertEquals(200, recvGenericMsg.domainType());
            assertFalse(recvGenericMsg.hasServiceId());
            assertEquals(true, recvGenericMsg.complete());
            assertEquals(DataType.DataTypes.UPDATE_MSG, recvGenericMsg.payload().dataType());

            updateMsg = recvGenericMsg.payload().updateMsg();
            assertEquals(DataType.DataTypes.FIELD_LIST, updateMsg.payload().dataType());

            /* Ensure that receives two generic messages from the two providers.*/
            assertEquals(3, consumerClient.queueSize());

            int count = 0;
            for (int i = 0; i < 3; i++)
            {
                message = consumerClient.popMessage();
                if (message instanceof GenericMsg) {
                    count++;
                    recvGenericMsg = (GenericMsg)message;
                    assertEquals(1, recvGenericMsg.streamId());
                    assertEquals("genericMsg", recvGenericMsg.name());
                    assertEquals(200, recvGenericMsg.domainType());
                    assertFalse(recvGenericMsg.hasServiceId());
                    assertEquals(true, recvGenericMsg.complete());
                    assertEquals(DataType.DataTypes.NO_DATA, recvGenericMsg.payload().dataType());
                }
                else { assertTrue(message instanceof RefreshMsg);}
            }
            assertEquals(2, count);

            consumer.unregister(loginHandle);
        }
        catch(OmmException excep)
        {
            System.out.println("Catch OmmException: " + excep.toString());
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
    public void testMultiConnectionsSubmittingGenericMsgWithServiceIdAndUnknownServiceIdViaLoginDomain()
    {
        TestUtilities.printTestHead("testMultiConnectionsSubmittingGenericMsgWithServiceIdAndUnknownServiceIdViaLoginDomain","");

        String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";

        OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);

        ProviderTestOptions providerTestOptions = new ProviderTestOptions();
        providerTestOptions.sendGenericMessage = true;

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

            ReqMsg reqMsg = EmaFactory.createReqMsg();


            long loginHandle = consumer.registerClient(reqMsg.domainType( EmaRdm.MMT_LOGIN ), consumerClient);

            Thread.sleep(100);

            assertEquals(1, consumerClient.queueSize());

            /* This is login refresh message */
            Msg message = consumerClient.popMessage();

            /* Submit a GenericMsg to the first item stream */
            GenericMsg genericMsg = EmaFactory.createGenericMsg();
            UpdateMsg nestedUpdateMsg = EmaFactory.createUpdateMsg();
            FieldList nestedFieldList = EmaFactory.createFieldList();

            nestedFieldList.add(EmaFactory.createFieldEntry().real(22, 34, OmmReal.MagnitudeType.EXPONENT_POS_1));
            nestedFieldList.add(EmaFactory.createFieldEntry().real(25, 35, OmmReal.MagnitudeType.EXPONENT_POS_1));
            nestedFieldList.add(EmaFactory.createFieldEntry().time(18, 11, 29, 30));
            nestedFieldList.add(EmaFactory.createFieldEntry().enumValue(37, 3));

            nestedUpdateMsg.payload(nestedFieldList );

            consumer.submit( genericMsg.name("genericMsg").domainType(200)
                    .complete(true).payload(nestedUpdateMsg), loginHandle );

            Thread.sleep(1000);

            /* Checks to ensure that the first provider receives the GenericMsg */
            assertEquals(1, providerClient1.queueSize());

            message = providerClient1.popMessage();

            assertTrue(message instanceof GenericMsg);

            GenericMsg recvGenericMsg = (GenericMsg)message;

            assertEquals(1, recvGenericMsg.streamId());
            assertEquals("genericMsg", recvGenericMsg.name());
            assertEquals(200, recvGenericMsg.domainType());
            assertEquals(true, recvGenericMsg.complete());
            assertEquals(DataType.DataTypes.UPDATE_MSG, recvGenericMsg.payload().dataType());

            UpdateMsg updateMsg = recvGenericMsg.payload().updateMsg();
            assertEquals(DataType.DataTypes.FIELD_LIST, updateMsg.payload().dataType());

            /* Checks to ensure that the second provider receives the GenericMsg */
            assertEquals(1, providerClient2.queueSize());

            message = providerClient2.popMessage();

            assertTrue(message instanceof GenericMsg);

            recvGenericMsg = (GenericMsg)message;

            assertEquals(1, recvGenericMsg.streamId());
            assertEquals("genericMsg", recvGenericMsg.name());
            assertEquals(200, recvGenericMsg.domainType());
            assertEquals(true, recvGenericMsg.complete());
            assertEquals(DataType.DataTypes.UPDATE_MSG, recvGenericMsg.payload().dataType());

            updateMsg = recvGenericMsg.payload().updateMsg();
            assertEquals(DataType.DataTypes.FIELD_LIST, updateMsg.payload().dataType());

            Thread.sleep(1000);

            /* Ensure that Consumer receives a generic message from provider */
            assertEquals(2, consumerClient.queueSize());

            message = consumerClient.popMessage();

            assertTrue(message instanceof GenericMsg);

            recvGenericMsg = (GenericMsg)message;

            assertEquals(1, recvGenericMsg.streamId());
            assertEquals("genericMsg", recvGenericMsg.name());
            assertEquals(200, recvGenericMsg.domainType());
            assertFalse(recvGenericMsg.hasServiceId());
            assertEquals(true, recvGenericMsg.complete());
            assertEquals(DataType.DataTypes.NO_DATA, recvGenericMsg.payload().dataType());

            message = consumerClient.popMessage();

            assertTrue(message instanceof GenericMsg);

            recvGenericMsg = (GenericMsg)message;

            assertEquals(1, recvGenericMsg.streamId());
            assertEquals("genericMsg", recvGenericMsg.name());
            assertEquals(200, recvGenericMsg.domainType());
            assertFalse(recvGenericMsg.hasServiceId());
            assertEquals(true, recvGenericMsg.complete());
            assertEquals(DataType.DataTypes.NO_DATA, recvGenericMsg.payload().dataType());

            /* Submit unknown service Id which there is no translation */
            consumer.submit( genericMsg.name("genericMsg2").domainType(205).serviceId(555)
                    .complete(true).payload(nestedUpdateMsg), loginHandle );

            Thread.sleep(1000);

            assertEquals(1, providerClient1.queueSize());

            message = providerClient1.popMessage();

            assertTrue(message instanceof GenericMsg);

            recvGenericMsg = (GenericMsg)message;

            assertEquals(1, recvGenericMsg.streamId());
            assertEquals("genericMsg2", recvGenericMsg.name());
            assertEquals(205, recvGenericMsg.domainType());
            assertEquals(555, recvGenericMsg.serviceId());
            assertEquals(true, recvGenericMsg.complete());
            assertEquals(DataType.DataTypes.UPDATE_MSG, recvGenericMsg.payload().dataType());

            updateMsg = recvGenericMsg.payload().updateMsg();
            assertEquals(DataType.DataTypes.FIELD_LIST, updateMsg.payload().dataType());

            assertEquals(1, providerClient2.queueSize());

            message = providerClient2.popMessage();

            assertTrue(message instanceof GenericMsg);

            recvGenericMsg = (GenericMsg)message;

            assertEquals(1, recvGenericMsg.streamId());
            assertEquals("genericMsg2", recvGenericMsg.name());
            assertEquals(205, recvGenericMsg.domainType());
            assertEquals(555, recvGenericMsg.serviceId());
            assertEquals(true, recvGenericMsg.complete());
            assertEquals(DataType.DataTypes.UPDATE_MSG, recvGenericMsg.payload().dataType());

            updateMsg = recvGenericMsg.payload().updateMsg();
            assertEquals(DataType.DataTypes.FIELD_LIST, updateMsg.payload().dataType());

            Thread.sleep(1000);

            /* Ensure that receives two generic messages from the two providers.*/
            assertEquals(2, consumerClient.queueSize());

            message = consumerClient.popMessage();

            assertTrue(message instanceof GenericMsg);

            recvGenericMsg = (GenericMsg)message;

            assertEquals(1, recvGenericMsg.streamId());
            assertEquals("genericMsg2", recvGenericMsg.name());
            assertEquals(205, recvGenericMsg.domainType());
            assertFalse(recvGenericMsg.hasServiceId());
            assertEquals(true, recvGenericMsg.complete());
            assertEquals(DataType.DataTypes.NO_DATA, recvGenericMsg.payload().dataType());

            message = consumerClient.popMessage();

            assertTrue(message instanceof GenericMsg);

            recvGenericMsg = (GenericMsg)message;

            assertEquals(1, recvGenericMsg.streamId());
            assertEquals("genericMsg2", recvGenericMsg.name());
            assertEquals(205, recvGenericMsg.domainType());
            assertFalse(recvGenericMsg.hasServiceId());
            assertEquals(true, recvGenericMsg.complete());
            assertEquals(DataType.DataTypes.NO_DATA, recvGenericMsg.payload().dataType());

            consumer.unregister(loginHandle);
        }
        catch(OmmException excep)
        {
            System.out.println("Catch OmmException: " + excep.toString());
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
    public void testMultiConnectionsForSingleItemWithWSBChannelsAndDownloadingNetworkDictionary()
    {
        TestUtilities.printTestHead("testMultiConnectionsForSingleItemWithWSBChannels","");

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
        	// Consumer_13_1 is configured to download dictionary from network
            consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_13_1"));

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
            assertEquals(DataType.DataTypes.MAP, genericMsg.payload().dataType());

            Map map = genericMsg.payload().map();
            Iterator<MapEntry> mapIt = map.iterator();
            assertTrue(mapIt.hasNext());
            MapEntry mapEntry = mapIt.next();
            assertEquals(EmaRdm.ENAME_WARMSTANDBY_INFO, mapEntry.key().ascii().toString());
            assertEquals(DataType.DataTypes.ELEMENT_LIST, mapEntry.loadType());

            ElementList elementList = mapEntry.elementList();
            Iterator<ElementEntry> elementListIt = elementList.iterator();
            assertTrue(elementListIt.hasNext());
            ElementEntry elementEntry = elementListIt.next();
            assertEquals(EmaRdm.ENAME_WARMSTANDBY_MODE, elementEntry.name());
            assertEquals(Login.ServerTypes.ACTIVE, elementEntry.uintValue());
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
            assertEquals(DataType.DataTypes.MAP, genericMsg.payload().dataType());

            map = genericMsg.payload().map();
            mapIt = map.iterator();
            assertTrue(mapIt.hasNext());
            mapEntry = mapIt.next();
            assertEquals(EmaRdm.ENAME_WARMSTANDBY_INFO, mapEntry.key().ascii().toString());
            assertEquals(DataType.DataTypes.ELEMENT_LIST, mapEntry.loadType());

            elementList = mapEntry.elementList();
            elementListIt = elementList.iterator();
            assertTrue(elementListIt.hasNext());
            elementEntry = elementListIt.next();
            assertEquals(EmaRdm.ENAME_WARMSTANDBY_MODE, elementEntry.name());
            assertEquals(Login.ServerTypes.STANDBY, elementEntry.uintValue());
            assertFalse(elementListIt.hasNext()); // End of ElementList

            assertFalse(mapIt.hasNext()); // End of Map

            /* Market price request message for standby server for the first connection */
            message = providerClient2.popMessage();
            recvReq = (ReqMsg)message;

            assertEquals(5, recvReq.streamId());
            assertEquals("DIRECT_FEED", recvReq.serviceName());
            assertEquals("IBM.N", recvReq.name());
            assertEquals(1, recvReq.serviceId());

            /* Receives one generic messages of active server for the second connection */
            assertEquals(1, providerClient3.queueSize());

            /* GenericMsg for active server for the second connection */
            message = providerClient3.popMessage();
            genericMsg = (GenericMsg)message;
            assertEquals(1, genericMsg.streamId());
            assertEquals(DomainTypes.LOGIN, genericMsg.domainType());
            assertEquals(EmaRdm.ENAME_CONS_CONN_STATUS, genericMsg.name());
            assertEquals(DataType.DataTypes.MAP, genericMsg.payload().dataType());

            map = genericMsg.payload().map();
            mapIt = map.iterator();
            assertTrue(mapIt.hasNext());
            mapEntry = mapIt.next();
            assertEquals(EmaRdm.ENAME_WARMSTANDBY_INFO, mapEntry.key().ascii().toString());
            assertEquals(DataType.DataTypes.ELEMENT_LIST, mapEntry.loadType());

            elementList = mapEntry.elementList();
            elementListIt = elementList.iterator();
            assertTrue(elementListIt.hasNext());
            elementEntry = elementListIt.next();
            assertEquals(EmaRdm.ENAME_WARMSTANDBY_MODE, elementEntry.name());
            assertEquals(Login.ServerTypes.ACTIVE, elementEntry.uintValue());
            assertFalse(elementListIt.hasNext()); // End of ElementList

            assertFalse(mapIt.hasNext()); // End of Map

            /* Receives one generic messages of standby server for the second connection */
            assertEquals(1, providerClient4.queueSize());
            /* GenericMsg for standby server for the second connection */
            message = providerClient4.popMessage();
            genericMsg = (GenericMsg)message;
            assertEquals(1, genericMsg.streamId());
            assertEquals(DomainTypes.LOGIN, genericMsg.domainType());
            assertEquals(EmaRdm.ENAME_CONS_CONN_STATUS, genericMsg.name());
            assertEquals(DataType.DataTypes.MAP, genericMsg.payload().dataType());

            map = genericMsg.payload().map();
            mapIt = map.iterator();
            assertTrue(mapIt.hasNext());
            mapEntry = mapIt.next();
            assertEquals(EmaRdm.ENAME_WARMSTANDBY_INFO, mapEntry.key().ascii().toString());
            assertEquals(DataType.DataTypes.ELEMENT_LIST, mapEntry.loadType());

            elementList = mapEntry.elementList();
            elementListIt = elementList.iterator();
            assertTrue(elementListIt.hasNext());
            elementEntry = elementListIt.next();
            assertEquals(EmaRdm.ENAME_WARMSTANDBY_MODE, elementEntry.name());
            assertEquals(Login.ServerTypes.STANDBY, elementEntry.uintValue());
            assertFalse(elementListIt.hasNext()); // End of ElementList

            assertFalse(mapIt.hasNext()); // End of Map

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
            assertEquals(32767, refreshMsg.serviceId());
            assertEquals("DIRECT_FEED", refreshMsg.serviceName());
            assertEquals("IBM.N", refreshMsg.name());
            assertEquals(DataType.DataTypes.FIELD_LIST, refreshMsg.payload().dataType());
            
            OmmConsumerTests.checkFieldListFromRefreshMsg(refreshMsg.payload().fieldList());

            assertEquals(1, consumerClient.channelInfoSize());
            ChannelInformation channelInfo = consumerClient.popChannelInfo();
            assertEquals("Channel_3", channelInfo.channelName());
            assertEquals("Connection_3", channelInfo.sessionChannelName());

            assertEquals(1, consumerClient.sessionChannelInfoSize());

            Iterator<ChannelInformation> it = consumerClient.popSessionChannelInfo().iterator();
            assertTrue(it.hasNext());
            channelInfo = it.next();
            assertEquals("Channel_3", channelInfo.channelName());
            assertEquals("Connection_3", channelInfo.sessionChannelName());
            assertTrue(it.hasNext());
            channelInfo = it.next();
            // assertEquals("Channel_7", channelInfo.channelName()); // Can't assert that it is going to be Channel_7 - Channel_8 might still be initializing.
            assertEquals("Connection_4", channelInfo.sessionChannelName());
            assertFalse(it.hasNext());

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
            assertEquals(DataType.DataTypes.MAP, genericMsg.payload().dataType());

            map = genericMsg.payload().map();
            mapIt = map.iterator();
            assertTrue(mapIt.hasNext());
            mapEntry = mapIt.next();
            assertEquals(EmaRdm.ENAME_WARMSTANDBY_INFO, mapEntry.key().ascii().toString());
            assertEquals(DataType.DataTypes.ELEMENT_LIST, mapEntry.loadType());

            elementList = mapEntry.elementList();
            elementListIt = elementList.iterator();
            assertTrue(elementListIt.hasNext());
            elementEntry = elementListIt.next();
            assertEquals(EmaRdm.ENAME_WARMSTANDBY_MODE, elementEntry.name());
            assertEquals(Login.ServerTypes.ACTIVE, elementEntry.uintValue());
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
            assertEquals("Connection_3", channelInfo.sessionChannelName());

            assertEquals(2, consumerClient.sessionChannelInfoSize());

            it = consumerClient.popSessionChannelInfo().iterator();
            assertTrue(it.hasNext());
            channelInfo = it.next();
            assertEquals("Channel_3", channelInfo.channelName());
            assertEquals("Connection_3", channelInfo.sessionChannelName());
            assertTrue(it.hasNext());
            channelInfo = it.next();
            assertEquals("Channel_8", channelInfo.channelName());
            assertEquals("Connection_4", channelInfo.sessionChannelName());
            assertFalse(it.hasNext());

            message = consumerClient.popMessage();
            refreshMsg = (RefreshMsg)message;

            assertEquals(5, refreshMsg.streamId());
            assertEquals(DomainTypes.MARKET_PRICE, refreshMsg.domainType());
            assertEquals("Open / Ok / None / 'Unsolicited Refresh Completed'", refreshMsg.state().toString());
            assertFalse(refreshMsg.solicited());
            assertTrue(refreshMsg.complete());
            assertTrue(refreshMsg.hasMsgKey());
            assertEquals(32767, refreshMsg.serviceId());
            assertEquals("DIRECT_FEED", refreshMsg.serviceName());
            assertEquals("IBM.N", refreshMsg.name());
            assertEquals(DataType.DataTypes.FIELD_LIST, refreshMsg.payload().dataType());
            
            OmmConsumerTests.checkFieldListFromRefreshMsg(refreshMsg.payload().fieldList());

            channelInfo = consumerClient.popChannelInfo();
            assertEquals("Channel_6", channelInfo.channelName());
            assertEquals("Connection_3", channelInfo.sessionChannelName());

            it = consumerClient.popSessionChannelInfo().iterator();
            assertTrue(it.hasNext());
            channelInfo = it.next();
            assertEquals("Channel_6", channelInfo.channelName());
            assertEquals("Connection_3", channelInfo.sessionChannelName());
            assertTrue(it.hasNext());
            channelInfo = it.next();
            assertEquals("Channel_8", channelInfo.channelName());
            assertEquals("Connection_4", channelInfo.sessionChannelName());
            assertFalse(it.hasNext());

            /* Closes the new active server for the first connection */
            ommprovider2.uninitialize();

            Thread.sleep(2000);

            assertEquals(1, providerClient3.queueSize());

            /* Receives market price request message for active server for the second connection */
            message = providerClient3.popMessage();
            recvReq = (ReqMsg)message;

            assertEquals(3, recvReq.streamId());
            assertEquals("DIRECT_FEED", recvReq.serviceName());
            assertEquals("IBM.N", recvReq.name());
            assertEquals(1, recvReq.serviceId());

            assertEquals(1, providerClient4.queueSize());

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
            assertEquals("Connection_3", channelInfo.sessionChannelName());

            assertEquals(2, consumerClient.sessionChannelInfoSize());
            it = consumerClient.popSessionChannelInfo().iterator();
            assertTrue(it.hasNext());
            channelInfo = it.next();
            assertEquals("Channel_6", channelInfo.channelName());
            assertEquals("Connection_3", channelInfo.sessionChannelName());
            assertTrue(it.hasNext());
            channelInfo = it.next();
            assertEquals("Channel_8", channelInfo.channelName());
            assertEquals("Connection_4", channelInfo.sessionChannelName());
            assertFalse(it.hasNext());

            message = consumerClient.popMessage();

            /* Receives Refresh message from the active server of the second connection */
            refreshMsg = (RefreshMsg)message;
            assertEquals(5, refreshMsg.streamId());
            assertEquals(DomainTypes.MARKET_PRICE, refreshMsg.domainType());
            assertEquals("Open / Ok / None / 'Refresh Completed'", refreshMsg.state().toString());
            assertTrue(refreshMsg.solicited());
            assertTrue(refreshMsg.complete());
            assertTrue(refreshMsg.hasMsgKey());
            assertEquals(32767, refreshMsg.serviceId());
            assertEquals("DIRECT_FEED", refreshMsg.serviceName());
            assertEquals("IBM.N", refreshMsg.name());
            assertEquals(DataType.DataTypes.FIELD_LIST, refreshMsg.payload().dataType());
            
            OmmConsumerTests.checkFieldListFromRefreshMsg(refreshMsg.payload().fieldList());

            channelInfo = consumerClient.popChannelInfo();
            assertEquals("Channel_7", channelInfo.channelName());
            assertEquals("Connection_4", channelInfo.sessionChannelName());

            it = consumerClient.popSessionChannelInfo().iterator();
            assertTrue(it.hasNext());
            channelInfo = it.next();
            assertEquals("Channel_6", channelInfo.channelName());
            assertEquals("Connection_3", channelInfo.sessionChannelName());
            assertTrue(it.hasNext());
            channelInfo = it.next();
            assertEquals("Channel_7", channelInfo.channelName());
            assertEquals("Connection_4", channelInfo.sessionChannelName());
            assertFalse(it.hasNext());

            consumer.unregister(itemHandle);
        }
        catch(OmmException ex)
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
    public void testMultiConnectionsSingleItemRequestWithUnmatchedQoSThenServiceProvideTheRequestedQoS()
    {
        TestUtilities.printTestHead("testMultiConnectionsSingleItemRequestWithUnmatchedQoSThenServiceProvideTheRequestedQoS","");

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

            ReqMsg reqMsg = EmaFactory.createReqMsg();

            long itemHandle = consumer.registerClient(reqMsg.clear().serviceName("DIRECT_FEED").name("LSEG.O").qos(55, 55), consumerClient);

            Thread.sleep(2000);

            /* Ensure that the provider doesn't receive the request message as the requested QoS doesn't match */
            assertEquals(0, providerClient1.queueSize());

            /* Receive a StatusMsg from EMA */
            assertEquals(1, consumerClient.queueSize());

            Msg message = consumerClient.popMessage();

            StatusMsg statusMsg = (StatusMsg)message;

            assertEquals("DIRECT_FEED", statusMsg.serviceName());
            assertEquals("LSEG.O", statusMsg.name());
            assertEquals(OmmState.StreamState.OPEN, statusMsg.state().streamState());
            assertEquals(OmmState.DataState.SUSPECT, statusMsg.state().dataState());
            assertEquals(OmmState.StatusCode.NONE, statusMsg.state().statusCode());
            assertEquals("Service does not provide a matching QoS", statusMsg.state().statusText());

            /* Provider sends source directory update to delete the DIRECT_FEED service name. */
            Map map = EmaFactory.createMap();
            map.add( EmaFactory.createMapEntry().keyUInt(1, MapEntry.MapAction.DELETE,  EmaFactory.createFilterList()));

            UpdateMsg updateMsg = EmaFactory.createUpdateMsg();
            ommprovider.submit( updateMsg.domainType(EmaRdm.MMT_DIRECTORY).
                    filter( EmaRdm.SERVICE_INFO_FILTER | EmaRdm.SERVICE_STATE_FILTER).
                    payload(map), 0);

            Thread.sleep(1000);

            /* Provider sends source directory update message to add the DIRECT_FEED service */
            OmmArray capablities = EmaFactory.createOmmArray();
            capablities.add(EmaFactory.createOmmArrayEntry().uintValue( EmaRdm.MMT_MARKET_PRICE));
            capablities.add(EmaFactory.createOmmArrayEntry().uintValue( EmaRdm.MMT_MARKET_BY_PRICE));
            OmmArray dictionaryUsed = EmaFactory.createOmmArray();
            dictionaryUsed.add(EmaFactory.createOmmArrayEntry().ascii( "RWFFld"));
            dictionaryUsed.add(EmaFactory.createOmmArrayEntry().ascii( "RWFEnum"));

            OmmArray qosList = EmaFactory.createOmmArray();
            qosList.add(EmaFactory.createOmmArrayEntry().qos(OmmQos.Timeliness.REALTIME, OmmQos.Rate.TICK_BY_TICK));
            qosList.add(EmaFactory.createOmmArrayEntry().qos(55, 55));

            ElementList serviceInfoId = EmaFactory.createElementList();

            serviceInfoId.add( EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_NAME, "DIRECT_FEED"));
            serviceInfoId.add( EmaFactory.createElementEntry().array(EmaRdm.ENAME_CAPABILITIES, capablities));
            serviceInfoId.add( EmaFactory.createElementEntry().array(EmaRdm.ENAME_DICTIONARYS_USED, dictionaryUsed));
            serviceInfoId.add( EmaFactory.createElementEntry().array(EmaRdm.ENAME_QOS, qosList));
            serviceInfoId.add( EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_ITEM_LIST, "#.itemlist"));
            serviceInfoId.add( EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_SUPPS_QOS_RANGE, 0));

            ElementList serviceStateId = EmaFactory.createElementList();
            serviceStateId.add( EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_UP));
            serviceStateId.add( EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_ACCEPTING_REQS, 1));

            FilterList filterList = EmaFactory.createFilterList();
            filterList.add( EmaFactory.createFilterEntry().elementList(EmaRdm.SERVICE_INFO_ID, FilterEntry.FilterAction.SET, serviceInfoId) );
            filterList.add( EmaFactory.createFilterEntry().elementList(EmaRdm.SERVICE_STATE_ID, FilterEntry.FilterAction.SET, serviceStateId));

            map.clear();
            map.add( EmaFactory.createMapEntry().keyUInt(1, MapEntry.MapAction.ADD, filterList));

            updateMsg.clear();
            ommprovider.submit( updateMsg.domainType(EmaRdm.MMT_DIRECTORY).
                    filter( EmaRdm.SERVICE_INFO_FILTER | EmaRdm.SERVICE_STATE_FILTER).
                    payload(map), 0);

            Thread.sleep(2000);

            /* Provider receives the request message */
            assertEquals(1, providerClient1.queueSize());

            message = providerClient1.popMessage();

            ReqMsg recvReq = (ReqMsg)message;

            assertEquals(3, recvReq.streamId());
            assertEquals("DIRECT_FEED", recvReq.serviceName());
            assertEquals("LSEG.O", recvReq.name());
            assertEquals(1, recvReq.serviceId());

            /* Receives a refresh from the provider */
            assertEquals(1, consumerClient.queueSize());
            message = consumerClient.popMessage();

            /* Receives Refresh message from the active server of the second connection */
            RefreshMsg refreshMsg = (RefreshMsg)message;
            assertEquals(5, refreshMsg.streamId());
            assertEquals(DomainTypes.MARKET_PRICE, refreshMsg.domainType());
            assertEquals("Open / Ok / None / 'Refresh Completed'", refreshMsg.state().toString());
            assertTrue(refreshMsg.solicited());
            assertTrue(refreshMsg.complete());
            assertTrue(refreshMsg.hasMsgKey());
            assertEquals(consumerSession.sessionDirectoryByName("DIRECT_FEED").service().serviceId(), refreshMsg.serviceId());
            assertEquals("DIRECT_FEED", refreshMsg.serviceName());
            assertEquals("LSEG.O", refreshMsg.name());
            assertEquals(DataType.DataTypes.FIELD_LIST, refreshMsg.payload().dataType());

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
    public void testMultiConnectionsSingleItemRequestServiceListWithUnmatchedQoSThenServiceProvideTheRequestedQoS()
    {
        TestUtilities.printTestHead("testMultiConnectionsSingleItemRequestServiceListWithUnmatchedQoSThenServiceProvideTheRequestedQoS","");

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

            long itemHandle = consumer.registerClient(reqMsg.clear().serviceListName("SVG1").name("LSEG.O").qos(55, 55), consumerClient);

            Thread.sleep(2000);

            /* Ensure that the provider doesn't receive the request message as the requested QoS doesn't match */
            assertEquals(0, providerClient1.queueSize());

            /* Receive a StatusMsg from EMA */
            assertEquals(1, consumerClient.queueSize());

            Msg message = consumerClient.popMessage();

            StatusMsg statusMsg = (StatusMsg)message;

            assertEquals("SVG1", statusMsg.serviceName());
            assertEquals("LSEG.O", statusMsg.name());
            assertEquals(OmmState.StreamState.OPEN, statusMsg.state().streamState());
            assertEquals(OmmState.DataState.SUSPECT, statusMsg.state().dataState());
            assertEquals(OmmState.StatusCode.NONE, statusMsg.state().statusCode());
            assertEquals("Service does not provide a matching QoS", statusMsg.state().statusText());

            /* Provider sends source directory update to delete the DIRECT_FEED service name. */
            Map map = EmaFactory.createMap();
            map.add( EmaFactory.createMapEntry().keyUInt(1, MapEntry.MapAction.DELETE,  EmaFactory.createFilterList()));

            UpdateMsg updateMsg = EmaFactory.createUpdateMsg();
            ommprovider.submit( updateMsg.domainType(EmaRdm.MMT_DIRECTORY).
                    filter( EmaRdm.SERVICE_INFO_FILTER | EmaRdm.SERVICE_STATE_FILTER).
                    payload(map), 0);

            Thread.sleep(1000);

            /* Provider sends source directory update message to add the DIRECT_FEED service */
            OmmArray capablities = EmaFactory.createOmmArray();
            capablities.add(EmaFactory.createOmmArrayEntry().uintValue( EmaRdm.MMT_MARKET_PRICE));
            capablities.add(EmaFactory.createOmmArrayEntry().uintValue( EmaRdm.MMT_MARKET_BY_PRICE));
            OmmArray dictionaryUsed = EmaFactory.createOmmArray();
            dictionaryUsed.add(EmaFactory.createOmmArrayEntry().ascii( "RWFFld"));
            dictionaryUsed.add(EmaFactory.createOmmArrayEntry().ascii( "RWFEnum"));

            OmmArray qosList = EmaFactory.createOmmArray();
            qosList.add(EmaFactory.createOmmArrayEntry().qos(OmmQos.Timeliness.REALTIME, OmmQos.Rate.TICK_BY_TICK));
            qosList.add(EmaFactory.createOmmArrayEntry().qos(55, 55));

            ElementList serviceInfoId = EmaFactory.createElementList();

            serviceInfoId.add( EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_NAME, "DIRECT_FEED"));
            serviceInfoId.add( EmaFactory.createElementEntry().array(EmaRdm.ENAME_CAPABILITIES, capablities));
            serviceInfoId.add( EmaFactory.createElementEntry().array(EmaRdm.ENAME_DICTIONARYS_USED, dictionaryUsed));
            serviceInfoId.add( EmaFactory.createElementEntry().array(EmaRdm.ENAME_QOS, qosList));
            serviceInfoId.add( EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_ITEM_LIST, "#.itemlist"));
            serviceInfoId.add( EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_SUPPS_QOS_RANGE, 0));

            ElementList serviceStateId = EmaFactory.createElementList();
            serviceStateId.add( EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_UP));
            serviceStateId.add( EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_ACCEPTING_REQS, 1));

            FilterList filterList = EmaFactory.createFilterList();
            filterList.add( EmaFactory.createFilterEntry().elementList(EmaRdm.SERVICE_INFO_ID, FilterEntry.FilterAction.SET, serviceInfoId) );
            filterList.add( EmaFactory.createFilterEntry().elementList(EmaRdm.SERVICE_STATE_ID, FilterEntry.FilterAction.SET, serviceStateId));

            map.clear();
            map.add( EmaFactory.createMapEntry().keyUInt(1, MapEntry.MapAction.ADD, filterList));

            updateMsg.clear();
            ommprovider.submit( updateMsg.domainType(EmaRdm.MMT_DIRECTORY).
                    filter( EmaRdm.SERVICE_INFO_FILTER | EmaRdm.SERVICE_STATE_FILTER).
                    payload(map), 0);

            Thread.sleep(2000);

            /* Provider receives the request message */
            assertEquals(1, providerClient1.queueSize());

            message = providerClient1.popMessage();

            ReqMsg recvReq = (ReqMsg)message;

            assertEquals(3, recvReq.streamId());
            assertEquals("DIRECT_FEED", recvReq.serviceName());
            assertEquals("LSEG.O", recvReq.name());
            assertEquals(1, recvReq.serviceId());

            /* Receives a refresh from the provider */
            assertEquals(1, consumerClient.queueSize());
            message = consumerClient.popMessage();

            /* Receives Refresh message from the active server of the second connection */
            RefreshMsg refreshMsg = (RefreshMsg)message;
            assertEquals(5, refreshMsg.streamId());
            assertEquals(DomainTypes.MARKET_PRICE, refreshMsg.domainType());
            assertEquals("Open / Ok / None / 'Refresh Completed'", refreshMsg.state().toString());
            assertTrue(refreshMsg.solicited());
            assertTrue(refreshMsg.complete());
            assertTrue(refreshMsg.hasMsgKey());
            assertEquals(32767, refreshMsg.serviceId());
            assertEquals("SVG1", refreshMsg.serviceName());
            assertEquals("LSEG.O", refreshMsg.name());
            assertEquals(DataType.DataTypes.FIELD_LIST, refreshMsg.payload().dataType());

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
    public void testMultiConnectionsSingleItemRequestWithUnmatchedCapabilityThenServiceProvideTheRequestedCapability()
    {
        TestUtilities.printTestHead("testMultiConnectionsSingleItemRequestWithUnmatchedCapabilityThenServiceProvideTheRequestedCapability","");

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

            ReqMsg reqMsg = EmaFactory.createReqMsg();

            long itemHandle = consumer.registerClient(reqMsg.clear().serviceName("DIRECT_FEED").name("LSEG.O").domainType(55), consumerClient);

            Thread.sleep(2000);

            /* Ensure that the provider doesn't receive the request message as the requested QoS doesn't match */
            assertEquals(0, providerClient1.queueSize());

            /* Receive a StatusMsg from EMA */
            assertEquals(1, consumerClient.queueSize());

            Msg message = consumerClient.popMessage();

            StatusMsg statusMsg = (StatusMsg)message;

            assertEquals("DIRECT_FEED", statusMsg.serviceName());
            assertEquals("LSEG.O", statusMsg.name());
            assertEquals(OmmState.StreamState.OPEN, statusMsg.state().streamState());
            assertEquals(OmmState.DataState.SUSPECT, statusMsg.state().dataState());
            assertEquals(OmmState.StatusCode.NONE, statusMsg.state().statusCode());
            assertEquals("Capability not supported", statusMsg.state().statusText());

            /* Provider sends source directory update to delete the DIRECT_FEED service name. */
            Map map = EmaFactory.createMap();
            map.add( EmaFactory.createMapEntry().keyUInt(1, MapEntry.MapAction.DELETE,  EmaFactory.createFilterList()));

            UpdateMsg updateMsg = EmaFactory.createUpdateMsg();
            ommprovider.submit( updateMsg.domainType(EmaRdm.MMT_DIRECTORY).
                    filter( EmaRdm.SERVICE_INFO_FILTER | EmaRdm.SERVICE_STATE_FILTER).
                    payload(map), 0);

            Thread.sleep(1000);

            /* Provider sends source directory update message to add the DIRECT_FEED service */
            OmmArray capablities = EmaFactory.createOmmArray();
            capablities.add(EmaFactory.createOmmArrayEntry().uintValue( EmaRdm.MMT_MARKET_PRICE));
            capablities.add(EmaFactory.createOmmArrayEntry().uintValue( EmaRdm.MMT_MARKET_BY_PRICE));
            capablities.add(EmaFactory.createOmmArrayEntry().uintValue( 55));
            OmmArray dictionaryUsed = EmaFactory.createOmmArray();
            dictionaryUsed.add(EmaFactory.createOmmArrayEntry().ascii( "RWFFld"));
            dictionaryUsed.add(EmaFactory.createOmmArrayEntry().ascii( "RWFEnum"));

            OmmArray qosList = EmaFactory.createOmmArray();
            qosList.add(EmaFactory.createOmmArrayEntry().qos(OmmQos.Timeliness.REALTIME, OmmQos.Rate.TICK_BY_TICK));
            qosList.add(EmaFactory.createOmmArrayEntry().qos(55, 55));

            ElementList serviceInfoId = EmaFactory.createElementList();

            serviceInfoId.add( EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_NAME, "DIRECT_FEED"));
            serviceInfoId.add( EmaFactory.createElementEntry().array(EmaRdm.ENAME_CAPABILITIES, capablities));
            serviceInfoId.add( EmaFactory.createElementEntry().array(EmaRdm.ENAME_DICTIONARYS_USED, dictionaryUsed));
            serviceInfoId.add( EmaFactory.createElementEntry().array(EmaRdm.ENAME_QOS, qosList));
            serviceInfoId.add( EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_ITEM_LIST, "#.itemlist"));
            serviceInfoId.add( EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_SUPPS_QOS_RANGE, 0));

            ElementList serviceStateId = EmaFactory.createElementList();
            serviceStateId.add( EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_UP));
            serviceStateId.add( EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_ACCEPTING_REQS, 1));

            FilterList filterList = EmaFactory.createFilterList();
            filterList.add( EmaFactory.createFilterEntry().elementList(EmaRdm.SERVICE_INFO_ID, FilterEntry.FilterAction.SET, serviceInfoId) );
            filterList.add( EmaFactory.createFilterEntry().elementList(EmaRdm.SERVICE_STATE_ID, FilterEntry.FilterAction.SET, serviceStateId));

            map.clear();
            map.add( EmaFactory.createMapEntry().keyUInt(1, MapEntry.MapAction.ADD, filterList));

            updateMsg.clear();
            ommprovider.submit( updateMsg.domainType(EmaRdm.MMT_DIRECTORY).
                    filter( EmaRdm.SERVICE_INFO_FILTER | EmaRdm.SERVICE_STATE_FILTER).
                    payload(map), 0);

            Thread.sleep(2000);

            /* Provider receives the request message */
            assertEquals(1, providerClient1.queueSize());

            message = providerClient1.popMessage();

            ReqMsg recvReq = (ReqMsg)message;

            assertEquals(3, recvReq.streamId());
            assertEquals("DIRECT_FEED", recvReq.serviceName());
            assertEquals("LSEG.O", recvReq.name());
            assertEquals(1, recvReq.serviceId());

            /* Receives a refresh from the provider */
            assertEquals(1, consumerClient.queueSize());
            message = consumerClient.popMessage();

            /* Receives Refresh message from the active server of the second connection */
            RefreshMsg refreshMsg = (RefreshMsg)message;
            assertEquals(5, refreshMsg.streamId());
            assertEquals(55, refreshMsg.domainType());
            assertEquals("Open / Ok / None / 'Refresh Completed'", refreshMsg.state().toString());
            assertTrue(refreshMsg.solicited());
            assertTrue(refreshMsg.complete());
            assertTrue(refreshMsg.hasMsgKey());
            assertEquals(consumerSession.sessionDirectoryByName("DIRECT_FEED").service().serviceId(), refreshMsg.serviceId());
            assertEquals("DIRECT_FEED", refreshMsg.serviceName());
            assertEquals("LSEG.O", refreshMsg.name());
            assertEquals(DataType.DataTypes.FIELD_LIST, refreshMsg.payload().dataType());

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
    public void testMultiConnectionsSingleItemRequestServiceListWithUnmatchedCapabilityThenServiceProvideTheRequestedCapability()
    {
        TestUtilities.printTestHead("testMultiConnectionsSingleItemRequestServiceListWithUnmatchedCapabilityThenServiceProvideTheRequestedCapability","");

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

            long itemHandle = consumer.registerClient(reqMsg.clear().serviceListName("SVG1").name("LSEG.O").domainType(55), consumerClient);

            Thread.sleep(2000);

            /* Ensure that the provider doesn't receive the request message as the requested QoS doesn't match */
            assertEquals(0, providerClient1.queueSize());

            /* Receive a StatusMsg from EMA */
            assertEquals(1, consumerClient.queueSize());

            Msg message = consumerClient.popMessage();

            StatusMsg statusMsg = (StatusMsg)message;

            assertEquals("SVG1", statusMsg.serviceName());
            assertEquals("LSEG.O", statusMsg.name());
            assertEquals(OmmState.StreamState.OPEN, statusMsg.state().streamState());
            assertEquals(OmmState.DataState.SUSPECT, statusMsg.state().dataState());
            assertEquals(OmmState.StatusCode.NONE, statusMsg.state().statusCode());
            assertEquals("Capability not supported", statusMsg.state().statusText());

            /* Provider sends source directory update to delete the DIRECT_FEED service name. */
            Map map = EmaFactory.createMap();
            map.add( EmaFactory.createMapEntry().keyUInt(1, MapEntry.MapAction.DELETE,  EmaFactory.createFilterList()));

            UpdateMsg updateMsg = EmaFactory.createUpdateMsg();
            ommprovider.submit( updateMsg.domainType(EmaRdm.MMT_DIRECTORY).
                    filter( EmaRdm.SERVICE_INFO_FILTER | EmaRdm.SERVICE_STATE_FILTER).
                    payload(map), 0);

            Thread.sleep(1000);

            /* Provider sends source directory update message to add the DIRECT_FEED service */
            OmmArray capablities = EmaFactory.createOmmArray();
            capablities.add(EmaFactory.createOmmArrayEntry().uintValue( EmaRdm.MMT_MARKET_PRICE));
            capablities.add(EmaFactory.createOmmArrayEntry().uintValue( EmaRdm.MMT_MARKET_BY_PRICE));
            capablities.add(EmaFactory.createOmmArrayEntry().uintValue( 55));
            OmmArray dictionaryUsed = EmaFactory.createOmmArray();
            dictionaryUsed.add(EmaFactory.createOmmArrayEntry().ascii( "RWFFld"));
            dictionaryUsed.add(EmaFactory.createOmmArrayEntry().ascii( "RWFEnum"));

            OmmArray qosList = EmaFactory.createOmmArray();
            qosList.add(EmaFactory.createOmmArrayEntry().qos(OmmQos.Timeliness.REALTIME, OmmQos.Rate.TICK_BY_TICK));
            qosList.add(EmaFactory.createOmmArrayEntry().qos(55, 55));

            ElementList serviceInfoId = EmaFactory.createElementList();

            serviceInfoId.add( EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_NAME, "DIRECT_FEED"));
            serviceInfoId.add( EmaFactory.createElementEntry().array(EmaRdm.ENAME_CAPABILITIES, capablities));
            serviceInfoId.add( EmaFactory.createElementEntry().array(EmaRdm.ENAME_DICTIONARYS_USED, dictionaryUsed));
            serviceInfoId.add( EmaFactory.createElementEntry().array(EmaRdm.ENAME_QOS, qosList));
            serviceInfoId.add( EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_ITEM_LIST, "#.itemlist"));
            serviceInfoId.add( EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_SUPPS_QOS_RANGE, 0));

            ElementList serviceStateId = EmaFactory.createElementList();
            serviceStateId.add( EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_UP));
            serviceStateId.add( EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_ACCEPTING_REQS, 1));

            FilterList filterList = EmaFactory.createFilterList();
            filterList.add( EmaFactory.createFilterEntry().elementList(EmaRdm.SERVICE_INFO_ID, FilterEntry.FilterAction.SET, serviceInfoId) );
            filterList.add( EmaFactory.createFilterEntry().elementList(EmaRdm.SERVICE_STATE_ID, FilterEntry.FilterAction.SET, serviceStateId));

            map.clear();
            map.add( EmaFactory.createMapEntry().keyUInt(1, MapEntry.MapAction.ADD, filterList));

            updateMsg.clear();
            ommprovider.submit( updateMsg.domainType(EmaRdm.MMT_DIRECTORY).
                    filter( EmaRdm.SERVICE_INFO_FILTER | EmaRdm.SERVICE_STATE_FILTER).
                    payload(map), 0);

            Thread.sleep(2000);

            /* Provider receives the request message */
            assertEquals(1, providerClient1.queueSize());

            message = providerClient1.popMessage();

            ReqMsg recvReq = (ReqMsg)message;

            assertEquals(3, recvReq.streamId());
            assertEquals("DIRECT_FEED", recvReq.serviceName());
            assertEquals("LSEG.O", recvReq.name());
            assertEquals(1, recvReq.serviceId());

            /* Receives a refresh from the provider */
            assertEquals(1, consumerClient.queueSize());
            message = consumerClient.popMessage();

            /* Receives Refresh message from the active server of the second connection */
            RefreshMsg refreshMsg = (RefreshMsg)message;
            assertEquals(5, refreshMsg.streamId());
            assertEquals(55, refreshMsg.domainType());
            assertEquals("Open / Ok / None / 'Refresh Completed'", refreshMsg.state().toString());
            assertTrue(refreshMsg.solicited());
            assertTrue(refreshMsg.complete());
            assertTrue(refreshMsg.hasMsgKey());
            assertEquals(32767, refreshMsg.serviceId());
            assertEquals("SVG1", refreshMsg.serviceName());
            assertEquals("LSEG.O", refreshMsg.name());
            assertEquals(DataType.DataTypes.FIELD_LIST, refreshMsg.payload().dataType());

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
    public void testMultiConnectionsForSingleItemWithWSBChannelsWithPrivateStream()
    {
        TestUtilities.printTestHead("testMultiConnectionsForSingleItemWithWSBChannelsWithPrivateStream","");

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
        ConsumerTestClient consumerClient = new ConsumerTestClient();

        try
        {
            consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_13"));

            ReqMsg reqMsg = EmaFactory.createReqMsg();

            consumer.registerClient(reqMsg.name("IBM.N").serviceName("DIRECT_FEED").privateStream(true), consumerClient);

            Thread.sleep(4000);

            /* Receives one generic message and one request message of active server for the first connection */
            assertEquals(2, providerClient.queueSize());

            Msg message = providerClient.popMessage();

            /* GenericMsg for active server for the first connection */
            GenericMsg genericMsg = (GenericMsg)message;
            assertEquals(1, genericMsg.streamId());
            assertEquals(DomainTypes.LOGIN, genericMsg.domainType());
            assertEquals(EmaRdm.ENAME_CONS_CONN_STATUS, genericMsg.name());
            assertEquals(DataType.DataTypes.MAP, genericMsg.payload().dataType());

            Map map = genericMsg.payload().map();
            Iterator<MapEntry> mapIt = map.iterator();
            assertTrue(mapIt.hasNext());
            MapEntry mapEntry = mapIt.next();
            assertEquals(EmaRdm.ENAME_WARMSTANDBY_INFO, mapEntry.key().ascii().toString());
            assertEquals(DataType.DataTypes.ELEMENT_LIST, mapEntry.loadType());

            ElementList elementList = mapEntry.elementList();
            Iterator<ElementEntry> elementListIt = elementList.iterator();
            assertTrue(elementListIt.hasNext());
            ElementEntry elementEntry = elementListIt.next();
            assertEquals(EmaRdm.ENAME_WARMSTANDBY_MODE, elementEntry.name());
            assertEquals(Login.ServerTypes.ACTIVE, elementEntry.uintValue());
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

            /* Receives only one generic message of standby server for the first connection */
            assertEquals(1, providerClient2.queueSize());
            /* GenericMsg for standby server for the first connection */
            message = providerClient2.popMessage();
            genericMsg = (GenericMsg)message;
            assertEquals(1, genericMsg.streamId());
            assertEquals(DomainTypes.LOGIN, genericMsg.domainType());
            assertEquals(EmaRdm.ENAME_CONS_CONN_STATUS, genericMsg.name());
            assertEquals(DataType.DataTypes.MAP, genericMsg.payload().dataType());

            map = genericMsg.payload().map();
            mapIt = map.iterator();
            assertTrue(mapIt.hasNext());
            mapEntry = mapIt.next();
            assertEquals(EmaRdm.ENAME_WARMSTANDBY_INFO, mapEntry.key().ascii().toString());
            assertEquals(DataType.DataTypes.ELEMENT_LIST, mapEntry.loadType());

            elementList = mapEntry.elementList();
            elementListIt = elementList.iterator();
            assertTrue(elementListIt.hasNext());
            elementEntry = elementListIt.next();
            assertEquals(EmaRdm.ENAME_WARMSTANDBY_MODE, elementEntry.name());
            assertEquals(Login.ServerTypes.STANDBY, elementEntry.uintValue());
            assertFalse(elementListIt.hasNext()); // End of ElementList

            assertFalse(mapIt.hasNext()); // End of Map

            /* Receives one generic messages of active server for the second connection */
            assertEquals(1, providerClient3.queueSize());

            /* GenericMsg for active server for the second connection */
            message = providerClient3.popMessage();
            genericMsg = (GenericMsg)message;
            assertEquals(1, genericMsg.streamId());
            assertEquals(DomainTypes.LOGIN, genericMsg.domainType());
            assertEquals(EmaRdm.ENAME_CONS_CONN_STATUS, genericMsg.name());
            assertEquals(DataType.DataTypes.MAP, genericMsg.payload().dataType());

            map = genericMsg.payload().map();
            mapIt = map.iterator();
            assertTrue(mapIt.hasNext());
            mapEntry = mapIt.next();
            assertEquals(EmaRdm.ENAME_WARMSTANDBY_INFO, mapEntry.key().ascii().toString());
            assertEquals(DataType.DataTypes.ELEMENT_LIST, mapEntry.loadType());

            elementList = mapEntry.elementList();
            elementListIt = elementList.iterator();
            assertTrue(elementListIt.hasNext());
            elementEntry = elementListIt.next();
            assertEquals(EmaRdm.ENAME_WARMSTANDBY_MODE, elementEntry.name());
            assertEquals(Login.ServerTypes.ACTIVE, elementEntry.uintValue());
            assertFalse(elementListIt.hasNext()); // End of ElementList

            assertFalse(mapIt.hasNext()); // End of Map

            /* Receives one generic messages of standby server for the second connection */
            assertEquals(1, providerClient4.queueSize());
            /* GenericMsg for standby server for the second connection */
            message = providerClient4.popMessage();
            genericMsg = (GenericMsg)message;
            assertEquals(1, genericMsg.streamId());
            assertEquals(DomainTypes.LOGIN, genericMsg.domainType());
            assertEquals(EmaRdm.ENAME_CONS_CONN_STATUS, genericMsg.name());
            assertEquals(DataType.DataTypes.MAP, genericMsg.payload().dataType());

            map = genericMsg.payload().map();
            mapIt = map.iterator();
            assertTrue(mapIt.hasNext());
            mapEntry = mapIt.next();
            assertEquals(EmaRdm.ENAME_WARMSTANDBY_INFO, mapEntry.key().ascii().toString());
            assertEquals(DataType.DataTypes.ELEMENT_LIST, mapEntry.loadType());

            elementList = mapEntry.elementList();
            elementListIt = elementList.iterator();
            assertTrue(elementListIt.hasNext());
            elementEntry = elementListIt.next();
            assertEquals(EmaRdm.ENAME_WARMSTANDBY_MODE, elementEntry.name());
            assertEquals(Login.ServerTypes.STANDBY, elementEntry.uintValue());
            assertFalse(elementListIt.hasNext()); // End of ElementList

            assertFalse(mapIt.hasNext()); // End of Map

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
            assertEquals(32767, refreshMsg.serviceId());
            assertEquals("DIRECT_FEED", refreshMsg.serviceName());
            assertEquals("IBM.N", refreshMsg.name());
            assertEquals(DataType.DataTypes.FIELD_LIST, refreshMsg.payload().dataType());

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
            assertEquals(DataType.DataTypes.MAP, genericMsg.payload().dataType());

            map = genericMsg.payload().map();
            mapIt = map.iterator();
            assertTrue(mapIt.hasNext());
            mapEntry = mapIt.next();
            assertEquals(EmaRdm.ENAME_WARMSTANDBY_INFO, mapEntry.key().ascii().toString());
            assertEquals(DataType.DataTypes.ELEMENT_LIST, mapEntry.loadType());

            elementList = mapEntry.elementList();
            elementListIt = elementList.iterator();
            assertTrue(elementListIt.hasNext());
            elementEntry = elementListIt.next();
            assertEquals(EmaRdm.ENAME_WARMSTANDBY_MODE, elementEntry.name());
            assertEquals(Login.ServerTypes.ACTIVE, elementEntry.uintValue());
            assertFalse(elementListIt.hasNext()); // End of ElementList

            assertFalse(mapIt.hasNext()); // End of Map

            assertEquals(1, consumerClient.queueSize());

            message = consumerClient.popMessage();

            StatusMsg statusMsg = (StatusMsg)message;
            assertEquals(5, statusMsg.streamId());
            assertEquals("DIRECT_FEED", statusMsg.serviceName());
            assertEquals("IBM.N", statusMsg.name());
            assertEquals(OmmState.StreamState.CLOSED_RECOVER, statusMsg.state().streamState());
            assertEquals(OmmState.DataState.SUSPECT, statusMsg.state().dataState());
            assertEquals(OmmState.StatusCode.NONE, statusMsg.state().statusCode());
            assertEquals("Service for this item was lost.", statusMsg.state().statusText());

            /* Closes the new active server for the first connection */
            ommprovider2.uninitialize();

            Thread.sleep(2000);

            /* Ensure that the active server of the second connection doesn't receive a market price request */
            assertEquals(0, providerClient3.queueSize());


            /* Ensure that the standby server of the second connection doesn't receive a market price request */
            assertEquals(0, providerClient4.queueSize());

            /* Ensure that the consumer doesn't receive any messages */
            assertEquals(0, consumerClient.queueSize());

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
            assertEquals(DataType.DataTypes.MAP, genericMsg.payload().dataType());

            Map map = genericMsg.payload().map();
            Iterator<MapEntry> mapIt = map.iterator();
            assertTrue(mapIt.hasNext());
            MapEntry mapEntry = mapIt.next();
            assertEquals(EmaRdm.ENAME_WARMSTANDBY_INFO, mapEntry.key().ascii().toString());
            assertEquals(DataType.DataTypes.ELEMENT_LIST, mapEntry.loadType());

            ElementList elementList = mapEntry.elementList();
            Iterator<ElementEntry> elementListIt = elementList.iterator();
            assertTrue(elementListIt.hasNext());
            ElementEntry elementEntry = elementListIt.next();
            assertEquals(EmaRdm.ENAME_WARMSTANDBY_MODE, elementEntry.name());
            assertEquals(Login.ServerTypes.ACTIVE, elementEntry.uintValue());
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

            /* Receives one generic message and one request message of standby server for the first connection */
            assertEquals(2, providerClient2.queueSize());
            /* GenericMsg for standby server for the first connection */
            message = providerClient2.popMessage();
            genericMsg = (GenericMsg)message;
            assertEquals(1, genericMsg.streamId());
            assertEquals(DomainTypes.LOGIN, genericMsg.domainType());
            assertEquals(EmaRdm.ENAME_CONS_CONN_STATUS, genericMsg.name());
            assertEquals(DataType.DataTypes.MAP, genericMsg.payload().dataType());

            map = genericMsg.payload().map();
            mapIt = map.iterator();
            assertTrue(mapIt.hasNext());
            mapEntry = mapIt.next();
            assertEquals(EmaRdm.ENAME_WARMSTANDBY_INFO, mapEntry.key().ascii().toString());
            assertEquals(DataType.DataTypes.ELEMENT_LIST, mapEntry.loadType());

            elementList = mapEntry.elementList();
            elementListIt = elementList.iterator();
            assertTrue(elementListIt.hasNext());
            elementEntry = elementListIt.next();
            assertEquals(EmaRdm.ENAME_WARMSTANDBY_MODE, elementEntry.name());
            assertEquals(Login.ServerTypes.STANDBY, elementEntry.uintValue());
            assertFalse(elementListIt.hasNext()); // End of ElementList

            assertFalse(mapIt.hasNext()); // End of Map

            /* Market price request message for standby server for the first connection */
            message = providerClient2.popMessage();
            recvReq = (ReqMsg)message;

            assertEquals(3, recvReq.streamId());
            assertEquals("DIRECT_FEED", recvReq.serviceName());
            assertEquals("IBM.N", recvReq.name());
            assertEquals(1, recvReq.serviceId());

            /* Receives one generic messages of active server for the second connection */
            assertEquals(1, providerClient3.queueSize());

            /* GenericMsg for active server for the second connection */
            message = providerClient3.popMessage();
            genericMsg = (GenericMsg)message;
            assertEquals(1, genericMsg.streamId());
            assertEquals(DomainTypes.LOGIN, genericMsg.domainType());
            assertEquals(EmaRdm.ENAME_CONS_CONN_STATUS, genericMsg.name());
            assertEquals(DataType.DataTypes.MAP, genericMsg.payload().dataType());

            map = genericMsg.payload().map();
            mapIt = map.iterator();
            assertTrue(mapIt.hasNext());
            mapEntry = mapIt.next();
            assertEquals(EmaRdm.ENAME_WARMSTANDBY_INFO, mapEntry.key().ascii().toString());
            assertEquals(DataType.DataTypes.ELEMENT_LIST, mapEntry.loadType());

            elementList = mapEntry.elementList();
            elementListIt = elementList.iterator();
            assertTrue(elementListIt.hasNext());
            elementEntry = elementListIt.next();
            assertEquals(EmaRdm.ENAME_WARMSTANDBY_MODE, elementEntry.name());
            assertEquals(Login.ServerTypes.ACTIVE, elementEntry.uintValue());
            assertFalse(elementListIt.hasNext()); // End of ElementList

            assertFalse(mapIt.hasNext()); // End of Map

            /* Receives one generic messages of standby server for the second connection */
            assertEquals(1, providerClient4.queueSize());
            /* GenericMsg for standby server for the second connection */
            message = providerClient4.popMessage();
            genericMsg = (GenericMsg)message;
            assertEquals(1, genericMsg.streamId());
            assertEquals(DomainTypes.LOGIN, genericMsg.domainType());
            assertEquals(EmaRdm.ENAME_CONS_CONN_STATUS, genericMsg.name());
            assertEquals(DataType.DataTypes.MAP, genericMsg.payload().dataType());

            map = genericMsg.payload().map();
            mapIt = map.iterator();
            assertTrue(mapIt.hasNext());
            mapEntry = mapIt.next();
            assertEquals(EmaRdm.ENAME_WARMSTANDBY_INFO, mapEntry.key().ascii().toString());
            assertEquals(DataType.DataTypes.ELEMENT_LIST, mapEntry.loadType());

            elementList = mapEntry.elementList();
            elementListIt = elementList.iterator();
            assertTrue(elementListIt.hasNext());
            elementEntry = elementListIt.next();
            assertEquals(EmaRdm.ENAME_WARMSTANDBY_MODE, elementEntry.name());
            assertEquals(Login.ServerTypes.STANDBY, elementEntry.uintValue());
            assertFalse(elementListIt.hasNext()); // End of ElementList

            assertFalse(mapIt.hasNext()); // End of Map

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
            assertEquals(32767, refreshMsg.serviceId());
            assertEquals("DIRECT_FEED", refreshMsg.serviceName());
            assertEquals("IBM.N", refreshMsg.name());
            assertEquals(DataType.DataTypes.FIELD_LIST, refreshMsg.payload().dataType());

            assertEquals(1, consumerClient.channelInfoSize());
            ChannelInformation channelInfo = consumerClient.popChannelInfo();
            assertEquals("Channel_3", channelInfo.channelName());
            assertEquals("Connection_7", channelInfo.sessionChannelName());

            assertEquals(1, consumerClient.sessionChannelInfoSize());

            Iterator<ChannelInformation> it = consumerClient.popSessionChannelInfo().iterator();
            assertTrue(it.hasNext());
            channelInfo = it.next();
            assertEquals("Channel_3", channelInfo.channelName());
            assertEquals("Connection_7", channelInfo.sessionChannelName());
            assertTrue(it.hasNext());
            channelInfo = it.next();
            assertTrue(channelInfo.channelName().equals("Channel_7") ||  channelInfo.channelName().equals("Channel_8"));
            assertEquals("Connection_8", channelInfo.sessionChannelName());
            assertFalse(it.hasNext());

            /* Closes the active server for the first connection */
            ommprovider1.uninitialize();

            Thread.sleep(4000);

            /* Receives one generic message to promote the standby to active server for the first connection.*/
            assertEquals(1, providerClient2.queueSize());
            /* GenericMsg for standby server for the first connection */
            message = providerClient2.popMessage();
            genericMsg = (GenericMsg)message;
            assertEquals(1, genericMsg.streamId());
            assertEquals(DomainTypes.LOGIN, genericMsg.domainType());
            assertEquals(EmaRdm.ENAME_CONS_CONN_STATUS, genericMsg.name());
            assertEquals(DataType.DataTypes.MAP, genericMsg.payload().dataType());

            map = genericMsg.payload().map();
            mapIt = map.iterator();
            assertTrue(mapIt.hasNext());
            mapEntry = mapIt.next();
            assertEquals(EmaRdm.ENAME_WARMSTANDBY_INFO, mapEntry.key().ascii().toString());
            assertEquals(DataType.DataTypes.ELEMENT_LIST, mapEntry.loadType());

            elementList = mapEntry.elementList();
            elementListIt = elementList.iterator();
            assertTrue(elementListIt.hasNext());
            elementEntry = elementListIt.next();
            assertEquals(EmaRdm.ENAME_WARMSTANDBY_MODE, elementEntry.name());
            assertEquals(Login.ServerTypes.ACTIVE, elementEntry.uintValue());
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
            assertEquals("Connection_7", channelInfo.sessionChannelName());

            assertEquals(2, consumerClient.sessionChannelInfoSize());

            it = consumerClient.popSessionChannelInfo().iterator();
            assertTrue(it.hasNext());
            channelInfo = it.next();
            assertEquals("Channel_3", channelInfo.channelName());
            assertEquals("Connection_7", channelInfo.sessionChannelName());
            assertTrue(it.hasNext());
            channelInfo = it.next();
            assertEquals("Channel_8", channelInfo.channelName());
            assertEquals("Connection_8", channelInfo.sessionChannelName());
            assertFalse(it.hasNext());

            message = consumerClient.popMessage();
            refreshMsg = (RefreshMsg)message;

            assertEquals(5, refreshMsg.streamId());
            assertEquals(DomainTypes.MARKET_PRICE, refreshMsg.domainType());
            assertEquals("Open / Ok / None / 'Unsolicited Refresh Completed'", refreshMsg.state().toString());
            assertFalse(refreshMsg.solicited());
            assertTrue(refreshMsg.complete());
            assertTrue(refreshMsg.hasMsgKey());
            assertEquals(32767, refreshMsg.serviceId());
            assertEquals("DIRECT_FEED", refreshMsg.serviceName());
            assertEquals("IBM.N", refreshMsg.name());
            assertEquals(DataType.DataTypes.FIELD_LIST, refreshMsg.payload().dataType());

            channelInfo = consumerClient.popChannelInfo();
            assertEquals("Channel_6", channelInfo.channelName());
            assertEquals("Connection_7", channelInfo.sessionChannelName());

            it = consumerClient.popSessionChannelInfo().iterator();
            assertTrue(it.hasNext());
            channelInfo = it.next();
            assertEquals("Channel_6", channelInfo.channelName());
            assertEquals("Connection_7", channelInfo.sessionChannelName());
            assertTrue(it.hasNext());
            channelInfo = it.next();
            assertEquals("Channel_8", channelInfo.channelName());
            assertEquals("Connection_8", channelInfo.sessionChannelName());
            assertFalse(it.hasNext());

            /* Closes the new active server for the first connection */
            ommprovider2.uninitialize();

            Thread.sleep(3000);

            assertEquals(1, providerClient3.queueSize());

            /* Receives market price request message for active server for the second connection */
            message = providerClient3.popMessage();
            recvReq = (ReqMsg)message;

            assertEquals(3, recvReq.streamId());
            assertEquals("DIRECT_FEED", recvReq.serviceName());
            assertEquals("IBM.N", recvReq.name());
            assertEquals(1, recvReq.serviceId());

            assertEquals(1, providerClient4.queueSize());

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
            assertEquals("Connection_7", channelInfo.sessionChannelName());

            assertEquals(2, consumerClient.sessionChannelInfoSize());
            it = consumerClient.popSessionChannelInfo().iterator();
            assertTrue(it.hasNext());
            channelInfo = it.next();
            assertEquals("Channel_6", channelInfo.channelName());
            assertEquals("Connection_7", channelInfo.sessionChannelName());
            assertTrue(it.hasNext());
            channelInfo = it.next();
            assertEquals("Channel_8", channelInfo.channelName());
            assertEquals("Connection_8", channelInfo.sessionChannelName());
            assertFalse(it.hasNext());

            message = consumerClient.popMessage();

            /* Receives Refresh message from the active server of the second connection */
            refreshMsg = (RefreshMsg)message;
            assertEquals(5, refreshMsg.streamId());
            assertEquals(DomainTypes.MARKET_PRICE, refreshMsg.domainType());
            assertEquals("Open / Ok / None / 'Refresh Completed'", refreshMsg.state().toString());
            assertTrue(refreshMsg.solicited());
            assertTrue(refreshMsg.complete());
            assertTrue(refreshMsg.hasMsgKey());
            assertEquals(32767, refreshMsg.serviceId());
            assertEquals("DIRECT_FEED", refreshMsg.serviceName());
            assertEquals("IBM.N", refreshMsg.name());
            assertEquals(DataType.DataTypes.FIELD_LIST, refreshMsg.payload().dataType());

            channelInfo = consumerClient.popChannelInfo();
            assertEquals("Channel_7", channelInfo.channelName());
            assertEquals("Connection_8", channelInfo.sessionChannelName());

            it = consumerClient.popSessionChannelInfo().iterator();
            assertTrue(it.hasNext());
            channelInfo = it.next();
            // There are attempts to reconnect for both Channel_3 and Channel_6, since they are unsuccessful, Channel Down events are sent
            // and it depends on timing which channel is going to get Channel Down event last.
            assertTrue("Channel_6".equals(channelInfo.channelName())
                    || "Channel_3".equals(channelInfo.channelName()));
            assertEquals("Connection_7", channelInfo.sessionChannelName());
            assertTrue(it.hasNext());
            channelInfo = it.next();
            assertEquals("Channel_7", channelInfo.channelName());
            assertEquals("Connection_8", channelInfo.sessionChannelName());
            assertFalse(it.hasNext());

            assertEquals(1, providerClient.queueSize());
            message = providerClient.popMessage();

            assertEquals(1, providerClient2.queueSize());
            message = providerClient2.popMessage();


            ProviderTestClient providerClient5 = new ProviderTestClient(providerTestOptions);
            ProviderTestClient providerClient6 = new ProviderTestClient(providerTestOptions);

            /* Bring up the Connection_7 again */
            ommprovider1 = EmaFactory.createOmmProvider(config.port("19003").providerName("Provider_5"), providerClient5);
            assertNotNull(ommprovider1);

            ommprovider2 = EmaFactory.createOmmProvider(config.port("19006").providerName("Provider_5"), providerClient6);
            assertNotNull(ommprovider2);

            Thread.sleep(1000);

            consumer.unregister(itemHandle);
        }
        catch(OmmException ex)
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
    public void testMultiConnectionsForSingleItemWithWSBChannelsAndChannelDownForServiceBased()
    {
        TestUtilities.printTestHead("testMultiConnectionsForSingleItemWithWSBChannelsAndChannelDownForServiceBased","");

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
            consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_14"));

            ReqMsg reqMsg = EmaFactory.createReqMsg();

            long itemHandle = consumer.registerClient(reqMsg.name("IBM.N").serviceName("DIRECT_FEED"), consumerClient);

            Thread.sleep(4000);

            /* Receives one generic message and one request message of active server for the first connection */
            assertEquals(2, providerClient.queueSize());

            Msg message = providerClient.popMessage();

            /* GenericMsg for active server for the first connection */
            GenericMsg genericMsg = (GenericMsg)message;
            assertEquals(2, genericMsg.streamId());
            assertEquals(DomainTypes.SOURCE, genericMsg.domainType());
            assertEquals(EmaRdm.ENAME_CONS_STATUS, genericMsg.name());
            assertEquals(DataType.DataTypes.MAP, genericMsg.payload().dataType());

            Map map = genericMsg.payload().map();
            Iterator<MapEntry> mapIt = map.iterator();
            assertTrue(mapIt.hasNext());
            MapEntry mapEntry = mapIt.next();

            assertEquals(1, mapEntry.key().uintValue());
            assertEquals(DataType.DataTypes.ELEMENT_LIST, mapEntry.loadType());

            ElementList elementList = mapEntry.elementList();
            Iterator<ElementEntry> elementListIt = elementList.iterator();
            assertTrue(elementListIt.hasNext());
            ElementEntry elementEntry = elementListIt.next();
            assertEquals(EmaRdm.ENAME_WARMSTANDBY_MODE, elementEntry.name());
            assertEquals(Login.ServerTypes.ACTIVE, elementEntry.uintValue());
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

            /* Receives one generic message and one request message of standby server for the first connection */
            assertEquals(2, providerClient2.queueSize());
            /* GenericMsg for standby server for the first connection */
            message = providerClient2.popMessage();
            genericMsg = (GenericMsg)message;
            assertEquals(2, genericMsg.streamId());
            assertEquals(DomainTypes.SOURCE, genericMsg.domainType());
            assertEquals(EmaRdm.ENAME_CONS_STATUS, genericMsg.name());
            assertEquals(DataType.DataTypes.MAP, genericMsg.payload().dataType());

            map = genericMsg.payload().map();
            mapIt = map.iterator();
            assertTrue(mapIt.hasNext());
            mapEntry = mapIt.next();
            assertEquals(1, mapEntry.key().uintValue());
            assertEquals(DataType.DataTypes.ELEMENT_LIST, mapEntry.loadType());

            elementList = mapEntry.elementList();
            elementListIt = elementList.iterator();
            assertTrue(elementListIt.hasNext());
            elementEntry = elementListIt.next();
            assertEquals(EmaRdm.ENAME_WARMSTANDBY_MODE, elementEntry.name());
            assertEquals(Login.ServerTypes.STANDBY, elementEntry.uintValue());
            assertFalse(elementListIt.hasNext()); // End of ElementList

            assertFalse(mapIt.hasNext()); // End of Map

            /* Market price request message for standby server for the first connection */
            message = providerClient2.popMessage();
            recvReq = (ReqMsg)message;

            assertEquals(3, recvReq.streamId());
            assertEquals("DIRECT_FEED", recvReq.serviceName());
            assertEquals("IBM.N", recvReq.name());
            assertEquals(1, recvReq.serviceId());

            /* Receives one generic messages of active server for the second connection */
            assertEquals(1, providerClient3.queueSize());

            /* GenericMsg for active server for the second connection */
            message = providerClient3.popMessage();
            genericMsg = (GenericMsg)message;
            assertEquals(2, genericMsg.streamId());
            assertEquals(DomainTypes.SOURCE, genericMsg.domainType());
            assertEquals(EmaRdm.ENAME_CONS_STATUS, genericMsg.name());
            assertEquals(DataType.DataTypes.MAP, genericMsg.payload().dataType());

            map = genericMsg.payload().map();
            mapIt = map.iterator();
            assertTrue(mapIt.hasNext());
            mapEntry = mapIt.next();
            assertEquals(1, mapEntry.key().uintValue());
            assertEquals(DataType.DataTypes.ELEMENT_LIST, mapEntry.loadType());

            elementList = mapEntry.elementList();
            elementListIt = elementList.iterator();
            assertTrue(elementListIt.hasNext());
            elementEntry = elementListIt.next();
            assertEquals(EmaRdm.ENAME_WARMSTANDBY_MODE, elementEntry.name());
            assertEquals(Login.ServerTypes.ACTIVE, elementEntry.uintValue());
            assertFalse(elementListIt.hasNext()); // End of ElementList

            assertFalse(mapIt.hasNext()); // End of Map

            /* Receives one generic messages of standby server for the second connection */
            assertEquals(1, providerClient4.queueSize());
            /* GenericMsg for standby server for the second connection */
            message = providerClient4.popMessage();
            genericMsg = (GenericMsg)message;
            assertEquals(2, genericMsg.streamId());
            assertEquals(DomainTypes.SOURCE, genericMsg.domainType());
            assertEquals(EmaRdm.ENAME_CONS_STATUS, genericMsg.name());
            assertEquals(DataType.DataTypes.MAP, genericMsg.payload().dataType());

            map = genericMsg.payload().map();
            mapIt = map.iterator();
            assertTrue(mapIt.hasNext());
            mapEntry = mapIt.next();
            assertEquals(1, mapEntry.key().uintValue());
            assertEquals(DataType.DataTypes.ELEMENT_LIST, mapEntry.loadType());

            elementList = mapEntry.elementList();
            elementListIt = elementList.iterator();
            assertTrue(elementListIt.hasNext());
            elementEntry = elementListIt.next();
            assertEquals(EmaRdm.ENAME_WARMSTANDBY_MODE, elementEntry.name());
            assertEquals(Login.ServerTypes.STANDBY, elementEntry.uintValue());
            assertFalse(elementListIt.hasNext()); // End of ElementList

            assertFalse(mapIt.hasNext()); // End of Map

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
            assertEquals(32767, refreshMsg.serviceId());
            assertEquals("DIRECT_FEED", refreshMsg.serviceName());
            assertEquals("IBM.N", refreshMsg.name());
            assertEquals(DataType.DataTypes.FIELD_LIST, refreshMsg.payload().dataType());

            assertEquals(1, consumerClient.channelInfoSize());
            ChannelInformation channelInfo = consumerClient.popChannelInfo();
            assertEquals("Channel_3", channelInfo.channelName());
            assertEquals("Connection_5", channelInfo.sessionChannelName());

            assertEquals(1, consumerClient.sessionChannelInfoSize());

            Iterator<ChannelInformation> it = consumerClient.popSessionChannelInfo().iterator();
            assertTrue(it.hasNext());
            channelInfo = it.next();
            assertEquals("Channel_3", channelInfo.channelName());
            assertEquals("Connection_5", channelInfo.sessionChannelName());
            assertTrue(it.hasNext());
            channelInfo = it.next();
            assertEquals("Connection_6", channelInfo.sessionChannelName());
            assertFalse(it.hasNext());

            /* Closes the active server for the first connection */
            ommprovider1.uninitialize();

            Thread.sleep(2000);

            /* Receives one generic message to promote the standby to active server for the first connection.*/
            assertEquals(1, providerClient2.queueSize());
            /* GenericMsg for standby server for the first connection */
            message = providerClient2.popMessage();
            genericMsg = (GenericMsg)message;
            assertEquals(2, genericMsg.streamId());
            assertEquals(DomainTypes.SOURCE, genericMsg.domainType());
            assertEquals(EmaRdm.ENAME_CONS_STATUS, genericMsg.name());
            assertEquals(DataType.DataTypes.MAP, genericMsg.payload().dataType());

            map = genericMsg.payload().map();
            mapIt = map.iterator();
            assertTrue(mapIt.hasNext());
            mapEntry = mapIt.next();
            assertEquals(1, mapEntry.key().uintValue());
            assertEquals(DataType.DataTypes.ELEMENT_LIST, mapEntry.loadType());

            elementList = mapEntry.elementList();
            elementListIt = elementList.iterator();
            assertTrue(elementListIt.hasNext());
            elementEntry = elementListIt.next();
            assertEquals(EmaRdm.ENAME_WARMSTANDBY_MODE, elementEntry.name());
            assertEquals(Login.ServerTypes.ACTIVE, elementEntry.uintValue());
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
            assertEquals("Connection_5", channelInfo.sessionChannelName());

            assertEquals(2, consumerClient.sessionChannelInfoSize());

            it = consumerClient.popSessionChannelInfo().iterator();
            assertTrue(it.hasNext());
            channelInfo = it.next();
            assertEquals("Channel_3", channelInfo.channelName());
            assertEquals("Connection_5", channelInfo.sessionChannelName());
            assertTrue(it.hasNext());
            channelInfo = it.next();
            assertEquals("Channel_8", channelInfo.channelName());
            assertEquals("Connection_6", channelInfo.sessionChannelName());
            assertFalse(it.hasNext());

            message = consumerClient.popMessage();
            refreshMsg = (RefreshMsg)message;

            assertEquals(5, refreshMsg.streamId());
            assertEquals(DomainTypes.MARKET_PRICE, refreshMsg.domainType());
            assertEquals("Open / Ok / None / 'Unsolicited Refresh Completed'", refreshMsg.state().toString());
            assertFalse(refreshMsg.solicited());
            assertTrue(refreshMsg.complete());
            assertTrue(refreshMsg.hasMsgKey());
            assertEquals(32767, refreshMsg.serviceId());
            assertEquals("DIRECT_FEED", refreshMsg.serviceName());
            assertEquals("IBM.N", refreshMsg.name());
            assertEquals(DataType.DataTypes.FIELD_LIST, refreshMsg.payload().dataType());

            channelInfo = consumerClient.popChannelInfo();
            assertEquals("Channel_6", channelInfo.channelName());
            assertEquals("Connection_5", channelInfo.sessionChannelName());

            it = consumerClient.popSessionChannelInfo().iterator();
            assertTrue(it.hasNext());
            channelInfo = it.next();
            assertEquals("Channel_6", channelInfo.channelName());
            assertEquals("Connection_5", channelInfo.sessionChannelName());
            assertTrue(it.hasNext());
            channelInfo = it.next();
            assertEquals("Channel_8", channelInfo.channelName());
            assertEquals("Connection_6", channelInfo.sessionChannelName());
            assertFalse(it.hasNext());

            /* Closes the new active server for the first connection */
            ommprovider2.uninitialize();

            Thread.sleep(2000);

            assertEquals(1, providerClient3.queueSize());

            /* Receives market price request message for active server for the second connection */
            message = providerClient3.popMessage();
            recvReq = (ReqMsg)message;

            assertEquals(3, recvReq.streamId());
            assertEquals("DIRECT_FEED", recvReq.serviceName());
            assertEquals("IBM.N", recvReq.name());
            assertEquals(1, recvReq.serviceId());

            assertEquals(1, providerClient4.queueSize());

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
            assertEquals("Connection_5", channelInfo.sessionChannelName());

            assertEquals(2, consumerClient.sessionChannelInfoSize());
            it = consumerClient.popSessionChannelInfo().iterator();
            assertTrue(it.hasNext());
            channelInfo = it.next();
            assertEquals("Channel_6", channelInfo.channelName());
            assertEquals("Connection_5", channelInfo.sessionChannelName());
            assertTrue(it.hasNext());
            channelInfo = it.next();
            assertEquals("Channel_8", channelInfo.channelName());
            assertEquals("Connection_6", channelInfo.sessionChannelName());
            assertFalse(it.hasNext());

            message = consumerClient.popMessage();

            /* Receives Refresh message from the active server of the second connection */
            refreshMsg = (RefreshMsg)message;
            assertEquals(5, refreshMsg.streamId());
            assertEquals(DomainTypes.MARKET_PRICE, refreshMsg.domainType());
            assertEquals("Open / Ok / None / 'Refresh Completed'", refreshMsg.state().toString());
            assertTrue(refreshMsg.solicited());
            assertTrue(refreshMsg.complete());
            assertTrue(refreshMsg.hasMsgKey());
            assertEquals(32767, refreshMsg.serviceId());
            assertEquals("DIRECT_FEED", refreshMsg.serviceName());
            assertEquals("IBM.N", refreshMsg.name());
            assertEquals(DataType.DataTypes.FIELD_LIST, refreshMsg.payload().dataType());

            channelInfo = consumerClient.popChannelInfo();
            assertEquals("Channel_7", channelInfo.channelName());
            assertEquals("Connection_6", channelInfo.sessionChannelName());

            it = consumerClient.popSessionChannelInfo().iterator();
            assertTrue(it.hasNext());
            channelInfo = it.next();
            assertEquals("Connection_5", channelInfo.sessionChannelName());
            channelInfo = it.next();
            assertEquals("Channel_7", channelInfo.channelName());
            assertEquals("Connection_6", channelInfo.sessionChannelName());
            assertFalse(it.hasNext());

            consumer.unregister(itemHandle);
        }
        catch(OmmException ex)
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
    public void testMultiConnectionsForSingleItemWithWSBChannelsAndServiceDownForServiceBased()
    {
        TestUtilities.printTestHead("testMultiConnectionsForSingleItemWithWSBChannelsAndServiceDownForServiceBased","");

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
            consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_14"));

            ReqMsg reqMsg = EmaFactory.createReqMsg();

            long itemHandle = consumer.registerClient(reqMsg.name("IBM.N").serviceName("DIRECT_FEED"), consumerClient);

            Thread.sleep(4000);

            /* Receives one generic message and one request message of active server for the first connection */
            assertEquals(2, providerClient.queueSize());

            Msg message = providerClient.popMessage();

            /* GenericMsg for active server for the first connection */
            GenericMsg genericMsg = (GenericMsg)message;
            assertEquals(2, genericMsg.streamId());
            assertEquals(DomainTypes.SOURCE, genericMsg.domainType());
            assertEquals(EmaRdm.ENAME_CONS_STATUS, genericMsg.name());
            assertEquals(DataType.DataTypes.MAP, genericMsg.payload().dataType());

            Map map = genericMsg.payload().map();
            Iterator<MapEntry> mapIt = map.iterator();
            assertTrue(mapIt.hasNext());
            MapEntry mapEntry = mapIt.next();

            assertEquals(1, mapEntry.key().uintValue());
            assertEquals(DataType.DataTypes.ELEMENT_LIST, mapEntry.loadType());

            ElementList elementList = mapEntry.elementList();
            Iterator<ElementEntry> elementListIt = elementList.iterator();
            assertTrue(elementListIt.hasNext());
            ElementEntry elementEntry = elementListIt.next();
            assertEquals(EmaRdm.ENAME_WARMSTANDBY_MODE, elementEntry.name());
            assertEquals(Login.ServerTypes.ACTIVE, elementEntry.uintValue());
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

            /* Receives one generic message and one request message of standby server for the first connection */
            assertEquals(2, providerClient2.queueSize());
            /* GenericMsg for standby server for the first connection */
            message = providerClient2.popMessage();
            genericMsg = (GenericMsg)message;
            assertEquals(2, genericMsg.streamId());
            assertEquals(DomainTypes.SOURCE, genericMsg.domainType());
            assertEquals(EmaRdm.ENAME_CONS_STATUS, genericMsg.name());
            assertEquals(DataType.DataTypes.MAP, genericMsg.payload().dataType());

            map = genericMsg.payload().map();
            mapIt = map.iterator();
            assertTrue(mapIt.hasNext());
            mapEntry = mapIt.next();
            assertEquals(1, mapEntry.key().uintValue());
            assertEquals(DataType.DataTypes.ELEMENT_LIST, mapEntry.loadType());

            elementList = mapEntry.elementList();
            elementListIt = elementList.iterator();
            assertTrue(elementListIt.hasNext());
            elementEntry = elementListIt.next();
            assertEquals(EmaRdm.ENAME_WARMSTANDBY_MODE, elementEntry.name());
            assertEquals(Login.ServerTypes.STANDBY, elementEntry.uintValue());
            assertFalse(elementListIt.hasNext()); // End of ElementList

            assertFalse(mapIt.hasNext()); // End of Map

            /* Market price request message for standby server for the first connection */
            message = providerClient2.popMessage();
            recvReq = (ReqMsg)message;

            assertEquals(3, recvReq.streamId());
            assertEquals("DIRECT_FEED", recvReq.serviceName());
            assertEquals("IBM.N", recvReq.name());
            assertEquals(1, recvReq.serviceId());

            /* Receives one generic messages of active server for the second connection */
            assertEquals(1, providerClient3.queueSize());

            /* GenericMsg for active server for the second connection */
            message = providerClient3.popMessage();
            genericMsg = (GenericMsg)message;
            assertEquals(2, genericMsg.streamId());
            assertEquals(DomainTypes.SOURCE, genericMsg.domainType());
            assertEquals(EmaRdm.ENAME_CONS_STATUS, genericMsg.name());
            assertEquals(DataType.DataTypes.MAP, genericMsg.payload().dataType());

            map = genericMsg.payload().map();
            mapIt = map.iterator();
            assertTrue(mapIt.hasNext());
            mapEntry = mapIt.next();
            assertEquals(1, mapEntry.key().uintValue());
            assertEquals(DataType.DataTypes.ELEMENT_LIST, mapEntry.loadType());

            elementList = mapEntry.elementList();
            elementListIt = elementList.iterator();
            assertTrue(elementListIt.hasNext());
            elementEntry = elementListIt.next();
            assertEquals(EmaRdm.ENAME_WARMSTANDBY_MODE, elementEntry.name());
            assertEquals(Login.ServerTypes.ACTIVE, elementEntry.uintValue());
            assertFalse(elementListIt.hasNext()); // End of ElementList

            assertFalse(mapIt.hasNext()); // End of Map

            /* Receives one generic messages of standby server for the second connection */
            assertEquals(1, providerClient4.queueSize());
            /* GenericMsg for standby server for the second connection */
            message = providerClient4.popMessage();
            genericMsg = (GenericMsg)message;
            assertEquals(2, genericMsg.streamId());
            assertEquals(DomainTypes.SOURCE, genericMsg.domainType());
            assertEquals(EmaRdm.ENAME_CONS_STATUS, genericMsg.name());
            assertEquals(DataType.DataTypes.MAP, genericMsg.payload().dataType());

            map = genericMsg.payload().map();
            mapIt = map.iterator();
            assertTrue(mapIt.hasNext());
            mapEntry = mapIt.next();
            assertEquals(1, mapEntry.key().uintValue());
            assertEquals(DataType.DataTypes.ELEMENT_LIST, mapEntry.loadType());

            elementList = mapEntry.elementList();
            elementListIt = elementList.iterator();
            assertTrue(elementListIt.hasNext());
            elementEntry = elementListIt.next();
            assertEquals(EmaRdm.ENAME_WARMSTANDBY_MODE, elementEntry.name());
            assertEquals(Login.ServerTypes.STANDBY, elementEntry.uintValue());
            assertFalse(elementListIt.hasNext()); // End of ElementList

            assertFalse(mapIt.hasNext()); // End of Map

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
            assertEquals(32767, refreshMsg.serviceId());
            assertEquals("DIRECT_FEED", refreshMsg.serviceName());
            assertEquals("IBM.N", refreshMsg.name());
            assertEquals(DataType.DataTypes.FIELD_LIST, refreshMsg.payload().dataType());

            assertEquals(1, consumerClient.channelInfoSize());
            ChannelInformation channelInfo = consumerClient.popChannelInfo();
            assertEquals("Channel_3", channelInfo.channelName());
            assertEquals("Connection_5", channelInfo.sessionChannelName());

            assertEquals(1, consumerClient.sessionChannelInfoSize());

            Iterator<ChannelInformation> it = consumerClient.popSessionChannelInfo().iterator();
            assertTrue(it.hasNext());
            channelInfo = it.next();
            assertEquals("Channel_3", channelInfo.channelName());
            assertEquals("Connection_5", channelInfo.sessionChannelName());
            assertTrue(it.hasNext());
            channelInfo = it.next();
            //assertEquals("Channel_7", channelInfo.channelName());
            assertEquals("Connection_6", channelInfo.sessionChannelName());
            assertFalse(it.hasNext());

            /* Send item group recoverable status from the first provider */
            ElementList serviceGroupId = EmaFactory.createElementList();
            serviceGroupId.add( EmaFactory.createElementEntry().buffer( EmaRdm.ENAME_GROUP, ByteBuffer.wrap("10".getBytes()) ));
            serviceGroupId.add( EmaFactory.createElementEntry().state( EmaRdm.ENAME_STATUS,
                    OmmState.StreamState.CLOSED_RECOVER, OmmState.DataState.SUSPECT, OmmState.StatusCode.NONE, "A23: Service has gone down. Will recall when service becomes available." ) );

            FilterList filterList = EmaFactory.createFilterList();
            filterList.add( EmaFactory.createFilterEntry().elementList( EmaRdm.SERVICE_GROUP_ID, FilterEntry.FilterAction.SET, serviceGroupId ) );

            Map mapEnc = EmaFactory.createMap();
            mapEnc.add( EmaFactory.createMapEntry().keyUInt(1, MapEntry.MapAction.UPDATE, filterList));

            ommprovider1.submit( EmaFactory.createUpdateMsg().domainType( EmaRdm.MMT_DIRECTORY ).
                    filter( EmaRdm.SERVICE_GROUP_FILTER ).
                    payload( mapEnc ), 0);	// use 0 item handle to fanout to all subscribers

            // Send service down for the starting server of the first connection.
            ElementList serviceState = EmaFactory.createElementList();
            serviceState.add( EmaFactory.createElementEntry().uintValue( EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_DOWN ));
            serviceState.add( EmaFactory.createElementEntry().uintValue( EmaRdm.ENAME_ACCEPTING_REQS, 0 ));
            serviceState.add( EmaFactory.createElementEntry().state( EmaRdm.ENAME_STATUS, OmmState.StreamState.CLOSED_RECOVER, OmmState.DataState.SUSPECT, OmmState.StatusCode.NONE, ""));

            FilterList filterListEnc = EmaFactory.createFilterList();
            filterListEnc.add( EmaFactory.createFilterEntry().elementList( EmaRdm.SERVICE_STATE_ID, FilterEntry.FilterAction.SET, serviceState ) );

            Map mapEnc2 = EmaFactory.createMap();
            mapEnc2.add( EmaFactory.createMapEntry().keyUInt( 1, MapEntry.MapAction.UPDATE, filterListEnc ));

            UpdateMsg updateMsg = EmaFactory.createUpdateMsg();
            ommprovider1.submit( updateMsg.domainType( EmaRdm.MMT_DIRECTORY ).
                    filter( EmaRdm.SERVICE_STATE_FILTER ).
                    payload( mapEnc2 ), 0);	// use 0 item handle to fan-out to all subscribers


            Thread.sleep(2000);

            /* Receives one generic message to promote the standby to active server for the first connection.*/
            assertEquals(1, providerClient2.queueSize());
            /* GenericMsg for standby server for the first connection */
            message = providerClient2.popMessage();
            genericMsg = (GenericMsg)message;
            assertEquals(2, genericMsg.streamId());
            assertEquals(DomainTypes.SOURCE, genericMsg.domainType());
            assertEquals(EmaRdm.ENAME_CONS_STATUS, genericMsg.name());
            assertEquals(DataType.DataTypes.MAP, genericMsg.payload().dataType());

            map = genericMsg.payload().map();
            mapIt = map.iterator();
            assertTrue(mapIt.hasNext());
            mapEntry = mapIt.next();
            assertEquals(1, mapEntry.key().uintValue());
            assertEquals(DataType.DataTypes.ELEMENT_LIST, mapEntry.loadType());

            elementList = mapEntry.elementList();
            elementListIt = elementList.iterator();
            assertTrue(elementListIt.hasNext());
            elementEntry = elementListIt.next();
            assertEquals(EmaRdm.ENAME_WARMSTANDBY_MODE, elementEntry.name());
            assertEquals(Login.ServerTypes.ACTIVE, elementEntry.uintValue());
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
            assertEquals("A23: Service has gone down. Will recall when service becomes available.", statusMsg.state().statusText());

            assertEquals(2, consumerClient.channelInfoSize());
            channelInfo = consumerClient.popChannelInfo();
            assertEquals("Channel_3", channelInfo.channelName());
            assertEquals("Connection_5", channelInfo.sessionChannelName());

            assertEquals(2, consumerClient.sessionChannelInfoSize());

            it = consumerClient.popSessionChannelInfo().iterator();
            assertTrue(it.hasNext());
            channelInfo = it.next();
            assertEquals("Channel_3", channelInfo.channelName());
            assertEquals("Connection_5", channelInfo.sessionChannelName());
            assertTrue(it.hasNext());
            channelInfo = it.next();
            assertEquals("Channel_8", channelInfo.channelName());
            assertEquals("Connection_6", channelInfo.sessionChannelName());
            assertFalse(it.hasNext());

            message = consumerClient.popMessage();
            refreshMsg = (RefreshMsg)message;

            assertEquals(5, refreshMsg.streamId());
            assertEquals(DomainTypes.MARKET_PRICE, refreshMsg.domainType());
            assertEquals("Open / Ok / None / 'Unsolicited Refresh Completed'", refreshMsg.state().toString());
            assertFalse(refreshMsg.solicited());
            assertTrue(refreshMsg.complete());
            assertTrue(refreshMsg.hasMsgKey());
            assertEquals(32767, refreshMsg.serviceId());
            assertEquals("DIRECT_FEED", refreshMsg.serviceName());
            assertEquals("IBM.N", refreshMsg.name());
            assertEquals(DataType.DataTypes.FIELD_LIST, refreshMsg.payload().dataType());

            channelInfo = consumerClient.popChannelInfo();
            assertEquals("Channel_6", channelInfo.channelName());
            assertEquals("Connection_5", channelInfo.sessionChannelName());

            it = consumerClient.popSessionChannelInfo().iterator();
            assertTrue(it.hasNext());
            channelInfo = it.next();
            assertEquals("Channel_6", channelInfo.channelName());
            assertEquals("Connection_5", channelInfo.sessionChannelName());
            assertTrue(it.hasNext());
            channelInfo = it.next();
            assertEquals("Channel_8", channelInfo.channelName());
            assertEquals("Connection_6", channelInfo.sessionChannelName());
            assertFalse(it.hasNext());

            // Send service down for the new starting server of the first connection.
            serviceGroupId.clear();
            serviceGroupId.add( EmaFactory.createElementEntry().buffer( EmaRdm.ENAME_GROUP, providerTestOptions.itemGroupId ));
            serviceGroupId.add( EmaFactory.createElementEntry().state( EmaRdm.ENAME_STATUS,
                    OmmState.StreamState.CLOSED_RECOVER, OmmState.DataState.SUSPECT, OmmState.StatusCode.NONE, "A23: Service has gone down. Will recall when service becomes available." ) );

            filterList.clear();
            filterList.add( EmaFactory.createFilterEntry().elementList( EmaRdm.SERVICE_GROUP_ID, FilterEntry.FilterAction.SET, serviceGroupId ) );

            mapEnc.clear();
            mapEnc.add( EmaFactory.createMapEntry().keyUInt(1, MapEntry.MapAction.UPDATE, filterList));

            ommprovider2.submit( EmaFactory.createUpdateMsg().domainType( EmaRdm.MMT_DIRECTORY ).
                    filter( EmaRdm.SERVICE_GROUP_FILTER ).
                    payload( mapEnc ), 0);	// use 0 item handle to fanout to all subscribers

            // Send service down for the starting server of the first connection.
            serviceState.clear();
            serviceState.add( EmaFactory.createElementEntry().uintValue( EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_DOWN ));
            serviceState.add( EmaFactory.createElementEntry().uintValue( EmaRdm.ENAME_ACCEPTING_REQS, 0 ));
            serviceState.add( EmaFactory.createElementEntry().state( EmaRdm.ENAME_STATUS, OmmState.StreamState.CLOSED_RECOVER, OmmState.DataState.SUSPECT, OmmState.StatusCode.NONE, ""));

            filterListEnc.clear();
            filterListEnc.add( EmaFactory.createFilterEntry().elementList( EmaRdm.SERVICE_STATE_ID, FilterEntry.FilterAction.SET, serviceState ) );

            mapEnc.clear();
            mapEnc.add( EmaFactory.createMapEntry().keyUInt( 1, MapEntry.MapAction.UPDATE, filterListEnc ));

            updateMsg.clear();
            ommprovider2.submit( updateMsg.domainType( EmaRdm.MMT_DIRECTORY ).
                    filter( EmaRdm.SERVICE_STATE_FILTER ).
                    payload( mapEnc ), 0);	// use 0 item handle to fan-out to all subscribers

            Thread.sleep(2000);

            assertEquals(1, providerClient3.queueSize());

            /* Receives market price request message for active server for the second connection */
            message = providerClient3.popMessage();
            recvReq = (ReqMsg)message;

            assertEquals(3, recvReq.streamId());
            assertEquals("DIRECT_FEED", recvReq.serviceName());
            assertEquals("IBM.N", recvReq.name());
            assertEquals(1, recvReq.serviceId());

            assertEquals(1, providerClient4.queueSize());

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
            assertEquals("A23: Service has gone down. Will recall when service becomes available.", statusMsg.state().statusText());

            assertEquals(2, consumerClient.channelInfoSize());
            channelInfo = consumerClient.popChannelInfo();
            assertEquals("Channel_6", channelInfo.channelName());
            assertEquals("Connection_5", channelInfo.sessionChannelName());

            assertEquals(2, consumerClient.sessionChannelInfoSize());
            it = consumerClient.popSessionChannelInfo().iterator();
            assertTrue(it.hasNext());
            channelInfo = it.next();
            assertEquals("Channel_6", channelInfo.channelName());
            assertEquals("Connection_5", channelInfo.sessionChannelName());
            assertTrue(it.hasNext());
            channelInfo = it.next();
            assertEquals("Channel_8", channelInfo.channelName());
            assertEquals("Connection_6", channelInfo.sessionChannelName());
            assertFalse(it.hasNext());

            message = consumerClient.popMessage();

            /* Receives Refresh message from the active server of the second connection */
            refreshMsg = (RefreshMsg)message;
            assertEquals(5, refreshMsg.streamId());
            assertEquals(DomainTypes.MARKET_PRICE, refreshMsg.domainType());
            assertEquals("Open / Ok / None / 'Refresh Completed'", refreshMsg.state().toString());
            assertTrue(refreshMsg.solicited());
            assertTrue(refreshMsg.complete());
            assertTrue(refreshMsg.hasMsgKey());
            assertEquals(32767, refreshMsg.serviceId());
            assertEquals("DIRECT_FEED", refreshMsg.serviceName());
            assertEquals("IBM.N", refreshMsg.name());
            assertEquals(DataType.DataTypes.FIELD_LIST, refreshMsg.payload().dataType());

            channelInfo = consumerClient.popChannelInfo();
            assertEquals("Channel_7", channelInfo.channelName());
            assertEquals("Connection_6", channelInfo.sessionChannelName());

            it = consumerClient.popSessionChannelInfo().iterator();
            assertTrue(it.hasNext());
            channelInfo = it.next();
            assertEquals("Channel_6", channelInfo.channelName());
            assertEquals("Connection_5", channelInfo.sessionChannelName());
            channelInfo = it.next();
            assertEquals("Channel_7", channelInfo.channelName());
            assertEquals("Connection_6", channelInfo.sessionChannelName());
            assertFalse(it.hasNext());

            consumer.unregister(itemHandle);
        }
        catch(OmmException ex)
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
    public void testMultiConnectionsOffStreamPostingWithServiceNameAndServiceDown()
    {
        TestUtilities.printTestHead("testMultiConnectionsOffStreamPostingWithServiceNameAndServiceDown","");

        String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";

        OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);

        ProviderTestOptions providerTestOptions = new ProviderTestOptions();
        providerTestOptions.sendRefreshAttrib = true;
        providerTestOptions.supportOMMPosting = true;

        ProviderTestClient providerClient1 = new ProviderTestClient(providerTestOptions);

        // Provider_1 provides the DIRECT_FEED service name for the first provider of the first connection.
        OmmProvider ommprovider = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_1"), providerClient1);

        assertNotNull(ommprovider);

        ProviderTestOptions providerTestOptions2 = new ProviderTestOptions();
        providerTestOptions2.sendRefreshAttrib = true;
        providerTestOptions2.supportOMMPosting = true;

        ProviderTestClient providerClient2 = new ProviderTestClient(providerTestOptions2);

        // Provider_1 provides the DIRECT_FEED service name for the first provider of the second connection.
        OmmProvider ommprovider2 = EmaFactory.createOmmProvider(config.port("19004").providerName("Provider_1"), providerClient2);

        assertNotNull(ommprovider2);

        // Provider_1 provides the DIRECT_FEED service name for the second provider of the first connection.
        ProviderTestClient providerClient3 = new ProviderTestClient(providerTestOptions);
        OmmProvider ommprovider3 = EmaFactory.createOmmProvider(config.port("19002").providerName("Provider_1"), providerClient3);

        assertNotNull(ommprovider);

        OmmConsumer consumer = null;
        ConsumerTestOptions consumerTestOptions = new ConsumerTestOptions();
        consumerTestOptions.submitPostOnLoginRefresh = true;
        ConsumerTestClient consumerClient = new ConsumerTestClient(consumerTestOptions);

        try
        {
            consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_9"));

            ReqMsg reqMsg = EmaFactory.createReqMsg();

            consumerClient.consumer(consumer);


            long loginHandle = consumer.registerClient(reqMsg.domainType( EmaRdm.MMT_LOGIN ), consumerClient);

            Thread.sleep(1000);

            /* Ensure there is no more message from provider as the Ack flag is not set */
            assertEquals(1, consumerClient.queueSize());

            /* This is login refresh message message */
            Msg message = consumerClient.popMessage();

            Thread.sleep(1000);

            /* Checks to ensure that the first provider of the first connection receives the PostMsg */
            assertEquals(1, providerClient1.queueSize());

            message = providerClient1.popMessage();

            assertTrue(message instanceof PostMsg);

            PostMsg recvPostMsg = (PostMsg)message;

            assertEquals(1, recvPostMsg.streamId());
            assertEquals("DIRECT_FEED", recvPostMsg.serviceName());
            assertEquals("IBM.N", recvPostMsg.name());
            assertEquals(1, recvPostMsg.postId());
            assertEquals(true, recvPostMsg.complete());
            assertEquals(DataType.DataTypes.UPDATE_MSG, recvPostMsg.payload().dataType());

            UpdateMsg updateMsg = recvPostMsg.payload().updateMsg();
            assertEquals(DataType.DataTypes.FIELD_LIST, updateMsg.payload().dataType());

            /* Checks to ensure that the first provider of the second connection receives the PostMsg */
            assertEquals(1, providerClient2.queueSize());

            message = providerClient2.popMessage();

            assertTrue(message instanceof PostMsg);

            recvPostMsg = (PostMsg)message;

            assertEquals(1, recvPostMsg.streamId());
            assertEquals("DIRECT_FEED", recvPostMsg.serviceName());
            assertEquals("IBM.N", recvPostMsg.name());
            assertEquals(1, recvPostMsg.postId());
            assertEquals(true, recvPostMsg.complete());
            assertEquals(DataType.DataTypes.UPDATE_MSG, recvPostMsg.payload().dataType());

            updateMsg = recvPostMsg.payload().updateMsg();
            assertEquals(DataType.DataTypes.FIELD_LIST, updateMsg.payload().dataType());

            /* Shutdown the first provider */
            ommprovider.uninitialize();

            Thread.sleep(5000);

            /* Checks to ensure that the second provider doesn't receive the post message as the source directory is not ready yet. */
            assertEquals(0, providerClient3.queueSize());

            /* Checks to ensure that only the first provider of the second connection receives the PostMsg */
            assertEquals(1, providerClient2.queueSize());

            message = providerClient2.popMessage();

            assertTrue(message instanceof PostMsg);

            recvPostMsg = (PostMsg)message;

            assertEquals(1, recvPostMsg.streamId());
            assertEquals("DIRECT_FEED", recvPostMsg.serviceName());
            assertEquals("IBM.N", recvPostMsg.name());
            assertEquals(2, recvPostMsg.postId());
            assertEquals(true, recvPostMsg.complete());
            assertEquals(DataType.DataTypes.UPDATE_MSG, recvPostMsg.payload().dataType());

            updateMsg = recvPostMsg.payload().updateMsg();
            assertEquals(DataType.DataTypes.FIELD_LIST, updateMsg.payload().dataType());

            consumer.unregister(loginHandle);
        }
        catch(OmmException excep)
        {
            System.out.println("Catch OmmException: " + excep.toString());
            assertFalse(true);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        finally {
            System.out.println("Uninitializing...");

            consumer.uninitialize();
            ommprovider.uninitialize();
            ommprovider2.uninitialize();
            ommprovider3.uninitialize();
        }
    }

    @Test
    public void testMultiConnectionsItemRequestsWithServiceListNameButConcreteServicesAreNotAvaliableUnregisterRequestThenConcreateServiceIsAdded()
    {
        TestUtilities.printTestHead("testMultiConnectionsItemRequestsWithServiceListNameButConcreteServicesAreNotAvaliableUnregisterRequestThenConcreateServiceIsAdded","");

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

            serviceList.concreteServiceList().add("UNKNOWN_SERVICE");
            serviceList.concreteServiceList().add("DIRECT_FEED2");

            consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_9").addServiceList(serviceList));

            ReqMsg reqMsg = EmaFactory.createReqMsg();

            long itemHandle = consumer.registerClient(reqMsg.clear().serviceListName("SVG1").name("LSEG.O"), consumerClient);

            Thread.sleep(2000);

            assertEquals(1, consumerClient.queueSize());

            Msg message = consumerClient.popMessage();

            StatusMsg statusMsg = (StatusMsg)message;

            assertEquals("SVG1", statusMsg.serviceName());
            assertEquals("LSEG.O", statusMsg.name());
            assertEquals(OmmState.StreamState.OPEN, statusMsg.state().streamState());
            assertEquals(OmmState.DataState.SUSPECT, statusMsg.state().dataState());
            assertEquals(OmmState.StatusCode.NONE, statusMsg.state().statusCode());
            assertEquals("No matching service present.", statusMsg.state().statusText());

            /* Cancel the request to remove the item */
            consumer.unregister(itemHandle);

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
    

	@Test
	public void testMultiConnectionForItemRecoveyWithPreferredHostWSBForLoginBasedEnabledByFile()
	{
		TestUtilities.printTestHead("testMultiConnectionForItemRecoveyWithPreferredHostWSBForLoginBasedEnabledByFile","");
		
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
		OmmProvider ommprovider1 = EmaFactory.createOmmProvider(config.port("19009").providerName("Provider_5"), providerClient);
		assertNotNull(ommprovider1);
		
		// Provider_5 provides the DIRECT_FEED service name for WSB channel(standby server)
		OmmProvider ommprovider2 = EmaFactory.createOmmProvider(config.port("19010").providerName("Provider_5"), providerClient2);
		assertNotNull(ommprovider2);
		
		
		/* WarmStandbyChannel_6 (non-preferred WSB channel) */
		// Provider_5 provides the DIRECT_FEED service name for WSB channel(starting server)
		OmmProvider ommprovider3 = EmaFactory.createOmmProvider(config.port("19011").providerName("Provider_5"), providerClient3);
		
		assertNotNull(ommprovider3);
		
		// Provider_5 provides the DIRECT_FEED service name for WSB channel(standby server)
		OmmProvider ommprovider4 = EmaFactory.createOmmProvider(config.port("19012").providerName("Provider_5"), providerClient4);
		assertNotNull(ommprovider4);
		
		OmmConsumer consumer = null;
		ConsumerTestOptions consumerOption = new ConsumerTestOptions();
		
		consumerOption.getChannelInformation = true;
		consumerOption.getSessionChannelInfo = false;
		ConsumerTestClient consumerClient = new ConsumerTestClient(consumerOption);
		
		OmmProvider ommprovider5 = null;
		OmmProvider ommprovider6 = null;
		OmmProvider ommprovider7 = null;
		OmmProvider ommprovider8 = null;
		
		try
		{
			consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_18"));
			
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
			
			assertEquals(3, recvReq.streamId());
			assertEquals("DIRECT_FEED", recvReq.serviceName());
			assertEquals("IBM.N", recvReq.name());
			assertEquals(1, recvReq.serviceId());
			
			Thread.sleep(1000);
			
			/* Checks the item refresh message from the starting server of Connection_13 */
			message = consumerClient.popMessage();
			/* Checks the market price item refresh */
			RefreshMsg refreshMsg = (RefreshMsg)message;
			
			assertEquals(5, refreshMsg.streamId());
			assertEquals(DomainTypes.MARKET_PRICE, refreshMsg.domainType());
			assertEquals("Open / Ok / None / 'Refresh Completed'", refreshMsg.state().toString());
			assertTrue(refreshMsg.complete());
			assertTrue(refreshMsg.solicited());
			assertTrue(refreshMsg.hasName());
			assertEquals(itemName, refreshMsg.name());
			assertTrue(refreshMsg.hasServiceId());
			assertEquals(32767, refreshMsg.serviceId());
			assertTrue(refreshMsg.hasServiceName());
			assertEquals(serviceName, refreshMsg.serviceName());
			assertEquals(DataTypes.FIELD_LIST, refreshMsg.payload().dataType());
			ChannelInformation channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_9", channelInfo.channelName());
			assertEquals("Connection_13", channelInfo.sessionChannelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			
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
			
			assertEquals(3, recvReq.streamId());
			assertEquals("DIRECT_FEED", recvReq.serviceName());
			assertEquals("IBM.N", recvReq.name());
			assertEquals(1, recvReq.serviceId());
			
			
			/* WarmStandbyChannel_1 (preferred WSB channel) */
			// Provider_5 provides the DIRECT_FEED service name for WSB channel(starting server)
			ommprovider5 = EmaFactory.createOmmProvider(config.port("19003").providerName("Provider_5"), providerClient);
			
			// Provider_5 provides the DIRECT_FEED service name for WSB channel(standby server)
			ommprovider6 = EmaFactory.createOmmProvider(config.port("19006").providerName("Provider_5"), providerClient2);
			
			/* WarmStandbyChannel_2 (preferred WSB channel) */
			// Provider_5 provides the DIRECT_FEED service name for WSB channel(starting server)
			ommprovider7 = EmaFactory.createOmmProvider(config.port("19007").providerName("Provider_5"), providerClient);
			
			// Provider_5 provides the DIRECT_FEED service name for WSB channel(standby server)
			ommprovider8 = EmaFactory.createOmmProvider(config.port("19008").providerName("Provider_5"), providerClient2);
			
			System.out.println("Waiting for fallback process...");
			Thread.sleep(6000);
			
			/* Checks the item status message from the starting server of Connection_13 when fallback process is trigger */
			System.out.println("consumerClient.queueSize() " + consumerClient.queueSize() );
			assertEquals(2, consumerClient.queueSize());
			
			message = consumerClient.popMessage();
			StatusMsg statusMsg = (StatusMsg)message;
			
			assertEquals(5, statusMsg.streamId());
			assertEquals(DomainTypes.MARKET_PRICE, statusMsg.domainType());
			assertTrue(statusMsg.hasState());
			assertEquals("Open / Suspect / None / 'channel down.'", statusMsg.state().toString());
			assertTrue(statusMsg.hasName());
			assertEquals(itemName, statusMsg.name());
			assertTrue(statusMsg.hasServiceId());
			assertEquals(32767, statusMsg.serviceId());
			assertTrue(statusMsg.hasServiceName());
			assertEquals(serviceName, statusMsg.serviceName());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_9", channelInfo.channelName());
			assertEquals("Connection_13", channelInfo.sessionChannelName());
			assertEquals(ChannelInformation.ChannelState.INACTIVE, channelInfo.channelState());
			
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
			assertEquals(32767, refreshMsg.serviceId());
			assertTrue(refreshMsg.hasServiceName());
			assertEquals(serviceName, refreshMsg.serviceName());
			assertEquals(DataTypes.FIELD_LIST, refreshMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_3", channelInfo.channelName());
			assertEquals("Connection_13", channelInfo.sessionChannelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
		
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
	public void testMultiConnectionForItemRecoveyWithPreferredHostWSBForServiceBasedEnabledByFile()
	{
		TestUtilities.printTestHead("testMultiConnectionForItemRecoveyWithPreferredHostWSBForServiceBasedEnabledByFile","");
		
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
		
		/* WarmStandbyChannel_7 (non-preferred WSB channel) */
		// Provider_5 provides the DIRECT_FEED service name for WSB channel(starting server)
		OmmProvider ommprovider1 = EmaFactory.createOmmProvider(config.port("19009").providerName("Provider_5"), providerClient);
		assertNotNull(ommprovider1);
		
		// Provider_5 provides the DIRECT_FEED service name for WSB channel(standby server)
		OmmProvider ommprovider2 = EmaFactory.createOmmProvider(config.port("19010").providerName("Provider_5"), providerClient2);
		assertNotNull(ommprovider2);
		
		/* WarmStandbyChannel_8 (non-preferred WSB channel) */
		// Provider_5 provides the DIRECT_FEED service name for WSB channel(starting server)
		OmmProvider ommprovider3 = EmaFactory.createOmmProvider(config.port("19011").providerName("Provider_5"), providerClient3);
		
		assertNotNull(ommprovider3);
		
		// Provider_5 provides the DIRECT_FEED service name for WSB channel(standby server)
		OmmProvider ommprovider4 = EmaFactory.createOmmProvider(config.port("19012").providerName("Provider_5"), providerClient4);
		assertNotNull(ommprovider4);
		
		OmmConsumer consumer = null;
		ConsumerTestOptions consumerOption = new ConsumerTestOptions();
		
		consumerOption.getChannelInformation = true;
		consumerOption.getSessionChannelInfo = false;
		ConsumerTestClient consumerClient = new ConsumerTestClient(consumerOption);
		
		OmmProvider ommprovider5 = null;
		OmmProvider ommprovider6 = null;
		OmmProvider ommprovider7 = null;
		OmmProvider ommprovider8 = null;
		
		try
		{
			consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_19"));
			
			String serviceName = "DIRECT_FEED";
			String itemName = "IBM.N";
			
			ReqMsg reqMsg = EmaFactory.createReqMsg();
			
			long itemHandle = consumer.registerClient(reqMsg.name(itemName).serviceName(serviceName), consumerClient);
			
			Thread.sleep(4000);
			
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
			
			/* Checks the item refresh message from the starting server of Connection_13 */
			message = consumerClient.popMessage();
			/* Checks the market price item refresh */
			RefreshMsg refreshMsg = (RefreshMsg)message;
			
			assertEquals(5, refreshMsg.streamId());
			assertEquals(DomainTypes.MARKET_PRICE, refreshMsg.domainType());
			assertEquals("Open / Ok / None / 'Refresh Completed'", refreshMsg.state().toString());
			assertTrue(refreshMsg.complete());
			assertTrue(refreshMsg.solicited());
			assertTrue(refreshMsg.hasName());
			assertEquals(itemName, refreshMsg.name());
			assertTrue(refreshMsg.hasServiceId());
			assertEquals(32767, refreshMsg.serviceId());
			assertTrue(refreshMsg.hasServiceName());
			assertEquals(serviceName, refreshMsg.serviceName());
			assertEquals(DataTypes.FIELD_LIST, refreshMsg.payload().dataType());
			ChannelInformation channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_9", channelInfo.channelName());
			assertEquals("Connection_15", channelInfo.sessionChannelName());
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
			
			
			/* WarmStandbyChannel_3 (preferred WSB channel) */
			// Provider_5 provides the DIRECT_FEED service name for WSB channel(starting server)
			ommprovider5 = EmaFactory.createOmmProvider(config.port("19003").providerName("Provider_5"), providerClient);
			
			// Provider_5 provides the DIRECT_FEED service name for WSB channel(standby server)
			ommprovider6 = EmaFactory.createOmmProvider(config.port("19006").providerName("Provider_5"), providerClient2);
			
			/* WarmStandbyChannel_4 (preferred WSB channel) */
			// Provider_5 provides the DIRECT_FEED service name for WSB channel(starting server)
			ommprovider7 = EmaFactory.createOmmProvider(config.port("19007").providerName("Provider_5"), providerClient);
			
			// Provider_5 provides the DIRECT_FEED service name for WSB channel(standby server)
			ommprovider8 = EmaFactory.createOmmProvider(config.port("19008").providerName("Provider_5"), providerClient2);
			
			System.out.println("Waiting for fallback process...");
			Thread.sleep(6000);
			
			/* Checks the item status message from the starting server of Connection_13 when fallback process is trigger */
			System.out.println("consumerClient.queueSize() " + consumerClient.queueSize() );
			assertEquals(2, consumerClient.queueSize());
			
			message = consumerClient.popMessage();
			StatusMsg statusMsg = (StatusMsg)message;
			
			assertEquals(5, statusMsg.streamId());
			assertEquals(DomainTypes.MARKET_PRICE, statusMsg.domainType());
			assertTrue(statusMsg.hasState());
			assertEquals("Open / Suspect / None / 'channel down.'", statusMsg.state().toString());
			assertTrue(statusMsg.hasName());
			assertEquals(itemName, statusMsg.name());
			assertTrue(statusMsg.hasServiceId());
			assertEquals(32767, statusMsg.serviceId());
			assertTrue(statusMsg.hasServiceName());
			assertEquals(serviceName, statusMsg.serviceName());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_9", channelInfo.channelName());
			assertEquals("Connection_15", channelInfo.sessionChannelName());
			assertEquals(ChannelInformation.ChannelState.INACTIVE, channelInfo.channelState());
			
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
			assertEquals(32767, refreshMsg.serviceId());
			assertTrue(refreshMsg.hasServiceName());
			assertEquals(serviceName, refreshMsg.serviceName());
			assertEquals(DataTypes.FIELD_LIST, refreshMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_3", channelInfo.channelName());
			assertEquals("Connection_15", channelInfo.sessionChannelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
		
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
	public void testMultiConnectionForItemRecoveyWithPreferredHostWSBForLoginBasedEnabledByFallbackMethod()
	{
		TestUtilities.printTestHead("testMultiConnectionForItemRecoveyWithPreferredHostWSBForLoginBasedEnabledByFallbackMethod","");
		
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
		OmmProvider ommprovider1 = EmaFactory.createOmmProvider(config.port("19009").providerName("Provider_5"), providerClient);
		assertNotNull(ommprovider1);
		
		// Provider_5 provides the DIRECT_FEED service name for WSB channel(standby server)
		OmmProvider ommprovider2 = EmaFactory.createOmmProvider(config.port("19010").providerName("Provider_5"), providerClient2);
		assertNotNull(ommprovider2);
		
		
		/* WarmStandbyChannel_6 (non-preferred WSB channel) */
		// Provider_5 provides the DIRECT_FEED service name for WSB channel(starting server)
		OmmProvider ommprovider3 = EmaFactory.createOmmProvider(config.port("19011").providerName("Provider_5"), providerClient3);
		
		assertNotNull(ommprovider3);
		
		// Provider_5 provides the DIRECT_FEED service name for WSB channel(standby server)
		OmmProvider ommprovider4 = EmaFactory.createOmmProvider(config.port("19012").providerName("Provider_5"), providerClient4);
		assertNotNull(ommprovider4);
		
		OmmConsumer consumer = null;
		ConsumerTestOptions consumerOption = new ConsumerTestOptions();
		
		consumerOption.getChannelInformation = true;
		consumerOption.getSessionChannelInfo = false;
		ConsumerTestClient consumerClient = new ConsumerTestClient(consumerOption);
		
		OmmProvider ommprovider5 = null;
		OmmProvider ommprovider6 = null;
		OmmProvider ommprovider7 = null;
		OmmProvider ommprovider8 = null;
		
		try
		{
			consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_18_1"));
			
			String serviceName = "DIRECT_FEED";
			String itemName = "IBM.N";
			
			ReqMsg reqMsg = EmaFactory.createReqMsg();
			
			long itemHandle = consumer.registerClient(reqMsg.name(itemName).serviceName(serviceName), consumerClient);
			
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
			
			assertEquals(3, recvReq.streamId());
			assertEquals("DIRECT_FEED", recvReq.serviceName());
			assertEquals("IBM.N", recvReq.name());
			assertEquals(1, recvReq.serviceId());
			
			Thread.sleep(1000);
			
			/* Checks the item refresh message from the starting server of Connection_13 */
			message = consumerClient.popMessage();
			/* Checks the market price item refresh */
			RefreshMsg refreshMsg = (RefreshMsg)message;
			
			assertEquals(5, refreshMsg.streamId());
			assertEquals(DomainTypes.MARKET_PRICE, refreshMsg.domainType());
			assertEquals("Open / Ok / None / 'Refresh Completed'", refreshMsg.state().toString());
			assertTrue(refreshMsg.complete());
			assertTrue(refreshMsg.solicited());
			assertTrue(refreshMsg.hasName());
			assertEquals(itemName, refreshMsg.name());
			assertTrue(refreshMsg.hasServiceId());
			assertEquals(32767, refreshMsg.serviceId());
			assertTrue(refreshMsg.hasServiceName());
			assertEquals(serviceName, refreshMsg.serviceName());
			assertEquals(DataTypes.FIELD_LIST, refreshMsg.payload().dataType());
			ChannelInformation channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_9", channelInfo.channelName());
			assertEquals("Connection_13_1", channelInfo.sessionChannelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			
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
			
			assertEquals(3, recvReq.streamId());
			assertEquals("DIRECT_FEED", recvReq.serviceName());
			assertEquals("IBM.N", recvReq.name());
			assertEquals(1, recvReq.serviceId());
			
			
			/* WarmStandbyChannel_1 (preferred WSB channel) */
			// Provider_5 provides the DIRECT_FEED service name for WSB channel(starting server)
			ommprovider5 = EmaFactory.createOmmProvider(config.port("19003").providerName("Provider_5"), providerClient);
			
			// Provider_5 provides the DIRECT_FEED service name for WSB channel(standby server)
			ommprovider6 = EmaFactory.createOmmProvider(config.port("19006").providerName("Provider_5"), providerClient2);
			
			/* WarmStandbyChannel_2 (preferred WSB channel) */
			// Provider_5 provides the DIRECT_FEED service name for WSB channel(starting server)
			ommprovider7 = EmaFactory.createOmmProvider(config.port("19007").providerName("Provider_5"), providerClient);
			
			// Provider_5 provides the DIRECT_FEED service name for WSB channel(standby server)
			ommprovider8 = EmaFactory.createOmmProvider(config.port("19008").providerName("Provider_5"), providerClient2);
			
			consumer.fallbackPreferredHost();
			Thread.sleep(4000);
			
			/* Checks the item status message from the starting server of Connection_13 when fallback process is trigger */
			System.out.println("consumerClient.queueSize() " + consumerClient.queueSize() );
			assertEquals(2, consumerClient.queueSize());
			
			message = consumerClient.popMessage();
			StatusMsg statusMsg = (StatusMsg)message;
			
			assertEquals(5, statusMsg.streamId());
			assertEquals(DomainTypes.MARKET_PRICE, statusMsg.domainType());
			assertTrue(statusMsg.hasState());
			assertEquals("Open / Suspect / None / 'channel down.'", statusMsg.state().toString());
			assertTrue(statusMsg.hasName());
			assertEquals(itemName, statusMsg.name());
			assertTrue(statusMsg.hasServiceId());
			assertEquals(32767, statusMsg.serviceId());
			assertTrue(statusMsg.hasServiceName());
			assertEquals(serviceName, statusMsg.serviceName());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_9", channelInfo.channelName());
			assertEquals("Connection_13_1", channelInfo.sessionChannelName());
			assertEquals(ChannelInformation.ChannelState.INACTIVE, channelInfo.channelState());
			
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
			assertEquals(32767, refreshMsg.serviceId());
			assertTrue(refreshMsg.hasServiceName());
			assertEquals(serviceName, refreshMsg.serviceName());
			assertEquals(DataTypes.FIELD_LIST, refreshMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_3", channelInfo.channelName());
			assertEquals("Connection_13_1", channelInfo.sessionChannelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
		
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
	public void testMultiConnectionForItemRecoveyWithPreferredHostWSBForServiceBasedEnabledByFallbackMethod()
	{
		TestUtilities.printTestHead("testMultiConnectionForItemRecoveyWithPreferredHostWSBForServiceBasedEnabledByFallbackMethod","");
		
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
		
		/* WarmStandbyChannel_7 (non-preferred WSB channel) */
		// Provider_5 provides the DIRECT_FEED service name for WSB channel(starting server)
		OmmProvider ommprovider1 = EmaFactory.createOmmProvider(config.port("19009").providerName("Provider_5"), providerClient);
		assertNotNull(ommprovider1);
		
		// Provider_5 provides the DIRECT_FEED service name for WSB channel(standby server)
		OmmProvider ommprovider2 = EmaFactory.createOmmProvider(config.port("19010").providerName("Provider_5"), providerClient2);
		assertNotNull(ommprovider2);
		
		/* WarmStandbyChannel_8 (non-preferred WSB channel) */
		// Provider_5 provides the DIRECT_FEED service name for WSB channel(starting server)
		OmmProvider ommprovider3 = EmaFactory.createOmmProvider(config.port("19011").providerName("Provider_5"), providerClient3);
		
		assertNotNull(ommprovider3);
		
		// Provider_5 provides the DIRECT_FEED service name for WSB channel(standby server)
		OmmProvider ommprovider4 = EmaFactory.createOmmProvider(config.port("19012").providerName("Provider_5"), providerClient4);
		assertNotNull(ommprovider4);
		
		OmmConsumer consumer = null;
		ConsumerTestOptions consumerOption = new ConsumerTestOptions();
		
		consumerOption.getChannelInformation = true;
		consumerOption.getSessionChannelInfo = false;
		ConsumerTestClient consumerClient = new ConsumerTestClient(consumerOption);
		
		OmmProvider ommprovider5 = null;
		OmmProvider ommprovider6 = null;
		OmmProvider ommprovider7 = null;
		OmmProvider ommprovider8 = null;
		
		try
		{
			consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_19_1"));
			
			String serviceName = "DIRECT_FEED";
			String itemName = "IBM.N";
			
			ReqMsg reqMsg = EmaFactory.createReqMsg();
			
			long itemHandle = consumer.registerClient(reqMsg.name(itemName).serviceName(serviceName), consumerClient);
			
			Thread.sleep(4000);
			
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
			
			/* Checks the item refresh message from the starting server of Connection_13 */
			message = consumerClient.popMessage();
			/* Checks the market price item refresh */
			RefreshMsg refreshMsg = (RefreshMsg)message;
			
			assertEquals(5, refreshMsg.streamId());
			assertEquals(DomainTypes.MARKET_PRICE, refreshMsg.domainType());
			assertEquals("Open / Ok / None / 'Refresh Completed'", refreshMsg.state().toString());
			assertTrue(refreshMsg.complete());
			assertTrue(refreshMsg.solicited());
			assertTrue(refreshMsg.hasName());
			assertEquals(itemName, refreshMsg.name());
			assertTrue(refreshMsg.hasServiceId());
			assertEquals(32767, refreshMsg.serviceId());
			assertTrue(refreshMsg.hasServiceName());
			assertEquals(serviceName, refreshMsg.serviceName());
			assertEquals(DataTypes.FIELD_LIST, refreshMsg.payload().dataType());
			ChannelInformation channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_9", channelInfo.channelName());
			assertEquals("Connection_15_1", channelInfo.sessionChannelName());
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
			
			
			/* WarmStandbyChannel_3 (preferred WSB channel) */
			// Provider_5 provides the DIRECT_FEED service name for WSB channel(starting server)
			ommprovider5 = EmaFactory.createOmmProvider(config.port("19003").providerName("Provider_5"), providerClient);
			
			// Provider_5 provides the DIRECT_FEED service name for WSB channel(standby server)
			ommprovider6 = EmaFactory.createOmmProvider(config.port("19006").providerName("Provider_5"), providerClient2);
			
			/* WarmStandbyChannel_4 (preferred WSB channel) */
			// Provider_5 provides the DIRECT_FEED service name for WSB channel(starting server)
			ommprovider7 = EmaFactory.createOmmProvider(config.port("19007").providerName("Provider_5"), providerClient);
			
			// Provider_5 provides the DIRECT_FEED service name for WSB channel(standby server)
			ommprovider8 = EmaFactory.createOmmProvider(config.port("19008").providerName("Provider_5"), providerClient2);
			
			consumer.fallbackPreferredHost();
			Thread.sleep(5000);
			
			/* Checks the item status message from the starting server of Connection_13 when fallback process is trigger */
			System.out.println("consumerClient.queueSize() " + consumerClient.queueSize() );
			assertEquals(2, consumerClient.queueSize());
			
			message = consumerClient.popMessage();
			StatusMsg statusMsg = (StatusMsg)message;
			
			assertEquals(5, statusMsg.streamId());
			assertEquals(DomainTypes.MARKET_PRICE, statusMsg.domainType());
			assertTrue(statusMsg.hasState());
			assertEquals("Open / Suspect / None / 'channel down.'", statusMsg.state().toString());
			assertTrue(statusMsg.hasName());
			assertEquals(itemName, statusMsg.name());
			assertTrue(statusMsg.hasServiceId());
			assertEquals(32767, statusMsg.serviceId());
			assertTrue(statusMsg.hasServiceName());
			assertEquals(serviceName, statusMsg.serviceName());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_9", channelInfo.channelName());
			assertEquals("Connection_15_1", channelInfo.sessionChannelName());
			assertEquals(ChannelInformation.ChannelState.INACTIVE, channelInfo.channelState()); // This must be fixed to ChannelInformation.ChannelState.INACTIVE
			
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
			assertEquals(32767, refreshMsg.serviceId());
			assertTrue(refreshMsg.hasServiceName());
			assertEquals(serviceName, refreshMsg.serviceName());
			assertEquals(DataTypes.FIELD_LIST, refreshMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_3", channelInfo.channelName());
			assertEquals("Connection_15_1", channelInfo.sessionChannelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
		
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
	public void testMultiConnectionForItemRecoveyWithPreferredHostWSBForLoginBasedEnabledByIOCtlMethod()
	{
		TestUtilities.printTestHead("testMultiConnectionForItemRecoveyWithPreferredHostWSBForLoginBasedEnabledByIOCtlMethod","");
		
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
		OmmProvider ommprovider1 = EmaFactory.createOmmProvider(config.port("19009").providerName("Provider_5"), providerClient);
		assertNotNull(ommprovider1);
		
		// Provider_5 provides the DIRECT_FEED service name for WSB channel(standby server)
		OmmProvider ommprovider2 = EmaFactory.createOmmProvider(config.port("19010").providerName("Provider_5"), providerClient2);
		assertNotNull(ommprovider2);
		
		
		/* WarmStandbyChannel_6 (non-preferred WSB channel) */
		// Provider_5 provides the DIRECT_FEED service name for WSB channel(starting server)
		OmmProvider ommprovider3 = EmaFactory.createOmmProvider(config.port("19011").providerName("Provider_5"), providerClient3);
		
		assertNotNull(ommprovider3);
		
		// Provider_5 provides the DIRECT_FEED service name for WSB channel(standby server)
		OmmProvider ommprovider4 = EmaFactory.createOmmProvider(config.port("19012").providerName("Provider_5"), providerClient4);
		assertNotNull(ommprovider4);
		
		OmmConsumer consumer = null;
		ConsumerTestOptions consumerOption = new ConsumerTestOptions();
		
		consumerOption.getChannelInformation = true;
		consumerOption.getSessionChannelInfo = false;
		ConsumerTestClient consumerClient = new ConsumerTestClient(consumerOption);
		
		OmmProvider ommprovider5 = null;
		OmmProvider ommprovider6 = null;
		OmmProvider ommprovider7 = null;
		OmmProvider ommprovider8 = null;
		
		try
		{
			consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_20"));
			
			String serviceName = "DIRECT_FEED";
			String itemName = "IBM.N";
			
			ReqMsg reqMsg = EmaFactory.createReqMsg();
			
			long itemHandle = consumer.registerClient(reqMsg.name(itemName).serviceName(serviceName), consumerClient);
			
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
			
			assertEquals(3, recvReq.streamId());
			assertEquals("DIRECT_FEED", recvReq.serviceName());
			assertEquals("IBM.N", recvReq.name());
			assertEquals(1, recvReq.serviceId());
			
			Thread.sleep(1000);
			
			/* Checks the item refresh message from the starting server of Connection_13 */
			message = consumerClient.popMessage();
			/* Checks the market price item refresh */
			RefreshMsg refreshMsg = (RefreshMsg)message;
			
			assertEquals(5, refreshMsg.streamId());
			assertEquals(DomainTypes.MARKET_PRICE, refreshMsg.domainType());
			assertEquals("Open / Ok / None / 'Refresh Completed'", refreshMsg.state().toString());
			assertTrue(refreshMsg.complete());
			assertTrue(refreshMsg.solicited());
			assertTrue(refreshMsg.hasName());
			assertEquals(itemName, refreshMsg.name());
			assertTrue(refreshMsg.hasServiceId());
			assertEquals(32767, refreshMsg.serviceId());
			assertTrue(refreshMsg.hasServiceName());
			assertEquals(serviceName, refreshMsg.serviceName());
			assertEquals(DataTypes.FIELD_LIST, refreshMsg.payload().dataType());
			ChannelInformation channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_9", channelInfo.channelName());
			assertEquals("Connection_17", channelInfo.sessionChannelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			
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
			
			assertEquals(3, recvReq.streamId());
			assertEquals("DIRECT_FEED", recvReq.serviceName());
			assertEquals("IBM.N", recvReq.name());
			assertEquals(1, recvReq.serviceId());
			
			
			/* WarmStandbyChannel_1 (preferred WSB channel) */
			// Provider_5 provides the DIRECT_FEED service name for WSB channel(starting server)
			ommprovider5 = EmaFactory.createOmmProvider(config.port("19003").providerName("Provider_5"), providerClient);
			
			// Provider_5 provides the DIRECT_FEED service name for WSB channel(standby server)
			ommprovider6 = EmaFactory.createOmmProvider(config.port("19006").providerName("Provider_5"), providerClient2);
			
			/* WarmStandbyChannel_2 (preferred WSB channel) */
			// Provider_5 provides the DIRECT_FEED service name for WSB channel(starting server)
			ommprovider7 = EmaFactory.createOmmProvider(config.port("19007").providerName("Provider_5"), providerClient);
			
			// Provider_5 provides the DIRECT_FEED service name for WSB channel(standby server)
			ommprovider8 = EmaFactory.createOmmProvider(config.port("19008").providerName("Provider_5"), providerClient2);
			
			// Enable the PH fallback process with the modifyIOCtl() method.
			PreferredHostOptions phOptions = EmaFactory.createPreferredHostOptions();
			phOptions.setPreferredHostEnabled(true);
			phOptions.setWsbChannelName("WarmStandbyChannel_1");
			phOptions.setDetectionTimeInterval(1);
			phOptions.sessionChannelName("Connection_17");
			
			PreferredHostOptions phOptions2 = EmaFactory.createPreferredHostOptions();
			phOptions2.setPreferredHostEnabled(true);
			phOptions2.setWsbChannelName("WarmStandbyChannel_2");
			phOptions2.setDetectionTimeInterval(1);
			phOptions2.sessionChannelName("Connection_18");
			
			consumer.modifyIOCtl(IOCtlCode.FALLBACK_PREFERRED_HOST_OPTIONS, phOptions);
			consumer.modifyIOCtl(IOCtlCode.FALLBACK_PREFERRED_HOST_OPTIONS, phOptions2);
			Thread.sleep(2000);
			
			/* Checks the item status message from the starting server of Connection_13 when fallback process is trigger */
			System.out.println("consumerClient.queueSize() " + consumerClient.queueSize() );
			assertEquals(2, consumerClient.queueSize());
			
			message = consumerClient.popMessage();
			StatusMsg statusMsg = (StatusMsg)message;
			
			assertEquals(5, statusMsg.streamId());
			assertEquals(DomainTypes.MARKET_PRICE, statusMsg.domainType());
			assertTrue(statusMsg.hasState());
			assertEquals("Open / Suspect / None / 'channel down.'", statusMsg.state().toString());
			assertTrue(statusMsg.hasName());
			assertEquals(itemName, statusMsg.name());
			assertTrue(statusMsg.hasServiceId());
			assertEquals(32767, statusMsg.serviceId());
			assertTrue(statusMsg.hasServiceName());
			assertEquals(serviceName, statusMsg.serviceName());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_9", channelInfo.channelName());
			assertEquals("Connection_17", channelInfo.sessionChannelName());
			assertEquals(ChannelInformation.ChannelState.INACTIVE, channelInfo.channelState());
			
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
			assertEquals(32767, refreshMsg.serviceId());
			assertTrue(refreshMsg.hasServiceName());
			assertEquals(serviceName, refreshMsg.serviceName());
			assertEquals(DataTypes.FIELD_LIST, refreshMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_3", channelInfo.channelName());
			assertEquals("Connection_17", channelInfo.sessionChannelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			
			// Disable the preferred host feature
			phOptions.setPreferredHostEnabled(false);
			phOptions2.setPreferredHostEnabled(false);
			
			consumer.modifyIOCtl(IOCtlCode.FALLBACK_PREFERRED_HOST_OPTIONS, phOptions);
			consumer.modifyIOCtl(IOCtlCode.FALLBACK_PREFERRED_HOST_OPTIONS, phOptions2);
		
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
			
			assertNotNull(consumer);
		}
	}
	
	@Test
	public void testMultiConnectionForItemRecoveyWithPreferredHostWSBForServiceBasedEnabledByIOCtrlMethod()
	{
		TestUtilities.printTestHead("testMultiConnectionForItemRecoveyWithPreferredHostWSBForServiceBasedEnabledByIOCtrlMethod","");
		
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
		
		/* WarmStandbyChannel_7 (non-preferred WSB channel) */
		// Provider_5 provides the DIRECT_FEED service name for WSB channel(starting server)
		OmmProvider ommprovider1 = EmaFactory.createOmmProvider(config.port("19009").providerName("Provider_5"), providerClient);
		assertNotNull(ommprovider1);
		
		// Provider_5 provides the DIRECT_FEED service name for WSB channel(standby server)
		OmmProvider ommprovider2 = EmaFactory.createOmmProvider(config.port("19010").providerName("Provider_5"), providerClient2);
		assertNotNull(ommprovider2);
		
		/* WarmStandbyChannel_8 (non-preferred WSB channel) */
		// Provider_5 provides the DIRECT_FEED service name for WSB channel(starting server)
		OmmProvider ommprovider3 = EmaFactory.createOmmProvider(config.port("19011").providerName("Provider_5"), providerClient3);
		
		assertNotNull(ommprovider3);
		
		// Provider_5 provides the DIRECT_FEED service name for WSB channel(standby server)
		OmmProvider ommprovider4 = EmaFactory.createOmmProvider(config.port("19012").providerName("Provider_5"), providerClient4);
		assertNotNull(ommprovider4);
		
		OmmConsumer consumer = null;
		ConsumerTestOptions consumerOption = new ConsumerTestOptions();
		
		consumerOption.getChannelInformation = true;
		consumerOption.getSessionChannelInfo = false;
		ConsumerTestClient consumerClient = new ConsumerTestClient(consumerOption);
		
		OmmProvider ommprovider5 = null;
		OmmProvider ommprovider6 = null;
		OmmProvider ommprovider7 = null;
		OmmProvider ommprovider8 = null;
		
		try
		{
			consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_21"));
			
			String serviceName = "DIRECT_FEED";
			String itemName = "IBM.N";
			
			ReqMsg reqMsg = EmaFactory.createReqMsg();
			
			long itemHandle = consumer.registerClient(reqMsg.name(itemName).serviceName(serviceName), consumerClient);
			
			Thread.sleep(4000);
			
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
			
			/* Checks the item refresh message from the starting server of Connection_13 */
			message = consumerClient.popMessage();
			/* Checks the market price item refresh */
			RefreshMsg refreshMsg = (RefreshMsg)message;
			
			assertEquals(5, refreshMsg.streamId());
			assertEquals(DomainTypes.MARKET_PRICE, refreshMsg.domainType());
			assertEquals("Open / Ok / None / 'Refresh Completed'", refreshMsg.state().toString());
			assertTrue(refreshMsg.complete());
			assertTrue(refreshMsg.solicited());
			assertTrue(refreshMsg.hasName());
			assertEquals(itemName, refreshMsg.name());
			assertTrue(refreshMsg.hasServiceId());
			assertEquals(32767, refreshMsg.serviceId());
			assertTrue(refreshMsg.hasServiceName());
			assertEquals(serviceName, refreshMsg.serviceName());
			assertEquals(DataTypes.FIELD_LIST, refreshMsg.payload().dataType());
			ChannelInformation channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_9", channelInfo.channelName());
			assertEquals("Connection_19", channelInfo.sessionChannelName());
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
			
			
			/* WarmStandbyChannel_3 (preferred WSB channel) */
			// Provider_5 provides the DIRECT_FEED service name for WSB channel(starting server)
			ommprovider5 = EmaFactory.createOmmProvider(config.port("19003").providerName("Provider_5"), providerClient);
			
			// Provider_5 provides the DIRECT_FEED service name for WSB channel(standby server)
			ommprovider6 = EmaFactory.createOmmProvider(config.port("19006").providerName("Provider_5"), providerClient2);
			
			/* WarmStandbyChannel_4 (preferred WSB channel) */
			// Provider_5 provides the DIRECT_FEED service name for WSB channel(starting server)
			ommprovider7 = EmaFactory.createOmmProvider(config.port("19007").providerName("Provider_5"), providerClient);
			
			// Provider_5 provides the DIRECT_FEED service name for WSB channel(standby server)
			ommprovider8 = EmaFactory.createOmmProvider(config.port("19008").providerName("Provider_5"), providerClient2);
			
			// Enable the PH fallback process with the modifyIOCtl() method.
			PreferredHostOptions phOptions = EmaFactory.createPreferredHostOptions();
			phOptions.setPreferredHostEnabled(true);
			phOptions.setWsbChannelName("WarmStandbyChannel_3");
			phOptions.setDetectionTimeInterval(1);
			phOptions.sessionChannelName("Connection_19");
			
			PreferredHostOptions phOptions2 = EmaFactory.createPreferredHostOptions();
			phOptions2.setPreferredHostEnabled(true);
			phOptions2.setWsbChannelName("WarmStandbyChannel_4");
			phOptions2.setDetectionTimeInterval(1);
			phOptions2.sessionChannelName("Connection_20");
			
			consumer.modifyIOCtl(IOCtlCode.FALLBACK_PREFERRED_HOST_OPTIONS, phOptions);
			consumer.modifyIOCtl(IOCtlCode.FALLBACK_PREFERRED_HOST_OPTIONS, phOptions2);
			Thread.sleep(2000);
			
			/* Checks the item status message from the starting server of Connection_13 when fallback process is trigger */
			System.out.println("consumerClient.queueSize() " + consumerClient.queueSize() );
			assertEquals(2, consumerClient.queueSize());
			
			message = consumerClient.popMessage();
			StatusMsg statusMsg = (StatusMsg)message;
			
			assertEquals(5, statusMsg.streamId());
			assertEquals(DomainTypes.MARKET_PRICE, statusMsg.domainType());
			assertTrue(statusMsg.hasState());
			assertEquals("Open / Suspect / None / 'channel down.'", statusMsg.state().toString());
			assertTrue(statusMsg.hasName());
			assertEquals(itemName, statusMsg.name());
			assertTrue(statusMsg.hasServiceId());
			assertEquals(32767, statusMsg.serviceId());
			assertTrue(statusMsg.hasServiceName());
			assertEquals(serviceName, statusMsg.serviceName());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_9", channelInfo.channelName());
			assertEquals("Connection_19", channelInfo.sessionChannelName());
			assertEquals(ChannelInformation.ChannelState.INACTIVE, channelInfo.channelState());
			
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
			assertEquals(32767, refreshMsg.serviceId());
			assertTrue(refreshMsg.hasServiceName());
			assertEquals(serviceName, refreshMsg.serviceName());
			assertEquals(DataTypes.FIELD_LIST, refreshMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_3", channelInfo.channelName());
			assertEquals("Connection_19", channelInfo.sessionChannelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			
			phOptions.setPreferredHostEnabled(false);
			consumer.modifyIOCtl(IOCtlCode.FALLBACK_PREFERRED_HOST_OPTIONS, phOptions);
			
			phOptions2.setPreferredHostEnabled(false);
			consumer.modifyIOCtl(IOCtlCode.FALLBACK_PREFERRED_HOST_OPTIONS, phOptions2);
		
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
			System.out.println("Uninitilizing...");
		
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
			
			assertNotNull(consumer);
			consumer.uninitialize();
		}
	}
	
	@Test
	public void testMultiConnectionForItemRecoveryWithPreferredHostFromChannelSetToWSBGroupTest()
	{
		TestUtilities.printTestHead("testMultiConnectionForItemRecoveryWithPreferredHostFromChannelSetToWSBGroupTest","");
		
		String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";
		
		OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);
		
		ProviderTestOptions providerTestOptions = new ProviderTestOptions();
		providerTestOptions.sendRefreshAttrib = true;
		providerTestOptions.itemGroupId = ByteBuffer.wrap("10".getBytes());
		
		ProviderTestOptions providerTestOptions2 = new ProviderTestOptions();
		providerTestOptions2.sendRefreshAttrib = true;
		providerTestOptions2.supportStandby = true;
		providerTestOptions2.itemGroupId = ByteBuffer.wrap("10".getBytes());
		
		ProviderTestClient providerClient = new ProviderTestClient(providerTestOptions);
		ProviderTestClient providerClient2 = new ProviderTestClient(providerTestOptions);
		ProviderTestClient providerClient3 = new ProviderTestClient(providerTestOptions2);
		ProviderTestClient providerClient4 = new ProviderTestClient(providerTestOptions2);
		
		/* Channel_1 of  Connection_21 */
		// Provider_5 provides the DIRECT_FEED service name for WSB channel(starting server)
		OmmProvider ommprovider1 = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_5"), providerClient);
		assertNotNull(ommprovider1);
		
		/* Channel_4 of Connection_2 */
		// Provider_5 provides the DIRECT_FEED service name for WSB channel(standby server)
		OmmProvider ommprovider2 = EmaFactory.createOmmProvider(config.port("19004").providerName("Provider_5"), providerClient2);
		assertNotNull(ommprovider2);
		
		OmmConsumer consumer = null;
		ConsumerTestOptions consumerOption = new ConsumerTestOptions();
		
		consumerOption.getChannelInformation = true;
		consumerOption.getSessionChannelInfo = false;
		ConsumerTestClient consumerClient = new ConsumerTestClient(consumerOption);
		
		OmmProvider ommprovider3 = null;
		OmmProvider ommprovider4 = null;
		
		try
		{
			consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_22"));
			
			String serviceName = "DIRECT_FEED";
			String itemName = "IBM.N";
			
			ReqMsg reqMsg = EmaFactory.createReqMsg();
			
			long itemHandle = consumer.registerClient(reqMsg.name(itemName).serviceName(serviceName), consumerClient);
			
			Thread.sleep(4000);
			
			/* Receives one generic message and one request message of active server for the first connection */
			assertEquals(1, providerClient.queueSize());
			
			/* Market price request message for active server for the first connection */
			Msg message = providerClient.popMessage();
			ReqMsg recvReq = (ReqMsg)message;
			
			assertEquals(3, recvReq.streamId());
			assertEquals("DIRECT_FEED", recvReq.serviceName());
			assertEquals("IBM.N", recvReq.name());
			assertEquals(1, recvReq.serviceId());
			
			Thread.sleep(1000);
			
			/* Checks the item refresh message from the starting server of Connection_13 */
			message = consumerClient.popMessage();
			/* Checks the market price item refresh */
			RefreshMsg refreshMsg = (RefreshMsg)message;
			
			assertEquals(5, refreshMsg.streamId());
			assertEquals(DomainTypes.MARKET_PRICE, refreshMsg.domainType());
			assertEquals("Open / Ok / None / 'Refresh Completed'", refreshMsg.state().toString());
			assertTrue(refreshMsg.complete());
			assertTrue(refreshMsg.solicited());
			assertTrue(refreshMsg.hasName());
			assertEquals(itemName, refreshMsg.name());
			assertTrue(refreshMsg.hasServiceId());
			assertEquals(32767, refreshMsg.serviceId());
			assertTrue(refreshMsg.hasServiceName());
			assertEquals(serviceName, refreshMsg.serviceName());
			assertEquals(DataTypes.FIELD_LIST, refreshMsg.payload().dataType());
			ChannelInformation channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_1", channelInfo.channelName());
			assertEquals("Connection_21", channelInfo.sessionChannelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			
			/* WarmStandbyChannel_1 (preferred WSB channel) */
			// Provider_5 provides the DIRECT_FEED service name for WSB channel(starting server)
			ommprovider3 = EmaFactory.createOmmProvider(config.port("19003").providerName("Provider_5"), providerClient3);
			
			// Provider_5 provides the DIRECT_FEED service name for WSB channel(standby server)
			ommprovider4 = EmaFactory.createOmmProvider(config.port("19006").providerName("Provider_5"), providerClient4);
			
			System.out.println("Waiting for fallback to the preferred host");
			Thread.sleep(5000);
			
			/* Checks the item status message from the starting server of Connection_13 when fallback process is trigger */
			System.out.println("consumerClient.queueSize() " + consumerClient.queueSize() );
			assertEquals(2, consumerClient.queueSize());
			
			message = consumerClient.popMessage();
			StatusMsg statusMsg = (StatusMsg)message;
			
			assertEquals(5, statusMsg.streamId());
			assertEquals(DomainTypes.MARKET_PRICE, statusMsg.domainType());
			assertTrue(statusMsg.hasState());
			assertEquals("Open / Suspect / None / 'channel down.'", statusMsg.state().toString());
			assertTrue(statusMsg.hasName());
			assertEquals(itemName, statusMsg.name());
			assertTrue(statusMsg.hasServiceId());
			assertEquals(32767, statusMsg.serviceId());
			assertTrue(statusMsg.hasServiceName());
			assertEquals(serviceName, statusMsg.serviceName());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_1", channelInfo.channelName());
			assertEquals("Connection_21", channelInfo.sessionChannelName());
			assertEquals(ChannelInformation.ChannelState.INACTIVE, channelInfo.channelState());
			
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
			assertEquals(32767, refreshMsg.serviceId());
			assertTrue(refreshMsg.hasServiceName());
			assertEquals(serviceName, refreshMsg.serviceName());
			assertEquals(DataTypes.FIELD_LIST, refreshMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_3", channelInfo.channelName());
			assertEquals("Connection_21", channelInfo.sessionChannelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
		
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
			
			if(ommprovider3 != null)
				ommprovider3.uninitialize();
			
			if(ommprovider4 != null)
				ommprovider4.uninitialize();
		}
	}
	
	public void testMultiConnectionsTriggerToConnectToPreferredWSBChannel_ServiceBasedTest()
	{
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
		
		// Connection_22
		// WarmStandbyChannel_3_2 (preferred WSB channel)
		// Provider_9 provides the DIRECT_FEED(RT/TickByTick) and DIRECT_FEED1(RT/JustInTimeConflated) service name for WSB channel(standby server)
		OmmProvider ommprovider2 = EmaFactory.createOmmProvider(config.port("19006").providerName("Provider_9"), providerClient2);
		assertNotNull(ommprovider2);
		
		/* WarmStandbyChannel_4_1 */
		// Provider_9 provides the DIRECT_FEED and DIRECT_FEED1 service name for WSB channel(starting server)
		OmmProvider ommprovider3 = EmaFactory.createOmmProvider(config.port("19007").providerName("Provider_9"), providerClient3);
		assertNotNull(ommprovider3);
		
		// Provider_9 provides the DIRECT_FEED and DIRECT_FEED1 service name for WSB channel(standby server)
		OmmProvider ommprovider4 = EmaFactory.createOmmProvider(config.port("19008").providerName("Provider_9"), providerClient4);
		assertNotNull(ommprovider4);
		
		// Connection_23
		// Provider_9 provides the DIRECT_FEED and DIRECT_FEED1 service name for channel_1
		OmmProvider ommprovider5 = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_9"), providerClient5);
		assertNotNull(ommprovider5);
		
		OmmConsumer consumer = null;
		ConsumerTestOptions consumerOption = new ConsumerTestOptions();
		
		consumerOption.getChannelInformation = true;
		consumerOption.getSessionChannelInfo = false;
		ConsumerTestClient consumerClient = new ConsumerTestClient(consumerOption);
		
		OmmProvider ommprovider1 = null;
		
		try
		{	
			String serviceName = "DIRECT_FEED";
			String itemName = "TRI.N";
			String serviceName2 = "DIRECT_FEED1";
			long serviceListId = 32767;
			
			ServiceList serviceList = EmaFactory.createServiceList("SVG1");
			
			serviceList.concreteServiceList().add(serviceName);
			serviceList.concreteServiceList().add(serviceName2);
			
			consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_25").addServiceList(serviceList), consumerClient);
			
			System.out.println("consumerClient.channelInfoSize() = "+ consumerClient.channelInfoSize());
			System.out.println("consumerClient.queueSize() = " + consumerClient.queueSize());
			
			Msg message = consumerClient.popMessage();
			StatusMsg statusMsg = (StatusMsg)message;
			
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Suspect / None / 'session channel down reconnecting'", statusMsg.state().toString());
			
			ChannelInformation channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_3", channelInfo.channelName());
			assertEquals("Connection_22", channelInfo.sessionChannelName());
			assertEquals(ChannelInformation.ChannelState.INITIALIZING, channelInfo.channelState());
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Suspect / None / 'session channel up'", statusMsg.state().toString());
			
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_1", channelInfo.channelName());
			assertEquals("Connection_23", channelInfo.sessionChannelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Suspect / None / 'session channel down reconnecting'", statusMsg.state().toString());
			
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_3", channelInfo.channelName());
			assertEquals("Connection_22", channelInfo.sessionChannelName());
			assertEquals(ChannelInformation.ChannelState.INITIALIZING, channelInfo.channelState());
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Suspect / None / 'session channel up'", statusMsg.state().toString());
			
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_7", channelInfo.channelName());
			assertEquals("Connection_22", channelInfo.sessionChannelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			
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
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_7", channelInfo.channelName());
			assertEquals("Connection_22", channelInfo.sessionChannelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			
			ReqMsg reqMsg = EmaFactory.createReqMsg();
			long itemHandle = consumer.registerClient(reqMsg.name(itemName).serviceListName("SVG1").qos(ReqMsg.Timeliness.REALTIME, ReqMsg.Rate.JIT_CONFLATED), 
					consumerClient);

			Thread.sleep(1000);
			
			long serviceId = 1;  // DIRECT_FEED
			long serviceId2 = 2; // DIRECT_FEED1
			
			/* Receives two generic messages and one request message of active server for the WarmStandbyChannel_4_1 */
			assertEquals(3, providerClient3.queueSize());
			
			message = providerClient3.popMessage();
			
			/* GenericMsg for DIRECT_FEED to active server of the WarmStandbyChannel_4_1*/
			GenericMsg genericMsg = (GenericMsg)message;
			assertEquals(2, genericMsg.streamId());
			assertEquals(DomainTypes.SOURCE, genericMsg.domainType());
			assertEquals(EmaRdm.ENAME_CONS_STATUS, genericMsg.name());
			assertEquals(DataTypes.MAP, genericMsg.payload().dataType());
			
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
			
			message = providerClient3.popMessage();
			
			/* GenericMsg for DIRECT_FEED1 to active server of the WarmStandbyChannel_4_1*/
			genericMsg = (GenericMsg)message;
			assertEquals(2, genericMsg.streamId());
			assertEquals(DomainTypes.SOURCE, genericMsg.domainType());
			assertEquals(EmaRdm.ENAME_CONS_STATUS, genericMsg.name());
			assertEquals(DataTypes.MAP, genericMsg.payload().dataType());
			
			map = genericMsg.payload().map();
			mapIt = map.iterator();
			assertTrue(mapIt.hasNext());
			mapEntry = mapIt.next();
			assertEquals(serviceId2, mapEntry.key().uintValue());
			assertEquals(DataTypes.ELEMENT_LIST, mapEntry.loadType());
			
			elementList = mapEntry.elementList();
			elementListIt = elementList.iterator();
			assertTrue(elementListIt.hasNext());
			elementEntry = elementListIt.next();
			assertEquals(EmaRdm.ENAME_WARMSTANDBY_MODE, elementEntry.name());
			assertEquals(ServerTypes.STANDBY, elementEntry.uintValue());
			assertFalse(elementListIt.hasNext()); // End of ElementList
			
			assertFalse(mapIt.hasNext()); // End of Map
			
			/* Market price request message for active server for the WarmStandbyChannel_4_1 */
			message = providerClient3.popMessage();
			ReqMsg recvReq = (ReqMsg)message;
			
			assertEquals(3, recvReq.streamId());
			assertEquals("DIRECT_FEED1", recvReq.serviceName());
			assertEquals(itemName, recvReq.name());
			assertEquals(2, recvReq.serviceId());
			
			/* Receives two generic messages and one request message of standby server for the WarmStandbyChannel_4_1 */
			assertEquals(3, providerClient4.queueSize());
			
			message = providerClient4.popMessage();
			
			/* GenericMsg for DIRECT_FEED to standby server of the WarmStandbyChannel_4_1*/
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
			
			message = providerClient4.popMessage();
			
			/* GenericMsg for DIRECT_FEED1 to standby server of the WarmStandbyChannel_4_1*/
			genericMsg = (GenericMsg)message;
			assertEquals(2, genericMsg.streamId());
			assertEquals(DomainTypes.SOURCE, genericMsg.domainType());
			assertEquals(EmaRdm.ENAME_CONS_STATUS, genericMsg.name());
			assertEquals(DataTypes.MAP, genericMsg.payload().dataType());
			
			map = genericMsg.payload().map();
			mapIt = map.iterator();
			assertTrue(mapIt.hasNext());
			mapEntry = mapIt.next();
			assertEquals(serviceId2, mapEntry.key().uintValue());
			assertEquals(DataTypes.ELEMENT_LIST, mapEntry.loadType());
			
			elementList = mapEntry.elementList();
			elementListIt = elementList.iterator();
			assertTrue(elementListIt.hasNext());
			elementEntry = elementListIt.next();
			assertEquals(EmaRdm.ENAME_WARMSTANDBY_MODE, elementEntry.name());
			assertEquals(ServerTypes.ACTIVE, elementEntry.uintValue());
			assertFalse(elementListIt.hasNext()); // End of ElementList
			
			assertFalse(mapIt.hasNext()); // End of Map
			
			/* Market price request message for standby server for the WarmStandbyChannel_4_1 */
			message = providerClient4.popMessage();
			recvReq = (ReqMsg)message;
			
			assertEquals(3, recvReq.streamId());
			assertEquals("DIRECT_FEED1", recvReq.serviceName());
			assertEquals(itemName, recvReq.name());
			assertEquals(2, recvReq.serviceId());
			
			Thread.sleep(1000);
			
			/* Checks the item refresh messages from the standby server of WarmStandbyChannel_4_1 */
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
			assertEquals(serviceListId, refreshMsg.serviceId());
			assertTrue(refreshMsg.hasServiceName());
			assertEquals("SVG1", refreshMsg.serviceName());
			assertEquals(DataTypes.FIELD_LIST, refreshMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_8", channelInfo.channelName());
			assertEquals("Connection_22", channelInfo.sessionChannelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			
			Thread.sleep(1000);
			
			// Stop ommprovider4 to get updates from ommprovider3 from the staring server of WarmStandbyChannel_4_1
			ommprovider4.uninitialize();
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			
			assertEquals(5, statusMsg.streamId());
			assertEquals(DomainTypes.MARKET_PRICE, statusMsg.domainType());
			assertTrue(statusMsg.hasState());
			assertEquals("Open / Suspect / None / 'Service for this item was lost.'", statusMsg.state().toString());
			assertTrue(statusMsg.hasName());
			assertEquals(itemName, statusMsg.name());
			assertTrue(statusMsg.hasServiceId());
			assertEquals(serviceListId, statusMsg.serviceId());
			assertTrue(statusMsg.hasServiceName());
			assertEquals("SVG1", statusMsg.serviceName());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_8", channelInfo.channelName());
			assertEquals("Connection_22", channelInfo.sessionChannelName());
			assertEquals(ChannelInformation.ChannelState.CLOSED, channelInfo.channelState());
			
			Thread.sleep(1000);
			
			message = providerClient3.popMessage();
			
			/* GenericMsg for DIRECT_FEED1 to active server of the WarmStandbyChannel_4_1*/
			genericMsg = (GenericMsg)message;
			assertEquals(2, genericMsg.streamId());
			assertEquals(DomainTypes.SOURCE, genericMsg.domainType());
			assertEquals(EmaRdm.ENAME_CONS_STATUS, genericMsg.name());
			assertEquals(DataTypes.MAP, genericMsg.payload().dataType());
			
			map = genericMsg.payload().map();
			mapIt = map.iterator();
			assertTrue(mapIt.hasNext());
			mapEntry = mapIt.next();
			assertEquals(serviceId2, mapEntry.key().uintValue());
			assertEquals(DataTypes.ELEMENT_LIST, mapEntry.loadType());
			
			elementList = mapEntry.elementList();
			elementListIt = elementList.iterator();
			assertTrue(elementListIt.hasNext());
			elementEntry = elementListIt.next();
			assertEquals(EmaRdm.ENAME_WARMSTANDBY_MODE, elementEntry.name());
			assertEquals(ServerTypes.ACTIVE, elementEntry.uintValue());
			assertFalse(elementListIt.hasNext()); // End of ElementList
			
			assertFalse(mapIt.hasNext()); // End of Map
			
			/* Checks the item unsolicited refresh messages from the active server of WarmStandbyChannel_4_1 */
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
			assertEquals(serviceListId, refreshMsg.serviceId());
			assertTrue(refreshMsg.hasServiceName());
			assertEquals("SVG1", refreshMsg.serviceName());
			assertEquals(DataTypes.FIELD_LIST, refreshMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_7", channelInfo.channelName());
			assertEquals("Connection_22", channelInfo.sessionChannelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			
			// Waits to start fallback for Connection_22 and Connection_23
			Thread.sleep(5000);
			
			// Start the staring server of WarmStandbyChannel_3_1 to fallback to it */
			// Provider_9 provides the DIRECT_FEED and DIRECT_FEED1 service name for WSB channel(starting server)
			ommprovider1 = EmaFactory.createOmmProvider(config.port("19003").providerName("Provider_9"), providerClient);
			assertNotNull(ommprovider1);
			
			Thread.sleep(3000);
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Ok / None / 'preferred host starting fallback'", statusMsg.state().toString());
			channelInfo = consumerClient.popChannelInfo();
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Ok / None / 'preferred host complete'", statusMsg.state().toString());
			channelInfo = consumerClient.popChannelInfo();
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Ok / None / 'preferred host starting fallback'", statusMsg.state().toString());
			channelInfo = consumerClient.popChannelInfo();
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Ok / None / 'session channel down reconnecting'", statusMsg.state().toString());
			
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_7", channelInfo.channelName());
			assertEquals("Connection_22", channelInfo.sessionChannelName());
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
			assertEquals(serviceListId, statusMsg.serviceId());
			assertTrue(statusMsg.hasServiceName());
			assertEquals("SVG1", statusMsg.serviceName());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_7", channelInfo.channelName());
			assertEquals("Connection_22", channelInfo.sessionChannelName());
			assertEquals(ChannelInformation.ChannelState.INACTIVE, channelInfo.channelState());
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Ok / None / 'preferred host complete'", statusMsg.state().toString());
			channelInfo = consumerClient.popChannelInfo();
			
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
			assertEquals("Channel_3", channelInfo.channelName());
			assertEquals("Connection_22", channelInfo.sessionChannelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			
			/* Market price request message for active server for the WarmStandbyChannel_3_2 */
			message = providerClient.popMessage();
			recvReq = (ReqMsg)message;
			
			assertEquals(4, recvReq.streamId());
			assertEquals("DIRECT_FEED1", recvReq.serviceName());
			assertEquals(itemName, recvReq.name());
			assertEquals(2, recvReq.serviceId());
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Ok / None / 'session channel up'", statusMsg.state().toString());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_3", channelInfo.channelName());
			assertEquals("Connection_22", channelInfo.sessionChannelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			
			/* The starting server of WarmStandbyChannel_3_2 receives two generic messages */
			assertEquals(2, providerClient.queueSize());
			
			message = providerClient.popMessage();
			
			/* GenericMsg for DIRECT_FEED to active server of the WarmStandbyChannel_3_2*/
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
			assertEquals(ServerTypes.ACTIVE, elementEntry.uintValue());
			assertFalse(elementListIt.hasNext()); // End of ElementList
			
			assertFalse(mapIt.hasNext()); // End of Map
			
			message = providerClient.popMessage();
			
			/* GenericMsg for DIRECT_FEED1 to active server of the WarmStandbyChannel_3_2*/
			genericMsg = (GenericMsg)message;
			assertEquals(2, genericMsg.streamId());
			assertEquals(DomainTypes.SOURCE, genericMsg.domainType());
			assertEquals(EmaRdm.ENAME_CONS_STATUS, genericMsg.name());
			assertEquals(DataTypes.MAP, genericMsg.payload().dataType());
			
			map = genericMsg.payload().map();
			mapIt = map.iterator();
			assertTrue(mapIt.hasNext());
			mapEntry = mapIt.next();
			assertEquals(serviceId2, mapEntry.key().uintValue());
			assertEquals(DataTypes.ELEMENT_LIST, mapEntry.loadType());
			
			elementList = mapEntry.elementList();
			elementListIt = elementList.iterator();
			assertTrue(elementListIt.hasNext());
			elementEntry = elementListIt.next();
			assertEquals(EmaRdm.ENAME_WARMSTANDBY_MODE, elementEntry.name());
			assertEquals(ServerTypes.STANDBY, elementEntry.uintValue());
			assertFalse(elementListIt.hasNext()); // End of ElementList
			
			assertFalse(mapIt.hasNext()); // End of Map
			
			/* The standby server of WarmStandbyChannel_3_2 receives two generic messages and one request message */
			assertEquals(3, providerClient2.queueSize());
			
			message = providerClient2.popMessage();
			
			/* GenericMsg for DIRECT_FEED to standby server of the WarmStandbyChannel_3_2*/
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
			
			message = providerClient2.popMessage();
			
			/* GenericMsg for DIRECT_FEED1 to active server of the WarmStandbyChannel_3_2*/
			genericMsg = (GenericMsg)message;
			assertEquals(2, genericMsg.streamId());
			assertEquals(DomainTypes.SOURCE, genericMsg.domainType());
			assertEquals(EmaRdm.ENAME_CONS_STATUS, genericMsg.name());
			assertEquals(DataTypes.MAP, genericMsg.payload().dataType());
			
			map = genericMsg.payload().map();
			mapIt = map.iterator();
			assertTrue(mapIt.hasNext());
			mapEntry = mapIt.next();
			assertEquals(serviceId2, mapEntry.key().uintValue());
			assertEquals(DataTypes.ELEMENT_LIST, mapEntry.loadType());
			
			elementList = mapEntry.elementList();
			elementListIt = elementList.iterator();
			assertTrue(elementListIt.hasNext());
			elementEntry = elementListIt.next();
			assertEquals(EmaRdm.ENAME_WARMSTANDBY_MODE, elementEntry.name());
			assertEquals(ServerTypes.ACTIVE, elementEntry.uintValue());
			assertFalse(elementListIt.hasNext()); // End of ElementList
			
			assertFalse(mapIt.hasNext()); // End of Map
			
			/* Market price request message for standby server for the WarmStandbyChannel_3_2 */
			message = providerClient2.popMessage();
			recvReq = (ReqMsg)message;
			
			assertEquals(3, recvReq.streamId());
			assertEquals("DIRECT_FEED1", recvReq.serviceName());
			assertEquals(itemName, recvReq.name());
			assertEquals(2, recvReq.serviceId());
			
			/* Checks the item refresh messages from the standby server of WarmStandbyChannel_3_2 */
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
			assertEquals(serviceListId, refreshMsg.serviceId());
			assertTrue(refreshMsg.hasServiceName());
			assertEquals("SVG1", refreshMsg.serviceName());
			assertEquals(DataTypes.FIELD_LIST, refreshMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_6", channelInfo.channelName());
			assertEquals("Connection_22", channelInfo.sessionChannelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			
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
			
			if(ommprovider1 != null)
				ommprovider1.uninitialize();

			ommprovider2.uninitialize();
			ommprovider3.uninitialize();
			ommprovider4.uninitialize();
			ommprovider5.uninitialize();
		}
	}
	
	@Test
	public void testMultiConnectionsFallBacktoPHWhilePreferredWSBChannelIsConnectingToStandbyServer()
	{
		String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";
		
		OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);
		
		ProviderTestOptions providerTestOptions = new ProviderTestOptions();
		providerTestOptions.supportStandby = true;
		providerTestOptions.sendRefreshAttrib = true;
		providerTestOptions.itemGroupId = ByteBuffer.wrap("10".getBytes());
		
		ProviderTestClient providerClient = new ProviderTestClient(new ProviderTestOptions());
		ProviderTestClient providerClient2 = new ProviderTestClient(providerTestOptions);
		ProviderTestClient providerClient3 = new ProviderTestClient(providerTestOptions);
		ProviderTestClient providerClient4 = new ProviderTestClient(providerTestOptions);
		ProviderTestClient providerClient5 = new ProviderTestClient(providerTestOptions);
		
		// Connection_1_1
		// Channel_1 (preferred Channel name)
		// Provider_9 provides the DIRECT_FEED(RT/TickByTick) and DIRECT_FEED1(RT/JustInTimeConflated) service name for WSB channel(standby server)
		OmmProvider ommprovider1 = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_9"), providerClient);
		assertNotNull(ommprovider1);
		
		// Connection_24
		// WarmStandbyChannel_3 (Preferred WSB Channel name). The preferred service name is not set for it. 
		// Start only the standby server
		// Provider_9 provides the DIRECT_FEED and DIRECT_FEED1 service name for WSB channel(starting server)
		OmmProvider ommprovider3 = EmaFactory.createOmmProvider(config.port("19008").providerName("Provider_9"), providerClient3);
		assertNotNull(ommprovider3);
		
		/* WarmStandbyChannel_8 (Non-preferred WSB Channel name). The preferred service name is not set for it. */
		// Provider_9 provides the DIRECT_FEED and DIRECT_FEED1 service name for WSB channel(starting server)
		OmmProvider ommprovider4 = EmaFactory.createOmmProvider(config.port("19011").providerName("Provider_9"), providerClient4);
		assertNotNull(ommprovider4);
		
		// Provider_9 provides the DIRECT_FEED and DIRECT_FEED1 service name for WSB channel(standby server)
		OmmProvider ommprovider5 = EmaFactory.createOmmProvider(config.port("19012").providerName("Provider_9"), providerClient5);
		assertNotNull(ommprovider5);
		
		
		OmmConsumer consumer = null;
		ConsumerTestOptions consumerOption = new ConsumerTestOptions();
		
		consumerOption.getChannelInformation = true;
		consumerOption.getSessionChannelInfo = false;
		ConsumerTestClient consumerClient = new ConsumerTestClient(consumerOption);
		
		OmmProvider ommprovider2 = null;
		try
		{
			ConsumerTestOptions options = new ConsumerTestOptions();
			options.getChannelInformation = true;
		
			consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_26"), consumerClient);

            boolean ch11_up_found = false;
            boolean ch_1_up_found = false;
            int queueSize = consumerClient.queueSize();
            for (int i = 0; i < queueSize; i++)
            {
                Msg message = consumerClient.popMessage();
                ChannelInformation channelInfo = consumerClient.popChannelInfo();

                if (message instanceof StatusMsg)
                {
                    StatusMsg statusMsg = (StatusMsg)message;
                    assertEquals(1, statusMsg.streamId());
                    assertEquals(DomainTypes.LOGIN, statusMsg.domainType());

                    if (statusMsg.state().toString().equals("Open / Suspect / None / 'session channel down reconnecting'"))
                    {
                        assertEquals("Channel_7", channelInfo.channelName());
                        assertEquals("Connection_24", channelInfo.sessionChannelName());
                        assertEquals(ChannelInformation.ChannelState.INITIALIZING, channelInfo.channelState());
                    }
                    else if (statusMsg.state().toString().equals("Open / Suspect / None / 'session channel up'"))
                    {
                        if ("Channel_1".equals(channelInfo.channelName())) ch_1_up_found = true;
                        else if ("Channel_11".equals(channelInfo.channelName())) ch11_up_found = true;
                        else assertTrue(false);
                        assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
                    }
                }
                else if (message instanceof RefreshMsg)
                {
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
                    assertEquals("Channel_11", channelInfo.channelName());
                    assertEquals("Connection_24", channelInfo.sessionChannelName());
                    assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
                }
            }

            assertTrue(ch_1_up_found);
            assertTrue(ch11_up_found);

			String serviceName = "DIRECT_FEED";
			String itemName = "TRI.N";
			
			ReqMsg reqMsg = EmaFactory.createReqMsg();
			long itemHandle = consumer.registerClient(reqMsg.name(itemName).serviceName(serviceName), consumerClient);
			
			/* Waits for refresh message from Connection_1_1 */
			Thread.sleep(1000);
			
			Msg message = consumerClient.popMessage();

			/* Checks the market price item refresh */
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
			assertEquals("Channel_1", channelInfo.channelName());
			assertEquals("Connection_1_1", channelInfo.sessionChannelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			
			/* Kills Provider1 to close Channel_1 */
			ommprovider1.uninitialize();
			
			Thread.sleep(1000);
			
			message = consumerClient.popMessage();
			StatusMsg statusMsg = (StatusMsg)message;
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Ok / None / 'session channel down reconnecting'", statusMsg.state().toString());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_1", channelInfo.channelName());
			assertEquals("Connection_1_1", channelInfo.sessionChannelName());
			assertEquals(ChannelInformation.ChannelState.CLOSED, channelInfo.channelState());
			
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
			assertEquals("Connection_1_1", channelInfo.sessionChannelName());
			assertEquals(ChannelInformation.ChannelState.CLOSED, channelInfo.channelState());
			
			/* Receives the refresh message from the starting server of  WarmStandbyChannel_8*/
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
			assertEquals("Channel_11", channelInfo.channelName());
			assertEquals("Connection_24", channelInfo.sessionChannelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			
			/* Closes the starting server of WarmStandbyChannel_8 */
			ommprovider4.uninitialize();
			
			Thread.sleep(1000);
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			assertEquals(5, statusMsg.streamId());
			assertEquals(DomainTypes.MARKET_PRICE, statusMsg.domainType());
			assertTrue(statusMsg.hasState());
			assertEquals("Open / Suspect / None / 'Service for this item was lost.'", statusMsg.state().toString());
			assertTrue(statusMsg.hasName());
			assertEquals(itemName, statusMsg.name());
			assertTrue(statusMsg.hasServiceId());
			assertTrue(statusMsg.hasServiceName());
			assertEquals(serviceName, statusMsg.serviceName());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_11", channelInfo.channelName());
			assertEquals("Connection_24", channelInfo.sessionChannelName());
			assertEquals(ChannelInformation.ChannelState.CLOSED, channelInfo.channelState());
			
			/* Receives the unsolicited refresh message from the standby server of WarmStandbyChannel_8 */
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
			assertEquals("Channel_12", channelInfo.channelName());
			assertEquals("Connection_24", channelInfo.sessionChannelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			
			Thread.sleep(1000);
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Ok / None / 'session channel closed'", statusMsg.state().toString());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_1", channelInfo.channelName());
			assertEquals("Connection_1_1", channelInfo.sessionChannelName());
			assertEquals(ChannelInformation.ChannelState.INITIALIZING, channelInfo.channelState());
			
			Thread.sleep(1000);
			
			// WarmStandbyChannel_3 (Preferred WSB Channel name). The preferred service name is not set for it. 
			// Start the starting server to fallback to this preferred group.
			// Provider_9 provides the DIRECT_FEED and DIRECT_FEED1 service name for WSB channel(starting server)
			ommprovider2 = EmaFactory.createOmmProvider(config.port("19007").providerName("Provider_9"), providerClient2);
			assertNotNull(ommprovider2);
			
			System.out.println("The starting server of the preferred WSB is started..");
			Thread.sleep(8000);
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Ok / None / 'preferred host starting fallback'", statusMsg.state().toString());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_12", channelInfo.channelName());
			assertEquals("Connection_24", channelInfo.sessionChannelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());

            boolean ch12OkStatusFound = false;
            boolean ch12SuspectStatusFound = false;
            boolean ch11SuspectStatusFound = false;

            for (int i = 0; i < 3; i++)
            {
                message = consumerClient.popMessage();
                statusMsg = (StatusMsg)message;
                channelInfo = consumerClient.popChannelInfo();

                if (channelInfo.channelName().equals("Channel_12"))
                {
                    assertEquals("Connection_24", channelInfo.sessionChannelName());
                    assertEquals(ChannelInformation.ChannelState.INACTIVE, channelInfo.channelState());
                    if ("Open / Ok / None / 'session channel down reconnecting'".equals(statusMsg.state().toString()))
                    {
                        ch12OkStatusFound = true;
                        assertEquals(1, statusMsg.streamId());
                        assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
                    }
                    else if ("Open / Suspect / None / 'channel down.'".equals(statusMsg.state().toString()))
                    {
                        ch12SuspectStatusFound = true;
                        assertEquals(5, statusMsg.streamId());
                        assertEquals(DomainTypes.MARKET_PRICE, statusMsg.domainType());
                        assertTrue(statusMsg.hasName());
                        assertEquals(itemName, statusMsg.name());
                        assertTrue(statusMsg.hasServiceId());
                        assertTrue(statusMsg.hasServiceName());
                        assertEquals(serviceName, statusMsg.serviceName());
                        assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
                    }
                }
                else if (channelInfo.channelName().equals("Channel_11"))
                {
                    assertEquals(1, statusMsg.streamId());
                    assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
                    assertEquals("Connection_24", channelInfo.sessionChannelName());
                    assertEquals(ChannelInformation.ChannelState.INACTIVE, channelInfo.channelState());
                    assertTrue("Open / Suspect / None / 'session channel down reconnecting'".equals(statusMsg.state().toString()));
                    ch11SuspectStatusFound = true;
                }
            }

            assertTrue(ch12OkStatusFound && ch12SuspectStatusFound && ch11SuspectStatusFound);

            message = consumerClient.popMessage();

            statusMsg = (StatusMsg)message;
            assertEquals(1, statusMsg.streamId());
            assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
            assertEquals("Open / Suspect / None / 'preferred host complete'", statusMsg.state().toString());
            channelInfo = consumerClient.popChannelInfo();
            assertEquals("Channel_7", channelInfo.channelName());
            assertEquals("Connection_24", channelInfo.sessionChannelName());
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
			assertEquals("Connection_24", channelInfo.sessionChannelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Ok / None / 'session channel up'", statusMsg.state().toString());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_7", channelInfo.channelName());
			assertEquals("Connection_24", channelInfo.sessionChannelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			
			/* Receives the refresh message from the starting server of  WarmStandbyChannel_3 */
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
			assertEquals("Connection_24", channelInfo.sessionChannelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			
			consumer.unregister(itemHandle);
		}
		catch (Exception ex)
		{
            ex.printStackTrace();
			assertFalse(true);
		}
		finally
		{
			assertNotNull(consumer);
		
			consumer.uninitialize();
			if (ommprovider1 != null) ommprovider1.uninitialize();
			if (ommprovider2 != null) ommprovider2.uninitialize();
			if (ommprovider3 != null) ommprovider3.uninitialize();
			if (ommprovider4 != null) ommprovider4.uninitialize();
			if (ommprovider5 != null) ommprovider5.uninitialize();
		}
	}
	

	@Test
	public void testMultiConnectionFallBackToPreferredWSBGroupWhenNonPreferredIsDownForLoginBasedWSB()
	{
		String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";

		OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);

		ProviderTestOptions providerTestOptions = new ProviderTestOptions();
		providerTestOptions.supportStandby = true;
		providerTestOptions.sendRefreshAttrib = true;
		providerTestOptions.itemGroupId = ByteBuffer.wrap("10".getBytes());

		ProviderTestClient providerClient = new ProviderTestClient(new ProviderTestOptions());
		ProviderTestClient providerClient2 = new ProviderTestClient(providerTestOptions);
		ProviderTestClient providerClient3 = new ProviderTestClient(providerTestOptions);
		ProviderTestClient providerClient4 = new ProviderTestClient(providerTestOptions);
		ProviderTestClient providerClient5 = new ProviderTestClient(providerTestOptions);

		// Connection_1_1
		// Channel_1 (preferred Channel name)
		// Provider_9 provides the DIRECT_FEED(RT/TickByTick) and DIRECT_FEED1(RT/JustInTimeConflated) service names.
		OmmProvider ommprovider1 = EmaFactory.createOmmProvider(config.port("19001").providerName("Provider_9"), providerClient);
		assertNotNull(ommprovider1);

		// Connection_25
		// WarmStandbyChannel_1 (Preferred WSB Channel name). This is login based WSB channel.
		// Start only the standby server
		// Provider_9 provides the DIRECT_FEED and DIRECT_FEED1 service name for WSB channel(starting server)
		OmmProvider ommprovider3 = EmaFactory.createOmmProvider(config.port("19006").providerName("Provider_9"), providerClient3);
		assertNotNull(ommprovider3);

		/* WarmStandbyChannel_2 (Non-preferred WSB Channel name). This is login based WSB channel. */
		// Provider_9 provides the DIRECT_FEED and DIRECT_FEED1 service name for WSB channel(starting server)
		OmmProvider ommprovider4 = EmaFactory.createOmmProvider(config.port("19007").providerName("Provider_9"), providerClient4);
		assertNotNull(ommprovider4);

		// Provider_9 provides the DIRECT_FEED and DIRECT_FEED1 service name for WSB channel(standby server)
		OmmProvider ommprovider5 = EmaFactory.createOmmProvider(config.port("19008").providerName("Provider_9"), providerClient5);
		assertNotNull(ommprovider5);


		OmmConsumer consumer = null;
		ConsumerTestOptions consumerOption = new ConsumerTestOptions();

		consumerOption.getChannelInformation = true;
		consumerOption.getSessionChannelInfo = false;
		ConsumerTestClient consumerClient = new ConsumerTestClient(consumerOption);

		OmmProvider ommprovider2 = null;

		try
		{
			ConsumerTestOptions options = new ConsumerTestOptions();
			options.getChannelInformation = true;

			consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_31"), consumerClient);

            Thread.sleep(2000);

            int ch3Count = 0;
            int ch7Count = 0;
            int ch1Count = 0;
            boolean foundRefresh = false;
            int count = consumerClient.queueSize();

            for (int i = 0; i < count; i++)
            {
                Msg message = consumerClient.popMessage();
                ChannelInformation channelInfo = consumerClient.popChannelInfo();

                if (message instanceof StatusMsg)
                {
                    StatusMsg statusMsg = (StatusMsg)message;

                    assertEquals(1, statusMsg.streamId());
                    assertEquals(DomainTypes.LOGIN, statusMsg.domainType());

                    if (channelInfo.channelName().equals("Channel_3"))
                    {
                        ch3Count++;
                        assertEquals("Open / Suspect / None / 'session channel down reconnecting'", statusMsg.state().toString());
                        assertEquals("Connection_25", channelInfo.sessionChannelName());
                        assertEquals(ChannelInformation.ChannelState.INITIALIZING, channelInfo.channelState());
                    }
                    else if (channelInfo.channelName().equals("Channel_1"))
                    {
                        ch1Count++;
                        assertEquals("Open / Suspect / None / 'session channel up'", statusMsg.state().toString());
                        assertEquals("Connection_1_1", channelInfo.sessionChannelName());
                        assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
                    }
                    else if (channelInfo.channelName().equals("Channel_7"))
                    {
                        ch7Count++;
                        assertEquals("Open / Suspect / None / 'session channel up'", statusMsg.state().toString());
                        assertEquals("Connection_25", channelInfo.sessionChannelName());
                        assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
                    }
                    else
                        assertFalse(true);
                }
                else if (message instanceof RefreshMsg)
                {
                    foundRefresh = true;
                    RefreshMsg refreshMsg = (RefreshMsg)message;
                    assertEquals(1, refreshMsg.streamId());
                    assertEquals(DomainTypes.LOGIN, refreshMsg.domainType());
                    assertEquals("Open / Ok / None / 'Login accepted'", refreshMsg.state().toString());
                    assertTrue(refreshMsg.solicited());
                    assertTrue(refreshMsg.complete());
                    assertTrue(refreshMsg.hasMsgKey());
                    assertEquals(DataTypes.NO_DATA, refreshMsg.payload().dataType());
                    assertEquals(DataTypes.ELEMENT_LIST, refreshMsg.attrib().dataType());
                    assertEquals("Channel_7", channelInfo.channelName());
                    assertEquals("Connection_25", channelInfo.sessionChannelName());
                    assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
                }
                else
                    assertFalse(true);
            }

			assertEquals(1, ch1Count);
            assertEquals(2, ch3Count);
            assertEquals(1, ch7Count);
			assertTrue(foundRefresh);
			
			String serviceName = "DIRECT_FEED";
			String itemName = "TRI.N";
			
			ReqMsg reqMsg = EmaFactory.createReqMsg();
			long itemHandle = consumer.registerClient(reqMsg.name(itemName).serviceName(serviceName), consumerClient);
			
			/* Waits for refresh message from Connection_1_1 */
			Thread.sleep(1000);
			
			Msg message = consumerClient.popMessage();

			/* Checks the market price item refresh */
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
			assertEquals("Channel_1", channelInfo.channelName());
			assertEquals("Connection_1_1", channelInfo.sessionChannelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			
			/* Kills Provider1 to close Channel_1 */
			ommprovider1.uninitialize();
			
			Thread.sleep(1000);
			
			message = consumerClient.popMessage();
			StatusMsg statusMsg = (StatusMsg)message;
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Ok / None / 'session channel down reconnecting'", statusMsg.state().toString());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_1", channelInfo.channelName());
			assertEquals("Connection_1_1", channelInfo.sessionChannelName());
			assertEquals(ChannelInformation.ChannelState.CLOSED, channelInfo.channelState());
			
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
			assertEquals("Connection_1_1", channelInfo.sessionChannelName());
			assertEquals(ChannelInformation.ChannelState.CLOSED, channelInfo.channelState());
			
			/* Receives the refresh message from the starting server of  WarmStandbyChannel_2*/
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
			assertEquals("Connection_25", channelInfo.sessionChannelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			
			/* Closes the starting server of WarmStandbyChannel_2 */
			ommprovider4.uninitialize();
			
			Thread.sleep(1000);
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			assertEquals(5, statusMsg.streamId());
			assertEquals(DomainTypes.MARKET_PRICE, statusMsg.domainType());
			assertTrue(statusMsg.hasState());
			assertEquals("Open / Suspect / None / 'Service for this item was lost.'", statusMsg.state().toString());
			assertTrue(statusMsg.hasName());
			assertEquals(itemName, statusMsg.name());
			assertTrue(statusMsg.hasServiceId());
			assertTrue(statusMsg.hasServiceName());
			assertEquals(serviceName, statusMsg.serviceName());
			assertEquals(DataTypes.NO_DATA, statusMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_7", channelInfo.channelName());
			assertEquals("Connection_25", channelInfo.sessionChannelName());
			assertEquals(ChannelInformation.ChannelState.CLOSED, channelInfo.channelState());
			
			/* Receives the unsolicited refresh message from the standby server of WarmStandbyChannel_2 */
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
			assertEquals("Connection_25", channelInfo.sessionChannelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			
			Thread.sleep(1000);
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Ok / None / 'session channel closed'", statusMsg.state().toString());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_1", channelInfo.channelName());
			assertEquals("Connection_1_1", channelInfo.sessionChannelName());
			assertEquals(ChannelInformation.ChannelState.INITIALIZING, channelInfo.channelState());
			
			Thread.sleep(1000);
			
			// WarmStandbyChannel_1 (Preferred WSB Channel name). 
			// Start the starting server to fallback to this preferred group.
			// Provider_9 provides the DIRECT_FEED and DIRECT_FEED1 service name for WSB channel(starting server)
			ommprovider2 = EmaFactory.createOmmProvider(config.port("19003").providerName("Provider_9"), providerClient2);
			assertNotNull(ommprovider2);
			
			System.out.println("The starting server of the preferred WSB is started..");
			Thread.sleep(8000);
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Ok / None / 'preferred host starting fallback'", statusMsg.state().toString());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_8", channelInfo.channelName());
			assertEquals("Connection_25", channelInfo.sessionChannelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Ok / None / 'session channel down reconnecting'", statusMsg.state().toString());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_8", channelInfo.channelName());
			assertEquals("Connection_25", channelInfo.sessionChannelName());
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
			assertEquals("Channel_8", channelInfo.channelName());
			assertEquals("Connection_25", channelInfo.sessionChannelName());
			assertEquals(ChannelInformation.ChannelState.INACTIVE, channelInfo.channelState());
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Suspect / None / 'session channel down reconnecting'", statusMsg.state().toString());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_7", channelInfo.channelName());
			assertEquals("Connection_25", channelInfo.sessionChannelName());
			assertEquals(ChannelInformation.ChannelState.INACTIVE, channelInfo.channelState());
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Suspect / None / 'preferred host complete'", statusMsg.state().toString());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_3", channelInfo.channelName());
			assertEquals("Connection_25", channelInfo.sessionChannelName());
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
			assertEquals("Channel_3", channelInfo.channelName());
			assertEquals("Connection_25", channelInfo.sessionChannelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;
			assertEquals(1, statusMsg.streamId());
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals("Open / Ok / None / 'session channel up'", statusMsg.state().toString());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_3", channelInfo.channelName());
			assertEquals("Connection_25", channelInfo.sessionChannelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			
			/* Receives the refresh message from the starting server of  WarmStandbyChannel_1 */
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
			assertEquals("Connection_25", channelInfo.sessionChannelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
			
			consumer.unregister(itemHandle);
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
            if (ommprovider1 != null) ommprovider1.uninitialize();
            if (ommprovider2 != null) ommprovider2.uninitialize();
            if (ommprovider3 != null) ommprovider3.uninitialize();
            if (ommprovider5 != null) ommprovider5.uninitialize();
		}
	}

	@Test
	public void testMultipleConnectionsWSBServiceBased_FallbackWithinGroupWhenPerServiceNameSet_OneItem()
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
			Provider_3 goes down.
			Provider_6 receives Generic message with ACTIVE mode for service DIRECT_FEED1 and sends unsolicited refresh to the Consumer.
			Consumer unregisters item TRI.N.
			Provider_6 receives closed message for TRI.N.
			Provider_3 goes up and receives Generic messages with STANDBY modes for services DIRECT_FEED and DIRECT_FEED1.
			Consumer triggers Preferred Host Fallback.
			Provider_3 receives Generic message with ACTIVE mode for DIRECT_FEED1 and Generic message with STANDBY mode for DIRECT_FEED
			Provider_6 receives Generic message with ACTIVE mode for DIRECT_FEED and Generic message with STANDBY mode for DIRECT_FEED1
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

		/* WarmStandbyChannel_10 (preferred WSB channel, all connections are down) */
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
			consumer  = EmaFactory.createOmmConsumer(EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_28"), consumerClient);

			String itemName = "TRI.N";
			String serviceName2 = "DIRECT_FEED1";
			long DIRECT_FEED_SERVICE_ID = 1; // This is for the DIRECT_FEED service
			long DIRECT_FEED1_SERVICE_ID = 2; // This is for the DIRECT_FEED1 service

			ReqMsg reqMsg = EmaFactory.createReqMsg();

			long itemHandle = consumer.registerClient(reqMsg.name(itemName).serviceName(serviceName2), consumerClient);

			Thread.sleep(2000);

			/* Receives two generic messages and two request messages of active server for the first connection */
			assertEquals(3, providerClient_3.queueSize());

			Msg message = providerClient_3.popMessage();

			/* GenericMsg of active server for the DIRECT_FEED1 service */
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

			message = providerClient_3.popMessage();

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


			// Market price request message for active server for the first connection
			message = providerClient_3.popMessage();
			ReqMsg recvReq = (ReqMsg)message;

			assertEquals(3, recvReq.streamId());
			assertEquals(serviceName2, recvReq.serviceName());
			assertEquals(itemName, recvReq.name());
			assertEquals(2, recvReq.serviceId());

			// Messages for standby server
			assertEquals(3, providerClient_6.queueSize());

			// GenericMsg of standby server for the DIRECT_FEED service name
			message = providerClient_6.popMessage();
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

			// GenericMsg of standby server for the DIRECT_FEED1 service name
			message = providerClient_6.popMessage();
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

			// Market price request message for standby server for the first connection
			message = providerClient_6.popMessage();
			recvReq = (ReqMsg)message;

			assertEquals(3, recvReq.streamId());
			assertEquals(serviceName2, recvReq.serviceName());
			assertEquals(itemName, recvReq.name());
			assertEquals(2, recvReq.serviceId());

			Thread.sleep(1500);

			message = consumerClient.popMessage(); // Status Msg

			StatusMsg statusMsg = (StatusMsg)message;
			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			ChannelInformation channelInfo = consumerClient.popChannelInfo();

			message = consumerClient.popMessage(); // Login Refresh Msg

			RefreshMsg refreshMsg = (RefreshMsg)message;
			assertEquals(DomainTypes.LOGIN, refreshMsg.domainType());
			channelInfo = consumerClient.popChannelInfo();

			// Checks the item refresh message from the starting server of WarmStandbyChannel_9
			message = consumerClient.popMessage();
			// Checks the market price item refresh
			refreshMsg = (RefreshMsg)message;

			assertEquals(5, refreshMsg.streamId());
			assertEquals(DomainTypes.MARKET_PRICE, refreshMsg.domainType());
			assertEquals("Open / Ok / None / 'Refresh Completed'", refreshMsg.state().toString());
			assertTrue(refreshMsg.complete());
			assertTrue(refreshMsg.solicited());
			assertTrue(refreshMsg.hasName());
			assertEquals(itemName, refreshMsg.name());
			assertTrue(refreshMsg.hasServiceId());
			assertEquals(DIRECT_FEED1_SERVICE_ID, refreshMsg.serviceId());
			assertTrue(refreshMsg.hasServiceName());
			assertEquals(serviceName2, refreshMsg.serviceName()); // The active server provides "DIRECT_FEED1" as the active service
			assertEquals(DataTypes.FIELD_LIST, refreshMsg.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_3", channelInfo.channelName());
			assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());

			System.out.println("\n >>>>>>>>>>>>>>>>>>>>>>>>> Closing Provider_3 (active for DIRECT_FEED1) \n");
			// kill Provider_3, Consumer should fallback to Standby server
			ommprovider_3.uninitialize();
			message = providerClient_3.popMessage(); // Close message for provider_3
			providerClient_3.processServiceDown(DIRECT_FEED_SERVICE_ID);
			providerClient_3.processServiceDown(DIRECT_FEED1_SERVICE_ID);

			// Waits for 2.5 seconds for provider_6 to get the Generic message
			Thread.sleep(1500);

			assertEquals(1, providerClient_6.queueSize());

			message = providerClient_6.popMessage();
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

			System.out.println("\n >>>>>>>>>>>>>>>>>>>>>>>> Unregistering item! \n");
			consumer.unregister(itemHandle);

			Thread.sleep(1500);

			message = providerClient_6.popMessage(); // Close message for item TRI.N provider_6

			System.out.println("\n >>>>>>>>>>>>>>>>>>>>>>> Provider_3 is being brought up \n");
			// restart Provider_3 in WarmStandbyChannel_9
			ommprovider_3 = EmaFactory.createOmmProvider(config.port("19003").providerName("Provider_9"), providerClient_3);
			assertNotNull(ommprovider_3);

			Thread.sleep(7000);

			assertEquals(2, providerClient_3.queueSize());
			message = providerClient_3.popMessage();

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
			assertEquals(ServerTypes.STANDBY, elementEntry.uintValue());
			assertFalse(elementListIt.hasNext()); // End of ElementList

			message = providerClient_3.popMessage();

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

			consumerClient.clearQueue();

			System.out.println("\n>>>>>>>>>>>>>>>>>>>>>>>> Consumer triggers PreferredHost Fallback\n");
			consumer.fallbackPreferredHost();

			Thread.sleep(1000);

			assertEquals(1, providerClient_3.queueSize());
			message = providerClient_3.popMessage();

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

			assertEquals(1, providerClient_6.queueSize());
			message = providerClient_6.popMessage();

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

			assertEquals(2, consumerClient.queueSize());

			// PH start fallback message
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;

			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals(1, statusMsg.streamId());
			OmmState state = statusMsg.state();
			assertEquals(OmmState.StreamState.OPEN, state.streamState());
			assertEquals(OmmState.DataState.OK, state.dataState());
			assertTrue(state.statusText().contains("preferred host starting fallback"));

			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;

			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals(1, statusMsg.streamId());
			state = statusMsg.state();
			assertEquals(OmmState.StreamState.OPEN, state.streamState());
			assertEquals(OmmState.DataState.OK, state.dataState());
			assertTrue(state.statusText().contains("preferred host complete"));

			System.out.println("\n >>>>>>>> Shutting sown...");
		}
		catch(OmmException ex)
		{
			assertFalse(true);
		} catch (Exception e) {

			e.printStackTrace();
		}
		finally
		{
			assertNotNull(consumer);

			System.out.println("Uninitializing...");
			consumer.uninitialize();
			ommprovider_3.uninitialize();
			ommprovider_6.uninitialize();
		}

	}

	@Test
	public void testMultipleConnectionsWSBServiceBased_FallbackWithinGroupWhenPerServiceNameSet_ProviderSendsServiceDown()
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

			String serviceName = "DIRECT_FEED";
			String itemName = "TRI.N";
			String serviceName2 = "DIRECT_FEED1";
			String itemName2 = "IBM.N";
			long DIRECT_FEED_SERVICE_ID = 1; // This is for the DIRECT_FEED service
			long DIRECT_FEED1_SERVICE_ID = 2; // This is for the DIRECT_FEED1 service

			ReqMsg reqMsg = EmaFactory.createReqMsg();

			long itemHandle = consumer.registerClient(reqMsg.name(itemName).serviceName(serviceName2), consumerClient);

			Thread.sleep(2000);

			/* Receives two generic messages and two request messages of active server for the first connection */
			assertEquals(3, providerClient_3.queueSize());

			Msg message = providerClient_3.popMessage();

			/* GenericMsg of active server for the DIRECT_FEED1 service */
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

			message = providerClient_3.popMessage();

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


			// Market price request message for active server for the first connection
			message = providerClient_3.popMessage();
			ReqMsg recvReq = (ReqMsg)message;

			assertEquals(3, recvReq.streamId());
			assertEquals(serviceName2, recvReq.serviceName());
			assertEquals(itemName, recvReq.name());
			assertEquals(2, recvReq.serviceId());

			// Messages for standby server
			assertEquals(3, providerClient_6.queueSize());

			// GenericMsg of standby server for the DIRECT_FEED service name
			message = providerClient_6.popMessage();
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

			// GenericMsg of standby server for the DIRECT_FEED1 service name
			message = providerClient_6.popMessage();
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

			// Market price request message for standby server for the first connection
			message = providerClient_6.popMessage();
			recvReq = (ReqMsg)message;

			assertEquals(3, recvReq.streamId());
			assertEquals(serviceName2, recvReq.serviceName());
			assertEquals(itemName, recvReq.name());
			assertEquals(2, recvReq.serviceId());

			Thread.sleep(1500);

			assertEquals(4, consumerClient.queueSize());
			boolean foundItemRefresh = false;
			ChannelInformation channelInfo;
			RefreshMsg refreshMsg;

			for (int i = 0; i < 4; i++)
			{
				message = consumerClient.popMessage();
				if (message instanceof RefreshMsg && message.domainType() != DomainTypes.LOGIN)
				{
					foundItemRefresh = true;
					refreshMsg = (RefreshMsg)message;

					assertEquals(5, refreshMsg.streamId());
					assertEquals(DomainTypes.MARKET_PRICE, refreshMsg.domainType());
					assertEquals("Open / Ok / None / 'Refresh Completed'", refreshMsg.state().toString());
					assertTrue(refreshMsg.complete());
					assertTrue(refreshMsg.solicited());
					assertTrue(refreshMsg.hasName());
					assertEquals(itemName, refreshMsg.name());
					assertTrue(refreshMsg.hasServiceId());
					assertEquals(DIRECT_FEED1_SERVICE_ID, refreshMsg.serviceId());
					assertTrue(refreshMsg.hasServiceName());
					assertEquals(serviceName2, refreshMsg.serviceName()); // The active server provides "DIRECT_FEED1" as the active service
					assertEquals(DataTypes.FIELD_LIST, refreshMsg.payload().dataType());
					channelInfo = consumerClient.popChannelInfo();
					assertEquals("Channel_3", channelInfo.channelName());
					assertEquals(ChannelInformation.ChannelState.ACTIVE, channelInfo.channelState());
				}
				else
				{
					channelInfo = consumerClient.popChannelInfo();
					assertEquals(DomainTypes.LOGIN, message.domainType());
				}
			}

			assertTrue(foundItemRefresh);

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

			System.out.println("\n >>>>>>>>>>>>>>>>>>>>>>> Provider_3 sends ServiceDown... \n");
			// Send service down for the starting server of the first connection.
			ElementList serviceState = EmaFactory.createElementList();
			serviceState.add( EmaFactory.createElementEntry().uintValue( EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_DOWN ));
			serviceState.add( EmaFactory.createElementEntry().uintValue( EmaRdm.ENAME_ACCEPTING_REQS, 0 ));
			serviceState.add( EmaFactory.createElementEntry().state( EmaRdm.ENAME_STATUS, OmmState.StreamState.CLOSED_RECOVER, OmmState.DataState.SUSPECT, OmmState.StatusCode.NONE, ""));
			FilterList filterListEnc = EmaFactory.createFilterList();
			filterListEnc.add( EmaFactory.createFilterEntry().elementList( EmaRdm.SERVICE_STATE_ID, FilterEntry.FilterAction.SET, serviceState ) );
			Map mapEnc2 = EmaFactory.createMap();
			mapEnc2.add( EmaFactory.createMapEntry().keyUInt( DIRECT_FEED1_SERVICE_ID, MapEntry.MapAction.UPDATE, filterListEnc ));
			UpdateMsg updateMsg = EmaFactory.createUpdateMsg();
			ommprovider_3.submit( updateMsg.domainType( EmaRdm.MMT_DIRECTORY ).
					filter( EmaRdm.SERVICE_STATE_FILTER ).
					payload( mapEnc2 ), 0);	// use 0 item handle to fan-out to all subscribers

			// Waits for 2.5 seconds for Provider_6 to get Generic message
			Thread.sleep(2500);

			assertEquals(1, providerClient_6.queueSize());

			message = providerClient_6.popMessage();
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

			assertEquals(1, providerClient_3.queueSize());
			message = providerClient_3.popMessage();
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

			message = consumerClient.popMessage();
			// Checks the market price item refresh
			refreshMsg = (RefreshMsg)message;

			assertEquals(DomainTypes.MARKET_PRICE, refreshMsg.domainType());
			assertEquals("Open / Ok / None / 'Unsolicited Refresh Completed'", refreshMsg.state().toString());
			assertTrue(refreshMsg.complete());
			assertTrue(refreshMsg.hasName());
			assertEquals(itemName, refreshMsg.name());
			assertTrue(refreshMsg.hasServiceId());
			assertEquals(DIRECT_FEED1_SERVICE_ID, refreshMsg.serviceId());
			assertTrue(refreshMsg.hasServiceName());
			assertEquals(serviceName2, refreshMsg.serviceName()); // The active server provides "DIRECT_FEED1" as the active service
			assertEquals(DataTypes.FIELD_LIST, refreshMsg.payload().dataType());

			System.out.println("\n >>>>>>>>>>>>>>>>>>> Consumer triggers PreferredHost Fallback... \n");
			// Trigger fallback to preferred host
			consumer.fallbackPreferredHost();

			Thread.sleep(4000);

			assertEquals(2, consumerClient.queueSize());

			// PH start fallback message
			message = consumerClient.popMessage();
			StatusMsg statusMsg = (StatusMsg)message;

			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals(1, statusMsg.streamId());
			OmmState state = statusMsg.state();
			assertEquals(OmmState.StreamState.OPEN, state.streamState());
			assertEquals(OmmState.DataState.OK, state.dataState());
			assertTrue(state.statusText().contains("preferred host starting fallback"));

			// PH fallback complete event
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;

			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals(1, statusMsg.streamId());
			state = statusMsg.state();
			assertEquals(OmmState.StreamState.OPEN, state.streamState());
			assertEquals(OmmState.DataState.OK, state.dataState());
			assertTrue(state.statusText().contains("preferred host complete"));

			assertEquals(0, providerClient_6.queueSize()); // Gets nothing
			assertEquals(0, providerClient_3.queueSize()); // Gets nothing

			System.out.println("\n >>>>>>>>>>>>>>>>>>>>>>>>>>>>>> Provider_3 sends ServiceUp for DIRECT_FEED1... \n");
			// Send service down for the starting server of the first connection.
			serviceState.clear();
			serviceState.add( EmaFactory.createElementEntry().uintValue( EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_UP ));
			serviceState.add( EmaFactory.createElementEntry().uintValue( EmaRdm.ENAME_ACCEPTING_REQS, 1 ));
			serviceState.add( EmaFactory.createElementEntry().state( EmaRdm.ENAME_STATUS, OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, ""));
			filterListEnc.clear();
			filterListEnc.add( EmaFactory.createFilterEntry().elementList( EmaRdm.SERVICE_STATE_ID, FilterEntry.FilterAction.SET, serviceState ) );
			mapEnc2.clear();
			mapEnc2.add( EmaFactory.createMapEntry().keyUInt( DIRECT_FEED1_SERVICE_ID, MapEntry.MapAction.UPDATE, filterListEnc ));
			updateMsg.clear();
			ommprovider_3.submit( updateMsg.domainType( EmaRdm.MMT_DIRECTORY ).
					filter( EmaRdm.SERVICE_STATE_FILTER ).
					payload( mapEnc2 ), 0);	// use 0 item handle to fan-out to all subscribers

			Thread.sleep(4000);

			assertEquals(1, providerClient_3.queueSize()); // Gets item request, sends refresh without payload
			message = providerClient_3.popMessage();
			recvReq = (ReqMsg)message;

			assertEquals(4, recvReq.streamId()); // streamId increased from 3 to 4
			assertEquals(serviceName2, recvReq.serviceName());
			assertEquals(itemName, recvReq.name());
			assertEquals(DIRECT_FEED1_SERVICE_ID, recvReq.serviceId());

			System.out.println("\n >>>>>>>>>>>>>>>>>>>>>>>>>>  Consumer triggers PreferredHost Fallback again... \n");
			// Trigger fallback to preferred host
			consumer.fallbackPreferredHost(); // nothing happens here at all

			Thread.sleep(4000);

			assertEquals(1, providerClient_3.queueSize());

			message = providerClient_3.popMessage();
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

			assertEquals(1, providerClient_6.queueSize());

			message = providerClient_6.popMessage();
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

			assertEquals(3, consumerClient.queueSize());

			// PH start fallback message
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;

			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals(1, statusMsg.streamId());
			state = statusMsg.state();
			assertEquals(OmmState.StreamState.OPEN, state.streamState());
			assertEquals(OmmState.DataState.OK, state.dataState());
			assertTrue(state.statusText().contains("preferred host starting fallback"));

			// PH fallback complete event
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;

			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals(1, statusMsg.streamId());
			state = statusMsg.state();
			assertEquals(OmmState.StreamState.OPEN, state.streamState());
			assertEquals(OmmState.DataState.OK, state.dataState());
			assertTrue(state.statusText().contains("preferred host complete"));

			message = consumerClient.popMessage();
			refreshMsg = (RefreshMsg)message;

			assertEquals(DomainTypes.MARKET_PRICE, refreshMsg.domainType());
			assertEquals("Open / Ok / None / 'Unsolicited Refresh Completed'", refreshMsg.state().toString());
			assertTrue(refreshMsg.complete());
			assertTrue(refreshMsg.hasName());
			assertEquals(itemName, refreshMsg.name());
			assertTrue(refreshMsg.hasServiceId());
			assertEquals(DIRECT_FEED1_SERVICE_ID, refreshMsg.serviceId());
			assertTrue(refreshMsg.hasServiceName());
			assertEquals(serviceName2, refreshMsg.serviceName()); // The active server provides "DIRECT_FEED1" as the active service
			assertEquals(DataTypes.FIELD_LIST, refreshMsg.payload().dataType());

			long itemHandle2 = consumer.registerClient(reqMsg.name("ABC.N").serviceName(serviceName), consumerClient);
			long itemHandle3 = consumer.registerClient(reqMsg.name("DEF.N").serviceName(serviceName2), consumerClient);

			Thread.sleep(4000);

			assertEquals(2, providerClient_3.queueSize());

			message = providerClient_3.popMessage();
			recvReq = (ReqMsg)message;

			assertEquals(serviceName, recvReq.serviceName());
			assertEquals("ABC.N", recvReq.name());
			assertEquals(DIRECT_FEED_SERVICE_ID, recvReq.serviceId());

			message = providerClient_3.popMessage();
			recvReq = (ReqMsg)message;

			assertEquals(serviceName2, recvReq.serviceName());
			assertEquals("DEF.N", recvReq.name());
			assertEquals(DIRECT_FEED1_SERVICE_ID, recvReq.serviceId());

			assertEquals(2, providerClient_6.queueSize());

			message = providerClient_6.popMessage();
			recvReq = (ReqMsg)message;

			//assertEquals(4, recvReq.streamId()); // streamId increased from 3 to 4
			assertEquals(serviceName, recvReq.serviceName());
			assertEquals("ABC.N", recvReq.name());
			assertEquals(DIRECT_FEED_SERVICE_ID, recvReq.serviceId());

			message = providerClient_6.popMessage();
			recvReq = (ReqMsg)message;

			//assertEquals(5, recvReq.streamId()); // streamId increased from 3 to 4
			assertEquals(serviceName2, recvReq.serviceName());
			assertEquals("DEF.N", recvReq.name());
			assertEquals(DIRECT_FEED1_SERVICE_ID, recvReq.serviceId());

			System.out.println("Shutting down...");
			/* So in the end Provider_6 (initially standby server with DIRECT_FEED marked by the consumer as active)
			   gets hints that it has both DIRECT_FEED and DIRECT_FEED1 active, and it sends filled refresh messages for both services */
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
	public void testMultipleConnectionsWSBServiceBased_FallbackWithinGroupWhenPerServiceNameSet_NonOverlappingServices()
	{
		String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";

		OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);

		ProviderTestOptions providerTestOptions = new ProviderTestOptions();
		providerTestOptions.supportStandby = true;
		providerTestOptions.sendRefreshAttrib = true;
		providerTestOptions.itemGroupId = ByteBuffer.wrap("10".getBytes());

		ProviderTestClient providerClient_6 = new ProviderTestClient(providerTestOptions);
		providerClient_6.name = "Provider_6";
		ProviderTestClient providerClient_10 = new ProviderTestClient(providerTestOptions);
		providerClient_10.name = "Provider_10";

		/* WarmStandbyChannel_11 goes up*/

		// Provider_9 provides the DIRECT_FEED and DIRECT_FEED1 service name for WSB channel(standby server)
		OmmProvider ommprovider_6 = EmaFactory.createOmmProvider(config.port("19006").providerName("Provider_9"), providerClient_6);
		assertNotNull(ommprovider_6);

		OmmProvider ommprovider_10 = EmaFactory.createOmmProvider(config.port("19010").providerName("Provider_10"), providerClient_10);
		assertNotNull(ommprovider_10);

		OmmConsumer consumer = null;
		ConsumerTestOptions consumerOption = new ConsumerTestOptions();

		consumerOption.getChannelInformation = true;
		consumerOption.getSessionChannelInfo = false;
		ConsumerTestClient consumerClient = new ConsumerTestClient(consumerOption);

		try
		{
			consumer  = EmaFactory.createOmmConsumer(EmaFactory
					.createOmmConsumerConfig(emaConfigFileLocation)
					.consumerName("Consumer_29"), consumerClient);

			String directFeedName = "DIRECT_FEED";
			String directFeed1Name = "DIRECT_FEED1";
			String directFeed_2_Name = "DIRECT_FEED_2";
			String directFeed_3_Name = "DIRECT_FEED_3";
			String itemName = "TRI.N";
			String itemName2 = "IBM.N";
			long DIRECT_FEED_SERVICE_ID = 1;
			long DIRECT_FEED1_SERVICE_ID = 2;
			long DIRECT_FEED_2_SERVICE_ID = 14;
			long DIRECT_FEED_3_SERVICE_ID = 15;

			Thread.sleep(3000);

			consumerClient.clearQueue();

			assertEquals(2, providerClient_6.queueSize());

			Msg message = providerClient_6.popMessage();

			// GenericMsg of active server for the DIRECT_FEED service
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
			assertEquals(ServerTypes.ACTIVE, elementEntry.uintValue());
			assertFalse(elementListIt.hasNext()); // End of ElementList

			assertFalse(mapIt.hasNext()); // End of Map

			message = providerClient_6.popMessage();

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

			assertEquals(2, providerClient_10.queueSize());

			message = providerClient_10.popMessage();

			// GenericMsg of active server for the DIRECT_FEED service
			genericMsg = (GenericMsg)message;
			assertEquals(2, genericMsg.streamId());
			assertEquals(DomainTypes.SOURCE, genericMsg.domainType());
			assertEquals(EmaRdm.ENAME_CONS_STATUS, genericMsg.name());
			assertEquals(DataTypes.MAP, genericMsg.payload().dataType());

			map = genericMsg.payload().map();
			mapIt = map.iterator();
			assertTrue(mapIt.hasNext());
			mapEntry = mapIt.next();
			assertEquals(DIRECT_FEED_2_SERVICE_ID, mapEntry.key().uintValue());
			assertEquals(DataTypes.ELEMENT_LIST, mapEntry.loadType());

			elementList = mapEntry.elementList();
			elementListIt = elementList.iterator();
			assertTrue(elementListIt.hasNext());
			elementEntry = elementListIt.next();
			assertEquals(EmaRdm.ENAME_WARMSTANDBY_MODE, elementEntry.name());
			assertEquals(ServerTypes.ACTIVE, elementEntry.uintValue());
			assertFalse(elementListIt.hasNext()); // End of ElementList

			assertFalse(mapIt.hasNext()); // End of Map

			message = providerClient_10.popMessage();

			genericMsg = (GenericMsg)message;
			assertEquals(2, genericMsg.streamId());
			assertEquals(DomainTypes.SOURCE, genericMsg.domainType());
			assertEquals(EmaRdm.ENAME_CONS_STATUS, genericMsg.name());
			assertEquals(DataTypes.MAP, genericMsg.payload().dataType());

			map = genericMsg.payload().map();
			mapIt = map.iterator();
			assertTrue(mapIt.hasNext());
			mapEntry = mapIt.next();
			assertEquals(DIRECT_FEED_3_SERVICE_ID, mapEntry.key().uintValue());
			assertEquals(DataTypes.ELEMENT_LIST, mapEntry.loadType());

			elementList = mapEntry.elementList();
			elementListIt = elementList.iterator();
			assertTrue(elementListIt.hasNext());
			elementEntry = elementListIt.next();
			assertEquals(EmaRdm.ENAME_WARMSTANDBY_MODE, elementEntry.name());
			assertEquals(ServerTypes.ACTIVE, elementEntry.uintValue());
			assertFalse(elementListIt.hasNext()); // End of ElementList

			ReqMsg reqMsg = EmaFactory.createReqMsg();

			long itemHandle = consumer.registerClient(reqMsg.name(itemName).serviceName(directFeed1Name), consumerClient);
			long itemHandle2 = consumer.registerClient(reqMsg.name(itemName).serviceName(directFeed_2_Name), consumerClient);

			Thread.sleep(2000);

			assertEquals(1, providerClient_6.queueSize());
			assertEquals(1, providerClient_10.queueSize());

			message = providerClient_6.popMessage();

			ReqMsg recvReq = (ReqMsg)message;

			assertEquals(directFeed1Name, recvReq.serviceName());
			assertEquals(itemName, recvReq.name());
			assertEquals(2, recvReq.serviceId());

			message = providerClient_10.popMessage();

			recvReq = (ReqMsg)message;

			assertEquals(directFeed_2_Name, recvReq.serviceName());
			assertEquals(itemName, recvReq.name());
			assertEquals(DIRECT_FEED_2_SERVICE_ID, recvReq.serviceId());

			assertEquals(2, consumerClient.queueSize());

			HashSet<String> serviceNames = new HashSet<>();
			for (int i = 0; i < 2; i++)
			{
				RefreshMsg refresh = (RefreshMsg)consumerClient.popMessage();
				assertEquals(DomainTypes.MARKET_PRICE, refresh.domainType());
				assertEquals("Open / Ok / None / 'Refresh Completed'", refresh.state().toString());
				assertTrue(refresh.complete());
				assertTrue(refresh.solicited());
				assertTrue(refresh.hasName());
				assertTrue(refresh.hasServiceName());
				assertTrue(directFeed1Name.equals(refresh.serviceName()) || directFeed_2_Name.equals(refresh.serviceName()));
				serviceNames.add(refresh.serviceName());
				assertEquals(DataTypes.FIELD_LIST, refresh.payload().dataType());
			}
			assertEquals(2, serviceNames.size());

			System.out.println("\n>>>>>>>>>>>>>>>>> Provider_6 goes down");
			ommprovider_6.uninitialize();

			Thread.sleep(2000);

			// Provider_10 does not provide any of Provider_6 services, so should get nothing
			assertEquals(0, providerClient_10.queueSize());

			System.out.println("\n >>>>>>>>> Triggering Preferred host fallback for the first time...");
			consumer.fallbackPreferredHost();

			Thread.sleep(2000);

			// Provider_10 does not provide any of Provider_6 services, so it should again get nothing
			assertEquals(0, providerClient_10.queueSize());

			assertEquals(2, consumerClient.queueSize());

			// PH start fallback message
			message = consumerClient.popMessage();
			StatusMsg statusMsg = (StatusMsg)message;

			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals(1, statusMsg.streamId());
			OmmState state = statusMsg.state();
			assertEquals(OmmState.StreamState.OPEN, state.streamState());
			assertEquals(OmmState.DataState.OK, state.dataState());
			assertTrue(state.statusText().contains("preferred host starting fallback"));

			// PH fallback complete event
			message = consumerClient.popMessage();
			statusMsg = (StatusMsg)message;

			assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
			assertEquals(1, statusMsg.streamId());
			state = statusMsg.state();
			assertEquals(OmmState.StreamState.OPEN, state.streamState());
			assertEquals(OmmState.DataState.OK, state.dataState());
			assertTrue(state.statusText().contains("preferred host complete"));

			System.out.println("\n>>>>>>>>>>>>>>>>> Provider_6 goes up again...");
			providerClient_6.clearQueue();
			ommprovider_6 = EmaFactory.createOmmProvider(config.port("19006").providerName("Provider_9"), providerClient_6);
			assertNotNull(ommprovider_6);

			Thread.sleep(2000);

			assertEquals(3, providerClient_6.queueSize());
			message = providerClient_6.popMessage();

			reqMsg = (ReqMsg)message;
			assertEquals(directFeed1Name, reqMsg.serviceName());
			assertEquals(itemName, reqMsg.name());
			assertEquals(2, reqMsg.serviceId());

			message = providerClient_6.popMessage();

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

			message = providerClient_6.popMessage();

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

			//Consumer gets Refresh from Provider_6
			assertEquals(1, consumerClient.queueSize());

			RefreshMsg refresh = (RefreshMsg)consumerClient.popMessage();
			assertEquals(DomainTypes.MARKET_PRICE, refresh.domainType());
			assertEquals("Open / Ok / None / 'Refresh Completed'", refresh.state().toString());
			assertTrue(refresh.complete());
			assertTrue(refresh.solicited());
			assertTrue(refresh.hasName());
			assertTrue(refresh.hasServiceName());
			assertTrue(directFeed1Name.equals(refresh.serviceName()) || directFeed_2_Name.equals(refresh.serviceName()));
			serviceNames.add(refresh.serviceName());
			assertEquals(DataTypes.FIELD_LIST, refresh.payload().dataType());

			// Provider_10 does not provide any of Provider_6 services, so it should again get nothing
			assertEquals(0, providerClient_10.queueSize());

			System.out.println("\n >>>>>>>>>> Triggering PH fallback again...");
			consumer.fallbackPreferredHost();

			Thread.sleep(3000);

			assertEquals(0, providerClient_6.queueSize());
			assertEquals(0, providerClient_10.queueSize());

			assertEquals(2, consumerClient.queueSize()); // PH Start and Complete messages
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
			ommprovider_6.uninitialize();
			ommprovider_10.uninitialize();
		}
	}


	@Test // ETA-RTSDK2.2.1-42309
	public void testMultipleConnectionsRepeatedServers()
	{
		String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";

		OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);

		ProviderTestOptions providerTestOptions = new ProviderTestOptions();
		providerTestOptions.supportStandby = true;
		providerTestOptions.sendRefreshAttrib = true;
		providerTestOptions.itemGroupId = ByteBuffer.wrap("10".getBytes());

		ProviderTestClient providerClient_6 = new ProviderTestClient(providerTestOptions);
		providerClient_6.name = "Provider_6";
		ProviderTestClient providerClient_10 = new ProviderTestClient(providerTestOptions);
		providerClient_10.name = "Provider_10";
		ProviderTestClient providerClient_3 = new ProviderTestClient(providerTestOptions);
		providerClient_3.name = "Provider_3";
		ProviderTestClient providerClient_12 = new ProviderTestClient(providerTestOptions);
		providerClient_12.name = "Provider_12";

		//WSB-G2 is up

		System.out.println("\n>>>>>>>>> S2 and S4 (G2) go up, other servers are down.");
		// S2 Service A (DIRECT_FEED)
		OmmProvider ommprovider_6 = EmaFactory.createOmmProvider(config.port("19006").providerName("Provider_5"), providerClient_6);
		assertNotNull(ommprovider_6);

		// S4 Service B (DIRECT_FEED_2)
		OmmProvider ommprovider_12 = EmaFactory.createOmmProvider(config.port("19012").providerName("Provider_11"), providerClient_12);
		assertNotNull(ommprovider_12);

		OmmProvider ommprovider_3 = null;

		OmmConsumer consumer = null;
		ConsumerTestOptions consumerOption = new ConsumerTestOptions();

		consumerOption.getChannelInformation = true;
		consumerOption.getSessionChannelInfo = false;
		ConsumerTestClient consumerClient = new ConsumerTestClient(consumerOption);

		try
		{
			ServiceList serviceList = EmaFactory.createServiceList("SVG1");

			serviceList.concreteServiceList().add("DIRECT_FEED");
			serviceList.concreteServiceList().add("DIRECT_FEED_2");

			consumer  = EmaFactory.createOmmConsumer(EmaFactory
					.createOmmConsumerConfig(emaConfigFileLocation)
					.consumerName("Consumer_35_1").addServiceList(serviceList), consumerClient);

			Thread.sleep(2000);

			consumerClient.clearQueue();

			Msg message = providerClient_6.popMessage();

			// GenericMsg of active server for the DIRECT_FEED service
			GenericMsg genericMsg = (GenericMsg)message;
			assertEquals(2, genericMsg.streamId());
			assertEquals(DomainTypes.SOURCE, genericMsg.domainType());
			assertEquals(EmaRdm.ENAME_CONS_STATUS, genericMsg.name());
			assertEquals(DataTypes.MAP, genericMsg.payload().dataType());

			Map map = genericMsg.payload().map();
			Iterator<MapEntry> mapIt = map.iterator();
			assertTrue(mapIt.hasNext());
			MapEntry mapEntry = mapIt.next();
			assertEquals(1, mapEntry.key().uintValue());
			assertEquals(DataTypes.ELEMENT_LIST, mapEntry.loadType());

			ElementList elementList = mapEntry.elementList();
			Iterator<ElementEntry> elementListIt = elementList.iterator();
			assertTrue(elementListIt.hasNext());
			ElementEntry elementEntry = elementListIt.next();
			assertEquals(EmaRdm.ENAME_WARMSTANDBY_MODE, elementEntry.name());
			assertEquals(ServerTypes.ACTIVE, elementEntry.uintValue());
			assertFalse(elementListIt.hasNext()); // End of ElementList

			assertFalse(mapIt.hasNext()); // End of Map

			message = providerClient_12.popMessage();

			genericMsg = (GenericMsg)message;
			assertEquals(2, genericMsg.streamId());
			assertEquals(DomainTypes.SOURCE, genericMsg.domainType());
			assertEquals(EmaRdm.ENAME_CONS_STATUS, genericMsg.name());
			assertEquals(DataTypes.MAP, genericMsg.payload().dataType());

			map = genericMsg.payload().map();
			mapIt = map.iterator();
			assertTrue(mapIt.hasNext());
			mapEntry = mapIt.next();
			assertEquals(2, mapEntry.key().uintValue());
			assertEquals(DataTypes.ELEMENT_LIST, mapEntry.loadType());

			elementList = mapEntry.elementList();
			elementListIt = elementList.iterator();
			assertTrue(elementListIt.hasNext());
			elementEntry = elementListIt.next();
			assertEquals(EmaRdm.ENAME_WARMSTANDBY_MODE, elementEntry.name());
			assertEquals(ServerTypes.ACTIVE, elementEntry.uintValue());
			assertFalse(elementListIt.hasNext()); // End of ElementList

			ReqMsg reqMsg = EmaFactory.createReqMsg();

			long itemHandle = consumer.registerClient(reqMsg.name("TRI.N").serviceName("DIRECT_FEED"), consumerClient);

			Thread.sleep(1000);

			assertEquals(1, providerClient_6.queueSize());

			message = providerClient_6.popMessage();

			reqMsg = (ReqMsg)message;
			assertEquals("DIRECT_FEED", reqMsg.serviceName());
			assertEquals("TRI.N", reqMsg.name());
			assertEquals(1, reqMsg.serviceId());

			assertEquals(1, consumerClient.queueSize());

			RefreshMsg refresh = (RefreshMsg)consumerClient.popMessage();
			assertEquals(DomainTypes.MARKET_PRICE, refresh.domainType());
			assertEquals("Open / Ok / None / 'Refresh Completed'", refresh.state().toString());
			assertTrue(refresh.complete());
			assertTrue(refresh.solicited());
			assertTrue(refresh.hasName());
			assertTrue(refresh.hasServiceName());
			assertTrue("DIRECT_FEED".equals(refresh.serviceName()));
			assertEquals(DataTypes.FIELD_LIST, refresh.payload().dataType());

			Thread.sleep(8000);

			System.out.println("\n>>>>>>>>>> S3 (Service A, G1*) goes up!");
			// S3 Service A (DIRECT_FEED)
			ommprovider_3 = EmaFactory.createOmmProvider(config.port("19003").providerName("Provider_5"), providerClient_3);
			assertNotNull(ommprovider_3);

			System.out.println("\n>>>>>>>>> Consumer triggers fallback to preferred host!");
			consumer.fallbackPreferredHost();

			Thread.sleep(3000);

			boolean foundStartFallback = false;
			boolean foundCompleteFallback = false;

            int count = consumerClient.queueSize();

			for (int i = 0; i < count; i++)
			{
				message = consumerClient.popMessage();
				if (message instanceof StatusMsg)
				{
					StatusMsg statusMsg = (StatusMsg)message;
					OmmState state = statusMsg.state();
					if (state.statusText().contains("preferred host complete")) foundCompleteFallback = true;
					else if (state.statusText().contains("preferred host starting fallback")) foundStartFallback = true;
				}
				else if (message instanceof RefreshMsg)
				{
					if (message.domainType() != DomainTypes.LOGIN)
					{
						refresh = (RefreshMsg)message;
						assertEquals(DomainTypes.MARKET_PRICE, refresh.domainType());
						assertEquals("Open / Ok / None / 'Refresh Completed'", refresh.state().toString());
						assertTrue(refresh.complete());
						assertTrue(refresh.solicited());
						assertTrue(refresh.hasName());
						assertTrue(refresh.hasServiceName());
						assertTrue("DIRECT_FEED".equals(refresh.serviceName()));
						assertEquals(DataTypes.FIELD_LIST, refresh.payload().dataType());
					}
				}
			}

			assertTrue(foundStartFallback);
			assertTrue(foundCompleteFallback);
		}
		catch (Exception e)
		{
			e.printStackTrace();
			assertTrue(false);
		}
		finally
		{
			consumer.uninitialize();

			if (ommprovider_3 != null) ommprovider_3.uninitialize();
			if (ommprovider_6 != null) ommprovider_6.uninitialize();
			if (ommprovider_12 != null) ommprovider_12.uninitialize();
		}
	}

	@Test //RTSDK2.2.1-42226
	public void testMultipleGroupSwitchesWSBLoginBased()
	{
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

		//WSB-G0 is up
		System.out.println("\n>>>>>>>>>S1 and S2 (G0) go up, other servers are down.");

		OmmProvider ommprovider_3 = EmaFactory.createOmmProvider(config.port("19003").providerName("Provider_9"), providerClient_3);
		assertNotNull(ommprovider_3);

		OmmProvider ommprovider_6 = EmaFactory.createOmmProvider(config.port("19006").providerName("Provider_9"), providerClient_6);
		assertNotNull(ommprovider_6);

		// WSB-G1 is down
		OmmProvider ommprovider_7 = null;
		OmmProvider ommprovider_8 = null;

		OmmConsumer consumer = null;
		ConsumerTestOptions consumerOption = new ConsumerTestOptions();

		consumerOption.getChannelInformation = true;
		consumerOption.getSessionChannelInfo = false;
		ConsumerTestClient consumerClient = new ConsumerTestClient(consumerOption);

		try
		{
			consumer = EmaFactory.createOmmConsumer(EmaFactory
					.createOmmConsumerConfig(emaConfigFileLocation)
					.consumerName("Consumer_35_3"), consumerClient);

			Thread.sleep(2000);

			consumerClient.clearQueue();
            while (consumerClient.channelInfoSize() > 0) consumerClient.popChannelInfo();

			assertEquals(1, providerClient_3.queueSize());

			Msg message = providerClient_3.popMessage();

			GenericMsg genericMsg = (GenericMsg)message;
			assertEquals(1, genericMsg.streamId());
			assertEquals(DomainTypes.LOGIN, genericMsg.domainType());
			assertEquals(EmaRdm.ENAME_CONS_CONN_STATUS, genericMsg.name());
			assertEquals(DataTypes.MAP, genericMsg.payload().dataType());

			Map map = genericMsg.payload().map();
			Iterator<MapEntry> mapIt = map.iterator();
			assertTrue(mapIt.hasNext());
			MapEntry mapEntry = mapIt.next();
			assertTrue("WarmStandbyInfo".equals(mapEntry.key().ascii().toString()));
			assertEquals(DataTypes.ELEMENT_LIST, mapEntry.loadType());

			ElementList elementList = mapEntry.elementList();
			Iterator<ElementEntry> elementListIt = elementList.iterator();
			assertTrue(elementListIt.hasNext());
			ElementEntry elementEntry = elementListIt.next();
			assertEquals(EmaRdm.ENAME_WARMSTANDBY_MODE, elementEntry.name());
			assertEquals(ServerTypes.ACTIVE, elementEntry.uintValue());
			assertFalse(elementListIt.hasNext()); // End of ElementList

			assertFalse(mapIt.hasNext()); // End of Map

			assertEquals(1, providerClient_6.queueSize());

			message = providerClient_6.popMessage();

			// GenericMsg of active server for the DIRECT_FEED service
			genericMsg = (GenericMsg)message;
			assertEquals(1, genericMsg.streamId());
			assertEquals(DomainTypes.LOGIN, genericMsg.domainType());
			assertEquals(EmaRdm.ENAME_CONS_CONN_STATUS, genericMsg.name());
			assertEquals(DataTypes.MAP, genericMsg.payload().dataType());

			map = genericMsg.payload().map();
			mapIt = map.iterator();
			assertTrue(mapIt.hasNext());
			mapEntry = mapIt.next();
			assertTrue("WarmStandbyInfo".equals(mapEntry.key().ascii().toString()));
			assertEquals(DataTypes.ELEMENT_LIST, mapEntry.loadType());

			elementList = mapEntry.elementList();
			elementListIt = elementList.iterator();
			assertTrue(elementListIt.hasNext());
			elementEntry = elementListIt.next();
			assertEquals(EmaRdm.ENAME_WARMSTANDBY_MODE, elementEntry.name());
			assertEquals(ServerTypes.STANDBY, elementEntry.uintValue());
			assertFalse(elementListIt.hasNext()); // End of ElementList

			assertFalse(mapIt.hasNext()); // End of Map

            while (consumerClient.channelInfoSize() > 0) consumerClient.popChannelInfo();

			ReqMsg reqMsg = EmaFactory.createReqMsg();

			reqMsg.clear();

			long itemHandle = consumer.registerClient(reqMsg.name("TRI.N").serviceName("DIRECT_FEED"), consumerClient);
			long itemHandle1 = consumer.registerClient(reqMsg.name("IBM.N").serviceName("DIRECT_FEED1"), consumerClient);

			Thread.sleep(2000);

			message = providerClient_3.popMessage();

			reqMsg = (ReqMsg)message;
			assertEquals("DIRECT_FEED", reqMsg.serviceName());
			assertEquals("TRI.N", reqMsg.name());
			assertEquals(1, reqMsg.serviceId());

			message = providerClient_3.popMessage();

			reqMsg = (ReqMsg)message;
			assertEquals("DIRECT_FEED1", reqMsg.serviceName());
			assertEquals("IBM.N", reqMsg.name());
			assertEquals(2, reqMsg.serviceId());

			message = providerClient_6.popMessage();

			reqMsg = (ReqMsg)message;
			assertEquals("DIRECT_FEED", reqMsg.serviceName());
			assertEquals("TRI.N", reqMsg.name());
			assertEquals(1, reqMsg.serviceId());

			message = providerClient_6.popMessage();

			reqMsg = (ReqMsg)message;
			assertEquals("DIRECT_FEED1", reqMsg.serviceName());
			assertEquals("IBM.N", reqMsg.name());
			assertEquals(2, reqMsg.serviceId());

			assertEquals(2, consumerClient.queueSize());

			RefreshMsg refresh = (RefreshMsg)consumerClient.popMessage();
			assertEquals(DomainTypes.MARKET_PRICE, refresh.domainType());
			assertEquals("Open / Ok / None / 'Refresh Completed'", refresh.state().toString());
			assertTrue("TRI.N".equals(refresh.name()));
			assertTrue(refresh.complete());
			assertTrue(refresh.solicited());
			assertTrue(refresh.hasName());
			assertTrue(refresh.hasServiceName());
			assertTrue("DIRECT_FEED".equals(refresh.serviceName()));
			assertEquals(DataTypes.FIELD_LIST, refresh.payload().dataType());
			ChannelInformation channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_3", channelInfo.channelName());

			refresh = (RefreshMsg)consumerClient.popMessage();
			assertEquals(DomainTypes.MARKET_PRICE, refresh.domainType());
			assertEquals("Open / Ok / None / 'Refresh Completed'", refresh.state().toString());
			assertTrue("IBM.N".equals(refresh.name()));
			assertTrue(refresh.complete());
			assertTrue(refresh.solicited());
			assertTrue(refresh.hasName());
			assertTrue(refresh.hasServiceName());
			assertTrue("DIRECT_FEED1".equals(refresh.serviceName()));
			assertEquals(DataTypes.FIELD_LIST, refresh.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_3", channelInfo.channelName());

			System.out.println("\n>>>>>>>>>>>>>>>>>>> Provider_7 (S3) goes up...");
			ommprovider_7 = EmaFactory.createOmmProvider(config.port("19007").providerName("Provider_9"), providerClient_7);
			assertNotNull(ommprovider_7);

			System.out.println("\n>>>>>>>>>>>>>>>>>>> Provider_8 (S4) goes up...");
			ommprovider_8 = EmaFactory.createOmmProvider(config.port("19008").providerName("Provider_9"), providerClient_8);
			assertNotNull(ommprovider_8);

			System.out.println("\n>>>>>>>>>>>>>>>>>>> Provider_3 (S1) goes down...");
			ommprovider_3.uninitialize();

			Thread.sleep(15000);
			System.out.println("\n>>>>>>>>>> Done waiting for 15 seconds...");

			message = providerClient_6.popMessage();

			// GenericMsg of active server for the DIRECT_FEED service
			genericMsg = (GenericMsg)message;
			assertEquals(1, genericMsg.streamId());
			assertEquals(DomainTypes.LOGIN, genericMsg.domainType());
			assertEquals(EmaRdm.ENAME_CONS_CONN_STATUS, genericMsg.name());
			assertEquals(DataTypes.MAP, genericMsg.payload().dataType());

			map = genericMsg.payload().map();
			mapIt = map.iterator();
			assertTrue(mapIt.hasNext());
			mapEntry = mapIt.next();
			assertTrue("WarmStandbyInfo".equals(mapEntry.key().ascii().toString()));
			assertEquals(DataTypes.ELEMENT_LIST, mapEntry.loadType());

			elementList = mapEntry.elementList();
			elementListIt = elementList.iterator();
			assertTrue(elementListIt.hasNext());
			elementEntry = elementListIt.next();
			assertEquals(EmaRdm.ENAME_WARMSTANDBY_MODE, elementEntry.name());
			assertEquals(ServerTypes.ACTIVE, elementEntry.uintValue());
			assertFalse(elementListIt.hasNext()); // End of ElementList

			StatusMsg statusMsg = (StatusMsg)consumerClient.popMessage();
			assertTrue("DIRECT_FEED1".equals(refresh.serviceName()) || "DIRECT_FEED1".equals(refresh.serviceName()));
			assertTrue("TRI.N".equals(refresh.name()) || "IBM.N".equals(refresh.name()));
			assertEquals(OmmState.StreamState.OPEN, statusMsg.state().streamState());
			assertEquals(OmmState.DataState.SUSPECT, statusMsg.state().dataState());
			assertEquals(OmmState.StatusCode.NONE, statusMsg.state().statusCode());
			assertEquals("Service for this item was lost.", statusMsg.state().statusText());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_3", channelInfo.channelName());

			statusMsg = (StatusMsg)consumerClient.popMessage();
			assertTrue("DIRECT_FEED1".equals(refresh.serviceName()) || "DIRECT_FEED1".equals(refresh.serviceName()));
			assertTrue("TRI.N".equals(refresh.name()) || "IBM.N".equals(refresh.name()));
			assertEquals(OmmState.StreamState.OPEN, statusMsg.state().streamState());
			assertEquals(OmmState.DataState.SUSPECT, statusMsg.state().dataState());
			assertEquals(OmmState.StatusCode.NONE, statusMsg.state().statusCode());
			assertEquals("Service for this item was lost.", statusMsg.state().statusText());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_3", channelInfo.channelName());

			refresh = (RefreshMsg)consumerClient.popMessage();
			assertEquals(DomainTypes.MARKET_PRICE, refresh.domainType());
			assertEquals("Open / Ok / None / 'Unsolicited Refresh Completed'", refresh.state().toString());
			assertTrue("TRI.N".equals(refresh.name()) || "IBM.N".equals(refresh.name()));
			assertTrue(refresh.complete());
			assertTrue(refresh.hasName());
			assertTrue(refresh.hasServiceName());
			assertTrue("DIRECT_FEED".equals(refresh.serviceName()) || "DIRECT_FEED1".equals(refresh.serviceName()));
			assertEquals(DataTypes.FIELD_LIST, refresh.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_6", channelInfo.channelName());

			refresh = (RefreshMsg)consumerClient.popMessage();
			assertEquals(DomainTypes.MARKET_PRICE, refresh.domainType());
			assertEquals("Open / Ok / None / 'Unsolicited Refresh Completed'", refresh.state().toString());
			assertTrue("TRI.N".equals(refresh.name()) || "IBM.N".equals(refresh.name()));
			assertTrue(refresh.complete());
			assertTrue(refresh.hasName());
			assertTrue(refresh.hasServiceName());
			assertTrue("DIRECT_FEED".equals(refresh.serviceName()) || "DIRECT_FEED1".equals(refresh.serviceName()));
			assertEquals(DataTypes.FIELD_LIST, refresh.payload().dataType());
			channelInfo = consumerClient.popChannelInfo();
			assertEquals("Channel_6", channelInfo.channelName());

			System.out.println("\n>>>>>>>>> S1 goes up...");
			ommprovider_3 = EmaFactory.createOmmProvider(config.port("19003").providerName("Provider_9"), providerClient_3);
			assertNotNull(ommprovider_3);

			Thread.sleep(11000);

			int msgCount = consumerClient.queueSize();

			boolean triRefreshFound = false;
			boolean ibmRefreshFound = false;
			boolean startFallbackFound = false;
			boolean completeFallbackFound = false;

			for (int i = 0; i < msgCount; i++)
			{
				Msg msg = consumerClient.popMessage();
				ChannelInformation channelInformation = consumerClient.popChannelInfo();

				if (msg instanceof RefreshMsg)
				{
					RefreshMsg refreshMsg = (RefreshMsg)msg;
					if (refreshMsg.domainType() == DomainTypes.MARKET_PRICE)
					{
						assertTrue(channelInformation.channelName().equals("Channel_7"));
						if (refreshMsg.name().equals("TRI.N"))
						{
							triRefreshFound = true;
							assertTrue(refreshMsg.serviceName().equals("DIRECT_FEED"));
						}
						else if (refreshMsg.name().equals("IBM.N"))
						{
							ibmRefreshFound = true;
							assertTrue(refreshMsg.serviceName().equals("DIRECT_FEED1"));
						}
					}
				}
				else if (msg instanceof StatusMsg)
				{
					statusMsg = (StatusMsg)msg;
					OmmState state = statusMsg.state();
					if (state.statusText().contains("preferred host complete")) completeFallbackFound = true;
					if (state.statusText().contains("preferred host starting fallback")) startFallbackFound = true;
				}
			}

			assertTrue(triRefreshFound);
			assertTrue(ibmRefreshFound);
			assertTrue(completeFallbackFound);
			assertTrue(startFallbackFound);

			assertEquals(3, providerClient_8.queueSize());

			message = providerClient_8.popMessage();

			genericMsg = (GenericMsg)message;
			assertEquals(1, genericMsg.streamId());
			assertEquals(DomainTypes.LOGIN, genericMsg.domainType());
			assertEquals(EmaRdm.ENAME_CONS_CONN_STATUS, genericMsg.name());
			assertEquals(DataTypes.MAP, genericMsg.payload().dataType());

			map = genericMsg.payload().map();
			mapIt = map.iterator();
			assertTrue(mapIt.hasNext());
			mapEntry = mapIt.next();
			assertTrue("WarmStandbyInfo".equals(mapEntry.key().ascii().toString()));
			assertEquals(DataTypes.ELEMENT_LIST, mapEntry.loadType());

			elementList = mapEntry.elementList();
			elementListIt = elementList.iterator();
			assertTrue(elementListIt.hasNext());
			elementEntry = elementListIt.next();
			assertEquals(EmaRdm.ENAME_WARMSTANDBY_MODE, elementEntry.name());
			assertEquals(ServerTypes.STANDBY, elementEntry.uintValue());
			assertFalse(elementListIt.hasNext()); // End of ElementList

			assertFalse(mapIt.hasNext()); // End of Map

			for (int i = 0; i < 2; i++)
			{
				message = providerClient_8.popMessage();
				reqMsg = (ReqMsg)message;
				assertTrue("DIRECT_FEED".equals(reqMsg.serviceName()) || "DIRECT_FEED1".equals(reqMsg.serviceName()));
				assertTrue("TRI.N".equals(reqMsg.name()) || "IBM.N".equals(reqMsg.name()));
				assertTrue(reqMsg.serviceId() == 1 || reqMsg.serviceId() == 2);
			}

			assertEquals(3, providerClient_7.queueSize());

			message = providerClient_7.popMessage();

			genericMsg = (GenericMsg)message;
			assertEquals(1, genericMsg.streamId());
			assertEquals(DomainTypes.LOGIN, genericMsg.domainType());
			assertEquals(EmaRdm.ENAME_CONS_CONN_STATUS, genericMsg.name());
			assertEquals(DataTypes.MAP, genericMsg.payload().dataType());

			map = genericMsg.payload().map();
			mapIt = map.iterator();
			assertTrue(mapIt.hasNext());
			mapEntry = mapIt.next();
			assertTrue("WarmStandbyInfo".equals(mapEntry.key().ascii().toString()));
			assertEquals(DataTypes.ELEMENT_LIST, mapEntry.loadType());

			elementList = mapEntry.elementList();
			elementListIt = elementList.iterator();
			assertTrue(elementListIt.hasNext());
			elementEntry = elementListIt.next();
			assertEquals(EmaRdm.ENAME_WARMSTANDBY_MODE, elementEntry.name());
			assertEquals(ServerTypes.ACTIVE, elementEntry.uintValue());
			assertFalse(elementListIt.hasNext()); // End of ElementList

			assertFalse(mapIt.hasNext()); // End of Map

			for (int i = 0; i < 2; i++)
			{
				message = providerClient_7.popMessage();
				reqMsg = (ReqMsg)message;
				assertTrue("DIRECT_FEED".equals(reqMsg.serviceName()) || "DIRECT_FEED1".equals(reqMsg.serviceName()));
				assertTrue("TRI.N".equals(reqMsg.name()) || "IBM.N".equals(reqMsg.name()));
				assertTrue(reqMsg.serviceId() == 1 || reqMsg.serviceId() == 2);
			}

			System.out.println(">>>>>>>>> Done!");
		}
		catch (Exception e)
		{
			e.printStackTrace();
			assertFalse(true);
		}
		finally
		{
			System.out.println("\n>>>>>>>>>> Uninitializing...");
			consumer.uninitialize();

			ommprovider_3.uninitialize();
			ommprovider_6.uninitialize();
			if (ommprovider_7 != null) ommprovider_7.uninitialize();
			if (ommprovider_8 != null) ommprovider_8.uninitialize();
		}
	}

    @Test // ETA-RTSDK2.2.1-42223
    public void testWSBLoginBasedCheckNoActionAfterServiceDown()
    {
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

        System.out.println("\n>>>>>>>>>G0 and G1* go up.");

        // WSB-G0 is up
        OmmProvider ommprovider_3 = EmaFactory.createOmmProvider(config.port("19003").providerName("Provider_9"), providerClient_3);
        assertNotNull(ommprovider_3);

        OmmProvider ommprovider_6 = EmaFactory.createOmmProvider(config.port("19006").providerName("Provider_9"), providerClient_6);
        assertNotNull(ommprovider_6);

        // WSB-G1 is up
        OmmProvider ommprovider_7 = EmaFactory.createOmmProvider(config.port("19007").providerName("Provider_9"), providerClient_7);
        assertNotNull(ommprovider_7);

        OmmProvider ommprovider_8 = EmaFactory.createOmmProvider(config.port("19008").providerName("Provider_9"), providerClient_8);
        assertNotNull(ommprovider_8);

        OmmConsumer consumer = null;
        ConsumerTestOptions consumerOption = new ConsumerTestOptions();

        consumerOption.getChannelInformation = true;
        consumerOption.getSessionChannelInfo = false;
        ConsumerTestClient consumerClient = new ConsumerTestClient(consumerOption);

        try
        {
            ServiceList serviceList = EmaFactory.createServiceList("SVG1");

            serviceList.concreteServiceList().add("DIRECT_FEED");
            serviceList.concreteServiceList().add("DIRECT_FEED1");

            consumer = EmaFactory.createOmmConsumer(EmaFactory
                    .createOmmConsumerConfig(emaConfigFileLocation).addServiceList(serviceList)
                    .consumerName("Consumer_35_4"), consumerClient);

            ReqMsg reqMsg = EmaFactory.createReqMsg();

            System.out.println(" >>>>>>>>>>>>>> Registering items...");
            long itemHandle = consumer.registerClient(reqMsg.name("IBM.N").serviceName("DIRECT_FEED"), consumerClient);
            long itemHandle2 = consumer.registerClient(reqMsg.clear().name("TRI.N").serviceName("DIRECT_FEED1"), consumerClient);

            Thread.sleep(5000);

            int consumerCount = consumerClient.queueSize();
            int itemRefreshCount = 0;
            int loginRefreshCount = 0;

            for (int i = 0; i < consumerCount; i++)
            {
                Msg message = consumerClient.popMessage();
                ChannelInformation channelInfo = consumerClient.popChannelInfo();

                if (message instanceof RefreshMsg)
                {
                    RefreshMsg refreshMsg = (RefreshMsg)message;
                    if (refreshMsg.domainType() == DomainTypes.MARKET_PRICE)
                    {
                        itemRefreshCount++;
                        if (refreshMsg.name().equals("IBM.N"))
                        {
                            assertTrue(refreshMsg.serviceName().equals("DIRECT_FEED"));
                            assertEquals(1, refreshMsg.serviceId());
                        }
                        else if (refreshMsg.name().equals("TRI.N"))
                        {
                            assertTrue(refreshMsg.serviceName().equals("DIRECT_FEED1"));
                            assertEquals(2, refreshMsg.serviceId());
                        }
                        assertTrue(refreshMsg.solicited());
                        assertEquals("Channel_7", channelInfo.channelName());
                    }
                    else if (refreshMsg.domainType() == DomainTypes.LOGIN)
                    {
                        loginRefreshCount++;
                    }
                }
            }

            assertEquals(2, itemRefreshCount);
            assertEquals(1, loginRefreshCount);

            assertEquals(3, providerClient_7.queueSize());

            Msg message = providerClient_7.popMessage();

            GenericMsg genericMsg = (GenericMsg)message;
            assertEquals(1, genericMsg.streamId());
            assertEquals(DomainTypes.LOGIN, genericMsg.domainType());
            assertEquals(EmaRdm.ENAME_CONS_CONN_STATUS, genericMsg.name());
            assertEquals(DataTypes.MAP, genericMsg.payload().dataType());

            Map map = genericMsg.payload().map();
            Iterator<MapEntry> mapIt = map.iterator();
            assertTrue(mapIt.hasNext());
            MapEntry mapEntry = mapIt.next();
            assertTrue("WarmStandbyInfo".equals(mapEntry.key().ascii().toString()));
            assertEquals(DataTypes.ELEMENT_LIST, mapEntry.loadType());

            ElementList elementList = mapEntry.elementList();
            Iterator<ElementEntry> elementListIt = elementList.iterator();
            assertTrue(elementListIt.hasNext());
            ElementEntry elementEntry = elementListIt.next();
            assertEquals(EmaRdm.ENAME_WARMSTANDBY_MODE, elementEntry.name());
            assertEquals(ServerTypes.ACTIVE, elementEntry.uintValue());
            assertFalse(elementListIt.hasNext()); // End of ElementList

            assertFalse(mapIt.hasNext()); // End of Map

            for (int i = 0; i < 2; i++)
            {
                ReqMsg request = (ReqMsg)providerClient_7.popMessage();

                assertTrue("DIRECT_FEED".equals(request.serviceName()) || "DIRECT_FEED1".equals(request.serviceName()));
                assertTrue("TRI.N".equals(request.name()) || "IBM.N".equals(request.name()));
                assertTrue(request.serviceId() == 1 || request.serviceId() == 2);
            }

            assertEquals(3, providerClient_8.queueSize());

            message = providerClient_8.popMessage();

            genericMsg = (GenericMsg)message;
            assertEquals(1, genericMsg.streamId());
            assertEquals(DomainTypes.LOGIN, genericMsg.domainType());
            assertEquals(EmaRdm.ENAME_CONS_CONN_STATUS, genericMsg.name());
            assertEquals(DataTypes.MAP, genericMsg.payload().dataType());

            map = genericMsg.payload().map();
            mapIt = map.iterator();
            assertTrue(mapIt.hasNext());
            mapEntry = mapIt.next();
            assertTrue("WarmStandbyInfo".equals(mapEntry.key().ascii().toString()));
            assertEquals(DataTypes.ELEMENT_LIST, mapEntry.loadType());

            elementList = mapEntry.elementList();
            elementListIt = elementList.iterator();
            assertTrue(elementListIt.hasNext());
            elementEntry = elementListIt.next();
            assertEquals(EmaRdm.ENAME_WARMSTANDBY_MODE, elementEntry.name());
            assertEquals(ServerTypes.STANDBY, elementEntry.uintValue());
            assertFalse(elementListIt.hasNext()); // End of ElementList

            assertFalse(mapIt.hasNext()); // End of Map

            for (int i = 0; i < 2; i++)
            {
                ReqMsg request = (ReqMsg)providerClient_8.popMessage();

                assertTrue("DIRECT_FEED".equals(request.serviceName()) || "DIRECT_FEED1".equals(request.serviceName()));
                assertTrue("TRI.N".equals(request.name()) || "IBM.N".equals(request.name()));
                assertTrue(request.serviceId() == 1 || request.serviceId() == 2);
            }

            System.out.println("Send the DIRECT_FEED service down state for ommprovider_7");
            ElementList serviceState2 = EmaFactory.createElementList();
            serviceState2.add( EmaFactory.createElementEntry().uintValue( EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_DOWN ));
            serviceState2.add( EmaFactory.createElementEntry().uintValue( EmaRdm.ENAME_ACCEPTING_REQS, 0 ));
            serviceState2.add( EmaFactory.createElementEntry().state( EmaRdm.ENAME_STATUS, OmmState.StreamState.OPEN, OmmState.DataState.SUSPECT, OmmState.StatusCode.NONE, ""));
            FilterList filterListEnc2 = EmaFactory.createFilterList();
            filterListEnc2.add( EmaFactory.createFilterEntry().elementList( EmaRdm.SERVICE_STATE_ID, FilterEntry.FilterAction.SET, serviceState2 ) );
            Map mapEnc2 = EmaFactory.createMap();
            mapEnc2.add( EmaFactory.createMapEntry().keyUInt( 1, MapEntry.MapAction.UPDATE, filterListEnc2 ));
            UpdateMsg updateMsg2 = EmaFactory.createUpdateMsg();
            ommprovider_7.submit( updateMsg2.domainType( EmaRdm.MMT_DIRECTORY ).
                    filter( EmaRdm.SERVICE_STATE_FILTER ).
                    payload( mapEnc2 ), 0);	// use 0 item handle to fan-out to all subscribers

            System.out.println("Send the DIRECT_FEED1 service down state for ommprovider_8");
            serviceState2 = EmaFactory.createElementList();
            serviceState2.add( EmaFactory.createElementEntry().uintValue( EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_DOWN ));
            serviceState2.add( EmaFactory.createElementEntry().uintValue( EmaRdm.ENAME_ACCEPTING_REQS, 0 ));
            serviceState2.add( EmaFactory.createElementEntry().state( EmaRdm.ENAME_STATUS, OmmState.StreamState.OPEN, OmmState.DataState.SUSPECT, OmmState.StatusCode.NONE, ""));
            filterListEnc2 = EmaFactory.createFilterList();
            filterListEnc2.add( EmaFactory.createFilterEntry().elementList( EmaRdm.SERVICE_STATE_ID, FilterEntry.FilterAction.SET, serviceState2 ) );
            mapEnc2 = EmaFactory.createMap();
            mapEnc2.add( EmaFactory.createMapEntry().keyUInt( 2, MapEntry.MapAction.UPDATE, filterListEnc2 ));
            updateMsg2 = EmaFactory.createUpdateMsg();
            ommprovider_8.submit( updateMsg2.domainType( EmaRdm.MMT_DIRECTORY ).
                    filter( EmaRdm.SERVICE_STATE_FILTER ).
                    payload( mapEnc2 ), 0);	// use 0 item handle to fan-out to all subscribers

            Thread.sleep(5000);

            assertEquals(1, consumerClient.queueSize());

            StatusMsg statusMsg = (StatusMsg)consumerClient.popMessage();
            ChannelInformation channelInfo = consumerClient.popChannelInfo();
            assertEquals(1, statusMsg.serviceId());
            assertEquals("DIRECT_FEED", statusMsg.serviceName());
            assertEquals("IBM.N", statusMsg.name());
            assertEquals("Channel_7", channelInfo.channelName());

            FieldList fieldList = EmaFactory.createFieldList();

            fieldList.clear();
            fieldList.add(EmaFactory.createFieldEntry().real(22, 3991, OmmReal.MagnitudeType.EXPONENT_NEG_2));
            fieldList.add(EmaFactory.createFieldEntry().real(30, 10, OmmReal.MagnitudeType.EXPONENT_0));

            System.out.println(" >>>>>>>>>>>> Sending updates...");
            ommprovider_7.submit( EmaFactory.createUpdateMsg().payload( fieldList ), providerClient_7.retriveItemHandle("IBM.N") );
            ommprovider_7.submit( EmaFactory.createUpdateMsg().payload( fieldList ), providerClient_7.retriveItemHandle("TRI.N") );

            ommprovider_8.submit( EmaFactory.createUpdateMsg().payload( fieldList ), providerClient_8.retriveItemHandle("IBM.N") );
            ommprovider_8.submit( EmaFactory.createUpdateMsg().payload( fieldList ), providerClient_8.retriveItemHandle("TRI.N") );

            Thread.sleep(5000);

            assertEquals(2, consumerClient.queueSize()); // consumer receives 2 updates since the provider anyway sends them

            for (int i = 0; i < 2; i++)
            {
                UpdateMsg update = (UpdateMsg)consumerClient.popMessage();
                ChannelInformation channelInformation = consumerClient.popChannelInfo();
                assertEquals(DomainTypes.MARKET_PRICE, update.domainType());
                assertEquals("Channel_7", channelInformation.channelName());
            }

            System.out.println(">>>>>>>>>> G1 servers go down");

            ommprovider_7.uninitialize();
            ommprovider_8.uninitialize();

            Thread.sleep(8000);

            consumerCount = consumerClient.queueSize();

            int itemLostCount = 0;
            boolean reconnectToG0Found = false;

            for (int i = 0; i < consumerCount; i++)
            {
                message = consumerClient.popMessage();
                ChannelInformation ci = consumerClient.popChannelInfo();

                if (message instanceof StatusMsg)
                {
                    StatusMsg sm = (StatusMsg)message;
                    OmmState state = sm.state();
                    if (state.statusText().contains("Service for this item was lost"))
                    {
                        itemLostCount++;
                        assertTrue("IBM.N".equals(sm.name()) || "TRI.N".equals(sm.name()));
                        assertEquals(DomainTypes.MARKET_PRICE, sm.domainType());
                        assertEquals("Channel_7", ci.channelName());
                    }
                }
                else if (message instanceof RefreshMsg)
                {
                    if (ci.channelName().equals("Channel_3")) reconnectToG0Found = true;
                }
            }

            assertEquals(2, itemLostCount);
            assertTrue(reconnectToG0Found);

            System.out.println(">>>>>>>>>> G1 servers go up");

            providerClient_7 = new ProviderTestClient(providerTestOptions);
            providerClient_7.name = "Provider_7";
            providerClient_8 = new ProviderTestClient(providerTestOptions);
            providerClient_8.name = "Provider_8";

            ommprovider_8 = EmaFactory.createOmmProvider(config.port("19008").providerName("Provider_9"), providerClient_8);
            assertNotNull(ommprovider_8);

            ommprovider_7 = EmaFactory.createOmmProvider(config.port("19007").providerName("Provider_9"), providerClient_7);
            assertNotNull(ommprovider_7);

            Thread.sleep(40000);

            boolean foundPHFallback = false;

            consumerCount = consumerClient.queueSize();
            for (int i = 0; i < consumerCount; i++)
            {
                message = consumerClient.popMessage();
                if (message instanceof StatusMsg && (message.domainType() == DomainTypes.LOGIN))
                {
                    StatusMsg msg = (StatusMsg)message;
                    OmmState state = msg.state();
                    if (state.statusText().contains("preferred host starting fallback")) { foundPHFallback = true; }
                }
                ChannelInformation ci = consumerClient.popChannelInfo();
                assertFalse("Channel_7".equals(ci.channelName()) || "Channel_8".equals(ci.channelName())); // we cannot ge events from these channels since we stay on G0!
            }

            assertTrue(foundPHFallback);
        }
        catch (Exception e)
        {
            e.printStackTrace();
            assertFalse(true);
        }
        finally
        {
            System.out.println("\n Uninitializing...");
            consumer.uninitialize();

            ommprovider_3.uninitialize();
            ommprovider_6.uninitialize();
            ommprovider_7.uninitialize();
            ommprovider_8.uninitialize();
        }
    }

    @Test //ETA-RTSDK2.2.1-42214
    public void testReconnectionPatternWSBGroupsAndConnectionListConfigured()
    {
        String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";

        OmmConsumer consumer = null;
        OmmProvider provider1 = null;
        OmmProvider provider2 = null;
        AtomicReference<OmmConsumer> consumerVariable = new AtomicReference<>(null);
        AtomicReference<OmmProvider> providerVariable1 = new AtomicReference<>(null);
        AtomicReference<OmmProvider> providerVariable2 = new AtomicReference<>(null);
        ConsumerTestOptions consumerOption = new ConsumerTestOptions();

        consumerOption.getChannelInformation = true;
        consumerOption.getSessionChannelInfo = false;
        ConsumerTestClient consumerClient = new ConsumerTestClient(consumerOption);

        ByteArrayOutputStream output = new ByteArrayOutputStream(100000);
        PrintStream printStream = new PrintStream(output);

        Logger logger = Logger.getLogger("com.refinitiv.ema.access.OmmConsumerImpl");
        StreamHandler streamHandler = new StreamHandler(printStream, new SimpleFormatter());
        logger.addHandler(streamHandler);

        OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig(emaConfigFileLocation);
        ProviderTestOptions providerTestOptions = new ProviderTestOptions();
        providerTestOptions.supportStandby = true;
        providerTestOptions.sendRefreshAttrib = true;
        providerTestOptions.itemGroupId = ByteBuffer.wrap("10".getBytes());
        ProviderTestClient providerClient_7 = new ProviderTestClient(providerTestOptions);
        providerClient_7.name = "Provider_7";
        ProviderTestClient providerClient_13 = new ProviderTestClient(providerTestOptions);
        providerClient_13.name = "Provider_13";

        try
        {
            ExecutorService executorService = Executors.newFixedThreadPool(2);

            Runnable providerThread = () -> {

                System.out.println("Provider thread runs!");
                try
                {
                    Thread.sleep(45000);
                } catch (Exception e) { }

                OmmProvider ommprovider_7 = EmaFactory.createOmmProvider(config.port("19007").providerName("Provider_9"), providerClient_7);
                assertNotNull(ommprovider_7);

                OmmProvider ommprovider_13 = EmaFactory.createOmmProvider(config.port("19013").providerName("Provider_9"), providerClient_13);
                assertNotNull(ommprovider_13);

                providerVariable1.set(ommprovider_7);
                providerVariable2.set(ommprovider_13);

            };

            Runnable consumerThread = () -> {
                OmmConsumer consumer1 = EmaFactory.createOmmConsumer(EmaFactory
                    .createOmmConsumerConfig(emaConfigFileLocation)
                    .consumerName("Consumer_39"), consumerClient);

                consumerVariable.set(consumer1);

            };

            executorService.execute(providerThread);
            executorService.execute(consumerThread);

            Thread.sleep(60000);

            streamHandler.flush();

            byte[] outputArr = output.toByteArray();
            String strOutput = new String(outputArr);

            String[] messages = strOutput.split("WARNING: loggerMsg");

            String[] resultingArray = new String[messages.length];

            for (int i = 0; i < messages.length; i++)
            {
                String[] lines = messages[i].split("\n");
                for (int j = 0; j < lines.length; j++)
                {
                    if (lines[j].contains("Text:"))
                    {
                        if (lines[j].indexOf("Channel_") != -1)
                        {
                            if (i < resultingArray.length)
                                resultingArray[i] = lines[j].substring(lines[j].indexOf("Channel_"));
                        }
                    }
                }
            }

            System.out.println(resultingArray);
            String result = Arrays.stream(resultingArray).reduce("", (a,b) -> a + " " + b);
            String expectedSubstring = "Channel_7 Channel_3 Channel_7 Channel_9 Channel_7 Channel_12 " +
                    "Channel_7 Channel_3 Channel_7 Channel_9 Channel_7 Channel_11 Channel_7 Channel_3 " +
                    "Channel_7 Channel_9 Channel_7 Channel_12 Channel_7 Channel_3 Channel_7 Channel_9 " +
                    "Channel_7 Channel_13 Channel_7 Channel_3 Channel_7 Channel_9";
            assertTrue(result.contains(expectedSubstring));

            assertTrue(providerClient_7.queueSize() > 0 || providerClient_13.queueSize() > 0); // one of the providers got connected
        }
        catch (Exception e)
        {
            e.printStackTrace();
            assertFalse(true);
        }
        finally
        {
            streamHandler.close();
            logger.removeHandler(streamHandler);

            System.out.println("Uninitializing...");

            consumer = consumerVariable.get();
            if (consumer != null) consumer.uninitialize();

            provider1 = providerVariable1.get();
            if (provider1 != null) provider1.uninitialize();

            provider2 = providerVariable2.get();
            if (provider2 != null) provider2.uninitialize();
        }
    }

    @Test //ETA-RTSDK2.2.1-42224
    public void testFallbackToWSBGroupAfterConnectionListByFunctionalCall()
    {
        String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";

        OmmConsumer consumer = null;
        ConsumerTestOptions consumerOption = new ConsumerTestOptions();

        consumerOption.getChannelInformation = true;
        consumerOption.getSessionChannelInfo = false;
        ConsumerTestClient consumerClient = new ConsumerTestClient(consumerOption);

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
        ProviderTestClient providerClient_10 = new ProviderTestClient(providerTestOptions);
        providerClient_10.name = "Provider_10";
        ProviderTestClient providerClient_11 = new ProviderTestClient(providerTestOptions);
        providerClient_11.name = "Provider_11";
        ProviderTestClient providerClient_12 = new ProviderTestClient(providerTestOptions);
        providerClient_12.name = "Provider_12";
        ProviderTestClient providerClient_13 = new ProviderTestClient(providerTestOptions);
        providerClient_13.name = "Provider_13";

        OmmProvider ommprovider_3 = null;
        OmmProvider ommprovider_6 = null;
        OmmProvider ommprovider_7 = null;
        OmmProvider ommprovider_8 = null;
        OmmProvider ommprovider_9 = null;
        OmmProvider ommprovider_10 = null;
        OmmProvider ommprovider_11 = null;
        OmmProvider ommprovider_13 = null;

        OmmProvider ommprovider_12 = EmaFactory.createOmmProvider(config.port("19012").providerName("Provider_9"), providerClient_12);
        assertNotNull(ommprovider_12);

        try
        {
            consumer = EmaFactory.createOmmConsumer(EmaFactory
                    .createOmmConsumerConfig(emaConfigFileLocation)
                    .consumerName("Consumer_39"), consumerClient);

            Thread.sleep(10000);

            assertEquals(2, consumerClient.queueSize());

            Msg message = consumerClient.popMessage();
            ChannelInformation ci = consumerClient.popChannelInfo();

            assertTrue(message instanceof StatusMsg);

            StatusMsg statusMsg = (StatusMsg)message;

            OmmState state = statusMsg.state();
            assertEquals(DomainTypes.LOGIN, statusMsg.domainType());

            message = consumerClient.popMessage();
            ci = consumerClient.popChannelInfo();

            assertTrue(message instanceof RefreshMsg);

            RefreshMsg refresh = (RefreshMsg)message;
            assertTrue(refresh.solicited());
            assertEquals(DomainTypes.LOGIN, refresh.domainType());

            assertEquals("Channel_12", ci.channelName());

            ommprovider_7 = EmaFactory.createOmmProvider(config.port("19007").providerName("Provider_9"), providerClient_7);
            assertNotNull(ommprovider_7);
            ommprovider_8 = EmaFactory.createOmmProvider(config.port("19008").providerName("Provider_9"), providerClient_8);
            assertNotNull(ommprovider_8);

            Thread.sleep(3000);

            assertEquals(0, consumerClient.queueSize());

            Thread.sleep(20000);

            consumer.fallbackPreferredHost();

            Thread.sleep(5000);

            message = consumerClient.popMessage();
            ci = consumerClient.popChannelInfo();

            assertTrue(message instanceof StatusMsg);

            statusMsg = (StatusMsg)message;
            state = statusMsg.state();
            assertTrue(state.statusText().toString().contains("preferred host starting fallback"));
            assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
            assertEquals("Channel_12", ci.channelName());

            message = consumerClient.popMessage();
            ci = consumerClient.popChannelInfo();

            assertTrue(message instanceof StatusMsg);

            message = consumerClient.popMessage();
            ci = consumerClient.popChannelInfo();

            assertTrue(message instanceof StatusMsg);

            statusMsg = (StatusMsg)message;
            state = statusMsg.state();
            assertTrue(state.statusText().toString().contains("preferred host complete"));
            assertEquals(DomainTypes.LOGIN, statusMsg.domainType());
            assertEquals("Channel_7", ci.channelName());

            message = consumerClient.popMessage();
            ci = consumerClient.popChannelInfo();

            assertTrue(message instanceof RefreshMsg);
            assertEquals("Channel_7", ci.channelName());

            assertEquals(1, providerClient_7.queueSize());

            message = providerClient_7.popMessage();
            assertTrue(message instanceof GenericMsg);

            GenericMsg genericMsg = (GenericMsg)message;
            assertEquals(1, genericMsg.streamId());
            assertEquals(DomainTypes.LOGIN, genericMsg.domainType());
            assertEquals(EmaRdm.ENAME_CONS_CONN_STATUS, genericMsg.name());
            assertEquals(DataTypes.MAP, genericMsg.payload().dataType());

            Map map = genericMsg.payload().map();
            Iterator<MapEntry> mapIt = map.iterator();
            assertTrue(mapIt.hasNext());
            MapEntry mapEntry = mapIt.next();
            assertTrue("WarmStandbyInfo".equals(mapEntry.key().ascii().toString()));
            assertEquals(DataTypes.ELEMENT_LIST, mapEntry.loadType());

            ElementList elementList = mapEntry.elementList();
            Iterator<ElementEntry> elementListIt = elementList.iterator();
            assertTrue(elementListIt.hasNext());
            ElementEntry elementEntry = elementListIt.next();
            assertEquals(EmaRdm.ENAME_WARMSTANDBY_MODE, elementEntry.name());
            assertEquals(ServerTypes.ACTIVE, elementEntry.uintValue());
            assertFalse(elementListIt.hasNext()); // End of ElementList

            assertFalse(mapIt.hasNext()); // End of Map


            assertEquals(1, providerClient_8.queueSize());

            message = providerClient_8.popMessage();
            assertTrue(message instanceof GenericMsg);

            genericMsg = (GenericMsg)message;
            assertEquals(1, genericMsg.streamId());
            assertEquals(DomainTypes.LOGIN, genericMsg.domainType());
            assertEquals(EmaRdm.ENAME_CONS_CONN_STATUS, genericMsg.name());
            assertEquals(DataTypes.MAP, genericMsg.payload().dataType());

            map = genericMsg.payload().map();
            mapIt = map.iterator();
            assertTrue(mapIt.hasNext());
            mapEntry = mapIt.next();
            assertTrue("WarmStandbyInfo".equals(mapEntry.key().ascii().toString()));
            assertEquals(DataTypes.ELEMENT_LIST, mapEntry.loadType());

            elementList = mapEntry.elementList();
            elementListIt = elementList.iterator();
            assertTrue(elementListIt.hasNext());
            elementEntry = elementListIt.next();
            assertEquals(EmaRdm.ENAME_WARMSTANDBY_MODE, elementEntry.name());
            assertEquals(ServerTypes.STANDBY, elementEntry.uintValue());
            assertFalse(elementListIt.hasNext()); // End of ElementList

            assertFalse(mapIt.hasNext()); // End of Map
        }
        catch (Exception e)
        {
            e.printStackTrace();
            assertFalse(true);
        }
        finally
        {
            System.out.println("Uninitializing...");
            if (consumer != null) consumer.uninitialize();

            if (ommprovider_12 != null) ommprovider_12.uninitialize();
            if (ommprovider_7 != null) ommprovider_7.uninitialize();
            if (ommprovider_8 != null) ommprovider_8.uninitialize();
        }

    }

    @Test //ETA-RTSDK2.2.1-42222
    public void testLoginBasedScenarioWhenPerServiceListSet()
    {
        String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";

        OmmConsumer consumer = null;
        ConsumerTestOptions consumerOption = new ConsumerTestOptions();

        consumerOption.getChannelInformation = true;
        consumerOption.getSessionChannelInfo = false;
        ConsumerTestClient consumerClient = new ConsumerTestClient(consumerOption);

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


        OmmProvider ommprovider_3 = EmaFactory.createOmmProvider(config.port("19003").providerName("Provider_9"), providerClient_3);
        assertNotNull(ommprovider_3);
        OmmProvider ommprovider_6 = EmaFactory.createOmmProvider(config.port("19006").providerName("Provider_9"), providerClient_6);
        assertNotNull(ommprovider_6);

        OmmProvider ommprovider_7 = EmaFactory.createOmmProvider(config.port("19007").providerName("Provider_9"), providerClient_7);
        assertNotNull(ommprovider_7);
        OmmProvider ommprovider_8 = EmaFactory.createOmmProvider(config.port("19008").providerName("Provider_9"), providerClient_8);
        assertNotNull(ommprovider_8);

        try
        {
            ServiceList serviceList = EmaFactory.createServiceList("SVG1");

            serviceList.concreteServiceList().add("DIRECT_FEED");
            serviceList.concreteServiceList().add("DIRECT_FEED1");

            consumer = EmaFactory.createOmmConsumer(EmaFactory
                    .createOmmConsumerConfig(emaConfigFileLocation)
                    .consumerName("Consumer_41").addServiceList(serviceList), consumerClient);

            Thread.sleep(5000);

            consumerClient.clearQueue();
            while (consumerClient.channelInfoSize() > 0) consumerClient.popChannelInfo();

            assertEquals(1, providerClient_3.queueSize());

            Msg message = providerClient_3.popMessage();
            assertTrue(message instanceof GenericMsg);

            GenericMsg genericMsg = (GenericMsg)message;
            assertEquals(1, genericMsg.streamId());
            assertEquals(DomainTypes.LOGIN, genericMsg.domainType());
            assertEquals(EmaRdm.ENAME_CONS_CONN_STATUS, genericMsg.name());
            assertEquals(DataTypes.MAP, genericMsg.payload().dataType());

            Map map = genericMsg.payload().map();
            Iterator<MapEntry> mapIt = map.iterator();
            assertTrue(mapIt.hasNext());
            MapEntry mapEntry = mapIt.next();
            assertTrue("WarmStandbyInfo".equals(mapEntry.key().ascii().toString()));
            assertEquals(DataTypes.ELEMENT_LIST, mapEntry.loadType());

            ElementList elementList = mapEntry.elementList();
            Iterator<ElementEntry> elementListIt = elementList.iterator();
            assertTrue(elementListIt.hasNext());
            ElementEntry elementEntry = elementListIt.next();
            assertEquals(EmaRdm.ENAME_WARMSTANDBY_MODE, elementEntry.name());
            assertEquals(ServerTypes.STANDBY, elementEntry.uintValue());
            assertFalse(elementListIt.hasNext()); // End of ElementList

            assertFalse(mapIt.hasNext()); // End of Map

            assertEquals(1, providerClient_6.queueSize());

            message = providerClient_6.popMessage();
            assertTrue(message instanceof GenericMsg);

            genericMsg = (GenericMsg)message;
            assertEquals(1, genericMsg.streamId());
            assertEquals(DomainTypes.LOGIN, genericMsg.domainType());
            assertEquals(EmaRdm.ENAME_CONS_CONN_STATUS, genericMsg.name());
            assertEquals(DataTypes.MAP, genericMsg.payload().dataType());

            map = genericMsg.payload().map();
            mapIt = map.iterator();
            assertTrue(mapIt.hasNext());
            mapEntry = mapIt.next();
            assertTrue("WarmStandbyInfo".equals(mapEntry.key().ascii().toString()));
            assertEquals(DataTypes.ELEMENT_LIST, mapEntry.loadType());

            elementList = mapEntry.elementList();
            elementListIt = elementList.iterator();
            assertTrue(elementListIt.hasNext());
            elementEntry = elementListIt.next();
            assertEquals(EmaRdm.ENAME_WARMSTANDBY_MODE, elementEntry.name());
            assertEquals(ServerTypes.ACTIVE, elementEntry.uintValue());
            assertFalse(elementListIt.hasNext()); // End of ElementList

            assertFalse(mapIt.hasNext()); // End of Map

            ReqMsg reqMsg = EmaFactory.createReqMsg();

            System.out.println(" >>>>>>>>>>>>>> Registering items...");
            long itemHandle = consumer.registerClient(reqMsg.name("IBM.N").serviceName("DIRECT_FEED"), consumerClient);
            long itemHandle2 = consumer.registerClient(reqMsg.clear().name("TRI.N").serviceName("DIRECT_FEED1"), consumerClient);

            Thread.sleep(5000);

            assertEquals(2, providerClient_3.queueSize());

            boolean ibmFound = false;
            boolean triFound = false;

            for (int i = 0; i < 2; i++)
            {
                ReqMsg request = (ReqMsg)providerClient_3.popMessage();

                if (request.name().equals("TRI.N"))
                {
                    assertEquals("DIRECT_FEED1", request.serviceName());
                    assertEquals(2, request.serviceId());
                    triFound = true;
                }
                else if (request.name().equals("IBM.N"))
                {
                    assertEquals("DIRECT_FEED", request.serviceName());
                    assertEquals(1, request.serviceId());
                    ibmFound = true;
                }
            }

            assertTrue(ibmFound && triFound);

            assertEquals(2, providerClient_6.queueSize());

            ibmFound = false;
            triFound = false;

            for (int i = 0; i < 2; i++)
            {
                ReqMsg request = (ReqMsg)providerClient_6.popMessage();

                if (request.name().equals("TRI.N"))
                {
                    assertEquals("DIRECT_FEED1", request.serviceName());
                    assertEquals(2, request.serviceId());
                    triFound = true;
                }
                else if (request.name().equals("IBM.N"))
                {
                    assertEquals("DIRECT_FEED", request.serviceName());
                    assertEquals(1, request.serviceId());
                    ibmFound = true;
                }
            }

            assertTrue(ibmFound && triFound);

            assertEquals(2, consumerClient.queueSize());

            ibmFound = false;
            triFound = false;

            for (int i = 0; i < 2; i++)
            {
                message = consumerClient.popMessage();
                ChannelInformation ci = consumerClient.popChannelInfo();

                assertTrue(message instanceof RefreshMsg);
                RefreshMsg refresh = (RefreshMsg)message;

                if (refresh.name().equals("TRI.N"))
                {
                    assertEquals("DIRECT_FEED1", refresh.serviceName());
                    assertEquals(2, refresh.serviceId());
                    triFound = true;
                }
                else if (refresh.name().equals("IBM.N"))
                {
                    assertEquals("DIRECT_FEED", refresh.serviceName());
                    assertEquals(1, refresh.serviceId());
                    ibmFound = true;
                }

                assertEquals("Channel_6", ci.channelName());
                assertTrue(refresh.solicited());
            }

            assertTrue(ibmFound && triFound);

            System.out.println("Send the DIRECT_FEED service down state for ommprovider_6");
            ElementList serviceState2 = EmaFactory.createElementList();
            serviceState2.add( EmaFactory.createElementEntry().uintValue( EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_DOWN ));
            serviceState2.add( EmaFactory.createElementEntry().uintValue( EmaRdm.ENAME_ACCEPTING_REQS, 0 ));
            serviceState2.add( EmaFactory.createElementEntry().state( EmaRdm.ENAME_STATUS, OmmState.StreamState.OPEN, OmmState.DataState.SUSPECT, OmmState.StatusCode.NONE, ""));
            FilterList filterListEnc2 = EmaFactory.createFilterList();
            filterListEnc2.add( EmaFactory.createFilterEntry().elementList( EmaRdm.SERVICE_STATE_ID, FilterEntry.FilterAction.SET, serviceState2 ) );
            Map mapEnc2 = EmaFactory.createMap();
            mapEnc2.add( EmaFactory.createMapEntry().keyUInt( 1, MapEntry.MapAction.UPDATE, filterListEnc2 ));
            UpdateMsg updateMsg2 = EmaFactory.createUpdateMsg();
            ommprovider_6.submit( updateMsg2.domainType( EmaRdm.MMT_DIRECTORY ).
                    filter( EmaRdm.SERVICE_STATE_FILTER ).
                    payload( mapEnc2 ), 0);	// use 0 item handle to fan-out to all subscribers

            System.out.println("Send the DIRECT_FEED1 service down state for ommprovider_3");
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

            Thread.sleep(5000);

            assertEquals(1, consumerClient.queueSize());

            message = consumerClient.popMessage();
            ChannelInformation ci = consumerClient.popChannelInfo();

            assertTrue(message instanceof StatusMsg);
            assertEquals("Channel_6", ci.channelName());
            assertTrue(((StatusMsg)message).state().toString().contains("Open / Suspect / None /"));

            System.out.println("Send the DIRECT_FEED service up state for ommprovider_6");
            serviceState2 = EmaFactory.createElementList();
            serviceState2.add( EmaFactory.createElementEntry().uintValue( EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_UP ));
            serviceState2.add( EmaFactory.createElementEntry().uintValue( EmaRdm.ENAME_ACCEPTING_REQS, 1 ));
            serviceState2.add( EmaFactory.createElementEntry().state( EmaRdm.ENAME_STATUS, OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, ""));
            filterListEnc2 = EmaFactory.createFilterList();
            filterListEnc2.add( EmaFactory.createFilterEntry().elementList( EmaRdm.SERVICE_STATE_ID, FilterEntry.FilterAction.SET, serviceState2 ) );
            mapEnc2 = EmaFactory.createMap();
            mapEnc2.add( EmaFactory.createMapEntry().keyUInt( 1, MapEntry.MapAction.UPDATE, filterListEnc2 ));
            updateMsg2 = EmaFactory.createUpdateMsg();
            ommprovider_6.submit( updateMsg2.domainType( EmaRdm.MMT_DIRECTORY ).
                    filter( EmaRdm.SERVICE_STATE_FILTER ).
                    payload( mapEnc2 ), 0);	// use 0 item handle to fan-out to all subscribers

            System.out.println("Send the DIRECT_FEED1 service up state for ommprovider_3");
            serviceState2 = EmaFactory.createElementList();
            serviceState2.add( EmaFactory.createElementEntry().uintValue( EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_UP ));
            serviceState2.add( EmaFactory.createElementEntry().uintValue( EmaRdm.ENAME_ACCEPTING_REQS, 1 ));
            serviceState2.add( EmaFactory.createElementEntry().state( EmaRdm.ENAME_STATUS, OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, ""));
            filterListEnc2 = EmaFactory.createFilterList();
            filterListEnc2.add( EmaFactory.createFilterEntry().elementList( EmaRdm.SERVICE_STATE_ID, FilterEntry.FilterAction.SET, serviceState2 ) );
            mapEnc2 = EmaFactory.createMap();
            mapEnc2.add( EmaFactory.createMapEntry().keyUInt( 2, MapEntry.MapAction.UPDATE, filterListEnc2 ));
            updateMsg2 = EmaFactory.createUpdateMsg();
            ommprovider_3.submit( updateMsg2.domainType( EmaRdm.MMT_DIRECTORY ).
                    filter( EmaRdm.SERVICE_STATE_FILTER ).
                    payload( mapEnc2 ), 0);	// use 0 item handle to fan-out to all subscribers

            Thread.sleep(5000);

            assertEquals(1, consumerClient.queueSize());

            message = consumerClient.popMessage();
            ci = consumerClient.popChannelInfo();

            assertTrue(message instanceof StatusMsg);
            assertEquals("Channel_6", ci.channelName());
            assertTrue(((StatusMsg)message).state().toString().contains("Open / Ok / None /"));

            Thread.sleep(20000);

            assertEquals(2, consumerClient.queueSize());

            message = consumerClient.popMessage();
            ci = consumerClient.popChannelInfo();

            assertTrue(message instanceof StatusMsg);
            assertEquals("Channel_6", ci.channelName());
            assertTrue(((StatusMsg)message).state().statusText().toString().contains("preferred host starting fallback"));

            message = consumerClient.popMessage();
            ci = consumerClient.popChannelInfo();

            assertTrue(message instanceof StatusMsg);
            assertEquals("Channel_6", ci.channelName());
            assertTrue(((StatusMsg)message).state().statusText().toString().contains("preferred host complete"));

            ommprovider_3.uninitialize();
            ommprovider_6.uninitialize();

            Thread.sleep(10000);

            boolean loginRefreshFound = false;
            ibmFound = false;
            triFound = false;
            int count = consumerClient.queueSize();

            for (int i = 0; i < count; i++)
            {
                message = consumerClient.popMessage();
                ci = consumerClient.popChannelInfo();

                if (message instanceof RefreshMsg)
                {
                    assertEquals("Channel_7", ci.channelName());
                    RefreshMsg refresh = (RefreshMsg)message;
                    if (refresh.domainType() == DomainTypes.LOGIN)
                    {
                        loginRefreshFound = true;
                    }
                    else if (refresh.domainType() == DomainTypes.MARKET_PRICE)
                    {
                        if (refresh.name().equals("TRI.N"))
                        {
                            triFound = true;
                            assertTrue(refresh.solicited());
                            assertEquals("DIRECT_FEED1", refresh.serviceName());
                        }
                        else if (refresh.name().equals("IBM.N"))
                        {
                            ibmFound = true;
                            assertTrue(refresh.solicited());;
                            assertEquals("DIRECT_FEED", refresh.serviceName());
                        }
                    }
                }
            }

            assertTrue(ibmFound && triFound && loginRefreshFound);

            providerClient_3 = new ProviderTestClient(providerTestOptions);
            providerClient_3.name = "Provider_3";
            providerClient_6 = new ProviderTestClient(providerTestOptions);
            providerClient_6.name = "Provider_6";

            ommprovider_3 = EmaFactory.createOmmProvider(config.port("19003").providerName("Provider_9"), providerClient_3);
            assertNotNull(ommprovider_3);
            ommprovider_6 = EmaFactory.createOmmProvider(config.port("19006").providerName("Provider_9"), providerClient_6);
            assertNotNull(ommprovider_6);

            Thread.sleep(30000); // Wait for switch to WSB-G1 during detectionTimeInterval recovery

            boolean phStartFound = false;
            boolean phCompleteFound = false;
            count = consumerClient.queueSize();

            for (int i = 0; i < count; i++)
            {
                message = consumerClient.popMessage();
                ci = consumerClient.popChannelInfo();

                if (message instanceof StatusMsg)
                {
                    if (((StatusMsg)message).state().statusText().toString().contains("preferred host starting fallback"))
                    {
                        phStartFound = true;
                    }
                    else if (((StatusMsg)message).state().statusText().toString().contains("preferred host complete"))
                    {
                        phCompleteFound = true;
                        assertEquals("Channel_6", ci.channelName());
                    }
                }
            }

            assertTrue(phStartFound && phCompleteFound);
        }
        catch (Exception e)
        {
            e.printStackTrace();
            assertFalse(true);
        }
        finally
        {
            System.out.println("Uninitializing...");
            if (consumer != null) consumer.uninitialize();

            if (ommprovider_3 != null) ommprovider_3.uninitialize();
            if (ommprovider_6 != null) ommprovider_6.uninitialize();
            if (ommprovider_7 != null) ommprovider_7.uninitialize();
            if (ommprovider_8 != null) ommprovider_8.uninitialize();
        }
    }
}
