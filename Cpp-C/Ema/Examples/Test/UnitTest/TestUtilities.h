/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2018-2020,2023-2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef __TestUtilities_h
#define __TestUtilities_h

#include "gtest/gtest.h"
#include "rtr/rsslDataDictionary.h"
#include "Access/Impl/StaticDecoder.h"
#include "rtr/rsslMsgDecoders.h"
#include "ActiveConfig.h"
#include "Ema.h"

#include <memory>
#include <array>

/* These are user defined FIDs to be used in the example so that we can show types */
/* that are not in the standard dictionary. User defined FIDs are always negative  */
#define FID_INT			-1
#define FID_DATETIME	-4
#define FID_QOS			-5
#define FID_STATE		-6
#define FID_BUFFER		-7
#define FID_ENUM		-8
#define FID_FLOAT		-9
#define FID_DOUBLE		-10

namespace refinitiv {

namespace ema {

namespace access {

class EmaString;
class ElementList;
class FieldList;
class Map;
class EmaBuffer;

}

}

}

extern refinitiv::ema::access::EmaString g_userName;
extern refinitiv::ema::access::EmaString g_password;

extern refinitiv::ema::access::EmaString g_proxyHost;
extern refinitiv::ema::access::EmaString g_proxyPort;

void encodeFieldList( RsslBuffer& rsslBuf, refinitiv::ema::access::EmaString& inText );
void encodeNonRWFData( RsslBuffer* rsslBuf, RsslBuffer* value );
bool loadDictionaryFromFile( RsslDataDictionary* pDictionary );

void perfDecode( const refinitiv::ema::access::ElementList& el );
void perfDecode( const refinitiv::ema::access::FieldList& fl );

void RsslEncodeElementListAll( RsslBuffer& rsslBuf );
void RsslEncodeFieldListAll( RsslBuffer& rsslBuf );
void RsslEncodeMapAll( RsslBuffer& mapBuffer );

void EmaEncodeFieldListAll( refinitiv::ema::access::FieldList& fl );
void EmaEncodeElementListAll( refinitiv::ema::access::ElementList& el );
void EmaEncodeMapAll( refinitiv::ema::access::Map& map );

void EmaDecodeFieldListAll( const refinitiv::ema::access::FieldList& fl );
void EmaDecodeElementListAll( const refinitiv::ema::access::ElementList& el );
void EmaDecodeMapAll( const refinitiv::ema::access::Map& map );

bool comparingData(RsslBuffer& rsslBuffer, const refinitiv::ema::access::EmaString& emaString);
bool checkNullTerminatedEmaString(const refinitiv::ema::access::EmaString& emaString);

void prepareMsgToCopy(RsslEncodeIterator& encIter, RsslBuffer& msgBuf,
	RsslMsg* pRsslMsg, RsslDecodeIterator& decodeIter, RsslMsg* pRsslMsgDecode, refinitiv::ema::access::Msg& respMsg,
	RsslDataDictionary const& dictionary);

// RAII wrapper over RsslBuffer of constant size
template<size_t N>
struct EsslBuffer
{
	EsslBuffer() :
	  _data(),
	  _buffer()
	{
		reset();
	};

	void reset()
	{
		_buffer.data = _data.data();
		_buffer.length = static_cast<refinitiv::ema::access::UInt32>(_data.size());
	};

	operator RsslBuffer*() { return &_buffer; };

	operator RsslBuffer&() { return _buffer; };

	RsslBuffer* operator->() { return &_buffer; };

	EsslBuffer( const EsslBuffer& ) = delete;
	EsslBuffer( EsslBuffer&& ) = delete;

private :

	std::array<char, N> _data;
	RsslBuffer _buffer;
};

void DictionaryDeleter(RsslDataDictionary*);

using DictionaryPtr = std::unique_ptr<RsslDataDictionary, decltype(&DictionaryDeleter)>;

DictionaryPtr makeDictionaryFromFile();

static void testSleep(int millisecs)
{
#if defined WIN32
	::Sleep((DWORD)(millisecs));
#else
	struct timespec sleeptime;
	sleeptime.tv_sec = millisecs / 1000;
	sleeptime.tv_nsec = (millisecs % 1000) * 1000000;
	nanosleep(&sleeptime, 0);
#endif
}

#endif // __TestUtilities_h
