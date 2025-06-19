/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020-2021,2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

/**
 * OmmIProviderConfig is used to specify configuration and behavior of Interactive OmmProvider.
 * 
 * <p>OmmIProviderConfig provides a default basic Interactive OmmProvider configuration.</p>
 * 
 * <p>The default configuration may be modified and or appended by using the EmaConfig.xml<br>
 * file or any methods of this class.</p>
 * 
 * <p>The EmaConfig.xml file is read in if it is present in the working directory of the application.</p>
 * 
 * <p>Calling any interface methods of OmmIProviderConfig class overrides or appends the existing<br>
 * configuration.</p>
 * 
 * @see OmmProvider
 * @see OmmProviderConfig
 */


public interface OmmIProviderConfig extends OmmProviderConfig
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
		 * specifies user submit directory and dictionary message
		 */
		public static final int USER_CONTROL = 0;
		
		/**
		 * specifies API sends down directory and dictionary refresh message based on the configuration
		 */
		public static final int API_CONTROL = 1;
	}
	
	/**
	 * Clears the OmmIProviderConfig and sets all the defaults.
	 * Invoking clear() method clears all the values and resets all the defaults.
	 * 
	 * @return reference to this object
	 */
	public OmmIProviderConfig clear();
	
	/**
	 * Specifies a port. Overrides prior value.
	 * Implies usage of TCP IP channel or RSSL connection type socket.
	 *       
	 * @param port specifies server port on which OmmProvider will accept client connections.
	 * <br>If port set to "", then 14002 is assumed.
	 * 
	 * @return reference to this object
	 */
	public OmmIProviderConfig port(String port);
	
	/**
	 * Specifies the operation model, overriding the default<br>
	 * The operation model specifies whether to dispatch messages
	 * in the user or application thread of control.
	 * 
	 * @param operationModel specifies threading and dispatching model used by application
	 * @return reference to this object
	 */
	public OmmIProviderConfig operationModel(int operationModel);
	
	/**
	 * Specifies whether API or user controls sending of Directory<br>
	 * refresh message.
	 * 
	 * @param control specifies who sends down the directory refresh message
	 * @return reference to this object
	 */
	public OmmIProviderConfig adminControlDirectory(int control);
	
	/**
	 * Specifies whether API or user controls sending of Dictionary<br>
	 * refresh message.
	 * 
	 * @param control specifies who sends down the dictionary refresh message
	 * @return reference to this object
	 */
	public OmmIProviderConfig adminControlDictionary(int control);
	
	/**
	 * Create an OmmProvider with provider name.<br>
     * This name identifies configuration section to be used by OmmProvider instance.
     * 
	 * @param providerName specifies name of OmmProvider instance
	 * @return reference to this object
	 */
	public OmmIProviderConfig providerName(String providerName);
	
	/**
	 * Specifies the local configuration, overriding and adding to the current content.
	 * 
	 * @param config specifies OmmProvider configuration
	 * @return reference to this object
	 */
	public OmmIProviderConfig config(Data config);
	
	/**
	 * Specifies an administrative refresh message to override the default administrative refresh.<br>
	 * Application may call multiple times prior to initialization.<br>Supports Directory and Dictionary domain only.
	 * 
	 * @param refreshMsg specifies administrative domain refresh message
	 * @return reference to this object
	 */
	public OmmIProviderConfig addAdminMsg(RefreshMsg refreshMsg);
	
	/**
	 * Specifies the keystore file for an encrypted connection, containing the server private key and certificate.
	 * This is optional, and the default behavior will load the JVM's default keystore set with javax.net.ssl.keystore.
	 * 
	 * @param keystoreFile the file name of the keystore file
	 * @return reference to this object
	 */
	public OmmIProviderConfig keystoreFile(String keystoreFile);
	
	/**
	 * Specifies the keystore password for the configured keystore.
	 * This is optional, and the default behavior will load the JVM's default keystore set with javax.net.ssl.keystorepassword.
	 * 
	 * @param keystorePasswd the keystore password
	 * @return reference to this object
	 */
	public OmmIProviderConfig keystorePasswd(String keystorePasswd);
	
	/**
	 * Specifies the keystore type for the configured keystore.
	 * This is optional, and the default behavior is set to "JKS".
	 * 
	 * @param keystoreType the keystore type
	 * @return reference to this object
	 */
	public OmmIProviderConfig keystoreType(String keystoreType);
	
	/**
	 * Specifies the security Protocol type for the configured server.
	 * This is optional, and the default behavior is set to "TLS".
	 * 
	 * @param securityProtocol the security protocol
	 * @return reference to this object
	 */
	public OmmIProviderConfig securityProtocol(String securityProtocol);
	
    /**
	 * The Cryptographic protocol versions to be used. RTSDK default is {"1.3" , "1.2"} for the default protocol "TLS"
	 * which will go to the latest one supported by the JDK version in use.
	 *  
	 * @param securityProtocolVersions specifies a cryptographic protocol versions list to use for the connection.
	 * @return reference to this object
	 */
    public OmmIProviderConfig securityProtocolVersions(String[] securityProtocolVersions); 
	
	/**
	 * Specifies the security provider type for the configured server.
	 * This is optional, and the default behavior is set to "SunJSSE".
	 * 
	 * @param securityProvider the security provider
	 * @return reference to this object
	 */
	public OmmIProviderConfig securityProvider(String securityProvider);
	
	/**
	 * Specifies the key manager algorithm for the configured server.
	 * This is optional, and the default behavior is set to "SunX509".
	 * 
	 * @param keyManagerAlgorithm the key manager algorithm
	 * @return reference to this object
	 */
	public OmmIProviderConfig keyManagerAlgorithm(String keyManagerAlgorithm);
	
	/**
	 * Specifies the trust manager algorithm for the configured server.
	 * This is optional, and the default behavior is set to "PKIX".
	 * 
	 * @param trustManagerAlgorithm the trust manager algorithm
	 * @return reference to this object
	 */
	public OmmIProviderConfig trustManagerAlgorithm(String trustManagerAlgorithm);
	
}
