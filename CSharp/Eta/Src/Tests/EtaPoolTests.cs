/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using Xunit;
using Xunit.Categories;
using LSEG.Eta.Transports;

namespace LSEG.Eta.Transports.Tests
{
    public class EtaPoolTests
    {
        internal class TestObject : EtaNode
        {
            int _id = 0;
            internal TestObject(int id)
            {
                _id = id;
            }

            int id()
            {
                return _id;
            }
        }

        [Fact]
        [Category("EtaPool")]
        public void TestEtaPool()
        {
            Pool pool = new Pool(this);
            Assert.Null(pool.Poll());

            // create some objects and add them to the pool.
            TestObject obj1 = new TestObject(1);
            obj1.Pool = pool;
            TestObject obj2 = new TestObject(2);
            obj2.Pool = pool;
            TestObject obj3 = new TestObject(3);
            obj3.Pool = pool;
            TestObject obj4 = new TestObject(4);
            obj4.Pool = pool;

            pool.Add(obj1);
            TestObject obj = (TestObject)pool.Poll();
            Assert.Equal(obj1, obj);
            Assert.Null(pool.Poll());
            obj1.ReturnToPool();
            Assert.Equal(1, pool.Size);
            obj = (TestObject)pool.Poll();
            Assert.Equal(obj1, obj);
            Assert.Null(pool.Poll());

            pool.Add(obj4);
            pool.Add(obj3);
            pool.Add(obj2);
            pool.Add(obj1);
            Assert.Equal(4, pool.Size);

            Assert.Equal(obj4, pool.Poll());
            Assert.Equal(obj3, pool.Poll());
            Assert.Equal(obj2, pool.Poll());
            Assert.Equal(obj1, pool.Poll());
            Assert.Null(pool.Poll());
            Assert.Equal(0, pool.Size);

            obj1.ReturnToPool();
            obj2.ReturnToPool();
            obj3.ReturnToPool();
            obj4.ReturnToPool();
            Assert.Equal(4, pool.Size);

            Assert.Equal(obj1, pool.Poll());
            Assert.Equal(3, pool.Size);
            Assert.Equal(obj2, pool.Poll());
            Assert.Equal(2, pool.Size);
            Assert.Equal(obj3, pool.Poll());
            Assert.Equal(1, pool.Size);
            Assert.Equal(obj4, pool.Poll());
            Assert.Null(pool.Poll());
            Assert.Equal(0, pool.Size);

            // create some objects and add them to the pool.
            TestObject obj11 = new TestObject(1);
            obj11.Pool = pool;
            TestObject obj12 = new TestObject(2);
            obj12.Pool = pool;
            TestObject obj13 = new TestObject(3);
            obj13.Pool = pool;
            TestObject obj14 = new TestObject(4);
            obj14.Pool = pool;

            pool.Add(obj11);
            obj = (TestObject)pool.Poll();
            Assert.Equal(obj11, obj);
            Assert.Null(pool.Poll());
            obj11.ReturnToPool();
            Assert.Equal(1, pool.Size);
            obj = (TestObject)pool.Poll();
            Assert.Equal(obj11, obj);
            Assert.Null(pool.Poll());

            pool.Add(obj11);
            pool.Add(obj12);
            pool.Add(obj13);
            pool.Add(obj14);
            Assert.Equal(4, pool.Size);

            Assert.Equal(obj11, pool.Poll());
            Assert.Equal(obj12, pool.Poll());
            Assert.Equal(obj13, pool.Poll());
            Assert.Equal(obj14, pool.Poll());
            Assert.Null(pool.Poll());
            Assert.Equal(0, pool.Size);

            obj11.ReturnToPool();
            obj12.ReturnToPool();
            obj13.ReturnToPool();
            obj14.ReturnToPool();
            Assert.Equal(4, pool.Size);

            Assert.Equal(obj11, pool.Poll());
            Assert.Equal(3, pool.Size);
            Assert.Equal(obj12, pool.Poll());
            Assert.Equal(2, pool.Size);
            Assert.Equal(obj13, pool.Poll());
            Assert.Equal(1, pool.Size);
            Assert.Equal(obj14, pool.Poll());
            Assert.Null(pool.Poll());
            Assert.Equal(0, pool.Size);
        }
    }
}
