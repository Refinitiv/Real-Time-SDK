/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport.crypto;

import com.refinitiv.eta.transport.EncryptionOptions;

import javax.net.ssl.SSLEngine;
import java.io.IOException;

class ClientHandshakeFactory implements CryptoHandshakeFactory {
    private CryptoHandshakeFactory basic;

    private ClientHandshakeFactory(CryptoHandshakeFactory basic) {
        this.basic = basic;
    }

    public CryptoHandshake create(SSLEngine engine, BufferData bufferData) throws IOException {
        CryptoHandshake cryptoHandshake = basic.create(engine, bufferData);
        CertificateValidator certificateValidator = new CertificateValidator(engine);
        return socketChannel -> {
            cryptoHandshake.perform(socketChannel);
            certificateValidator.perform(socketChannel);
        };
    }

    public static CryptoHandshakeFactory create(EncryptionOptions options, CryptoHandshakeFactory basicHandshakeManager) {
        // check certificate(s) (a null value implies "X509" algorithm)
        boolean checkCert = options.KeyManagerAlgorithm() == null || options.KeyManagerAlgorithm().endsWith("X509");
        if (checkCert) {
            return new ClientHandshakeFactory(basicHandshakeManager);
        } else {
            return basicHandshakeManager;
        }
    }
}
