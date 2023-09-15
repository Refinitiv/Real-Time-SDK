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
     * Cryptographic protocol used. Sun JDK default is TLS.
     *
     * @param securityProtocol the security protocol
     */
    public void securityProtocol(String securityProtocol);

    /**
     * Cryptographic protocol used. Sun JDK default is TLS.
     * 
     * @return the SecurityProtocol
     */
    public String securityProtocol();

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
