package com.refinitiv.ema.access;

import com.refinitiv.eta.valueadd.reactor.ReactorChannel;

/**
 * Provides session information for {@link OmmConsumer} when EMA throws {@link OmmJsonConverterException}.
 */
public class ConsumerSessionInfo extends SessionInfo {
    void loadConsumerSession(ReactorChannel reactorChannel) {
        super.loadSessionInfo(reactorChannel);
    }
}