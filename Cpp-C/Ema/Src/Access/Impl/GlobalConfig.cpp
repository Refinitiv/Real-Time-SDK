/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "GlobalConfig.h"
#include "GlobalPool.h"

using namespace refinitiv::ema::access;

void GlobalConfig::setMsgTypePoolLimit(UInt32 limit)
{
	g_pool.setMsgTypePoolLimit(limit, SettingMode::Overwrite);
}

void GlobalConfig::setComplexTypePoolLimit(UInt32 limit)
{
	g_pool.setComplexTypePoolLimit(limit, SettingMode::Overwrite);
}

void GlobalConfig::setDataTypePoolLimit(UInt32 limit)
{
	g_pool.setDataTypePoolLimit(limit, SettingMode::Overwrite);
}

UInt32 GlobalConfig::getMsgTypePoolLimit()
{
	return g_pool.getMsgTypePoolLimit();
}

UInt32 GlobalConfig::getComplexTypePoolLimit()
{
	return g_pool.getComplexTypePoolLimit();
}

UInt32 GlobalConfig::getDataTypePoolLimit()
{
	return g_pool.getDataTypePoolLimit();
}

UInt32 GlobalConfig::getAckMsgInPoolCount()
{
	return g_pool.getAckMsgInPoolCount();
}

UInt32 GlobalConfig::getGenericMsgInPoolCount()
{
	return g_pool.getGenericMsgInPoolCount();
}

UInt32 GlobalConfig::getPostMsgInPoolCount()
{
	return g_pool.getPostMsgInPoolCount();
}

UInt32 GlobalConfig::getReqMsgInPoolCount()
{
	return g_pool.getReqMsgInPoolCount();
}

UInt32 GlobalConfig::getRefreshMsgInPoolCount()
{
	return g_pool.getRefreshMsgInPoolCount();
}

UInt32 GlobalConfig::getStatusMsgInPoolCount()
{
	return g_pool.getStatusMsgInPoolCount();
}

UInt32 GlobalConfig::getUpdateMsgInPoolCount()
{
	return g_pool.getUpdateMsgInPoolCount();
}
