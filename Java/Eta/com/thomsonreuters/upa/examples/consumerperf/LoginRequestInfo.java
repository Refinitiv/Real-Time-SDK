package com.thomsonreuters.upa.examples.consumerperf;

import com.thomsonreuters.upa.transport.Channel;

public class LoginRequestInfo
{
	public int			streamId;
	public String		username;
	public String		applicationId;
	public String		applicationName;
	public String		position;
	public String		password;
	public int			providePermissionProfile;
	public int			providePermissionExpressions;
	public int			singleOpen;
	public int			allowSuspectData;
	public String		instanceId;
	public int			role;
	public int			downloadConnectionConfig;
	public Channel	chnl;
	public boolean		isInUse;
	
	/*
	 * Clears the login request information.
	 */
	public void clear()
	{
		streamId = 0;
		username = null;
		applicationId = null;
		applicationName = null;
		position = null;
		password = null;
		providePermissionProfile = 0;
		providePermissionExpressions = 0;
		singleOpen = 0;
		allowSuspectData = 0;
		instanceId = null;
		role = 0;
		downloadConnectionConfig = 0;
		chnl = null;
		isInUse = false;
	}

	/*
	 * Initializes the login request information.
	 */
	public void init()
	{
		clear();
		providePermissionProfile = 1;
		providePermissionExpressions = 1;
		singleOpen = 1;
		allowSuspectData = 1;
	}
}
