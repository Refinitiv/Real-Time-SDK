package com.refinitiv.eta.valueadd.reactor;

import java.io.OutputStream;

public interface ReactorDebuggerOptions {

    /**
     * Default output stream capacity that is used when creating ReactorDebugger's default ByteArrayOutputStream stream
     */
    int DEFAULT_CAPACITY = 65535;

    /**
     * Capacity value which means that no explicit limit on the ReactorDebugger's stream capacity is set.
     * The Debugger will not check whether the capacity is enough while writing the message.
     */
    int NO_LIMIT_SET = -1;

    /**
     * Copies this instance into the instance provided
     * @param dst the instance which receives the copied information
     */
    void copy(ReactorDebuggerOptions dst);

    /**
     * Sets the levels to be debugged in batch, see {@link ReactorDebuggerLevels}
     * @param value debugging levels to be set
     */
    void setDebuggingLevels(int value);

    /**
     * Enables the provided debugging level, see {@link ReactorDebuggerLevels}
     * @param level the level to be enabled
     */
    void enableLevel(int level);

    /**
     * Disables the provided debugging level, see {@link ReactorDebuggerLevels}
     * @param level the level to be disabled
     */
    void disableLevel(int level);

    /**
     * Determines whether the CONNECTION debugging level is enabled
     * @return true if the CONNECTION debugging level is enabled, false otherwise
     */
    boolean debugConnectionLevel();

    /**
     * Determines whether the EVENTQUEUE debugging level is enabled
     * @return true if the EVENTQUEUE debugging level is enabled, false otherwise
     */
    boolean debugEventQueueLevel();

    /**
     * Determines whether the TUNNELSTREAM debugging level is enabled
     * @return true if the TUNNELSTREAM debugging level is enabled, false otherwise
     */
    boolean debugTunnelStreamLevel();

    /**
     * Determines whether any debugging is done at all
     * @return true if at least one debugging level is enabled, false otherwise
     */
    boolean debugEnabled();

    /**
     * Returns all debugging levels currently set for the Reactor by this instance of ReactorDebuggerOptions
     * @return integer representing debugging levels
     */
    int debuggingLevels();

    /**
     * The output stream that will be used to create the Reactor debugger
     * @return OutputStream instance
     */
    OutputStream outputStream();

    /**
     * Setter for the output stream that will be used to create the Reactor debugger
     * @param outputStream OutputStream instance
     */
    void outputStream(OutputStream outputStream);

    /**
     * Getter for the stream capacity that will be used to create the Reactor debugger
     * @return intended stream capacity
     */
    int capacity();

    /**
     * Setter for the stream capacity that will be used to create the Reactor debugger
     * @param capacity intended stream capacity
     */
    void capacity(int capacity);

    /**
     * Clears this ReactorDebuggerOptions instance
     */
    void clear();
}
