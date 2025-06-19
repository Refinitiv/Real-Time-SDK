/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport.crypto;

import com.refinitiv.eta.transport.EncryptionOptions;

import javax.net.ssl.SSLContext;
import javax.net.ssl.SSLEngine;
import javax.net.ssl.SSLParameters;
import java.io.IOException;

class ServerSSLEngineFactory implements SSLEngineFactory {
    private final SSLContextFactory sslContextFactory;
    private EncryptionOptions options;

    public ServerSSLEngineFactory(SSLContextFactory sslContextFactory, EncryptionOptions options) {
        this.sslContextFactory = sslContextFactory;
        this.options = options;
    }

    public SSLEngine create() throws IOException {
        SSLContext context = sslContextFactory.create(options);
        SSLEngine engine = context.createSSLEngine();
        engine.setUseClientMode(false);
        SSLParameters sslParameters = new SSLParameters();
        sslParameters.setUseCipherSuitesOrder(true);
        String[] clientProtocolVersions = new String[options.SecurityProtocolVersions().length];
        for (int i = 0; i < options.SecurityProtocolVersions().length; ++i) {
            clientProtocolVersions[i] = options.SecurityProtocol() + "v" + options.SecurityProtocolVersions()[i];
        }
        sslParameters.setNeedClientAuth(false);
        sslParameters.setProtocols(clientProtocolVersions);
        engine.setSSLParameters(sslParameters);
        return engine;
    }
}
