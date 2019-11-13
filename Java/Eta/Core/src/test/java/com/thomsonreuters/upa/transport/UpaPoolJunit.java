///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.upa.transport;

import static org.junit.Assert.*;

import org.junit.Test;

public class UpaPoolJunit
{
    public class TestObject extends UpaNode
    {
        int _id = 0;
        TestObject(int id)
        {
            _id=id;
        }
        
        int id()
        {
            return _id;
        }
    }

    @Test
    public void test()
    {
        Pool pool = new Pool(this);
        assertEquals(null, pool.poll());
        
        // create some objects and add them to the pool.
        TestObject obj1 = new TestObject(1);
        obj1.pool(pool);
        TestObject obj2 = new TestObject(2);
        obj2.pool(pool);
        TestObject obj3 = new TestObject(3);
        obj3.pool(pool);
        TestObject obj4 = new TestObject(4);
        obj4.pool(pool);
        
        pool.add(obj1);
        TestObject obj = (TestObject) pool.poll();
        assertEquals(obj1, obj);
        assertEquals(null, pool.poll());
        obj1.returnToPool();
        assertEquals(1, pool.size());
        obj = (TestObject) pool.poll();
        assertEquals(obj1, obj);
        assertEquals(null, pool.poll());
        
        pool.add(obj4);
        pool.add(obj3);
        pool.add(obj2);
        pool.add(obj1);
        assertEquals(4, pool.size());
        
        assertEquals(obj4, pool.poll());
        assertEquals(obj3, pool.poll());
        assertEquals(obj2, pool.poll());
        assertEquals(obj1, pool.poll());
        assertEquals(null, pool.poll());
        assertEquals(0, pool.size());
        
        obj1.returnToPool();
        obj2.returnToPool();
        obj3.returnToPool();
        obj4.returnToPool();
        assertEquals(4, pool.size());
        
        assertEquals(obj1, pool.poll());
        assertEquals(3, pool.size());
        assertEquals(obj2, pool.poll());
        assertEquals(2, pool.size());
        assertEquals(obj3, pool.poll());
        assertEquals(1, pool.size());
        assertEquals(obj4, pool.poll());
        assertEquals(null, pool.poll());
        assertEquals(0, pool.size());
        
        // create some objects and add them to the pool.
        TestObject obj11 = new TestObject(1);
        obj11.pool(pool);
        TestObject obj12 = new TestObject(2);
        obj12.pool(pool);
        TestObject obj13 = new TestObject(3);
        obj13.pool(pool);
        TestObject obj14 = new TestObject(4);
        obj14.pool(pool);
        
        pool.add(obj11);
        obj = (TestObject) pool.poll();
        assertEquals(obj11, obj);
        assertEquals(null, pool.poll());
        obj11.returnToPool();
        assertEquals(1, pool.size());
        obj = (TestObject) pool.poll();
        assertEquals(obj11, obj);
        assertEquals(null, pool.poll());
        
        pool.add(obj11);
        pool.add(obj12);
        pool.add(obj13);
        pool.add(obj14);
        assertEquals(4, pool.size());
        
        assertEquals(obj11, pool.poll());
        assertEquals(obj12, pool.poll());
        assertEquals(obj13, pool.poll());
        assertEquals(obj14, pool.poll());
        assertEquals(null, pool.poll());
        assertEquals(0, pool.size());
        
        obj11.returnToPool();
        obj12.returnToPool();
        obj13.returnToPool();
        obj14.returnToPool();
        assertEquals(4, pool.size());
        
        assertEquals(obj11, pool.poll());
        assertEquals(3, pool.size());
        assertEquals(obj12, pool.poll());
        assertEquals(2, pool.size());
        assertEquals(obj13, pool.poll());
        assertEquals(1, pool.size());
        assertEquals(obj14, pool.poll());
        assertEquals(null, pool.poll());
        assertEquals(0, pool.size());
    }

}
