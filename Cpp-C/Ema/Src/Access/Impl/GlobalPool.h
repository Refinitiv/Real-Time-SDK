/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_GlobalPool_h
#define __thomsonreuters_ema_access_GlobalPool_h

#include "OmmArrayDecoder.h"
#include "ElementListDecoder.h"
#include "FieldListDecoder.h"
#include "FilterListDecoder.h"
#include "MapDecoder.h"
#include "SeriesDecoder.h"
#include "VectorDecoder.h"

#include "AckMsgDecoder.h"
#include "GenericMsgDecoder.h"
#include "PostMsgDecoder.h"
#include "ReqMsgDecoder.h"
#include "RefreshMsgDecoder.h"
#include "StatusMsgDecoder.h"
#include "UpdateMsgDecoder.h"

#include "OmmArrayEncoder.h"
#include "ElementListEncoder.h"
#include "FieldListEncoder.h"
#include "VectorEncoder.h"
#include "SeriesEncoder.h"
#include "FilterListEncoder.h"
#include "MapEncoder.h"
#include "OmmAnsiPageEncoder.h"
#include "OmmOpaqueEncoder.h"
#include "OmmXmlEncoder.h"

#include "AckMsgEncoder.h"
#include "GenericMsgEncoder.h"
#include "PostMsgEncoder.h"
#include "ReqMsgEncoder.h"
#include "RefreshMsgEncoder.h"
#include "StatusMsgEncoder.h"
#include "UpdateMsgEncoder.h"

#include "ElementListSetDef.h"
#include "FieldListSetDef.h"

namespace rtsdk {

namespace ema {

namespace access {

class GlobalPool
{
public :

	GlobalPool();

	virtual ~GlobalPool();

	ElementListSetDefPool		_elementListSetDefPool;
	FieldListSetDefPool			_fieldListSetDefPool;

	EncodeIteratorPool			_encodeIteratorPool;
	OmmArrayEncoderPool			_arrayEncoderPool;
	ElementListEncoderPool		_elementListEncoderPool;
	FieldListEncoderPool		_fieldListEncoderPool;
	MapEncoderPool				_mapEncoderPool;
	VectorEncoderPool			_vectorEncoderPool;
	SeriesEncoderPool			_seriesEncoderPool;
	FilterListEncoderPool		_filterListEncoderPool;
	OmmAnsiPageEncoderPool		_ommAnsiPageEncoderPool;
	OmmOpaqueEncoderPool		_ommOpaqueEncoderPool;
	OmmXmlEncoderPool			_ommXmlEncoderPool;
	
	AckMsgEncoderPool			_ackMsgEncoderPool;
	GenericMsgEncoderPool		_genericMsgEncoderPool;
	PostMsgEncoderPool			_postMsgEncoderPool;
	ReqMsgEncoderPool			_reqMsgEncoderPool;
	RefreshMsgEncoderPool		_refreshMsgEncoderPool;
	StatusMsgEncoderPool		_statusMsgEncoderPool;
	UpdateMsgEncoderPool		_updateMsgEncoderPool;

	OmmArrayDecoderPool			_arrayDecoderPool;
	ElementListDecoderPool		_elementListDecoderPool;
	FieldListDecoderPool		_fieldListDecoderPool;
	FilterListDecoderPool		_filterListDecoderPool;
	MapDecoderPool				_mapDecoderPool;
	VectorDecoderPool			_vectorDecoderPool;
	SeriesDecoderPool			_seriesDecoderPool;

	AckMsgDecoderPool			_ackMsgDecoderPool;
	GenericMsgDecoderPool		_genericMsgDecoderPool;
	PostMsgDecoderPool			_postMsgDecoderPool;
	ReqMsgDecoderPool			_reqMsgDecoderPool;
	RefreshMsgDecoderPool		_refreshMsgDecoderPool;
	StatusMsgDecoderPool		_statusMsgDecoderPool;
	UpdateMsgDecoderPool		_updateMsgDecoderPool;
};

}

}

}

extern rtsdk::ema::access::GlobalPool g_pool;

#endif // __thomsonreuters_ema_access_GlobalPool_h
