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
import java.security.NoSuchProviderException;

class DefaultTrustManagerFactoryFactory implements TrustManagerFactoryFactory {
    public TrustManagerFactory Create(EncryptionOptions options, KeyStore keyStore) throws IOException {
        String securityProvider = options.SecurityProvider() == null || options.SecurityProvider().equals("")
                ? options.DefaultSecurityProvider()
                : options.SecurityProvider();

        // get default trust management algorithm for security provider
        // (default: PKIX for security provider SunJSSE)
        String trustManagerAlgorithm = options.TrustManagerAlgorithm() == null || options.TrustManagerAlgorithm().equals("")
                ? options.DefaultTrustManagerAlgorithm()
                : options.TrustManagerAlgorithm();

        TrustManagerFactory trustManagerFactory;
        // create a TrustManagerFactory
        try {
            trustManagerFactory = TrustManagerFactory.getInstance(trustManagerAlgorithm, securityProvider);
            trustManagerFactory.init(keyStore);
        } catch (NoSuchAlgorithmException | NoSuchProviderException | KeyStoreException e) {
            throw new IOException("Error when creating TrustManagerFactory: " + e.getMessage());
        }
        return trustManagerFactory;
    }
}
