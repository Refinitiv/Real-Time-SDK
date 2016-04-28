///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

/**
 * OmmConsumerConfig is used to modify configuration and behaviour of OmmConsumer.
 * <p>OmmConsumerConfig provides a default basic OmmConsumer configuration.</p>
 * 
 * <p>The default configuration may be modified and or appended by using
 * any methods from OmmConsumerConfg.</p>
 * 
 * <p>OmmConsumerconfig methods override or append the existing configuration.</p>
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
}