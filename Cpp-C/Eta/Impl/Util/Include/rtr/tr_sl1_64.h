/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2018. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */
 
#ifndef _TR_SL1_64_H_
#define _TR_SL1_64_H_

#include "rtr/os.h"
#include "application_signing.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TR_SL1_64_TERMINATION	"TrEp"
#define TR_SL1_64_KEY_BIT_LEN	64
#define TR_SL1_64_KEY_LEN		(TR_SL1_64_KEY_BIT_LEN/8)


// make inline
RTR_C_ALWAYS_INLINE rtrUInt32 CalculateEncryptedLength(const RwfBuffer *pUnencryptedInput)
{
	/* multiplications/divisions and +7 are to ensure ending on a double word boundary. */
	return  (((pUnencryptedInput->length + sizeof(rtrUInt32) + sizeof(rtrUInt32) + 4 + 7)/8)*8);
};


/* Forward reference TR_SL1_64 encrypt / decrypt */
extern rtrInt32 Encrypt_TR_SL1_64(const rtrUInt8 *pSharedKey, const RwfBuffer *pUnencryptedInput, RwfBuffer *pEncryptedOutput); 
extern rtrInt32 Decrypt_TR_SL1_64(const rtrUInt8 *pSharedKey, const RwfBuffer *pEncryptedInput, RwfBuffer *pDecryptedOutput);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif

