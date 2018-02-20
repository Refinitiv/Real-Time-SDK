///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2016. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

/**
 * OmmNiProviderConfig is used to modify configuration and behavior of OmmProvider<br>
 * for non-interactive application.
 * 
 * <p>OmmNiProviderConfig provides a default basic OmmProvider configuration.</p>
 * 
 * <p>The default configuration may be modified and or appended by using the EmaConfig.xml<br>
 * file or any methods of this class.</p>
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
