///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|          Copyright (C) 2019-2023 Refinitiv. All rights reserved.          --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

import com.refinitiv.ema.rdm.DataDictionary;

/**
 * OmmConsumerConfig is used to modify configuration and behaviour of OmmConsumer.
 * <p>OmmConsumerConfig provides a default basic OmmConsumer configuration.</p>
 * 
 * <p>The default configuration may be modified and or appended by using
 * any methods from OmmConsumerConfg.</p>
 * 
 * <p>OmmConsumerconfig methods override or append the existing configuration.</p>
 * 
 * <p> For a tunneling connection, ChannelType::RSSL_HTTP or ChannelType::RSSL_ENCRYPTED must be configured in
 *  Ema configuration file such as EmaConfig.xml. </p>
 *  
 * <p> For a tunneling connection, supported authentication protocols are: Negotiate/Kerberos, Kerberos, NTLM, and Basic. </p>
 * 
 *  <p>Protocols Negotiate/Kerberos or Kerberos require the following configurations:
 *             tunnelingCredentialUserName, tunnelingCredentialPasswd, tunnelingCredentialDomain, and tunnelingCredentialKRB5ConfigFile   </p>
 *                                                             
 *  <p>Protocol NTLM requires the following configurations:
 *             tunnelingCredentialUserName, tunnelingCredentialPasswd, tunnelingCredentialDomain  </p>
 *             
 *  <p> Protocol Basic requires the following configurations:
 *             tunnelingCredentialUserName and tunnelingCredentialPasswd   </p>
 * 
 * @see OmmConsumer
 */
public interface OmmConsumerConfig
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

	/**
	 * Clears the OmmConsumerConfig and sets all the defaults.
	 * Invoking clear() method clears all the values and resets all the defaults.
	 * 
	 * @return reference to this object
	 */
	public OmmConsumerConfig clear();

	/**
	 * Specifies the username.
	 * Overrides username that was used when sending the login lequest.
	 * 
	 * @param username specifies name used on login request
	 * @return reference to this object
	 */
	public OmmConsumerConfig username(String username);

	/**
	 * Specifies the password.
	 * Overrides password that was used when sending the login lequest.
	 * 
	 * @param password specifies respective login request attribute
	 * @return reference to this object
	 */
	public OmmConsumerConfig password(String password);

	/**
	 * Specifies the position.
	 * Overrides position that was used when sending the login lequest.
	 * 
	 * @param position specifies respective login request attribute
	 * @return reference to this object
	 */
	public OmmConsumerConfig position(String position);

	/**
	 * Specifies the authorization application identifier. Must be unique for
	 * each application. Range 257 to 65535 is available for site-specific use.
	 * Range 1 to 256 is reserved.
	 * 
	 * @param applicationId specifies respective login request attribute
	 * @return reference to this object
	 */
	public OmmConsumerConfig applicationId(String applicationId);
	
	/**
	 * Specifies the clientID used for RDP token service. Mandatory, used to specify Application ID obtained from App Generator for V1 oAuth Password Credentials, or to specify Service Account username for V2 Client Credentials and V2 Client Credentials with JWT Logins.
	 * 
	 * @param clientId specifies an unique identifier.
	 * @return reference to this object
	 */
	public OmmConsumerConfig clientId(String clientId);
	
	/**
	 * Specifies optionally a client secret used by OAuth client to authenticate to the Authorization Server.
	 * 
	 * @param clientSecret specifies a client secret.
	 * @return reference to this object
	 */
	public OmmConsumerConfig clientSecret(String clientSecret);
	
	/**
	 * Specifies optionally a client JWK used by OAuth client to authenticate to the Authorization Server.
	 * 
	 * @param clientJWK specifies the full JSON encoded JWK string 
	 * @return reference to this object
	 */
	public OmmConsumerConfig clientJWK(String clientJWK);
	
	/**
	 * Specifies optionally a token scope to limit the scope of generated token from the token service.
	 * 
	 * @param tokenScope specifies a token scope
	 * @return reference to this object
	 */
	public OmmConsumerConfig tokenScope(String tokenScope);
	
	/**
	 * Specifies optionally an audience used with JWT authentication from the token service.
	 * 
	 * @param audience specifies a token scope
	 * @return reference to this object
	 */
	public OmmConsumerConfig audience(String audience);
	
	/**
     * Sets the exclusive sign on control to force sign-out of other applications using the same credentials.
     * <p>Defaults to true</p>
     *
     * @param takeExclusiveSignOnControl the exclusive sign on control.
     * @return reference to this object
     */
	public OmmConsumerConfig takeExclusiveSignOnControl(boolean takeExclusiveSignOnControl);
	
	/**
	 * Specifies an URL to override the default for token service V1 to perform authentication to get access and refresh tokens.
	 * <p>Defaults to "https://api.refinitiv.com/auth/oauth2/v1/token".</p>
	 * @param tokenServiceUrl specifies an URL for token service.
	 * @return reference to this object
	 */
	public OmmConsumerConfig tokenServiceUrl(String tokenServiceUrl);
	
	/**
	 * Specifies an URL to override the default for token service V1 to perform authentication to get access and refresh tokens.
	 * <p>Defaults to "https://api.refinitiv.com/auth/oauth2/v1/token".</p>
	 * @param tokenServiceUrlV1 specifies an URL for token service.
	 * @return reference to this object
	 */
	public OmmConsumerConfig tokenServiceUrlV1(String tokenServiceUrlV1);
	
	/**
	 * Specifies an URL to override the default for token service V2 to perform authentication to get access and refresh tokens.
	 * <p>Defaults to "https://api.refinitiv.com/auth/oauth2/v2/token".</p>
	 * @param tokenServiceUrlV2 specifies an URL for token service.
	 * @return reference to this object
	 */
	public OmmConsumerConfig tokenServiceUrlV2(String tokenServiceUrlV2);
	
	/**
	 * Specifies an URL to override the default for the RDP service discovery to get global endpoints.
	 * <p>Defaults to "https://api.refinitiv.com/streaming/pricing/v1/".</p>
	 * @param serviceDiscoveryUrl specifies an URL for RDP service discovery.
	 * @return reference to this object
	 */
	public OmmConsumerConfig serviceDiscoveryUrl(String serviceDiscoveryUrl);

	/**
	 * Specifies a hostname and port. Overrides prior value.
	 * Implies usage of TCP IP channel or RSSL connection type socket.
	 *       
	 * @param host specifies server and port to which OmmConsumer will connect.
	 * <br>If host set to "&lt;hostname&gt;:&lt;port&gt;", then hostname:port is assumed.
	 * <br>If host set to "", then localhost:14002 is assumed.
	 * <br>If host set to ":", then localhost:14002 is assumed.
	 * <br>If host set to "&lt;hostname&gt;", then hostname:14002 is assumed.
	 * <br>If host set to "&lt;hostname&gt;:", then hostname:14002 is assumed.
	 * <br>If host set to ":&lt;port&gt;", then localhost:port is assumed.
	 * 
	 * @return reference to this object
	 */
	public OmmConsumerConfig host(String host);

	/**
	 * Specifies the operation model, overriding the default.<br>
	 * The operation model specifies whether to dispatch messages
	 * in the user or application thread of control.
	 * 
	 * @param operationModel specifies threading and dispatching model used by application
	 * @return reference to this object
	 */
	public OmmConsumerConfig operationModel(int operationModel);

	/**
	 * Create an OmmConsumer with consumer name.<br>
	 * The OmmConsumer enables functionality that includes
     * subscribing, posting and distributing generic messages.<br>
     * This name identifies configuration section to be used by OmmConsumer instance.
     * 
	 * @param consumerName specifies name of OmmConsumer instance
	 * @return reference to this object
	 */
	public OmmConsumerConfig consumerName(String consumerName);

	/**
	 * The address or host name of HTTP proxy server to connect to.
     * 
	 * @param proxyHostName specifies the address or host name of HTTP proxy server
	 *  for tunneling connection.
	 * @return reference to this object
	 */
	public OmmConsumerConfig tunnelingProxyHostName(String proxyHostName); 
	
	/**
	 * The port number of HTTP proxy server to connect to. Must be in the range of 0 - 65535.
     * 
	 * @param proxyPort specifies the port number of HTTP proxy server for tunneling connection.
	 * @return reference to this object
	 */
	public OmmConsumerConfig tunnelingProxyPort(String proxyPort); 
	
	/**
	 * The object name for load balancing to the various providers that are part of a hosted solution.
     * 
	 * @param objectName specifies object name for load balancing used for tunneling connection.
	 * @return reference to this object
	 */
	public OmmConsumerConfig tunnelingObjectName(String objectName);  
	
	/**
	 * The user name to authenticate. Needed for all authentication protocols.
     * 
	 * @param userName specifies user name used for tunneling connection.
	 * @return reference to this object
	 */
	public OmmConsumerConfig tunnelingCredentialUserName(String userName);
	
	/**
	 * The passwd to authenticate. Needed for all authentication protocols.
     * 
	 * @param passwd specifies password used for tunneling connection.
	 * @return reference to this object
	 */
	public OmmConsumerConfig tunnelingCredentialPasswd(String passwd);  

	/**
	  * The domain of the user to authenticate.
     *  Needed for NTLM or for Negotiate/Kerberos or for Kerberos authentication protocols.
     * 
     * 	For Negotiate/Kerberos or for Kerberos authentication protocols, tunnelingCredentialDomain
     * 	should be the same as the domain in the 'realms' and 'domain_realm' sections of
     * 	the Kerberos configuration file ({@link #tunnelingCredentialKRB5ConfigFile(String krb5ConfigFile)}).
     * 
	 * @param domain specifies the domain used for tunneling connection.
	 * @return reference to this object
	 */
	public OmmConsumerConfig tunnelingCredentialDomain(String domain); 
	
	/**
	 * The complete path of the Kerberos5 configuration file (krb5.ini or krb5.conf, or custom file).
     * Needed for Negotiate/Kerberos and Kerberos authentication protocols.
     * 
	 * @param krb5ConfigFile specifies the full path of kerberos5 config file for tunneling connection.
	 * @return reference to this object
	 */
	public OmmConsumerConfig tunnelingCredentialKRB5ConfigFile(String krb5ConfigFile); 
	
	/**
	 * The localHostName of the client. Needed for NTLM authentication protocol only.
     * 
	 * @param localHostName specifies the client local host name used for tunneling connection.
	 * @return reference to this object
	 */
	public OmmConsumerConfig tunnelingCredentialLocalHostName(String localHostName);  
	
	/**
	 * The type of the key store for certificate file.
     * RTSDK Default = JKS
     * 
	 * @param keyStoreType specifies the type of key store for tunneling connection.
	 * @return reference to this object
	 */
    public OmmConsumerConfig tunnelingKeyStoreType(String keyStoreType);
    
    /**
	 * The key store file that contains your own private keys, and public key certificates you received
	 * from someone else.
     * 
	 * @param keyStoreFile specifies key storing file for tunneling connection. The JDK utility 'keytool' creates this file.
	 * @return reference to this object
	 */
    public OmmConsumerConfig tunnelingKeyStoreFile(String keyStoreFile);
    
    /**
	 * The passwd for the key store file. 
     * 
	 * @param keyStorePasswd specifies passwd for key storing file for tunneling connection.
	 * @return reference to this object
	 */
    public OmmConsumerConfig tunnelingKeyStorePasswd(String keyStorePasswd);  
	
    /**
	 * The Cryptographic protocol to be used. Sun JDK default is TLS which will go the latest one
	 * supported by JDK (currently is TLSv1.2).
	 *  
	 * @param securityProtocol specifies a cryptographic protocol for tunneling connection.
	 * @return reference to this object
	 */
    public OmmConsumerConfig tunnelingSecurityProtocol(String securityProtocol); 
    
    /**
	 * The Java Cryptography Package provider to be used. The Oracle JDK default is SunJSSE.
     * 
	 * @param securityProvider specifies a java cryptography package provider for tunneling connection.
	 * @return reference to this object
	 */
    public OmmConsumerConfig tunnelingSecurityProvider(String securityProvider); 
    
    /**
	 * The Java Key Management algorithm to be used. 
     * Defaults to the property ssl.KeyManagerFactory.algorithm in the JDK security properties file (java.security).
     * Sun JDK default = SunX509 
     * 
	 * @param KeyManagerAlgorithm specifies a java key manager algorithm for tunneling connection.
	 * @return reference to this object
	 */
    public OmmConsumerConfig tunnelingKeyManagerAlgorithm(String KeyManagerAlgorithm); 
    
    /**
	 * The Java Trust Management algorithm to be used.
     * Defaults to the property ssl.TrustManagerFactory.algorithm in the JDK security properties file (java.security).
     * Sun JDK default = PKIX
     * 
	 * @param trustManagerAlgorithm specifies a java trust manager algorithm for tunneling connection.
	 * @return reference to this object
	 */
    public OmmConsumerConfig tunnelingTrustManagerAlgorithm(String trustManagerAlgorithm); 
    
	/**
	 * Specifies the local configuration, overriding and adding to the current content.
	 * 
	 * @param config specifies OmmConsumer configuration
	 * @return reference to this object
	 */
	public OmmConsumerConfig config(Data config);

	/**
	 * Specifies an administrative request message to override the default administrative request.<br>
	 * Application may call multiple times prior to initialization.<br>
	 * Supported domains include Login, Directory, and Dictionary.
	 * 
	 * @param reqMsg specifies administrative domain request message
	 * @return reference to this object
	 */
	public OmmConsumerConfig addAdminMsg(ReqMsg reqMsg);

	/**
	 * Specifies the DataDictionary object.
	 * Overrides DataDictionary object that is provided via EmaConfig.xml or
	 * Programmatic configure.
	 * 
	 * If shouldCopyIntoAPI is true, the DataDictionary object will be copied
	 * into the application space, otherwise it will be passed in as a reference.
	 *
	 * @param dataDictionary specifies the DataDictionary object.
	 * @param shouldCopyIntoAPI specifies whether to copy dataDictionary into API or pass in as reference.
	 * @return reference to this object.
	 * @throws OmmInvalidUsageException if dataDictionary object instance does not contain entire dictionary information.
	 */
	public OmmConsumerConfig dataDictionary(DataDictionary dataDictionary, boolean shouldCopyIntoAPI);
}
