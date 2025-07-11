package com.refinitiv.eta.valueadd.reactor;

public class JUnitConfigVariables
{
    public static final int WAIT_AFTER_TEST; // Time in milliseconds
    public static final int SERVER_BIND_RETRY_COUNT; // The number of times server tries to bind to the socket before it fails.
    public static final int SLEEP_SERVER_WAIT;
    public static final int REACTOR_TEST_SLEEP_TIMEOUT;
    public static final int VA_MULTITHREADED_SLEEP_TIME;

    public static final int TEST_RETRY_COUNT;

    static
    {
        int waitAfterTestVal = 500;
        int serverBindRetryCountVal = 5;
        int sleepServerWaitVal = 50;
        int reactorSleepTimeoutVal = 700;
        int testRetryCountVal = 3;
        int vaMultithreadedSleepTimeVal = 15;

        String waitAfterTest = System.getProperty("junitWaitAfterTest");
        String serverBindRetryCount = System.getProperty("junitServerBindRetryCount");
        String sleepServerWait = System.getProperty("junitSleepServerWait");
        String reactorSleepTimeout = System.getProperty("junitReactorSleepTimeout");
        String testRetryCount = System.getProperty("junitTestRetryCount");
        String vaMultithreadedSleepTime = System.getProperty("junitvaMultithreadedSleepTime");

        try {
            if (waitAfterTest != null) waitAfterTestVal = Integer.parseInt(waitAfterTest);
            if (serverBindRetryCount != null) serverBindRetryCountVal = Integer.parseInt(serverBindRetryCount);
            if (sleepServerWait != null) sleepServerWaitVal = Integer.parseInt(sleepServerWait);
            if (reactorSleepTimeout != null) reactorSleepTimeoutVal = Integer.parseInt(reactorSleepTimeout);
            if (testRetryCount != null) testRetryCountVal = Integer.parseInt(testRetryCount);
            if (vaMultithreadedSleepTime != null) vaMultithreadedSleepTimeVal = Integer.parseInt(vaMultithreadedSleepTime);
        }
        catch (Exception e) {}

        WAIT_AFTER_TEST = waitAfterTestVal;
        SERVER_BIND_RETRY_COUNT = serverBindRetryCountVal;
        SLEEP_SERVER_WAIT = sleepServerWaitVal;
        REACTOR_TEST_SLEEP_TIMEOUT = reactorSleepTimeoutVal;
        TEST_RETRY_COUNT = testRetryCountVal;
        VA_MULTITHREADED_SLEEP_TIME = vaMultithreadedSleepTimeVal;
    }
}
