/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "GlobalPool.h"
#include "libxml/parser.h"

using namespace refinitiv::ema::access;

GlobalPool g_pool;

GlobalPool::GlobalPool()
{
	xmlInitParser();
}

GlobalPool::~GlobalPool()
{
	bool needToClear = true;

	while ( needToClear )
	{
		needToClear = false;

		if ( _refreshMsgDecoderPool.count() )
			_refreshMsgDecoderPool.clear(), needToClear = true;

		if ( _statusMsgDecoderPool.count() )
			_statusMsgDecoderPool.clear(), needToClear = true;

		if ( _updateMsgDecoderPool.count() )
			_updateMsgDecoderPool.clear(), needToClear = true;

		if ( _reqMsgDecoderPool.count() )
			_reqMsgDecoderPool.clear(), needToClear = true;

		if ( _postMsgDecoderPool.count() )
			_postMsgDecoderPool.clear(), needToClear = true;

		if ( _genericMsgDecoderPool.count() )
			_genericMsgDecoderPool.clear(), needToClear = true;

		if ( _ackMsgDecoderPool.count() )
			_ackMsgDecoderPool.clear(), needToClear = true;

		if ( _seriesDecoderPool.count() )
			_seriesDecoderPool.clear(), needToClear = true;

		if ( _vectorDecoderPool.count() )
			_vectorDecoderPool.clear(), needToClear = true;

		if ( _mapDecoderPool.count() )
			_mapDecoderPool.clear(), needToClear = true;

		if ( _filterListDecoderPool.count() )
			_filterListDecoderPool.clear(), needToClear = true;

		if ( _fieldListDecoderPool.count() )
			_fieldListDecoderPool.clear(), needToClear = true;

		if ( _elementListDecoderPool.count() )
			_elementListDecoderPool.clear(), needToClear = true;

		if ( _arrayDecoderPool.count() )
			_arrayDecoderPool.clear(), needToClear = true;

		if ( _refreshMsgEncoderPool.count() )
			_refreshMsgEncoderPool.clear(), needToClear = true;

		if ( _statusMsgEncoderPool.count() )
			_statusMsgEncoderPool.clear(), needToClear = true;

		if ( _updateMsgEncoderPool.count() )
			_updateMsgEncoderPool.clear(), needToClear = true;

		if ( _reqMsgEncoderPool.count() )
			_reqMsgEncoderPool.clear(), needToClear = true;

		if ( _postMsgEncoderPool.count() )
			_postMsgEncoderPool.clear(), needToClear = true;

		if ( _genericMsgEncoderPool.count() )
			_genericMsgEncoderPool.clear(), needToClear = true;

		if ( _ackMsgEncoderPool.count() )
			_ackMsgEncoderPool.clear(), needToClear = true;

		if ( _fieldListEncoderPool.count() )
			_fieldListEncoderPool.clear(), needToClear = true;

		if ( _elementListEncoderPool.count() )
			_elementListEncoderPool.clear(), needToClear = true;

		if ( _arrayEncoderPool.count() )
			_arrayEncoderPool.clear(), needToClear = true;

		if ( _encodeIteratorPool.count() )
			_encodeIteratorPool.clear(), needToClear = true;

		if ( _fieldListSetDefPool.count() )
			_fieldListSetDefPool.clear(), needToClear = true;

		if ( _elementListSetDefPool.count() )
			_elementListSetDefPool.clear(), needToClear = true;
	}

	xmlCleanupParser();
}
