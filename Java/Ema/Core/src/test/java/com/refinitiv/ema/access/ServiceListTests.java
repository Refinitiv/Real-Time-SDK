package com.refinitiv.ema.access;

import com.refinitiv.ema.JUnitConfigVariables;
import org.junit.After;
import org.junit.Test;

import static org.junit.Assert.*;

public class ServiceListTests {

    @After
    public void tearDown()
    {
        try { Thread.sleep(JUnitConfigVariables.WAIT_AFTER_TEST); }
        catch (Exception e) { }
    }

    @Test
    public void testCreateAndClearServiceList()
    {
        ServiceList serviceList = EmaFactory.createServiceList("serviceList1");

        serviceList.concreteServiceList().add("FEED_1");
        serviceList.concreteServiceList().add("FEED_2");
        serviceList.concreteServiceList().add("FEED_3");
        assertEquals("serviceList1", serviceList.name());

        String expectedString = "Service list name: serviceList1"
                + "\nConcrete service names:"
                + "\n\tFEED_1"
                + "\n\tFEED_2"
                + "\n\tFEED_3";

        assertEquals(expectedString, serviceList.toString());

        serviceList.clear();

        expectedString = "Service list name: "
                + "\nConcrete service names:";

        assertEquals(expectedString, serviceList.toString());

        serviceList.name("serviceList2");
        serviceList.concreteServiceList().add("FEED_4");
        serviceList.concreteServiceList().add("FEED_5");
        serviceList.concreteServiceList().add("FEED_6");

        expectedString = "Service list name: serviceList2"
                + "\nConcrete service names:"
                + "\n\tFEED_4"
                + "\n\tFEED_5"
                + "\n\tFEED_6";

        assertEquals(expectedString, serviceList.toString());
    }

    @Test
    public void testAddEmptyOrNullServiceListName()
    {
        ServiceList serviceList = EmaFactory.createServiceList("");

        OmmConsumerConfig consumerConfig = EmaFactory.createOmmConsumerConfig();

        Exception exception = assertThrows(OmmInvalidUsageException.class,  () -> consumerConfig.addServiceList(serviceList));

        assertEquals("The ServiceList's name must be non-empty string value.", exception.getLocalizedMessage());

        serviceList.name(null);

        exception = assertThrows(OmmInvalidUsageException.class,  () -> consumerConfig.addServiceList(serviceList));

        assertEquals("The ServiceList's name must be non-empty string value.", exception.getLocalizedMessage());
    }

    @Test
    public void testDuplicateServiceListName()
    {
        ServiceList serviceList = EmaFactory.createServiceList("ServiceGroup");

        serviceList.concreteServiceList().add("A");
        serviceList.concreteServiceList().add("B");

        ServiceList serviceList2 = EmaFactory.createServiceList("ServiceGroup");

        serviceList.concreteServiceList().add("C");
        serviceList.concreteServiceList().add("D");

        OmmConsumerConfig consumerConfig = EmaFactory.createOmmConsumerConfig();

        consumerConfig.addServiceList(serviceList);

        Exception exception = assertThrows(OmmInvalidUsageException.class,  () -> consumerConfig.addServiceList(serviceList2));

        assertEquals("The ServiceGroup name of ServiceList has been added to OmmConsumerConfig.", exception.getLocalizedMessage());
    }

    @Test
    public void testSetandGetReqMsgWithServiceNameList()
    {
        ReqMsg reqMsg = EmaFactory.createReqMsg();

        String serviceListName = "ServiceList1";

        reqMsg.serviceListName(serviceListName);

        assertTrue(reqMsg.hasServiceListName());

        assertEquals(serviceListName, reqMsg.serviceListName());

        reqMsg.clear();

        assertFalse(reqMsg.hasServiceListName());
        assertNull(reqMsg.serviceListName());
    }

    @Test
    public void testSetServiceListNameAfterSettingServiceNameOrServiceId()
    {
        ReqMsg reqMsg = EmaFactory.createReqMsg();

        String serviceListName = "ServiceList1";

        reqMsg.serviceName("DIRECT_FEED");

        Exception exception = assertThrows(OmmInvalidUsageException.class,  () -> reqMsg.serviceListName(serviceListName));

        assertEquals("Service name is already set for this ReqMsg.", exception.getLocalizedMessage());

        reqMsg.clear();

        reqMsg.serviceId(1);

        exception = assertThrows(OmmInvalidUsageException.class,  () -> reqMsg.serviceListName(serviceListName));

        assertEquals("Service Id is already set for this ReqMsg.", exception.getLocalizedMessage());
    }
}
