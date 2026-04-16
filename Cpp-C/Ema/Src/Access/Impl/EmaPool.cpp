/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "GlobalPool.h"
#include "libxml/parser.h"

using namespace refinitiv::ema::access;

GlobalPool g_pool;

bool GlobalPool::_isFinalState = false;

GlobalPool::GlobalPool()
{
	xmlInitParser();
	setMsgTypePoolLimit(DEFAULT_POOL_MSGTYPE_LIMIT, SettingMode::Initialize);
	setComplexTypePoolLimit(DEFAULT_POOL_COMPLEXTYPE_LIMIT, SettingMode::Initialize);
	setDataTypePoolLimit(DEFAULT_POOL_DATATYPE_LIMIT, SettingMode::Initialize);
}

GlobalPool::~GlobalPool()
{
	_isFinalState = true; // the global pool is being destroyed

	bool needToClear = true;

	while (needToClear)
	{
		needToClear = false;

		if (_refreshMsgImplPool.count())
			_refreshMsgImplPool.clear(), needToClear = true;

		if (_statusMsgImplPool.count())
			_statusMsgImplPool.clear(), needToClear = true;

		if (_updateMsgImplPool.count())
			_updateMsgImplPool.clear(), needToClear = true;

		if (_reqMsgImplPool.count())
			_reqMsgImplPool.clear(), needToClear = true;

		if (_postMsgImplPool.count())
			_postMsgImplPool.clear(), needToClear = true;

		if (_genericMsgImplPool.count())
			_genericMsgImplPool.clear(), needToClear = true;

		if (_ackMsgImplPool.count())
			_ackMsgImplPool.clear(), needToClear = true;

		if (_seriesDecoderPool.count())
			_seriesDecoderPool.clear(), needToClear = true;

		if (_vectorDecoderPool.count())
			_vectorDecoderPool.clear(), needToClear = true;

		if (_mapDecoderPool.count())
			_mapDecoderPool.clear(), needToClear = true;

		if (_filterListDecoderPool.count())
			_filterListDecoderPool.clear(), needToClear = true;

		if (_fieldListDecoderPool.count())
			_fieldListDecoderPool.clear(), needToClear = true;

		if (_elementListDecoderPool.count())
			_elementListDecoderPool.clear(), needToClear = true;

		if (_arrayDecoderPool.count())
			_arrayDecoderPool.clear(), needToClear = true;

		if (_fieldListEncoderPool.count())
			_fieldListEncoderPool.clear(), needToClear = true;

		if (_elementListEncoderPool.count())
			_elementListEncoderPool.clear(), needToClear = true;

		if (_arrayEncoderPool.count())
			_arrayEncoderPool.clear(), needToClear = true;

		if (_encodeIteratorPool.count())
			_encodeIteratorPool.clear(), needToClear = true;

		if (_fieldListSetDefPool.count())
			_fieldListSetDefPool.clear(), needToClear = true;

		if (_elementListSetDefPool.count())
			_elementListSetDefPool.clear(), needToClear = true;
	}

	xmlCleanupParser();
}

/** Ensure that the configuration (XML or Programmatic) modifies pool limit only once.
 *
 * @return true when the limit can be modified
 */
static bool validatePoolLimit(UInt32& limit, bool& limitSetFlag, const UInt32 newLimit,
							  const SettingMode mode)
{
	switch (mode)
	{
	case SettingMode::Configure:
		if (limitSetFlag && limit != newLimit)
		{
			// attempt to modify already configured pool limit via configuration
			return false;
		}
		limitSetFlag = true;
		break;
	case SettingMode::Initialize:
		limitSetFlag = false;
		break;
	case SettingMode::Overwrite:
		break;
	}

	limit = newLimit;
	return true;
}

void GlobalPool::setMsgTypePoolLimit(UInt32 limit, SettingMode mode)
{
	if (!validatePoolLimit(_msgTypeLimit, _msgTypeLimitSet, limit, mode))
	{
		EmaString errorMsg("Can not modify already configured EmaObjectManagerMsgTypeLimit, "
						   "use GlobalConfig::setMsgTypePoolLimit function instead");
		throwIceException(errorMsg);
	}

	_ackMsgImplPool.setLimit(_msgTypeLimit);
	_genericMsgImplPool.setLimit(_msgTypeLimit);
	_postMsgImplPool.setLimit(_msgTypeLimit);
	_reqMsgImplPool.setLimit(_msgTypeLimit);
	_refreshMsgImplPool.setLimit(_msgTypeLimit);
	_statusMsgImplPool.setLimit(_msgTypeLimit);
	_updateMsgImplPool.setLimit(_msgTypeLimit);
}

void GlobalPool::setComplexTypePoolLimit(UInt32 limit, SettingMode mode)
{
	if (!validatePoolLimit(_complexTypeLimit, _complexTypeLimitSet, limit, mode))
	{
		EmaString errorMsg("Can not modify already configured EmaObjectManagerComplexTypeLimit, "
						   "use GlobalConfig::setComplexTypePoolLimit function instead");
		throwIceException(errorMsg);
	}

	_encodeIteratorPool.setLimit(_complexTypeLimit);

	_elementListSetDefPool.setLimit(_complexTypeLimit);
	_fieldListSetDefPool.setLimit(_complexTypeLimit);

	_elementListEncoderPool.setLimit(_complexTypeLimit);
	_elementListDecoderPool.setLimit(_complexTypeLimit);

	_fieldListEncoderPool.setLimit(_complexTypeLimit);
	_fieldListDecoderPool.setLimit(_complexTypeLimit);

	_filterListEncoderPool.setLimit(_complexTypeLimit);
	_filterListDecoderPool.setLimit(_complexTypeLimit);

	_mapEncoderPool.setLimit(_complexTypeLimit);
	_mapDecoderPool.setLimit(_complexTypeLimit);

	_ommAnsiPageEncoderPool.setLimit(_complexTypeLimit);

	_ommJsonEncoderPool.setLimit(_complexTypeLimit);

	_ommOpaqueEncoderPool.setLimit(_complexTypeLimit);

	_ommXmlEncoderPool.setLimit(_complexTypeLimit);

	_seriesDecoderPool.setLimit(_complexTypeLimit);
	_seriesEncoderPool.setLimit(_complexTypeLimit);

	_vectorDecoderPool.setLimit(_complexTypeLimit);
	_vectorEncoderPool.setLimit(_complexTypeLimit);
}

void GlobalPool::setDataTypePoolLimit(UInt32 limit, SettingMode mode)
{
	if (!validatePoolLimit(_dataTypeLimit, _dataTypeLimitSet, limit, mode))
	{
		EmaString errorMsg("Can not modify already configured EmaObjectManagerDataTypeLimit, "
						   "use GlobalConfig::setDataTypePoolLimit function instead");
		throwIceException(errorMsg);
	}

	_arrayDecoderPool.setLimit(_dataTypeLimit);
	_arrayEncoderPool.setLimit(_dataTypeLimit);
}

UInt32 GlobalPool::getMsgTypePoolLimit() const
{
	return _msgTypeLimit;
}

UInt32 GlobalPool::getComplexTypePoolLimit() const
{
	return _complexTypeLimit;
}

UInt32 GlobalPool::getDataTypePoolLimit() const
{
	return _dataTypeLimit;
}

void GlobalPool::resetMode()
{
	_msgTypeLimitSet = false;
	_complexTypeLimitSet = false;
	_dataTypeLimitSet = false;
}
