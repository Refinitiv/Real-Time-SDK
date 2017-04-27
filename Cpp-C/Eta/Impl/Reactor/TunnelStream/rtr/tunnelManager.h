/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

#ifndef TUNNEL_MANAGER_H
#define TUNNEL_MANAGER_H

#include "rtr/rsslErrorInfo.h"
#include "rtr/rsslTunnelStream.h"
#include "rtr/rsslReactor.h"

static const RsslInt32 TS_USE_DEFAULT_RECV_WINDOW_SIZE = -1;

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	RsslBool		_needsDispatchNow;
	RsslInt64		_nextDispatchTime;
	RsslReactorChannel* _pReactorChannel;
} TunnelManager;

/* Create a new TunnelManager. */
TunnelManager*	tunnelManagerOpen(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, RsslErrorInfo *pErrorInfo); 

/* Dispatch tunnel events. */
RsslRet tunnelManagerDispatch(TunnelManager *pManager, 
		RsslErrorInfo *pErrorInfo);

/* Process time-related events, e.g. messages with timeouts. */
RsslRet tunnelManagerProcessTimer(TunnelManager *pManager, RsslInt64 currentTime, 
		RsslErrorInfo *pErrorInfo);

/* Open a RsslTunnelStream. 
 * If copyName is false, it will presume the passed-in name is already allocated and
 * take that reference instead of creating a new copy. */
RsslTunnelStream* tunnelManagerOpenStream(TunnelManager *pManager,
		RsslTunnelStreamOpenOptions *pOptions, RsslBool isProvider, RsslClassOfService *pRemoteCos, RsslUInt streamVersion, RsslErrorInfo *pErrorInfo);

/* Read a message for a RsslTunnelStream. */
RsslRet tunnelManagerReadMsg(TunnelManager *pManager, RsslMsg *pMsg, RsslErrorInfo *pErrorInfo);

/* Submit a message to a RsslTunnelStream. */
RsslRet tunnelManagerSubmit(TunnelManager *pManager, RsslTunnelStream *pTunnel,
		RsslTunnelStreamSubmitMsgOptions *pOpts, RsslErrorInfo *pErrorInfo);

/* Submit a buffer to a RsslTunnelStream. */
RsslRet tunnelManagerSubmitBuffer(TunnelManager *pManager, RsslTunnelStream *pTunnel,
		RsslBuffer *pBuffer, RsslTunnelStreamSubmitOptions *pOpts, RsslErrorInfo *pErrorInfo);

/* Close a RsslTunnelStream. */
RsslRet tunnelManagerCloseStream(TunnelManager *pManager,
		RsslTunnelStream *pTunnel, RsslTunnelStreamCloseOptions *pOptions, RsslErrorInfo *pErrorInfo);

/* Handle channel close. */
RsslRet	tunnelManagerHandleChannelClosed(TunnelManager *pManager, RsslErrorInfo *pErrorInfo);

/* Handle errors returned from a tunnel stream. */
RsslRet tunnelManagerHandleStreamError(TunnelManager *pManager, RsslTunnelStream *pTunnelStream, RsslRet errorRetCode, RsslErrorInfo *pErrorInfo);

/* Close a TunnelManager. */
RsslRet tunnelManagerClose(TunnelManager *pManager, RsslErrorInfo *pErrorInfo);

/* Indicates if tunnelManagerDispatch should be called immediately. */
RTR_C_INLINE RsslBool tunnelManagerNeedsDispatchNow(TunnelManager *pManager)
{
	return pManager->_needsDispatchNow;
}

/* Indicates if tunnelManagerDispatch should be called at a later time. */
RTR_C_INLINE RsslBool tunnelManagerHasExpireTime(TunnelManager *pManager)
{
	return pManager->_nextDispatchTime != RDM_QMSG_TC_INFINITE;
}

/* Returns the next time at which the manager should call tunnelManagerDispatch.
 * Call tunnelManagerHasExpireTime to check if this value is valid. */
RTR_C_INLINE RsslInt64 tunnelManagerGetExpireTime(TunnelManager *pManager)
{
	return pManager->_nextDispatchTime;
}

/* Accepts a tunnel stream request. */
RsslRet tunnelManagerAcceptStream(TunnelManager *pManager, RsslTunnelStreamRequestEvent *pEvent,
								  RsslReactorAcceptTunnelStreamOptions *pOptions, RsslErrorInfo *pErrorInfo);

/* Rejects a tunnel stream request. */
RsslRet tunnelManagerRejectStream(TunnelManager *pManager, RsslTunnelStreamRequestEvent *pEvent,
								  RsslReactorRejectTunnelStreamOptions *pOptions, RsslErrorInfo *pErrorInfo);

/* Checks if a ClassOfService is valid */
RsslBool tunnelManagerValidateCos(RsslClassOfService *pCos, RsslBool isProvider, RsslErrorInfo *pErrorInfo);


#ifdef __cplusplus
};
#endif

#endif
