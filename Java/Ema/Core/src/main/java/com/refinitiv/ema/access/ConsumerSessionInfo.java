/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2021,2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

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