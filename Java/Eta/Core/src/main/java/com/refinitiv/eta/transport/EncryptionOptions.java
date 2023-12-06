package com.refinitiv.eta.transport;

/**
 * ETA Connect Options used in the
 * {@link ConnectOptions} class.
 * 
 * @see Transport
 */
public interface EncryptionOptions
{
    
    /**
     * Type of connection to establish. Must be one of the following:
     * {@link ConnectionTypes#SOCKET} or {@link ConnectionTypes#HTTP}.
     * 
     * @param connectionType the connectionType to set
     * 
     * @see ConnectionTypes
     */
    public void connectionType(int connectionType);

    /**
     * Type of connection to establish.
     * 
     * @return the connectionType
     * 
     * @see ConnectionTypes
     */
    public int connectionType();

    /**
     * Type of keystore for certificate file.
     * RTSDK Default = JKS
     *
     * @param KeystoreType the keystore type
     */
    public void KeystoreType(String KeystoreType);

    /**
     * Type of keystore for certificate file.
     * RTSDK Default = JKS
     * 
     * @return the KeystoreType
     */
    public String KeystoreType();

    /**
     * Keystore file that contains your own private keys, and public key certificates you received from someone else.
     *
     * @param KeystoreFile the keystore file
     */
    public void KeystoreFile(String KeystoreFile);

    /**
     * Keystore file that contains your own private keys, and public key certificates you received from someone else.
     * 
     * @return the KeystoreFile
     */
    public String KeystoreFile();

    /**
     * Password for keystore file.
     *
     * @param KeystorePasswd the keystore passwd
     */
    public void KeystorePasswd(String KeystorePasswd);

    /**
     * Password for keystore file.
     * 
     * @return the KeystorePasswd
     */
    public String KeystorePasswd();

    /**
     * Cryptographic protocol used. RTSDK default is set to TLS.
     *
     * @param SecurityProtocol the security protocol
     */
    public void SecurityProtocol(String SecurityProtocol);

    /**
     * Cryptographic protocol used. RTSDK default is set to TLS.
     * 
     * @return the SecurityProtocol
     */
    public String SecurityProtocol();
    
    /**
     * Cryptographic protocol versions used. Array of Strings
     * should designate what versions to use. RTSDK default is {"1.3", "1.2"}.
     *
     * @param SecurityProtocolVersions the array list of security protocol versions to use
     */
    public void SecurityProtocolVersions(String[] SecurityProtocolVersions);
    
    /**
     * Cryptographic protocol versions used. RTSDK default is {"1.3", "1.2"}..
     * 
     * @return the SecurityProtocolVersions designated to use
     */
    public String[] SecurityProtocolVersions();

    /**
     * Java Cryptography Package provider.
     * Sun JDK default = SunJSSE
     *
     * @param SecurityProvider the security provider
     */
    public void SecurityProvider(String SecurityProvider);

    /**
     * Java Cryptography Package provider.
     * Sun JDK default = SunJSSE
     * 
     * @return the SecurityProvider
     */
    public String SecurityProvider();

    /**
     * Java Key Management algorithm. 
     * Defaults to the property ssl.KeyManagerFactory.algorithm
     * in the JDK security properties file (java.security).
     * Sun JDK default = SunX509
     *
     * @param KeyManagerAlgorithm the key manager algorithm
     */
    public void KeyManagerAlgorithm(String KeyManagerAlgorithm);

    /**
     * Java Key Management algorithm.
     * Defaults to the property ssl.KeyManagerFactory.algorithm
     * in the JDK security properties file (java.security).
     * Sun JDK default = SunX509
     * 
     * @return the KeyManagerAlgorithm
     */
    public String KeyManagerAlgorithm();

    /**
     * Java Trust Management algorithm.
     * Defaults to the property ssl.TrustManagerFactory.algorithm in the JDK security properties file (java.security).
     * Sun JDK default = PKIX
     *
     * @param TrustManagerAlgorithm the trust manager algorithm
     */
    public void TrustManagerAlgorithm(String TrustManagerAlgorithm);

    /**
     * Java Trust Management algorithm.
     * Defaults to the property ssl.TrustManagerFactory.algorithm in the JDK security properties file (java.security).
     * Sun JDK default = PKIX
     * 
     * @return the TrustManagerAlgorithm
     */
    public String TrustManagerAlgorithm();

}
