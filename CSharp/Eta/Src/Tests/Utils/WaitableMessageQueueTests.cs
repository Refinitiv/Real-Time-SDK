/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Tests.Common.Utils;
using System;
using System.Threading.Tasks;
using Xunit;
using Xunit.Sdk;

namespace LSEG.Eta.Tests.Utils
{
    public class WaitableMessageQueueTests
    {
        private static readonly TimeSpan TestWaitTimeout = TimeSpan.FromSeconds(30);

        public class WaitForMessageAsyncTests
        {
            [Fact]
            public async Task Should_return_enqueued_message_when_its_type_is_proper()
            {
                // Arrange
                var queue = new WaitableMessageQueue<Msg>();
                var msg = new ConcreteMsgA();
                // Act
                var task = queue.WaitForMessageAsync<ConcreteMsgA>(TestWaitTimeout);
                queue.Enqueue(msg);
                // Assert
                Assert.Same(msg, await task);
            }

            [Fact]
            public async Task Should_return_enqueued_message_when_specified_base_type()
            {
                // Arrange
                var queue = new WaitableMessageQueue<Msg>();
                var msg = new ConcreteMsgA();
                // Act
                var task = queue.WaitForMessageAsync<Msg>(TestWaitTimeout);
                queue.Enqueue(msg);
                // Assert
                Assert.Same(msg, await task);
            }

            [Fact]
            public async Task Should_support_waiting_for_multiple_messages_of_different_type()
            {
                // Arrange
                var queue = new WaitableMessageQueue<Msg>();
                var msg1 = new ConcreteMsgA();
                var msg2 = new ConcreteMsgB();
                // Act
                var task1 = queue.WaitForMessageAsync<ConcreteMsgA>(TestWaitTimeout);
                var task2 = queue.WaitForMessageAsync<ConcreteMsgB>(TestWaitTimeout);
                queue.Enqueue(msg1);
                queue.Enqueue(msg2);
                // Assert
                await Task.WhenAll(task1, task2);
                Assert.Same(msg1, await task1);
                Assert.Same(msg2, await task2);
            }

            [Fact]
            public async Task Should_support_waiting_for_multiple_messages_of_the_same_type()
            {
                // Arrange
                var queue = new WaitableMessageQueue<Msg>();
                var msg1 = new ConcreteMsgA();
                var msg2 = new ConcreteMsgA();
                // Act
                var task1 = queue.WaitForMessageAsync<ConcreteMsgA>(TestWaitTimeout);
                var task2 = queue.WaitForMessageAsync<ConcreteMsgA>(TestWaitTimeout);
                queue.Enqueue(msg1);
                queue.Enqueue(msg2);
                // Assert
                await Task.WhenAll(task1, task2);
                Assert.Same(msg1, await task1);
                Assert.Same(msg2, await task2);
            }

            [Fact]
            public async Task Should_throw_exception_when_message_of_of_wrong_type_is_enqueued()
            {
                // Arrange
                var queue = new WaitableMessageQueue<Msg>();
                var msg = new ConcreteMsgA();
                // Act
                var task = queue.WaitForMessageAsync<ConcreteMsgB>(TimeSpan.FromSeconds(0.5));
                queue.Enqueue(msg);
                // Assert
                var e = await Assert.ThrowsAsync<FailException>(async () => await task);
                Assert.Matches(@"No .+ message is received during .+ timeout\.", e.Message);
            }

            [Fact]
            public async Task Should_throw_exception_when_no_message_is_enqueued()
            {
                // Arrange
                var queue = new WaitableMessageQueue<Msg>();
                // Act
                var task = queue.WaitForMessageAsync<ConcreteMsgA>(TimeSpan.FromSeconds(0.5));
                // Assert
                var e = await Assert.ThrowsAsync<FailException>(async () => await task);
                Assert.Matches(@"No .+ message is received during .+ timeout\.", e.Message);
            }

            [Fact]
            public async Task Should_throw_exception_when_message_is_enqueued_too_late()
            {
                // Arrange
                var queue = new WaitableMessageQueue<Msg>();
                var msg = new ConcreteMsgA();
                // Act
                var task = queue.WaitForMessageAsync<ConcreteMsgA>(TimeSpan.FromSeconds(0.5));
                await Task.Delay(TimeSpan.FromSeconds(0.6), TestContext.Current.CancellationToken);
                queue.Enqueue(msg);
                // Assert
                var e = await Assert.ThrowsAsync<FailException>(async () => await task);
                Assert.Matches(@"No .+ message is received during .+ timeout\.", e.Message);
            }
        }

        public class WaitForNonEmptinessAsyncTests
        {
            [Fact]
            public async Task Should_return_successfully_when_any_message_arrived_to_queue()
            {
                // Arrange
                var queue = new WaitableMessageQueue<Msg>();
                var msg = new ConcreteMsgA();
                // Act
                var task = queue.WaitForNonEmptinessAsync(TestWaitTimeout);
                queue.Enqueue(msg);
                // Assert
                await task; // absence of exception means success
            }

            [Fact]
            public async Task Should_return_successfully_when_several_messages_arrived_to_queue()
            {
                // Arrange
                var queue = new WaitableMessageQueue<Msg>();
                var msg1 = new ConcreteMsgA();
                var msg2 = new ConcreteMsgB();
                // Act
                var task = queue.WaitForNonEmptinessAsync(TestWaitTimeout);
                queue.Enqueue(msg1);
                queue.Enqueue(msg2);
                // Assert
                await task; // absence of exception means success
            }

            [Fact]
            public async Task Should_throw_exception_when_no_message_is_enqueued()
            {
                // Arrange
                var queue = new WaitableMessageQueue<Msg>();
                // Act
                var task = queue.WaitForNonEmptinessAsync(TimeSpan.FromSeconds(0.5));
                // Assert
                var e = await Assert.ThrowsAsync<FailException>(async () => await task);
                Assert.Matches(@"No message is received during .+ timeout\.", e.Message);
            }

            [Fact]
            public async Task Should_throw_exception_when_message_is_enqueued_too_late()
            {
                // Arrange
                var queue = new WaitableMessageQueue<Msg>();
                var msg = new ConcreteMsgA();
                // Act
                var task = queue.WaitForNonEmptinessAsync(TimeSpan.FromSeconds(0.5));
                await Task.Delay(TimeSpan.FromSeconds(0.6), TestContext.Current.CancellationToken);
                queue.Enqueue(msg);
                // Assert
                var e = await Assert.ThrowsAsync<FailException>(async () => await task);
                Assert.Matches(@"No message is received during .+ timeout\.", e.Message);
            }
        }

        internal class Msg { }
        internal class ConcreteMsgA : Msg
        {
            public int Prop1 { get; set; }
        }
        internal class ConcreteMsgB : Msg { }
    }
}
