/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

/* this file was machine generated */
#include <jni.h>
/* Header for class com_thomsonreuters_upa_transport_JNIProtocol */

#ifndef _Included_com_thomsonreuters_upa_transport_JNIProtocol
#define _Included_com_thomsonreuters_upa_transport_JNIProtocol
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     com_thomsonreuters_upa_transport_JNIProtocol
 * Method:    rsslInitialize
 * Signature: (ILcom/thomsonreuters/upa/transport/ErrorImpl;)I
 */
JNIEXPORT jint JNICALL Java_com_thomsonreuters_upa_transport_JNIProtocol_rsslInitialize
  (JNIEnv *, jobject, jint, jobject);

/*
 * Class:     com_thomsonreuters_upa_transport_JNIProtocol
 * Method:    rsslUninitialize
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_thomsonreuters_upa_transport_JNIProtocol_rsslUninitialize
  (JNIEnv *, jobject);

/*
 * Class:     com_thomsonreuters_upa_transport_JNIProtocol
 * Method:    rsslConnect
 * Signature: (Lcom/thomsonreuters/upa/transport/ConnectOptionsImpl;Lcom/thomsonreuters/upa/transport/ErrorImpl;Lcom/thomsonreuters/upa/transport/JNIChannel;)I
 */
JNIEXPORT jint JNICALL Java_com_thomsonreuters_upa_transport_JNIProtocol_rsslConnect
  (JNIEnv *, jobject, jobject, jobject, jobject);

/*
 * Class:     com_thomsonreuters_upa_transport_JNIProtocol
 * Method:    rsslBind
 * Signature: (Lcom/thomsonreuters/upa/transport/BindOptionsImpl;Lcom/thomsonreuters/upa/transport/ErrorImpl;Lcom/thomsonreuters/upa/transport/JNIServer;)I
 */
JNIEXPORT jint JNICALL Java_com_thomsonreuters_upa_transport_JNIProtocol_rsslBind
  (JNIEnv *, jobject, jobject, jobject, jobject);

/*
 * Class:     com_thomsonreuters_upa_transport_JNIProtocol
 * Method:    rsslAccept
 * Signature: (Lcom/thomsonreuters/upa/transport/JNIServer;Lcom/thomsonreuters/upa/transport/AcceptOptionsImpl;Lcom/thomsonreuters/upa/transport/ErrorImpl;Lcom/thomsonreuters/upa/transport/JNIChannel;)I
 */
JNIEXPORT jint JNICALL Java_com_thomsonreuters_upa_transport_JNIProtocol_rsslAccept
  (JNIEnv *, jobject, jobject, jobject, jobject, jobject);

#ifdef __cplusplus
}
#endif
#endif
