/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport.crypto;

import com.refinitiv.eta.transport.EncryptionOptions;

import javax.net.ssl.TrustManagerFactory;
import java.io.IOException;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;

class ConscryptTrustManagerFactoryFactory implements TrustManagerFactoryFactory {
    public TrustManagerFactory Create(EncryptionOptions options, KeyStore keyStore) throws IOException {
        // get default trust management algorithm for security provider
        // (default: PKIX for security provider SunJSSE)
        String trustManagerAlgorithm = options.TrustManagerAlgorithm() == null || options.TrustManagerAlgorithm().equals("")
                ? options.DefaultTrustManagerAlgorithm()
                : options.TrustManagerAlgorithm();

        TrustManagerFactory trustManagerFactory;
        // create a TrustManagerFactory
        try {
            trustManagerFactory = TrustManagerFactory.getInstance(trustManagerAlgorithm
                    /*, securityProvider intentionally left default as Conscrypt does not implement its own trust manager algorithm */);
            trustManagerFactory.init(keyStore);
        } catch (NoSuchAlgorithmException | KeyStoreException e) {
            throw new IOException("Error when creating TrustManagerFactory: " + e.getMessage());
        }
        return trustManagerFactory;
    }
}
