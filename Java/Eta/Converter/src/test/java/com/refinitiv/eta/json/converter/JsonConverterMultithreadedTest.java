/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.json.converter;

import com.refinitiv.eta.codec.*;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.rdm.UpdateEventTypes;
import com.refinitiv.eta.transport.TransportFactory;
import org.junit.Before;
import org.junit.Test;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.*;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import static com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS;
import static org.junit.Assert.*;

/**
 * Multithreaded test for JSON Converter RWF to JSON conversion.
 *
 * This test ensures that spinning up multiple JSON Converters in a multithreaded
 * scenario does not cause issues. Previously, there were concurrency problems
 * when multiple converters were created and used simultaneously. This test
 * validates that the fix for those issues is working correctly.
 */
public class JsonConverterMultithreadedTest {

    private static final int THREAD_COUNT = 8;
    private static final int MESSAGES_PER_THREAD = 100000;
    private static final int QUEUE_CAPACITY = 1000;
    private static final int TEST_TIMEOUT_SECONDS = 600;
    private static final int MSG_BUFFER_SIZE = 4096;

    // Patterns for extracting values from JSON output to verify correctness
    private static final Pattern STREAM_ID_PATTERN = Pattern.compile("\"ID\"\\s*:\\s*(\\d+)");
    private static final Pattern KEY_NAME_PATTERN = Pattern.compile("\"Name\"\\s*:\\s*\"([^\"]+)\"");

    private DataDictionary dictionary;
    private BlockingQueue<TestMessage> inputQueue;
    private BlockingQueue<ConversionResult> resultQueue;
    private final AtomicInteger successCount = new AtomicInteger(0);
    private final AtomicInteger failureCount = new AtomicInteger(0);
    private final AtomicInteger dataCorruptionCount = new AtomicInteger(0);

    /**
     * Wrapper class to hold a message and its expected stream ID for verification.
     */
    private static class TestMessage {
        final Msg rwfMsg;
        final int expectedStreamId;
        final String expectedKeyName;
        final int threadId;
        final int messageIndex;

        TestMessage(Msg rwfMsg, int expectedStreamId, String expectedKeyName, int threadId, int messageIndex) {
            this.rwfMsg = rwfMsg;
            this.expectedStreamId = expectedStreamId;
            this.expectedKeyName = expectedKeyName;
            this.threadId = threadId;
            this.messageIndex = messageIndex;
        }
    }

    /**
     * Wrapper class to hold conversion results.
     */
    private static class ConversionResult {
        final boolean success;
        final boolean dataCorrupted;
        final int streamId;
        final int threadId;
        final int messageIndex;
        final String errorMessage;
        final String jsonOutput;

        ConversionResult(boolean success, boolean dataCorrupted, int streamId, int threadId, int messageIndex, String errorMessage, String jsonOutput) {
            this.success = success;
            this.dataCorrupted = dataCorrupted;
            this.streamId = streamId;
            this.threadId = threadId;
            this.messageIndex = messageIndex;
            this.errorMessage = errorMessage;
            this.jsonOutput = jsonOutput;
        }
    }

    @Before
    public void init() {
        dictionary = CodecFactory.createDataDictionary();
        final String dictionaryFileName = "../../etc/RDMFieldDictionary";
        com.refinitiv.eta.transport.Error error = TransportFactory.createError();
        dictionary.clear();
        dictionary.loadFieldDictionary(dictionaryFileName, error);

        inputQueue = new LinkedBlockingQueue<>(QUEUE_CAPACITY);
        resultQueue = new LinkedBlockingQueue<>(QUEUE_CAPACITY);
        successCount.set(0);
        failureCount.set(0);
        dataCorruptionCount.set(0);
    }

    /**
     * Test that multiple JSON Converters can be created and used concurrently
     * without interfering with each other.
     */
    @Test
    public void testMultipleConvertersInParallel() throws InterruptedException, ExecutionException, TimeoutException {
        ExecutorService executor = Executors.newFixedThreadPool(THREAD_COUNT);
        List<Future<ConverterWorkerResult>> futures = new ArrayList<>();

        // Start multiple converter workers, each with its own converter instance
        for (int i = 0; i < THREAD_COUNT; i++) {
            final int threadId = i;
            futures.add(executor.submit(() -> runConverterWorker(threadId)));
        }

        // Wait for all workers to complete
        executor.shutdown();
        assertTrue("Executor should terminate within timeout",
                   executor.awaitTermination(TEST_TIMEOUT_SECONDS, TimeUnit.SECONDS));

        // Verify results
        int totalSuccess = 0;
        int totalFailure = 0;
        int totalProcessed = 0;
        int totalDataCorruption = 0;

        for (Future<ConverterWorkerResult> future : futures) {
            ConverterWorkerResult result = future.get(5, TimeUnit.SECONDS);
            totalSuccess += result.successCount;
            totalFailure += result.failureCount;
            totalProcessed += result.processedCount;
            totalDataCorruption += result.dataCorruptionCount;
        }

        assertEquals("All messages should be processed", THREAD_COUNT * MESSAGES_PER_THREAD, totalProcessed);
        assertEquals("No data corruption should occur (thread interference)", 0, totalDataCorruption);
        assertEquals("All conversions should succeed", THREAD_COUNT * MESSAGES_PER_THREAD, totalSuccess);
        assertEquals("No conversions should fail", 0, totalFailure);
    }

    /**
     * Test producer-consumer pattern where multiple producers add messages to a queue
     * and multiple consumers (each with their own converter) process them.
     */
    @Test
    public void testProducerConsumerWithMultipleConverters() throws InterruptedException {
        int producerCount = 4;
        int consumerCount = 4;
        int messagesPerProducer = 50;

        ExecutorService executor = Executors.newFixedThreadPool(producerCount + consumerCount);
        CountDownLatch producersDone = new CountDownLatch(producerCount);
        CountDownLatch consumersDone = new CountDownLatch(consumerCount);
        AtomicInteger messagesProduced = new AtomicInteger(0);
        AtomicInteger messagesConsumed = new AtomicInteger(0);

        // Start producers
        for (int i = 0; i < producerCount; i++) {
            final int producerId = i;
            executor.submit(() -> {
                try {
                    for (int j = 0; j < messagesPerProducer; j++) {
                        int streamId = producerId * 1000 + j;
                        String keyName = "P" + producerId + "_M" + j + ".RIC";
                        Msg msg = createUpdateMessage(streamId, DomainTypes.MARKET_PRICE, keyName);
                        inputQueue.put(new TestMessage(msg, streamId, keyName, producerId, j));
                        messagesProduced.incrementAndGet();
                    }
                } catch (InterruptedException e) {
                    Thread.currentThread().interrupt();
                } finally {
                    producersDone.countDown();
                }
            });
        }

        // Start consumers (each with their own JsonConverter)
        for (int i = 0; i < consumerCount; i++) {
            final int consumerId = i;
            executor.submit(() -> {
                JsonConverter converter = createConverter();
                assertNotNull("Converter should be created successfully for consumer " + consumerId, converter);

                try {
                    while (true) {
                        TestMessage testMsg = inputQueue.poll(100, TimeUnit.MILLISECONDS);
                        if (testMsg == null) {
                            // Check if producers are done and queue is empty
                            if (producersDone.getCount() == 0 && inputQueue.isEmpty()) {
                                break;
                            }
                            continue;
                        }

                        ConversionResult result = convertMessage(converter, testMsg);
                        resultQueue.put(result);
                        messagesConsumed.incrementAndGet();

                        if (result.success) {
                            successCount.incrementAndGet();
                        } else {
                            failureCount.incrementAndGet();
                        }
                    }
                } catch (InterruptedException e) {
                    Thread.currentThread().interrupt();
                } finally {
                    consumersDone.countDown();
                }
            });
        }

        // Wait for producers to finish
        assertTrue("Producers should complete within timeout",
                   producersDone.await(TEST_TIMEOUT_SECONDS, TimeUnit.SECONDS));

        // Wait for consumers to finish
        assertTrue("Consumers should complete within timeout",
                   consumersDone.await(TEST_TIMEOUT_SECONDS, TimeUnit.SECONDS));

        executor.shutdown();
        assertTrue("Executor should terminate",
                   executor.awaitTermination(5, TimeUnit.SECONDS));

        int totalExpected = producerCount * messagesPerProducer;
        assertEquals("All produced messages should be consumed", totalExpected, messagesConsumed.get());
        assertEquals("All conversions should succeed", totalExpected, successCount.get());
        assertEquals("No conversions should fail", 0, failureCount.get());

        // Verify all results
        assertEquals("Result queue should have all results", totalExpected, resultQueue.size());
    }

    /**
     * Test that converters can be created and destroyed rapidly without issues.
     */
    @Test
    public void testRapidConverterCreationAndDestruction() throws InterruptedException {
        int iterations = 5000;
        ExecutorService executor = Executors.newFixedThreadPool(THREAD_COUNT);
        CountDownLatch latch = new CountDownLatch(THREAD_COUNT);
        AtomicInteger totalConversions = new AtomicInteger(0);
        AtomicInteger totalErrors = new AtomicInteger(0);

        for (int t = 0; t < THREAD_COUNT; t++) {
            final int threadId = t;
            executor.submit(() -> {
                try {
                    for (int i = 0; i < iterations; i++) {
                        // Create a new converter for each iteration
                        JsonConverter converter = createConverter();
                        assertNotNull("Converter should be created for thread " + threadId + " iteration " + i, converter);

                        // Do some conversions with this converter
                        for (int j = 0; j < 5; j++) {
                            int streamId = threadId * 10000 + i * 100 + j;
                            Msg msg = createUpdateMessage(streamId, DomainTypes.MARKET_PRICE);

                            JsonConverterError convError = ConverterFactory.createJsonConverterError();
                            RWFToJsonOptions rwfToJsonOptions = ConverterFactory.createRWFToJsonOptions();
                            rwfToJsonOptions.setJsonProtocolType(JsonProtocol.JSON_JPT_JSON2);
                            ConversionResults convRes = ConverterFactory.createConversionResults();

                            int result = converter.convertRWFToJson(msg, rwfToJsonOptions, convRes, convError);
                            if (result == SUCCESS) {
                                totalConversions.incrementAndGet();
                            } else {
                                totalErrors.incrementAndGet();
                            }
                        }
                        // Converter goes out of scope here - simulating destruction
                    }
                } finally {
                    latch.countDown();
                }
            });
        }

        assertTrue("All threads should complete within timeout",
                   latch.await(TEST_TIMEOUT_SECONDS, TimeUnit.SECONDS));

        executor.shutdown();
        assertTrue("Executor should terminate",
                   executor.awaitTermination(5, TimeUnit.SECONDS));

        int expectedConversions = THREAD_COUNT * iterations * 5;
        assertEquals("All conversions should succeed", expectedConversions, totalConversions.get());
        assertEquals("No errors should occur", 0, totalErrors.get());
    }

    /**
     * Test concurrent RWF to JSON and JSON to RWF conversions.
     */
    @Test
    public void testBidirectionalConversionsConcurrently() throws InterruptedException {
        int conversionsPerThread = 50;
        ExecutorService executor = Executors.newFixedThreadPool(THREAD_COUNT);
        CountDownLatch latch = new CountDownLatch(THREAD_COUNT);
        AtomicInteger successfulRoundTrips = new AtomicInteger(0);
        AtomicInteger failedRoundTrips = new AtomicInteger(0);

        for (int t = 0; t < THREAD_COUNT; t++) {
            final int threadId = t;
            executor.submit(() -> {
                JsonConverter converter = createConverter();
                assertNotNull("Converter should be created for thread " + threadId, converter);

                try {
                    for (int i = 0; i < conversionsPerThread; i++) {
                        int streamId = threadId * 1000 + i;
                        boolean success = performRoundTripConversion(converter, streamId);
                        if (success) {
                            successfulRoundTrips.incrementAndGet();
                        } else {
                            failedRoundTrips.incrementAndGet();
                        }
                    }
                } finally {
                    latch.countDown();
                }
            });
        }

        assertTrue("All threads should complete within timeout",
                   latch.await(TEST_TIMEOUT_SECONDS, TimeUnit.SECONDS));

        executor.shutdown();
        assertTrue("Executor should terminate",
                   executor.awaitTermination(5, TimeUnit.SECONDS));

        int expectedRoundTrips = THREAD_COUNT * conversionsPerThread;
        assertEquals("All round-trip conversions should succeed", expectedRoundTrips, successfulRoundTrips.get());
        assertEquals("No round-trip conversions should fail", 0, failedRoundTrips.get());
    }

    /**
     * Test that different message types can be converted concurrently.
     */
    @Test
    public void testMixedMessageTypesConcurrently() throws InterruptedException {
        int messagesPerThread = 30;
        ExecutorService executor = Executors.newFixedThreadPool(THREAD_COUNT);
        CountDownLatch latch = new CountDownLatch(THREAD_COUNT);
        AtomicInteger successCount = new AtomicInteger(0);
        AtomicInteger failureCount = new AtomicInteger(0);

        for (int t = 0; t < THREAD_COUNT; t++) {
            final int threadId = t;
            executor.submit(() -> {
                JsonConverter converter = createConverter();
                assertNotNull("Converter should be created for thread " + threadId, converter);

                try {
                    for (int i = 0; i < messagesPerThread; i++) {
                        int streamId = threadId * 1000 + i;
                        Msg msg;

                        // Alternate between different message types
                        switch (i % 3) {
                            case 0:
                                msg = createUpdateMessage(streamId, DomainTypes.MARKET_PRICE);
                                break;
                            case 1:
                                msg = createRefreshMessage(streamId, DomainTypes.MARKET_PRICE);
                                break;
                            default:
                                msg = createStatusMessage(streamId, DomainTypes.MARKET_PRICE);
                                break;
                        }

                        JsonConverterError convError = ConverterFactory.createJsonConverterError();
                        RWFToJsonOptions rwfToJsonOptions = ConverterFactory.createRWFToJsonOptions();
                        rwfToJsonOptions.setJsonProtocolType(JsonProtocol.JSON_JPT_JSON2);
                        ConversionResults convRes = ConverterFactory.createConversionResults();

                        int result = converter.convertRWFToJson(msg, rwfToJsonOptions, convRes, convError);
                        if (result == SUCCESS) {
                            // Also verify we can get the JSON buffer
                            Buffer jsonBuffer = CodecFactory.createBuffer();
                            jsonBuffer.data(ByteBuffer.allocate(MSG_BUFFER_SIZE));
                            GetJsonMsgOptions getJsonMsgOptions = ConverterFactory.createGetJsonMsgOptions();
                            getJsonMsgOptions.jsonProtocolType(JsonProtocol.JSON_JPT_JSON2);
                            getJsonMsgOptions.streamId(streamId);

                            int getResult = converter.getJsonBuffer(jsonBuffer, getJsonMsgOptions, convError);
                            if (getResult == SUCCESS && jsonBuffer.length() > 0) {
                                successCount.incrementAndGet();
                            } else {
                                failureCount.incrementAndGet();
                            }
                        } else {
                            failureCount.incrementAndGet();
                        }
                    }
                } finally {
                    latch.countDown();
                }
            });
        }

        assertTrue("All threads should complete within timeout",
                   latch.await(TEST_TIMEOUT_SECONDS, TimeUnit.SECONDS));

        executor.shutdown();
        assertTrue("Executor should terminate",
                   executor.awaitTermination(5, TimeUnit.SECONDS));

        int expected = THREAD_COUNT * messagesPerThread;
        assertEquals("All conversions should succeed", expected, successCount.get());
        assertEquals("No conversions should fail", 0, failureCount.get());
    }

    /**
     * CRITICAL TEST: Test that a SINGLE shared converter can be used across multiple threads.
     * This is the most likely scenario to expose thread interference issues, as all threads
     * share the same internal state/buffers of the converter.
     *
     * If there is any shared mutable state that isn't thread-safe, this test will catch it
     * by detecting data corruption (wrong stream IDs or key names in output).
     */
    @Test
    public void testSharedConverterAcrossMultipleThreads() throws InterruptedException {
        int messagesPerThread = 10000;

        // Create a SINGLE converter to be shared across all threads
        final JsonConverter sharedConverter = createConverter();
        assertNotNull("Shared converter should be created successfully", sharedConverter);

        ExecutorService executor = Executors.newFixedThreadPool(THREAD_COUNT);
        CountDownLatch latch = new CountDownLatch(THREAD_COUNT);
        AtomicInteger successCount = new AtomicInteger(0);
        AtomicInteger failureCount = new AtomicInteger(0);
        AtomicInteger dataCorruptionCount = new AtomicInteger(0);

        for (int t = 0; t < THREAD_COUNT; t++) {
            final int threadId = t;
            executor.submit(() -> {
                try {
                    for (int i = 0; i < messagesPerThread; i++) {
                        int streamId = threadId * 1000000 + i;
                        String expectedKeyName = "SHARED_T" + threadId + "_M" + i + ".RIC";
                        Msg msg = createUpdateMessage(streamId, DomainTypes.MARKET_PRICE, expectedKeyName);

                        JsonConverterError convError = ConverterFactory.createJsonConverterError();
                        RWFToJsonOptions rwfToJsonOptions = ConverterFactory.createRWFToJsonOptions();
                        rwfToJsonOptions.setJsonProtocolType(JsonProtocol.JSON_JPT_JSON2);
                        ConversionResults convRes = ConverterFactory.createConversionResults();

                        // Use the SHARED converter
                        int result = sharedConverter.convertRWFToJson(msg, rwfToJsonOptions, convRes, convError);

                        if (result == SUCCESS) {
                            Buffer jsonBuffer = CodecFactory.createBuffer();
                            jsonBuffer.data(ByteBuffer.allocate(MSG_BUFFER_SIZE));
                            GetJsonMsgOptions getJsonMsgOptions = ConverterFactory.createGetJsonMsgOptions();
                            getJsonMsgOptions.jsonProtocolType(JsonProtocol.JSON_JPT_JSON2);
                            getJsonMsgOptions.streamId(streamId);

                            int getResult = sharedConverter.getJsonBuffer(jsonBuffer, getJsonMsgOptions, convError);
                            if (getResult == SUCCESS && jsonBuffer.length() > 0) {
                                String jsonOutput = jsonBuffer.toString();

                                // Validate stream ID
                                Matcher streamIdMatcher = STREAM_ID_PATTERN.matcher(jsonOutput);
                                if (streamIdMatcher.find()) {
                                    int foundStreamId = Integer.parseInt(streamIdMatcher.group(1));
                                    if (foundStreamId != streamId) {
                                        dataCorruptionCount.incrementAndGet();
                                        System.err.println("SHARED CONVERTER DATA CORRUPTION: Thread " + threadId +
                                                " msg " + i + " expected streamId " + streamId +
                                                " but found " + foundStreamId);
                                        continue;
                                    }
                                }

                                // Validate key name
                                Matcher keyNameMatcher = KEY_NAME_PATTERN.matcher(jsonOutput);
                                if (keyNameMatcher.find()) {
                                    String foundKeyName = keyNameMatcher.group(1);
                                    if (!expectedKeyName.equals(foundKeyName)) {
                                        dataCorruptionCount.incrementAndGet();
                                        System.err.println("SHARED CONVERTER DATA CORRUPTION: Thread " + threadId +
                                                " msg " + i + " expected keyName '" + expectedKeyName +
                                                "' but found '" + foundKeyName + "'");
                                        continue;
                                    }
                                }

                                successCount.incrementAndGet();
                            } else {
                                failureCount.incrementAndGet();
                            }
                        } else {
                            failureCount.incrementAndGet();
                        }
                    }
                } finally {
                    latch.countDown();
                }
            });
        }

        assertTrue("All threads should complete within timeout",
                   latch.await(TEST_TIMEOUT_SECONDS, TimeUnit.SECONDS));

        executor.shutdown();
        assertTrue("Executor should terminate",
                   executor.awaitTermination(5, TimeUnit.SECONDS));

        int expected = THREAD_COUNT * messagesPerThread;
        assertEquals("No data corruption should occur with shared converter", 0, dataCorruptionCount.get());
        assertEquals("All conversions should succeed with shared converter", expected, successCount.get());
        assertEquals("No conversions should fail with shared converter", 0, failureCount.get());
    }

    // ==================== Helper Methods ====================

    private static class ConverterWorkerResult {
        final int successCount;
        final int failureCount;
        final int processedCount;
        final int dataCorruptionCount;

        ConverterWorkerResult(int successCount, int failureCount, int processedCount, int dataCorruptionCount) {
            this.successCount = successCount;
            this.failureCount = failureCount;
            this.processedCount = processedCount;
            this.dataCorruptionCount = dataCorruptionCount;
        }
    }

    private ConverterWorkerResult runConverterWorker(int threadId) {
        JsonConverter converter = createConverter();
        assertNotNull("Converter should be created successfully for thread " + threadId, converter);

        int localSuccess = 0;
        int localFailure = 0;
        int processed = 0;
        int localDataCorruption = 0;

        for (int i = 0; i < MESSAGES_PER_THREAD; i++) {
            int streamId = threadId * 1000000 + i;
            String expectedKeyName = "T" + threadId + "_M" + i + ".RIC";
            Msg msg = createUpdateMessage(streamId, DomainTypes.MARKET_PRICE, expectedKeyName);

            JsonConverterError convError = ConverterFactory.createJsonConverterError();
            RWFToJsonOptions rwfToJsonOptions = ConverterFactory.createRWFToJsonOptions();
            rwfToJsonOptions.setJsonProtocolType(JsonProtocol.JSON_JPT_JSON2);
            ConversionResults convRes = ConverterFactory.createConversionResults();

            int result = converter.convertRWFToJson(msg, rwfToJsonOptions, convRes, convError);
            processed++;

            if (result == SUCCESS) {
                // Also get the JSON buffer to verify conversion completed
                Buffer jsonBuffer = CodecFactory.createBuffer();
                jsonBuffer.data(ByteBuffer.allocate(MSG_BUFFER_SIZE));
                GetJsonMsgOptions getJsonMsgOptions = ConverterFactory.createGetJsonMsgOptions();
                getJsonMsgOptions.jsonProtocolType(JsonProtocol.JSON_JPT_JSON2);
                getJsonMsgOptions.streamId(streamId);

                int getResult = converter.getJsonBuffer(jsonBuffer, getJsonMsgOptions, convError);
                if (getResult == SUCCESS && jsonBuffer.length() > 0) {
                    // CRITICAL: Validate the JSON content matches expected values
                    String jsonOutput = jsonBuffer.toString();

                    // Extract and validate stream ID from JSON
                    Matcher streamIdMatcher = STREAM_ID_PATTERN.matcher(jsonOutput);
                    if (streamIdMatcher.find()) {
                        int foundStreamId = Integer.parseInt(streamIdMatcher.group(1));
                        if (foundStreamId != streamId) {
                            localDataCorruption++;
                            System.err.println("DATA CORRUPTION: Thread " + threadId + " msg " + i +
                                    " expected streamId " + streamId + " but found " + foundStreamId);
                            continue;
                        }
                    } else {
                        localDataCorruption++;
                        System.err.println("DATA CORRUPTION: Thread " + threadId + " msg " + i +
                                " - could not find stream ID in JSON: " + jsonOutput);
                        continue;
                    }

                    // Extract and validate key name from JSON
                    Matcher keyNameMatcher = KEY_NAME_PATTERN.matcher(jsonOutput);
                    if (keyNameMatcher.find()) {
                        String foundKeyName = keyNameMatcher.group(1);
                        if (!expectedKeyName.equals(foundKeyName)) {
                            localDataCorruption++;
                            System.err.println("DATA CORRUPTION: Thread " + threadId + " msg " + i +
                                    " expected keyName '" + expectedKeyName + "' but found '" + foundKeyName + "'");
                            continue;
                        }
                    } else {
                        localDataCorruption++;
                        System.err.println("DATA CORRUPTION: Thread " + threadId + " msg " + i +
                                " - could not find key name in JSON: " + jsonOutput);
                        continue;
                    }

                    localSuccess++;
                } else {
                    localFailure++;
                }
            } else {
                localFailure++;
            }
        }

        return new ConverterWorkerResult(localSuccess, localFailure, processed, localDataCorruption);
    }

    private JsonConverter createConverter() {
        JsonConverterError convError = ConverterFactory.createJsonConverterError();
        return ConverterFactory.createJsonConverterBuilder()
                .setProperty(JsonConverterProperties.JSON_CPC_CATCH_UNKNOWN_JSON_KEYS, false)
                .setProperty(JsonConverterProperties.JSON_CPC_PROTOCOL_VERSION, JsonProtocol.JSON_JPT_JSON2)
                .setServiceConverter(new ServiceNameIdTestConverter())
                .setDictionary(dictionary)
                .build(convError);
    }

    private ConversionResult convertMessage(JsonConverter converter, TestMessage testMsg) {
        JsonConverterError convError = ConverterFactory.createJsonConverterError();
        RWFToJsonOptions rwfToJsonOptions = ConverterFactory.createRWFToJsonOptions();
        rwfToJsonOptions.setJsonProtocolType(JsonProtocol.JSON_JPT_JSON2);
        ConversionResults convRes = ConverterFactory.createConversionResults();

        int result = converter.convertRWFToJson(testMsg.rwfMsg, rwfToJsonOptions, convRes, convError);

        if (result != SUCCESS) {
            return new ConversionResult(false, false, testMsg.expectedStreamId, testMsg.threadId,
                                       testMsg.messageIndex, convError.getText(), null);
        }

        Buffer jsonBuffer = CodecFactory.createBuffer();
        jsonBuffer.data(ByteBuffer.allocate(MSG_BUFFER_SIZE));
        GetJsonMsgOptions getJsonMsgOptions = ConverterFactory.createGetJsonMsgOptions();
        getJsonMsgOptions.jsonProtocolType(JsonProtocol.JSON_JPT_JSON2);
        getJsonMsgOptions.streamId(testMsg.expectedStreamId);

        result = converter.getJsonBuffer(jsonBuffer, getJsonMsgOptions, convError);

        if (result != SUCCESS) {
            return new ConversionResult(false, false, testMsg.expectedStreamId, testMsg.threadId,
                                       testMsg.messageIndex, convError.getText(), null);
        }

        String jsonOutput = jsonBuffer.toString();

        // Validate stream ID in JSON
        Matcher streamIdMatcher = STREAM_ID_PATTERN.matcher(jsonOutput);
        if (streamIdMatcher.find()) {
            int foundStreamId = Integer.parseInt(streamIdMatcher.group(1));
            if (foundStreamId != testMsg.expectedStreamId) {
                return new ConversionResult(false, true, testMsg.expectedStreamId, testMsg.threadId,
                        testMsg.messageIndex, "Stream ID mismatch: expected " + testMsg.expectedStreamId +
                        " but found " + foundStreamId, jsonOutput);
            }
        }

        // Validate key name in JSON
        Matcher keyNameMatcher = KEY_NAME_PATTERN.matcher(jsonOutput);
        if (keyNameMatcher.find()) {
            String foundKeyName = keyNameMatcher.group(1);
            if (!testMsg.expectedKeyName.equals(foundKeyName)) {
                return new ConversionResult(false, true, testMsg.expectedStreamId, testMsg.threadId,
                        testMsg.messageIndex, "Key name mismatch: expected '" + testMsg.expectedKeyName +
                        "' but found '" + foundKeyName + "'", jsonOutput);
            }
        }

        return new ConversionResult(true, false, testMsg.expectedStreamId, testMsg.threadId,
                                   testMsg.messageIndex, null, jsonOutput);
    }

    private boolean performRoundTripConversion(JsonConverter converter, int streamId) {
        // Create original message
        Msg originalMsg = createUpdateMessage(streamId, DomainTypes.MARKET_PRICE);

        // Convert RWF to JSON
        JsonConverterError convError = ConverterFactory.createJsonConverterError();
        RWFToJsonOptions rwfToJsonOptions = ConverterFactory.createRWFToJsonOptions();
        rwfToJsonOptions.setJsonProtocolType(JsonProtocol.JSON_JPT_JSON2);
        ConversionResults convRes = ConverterFactory.createConversionResults();

        if (converter.convertRWFToJson(originalMsg, rwfToJsonOptions, convRes, convError) != SUCCESS) {
            return false;
        }

        // Get JSON buffer
        Buffer jsonBuffer = CodecFactory.createBuffer();
        jsonBuffer.data(ByteBuffer.allocate(MSG_BUFFER_SIZE));
        GetJsonMsgOptions getJsonMsgOptions = ConverterFactory.createGetJsonMsgOptions();
        getJsonMsgOptions.jsonProtocolType(JsonProtocol.JSON_JPT_JSON2);
        getJsonMsgOptions.streamId(streamId);

        if (converter.getJsonBuffer(jsonBuffer, getJsonMsgOptions, convError) != SUCCESS) {
            return false;
        }

        // Parse JSON back
        ParseJsonOptions parseJsonOptions = ConverterFactory.createParseJsonOptions();
        parseJsonOptions.setProtocolType(JsonProtocol.JSON_JPT_JSON2);

        if (converter.parseJsonBuffer(jsonBuffer, parseJsonOptions, convError) != SUCCESS) {
            return false;
        }

        // Decode JSON message
        JsonMsg jsonMsg = ConverterFactory.createJsonMsg();
        DecodeJsonMsgOptions decodeJsonMsgOptions = ConverterFactory.createDecodeJsonMsgOptions();
        decodeJsonMsgOptions.setJsonProtocolType(JsonProtocol.JSON_JPT_JSON2);

        if (converter.decodeJsonMsg(jsonMsg, decodeJsonMsgOptions, convError) != SUCCESS) {
            return false;
        }

        // Verify the decoded message matches the original
        Msg decodedMsg = CodecFactory.createMsg();
        DecodeIterator decodeIter = CodecFactory.createDecodeIterator();
        decodeIter.setBufferAndRWFVersion(jsonMsg.rwfMsg().encodedMsgBuffer(), Codec.majorVersion(), Codec.minorVersion());

        if (decodedMsg.decode(decodeIter) != SUCCESS) {
            return false;
        }

        // Basic verification
        return decodedMsg.msgClass() == originalMsg.msgClass() &&
               decodedMsg.streamId() == originalMsg.streamId() &&
               decodedMsg.domainType() == originalMsg.domainType();
    }

    private Msg createUpdateMessage(int streamId, int domainType) {
        return createUpdateMessage(streamId, domainType, "TEST.RIC");
    }

    private Msg createUpdateMessage(int streamId, int domainType, String keyName) {
        UpdateMsg updateMsg = (UpdateMsg) CodecFactory.createMsg();
        updateMsg.msgClass(MsgClasses.UPDATE);
        updateMsg.streamId(streamId);
        updateMsg.domainType(domainType);
        updateMsg.containerType(DataTypes.NO_DATA);
        updateMsg.updateType(UpdateEventTypes.QUOTE);

        Buffer emptyBuffer = CodecFactory.createBuffer();
        emptyBuffer.data(ByteBuffer.allocate(0));
        updateMsg.encodedDataBody(emptyBuffer);

        // Add message key
        updateMsg.applyHasMsgKey();
        updateMsg.msgKey().applyHasName();
        Buffer nameBuffer = CodecFactory.createBuffer();
        nameBuffer.data(keyName);
        updateMsg.msgKey().name(nameBuffer);
        updateMsg.msgKey().applyHasServiceId();
        updateMsg.msgKey().serviceId(1);

        return updateMsg;
    }

    private Msg createRefreshMessage(int streamId, int domainType) {
        RefreshMsg refreshMsg = (RefreshMsg) CodecFactory.createMsg();
        refreshMsg.msgClass(MsgClasses.REFRESH);
        refreshMsg.streamId(streamId);
        refreshMsg.domainType(domainType);
        refreshMsg.containerType(DataTypes.NO_DATA);

        Buffer emptyBuffer = CodecFactory.createBuffer();
        emptyBuffer.data(ByteBuffer.allocate(0));
        refreshMsg.encodedDataBody(emptyBuffer);

        // Set state
        refreshMsg.state().streamState(StreamStates.OPEN);
        refreshMsg.state().dataState(DataStates.OK);
        refreshMsg.state().code(StateCodes.NONE);
        Buffer stateText = CodecFactory.createBuffer();
        stateText.data("OK");
        refreshMsg.state().text(stateText);

        // Add message key
        refreshMsg.applyHasMsgKey();
        refreshMsg.msgKey().applyHasName();
        Buffer nameBuffer = CodecFactory.createBuffer();
        nameBuffer.data("TEST.RIC");
        refreshMsg.msgKey().name(nameBuffer);
        refreshMsg.msgKey().applyHasServiceId();
        refreshMsg.msgKey().serviceId(1);

        // Set QoS
        refreshMsg.applyHasQos();
        refreshMsg.qos().timeliness(QosTimeliness.REALTIME);
        refreshMsg.qos().rate(QosRates.TICK_BY_TICK);

        return refreshMsg;
    }

    private Msg createStatusMessage(int streamId, int domainType) {
        StatusMsg statusMsg = (StatusMsg) CodecFactory.createMsg();
        statusMsg.msgClass(MsgClasses.STATUS);
        statusMsg.streamId(streamId);
        statusMsg.domainType(domainType);
        statusMsg.containerType(DataTypes.NO_DATA);

        Buffer emptyBuffer = CodecFactory.createBuffer();
        emptyBuffer.data(ByteBuffer.allocate(0));
        statusMsg.encodedDataBody(emptyBuffer);

        // Set state
        statusMsg.applyHasState();
        statusMsg.state().streamState(StreamStates.OPEN);
        statusMsg.state().dataState(DataStates.OK);
        statusMsg.state().code(StateCodes.NONE);
        Buffer stateText = CodecFactory.createBuffer();
        stateText.data("OK");
        statusMsg.state().text(stateText);

        return statusMsg;
    }
}

