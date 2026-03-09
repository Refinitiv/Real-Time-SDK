/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using LSEG.Eta.ValueAdd.Rdm;
using LSEG.Eta.ValueAdd.Reactor;
using System;
using Xunit;

namespace LSEG.Eta.Tests.ValueAddTest
{
    public class TestReactorSession : IDisposable
    {
        private readonly Consumer m_Consumer;
        private readonly Provider m_Provider;
        private bool m_ExpectConsumerFailure = false;

        private TestReactorSession(Consumer consumer, Provider provider)
        {
            m_Consumer = consumer;
            m_Provider = provider;
        }

        public TestReactorSession ExpectConsumerFailure(bool value)
        {
            m_ExpectConsumerFailure = value;
            return this;
        }

        public void Dispose()
        {
            CloseSession(m_Consumer, m_Provider, m_ExpectConsumerFailure);
        }

        /// <summary>
        /// Connects a Consumer and Provider component to each other.
        /// </summary>
        public static TestReactorSession OpenSession(Consumer consumer, Provider provider, ConsumerProviderSessionOptions opts, bool recoveringChannel = false)
        {
            TestReactorEvent evt;
            ReactorChannelEvent channelEvent;
            RDMLoginMsgEvent loginMsgEvent;
            RDMDirectoryMsgEvent directoryMsgEvent;
            ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
            ReactorSubmitOptions submitOptions = new();

            var result = new TestReactorSession(consumer, provider);

            if (!recoveringChannel)
                consumer.TestReactor.Connect(opts, consumer, provider.ServerPort);

            /* Preset login message required if automatically setting up login stream. */
            Assert.True(opts.SetupDefaultLoginStream == false || consumerRole.RdmLoginRequest != null);

            /* Preset directory message required, or watchlist must be enabled, if automatically setting up directory stream */
            Assert.True(opts.SetupDefaultDirectoryStream == false
                || consumerRole.WatchlistOptions.EnableWatchlist == true
                || consumerRole.RdmDirectoryRequest != null);

            if (consumerRole.WatchlistOptions.EnableWatchlist)
            {
                foreach (var component in consumer.TestReactor.ComponentList)
                {
                    if (opts.LoginHandler != null)
                        component.ReactorChannel.Watchlist.LoginHandler = opts.LoginHandler;
                    if (opts.DirectoryHandler != null)
                        component.ReactorChannel.Watchlist.DirectoryHandler = opts.DirectoryHandler;
                    if (opts.ItemHandler != null)
                        component.ReactorChannel.Watchlist.ItemHandler = opts.ItemHandler;
                }
            }
            /* If watchlist enabled, should get ChannelOpenCallback */
            if (consumerRole.WatchlistOptions.EnableWatchlist
                && consumerRole.WatchlistOptions.ChannelOpenEventCallback != null
                && recoveringChannel == false)
            {
                evt = consumer.TestReactor.PollEvent();
                Assert.Equal(TestReactorEventType.CHANNEL_EVENT, evt.EventType);
                channelEvent = (ReactorChannelEvent)evt.ReactorEvent;
                Assert.Equal(ReactorChannelEventType.CHANNEL_OPENED, channelEvent.EventType);
            }
            else
            {
                consumer.TestReactor.Dispatch(0);
            }

            provider.TestReactor.Accept(opts, provider);

            /* Provider receives channel-up/channel-ready */
            if (recoveringChannel)
            {
                provider.TestReactor.Dispatch(2, TimeSpan.FromSeconds(6));
            }
            else
            {
                provider.TestReactor.Dispatch(2);
            }

            evt = provider.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.CHANNEL_EVENT, evt.EventType);
            channelEvent = (ReactorChannelEvent)evt.ReactorEvent;
            Assert.Equal(ReactorChannelEventType.CHANNEL_UP, channelEvent.EventType);

            evt = provider.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.CHANNEL_EVENT, evt.EventType);
            channelEvent = (ReactorChannelEvent)evt.ReactorEvent;
            Assert.Equal(ReactorChannelEventType.CHANNEL_READY, channelEvent.EventType);

            /* Consumer receives channel-up and any status events due the watchlist
             * items submitted in channel open callback. */

            if (consumerRole.RdmLoginRequest == null)
                consumer.TestReactor.Dispatch(2 + opts.NumStatusEvents);
            else
                consumer.TestReactor.Dispatch(1 + opts.NumStatusEvents);
            for (int i = 0; i < opts.NumStatusEvents; i++)
            {
                evt = consumer.TestReactor.PollEvent();
                Assert.Equal(TestReactorEventType.MSG, evt.EventType);
                ReactorMsgEvent msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
                Assert.Equal(MsgClasses.STATUS, msgEvent.Msg.MsgClass);
            }
            evt = consumer.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.CHANNEL_EVENT, evt.EventType);
            channelEvent = (ReactorChannelEvent)evt.ReactorEvent;
            Assert.Equal(ReactorChannelEventType.CHANNEL_UP, channelEvent.EventType);

            if (consumerRole.RdmLoginRequest == null)
            {
                /* Consumer receives channel-ready, then we're done. */
                //consumer.TestReactor.Dispatch(1);
                evt = consumer.TestReactor.PollEvent();
                Assert.Equal(TestReactorEventType.CHANNEL_EVENT, evt.EventType);
                channelEvent = (ReactorChannelEvent)evt.ReactorEvent;
                Assert.Equal(ReactorChannelEventType.CHANNEL_READY, channelEvent.EventType);

                provider.TestReactor.Dispatch(0);

                return result;
            }

            if (!opts.SetupDefaultLoginStream)
                return result;

            /* Provider receives login request. */
            provider.TestReactor.Dispatch(1);
            evt = provider.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.LOGIN_MSG, evt.EventType);
            loginMsgEvent = (RDMLoginMsgEvent)evt.ReactorEvent;
            Assert.Equal(LoginMsgType.REQUEST, loginMsgEvent.LoginMsg.LoginMsgType);

            /* Provider sends a default login refresh. */
            LoginRequest loginRequest = new();
            LoginRefresh loginRefresh = new();

            loginRefresh.Clear();
            loginRefresh.Solicited = true;
            loginRefresh.UserNameType = loginRequest.UserNameType;
            loginRefresh.HasUserName = !string.IsNullOrEmpty(loginRequest.UserName.ToString());
            loginRefresh.UserName = loginRequest.UserName;
            loginRefresh.StreamId = loginRequest.StreamId;
            loginRefresh.HasFeatures = true;
            loginRefresh.SupportedFeatures.HasSupportOptimizedPauseResume = true;
            loginRefresh.SupportedFeatures.SupportOptimizedPauseResume = 1;
            loginRefresh.SupportedFeatures.HasSupportViewRequests = true;
            loginRefresh.SupportedFeatures.SupportViewRequests = 1;
            loginRefresh.SupportedFeatures.HasSupportPost = true;
            loginRefresh.SupportedFeatures.SupportOMMPost = 1;

            // required for RequestSymbolListTest_Socket to correctly mimic real-life scenario
            loginRefresh.SupportedFeatures.HasSupportEnhancedSymbolList = true;
            loginRefresh.SupportedFeatures.SupportEnhancedSymbolList = 1;

            loginRefresh.State.StreamState(StreamStates.OPEN);
            loginRefresh.State.DataState(DataStates.OK);
            loginRefresh.State.Code(StateCodes.NONE);
            loginRefresh.State.Text().Data("Login OK");

            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCode.SUCCESS);

            /* Consumer receives login refresh. */
            if (consumerRole.RdmDirectoryRequest == null && consumerRole.WatchlistOptions.EnableWatchlist == false)
                consumer.TestReactor.Dispatch(2);
            else
                consumer.TestReactor.Dispatch(1);

            /* Save the stream ID used by each component to open the login stream (may be
             * different if the watchlist is enabled). */
            consumer.DefaultSessionLoginStreamId = consumerRole.RdmLoginRequest.StreamId;
            provider.DefaultSessionLoginStreamId = loginRequest.StreamId;

            evt = consumer.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.LOGIN_MSG, evt.EventType);
            loginMsgEvent = (RDMLoginMsgEvent)evt.ReactorEvent;
            Assert.Equal(LoginMsgType.REFRESH, loginMsgEvent.LoginMsg.LoginMsgType);

            if (consumerRole.RdmDirectoryRequest == null && consumerRole.WatchlistOptions.EnableWatchlist == false)
            {
                /* Consumer receives channel-ready. */
                evt = consumer.TestReactor.PollEvent();
                Assert.Equal(TestReactorEventType.CHANNEL_EVENT, evt.EventType);
                channelEvent = (ReactorChannelEvent)evt.ReactorEvent;
                Assert.Equal(ReactorChannelEventType.CHANNEL_READY, channelEvent.EventType);

                provider.TestReactor.Dispatch(0);

                /* when watchlist is not enabled, no directory exchange occurs. We're done. */
                if (consumerRole.WatchlistOptions.EnableWatchlist == false)
                {
                    return result;
                }
            }

            if (opts.SetupDefaultDirectoryStream == false && consumerRole.WatchlistOptions.EnableWatchlist == false)
                return result;

            /* Provider receives directory request. */
            provider.TestReactor.Dispatch(1);
            evt = provider.TestReactor.PollEvent();
            Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
            directoryMsgEvent = (RDMDirectoryMsgEvent)evt.ReactorEvent;
            Assert.Equal(DirectoryMsgType.REQUEST, directoryMsgEvent.DirectoryMsg.DirectoryMsgType);

            /* Provider sends a default directory refresh. */
            DirectoryRequest directoryRequest = directoryMsgEvent.DirectoryMsg.DirectoryRequest;
            DirectoryRefresh directoryRefresh = new();

            directoryRefresh.Clear();
            directoryRefresh.StreamId = directoryRequest.StreamId;
            directoryRefresh.Filter = directoryRequest.Filter;
            directoryRefresh.Solicited = true;
            directoryRefresh.ClearCache = true;
            directoryRefresh.State.StreamState(StreamStates.OPEN);
            directoryRefresh.State.DataState(DataStates.OK);
            directoryRefresh.State.Code(StateCodes.NONE);
            directoryRefresh.State.Text().Data("Source Directory Refresh Complete");

            Service service = new();
            Service service2 = new();
            Provider.DefaultService.Copy(service);
            Provider.DefaultService2.Copy(service2);

            // Apply OpenWindow to the service if one is specified.
            if (opts.OpenWindow >= 0)
            {
                service.HasLoad = true;
                service.Load.HasOpenWindow = true;
                service.Load.OpenWindow = opts.OpenWindow;
            }

            directoryRefresh.ServiceList.Add(service);
            if (opts.SetupSecondDefaultDirectoryStream)
            {
                directoryRefresh.ServiceList.Add(service2);
            }
            submitOptions.Clear();
            Assert.True(provider.SubmitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCode.SUCCESS);

            if (opts.SetupDefaultDirectoryStream == true)
                consumer.TestReactor.Dispatch(2);
            else
                consumer.TestReactor.Dispatch(1);

            /* Consumer receives directory refresh. */
            evt = consumer.TestReactor.PollEvent();
            if (opts.SetupDefaultDirectoryStream == true)
            {
                Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
                directoryMsgEvent = (RDMDirectoryMsgEvent)evt.ReactorEvent;
                if (!recoveringChannel || consumerRole.WatchlistOptions.EnableWatchlist == false)
                    Assert.Equal(DirectoryMsgType.REFRESH, directoryMsgEvent.DirectoryMsg.DirectoryMsgType);
                else
                    Assert.Equal(DirectoryMsgType.UPDATE, directoryMsgEvent.DirectoryMsg.DirectoryMsgType);

                /* Save the stream ID used by each component to open the directory stream (may
                 * be different if the watchlist is enabled). */
                consumer.DefaultSessionDirectoryStreamId = consumerRole.RdmDirectoryRequest.StreamId;
                provider.DefaultSessionDirectoryStreamId = directoryRequest.StreamId;

                /* Consumer receives channel-ready. */
                evt = consumer.TestReactor.PollEvent();
                Assert.Equal(TestReactorEventType.CHANNEL_EVENT, evt.EventType);
                channelEvent = (ReactorChannelEvent)evt.ReactorEvent;
                Assert.Equal(ReactorChannelEventType.CHANNEL_READY, channelEvent.EventType);
            }
            else // only channel event comes in this case
            {
                /* Consumer receives channel-ready. */
                Assert.Equal(TestReactorEventType.CHANNEL_EVENT, evt.EventType);
                channelEvent = (ReactorChannelEvent)evt.ReactorEvent;
                Assert.Equal(ReactorChannelEventType.CHANNEL_READY, channelEvent.EventType);
            }

            return result;
        }

        /// <summary>
        /// Disconnect a consumer and provider component and clean them up.
        /// Do additional checks to not fail on dirty client disonnection.
        /// <para>Instead of calling this method directly it's recommended to dispose value returned from <see cref="OpenSession(Consumer, Provider, ConsumerProviderSessionOptions, bool)"/>.</para>
        /// </summary>
        public static void CloseSession(Consumer consumer, Provider provider, bool expectConsumerFailure = false)
        {
            /* Make sure there's nothing left in the dispatch queue. */
            consumer.TestReactor.Dispatch(0, expectConsumerFailure);
            provider.TestReactor.Dispatch(0);

            consumer.Dispose();
            provider.Dispose();
        }
    }
}
