///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.rtsdk.ema.access;

/**
 * OmmNiProviderConfig is used to modify configuration and behavior of OmmProvider<br>
 * for non-interactive application.
 * 
 * <p>OmmNiProviderConfig provides a default basic OmmProvider configuration.</p>
 * 
 * <p>The default configuration may be modified and or appended by using the EmaConfig.xml<br>
 * file or any methods of this class.</p>
 * 
 * <p>For a tunneling connection, ChannelType::RSSL_HTTP or ChannelType::RSSL_ENCRYPTED must be configured in
 *  Ema configuration file such as EmaConfig.xml. </p>
 *  
 * <p>For a tunneling connection, supported authentication protocols are: Negotiate/Kerberos, Kerberos, NTLM, and Basic.</p>
 * 
 * <p>Protocols Negotiate/Kerberos or Kerberos require the following configurations:<br>
 *  tunnelingCredentialUserName, tunnelingCredentialPasswd, tunnelingCredentialDomain, and tunnelingCredentialKRB5ConfigFile</p>
 *                                                             
 * <p>Protocol NTLM requires the following configurations:<br>
 *  tunnelingCredentialUserName, tunnelingCredentialPasswd, tunnelingCredentialDomain</p>
 *             
 * <p>Protocol Basic requires the following configurations:<br>
 *  tunnelingCredentialUserName and tunnelingCredentialPasswd</p>
 * 
 * <p>The EmaConfig.xml file is read in if it is present in the working directory of the application.</p>
 * 
 * <p>Calling any interface methods of OmmNiProviderConfig class overrides or appends the existing<br>
 * configuration.</p>
 * 
 * @see OmmProvider
 * @see OmmProviderConfig
 */


public interface OmmNiProviderConfig extends OmmProviderConfig
{
	
	public static class OperationModel
	{
		/**
		 * specifies callbacks happen on user thread of control
		 */
		public static final int USER_DISPATCH = 0;
		
		/**
		 * specifies callbacks happen on API thread of control
		 */
		public static final int API_DISPATCH = 1;
	}
	
	public static class AdminControl
	{
		/**
		 * specifies user submit directory refresh message
		 */
		public static final int USER_CONTROL = 0;
		
		/**
		 * specifies API sends down directory refresh message based on the configuration
		 */
		public static final int API_CONTROL = 1;
	}
	
	/**
	 * Clears the OmmNiProviderConfig and sets all the defaults.
	 * Invoking clear() method clears all the values and resets all the defaults.
	 * 
	 * @return reference to this object
	 */
	public OmmNiProviderConfig clear();

	/**
	 * Specifies the username.
	 * Overrides a value specified in Login domain via the addAdminMsg( ReqMsg ) method.
	 * 
	 * @param username specifies name used on login request
	 * @return reference to this object
	 */
	public OmmNiProviderConfig username(String username);

	/**
	 * Specifies the password.
	 * Overrides a value specified in Login domain via the addAdminMsg( ReqMsg ) method.
	 * 
	 * @param password specifies respective login request attribute
	 * @return reference to this object
	 */
	public OmmNiProviderConfig password(String password);

	/**
	 * Specifies the position.
	 * Overrides a value specified in Login domain via the addAdminMsg( ReqMsg ) method.
	 * 
	 * @param position specifies respective login request attribute
	 * @return reference to this object
	 */
	public OmmNiProviderConfig position(String position);

	/**
	 * Specifies the authorization application identifier. Must be unique for
	 * each application. Range 257 to 65535 is available for site-specific use.
	 * Range 1 to 256 is reserved.
	 * 
	 * @param applicationId specifies respective login request attribute
	 * @return reference to this object
	 */
	public OmmNiProviderConfig applicationId(String applicationId);
	
	/**
	 * Specifies the instance identifier. Can be any ASCII string, e.g. "Instance1".
	 * Used to differentiate applications running on the same client host.
	 * 
	 * @param instanceId specifies respective login request attribute
	 * @return reference to this object
	 */
	public OmmNiProviderConfig instanceId(String instanceId);

	/**
	 * Specifies a hostname and port. Overrides prior value.
	 * Implies usage of TCP IP channel or RSSL connection type socket.
	 *       
	 * @param host specifies server and port to which OmmProvider will connect.
	 * <br>If host set to "&lt;hostname&gt;:&lt;port&gt;", then hostname:port is assumed.
	 * <br>If host set to "", then localhost:14003 is assumed.
	 * <br>If host set to ":", then localhost:14003 is assumed.
	 * <br>If host set to "&lt;hostname&gt;", then hostname:14003 is assumed.
	 * <br>If host set to "&lt;hostname&gt;:", then hostname:14003 is assumed.
	 * <br>If host set to ":&lt;port&gt;", then localhost:port is assumed.
	 * 
	 * @return reference to this object
	 */
	public OmmNiProviderConfig host(String host);

	/**
	 * Specifies the operation model, overriding the default<br>
	 * The operation model specifies whether to dispatch messages
	 * in the user or application thread of control.
	 * 
	 * @param operationModel specifies threading and dispatching model used by application
	 * @return reference to this object
	 */
	public OmmNiProviderConfig operationModel(int operationModel);
	
	/**
	 * Specifies whether API or user controls sending of Directory<br>
	 * refresh message.
	 * 
	 * @param control specifies who sends down the directory refresh message
	 * @return reference to this object
	 */
	public OmmNiProviderConfig adminControlDirectory( int control);
	
	/**
	 * Create an OmmProvider with provider name.<br>
     * This name identifies configuration section to be used by OmmProvider instance.
     * 
	 * @param providerName specifies name of OmmProvider instance
	 * @return reference to this object
	 */
	public OmmNiProviderConfig providerName(String providerName);
	
	/**
	 * Specifies the address or host name of HTTP proxy server to connect to.
     * 
	 * @param proxyHostName specifies the address or host name of HTTP proxy server
	 *  for tunneling connection.
	 * @return reference to this object
	 */
	public OmmNiProviderConfig tunnelingProxyHostName(String proxyHostName); 
	
	/**
	 * Specifies the port number of HTTP proxy server to connect to. Must be in the range of 0 - 65535.
     * 
	 * @param proxyPort specifies the port number of HTTP proxy server for tunneling connection.
	 * @return reference to this object
	 */
	public OmmNiProviderConfig tunnelingProxyPort(String proxyPort); 
	
	/**
	 * Specifies the object name for load balancing to the various providers that are part of a hosted solution.
     * 
	 * @param objectName specifies object name for load balancing used for tunneling connection.
	 * @return reference to this object
	 */
	public OmmNiProviderConfig tunnelingObjectName(String objectName);  
	
	/**
	 * Specifies the user name to authenticate. Needed for all authentication protocols.
     * 
	 * @param userName specifies user name used for tunneling connection.
	 * @return reference to this object
	 */
	public OmmNiProviderConfig tunnelingCredentialUserName(String userName);
	
	/**
	 * Specifies the password to authenticate. Needed for all authentication protocols.
     * 
	 * @param passwd specifies password used for tunneling connection.
	 * @return reference to this object
	 */
	public OmmNiProviderConfig tunnelingCredentialPasswd(String passwd);  

	/**
	  * Specifies the domain of the user to authenticate.
     *  Needed for NTLM or for Negotiate/Kerberos or for Kerberos authentication protocols.
     * 
     * 	For Negotiate/Kerberos or for Kerberos authentication protocols, tunnelingCredentialDomain
     * 	should be the same as the domain in the 'realms' and 'domain_realm' sections of
     * 	the Kerberos configuration file ({@link #tunnelingCredentialKRB5ConfigFile(String krb5ConfigFile)}).
     * 
	 * @param domain specifies the domain used for tunneling connection.
	 * @return reference to this object
	 */
	public OmmNiProviderConfig tunnelingCredentialDomain(String domain); 
	
	/**
	 * Specifies the complete path of the Kerberos5 configuration file (krb5.ini or krb5.conf, or custom file).
     * Needed for Negotiate/Kerberos and Kerberos authentication protocols.
     * 
	 * @param krb5ConfigFile specifies the full path of kerberos5 config file for tunneling connection.
	 * @return reference to this object
	 */
	public OmmNiProviderConfig tunnelingCredentialKRB5ConfigFile(String krb5ConfigFile); 
	
	/**
	 * Specifies the local hostname of the client. Needed for NTLM authentication protocol only.
     * 
	 * @param localHostName specifies the client local host name used for tunneling connection.
	 * @return reference to this object
	 */
	public OmmNiProviderConfig tunnelingCredentialLocalHostName(String localHostName);  
	
	/**
	 * Specifies the type of the key store for certificate file.
     * Defaults to the property keystore.type in the JDK security properties file (java.security).
     * Sun JDK default = JKS
     * 
	 * @param keyStoreType specifies the type of key store for tunneling connection.
	 * @return reference to this object
	 */
    public OmmNiProviderConfig tunnelingKeyStoreType(String keyStoreType);
    
    /**
	 * Specifies the key store file that contains your own private keys, and public key certificates you received
	 * from someone else.
     * 
	 * @param keyStoreFile specifies key storing file for tunneling connection. The JDK utility 'keytool' creates this file.
	 * @return reference to this object
	 */
    public OmmNiProviderConfig tunnelingKeyStoreFile(String keyStoreFile);
    
    /**
	 * Specifies the password for the key store file. 
     * 
	 * @param keyStorePasswd specifies password for key storing file for tunneling connection.
	 * @return reference to this object
	 */
    public OmmNiProviderConfig tunnelingKeyStorePasswd(String keyStorePasswd);  
	
    /**
	 * Specifies the Cryptographic protocol to be used. Sun JDK default is TLS which will go the latest one
	 * supported by JDK (currently is TLSv1.2).
	 *  
	 * @param securityProtocol specifies a cryptographic protocol for tunneling connection.
	 * @return reference to this object
	 */
    public OmmNiProviderConfig tunnelingSecurityProtocol(String securityProtocol); 
    
    /**
	 * Specifies the Java Cryptography Package provider to be used. The Oracle JDK default is SunJSSE.
     * 
	 * @param securityProvider specifies a java cryptography package provider for tunneling connection.
	 * @return reference to this object
	 */
    public OmmNiProviderConfig tunnelingSecurityProvider(String securityProvider); 
    
    /**
	 * Specifies the Java Key Management algorithm to be used. 
     * Defaults to the property ssl.KeyManagerFactory.algorithm in the JDK security properties file (java.security).
     * Sun JDK default = SunX509 
     * 
	 * @param keyManagerAlgorithm specifies a java key manager algorithm for tunneling connection.
	 * @return reference to this object
	 */
    public OmmNiProviderConfig tunnelingKeyManagerAlgorithm(String keyManagerAlgorithm); 
    
    /**
	 * Specifies the Java Trust Management algorithm to be used.
     * Defaults to the property ssl.TrustManagerFactory.algorithm in the JDK security properties file (java.security).
     * Sun JDK default = PKIX
     * 
	 * @param trustManagerAlgorithm specifies a java trust manager algorithm for tunneling connection.
	 * @return reference to this object
	 */
    public OmmNiProviderConfig tunnelingTrustManagerAlgorithm(String trustManagerAlgorithm); 
	
	/**
	 * Specifies the local configuration, overriding and adding to the current content.
	 * 
	 * @param config specifies OmmProvider configuration
	 * @return reference to this object
	 */
	public OmmNiProviderConfig config(Data config);
	
	/**
	 * Specifies an administrative request message to override the default administrative request.<br>
	 * Application may call multiple times prior to initialization. Supports Login domain only.<br>
	 * 
	 * @param reqMsg specifies administrative domain request message
	 * @return reference to this object
	 */
	public OmmNiProviderConfig addAdminMsg(ReqMsg reqMsg);
	
	/**
	 * Specifies an administrative refresh message to override the default administrative refresh.<br>
	 * Application may call multiple times prior to initialization.<br>Supports Directory domain only.
	 * 
	 * @param refreshMsg specifies administrative domain refresh message
	 * @return reference to this object
	 */
	public OmmNiProviderConfig addAdminMsg(RefreshMsg refreshMsg);

}
