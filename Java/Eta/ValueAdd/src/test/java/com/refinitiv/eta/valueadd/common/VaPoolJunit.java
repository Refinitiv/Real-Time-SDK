///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019 LSEG. All rights reserved.     
///*|-----------------------------------------------------------------------------

package com.refinitiv.eta.valueadd.common;

import static org.junit.Assert.*;

import org.junit.Test;

public class VaPoolJunit
{
    public class TestObject extends VaNode
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
        VaPool pool = new VaPool();
        assertEquals(null, pool.poll());
        
        // create some objects and add the to the pool.
        TestObject obj1 = new TestObject(1);
        TestObject obj2 = new TestObject(2);
        TestObject obj3 = new TestObject(3);
        TestObject obj4 = new TestObject(4);
        
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
        
    }

}
