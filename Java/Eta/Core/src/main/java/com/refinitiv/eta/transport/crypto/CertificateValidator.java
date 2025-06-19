/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport.crypto;

import javax.net.ssl.SSLEngine;
import javax.net.ssl.SSLPeerUnverifiedException;
import java.io.IOException;
import java.nio.channels.SocketChannel;
import java.security.cert.CertificateExpiredException;
import java.security.cert.CertificateNotYetValidException;
import java.security.cert.X509Certificate;

class CertificateValidator implements CryptoHandshake {
    private SSLEngine _engine;

    public CertificateValidator(SSLEngine sslEngine) {
        _engine = sslEngine;
    }

    @Override
    public void perform(SocketChannel socketChannel) throws IOException {
        try {
            X509Certificate[] cert = (X509Certificate[]) (_engine.getSession().getPeerCertificates());

            for (int i = 0; i < cert.length; i++) {
                try {
                    cert[i].checkValidity();
                } catch (CertificateExpiredException e) // certificate has expired
                {
                    throw new IOException("Invalid certificate (Expired):   " + e.getMessage());
                } catch (CertificateNotYetValidException e) // certificate not yet valid
                {
                    throw new IOException("Invalid certificate (Not Yet Valid):   " + e.getMessage());
                }
            }
        } catch (SSLPeerUnverifiedException e) {
            throw new IOException("Missing certificate:   " + e.getMessage());
        }
    }
}
