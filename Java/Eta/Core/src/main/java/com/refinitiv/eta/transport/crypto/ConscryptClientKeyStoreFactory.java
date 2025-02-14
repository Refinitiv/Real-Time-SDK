/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport.crypto;

import com.refinitiv.eta.transport.EncryptionOptions;

import java.io.IOException;
import java.security.KeyStore;
import java.security.cert.Certificate;
import java.security.cert.X509Certificate;
import java.util.Enumeration;

class ConscryptClientKeyStoreFactory implements KeyStoreFactory {
    private KeyStoreFactory wrapped;

    public ConscryptClientKeyStoreFactory(KeyStoreFactory wrapped) {
        this.wrapped = wrapped;
    }

    @Override
    public KeyStore create(EncryptionOptions options) throws IOException {
        try {
            KeyStore originalKeyStore = wrapped.create(options);
            X509Certificate caCert = null;
            Enumeration<String> aliases = originalKeyStore.aliases();
            while (aliases.hasMoreElements()) {
                String alias = aliases.nextElement();
                if (originalKeyStore.isCertificateEntry(alias)) {
                    Certificate cert = originalKeyStore.getCertificate(alias);
                    if (cert instanceof X509Certificate) {
                        caCert = (X509Certificate) cert;
                    }
                }
            }
            if (caCert == null) {
                return originalKeyStore;
            }
            KeyStore keyStore = KeyStore.getInstance(KeyStore.getDefaultType());
            keyStore.load(null, null);
            keyStore.setCertificateEntry("caCert", caCert);
            return keyStore;
        } catch (Exception e) {
            throw new IOException("Error loading or repacking KeyStore for Conscrypt: " + e.getMessage(), e);
        }
    }
}
