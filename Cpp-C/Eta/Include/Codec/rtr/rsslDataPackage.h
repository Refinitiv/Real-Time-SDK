/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __RSSL_DATA_PACKAGE_H
#define __RSSL_DATA_PACKAGE_H


#ifdef __cplusplus
extern "C"
{
#endif

/* This includes all headers needed to use the data package */
#include "rtr/rsslDataTypeEnums.h"
#include "rtr/rsslRetCodes.h"
#include "rtr/rsslReal.h"
#include "rtr/rsslDateTime.h"
#include "rtr/rsslQos.h"
#include "rtr/rsslState.h"
#include "rtr/rsslIterators.h"
#include "rtr/rsslArray.h"
#include "rtr/rsslElementList.h"
#include "rtr/rsslFieldList.h"
#include "rtr/rsslFilterList.h"
#include "rtr/rsslMap.h"
#include "rtr/rsslSeries.h"
#include "rtr/rsslVector.h"
#include "rtr/rsslRmtes.h"
#include "rtr/rsslPrimitiveEncoders.h"
#include "rtr/rsslPrimitiveDecoders.h"
#include "rtr/rsslDataDictionary.h"
#include "rtr/rsslSetData.h"
#include "rtr/rsslDataUtils.h"



/** 
 *	@addtogroup DataPkgVersion Data Package Library Version Helper
 *	@{
 */

/** 
 * @brief Programmatically extracts library and product version information that is compiled into this library
 *
 * User can call this function to programmatically extract version information, or <BR>
 * query version information externally (via 'strings' command or something similar<BR>
 * and grep for the following:<BR>
 * 'VERSION' - contains internal library version information (e.g. eta3.6.0.1)<BR>
 * 'PACKAGE' - contains product information for load/package naming<BR>
 * @param pVerInfo RsslLibraryVersionInfo structure to populate with library version information
 * @see RsslLibraryVersionInfo
 */
RSSL_API void rsslQueryDataLibraryVersion(RsslLibraryVersionInfo *pVerInfo);


/**
 * @}
 */



#ifdef __cplusplus
}
#endif

 
#endif

