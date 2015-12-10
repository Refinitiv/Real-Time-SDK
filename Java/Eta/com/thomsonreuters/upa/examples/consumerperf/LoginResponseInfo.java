package com.thomsonreuters.upa.examples.consumerperf;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.UInt;

/* login response information */
public class LoginResponseInfo
{
	public int			streamId;
	public Buffer	username;
	public Buffer	applicationId;
	public Buffer	applicationName;
	public Buffer	position;
	public UInt	providePermissionProfile;
	public UInt	providePermissionExpressions;
	public UInt	singleOpen;
	public UInt	allowSuspectData;
	public UInt	supportPauseResume;
	public UInt	supportOptimizedPauseResume;
	public UInt	supportOMMPost;
	public UInt	supportViewRequests;
	public UInt	supportBatchRequests;
	public UInt	supportStandby;
	public boolean isSolicited;
	
	public LoginResponseInfo()
	{
		username = CodecFactory.createBuffer();
		applicationId = CodecFactory.createBuffer();
		applicationName = CodecFactory.createBuffer();
		position = CodecFactory.createBuffer();
		providePermissionProfile = CodecFactory.createUInt();
		providePermissionExpressions = CodecFactory.createUInt();
		singleOpen = CodecFactory.createUInt();
		allowSuspectData = CodecFactory.createUInt();
		supportPauseResume = CodecFactory.createUInt();
		supportOptimizedPauseResume = CodecFactory.createUInt();
		supportOMMPost = CodecFactory.createUInt();
		supportViewRequests = CodecFactory.createUInt();
		supportBatchRequests = CodecFactory.createUInt();
		supportStandby = CodecFactory.createUInt();
	}
		
	/*
	 * Clears the login response information.
	 */
	public void clear()
	{
		streamId = 0;
		username.clear();
		applicationId.clear();
		applicationName.clear();
		position.clear();
		providePermissionProfile.clear();
		providePermissionExpressions.clear();
		singleOpen.clear();
		allowSuspectData.clear();
		supportPauseResume.clear();
		supportOptimizedPauseResume.clear();
		supportOMMPost.clear();
		supportViewRequests.clear();
		supportBatchRequests.clear();
		supportStandby.clear();
		isSolicited = false;
	}

	/*
	 * Initializes the login response information.
	 */
	public void init()
	{
		clear();
		providePermissionProfile.value(1);
		providePermissionExpressions.value(1);
		singleOpen.value(1);
		allowSuspectData.value(1);
	}
	
	public String toString()
	{
		return "\nLoginResponseInfo" + "\n" +
		"\tstreamId: " + streamId + "\n" +
		"\tusername: " + username.toString() + "\n" +
		"\tapplicationId: " + applicationId.toString() + "\n" +
		"\tapplicationName: " + applicationName.toString() + "\n" +
		"\tposition: " + position.toString() + "\n" +
		"\tprovidePermissionProfile: " + providePermissionProfile.toLong() + "\n" +
		"\tprovidePermissionExpressions: " + providePermissionExpressions.toLong() + "\n" +
		"\tsingleOpen: " + singleOpen.toLong() + "\n" +
		"\tallowSuspectData: " + allowSuspectData.toLong() + "\n" +
		"\tsupportPauseResume: " + supportPauseResume.toLong() + "\n" +
		"\tsupportOptimizedPauseResume: " + supportOptimizedPauseResume.toLong() + "\n" +
		"\tsupportOMMPost: " + supportOMMPost.toLong() + "\n" +
		"\tsupportViewRequests: " + supportViewRequests.toLong() + "\n" +
		"\tsupportBatchRequests: " + supportBatchRequests.toLong() + "\n" +
		"\tsupportStandby: " + supportStandby.toLong() + "\n" +
		"\tisSolicited: " + isSolicited;
	}
}
