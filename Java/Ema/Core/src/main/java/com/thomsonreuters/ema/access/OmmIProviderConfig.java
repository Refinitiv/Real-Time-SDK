///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

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
	
}
