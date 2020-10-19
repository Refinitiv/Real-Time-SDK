package com.refinitiv.eta.transport;

/**
 * Options used for configuring a tunneling connection.
 * 
 * @see ConnectOptions
 */
public interface TunnelingInfo
{
    /**
     * Tunneling type.
     * Possible values are "None", http", or "encrypted"
     * For HTTP Tunneling, tunnelingType has to be set to "http" or "encrypted"
     * 
     * @param tunnelingType the tunnelingType to set
     */
    public void tunnelingType(String tunnelingType);

    /**
     * Tunneling type.
     * Possible values are "None", http", or "encrypted"
     * For HTTP Tunneling, tunnelingType has to be set to "http" or "encrypted"
     * 
     * @return the tunnelingType
     */
    public String tunnelingType();

    /**
     * If set to true, we are going through an HTTP proxy server for tunneling.
     *
     * @param HTTPproxy the HTT pproxy
     */
    public void HTTPproxy(boolean HTTPproxy);

    /**
     * If set true, we are going through an HTTP proxy server for tunneling.
     *
     * @return HTTPproxy
     */
    public boolean HTTPproxy();

    /**
     * Address or hostname of HTTP proxy server to connect to.
     * HTTPproxy has to be true.
     * 
     * @param HTTPproxyHostName the HTTP proxy server address to set
     */
    public void HTTPproxyHostName(String HTTPproxyHostName);

    /**
     * Address or hostname of HTTP proxy server to connect to.
     *
     * @return the HTTPproxyHostName
     */
    public String HTTPproxyHostName();

    /**
     * Port Number of HTTP proxy server to connect to.
     * HTTPproxy has to be true. Must be in the range of 0 - 65535.
     * 
     * @param HTTPproxyPort the HTTP proxy server address to set
     */
    public void HTTPproxyPort(int HTTPproxyPort);

    /**
     * Port Number of HTTP proxy server to connect to.
     * 
     * @return the HTTPproxyPort
     */
    public int HTTPproxyPort();
    
    /**
     * Object name for load balancing to the various P2PSs that are part of a hosted solution.
     * 
     * @param objectName the object name to set
     */
    public void objectName(String objectName);

    /**
     * Object name for load balancing to the various P2PSs that are part of a hosted solution.
     * 
     * @return the objectName
     */
    public String objectName();

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
}