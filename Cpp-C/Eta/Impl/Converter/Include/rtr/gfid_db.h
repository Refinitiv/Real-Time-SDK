
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

