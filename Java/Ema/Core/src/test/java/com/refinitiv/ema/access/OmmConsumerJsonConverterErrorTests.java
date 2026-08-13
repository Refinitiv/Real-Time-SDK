/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;

import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.locks.ReentrantLock;

import org.junit.Test;

public class OmmConsumerJsonConverterErrorTests {

    @Test
    public void testHandleJsonConverterErrorUnlockFromNonOwnerThreadDoesNotThrowIllegalMonitorStateException() throws Exception {
        String emaConfigFileLocation = "./src/test/resources/com/refinitiv/ema/unittest/OmmConsumerTests/EmaConfigTest.xml";

        OmmConsumer consumer = JUnitTestConnect.createOmmConsumer(
                EmaFactory.createOmmConsumerConfig(emaConfigFileLocation).consumerName("Consumer_1"));
        OmmConsumerImpl consumerImpl = (OmmConsumerImpl) consumer;

        ReentrantLock userLock = consumerImpl.userLock();
        CountDownLatch lockAcquired = new CountDownLatch(1);
        CountDownLatch releaseLock = new CountDownLatch(1);

        Thread lockerThread = new Thread(() -> {
            userLock.lock();
            try {
                lockAcquired.countDown();
                releaseLock.await();
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            } finally {
                userLock.unlock();
            }
        });

        lockerThread.start();
        assertTrue("Failed to lock userLock from helper thread", lockAcquired.await(5, TimeUnit.SECONDS));

        try {
            try {
                consumerImpl.handleJsonConverterError(null, 1234, "forced test converter error");
            } catch (IllegalMonitorStateException e) {
                fail("Unexpected IllegalMonitorStateException from handleJsonConverterError: " + e.getMessage());
            }
        } finally {
            releaseLock.countDown();
            lockerThread.join(5000);
            consumer.uninitialize();
        }
    }
}

