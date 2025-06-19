/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef __GLOBAL_FID_DB__
#define __GLOBAL_FID_DB__

#include "rtr/fid_db.h"

class RTRGlobalFidDb
{
public:
	static RTRFidDb& fidDb() { return *_gFidDb; }

	static void setFidDb(RTRFidDb& fidDb);

	static RTRFidDb** fidDbPtr() { return &_gFidDb; }

protected:

	static RTRFidDb *_gFidDb;
};

#endif

