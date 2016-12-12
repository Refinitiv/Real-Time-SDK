package com.thomsonreuters.upa.valueadd.domainrep.rdm.login;

import com.thomsonreuters.upa.codec.Buffer;

/**
 * The RDM login attrib. LoginAttrib be used to send additional authentication
 * information and user preferences between the components.
 */
public interface LoginAttrib {
	/**
	 * Sets login attrib flags. Populated by {@link LoginAttribFlags}.
	 * 
	 * @param flags
	 */
	public void flags(int flags);

	/**
	 * Returns login request attrib flags. Populated by {@link LoginAttribFlags}
	 * 
	 * @return flags
	 */
	public int flags();

	/**
	 * Performs a deep copy of {@link LoginAttrib} object.
	 * 
	 * @param destAttrib
	 *            Login attrib object to copy this object into. It cannot be
	 *            null.
	 * 
	 * @return UPA return value indicating success or failure of copy operation.
	 */
	public int copy(LoginAttrib destAttrib);

	/**
	 * Clears the current contents of the login attrib object and prepares it
	 * for re-use.
	 */
	public void clear();

	/**
	 * Returns DACS application id for the login message.
	 * 
	 * @return The DACS Application ID.
	 */
	public Buffer applicationId();

	/**
	 * Sets DACS application id for the login message. Note that this creates
	 * garbage if buffer is backed by String object.
	 * 
	 * @param applicationId    -DACS Application ID.
	 */
	public void applicationId(Buffer applicationId);

	/**
	 * Checks the presence of application id field.
	 * 
	 * This flag can also be bulk-get by {@link #flags()}
	 * 
	 * @return true - if application id is present, false - if not.
	 */
	public boolean checkHasApplicationId();

	/**
	 * Applies application id presence flag.
	 * 
	 * This flag can also be bulk-set by {@link #flags(int)}
	 * 
	 */
	public void applyHasApplicationId();

	/**
	 * Returns applicationName for the login message. This field identifies the
	 * application sending the Login request or response message. When present,
	 * the application name in the Login request identifies the OMM Consumer and
	 * the application name in the Login response identifies the OMM provider.
	 * 
	 * @return The ApplicationName
	 */
	public Buffer applicationName();

	/**
	 * Sets applicationName for the login message. Note that this method creates
	 * garbage if buffer is backed by String object.
	 * 
	 * @param applicationName
	 */
	public void applicationName(Buffer applicationName);

	/**
	 * Checks the presence of application name field.
	 * 
	 * This flag can also be bulk-get by {@link #flags()}
	 * 
	 * @return true - if application name is present, false - if not.
	 */
	public boolean checkHasApplicationName();

	/**
	 * Applies application name presence flag.
	 * 
	 * This flag can also be bulk-set by {@link #flags(int)}
	 */
	public void applyHasApplicationName();

	/**
	 * Returns the DACS position for login message. If the server authenticates
	 * with DACS, the consumer application might be required to pass in a valid
	 * position. If present, this should match whatever was sent in the request
	 * or be set to the IP address of the connected client.
	 * 
	 * @return position
	 */
	public Buffer position();

	/**
	 * Sets the DACS position for login message. If the server authenticates
	 * with DACS, the consumer application might be required to pass in a valid
	 * position. If present, this should match whatever was sent in the request
	 * or be set to the IP address of the connected client. Note that this
	 * method creates garbage if buffer is backed by String object.
	 * 
	 * @param position
	 */
	public void position(Buffer position);

	/**
	 * Checks the presence of position field.
	 * 
	 * This flag can also be bulk-get by {@link #flags()}
	 * 
	 * @return true - if position is present, false - if not.
	 */
	public boolean checkHasPosition();

	/**
	 * Applies position presence flag.
	 * 
	 * This flag can also be bulk-set by {@link #flags(int)}
	 */
	public void applyHasPosition();

	/**
	 * Returns providePermissionProfile for the login message. If specified on
	 * the Login Refresh, indicates that the permission profile is provided.
	 * This is typically present because the login request message requested
	 * this information. An application can use the permission profile to
	 * perform proxy permissioning.
	 * 
	 * @return providePermissionProfile
	 */
	public long providePermissionProfile();

	/**
	 * Sets providePermissionProfile for the login message. If specified on the
	 * Login Refresh, indicates that the permission profile is provided. This is
	 * typically present because the login request message requested this
	 * information. An application can use the permission profile to perform
	 * proxy permissioning.
	 * 
	 * @param providePermissionProfile
	 */
	public void providePermissionProfile(long providePermissionProfile);

	/**
	 * Checks the presence of provide permission profile field.
	 * 
	 * This flag can also be bulk-get by {@link #flags()}
	 * 
	 * @return true - if provide permission profile field is present, false - if
	 *         not.
	 */
	public boolean checkHasProvidePermissionProfile();

	/**
	 * Applies provide permission profile presence flag.
	 * 
	 * This flag can also be bulk-set by {@link #flags(int)}
	 */
	public void applyHasProvidePermissionProfile();

	/**
	 * Sets providePermissionExpressions for the login message. If specified on
	 * a Login Refresh, indicates that a provider will send permission
	 * expression information with its responses. ProvidePermissionExpressions
	 * is typically present because the login request message requested this
	 * information. Permission expressions allow for items to be proxy
	 * permissioned by a consumer via content-based entitlements.
	 * 
	 * @return providePermissionExpressions
	 */
	public long providePermissionExpressions();

	/**
	 * Returns providePermissionExpressions for the login message. If specified
	 * on a Login Refresh, indicates that a provider will send permission
	 * expression information with its responses. ProvidePermissionExpressions
	 * is typically present because the login request message requested this
	 * information. Permission expressions allow for items to be proxy
	 * permissioned by a consumer via content-based entitlements.
	 * 
	 * @param providePermissionExpressions
	 */
	public void providePermissionExpressions(long providePermissionExpressions);

	/**
	 * Checks the presence of provide permission expressions field.
	 * 
	 * This flag can also be bulk-get by {@link #flags()}
	 * 
	 * @return true - if provide permission expressions field is present, false
	 *         - if not.
	 */
	public boolean checkHasProvidePermissionExpressions();

	/**
	 * Applies provide permission expressions presence flag.
	 * 
	 * This flag can also be bulk-set by {@link #flags(int)}
	 */
	public void applyHasProvidePermissionExpressions();

	/**
	 * Returns singleOpen field of the login message. Value of 1 indicated
	 * provider drives stream recovery. 0 indicates provider does not drive
	 * stream recovery; it is the responsibility of the downstream application.
	 * 
	 * @return singleOpen
	 */
	public long singleOpen();

	/**
	 * Sets singleOpen field of the login message. Value of 1 indicated provider
	 * drives stream recovery. 0 indicates provider does not drive stream
	 * recovery; it is the responsibility of the downstream application.
	 * 
	 * @param singleOpen
	 */
	public void singleOpen(long singleOpen);

	/**
	 * Checks the presence of single open field.
	 * 
	 * This flag can also be bulk-get by {@link #flags()}
	 * 
	 * @return true - if single open field is present, false - if not.
	 */
	public boolean checkHasSingleOpen();

	/**
	 * Applies single open presence flag.
	 * 
	 * This flag can also be bulk-set by {@link #flags(int)}
	 */
	public void applyHasSingleOpen();

	/**
	 * Returns allowSuspectData for the login message. Value of 1 indicated
	 * provider application passes along suspect streamState information. 0
	 * indicates provider application does not pass along suspect data.
	 * 
	 * @return allowSuspectData
	 */
	public long allowSuspectData();

	/**
	 * Sets allowSuspectData for the login message. Value of 1 indicated
	 * provider application passes along suspect streamState information. 0
	 * indicates provider application does not pass along suspect data.
	 * 
	 * @param allowSuspectData
	 */
	public void allowSuspectData(long allowSuspectData);

	/**
	 * Checks the presence of allow suspect data field.
	 * 
	 * This flag can also be bulk-get by {@link #flags()}
	 * 
	 * @return true - if allow suspect data field is present, false - if not.
	 */
	public boolean checkHasAllowSuspectData();

	/**
	 * Applies allow suspect data presence flag.
	 * 
	 * This flag can also be bulk-set by {@link #flags(int)}
	 */
	public void applyHasAllowSuspectData();
	
	/**
	 * Returns supportProviderDictionaryDownload for the login message.
	 * Value of 1 indicates that provider can request dictionary.
	 * 0 indicates that provider cannot request dictionary.
	 * 
	 * @return allowSuspectData
	 */
 	public long supportProviderDictionaryDownload();
 	
	/**
	 * Sets supportProviderDictionaryDownload for the login message.
	 * Value of 1 indicates that provider can request dictionary.
	 * 0 indicates that provider cannot request dictionary.
	 * 
	 * @param supportProviderDictionaryDownload
	 */
	public void supportProviderDictionaryDownload(long supportProviderDictionaryDownload);
	
	/**
	 * Checks the presence of provider support dictionary download field.
	 * 
	 * This flag can also be bulk-get by {@link #flags()}
	 * 
	 * @return true - if provider support dictionary download field is present, false - if not.
	 */
	public boolean checkHasProviderSupportDictionaryDownload();

	/**
	 * Applies provider support dictionary download flag.
	 * 
	 * This flag can also be bulk-set by {@link #flags(int)}
	 */
	public void applyHasProviderSupportDictionaryDownload();
}
