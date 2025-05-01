/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport;

public interface ServerEncryptionOptions {
	
	/**
     * Type of keystore for certificate file.
     * RTSDK Default = JKS
     *
     * @param keystoreType the keystore type
     */
    public void keystoreType(String keystoreType);

    /**
     * Type of keystore for certificate file.
     * RTSDK Default = JKS
     * 
     * @return the KeystoreType
     */
    public String keystoreType();

    /**
     * Keystore file that contains the server private key, and the certificate file for this server.
     *
     * @param keystoreFile the keystore file location
     */
    public void keystoreFile(String keystoreFile);

    /**
     * Keystore file that contains the server private key, and the certificate file for this server.
     * 
     * @return the KeystoreFile location
     */
    public String keystoreFile();

    /**
     * Password for keystore file.
     *
     * @param keystorePasswd the keystore passwd
     */
    public void keystorePasswd(String keystorePasswd);

    /**
     * Password for keystore file.
     * 
     * @return the KeystorePasswd
     */
    public String keystorePasswd();

    /**
     * Cryptographic protocol used. RTSDK default is set to TLS.
     *
     * @param securityProtocol the security protocol
     */
    public void securityProtocol(String securityProtocol);

    /**
     * Cryptographic protocol used. RTSDK default is set to TLS.
     * 
     * @return the SecurityProtocol
     */
    public String securityProtocol();
    
    /**
     * Cryptographic protocol versions used. Array of Strings
     * should designate what versions to use. RTSDK default is {"1.3", "1.2"}.
     *
     * @param securityProtocolVersions the array list of security protocol versions to use
     */
    public void securityProtocolVersions(String[] securityProtocolVersions);
    
    /**
     * Cryptographic protocol versions used. RTSDK default is {"1.3", "1.2"}..
     * 
     * @return the securityProtocolVersions designated to use
     */
    public String[] securityProtocolVersions();

    /**
     * Java Cryptography Package provider.
     * Sun JDK default = SunJSSE
     *
     * @param securityProvider the security provider
     */
    public void securityProvider(String securityProvider);

    /**
     * Java Cryptography Package provider.
     * Sun JDK default = SunJSSE
     * 
     * @return the SecurityProvider
     */
    public String securityProvider();

    /**
     * Java Key Management algorithm. 
     * Defaults to the property ssl.KeyManagerFactory.algorithm
     * in the JDK security properties file (java.security).
     * Sun JDK default = SunX509
     *
     * @param keyManagerAlgorithm the key manager algorithm
     */
    public void keyManagerAlgorithm(String keyManagerAlgorithm);

    /**
     * Java Key Management algorithm.
     * Defaults to the property ssl.KeyManagerFactory.algorithm
     * in the JDK security properties file (java.security).
     * Sun JDK default = SunX509
     * 
     * @return the KeyManagerAlgorithm
     */
    public String keyManagerAlgorithm();

    /**
     * Java Trust Management algorithm.
     * Defaults to the property ssl.TrustManagerFactory.algorithm in the JDK security properties file (java.security).
     * Sun JDK default = PKIX
     *
     * @param trustManagerAlgorithm the trust manager algorithm
     */
    public void trustManagerAlgorithm(String trustManagerAlgorithm);

    /**
     * Java Trust Management algorithm.
     * Defaults to the property ssl.TrustManagerFactory.algorithm in the JDK security properties file (java.security).
     * Sun JDK default = PKIX
     * 
     * @return the TrustManagerAlgorithm
     */
    public String trustManagerAlgorithm();

}
