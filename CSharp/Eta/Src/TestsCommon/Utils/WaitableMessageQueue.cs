/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025-2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Threading.Tasks;

namespace LSEG.Eta.Tests.Common.Utils
{
    /// <summary>
    /// Queue that allow waiting for new messages from another thread.
    /// </summary>
    /// <typeparam name="TBaseMsg">Base class of messages. Used to parameterize underlying queue, that stores messages.</typeparam>
    public class WaitableMessageQueue<TBaseMsg>
    {
        private ConcurrentQueue<Action<TBaseMsg>> m_OneTimeSubscriptionsQueue = new();
        private ConcurrentQueue<TBaseMsg> m_MessageQueue = new();

        public static readonly TimeSpan DefaultWaitTimeout = TimeSpan.FromSeconds(15);

        public int Count => m_MessageQueue.Count;

        public Task WaitForNonEmptinessAsync(TimeSpan timeout)
        {
            return WaitForMessageInternalAsync(
                timeout,
                msg => true,
                tryDequeueMessage: (out TBaseMsg? msg, Func<TBaseMsg, bool> predicate) =>
                {
                    return m_MessageQueue.TryPeek(out msg);
                },
                onMessageNotReceived: () =>
                    Assert.Fail($"No message is received during {timeout} timeout."));
        }

        public Task WaitForNonEmptinessAsync()
            => WaitForNonEmptinessAsync(DefaultWaitTimeout);

        public async Task<T> WaitForMessageAsync<T>(TimeSpan? timeout = null)
            where T : TBaseMsg
        {
            timeout ??= DefaultWaitTimeout;
            var task = WaitForMessageInternalAsync(
                timeout.Value,
                msg => msg is T,
                tryDequeueMessage: (out TBaseMsg? msg, Func<TBaseMsg, bool> predicate) =>
                {
                    if (m_MessageQueue.TryPeek(out msg) && predicate(msg))
                    {
                        m_MessageQueue.TryDequeue(out _);
                        return true;
                    }
                    msg = default;
                    return false;
                },
                onMessageNotReceived: () =>
                    Assert.Fail($"No {typeof(T).Name} message is received during {timeout} timeout.{NewLine}" +
                        $"Message queue contents:{NewLine}" +
                        string.Join(NewLine, m_MessageQueue)));
            return ((T)(await task)!)!;
        }

        public T WaitForMessage<T>(TimeSpan? timeout = null)
            where T : TBaseMsg
            => WaitForMessageAsync<T>(timeout).GetAwaiter().GetResult();

        private async Task<TBaseMsg> WaitForMessageInternalAsync(
            TimeSpan timeout,
            Func<TBaseMsg, bool> predicate,
            TryDequeueMessageDelegate tryDequeueMessage,
            Action onMessageNotReceived)
        {
            if (tryDequeueMessage(out var message, predicate))
                return message!;

            var subscription = new TaskCompletionSource();
            m_OneTimeSubscriptionsQueue.Enqueue(msg =>
            {
                if (predicate(msg))
                {
                    subscription.TrySetResult();
                }
            });

            var subscriptionTask = subscription.Task;
            await Task.WhenAny(subscriptionTask, Task.Delay(timeout));

            if (!tryDequeueMessage(out message, predicate))
            {
                onMessageNotReceived();
            }

            return message!;
        }
        private delegate bool TryDequeueMessageDelegate(out TBaseMsg? msg, Func<TBaseMsg, bool> predicate);

        public void Enqueue<T>(T msg)
            where T : TBaseMsg
        {
            m_MessageQueue.Enqueue(msg);
            if (m_OneTimeSubscriptionsQueue.TryPeek(out var subscription))
            {
                subscription(msg);
                m_OneTimeSubscriptionsQueue.TryDequeue(out _);
            }
        }

        public TBaseMsg Dequeue() =>
            m_MessageQueue.TryDequeue(out var result)
                ? result
                : throw new InvalidOperationException($"Cannot extract message from queue. Queue size is {m_MessageQueue.Count}.");

    }
}
