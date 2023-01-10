/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.ValueAdd.Common;

namespace LSEG.Eta.ValueAdd.Reactor
{
    internal class ReactorPool
    {
        private const int DEFAULT_POOL_LIMIT = -1;

        private VaPool m_ReactorChannelPool = new VaPool(true);
        private VaLimitedPool m_ReactorChannelEventPool = new VaLimitedPool(true);
        private VaLimitedPool m_ReactorEventImplPool = new VaLimitedPool(true);
        private VaLimitedPool m_ReactorMsgEventPool = new VaLimitedPool(true);
        private VaLimitedPool m_ReactorRDMLoginMsgEventPool = new VaLimitedPool(true);
        private VaLimitedPool m_ReactorRDMDirectoryMsgEventPool = new VaLimitedPool(true);
        private VaLimitedPool m_ReactorRDMDictionaryMsgEventPool = new VaLimitedPool(true);
        private VaLimitedPool m_ReactorServiceEndpointEvent = new VaLimitedPool(true);

        /// <summary>
        /// Sets maximum number of events in m_ReactorChannelEventPool, if value is negative then amount of events is unlimited
        /// </summary>
        /// <param name="limit">the limit to set</param>
        public void SetReactorChannelEventPoolLimit(int limit)
            => m_ReactorChannelEventPool.SetLimit(limit > 0 ? limit : DEFAULT_POOL_LIMIT);

        /// <summary>
        /// Sets maximum number of events in m_ReactorEventImplPool, if value is negative then amount of events is unlimited
        /// </summary>
        /// <param name="limit">the limit to set</param>
        public void SetReactorEventImplPoolLimit(int limit)
            => m_ReactorEventImplPool.SetLimit(limit > 0 ? limit : DEFAULT_POOL_LIMIT);

        /// <summary>
        /// Sets maximum number of events in m_ReactorRDMDictionaryMsgEventPool, if value is negative then amount of events is unlimited
        /// </summary>
        /// <param name="limit">the limit to set</param>
        public void SetReactorRDMDirectoryMsgEventPoolLimit(int limit)
            => m_ReactorRDMDirectoryMsgEventPool.SetLimit(limit > 0 ? limit : DEFAULT_POOL_LIMIT);

        /// <summary>
        /// Sets maximum number of events in m_ReactorRDMDictionaryMsgEventPool, if value is negative then amount of events is unlimited
        /// </summary>
        /// <param name="limit">the limit to set</param>
        public void SetReactorRDMDictionaryMsgEventPoolLimit(int limit)
            => m_ReactorRDMDictionaryMsgEventPool.SetLimit(limit > 0 ? limit : DEFAULT_POOL_LIMIT);

        public void InitReactorChannelEventPool(int size)
        {
            for(int i = 0; i < size; i++)
            {
                var reactorChannelEvent = new ReactorChannelEvent();
                m_ReactorChannelEventPool.UpdatePool(reactorChannelEvent);
            }
        }

        public ReactorChannelEvent CreateReactorChannelEvent()
        {
            ReactorChannelEvent? reactorChannelEvent = (ReactorChannelEvent?)m_ReactorChannelEventPool.Poll();
            if(reactorChannelEvent is null)
            {
                reactorChannelEvent = new ReactorChannelEvent();
                m_ReactorChannelEventPool.UpdatePool(reactorChannelEvent);
            }
            else
            {
                reactorChannelEvent.Clear();
            }

            return reactorChannelEvent;
        }

        public void InitReactorEventImplPool(int size)
        {
            for (int i = 0; i < size; i++)
            {
                var reactorEventImpl = new ReactorEventImpl();
                m_ReactorEventImplPool.UpdatePool(reactorEventImpl);
            }
        }

        public ReactorEventImpl CreateReactorEventImpl()
        {
            ReactorEventImpl? reactorEventImpl = (ReactorEventImpl?)m_ReactorEventImplPool.Poll();
            if (reactorEventImpl is null)
            {
                reactorEventImpl = new ReactorEventImpl();
                m_ReactorEventImplPool.UpdatePool(reactorEventImpl);
            }
            else
            {
                reactorEventImpl.Clear();
            }

            return reactorEventImpl;
        }

        public void InitReactorChannelPool(int size)
        {
            for (int i = 0; i < size; i++)
            {
                var reactorChannel = new ReactorChannel();
                m_ReactorChannelPool.UpdatePool(reactorChannel);
            }
        }

        public ReactorChannel CreateReactorChannel()
        {
            ReactorChannel? reactorChannel = (ReactorChannel?)m_ReactorChannelPool.Poll();
            if(reactorChannel is null)
            {
                reactorChannel = new ReactorChannel();
                m_ReactorChannelPool.UpdatePool(reactorChannel);
            }
            else
            {
                reactorChannel.Clear();
            }

            return reactorChannel;
        }

        public void InitReactorMsgEventImplPool(int size)
        {
            for (int i = 0; i < size; i++)
            {
                var reactorEventImpl = new ReactorMsgEvent();
                m_ReactorMsgEventPool.UpdatePool(reactorEventImpl);
            }
        }

        public ReactorMsgEvent CreateReactorMsgEventImpl()
        {
            ReactorMsgEvent? reactorEventImpl = m_ReactorMsgEventPool.Poll() as ReactorMsgEvent;

            if (reactorEventImpl is null)
            {
                reactorEventImpl = new ReactorMsgEvent();
                m_ReactorMsgEventPool.UpdatePool(reactorEventImpl);
            }
            else
            {
                reactorEventImpl.Clear();
            }

            return reactorEventImpl;
        }

        public void InitReactorRDMLoginMsgEventImplPool(int size)
        {
            for (int i = 0; i < size; i++)
            {
                var reactorEventImpl = new RDMLoginMsgEvent();
                m_ReactorRDMLoginMsgEventPool.UpdatePool(reactorEventImpl);
            }
        }

        public RDMLoginMsgEvent CreateReactorRDMLoginMsgEventImpl()
        {
            RDMLoginMsgEvent? reactorEventImpl = m_ReactorRDMLoginMsgEventPool.Poll() as RDMLoginMsgEvent;

            if (reactorEventImpl is null)
            {
                reactorEventImpl = new RDMLoginMsgEvent();
                m_ReactorRDMLoginMsgEventPool.UpdatePool(reactorEventImpl);
            }
            else
            {
                reactorEventImpl.Clear();
            }

            return reactorEventImpl;
        }

        public void InitReactorRDMDirectoryMsgEventImplPool(int size)
        {
            for (int i = 0; i < size; i++)
            {
                var reactorEventImpl = new RDMDirectoryMsgEvent();
                m_ReactorRDMDirectoryMsgEventPool.UpdatePool(reactorEventImpl);
            }
        }

        public RDMDirectoryMsgEvent CreateReactorRDMDirectoryMsgEventImpl()
        {
            RDMDirectoryMsgEvent? reactorEventImpl = (RDMDirectoryMsgEvent?)m_ReactorRDMDirectoryMsgEventPool.Poll();
            if (reactorEventImpl is null)
            {
                reactorEventImpl = new RDMDirectoryMsgEvent();
                m_ReactorRDMDirectoryMsgEventPool.UpdatePool(reactorEventImpl);
            }
            else
            {
                reactorEventImpl.Clear();
            }

            return reactorEventImpl;
        }

        public void InitReactorRDMDictionaryMsgEventImplPool(int size)
        {
            for (int i = 0; i < size; i++)
            {
                var reactorEventImpl = new RDMDictionaryMsgEvent();
                m_ReactorRDMDictionaryMsgEventPool.UpdatePool(reactorEventImpl);
            }
        }

        public RDMDictionaryMsgEvent CreateReactorRDMDictionaryMsgEventImpl()
        {
            RDMDictionaryMsgEvent? reactorEventImpl = (RDMDictionaryMsgEvent?)m_ReactorRDMDictionaryMsgEventPool.Poll();
            if (reactorEventImpl is null)
            {
                reactorEventImpl = new RDMDictionaryMsgEvent();
                m_ReactorRDMDictionaryMsgEventPool.UpdatePool(reactorEventImpl);
            }
            else
            {
                reactorEventImpl.Clear();
            }

            return reactorEventImpl;
        }

        public ReactorServiceEndpointEvent CreateReactorServiceEndpointEvent()
        {
            ReactorServiceEndpointEvent? serviceEndpointEvent = (ReactorServiceEndpointEvent?)m_ReactorServiceEndpointEvent.Poll();
            if (serviceEndpointEvent is null)
            {
                serviceEndpointEvent = new ReactorServiceEndpointEvent();
                m_ReactorServiceEndpointEvent.UpdatePool(serviceEndpointEvent);
            }
            else
            {
                serviceEndpointEvent.Clear();
            }

            return serviceEndpointEvent;
        }
    }
}
