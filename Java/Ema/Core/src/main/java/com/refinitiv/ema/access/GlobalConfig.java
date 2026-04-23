/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

public class GlobalConfig
{
	final static int DEFAULT_EVENT_POOL_LIMIT = -1;
	final static int DEFAULT_WATCHLIST_OBJECTS_POOL_LIMIT = -1;
	final static int DEFAULT_SOCKET_PROTOCOL_POOL_LIMIT = -1;
	static final int JSON_CONVERTER_DEFAULT_POOLS_SIZE = 10;
	final static int DEFAULT_EMA_POOLS_LIMIT = -1;

	int reactorMsgEventPoolLimit;
	int reactorChannelEventPoolLimit;
	int workerEventPoolLimit;
	int tunnelStreamMsgEventPoolLimit;
	int tunnelStreamStatusEventPoolLimit;
	int jsonConverterPoolsSize;
	int watchlistPoolLimit;
	int watchlistObjectsPoolLimit;
	int socketProtocolPoolLimit;

	int dataTypePoolLimit;
	int complexTypePoolLimit;
	int msgTypePoolLimit;
	int etaObjectsPoolLimit;
	int sessionObjectsPoolLimit;

	GlobalConfig()
	{
		reactorMsgEventPoolLimit = DEFAULT_EVENT_POOL_LIMIT;
		reactorChannelEventPoolLimit = DEFAULT_EVENT_POOL_LIMIT;
		workerEventPoolLimit = DEFAULT_EVENT_POOL_LIMIT;
		tunnelStreamMsgEventPoolLimit = DEFAULT_EVENT_POOL_LIMIT;
		tunnelStreamStatusEventPoolLimit = DEFAULT_EVENT_POOL_LIMIT;
		jsonConverterPoolsSize = JSON_CONVERTER_DEFAULT_POOLS_SIZE;
		watchlistPoolLimit = DEFAULT_WATCHLIST_OBJECTS_POOL_LIMIT;
		watchlistObjectsPoolLimit = DEFAULT_WATCHLIST_OBJECTS_POOL_LIMIT;
		socketProtocolPoolLimit = DEFAULT_SOCKET_PROTOCOL_POOL_LIMIT;

		dataTypePoolLimit = DEFAULT_EMA_POOLS_LIMIT;
		complexTypePoolLimit = DEFAULT_EMA_POOLS_LIMIT;
		msgTypePoolLimit = DEFAULT_EMA_POOLS_LIMIT;
		etaObjectsPoolLimit = DEFAULT_EMA_POOLS_LIMIT;
		sessionObjectsPoolLimit = DEFAULT_EMA_POOLS_LIMIT;
	}

	void clear()
	{
		reactorMsgEventPoolLimit = DEFAULT_EVENT_POOL_LIMIT;
		reactorChannelEventPoolLimit = DEFAULT_EVENT_POOL_LIMIT;
		workerEventPoolLimit = DEFAULT_EVENT_POOL_LIMIT;
		tunnelStreamMsgEventPoolLimit = DEFAULT_EVENT_POOL_LIMIT;
		tunnelStreamStatusEventPoolLimit = DEFAULT_EVENT_POOL_LIMIT;
		jsonConverterPoolsSize = JSON_CONVERTER_DEFAULT_POOLS_SIZE;
		watchlistPoolLimit = DEFAULT_WATCHLIST_OBJECTS_POOL_LIMIT;
		watchlistObjectsPoolLimit = DEFAULT_WATCHLIST_OBJECTS_POOL_LIMIT;
		socketProtocolPoolLimit = DEFAULT_SOCKET_PROTOCOL_POOL_LIMIT;

		dataTypePoolLimit = DEFAULT_EMA_POOLS_LIMIT;
		complexTypePoolLimit = DEFAULT_EMA_POOLS_LIMIT;
		msgTypePoolLimit = DEFAULT_EMA_POOLS_LIMIT;
		etaObjectsPoolLimit = DEFAULT_EMA_POOLS_LIMIT;
		sessionObjectsPoolLimit = DEFAULT_EMA_POOLS_LIMIT;
	}

	// Methods for changing Global EmaObjectManager limits (EMA objects)

	/**
	 * Gets the current limit of the pool in the global pool manager that holds Data type EMA objects
	 * @return the current limit of the pool
	 */
	public static int getGlobalEmaObjectManagerDataTypePoolLimit()
	{
		return EmaObjectManager.GlobalObjectManager.getEmaObjectDataTypePoolLimit();
	}

	/**
	 * Gets the current limit of the pool in the global pool manager that holds Complex type EMA objects
	 * @return the current limit of the pool
	 */
	public static int getGlobalEmaObjectManagerComplexTypePoolLimit()
	{
		return EmaObjectManager.GlobalObjectManager.getEmaObjectComplexTypePoolLimit();
	}

	/**
	 * Gets the current limit of the pool in the global pool manager that holds Message type EMA objects
	 * @return the current limit of the pool
	 */
	public static int getGlobalEmaObjectManagerMsgTypePoolLimit()
	{
		return EmaObjectManager.GlobalObjectManager.getEmaObjectMsgTypePoolLimit();
	}

	/**
	 * Sets the current limit of the pool in the global pool manager that holds Data type EMA objects
	 * @param limit the new limit to be set
	 */
	public static void setGlobalEmaObjectManagerDataTypePoolLimit(int limit)
	{
		EmaObjectManager.GlobalObjectManager.setDataTypePoolsLimit(limit);
	}

	/**
	 * Sets the current limit of the pool in the global pool manager that holds Complex type EMA objects
	 * @param limit the new limit to be set
	 */
	public static void setGlobalEmaObjectManagerComplexTypePoolLimit(int limit)
	{
		EmaObjectManager.GlobalObjectManager.setComplexTypePoolsLimit(limit);
	}

	/**
	 * Sets the current limit of the pool in the global pool manager that holds Message type EMA objects
	 * @param limit the new limit to be set
	 */
	public static void setGlobalEmaObjectManagerMsgTypePoolLimit(int limit)
	{
		EmaObjectManager.GlobalObjectManager.setMsgTypePoolsLimit(limit);
	}

	// Methods for changing Global EmaObjectManager limits (ETA objects)

	/**
	 * Gets the current limit of all pools in the global pool manager that hold ETA objects
	 * @return the current limit of the pools
	 */
	public static int getGlobalEmaObjectManagerEtaObjectsPoolLimit()
	{
		return EmaObjectManager.GlobalObjectManager.getEtaObjectsPoolsLimit();
	}

	/**
	 * Sets the current limit of all pools in the global pool manager that hold ETA objects
	 * @param limit the new limit of the pools
	 */
	public static void setGlobalEmaObjectManagerEtaObjectsPoolLimit(int limit)
	{
		EmaObjectManager.GlobalObjectManager.setEtaObjectsPoolsLimit(limit);
	}

	// Methods for changing Global EmaObjectManager limits (objects related to Sessions)

	/**
	 * Gets the current limit of all pools in the global pool manager that hold objects related to Sessions
	 * @return the current limit of the pools
	 */
	public static int getGlobalEmaObjectManagerSessionObjectsPoolLimit()
	{
		return EmaObjectManager.GlobalObjectManager.getSessionObjectPoolLimit();
	}

	/**
	 * Sets the current limit of all pools in the global pool manager that hold objects related to Sessions
	 * @param limit the new limit of the pools
	 */
	public static void setGlobalEmaObjectManagerSessionObjectsPoolLimit(int limit)
	{
		EmaObjectManager.GlobalObjectManager.setSessionObjectPoolLimit(limit);
	}

	// Methods for getting the number of messages in the Message type pools

	/**
	 * Gets the current count of the objects in the Request messages pool
	 * @return the number of objects in the pool
	 */
	public static int getGlobalEmaObjectManagerRequestMsgPoolCount()
	{
		return EmaObjectManager.GlobalObjectManager.getEmaObjectPoolCount(DataType.DataTypes.REQ_MSG);
	}

	/**
	 * Gets the current count of the objects in the Refresh messages pool
	 * @return the number of objects in the pool
	 */
	public static int getGlobalEmaObjectManagerRefreshMsgPoolCount()
	{
		return EmaObjectManager.GlobalObjectManager.getEmaObjectPoolCount(DataType.DataTypes.REFRESH_MSG);
	}

	/**
	 * Gets the current count of the objects in the Update messages pool
	 * @return the number of objects in the pool
	 */
	public static int getGlobalEmaObjectManagerUpdateMsgPoolCount()
	{
		return EmaObjectManager.GlobalObjectManager.getEmaObjectPoolCount(DataType.DataTypes.UPDATE_MSG);
	}

	/**
	 * Gets the current count of the objects in the Generic messages pool
	 * @return the number of objects in the pool
	 */
	public static int getGlobalEmaObjectManagerGenericMsgPoolCount()
	{
		return EmaObjectManager.GlobalObjectManager.getEmaObjectPoolCount(DataType.DataTypes.GENERIC_MSG);
	}

	/**
	 * Gets the current count of the objects in the Ack messages pool
	 * @return the number of objects in the pool
	 */
	public static int getGlobalEmaObjectManagerAckMsgPoolCount()
	{
		return EmaObjectManager.GlobalObjectManager.getEmaObjectPoolCount(DataType.DataTypes.ACK_MSG);
	}

	/**
	 * Gets the current count of the objects in the Post messages pool
	 * @return the number of objects in the pool
	 */
	public static int getGlobalEmaObjectManagerPostMsgPoolCount()
	{
		return EmaObjectManager.GlobalObjectManager.getEmaObjectPoolCount(DataType.DataTypes.POST_MSG);
	}

	/**
	 * Gets the current count of the objects in the Status messages pool
	 * @return the number of objects in the pool
	 */
	public static int getGlobalEmaObjectManagerStatusMsgPoolCount()
	{
		return EmaObjectManager.GlobalObjectManager.getEmaObjectPoolCount(DataType.DataTypes.STATUS_MSG);
	}
}
