package com.refinitiv.eta;

public class JUnitConfigVariables
{
    public static final int WAIT_AFTER_TEST; // Time in milliseconds
    public static final int SERVER_BIND_RETRY_COUNT; // The number of times server tries to bind to the socket before it fails.
    public static final int SLEEP_SERVER_WAIT;

    public static final int TEST_RETRY_COUNT;

    static
    {
        int waitAfterTestVal = 500;
        int serverBindRetryCountVal = 5;
        int sleepServerWaitVal = 50;
        int testRetryCountVal = 3;

        String waitAfterTest = System.getProperty("junitWaitAfterTest");
        String serverBindRetryCount = System.getProperty("junitServerBindRetryCount");
        String sleepServerWait = System.getProperty("junitSleepServerWait");
        String testRetryCount = System.getProperty("junitTestRetryCount");

        try {
            if (waitAfterTest != null) waitAfterTestVal = Integer.parseInt(waitAfterTest);
            if (serverBindRetryCount != null) serverBindRetryCountVal = Integer.parseInt(serverBindRetryCount);
            if (serverBindRetryCount != null) sleepServerWaitVal = Integer.parseInt(sleepServerWait);
            if (testRetryCount != null) testRetryCountVal = Integer.parseInt(testRetryCount);
        }
        catch (Exception e) {}

        WAIT_AFTER_TEST = waitAfterTestVal;
        SERVER_BIND_RETRY_COUNT = serverBindRetryCountVal;
        SLEEP_SERVER_WAIT = sleepServerWaitVal;
        TEST_RETRY_COUNT = testRetryCountVal;
    }
}