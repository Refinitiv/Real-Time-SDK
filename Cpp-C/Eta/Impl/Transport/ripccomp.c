/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2018. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */


#include <stdio.h>
#include "rtr/rsslAlloc.h"
#include "rtr/rsslErrors.h"
#include "rtr/rsslSocketTransportImpl.h"

static RsslInt32 compressionDebug = 0;

#ifndef _RIPC_NO_ZLIB

#include "zlib.h"
#include "lz4.h"

//
// zlib routines start here
//
static void *zlibCompInit(RsslInt32 compressionLevel, RsslError *error)
{
	RsslInt32 err;
	z_stream *zs=(z_stream*)_rsslMalloc(sizeof(z_stream));

	if (zs == 0)
		return 0;

	zs->zalloc = 0;
	zs->zfree = 0;
	zs->opaque = 0;

	if (compressionLevel < 0 || compressionLevel > 9)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: 1004 Invalid zlib compression level %d.  Level must be between 0 and 9.\n",
			__FILE__, __LINE__, compressionLevel);

		_rsslFree(zs);
		zs = 0;
		return 0;
	}

	err = deflateInit(zs,compressionLevel);
	if (err != Z_OK)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT,
			"<%s:%d> Error: 1002 deflateInit() failed. Zlib error: %d\n",
			__FILE__, __LINE__, err);

		_rsslFree(zs);
		zs = 0;
	}
	if (rtrUnlikely(compressionDebug)) printf("zlib using compression level=%d\n", compressionLevel);
	return zs;
}

static void *zlibDecompInit(RsslError *error)
{
	RsslInt32 err;
	z_stream *zs = (z_stream*)_rsslMalloc(sizeof(z_stream));

	if (zs == 0)
		return 0;

	zs->zalloc = 0;
	zs->zfree = 0;
	zs->opaque = 0;

	err = inflateInit(zs);
	if (err != Z_OK)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Error: 1002 inflateInit() failed. Zlib error: %d\n",
					__FILE__,__LINE__,err);
		_rsslFree(zs);
		zs = 0;
	}
	return zs;
}

static void zlibCompEnd(void *zstream)
{
	z_stream *zs=(z_stream*)zstream;
	if (zs)
	{
		deflateEnd(zs);
		free(zs);
	}
}

static void zlibDecompEnd(void *zstream)
{
	z_stream *zs=(z_stream*)zstream;
	if (zs)
	{
		inflateEnd(zs);
		free(zs);
	}
}

static RsslRet zlibcompress(void *zstream, ripcCompBuffer *buf, RsslError *error)
{
	RsslInt32 err;
	z_stream *zs=(z_stream*)zstream;

    zs->next_in  = (Byte*)buf->next_in;
    zs->avail_in = buf->avail_in;
    zs->next_out = (Byte*)buf->next_out;
    zs->avail_out = buf->avail_out;

    err = deflate(zs, Z_SYNC_FLUSH);
    if (err != Z_OK)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Error: 1002 deflate() failed.  Zlib error: %d\n",
				__FILE__,__LINE__,err);
        return -1;
	}

	buf->bytes_in_used = buf->avail_in - zs->avail_in;
	buf->bytes_out_used = buf->avail_out - zs->avail_out;
    
	buf->next_in = (char*)zs->next_in;
	buf->avail_in = zs->avail_in;
	buf->next_out = (char*)zs->next_out;
	buf->avail_out = zs->avail_out;
	if (rtrUnlikely(compressionDebug)) printf("zlib Compressed %d inbytes to %d outbytes (avail_out = %lu)\n", buf->bytes_in_used, buf->bytes_out_used, buf->avail_out);

    return 1;
}

static RsslRet zlibdecompress(void *zstream, ripcCompBuffer *buf, RsslError *error)
{
	RsslInt32 err;
	z_stream *zs=(z_stream*)zstream;

    zs->next_in  = (Byte*)buf->next_in;
    zs->avail_in = buf->avail_in;
    zs->next_out = (Byte*)buf->next_out;
    zs->avail_out = buf->avail_out;

    err = inflate(zs, Z_SYNC_FLUSH);
    if (err < Z_OK)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Error: 1002 inflate() failed. Zlib error: %d\n",
				__FILE__,__LINE__,err);
        return -1;
	}

	buf->bytes_in_used = buf->avail_in - zs->avail_in;
	buf->bytes_out_used = buf->avail_out - zs->avail_out;
    
	buf->next_in = (char*)zs->next_in;
	buf->avail_in = zs->avail_in;
	buf->next_out = (char*)zs->next_out;
	buf->avail_out = zs->avail_out;
	if (rtrUnlikely(compressionDebug)) printf("zlib Decompressed %d inbytes to %d outbytes\n", buf->bytes_in_used, buf->bytes_out_used);

	return 1;
}

RsslInt32 ripcInitZlibComp()
{
	ripcCompFuncs funcs;
	funcs.compressInit = zlibCompInit;
	funcs.decompressInit = zlibDecompInit;
	funcs.compressEnd = zlibCompEnd;
	funcs.decompressEnd = zlibDecompEnd;
	funcs.compress = zlibcompress;
	funcs.decompress = zlibdecompress;

	return(ipcSetCompFunc(RSSL_COMP_ZLIB,&funcs));
}

//
//	LZ4 compression routines start here
//
static char lz4PlaceHolder;
static void *lz4CompInit(RsslInt32 compressionLevel, RsslError *error)
{
	return(&lz4PlaceHolder);	// returning NULL is considered a failure, so return a valid address as a fake zstream
}

static void *lz4DecompInit(RsslError *error)
{
	return(&lz4PlaceHolder);	// returning NULL is considered a failure, so return a valid address as a fake zstream
}

static void lz4CompEnd(void *zstream)
{
	// we do not need to do anything with the fake zstream that was returned by lz4CompInit()
}

static void lz4DecompEnd(void *zstream)
{
	// we do not need to do anything with the fake zstream that was returned by lz4DecompInit()
}

static RsslRet lz4Comp(void* stream, ripcCompBuffer *buf, RsslError *error)
{
	RsslInt32 err;

	err = LZ4_compress_default(buf->next_in, buf->next_out, buf->avail_in, buf->avail_out);

	if(err < 0)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Error: 1002 LZ4_compress failed. LZ4 error: %d\n", __FILE__, __LINE__, err);
		return -1;
	}

	if (compressionDebug) printf("LZ4 Compressed %u inbytes to %d outbytes (max = %d)\n", buf->avail_in, err, LZ4_compressBound((int)buf->avail_in));
	// 
	// All we need is the bytes in used and bytes out used. 
	// Assuming it doesn't error out, that's the entirety of the input buffer and the return value from LZ4_compress 
	//
	buf->bytes_in_used = buf->avail_in;
	buf->bytes_out_used = err;
	
	buf->next_in = buf->next_in + buf->bytes_in_used;
	buf->avail_in = buf->avail_in - buf->bytes_in_used;
	buf->next_out = buf->next_out + buf->bytes_out_used;
	buf->avail_out = buf->avail_out - buf->bytes_out_used;

	return 1;
}

static RsslRet lz4Decomp(void* stream, ripcCompBuffer *buf, RsslError *error)
{
	RsslInt32 err;
	err = LZ4_decompress_safe(buf->next_in, buf->next_out, buf->avail_in, buf->avail_out);

	if(err < 0)
	{
		_rsslSetError(error, NULL, RSSL_RET_FAILURE, 0);
		snprintf(error->text, MAX_RSSL_ERROR_TEXT, "<%s:%d> Error: 1002 LZ4_decompress failed. LZ4 error: %d\n", __FILE__, __LINE__, err);
		return -1;
	}
	if (compressionDebug) printf("LZ4 Decompressed %u inbytes to %d outbytes\n", buf->avail_in, err);

	// All that's used after this function returns is next_out(needs to be 0)
	buf->bytes_in_used = buf->avail_in;
	buf->bytes_out_used = err;

	buf->avail_in = 0;
	buf->next_in = buf->next_in + buf->bytes_in_used;
	buf->next_out = buf->next_out + err;
	buf->avail_out = buf->avail_out - buf->bytes_out_used;

	return 1;
}

int ripcInitLz4Comp()
{
	ripcCompFuncs funcs;
	funcs.compressInit = lz4CompInit;
	funcs.decompressInit = lz4DecompInit;
	funcs.compressEnd = lz4CompEnd;
	funcs.decompressEnd = lz4DecompEnd;
	funcs.compress = lz4Comp;
	funcs.decompress = lz4Decomp;

	return(ipcSetCompFunc(RSSL_COMP_LZ4,&funcs));
}

#endif
