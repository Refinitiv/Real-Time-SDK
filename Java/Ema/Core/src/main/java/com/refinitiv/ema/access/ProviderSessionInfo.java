/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

import com.refinitiv.eta.valueadd.reactor.ReactorChannel;

/**
 * Provides session information for {@link OmmProvider} when EMA throws {@link OmmJsonConverterException}.
 */
public class ProviderSessionInfo extends SessionInfo {

    private OmmProvider provider;
    private long clientHandle;
    private long handle;

    void loadProviderSession(OmmProvider provider, ReactorChannel reactorChannel) {
        super.loadSessionInfo(reactorChannel);
        final ClientSession clientSession = (ClientSession) reactorChannel.userSpecObj();
        if (clientSession != null) {
            handle = clientSession.getLoginHandle();
            clientHandle = clientSession.clientHandle().value();
        }
        this.provider = provider;
    }

    /**
     * Return {@link OmmProvider} instance for this event.
     * @return reference to {@link OmmProvider}
     */
    public OmmProvider getProvider() {
        return provider;
    }

    /**
     * Returns a unique client identifier (a.k.a., client handle) associated by EMA with a connected client.
     * @return client identifier or handle.
     */
    public long getClientHandle() {
        return clientHandle;
    }

    /**
     * Returns a unique login identifier for the client session of a connected client.
     * @return login identifier or login handle.
     */
    public long getHandle() {
        return handle;
    }
}
