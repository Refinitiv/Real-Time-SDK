/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Common;
using LSEG.Eta.Codec;
using LSEG.Eta.Rdm;

using Xunit;
using Xunit.Categories;
using LSEG.Eta.Transports;

namespace LSEG.Eta.Transports.Tests
{
    public class EtaQueueTests
    {
        internal class TestObject : EtaNode
        {
            int _id = 0;

            internal TestObject(int id)
            {
                _id = id;
            }

            internal int id()
            {
                return _id;
            }
        }

        [Fact]
        [Category("EtaQueue")]
        public void DequeueFromEmptyQueueTest()
        {
            EtaQueue queue = new EtaQueue();
            Assert.Null(queue.Poll());
        }

        [Fact]
        [Category("EtaQueue")]
        public void EnqueueDequeueTest()
        {
            EtaQueue queue = new EtaQueue();

            // create some objects and add the to the queue.
            TestObject obj1 = new TestObject(1);
            TestObject obj2 = new TestObject(2);
            TestObject obj3 = new TestObject(3);
            TestObject obj4 = new TestObject(4);

            queue.Add(obj1);
            TestObject obj = (TestObject)queue.Poll();
            Assert.Equal(obj1, obj);
            Assert.Null(queue.Poll());

            queue.Add(obj1);
            queue.Add(obj2);
            obj = (TestObject)queue.Poll();
            Assert.Equal(obj1, obj);
            obj = (TestObject)queue.Poll();
            Assert.Equal(obj2, obj);
            Assert.Null(queue.Poll());

            queue.Add(obj1);
            queue.Add(obj2);
            queue.Add(obj3);
            obj = (TestObject)queue.Poll();
            Assert.Equal(obj1, obj);
            obj = (TestObject)queue.Poll();
            Assert.Equal(obj2, obj);
            obj = (TestObject)queue.Poll();
            Assert.Equal(obj3, obj);
            Assert.Null(queue.Poll());

            // add objs to the queue
            queue.Add(obj1);
            queue.Add(obj2);
            queue.Add(obj3);
            queue.Add(obj4);
            obj = (TestObject)queue.Poll();
            Assert.Equal(obj1, obj);
            obj = (TestObject)queue.Poll();
            Assert.Equal(obj2, obj);
            obj = (TestObject)queue.Poll();
            Assert.Equal(obj3, obj);
            // add obj2 and obj1 back to the queue
            queue.Add(obj2);
            queue.Add(obj1);
            obj = (TestObject)queue.Poll();
            Assert.Equal(obj4, obj);
            // add obj4 and obj3 back to the queue
            queue.Add(obj4);
            queue.Add(obj3);
            // expect the order to be obj2, obj1, obj4, obj3
            obj = (TestObject)queue.Poll();
            Assert.Equal(obj2, obj);
            obj = (TestObject)queue.Poll();
            Assert.Equal(obj1, obj);
            obj = (TestObject)queue.Poll();
            Assert.Equal(obj4, obj);
            obj = (TestObject)queue.Poll();
            Assert.Equal(obj3, obj);
            Assert.Null(queue.Poll());
            Assert.Null(queue.Poll());

            // create some objects and add the to the queue.
            TestObject obj11 = new TestObject(1);
            TestObject obj12 = new TestObject(2);
            TestObject obj13 = new TestObject(3);
            TestObject obj14 = new TestObject(4);

            queue.Add(obj11);
            obj = (TestObject)queue.Poll();
            Assert.Equal(obj11, obj);
            Assert.Null(queue.Poll());

            queue.Add(obj11);
            queue.Add(obj12);
            obj = (TestObject)queue.Poll();
            Assert.Equal(obj11, obj);
            obj = (TestObject)queue.Poll();
            Assert.Equal(obj12, obj);
            Assert.Null(queue.Poll());

            queue.Add(obj11);
            queue.Add(obj12);
            queue.Add(obj13);
            obj = (TestObject)queue.Poll();
            Assert.Equal(obj11, obj);
            obj = (TestObject)queue.Poll();
            Assert.Equal(obj12, obj);
            obj = (TestObject)queue.Poll();
            Assert.Equal(obj13, obj);
            Assert.Null(queue.Poll());

            // add objs to the queue
            queue.Add(obj11);
            queue.Add(obj12);
            queue.Add(obj13);
            queue.Add(obj14);
            obj = (TestObject)queue.Poll();
            Assert.Equal(obj11, obj);
            obj = (TestObject)queue.Poll();
            Assert.Equal(obj12, obj);
            obj = (TestObject)queue.Poll();
            Assert.Equal(obj13, obj);
            // add obj12 and obj11 back to the queue
            queue.Add(obj12);
            queue.Add(obj11);
            obj = (TestObject)queue.Poll();
            Assert.Equal(obj14, obj);
            // add obj14 and obj13 back to the queue
            queue.Add(obj14);
            queue.Add(obj13);
            // expect the order to be obj12, obj11, obj14, obj13
            obj = (TestObject)queue.Poll();
            Assert.Equal(obj12, obj);
            obj = (TestObject)queue.Poll();
            Assert.Equal(obj11, obj);
            obj = (TestObject)queue.Poll();
            Assert.Equal(obj14, obj);
            obj = (TestObject)queue.Poll();
            Assert.Equal(obj13, obj);
            Assert.Null(queue.Poll());
            Assert.Null(queue.Poll());
        }

        [Fact]
        [Category("EtaQueue")]
        public void RemoveTest()
        {
            EtaQueue queue = new EtaQueue();

            // create some objects and add the to the queue.
            TestObject obj1 = new TestObject(1);
            TestObject obj2 = new TestObject(2);
            TestObject obj3 = new TestObject(3);
            TestObject obj4 = new TestObject(4);
            TestObject obj5 = new TestObject(5);

            // test remove on an empty queue.
            Assert.Equal(0, queue.Size);
            Assert.False(queue.Remove(obj1));

            // test queue.Remove() on a queue with one node.
            queue.Add(obj1);
            Assert.Equal(1, queue.Size);
            Assert.False(queue.Remove(obj2));
            Assert.Equal(1, queue.Size);
            Assert.True(queue.Remove(obj1));
            Assert.Equal(0, queue.Size);
            Assert.Null(queue.Poll());

            // test queue.Remove() on a queue with two nodes (node being removed is first).
            queue.Add(obj1);
            queue.Add(obj2);
            Assert.Equal(2, queue.Size);
            Assert.False(queue.Remove(obj3));
            Assert.Equal(2, queue.Size);
            Assert.True(queue.Remove(obj1));
            Assert.Equal(1, queue.Size);
            Assert.Equal(obj2, queue.Poll());
            Assert.Equal(0, queue.Size);
            Assert.Null(queue.Poll());

            // test queue.Remove() on a queue with two nodes (node being removed is
            // last).
            queue.Add(obj1);
            queue.Add(obj2);
            Assert.Equal(2, queue.Size);
            Assert.False(queue.Remove(obj4));
            Assert.Equal(2, queue.Size);
            Assert.True(queue.Remove(obj2));
            Assert.Equal(1, queue.Size);
            Assert.Equal(obj1, queue.Poll());
            Assert.Equal(0, queue.Size);
            Assert.Null(queue.Poll());

            // test queue.Remove() on a queue with three nodes (node being removed is first).
            queue.Add(obj1);
            queue.Add(obj2);
            queue.Add(obj3);
            Assert.Equal(3, queue.Size);
            Assert.False(queue.Remove(obj4));
            Assert.Equal(3, queue.Size);
            Assert.True(queue.Remove(obj1));
            Assert.Equal(2, queue.Size);
            Assert.Equal(obj2, queue.Poll());
            Assert.Equal(1, queue.Size);
            Assert.Equal(obj3, queue.Poll());
            Assert.Equal(0, queue.Size);
            Assert.Null(queue.Poll());

            // test queue.Remove() on a queue with three nodes (node being removed is second).
            queue.Add(obj1);
            queue.Add(obj2);
            queue.Add(obj3);
            Assert.Equal(3, queue.Size);
            Assert.False(queue.Remove(obj4));
            Assert.Equal(3, queue.Size);
            Assert.True(queue.Remove(obj2));
            Assert.Equal(2, queue.Size);
            Assert.Equal(obj1, queue.Poll());
            Assert.Equal(1, queue.Size);
            Assert.Equal(obj3, queue.Poll());
            Assert.Equal(0, queue.Size);
            Assert.Null(queue.Poll());

            // test queue.Remove() on a queue with three nodes (node being removed is last).
            queue.Add(obj1);
            queue.Add(obj2);
            queue.Add(obj3);
            Assert.Equal(3, queue.Size);
            Assert.False(queue.Remove(obj4));
            Assert.Equal(3, queue.Size);
            Assert.True(queue.Remove(obj3));
            Assert.Equal(2, queue.Size);
            Assert.Equal(obj1, queue.Poll());
            Assert.Equal(1, queue.Size);
            Assert.Equal(obj2, queue.Poll());
            Assert.Equal(0, queue.Size);
            Assert.Null(queue.Poll());

            // test queue.Remove() on a queue with four nodes (node being removed is first).
            queue.Add(obj1);
            queue.Add(obj2);
            queue.Add(obj3);
            queue.Add(obj4);
            Assert.Equal(4, queue.Size);
            Assert.False(queue.Remove(obj5));
            Assert.Equal(4, queue.Size);
            Assert.True(queue.Remove(obj1));
            Assert.Equal(3, queue.Size);
            Assert.Equal(obj2, queue.Poll());
            Assert.Equal(2, queue.Size);
            Assert.Equal(obj3, queue.Poll());
            Assert.Equal(1, queue.Size);
            Assert.Equal(obj4, queue.Poll());
            Assert.Equal(0, queue.Size);
            Assert.Null(queue.Poll());

            // test queue.Remove() on a queue with four nodes (node being removed is second).
            queue.Add(obj1);
            queue.Add(obj2);
            queue.Add(obj3);
            queue.Add(obj4);
            Assert.Equal(4, queue.Size);
            Assert.False(queue.Remove(obj5));
            Assert.Equal(4, queue.Size);
            Assert.True(queue.Remove(obj2));
            Assert.Equal(3, queue.Size);
            Assert.Equal(obj1, queue.Poll());
            Assert.Equal(2, queue.Size);
            Assert.Equal(obj3, queue.Poll());
            Assert.Equal(1, queue.Size);
            Assert.Equal(obj4, queue.Poll());
            Assert.Equal(0, queue.Size);
            Assert.Null(queue.Poll());

            // test queue.Remove() on a queue with four nodes (node being removed is third).
            queue.Add(obj1);
            queue.Add(obj2);
            queue.Add(obj3);
            queue.Add(obj4);
            Assert.Equal(4, queue.Size);
            Assert.False(queue.Remove(obj5));
            Assert.Equal(4, queue.Size);
            Assert.True(queue.Remove(obj3));
            Assert.Equal(3, queue.Size);
            Assert.Equal(obj1, queue.Poll());
            Assert.Equal(2, queue.Size);
            Assert.Equal(obj2, queue.Poll());
            Assert.Equal(1, queue.Size);
            Assert.Equal(obj4, queue.Poll());
            Assert.Equal(0, queue.Size);
            Assert.Null(queue.Poll());

            // test queue.Remove() on a queue with four nodes (node being removed is last).
            queue.Add(obj1);
            queue.Add(obj2);
            queue.Add(obj3);
            queue.Add(obj4);
            Assert.Equal(4, queue.Size);
            Assert.False(queue.Remove(obj5));
            Assert.Equal(4, queue.Size);
            Assert.True(queue.Remove(obj4));
            Assert.Equal(3, queue.Size);
            Assert.Equal(obj1, queue.Poll());
            Assert.Equal(2, queue.Size);
            Assert.Equal(obj2, queue.Poll());
            Assert.Equal(1, queue.Size);
            Assert.Equal(obj3, queue.Poll());
            Assert.Equal(0, queue.Size);
            Assert.Null(queue.Poll());

            // remove two nodes and dequeue the other two.
            queue.Add(obj1);
            queue.Add(obj2);
            queue.Add(obj3);
            queue.Add(obj4);
            Assert.Equal(4, queue.Size);
            Assert.True(queue.Remove(obj4));
            Assert.Equal(3, queue.Size);
            Assert.True(queue.Remove(obj2));
            Assert.Equal(2, queue.Size);
            Assert.Equal(obj1, queue.Poll());
            Assert.Equal(1, queue.Size);
            Assert.Equal(obj3, queue.Poll());
            Assert.Equal(0, queue.Size);
            Assert.Null(queue.Poll());

            // remove all (4) nodes.
            queue.Add(obj1);
            queue.Add(obj2);
            queue.Add(obj3);
            queue.Add(obj4);
            Assert.Equal(4, queue.Size);
            Assert.True(queue.Remove(obj4));
            Assert.Equal(3, queue.Size);
            Assert.True(queue.Remove(obj2));
            Assert.Equal(2, queue.Size);
            Assert.True(queue.Remove(obj1));
            Assert.Equal(1, queue.Size);
            Assert.True(queue.Remove(obj3));
            Assert.Equal(0, queue.Size);
            Assert.Null(queue.Poll());
        }
    }
}
