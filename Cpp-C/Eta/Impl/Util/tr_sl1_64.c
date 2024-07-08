/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
 *|-----------------------------------------------------------------------------
 */
 
#include <stdlib.h>
#include <time.h>
#include <memory.h>
#ifdef _WIN32
#include <windows.h>
#include <wincrypt.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif

#include "rtr/rwfNetwork.h"
#include "rtr/tr_sl1_64.h"



// returns negative value for failure, 0 for success; parameters are expected to be non null, 
// error -1: passed in encrypted buffer is too small
// error -2: passed in encrypted buffer does not have proper length
// error -3: could not create space to decrypt
// error -4: could not create memory for secret key
// error -5: content does not appear valid after decryption (this could indicate it was not encrypted as well)
// error -6: decrypted content length does not appear valid
// error -7: passed in decrypt buffer is too small to hold decrypted content
rtrInt32 Decrypt_TR_SL1_64(const rtrUInt8* pSharedKey, const RwfBuffer *pEncryptedInput, RwfBuffer *pDecryptedOutput) 
{
	rtrInt32	iEncrypt, iTemp;	
	rtrUInt32	randomHead;
	rtrUInt8	*pMessageTemp;
	rtrUInt8	*secretBuffer;
	int			iterator = 0;
	rtrUInt32   i, j;

	// Minimum block size of 16 bytes
	if(pEncryptedInput->length < (TR_SL1_64_KEY_LEN*2))
		return -1;

	// Messages being decrypted must be a modulus of shared secret
	if((pEncryptedInput->length % TR_SL1_64_KEY_LEN) != 0)
		return -2;
	

	// Copy the message that will be decrypted
	pMessageTemp = (rtrUInt8 *)alloca(pEncryptedInput->length);
	if(pMessageTemp == NULL) 
		return -3;
	
	memcpy(pMessageTemp, pEncryptedInput->data, pEncryptedInput->length);

	// TODO: clean this up, just trying to treat the 64 bit uint like a char*
	// Get the secret key, we need a copy since it gets modifed due to CBC
	secretBuffer = (rtrUInt8 *)alloca(TR_SL1_64_KEY_LEN);
	if(secretBuffer == NULL) 
		return -4;
	memcpy(secretBuffer, pSharedKey, TR_SL1_64_KEY_LEN);

	// Perform decryption
	for( i = 0; i < pEncryptedInput->length; i += TR_SL1_64_KEY_LEN)
	{
		for( j = 0; j < TR_SL1_64_KEY_LEN; j++) 
		{
			rtrUInt8 temp = ~(pMessageTemp[i+j] - 1); // For CBC encryption, with twist
			pMessageTemp[i+j] = pMessageTemp[i+j] ^ secretBuffer[j];
			secretBuffer[j] = temp;
		}
	}

	// Get the random header, put back into host order
	iterator += rwfGet32(randomHead, (pMessageTemp+iterator));
	
	// Set the message length, untwist with randomHead
	iterator += rwfGet32(iEncrypt, (pMessageTemp + iterator));
	iTemp = iEncrypt ^ randomHead;
	if((iTemp < 0) || (iTemp > ((rtrInt32)(pEncryptedInput->length) - (rtrInt32)(sizeof(rtrUInt32)+(sizeof(rtrUInt32)+4)))))
		return -6;

	// Verify the last 4 bytes are "TrEp". If not it is not valid
	iterator += iTemp;
	if(memcmp((pMessageTemp + iterator), TR_SL1_64_TERMINATION, 4) != 0) 
		return -5;
	iterator -= iTemp;

	// make sure the output length is sufficient to hold the input.  
	if ((rtrInt32)pDecryptedOutput->length < iTemp)
		return -7;
	// Set the decrypted message
	memcpy(pDecryptedOutput->data, (pMessageTemp + iterator), iTemp);
	pDecryptedOutput->length = iTemp;
	return 0;
}



// returns negative value for failure, 0 for success; parameters are expected to be non null, 
// error -1: passed in buffer is too small
// error -2: could not allocate memory for secret key
rtrInt32 Encrypt_TR_SL1_64(const rtrUInt8 *pSharedKey, const RwfBuffer *pUnencryptedInput, RwfBuffer *pEncryptedOutput) 
{
	rtrUInt32	iActualLen;
	rtrUInt32	randomHead = randu32();
	rtrUInt8	*secretBuffer;
	int			iterator = 0;
	rtrUInt32			i, j;
	rtrUInt32   randomHeader = 0;


	/******************************************************************
	* Calculate encrypt block len. the length is
	* randomHead = 4 bytes
	* unencrypted len = 4 bytes (twisted with randomHead)
	* unencrypted information = variable
	* termination = 4 bytes
	******************************************************************/
	iActualLen = CalculateEncryptedLength(pUnencryptedInput);
	if (pEncryptedOutput->length < iActualLen)
		return -1; // failure	
	// Put random number into the first 4 bytes
	iterator += rwfPut32((pEncryptedOutput->data + iterator), randomHead);	
	// Put unencrypted (len ^ random) in next 4 bytes in network order
	randomHeader = (pUnencryptedInput->length ^ randomHead);
	iterator += rwfPut32((pEncryptedOutput->data + iterator), randomHeader);
	// Add the actual message to encrypt
	memcpy((pEncryptedOutput->data + iterator), pUnencryptedInput->data, pUnencryptedInput->length);
	iterator += pUnencryptedInput->length;
	// End the ending termination
	memcpy((pEncryptedOutput->data + iterator), TR_SL1_64_TERMINATION, 4);
	iterator += 4;

	// TODO: clean this up, just trying to treat this like Get the DH secret key
	secretBuffer = (rtrUInt8 *)alloca(TR_SL1_64_KEY_LEN);
	if(secretBuffer == NULL) {
		return -2;
	}
	memcpy(secretBuffer, pSharedKey, TR_SL1_64_KEY_LEN);

	// Perform the encryption
	for(i = 0; i < iActualLen; i += TR_SL1_64_KEY_LEN) 
	{
		for(j = 0; j < TR_SL1_64_KEY_LEN; j++) 
		{
			pEncryptedOutput->data[i+j] = pEncryptedOutput->data[i+j] ^ secretBuffer[j];
			secretBuffer[j] = ~(pEncryptedOutput->data[i+j]); // For CBC encryption with twist
			secretBuffer[j]++;
		}
	}
	pEncryptedOutput->length = iActualLen;
	return 0;
}
