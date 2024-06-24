/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
 *|-----------------------------------------------------------------------------
 */

#include "rtr/ripcinetutils.h"
#include "rtr/ripcflip.h"
#include <stdlib.h>

#include "rtr/rsslTransport.h"

/* #define DBUG_INIT */
/* #define DBUG_CALLBACK */
/* #define DBUG_RECONNECT */
/* #define DBUG_CBACKCOUNT */

void __stdcall dummyWinInetCallback(HINTERNET hInternet, 
						 DWORD_PTR dwContext, 
						 DWORD dwInternetStatus, 
						 LPVOID lpvStatusInformation, 
						 DWORD dwStatusInformationLength)
{
	;  /* dont do anything */
}


void __stdcall dummyCleanupWinInetCallback(HINTERNET hInternet, 
						 DWORD_PTR dwContext, 
						 DWORD dwInternetStatus, 
						 LPVOID lpvStatusInformation, 
						 DWORD dwStatusInformationLength)
{
	switch (dwInternetStatus)
	{
		case INTERNET_STATUS_REQUEST_COMPLETE:
			if (hInternet != NULL)
			{
				/*	InternetSetStatusCallback(hInternet, dummyWinInetCallback);
					InternetCloseHandle(hInternet);*/
			}
		break;
		default:
			;
	}
}

RTR_C_INLINE void ripcWinInetUnregisterCallbacks(ripcWinInetSession* sess)
{
	if (sess->streamingReqHandle)
		InternetSetStatusCallback(sess->streamingReqHandle, dummyWinInetCallback);
	if (sess->newStreamingReqHandle)
		InternetSetStatusCallback(sess->newStreamingReqHandle, dummyWinInetCallback);
	if (sess->connectHandle)
		InternetSetStatusCallback(sess->connectHandle, dummyWinInetCallback);
	if (sess->controlReqHandle)
		InternetSetStatusCallback(sess->controlReqHandle, dummyWinInetCallback);
	if (sess->openHandle)
		InternetSetStatusCallback(sess->openHandle, dummyWinInetCallback);
	if (sess->newControlReqHandle)
		InternetSetStatusCallback(sess->newControlReqHandle, dummyWinInetCallback);
}

/* our WinInet callback function */
void __stdcall winInetCallback(HINTERNET hInternet, 
						 DWORD_PTR dwContext, 
						 DWORD dwInternetStatus, 
						 LPVOID lpvStatusInformation, 
						 DWORD dwStatusInformationLength )
{
	ripcWinInetSession *sess = 0;
	DWORD dwCert = 0;
	char setBTR = 0;
	/* We use this to keep track of if we need to lock/unlock during the reconnection 
	 * process.  Because the sessions reconnection state can change while we are in here
	 * we want something independent 
	 */
	char reconnectLocked = 0;  

	INTERNET_ASYNC_RESULT *pRes  = (INTERNET_ASYNC_RESULT*)lpvStatusInformation;
	sess = (ripcWinInetSession*)dwContext;
	
#ifdef DBUG_CALLBACK
	printf("winInetCallback called with callbackPending = %d, callback status code: %d\n", sess->callbackPending, dwInternetStatus);
#endif

	switch (dwInternetStatus)
	{
	case INTERNET_STATUS_REQUEST_COMPLETE:
	{
		if (!sess)
			return;

		sess->inCallback = 1;

		/* once we are active, only shutdown changes this */
		/* streamline this and only lock the initialization cases */
		if (sess->state == WININET_ACTIVE)
		{
			/* we should lock all of this if we are in the reconnect process */
			if (sess->reconnectWindow)
			{
#ifdef DBUG_RECONNECT
				printf("Reconnect locking in callback\n");
#endif
				RSSL_MUTEX_LOCK(&sess->tunnelMutex);
				reconnectLocked = 1;
			}

			/* check to see if this is a callback for the control channel blocking write */
			if ( (hInternet == sess->controlReqHandle) && (sess->writeCallbackPending == 1) )
			{
				sess->writeCallbackPending = 0;
				if (reconnectLocked)
					RSSL_MUTEX_UNLOCK(&sess->tunnelMutex);

				/* now return */
				sess->inCallback = 0;
				return;
			}

			if ((hInternet == sess->streamingReqHandle) && (sess->reconnectState != WININET_FINAL_CHUNK_RECEIVED))
			{
				if(pRes->dwResult)
				{
					if( sess->bytesToRead == 0)
					{
						/* Our streaming channel just closed, 
						 * Switch to the new streaming channel.
						 * We still want to signal a the client to call read
						 * so we begin to read the other channel.
						 */
						
						if (!reconnectLocked)
							RSSL_MUTEX_LOCK(&sess->tunnelMutex);
#ifdef DBUG_RECONNECT
						printf("Got final chunk in callback on handle %d\n", sess->streamingReqHandle);
#endif						
						/* just always go to this state and let the read sort it out */
						sess->reconnectState = WININET_FINAL_CHUNK_RECEIVED;

						if (!reconnectLocked)
							RSSL_MUTEX_UNLOCK(&sess->tunnelMutex);
					}
				}
				else
				{
					/* IQDA may have returned an error */
					/* We can ignore this specific error - here is what MS says about it
					 * The operation was canceled, usually because the handle on which the request was operating
					 * was closed before the operation completed.
					 * We have occasionally seen this while the reconnection process is underway, so we suspect an IQDA
					 * call gets cancelled as a result to our call to InternetCloseHandle on the streaming handle
					 */
					if (pRes->dwError != ERROR_INTERNET_OPERATION_CANCELLED)
					{
						snprintf(sess->errorText, MAX_RSSL_ERROR_TEXT, "<%s:%d> winInetCallback error - InternetQueryDataAvailable returned error %d ", __FILE__, __LINE__, pRes->dwError);
						sess->state = WININET_ERROR;
					}
				}

				/* wake up the client application.
				 * even if we took an error.
				 */
				if (!reconnectLocked)
					RSSL_MUTEX_LOCK(&sess->tunnelMutex);

				/* if we came in here for the final chunk dont decrement */
				if (--sess->callbackPending)
				{
					snprintf(sess->errorText, MAX_RSSL_ERROR_TEXT, "<%s:%d> winInetCallback error - unexpected number of pending callbacks ", __FILE__, __LINE__);
					sess->state = WININET_ERROR;
				}	
#ifdef DBUG_CBACKCOUNT
				printf("<%s:%d> Callback Count: %d\n", __FILE__, __LINE__, sess->callbackPending);
#endif
				if (InterlockedCompareExchange (&sess->byteWritten, 1L, 0L) == 0)
				{
                	if (rssl_pipe_write(&sess->_pipe, "1", 1) != 1)
					{
						sess->byteWritten = 0;
						snprintf(sess->errorText, MAX_RSSL_ERROR_TEXT, "<%s:%d> winInetCallback error - unable to write byte to pipe ", __FILE__, __LINE__);
						sess->state = WININET_ERROR;
					}
				}
				if (!reconnectLocked)
					RSSL_MUTEX_UNLOCK(&sess->tunnelMutex);
			}
			else
			{
				/* this will probably never occur because we should always be in the reconnectWindow if we get in here */
				if (!reconnectLocked)  
					RSSL_MUTEX_LOCK(&sess->tunnelMutex);
				if ((sess->newControlReqHandle) && (hInternet == sess->newControlReqHandle))
				{			
					/* this is the case where we are now ready to flip our ControlReqHandles */
					
					/* toggle readstate */
					/* first time through this - send the request */
					/* this will trigger a call to read which will write our session request */
					/* this means we set up the control req handle, lets send our req now */
					
					switch (sess->reconnectState)
					{
						/* Got the callback from HttpSendRequestEx - change state and write to pipe */
						case WININET_CONTROL_CONN_REINITIALIZING:
						{
#ifdef DBUG_RECONNECT
							printf("Got callback on new control handle - in state CONTROL_CONN_REINITIALIZING, changing to CONTROL_CONN_SEND_POST\n");
#endif
							/* we can send our control conn request */
							sess->reconnectState = WININET_CONTROL_CONN_SEND_POST;
							if (InterlockedCompareExchange (&sess->byteWritten, 1L, 0L) == 0)
							{
			                	if (rssl_pipe_write(&sess->_pipe, "1", 1) != 1)
								{
									sess->byteWritten = 0;
									snprintf(sess->errorText, MAX_RSSL_ERROR_TEXT, "<%s:%d> winInetCallback error - unable to write byte to pipe ", __FILE__, __LINE__);
									sess->state = WININET_ERROR;
								}
							}
						}
						break;

						default:
						break;
					}			
				}
				else if ((sess->newStreamingReqHandle) && (hInternet == sess->newStreamingReqHandle))
				{	
					switch (sess->reconnectState)
					{
						/* HttpSendRequest is done, now we need to call IQDA */
						case WININET_STREAMING_CONN_REINITIALIZING:
						{
#ifdef DBUG_RECONNECT
							printf("In new streaming callback - switching from STREAMING_CONN_REINITIALIZING to STREAMING_CALL_IQDA\n");
#endif
							sess->reconnectState = WININET_STREAMING_CONN_CALL_IQDA;
							if (InterlockedCompareExchange (&sess->byteWritten, 1L, 0L) == 0)
							{
			                	if (rssl_pipe_write(&sess->_pipe, "1", 1) != 1)
								{
									sess->byteWritten = 0;
									snprintf(sess->errorText, MAX_RSSL_ERROR_TEXT, "<%s:%d> winInetCallback error - unable to write byte to pipe ", __FILE__, __LINE__);
									sess->state = WININET_ERROR;
								}
							}
						}
						break;

						/* waiting for IQDA to return a size to us for streaming ack  */
						case WININET_STREAMING_CONN_WAIT_ACK:
						{
							if(pRes->dwResult)
							{
								/* Believe it or not, this is according to MS 
								 * On the success case dwError contains the number of bytes to read.
								 */
								if( sess->reconnectBytesToRead == 0)
								{
									/* The streaming channel has just closed, we should have had a new one.	*/
									/* Unless we change the reconnection process, if this happens its an error */
									snprintf(sess->errorText, MAX_RSSL_ERROR_TEXT, "<%s:%d> winInetCallback error - connection closed ", __FILE__, __LINE__);
									sess->state = WININET_ERROR;
								}
								else
								{
									sess->reconnectState = WININET_STREAMING_CONN_ACK_RECEIVED;
								}
							}
							else
							{
								/* IQDA returned an error */
								snprintf(sess->errorText, MAX_RSSL_ERROR_TEXT, "<%s:%d> winInetCallback error - InternetQueryDataAvailable returned error %d ", __FILE__, __LINE__, pRes->dwError);
								sess->state = WININET_ERROR;
							}

							/* wake up the client application.
							 * even if we took an error.
							 */
							if (InterlockedCompareExchange (&sess->byteWritten, 1L, 0L) == 0)
							{
			                	if (rssl_pipe_write(&sess->_pipe, "1", 1) != 1)
								{
									sess->byteWritten = 0;
									snprintf(sess->errorText, MAX_RSSL_ERROR_TEXT, "<%s:%d> winInetCallback error - unable to write byte to pipe ", __FILE__, __LINE__);
									sess->state = WININET_ERROR;
								}
							}
						}
						break;
					
						/* Waiting for IQDA to return size of control conn ack to us */
						case WININET_CONTROL_CONN_WAIT_ACK:
						{
							/* should have gotten bytes from IQDA */
							if(pRes->dwResult)
							{
								/* Believe it or not, this is according to MS ??? 
								 * On the success case dwError contains the number of bytes to read.
								 */
								if( sess->reconnectBytesToRead == 0)
								{
									/* The streaming channel has just closed, we should have had a new one. */
									/* Unless we change the reconnection process, if this happens its an error */
									snprintf(sess->errorText, MAX_RSSL_ERROR_TEXT, "<%s:%d> winInetCallback error - connection closed ", __FILE__, __LINE__);
									sess->state = WININET_ERROR;
								}
								else
								{	
									sess->reconnectState = WININET_CONTROL_CONN_ACK_RECEIVED;
								}
							}
							else
							{
								/* IQDA returned an error */
								snprintf(sess->errorText, MAX_RSSL_ERROR_TEXT, "<%s:%d> winInetCallback error - InternetQueryDataAvailable returned error %d ", __FILE__, __LINE__, pRes->dwError);
								sess->state = WININET_ERROR;
							}

							/* wake up the client application.
							 * even if we took an error.
							 */
							if (InterlockedCompareExchange (&sess->byteWritten, 1L, 0L) == 0)
							{
			                	if (rssl_pipe_write(&sess->_pipe, "1", 1) != 1)
								{
									sess->byteWritten = 0;
									snprintf(sess->errorText, MAX_RSSL_ERROR_TEXT, "<%s:%d> winInetCallback error - unable to write byte to pipe ", __FILE__, __LINE__);
									sess->state = WININET_ERROR;
								}
							}
						}
						break;
					
						/* shouldnt get here for the others states */
						default:
						break;
					}
				}
				
				if (!reconnectLocked)
					RSSL_MUTEX_UNLOCK(&sess->tunnelMutex);
			}
			if (reconnectLocked)
				RSSL_MUTEX_UNLOCK(&sess->tunnelMutex);

			/* now return */
			sess->inCallback = 0;
			return;
		}
		
		if (sess->state == WININET_ERROR)
		{
			sess->inCallback = 0;
			return;
		}

		RSSL_MUTEX_LOCK(&sess->tunnelMutex);
		switch (sess->state)
		{
			case WININET_INIT_STREAM_SENDRQST:
#ifdef DBUG_INIT
				printf("CALLBACK - WININET_INIT_STREAM_SENDRQST\n");
#endif

				if (hInternet == sess->streamingReqHandle)
				{
					if (pRes->dwError == ERROR_INTERNET_CLIENT_AUTH_CERT_NEEDED)
					{
						if (!(InternetSetOption(sess->streamingReqHandle, INTERNET_OPTION_SECURITY_SELECT_CLIENT_CERT, &dwCert, sizeof(dwCert))));
						{
							snprintf(sess->errorText, MAX_RSSL_ERROR_TEXT, "<%s:%d> winInetCallback error - unable to use default client certificate and proxy requires it ", __FILE__, __LINE__);
							sess->state = WININET_ERROR;
						}

						if (InterlockedCompareExchange (&sess->byteWritten, 1L, 0L) == 0)
						{
		                	if (rssl_pipe_write(&sess->_pipe, "1", 1) != 1)
							{
								sess->byteWritten = 0;
								snprintf(sess->errorText, MAX_RSSL_ERROR_TEXT, "<%s:%d> winInetCallback error - unable to write byte to pipe ", __FILE__, __LINE__);
								sess->state = WININET_ERROR;
							}
						}
					}
					else if (pRes->dwError == ERROR_INTERNET_INVALID_CA)
					{
						snprintf(sess->errorText, MAX_RSSL_ERROR_TEXT, "<%s:%d> winInetCallback error - server is using invalid or untrusted certificate. ", __FILE__,__LINE__);
						sess->state = WININET_ERROR;
					}
					else
					{
						/* The call to HTTPsendRequest just finished asynchoronously.
						 * Write to the pipe to get them to call init again so we can call IQDA
						 */
						if(pRes->dwResult)
						{
							/* The call to HTTPsendRequest just finished asynchoronously.
							 * Write to the pipe to get them to call init again so we can call IQDA
							 */
							sess->state = WININET_INIT_STREAM_RESPONSE;
						}
						else
						{
							/* HTTPsendRequestEx failed */
							snprintf(sess->errorText, MAX_RSSL_ERROR_TEXT, "<%s:%d> winInetCallback error - HTTPSendRequest unable to complete successfully ", __FILE__, __LINE__);
							sess->state = WININET_ERROR;
						}
					}

					if (--sess->callbackPending)
					{
						snprintf(sess->errorText, MAX_RSSL_ERROR_TEXT, "<%s:%d> winInetCallback error - unexpected number of pending callbacks ", __FILE__, __LINE__);
						sess->state = WININET_ERROR;
					}
#ifdef DBUG_CBACKCOUNT
					printf("<%s:%d> Callback Count: %d\n", __FILE__, __LINE__, sess->callbackPending);
#endif

					/* wake up app, even if we got an error */
					if (InterlockedCompareExchange (&sess->byteWritten, 1L, 0L) == 0)
					{
	                	if (rssl_pipe_write(&sess->_pipe, "1", 1) != 1)
						{
							sess->byteWritten = 0;
							snprintf(sess->errorText, MAX_RSSL_ERROR_TEXT, "<%s:%d> winInetCallback error - unable to write byte to pipe ", __FILE__, __LINE__);
							sess->state = WININET_ERROR;
						}
					}
				}
			break;

			case WININET_INIT_STREAM_RESPONSE:
#ifdef DBUG_INIT
				printf("CALLBACK - WININET_INIT_STREAM_RESPONSE\n");
#endif
				if (hInternet == sess->streamingReqHandle)
				{
					/* The call to IQDA just finished asynchoronously.
					 * Write to the pipe to get them to call init again so we can call IQDA
					 */
					if(sess->bytesToRead)
					{
						sess->state = WININET_INIT_STREAM_ACK_RECEIVED;
					}
					else
					{
						snprintf(sess->errorText, MAX_RSSL_ERROR_TEXT, "<%s:%d> winInetCallback error - InternetQueryDataAvailable returned error %d ", __FILE__, __LINE__, pRes->dwError);
						sess->state = WININET_ERROR;
					}

					if(--sess->callbackPending)
					{
						snprintf(sess->errorText, MAX_RSSL_ERROR_TEXT, "<%s:%d> winInetCallback error - unexpected number of pending callbacks ", __FILE__, __LINE__);
						sess->state = WININET_ERROR;
					}
#ifdef DBUG_CBACKCOUNT
					printf("<%s:%d> Callback Count: %d\n", __FILE__, __LINE__, sess->callbackPending);
#endif

					/* wake up app, even if we got an error */
					if (InterlockedCompareExchange (&sess->byteWritten, 1L, 0L) == 0)
					{
	                	if (rssl_pipe_write(&sess->_pipe, "1", 1) != 1)
						{
							sess->byteWritten = 0;
							snprintf(sess->errorText, MAX_RSSL_ERROR_TEXT, "<%s:%d> winInetCallback error - unable to write byte to pipe ", __FILE__, __LINE__);
							sess->state = WININET_ERROR;
						}
					}
				}
			break;

			case WININET_INIT_CONTROL_SENDRQST:
#ifdef DBUG_INIT
				printf("CALLBACK - WININET_INIT_CONTROL_SENDRQST\n");
#endif
				/* The Control channel sendrquest command just finished asynchronously */

				if (hInternet == sess->controlReqHandle)
				{
					/* The call to HTTPsendRequest just finished asynchoronously.
					 * Write to the pipe to get them to call init again so we can call IQDA
					 */
					if(pRes->dwResult)
					{
						/* The call to HTTPsendRequest just finished asynchoronously.
						 * Write to the pipe to get them to call init again so we can call IQDA
						 */
						sess->state = WININET_INIT_CONTROL_SNDRQST_COMPL;
					}
					else
					{
						/* HTTPsendRequestEx failed */
						snprintf(sess->errorText, MAX_RSSL_ERROR_TEXT, "<%s:%d> winInetCallback error - HTTPSendRequestEx unable to complete successfully ", __FILE__, __LINE__);
						sess->state = WININET_ERROR;
					}

					if(--sess->callbackPending)
					{
						snprintf(sess->errorText, MAX_RSSL_ERROR_TEXT, "<%s:%d> winInetCallback error - unexpected number of pending callbacks ", __FILE__, __LINE__);
						sess->state = WININET_ERROR;
					}
#ifdef DBUG_CBACKCOUNT
					printf("<%s:%d> Callback Count: %d\n", __FILE__, __LINE__, sess->callbackPending);
#endif

					/* wake up app even if we got an error */
					if (InterlockedCompareExchange (&sess->byteWritten, 1L, 0L) == 0)
					{
	                	if (rssl_pipe_write(&sess->_pipe, "1", 1) != 1)
						{
							sess->byteWritten = 0;
							snprintf(sess->errorText, MAX_RSSL_ERROR_TEXT, "<%s:%d> winInetCallback error - unable to write byte to pipe ", __FILE__, __LINE__);
							sess->state = WININET_ERROR;
						}
					}
				}
			break;

			case WININET_INIT_CONTROL_SNDRQST_PARTIAL:
#ifdef DBUG_INIT
				printf("CALLBACK - WININET_INIT_CONTROL_SNDRQST_PARTIAL\n");
#endif
				/* The Control channel sendrquest command just finished asynchronously */

				if (hInternet == sess->controlReqHandle)
				{
					/* The call to HTTPsendRequest just finished asynchoronously.
					 * Write to the pipe to get them to call init again so we can call IQDA
					 */
					if(pRes->dwResult)
					{
						/* The call to HTTPsendRequest just finished asynchoronously.
						 * Write to the pipe to get them to call init again so we can call IQDA
						 */
						sess->state = WININET_INIT_CONTROL_SNDRQST_FULL_COMPL;
					}
					else
					{
						/* HTTPsendRequestEx failed */
						snprintf(sess->errorText, MAX_RSSL_ERROR_TEXT, "<%s:%d> winInetCallback error - HTTPSendRequestEx unable to complete successfully ", __FILE__, __LINE__);
						sess->state = WININET_ERROR;
					}

					if(--sess->callbackPending)
					{
						snprintf(sess->errorText, MAX_RSSL_ERROR_TEXT, "<%s:%d> winInetCallback error - unexpected number of pending callbacks ", __FILE__, __LINE__);
						sess->state = WININET_ERROR;
					}
#ifdef DBUG_CBACKCOUNT
					printf("<%s:%d> Callback Count: %d\n", __FILE__, __LINE__, sess->callbackPending);
#endif

					/* wake up app even if we got an error */
					if (InterlockedCompareExchange (&sess->byteWritten, 1L, 0L) == 0)
					{
	                	if (rssl_pipe_write(&sess->_pipe, "1", 1) != 1)
						{
							sess->byteWritten = 0;
							snprintf(sess->errorText, MAX_RSSL_ERROR_TEXT, "<%s:%d> winInetCallback error - unable to write byte to pipe ", __FILE__, __LINE__);
							sess->state = WININET_ERROR;
						}
					}
				}
			break;

			default: 
				/* some unhandled state - not sure how we could get here */
				snprintf(sess->errorText, MAX_RSSL_ERROR_TEXT, "<%s:%d> winInetCallback error - unexpected channel state (%d) from inside callback ", __FILE__, __LINE__, sess->state);
				sess->state = WININET_ERROR;
		}
		RSSL_MUTEX_UNLOCK(&sess->tunnelMutex);
		sess->inCallback = 0;
		break;
	}

	default:
		return;
	}

	return;
}

/* our transport read function -
 this will read from the network using SSL and return the appropriate value to the ripc layer */
RsslRet ripcWinInetRead( void *winInetSess, char *buf, RsslInt32 max_len, ripcRWFlags flags, RsslError *error )
{
	RsslInt32 totalBytes = 0;
	DWORD _error = 0;
	ripcWinInetSession *sess = (ripcWinInetSession*)winInetSess;
	char tempBuf[5];
	char* inBuf;
	RsslInt32 inLen;
	RsslInt32 errorCode;
	RsslInt32 retval;
	char stayLocked = 0;
	sess->outBytesForRead = 0;

	if (sess->state == WININET_ERROR)
	{
		/* An Error happened - 
		 * Return the text and code and fail */
		/* Move the errorText from the session into the error structure */
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "%s", sess->errorText);
		ripcWinInetErrors(error, strlen(error->text));
		/* unset the callback */
		ripcWinInetUnregisterCallbacks(sess);
		return RSSL_RET_FAILURE;
	}

	RSSL_MUTEX_LOCK(&sess->tunnelMutex);
	if (InterlockedCompareExchange (&sess->byteWritten, 0L, 1L) == 1)
	{
		if (rssl_pipe_read(&sess->_pipe, tempBuf, 1) <= 0)
		{
			sess->byteWritten = 1;	/* we didnt read the byte, assume its still coming, fix the flag to say its still there */
			errorCode = WSAGetLastError();
			if (errorCode != WSAEWOULDBLOCK) /* if the byte isnt ready, leave it for next time. We cant wait forever */
			{
				/* if we cant read from the pipe thats bad */
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, errorCode);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcWinInetRead: Unable to read byte from pipe. ", __FILE__, __LINE__);
				sess->state = WININET_ERROR;
				ripcWinInetUnregisterCallbacks(sess);	/* unset the callback */
				RSSL_MUTEX_UNLOCK(&sess->tunnelMutex);
				return RSSL_RET_FAILURE;
			}
		}
	}
	
	/* we are in the process of changing the control channel */
	/* This can probably be made a different lock so that we can enter the callback for normal data flow */
	switch (sess->reconnectState)
	{
		case WININET_INIT_OK:						/* do nothing here */
		case WININET_STREAMING_CONN_REINITIALIZING: /* handled by the callback */
		case WININET_STREAMING_CONN_WAIT_ACK:		/* handled by the callback */
		case WININET_CONTROL_CONN_REINITIALIZING:   /* handled by the callback */
		case WININET_CONTROL_CONN_WAIT_ACK:			/* handled by the callback */
		case WININET_WAIT_FINAL_CHUNK:				/* handled by the callback */
		break;
		
		case WININET_FINAL_CHUNK_RECEIVED:
		{
			/* if we have a new streaming handle, switch it and loop around again */
			/* if we dont have a new handle there is likely some problem */
			if (sess->newStreamingReqHandle)
			{
				HINTERNET tempHandle = 0;
#ifdef DBUG_RECONNECT
				printf("In read reconnect section, got final chunk and switching from %d to new streaming handle %d and ending reconnect window\n", sess->streamingReqHandle, sess->newStreamingReqHandle);
#endif
				InternetSetStatusCallback(sess->streamingReqHandle, dummyWinInetCallback);
				tempHandle = sess->streamingReqHandle;
				sess->streamingReqHandle = sess->newStreamingReqHandle;
				sess->newStreamingReqHandle = 0;
				/* reconnect window is over now so we can stop locking in write */
				
				sess->reconnectState = WININET_INIT_OK;

				/* Call IQDA now */
				if (InternetQueryDataAvailable(sess->streamingReqHandle,&sess->bytesToRead , 0, 0))
				{
					if (sess->bytesToRead == 0)
					{
						/* this is the end of a the chunk  */
						_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
						snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcWinInetRead: Received final chunk on new streaming channel (%d). ", __FILE__, __LINE__, sess->bytesToRead);
						/* unset the callback */
						ripcWinInetUnregisterCallbacks(sess);
						InternetCloseHandle(tempHandle);
						sess->state = WININET_ERROR;
						RSSL_MUTEX_UNLOCK(&sess->tunnelMutex);
						return RSSL_RET_FAILURE;
					}
				}
				else
				{
					_error = GetLastError();
			
					if (_error != ERROR_IO_PENDING) 
					{
						/* We should not get error 12017 here - we only need to handle that in the callback */
						_rsslSetError(error, NULL, RSSL_RET_FAILURE, _error);
						snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcWinInetRead IQDA returns error (%d). ", __FILE__, __LINE__, _error);
						ripcWinInetErrors(error, strlen(error->text));
						/* unset the callback */
						ripcWinInetUnregisterCallbacks(sess);
						InternetCloseHandle(tempHandle);
						sess->state = WININET_ERROR;
						RSSL_MUTEX_UNLOCK(&sess->tunnelMutex);
						return RSSL_RET_FAILURE;
					}
					else
					{
						if (++sess->callbackPending > 1)
						{
							_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
							snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcWinInetRead - unexpected number of pending callbacks ", __FILE__, __LINE__);
							sess->state = WININET_ERROR;
							/* unset the callback */
							ripcWinInetUnregisterCallbacks(sess);
							InternetCloseHandle(tempHandle);
							RSSL_MUTEX_UNLOCK(&sess->tunnelMutex);
							return RSSL_RET_FAILURE;
						}
#ifdef DBUG_CBACKCOUNT
						printf("<%s:%d> Callback Count: %d\n", __FILE__, __LINE__, sess->callbackPending);
#endif
						InternetCloseHandle(tempHandle);
						sess->reconnectWindow = 0;

						RSSL_MUTEX_UNLOCK(&sess->tunnelMutex);
						/* There is no data to read so just return and wait for the callback */
						return RSSL_RET_SUCCESS;
					}
				}
				InternetCloseHandle(tempHandle);
				sess->reconnectWindow = 0; /* reconnect window is done */
			}
			/* Ignore closed channel here - we should catch it somewhere else */
			else
			{
				/* change this error text if we are not in a reconnect process */
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);

				if (sess->reconnectWindow)
					snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcWinInetRead: Received final chunk but no new streaming channel to complete reconnection process. ", __FILE__, __LINE__);
				else
					snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcWinInetRead: Received final chunk, connection closed by upstream device. ", __FILE__, __LINE__);

				/* unset the callback */
				ripcWinInetUnregisterCallbacks(sess);
				sess->state = WININET_ERROR;
				RSSL_MUTEX_UNLOCK(&sess->tunnelMutex);
				return RSSL_RET_FAILURE;
			}
		}
		break;

		/* should call IQDA from here */
		case WININET_STREAMING_CONN_CALL_IQDA:
		{
			if (!InternetQueryDataAvailable(sess->newStreamingReqHandle,&sess->reconnectBytesToRead, 0, 0))
			{
				_error = GetLastError();
				if (_error != ERROR_IO_PENDING)
				{
					_rsslSetError(error, NULL, RSSL_RET_FAILURE, _error);
					snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcWinInetRead error with reconnection ", __FILE__, __LINE__);
					sess->state = WININET_ERROR;
					ripcWinInetErrors(error, strlen(error->text));
					ripcWinInetUnregisterCallbacks(sess);
					RSSL_MUTEX_UNLOCK(&sess->tunnelMutex);
					return RSSL_RET_FAILURE;
				}
				else
				{
#ifdef DBUG_RECONNECT
					printf("In read, state STREAMING_CONN_CALL_IQDA - IQDA returns ERROR_IO_PENDING - switching to STREAMING_CONN_WAIT_ACK\n");
#endif
					/* wait for a callback on this */
					sess->reconnectState = WININET_STREAMING_CONN_WAIT_ACK;
					break;  /* break out and do the rest of read */
				}
			}
			else
			{
				if (sess->reconnectBytesToRead)
				{
					/* Unlikely but this may return right away but just in case.... */
					sess->reconnectState = WININET_STREAMING_CONN_ACK_RECEIVED;
					/* we should fall through here */
				}
#ifdef DBUG_RECONNECT
				printf("STREAMING_CONN_CALL_IQDA returns %d - switching to STREAMING_CONN_ACK_RECEIVED and falling through\n", _error);
#endif
			}
		}
		/* no break intentionally */
		case WININET_STREAMING_CONN_ACK_RECEIVED:  /* open control channel */
		{
			char tempInBuf[5];
			DWORD inSize = 0;
								
			/* both acknowledgment messages are 1 byte */
			if (sess->reconnectBytesToRead != 1)
			{
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcWinInetRead invalid size for new streaming connection ack (%d) ", __FILE__, __LINE__, sess->reconnectBytesToRead);
				sess->state = WININET_ERROR;
				/* unset the callback */
				ripcWinInetUnregisterCallbacks(sess);
				RSSL_MUTEX_UNLOCK(&sess->tunnelMutex);
				return RSSL_RET_FAILURE;
			}
			InternetReadFile(sess->newStreamingReqHandle, tempInBuf, sess->reconnectBytesToRead, &inSize);

			if (inSize != sess->reconnectBytesToRead)
			{
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcWinInetRead invalid size for new streaming connection ack (%d) ", __FILE__, __LINE__, sess->reconnectBytesToRead);
				sess->state = WININET_ERROR;
				/* unset the callback */
				ripcWinInetUnregisterCallbacks(sess);
				RSSL_MUTEX_UNLOCK(&sess->tunnelMutex);
				return RSSL_RET_FAILURE;
			}
			else
				sess->reconnectBytesToRead -= inSize;

#ifdef DBUG_RECONNECT
			printf("read state STREAMING_CONN_ACK_RECEIVED read the ack\n");
#endif
			
			/* dont call IQDA until after this stuff */						
			
			/* now set up the new control channel */
			sess->newControlReqHandle = HttpOpenRequest(sess->connectHandle, "POST", sess->config.objectName, NULL, NULL, NULL, sess->config.connectionFlags, (DWORD_PTR)sess);
					
			if (!sess->newControlReqHandle) 
			{
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcWinInetRead unable to open new control channel ", __FILE__, __LINE__);
				sess->state = WININET_ERROR;
				ripcWinInetErrors(error, strlen(error->text));
				/* unset the callback */
				ripcWinInetUnregisterCallbacks(sess);
				RSSL_MUTEX_UNLOCK(&sess->tunnelMutex);
				return RSSL_RET_FAILURE;
			}
							
			if (!(HttpSendRequestEx(sess->newControlReqHandle, &sess->iBuffer, NULL, (((sess->config.connectionFlags & SECURITY_FLAG_IGNORE_REVOCATION) == 0) ? 0 : SECURITY_FLAG_IGNORE_REVOCATION), (DWORD_PTR)sess)))
			{
				_error = GetLastError();
				if (_error != ERROR_IO_PENDING)
				{
					_rsslSetError(error, NULL, RSSL_RET_FAILURE, _error);
					snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcWinInetRead error sending new control channel HTTP header ", __FILE__, __LINE__);
					sess->state = WININET_ERROR;
					ripcWinInetErrors(error, strlen(error->text));
					/* unset the callback */
					ripcWinInetUnregisterCallbacks(sess);
					RSSL_MUTEX_UNLOCK(&sess->tunnelMutex);
					return RSSL_RET_FAILURE;
				}
				else
				{
#ifdef DBUG_RECONNECT
					printf("HttpSendRequestEx returns ERROR_IO_PENDING - switching to CONTROL_CONN_REINITIALIZING\n");
#endif
					/* wait for callback and come in here to send the request */
					sess->reconnectState = WININET_CONTROL_CONN_REINITIALIZING;
					break;  /* break out and do more read stuff */
				}			
			}
			else
			{
				/* Send request returned synchronously
				 * We'll fall through to the next case and send the control request msg.
				 */
#ifdef DBUG_RECONNECT
				printf("HttpSendRequestEx is done already - switch to CONTROL_CONN_SEND_POST and fall through\n");
#endif
				sess->reconnectState = WININET_CONTROL_CONN_SEND_POST;
			}
		}
		/* no break on purpose */

		case WININET_CONTROL_CONN_SEND_POST:  /* send control req */
		{
			char outBuf[20];
			DWORD outLen = 0;
			
			/* put together the control reconnection request */
			outLen += 2;
			outBuf[2] = (RIPC_TUNNEL_CONTROL | RIPC_WININET_TUNNELING | RIPC_TUNNEL_RECONNECT);
			++outLen;
			_move_u32_swap((outBuf + outLen), &sess->config.id);
			outLen += 4;
			_move_u16_swap((outBuf + outLen), &sess->config.pID);
			outLen += 2;
			_move_u32_swap((outBuf + outLen), &sess->config.address);
			outLen += 4;
			/* now the length */
			_move_u16_swap(outBuf, &outLen);

			/* write control reconnect request and make sure it was all sent */
			if (!(InternetWriteFile(sess->newControlReqHandle, outBuf, outLen, &sess->outBytesForRead)))
			{
				errorCode = GetLastError();
				if (errorCode != ERROR_IO_PENDING)
				{
					_rsslSetError(error, NULL, RSSL_RET_FAILURE, errorCode);
					snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcWinInetRead: unable to send new control channel request ", __FILE__, __LINE__);
					sess->state = WININET_ERROR;
					ripcWinInetErrors(error, strlen(error->text));
					/* unset the callback */
					ripcWinInetUnregisterCallbacks(sess);
					RSSL_MUTEX_UNLOCK(&sess->tunnelMutex);
					return RSSL_RET_FAILURE;
				}
			}
			if (sess->outBytesForRead != outLen)
			{
				errorCode = GetLastError();
				if (errorCode != ERROR_IO_PENDING)
				{
					_rsslSetError(error, NULL, RSSL_RET_FAILURE, errorCode);
					snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcWinInetRead: Unable to write entire control channel reconnect message (%d). ", __FILE__, __LINE__, sess->outBytesForRead);
					sess->state = WININET_ERROR;
					/* unset the callback */
					ripcWinInetUnregisterCallbacks(sess);
					RSSL_MUTEX_UNLOCK(&sess->tunnelMutex);
					return RSSL_RET_FAILURE;
				}
				/*  Partial write of control channel reconnect message.  We may need to handle this differently */
			}

#ifdef DBUG_RECONNECT
			printf("In CONTROL_CONN_SEND_POST - sent message\n");
#endif

			/* now call IQDA to get callbacks going again */
			if (!InternetQueryDataAvailable(sess->newStreamingReqHandle,&sess->reconnectBytesToRead, 0, 0))
			{
				_error = GetLastError();
				if (_error != ERROR_IO_PENDING)
				{
					_rsslSetError(error, NULL, RSSL_RET_FAILURE, _error);
					snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcWinInetRead: Error on IQDA. ", __FILE__, __LINE__);
					ripcWinInetErrors(error, strlen(error->text));
					sess->state = WININET_ERROR;
					/* unset the callback */
					ripcWinInetUnregisterCallbacks(sess);
					RSSL_MUTEX_UNLOCK(&sess->tunnelMutex);
					return RSSL_RET_FAILURE;
				}
				else
				{
					/* break out and do other read stuff */
#ifdef DBUG_RECONNECT
					printf("IQDA from CONTROL_CONN_SEND_POST returned ERROR_IO_PENDING - go to CONTROL_CONN_WAIT_ACK\n");
#endif
					sess->reconnectState = WININET_CONTROL_CONN_WAIT_ACK;
					break;
				}
			}
			else
			{
				/* fall through and read the ack */
				sess->reconnectState = WININET_CONTROL_CONN_ACK_RECEIVED;
#ifdef DBUG_RECONNECT
				printf("IQDA from CONTROL_CONN_SEND_POST returned %d - go to CONTROL_CONN_ACK_RECEIVED and fall through\n", _error);
#endif
			}
		}
		/* no break intentionally */

		case WININET_CONTROL_CONN_ACK_RECEIVED:  /* switch file descriptors */
		{
			char tempInBuf[5];
			DWORD inSize = 0;
	
			/* both acknowledgment messages are 1 byte */
			if (sess->reconnectBytesToRead != 1)
			{
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcWinInetRead invalid size for new control connection ack (%d) ", __FILE__, __LINE__, sess->reconnectBytesToRead);
				sess->state = WININET_ERROR;
				/* unset the callback */
				ripcWinInetUnregisterCallbacks(sess);
				RSSL_MUTEX_UNLOCK(&sess->tunnelMutex);
				return RSSL_RET_FAILURE;
			}
			InternetReadFile(sess->newStreamingReqHandle, tempInBuf, sess->reconnectBytesToRead, &inSize);

			if (inSize != sess->reconnectBytesToRead)
			{
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcWinInetRead invalid size for new control connection ack (%d) ", __FILE__, __LINE__, sess->reconnectBytesToRead);
				sess->state = WININET_ERROR;
				/* unset the callback */
				ripcWinInetUnregisterCallbacks(sess);
				RSSL_MUTEX_UNLOCK(&sess->tunnelMutex);
				return RSSL_RET_FAILURE;
			}
			else
				sess->reconnectBytesToRead -= inSize;

#ifdef DBUG_RECONNECT 
			printf("In CONTROL_CONN_ACK_RECEIVED and read the ack\n");
#endif
			/* Dont call IQDA here - wait until we switch to use this as the 
			 * official streamingChannel and then call it
			 */

			/* end the request on the old control channel */
			if (!(HttpEndRequest(sess->controlReqHandle, NULL, 0, 0)))
			{
				errorCode = GetLastError();
				if (errorCode != ERROR_IO_PENDING)
				{
					_rsslSetError(error, NULL, RSSL_RET_FAILURE, errorCode);
					snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcWinInetRead: Unable to end control channel request. ", __FILE__, __LINE__);
					sess->state = WININET_ERROR;
					ripcWinInetErrors(error, strlen(error->text));
					/* unset the callback */
					ripcWinInetUnregisterCallbacks(sess);
					RSSL_MUTEX_UNLOCK(&sess->tunnelMutex);
					return RSSL_RET_FAILURE;
				}	
			}

#ifdef DBUG_RECONNECT
			printf("switching from control handle %d to control handle %d\n", sess->controlReqHandle, sess->newControlReqHandle);
#endif

			/* this should complete the reconnection process - we should
			   be getting the final chunk some time soon */
			/* close the old handle and flip the handles */
			InternetCloseHandle(sess->controlReqHandle);
			sess->controlReqHandle = sess->newControlReqHandle;
			sess->newControlReqHandle = 0;

			/* since we have reconnected and started a new control channel, reset the totalBytesWritten */
			sess->totalBytesWritten = 0;

			/* Just in case we had a pending callback on the old handle, we don't care anymore */
			sess->writeCallbackPending = 0;
			sess->writeBytesPending = 0;

			/* Now wait for the final chunk */
			sess->reconnectState = WININET_WAIT_FINAL_CHUNK;

			/* Now we just wait for the final chunk */
		}
		break;
	}
	
	if (!sess->reconnectWindow)
		RSSL_MUTEX_UNLOCK(&sess->tunnelMutex);
	else
		stayLocked = 1;  /* if we are in the reconnect process, lock the entire read */
	/* This can probably be optimized out but we saw strange behavior when this was not 
	 * synchronized with the callback.
	 */
		
	
	/* Dont think we actually need to lock this -
     * callbackPending is only decremented from 
     * the callback for active channels
	 */
	while (!sess->callbackPending && totalBytes < max_len)
	{
		inBuf = buf + totalBytes;
		inLen = max_len - totalBytes;
	
		if (sess->bytesToRead)
		{
			if (sess->bytesToRead < (RsslUInt32)inLen)
				retval = InternetReadFile(sess->streamingReqHandle, inBuf, sess->bytesToRead, &sess->outBytesForRead);
			else
				retval = InternetReadFile(sess->streamingReqHandle, inBuf, inLen, &sess->outBytesForRead);
		
			totalBytes += sess->outBytesForRead;
			sess->bytesToRead -= sess->outBytesForRead;
				
			/* Check return code from InternetReadFile */
			if (!retval)
			{
				errorCode = GetLastError();
				if (errorCode != ERROR_IO_PENDING)
				{
					_rsslSetError(error, NULL, RSSL_RET_FAILURE, errorCode);
					snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcWinInetRead: Error reading from stream. ", __FILE__, __LINE__);
					ripcWinInetErrors(error, strlen(error->text));
					/* unset the callback */
					ripcWinInetUnregisterCallbacks(sess);
					sess->state = WININET_ERROR;
					if (stayLocked)
						RSSL_MUTEX_UNLOCK(&sess->tunnelMutex);
					return RSSL_RET_FAILURE;
				}
				else
				{
					/* return error here too */
					_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
					snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcWinInetRead: Error reading - attempting to call back with read buffer. ", __FILE__, __LINE__);
					ripcWinInetErrors(error, strlen(error->text));

					ripcWinInetUnregisterCallbacks(sess);
					sess->state = WININET_ERROR;
					if (stayLocked)
						RSSL_MUTEX_UNLOCK(&sess->tunnelMutex);
					return RSSL_RET_FAILURE;
				}
			}
		}
		else
		{
			/* See if there are more bytes available synchronously
			 * otherwise setup for another asynch callback
			 */
#ifdef DBUG_RECONNECT
			printf("Calling IQDA from read\n");
#endif
			/* increment callback count before calling IQDA -
			   we should not get a callback until after (or during) 
			   IQDA is called */
			/* pre-increment this */
			++sess->callbackPending;
			if (InternetQueryDataAvailable(sess->streamingReqHandle,&sess->bytesToRead , 0, 0))
			{
				if (sess->bytesToRead)
				{
					/* dont expect callback in this case */
					--sess->callbackPending;
				}	
				else
				{
					if (!stayLocked)  /* we werent locked */
						RSSL_MUTEX_LOCK(&sess->tunnelMutex);
					sess->reconnectState = WININET_FINAL_CHUNK_RECEIVED;
					/* 0 bytes read */
					/* this is the end of a the chunk  */
					error->text[0] = '\0';

					/* if we have a new streaming handle, switch it and loop around again */
					/* if we dont have a new handle there is likely some problem */
					if (sess->newStreamingReqHandle)
					{
						HINTERNET tempHandle = 0;
#ifdef DBUG_RECONNECT
						printf("In read, got final chunk and switching to new streaming handle and ending reconnect window\n");
#endif
						InternetSetStatusCallback(sess->streamingReqHandle, dummyWinInetCallback);
						tempHandle = sess->streamingReqHandle;					
						sess->streamingReqHandle = sess->newStreamingReqHandle;
						sess->newStreamingReqHandle = 0;
						/* reconnect window is over now so we can stop locking in write */
						
						sess->reconnectState = WININET_INIT_OK;

						InternetCloseHandle(tempHandle);
						sess->reconnectWindow = 0; /* reconnect window is done */

						/* Call IQDA now - this is the new streaming handle */
						if (InternetQueryDataAvailable(sess->streamingReqHandle,&sess->bytesToRead , 0, 0))
						{
							/* dont expect callback in this case */
							--sess->callbackPending;

							if (sess->bytesToRead == 0)
							{
								/* 0 bytes read */
								/* this is the end of a the chunk  */
								error->text[0] = '\0';
	
								if (totalBytes)
								{
									InternetCloseHandle(tempHandle);
									sess->reconnectWindow = 0; /* reconnect window is done */
									RSSL_MUTEX_UNLOCK(&sess->tunnelMutex);  /* always unlock here */
									return totalBytes;
								}
								else
								{
									/* change this error text if we are not in a reconnect process */
									_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
									if (sess->reconnectWindow)
										snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcWinInetRead: Received final chunk but no new streaming channel to complete reconnection process. ", __FILE__, __LINE__);
									else
										snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcWinInetRead: Received final chunk, connection closed by upstream device. ", __FILE__, __LINE__);

									/* unset the callback */
									ripcWinInetUnregisterCallbacks(sess);
									InternetCloseHandle(tempHandle);
									sess->state = WININET_ERROR;
									RSSL_MUTEX_UNLOCK(&sess->tunnelMutex);  /* always unlock here */
									return RSSL_RET_FAILURE;
								}
							}
						}							
						else
						{
							_error = GetLastError();
			
							if (_error != ERROR_IO_PENDING) 
							{
								/* We should not get error 12017 here - we only need to handle that in the callback */
								_rsslSetError(error, NULL, RSSL_RET_FAILURE, _error);
								snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcWinInetRead IQDA returns error (%d). ", __FILE__, __LINE__, _error);
								ripcWinInetErrors(error, strlen(error->text));
								/* unset the callback */
								/* no need to decrement count since we unregister callbacks */
								ripcWinInetUnregisterCallbacks(sess);
								InternetCloseHandle(tempHandle);
								sess->state = WININET_ERROR;
								RSSL_MUTEX_UNLOCK(&sess->tunnelMutex);  /* always unlock here */
								return RSSL_RET_FAILURE;
							}
							else
							{
								/* already incremented above */
								/* since we return, no issue with looping around */
#ifdef DBUG_CBACKCOUNT
								printf("<%s:%d> Callback Count: %d\n", __FILE__, __LINE__, sess->callbackPending);
#endif
								InternetCloseHandle(tempHandle);
								sess->reconnectWindow = 0; /* reconnect window is done */
								RSSL_MUTEX_UNLOCK(&sess->tunnelMutex);  /* always unlock here */
								/* There is no data to read so just return and wait for the callback */
								return totalBytes;
							}
						}

						InternetCloseHandle(tempHandle);
						sess->reconnectWindow = 0; /* reconnect window is done */
						
						if (!stayLocked)
							RSSL_MUTEX_UNLOCK(&sess->tunnelMutex);
					}
					else
					{
						/* This session has been closed unexpectedly */
						/* Set the session in an error state, */
						/*  if we have data return it to the user and wait until next call to read/write to return error */
						/* unset the callback */
						/* no need to decrement count since we unregister callbacks */
						ripcWinInetUnregisterCallbacks(sess);
						
						if (totalBytes)
						{
							/* Return the bytes we have and signal the user to call read again so we can return the error */
							if (InterlockedCompareExchange (&sess->byteWritten, 1L, 0L) == 0)
							{
			                	if (rssl_pipe_write(&sess->_pipe, "1", 1) != 1)
								{
									/* if we cant write to this its bad */
									sess->byteWritten = 0;
									_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
									snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcWinInetRead: Unable to write byte to pipe. ", __FILE__, __LINE__);
									sess->state = WININET_ERROR;
									/* unset the callback */
									RSSL_MUTEX_UNLOCK(&sess->tunnelMutex);  /* always unlock */
									return RSSL_RET_FAILURE;
								}
							}
							/* change this error text if we are not in a reconnect process */
							_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
							if (sess->reconnectWindow)
								snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcWinInetRead: Received final chunk but no new streaming channel to complete reconnection process. ", __FILE__, __LINE__);
							else
								snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcWinInetRead: Received final chunk, connection closed by upstream device. ", __FILE__, __LINE__);

							sess->state = WININET_ERROR;
							RSSL_MUTEX_UNLOCK(&sess->tunnelMutex);  /* always unlock */
							return totalBytes;
						}
						else
						{
							/* change this error text if we are not in a reconnect process */
							_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
							if (sess->reconnectWindow)
								snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcWinInetRead: Received final chunk but no new streaming channel to complete reconnection process. ", __FILE__, __LINE__);
							else
								snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcWinInetRead: Received final chunk, connection closed by upstream device. ", __FILE__, __LINE__);

							sess->state = WININET_ERROR;
							RSSL_MUTEX_UNLOCK(&sess->tunnelMutex); /* always unlock */
							return RSSL_RET_FAILURE;
						}
					}
				}
			}
			else
			{
				_error = GetLastError();
			
				if ((_error != ERROR_IO_PENDING) && (_error != ERROR_INTERNET_OPERATION_CANCELLED))
				{
					/* We should not get error 12017 here - we only need to handle that in the callback */
					_rsslSetError(error, NULL, RSSL_RET_FAILURE, _error);
					snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcWinInetRead IQDA returns error (%d). ", __FILE__, __LINE__, _error);
					ripcWinInetErrors(error, strlen(error->text));
					/* unset the callback */
					/* no need to decrement count since we unregister callbacks */
					ripcWinInetUnregisterCallbacks(sess);
					sess->state = WININET_ERROR;
					if (stayLocked)
						RSSL_MUTEX_UNLOCK(&sess->tunnelMutex);
					return RSSL_RET_FAILURE;
				}
				else
				{
#ifdef  DBUG_CBACKCOUNT
					if (_error == ERROR_INTERNET_OPERATION_CANCELLED)
						printf("<%s:%d> Got operation cancelled - this may throw off callback counts\n");
#endif
					if (!stayLocked)  /* if we arent already locked */
						RSSL_MUTEX_LOCK(&sess->tunnelMutex);
#ifdef DBUG_CBACKCOUNT
					printf("<%s:%d> Callback Count: %d\n", __FILE__, __LINE__, sess->callbackPending);
#endif
					
					RSSL_MUTEX_UNLOCK(&sess->tunnelMutex);  /* always unlock here */
					/* we have to wait for IQDA to call us back 
  					 * We will return after the while loop ends
					 * which should happen now because callbackPending was just incremented
			 		 */
					/* because of what IQDA returned, we should not have any bytes left to read
					   and we know we are waiting on a callback - we should return here */
					return totalBytes;
				}
			}
		}
	}

	/* If we still have bytes we need to read after breaking out of the while, or we are not
     * waiting on a callback we should trigger the user with the pipe
	 */
	if ( sess->bytesToRead || !sess->callbackPending )
	{
		/* this is essentially the more data case without the app knowing */
		if (!stayLocked)  /* if we arent already locked */
			RSSL_MUTEX_LOCK(&sess->tunnelMutex);

		if (InterlockedCompareExchange (&sess->byteWritten, 1L, 0L) == 0)
		{
           	if (rssl_pipe_write(&sess->_pipe, "1", 1) != 1)
			{
				/* if we cant write to this its bad */
				sess->byteWritten = 0;
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcWinInetRead: Unable to write byte to pipe. ", __FILE__, __LINE__);
				sess->state = WININET_ERROR;
				/* unset the callback */
				ripcWinInetUnregisterCallbacks(sess);
				RSSL_MUTEX_UNLOCK(&sess->tunnelMutex);  /* always unlock */
				return RSSL_RET_FAILURE;
			}
		}
		if (!stayLocked)
			RSSL_MUTEX_UNLOCK(&sess->tunnelMutex);
	}

	if (stayLocked)
		RSSL_MUTEX_UNLOCK(&sess->tunnelMutex);
	return totalBytes;
}

/* our transport write function -
   this will write to the network using SSL and return the appropriate value to the ripc layer */
RsslRet ripcWinInetWrite( void *winInetSess, char *buf, RsslInt32 len, ripcRWFlags flags, RsslError *error)
{
	DWORD outLen;
	char* outBuf;
	RsslInt32 errorCode = 0;
	RsslInt32 totalOut = 0;
	ripcWinInetSession *sess = (ripcWinInetSession*)winInetSess;
	sess->outBytesForWrite = 0;

	if (sess->state == WININET_ERROR)
	{
		/* An Error happened - 
		 * Return the text and code and fail */

		/* Move the errorText from the session into the error structure */
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "%s", sess->errorText);
		ripcWinInetErrors(error, strlen(error->text));
		/* unset the callback */
		ripcWinInetUnregisterCallbacks(sess);
		return RSSL_RET_FAILURE;
	}

	/* We only care about this in the non-SSL case - is this true??? */
	if (!(sess->config.connectionFlags & INTERNET_FLAG_SECURE))
	{
		/* We use the 600 as a fudge factor to include the HTTP headers on the initial connection 
		 * as well as all of our initial connection messages
		 */
		if ((sess->totalBytesWritten + len) > (MAX_OUT_BYTES - 600))
		{
			/* this message will cause us to write too much */ 
			/* Reconnect at this point to reset the counts */
			ripcWinInetReconnection(sess, error);
		}
	}

	/* Make sure we're not waiting for a write callback */
	if (sess->writeCallbackPending == 1)
	{
		return RSSL_RET_SUCCESS;
	}
	/*
	 * if the last write was a partial write, then we need to make adjustments
	 * if InternetWriteFile() cannot write everything we ask it to, it will return to us and write
	 * the remaining bytes asynchronously when it can. We track the amount it didnt write (and is still
	 * going to write) in writeBytesPending. This gets tricky because we cannot attempt another write until
	 * wininet has written all the bytes we asked it to write last time. We use writeCallbackPending for that.
	 */
	outBuf = buf;
	outLen = len;
	sess->writeBytesPending = 0;

	/* check to see if there was nothing new to write this time */
	/* InternetWriteFile() doesnt like being called with zero bytes to write */
	if (outLen == 0)
		return totalOut;

	if (sess->reconnectWindow)
	{
		/* this flag gets set at the start of the reconnection period */
		/* this should provide ample time for it to be set before we are changing the controlReqHandle */
		RSSL_MUTEX_LOCK(&sess->tunnelMutex);
		/* increment callback here.  The callback wont come until InternetWriteFile() is called */
		/* we might get the writeCallBack before InternetWriteFile() returns */
		sess->writeCallbackPending = 1;
		if (!(InternetWriteFile(sess->controlReqHandle, outBuf, outLen, &sess->outBytesForWrite)))
		{
			errorCode = GetLastError();
			if (errorCode != ERROR_IO_PENDING)
			{
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, errorCode);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcWinInetWrite InternetWriteFile failure (%d). ", __FILE__, __LINE__, errorCode);
				sess->state = WININET_ERROR;
				ripcWinInetErrors(error, strlen(error->text));
				/* unset the callback */
				/* no need to decrement writeCallbackPending here since we unregister callbacks */
				ripcWinInetUnregisterCallbacks(sess);
				RSSL_MUTEX_UNLOCK(&sess->tunnelMutex);
				return RSSL_RET_FAILURE;
			}
			else
			{
				/* sess->outBytesForWrite contains the number of bytes we actually wrote so far in this partial write */
				/* wininet remembers how much it didnt write and will write it when it can */
				sess->writeBytesPending = outLen - sess->outBytesForWrite;	/* save how much wininet will write asynchrously in the background */
				error->text[0] = '\0';
			}
		}
		else
		{
			/* callback is not coming because we wrote everything we were supposed to write */	

			if (outLen == sess->outBytesForWrite)
				sess->writeCallbackPending = 0;
			else
			{			
				sess->writeBytesPending = outLen - sess->outBytesForWrite;	/* save how much wininet will write asynchrously in the background */
				error->text[0] = '\0';
			}
		}
		RSSL_MUTEX_UNLOCK(&sess->tunnelMutex);
	}
	else
	{
		/* increment here - callback cant come until we call InternetWriteFile */
		sess->writeCallbackPending = 1;
		if (!(InternetWriteFile(sess->controlReqHandle, outBuf, outLen, &sess->outBytesForWrite)))
		{
			errorCode = GetLastError();
			if (errorCode != ERROR_IO_PENDING)
			{
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, errorCode);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcWinInetWrite InternetWriteFile failure (%d). ", __FILE__, __LINE__, errorCode);
				sess->state = WININET_ERROR;
				ripcWinInetErrors(error, strlen(error->text));
				/* unset the callback */
				/* no need to decrement writeCallbackPending since we unregister callbacks */
				ripcWinInetUnregisterCallbacks(sess);
				return RSSL_RET_FAILURE;
			}
			else
			{
				/* sess->outBytesForWrite contains the number of bytes we actually wrote so far in this partial write */
				/* wininet remembers how much it didnt write and will write it when it can */
				sess->writeBytesPending = outLen - sess->outBytesForWrite;	/* save how much wininet will write asynchrously in the background */
				error->text[0] = '\0';
			}
		}
		else
		{
			/* callback is not coming because we wrote everything we were supposed to write */	
			if (outLen == sess->outBytesForWrite)
				sess->writeCallbackPending = 0;
			else
			{
				sess->writeBytesPending = outLen - sess->outBytesForWrite;	/* save how much wininet will write asynchrously in the background */
				error->text[0] = '\0';
			}
		}
	}

	/* keep track of totalBytesWritten so we do not go over the Content-Length of our controlChannel request
	 * The periodic reconnection should cause a refresh of this to occur periodically, 
	 * and it should occur prior to us hitting the 0x7FFFFFFF length
	 */
	sess->totalBytesWritten += outLen;
	totalOut += outLen;

	return totalOut;
}

RsslRet ripcWinInetIoctl(void *session, RsslInt32 code, RsslInt32 value, RsslError *error)
{
	ripcWinInetSession *sess = (ripcWinInetSession*)session;

	if (sess)
	{
		switch (code)
		{
			case RIPC_IGNORE_CERT_REVOCATION:
				if (value != 0)
					sess->config.connectionFlags |= SECURITY_FLAG_IGNORE_REVOCATION;
				else
					sess->config.connectionFlags &= ~SECURITY_FLAG_IGNORE_REVOCATION;
				return 1;
			break;
			default:
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
				snprintf(error->text,  MAX_RSSL_ERROR_TEXT,
					"<%s:%d> Error: 1004 ripcWinInetIoctl() invalid Ioctl code of <%d>.\n", __FILE__, __LINE__, code);
				return RSSL_RET_FAILURE;
		}
	}
	_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
	snprintf(error->text, MAX_RSSL_ERROR_TEXT, 
		"<%s:%d> Error: 1004 ripcWinInetIoctl() invalid use of Ioctl.  Internal session is not available.\n", __FILE__, __LINE__);

	return RSSL_RET_FAILURE;
}

RsslRet ripcShutdownWinInetSocket(void *session)
{
	RsslError error;
	ripcWinInetSession *sess = (ripcWinInetSession*)session;

	/* do winInet channel shutdown here */
	ripcWinInetUnregisterCallbacks(sess);

	ripcReleaseWinInetSession(sess, &error);

	return 1;
}

RsslRet ripcWinInetInitConnection(void *session, ripcSessInProg *inPr, RsslError *error)
{
	ripcWinInetSession *sess = (ripcWinInetSession*)session;
	char tempBuf[6144];
	DWORD _error;
	DWORD outLen = 0;
	char reqFlags = RIPC_TUNNEL_CONTROL | RIPC_WININET_TUNNELING;
	RsslInt16 hdrLen;
	sess->outBytesForRead = 0; /* We could use the read or write version of this for initialization, we chose read */

	/* set internal state before to cover error returns */
	/* put this into the second byte; standard handshake gets first byte; top two are unused for now */
	inPr->intConnState = (sess->state << 8);  
	/* Make sure to reset this if we change state before returning */

	/* we should keep this locked all the time in here since the state can change in a callback */
	RSSL_MUTEX_LOCK(&sess->tunnelMutex);

	switch (sess->state)
	{
		case WININET_INIT_STREAM_SENDRQST:
		{
#ifdef DBUG_INIT

			printf("InitConnection - WININET_INIT_STREAM_SENDRQST\n");
#endif
			/* We've sent the streaming request and now wating for the response
			 * Do nothing
			 */
			RSSL_MUTEX_UNLOCK(&sess->tunnelMutex);
			return RSSL_RET_SUCCESS;
		}
		case WININET_INIT_CONTROL_SENDRQST:
		{
#ifdef DBUG_INIT

			printf("InitConnection - WININET_INIT_CONTROL_SENDRQST\n");
#endif
			/* We've sent the streaming request and now wating for the response
			 * Do nothing
			 */
			RSSL_MUTEX_UNLOCK(&sess->tunnelMutex);
			return RSSL_RET_SUCCESS;
		}
		case WININET_INIT_CONTROL_SNDRQST_PARTIAL:
		{
#ifdef DBUG_INIT
			printf("InitConnection - WININET_INIT_CONTROL_SNDRQST_PARTIAL\n");
#endif
			/* We have partially sent the control request, waiting for callback to tell us its done */
			RSSL_MUTEX_UNLOCK(&sess->tunnelMutex);
			inPr->intConnState = (sess->state << 8);  
			return RSSL_RET_SUCCESS;
		}

		case WININET_INIT_STREAM_RESPONSE:
		{
#ifdef DBUG_INIT
			printf("InitConnection - WININET_INIT_STREAM_RESPONSE\n");
#endif
			/* Check to see if we've already received the response
			 * there is a possibility that it was returned synchronously in an earlier call to IQDA
			 * Otherwise see if we've received it yet, if not setup the acync callback.
			 */
			
			if (sess->bytesToRead == 0)
			{
				if (!InternetQueryDataAvailable(sess->streamingReqHandle, &sess->bytesToRead, 0, 0))		
				{
					_error = GetLastError();
					if (_error != ERROR_IO_PENDING)
					{	
						_rsslSetError(error, NULL, RSSL_RET_FAILURE, _error);
						snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcWinInetInitConnection - error calling IQDA ", __FILE__, __LINE__);
						sess->state = WININET_ERROR;
						inPr->intConnState = (sess->state << 8);  
						/* unset the callback */
						ripcWinInetUnregisterCallbacks(sess);
						RSSL_MUTEX_UNLOCK(&sess->tunnelMutex);
						return RSSL_RET_FAILURE;
					}
					else
					{
						/* need to wait for async callback before reading data */
						if (++sess->callbackPending > 1)
						{
							_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
							snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcWinInetInitConnection - unexpected number of pending callbacks ", __FILE__, __LINE__);
							sess->state = WININET_ERROR;
							inPr->intConnState = (sess->state << 8);  
							/* unset the callback */
							ripcWinInetUnregisterCallbacks(sess);
							RSSL_MUTEX_UNLOCK(&sess->tunnelMutex);
							return RSSL_RET_FAILURE;
						}
#ifdef DBUG_CBACKCOUNT
					printf("<%s:%d> Callback Count: %d\n", __FILE__, __LINE__, sess->callbackPending);
#endif
						if (InterlockedCompareExchange (&sess->byteWritten, 0L, 1L) == 1)
						{
							if (rssl_pipe_read(&sess->_pipe, tempBuf, 1) <= 0)
							{
								/* cant read from pipe is bad */
								_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
								snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcWinInetInitConnection error reading from pipe ", __FILE__, __LINE__);
								sess->state = WININET_ERROR;
								inPr->intConnState = (sess->state << 8);  
								/* unset the callback */
								ripcWinInetUnregisterCallbacks(sess);
								RSSL_MUTEX_UNLOCK(&sess->tunnelMutex);
								return RSSL_RET_FAILURE;
							}
						}
						RSSL_MUTEX_UNLOCK(&sess->tunnelMutex);
						return RSSL_RET_SUCCESS;
					}
				}
			}

			/* if we got this far then we have already received the response
			 * Change the state and fall through to the next case to read the response
			 */
			sess->state = WININET_INIT_STREAM_ACK_RECEIVED;
			inPr->intConnState = (sess->state << 8);  

#ifdef DBUG_INIT
			printf("\tFalling through to STREAM ACK RCVD: sess->bytesToRead = %d\n", sess->bytesToRead);
#endif

			/* Intentionally no break statement */
		}
		case WININET_INIT_STREAM_ACK_RECEIVED: 
		{
#ifdef DBUG_INIT
			printf("InitConnection - WININET_INIT_STREAM_ACK_RECEIVED\n");
#endif

			/* if bytesToRead is 0, this should be the signal that this is the last chunk */
			/* only go in here if we have the entire ack */
			if (sess->bytesToRead < 7)
			{
				/* We should have received this as a single response */
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcWinInetInitConnection invalid HTTP ack ", __FILE__, __LINE__);
				ripcWinInetErrors(error, strlen(error->text));
				sess->state = WININET_ERROR;
				inPr->intConnState = (sess->state << 8);  
				/* unset the callback */
				ripcWinInetUnregisterCallbacks(sess);
				RSSL_MUTEX_UNLOCK(&sess->tunnelMutex);
				return RSSL_RET_FAILURE;
			}
			
			/* we got our HTTP ack */
			if (sess->bytesToRead > 6144)
			{
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcWinInetInitConnection unable to read HTTP ack ", __FILE__, __LINE__);
				sess->state = WININET_ERROR;
				inPr->intConnState = (sess->state << 8);  
				/* unset the callback */
				ripcWinInetUnregisterCallbacks(sess);
				RSSL_MUTEX_UNLOCK(&sess->tunnelMutex);
				return RSSL_RET_FAILURE;
			}

			if ( !InternetReadFile(sess->streamingReqHandle, tempBuf, sess->bytesToRead, &sess->outBytesForRead) )
				memset(tempBuf, 0, sizeof(tempBuf));
			
			/* this will protect us from not getting the entire message, and also from any IRF errors */
			if (sess->outBytesForRead < 7)
			{
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcWinInetInitConnection unable to read HTTP ack ", __FILE__, __LINE__);
				ripcWinInetErrors(error, strlen(error->text));
				sess->state = WININET_ERROR;
				inPr->intConnState = (sess->state << 8);  
				/* unset the callback */
				ripcWinInetUnregisterCallbacks(sess);
				RSSL_MUTEX_UNLOCK(&sess->tunnelMutex);
				return RSSL_RET_FAILURE;
			}

			sess->bytesToRead -= sess->outBytesForRead;

			_move_u16_swap(&hdrLen, tempBuf);
			if (hdrLen == 7)
			{
				/* Send actual sessionID from the session */
				_move_u32_swap(&sess->config.id, tempBuf + 3);
			}
			else
			{
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcWinInetInitConnection invalid HTTP ack ", __FILE__, __LINE__);
				ripcWinInetErrors(error, strlen(error->text));
				sess->state = WININET_ERROR;
				inPr->intConnState = (sess->state << 8);  
				/* unset the callback */
				ripcWinInetUnregisterCallbacks(sess);
				RSSL_MUTEX_UNLOCK(&sess->tunnelMutex);
				return RSSL_RET_FAILURE;
			}

			sess->controlReqHandle = HttpOpenRequest(sess->connectHandle, "POST", sess->config.objectName, NULL, NULL, sess->acceptTypes, sess->config.connectionFlags, (DWORD_PTR)sess);
		
			if (!sess->controlReqHandle) 
			{
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcWinInetInitConnection error on HttpOpenRequest ", __FILE__, __LINE__);
				ripcWinInetErrors(error, strlen(error->text));
				sess->state = WININET_ERROR;
				inPr->intConnState = (sess->state << 8);  
				/* unset the callback */
				ripcWinInetUnregisterCallbacks(sess);
				RSSL_MUTEX_UNLOCK(&sess->tunnelMutex);
				return RSSL_RET_FAILURE;
			}
								
			if (!(HttpSendRequestEx(sess->controlReqHandle, &sess->iBuffer, NULL,(((sess->config.connectionFlags & SECURITY_FLAG_IGNORE_REVOCATION) == 0) ? 0 : SECURITY_FLAG_IGNORE_REVOCATION), (DWORD_PTR)sess)))
			{
				_error = GetLastError();
				if (_error != ERROR_IO_PENDING)
				{
					_rsslSetError(error, NULL, RSSL_RET_FAILURE, _error);
					snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcWinInetInitConnection error %d on HttpSendRequestEx ", __FILE__, __LINE__, _error);
					ripcWinInetErrors(error, strlen(error->text));
					sess->state = WININET_ERROR;
					inPr->intConnState = (sess->state << 8);  
					/* unset the callback */
					ripcWinInetUnregisterCallbacks(sess);
					RSSL_MUTEX_UNLOCK(&sess->tunnelMutex);
					return RSSL_RET_FAILURE;
				}
				else
				{
					/* Need to wait for HttpSendRequestEx to complete */
					if (++sess->callbackPending > 1)
					{
						_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
						snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcWinInetInitConnection - unexpected number of pending callbacks ", __FILE__, __LINE__);
						sess->state = WININET_ERROR;
						inPr->intConnState = (sess->state << 8);  
						/* unset the callback */
						ripcWinInetUnregisterCallbacks(sess);
						RSSL_MUTEX_UNLOCK(&sess->tunnelMutex);
						return RSSL_RET_FAILURE;
					}
#ifdef DBUG_CBACKCOUNT
					printf("<%s:%d> Callback Count: %d\n", __FILE__, __LINE__, sess->callbackPending);
#endif
					if (InterlockedCompareExchange (&sess->byteWritten, 0L, 1L) == 1)
					{
						if (rssl_pipe_read(&sess->_pipe, tempBuf, 1) <= 0)
						{
							/* cant read from pipe is bad */
							_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
							snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcWinInetInitConnection error reading from pipe ", __FILE__, __LINE__);
							sess->state = WININET_ERROR;
							inPr->intConnState = (sess->state << 8);  
							/* unset the callback */
							ripcWinInetUnregisterCallbacks(sess);
							RSSL_MUTEX_UNLOCK(&sess->tunnelMutex);
							return RSSL_RET_FAILURE;
						}
					}
					sess->state = WININET_INIT_CONTROL_SENDRQST;
					inPr->intConnState = (sess->state << 8);  
					RSSL_MUTEX_UNLOCK(&sess->tunnelMutex);
					return RSSL_RET_SUCCESS;
				}			
			}
			else
			{
				/* Send request returned synchronously
				 * We'll fall through to the next case and send the control request msg.
				 */
				sess->state = WININET_INIT_CONTROL_SNDRQST_COMPL;
				inPr->intConnState = (sess->state << 8);  
			}

			/* Intentionally no break statement */
#ifdef DBUG_INIT
			printf("\tFalling through to INIT CONTROL SNDRQST COMPL\n");
#endif
		}

        case WININET_INIT_CONTROL_SNDRQST_COMPL:
#ifdef DBUG_INIT
			printf("InitConnection - WININET_INIT_CONTROL_SNDRQST_COMPL\n");
#endif
			if (InterlockedCompareExchange (&sess->byteWritten, 0L, 1L) == 1)
			{
				if (rssl_pipe_read(&sess->_pipe, tempBuf, 1) <= 0)
				{
					/* cant read from pipe is bad */
					_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
					snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcWinInetInitConnection error reading from pipe ", __FILE__, __LINE__);
					sess->state = WININET_ERROR;
					inPr->intConnState = (sess->state << 8);  
					/* unset the callback */
					ripcWinInetUnregisterCallbacks(sess);
					RSSL_MUTEX_UNLOCK(&sess->tunnelMutex);
					return RSSL_RET_FAILURE;
				}
			}
			outLen += 2;
			sess->outPutBuffer[2] = reqFlags;
			++outLen;
			_move_u32_swap((sess->outPutBuffer + outLen), &sess->config.id);
			outLen += 4;
			_move_u16_swap((sess->outPutBuffer + outLen), &sess->config.pID);
			outLen += 2;
			_move_u32_swap((sess->outPutBuffer + outLen), &sess->config.address);
			outLen += 4;
			/* now the length */
			_move_u16_swap(sess->outPutBuffer, &outLen);

			if (!(InternetWriteFile(sess->controlReqHandle, sess->outPutBuffer, outLen, &sess->outBytesForRead)))
			{
				_error = GetLastError();
				if (_error != ERROR_IO_PENDING)
				{
					_rsslSetError(error, NULL, RSSL_RET_FAILURE, _error);
					snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcWinInetInitConnection unable to send control channel request ", __FILE__, __LINE__);
					ripcWinInetErrors(error, strlen(error->text));
					sess->state = WININET_ERROR;
					inPr->intConnState = (sess->state << 8);  
					/* unset the callback */
					ripcWinInetUnregisterCallbacks(sess);
					RSSL_MUTEX_UNLOCK(&sess->tunnelMutex);
					return RSSL_RET_FAILURE;
				}
			}
			
			if (sess->outBytesForRead != outLen)
			{
				_error = GetLastError();
				if (_error != ERROR_IO_PENDING)
				{
					_rsslSetError(error, NULL, RSSL_RET_FAILURE, _error);
					snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcWinInetInitConnection unable to send entire control channel request (%d) ", __FILE__, __LINE__, sess->outBytesForRead);
					ripcWinInetErrors(error, strlen(error->text));
					sess->state = WININET_ERROR;
					inPr->intConnState = (sess->state << 8);  
					/* unset the callback */
					ripcWinInetUnregisterCallbacks(sess);
					RSSL_MUTEX_UNLOCK(&sess->tunnelMutex);
					return RSSL_RET_FAILURE;
				}
				
				/* Need to wait for InternetWriteFile to complete */
				if (++sess->callbackPending > 1)
				{
					_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
					snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcWinInetInitConnection - unexpected number of pending callbacks ", __FILE__, __LINE__);
					sess->state = WININET_ERROR;
					inPr->intConnState = (sess->state << 8);  
					/* unset the callback */
					ripcWinInetUnregisterCallbacks(sess);
					RSSL_MUTEX_UNLOCK(&sess->tunnelMutex);
					return RSSL_RET_FAILURE;
				}
#ifdef DBUG_CBACKCOUNT
				printf("<%s:%d> Callback Count: %d\n", __FILE__, __LINE__, sess->callbackPending);
#endif
				if (InterlockedCompareExchange (&sess->byteWritten, 0L, 1L) == 1)
				{
					if (rssl_pipe_read(&sess->_pipe, tempBuf, 1) <= 0)
					{
						/* cant read from pipe is bad */
						_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
						snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcWinInetInitConnection error reading from pipe ", __FILE__, __LINE__);
						sess->state = WININET_ERROR;
						inPr->intConnState = (sess->state << 8);  
						/* unset the callback */
						ripcWinInetUnregisterCallbacks(sess);
						RSSL_MUTEX_UNLOCK(&sess->tunnelMutex);
						return RSSL_RET_FAILURE;
					}
				}
				sess->state = WININET_INIT_CONTROL_SNDRQST_PARTIAL;
				RSSL_MUTEX_UNLOCK(&sess->tunnelMutex);
				return RSSL_RET_SUCCESS;
			}
			else
			{
				/* Send request returned synchronously
				 * We'll fall through to the next case and send the control request msg.
				 */
				sess->state = WININET_INIT_CONTROL_SNDRQST_FULL_COMPL;
			}
			
		/* Intentionally falling through */

		case WININET_INIT_CONTROL_SNDRQST_FULL_COMPL:
#ifdef DBUG_INIT
			printf("InitConnection - WININET_INIT_CONTROL_SNDRQST_FULL_COMPL\n");
#endif
			/* now send ripc connection request */
			sess->state = WININET_ACTIVE;
			inPr->intConnState = (sess->state << 8);  
			/* Now that we're active setup a callback to receive data from the sreaming channel */
			if (InternetQueryDataAvailable(sess->streamingReqHandle,&sess->bytesToRead , 0, 0))
			{
				/* If we get anything other than ERROR_IO_PENDING here we may be in an error condition since we
				 * are not expecting any data over the channel yet.
				 */
				if (sess->bytesToRead)
				{
					if (InterlockedCompareExchange (&sess->byteWritten, 1L, 0L) == 0)
					{
	                	if (rssl_pipe_write(&sess->_pipe, "1", 1) != 1)
						{
							sess->byteWritten = 0;
							_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
							snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcWinInetInitConnection error writing the pipe ", __FILE__, __LINE__);
							sess->state = WININET_ERROR;
							inPr->intConnState = (sess->state << 8);  
							/* unset the callback */
							ripcWinInetUnregisterCallbacks(sess);
							RSSL_MUTEX_UNLOCK(&sess->tunnelMutex);
							return RSSL_RET_FAILURE;
						}
					}
				}
			}
			else
			{
				_error = GetLastError();
				if (_error != ERROR_IO_PENDING)
				{
					ripcWinInetErrors(error, _error);
					sess->state = WININET_ERROR;
					inPr->intConnState = (sess->state << 8);  
					/* unset the callback */
					ripcWinInetUnregisterCallbacks(sess);
					RSSL_MUTEX_UNLOCK(&sess->tunnelMutex);
					return RSSL_RET_FAILURE;
				}
				else
				{
					if (++sess->callbackPending > 1)
					{
						_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
						snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcWinInetInitConnection - unexpected number of pending callbacks ", __FILE__, __LINE__);
						sess->state = WININET_ERROR;
						inPr->intConnState = (sess->state << 8);  
						/* unset the callback */
						ripcWinInetUnregisterCallbacks(sess);
						RSSL_MUTEX_UNLOCK(&sess->tunnelMutex);
						return RSSL_RET_FAILURE;
					}
#ifdef DBUG_CBACKCOUNT
					printf("<%s:%d> Callback Count: %d\n", __FILE__, __LINE__, sess->callbackPending);
#endif
					if (InterlockedCompareExchange (&sess->byteWritten, 0L, 1L) == 1)
					{
						if (rssl_pipe_read(&sess->_pipe, tempBuf, 1) <= 0)
						{
							/* if we cant read from the pipe thats bad */
							_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
							snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcWinInetInitConnection unable to send control channel request ", __FILE__, __LINE__);
							sess->state = WININET_ERROR;
							inPr->intConnState = (sess->state << 8);  
							/* unset the callback */
							ripcWinInetUnregisterCallbacks(sess);
							RSSL_MUTEX_UNLOCK(&sess->tunnelMutex);
							return RSSL_RET_FAILURE;
						}
					}
				}
			}
			RSSL_MUTEX_UNLOCK(&sess->tunnelMutex);
			if (sess->outBytesForRead > 0)
				return sess->outBytesForRead;
			else
				return 1;
#ifdef DBUG_INIT
			printf("InitConnection - WININET_ACTIVE\n");
#endif
		break;

		case WININET_ERROR:
			/* this happened in the callback - transfer the error text */
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT, "%s", sess->errorText);
			ripcWinInetErrors(error, strlen(error->text));
			sess->state = WININET_ERROR;
			inPr->intConnState = (sess->state << 8);  
			/* unset the callback */
			ripcWinInetUnregisterCallbacks(sess);
			RSSL_MUTEX_UNLOCK(&sess->tunnelMutex);
			return RSSL_RET_FAILURE;

		default:
			error->text[0] = '\0';
			sess->state = WININET_ERROR;
			inPr->intConnState = (sess->state << 8);  
			/* unset the callback */
			ripcWinInetUnregisterCallbacks(sess);
			RSSL_MUTEX_UNLOCK(&sess->tunnelMutex);
			return RSSL_RET_FAILURE;
	}

	RSSL_MUTEX_UNLOCK(&sess->tunnelMutex);

	return RSSL_RET_SUCCESS;
}


/* this function is used in the reconnection cases, so we can condense it a bit */
RsslRet ripcWinInetReconnection(void *session, RsslError *error)
{
	RsslInt16 outLen = 0;
	DWORD _error = 0;
	RsslUInt32 respTimeout = 0x7FFFFFFF;
	char reqFlags = RIPC_TUNNEL_STREAMING | RIPC_WININET_TUNNELING | RIPC_TUNNEL_RECONNECT;
	ripcWinInetSession *sess = (ripcWinInetSession*)session;

	/* set this here - this should allow plenty of time to start locking on writes */

	RSSL_MUTEX_LOCK(&sess->tunnelMutex);
	sess->reconnectWindow = 1;
	RSSL_MUTEX_UNLOCK(&sess->tunnelMutex);

	/* Dont disable the callback if this fails - 
	   we are still getting data on the main connections */

	/* Set up new streaming request */
           
	sess->newStreamingReqHandle = HttpOpenRequest(sess->connectHandle, "POST", sess->config.objectName, NULL, NULL, NULL, sess->config.connectionFlags, (DWORD_PTR)sess);
	if (!sess->newStreamingReqHandle) 
	{
		/* couldnt register callback */
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, GetLastError());
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcWinInetReconnect error on HttpOpenRequest ", __FILE__, __LINE__);
		ripcWinInetErrors(error, strlen(error->text));
		return 0;
	}

#ifdef DBUG_RECONNECT
	printf("Reconnection beginning - created new streaming handle %d\n", sess->newStreamingReqHandle);
#endif

	/* encode our tunneling connection request */
	outLen += 2;
	sess->outPutBuffer[outLen++] = reqFlags;
	/* ID should be populated at this point */
	_move_u32_swap((sess->outPutBuffer + outLen), &sess->config.id);
	outLen += 4;
	_move_u16_swap((sess->outPutBuffer + outLen), &sess->config.pID);
	outLen += 2;
	_move_u32_swap((sess->outPutBuffer + outLen), &sess->config.address);
	outLen += 4;
		
	/* now the length */
	_move_u16_swap(sess->outPutBuffer, &outLen);
	
	if (!(HttpSendRequest(sess->newStreamingReqHandle, NULL, 0, sess->outPutBuffer, outLen)))
	{
		_error = GetLastError();
		if (_error != ERROR_IO_PENDING)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, _error);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcWinInetReconnect error on HttpSendRequest ", __FILE__, __LINE__);
			ripcWinInetErrors(error, strlen(error->text));
			return 0;
		}
		else
		{
			sess->reconnectState = WININET_STREAMING_CONN_REINITIALIZING;
		}
	}	
	else
	{
		if (InternetQueryDataAvailable(sess->streamingReqHandle,&sess->reconnectBytesToRead , 0, 0))
		{
			if (sess->reconnectBytesToRead)
			{
				/* Unlikely but this may return right away but just in case.... 
				 * No need to grab the lock yet
				 */
				sess->reconnectState = WININET_STREAMING_CONN_ACK_RECEIVED;
				
				RSSL_MUTEX_LOCK(&sess->tunnelMutex);
				if (InterlockedCompareExchange (&sess->byteWritten, 1L, 0L) == 0)
				{
                	if (rssl_pipe_write(&sess->_pipe, "1", 1) != 1)
					{
						sess->byteWritten = 0;
						_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
						snprintf(sess->errorText, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcWinInetReconnect error - unable to write byte to pipe ", __FILE__, __LINE__);
						/* unset the callback */
						RSSL_MUTEX_UNLOCK(&sess->tunnelMutex);
						return 0;	
					}
				}		
				RSSL_MUTEX_UNLOCK(&sess->tunnelMutex);
			}
		}
		else
		{
			_error = GetLastError();
			if (_error != ERROR_IO_PENDING)
			{	
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, _error);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcWinInetReconnect error on HttpSendRequest ", __FILE__, __LINE__);
				ripcWinInetErrors(error, strlen(error->text));
				return 0;
			}
			else
			{
				/* There should be nothing in the pipe yet, so we won't worry about reading from it.
				 */
				sess->reconnectState = WININET_STREAMING_CONN_WAIT_ACK;
			}
		}
	}

	return 1;
}


RsslSocket ripcWinInetCreatePipe(RsslInt32 *portnum, RsslSocketChannel *opts, RsslInt32 flags, void** userSpecPtr, RsslError *error)
{
	ripcWinInetSession *sess = 0;
	RsslSocket fd = 0;
	RsslInt32 length = 0;
	RsslInt32 hnIter = 0;
	RsslInt32 proxyPort = 0;
	char hostname[IPC_MAX_HOST_NAME];
	/* create the pipe and the session here - return the session as the userSpecPtr  which will then be passed into the ripcWinInetConnect function */

	sess = ripcInitializeWinInetSession(fd, 0, error);

	if (!(rssl_pipe_create(&sess->_pipe)))
	{
		/* do some kind of error here */
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcWinInetCreatePipe error on rtr_create_pipe ", __FILE__, __LINE__);
		return 0;
	}

	if (opts->serverName)
	{
		sess->config.portNum = atoi(opts->serverName);
		if (((sess->config.portNum) < 0) || (sess->config.portNum > 65535))
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,"<%s:%d> ripcWinInetCreatePipe: Invalid serverName/portNumber value",
		    __FILE__,__LINE__);
			ripcReleaseWinInetSession(sess, error);
			return 0;
		}
	}

	if (opts->proxyPort)
	{
		proxyPort = atoi(opts->proxyPort);
		if (((proxyPort) < 0) || (proxyPort > 65535))
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,"<%s:%d> ripcWinInetCreatePipe: Invalid proxyPort value",
		    __FILE__,__LINE__);
			ripcReleaseWinInetSession(sess, error);
			return 0;
		}
	}

	sess->config.connectionFlags = INTERNET_FLAG_IGNORE_CERT_CN_INVALID | 
				 INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTP | INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTPS |
				 INTERNET_FLAG_NO_CACHE_WRITE |INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_NO_UI |
				 INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_AUTO_REDIRECT;

	/* if the non-secure flag is set, we do not want to set the connect flag */
	if (!(flags & RIPC_INT_CS_FLAG_TUNNEL_NO_ENCRYPTION))
		sess->config.connectionFlags |= INTERNET_FLAG_SECURE;

	/* copy host name */
	if (opts->hostName)
		length = (RsslInt32)strlen(opts->hostName);

	if (length)
	{
		sess->config.hostName = (char*)_rsslMalloc(length + 9);
		if (!sess->config.hostName)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,"<%s:%d> ripcWinInetCreatePipe: Unable to create memmory for hostname storage",
		    __FILE__,__LINE__);
			ripcReleaseWinInetSession(sess, error);
			return 0;
		}
		memcpy((sess->config.hostName + hnIter), opts->hostName, length);
		hnIter += length;
		sess->config.hostName[hnIter] = '\0';
	}

	/* copy proxy host name */
	length = 0;
	if (opts->proxyHostName)
		length = (RsslInt32)strlen(opts->proxyHostName);
	hnIter = 0;
	if (length)
	{
		sess->config.proxyHostName = (char*)_rsslMalloc(length + 9);
		if (!sess->config.proxyHostName)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT,"<%s:%d> ripcWinInetCreatePipe: Unable to create memmory for proxyHostName storage",
		    __FILE__,__LINE__);
			ripcReleaseWinInetSession(sess, error);
			return 0;
		}
		memcpy((sess->config.proxyHostName + hnIter), opts->proxyHostName, length);
		hnIter += length;
		hnIter += snprintf((sess->config.proxyHostName + hnIter), 8, ":%d", proxyPort);
		sess->config.proxyHostName[hnIter] = '\0';
	}

	if (opts->objectName)
	{
		length = (RsslInt32)strlen(opts->objectName);

		if (length)
		{
			sess->config.objectName = (char*)_rsslMalloc(length + 1);
			if (!sess->config.objectName)
			{
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT,"<%s:%d> ripcWinInetCreatePipe: Unable to create memmory for objectName storage",
				__FILE__,__LINE__);
				ripcReleaseWinInetSession(sess, error);
				return 0;
			}
			memcpy((sess->config.objectName), opts->objectName, length);
			sess->config.objectName[length] = '\0';
		}
	}
	
	if (gethostname(hostname, IPC_MAX_HOST_NAME) != 0)
	{
		/* didnt get a hostname - return localhost */
		memcpy(hostname, "localhost\0", 10);
	}

	/* now get the IP address from the hostname */
	if (rsslGetHostByName(hostname, &sess->config.address) < 0)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,"<%s:%d> ripcWinInetCreatePipe rsslGetHostByName() failed",
		    __FILE__,__LINE__);
		ripcReleaseWinInetSession(sess, error);
		return 0;
	}

	/* get pid */
	sess->config.pID = _getpid();

	if (flags & RIPC_INT_CS_FLAG_BLOCKING)
		sess->config.blocking = 1;

	*userSpecPtr = sess;
	return (rssl_pipe_get_read_fd(&sess->_pipe));
}


/* this is going to have to change a bit since we dont get fd's*/
void *ripcWinInetConnect(RsslSocket fd, RsslInt32 *initComplete, void* userSpecPtr, RsslError *error)
{
	RsslInt32 retVal = 0;
	DWORD _error = 0;
	ripcWinInetSession *sess = (ripcWinInetSession*)userSpecPtr;
	RsslInt16 outLen = 0;
	char reqFlags = RIPC_TUNNEL_STREAMING | RIPC_WININET_TUNNELING;
	/* if this works, manage this in the session */
	RsslUInt32 respTimeout = 0x7FFFFFF;
	RsslUInt32  inetConnections = 128;  /* this should allow a lot of independent connections to be established */
	RsslUInt32 connLimitFail = 0;		  /* used to track error conditions on the setting of the connection limit */
	
	/* create and initialize pipe here */

	/* this will typically happen if we get into the reconnectOld function which means we are not connecting to a valid server */
	if (!sess)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcWinInetConnect trying to create tunneling connection with no session ", __FILE__, __LINE__);
		return 0;
	}

	/* check the connection status to make sure we have an internet connection */
	if (InternetAttemptConnect(0) == ERROR_SUCCESS)
	{
		if (sess->config.proxyHostName != 0)
			sess->openHandle = InternetOpen("RFA", INTERNET_OPEN_TYPE_PROXY, sess->config.proxyHostName, NULL, INTERNET_FLAG_ASYNC);		
		else
			sess->openHandle = InternetOpen("RFA", INTERNET_OPEN_TYPE_PRECONFIG, 0, 0, INTERNET_FLAG_ASYNC);

		if (!sess->openHandle)
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcWinInetConnect error on InternetOpen ", __FILE__, __LINE__);
			ripcWinInetErrors(error, strlen(error->text));
			return 0;
		}

		/* register callback function */
		if (InternetSetStatusCallback(sess->openHandle, (INTERNET_STATUS_CALLBACK)winInetCallback) == INTERNET_INVALID_STATUS_CALLBACK)
		{
			/* couldnt register callback */
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcWinInetConnect error on InternetSetStatusCallback ", __FILE__, __LINE__);
			ripcWinInetErrors(error, strlen(error->text));
			ripcReleaseWinInetSession(sess, error);
			return 0;
		}

		sess->connectHandle = InternetConnect(sess->openHandle, 
												 sess->config.hostName, 
												 sess->config.portNum,
												 NULL, NULL, 
											     INTERNET_SERVICE_HTTP, 
												 0, 
												 (DWORD_PTR)sess);

		if (!sess->connectHandle) 
		{
			/* couldnt register callback */
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcWinInetConnect error on InternetSetStatusCallback ", __FILE__, __LINE__);
			ripcWinInetErrors(error, strlen(error->text));
			ripcReleaseWinInetSession(sess, error);
			return 0;
		}

		if (!(InternetSetOption(sess->connectHandle, INTERNET_OPTION_RECEIVE_TIMEOUT, &respTimeout, 4)))
		{
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcWinInetConnect error on InternetSetOption with option %d ", __FILE__, __LINE__, INTERNET_OPTION_RECEIVE_TIMEOUT );
			ripcWinInetErrors(error, strlen(error->text));
			ripcReleaseWinInetSession(sess, error);
			return 0;
		}

		/* send the tunneling connection request */
		sess->streamingReqHandle = HttpOpenRequest(sess->connectHandle, "POST", sess->config.objectName, NULL, NULL, sess->acceptTypes, sess->config.connectionFlags, (DWORD_PTR)sess);

		if (!sess->streamingReqHandle) 
		{
			/* couldnt register callback */
			_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
			snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcWinInetConnect error on HttpOpenRequest ", __FILE__, __LINE__);
			ripcWinInetErrors(error, strlen(error->text));
			ripcReleaseWinInetSession(sess, error);
			return 0;
		}

		/* set connection limits */
	
		/* try to do this first using just our InternetConnect handle.  If this fails, it implies that 
		   this is using a WinInet from a version of IE that is older than IE7.  */

		if (!(InternetSetOption(sess->connectHandle, INTERNET_OPTION_MAX_CONNS_PER_SERVER, &inetConnections, 4)))
		{
			/* if this failed, the next one may work */
			++connLimitFail;	
		}
		
		if (!(InternetSetOption(sess->connectHandle, INTERNET_OPTION_MAX_CONNS_PER_1_0_SERVER, &inetConnections, 4)))
		{
			/* if this failed, the previous one may have worked */
			++connLimitFail;
		}
		
		/* if both of the above failed, we should be at connLimitFail == 2 */
		if (connLimitFail == 2)
		{
			/* if setting both connection limits fail, we can only set this global for the process.
			   We still need this to be done, so we have no option */
			/* if these fail as well, this is a legitimate error condition */
			if (!(InternetSetOption(NULL, INTERNET_OPTION_MAX_CONNS_PER_SERVER, &inetConnections, 4)))
			{
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcWinInetConnect error on InternetSetOption with option %d ", __FILE__, __LINE__,INTERNET_OPTION_MAX_CONNS_PER_SERVER  );
				ripcWinInetErrors(error, strlen(error->text));
				ripcReleaseWinInetSession(sess, error);
				return 0;
			}
		            
			if (!(InternetSetOption(NULL, INTERNET_OPTION_MAX_CONNS_PER_1_0_SERVER, &inetConnections, 4)))
			{
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcWinInetConnect error on InternetSetOption with option %d ", __FILE__, __LINE__,INTERNET_OPTION_MAX_CONNS_PER_1_0_SERVER  );
				ripcWinInetErrors(error, strlen(error->text));
				ripcReleaseWinInetSession(sess, error);
				return 0;
			}
		}

		/* encode our tunneling connection request */

		outLen += 2;
		sess->outPutBuffer[2] = reqFlags;
		++outLen;
		_move_u32_swap((sess->outPutBuffer + outLen), &sess->config.id);
		outLen += 4;
		_move_u16_swap((sess->outPutBuffer + outLen), &sess->config.pID);
		outLen += 2;
		_move_u32_swap((sess->outPutBuffer + outLen), &sess->config.address);
		outLen += 4;
		
		/* now the length */
		_move_u16_swap(sess->outPutBuffer, &outLen);
	
		/* lock here so we dont change behavior in callback */
		RSSL_MUTEX_LOCK(&sess->tunnelMutex);
		
		if (!(HttpSendRequest(sess->streamingReqHandle, NULL, 0, sess->outPutBuffer, outLen)))
		{
			DWORD _error = GetLastError();
			if (_error != ERROR_IO_PENDING)
			{
				_rsslSetError(error, NULL, RSSL_RET_FAILURE, _error);
				snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcWinInetConnect error on HttpSendRequest ", __FILE__, __LINE__);
				ripcWinInetErrors(error, strlen(error->text));
				/* unset the callback */
				ripcWinInetUnregisterCallbacks(sess);
				RSSL_MUTEX_UNLOCK(&sess->tunnelMutex);
				ripcReleaseWinInetSession(sess, error);
				return 0;
			}
			else
			{
				sess->state = WININET_INIT_STREAM_SENDRQST;
				if (++sess->callbackPending > 1)
				{
					_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
					snprintf(sess->errorText, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcWinInetConnect error - unexpected number of pending callbacks ", __FILE__, __LINE__);
					/* unset the callback */
					ripcWinInetUnregisterCallbacks(sess);
					RSSL_MUTEX_UNLOCK(&sess->tunnelMutex);
					ripcReleaseWinInetSession(sess, error);
					return 0;
				}
#ifdef DBUG_CBACKCOUNT
					printf("<%s:%d> Callback Count: %d\n", __FILE__, __LINE__, sess->callbackPending);
#endif
			}
		}	
		else
		{
			/* preincrement this - callback should not be triggered until IQDA is called */
			++sess->callbackPending;
			if (InternetQueryDataAvailable(sess->streamingReqHandle,&sess->bytesToRead , 0, 0))
			{
				if (sess->bytesToRead)
				{
					/* Unlikely but this may return right away but just in case.... 
					 * No need to grab the lock yet
					 */
					--sess->callbackPending;
					sess->state = WININET_INIT_STREAM_RESPONSE;
					
					if (InterlockedCompareExchange (&sess->byteWritten, 1L, 0L) == 0)
					{
	                	if (rssl_pipe_write(&sess->_pipe, "1", 1) != 1)
						{
							sess->byteWritten = 0;
							_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
							snprintf(sess->errorText, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcWinInetConnect error - unable to write byte to pipe ", __FILE__, __LINE__);
							/* unset the callback */
							ripcWinInetUnregisterCallbacks(sess);
							RSSL_MUTEX_UNLOCK(&sess->tunnelMutex);
							ripcReleaseWinInetSession(sess, error);
							return 0;	
						}
					}				
				}
			}
			else
			{
				_error = GetLastError();
				if (_error != ERROR_IO_PENDING)
				{
					_rsslSetError(error, NULL, RSSL_RET_FAILURE, _error);
					snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcWinInetConnect error on HttpSendRequest ", __FILE__, __LINE__);
					ripcWinInetErrors(error, strlen(error->text));
					/* unset the callback */
					/* no need to decrement pending count - we unregister callbacks here */
					ripcWinInetUnregisterCallbacks(sess);
					RSSL_MUTEX_UNLOCK(&sess->tunnelMutex);
					ripcReleaseWinInetSession(sess, error);
					return 0;
				}
				else
				{
					/* There should be nothing in the pipe yet, so we won't worry about reading from it.
					 */
					sess->state = WININET_INIT_STREAM_RESPONSE;

#ifdef DBUG_CBACKCOUNT
					printf("<%s:%d> Callback Count: %d\n", __FILE__, __LINE__, sess->callbackPending);
#endif
				}
			}
		}
		RSSL_MUTEX_UNLOCK(&sess->tunnelMutex);

		*initComplete = 0;

		return sess;
	}
	else
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> ripcWinInetConnect cannot find an active internet connection", __FILE__, __LINE__);
		ripcWinInetErrors(error, strlen(error->text));
		return 0;
	}
}

RsslRet ripcReleaseWinInetSession(void* session, RsslError *error)
{
	RsslRet retVal = 1;
 	if (session)
	{
		ripcWinInetSession *sess=(ripcWinInetSession*)session;

		/* dont cleanup until we come out of the callback
		 * This can happen if we are in a callback as the shutdown occurs 
		 */

		/* hopefully this would prevent the callbacks from triggering again */
		sess->state = WININET_ERROR;
		
		/* just in case it wasnt done, unset the callback */
		ripcWinInetUnregisterCallbacks(sess);

		/* wait for the callback to exit */
		while (sess->inCallback)
			;
		
		RSSL_MUTEX_LOCK(&sess->tunnelMutex);

		if (sess->streamingReqHandle)
		{
			if (sess->callbackPending == 0)
			{
				InternetCloseHandle(sess->streamingReqHandle);
				sess->streamingReqHandle = 0;
			}
			else
			{
				/* cleanup in dummy callout */
				InternetSetStatusCallback(sess->streamingReqHandle, dummyCleanupWinInetCallback);
				sess->streamingReqHandle = 0;
			}
		}

		if (sess->newStreamingReqHandle)
		{
			if (sess->callbackPending == 0)
			{
				InternetCloseHandle(sess->newStreamingReqHandle);
				sess->newStreamingReqHandle = 0;
			}
			else
			{
				/* cleanup in dummy callout */
				InternetSetStatusCallback(sess->newStreamingReqHandle, dummyCleanupWinInetCallback);
			}
		}

		RSSL_MUTEX_UNLOCK(&sess->tunnelMutex);
		
		if (sess->newControlReqHandle)
		{
			InternetCloseHandle(sess->newControlReqHandle);
			sess->newControlReqHandle = 0;
		}

		if (sess->controlReqHandle) 
		{
			InternetCloseHandle(sess->controlReqHandle);
			sess->controlReqHandle = 0;
		}

		if (sess->connectHandle)
		{
			InternetCloseHandle(sess->connectHandle);
			sess->connectHandle = 0;
		}

		if (sess->openHandle)
		{
			InternetCloseHandle(sess->openHandle);
			sess->openHandle = 0;
		}

		rssl_pipe_close(&sess->_pipe);

		if (sess->outPutBuffer)
		{
			_rsslFree(sess->outPutBuffer);
			sess->outPutBuffer = 0;
		}

		if (sess->acceptTypes)
		{
			_rsslFree((void*)sess->acceptTypes[0]);  /* this frees the accept type */
			_rsslFree((void*)sess->acceptTypes[1]);
			_rsslFree((void*)sess->acceptTypes);  /* this frees the LPCTRS */
			sess->acceptTypes = 0;
		}
		
		ripcFreeWinInetConnectOpts(&sess->config);
		
		/* do other session cleanup stuff here - we should already be disconnected from winInet */	
		/* just in case */
		
		RSSL_MUTEX_DESTROY(&sess->tunnelMutex);
		_rsslFree(sess);
	}
	return retVal;
}


ripcWinInetSession *ripcInitializeWinInetSession(RsslSocket fd, char* name, RsslError *error)
{
	ripcWinInetSession *sess = ripcWinInetNewSession(fd, name,error);

	if (sess == 0)
		return 0;

	/* do session initialization stuff here */
	return sess;
}
