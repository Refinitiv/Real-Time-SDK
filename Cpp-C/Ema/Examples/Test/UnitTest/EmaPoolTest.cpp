/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include <vector>

#include "gtest/gtest.h"

#include "Access/Impl/EmaPool.h"
#include "GlobalConfig.h"

#include "AckMsg.h"
#include "GenericMsg.h"
#include "PostMsg.h"
#include "RefreshMsg.h"
#include "ReqMsg.h"
#include "StatusMsg.h"
#include "UpdateMsg.h"

using namespace refinitiv::ema::access;

class EmaPoolTest : public ::testing::Test
{
};

struct PooledObject
{
	// use this counter to track the number of objects alive
	int* _counter;
	PooledObject() :
	 _counter{nullptr}
	{
	}

	void setCounter(int* counter)
	{
		if (_counter == nullptr)
		{
			_counter = counter;
			(*_counter)++;
		}
		else
		{
			_counter = counter;
		}
	}

	~PooledObject()
	{
		(*_counter)--;
	}
};

struct PooledObjectRecycler
{
	static void recycle(PooledObject* o)
	{
	}
};

struct TestPool : public Pool<PooledObject, PooledObjectRecycler>
{
	TestPool(unsigned int size = 5) :
	 Pool<PooledObject, PooledObjectRecycler>(size) {};
};

//** TESTS

/// no limit is defined, pool grows along with the number of objects taken from it: when an
/// object is returned to the pool, it is not deleted immediately
TEST_F(EmaPoolTest, testDefaultLimit)
{
	constexpr int ITEMS_COUNT = 100;

	int counter = 0;
	{
		TestPool testPool;
		{
			std::vector<PooledObject*> objects;
			objects.reserve(ITEMS_COUNT);

			for (int i = 0; i < ITEMS_COUNT; i++)
			{
				PooledObject* obj = testPool.getItem();
				obj->setCounter(&counter);
				objects.push_back(obj);
			}

			// there are no objects left in the pool
			EXPECT_EQ(0, testPool.count());

			// ensure that all objects are alive
			EXPECT_EQ(ITEMS_COUNT, counter);

			// now, return all to the pool
			for (PooledObject* obj : objects)
			{
				testPool.returnItem(obj);
			}

			objects.clear();
		}

		// but all objects are still alive because they are in the pool
		EXPECT_EQ(ITEMS_COUNT, counter);

		// there are no objects left in the pool
		EXPECT_EQ(ITEMS_COUNT, testPool.count());
	}
	// now the pool is destroyed, all objects must be gone
	EXPECT_EQ(0, counter);
}

/// Limit for the pool is defined, check that the excess objects are deleted when the pool is full
TEST_F(EmaPoolTest, testCappedPool)
{
	constexpr int ITEMS_COUNT = 101;
	constexpr int LIMIT = 13;

	int counter = 0;
	{
		TestPool testPool;
		testPool.setLimit(LIMIT);
		{
			std::vector<PooledObject*> objects;
			objects.reserve(ITEMS_COUNT);

			for (int i = 0; i < ITEMS_COUNT; i++)
			{
				PooledObject* obj = testPool.getItem();
				obj->setCounter(&counter);
				objects.push_back(obj);
			}

			// there are no objects left in the pool
			EXPECT_EQ(0, testPool.count());

			// ensure that all objects are alive
			EXPECT_EQ(ITEMS_COUNT, counter);

			// now, return all to the pool
			for (PooledObject* obj : objects)
			{
				testPool.returnItem(obj);
			}

			objects.clear();
		}

		// only pooled objects are alive, the rest were destroyed
		EXPECT_EQ(LIMIT, counter);

		// now make pool create objects on the fly again
		{
			std::vector<PooledObject*> objects;
			objects.reserve(ITEMS_COUNT);

			for (int i = 0; i < ITEMS_COUNT; i++)
			{
				PooledObject* obj = testPool.getItem();
				obj->setCounter(&counter);
				objects.push_back(obj);
			}

			// ensure that all objects are alive
			EXPECT_EQ(ITEMS_COUNT, counter);

			// now, return all to the pool
			for (PooledObject* obj : objects)
			{
				testPool.returnItem(obj);
			}

			objects.clear();
		}

		// only pooled objects are alive, the rest were destroyed
		EXPECT_EQ(LIMIT, counter);
	}
	// now the pool is destroyed, all objects must be gone
	EXPECT_EQ(0, counter);
}

TEST_F(EmaPoolTest, testMessagePool)
{
	const UInt32	 originalPoolLimit = GlobalConfig::getMsgTypePoolLimit();
	constexpr UInt32 POOL_LIMIT = 13;

	GlobalConfig::setMsgTypePoolLimit(POOL_LIMIT);
	EXPECT_EQ(POOL_LIMIT, GlobalConfig::getMsgTypePoolLimit());

	constexpr UInt32 MESSAGE_COUNT = POOL_LIMIT * 10 + 1;

	{
		std::vector<AckMsg>		ackMsgs{MESSAGE_COUNT};
		std::vector<GenericMsg> genericMsgs{MESSAGE_COUNT};
		std::vector<PostMsg>	postMsgs{MESSAGE_COUNT};
		std::vector<ReqMsg>		reqMsgs{MESSAGE_COUNT};
		std::vector<RefreshMsg> refreshMsgs{MESSAGE_COUNT};
		std::vector<StatusMsg>	statusMsgs{MESSAGE_COUNT};
		std::vector<UpdateMsg>	updateMsgs{MESSAGE_COUNT};

		// all objects were taken from the pool. This can break if some previous test has left some
		// messages in the pools

		EXPECT_EQ(0, GlobalConfig::getAckMsgInPoolCount());
		EXPECT_EQ(0, GlobalConfig::getGenericMsgInPoolCount());
		EXPECT_EQ(0, GlobalConfig::getPostMsgInPoolCount());
		EXPECT_EQ(0, GlobalConfig::getReqMsgInPoolCount());
		EXPECT_EQ(0, GlobalConfig::getRefreshMsgInPoolCount());
		EXPECT_EQ(0, GlobalConfig::getStatusMsgInPoolCount());
		EXPECT_EQ(0, GlobalConfig::getUpdateMsgInPoolCount());
	}

	EXPECT_EQ(POOL_LIMIT, GlobalConfig::getAckMsgInPoolCount());
	EXPECT_EQ(POOL_LIMIT, GlobalConfig::getGenericMsgInPoolCount());
	EXPECT_EQ(POOL_LIMIT, GlobalConfig::getPostMsgInPoolCount());
	EXPECT_EQ(POOL_LIMIT, GlobalConfig::getReqMsgInPoolCount());
	EXPECT_EQ(POOL_LIMIT, GlobalConfig::getRefreshMsgInPoolCount());
	EXPECT_EQ(POOL_LIMIT, GlobalConfig::getStatusMsgInPoolCount());
	EXPECT_EQ(POOL_LIMIT, GlobalConfig::getUpdateMsgInPoolCount());

	GlobalConfig::setMsgTypePoolLimit(originalPoolLimit);
}
