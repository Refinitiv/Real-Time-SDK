/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

/* this file was machine generated */
#include <jni.h>
/* Header for class com_refinitiv_eta_transport_JNIProtocol */

#ifndef _Included_com_refinitiv_eta_transport_JNIProtocol
#define _Included_com_refinitiv_eta_transport_JNIProtocol
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     com_refinitiv_eta_transport_JNIProtocol
 * Method:    rsslInitialize
 * Signature: (ILcom/refinitiv/eta/transport/ErrorImpl;)I
 */
JNIEXPORT jint JNICALL Java_com_refinitiv_eta_transport_JNIProtocol_rsslInitialize
  (JNIEnv *, jobject, jint, jobject);

/*
 * Class:     com_refinitiv_eta_transport_JNIProtocol
 * Method:    rsslUninitialize
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_refinitiv_eta_transport_JNIProtocol_rsslUninitialize
  (JNIEnv *, jobject);

/*
 * Class:     com_refinitiv_eta_transport_JNIProtocol
 * Method:    rsslConnect
 * Signature: (Lcom/refinitiv/eta/transport/ConnectOptionsImpl;Lcom/refinitiv/eta/transport/ErrorImpl;Lcom/refinitiv/eta/transport/JNIChannel;)I
 */
JNIEXPORT jint JNICALL Java_com_refinitiv_eta_transport_JNIProtocol_rsslConnect
  (JNIEnv *, jobject, jobject, jobject, jobject);

/*
 * Class:     com_refinitiv_eta_transport_JNIProtocol
 * Method:    rsslBind
 * Signature: (Lcom/refinitiv/eta/transport/BindOptionsImpl;Lcom/refinitiv/eta/transport/ErrorImpl;Lcom/refinitiv/eta/transport/JNIServer;)I
 */
JNIEXPORT jint JNICALL Java_com_refinitiv_eta_transport_JNIProtocol_rsslBind
  (JNIEnv *, jobject, jobject, jobject, jobject);

/*
 * Class:     com_refinitiv_eta_transport_JNIProtocol
 * Method:    rsslAccept
 * Signature: (Lcom/refinitiv/eta/transport/JNIServer;Lcom/refinitiv/eta/transport/AcceptOptionsImpl;Lcom/refinitiv/eta/transport/ErrorImpl;Lcom/refinitiv/eta/transport/JNIChannel;)I
 */
JNIEXPORT jint JNICALL Java_com_refinitiv_eta_transport_JNIProtocol_rsslAccept
  (JNIEnv *, jobject, jobject, jobject, jobject, jobject);

#ifdef __cplusplus
}
#endif
#endif
