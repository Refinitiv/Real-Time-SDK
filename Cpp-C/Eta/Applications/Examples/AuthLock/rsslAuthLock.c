/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019 LSEG. All rights reserved.
*/


/*
 * This is the main file for the rsslAuthLock application.  Its
 * purpose is to demonstrate the functionality of the DACS library.
 */

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "dacs_lib.h"

#define		MAX_PRODUCT_CODES	256

typedef struct {
	char            _operator;
	unsigned short  pc_listLen;
	unsigned long   pc_list[MAX_PRODUCT_CODES];
} PC_DATA;

static void rsslAuthLock();

int main(int argc, char **argv)
{
	rsslAuthLock();
}

static void rsslAuthLock()
{
	PC_DATA pcData;
	PRODUCT_CODE_TYPE* pcTypePtr = (PRODUCT_CODE_TYPE *)&pcData;
	COMB_LOCK_TYPE combineLockList[5];
	COMB_LOCK_TYPE* combineLockListPtr = (COMB_LOCK_TYPE *)&combineLockList[0];
	unsigned char* lockPtr = NULL;
	unsigned char lockData[32];
	unsigned char* lock1Ptr = NULL;
	unsigned char lock1Data[32];
	unsigned char* lock2Ptr = NULL;
	unsigned char lock2Data[32];
	unsigned char* lock3Ptr = NULL;
	unsigned char lock3Data[32];
	unsigned char* lock4Ptr = NULL;
	unsigned char lock4Data[32];
	unsigned char* combineLockPtr = NULL;
	unsigned char* combineLock1Ptr = NULL;
	unsigned char* combineLock2Ptr = NULL;
	int lockLen = 0;
	int lock1Len = 0;
	int lock2Len = 0;
	int lock3Len = 0;
	int lock4Len = 0;
	int combineLockLen = 0;
	int combineLock1Len = 0;
	int combineLock2Len = 0;
	DACS_ERROR_TYPE dacsError;
	unsigned char dacsErrorBuffer[128];

	printf("\nCreate lock\n");
	pcData._operator = OR_PRODUCT_CODES;
	pcData.pc_listLen = 1;
	pcData.pc_list[0] = 62;

	if (DACS_GetLock(5000, pcTypePtr, &lockPtr, &lockLen, &dacsError) == DACS_FAILURE)
	{
		if (DACS_perror(dacsErrorBuffer, sizeof(dacsErrorBuffer), (unsigned char *)"DACS_GetLock() failed with error", &dacsError) == DACS_SUCCESS)
		{
			printf("%s\n", dacsErrorBuffer);
		}
		else
		{
			printf("DACS_GetLock() failed\n");
		}
		return;
	}
	printf("DACS_GetLock() - Success\n");

	printf("\nCreate lock1\n");
	pcData.pc_list[1] = 144;
	pcData.pc_listLen += 1;
	if (DACS_GetLock(5000, pcTypePtr, &lock1Ptr, &lock1Len, &dacsError) == DACS_FAILURE)
	{
		if (DACS_perror(dacsErrorBuffer, sizeof(dacsErrorBuffer), (unsigned char *)"DACS_GetLock() failed with error", &dacsError) == DACS_SUCCESS)
		{
			printf("%s\n", dacsErrorBuffer);
		}
		else
		{
			printf("DACS_GetLock() failed\n");
		}
		return;
	}
	printf("DACS_GetLock() - Success\n");

	printf("\nCreate lock2\n");
	pcData.pc_list[1] = 62;
	pcData.pc_list[2] = 144;
	pcData.pc_listLen += 1;
	if (DACS_GetLock(5000, pcTypePtr, &lock2Ptr, &lock2Len, &dacsError) == DACS_FAILURE)
	{
		if (DACS_perror(dacsErrorBuffer, sizeof(dacsErrorBuffer), (unsigned char *)"DACS_GetLock() failed with error", &dacsError) == DACS_SUCCESS)
		{
			printf("%s\n", dacsErrorBuffer);
		}
		else
		{
			printf("DACS_GetLock() failed\n");
		}
		return;
	}
	printf("DACS_GetLock() - Success\n");

	printf("\nCreate lock3\n");
	pcData.pc_list[0] = pcData.pc_list[1] = pcData.pc_list[2] = 0;
	pcData.pc_listLen = 0;
	if (DACS_GetLock(5000, pcTypePtr, &lock3Ptr, &lock3Len, &dacsError) == DACS_FAILURE)
	{
		if (DACS_perror(dacsErrorBuffer, sizeof(dacsErrorBuffer), (unsigned char *)"DACS_GetLock() failed with error", &dacsError) == DACS_SUCCESS)
		{
			printf("%s\n", dacsErrorBuffer);
		}
		else
		{
			printf("DACS_GetLock() failed\n");
		}
		return;
	}
	printf("DACS_GetLock() - Success\n");

	printf("\nCreate invalid lock4\n");
	lock4Len = lock2Len - 1;
	memcpy(&lock4Data[0], lock2Ptr, lock4Len);
	lock4Ptr = &lock4Data[0];

	printf("\nCompare lock1 and lock2\n");
	if (DACS_CmpLock(lock1Ptr, lock1Len, lock2Ptr, lock2Len, &dacsError) == DACS_DIFF) // identical
	{
		printf("DACS_CmpLock() - Two locks are different\n");
	}
	else
	{
		printf("DACS_CmpLock() - Two locks are identical\n");
	}

	printf("\nChange Service id in lock1\n");
	/* free existing lock1 */
	free(lock1Ptr);
	lock1Ptr = NULL;
	pcData.pc_list[0] = 62;
	pcData.pc_list[1] = 144;
	pcData.pc_listLen = 2;
	// Service id changed from 5000 to 30
	if (DACS_GetLock(30, pcTypePtr, &lock1Ptr, &lock1Len, &dacsError) == DACS_FAILURE)
	{
		if (DACS_perror(dacsErrorBuffer, sizeof(dacsErrorBuffer), (unsigned char *)"DACS_GetLock() failed with error", &dacsError) == DACS_SUCCESS)
		{
			printf("%s\n", dacsErrorBuffer);
		}
		else
		{
			printf("DACS_GetLock() failed\n");
		}
		return;
	}
	printf("DACS_GetLock() - Success\n");

	printf("\nCompare lock1 and lock2\n");
	if (DACS_CmpLock(lock1Ptr, lock1Len, lock2Ptr, lock2Len, &dacsError) == DACS_DIFF) // identical
	{
		printf("DACS_CmpLock() - Two locks are different\n");
	}
	else
	{
		printf("DACS_CmpLock() - Two locks are identical\n");
	}

	printf("\nCompare lock1 and lock3\n");
	if (DACS_CmpLock(lock1Ptr, lock1Len, lock3Ptr, lock3Len, &dacsError) == DACS_DIFF) // identical
	{
		printf("DACS_CmpLock() - Two locks are different\n");
	}
	else
	{
		printf("DACS_CmpLock() - Two locks are identical\n");
	}

	printf("\nCompare lock1 and lock4\n");
	if (DACS_CmpLock(lock1Ptr, lock1Len, lock4Ptr, lock4Len, &dacsError) == DACS_DIFF) // identical
	{
		printf("DACS_CmpLock() - Two locks are different\n");
	}
	else
	{
		printf("DACS_CmpLock() - Two locks are identical\n");
	}

	printf("\nCombine lock, lock1, and lock2 into combineLock\n");
	combineLockList[0].server_type = 0;
	combineLockList[0].item_name =  NULL;
	combineLockList[0].lockLen = lockLen;
	memcpy(lockData, lockPtr, lockLen);
	combineLockList[0].access_lock = &lockData[0];
	combineLockList[1].server_type = 0;
	combineLockList[1].item_name =  NULL;
	combineLockList[1].lockLen = lock1Len;
	memcpy(lock1Data, lock1Ptr, lock1Len);
	combineLockList[1].access_lock = &lock1Data[0];
	combineLockList[2].server_type = 0;
	combineLockList[2].item_name =  NULL;
	combineLockList[2].lockLen = lock2Len;
	memcpy(lock2Data, lock2Ptr, lock2Len);
	combineLockList[2].access_lock = &lock2Data[0];
	combineLockList[3].server_type = 0;
	combineLockList[3].item_name =  NULL;
	combineLockList[3].access_lock = NULL;
	combineLockList[3].lockLen = 0;
	if (DACS_CsLock(0, (char *)"", &combineLockPtr, &combineLockLen, combineLockListPtr, &dacsError) == DACS_FAILURE)
	{
		if (DACS_perror(dacsErrorBuffer, sizeof(dacsErrorBuffer), (unsigned char *)"DACS_CsLock() failed with error", &dacsError) == DACS_SUCCESS)
		{
			printf("%s\n", dacsErrorBuffer);
		}
		else
		{
			printf("DACS_CsLock() failed\n");
		}
		return;
	}
	printf("DACS_CsLock() - Success\n");

	printf("\nCombine lock, lock1, and lock3 into combineLock1\n");
	/* lock and lock1 already in list, just add lock3 and terminate */
	combineLockList[2].server_type = 0;
	combineLockList[2].item_name =  NULL;
	combineLockList[2].lockLen = lock3Len;
	memcpy(lock3Data, lock3Ptr, lock3Len);
	combineLockList[2].access_lock = &lock3Data[0];
	combineLockList[3].server_type = 0;
	combineLockList[3].item_name =  NULL;
	combineLockList[3].access_lock = NULL;
	combineLockList[3].lockLen = 0;
	if (DACS_CsLock(0, (char *)"", &combineLock1Ptr, &combineLock1Len, combineLockListPtr, &dacsError) == DACS_FAILURE)
	{
		if (DACS_perror(dacsErrorBuffer, sizeof(dacsErrorBuffer), (unsigned char *)"DACS_CsLock() failed with error", &dacsError) == DACS_SUCCESS)
		{
			printf("%s\n", dacsErrorBuffer);
		}
		else
		{
			printf("DACS_CsLock() failed\n");
		}
		return;
	}
	printf("DACS_CsLock() - Success\n");

	printf("\nCompare combineLock and combineLock1\n");
	if (DACS_CmpLock(combineLockPtr, combineLockLen, combineLock1Ptr, combineLock1Len, &dacsError) == DACS_DIFF) // identical
	{
		printf("DACS_CmpLock() - Two locks are different\n");
	}
	else
	{
		printf("DACS_CmpLock() - Two locks are identical\n");
	}

	printf("\nCombine lock, lock1, and invalid lock4\n");
	combineLockList[0].server_type = 0;
	combineLockList[0].item_name =  NULL;
	combineLockList[0].access_lock = lockPtr;
	combineLockList[0].lockLen = lockLen;
	combineLockList[1].server_type = 0;
	combineLockList[1].item_name =  NULL;
	combineLockList[1].access_lock = lock1Ptr;
	combineLockList[1].lockLen = lock1Len;
	combineLockList[2].server_type = 0;
	combineLockList[2].item_name =  NULL;
	combineLockList[2].access_lock = lock4Ptr;
	combineLockList[2].lockLen = lock4Len;
	combineLockList[3].server_type = 0;
	combineLockList[3].item_name =  NULL;
	combineLockList[3].access_lock = NULL;
	combineLockList[3].lockLen = 0;
	if (DACS_CsLock(0, (char *)"", &combineLock2Ptr, &combineLock2Len, combineLockListPtr, &dacsError) == DACS_FAILURE)
	{
		if (DACS_perror(dacsErrorBuffer, sizeof(dacsErrorBuffer), (unsigned char *)"DACS_CsLock() failed with error", &dacsError) == DACS_SUCCESS)
		{
			printf("%s\n", dacsErrorBuffer);
		}
		else
		{
			printf("DACS_CsLock() failed\n");
		}
	}
	else
	{
		printf("combineLock2Len = %d\n", combineLock2Len);
	}

	/* free locks */
	free(lockPtr);
	free(lock1Ptr);
	free(lock2Ptr);
	free(lock3Ptr);
	free(combineLockPtr);
	free(combineLock1Ptr);
	free(combineLock2Ptr);
}

