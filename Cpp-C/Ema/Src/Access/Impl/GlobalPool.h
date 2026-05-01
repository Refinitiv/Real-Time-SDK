/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015,2019-2020,2022-2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_GlobalPool_h
#define __refinitiv_ema_access_GlobalPool_h

#include "OmmArrayDecoder.h"
#include "ElementListDecoder.h"
#include "FieldListDecoder.h"
#include "FilterListDecoder.h"
#include "MapDecoder.h"
#include "SeriesDecoder.h"
#include "VectorDecoder.h"

#include "AckMsgImpl.h"
#include "GenericMsgImpl.h"
#include "PostMsgImpl.h"
#include "ReqMsgImpl.h"
#include "RefreshMsgImpl.h"
#include "StatusMsgImpl.h"
#include "UpdateMsgImpl.h"

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
#include "OmmJsonEncoder.h"

#include "ElementListSetDef.h"
#include "FieldListSetDef.h"

#include "EmaPool.h"

namespace refinitiv {

namespace ema {

namespace access {

enum class SettingMode
{
	Initialize,
	Configure,
	Overwrite
};

#define DO_ITEM_FUNCS(item, pool)\
item * get##item##Item()\
{\
	return pool.getItem();\
}\
void returnItem(item * pEncoder)\
{\
	if (_isFinalState)\
		delete pEncoder;\
	else\
		pool.returnItem(pEncoder);\
}

class GlobalPool
{
public:

	constexpr static UInt32 DEFAULT_POOL_MSGTYPE_LIMIT = RWF_MAX_32;
	constexpr static UInt32 DEFAULT_POOL_COMPLEXTYPE_LIMIT = RWF_MAX_32;
	constexpr static UInt32 DEFAULT_POOL_DATATYPE_LIMIT = RWF_MAX_32;

	GlobalPool();
	virtual ~GlobalPool();

	// When the global pool is in final state
	// then clients must stop all the operations with it
	static void setFinalState() {
		_isFinalState = true;
	}

	/*!< Limit the number of message objects in the pools */
	void setMsgTypePoolLimit(UInt32 limit, SettingMode mode);

	/*!< Limit the number of complex type decoders and encoders in the pools */
	void setComplexTypePoolLimit(UInt32 limit, SettingMode mode);

	/*!< Limit the number of ordinary data type objects in the pools. In Ema C++ this affects only
	 * Array encoder and decoder objects. */
	void setDataTypePoolLimit(UInt32 limit, SettingMode mode);

	UInt32 getMsgTypePoolLimit() const;

	UInt32 getComplexTypePoolLimit() const;

	UInt32 getDataTypePoolLimit() const;

	UInt32 getAckMsgInPoolCount() const
	{
		return _ackMsgImplPool.count();
	}

	UInt32 getGenericMsgInPoolCount() const
	{
		return _genericMsgImplPool.count();
	}

	UInt32 getPostMsgInPoolCount() const
	{
		return _postMsgImplPool.count();
	}

	UInt32 getReqMsgInPoolCount() const
	{
		return _reqMsgImplPool.count();
	}

	UInt32 getRefreshMsgInPoolCount() const
	{
		return _refreshMsgImplPool.count();
	}

	UInt32 getStatusMsgInPoolCount() const
	{
		return _statusMsgImplPool.count();
	}

	UInt32 getUpdateMsgInPoolCount() const
	{
		return _updateMsgImplPool.count();
	}

	/// Used by tests to enable subsequent OmmConsumer or Provider initialization overwrite pool limits
	/// without triggering an exception
	void resetMode();

	DO_ITEM_FUNCS(ElementListSetDef, _elementListSetDefPool);
	DO_ITEM_FUNCS(FieldListSetDef, _fieldListSetDefPool);

	DO_ITEM_FUNCS(EncodeIterator, _encodeIteratorPool);
	DO_ITEM_FUNCS(OmmArrayEncoder, _arrayEncoderPool);
	DO_ITEM_FUNCS(ElementListEncoder, _elementListEncoderPool);
	DO_ITEM_FUNCS(FieldListEncoder, _fieldListEncoderPool);
	DO_ITEM_FUNCS(MapEncoder, _mapEncoderPool);
	DO_ITEM_FUNCS(VectorEncoder, _vectorEncoderPool);
	DO_ITEM_FUNCS(SeriesEncoder, _seriesEncoderPool);
	DO_ITEM_FUNCS(FilterListEncoder, _filterListEncoderPool);
	DO_ITEM_FUNCS(OmmAnsiPageEncoder, _ommAnsiPageEncoderPool);
	DO_ITEM_FUNCS(OmmOpaqueEncoder, _ommOpaqueEncoderPool);
	DO_ITEM_FUNCS(OmmXmlEncoder, _ommXmlEncoderPool);
	DO_ITEM_FUNCS(OmmJsonEncoder, _ommJsonEncoderPool);

	DO_ITEM_FUNCS(AckMsgImpl, _ackMsgImplPool);
	DO_ITEM_FUNCS(GenericMsgImpl, _genericMsgImplPool);
	DO_ITEM_FUNCS(PostMsgImpl, _postMsgImplPool);
	DO_ITEM_FUNCS(ReqMsgImpl, _reqMsgImplPool);
	DO_ITEM_FUNCS(RefreshMsgImpl, _refreshMsgImplPool);
	DO_ITEM_FUNCS(StatusMsgImpl, _statusMsgImplPool);
	DO_ITEM_FUNCS(UpdateMsgImpl, _updateMsgImplPool);

	DO_ITEM_FUNCS(OmmArrayDecoder, _arrayDecoderPool);

	DO_ITEM_FUNCS(ElementListDecoder, _elementListDecoderPool);
	DO_ITEM_FUNCS(FieldListDecoder, _fieldListDecoderPool);
	DO_ITEM_FUNCS(FilterListDecoder, _filterListDecoderPool);
	DO_ITEM_FUNCS(MapDecoder, _mapDecoderPool);
	DO_ITEM_FUNCS(VectorDecoder, _vectorDecoderPool);
	DO_ITEM_FUNCS(SeriesDecoder, _seriesDecoderPool);

private:

	static bool					_isFinalState;  // indicates that the global pool is destroyed, clients must stop all operations with it

	UInt32 _msgTypeLimit;
	bool   _msgTypeLimitSet;
	UInt32 _complexTypeLimit;
	bool   _complexTypeLimitSet;
	UInt32 _dataTypeLimit;
	bool   _dataTypeLimitSet;

	Pool<ElementListSetDef> _elementListSetDefPool;
	Pool<FieldListSetDef>	_fieldListSetDefPool;

	Pool<EncodeIterator>			_encodeIteratorPool;
	EncoderPool<OmmArrayEncoder>	_arrayEncoderPool;
	EncoderPool<ElementListEncoder> _elementListEncoderPool;
	EncoderPool<FieldListEncoder>	_fieldListEncoderPool;
	EncoderPool<MapEncoder>			_mapEncoderPool;
	EncoderPool<VectorEncoder>		_vectorEncoderPool;
	EncoderPool<SeriesEncoder>		_seriesEncoderPool;
	EncoderPool<FilterListEncoder>	_filterListEncoderPool;
	EncoderPool<OmmAnsiPageEncoder> _ommAnsiPageEncoderPool;
	EncoderPool<OmmOpaqueEncoder>	_ommOpaqueEncoderPool;
	EncoderPool<OmmXmlEncoder>		_ommXmlEncoderPool;
	EncoderPool<OmmJsonEncoder>		_ommJsonEncoderPool;

	Pool<AckMsgImpl>	 _ackMsgImplPool;
	Pool<GenericMsgImpl> _genericMsgImplPool;
	Pool<PostMsgImpl>	 _postMsgImplPool;
	Pool<ReqMsgImpl>	 _reqMsgImplPool;
	Pool<RefreshMsgImpl> _refreshMsgImplPool;
	Pool<StatusMsgImpl>	 _statusMsgImplPool;
	Pool<UpdateMsgImpl>	 _updateMsgImplPool;

	Pool<OmmArrayDecoder>	 _arrayDecoderPool;
	Pool<ElementListDecoder> _elementListDecoderPool;
	Pool<FieldListDecoder>	 _fieldListDecoderPool;
	Pool<FilterListDecoder>	 _filterListDecoderPool;
	Pool<MapDecoder>		 _mapDecoderPool;
	Pool<VectorDecoder>		 _vectorDecoderPool;
	Pool<SeriesDecoder>		 _seriesDecoderPool;
};

#undef DO_ITEM_FUNCS

}

}

}

extern refinitiv::ema::access::GlobalPool g_pool;

#endif // __refinitiv_ema_access_GlobalPool_h
