/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019 LSEG. All rights reserved.
*/


#ifndef __RSSL_VA_EXPORTS_H
#define __RSSL_VA_EXPORTS_H


#ifdef __cplusplus
extern "C" {
#endif

#include "rtr/os.h"

/* Used when exporting or importing as Windows DLL or linking as static library */
#if defined(RSSL_VA_EXPORTS)
	#define		RSSL_VA_API			RTR_API_EXPORT
	#define		RSSL_VA_FST(ret)	RTR_API_EXPORT ret RTR_FASTCALL
#elif defined(RSSL_VA_IMPORTS)
	#define		RSSL_VA_API			RTR_API_IMPORT
	#define		RSSL_VA_FST(ret)	RTR_API_IMPORT ret RTR_FASTCALL
#else
	#define		RSSL_VA_API
	#define		RSSL_VA_FST(ret)	ret RTR_FASTCALL
#endif

#ifdef __cplusplus
}
#endif 

#endif
