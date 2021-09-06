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
     * Defaults to the property keystore.type in the JDK security properties file (java.security).
     * Sun JDK default = JKS
     *
     * @param KeystoreType the keystore type
     */
    public void KeystoreType(String KeystoreType);

    /**
     * Type of keystore for certificate file.
     * Defaults to the property keystore.type in the JDK security properties file (java.security).
     * Sun JDK default = JKS
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
     * Cryptographic protocol used. Sun JDK default is TLS.
     *
     * @param SecurityProtocol the security protocol
     */
    public void SecurityProtocol(String SecurityProtocol);

    /**
     * Cryptographic protocol used. Sun JDK default is TLS.
     * 
     * @return the SecurityProtocol
     */
    public String SecurityProtocol();

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

    /**
     * Gets the size that is going to be set to the _netRecvBuffer used inside the CryptoHelper for reading network data
     * @return the size of _netRecvBuffer
     */
    public int getNetRecvBufSize();

    /**
     * Sets the number of bytes that are going to be allocated for the _netRecvBuffer used inside the CryptoHelper for reading network data
     * @param size the expected size (will be ignored in case it is smaller than the size of the packet returned by SSLEngine)
     */
    public void setNetRecvBufSize(int size);
}
