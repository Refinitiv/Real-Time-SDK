/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2018. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "JNIProtocol.h"
#include "JNIChannel.h"
#include "JNIServer.h"
#include "rtr/rsslTransport.h"
#include "rtr/rsslRetCodes.h"
#include "rtr/rsslQueue.h"
#include "rtr/rsslThread.h"
#include <stdlib.h>
#ifdef _WIN32
#include <winsock2.h>
#include <WS2tcpip.h>
#include <process.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/timeb.h>
#endif

#define RSSL_JNI_NULL_FD -1

typedef struct {
	RsslQueueLink link1;
	int cChnlScktFD;
	int jChnlScktFD;
	int connectionType;
	RsslBool isSrvrChnl;
	int numSrvrNotifies;
} JNIChnlFDs;

typedef struct {
	RsslQueueLink link1;
	int cSrvrScktFD;
	int jSrvrPort;
} JNISrvrFDs;

static RsslQueue JNIChnlFDList;
static RsslQueue JNISrvrFDList;

static RsslBool jniInitialized = RSSL_FALSE;
static long selectLoopThreadId = 0;
static int selectLoopServerFD = 0;
static int selectLoopServerPort = 49210;
static int javaServerPort = 49310;
fd_set	readfds;
fd_set	wrtfds;
fd_set	exceptfds;
RsslMutex fdsLock;

/* UTILITY FUNCTIONS */

/* get lock */
RTR_C_ALWAYS_INLINE void getLock()
{
	RSSL_MUTEX_LOCK(&fdsLock);
}

/* release lock */
RTR_C_ALWAYS_INLINE void releaseLock()
{
	RSSL_MUTEX_UNLOCK(&fdsLock);
}

/* initialize JNIChnlFDList and JNISrvrFDList */
static void initJNILists()
{
	/* return if already initialized */
	if (jniInitialized)
	{
		return;
	}

	rsslInitQueue(&JNISrvrFDList);
	rsslInitQueue(&JNIChnlFDList);

	jniInitialized = RSSL_TRUE;
}

/* get the C RsslChannel structure from the Java JNIChannel class */
static RsslChannel* getCChannel(JNIEnv *env, jobject *jchnl)
{
	static jclass jniChannelClass = NULL;
	jclass localRefClass;
	static jfieldID fid = NULL;

	if (!*jchnl)
	{
		return NULL;
	}

	/* get the JNIChannel class */
	if (jniChannelClass == NULL)
	{
		localRefClass =  (*env)->GetObjectClass(env, *jchnl);
		if (localRefClass == NULL)
		{
			return RSSL_FALSE;
		}
		/* Create a global reference */
		jniChannelClass = (*env)->NewGlobalRef(env, localRefClass);
		(*env)->DeleteLocalRef(env, localRefClass);
		if (jniChannelClass == NULL)
		{
			return RSSL_FALSE;
		}
	}

	/* get rsslChannelCPtr field id */
	if (fid == NULL)
	{
		fid = (*env)->GetFieldID(env, jniChannelClass, "_rsslChannelCPtr", "J");
		if (fid == NULL)
		{
			return NULL;
		}
	}

	/* return C RSSL channel structure from field */
	return (RsslChannel *)(*env)->GetLongField(env, *jchnl, fid);
}

/* copies a C RsslChannel structure to a Java JNIChannel class */
static RsslBool populateJavaChannel(JNIEnv *env, RsslChannel *rsslChnl, jobject *jchnl, RsslBool newFlag)
{
	static jclass jniChannelClass = NULL;
	jclass localRefClass;
	jfieldID fid;
	jstring jclientIP, jclientHostname;

	if (!rsslChnl || !*jchnl)
	{
		return RSSL_FALSE;
	}

	/* get the JNIChannel class */
	if (jniChannelClass == NULL)
	{
		localRefClass =  (*env)->GetObjectClass(env, *jchnl);
		if (localRefClass == NULL)
		{
			return RSSL_FALSE;
		}
		/* Create a global reference */
		jniChannelClass = (*env)->NewGlobalRef(env, localRefClass);
		(*env)->DeleteLocalRef(env, localRefClass);
		if (jniChannelClass == NULL)
		{
			return RSSL_FALSE;
		}
	}

	/* get socketId field id */
	fid = (*env)->GetFieldID(env, jniChannelClass, "_socketId", "I");
	if (fid == NULL)
	{
		return RSSL_FALSE;
	}

	/* set socketId field */
	(*env)->SetIntField(env, *jchnl, fid, rsslChnl->socketId);

	/* get oldSocketId field id */
	fid = (*env)->GetFieldID(env, jniChannelClass, "_oldSocketId", "I");
	if (fid == NULL)
	{
		return RSSL_FALSE;
	}

	/* set oldSocketId field */
	(*env)->SetIntField(env, *jchnl, fid, rsslChnl->oldSocketId);

	/* get state field id */
	fid = (*env)->GetFieldID(env, jniChannelClass, "_state", "I");
	if (fid == NULL)
	{
		return RSSL_FALSE;
	}

	/* set state field */
	(*env)->SetIntField(env, *jchnl, fid, rsslChnl->state);

	/* get connectionType field id */
	fid = (*env)->GetFieldID(env, jniChannelClass, "_connectionType", "I");
	if (fid == NULL)
	{
		return RSSL_FALSE;
	}

	/* set connectionType field */
	(*env)->SetIntField(env, *jchnl, fid, rsslChnl->connectionType);

	if (rsslChnl->clientIP)
	{
		/* get clientIP field id */
		fid = (*env)->GetFieldID(env, jniChannelClass, "_clientIP", "Ljava/lang/String;");
		if (fid == NULL)
		{
			return RSSL_FALSE;
		}

		/* set clientIP field */
		jclientIP = (*env)->NewStringUTF(env, rsslChnl->clientIP);
		(*env)->SetObjectField(env, *jchnl, fid, jclientIP);
	}

	if (rsslChnl->clientHostname)
	{
		/* get clientHostname field id */
		fid = (*env)->GetFieldID(env, jniChannelClass, "_clientHostname", "Ljava/lang/String;");
		if (fid == NULL)
		{
			return RSSL_FALSE;
		}

		/* set clientHostname field */
		jclientHostname = (*env)->NewStringUTF(env, rsslChnl->clientHostname);
		(*env)->SetObjectField(env, *jchnl, fid, jclientHostname);
	}

	/* get pingTimeout field id */
	fid = (*env)->GetFieldID(env, jniChannelClass, "_pingTimeout", "I");
	if (fid == NULL)
	{
		return RSSL_FALSE;
	}

	/* set pingTimeout field */
	(*env)->SetIntField(env, *jchnl, fid, rsslChnl->pingTimeout);

	/* get majorVersion field id */
	fid = (*env)->GetFieldID(env, jniChannelClass, "_majorVersion", "I");
	if (fid == NULL)
	{
		return RSSL_FALSE;
	}

	/* set majorVersion field */
	(*env)->SetIntField(env, *jchnl, fid, rsslChnl->majorVersion);

	/* get minorVersion field id */
	fid = (*env)->GetFieldID(env, jniChannelClass, "_minorVersion", "I");
	if (fid == NULL)
	{
		return RSSL_FALSE;
	}

	/* set minorVersion field */
	(*env)->SetIntField(env, *jchnl, fid, rsslChnl->minorVersion);

	/* get protocolType field id */
	fid = (*env)->GetFieldID(env, jniChannelClass, "_protocolType", "I");
	if (fid == NULL)
	{
		return RSSL_FALSE;
	}

	/* set protocolType field */
	(*env)->SetIntField(env, *jchnl, fid, rsslChnl->protocolType);

	/* populate userSpecObject and rsslChannelCPtr if new flag set */
	if (newFlag)
	{
		/* get userSpecObject field id */
		fid = (*env)->GetFieldID(env, jniChannelClass, "_userSpecObject", "Ljava/lang/Object;");
		if (fid == NULL)
		{
			return RSSL_FALSE;
		}

		/* set userSpecObject field */
		(*env)->SetObjectField(env, *jchnl, fid, rsslChnl->userSpecPtr);

		/* get rsslChannelCPtr field id */
		fid = (*env)->GetFieldID(env, jniChannelClass, "_rsslChannelCPtr", "J");
		if (fid == NULL)
		{
			return RSSL_FALSE;
		}

		/* set rsslChannelCPtr field */
		(*env)->SetLongField(env, *jchnl, fid, (jlong)rsslChnl);
	}

	return RSSL_TRUE;
}

/* writes an FD and connection type to a java channel */
static RsslBool writeJavaChannel(JNIEnv *env, RsslChannel *rsslChnl, jobject *jchnl)
{
	static jclass jniChannelClass = NULL;
	jclass localRefClass;
	static jmethodID mid = NULL;

	if (!rsslChnl || !*jchnl)
	{
		return RSSL_FALSE;
	}

	/* get the JNIChannel class */
	if (jniChannelClass == NULL)
	{
		localRefClass =  (*env)->GetObjectClass(env, *jchnl);
		if (localRefClass == NULL)
		{
			return RSSL_FALSE;
		}
		/* Create a global reference */
		jniChannelClass = (*env)->NewGlobalRef(env, localRefClass);
		(*env)->DeleteLocalRef(env, localRefClass);
		if (jniChannelClass == NULL)
		{
			return RSSL_FALSE;
		}
	}

	/* get the method ID for the writeJavaChannel(int) method */
	if (mid == NULL)
	{
		mid = (*env)->GetMethodID(env, jniChannelClass, "writeJavaChannel", "(II)V");
		if (mid == NULL)
		{
			return RSSL_FALSE;
		}
	}

	/* call writeJavaChannel method */
	(*env)->CallIntMethod(env, *jchnl, mid, rsslChnl->socketId, rsslChnl->connectionType);

	return RSSL_TRUE;
}

/* connects a java channel with the select server */
static RsslBool connectJavaChannel(JNIEnv *env, RsslChannel *rsslChnl, jobject *jchnl)
{
	static jclass jniChannelClass = NULL;
	jclass localRefClass;
	static jmethodID mid = NULL;

	if (!rsslChnl || !*jchnl)
	{
		return RSSL_FALSE;
	}

	/* get the JNIChannel class */
	if (jniChannelClass == NULL)
	{
		localRefClass =  (*env)->GetObjectClass(env, *jchnl);
		if (localRefClass == NULL)
		{
			return RSSL_FALSE;
		}
		/* Create a global reference */
		jniChannelClass = (*env)->NewGlobalRef(env, localRefClass);
		(*env)->DeleteLocalRef(env, localRefClass);
		if (jniChannelClass == NULL)
		{
			return RSSL_FALSE;
		}
	}

	/* get the method ID for the connectJavaChannel(int) method */
	if (mid == NULL)
	{
		mid = (*env)->GetMethodID(env, jniChannelClass, "connectJavaChannel", "(I)V");
		if (mid == NULL)
		{
			return RSSL_FALSE;
		}
	}

	/* call connectJavaChannel method */
	(*env)->CallIntMethod(env, *jchnl, mid, selectLoopServerPort);

	if (!writeJavaChannel(env, rsslChnl, jchnl))
	{
		return RSSL_FALSE;
	}

	return RSSL_TRUE;
}

/* clears the dummy bytes written to a java channel */
static RsslBool clearJavaChannel(JNIEnv *env, jobject *jchnl)
{
	static jclass jniChannelClass = NULL;
	jclass localRefClass;
	static jmethodID mid;

	if (!*jchnl)
	{
		return RSSL_FALSE;
	}

	/* get the JNIChannel class */
	if (jniChannelClass == NULL)
	{
		localRefClass =  (*env)->GetObjectClass(env, *jchnl);
		if (localRefClass == NULL)
		{
			return RSSL_FALSE;
		}
		/* Create a global reference */
		jniChannelClass = (*env)->NewGlobalRef(env, localRefClass);
		(*env)->DeleteLocalRef(env, localRefClass);
		if (jniChannelClass == NULL)
		{
			return RSSL_FALSE;
		}
	}

	/* get the method ID for the clearJavaChannel(int) method */
	if (mid == NULL)
	{
		mid = (*env)->GetMethodID(env, jniChannelClass, "clearJavaChannel", "()V");
		if (mid == NULL)
		{
			return RSSL_FALSE;
		}
	}

	/* call clearJavaChannel method */
	(*env)->CallVoidMethod(env, *jchnl, mid, selectLoopServerPort);

	return RSSL_TRUE;
}

/* close a java channel to the select server */
static RsslBool closeJavaChannel(JNIEnv *env, RsslChannel *rsslChnl, jobject *jchnl)
{
	static jclass jniChannelClass = NULL;
	jclass localRefClass;
	static jmethodID mid = NULL;

	if (!*jchnl)
	{
		return RSSL_FALSE;
	}

	/* get the JNIChannel class */
	if (jniChannelClass == NULL)
	{
		localRefClass =  (*env)->GetObjectClass(env, *jchnl);
		if (localRefClass == NULL)
		{
			return RSSL_FALSE;
		}
		/* Create a global reference */
		jniChannelClass = (*env)->NewGlobalRef(env, localRefClass);
		(*env)->DeleteLocalRef(env, localRefClass);
		if (jniChannelClass == NULL)
		{
			return RSSL_FALSE;
		}
	}

	/* get the method ID for the writeJavaChannel(int) method */
	if (mid == NULL)
	{
		mid = (*env)->GetMethodID(env, jniChannelClass, "writeJavaChannel", "(II)V");
		if (mid == NULL)
		{
			return RSSL_FALSE;
		}
	}

#ifdef RSSL_JNI_DEBUG
	printf("closeJavaChannel() called\n");
#endif

	/* write 0 to channel to indicate close */
	(*env)->CallIntMethod(env, *jchnl, mid, 0, rsslChnl->connectionType);

	return RSSL_TRUE;
}

/* binds a java server */
static RsslBool bindJavaServer(JNIEnv *env, RsslServer *rsslSrvr, jobject *jsrvr)
{
	JNISrvrFDs *jniSrvrFDs = NULL;
	static jclass jniServerClass = NULL;
	jclass localRefClass;
	static jmethodID mid = NULL;

	if (!rsslSrvr || !*jsrvr)
	{
		return RSSL_FALSE;
	}

	/* get the jniServerClass class */
	if (jniServerClass == NULL)
	{
		localRefClass =  (*env)->GetObjectClass(env, *jsrvr);
		if (localRefClass == NULL)
		{
			return RSSL_FALSE;
		}
		/* Create a global reference */
		jniServerClass = (*env)->NewGlobalRef(env, localRefClass);
		(*env)->DeleteLocalRef(env, localRefClass);
		if (jniServerClass == NULL)
		{
			return RSSL_FALSE;
		}
	}

	/* get the method ID for the bindJavaServer(int) method */
	if (mid == NULL)
	{
		mid = (*env)->GetMethodID(env, jniServerClass, "bindJavaServer", "(I)Z");
		if (mid == NULL)
		{
			return RSSL_FALSE;
		}
	}

	while (1)
	{
		/* call bindJavaServer method */
		if ((*env)->CallIntMethod(env, *jsrvr, mid, javaServerPort) == JNI_TRUE)
		{
			break;
		}
		javaServerPort++;
	}

	/* add java server FD to JNISrvrFDList */
	jniSrvrFDs = (JNISrvrFDs *)malloc(sizeof(JNISrvrFDs));
	jniSrvrFDs->jSrvrPort = javaServerPort;
	jniSrvrFDs->cSrvrScktFD = rsslSrvr->socketId;
	rsslInitQueueLink(&jniSrvrFDs->link1);
	rsslQueueAddLinkToBack(&JNISrvrFDList, &jniSrvrFDs->link1);
	getLock();
#ifdef RSSL_JNI_DEBUG
	printf("FD_SET(jniSrvrFDs->cSrvrScktFD, &readfds): %d\n", jniSrvrFDs->cSrvrScktFD);
#endif
	FD_SET(jniSrvrFDs->cSrvrScktFD, &readfds);
	releaseLock();

	return RSSL_TRUE;
}

/* accepts a java server */
static RsslBool acceptJavaServer(JNIEnv *env, RsslServer *rsslSrvr, jobject *jsrvr)
{
	static jclass jniServerClass = NULL;
	jclass localRefClass;
	static jmethodID mid = NULL;

	if (!*jsrvr)
	{
		return RSSL_FALSE;
	}

	/* get the jniServerClass class */
	if (jniServerClass == NULL)
	{
		localRefClass =  (*env)->GetObjectClass(env, *jsrvr);
		if (localRefClass == NULL)
		{
			return RSSL_FALSE;
		}
		/* Create a global reference */
		jniServerClass = (*env)->NewGlobalRef(env, localRefClass);
		(*env)->DeleteLocalRef(env, localRefClass);
		if (jniServerClass == NULL)
		{
			return RSSL_FALSE;
		}
	}

	/* get the method ID for the acceptJavaServer() method */
	if (mid == NULL)
	{
		mid = (*env)->GetMethodID(env, jniServerClass, "acceptJavaServer", "()V");
		if (mid == NULL)
		{
			return RSSL_FALSE;
		}
	}

	/* call acceptJavaServer method */
	(*env)->CallIntMethod(env, *jsrvr, mid);

	return RSSL_TRUE;
}

/* closes a java server */
static RsslBool closeJavaServer(JNIEnv *env, RsslServer *rsslSrvr, jobject *jsrvr)
{
	JNISrvrFDs *jniSrvrFDs = NULL;
	static jclass jniServerClass = NULL;
	jclass localRefClass;
	static jmethodID mid = NULL;
	RsslQueueLink *pLink;

	if (!rsslSrvr || !*jsrvr)
	{
		return RSSL_FALSE;
	}

	/* get the jniServerClass class */
	if (jniServerClass == NULL)
	{
		localRefClass =  (*env)->GetObjectClass(env, *jsrvr);
		if (localRefClass == NULL)
		{
			return RSSL_FALSE;
		}
		/* Create a global reference */
		jniServerClass = (*env)->NewGlobalRef(env, localRefClass);
		(*env)->DeleteLocalRef(env, localRefClass);
		if (jniServerClass == NULL)
		{
			return RSSL_FALSE;
		}
	}

	/* get the method ID for the closeJavaServer(int) method */
	if (mid == NULL)
	{
		mid = (*env)->GetMethodID(env, jniServerClass, "closeJavaServer", "(I)V");
		if (mid == NULL)
		{
			return RSSL_FALSE;
		}
	}

	/* close java server and remove java server FD from JNISrvrFDList */
	RSSL_QUEUE_FOR_EACH_LINK(&JNISrvrFDList, pLink)
	{
		jniSrvrFDs = RSSL_QUEUE_LINK_TO_OBJECT(JNISrvrFDs, link1, pLink);
		if (jniSrvrFDs->cSrvrScktFD == rsslSrvr->socketId)
		{
			/* call closeJavaServer method */
			(*env)->CallIntMethod(env, *jsrvr, mid, jniSrvrFDs->jSrvrPort);
			getLock();
#ifdef RSSL_JNI_DEBUG
			printf("FD_CLR(jniSrvrFDs->cSrvrScktFD, &readfds): %d\n", jniSrvrFDs->cSrvrScktFD);
#endif
			FD_CLR(jniSrvrFDs->cSrvrScktFD, &readfds);
			releaseLock();
			rsslQueueRemoveLink(&JNISrvrFDList, &jniSrvrFDs->link1);
			free(jniSrvrFDs);
			break;
		}
	}

	return RSSL_TRUE;
}

/* copies a C RsslChannelInfo structure to a Java ChannelInfo class */
static RsslBool populateJavaChannelInfo(JNIEnv *env, RsslChannelInfo *rsslChnlInfo, jobject *jchnlinfo)
{
	jclass channelInfoClass, mcastStatsClass;
	jmethodID mid;
	jstring jpriorityFlushStrategy, jcomponentVersion;
	jobject jmcastStatsObject;
	RsslUInt32 i;

	if (!rsslChnlInfo || !*jchnlinfo)
	{
		return RSSL_FALSE;
	}

	/* get the ChannelInfo class */
	channelInfoClass = (*env)->GetObjectClass(env, *jchnlinfo);

	/* get the method ID for the maxFragmentSize(int) method */
	mid = (*env)->GetMethodID(env, channelInfoClass, "maxFragmentSize", "(I)V");
	if (mid == NULL)
	{
		return RSSL_FALSE;
	}

	/* call maxFragmentSize method */
	(*env)->CallIntMethod(env, *jchnlinfo, mid, rsslChnlInfo->maxFragmentSize);

	/* get the method ID for the maxOutputBuffers(int) method */
	mid = (*env)->GetMethodID(env, channelInfoClass, "maxOutputBuffers", "(I)V");
	if (mid == NULL)
	{
		return RSSL_FALSE;
	}

	/* call maxOutputBuffers method */
	(*env)->CallIntMethod(env, *jchnlinfo, mid, rsslChnlInfo->maxOutputBuffers);

	/* get the method ID for the guaranteedOutputBuffers(int) method */
	mid = (*env)->GetMethodID(env, channelInfoClass, "guaranteedOutputBuffers", "(I)V");
	if (mid == NULL)
	{
		return RSSL_FALSE;
	}

	/* call guaranteedOutputBuffers method */
	(*env)->CallIntMethod(env, *jchnlinfo, mid, rsslChnlInfo->guaranteedOutputBuffers);

	/* get the method ID for the numInputBuffers(int) method */
	mid = (*env)->GetMethodID(env, channelInfoClass, "numInputBuffers", "(I)V");
	if (mid == NULL)
	{
		return RSSL_FALSE;
	}

	/* call numInputBuffers method */
	(*env)->CallIntMethod(env, *jchnlinfo, mid, rsslChnlInfo->numInputBuffers);

	/* get the method ID for the pingTimeout(int) method */
	mid = (*env)->GetMethodID(env, channelInfoClass, "pingTimeout", "(I)V");
	if (mid == NULL)
	{
		return RSSL_FALSE;
	}

	/* call pingTimeout method */
	(*env)->CallIntMethod(env, *jchnlinfo, mid, rsslChnlInfo->pingTimeout);

	/* get the method ID for the clientToServerPings(boolean) method */
	mid = (*env)->GetMethodID(env, channelInfoClass, "clientToServerPings", "(Z)V");
	if (mid == NULL)
	{
		return RSSL_FALSE;
	}

	/* call clientToServerPings method */
	(*env)->CallBooleanMethod(env, *jchnlinfo, mid, rsslChnlInfo->clientToServerPings);

	/* get the method ID for the serverToClientPings(boolean) method */
	mid = (*env)->GetMethodID(env, channelInfoClass, "serverToClientPings", "(Z)V");
	if (mid == NULL)
	{
		return RSSL_FALSE;
	}

	/* call serverToClientPings method */
	(*env)->CallBooleanMethod(env, *jchnlinfo, mid, rsslChnlInfo->serverToClientPings);

	/* get the method ID for the sysSendBufSize(int) method */
	mid = (*env)->GetMethodID(env, channelInfoClass, "sysSendBufSize", "(I)V");
	if (mid == NULL)
	{
		return RSSL_FALSE;
	}

	/* call sysSendBufSize method */
	(*env)->CallIntMethod(env, *jchnlinfo, mid, rsslChnlInfo->sysSendBufSize);

	/* get the method ID for the sysRecvBufSize(int) method */
	mid = (*env)->GetMethodID(env, channelInfoClass, "sysRecvBufSize", "(I)V");
	if (mid == NULL)
	{
		return RSSL_FALSE;
	}

	/* call sysRecvBufSize method */
	(*env)->CallIntMethod(env, *jchnlinfo, mid, rsslChnlInfo->sysRecvBufSize);

	/* get the method ID for the compressionType(int) method */
	mid = (*env)->GetMethodID(env, channelInfoClass, "compressionType", "(I)V");
	if (mid == NULL)
	{
		return RSSL_FALSE;
	}

	/* call compressionType method */
	(*env)->CallIntMethod(env, *jchnlinfo, mid, rsslChnlInfo->compressionType);

	/* get the method ID for the compressionThreshold(int) method */
	mid = (*env)->GetMethodID(env, channelInfoClass, "compressionThreshold", "(I)V");
	if (mid == NULL)
	{
		return RSSL_FALSE;
	}

	/* call compressionThreshold method */
	(*env)->CallIntMethod(env, *jchnlinfo, mid, rsslChnlInfo->compressionThreshold);

	/* get the method ID for the priorityFlushStrategy(String) method */
	mid = (*env)->GetMethodID(env, channelInfoClass, "priorityFlushStrategy", "(Ljava/lang/String;)V");
	if (mid == NULL)
	{
		return RSSL_FALSE;
	}

	/* call priorityFlushStrategy method */
	rsslChnlInfo->priorityFlushStrategy[sizeof(rsslChnlInfo->priorityFlushStrategy) - 1] = '\0';
	jpriorityFlushStrategy = (*env)->NewStringUTF(env, rsslChnlInfo->priorityFlushStrategy);
	(*env)->CallObjectMethod(env, *jchnlinfo, mid, jpriorityFlushStrategy);

	/* get the method ID for the multicastStats() method */
	mid = (*env)->GetMethodID(env, channelInfoClass, "multicastStats", "()Lcom/thomsonreuters/upa/transport/MCastStats;");
	if (mid == NULL)
	{
		return RSSL_FALSE;
	}

	/* call multicastStats method and get MCastStats object */
	jmcastStatsObject = (*env)->CallObjectMethod(env, *jchnlinfo, mid);
	if (jmcastStatsObject == NULL)
	{
		return RSSL_FALSE;
	}

	/* get the MCastStats class */
	mcastStatsClass = (*env)->GetObjectClass(env, jmcastStatsObject);

	/* get the method ID for the mcastSent(long) method */
	mid = (*env)->GetMethodID(env, mcastStatsClass, "mcastSent", "(J)V");
	if (mid == NULL)
	{
		return RSSL_FALSE;
	}

	/* call mcastSent method */
	(*env)->CallLongMethod(env, jmcastStatsObject, mid, rsslChnlInfo->multicastStats.mcastSent);

	/* get the method ID for the mcastRcvd(long) method */
	mid = (*env)->GetMethodID(env, mcastStatsClass, "mcastRcvd", "(J)V");
	if (mid == NULL)
	{
		return RSSL_FALSE;
	}

	/* call mcastRcvd method */
	(*env)->CallLongMethod(env, jmcastStatsObject, mid, rsslChnlInfo->multicastStats.mcastRcvd);

	/* get the method ID for the unicastSent(long) method */
	mid = (*env)->GetMethodID(env, mcastStatsClass, "unicastSent", "(J)V");
	if (mid == NULL)
	{
		return RSSL_FALSE;
	}

	/* call unicastSent method */
	(*env)->CallLongMethod(env, jmcastStatsObject, mid, rsslChnlInfo->multicastStats.unicastSent);

	/* get the method ID for the unicastRcvd(long) method */
	mid = (*env)->GetMethodID(env, mcastStatsClass, "unicastRcvd", "(J)V");
	if (mid == NULL)
	{
		return RSSL_FALSE;
	}

	/* call unicastRcvd method */
	(*env)->CallLongMethod(env, jmcastStatsObject, mid, rsslChnlInfo->multicastStats.unicastRcvd);

	/* get the method ID for the gapsDetected(long) method */
	mid = (*env)->GetMethodID(env, mcastStatsClass, "gapsDetected", "(J)V");
	if (mid == NULL)
	{
		return RSSL_FALSE;
	}

	/* call gapsDetected method */
	(*env)->CallLongMethod(env, jmcastStatsObject, mid, rsslChnlInfo->multicastStats.gapsDetected);

	/* get the method ID for the retransReqSent(long) method */
	mid = (*env)->GetMethodID(env, mcastStatsClass, "retransReqSent", "(J)V");
	if (mid == NULL)
	{
		return RSSL_FALSE;
	}

	/* call retransReqSent method */
	(*env)->CallLongMethod(env, jmcastStatsObject, mid, rsslChnlInfo->multicastStats.retransReqSent);

	/* get the method ID for the retransReqRcvd(long) method */
	mid = (*env)->GetMethodID(env, mcastStatsClass, "retransReqRcvd", "(J)V");
	if (mid == NULL)
	{
		return RSSL_FALSE;
	}

	/* call retransReqRcvd method */
	(*env)->CallLongMethod(env, jmcastStatsObject, mid, rsslChnlInfo->multicastStats.retransReqRcvd);

	/* get the method ID for the retransPktsSent(long) method */
	mid = (*env)->GetMethodID(env, mcastStatsClass, "retransPktsSent", "(J)V");
	if (mid == NULL)
	{
		return RSSL_FALSE;
	}

	/* call retransPktsSent method */
	(*env)->CallLongMethod(env, jmcastStatsObject, mid, rsslChnlInfo->multicastStats.retransPktsSent);

	/* get the method ID for the retransPktsRcvd(long) method */
	mid = (*env)->GetMethodID(env, mcastStatsClass, "retransPktsRcvd", "(J)V");
	if (mid == NULL)
	{
		return RSSL_FALSE;
	}

	/* call retransPktsRcvd method */
	(*env)->CallLongMethod(env, jmcastStatsObject, mid, rsslChnlInfo->multicastStats.retransPktsRcvd);

	/* get the method ID for the addCompVersionString(String) method */
	mid = (*env)->GetMethodID(env, channelInfoClass, "addCompVersionString", "(Ljava/lang/String;)V");
	if (mid == NULL)
	{
		return RSSL_FALSE;
	}

	/* call addCompVersionString method for each UPAC component version string */
	for (i = 0; i < rsslChnlInfo->componentInfoCount; i++)
	{
		jcomponentVersion = (*env)->NewStringUTF(env, rsslChnlInfo->componentInfo[i]->componentVersion.data);
		(*env)->CallObjectMethod(env, *jchnlinfo, mid, jcomponentVersion);
	}

	return RSSL_TRUE;
}

/* copies a Java ConnectOptions class to a C RsslConnectOptions structure  */
static RsslBool populateCConnectOptions(JNIEnv *env, jobject *jconnectopts, RsslConnectOptions *rsslConnectOpts)
{
	jclass connectOptsClass, tunnelingInfoClass, segmentedNetworkClass, unifiedNetworkClass;
	jclass tcpOptsClass, mcastOptsClass, shmemOptsClass;
	jmethodID mid;
	jobject jtunnelinginfo, jsegmented, junified, jtcpopts, jmcastopts, jshmemopts;
	jstring jaddress, jserviceName, jobjectName, jinterfaceName, junicastServiceName, jtcpControlPort, jcomponentVersion;
	const char *objectName, *address, *serviceName, *interfaceName, *unicastServiceName, *tcpControlPort, *componentVersion;

	if (!rsslConnectOpts || !*jconnectopts)
	{
		return RSSL_FALSE;
	}

	/* get the ConnectOptions class */
	connectOptsClass = (*env)->GetObjectClass(env, *jconnectopts);

	/* get the method ID for the tunnelingInfo() method */
	mid = (*env)->GetMethodID(env, connectOptsClass, "tunnelingInfo", "()Lcom/thomsonreuters/upa/transport/TunnelingInfo;");
	if (mid == NULL)
	{
		return RSSL_FALSE;
	}

	/* get tunnelingInfo object */
	jtunnelinginfo = (*env)->CallObjectMethod(env, *jconnectopts, mid);

	/* get the TunnelingInfo class */
	tunnelingInfoClass = (*env)->GetObjectClass(env, jtunnelinginfo);

	/* get the method ID for the componentVersion() method */
	mid = (*env)->GetMethodID(env, connectOptsClass, "componentVersion", "()Ljava/lang/String;");
	if (mid == NULL)
	{
		return RSSL_FALSE;
	}

	/* set C componentVersion */
	jcomponentVersion = (*env)->CallObjectMethod(env, *jconnectopts, mid);
	if (jcomponentVersion)
	{
		componentVersion = (*env)->GetStringUTFChars(env, jcomponentVersion, NULL);
		strncpy(rsslConnectOpts->componentVersion, componentVersion, 256);
		(*env)->ReleaseStringUTFChars(env, jcomponentVersion, componentVersion);
	}
	/* get the method ID for the objectName() method */
	mid = (*env)->GetMethodID(env, tunnelingInfoClass, "objectName", "()Ljava/lang/String;");
	if (mid == NULL)
	{
		return RSSL_FALSE;
	}

	/* set C objectName */
	jobjectName = (*env)->CallObjectMethod(env, jtunnelinginfo, mid);
	if (jobjectName)
	{
		objectName = (*env)->GetStringUTFChars(env, jobjectName, NULL);
		strcpy(rsslConnectOpts->objectName, objectName);
		(*env)->ReleaseStringUTFChars(env, jobjectName, objectName);
	}

	/* get the method ID for the connectionType() method */
	mid = (*env)->GetMethodID(env, connectOptsClass, "connectionType", "()I");
	if (mid == NULL)
	{
		return RSSL_FALSE;
	}

	/* set C connectionType */
	rsslConnectOpts->connectionType = (*env)->CallIntMethod(env, *jconnectopts, mid);

	/* determine if segmented or unified network is used by whether or not unified network address is null */

	/* get the method ID for the unifiedNetworkInfo() method */
	mid = (*env)->GetMethodID(env, connectOptsClass, "unifiedNetworkInfo", "()Lcom/thomsonreuters/upa/transport/UnifiedNetworkInfo;");
	if (mid == NULL)
	{
		return RSSL_FALSE;
	}

	/* get unifiedNetworkInfo object */
	junified = (*env)->CallObjectMethod(env, *jconnectopts, mid);

	/* get the UnifiedNetworkInfo class */
	unifiedNetworkClass = (*env)->GetObjectClass(env, junified);

	/* get the method ID for the address() method */
	mid = (*env)->GetMethodID(env, unifiedNetworkClass, "address", "()Ljava/lang/String;");
	if (mid == NULL)
	{
		return RSSL_FALSE;
	}

	/* get unified network address */
	jaddress = (*env)->CallObjectMethod(env, junified, mid);

	if (!jaddress) /* segmented */
	{
		/* get the method ID for the segmentedNetworkInfo() method */
		mid = (*env)->GetMethodID(env, connectOptsClass, "segmentedNetworkInfo", "()Lcom/thomsonreuters/upa/transport/SegmentedNetworkInfo;");
		if (mid == NULL)
		{
			return RSSL_FALSE;
		}

		/* get segmentedNetworkInfo object */
		jsegmented = (*env)->CallObjectMethod(env, *jconnectopts, mid);

		/* get the SegmentedNetworkInfo class */
		segmentedNetworkClass = (*env)->GetObjectClass(env, jsegmented);

		/* get the method ID for the recvAddress() method */
		mid = (*env)->GetMethodID(env, segmentedNetworkClass, "recvAddress", "()Ljava/lang/String;");
		if (mid == NULL)
		{
			return RSSL_FALSE;
		}

		/* set C recvAddress */
		jaddress = (*env)->CallObjectMethod(env, jsegmented, mid);
		if (jaddress)
		{
			address = (*env)->GetStringUTFChars(env, jaddress, NULL);
			strcpy(rsslConnectOpts->connectionInfo.segmented.recvAddress, address);
			(*env)->ReleaseStringUTFChars(env, jaddress, address);
		}

		/* get the method ID for the recvServiceName() method */
		mid = (*env)->GetMethodID(env, segmentedNetworkClass, "recvServiceName", "()Ljava/lang/String;");
		if (mid == NULL)
		{
			return RSSL_FALSE;
		}

		/* set C recvServiceName */
		jserviceName = (*env)->CallObjectMethod(env, jsegmented, mid);
		if (jserviceName)
		{
			serviceName = (*env)->GetStringUTFChars(env, jserviceName, NULL);
			strcpy(rsslConnectOpts->connectionInfo.segmented.recvServiceName, serviceName);
			(*env)->ReleaseStringUTFChars(env, jserviceName, serviceName);
		}

		/* get the method ID for the unicastServiceName() method */
		mid = (*env)->GetMethodID(env, segmentedNetworkClass, "unicastServiceName", "()Ljava/lang/String;");
		if (mid == NULL)
		{
			return RSSL_FALSE;
		}

		/* set C unicastServiceName */
		junicastServiceName = (*env)->CallObjectMethod(env, jsegmented, mid);
		if (junicastServiceName)
		{
			unicastServiceName = (*env)->GetStringUTFChars(env, junicastServiceName, NULL);
			strcpy(rsslConnectOpts->connectionInfo.segmented.unicastServiceName, unicastServiceName);
			(*env)->ReleaseStringUTFChars(env, junicastServiceName, unicastServiceName);
		}

		/* get the method ID for the interfaceName() method */
		mid = (*env)->GetMethodID(env, segmentedNetworkClass, "interfaceName", "()Ljava/lang/String;");
		if (mid == NULL)
		{
			return RSSL_FALSE;
		}

		/* set C interfaceName */
		jinterfaceName = (*env)->CallObjectMethod(env, jsegmented, mid);
		if (jinterfaceName)
		{
			interfaceName = (*env)->GetStringUTFChars(env, jinterfaceName, NULL);
			strcpy(rsslConnectOpts->connectionInfo.segmented.interfaceName, interfaceName);
			(*env)->ReleaseStringUTFChars(env, jinterfaceName, interfaceName);
		}

		/* get the method ID for the sendAddress() method */
		mid = (*env)->GetMethodID(env, segmentedNetworkClass, "sendAddress", "()Ljava/lang/String;");
		if (mid == NULL)
		{
			return RSSL_FALSE;
		}

		/* set C sendAddress */
		jaddress = (*env)->CallObjectMethod(env, jsegmented, mid);
		if (jaddress)
		{
			address = (*env)->GetStringUTFChars(env, jaddress, NULL);
			strcpy(rsslConnectOpts->connectionInfo.segmented.sendAddress, address);
			(*env)->ReleaseStringUTFChars(env, jaddress, address);
		}

		/* get the method ID for the sendServiceName() method */
		mid = (*env)->GetMethodID(env, segmentedNetworkClass, "sendServiceName", "()Ljava/lang/String;");
		if (mid == NULL)
		{
			return RSSL_FALSE;
		}

		/* set C sendServiceName */
		jserviceName = (*env)->CallObjectMethod(env, jsegmented, mid);
		if (jserviceName)
		{
			serviceName = (*env)->GetStringUTFChars(env, jserviceName, NULL);
			strcpy(rsslConnectOpts->connectionInfo.segmented.sendServiceName, serviceName);
			(*env)->ReleaseStringUTFChars(env, jserviceName, serviceName);
		}
	}
	else /* unified */
	{
		/* set C address */
		if (jaddress)
		{
			address = (*env)->GetStringUTFChars(env, jaddress, NULL);
			strcpy(rsslConnectOpts->connectionInfo.unified.address, address);
			(*env)->ReleaseStringUTFChars(env, jaddress, address);
		}

		/* get the method ID for the serviceName() method */
		mid = (*env)->GetMethodID(env, unifiedNetworkClass, "serviceName", "()Ljava/lang/String;");
		if (mid == NULL)
		{
			return RSSL_FALSE;
		}

		/* set C serviceName */
		jserviceName = (*env)->CallObjectMethod(env, junified, mid);
		if (jserviceName)
		{
			serviceName = (*env)->GetStringUTFChars(env, jserviceName, NULL);
			strcpy(rsslConnectOpts->connectionInfo.unified.serviceName, serviceName);
			(*env)->ReleaseStringUTFChars(env, jserviceName, serviceName);
		}

		/* get the method ID for the interfaceName() method */
		mid = (*env)->GetMethodID(env, unifiedNetworkClass, "interfaceName", "()Ljava/lang/String;");
		if (mid == NULL)
		{
			return RSSL_FALSE;
		}

		/* set C interfaceName */
		jinterfaceName = (*env)->CallObjectMethod(env, junified, mid);
		if (jinterfaceName)
		{
			interfaceName = (*env)->GetStringUTFChars(env, jinterfaceName, NULL);
			strcpy(rsslConnectOpts->connectionInfo.unified.interfaceName, interfaceName);
			(*env)->ReleaseStringUTFChars(env, jinterfaceName, interfaceName);
		}

		/* get the method ID for the unicastServiceName() method */
		mid = (*env)->GetMethodID(env, unifiedNetworkClass, "unicastServiceName", "()Ljava/lang/String;");
		if (mid == NULL)
		{
			return RSSL_FALSE;
		}

		/* set C unicastServiceName */
		junicastServiceName = (*env)->CallObjectMethod(env, junified, mid);
		if (junicastServiceName)
		{
			unicastServiceName = (*env)->GetStringUTFChars(env, junicastServiceName, NULL);
			strcpy(rsslConnectOpts->connectionInfo.unified.unicastServiceName, unicastServiceName);
			(*env)->ReleaseStringUTFChars(env, junicastServiceName, unicastServiceName);
		}

		/* make segmented sendAddress and sendServiceName NULL */
		rsslConnectOpts->connectionInfo.segmented.sendAddress = NULL;
		rsslConnectOpts->connectionInfo.segmented.sendServiceName = NULL;
	}

	/* get the method ID for the compressionType() method */
	mid = (*env)->GetMethodID(env, connectOptsClass, "compressionType", "()I");
	if (mid == NULL)
	{
		return RSSL_FALSE;
	}

	/* set C compressionType */
	rsslConnectOpts->compressionType = (*env)->CallIntMethod(env, *jconnectopts, mid);

	/* get the method ID for the blocking() method */
	mid = (*env)->GetMethodID(env, connectOptsClass, "blocking", "()Z");
	if (mid == NULL)
	{
		return RSSL_FALSE;
	}

	/* set C blocking */
	rsslConnectOpts->blocking = (*env)->CallBooleanMethod(env, *jconnectopts, mid);

	/* get the method ID for the pingTimeout() method */
	mid = (*env)->GetMethodID(env, connectOptsClass, "pingTimeout", "()I");
	if (mid == NULL)
	{
		return RSSL_FALSE;
	}

	/* set C pingTimeout */
	rsslConnectOpts->pingTimeout = (*env)->CallIntMethod(env, *jconnectopts, mid);

	/* get the method ID for the guaranteedOutputBuffers() method */
	mid = (*env)->GetMethodID(env, connectOptsClass, "guaranteedOutputBuffers", "()I");
	if (mid == NULL)
	{
		return RSSL_FALSE;
	}

	/* set C guaranteedOutputBuffers */
	rsslConnectOpts->guaranteedOutputBuffers = (*env)->CallIntMethod(env, *jconnectopts, mid);

	/* get the method ID for the numInputBuffers() method */
	mid = (*env)->GetMethodID(env, connectOptsClass, "numInputBuffers", "()I");
	if (mid == NULL)
	{
		return RSSL_FALSE;
	}

	/* set C numInputBuffers */
	rsslConnectOpts->numInputBuffers = (*env)->CallIntMethod(env, *jconnectopts, mid);

	/* get the method ID for the majorVersion() method */
	mid = (*env)->GetMethodID(env, connectOptsClass, "majorVersion", "()I");
	if (mid == NULL)
	{
		return RSSL_FALSE;
	}

	/* set C majorVersion */
	rsslConnectOpts->majorVersion = (RsslUInt8)(*env)->CallIntMethod(env, *jconnectopts, mid);

	/* get the method ID for the minorVersion() method */
	mid = (*env)->GetMethodID(env, connectOptsClass, "minorVersion", "()I");
	if (mid == NULL)
	{
		return RSSL_FALSE;
	}

	/* set C minorVersion */
	rsslConnectOpts->minorVersion = (RsslUInt8)(*env)->CallIntMethod(env, *jconnectopts, mid);

	/* get the method ID for the protocolType() method */
	mid = (*env)->GetMethodID(env, connectOptsClass, "protocolType", "()I");
	if (mid == NULL)
	{
		return RSSL_FALSE;
	}

	/* set C protocolType */
	rsslConnectOpts->protocolType = (RsslUInt8)(*env)->CallIntMethod(env, *jconnectopts, mid);

	/* get the method ID for the sysSendBufSize() method */
	mid = (*env)->GetMethodID(env, connectOptsClass, "sysSendBufSize", "()I");
	if (mid == NULL)
	{
		return RSSL_FALSE;
	}

	/* set C sysSendBufSize */
	rsslConnectOpts->sysSendBufSize = (*env)->CallIntMethod(env, *jconnectopts, mid);

	/* get the method ID for the sysRecvBufSize() method */
	mid = (*env)->GetMethodID(env, connectOptsClass, "sysRecvBufSize", "()I");
	if (mid == NULL)
	{
		return RSSL_FALSE;
	}

	/* set C sysRecvBufSize */
	rsslConnectOpts->sysRecvBufSize = (*env)->CallIntMethod(env, *jconnectopts, mid);

	/* get the method ID for the userSpecObject() method */
	mid = (*env)->GetMethodID(env, connectOptsClass, "userSpecObject", "()Ljava/lang/Object;");
	if (mid == NULL)
	{
		return RSSL_FALSE;
	}

	/* set C userSpecPtr */
	rsslConnectOpts->userSpecPtr = (*env)->CallObjectMethod(env, *jconnectopts, mid);

	/* get the method ID for the tcpOpts() method */
	mid = (*env)->GetMethodID(env, connectOptsClass, "tcpOpts", "()Lcom/thomsonreuters/upa/transport/TcpOpts;");
	if (mid == NULL)
	{
		return RSSL_FALSE;
	}

	/* get tcpOpts object */
	jtcpopts = (*env)->CallObjectMethod(env, *jconnectopts, mid);

	/* get the TcpOpts class */
	tcpOptsClass = (*env)->GetObjectClass(env, jtcpopts);

	/* get the method ID for the tcpNoDelay() method */
	mid = (*env)->GetMethodID(env, tcpOptsClass, "tcpNoDelay", "()Z");
	if (mid == NULL)
	{
		return RSSL_FALSE;
	}

	/* set C tcp_nodelay */
	rsslConnectOpts->tcpOpts.tcp_nodelay = (*env)->CallBooleanMethod(env, jtcpopts, mid);

	/* get the method ID for the multicastOpts() method */
	mid = (*env)->GetMethodID(env, connectOptsClass, "multicastOpts", "()Lcom/thomsonreuters/upa/transport/MCastOpts;");
	if (mid == NULL)
	{
		return RSSL_FALSE;
	}

	/* get multicastOpts object */
	jmcastopts = (*env)->CallObjectMethod(env, *jconnectopts, mid);
	if (jmcastopts == NULL)
	{
		return RSSL_FALSE;
	}

	/* get the MCastOpts class */
	mcastOptsClass = (*env)->GetObjectClass(env, jmcastopts);

	/* get the method ID for the disconnectOnGaps() method */
	mid = (*env)->GetMethodID(env, mcastOptsClass, "disconnectOnGaps", "()Z");
	if (mid == NULL)
	{
		return RSSL_FALSE;
	}

	/* set C disconnectOnGaps */
	rsslConnectOpts->multicastOpts.disconnectOnGaps = (*env)->CallBooleanMethod(env, jmcastopts, mid);

	/* get the method ID for the packetTTL() method */
	mid = (*env)->GetMethodID(env, mcastOptsClass, "packetTTL", "()I");
	if (mid == NULL)
	{
		return RSSL_FALSE;
	}

	/* set C packetTTL */
	rsslConnectOpts->multicastOpts.packetTTL = (RsslUInt8)(*env)->CallIntMethod(env, jmcastopts, mid);


	/* get the method ID for the tcpControlPort() method */
	mid = (*env)->GetMethodID(env, mcastOptsClass, "tcpControlPort", "()Ljava/lang/String;");
	if (mid == NULL)
	{
		return RSSL_FALSE;
	}

	/* set C tcpControlPort */
	jtcpControlPort = (*env)->CallObjectMethod(env, jmcastopts, mid);
	if (jtcpControlPort)
	{
		tcpControlPort = (*env)->GetStringUTFChars(env, jtcpControlPort, NULL);
		strcpy(rsslConnectOpts->multicastOpts.tcpControlPort, tcpControlPort);
		(*env)->ReleaseStringUTFChars(env, jtcpControlPort, tcpControlPort);
	}

	/* get the method ID for the portRoamRange() method */
	mid = (*env)->GetMethodID(env, mcastOptsClass, "portRoamRange", "()I");
	if (mid == NULL)
	{
		return RSSL_FALSE;
	}

	/* set C portRoamRange */
	rsslConnectOpts->multicastOpts.portRoamRange = (RsslUInt16)(*env)->CallIntMethod(env, jmcastopts, mid);


	/* get the method ID for the shmemOpts() method */
	mid = (*env)->GetMethodID(env, connectOptsClass, "shmemOpts", "()Lcom/thomsonreuters/upa/transport/ShmemOpts;");
	if (mid == NULL)
	{
		return RSSL_FALSE;
	}

	/* get shmemOpts object */
	jshmemopts = (*env)->CallObjectMethod(env, *jconnectopts, mid);

	/* get the ShmemOpts class */
	shmemOptsClass = (*env)->GetObjectClass(env, jshmemopts);

	/* get the method ID for the maxReaderLag() method */
	mid = (*env)->GetMethodID(env, shmemOptsClass, "maxReaderLag", "()J");
	if (mid == NULL)
	{
		return RSSL_FALSE;
	}

	/* set C maxReaderLag */
	rsslConnectOpts->shmemOpts.maxReaderLag = (*env)->CallLongMethod(env, jshmemopts, mid);

	return RSSL_TRUE;
}

/* copies a C RsslError structure to a Java Error class */
static void populateJavaError(JNIEnv *env, RsslError *rsslError, jobject *jerror, jobject *jchannel)
{
	jclass errorClass;
	jmethodID mid;
	jstring jtext;
	jclass jniChannelClass;
	jobject jchannelLocal;

	/* get the Error class */
	errorClass = (*env)->GetObjectClass(env, *jerror);

	/* get the method ID for the errorId(int) method */
	mid = (*env)->GetMethodID(env, errorClass, "errorId", "(I)V");
	if (mid == NULL)
	{
		return;
	}

	/* call errorId method */
	(*env)->CallIntMethod(env, *jerror, mid, rsslError->rsslErrorId);

	/* get the method ID for the sysError(int) method */
	mid = (*env)->GetMethodID(env, errorClass, "sysError", "(I)V");
	if (mid == NULL)
	{
		return;
	}

	/* call sysError method */
	(*env)->CallIntMethod(env, *jerror, mid, rsslError->sysError);

	/* get the method ID for the text(String) method */
	mid = (*env)->GetMethodID(env, errorClass, "text", "(Ljava/lang/String;)V");
	if (mid == NULL)
	{
		return;
	}

	/* call text method */
	jtext = (*env)->NewStringUTF(env, rsslError->text);
	(*env)->CallObjectMethod(env, *jerror, mid, jtext);

	if (rsslError->channel != NULL)
	{
		/* find the JNIChannel class */
		jniChannelClass = (*env)->FindClass(env, "com/thomsonreuters/upa/transport/JNIChannel");
		if (jniChannelClass == NULL)
		{
			return;
		}

		if (jchannel == NULL)
		{
		/* get the method ID for the JNIChannel() constructor */
		mid = (*env)->GetMethodID(env, jniChannelClass, "<init>", "()V");
		if (mid == NULL)
		{
			return;
		}

		/* construct a Java JNIChannel object */
			jchannelLocal = (*env)->NewObject(env, jniChannelClass, mid);

		/* populate channel object */
			if (!populateJavaChannel(env, rsslError->channel, &jchannelLocal, RSSL_FALSE))
		{
			return;
		}
		}

		/* get the method ID for the channel(Lcom/thomsonreuters/upa/transport/Channel;) method */
		mid = (*env)->GetMethodID(env, errorClass, "channel", "(Lcom/thomsonreuters/upa/transport/Channel;)V");
		if (mid == NULL)
		{
			return;
		}

		/* call channel method */
		if (jchannel == NULL)
		{
			(*env)->CallObjectMethod(env, *jerror, mid, jchannelLocal);
		}
		else
		{
			(*env)->CallObjectMethod(env, *jerror, mid, *jchannel);
		}
	}
}

/* populates a Java RsslInProgInfo class from a Java RsslChannel */
static RsslBool populateJavaInProg(JNIEnv *env, jobject *jchnl, jobject *jinprog, int flags)
{
	jclass jniChannelClass, rsslInProgInfoClass;
	jfieldID chnlFid;
	jmethodID inProgMid;

	/* get the RsslInProgInfo class */
	rsslInProgInfoClass = (*env)->GetObjectClass(env, *jinprog);

	/* get the JNIChannel class */
	jniChannelClass = (*env)->GetObjectClass(env, *jchnl);

	/* get the method ID for the flags(int) method */
	inProgMid = (*env)->GetMethodID(env, rsslInProgInfoClass, "flags", "(I)V");
	if (inProgMid == NULL)
	{
		return RSSL_FALSE;
	}

	/* call flags method */
	(*env)->CallIntMethod(env, *jinprog, inProgMid, flags);

	/* get the method ID for the oldScktChannel(SocketChannel) method */
	inProgMid = (*env)->GetMethodID(env, rsslInProgInfoClass, "oldScktChannel", "(Ljava/nio/channels/SocketChannel;)V");
	if (inProgMid == NULL)
	{
		return RSSL_FALSE;
	}

	/* get JNIChannel oldScktChannel field id */
	chnlFid = (*env)->GetFieldID(env, jniChannelClass, "_oldScktChannel", "Ljava/nio/channels/SocketChannel;");
	if (chnlFid == NULL)
	{
		return RSSL_FALSE;
	}

	/* call oldScktChannel method with argument from JNIChannel oldScktChannel field */
	(*env)->CallObjectMethod(env, *jinprog, inProgMid, (*env)->GetObjectField(env, *jchnl, chnlFid));

	/* get the method ID for the newScktChannel(SocketChannel) method */
	inProgMid = (*env)->GetMethodID(env, rsslInProgInfoClass, "newScktChannel", "(Ljava/nio/channels/SelectableChannel;)V");
	if (inProgMid == NULL)
	{
		return RSSL_FALSE;
	}

	/* get JNIChannel scktChannel field id */
	chnlFid = (*env)->GetFieldID(env, jniChannelClass, "_scktChannel", "Ljava/nio/channels/SocketChannel;");
	if (chnlFid == NULL)
	{
		return RSSL_FALSE;
	}

	/* call newScktChannel method with argument from JNIChannel scktChannel field */
	(*env)->CallObjectMethod(env, *jinprog, inProgMid, (*env)->GetObjectField(env, *jchnl, chnlFid));

	return RSSL_TRUE;
}

/* copies a C RsslLibraryVersionInfo structure to a Java RsslLibraryVersionInfo class */
static void populateJavaVersionInfo(JNIEnv *env, RsslLibraryVersionInfo *verInfo, jobject *jverinfo)
{
	jclass rsslLibraryVersionInfoClass;
	jfieldID fid;
	jstring jtext;

	/* get the RsslLibraryVersionInfo class */
	rsslLibraryVersionInfoClass = (*env)->GetObjectClass(env, *jverinfo);

	/* get productVersion field id */
	fid = (*env)->GetFieldID(env, rsslLibraryVersionInfoClass, "productVersion", "Ljava/lang/String;");
	if (fid == NULL)
	{
		return;
	}

	/* set productVersion field */
	jtext = (*env)->NewStringUTF(env, verInfo->productVersion);
	(*env)->SetObjectField(env, *jverinfo, fid, jtext);

	/* get internalVersion field id */
	fid = (*env)->GetFieldID(env, rsslLibraryVersionInfoClass, "internalVersion", "Ljava/lang/String;");
	if (fid == NULL)
	{
		return;
	}

	/* set internalVersion field */
	jtext = (*env)->NewStringUTF(env, verInfo->internalVersion);
	(*env)->SetObjectField(env, *jverinfo, fid, jtext);

	/* get productDate field id */
	fid = (*env)->GetFieldID(env, rsslLibraryVersionInfoClass, "productDate", "Ljava/lang/String;");
	if (fid == NULL)
	{
		return;
	}

	/* set productDate field */
	jtext = (*env)->NewStringUTF(env, verInfo->productDate);
	(*env)->SetObjectField(env, *jverinfo, fid, jtext);
}

/* copies a C RetVal and RsslReadOutArgs variable to a Java ReadArgs class */
static void populateReadReturnValue(JNIEnv *env, RsslRet rsslRetVal, RsslReadOutArgs *readOutArgs, jobject *jreadargs)
{
	static jclass readArgsClass = NULL;
	jclass localRefClass;
	static jmethodID readRetValMid = NULL, bytesReadMid = NULL, uncompressedBytesReadMid = NULL;

	/* get the ReadArgs class */
	if (readArgsClass == NULL)
	{
		localRefClass =  (*env)->GetObjectClass(env, *jreadargs);
		if (localRefClass == NULL)
		{
			return;
		}
		/* Create a global reference */
		readArgsClass = (*env)->NewGlobalRef(env, localRefClass);
		(*env)->DeleteLocalRef(env, localRefClass);
		if (readArgsClass == NULL)
		{
			return;
		}
	}

	/* get the method ID for the readRetVal(int) method */
	if (readRetValMid == NULL)
	{
		readRetValMid = (*env)->GetMethodID(env, readArgsClass, "readRetVal", "(I)V");
		if (readRetValMid == NULL)
		{
			return;
		}
	}

	/* call readRetVal method */
	(*env)->CallIntMethod(env, *jreadargs, readRetValMid, rsslRetVal);

	/* get the method ID for the bytesRead(int) method */
	if (bytesReadMid == NULL)
	{
		bytesReadMid = (*env)->GetMethodID(env, readArgsClass, "bytesRead", "(I)V");
		if (bytesReadMid == NULL)
		{
			return;
		}
	}

	/* call bytesRead method */
	(*env)->CallIntMethod(env, *jreadargs, bytesReadMid, readOutArgs->bytesRead);

	/* get the method ID for the uncompressedBytesRead(int) method */
	if (uncompressedBytesReadMid == NULL)
	{
		uncompressedBytesReadMid = (*env)->GetMethodID(env, readArgsClass, "uncompressedBytesRead", "(I)V");
		if (uncompressedBytesReadMid == NULL)
		{
			return;
		}
	}

	/* call uncompressedBytesRead method */
	(*env)->CallIntMethod(env, *jreadargs, uncompressedBytesReadMid, readOutArgs->uncompressedBytesRead);
}

/* copies a C bytes written info to a Java WriteArgs class */
static void populateBytesWritten(JNIEnv *env, RsslWriteOutArgs *writeOutArgs, jobject *jwriteargs)
{
	static jclass writeArgsClass = NULL;
	jclass localRefClass;
	static jmethodID bytesWrittenMid = NULL, uncompressedBytesWrittenMid = NULL;

	/* get the WriteArgs class */
	if (writeArgsClass == NULL)
	{
		localRefClass = (*env)->GetObjectClass(env, *jwriteargs);
		if (localRefClass == NULL)
		{
			return;
		}
		/* Create a global reference */
		writeArgsClass = (*env)->NewGlobalRef(env, localRefClass);
		(*env)->DeleteLocalRef(env, localRefClass);
		if (writeArgsClass == NULL)
		{
			return;
		}
	}

	/* get the method ID for the bytesWritten(int) method */
	if (bytesWrittenMid == NULL)
	{
		bytesWrittenMid = (*env)->GetMethodID(env, writeArgsClass, "bytesWritten", "(I)V");
		if (bytesWrittenMid == NULL)
		{
			return;
		}
	}

	/* call bytesWritten method */
	(*env)->CallIntMethod(env, *jwriteargs, bytesWrittenMid, writeOutArgs->bytesWritten);

	/* get the method ID for the uncompressedBytesWritten(int) method */
	if (uncompressedBytesWrittenMid == NULL)
	{
		uncompressedBytesWrittenMid = (*env)->GetMethodID(env, writeArgsClass, "uncompressedBytesWritten", "(I)V");
		if (uncompressedBytesWrittenMid == NULL)
		{
			return;
		}
	}

	/* call uncompressedBytesWritten method */
	(*env)->CallIntMethod(env, *jwriteargs, uncompressedBytesWrittenMid, writeOutArgs->uncompressedBytesWritten);
}

/* returns a Java TransportBuffer class copied from a C RsslBuffer structure */
static jobject createJavaBuffer(JNIEnv *env, RsslBuffer *rsslBuffer)
{
	static jclass jniBufferClass = NULL;
	jclass localRefClass;
	jobject jbuffer, byteBufferObject;
	static jmethodID mid = NULL;
	static jfieldID fid = NULL;

	/* create direct ByteBuffer from UPAC buffer */
	byteBufferObject = (*env)->NewDirectByteBuffer(env, rsslBuffer->data, rsslBuffer->length);
	if (byteBufferObject == NULL)
	{
		return NULL;
	}

	if (jniBufferClass == NULL)
	{
		localRefClass = (*env)->FindClass(env, "com/thomsonreuters/upa/transport/JNIBuffer");
		if (localRefClass == NULL)
		{
			return NULL;
		}
		/* Create a global reference */
		jniBufferClass = (*env)->NewGlobalRef(env, localRefClass);
		(*env)->DeleteLocalRef(env, localRefClass);
		if (jniBufferClass == NULL)
		{
			return NULL;
		}
	}

	/* get the method ID for the JNIBuffer(ByteBuffer, int) constructor */
	if (mid == NULL)
	{
		mid = (*env)->GetMethodID(env, jniBufferClass, "<init>", "(Ljava/nio/ByteBuffer;I)V");
		if (mid == NULL)
		{
			return NULL;
		}
	}

	/* construct a JNIBuffer object */
	jbuffer = (*env)->NewObject(env, jniBufferClass, mid, byteBufferObject, rsslBuffer->length);

	/* get rsslBufferCPtr field id */
	if (fid  == NULL)
	{
		fid = (*env)->GetFieldID(env, jniBufferClass, "_rsslBufferCPtr", "J");
		if (fid == NULL)
		{
			return NULL;
		}
	}

	/* set rsslBufferCPtr field */
	(*env)->SetLongField(env, jbuffer, fid, (jlong)rsslBuffer);

	return jbuffer;
}

/* returns a Java READ TransportBuffer class copied from a C RsslBuffer structure */
static jobject getJavaReadBuffer(JNIEnv *env, RsslBuffer *rsslBuffer, jobject *jchnl)
{
	static jclass jniChannelClass = NULL, jniBufferClass = NULL;
	jclass localRefClass;
	jobject jbuffer, byteBufferObject;
	static jmethodID mid = NULL;
	static jfieldID fid = NULL;

	/* create direct ByteBuffer from UPAC buffer */
	byteBufferObject = (*env)->NewDirectByteBuffer(env, rsslBuffer->data, rsslBuffer->length);

	/* get the JNIChannel class */
	if (jniChannelClass == NULL)
	{
		localRefClass =  (*env)->GetObjectClass(env, *jchnl);
		if (localRefClass == NULL)
		{
			return NULL;
		}
		/* Create a global reference */
		jniChannelClass = (*env)->NewGlobalRef(env, localRefClass);
		(*env)->DeleteLocalRef(env, localRefClass);
		if (jniChannelClass == NULL)
		{
			return NULL;
		}
	}

	/* get the method ID for the JNIChannel.readBuffer(ByteBuffer, int) method */
	if (mid == NULL)
	{
		mid = (*env)->GetMethodID(env, jniChannelClass, "readBuffer", "(Ljava/nio/ByteBuffer;I)Lcom/thomsonreuters/upa/transport/JNIBuffer;");
		if (mid == NULL)
		{
			return NULL;
		}
	}

	/* get the JNIChannel READ JNIBuffer object */
	jbuffer = (*env)->CallObjectMethod(env, *jchnl, mid, byteBufferObject, rsslBuffer->length);

	/* get the JNIBuffer class */
	if (jniBufferClass == NULL)
	{
		localRefClass =  (*env)->GetObjectClass(env, jbuffer);
		if (localRefClass == NULL)
		{
			return NULL;
		}
		/* Create a global reference */
		jniBufferClass = (*env)->NewGlobalRef(env, localRefClass);
		(*env)->DeleteLocalRef(env, localRefClass);
		if (jniBufferClass == NULL)
		{
			return NULL;
		}
	}

	/* get JNIBuffer rsslBufferCPtr field id */
	if (fid == NULL)
	{
		fid = (*env)->GetFieldID(env, jniBufferClass, "_rsslBufferCPtr", "J");
		if (fid == NULL)
		{
			return NULL;
		}
	}

	/* set rsslBufferCPtr field */
	(*env)->SetLongField(env, jbuffer, fid, (jlong)rsslBuffer);

	return jbuffer;
}

/* get the C RsslBuffer structure from the Java JNIBuffer class */
static RsslBuffer* getCBuffer(JNIEnv *env, jobject *jbuffer)
{
	static jclass jniBufferClass = NULL;
	jclass localRefClass;
	static jfieldID fid = NULL;

	if (!*jbuffer)
	{
		return NULL;
	}

	/* get the jniBufferClass class */
	if (jniBufferClass == NULL)
	{
		localRefClass =  (*env)->GetObjectClass(env, *jbuffer);
		if (localRefClass == NULL)
		{
			return NULL;
		}
		/* Create a global reference */
		jniBufferClass = (*env)->NewGlobalRef(env, localRefClass);
		(*env)->DeleteLocalRef(env, localRefClass);
		if (jniBufferClass == NULL)
		{
			return NULL;
		}
	}

	/* get rsslBufferCPtr field id */
	if (fid == NULL)
	{
		fid = (*env)->GetFieldID(env, jniBufferClass, "_rsslBufferCPtr", "J");
		if (fid == NULL)
		{
			return NULL;
		}
	}

	/* return C rsslBuffer from field */
	return (RsslBuffer *)(*env)->GetLongField(env, *jbuffer, fid);
}

/* copies the Java JNIBuffer object info to the C RsslBuffer structure */
static RsslBuffer* copyToCBuffer(JNIEnv *env, jobject *jbuffer)
{
	static jclass jniBufferClass = NULL;
	jclass localRefClass;
	RsslBuffer *rsslBuffer;
	static jmethodID mid = NULL;

	if (!*jbuffer)
	{
		return NULL;
	}

	/* get C RsslBuffer first */
	rsslBuffer = getCBuffer(env, jbuffer);

	/* get the jniBufferClass class */
	if (jniBufferClass == NULL)
	{
		localRefClass =  (*env)->GetObjectClass(env, *jbuffer);
		if (localRefClass == NULL)
		{
			return NULL;
		}
		/* Create a global reference */
		jniBufferClass = (*env)->NewGlobalRef(env, localRefClass);
		(*env)->DeleteLocalRef(env, localRefClass);
		if (jniBufferClass == NULL)
		{
			return NULL;
		}
	}

	/* get the method ID for the length() method */
	if (mid == NULL)
	{
		mid = (*env)->GetMethodID(env, jniBufferClass, "length", "()I");
		if (mid == NULL)
		{
			return RSSL_FALSE;
		}
	}

	/* call length method */
	rsslBuffer->length = (*env)->CallIntMethod(env, *jbuffer, mid);

	return rsslBuffer;
}

/* copies a Java BindOptions class to a C RsslBindOptions structure  */
static RsslBool populateCBindOptions(JNIEnv *env, jobject *jbindopts, RsslBindOptions *rsslBindOpts)
{
	jclass bindOptsClass, tcpOptsClass;
	jobject jtcpopts;
	jmethodID mid;
	jstring jserviceName, jinterfaceName, jcomponentVersion;
	const char *serviceName, *interfaceName, *componentVersion;

	if (!rsslBindOpts || !*jbindopts)
	{
		return RSSL_FALSE;
	}

	/* get the BindOptions class */
	bindOptsClass = (*env)->GetObjectClass(env, *jbindopts);

	/* get the method ID for the componentVersion() method */
	mid = (*env)->GetMethodID(env, bindOptsClass, "componentVersion", "()Ljava/lang/String;");
	if (mid == NULL)
	{
		return RSSL_FALSE;
	}

	/* set C componentVersion */
	jcomponentVersion = (*env)->CallObjectMethod(env, *jbindopts, mid);
	if (jcomponentVersion)
	{
		componentVersion = (*env)->GetStringUTFChars(env, jcomponentVersion, NULL);
		strcpy(rsslBindOpts->componentVersion, componentVersion);
		(*env)->ReleaseStringUTFChars(env, jcomponentVersion, componentVersion);
	}
	/* get the method ID for the serviceName() method */
	mid = (*env)->GetMethodID(env, bindOptsClass, "serviceName", "()Ljava/lang/String;");
	if (mid == NULL)
	{
		return RSSL_FALSE;
	}

	/* set C serviceName */
	jserviceName = (*env)->CallObjectMethod(env, *jbindopts, mid);
	if (jserviceName)
	{
		serviceName = (*env)->GetStringUTFChars(env, jserviceName, NULL);
		strcpy(rsslBindOpts->serviceName, serviceName);
		(*env)->ReleaseStringUTFChars(env, jserviceName, serviceName);
	}

	/* get the method ID for the interfaceName() method */
	mid = (*env)->GetMethodID(env, bindOptsClass, "interfaceName", "()Ljava/lang/String;");
	if (mid == NULL)
	{
		return RSSL_FALSE;
	}

	/* set C interfaceName */
	jinterfaceName = (*env)->CallObjectMethod(env, *jbindopts, mid);
	if (jinterfaceName)
	{
		interfaceName = (*env)->GetStringUTFChars(env, jinterfaceName, NULL);
		strcpy(rsslBindOpts->interfaceName, interfaceName);
		(*env)->ReleaseStringUTFChars(env, jinterfaceName, interfaceName);
	}

	/* get the method ID for the compressionType() method */
	mid = (*env)->GetMethodID(env, bindOptsClass, "compressionType", "()I");
	if (mid == NULL)
	{
		return RSSL_FALSE;
	}

	/* set C compressionType */
	rsslBindOpts->compressionType = (*env)->CallIntMethod(env, *jbindopts, mid);

	/* get the method ID for the compressionLevel() method */
	mid = (*env)->GetMethodID(env, bindOptsClass, "compressionLevel", "()I");
	if (mid == NULL)
	{
		return RSSL_FALSE;
	}

	/* set C compressionLevel */
	rsslBindOpts->compressionLevel = (*env)->CallIntMethod(env, *jbindopts, mid);

	/* get the method ID for the forceCompression() method */
	mid = (*env)->GetMethodID(env, bindOptsClass, "forceCompression", "()Z");
	if (mid == NULL)
	{
		return RSSL_FALSE;
	}

	/* set C forceCompression */
	rsslBindOpts->forceCompression = (*env)->CallBooleanMethod(env, *jbindopts, mid);

	/* get the method ID for the serverBlocking() method */
	mid = (*env)->GetMethodID(env, bindOptsClass, "serverBlocking", "()Z");
	if (mid == NULL)
	{
		return RSSL_FALSE;
	}

	/* set C serverBlocking */
	rsslBindOpts->serverBlocking = (*env)->CallBooleanMethod(env, *jbindopts, mid);

	/* get the method ID for the channelsBlocking() method */
	mid = (*env)->GetMethodID(env, bindOptsClass, "channelsBlocking", "()Z");
	if (mid == NULL)
	{
		return RSSL_FALSE;
	}

	/* set C channelsBlocking */
	rsslBindOpts->channelsBlocking = (*env)->CallBooleanMethod(env, *jbindopts, mid);

	/* get the method ID for the serverToClientPings() method */
	mid = (*env)->GetMethodID(env, bindOptsClass, "serverToClientPings", "()Z");
	if (mid == NULL)
	{
		return RSSL_FALSE;
	}

	/* set C serverToClientPings */
	rsslBindOpts->serverToClientPings = (*env)->CallBooleanMethod(env, *jbindopts, mid);

	/* get the method ID for the clientToServerPings() method */
	mid = (*env)->GetMethodID(env, bindOptsClass, "clientToServerPings", "()Z");
	if (mid == NULL)
	{
		return RSSL_FALSE;
	}

	/* set C clientToServerPings */
	rsslBindOpts->clientToServerPings = (*env)->CallBooleanMethod(env, *jbindopts, mid);

	/* get the method ID for the connectionType() method */
	mid = (*env)->GetMethodID(env, bindOptsClass, "connectionType", "()I");
	if (mid == NULL)
	{
		return RSSL_FALSE;
	}

	/* set C connectionType */
	rsslBindOpts->connectionType = (*env)->CallIntMethod(env, *jbindopts, mid);

	/* get the method ID for the pingTimeout() method */
	mid = (*env)->GetMethodID(env, bindOptsClass, "pingTimeout", "()I");
	if (mid == NULL)
	{
		return RSSL_FALSE;
	}

	/* set C pingTimeout */
	rsslBindOpts->pingTimeout = (*env)->CallIntMethod(env, *jbindopts, mid);

	/* get the method ID for the minPingTimeout() method */
	mid = (*env)->GetMethodID(env, bindOptsClass, "minPingTimeout", "()I");
	if (mid == NULL)
	{
		return RSSL_FALSE;
	}

	/* set C minPingTimeout */
	rsslBindOpts->minPingTimeout = (*env)->CallIntMethod(env, *jbindopts, mid);

	/* get the method ID for the maxFragmentSize() method */
	mid = (*env)->GetMethodID(env, bindOptsClass, "maxFragmentSize", "()I");
	if (mid == NULL)
	{
		return RSSL_FALSE;
	}

	/* set C maxFragmentSize */
	rsslBindOpts->maxFragmentSize = (*env)->CallIntMethod(env, *jbindopts, mid);

	/* get the method ID for the maxOutputBuffers() method */
	mid = (*env)->GetMethodID(env, bindOptsClass, "maxOutputBuffers", "()I");
	if (mid == NULL)
	{
		return RSSL_FALSE;
	}

	/* set C maxOutputBuffers */
	rsslBindOpts->maxOutputBuffers = (*env)->CallIntMethod(env, *jbindopts, mid);

	/* get the method ID for the guaranteedOutputBuffers() method */
	mid = (*env)->GetMethodID(env, bindOptsClass, "guaranteedOutputBuffers", "()I");
	if (mid == NULL)
	{
		return RSSL_FALSE;
	}

	/* set C guaranteedOutputBuffers */
	rsslBindOpts->guaranteedOutputBuffers = (*env)->CallIntMethod(env, *jbindopts, mid);

	/* get the method ID for the numInputBuffers() method */
	mid = (*env)->GetMethodID(env, bindOptsClass, "numInputBuffers", "()I");
	if (mid == NULL)
	{
		return RSSL_FALSE;
	}

	/* set C numInputBuffers */
	rsslBindOpts->numInputBuffers = (*env)->CallIntMethod(env, *jbindopts, mid);

	/* get the method ID for the sharedPoolSize() method */
	mid = (*env)->GetMethodID(env, bindOptsClass, "sharedPoolSize", "()I");
	if (mid == NULL)
	{
		return RSSL_FALSE;
	}

	/* set C sharedPoolSize */
	rsslBindOpts->sharedPoolSize = (*env)->CallIntMethod(env, *jbindopts, mid);

	/* get the method ID for the sharedPoolLock() method */
	mid = (*env)->GetMethodID(env, bindOptsClass, "sharedPoolLock", "()Z");
	if (mid == NULL)
	{
		return RSSL_FALSE;
	}

	/* set C sharedPoolLock */
	rsslBindOpts->sharedPoolLock = (*env)->CallBooleanMethod(env, *jbindopts, mid);

	/* get the method ID for the majorVersion() method */
	mid = (*env)->GetMethodID(env, bindOptsClass, "majorVersion", "()I");
	if (mid == NULL)
	{
		return RSSL_FALSE;
	}

	/* set C majorVersion */
	rsslBindOpts->majorVersion = (RsslUInt8)(*env)->CallIntMethod(env, *jbindopts, mid);

	/* get the method ID for the minorVersion() method */
	mid = (*env)->GetMethodID(env, bindOptsClass, "minorVersion", "()I");
	if (mid == NULL)
	{
		return RSSL_FALSE;
	}

	/* set C minorVersion */
	rsslBindOpts->minorVersion = (RsslUInt8)(*env)->CallIntMethod(env, *jbindopts, mid);

	/* get the method ID for the protocolType() method */
	mid = (*env)->GetMethodID(env, bindOptsClass, "protocolType", "()I");
	if (mid == NULL)
	{
		return RSSL_FALSE;
	}

	/* set C protocolType */
	rsslBindOpts->protocolType = (RsslUInt8)(*env)->CallIntMethod(env, *jbindopts, mid);

	/* get the method ID for the userSpecObject() method */
	mid = (*env)->GetMethodID(env, bindOptsClass, "userSpecObject", "()Ljava/lang/Object;");
	if (mid == NULL)
	{
		return RSSL_FALSE;
	}

	/* set C userSpecPtr */
	rsslBindOpts->userSpecPtr = (*env)->CallObjectMethod(env, *jbindopts, mid);

	/* get the method ID for the tcpOpts() method */
	mid = (*env)->GetMethodID(env, bindOptsClass, "tcpOpts", "()Lcom/thomsonreuters/upa/transport/TcpOpts;");
	if (mid == NULL)
	{
		return RSSL_FALSE;
	}

	/* get tcpOpts object */
	jtcpopts = (*env)->CallObjectMethod(env, *jbindopts, mid);

	/* get the TcpOpts class */
	tcpOptsClass = (*env)->GetObjectClass(env, jtcpopts);

	/* get the method ID for the tcpNoDelay() method */
	mid = (*env)->GetMethodID(env, tcpOptsClass, "tcpNoDelay", "()Z");
	if (mid == NULL)
	{
		return RSSL_FALSE;
	}

	/* set C tcp_nodelay */
	rsslBindOpts->tcpOpts.tcp_nodelay = (*env)->CallBooleanMethod(env, jtcpopts, mid);

	return RSSL_TRUE;
}

/* copies a C RsslServer structure to a Java JNIServer class */
static RsslBool populateJavaServer(JNIEnv *env, RsslServer *rsslSrvr, jobject *jsrvr, RsslBindOptions *rsslBindOpts, RsslBool newFlag)
{
	static jclass jniServerClass = NULL;
	jclass localRefClass;
	jfieldID fid;

	if (!rsslSrvr || !*jsrvr)
	{
		return RSSL_FALSE;
	}

	/* get the JNIServer class */
	if (jniServerClass == NULL)
	{
		localRefClass =  (*env)->GetObjectClass(env, *jsrvr);
		if (localRefClass == NULL)
		{
			return RSSL_FALSE;
		}
		/* Create a global reference */
		jniServerClass = (*env)->NewGlobalRef(env, localRefClass);
		(*env)->DeleteLocalRef(env, localRefClass);
		if (jniServerClass == NULL)
		{
			return RSSL_FALSE;
		}
	}

	/* get socketId field id */
	fid = (*env)->GetFieldID(env, jniServerClass, "_socketId", "I");
	if (fid == NULL)
	{
		return RSSL_FALSE;
	}

	/* set socketId field */
	(*env)->SetIntField(env, *jsrvr, fid, rsslSrvr->socketId);

	/* get state field id */
	fid = (*env)->GetFieldID(env, jniServerClass, "_state", "I");
	if (fid == NULL)
	{
		return RSSL_FALSE;
	}

	/* set state field */
	(*env)->SetIntField(env, *jsrvr, fid, rsslSrvr->state);

	/* get portNumber field id */
	fid = (*env)->GetFieldID(env, jniServerClass, "_portNumber", "I");
	if (fid == NULL)
	{
		return RSSL_FALSE;
	}

	/* set portNumber field */
	(*env)->SetIntField(env, *jsrvr, fid, rsslSrvr->portNumber);

	/* populate userSpecObject and rsslServerCPtr if new flag set */
	if (newFlag)
	{
		/* get userSpecObject field id */
		fid = (*env)->GetFieldID(env, jniServerClass, "_userSpecObject", "Ljava/lang/Object;");
		if (fid == NULL)
		{
			return RSSL_FALSE;
		}

		/* set userSpecObject field */
		(*env)->SetObjectField(env, *jsrvr, fid, rsslSrvr->userSpecPtr);

		/* get rsslServerCPtr field id */
		fid = (*env)->GetFieldID(env, jniServerClass, "_rsslServerCPtr", "J");
		if (fid == NULL)
		{
			return RSSL_FALSE;
		}

		/* set rsslServerCPtr field */
		(*env)->SetLongField(env, *jsrvr, fid, (jlong)rsslSrvr);

		/* get serverBlocking field id */
		fid = (*env)->GetFieldID(env, jniServerClass, "_serverBlocking", "Z");
		if (fid == NULL)
		{
			return RSSL_FALSE;
		}

		/* set serverBlocking field */
		(*env)->SetBooleanField(env, *jsrvr, fid, rsslBindOpts->serverBlocking);

		/* get channelsBlocking field id */
		fid = (*env)->GetFieldID(env, jniServerClass, "_channelsBlocking", "Z");
		if (fid == NULL)
		{
			return RSSL_FALSE;
		}

		/* set channelsBlocking field */
		(*env)->SetBooleanField(env, *jsrvr, fid, rsslBindOpts->channelsBlocking);
	}

	return RSSL_TRUE;
}

/* get the C RsslServer structure from the Java JNIServer class */
static RsslServer* getCServer(JNIEnv *env, jobject *jsrvr)
{
	static jclass jniServerClass = NULL;
	jclass localRefClass;
	static jfieldID fid = NULL;

	if (!*jsrvr)
	{
		return NULL;
	}

	/* get the JNIServer class */
	if (jniServerClass == NULL)
	{
		localRefClass =  (*env)->GetObjectClass(env, *jsrvr);
		if (localRefClass == NULL)
		{
			return RSSL_FALSE;
		}
		/* Create a global reference */
		jniServerClass = (*env)->NewGlobalRef(env, localRefClass);
		(*env)->DeleteLocalRef(env, localRefClass);
		if (jniServerClass == NULL)
		{
			return RSSL_FALSE;
		}
	}

	/* get rsslServerCPtr field id */
	if (fid == NULL)
	{
		fid = (*env)->GetFieldID(env, jniServerClass, "_rsslServerCPtr", "J");
		if (fid == NULL)
		{
			return NULL;
		}
	}

	/* return C RSSL server structure from field */
	return (RsslServer *)(*env)->GetLongField(env, *jsrvr, fid);
}

/* copies a Java AcceptOptions class to a C RsslAcceptOptions structure  */
static RsslBool populateCAcceptOptions(JNIEnv *env, jobject *jacceptopts, RsslAcceptOptions *rsslAcceptOpts)
{
	jclass acceptOptsClass;
	jmethodID mid;

	if (!rsslAcceptOpts || !*jacceptopts)
	{
		return RSSL_FALSE;
	}

	/* get the AcceptOptions class */
	acceptOptsClass = (*env)->GetObjectClass(env, *jacceptopts);

	/* get the method ID for the nakMount() method */
	mid = (*env)->GetMethodID(env, acceptOptsClass, "nakMount", "()Z");
	if (mid == NULL)
	{
		return RSSL_FALSE;
	}

	/* set C nakMount */
	rsslAcceptOpts->nakMount = (*env)->CallBooleanMethod(env, *jacceptopts, mid);

	/* get the method ID for the userSpecObject() method */
	mid = (*env)->GetMethodID(env, acceptOptsClass, "userSpecObject", "()Ljava/lang/Object;");
	if (mid == NULL)
	{
		return RSSL_FALSE;
	}

	/* set C userSpecPtr */
	rsslAcceptOpts->userSpecPtr = (*env)->CallObjectMethod(env, *jacceptopts, mid);

	return RSSL_TRUE;
}

/* set the Java JNIChannel UserSpecObject from the Java RsslServer UserSpecObject */
static void setUserSpecObjectFromServer(JNIEnv *env, jobject *jsrvr, jobject *jchnl)
{
	jclass jniServerClass, jniChannelClass;
	jfieldID fidSrvr, fidChnl;

	if (!*jsrvr || !*jchnl)
	{
		return;
	}

	/* get the JNIServer class */
	jniServerClass = (*env)->GetObjectClass(env, *jsrvr);

	/* get userSpecObject field id */
	fidSrvr = (*env)->GetFieldID(env, jniServerClass, "_userSpecObject", "Ljava/lang/Object;");
	if (fidSrvr == NULL)
	{
		return;
	}

	/* get the jniChannelClass class */
	jniChannelClass = (*env)->GetObjectClass(env, *jchnl);

	/* get userSpecObject field id */
	fidChnl = (*env)->GetFieldID(env, jniChannelClass, "_userSpecObject", "Ljava/lang/Object;");
	if (fidChnl == NULL)
	{
		return;
	}

	/* set RsslChannel userSpecObject field from RsslServer userSpecObject field */
	(*env)->SetObjectField(env, *jchnl, fidChnl, (*env)->GetObjectField(env, *jsrvr, fidSrvr));
}

/* set the Java JNIChannel UserSpecObject from the Java AcceptOptions UserSpecObject */
static void setUserSpecObjectFromAcceptOpts(JNIEnv *env, jobject *jopts, jobject *jchnl)
{
	jclass acceptOptsClass, jniChannelClass;
	jfieldID fid;
	jmethodID mid;

	if (!*jopts || !*jchnl)
	{
		return;
	}

	/* get the AcceptOptions class */
	acceptOptsClass = (*env)->GetObjectClass(env, *jopts);

	/* get the method ID for the userSpecObject() method */
	mid = (*env)->GetMethodID(env, acceptOptsClass, "userSpecObject", "()Ljava/lang/Object;");
	if (mid == NULL)
	{
		return;
	}

	/* get the jniChannelClass class */
	jniChannelClass = (*env)->GetObjectClass(env, *jchnl);

	/* get userSpecObject field id */
	fid = (*env)->GetFieldID(env, jniChannelClass, "_userSpecObject", "Ljava/lang/Object;");
	if (fid == NULL)
	{
		return;
	}

	/* set JNIChannel userSpecObject field from AcceptOptions userSpecObject */
	(*env)->SetObjectField(env, *jchnl, fid, (*env)->CallObjectMethod(env, *jopts, mid));
}

/* get the C int value from the Java Integer object */
void getCIntegerValue(JNIEnv *env, jobject *jvalue, int *intVal)
{
	jclass integerClass;
	jmethodID mid;

	/* get the Integer class */
	integerClass = (*env)->GetObjectClass(env, *jvalue);

	/* get the method ID for the intValue() method */
	mid = (*env)->GetMethodID(env, integerClass, "toLong", "()J");
	if (mid == NULL)
	{
		return;
	}

	/* call intValue() method */
	*intVal = (int)(*env)->CallLongMethod(env, *jvalue, mid);
}

/* get the C string value from the Java String object */
void getCStringValue(JNIEnv *env, jobject *jvalue, char *strVal)
{
	const char *jstrVal;

	jstrVal = (*env)->GetStringUTFChars(env, *jvalue, NULL);
	strcpy(strVal, jstrVal);
	(*env)->ReleaseStringUTFChars(env, *jvalue, jstrVal);
}

/* copies a C RsslServerInfo structure to a Java ServerInfo class */
static RsslBool populateJavaServerInfo(JNIEnv *env, RsslServerInfo *rsslSrvrInfo, jobject *jsrvrinfo)
{
	jclass serverInfoClass;
	jmethodID mid;

	if (!rsslSrvrInfo || !*jsrvrinfo)
	{
		return RSSL_FALSE;
	}

	/* get the ServerInfo class */
	serverInfoClass = (*env)->GetObjectClass(env, *jsrvrinfo);

	/* get the method ID for the currentBufferUsage(int) method */
	mid = (*env)->GetMethodID(env, serverInfoClass, "currentBufferUsage", "(I)V");
	if (mid == NULL)
	{
		return RSSL_FALSE;
	}

	/* call currentBufferUsage method */
	(*env)->CallIntMethod(env, *jsrvrinfo, mid, rsslSrvrInfo->currentBufferUsage);

	/* get the method ID for the peakBufferUsage(int) method */
	mid = (*env)->GetMethodID(env, serverInfoClass, "peakBufferUsage", "(I)V");
	if (mid == NULL)
	{
		return RSSL_FALSE;
	}

	/* call peakBufferUsage method */
	(*env)->CallIntMethod(env, *jsrvrinfo, mid, rsslSrvrInfo->peakBufferUsage);

	return RSSL_TRUE;
}

/* notifies a java server that something is trying to connect to it */
static void notifyJavaServer(int jSrvrPort)
{
	struct sockaddr_in scktAddr;
	int scktFD;

	scktFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (scktFD == -1)
	{
		return;
	}

	memset(&scktAddr, 0, sizeof(scktAddr));
	scktAddr.sin_family = AF_INET;
	scktAddr.sin_port = htons(jSrvrPort);
#ifdef _WIN32
	InetPton(AF_INET, "127.0.0.1", &scktAddr.sin_addr.s_addr);
#else
	scktAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
#endif

	if(connect(scktFD, (struct sockaddr *)&scktAddr, sizeof(scktAddr)) == -1)
	{
		printf("Connect() failed!\n");
#ifdef _WIN32
		closesocket(scktFD);
#else
		close(scktFD);
#endif
	}
}

/* notifies a java channel that there is data available */
static void notifyJavaChannel(JNIChnlFDs *jniChnlFD)
{
	int sendLen;
	char dummyByte = 1;

	if (!jniChnlFD->isSrvrChnl ||
		(jniChnlFD->isSrvrChnl && jniChnlFD->connectionType != RSSL_CONN_TYPE_UNIDIR_SHMEM) ||
		(jniChnlFD->isSrvrChnl && jniChnlFD->connectionType == RSSL_CONN_TYPE_UNIDIR_SHMEM && jniChnlFD->numSrvrNotifies < 1))
	{
		sendLen = send(jniChnlFD->jChnlScktFD, &dummyByte, 1, 0);
		jniChnlFD->numSrvrNotifies++;

		if (sendLen < 0)
		{
			printf("notifyJavaChannel() failed\n");
		}
	}
}

/* accepts a java channel that the native code can communicate with */
static void acceptJavaChannel(int selectLoopServerFD)
{
	JNIChnlFDs *jniChnlFDs = NULL;
	int jChnlScktFD = accept(selectLoopServerFD, NULL, NULL);
 
	if(jChnlScktFD == -1)
	{
#ifdef _WIN32
		closesocket(jChnlScktFD);
#else
		close(jChnlScktFD);
#endif
		return;
	}

	/* add java channel FD to JNIChnlFDList */
	jniChnlFDs = (JNIChnlFDs *)malloc(sizeof(JNIChnlFDs));
	jniChnlFDs->jChnlScktFD = jChnlScktFD;
	jniChnlFDs->cChnlScktFD = RSSL_JNI_NULL_FD;
	rsslInitQueueLink(&jniChnlFDs->link1);
	rsslQueueAddLinkToBack(&JNIChnlFDList, &jniChnlFDs->link1);
	getLock();
#ifdef RSSL_JNI_DEBUG
	printf("FD_SET(jniChnlFDs->jChnlScktFD, &readfds): %d\n", jniChnlFDs->jChnlScktFD);
#endif
	FD_SET(jniChnlFDs->jChnlScktFD, &readfds);
	releaseLock();
}

/* reads from a java channel so native code can associate rssl c fds with java fds */
static void readJavaChannel(int jChnlScktFD)
{
	int readLen = 0, scktFD = 0;
	RsslConnectionTypes	connectionType = 0; 
	char readBuffer[32];
	JNIChnlFDs *jniChnlFDs = NULL;
	RsslBool isSrvrChnl = 0;
	RsslQueueLink *pLink;

#ifdef _WIN32
	readLen = recv(jChnlScktFD, readBuffer, 9, 0);
#else
	readLen = read(jChnlScktFD, readBuffer, 9);
#endif

	if (readLen > 0)
	{
		readBuffer[readLen] = '\0';
		scktFD = atoi(strtok(readBuffer, ":"));
		connectionType = (RsslConnectionTypes)atoi(strtok(NULL, ":"));
		isSrvrChnl = (RsslBool)atoi(strtok(NULL, ":"));
	}
	else
	{
		printf("readJavaChannel() failed\n");
	}

	/* add C channel FD to JNIChnlFDList, replace it, or close it */
	RSSL_QUEUE_FOR_EACH_LINK(&JNIChnlFDList, pLink)
	{
		jniChnlFDs = RSSL_QUEUE_LINK_TO_OBJECT(JNIChnlFDs, link1, pLink);
		if (jniChnlFDs->jChnlScktFD == jChnlScktFD)
		{
			if (jniChnlFDs->cChnlScktFD == RSSL_JNI_NULL_FD) /* add */
			{
				jniChnlFDs->cChnlScktFD = scktFD;
				jniChnlFDs->connectionType = connectionType;
				jniChnlFDs->isSrvrChnl = isSrvrChnl;
				jniChnlFDs->numSrvrNotifies = 0;
				getLock();
#ifdef RSSL_JNI_DEBUG
				printf("readJavaChannel() ADD for jniChnlFDs->jChnlScktFD: %d\n", jniChnlFDs->jChnlScktFD);
				printf("FD_SET(jniChnlFDs->cChnlScktFD, &readfds): %d\n", jniChnlFDs->cChnlScktFD);
				if (connectionType < RSSL_CONN_TYPE_RELIABLE_MCAST)
				{
					printf("FD_SET(jniChnlFDs->cChnlScktFD, &wrtfds): %d\n", jniChnlFDs->cChnlScktFD);
				}
#endif
				FD_SET(jniChnlFDs->cChnlScktFD, &readfds);
				if (connectionType < RSSL_CONN_TYPE_RELIABLE_MCAST)
				{
					FD_SET(jniChnlFDs->cChnlScktFD, &wrtfds);
				}
				releaseLock();
			}
			else if (scktFD != 0) /* replace */
			{
				getLock();
				jniChnlFDs->cChnlScktFD = scktFD;
#ifdef RSSL_JNI_DEBUG
				printf("readJavaChannel() REPLACE\n");
				printf("FD_SET(jniChnlFDs->cChnlScktFD, &readfds): %d\n", jniChnlFDs->cChnlScktFD);
				if (connectionType < RSSL_CONN_TYPE_RELIABLE_MCAST)
				{
					printf("FD_SET(jniChnlFDs->cChnlScktFD, &wrtfds): %d\n", jniChnlFDs->cChnlScktFD);
				}
#endif
				FD_SET(jniChnlFDs->cChnlScktFD, &readfds);
				if (connectionType < RSSL_CONN_TYPE_RELIABLE_MCAST)
				{
					FD_SET(jniChnlFDs->cChnlScktFD, &wrtfds);
				}
				releaseLock();
			}
			else if (scktFD == 0) /* close */
			{
#ifdef _WIN32
				shutdown(jniChnlFDs->jChnlScktFD, SD_BOTH);
				closesocket(jniChnlFDs->jChnlScktFD);
#else
				shutdown(jniChnlFDs->jChnlScktFD, SHUT_RDWR);
				close(jniChnlFDs->jChnlScktFD);
#endif
				getLock();
#ifdef RSSL_JNI_DEBUG
				printf("readJavaChannel() CLOSE\n");
				printf("FD_CLR(jniChnlFDs->jChnlScktFD, &readfds): %d\n", jniChnlFDs->jChnlScktFD);
#endif
				FD_CLR(jniChnlFDs->jChnlScktFD, &readfds);
				releaseLock();
				jniChnlFDs->jChnlScktFD = RSSL_JNI_NULL_FD;
				jniChnlFDs->cChnlScktFD = RSSL_JNI_NULL_FD;
				rsslQueueRemoveLink(&JNIChnlFDList, &jniChnlFDs->link1);
				free(jniChnlFDs);
			}
			break;
		}
	}
}

/* select loop */
/* facilitates communications between java connections and native connections */
static void* selectLoop(void* threadArg)
{
	JNIChnlFDs *jniChnlFDs = NULL;
	JNISrvrFDs *jniSrvrFDs = NULL;
	fd_set useRead;
	fd_set useExcept;
	fd_set useWrt;
	int selRet;
	struct timeval time_interval;
	RsslQueueLink *pLink;
#ifdef RSSL_JNI_DEBUG
	unsigned int selFailCount = 0;
#endif

	while(jniInitialized)
	{
		getLock();
		useRead = readfds;
		useExcept = exceptfds;
		useWrt = wrtfds;
		releaseLock();
		time_interval.tv_sec = 0L;
		time_interval.tv_usec = 20000;

#if defined(_WIN32)
		selRet = select(FD_SETSIZE, &useRead, &useWrt, &useExcept, &time_interval);
#else    
		selRet = select(1024, (fd_set *)&useRead, (fd_set *)&useWrt, (fd_set *)&useExcept, &time_interval);
#endif
#ifdef RSSL_JNI_DEBUG
		if (selRet == -1)
		{
			selFailCount++;
			if (selFailCount % 100000 == 1)
			{
				printf("selRet: %d\n", selRet);
#ifdef _WIN32
				printf("WSAGetLastError(): %d\n", WSAGetLastError());
#else
				printf("errno: %d\n", errno);
#endif
			}
		}
#endif
		if (selRet > 0)
		{
			if (FD_ISSET(selectLoopServerFD, &useRead))
			{
				acceptJavaChannel(selectLoopServerFD);
			}
			RSSL_QUEUE_FOR_EACH_LINK(&JNISrvrFDList, pLink)
			{
				jniSrvrFDs = RSSL_QUEUE_LINK_TO_OBJECT(JNISrvrFDs, link1, pLink);
				if ((jniSrvrFDs->cSrvrScktFD != RSSL_JNI_NULL_FD) &&
					(FD_ISSET(jniSrvrFDs->cSrvrScktFD, &useRead)))
				{
#ifdef RSSL_JNI_DEBUG
					printf("notifyJavaServer() jSrvrPort = %d\n", jniSrvrFDs->jSrvrPort);
#endif
					/* de-activate FD for accept (avoids multiple triggers for same connection) */
					getLock();
#ifdef RSSL_JNI_DEBUG
					printf("FD_CLR(jniSrvrFDs->cSrvrScktFD, &readfds): %d\n", jniSrvrFDs->cSrvrScktFD);
#endif
					FD_CLR(jniSrvrFDs->cSrvrScktFD, &readfds);
					releaseLock();
					notifyJavaServer(jniSrvrFDs->jSrvrPort);
				}
			}
			RSSL_QUEUE_FOR_EACH_LINK(&JNIChnlFDList, pLink)
			{
				jniChnlFDs = RSSL_QUEUE_LINK_TO_OBJECT(JNIChnlFDs, link1, pLink);

				if ((jniChnlFDs->jChnlScktFD != RSSL_JNI_NULL_FD) &&
					(FD_ISSET(jniChnlFDs->jChnlScktFD, &useRead)))
				{
#ifdef RSSL_JNI_DEBUG
					printf("readJavaChannel()\n");
#endif
					readJavaChannel(jniChnlFDs->jChnlScktFD);
				}
				if ((jniChnlFDs->cChnlScktFD != RSSL_JNI_NULL_FD) &&
					(FD_ISSET(jniChnlFDs->cChnlScktFD, &useRead)))
				{
#ifdef RSSL_JNI_DEBUG
					printf("notifyJavaChannel() Read FD\n");
#endif
					notifyJavaChannel(jniChnlFDs);
				}
				if ((jniChnlFDs->cChnlScktFD != RSSL_JNI_NULL_FD) &&
					(FD_ISSET(jniChnlFDs->cChnlScktFD, &useWrt)))
				{
#ifdef RSSL_JNI_DEBUG
					printf("FD_CLR(jniChnlFDsTemp->cChnlScktFD, &wrtfds): %d\n", jniChnlFDs->cChnlScktFD);
#endif
					getLock();
					FD_CLR(jniChnlFDs->cChnlScktFD, &wrtfds);
					releaseLock();
#ifdef RSSL_JNI_DEBUG
					printf("notifyJavaChannel() Write FD\n");
#endif
					notifyJavaChannel(jniChnlFDs);
				}
				if ((jniChnlFDs->cChnlScktFD != RSSL_JNI_NULL_FD) &&
					(FD_ISSET(jniChnlFDs->cChnlScktFD, &useExcept)))
				{
#ifdef RSSL_JNI_DEBUG
					printf("select() exceptfds triggered for socket id: %d\n", jniChnlFDs->cChnlScktFD);
#endif
				}
			}
		}
	}

	return NULL;
}

/* NATIVE JAVA METHODS */

/*
 * Class:     com_thomsonreuters_upa_transport_JNIProtocol
 * Method:    rsslInitialize
 * Signature: (ILcom/thomsonreuters/upa/transport/ErrorImpl;)I
 */
JNIEXPORT jint JNICALL Java_com_thomsonreuters_upa_transport_JNIProtocol_rsslInitialize
  (JNIEnv *env, jobject arg, jint jlocking, jobject jerror)
{
	RsslRet rsslRetVal;
	RsslError error;
	struct sockaddr_in selectLoopServerAddr; 

	error.channel = NULL;

	rsslRetVal = rsslInitialize(jlocking, &error);

	if (rsslRetVal == RSSL_RET_SUCCESS)
	{
		/* initialize lock for FDs */
		RSSL_MUTEX_INIT_ESDK(&fdsLock);

		/* clear selectLoopServer FDs */
		FD_ZERO(&readfds);
		FD_ZERO(&exceptfds);
		FD_ZERO(&wrtfds);
#ifndef _WIN32
		getLock();
		FD_SET(0,&readfds);
		releaseLock();
#endif

		/* create selectLoopServer socket */
		selectLoopServerFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

		if (selectLoopServerFD == -1)
		{
			return RSSL_RET_FAILURE;
		}

		/* set selectLoopServer socket */
		memset(&selectLoopServerAddr, 0, sizeof(selectLoopServerAddr));
		selectLoopServerAddr.sin_family = AF_INET;
		selectLoopServerAddr.sin_addr.s_addr = INADDR_ANY;
		while (RSSL_TRUE)
		{
			selectLoopServerAddr.sin_port = htons(selectLoopServerPort);

			/* bind selectLoopServer socket */
			if(bind(selectLoopServerFD, (struct sockaddr *)&selectLoopServerAddr, sizeof(selectLoopServerAddr)) == 0)
			{
				break;
			}
			selectLoopServerPort++;
		}
 
		/* listen with selectLoopServer socket */
		if(listen(selectLoopServerFD, 11) == -1)
		{
#ifdef _WIN32
			closesocket(selectLoopServerFD);
#else
			close(selectLoopServerFD);
#endif
			return RSSL_RET_FAILURE;
		}

		/* create selectLoopServer thread */
#if defined WIN32
		if (_beginthreadex(NULL, 0, (void *)&selectLoop, NULL, 0, &selectLoopThreadId) <= 0)
		{
			closesocket(selectLoopServerFD);
			return RSSL_RET_FAILURE;
		}
#else
		if (pthread_create(&selectLoopThreadId, NULL, selectLoop, NULL) != 0)
		{
			close(selectLoopServerFD);
			return RSSL_RET_FAILURE;
		}
#endif

		/* initialize internal structures */
		initJNILists();

		/* set selectLoopServer FD */
		getLock();
#ifdef RSSL_JNI_DEBUG
		printf("FD_SET(selectLoopServerFD, &readfds): %d\n", selectLoopServerFD);
#endif
		FD_SET(selectLoopServerFD, &readfds);
		releaseLock();
	}
	else /* populate error info in case of failure */
	{
		populateJavaError(env, &error, &jerror, NULL);
	}

	return rsslRetVal;
}

/*
 * Class:     com_thomsonreuters_upa_transport_JNIProtocol
 * Method:    rsslUninitialize
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_thomsonreuters_upa_transport_JNIProtocol_rsslUninitialize
  (JNIEnv *env, jobject arg)
{
	jniInitialized = RSSL_FALSE;

	/* clear selectLoopServer FD */
	getLock();
#ifdef RSSL_JNI_DEBUG
	printf("FD_CLR(selectLoopServerFD, &readfds): %d\n", selectLoopServerFD);
#endif
	FD_CLR(selectLoopServerFD, &readfds);
	releaseLock();
 
#ifdef _WIN32
	closesocket(selectLoopServerFD);
#else
	close(selectLoopServerFD);
#endif

	return rsslUninitialize();
}

/*
 * Class:     com_thomsonreuters_upa_transport_JNIProtocol
 * Method:    rsslConnect
 * Signature: (Lcom/thomsonreuters/upa/transport/ConnectOptionsImpl;Lcom/thomsonreuters/upa/transport/ErrorImpl;)Lcom/thomsonreuters/upa/transport/JNIChannel;
 */
JNIEXPORT jint JNICALL Java_com_thomsonreuters_upa_transport_JNIProtocol_rsslConnect
  (JNIEnv *env, jobject arg, jobject jopts, jobject jerror, jobject jniChannel)
{
	RsslError error;
	RsslChannel *rsslChnl = NULL;
	RsslConnectOptions rsslConnectOpts;
	char objectName[256], address[256], serviceName[256], interfaceName[256], unicastServiceName[256], sendAddr[256], sendPort[256], tcpControlPort[256], componentVersion[256];
	jclass jniChannelClass;
	jmethodID mid;

	error.channel = NULL;

	rsslClearConnectOpts(&rsslConnectOpts);
	rsslConnectOpts.objectName = objectName;
	rsslConnectOpts.objectName[0] = '\0';
	rsslConnectOpts.connectionInfo.unified.address = address;
	rsslConnectOpts.connectionInfo.unified.address[0] = '\0';
	rsslConnectOpts.connectionInfo.unified.serviceName = serviceName;
	rsslConnectOpts.connectionInfo.unified.serviceName[0] = '\0';
	rsslConnectOpts.connectionInfo.unified.interfaceName = interfaceName;
	rsslConnectOpts.connectionInfo.unified.interfaceName[0] = '\0';
	rsslConnectOpts.connectionInfo.unified.unicastServiceName = unicastServiceName;
	rsslConnectOpts.connectionInfo.unified.unicastServiceName[0] = '\0';
	rsslConnectOpts.connectionInfo.segmented.sendAddress = sendAddr;
	rsslConnectOpts.connectionInfo.segmented.sendAddress[0] = '\0';
	rsslConnectOpts.connectionInfo.segmented.sendServiceName = sendPort;
	rsslConnectOpts.connectionInfo.segmented.sendServiceName[0] = '\0';
	rsslConnectOpts.multicastOpts.tcpControlPort = tcpControlPort;
	rsslConnectOpts.multicastOpts.tcpControlPort[0] = '\0';
	rsslConnectOpts.componentVersion = componentVersion;
	rsslConnectOpts.componentVersion[0] = '\0';

	
	if (!populateCConnectOptions(env, &jopts, &rsslConnectOpts))
	{
		strcpy(error.text, "Invalid RsslConnectOptions");
		populateJavaError(env, &error, &jerror, &jniChannel);
		return RSSL_RET_FAILURE;
	}

	if (strlen(rsslConnectOpts.connectionInfo.unified.interfaceName) == 0)
	{
		rsslConnectOpts.connectionInfo.unified.interfaceName = 0;
	}
	if (strlen(rsslConnectOpts.componentVersion) == 0)
	{
		rsslConnectOpts.componentVersion = 0;
	}
	rsslChnl = rsslConnect(&rsslConnectOpts, &error);
	if (rsslChnl != NULL)
	{
		/* populate Java JNIChannel object */
		if (!populateJavaChannel(env, rsslChnl, &jniChannel, RSSL_TRUE))
		{
			return RSSL_RET_FAILURE;;
		}

		/* connect java channel */
		if (!connectJavaChannel(env, rsslChnl, &jniChannel))
		{
			return RSSL_RET_FAILURE;
		}

		/* get the JNIChannel class */
		jniChannelClass = (*env)->GetObjectClass(env, jniChannel);
		if (jniChannelClass == NULL)
		{
			return RSSL_RET_FAILURE;
		}

		/* configure blocking mode */
		/* get the method ID for the configureBlockingMode(boolean) method */
		mid = (*env)->GetMethodID(env, jniChannelClass, "configureBlockingMode", "(Z)V");
		if (mid == NULL)
		{
			return RSSL_RET_FAILURE;
		}

		/* call configureBlockingMode method */
		(*env)->CallBooleanMethod(env, jniChannel, mid, rsslConnectOpts.blocking);
	}
	else /* populate error info in case of failure */
	{
		populateJavaError(env, &error, &jerror, &jniChannel);
		return RSSL_RET_FAILURE;
	}

	return RSSL_RET_SUCCESS;
}

/*
 * Class:     com_thomsonreuters_upa_transport_JNIProtocol
 * Method:    rsslBind
 * Signature: (Lcom/thomsonreuters/upa/transport/BindOptionsImpl;Lcom/thomsonreuters/upa/transport/ErrorImpl;)Lcom/thomsonreuters/upa/transport/JNIServer;
 */
JNIEXPORT jint JNICALL Java_com_thomsonreuters_upa_transport_JNIProtocol_rsslBind
  (JNIEnv *env, jobject arg, jobject jopts, jobject jerror, jobject jniServer)
{
	RsslError error;
	RsslServer *rsslSrvr;
	RsslBindOptions rsslBindOpts;
	char serviceName[256], interfaceName[256], componentVersion[256];
	jclass jniServerClass;
	jmethodID cid;

	error.channel = NULL;

	rsslClearBindOpts(&rsslBindOpts);
	rsslBindOpts.serviceName = serviceName;
	rsslBindOpts.serviceName[0] = '\0';
	rsslBindOpts.interfaceName = interfaceName;
	rsslBindOpts.interfaceName[0] = '\0';
	rsslBindOpts.componentVersion = componentVersion;
	rsslBindOpts.componentVersion[0] = '\0';
	
	if (!populateCBindOptions(env, &jopts, &rsslBindOpts))
	{
		return RSSL_RET_FAILURE;
	}

	if (strlen(rsslBindOpts.interfaceName) == 0)
	{
		rsslBindOpts.interfaceName = 0;
	}

	if (strlen(rsslBindOpts.componentVersion) == 0)
	{
		rsslBindOpts.componentVersion = 0;
	}

	rsslSrvr = rsslBind(&rsslBindOpts, &error);
	if (rsslSrvr != NULL)
	{
		/* populate Java JNIServer object */
		if (!populateJavaServer(env, rsslSrvr, &jniServer, &rsslBindOpts, RSSL_TRUE))
		{
			return RSSL_RET_FAILURE;
		}

		/* bind java server */
		if (!bindJavaServer(env, rsslSrvr, &jniServer))
		{
			return RSSL_RET_FAILURE;
		}

		/* get the JNIServer class */
		jniServerClass = (*env)->GetObjectClass(env, jniServer);
		if (jniServerClass == NULL)
		{
			return RSSL_RET_FAILURE;
		}

		/* configure blocking mode */
		/* get the method ID for the configureBlockingMode() method */
		cid = (*env)->GetMethodID(env, jniServerClass, "configureBlockingMode", "()V");
		if (cid == NULL)
		{
			return RSSL_RET_FAILURE;
		}

		/* call configureBlockingMode method */
		(*env)->CallBooleanMethod(env, jniServer, cid);
	}
	else /* populate error info in case of failure */
	{
		populateJavaError(env, &error, &jerror, NULL);
	}

	return RSSL_RET_SUCCESS;
}

/*
 * Class:     com_thomsonreuters_upa_transport_JNIProtocol
 * Method:    rsslAccept
 * Signature: (Lcom/thomsonreuters/upa/transport/JNIServer;Lcom/thomsonreuters/upa/transport/AcceptOptionsImpl;Lcom/thomsonreuters/upa/transport/ErrorImpl;)Lcom/thomsonreuters/upa/transport/JNIChannel;
 */
JNIEXPORT jint JNICALL Java_com_thomsonreuters_upa_transport_JNIProtocol_rsslAccept
  (JNIEnv *env, jobject arg, jobject jsrvr, jobject jopts, jobject jerror, jobject jniChannel)
{
	RsslError error;
	RsslChannel *rsslChnl;
	RsslServer *rsslSrvr;
	RsslAcceptOptions rsslAcceptsOpts;
	jclass jniChannelClass, jniServerClass, acceptOptsClass;
	jmethodID cid;
	jfieldID fid;
	jboolean channelsBlocking;
	jmethodID mid;
	int sysSendBufSize;

	error.channel = NULL;

	rsslClearAcceptOpts(&rsslAcceptsOpts);
	if (!populateCAcceptOptions(env, &jopts, &rsslAcceptsOpts))
	{
		return RSSL_RET_FAILURE;
	}

	rsslSrvr = getCServer(env, &jsrvr);
	if (rsslSrvr == NULL)
	{
		return RSSL_RET_FAILURE;
	}

	rsslChnl = rsslAccept(rsslSrvr, &rsslAcceptsOpts, &error);
	if (rsslChnl != NULL)
	{
		/* handle Java AcceptOptions.sysSendBufSize by setting value via UPAC ioctl */
		
		/* get the AcceptOptions class */
		acceptOptsClass = (*env)->GetObjectClass(env, jopts);

		/* get the method ID for the sysSendBufSize() method */
		mid = (*env)->GetMethodID(env, acceptOptsClass, "sysSendBufSize", "()I");
		if (mid == NULL)
		{
			return RSSL_RET_FAILURE;
		}

		/* get sysSendBufSize */
		sysSendBufSize = (*env)->CallIntMethod(env, jopts, mid);

		/* set value via UPAC ioctl */
		if (sysSendBufSize > 0)
		{
			if (rsslIoctl(rsslChnl, RSSL_SYSTEM_WRITE_BUFFERS, &sysSendBufSize, &error) < RSSL_RET_SUCCESS)
			{
				populateJavaError(env, &error, &jerror, &jniChannel);
				return RSSL_RET_FAILURE;
			}
		}

		/* accept java server */
		if (!acceptJavaServer(env, rsslSrvr, &jsrvr))
		{
			return RSSL_RET_FAILURE;
		}

		/* populate Java JNIChannel object */
		if (!populateJavaChannel(env, rsslChnl, &jniChannel, RSSL_TRUE))
		{
			return RSSL_RET_FAILURE;
		}

		if (rsslAcceptsOpts.userSpecPtr == NULL)
		{
			/* set channel userSpecObject from server */
			setUserSpecObjectFromServer(env, &jsrvr, &jniChannel);
		}
		else
		{
			/* set channel userSpecObject from acceptOpts */
			setUserSpecObjectFromAcceptOpts(env, &jopts, &jniChannel);
		}

		/* connect java channel */
		if (!connectJavaChannel(env, rsslChnl, &jniChannel))
		{
			return RSSL_RET_FAILURE;
		}

		/* configure blocking mode */
		/* get the JNIServer class */
		jniServerClass = (*env)->GetObjectClass(env, jsrvr);

		/* get channelsBlocking field id */
		fid = (*env)->GetFieldID(env, jniServerClass, "_channelsBlocking", "Z");
		if (fid == NULL)
		{
			return RSSL_RET_FAILURE;
		}
		channelsBlocking = (*env)->GetBooleanField(env, jsrvr, fid);

		/* get the JNIChannel class */
		jniChannelClass = (*env)->GetObjectClass(env, jniChannel);
		if (jniChannelClass == NULL)
		{
			return RSSL_RET_FAILURE;
		}

		/* get the method ID for the configureBlockingMode(boolean) method */
		cid = (*env)->GetMethodID(env, jniChannelClass, "configureBlockingMode", "(Z)V");
		if (cid == NULL)
		{
			return RSSL_RET_FAILURE;
		}

		/* call configureBlockingMode method */
		(*env)->CallBooleanMethod(env, jniChannel, cid, channelsBlocking);
	}
	else /* populate error info in case of failure */
	{
		populateJavaError(env, &error, &jerror, &jniChannel);
	}

	/* re-activate FD for accept (avoids multiple triggers for same connection) */
	getLock();
	FD_SET(rsslSrvr->socketId, &readfds);
	releaseLock();

	return RSSL_RET_SUCCESS;
}

/*
 * Class:     com_thomsonreuters_upa_transport_JNIServer
 * Method:    rsslGetServerInfo
 * Signature: (Lcom/thomsonreuters/upa/transport/JNIServer;Lcom/thomsonreuters/upa/transport/ServerInfoImpl;Lcom/thomsonreuters/upa/transport/ErrorImpl;)I
 */
JNIEXPORT jint JNICALL Java_com_thomsonreuters_upa_transport_JNIServer_rsslGetServerInfo
  (JNIEnv *env, jobject arg, jobject jsrvr, jobject jinfo, jobject jerror)
{
	RsslRet rsslRetVal;
	RsslError error;
	RsslServer *rsslSrvr;
	RsslServerInfo rsslSrvrInfo;

	error.channel = NULL;

	rsslSrvr = getCServer(env, &jsrvr);
	if (rsslSrvr == NULL)
	{
		return RSSL_RET_FAILURE;
	}

	rsslRetVal = rsslGetServerInfo(rsslSrvr, &rsslSrvrInfo, &error);

	if (rsslRetVal == RSSL_RET_SUCCESS)
	{
		/* populate serverInfo object */
		if (!populateJavaServerInfo(env, &rsslSrvrInfo, &jinfo))
		{
			return RSSL_RET_FAILURE;
		}
	}
	else /* populate error info in case of failure */
	{
		populateJavaError(env, &error, &jerror, NULL);
	}

	return rsslRetVal;
}

/*
 * Class:     com_thomsonreuters_upa_transport_JNIServer
 * Method:    rsslServerIoctl
 * Signature: (Lcom/thomsonreuters/upa/transport/JNIServer;ILjava/lang/Object;Lcom/thomsonreuters/upa/transport/ErrorImpl;)I
 */
JNIEXPORT jint JNICALL Java_com_thomsonreuters_upa_transport_JNIServer_rsslServerIoctl
  (JNIEnv *env, jobject arg, jobject jsrvr, jint jcode, jobject jvalue, jobject jerror)
{
	RsslRet rsslRetVal;
	RsslError error;
	RsslServer *rsslSrvr;
	int intVal = 0;
	char strVal[256];

	error.channel = NULL;
	strVal[0] = '\0';

	rsslSrvr = getCServer(env, &jsrvr);
	if (rsslSrvr == NULL)
	{
		return RSSL_RET_FAILURE;
	}

	switch (jcode)
	{
		case RSSL_SERVER_NUM_POOL_BUFFERS:
			getCIntegerValue(env, &jvalue, &intVal);
		case RSSL_SERVER_PEAK_BUF_RESET:
			rsslRetVal = rsslServerIoctl(rsslSrvr, jcode, &intVal, &error);
			break;
		default:
			return RSSL_RET_FAILURE;
			break;
	}
	/* populate error info in case of failure */
	if (rsslRetVal < RSSL_RET_SUCCESS)
	{
		populateJavaError(env, &error, &jerror, NULL);
	}

	return rsslRetVal;
}

/*
 * Class:     com_thomsonreuters_upa_transport_JNIServer
 * Method:    rsslServerBufferUsage
 * Signature: (Lcom/thomsonreuters/upa/transport/JNIServer;Lcom/thomsonreuters/upa/transport/ErrorImpl;)I
 */
JNIEXPORT jint JNICALL Java_com_thomsonreuters_upa_transport_JNIServer_rsslServerBufferUsage
  (JNIEnv *env, jobject arg, jobject jsrvr, jobject jerror)
{
	RsslRet rsslRetVal;
	RsslError error;
	RsslServer *rsslSrvr;

	error.channel = NULL;

	rsslSrvr = getCServer(env, &jsrvr);
	if (rsslSrvr == NULL)
	{
		return RSSL_RET_FAILURE;
	}

	rsslRetVal = rsslServerBufferUsage(rsslSrvr, &error);
	/* populate error info in case of failure */
	if (rsslRetVal < RSSL_RET_SUCCESS)
	{
		populateJavaError(env, &error, &jerror, NULL);
	}

	return rsslRetVal;
}

/*
 * Class:     com_thomsonreuters_upa_transport_JNIServer
 * Method:    rsslCloseServer
 * Signature: (Lcom/thomsonreuters/upa/transport/JNIServer;Lcom/thomsonreuters/upa/transport/ErrorImpl;)I
 */
JNIEXPORT jint JNICALL Java_com_thomsonreuters_upa_transport_JNIServer_rsslCloseServer
  (JNIEnv *env, jobject arg, jobject jsrvr, jobject jerror)
{
	RsslRet rsslRetVal;
	RsslError error;
	RsslServer *rsslSrvr;

	error.channel = NULL;

	rsslSrvr = getCServer(env, &jsrvr);
	if (rsslSrvr == NULL)
	{
		return RSSL_RET_FAILURE;
	}

	/* close java server */
	if (!closeJavaServer(env, rsslSrvr, &jsrvr))
	{
		return RSSL_RET_FAILURE;
	}

	rsslRetVal = rsslCloseServer(rsslSrvr, &error);

	/* populate error info in case of failure */
	if (rsslRetVal < RSSL_RET_SUCCESS)
	{
		populateJavaError(env, &error, &jerror, NULL);
	}

	return rsslRetVal;
}
  
/*
 * Class:     com_thomsonreuters_upa_transport_JNIChannel
 * Method:    rsslGetChannelInfo
 * Signature: (Lcom/thomsonreuters/upa/transport/JNIChannel;Lcom/thomsonreuters/upa/transport/ChannelInfoImpl;Lcom/thomsonreuters/upa/transport/ErrorImpl;)I
 */
JNIEXPORT jint JNICALL Java_com_thomsonreuters_upa_transport_JNIChannel_rsslGetChannelInfo
  (JNIEnv *env, jobject arg, jobject jchnl, jobject jinfo, jobject jerror)
{
	RsslRet rsslRetVal;
	RsslError error;
	RsslChannel *rsslChnl;
	RsslChannelInfo rsslChnlInfo;

	error.channel = NULL;

	rsslChnl = getCChannel(env, &jchnl);
	if (rsslChnl == NULL)
	{
		return RSSL_RET_FAILURE;
	}

	rsslRetVal = rsslGetChannelInfo(rsslChnl, &rsslChnlInfo, &error);

	if (rsslRetVal == RSSL_RET_SUCCESS)
	{
		/* populate channelInfo object */
		if (!populateJavaChannelInfo(env, &rsslChnlInfo, &jinfo))
		{
			return RSSL_RET_FAILURE;
		}
	}
	else /* populate error info in case of failure */
	{
		populateJavaError(env, &error, &jerror, NULL);
	}

	return rsslRetVal;
}

/*
 * Class:     com_thomsonreuters_upa_transport_JNIChannel
 * Method:    rsslIoctl
 * Signature: (Lcom/thomsonreuters/upa/transport/JNIChannel;ILjava/lang/Object;Lcom/thomsonreuters/upa/transport/ErrorImpl;)I
 */
JNIEXPORT jint JNICALL Java_com_thomsonreuters_upa_transport_JNIChannel_rsslIoctl
  (JNIEnv *env, jobject arg, jobject jchnl, jint jcode, jobject jvalue, jobject jerror)
{
	RsslRet rsslRetVal;
	RsslError error;
	RsslChannel *rsslChnl;
	int intVal = 0;
	char strVal[256];

	error.channel = NULL;
	strVal[0] = '\0';

	rsslChnl = getCChannel(env, &jchnl);
	if (rsslChnl == NULL)
	{
		return RSSL_RET_FAILURE;
	}

	switch (jcode)
	{
		case RSSL_MAX_NUM_BUFFERS:
		case RSSL_NUM_GUARANTEED_BUFFERS:
		case RSSL_HIGH_WATER_MARK:
		case RSSL_SYSTEM_READ_BUFFERS:
		case RSSL_SYSTEM_WRITE_BUFFERS:
		case RSSL_COMPRESSION_THRESHOLD:
			getCIntegerValue(env, &jvalue, &intVal);
			rsslRetVal = rsslIoctl(rsslChnl, jcode, &intVal, &error);
			break;
		case RSSL_PRIORITY_FLUSH_ORDER:
			getCStringValue(env, &jvalue, strVal);
			rsslRetVal = rsslIoctl(rsslChnl, jcode, strVal, &error);
			break;
		default:
			return RSSL_RET_FAILURE;
			break;
	}
	/* populate error info in case of failure */
	if (rsslRetVal < RSSL_RET_SUCCESS)
	{
		populateJavaError(env, &error, &jerror, NULL);
	}

	return rsslRetVal;
}

/*
 * Class:     com_thomsonreuters_upa_transport_JNIChannel
 * Method:    rsslBufferUsage
 * Signature: (Lcom/thomsonreuters/upa/transport/JNIChannel;Lcom/thomsonreuters/upa/transport/ErrorImpl;)I
 */
JNIEXPORT jint JNICALL Java_com_thomsonreuters_upa_transport_JNIChannel_rsslBufferUsage
  (JNIEnv *env, jobject arg, jobject jchnl, jobject jerror)
{
	RsslRet rsslRetVal;
	RsslError error;
	RsslChannel *rsslChnl;

	error.channel = NULL;

	rsslChnl = getCChannel(env, &jchnl);
	if (rsslChnl == NULL)
	{
		return RSSL_RET_FAILURE;
	}

	rsslRetVal = rsslBufferUsage(rsslChnl, &error);
	/* populate error info in case of failure */
	if (rsslRetVal < RSSL_RET_SUCCESS)
	{
		populateJavaError(env, &error, &jerror, NULL);
	}

	return rsslRetVal;
}

/*
 * Class:     com_thomsonreuters_upa_transport_JNIChannel
 * Method:    rsslInitChannel
 * Signature: (Lcom/thomsonreuters/upa/transport/JNIChannel;Lcom/thomsonreuters/upa/transport/InProgInfoImpl;Lcom/thomsonreuters/upa/transport/ErrorImpl;)I
 */
JNIEXPORT jint JNICALL Java_com_thomsonreuters_upa_transport_JNIChannel_rsslInitChannel
  (JNIEnv *env, jobject arg, jobject jchnl, jobject jinprog, jobject jerror)
{
	RsslRet rsslRetVal;
	RsslError error;
	RsslChannel *rsslChnl;
	RsslInProgInfo inProg;

	error.channel = NULL;

	if (!clearJavaChannel(env, &jchnl))
	{
		return RSSL_RET_FAILURE;
	}

	rsslChnl = getCChannel(env, &jchnl);
	if (rsslChnl == NULL)
	{
		return RSSL_RET_FAILURE;
	}

	rsslRetVal = rsslInitChannel(rsslChnl, &inProg, &error);
	if (rsslRetVal >= RSSL_RET_SUCCESS)
	{
		/* populate channel object */
		if (!populateJavaChannel(env, rsslChnl, &jchnl, RSSL_FALSE))
		{
			return RSSL_RET_FAILURE;
		}

		/* populate inProg object */
		if (!populateJavaInProg(env, &jchnl, &jinprog, inProg.flags))
		{
			return RSSL_RET_FAILURE;
		}

		if ((rsslRetVal == RSSL_RET_CHAN_INIT_IN_PROGRESS) && (inProg.flags & RSSL_IP_FD_CHANGE))
		{
			/* clear FDs */
			getLock();
#ifdef RSSL_JNI_DEBUG
			printf("FD_CLR(inProg.oldSocket, &readfds): %d\n", inProg.oldSocket);
#endif
			FD_CLR(inProg.oldSocket, &readfds);
			releaseLock();

			/* perform the FD change */
			if (!writeJavaChannel(env, rsslChnl, &jchnl))
			{
				return RSSL_RET_FAILURE;
			}
		}
	}
	else /* populate error info in case of failure */
	{
		populateJavaError(env, &error, &jerror, NULL);
	}

	return rsslRetVal;
}

/*
 * Class:     com_thomsonreuters_upa_transport_JNIChannel
 * Method:    rsslCloseChannel
 * Signature: (Lcom/thomsonreuters/upa/transport/JNIChannel;Lcom/thomsonreuters/upa/transport/ErrorImpl;)I
 */
JNIEXPORT jint JNICALL Java_com_thomsonreuters_upa_transport_JNIChannel_rsslCloseChannel
  (JNIEnv *env, jobject arg, jobject jchnl, jobject jerror)
{
	RsslRet rsslRetVal;
	RsslError error;
	RsslChannel *rsslChnl;

	error.channel = NULL;

	rsslChnl = getCChannel(env, &jchnl);
	if (rsslChnl == NULL)
	{
		return RSSL_RET_FAILURE;
	}

	/* close java channel */
	if (!closeJavaChannel(env, rsslChnl, &jchnl))
	{
		return RSSL_RET_FAILURE;
	}

	/* clear FDs */
	getLock();
#ifdef RSSL_JNI_DEBUG
	printf("FD_CLR(rsslChnl->socketId, &readfds): %d\n", rsslChnl->socketId);
#endif
	FD_CLR(rsslChnl->socketId, &readfds);
	releaseLock();

	rsslRetVal = rsslCloseChannel(rsslChnl, &error);

	/* populate error info in case of failure */
	if (rsslRetVal < RSSL_RET_SUCCESS)
	{
		populateJavaError(env, &error, &jerror, NULL);
	}

	return rsslRetVal;
}

/*
 * Class:     com_thomsonreuters_upa_transport_JNIChannel
 * Method:    rsslRead
 * Signature: (Lcom/thomsonreuters/upa/transport/JNIChannel;Lcom/thomsonreuters/upa/transport/ReadArgsImpl;Lcom/thomsonreuters/upa/transport/ErrorImpl;)Lcom/thomsonreuters/upa/transport/JNIBuffer;
 */
JNIEXPORT jobject JNICALL Java_com_thomsonreuters_upa_transport_JNIChannel_rsslRead
  (JNIEnv *env, jobject arg, jobject jchnl, jobject jreadargs, jobject jerror)
{
	RsslRet rsslRetVal;
	RsslReadInArgs readInArgs;
	RsslReadOutArgs readOutArgs;
	RsslError error;
	RsslChannel *rsslChnl;
	RsslBuffer *rsslBuffer;
	jobject jbuffer;

	error.channel = NULL;

	if (!clearJavaChannel(env, &jchnl))
	{
		return NULL;
	}

	rsslChnl = getCChannel(env, &jchnl);
	if (rsslChnl == NULL)
	{
		return NULL;
	}

	rsslClearReadInArgs(&readInArgs);
	rsslClearReadOutArgs(&readOutArgs);
	rsslBuffer = rsslReadEx(rsslChnl, &readInArgs, &readOutArgs, &rsslRetVal, &error);
	if (rsslRetVal >= RSSL_RET_SUCCESS && rsslBuffer != NULL)
	{
		/* get java read buffer object */
		jbuffer = getJavaReadBuffer(env, rsslBuffer, &jchnl);
	}
	else /* populate return error info in case of failure */
	{
		if (rsslRetVal != RSSL_RET_READ_WOULD_BLOCK)
		{
			populateJavaError(env, &error, &jerror, NULL);
		}
		jbuffer = NULL;
	}

	/* handle RSSL_RET_READ_FD_CHANGE */
	if (rsslRetVal == RSSL_RET_READ_FD_CHANGE)
	{
		/* clear FDs */
		getLock();
#ifdef RSSL_JNI_DEBUG
		printf("FD_CLR(rsslChnl->oldSocketId, &readfds): %d\n", rsslChnl->oldSocketId);
#endif
		FD_CLR(rsslChnl->oldSocketId, &readfds);
		releaseLock();
	}

	/* populate return value and bytes read */
	populateReadReturnValue(env, rsslRetVal, &readOutArgs, &jreadargs);

	return jbuffer;
}

/*
 * Class:     com_thomsonreuters_upa_transport_JNIChannel
 * Method:    rsslGetBuffer
 * Signature: (Lcom/thomsonreuters/upa/transport/JNIChannel;IZLcom/thomsonreuters/upa/transport/ErrorImpl;)Lcom/thomsonreuters/upa/transport/JNIBuffer;
 */
JNIEXPORT jobject JNICALL Java_com_thomsonreuters_upa_transport_JNIChannel_rsslGetBuffer
  (JNIEnv *env, jobject arg, jobject jchnl, jint jsize, jboolean jpackbuffer, jobject jerror)
{
	RsslError error;
	RsslChannel *rsslChnl;
	RsslBuffer *rsslBuffer;
	jobject jbuffer;

	error.channel = NULL;

	rsslChnl = getCChannel(env, &jchnl);
	if (rsslChnl == NULL)
	{
		return NULL;
	}
	rsslBuffer = rsslGetBuffer(rsslChnl, jsize, jpackbuffer, &error);
	if (rsslBuffer != NULL)
	{
		/* create java buffer object */
		jbuffer = createJavaBuffer(env, rsslBuffer);
	}
	else /* populate return error info in case of failure */
	{
		populateJavaError(env, &error, &jerror, NULL);
		jbuffer = NULL;
	}

	return jbuffer;
}

/*
 * Class:     com_thomsonreuters_upa_transport_JNIChannel
 * Method:    rsslReleaseBuffer
 * Signature: (Lcom/thomsonreuters/upa/transport/JNIBuffer;Lcom/thomsonreuters/upa/transport/ErrorImpl;)I
 */
JNIEXPORT jint JNICALL Java_com_thomsonreuters_upa_transport_JNIChannel_rsslReleaseBuffer
  (JNIEnv *env, jobject arg, jobject jbuffer, jobject jerror)
{
	RsslRet rsslRetVal;
	RsslError error;
	RsslBuffer *rsslBuffer;

	error.channel = NULL;

	rsslBuffer = getCBuffer(env, &jbuffer);

	rsslRetVal = rsslReleaseBuffer(rsslBuffer, &error);
	/* populate return error info in case of failure */
	if (rsslRetVal != RSSL_RET_SUCCESS)
	{
		populateJavaError(env, &error, &jerror, NULL);
	}

	return rsslRetVal;
}

/*
 * Class:     com_thomsonreuters_upa_transport_JNIChannel
 * Method:    rsslPackBuffer
 * Signature: (Lcom/thomsonreuters/upa/transport/JNIChannel;Lcom/thomsonreuters/upa/transport/JNIBuffer;Lcom/thomsonreuters/upa/transport/ErrorImpl;)I
 */
JNIEXPORT jint JNICALL Java_com_thomsonreuters_upa_transport_JNIChannel_rsslPackBuffer
  (JNIEnv *env, jobject arg, jobject jchnl, jobject jbuffer, jobject jerror)
{
	RsslError error;
	RsslChannel *rsslChnl;
	RsslBuffer *rsslBuffer, *rsslPackedBuffer;
	jobject jpackedbuffer;

	error.channel = NULL;

	rsslChnl = getCChannel(env, &jchnl);
	if (rsslChnl == NULL)
	{
		return 0;
	}

	rsslBuffer = copyToCBuffer(env, &jbuffer);

	rsslPackedBuffer = rsslPackBuffer(rsslChnl, rsslBuffer, &error);
	if (rsslPackedBuffer != NULL)
	{
		/* create java buffer object */
		jpackedbuffer = createJavaBuffer(env, rsslPackedBuffer);
	}
	else /* populate return error info in case of failure */
	{
		populateJavaError(env, &error, &jerror, NULL);
		jpackedbuffer = NULL;
	}

	return rsslPackedBuffer->length;
}

/*
 * Class:     com_thomsonreuters_upa_transport_JNIChannel
 * Method:    rsslWrite
 * Signature: (Lcom/thomsonreuters/upa/transport/JNIChannel;Lcom/thomsonreuters/upa/transport/JNIBuffer;Lcom/thomsonreuters/upa/transport/WriteArgsImpl;Lcom/thomsonreuters/upa/transport/ErrorImpl;)I
 */
JNIEXPORT jint JNICALL Java_com_thomsonreuters_upa_transport_JNIChannel_rsslWrite
  (JNIEnv *env, jobject arg, jobject jchnl, jobject jbuffer, jobject jwriteargs, jobject jerror)
{
	RsslRet rsslRetVal;
	RsslError error;
	RsslChannel *rsslChnl;
	RsslBuffer *rsslBuffer;
	RsslWriteInArgs writeInArgs;
	RsslWriteOutArgs writeOutArgs;
	static jclass writeArgsClass = NULL;
	jclass localRefClass;
	static jmethodID priorityMid = NULL, flagsMid = NULL;

	error.channel = NULL;

	rsslChnl = getCChannel(env, &jchnl);
	if (rsslChnl == NULL)
	{
		return RSSL_RET_FAILURE;
	}

	rsslBuffer = copyToCBuffer(env, &jbuffer);

	rsslClearWriteInArgs(&writeInArgs);
	rsslClearWriteOutArgs(&writeOutArgs);

	/* get the WriteArgs class */
	if (writeArgsClass == NULL)
	{
		localRefClass = (*env)->GetObjectClass(env, jwriteargs);
		if (localRefClass == NULL)
		{
			return RSSL_RET_FAILURE;
		}
		/* Create a global reference */
		writeArgsClass = (*env)->NewGlobalRef(env, localRefClass);
		(*env)->DeleteLocalRef(env, localRefClass);
		if (writeArgsClass == NULL)
		{
			return RSSL_RET_FAILURE;
		}
	}

	/* get the method ID for the priority() method */
	if (priorityMid == NULL)
	{
		priorityMid = (*env)->GetMethodID(env, writeArgsClass, "priority", "()I");
		if (priorityMid == NULL)
		{
			return RSSL_RET_FAILURE;
		}
	}

	/* call priority() method to get the priority */
	writeInArgs.rsslPriority = (RsslUInt8)(*env)->CallIntMethod(env, jwriteargs, priorityMid);

	/* get the method ID for the flags() method */
	if (flagsMid == NULL)
	{
		flagsMid = (*env)->GetMethodID(env, writeArgsClass, "flags", "()I");
		if (flagsMid == NULL)
		{
			return RSSL_RET_FAILURE;
		}
	}

	/* call flags() method to get the flags */
	writeInArgs.writeInFlags = (RsslUInt8)(*env)->CallIntMethod(env, jwriteargs, flagsMid);

	rsslRetVal = rsslWriteEx(rsslChnl, rsslBuffer, &writeInArgs, &writeOutArgs, &error);
	/* populate return error info in case of failure */
	if (rsslRetVal < RSSL_RET_SUCCESS)
	{
		populateJavaError(env, &error, &jerror, NULL);
	}
	else /* populate WriteArgs object */
	{
		populateBytesWritten(env, &writeOutArgs, &jwriteargs);
	}
								 
	return rsslRetVal;
}

/*
 * Class:     com_thomsonreuters_upa_transport_JNIChannel
 * Method:    rsslFlush
 * Signature: (Lcom/thomsonreuters/upa/transport/JNIChannel;Lcom/thomsonreuters/upa/transport/ErrorImpl;)I
 */
JNIEXPORT jint JNICALL Java_com_thomsonreuters_upa_transport_JNIChannel_rsslFlush
  (JNIEnv *env, jobject arg, jobject jchnl, jobject jerror)
{
	RsslRet rsslRetVal;
	RsslError error;
	RsslChannel *rsslChnl;

	error.channel = NULL;

	rsslChnl = getCChannel(env, &jchnl);
	if (rsslChnl == NULL)
	{
		return RSSL_RET_FAILURE;
	}

	rsslRetVal = rsslFlush(rsslChnl, &error);
	/* populate return error info in case of failure */
	if (rsslRetVal < RSSL_RET_SUCCESS)
	{
		populateJavaError(env, &error, &jerror, NULL);
	}

	return rsslRetVal;
}

/*
 * Class:     com_thomsonreuters_upa_transport_JNIChannel
 * Method:    rsslPing
 * Signature: (Lcom/thomsonreuters/upa/transport/JNIChannel;Lcom/thomsonreuters/upa/transport/ErrorImpl;)I
 */
JNIEXPORT jint JNICALL Java_com_thomsonreuters_upa_transport_JNIChannel_rsslPing
  (JNIEnv *env, jobject arg, jobject jchnl, jobject jerror)
{
	RsslRet rsslRetVal;
	RsslError error;
	RsslChannel *rsslChnl;

	error.channel = NULL;

	rsslChnl = getCChannel(env, &jchnl);
	if (rsslChnl == NULL)
	{
		return RSSL_RET_FAILURE;
	}

	rsslRetVal = rsslPing(rsslChnl, &error);
	/* populate return error info in case of failure */
	if (rsslRetVal < RSSL_RET_SUCCESS)
	{
		populateJavaError(env, &error, &jerror, NULL);
	}

	return rsslRetVal;
}

/*
 * Class:     com_thomsonreuters_upa_transport_JNIChannel
 * Method:    rsslReconnectClient
 * Signature: (Lcom/thomsonreuters/upa/transport/JNIChannel;Lcom/thomsonreuters/upa/transport/ErrorImpl;)I
 */
JNIEXPORT jint JNICALL Java_com_thomsonreuters_upa_transport_JNIChannel_rsslReconnectClient
  (JNIEnv *env, jobject arg, jobject jchnl, jobject jerror)
{
	RsslRet rsslRetVal;
	RsslError error;
	RsslChannel *rsslChnl;

	error.channel = NULL;

	rsslChnl = getCChannel(env, &jchnl);
	if (rsslChnl == NULL)
	{
		return RSSL_RET_FAILURE;
	}

	rsslRetVal = rsslReconnectClient(rsslChnl, &error);
	/* populate error info in case of failure */
	if (rsslRetVal < RSSL_RET_SUCCESS)
	{
		populateJavaError(env, &error, &jerror, NULL);
	}

	return rsslRetVal;
}
