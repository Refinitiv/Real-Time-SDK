/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport.crypto;

import com.refinitiv.eta.transport.EncryptionOptions;

import javax.net.ssl.SNIHostName;
import javax.net.ssl.SSLContext;
import javax.net.ssl.SSLEngine;
import javax.net.ssl.SSLParameters;
import java.io.IOException;
import java.util.Collections;

class ClientSSLEngineFactory implements SSLEngineFactory {
    /**
     * "HTTPS" algorithm performs endpoint verification as described in
     * https://tools.ietf.org/html/rfc2818#section-3
     * When client receives certificate from the server, client verifies that certificate
     * matches hostname of the remote server
     */
    public static final String ENDPOINT_IDENTIFICATION_ALGORITHM = "HTTPS";
    private final String hostName;
    private final int hostPort;
    private EncryptionOptions options;
    private final SSLContextFactory sslContextFactory;

    public ClientSSLEngineFactory(String hostName, int hostPort, EncryptionOptions options, SSLContextFactory sslContextFactory) {
        this.hostName = hostName;
        this.hostPort = hostPort;
        this.options = options;
        this.sslContextFactory = sslContextFactory;
    }

    public SSLEngine create() throws IOException {
        SSLContext context = sslContextFactory.create(options);
        SSLEngine engine = context.createSSLEngine(hostName, hostPort);
        engine.setUseClientMode(true);
        SSLParameters sslParameters = new SSLParameters();
        String[] clientProtocolVersions = new String[options.SecurityProtocolVersions().length];
        for (int i = 0; i < options.SecurityProtocolVersions().length; ++i) {
            clientProtocolVersions[i] = options.SecurityProtocol() + "v" + options.SecurityProtocolVersions()[i];
        }
        // Check SSL engine supported protocols against configured protocols, and remove
        //	any configured protocols that are not supported
        for (int i = 0; i < clientProtocolVersions.length; ++i) {
            boolean foundVersion = false;
            for (int j = 0; j < engine.getSupportedProtocols().length; ++j) {
                if (clientProtocolVersions[i].equals(engine.getSupportedProtocols()[j]))
                    foundVersion = true;
            }
            if (!foundVersion) {
                // Remove unsupported version from the client protocol version list
                String[] newList = new String[clientProtocolVersions.length - 1];
                for (int j = 0, k = 0; j < clientProtocolVersions.length; j++) {
                    if (j != i) {
                        newList[k] = clientProtocolVersions[j];
                        k++;
                    }
                }
                clientProtocolVersions = newList;
            }
        }
        engine.setEnabledProtocols(clientProtocolVersions);
        sslParameters.setEndpointIdentificationAlgorithm(ENDPOINT_IDENTIFICATION_ALGORITHM);
        sslParameters.setServerNames(Collections.singletonList(new SNIHostName(hostName)));
        sslParameters.setNeedClientAuth(false);
        sslParameters.setUseCipherSuitesOrder(true);
        sslParameters.setProtocols(clientProtocolVersions);
        engine.setSSLParameters(sslParameters);
        return engine;
    }
}
