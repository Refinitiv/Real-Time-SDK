/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2018. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef	__ripc_internal_h
#define	__ripc_internal_h


#ifdef __cplusplus
extern "C" {
#endif


/* used to override config at global layer - internal only, used with ripcInitConfigOverride */
typedef struct {
	char *elLibraryThreadBind;			// this is to bind the thread in the EL library
	char *elRipcThreadBind;				// this is to bind the ELthread in ripc
	int elByPassMode;  
	int elGlobalReadWriteLock;  
	int elNumMaxConnectionsInELSocket;
	int elNumMaxELSocketInSet;
	int elShMemEnable;
	int	elShMemMaxELSocket;
	int elShMemMaxConns;
	int elShMemLatencyCheckSampleRate;
	int elDummy1;
	int elDummy2;
	int elDummy3;
} RIPC_INITIALIZE_CONFIG;

/* this initializes to the same values we set on the EL_Config if nothing is done */
#define RIPC_INIT_INITIALIZE_CONFIG {NULL, NULL, 0, 0, 100, 1000, 0, 13, 100, 1024, 0, 0, 0}

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
