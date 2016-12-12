package com.thomsonreuters.upa.valueadd.reactor;

import com.thomsonreuters.upa.transport.ReadArgs;
import com.thomsonreuters.upa.transport.TransportFactory;

/**
 * ReactorDispatchOptions to be used in the {@link Reactor#dispatchAll(java.util.Set,
 * ReactorDispatchOptions, ReactorErrorInfo)} or {@link Reactor#dispatchChannel(ReactorChannel,
 * ReactorDispatchOptions, ReactorErrorInfo)} call.
 */
public class ReactorDispatchOptions
{
    ReadArgs _readArgs = null;
    int DEFAULT_MAX_MESSAGES = 100;
    int _maxMessages = DEFAULT_MAX_MESSAGES;

    public ReactorDispatchOptions()
    {
        _readArgs = TransportFactory.createReadArgs();
    }

    /**
     * Returns the ReadArgs.
     * 
     * @return the ReadArgs
     */
    public ReadArgs readArgs()
    {
        return _readArgs;
    }

    /**
     * Controls the maximum number of events or messages processed in this call.
     * If this is larger than the number of available messages,
     * {@link Reactor#dispatchAll(java.util.Set, ReactorDispatchOptions, ReactorErrorInfo)}
     * or {@link Reactor#dispatchChannel(ReactorChannel, ReactorDispatchOptions,
     * ReactorErrorInfo)} will return when there are no more events to
     * process. MaxMessages must be greater than zero. Default is 100, which
     * allows for up to 100 messages to be returned with a single call.
     * 
     * @return {@link ReactorReturnCodes#SUCCESS} if maxMessages is valid,
     *         otherwise {@link ReactorReturnCodes#PARAMETER_OUT_OF_RANGE} if
     *         the maxMessages is out of range
     */
    public int maxMessages(int maxMessages)
    {
        if (maxMessages < 1)
            return ReactorReturnCodes.PARAMETER_OUT_OF_RANGE;

        _maxMessages = maxMessages;
        return ReactorReturnCodes.SUCCESS;
    }

    /**
     * Returns the maxMessages value.
     * 
     * @return the maxMessages value
     */
    public int maxMessages()
    {
        return _maxMessages;
    }

    /**
     * Clears this object for reuse.
     */
    public void clear()
    {
        _readArgs.clear();
        _maxMessages = DEFAULT_MAX_MESSAGES;
    }
}
