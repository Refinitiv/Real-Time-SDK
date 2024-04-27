/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using LSEG.Eta.Rdm;
using LSEG.Eta.ValueAdd.Rdm;
using LSEG.Eta.ValueAdd.Reactor;
using System;
using System.Collections.Generic;
using Xunit;
using static LSEG.Eta.Rdm.Directory;

namespace LSEG.Eta.ValuedAdd.Tests
{
    public class WlDirectoryHandlerTests
    {
        const long ALL_FILTERS = ServiceFilterFlags.INFO | ServiceFilterFlags.STATE | ServiceFilterFlags.GROUP |
            ServiceFilterFlags.DATA | ServiceFilterFlags.LINK | ServiceFilterFlags.LOAD;

        [Fact]
        public void WatchlistServiceCacheAddUpdateDeleteTest()
        {
            ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
            submitOptions.Clear();

            SetUp(out var consumer, out var provider, out var consumerReactor, out var providerReactor);

            int servicesAdded = 0;
            int servicesDeleted = 0;
            int servicesUpdated = 0;

            consumer.ReactorChannel.Watchlist.DirectoryHandler.ServiceCache.ServiceRemovedCallback += (s, val) => servicesDeleted++;
            consumer.ReactorChannel.Watchlist.DirectoryHandler.ServiceCache.ServiceAddedCallback += s => servicesAdded++;
            consumer.ReactorChannel.Watchlist.DirectoryHandler.ServiceCache.ServiceUpdatedCallback += s => servicesUpdated++;

            consumer.ReactorChannel.Watchlist.DirectoryHandler.LoginStreamOpen(out _);

            // Watchlist requested a directory, provide it
            providerReactor.Dispatch(1);
            TestReactorEvent evt = providerReactor.PollEvent();

            Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
            ReactorMsgEvent msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

            RDMDirectoryMsgEvent dirEvent = msgEvent as RDMDirectoryMsgEvent;
            Assert.NotNull(dirEvent);

            DirectoryMsg dirMsg = dirEvent.DirectoryMsg;
            Assert.NotNull(dirMsg);
            Assert.Equal(DirectoryMsgType.REQUEST, dirMsg.DirectoryMsgType);

            DirectoryRequest dirRequest = dirMsg.DirectoryRequest;
            Assert.NotNull(dirRequest);

            List<String> serviceNames = new List<string>{"IDN_RDF", "ELEKTRON_DD", "QPR0V1", "QPR0V2", "NI_PUB"};
            List<int> serviceIds = new List<int> { 460, 7191, 7001, 7002, 37397 };
            List<ServiceFlags> serviceFlags = new List<ServiceFlags>() { ServiceFlags.HAS_INFO | ServiceFlags.HAS_LOAD, 
                ServiceFlags.HAS_INFO | ServiceFlags.HAS_LINK | ServiceFlags.HAS_DATA,
                ServiceFlags.HAS_INFO | ServiceFlags.HAS_STATE | ServiceFlags.HAS_DATA,
                ServiceFlags.HAS_INFO | ServiceFlags.HAS_LOAD | ServiceFlags.HAS_DATA,
                ServiceFlags.HAS_INFO | ServiceFlags.HAS_LINK | ServiceFlags.HAS_LOAD
            };
            List<MapEntryActions> actions = new List<MapEntryActions> { MapEntryActions.ADD, MapEntryActions.ADD, MapEntryActions.ADD, MapEntryActions.ADD, MapEntryActions.ADD };

            var directoryRefresh = CreateDirectoryRefresh(serviceNames, serviceIds, serviceFlags, actions, true, ALL_FILTERS, false, StreamStates.OPEN, DataStates.OK);

            Msg msg = new Msg();
            Assert.Equal(CodecReturnCode.SUCCESS, consumer.ReactorChannel.Watchlist.ConvertRDMToCodecMsg(directoryRefresh, msg));
            provider.SubmitAndDispatch(msg, submitOptions);

            consumerReactor.Dispatch(0);

            Assert.Equal(serviceNames.Count, servicesAdded);

            var serviceCache = consumer.ReactorChannel.Watchlist.DirectoryHandler.ServiceCache;
            Assert.NotNull(serviceCache.Service(460));
            Assert.Equal(MapEntryActions.ADD, serviceCache.Service(460).RdmService.Action);
            Assert.NotNull(serviceCache.Service("IDN_RDF"));
            Assert.Equal(MapEntryActions.ADD, serviceCache.Service("IDN_RDF").RdmService.Action);
            Assert.NotNull(serviceCache.Service(7191));
            Assert.Equal(MapEntryActions.ADD, serviceCache.Service(7191).RdmService.Action);
            Assert.NotNull(serviceCache.Service("ELEKTRON_DD"));
            Assert.Equal(MapEntryActions.ADD, serviceCache.Service("ELEKTRON_DD").RdmService.Action);
            Assert.NotNull(serviceCache.Service(7001));
            Assert.Equal(MapEntryActions.ADD, serviceCache.Service(7001).RdmService.Action);
            Assert.NotNull(serviceCache.Service("QPR0V1"));
            Assert.Equal(MapEntryActions.ADD, serviceCache.Service("QPR0V1").RdmService.Action);
            Assert.NotNull(serviceCache.Service(7002));
            Assert.Equal(MapEntryActions.ADD, serviceCache.Service(7002).RdmService.Action);
            Assert.NotNull(serviceCache.Service("QPR0V2"));
            Assert.Equal(MapEntryActions.ADD, serviceCache.Service("QPR0V2").RdmService.Action);
            Assert.NotNull(serviceCache.Service(37397));
            Assert.Equal(MapEntryActions.ADD, serviceCache.Service(37397).RdmService.Action);
            Assert.NotNull(serviceCache.Service("NI_PUB"));
            Assert.Equal(MapEntryActions.ADD, serviceCache.Service("NI_PUB").RdmService.Action);

            // verify service name can be retrieved by service id and vice-versa
            Assert.Equal("IDN_RDF", serviceCache.ServiceName(460));
            Assert.Equal(460, serviceCache.ServiceId("IDN_RDF"));
            Assert.Equal("ELEKTRON_DD", serviceCache.ServiceName(7191));
            Assert.Equal(7191, serviceCache.ServiceId("ELEKTRON_DD"));
            Assert.Equal("QPR0V1", serviceCache.ServiceName(7001));
            Assert.Equal(7001, serviceCache.ServiceId("QPR0V1"));
            Assert.Equal("QPR0V2", serviceCache.ServiceName(7002));
            Assert.Equal(7002, serviceCache.ServiceId("QPR0V2"));
            Assert.Equal("NI_PUB", serviceCache.ServiceName(37397));
            Assert.Equal(37397, serviceCache.ServiceId("NI_PUB"));

            directoryRefresh = CreateDirectoryRefresh(new List<String> { "NI_PUB" }, 
                new List<int> { 37397 }, 
                new List<ServiceFlags> { ServiceFlags.HAS_INFO | ServiceFlags.HAS_LINK | ServiceFlags.HAS_LOAD },
                new List<MapEntryActions> { MapEntryActions.DELETE }, false, 7, false, StreamStates.OPEN, DataStates.OK);

            msg.Clear();
            submitOptions.Clear();
            Assert.Equal(CodecReturnCode.SUCCESS, consumer.ReactorChannel.Watchlist.ConvertRDMToCodecMsg(directoryRefresh, msg));
            Assert.Equal(ReactorReturnCode.SUCCESS, provider.SubmitAndDispatch(msg, submitOptions));

            consumerReactor.Dispatch(0);

            Assert.Equal(1, servicesDeleted);
            Assert.Null(serviceCache.Service("NI_PUB"));
            Assert.Null(serviceCache.ServiceName(37397));
            Assert.Equal((int)ReactorReturnCode.PARAMETER_INVALID, serviceCache.ServiceId("NI_PUB"));

            Service serviceByNameBeforeUpdate = new Service();
            serviceCache.Service("IDN_RDF").RdmService.Copy(serviceByNameBeforeUpdate);
            Service serviceByIdBeforeUpdate = new Service();
            serviceCache.Service(460).RdmService.Copy(serviceByIdBeforeUpdate);

            directoryRefresh = CreateDirectoryRefresh(new List<String> { "IDN_RDF" },
                new List<int> { 460 },
                new List<ServiceFlags> { ServiceFlags.HAS_INFO | ServiceFlags.HAS_LOAD | ServiceFlags.HAS_LINK | ServiceFlags.HAS_DATA },
                new List<MapEntryActions> { MapEntryActions.UPDATE }, false, 7, false, StreamStates.OPEN, DataStates.OK);

            msg.Clear();
            submitOptions.Clear();
            Assert.Equal(CodecReturnCode.SUCCESS, consumer.ReactorChannel.Watchlist.ConvertRDMToCodecMsg(directoryRefresh, msg));
            Assert.Equal(ReactorReturnCode.SUCCESS, provider.SubmitAndDispatch(msg, submitOptions));

            consumerReactor.Dispatch(0);

            Assert.Equal(1, servicesUpdated);
            var serviceByIdAfterUpdate = serviceCache.Service(460);
            Assert.Equal(ServiceFlags.HAS_INFO | ServiceFlags.HAS_LOAD | ServiceFlags.HAS_LINK | ServiceFlags.HAS_DATA, serviceByIdAfterUpdate.RdmService.Flags);
            Assert.Equal(serviceByIdBeforeUpdate.Load.LoadFactor, serviceByIdAfterUpdate.RdmService.Load.LoadFactor);
            var serviceByNameAfterUpdate = serviceCache.Service("IDN_RDF");
            Assert.Equal(ServiceFlags.HAS_INFO | ServiceFlags.HAS_LOAD | ServiceFlags.HAS_LINK | ServiceFlags.HAS_DATA, serviceByNameAfterUpdate.RdmService.Flags);
            Assert.Equal(serviceByNameBeforeUpdate.Load.LoadFactor, serviceByNameAfterUpdate.RdmService.Load.LoadFactor);

            TestReactorComponent.CloseSession(consumer, provider);
            consumerReactor.Close();
            providerReactor.Close();
        }

        [Fact]
        public void WatchlistUserDirectoryRequestAndReissueTest()
        {
            ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
            submitOptions.Clear();

            SetUp(out var consumer, out var provider, out var consumerReactor, out var providerReactor);

            int servicesAdded = 0;
            int servicesDeleted = 0;
            int servicesUpdated = 0;

            consumer.ReactorChannel.Watchlist.DirectoryHandler.ServiceCache.ServiceRemovedCallback += (s, val) => servicesDeleted++;
            consumer.ReactorChannel.Watchlist.DirectoryHandler.ServiceCache.ServiceAddedCallback += s => servicesAdded++;
            consumer.ReactorChannel.Watchlist.DirectoryHandler.ServiceCache.ServiceUpdatedCallback += s => servicesUpdated++;

            consumer.ReactorChannel.Watchlist.DirectoryHandler.LoginStreamOpen(out _);

            var directoryRequest = CreateDirectoryRequest(460, ServiceFilterFlags.INFO | ServiceFilterFlags.STATE);

            Msg msg = new Msg();
            Assert.Equal(CodecReturnCode.SUCCESS, consumer.ReactorChannel.Watchlist.ConvertRDMToCodecMsg(directoryRequest, msg));
            Assert.Equal(ReactorReturnCode.SUCCESS, consumer.Submit(msg, submitOptions));

            submitOptions.Clear();

            // Watchlist requested a directory, provide it
            providerReactor.Dispatch(1);
            TestReactorEvent evt = providerReactor.PollEvent();

            Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
            ReactorMsgEvent msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

            RDMDirectoryMsgEvent dirEvent = msgEvent as RDMDirectoryMsgEvent;
            Assert.NotNull(dirEvent);

            DirectoryMsg dirMsg = dirEvent.DirectoryMsg;
            Assert.NotNull(dirMsg);
            Assert.Equal(DirectoryMsgType.REQUEST, dirMsg.DirectoryMsgType);

            DirectoryRequest dirRequest = dirMsg.DirectoryRequest;
            Assert.NotNull(dirRequest);

            Assert.Equal(ALL_FILTERS, dirRequest.Filter);

            List<String> serviceNames = new List<string> { "IDN_RDF", "ELEKTRON_DD", "QPR0V1", "QPR0V2", "NI_PUB" };
            List<int> serviceIds = new List<int> { 460, 7191, 7001, 7002, 37397 };
            List<ServiceFlags> serviceFlags = new List<ServiceFlags>() { ServiceFlags.HAS_INFO | ServiceFlags.HAS_LOAD,
                ServiceFlags.HAS_INFO | ServiceFlags.HAS_LINK | ServiceFlags.HAS_DATA,
                ServiceFlags.HAS_INFO | ServiceFlags.HAS_STATE | ServiceFlags.HAS_DATA,
                ServiceFlags.HAS_INFO | ServiceFlags.HAS_LOAD | ServiceFlags.HAS_DATA,
                ServiceFlags.HAS_INFO | ServiceFlags.HAS_LINK | ServiceFlags.HAS_LOAD
            };
            List<MapEntryActions> actions = new List<MapEntryActions> { MapEntryActions.ADD, MapEntryActions.ADD, MapEntryActions.ADD, MapEntryActions.ADD, MapEntryActions.ADD };

            var directoryRefresh = CreateDirectoryRefresh(serviceNames, serviceIds, serviceFlags, actions, true, ALL_FILTERS, false, StreamStates.OPEN, DataStates.OK);

            msg.Clear();
            Assert.Equal(CodecReturnCode.SUCCESS, consumer.ReactorChannel.Watchlist.ConvertRDMToCodecMsg(directoryRefresh, msg));
            provider.SubmitAndDispatch(msg, submitOptions);

            consumerReactor.Dispatch(1);

            evt = consumerReactor.PollEvent();

            Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);

            dirEvent = msgEvent as RDMDirectoryMsgEvent;
            Assert.NotNull(dirEvent);

            dirMsg = dirEvent.DirectoryMsg;
            Assert.NotNull(dirMsg);
            Assert.Equal(DirectoryMsgType.REFRESH, dirMsg.DirectoryMsgType);

            DirectoryRefresh dirRefresh = dirMsg.DirectoryRefresh;
            Assert.NotNull(dirRefresh);
            Assert.Equal(directoryRequest.Filter, dirRefresh.Filter);
            Assert.Single(dirRefresh.ServiceList);
            Assert.Equal(460, dirRefresh.ServiceList[0].ServiceId);
            Assert.False(dirRefresh.ServiceList[0].HasData);
            Assert.False(dirRefresh.ServiceList[0].HasLink);
            Assert.False(dirRefresh.ServiceList[0].HasLoad);
            Assert.True(dirRefresh.ServiceList[0].HasInfo);

            directoryRequest = CreateDirectoryRequest(460, ServiceFilterFlags.LOAD);

            msg.Clear();
            submitOptions.Clear();
            Assert.Equal(CodecReturnCode.SUCCESS, consumer.ReactorChannel.Watchlist.ConvertRDMToCodecMsg(directoryRequest, msg));
            Assert.Equal(ReactorReturnCode.SUCCESS, consumer.Submit(msg, submitOptions));

            consumerReactor.Dispatch(1);

            evt = consumerReactor.PollEvent();

            Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);

            dirEvent = msgEvent as RDMDirectoryMsgEvent;
            Assert.NotNull(dirEvent);

            dirMsg = dirEvent.DirectoryMsg;
            Assert.NotNull(dirMsg);
            Assert.Equal(DirectoryMsgType.REFRESH, dirMsg.DirectoryMsgType);

            dirRefresh = dirMsg.DirectoryRefresh;
            Assert.NotNull(dirRefresh);
            Assert.Equal(directoryRequest.Filter, dirRefresh.Filter);
            Assert.False(dirRefresh.ServiceList[0].HasData);
            Assert.False(dirRefresh.ServiceList[0].HasLink);
            Assert.False(dirRefresh.ServiceList[0].HasInfo);
            Assert.False(dirRefresh.ServiceList[0].HasState);
            Assert.True(dirRefresh.ServiceList[0].HasLoad);

            Assert.Equal(5, servicesAdded);
            Assert.Equal(0, servicesUpdated);
            Assert.Equal(0, servicesDeleted);

            TestReactorComponent.CloseSession(consumer, provider);
            consumerReactor.Close();
            providerReactor.Close();
        }

        [Fact]
        public void WatchlistUserDirectoryRequestServiceSuccessAndFailTest()
        {
            ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
            submitOptions.Clear();

            SetUp(out var consumer, out var provider, out var consumerReactor, out var providerReactor);

            int servicesAdded = 0;
            int servicesDeleted = 0;
            int servicesUpdated = 0;

            consumer.ReactorChannel.Watchlist.DirectoryHandler.ServiceCache.ServiceRemovedCallback += (s, val) => servicesDeleted++;
            consumer.ReactorChannel.Watchlist.DirectoryHandler.ServiceCache.ServiceAddedCallback += s => servicesAdded++;
            consumer.ReactorChannel.Watchlist.DirectoryHandler.ServiceCache.ServiceUpdatedCallback += s => servicesUpdated++;

            consumer.ReactorChannel.Watchlist.DirectoryHandler.LoginStreamOpen(out _);

            var directoryRequest = new DirectoryRequest();
            directoryRequest.Filter = ServiceFilterFlags.INFO | ServiceFilterFlags.STATE;
            directoryRequest.StreamId = 2;

            Msg msg = new Msg();
            Assert.Equal(CodecReturnCode.SUCCESS, consumer.ReactorChannel.Watchlist.ConvertRDMToCodecMsg(directoryRequest, msg));
            submitOptions.ServiceName = "IDN_RDF";
            Assert.Equal(ReactorReturnCode.SUCCESS, consumer.Submit(msg, submitOptions));

            submitOptions.Clear();

            // Watchlist requested a directory, provide it
            providerReactor.Dispatch(1);
            TestReactorEvent evt = providerReactor.PollEvent();

            Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
            ReactorMsgEvent msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

            RDMDirectoryMsgEvent dirEvent = msgEvent as RDMDirectoryMsgEvent;
            Assert.NotNull(dirEvent);

            DirectoryMsg dirMsg = dirEvent.DirectoryMsg;
            Assert.NotNull(dirMsg);
            Assert.Equal(DirectoryMsgType.REQUEST, dirMsg.DirectoryMsgType);

            DirectoryRequest dirRequest = dirMsg.DirectoryRequest;
            Assert.NotNull(dirRequest);

            Assert.Equal(ALL_FILTERS, dirRequest.Filter);

            List<String> serviceNames = new List<string> { "IDN_RDF", "ELEKTRON_DD", "QPR0V1", "QPR0V2", "NI_PUB" };
            List<int> serviceIds = new List<int> { 460, 7191, 7001, 7002, 37397 };
            List<ServiceFlags> serviceFlags = new List<ServiceFlags>() { ServiceFlags.HAS_INFO | ServiceFlags.HAS_LOAD,
                ServiceFlags.HAS_INFO | ServiceFlags.HAS_LINK | ServiceFlags.HAS_DATA,
                ServiceFlags.HAS_INFO | ServiceFlags.HAS_STATE | ServiceFlags.HAS_DATA,
                ServiceFlags.HAS_INFO | ServiceFlags.HAS_LOAD | ServiceFlags.HAS_DATA,
                ServiceFlags.HAS_INFO | ServiceFlags.HAS_LINK | ServiceFlags.HAS_LOAD
            };
            List<MapEntryActions> actions = new List<MapEntryActions> { MapEntryActions.ADD, MapEntryActions.ADD, MapEntryActions.ADD, MapEntryActions.ADD, MapEntryActions.ADD };

            var directoryRefresh = CreateDirectoryRefresh(serviceNames, serviceIds, serviceFlags, actions, true, ALL_FILTERS, false, StreamStates.OPEN, DataStates.OK);

            msg.Clear();
            submitOptions.Clear();
            Assert.Equal(CodecReturnCode.SUCCESS, consumer.ReactorChannel.Watchlist.ConvertRDMToCodecMsg(directoryRefresh, msg));
            provider.SubmitAndDispatch(msg, submitOptions);

            consumerReactor.Dispatch(1);

            evt = consumerReactor.PollEvent();

            Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);

            dirEvent = msgEvent as RDMDirectoryMsgEvent;
            Assert.NotNull(dirEvent);

            dirMsg = dirEvent.DirectoryMsg;
            Assert.NotNull(dirMsg);
            Assert.Equal(DirectoryMsgType.REFRESH, dirMsg.DirectoryMsgType);

            DirectoryRefresh dirRefresh = dirMsg.DirectoryRefresh;
            Assert.NotNull(dirRefresh);
            Assert.Equal(directoryRequest.Filter, dirRefresh.Filter);
            Assert.Single(dirRefresh.ServiceList);
            Assert.Equal(460, dirRefresh.ServiceList[0].ServiceId);
            Assert.False(dirRefresh.ServiceList[0].HasData);
            Assert.False(dirRefresh.ServiceList[0].HasLink);
            Assert.False(dirRefresh.ServiceList[0].HasLoad);
            Assert.True(dirRefresh.ServiceList[0].HasInfo);
            
            directoryRequest = new DirectoryRequest();
            directoryRequest.StreamId = 2;
            directoryRequest.Filter = ServiceFilterFlags.INFO | ServiceFilterFlags.STATE;

            // Send failing directory request because of changing Service Name
            msg.Clear();
            submitOptions.Clear();
            submitOptions.ServiceName = "TRI";
            Assert.Equal(CodecReturnCode.SUCCESS, consumer.ReactorChannel.Watchlist.ConvertRDMToCodecMsg(directoryRequest, msg));
            Assert.Equal(ReactorReturnCode.INVALID_USAGE, consumer.Submit(msg, submitOptions, true));

            // Send failing directory request because of changing service Id
            Msg requestMsg = new Msg();
            requestMsg.DomainType = (int)DomainType.SOURCE;
            requestMsg.MsgClass = MsgClasses.REQUEST;
            requestMsg.MsgKey.ApplyHasServiceId();
            requestMsg.MsgKey.ApplyHasFilter();
            requestMsg.StreamId = 2;
            requestMsg.MsgKey.Filter = Directory.ServiceFilterFlags.GROUP | Directory.ServiceFilterFlags.LINK;
            requestMsg.MsgKey.ServiceId = 10;
            submitOptions.ServiceName = "IDN_RDF";
            Assert.Equal(ReactorReturnCode.INVALID_USAGE, consumer.Submit(requestMsg, submitOptions, true));

            Assert.Equal(5, servicesAdded);
            Assert.Equal(0, servicesUpdated);
            Assert.Equal(0, servicesDeleted);

            TestReactorComponent.CloseSession(consumer, provider);
            consumerReactor.Close();
            providerReactor.Close();
        }

        [Fact]
        public void WatchlistDirectoryDownTest()
        {
            ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
            submitOptions.Clear();

            SetUp(out var consumer, out var provider, out var consumerReactor, out var providerReactor);

            int servicesAdded = 0;
            int servicesDeleted = 0;
            int servicesUpdated = 0;

            consumer.ReactorChannel.Watchlist.DirectoryHandler.ServiceCache.ServiceRemovedCallback += (s, val) => servicesDeleted++;
            consumer.ReactorChannel.Watchlist.DirectoryHandler.ServiceCache.ServiceAddedCallback += s => servicesAdded++;
            consumer.ReactorChannel.Watchlist.DirectoryHandler.ServiceCache.ServiceUpdatedCallback += s => servicesUpdated++;

            consumer.ReactorChannel.Watchlist.DirectoryHandler.LoginStreamOpen(out _);

            var directoryRequest = new DirectoryRequest();
            directoryRequest.Filter = ServiceFilterFlags.INFO | ServiceFilterFlags.STATE;
            directoryRequest.StreamId = 2;

            Msg msg = new Msg();
            Assert.Equal(CodecReturnCode.SUCCESS, consumer.ReactorChannel.Watchlist.ConvertRDMToCodecMsg(directoryRequest, msg));
            submitOptions.ServiceName = "IDN_RDF";
            Assert.Equal(ReactorReturnCode.SUCCESS, consumer.Submit(msg, submitOptions));

            submitOptions.Clear();

            // Watchlist requested a directory, provide it
            providerReactor.Dispatch(1);
            TestReactorEvent evt = providerReactor.PollEvent();

            Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
            ReactorMsgEvent msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

            RDMDirectoryMsgEvent dirEvent = msgEvent as RDMDirectoryMsgEvent;
            Assert.NotNull(dirEvent);

            DirectoryMsg dirMsg = dirEvent.DirectoryMsg;
            Assert.NotNull(dirMsg);
            Assert.Equal(DirectoryMsgType.REQUEST, dirMsg.DirectoryMsgType);

            DirectoryRequest dirRequest = dirMsg.DirectoryRequest;
            Assert.NotNull(dirRequest);

            Assert.Equal(ALL_FILTERS, dirRequest.Filter);

            List<String> serviceNames = new List<string> { "IDN_RDF", "ELEKTRON_DD", "QPR0V1", "QPR0V2", "NI_PUB" };
            List<int> serviceIds = new List<int> { 460, 7191, 7001, 7002, 37397 };
            List<ServiceFlags> serviceFlags = new List<ServiceFlags>() { ServiceFlags.HAS_INFO | ServiceFlags.HAS_LOAD,
                ServiceFlags.HAS_INFO | ServiceFlags.HAS_LINK | ServiceFlags.HAS_DATA,
                ServiceFlags.HAS_INFO | ServiceFlags.HAS_STATE | ServiceFlags.HAS_DATA,
                ServiceFlags.HAS_INFO | ServiceFlags.HAS_LOAD | ServiceFlags.HAS_DATA,
                ServiceFlags.HAS_INFO | ServiceFlags.HAS_LINK | ServiceFlags.HAS_LOAD
            };
            List<MapEntryActions> actions = new List<MapEntryActions> { MapEntryActions.ADD, MapEntryActions.ADD, MapEntryActions.ADD, MapEntryActions.ADD, MapEntryActions.ADD };

            var directoryRefresh = CreateDirectoryRefresh(serviceNames, serviceIds, serviceFlags, actions, true, ALL_FILTERS, false, StreamStates.OPEN, DataStates.OK);

            msg.Clear();
            submitOptions.Clear();
            Assert.Equal(CodecReturnCode.SUCCESS, consumer.ReactorChannel.Watchlist.ConvertRDMToCodecMsg(directoryRefresh, msg));
            provider.SubmitAndDispatch(msg, submitOptions);

            consumerReactor.Dispatch(1);

            evt = consumerReactor.PollEvent();

            Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);

            dirEvent = msgEvent as RDMDirectoryMsgEvent;
            Assert.NotNull(dirEvent);

            dirMsg = dirEvent.DirectoryMsg;
            Assert.NotNull(dirMsg);
            Assert.Equal(DirectoryMsgType.REFRESH, dirMsg.DirectoryMsgType);

            DirectoryStatus directoryStatus = new DirectoryStatus();
            directoryStatus.StreamId = dirRequest.StreamId;
            directoryStatus.HasState = true;
            directoryStatus.State.StreamState(StreamStates.CLOSED);
            directoryStatus.State.DataState(DataStates.SUSPECT);

            msg.Clear();
            submitOptions.Clear();
            Assert.Equal(CodecReturnCode.SUCCESS, consumer.ReactorChannel.Watchlist.ConvertRDMToCodecMsg(directoryStatus, msg));
            provider.SubmitAndDispatch(msg, submitOptions);

            consumerReactor.Dispatch(1);

            evt = consumerReactor.PollEvent();

            Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.UPDATE, msgEvent.Msg.MsgClass);

            dirEvent = msgEvent as RDMDirectoryMsgEvent;
            Assert.NotNull(dirEvent);

            dirMsg = dirEvent.DirectoryMsg;
            Assert.NotNull(dirMsg);
            Assert.Equal(DirectoryMsgType.UPDATE, dirMsg.DirectoryMsgType);

            Assert.Equal(5, servicesAdded);
            Assert.Equal(0, servicesUpdated);
            Assert.Equal(5, servicesDeleted);

            TestReactorComponent.CloseSession(consumer, provider);
            consumerReactor.Close();
            providerReactor.Close();
        }

        [Fact]
        public void WatchlistUserDirectoryRequestTestSpecificDirectory()
        {
            ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
            submitOptions.Clear();

            SetUp(out var consumer, out var provider, out var consumerReactor, out var providerReactor);

            int servicesAdded = 0;
            int servicesDeleted = 0;
            int servicesUpdated = 0;

            consumer.ReactorChannel.Watchlist.DirectoryHandler.ServiceCache.ServiceRemovedCallback += (s, val) => servicesDeleted++;
            consumer.ReactorChannel.Watchlist.DirectoryHandler.ServiceCache.ServiceAddedCallback += s => servicesAdded++;
            consumer.ReactorChannel.Watchlist.DirectoryHandler.ServiceCache.ServiceUpdatedCallback += s => servicesUpdated++;

            consumer.ReactorChannel.Watchlist.DirectoryHandler.LoginStreamOpen(out _);

            var directoryRequest = new DirectoryRequest();
            directoryRequest.Filter = ServiceFilterFlags.INFO | ServiceFilterFlags.STATE;
            directoryRequest.StreamId = 2;

            Msg msg = new Msg();
            Assert.Equal(CodecReturnCode.SUCCESS, consumer.ReactorChannel.Watchlist.ConvertRDMToCodecMsg(directoryRequest, msg));
            submitOptions.ServiceName = "IDN_RDF";
            Assert.Equal(ReactorReturnCode.SUCCESS, consumer.Submit(msg, submitOptions));

            submitOptions.Clear();

            // Watchlist requested a directory, provide it
            providerReactor.Dispatch(1);
            TestReactorEvent evt = providerReactor.PollEvent();

            Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
            ReactorMsgEvent msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

            RDMDirectoryMsgEvent dirEvent = msgEvent as RDMDirectoryMsgEvent;
            Assert.NotNull(dirEvent);

            DirectoryMsg dirMsg = dirEvent.DirectoryMsg;
            Assert.NotNull(dirMsg);
            Assert.Equal(DirectoryMsgType.REQUEST, dirMsg.DirectoryMsgType);

            DirectoryRequest dirRequest = dirMsg.DirectoryRequest;
            Assert.NotNull(dirRequest);

            Assert.Equal(ALL_FILTERS, dirRequest.Filter);

            List<String> serviceNames = new List<string> { "IDN_RDF", "ELEKTRON_DD", "QPR0V1", "QPR0V2", "NI_PUB" };
            List<int> serviceIds = new List<int> { 460, 7191, 7001, 7002, 37397 };
            List<ServiceFlags> serviceFlags = new List<ServiceFlags>() { ServiceFlags.HAS_INFO | ServiceFlags.HAS_LOAD,
                ServiceFlags.HAS_INFO | ServiceFlags.HAS_LINK | ServiceFlags.HAS_DATA,
                ServiceFlags.HAS_INFO | ServiceFlags.HAS_STATE | ServiceFlags.HAS_DATA,
                ServiceFlags.HAS_INFO | ServiceFlags.HAS_LOAD | ServiceFlags.HAS_DATA,
                ServiceFlags.HAS_INFO | ServiceFlags.HAS_LINK | ServiceFlags.HAS_LOAD
            };
            List<MapEntryActions> actions = new List<MapEntryActions> { MapEntryActions.ADD, MapEntryActions.ADD, MapEntryActions.ADD, MapEntryActions.ADD, MapEntryActions.ADD };

            var directoryRefresh = CreateDirectoryRefresh(serviceNames, serviceIds, serviceFlags, actions, true, ALL_FILTERS, false, StreamStates.OPEN, DataStates.OK);

            msg.Clear();
            submitOptions.Clear();
            Assert.Equal(CodecReturnCode.SUCCESS, consumer.ReactorChannel.Watchlist.ConvertRDMToCodecMsg(directoryRefresh, msg));
            provider.SubmitAndDispatch(msg, submitOptions);

            consumerReactor.Dispatch(1);

            evt = consumerReactor.PollEvent();

            Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);

            dirEvent = msgEvent as RDMDirectoryMsgEvent;
            Assert.NotNull(dirEvent);

            dirMsg = dirEvent.DirectoryMsg;
            Assert.NotNull(dirMsg);
            Assert.Equal(DirectoryMsgType.REFRESH, dirMsg.DirectoryMsgType);

            DirectoryRefresh dirRefresh = dirMsg.DirectoryRefresh;
            Assert.NotNull(dirRefresh);
            Assert.Equal(2, dirRefresh.StreamId);
            Assert.Equal(directoryRequest.Filter, dirRefresh.Filter);
            Assert.Single(dirRefresh.ServiceList);
            Assert.Equal(460, dirRefresh.ServiceList[0].ServiceId);
            Assert.False(dirRefresh.ServiceList[0].HasData);
            Assert.False(dirRefresh.ServiceList[0].HasLink);
            Assert.False(dirRefresh.ServiceList[0].HasLoad);
            Assert.True(dirRefresh.ServiceList[0].HasInfo);
            Assert.False(dirRefresh.ServiceList[0].HasState);
            Assert.Equal("IDN_RDF", dirRefresh.ServiceList[0].Info.ServiceName.ToString());

            Assert.Equal(5, servicesAdded);
            Assert.Equal(0, servicesUpdated);
            Assert.Equal(0, servicesDeleted);

            TestReactorComponent.CloseSession(consumer, provider);
            consumerReactor.Close();
            providerReactor.Close();
        }

        [Fact]
        public void WatchlistUserDirectoryRequestBeforeLoginTest()
        {
            ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
            submitOptions.Clear();

            SetUp(out var consumer, out var provider, out var consumerReactor, out var providerReactor);

            int servicesAdded = 0;
            int servicesDeleted = 0;
            int servicesUpdated = 0;

            consumer.ReactorChannel.Watchlist.DirectoryHandler.ServiceCache.ServiceRemovedCallback += (s, val) => servicesDeleted++;
            consumer.ReactorChannel.Watchlist.DirectoryHandler.ServiceCache.ServiceAddedCallback += s => servicesAdded++;
            consumer.ReactorChannel.Watchlist.DirectoryHandler.ServiceCache.ServiceUpdatedCallback += s => servicesUpdated++;

            var directoryRequest = new DirectoryRequest();
            directoryRequest.Filter = ServiceFilterFlags.INFO | ServiceFilterFlags.STATE;
            directoryRequest.StreamId = 2;

            Msg msg = new Msg();
            Assert.Equal(CodecReturnCode.SUCCESS, consumer.ReactorChannel.Watchlist.ConvertRDMToCodecMsg(directoryRequest, msg));
            submitOptions.ServiceName = "IDN_RDF";
            Assert.Equal(ReactorReturnCode.SUCCESS, consumer.Submit(msg, submitOptions));

            consumer.ReactorChannel.Watchlist.DirectoryHandler.LoginStreamOpen(out _);      

            submitOptions.Clear();

            // Watchlist requested a directory, provide it
            providerReactor.Dispatch(1);
            TestReactorEvent evt = providerReactor.PollEvent();

            Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
            ReactorMsgEvent msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

            RDMDirectoryMsgEvent dirEvent = msgEvent as RDMDirectoryMsgEvent;
            Assert.NotNull(dirEvent);

            DirectoryMsg dirMsg = dirEvent.DirectoryMsg;
            Assert.NotNull(dirMsg);
            Assert.Equal(DirectoryMsgType.REQUEST, dirMsg.DirectoryMsgType);

            DirectoryRequest dirRequest = dirMsg.DirectoryRequest;
            Assert.NotNull(dirRequest);

            Assert.Equal(ALL_FILTERS, dirRequest.Filter);

            List<String> serviceNames = new List<string> { "IDN_RDF", "ELEKTRON_DD", "QPR0V1", "QPR0V2", "NI_PUB" };
            List<int> serviceIds = new List<int> { 460, 7191, 7001, 7002, 37397 };
            List<ServiceFlags> serviceFlags = new List<ServiceFlags>() { ServiceFlags.HAS_INFO | ServiceFlags.HAS_LOAD,
                ServiceFlags.HAS_INFO | ServiceFlags.HAS_LINK | ServiceFlags.HAS_DATA,
                ServiceFlags.HAS_INFO | ServiceFlags.HAS_STATE | ServiceFlags.HAS_DATA,
                ServiceFlags.HAS_INFO | ServiceFlags.HAS_LOAD | ServiceFlags.HAS_DATA,
                ServiceFlags.HAS_INFO | ServiceFlags.HAS_LINK | ServiceFlags.HAS_LOAD
            };
            List<MapEntryActions> actions = new List<MapEntryActions> { MapEntryActions.ADD, MapEntryActions.ADD, MapEntryActions.ADD, MapEntryActions.ADD, MapEntryActions.ADD };

            var directoryRefresh = CreateDirectoryRefresh(serviceNames, serviceIds, serviceFlags, actions, true, ALL_FILTERS, false, StreamStates.OPEN, DataStates.OK);

            msg.Clear();
            submitOptions.Clear();
            Assert.Equal(CodecReturnCode.SUCCESS, consumer.ReactorChannel.Watchlist.ConvertRDMToCodecMsg(directoryRefresh, msg));
            provider.SubmitAndDispatch(msg, submitOptions);

            consumerReactor.Dispatch(1);

            evt = consumerReactor.PollEvent();

            Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);

            dirEvent = msgEvent as RDMDirectoryMsgEvent;
            Assert.NotNull(dirEvent);

            dirMsg = dirEvent.DirectoryMsg;
            Assert.NotNull(dirMsg);
            Assert.Equal(DirectoryMsgType.REFRESH, dirMsg.DirectoryMsgType);

            DirectoryRefresh dirRefresh = dirMsg.DirectoryRefresh;
            Assert.NotNull(dirRefresh);
            Assert.Equal(2, dirRefresh.StreamId);
            Assert.Equal(directoryRequest.Filter, dirRefresh.Filter);
            Assert.Single(dirRefresh.ServiceList);
            Assert.Equal(460, dirRefresh.ServiceList[0].ServiceId);
            Assert.False(dirRefresh.ServiceList[0].HasData);
            Assert.False(dirRefresh.ServiceList[0].HasLink);
            Assert.False(dirRefresh.ServiceList[0].HasLoad);
            Assert.True(dirRefresh.ServiceList[0].HasInfo);
            Assert.Equal("IDN_RDF", dirRefresh.ServiceList[0].Info.ServiceName.ToString());

            Assert.Equal(5, servicesAdded);
            Assert.Equal(0, servicesUpdated);
            Assert.Equal(0, servicesDeleted);

            TestReactorComponent.CloseSession(consumer, provider);
            consumerReactor.Close();
            providerReactor.Close();
        }

        [Fact]
        public void WatchlistMultipleUserDirectoryRequestTest()
        {
            ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
            submitOptions.Clear();

            SetUp(out var consumer, out var provider, out var consumerReactor, out var providerReactor);

            int servicesAdded = 0;
            int servicesDeleted = 0;
            int servicesUpdated = 0;

            consumer.ReactorChannel.Watchlist.DirectoryHandler.ServiceCache.ServiceRemovedCallback += (s, val) => servicesDeleted++;
            consumer.ReactorChannel.Watchlist.DirectoryHandler.ServiceCache.ServiceAddedCallback += s => servicesAdded++;
            consumer.ReactorChannel.Watchlist.DirectoryHandler.ServiceCache.ServiceUpdatedCallback += s => servicesUpdated++;

            consumer.ReactorChannel.Watchlist.DirectoryHandler.LoginStreamOpen(out _);

            var directoryRequest1 = CreateDirectoryRequest(460, ServiceFilterFlags.INFO | ServiceFilterFlags.STATE);

            Msg msg = new Msg();
            Assert.Equal(CodecReturnCode.SUCCESS, consumer.ReactorChannel.Watchlist.ConvertRDMToCodecMsg(directoryRequest1, msg));
            Assert.Equal(ReactorReturnCode.SUCCESS, consumer.Submit(msg, submitOptions));

            submitOptions.Clear();

            var directoryRequest2 = CreateDirectoryRequest(7191, ServiceFilterFlags.INFO | ServiceFilterFlags.DATA, 4);

            msg.Clear();
            Assert.Equal(CodecReturnCode.SUCCESS, consumer.ReactorChannel.Watchlist.ConvertRDMToCodecMsg(directoryRequest2, msg));
            Assert.Equal(ReactorReturnCode.SUCCESS, consumer.Submit(msg, submitOptions));

            submitOptions.Clear();

            var directoryRequest3 = CreateDirectoryRequest(7001, ServiceFilterFlags.INFO | ServiceFilterFlags.STATE, 5);

            msg.Clear();
            Assert.Equal(CodecReturnCode.SUCCESS, consumer.ReactorChannel.Watchlist.ConvertRDMToCodecMsg(directoryRequest3, msg));
            Assert.Equal(ReactorReturnCode.SUCCESS, consumer.Submit(msg, submitOptions));

            // Watchlist requested a directory, provide it
            providerReactor.Dispatch(1);
            TestReactorEvent evt = providerReactor.PollEvent();

            Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
            ReactorMsgEvent msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

            RDMDirectoryMsgEvent dirEvent = msgEvent as RDMDirectoryMsgEvent;
            Assert.NotNull(dirEvent);

            DirectoryMsg dirMsg = dirEvent.DirectoryMsg;
            Assert.NotNull(dirMsg);
            Assert.Equal(DirectoryMsgType.REQUEST, dirMsg.DirectoryMsgType);

            DirectoryRequest dirRequest = dirMsg.DirectoryRequest;
            Assert.NotNull(dirRequest);

            Assert.Equal(ALL_FILTERS, dirRequest.Filter);

            List<String> serviceNames = new List<string> { "IDN_RDF", "ELEKTRON_DD", "QPR0V1", "QPR0V2", "NI_PUB" };
            List<int> serviceIds = new List<int> { 460, 7191, 7001, 7002, 37397 };
            List<ServiceFlags> serviceFlags = new List<ServiceFlags>() { ServiceFlags.HAS_INFO | ServiceFlags.HAS_LOAD,
                ServiceFlags.HAS_INFO | ServiceFlags.HAS_LINK | ServiceFlags.HAS_DATA,
                ServiceFlags.HAS_INFO | ServiceFlags.HAS_STATE | ServiceFlags.HAS_DATA,
                ServiceFlags.HAS_INFO | ServiceFlags.HAS_LOAD | ServiceFlags.HAS_DATA,
                ServiceFlags.HAS_INFO | ServiceFlags.HAS_LINK | ServiceFlags.HAS_LOAD
            };
            List<MapEntryActions> actions = new List<MapEntryActions> { MapEntryActions.ADD, MapEntryActions.ADD, MapEntryActions.ADD, MapEntryActions.ADD, MapEntryActions.ADD };

            var directoryRefresh = CreateDirectoryRefresh(serviceNames, serviceIds, serviceFlags, actions, true, ALL_FILTERS, false, StreamStates.OPEN, DataStates.OK);

            msg.Clear();
            Assert.Equal(CodecReturnCode.SUCCESS, consumer.ReactorChannel.Watchlist.ConvertRDMToCodecMsg(directoryRefresh, msg));
            provider.SubmitAndDispatch(msg, submitOptions);

            consumerReactor.Dispatch(3);

            Dictionary<int, RDMDirectoryMsgEvent> eventDictionary = new Dictionary<int, RDMDirectoryMsgEvent>();

            for (int i = 0; i < 3; i++)
            {
                evt = consumerReactor.PollEvent();

                Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
                msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
                Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);

                dirEvent = msgEvent as RDMDirectoryMsgEvent;
                Assert.NotNull(dirEvent);

                dirMsg = dirEvent.DirectoryMsg;
                Assert.NotNull(dirMsg);
                Assert.Equal(DirectoryMsgType.REFRESH, dirMsg.DirectoryMsgType);

                eventDictionary.Add(dirMsg.DirectoryRefresh.StreamId, dirEvent);
            }

            Assert.Contains(2, eventDictionary.Keys);
            Assert.Contains(4, eventDictionary.Keys);
            Assert.Contains(5, eventDictionary.Keys);

            var refresh = eventDictionary[2].DirectoryMsg.DirectoryRefresh;
            Assert.Equal(460, refresh.ServiceList[0].ServiceId);
            Assert.False(refresh.ServiceList[0].HasState);
            Assert.False(refresh.ServiceList[0].HasData);
            Assert.False(refresh.ServiceList[0].HasLink);
            Assert.False(refresh.ServiceList[0].HasLoad);
            Assert.True(refresh.ServiceList[0].HasInfo);

            refresh = eventDictionary[4].DirectoryMsg.DirectoryRefresh;
            Assert.Equal(7191, refresh.ServiceList[0].ServiceId);
            Assert.False(refresh.ServiceList[0].HasState);
            Assert.True(refresh.ServiceList[0].HasData);
            Assert.False(refresh.ServiceList[0].HasLink);
            Assert.False(refresh.ServiceList[0].HasLoad);
            Assert.True(refresh.ServiceList[0].HasInfo);

            refresh = eventDictionary[5].DirectoryMsg.DirectoryRefresh;
            Assert.Equal(7001, refresh.ServiceList[0].ServiceId);
            Assert.True(refresh.ServiceList[0].HasState);
            Assert.False(refresh.ServiceList[0].HasData);
            Assert.False(refresh.ServiceList[0].HasLink);
            Assert.False(refresh.ServiceList[0].HasLoad);
            Assert.True(refresh.ServiceList[0].HasInfo);

            Assert.Equal(5, servicesAdded);
            Assert.Equal(0, servicesUpdated);
            Assert.Equal(0, servicesDeleted);

            TestReactorComponent.CloseSession(consumer, provider);
            consumerReactor.Close();
            providerReactor.Close();
        }

        [Fact]
        public void WatchlistDirectoryUpdateTest()
        {
            ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
            submitOptions.Clear();

            SetUp(out var consumer, out var provider, out var consumerReactor, out var providerReactor);

            int servicesAdded = 0;
            int servicesDeleted = 0;
            int servicesUpdated = 0;

            consumer.ReactorChannel.Watchlist.DirectoryHandler.ServiceCache.ServiceRemovedCallback += (s, val) => servicesDeleted++;
            consumer.ReactorChannel.Watchlist.DirectoryHandler.ServiceCache.ServiceAddedCallback += s => servicesAdded++;
            consumer.ReactorChannel.Watchlist.DirectoryHandler.ServiceCache.ServiceUpdatedCallback += s => servicesUpdated++;

            consumer.ReactorChannel.Watchlist.DirectoryHandler.LoginStreamOpen(out _);

            var directoryRequest1 = CreateDirectoryRequest(460, ServiceFilterFlags.INFO | ServiceFilterFlags.LOAD | ServiceFilterFlags.LINK);
            Msg msg = new Msg();
            Assert.Equal(CodecReturnCode.SUCCESS, consumer.ReactorChannel.Watchlist.ConvertRDMToCodecMsg(directoryRequest1, msg));
            Assert.Equal(ReactorReturnCode.SUCCESS, consumer.Submit(msg, submitOptions));

            submitOptions.Clear();
            msg.Clear();
            var directoryRequest2 = CreateDirectoryRequest(7191, ServiceFilterFlags.INFO | ServiceFilterFlags.LINK | ServiceFilterFlags.DATA | ServiceFilterFlags.STATE, 4);          
            Assert.Equal(CodecReturnCode.SUCCESS, consumer.ReactorChannel.Watchlist.ConvertRDMToCodecMsg(directoryRequest2, msg));
            Assert.Equal(ReactorReturnCode.SUCCESS, consumer.Submit(msg, submitOptions));

            submitOptions.Clear();
            msg.Clear();
            var directoryRequest3 = CreateDirectoryRequest(7001, ServiceFilterFlags.INFO | ServiceFilterFlags.STATE, 5);         
            Assert.Equal(CodecReturnCode.SUCCESS, consumer.ReactorChannel.Watchlist.ConvertRDMToCodecMsg(directoryRequest3, msg));
            Assert.Equal(ReactorReturnCode.SUCCESS, consumer.Submit(msg, submitOptions));

            // Watchlist requested a directory, provide it
            providerReactor.Dispatch(1);
            TestReactorEvent evt = providerReactor.PollEvent();

            Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
            ReactorMsgEvent msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            RDMDirectoryMsgEvent dirEvent = msgEvent as RDMDirectoryMsgEvent;
            Assert.NotNull(dirEvent);
            DirectoryMsg dirMsg = dirEvent.DirectoryMsg;
            Assert.NotNull(dirMsg);
            Assert.Equal(DirectoryMsgType.REQUEST, dirMsg.DirectoryMsgType);
            DirectoryRequest dirRequest = dirMsg.DirectoryRequest;
            Assert.NotNull(dirRequest);
            Assert.Equal(ALL_FILTERS, dirRequest.Filter);

            List<String> serviceNames = new List<string> { "IDN_RDF", "ELEKTRON_DD", "QPR0V1", "QPR0V2", "NI_PUB" };
            List<int> serviceIds = new List<int> { 460, 7191, 7001, 7002, 37397 };
            List<ServiceFlags> serviceFlags = new List<ServiceFlags>() { ServiceFlags.HAS_INFO | ServiceFlags.HAS_LOAD,
                ServiceFlags.HAS_INFO | ServiceFlags.HAS_LINK | ServiceFlags.HAS_DATA,
                ServiceFlags.HAS_INFO | ServiceFlags.HAS_STATE | ServiceFlags.HAS_DATA,
                ServiceFlags.HAS_INFO | ServiceFlags.HAS_LOAD | ServiceFlags.HAS_DATA,
                ServiceFlags.HAS_INFO | ServiceFlags.HAS_LINK | ServiceFlags.HAS_LOAD
            };
            List<MapEntryActions> actions = new List<MapEntryActions> { MapEntryActions.ADD, MapEntryActions.ADD, MapEntryActions.ADD, MapEntryActions.ADD, MapEntryActions.ADD };

            var directoryRefresh = CreateDirectoryRefresh(serviceNames, serviceIds, serviceFlags, actions, true, ALL_FILTERS, false, StreamStates.OPEN, DataStates.OK);

            msg.Clear();
            Assert.Equal(CodecReturnCode.SUCCESS, consumer.ReactorChannel.Watchlist.ConvertRDMToCodecMsg(directoryRefresh, msg));
            provider.SubmitAndDispatch(msg, submitOptions);

            consumerReactor.Dispatch(3);
            for (int i = 0; i < 3; i++)
            {
                consumerReactor.PollEvent();
            }

            serviceNames = new List<string> { "IDN_RDF", "ELEKTRON_DD" };
            serviceIds = new List<int> { 460, 7191 };
            serviceFlags = new List<ServiceFlags>() { ServiceFlags.HAS_INFO | ServiceFlags.HAS_LOAD | ServiceFlags.HAS_LINK,
                ServiceFlags.HAS_INFO | ServiceFlags.HAS_LINK | ServiceFlags.HAS_DATA | ServiceFlags.HAS_STATE
            };
            actions = new List<MapEntryActions> { MapEntryActions.UPDATE, MapEntryActions.UPDATE };

            var directoryUpdate = CreateDirectoryUpdate(serviceNames, serviceIds, serviceFlags, actions, ALL_FILTERS);
            msg.Clear();
            submitOptions.Clear();
            Assert.Equal(CodecReturnCode.SUCCESS, consumer.ReactorChannel.Watchlist.ConvertRDMToCodecMsg(directoryUpdate, msg));
            provider.SubmitAndDispatch(msg, submitOptions);

            consumerReactor.Dispatch(2);

            Dictionary<int, RDMDirectoryMsgEvent> eventDictionary = new Dictionary<int, RDMDirectoryMsgEvent>();

            for (int i = 0; i < 2; i++)
            {
                evt = consumerReactor.PollEvent();

                Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
                msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
                Assert.Equal(MsgClasses.UPDATE, msgEvent.Msg.MsgClass);

                dirEvent = msgEvent as RDMDirectoryMsgEvent;
                Assert.NotNull(dirEvent);

                dirMsg = dirEvent.DirectoryMsg;
                Assert.NotNull(dirMsg);
                Assert.Equal(DirectoryMsgType.UPDATE, dirMsg.DirectoryMsgType);

                eventDictionary.Add(dirMsg.DirectoryUpdate.StreamId, dirEvent);
            }

            Assert.Contains(2, eventDictionary.Keys);
            Assert.Contains(4, eventDictionary.Keys);

            var update = eventDictionary[2].DirectoryMsg.DirectoryUpdate;
            Assert.Equal(460, update.ServiceList[0].ServiceId);
            Assert.Equal(MapEntryActions.UPDATE, update.ServiceList[0].Action);
            Assert.False(update.ServiceList[0].HasState);
            Assert.False(update.ServiceList[0].HasData);
            Assert.True(update.ServiceList[0].HasLink);
            Assert.True(update.ServiceList[0].HasLoad);
            Assert.True(update.ServiceList[0].HasInfo);

            update = eventDictionary[4].DirectoryMsg.DirectoryUpdate;
            Assert.Equal(7191, update.ServiceList[0].ServiceId);
            Assert.Equal(MapEntryActions.UPDATE, update.ServiceList[0].Action);
            Assert.True(update.ServiceList[0].HasState);
            Assert.True(update.ServiceList[0].HasData);
            Assert.True(update.ServiceList[0].HasLink);
            Assert.False(update.ServiceList[0].HasLoad);
            Assert.True(update.ServiceList[0].HasInfo);

            Assert.Equal(5, servicesAdded);
            Assert.Equal(2, servicesUpdated);
            Assert.Equal(0, servicesDeleted);

            TestReactorComponent.CloseSession(consumer, provider);
            consumerReactor.Close();
            providerReactor.Close();
        }

        [Fact]
        public void WatchlistDirectoryRefreshClearCacheTest()
        {
            ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
            submitOptions.Clear();

            SetUp(out var consumer, out var provider, out var consumerReactor, out var providerReactor);

            int servicesAdded = 0;
            int servicesDeleted = 0;
            int servicesUpdated = 0;

            consumer.ReactorChannel.Watchlist.DirectoryHandler.ServiceCache.ServiceRemovedCallback += (s, val) => servicesDeleted++;
            consumer.ReactorChannel.Watchlist.DirectoryHandler.ServiceCache.ServiceAddedCallback += s => servicesAdded++;
            consumer.ReactorChannel.Watchlist.DirectoryHandler.ServiceCache.ServiceUpdatedCallback += s => servicesUpdated++;

            consumer.ReactorChannel.Watchlist.LoginHandler = new MockLoginHandler();
            consumer.ReactorChannel.Watchlist.ItemHandler = new MockItemHandler();

            consumer.ReactorChannel.Watchlist.DirectoryHandler.LoginStreamOpen(out _);

            var directoryRequest1 = CreateDirectoryRequest(460, ServiceFilterFlags.INFO | ServiceFilterFlags.LOAD | ServiceFilterFlags.LINK);
            Msg msg = new Msg();
            Assert.Equal(CodecReturnCode.SUCCESS, consumer.ReactorChannel.Watchlist.ConvertRDMToCodecMsg(directoryRequest1, msg));
            Assert.Equal(ReactorReturnCode.SUCCESS, consumer.Submit(msg, submitOptions));

            submitOptions.Clear();
            msg.Clear();
            var directoryRequest2 = CreateDirectoryRequest(7191, ServiceFilterFlags.INFO | ServiceFilterFlags.LINK | ServiceFilterFlags.DATA | ServiceFilterFlags.STATE, 4);
            Assert.Equal(CodecReturnCode.SUCCESS, consumer.ReactorChannel.Watchlist.ConvertRDMToCodecMsg(directoryRequest2, msg));
            Assert.Equal(ReactorReturnCode.SUCCESS, consumer.Submit(msg, submitOptions));

            providerReactor.Dispatch(1);
            TestReactorEvent evt = providerReactor.PollEvent();

            Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
            ReactorMsgEvent msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            RDMDirectoryMsgEvent dirEvent = msgEvent as RDMDirectoryMsgEvent;
            Assert.NotNull(dirEvent);
            DirectoryMsg dirMsg = dirEvent.DirectoryMsg;
            Assert.NotNull(dirMsg);
            Assert.Equal(DirectoryMsgType.REQUEST, dirMsg.DirectoryMsgType);
            DirectoryRequest dirRequest = dirMsg.DirectoryRequest;
            Assert.NotNull(dirRequest);
            Assert.Equal(ALL_FILTERS, dirRequest.Filter);

            List<String> serviceNames = new List<string> { "IDN_RDF", "ELEKTRON_DD", "QPR0V1", "QPR0V2", "NI_PUB" };
            List<int> serviceIds = new List<int> { 460, 7191, 7001, 7002, 37397 };
            List<ServiceFlags> serviceFlags = new List<ServiceFlags>() { ServiceFlags.HAS_INFO | ServiceFlags.HAS_LOAD,
                ServiceFlags.HAS_INFO | ServiceFlags.HAS_LINK | ServiceFlags.HAS_DATA,
                ServiceFlags.HAS_INFO | ServiceFlags.HAS_STATE | ServiceFlags.HAS_DATA,
                ServiceFlags.HAS_INFO | ServiceFlags.HAS_LOAD | ServiceFlags.HAS_DATA,
                ServiceFlags.HAS_INFO | ServiceFlags.HAS_LINK | ServiceFlags.HAS_LOAD
            };
            List<MapEntryActions> actions = new List<MapEntryActions> { MapEntryActions.ADD, MapEntryActions.ADD, MapEntryActions.ADD, MapEntryActions.ADD, MapEntryActions.ADD };

            var directoryRefresh = CreateDirectoryRefresh(serviceNames, serviceIds, serviceFlags, actions, true, ALL_FILTERS, false, StreamStates.OPEN, DataStates.OK);

            msg.Clear();
            Assert.Equal(CodecReturnCode.SUCCESS, consumer.ReactorChannel.Watchlist.ConvertRDMToCodecMsg(directoryRefresh, msg));
            provider.SubmitAndDispatch(msg, submitOptions);

            consumerReactor.Dispatch(2);
            for (int i = 0; i < 2; i++)
            {
                consumerReactor.PollEvent();
            }

            directoryRefresh = CreateDirectoryRefresh(new List<string>(), null, null, null, false, ALL_FILTERS, true, StreamStates.OPEN, DataStates.OK);
            
            msg.Clear();
            submitOptions.Clear();
            Assert.Equal(CodecReturnCode.SUCCESS, consumer.ReactorChannel.Watchlist.ConvertRDMToCodecMsg(directoryRefresh, msg));
            provider.SubmitAndDispatch(msg, submitOptions);

            consumerReactor.Dispatch(2);

            Dictionary<int, RDMDirectoryMsgEvent> eventDictionary = new Dictionary<int, RDMDirectoryMsgEvent>();

            for (int i = 0; i < 2; i++)
            {
                evt = consumerReactor.PollEvent();

                Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
                msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
                Assert.Equal(MsgClasses.UPDATE, msgEvent.Msg.MsgClass);

                dirEvent = msgEvent as RDMDirectoryMsgEvent;
                Assert.NotNull(dirEvent);

                dirMsg = dirEvent.DirectoryMsg;
                Assert.NotNull(dirMsg);
                Assert.Equal(DirectoryMsgType.UPDATE, dirMsg.DirectoryMsgType);

                eventDictionary.Add(dirMsg.DirectoryUpdate.StreamId, dirEvent);
            }

            Assert.Contains(2, eventDictionary.Keys);
            Assert.Contains(4, eventDictionary.Keys);

            var update = eventDictionary[2].DirectoryMsg.DirectoryUpdate;
            Assert.Equal(460, update.ServiceList[0].ServiceId);
            Assert.Equal(MapEntryActions.DELETE, update.ServiceList[0].Action);

            update = eventDictionary[4].DirectoryMsg.DirectoryUpdate;
            Assert.Equal(7191, update.ServiceList[0].ServiceId);
            Assert.Equal(MapEntryActions.DELETE, update.ServiceList[0].Action);

            Assert.Equal(5, servicesAdded);
            Assert.Equal(0, servicesUpdated);
            Assert.Equal(5, servicesDeleted);

            // Verify that we are able to add other services after a refresh with ClearCache flag
            directoryRefresh = CreateDirectoryRefresh(serviceNames, serviceIds, serviceFlags, actions, false, ALL_FILTERS, false, StreamStates.OPEN, DataStates.OK);
            
            msg.Clear();
            submitOptions.Clear();
            Assert.Equal(CodecReturnCode.SUCCESS, consumer.ReactorChannel.Watchlist.ConvertRDMToCodecMsg(directoryRefresh, msg));
            provider.SubmitAndDispatch(msg, submitOptions);

            consumerReactor.Dispatch(2);
            for (int i = 0; i < 2; i++)
            {
                evt = consumerReactor.PollEvent();

                Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
                msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
                Assert.Equal(MsgClasses.UPDATE, msgEvent.Msg.MsgClass);

                dirEvent = msgEvent as RDMDirectoryMsgEvent;
                Assert.NotNull(dirEvent);

                dirMsg = dirEvent.DirectoryMsg;
                Assert.NotNull(dirMsg);
                Assert.Equal(DirectoryMsgType.UPDATE, dirMsg.DirectoryMsgType);
            }

            Assert.Equal(10, servicesAdded);
            Assert.Equal(0, servicesUpdated);
            Assert.Equal(5, servicesDeleted);

            TestReactorComponent.CloseSession(consumer, provider);
            consumerReactor.Close();
            providerReactor.Close();
        }

        [Fact]
        public void WatchlistDirectoryGenericTest()
        {
            ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
            submitOptions.Clear();

            SetUp(out var consumer, out var provider, out var consumerReactor, out var providerReactor);

            int servicesAdded = 0;
            int servicesDeleted = 0;
            int servicesUpdated = 0;

            consumer.ReactorChannel.Watchlist.DirectoryHandler.ServiceCache.ServiceRemovedCallback += (s, val) => servicesDeleted++;
            consumer.ReactorChannel.Watchlist.DirectoryHandler.ServiceCache.ServiceAddedCallback += s => servicesAdded++;
            consumer.ReactorChannel.Watchlist.DirectoryHandler.ServiceCache.ServiceUpdatedCallback += s => servicesUpdated++;

            consumer.ReactorChannel.Watchlist.DirectoryHandler.LoginStreamOpen(out _);

            var directoryRequest1 = CreateDirectoryRequest(460, ServiceFilterFlags.INFO | ServiceFilterFlags.LOAD | ServiceFilterFlags.LINK);
            Msg msg = new Msg();
            Assert.Equal(CodecReturnCode.SUCCESS, consumer.ReactorChannel.Watchlist.ConvertRDMToCodecMsg(directoryRequest1, msg));
            Assert.Equal(ReactorReturnCode.SUCCESS, consumer.Submit(msg, submitOptions));

            submitOptions.Clear();
            msg.Clear();
            var directoryRequest2 = CreateDirectoryRequest(7191, ServiceFilterFlags.INFO | ServiceFilterFlags.LINK | ServiceFilterFlags.DATA | ServiceFilterFlags.STATE, 4);
            Assert.Equal(CodecReturnCode.SUCCESS, consumer.ReactorChannel.Watchlist.ConvertRDMToCodecMsg(directoryRequest2, msg));
            Assert.Equal(ReactorReturnCode.SUCCESS, consumer.Submit(msg, submitOptions));

            submitOptions.Clear();
            msg.Clear();
            var directoryRequest3 = CreateDirectoryRequest(7001, ServiceFilterFlags.INFO | ServiceFilterFlags.STATE, 5);
            Assert.Equal(CodecReturnCode.SUCCESS, consumer.ReactorChannel.Watchlist.ConvertRDMToCodecMsg(directoryRequest3, msg));
            Assert.Equal(ReactorReturnCode.SUCCESS, consumer.Submit(msg, submitOptions));

            // Watchlist requested a directory, provide it
            providerReactor.Dispatch(1);
            TestReactorEvent evt = providerReactor.PollEvent();

            Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
            ReactorMsgEvent msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            RDMDirectoryMsgEvent dirEvent = msgEvent as RDMDirectoryMsgEvent;
            Assert.NotNull(dirEvent);
            DirectoryMsg dirMsg = dirEvent.DirectoryMsg;
            Assert.NotNull(dirMsg);
            Assert.Equal(DirectoryMsgType.REQUEST, dirMsg.DirectoryMsgType);
            DirectoryRequest dirRequest = dirMsg.DirectoryRequest;
            Assert.NotNull(dirRequest);
            Assert.Equal(ALL_FILTERS, dirRequest.Filter);

            List<String> serviceNames = new List<string> { "IDN_RDF", "ELEKTRON_DD", "QPR0V1", "QPR0V2", "NI_PUB" };
            List<int> serviceIds = new List<int> { 460, 7191, 7001, 7002, 37397 };
            List<ServiceFlags> serviceFlags = new List<ServiceFlags>() { ServiceFlags.HAS_INFO | ServiceFlags.HAS_LOAD,
                ServiceFlags.HAS_INFO | ServiceFlags.HAS_LINK | ServiceFlags.HAS_DATA,
                ServiceFlags.HAS_INFO | ServiceFlags.HAS_STATE | ServiceFlags.HAS_DATA,
                ServiceFlags.HAS_INFO | ServiceFlags.HAS_LOAD | ServiceFlags.HAS_DATA,
                ServiceFlags.HAS_INFO | ServiceFlags.HAS_LINK | ServiceFlags.HAS_LOAD
            };
            List<MapEntryActions> actions = new List<MapEntryActions> { MapEntryActions.ADD, MapEntryActions.ADD, MapEntryActions.ADD, MapEntryActions.ADD, MapEntryActions.ADD };

            var directoryRefresh = CreateDirectoryRefresh(serviceNames, serviceIds, serviceFlags, actions, true, ALL_FILTERS, false, StreamStates.OPEN, DataStates.OK);

            msg.Clear();
            Assert.Equal(CodecReturnCode.SUCCESS, consumer.ReactorChannel.Watchlist.ConvertRDMToCodecMsg(directoryRefresh, msg));
            provider.SubmitAndDispatch(msg, submitOptions);

            consumerReactor.Dispatch(3);
            for (int i = 0; i < 3; i++)
            {
                consumerReactor.PollEvent();
            }

            DirectoryConsumerStatus directoryStatus = new DirectoryConsumerStatus();
            directoryStatus.StreamId = 2;
            ConsumerStatusService statusService = new ConsumerStatusService();
            statusService.Action = MapEntryActions.UPDATE;
            statusService.ServiceId = 460;

            msg.Clear();
            submitOptions.Clear();
            Assert.Equal(CodecReturnCode.SUCCESS, consumer.ReactorChannel.Watchlist.ConvertRDMToCodecMsg(directoryStatus, msg));
            provider.SubmitAndDispatch(msg, submitOptions);

            consumerReactor.Dispatch(3);
            for (int i = 0; i < 3; i++)
            {
                evt = consumerReactor.PollEvent();
                Assert.Equal(TestReactorEventType.MSG, evt.EventType);
                msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
                Assert.Equal(MsgClasses.GENERIC, msgEvent.Msg.MsgClass);
            }

            TestReactorComponent.CloseSession(consumer, provider);
            consumerReactor.Close();
            providerReactor.Close();
        }

        [Fact]
        public void WatchlistDirectoryFirstRefreshEmptyTest()
        {
            ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
            submitOptions.Clear();

            SetUp(out var consumer, out var provider, out var consumerReactor, out var providerReactor);

            int servicesAdded = 0;
            int servicesDeleted = 0;
            int servicesUpdated = 0;

            consumer.ReactorChannel.Watchlist.DirectoryHandler.ServiceCache.ServiceRemovedCallback = (s, val) => servicesDeleted++;
            consumer.ReactorChannel.Watchlist.DirectoryHandler.ServiceCache.ServiceAddedCallback = s => servicesAdded++;
            consumer.ReactorChannel.Watchlist.DirectoryHandler.ServiceCache.ServiceUpdatedCallback = s => servicesUpdated++;

            consumer.ReactorChannel.Watchlist.DirectoryHandler.LoginStreamOpen(out _);

            // Watchlist requested a directory, provide it
            providerReactor.Dispatch(1);
            TestReactorEvent evt = providerReactor.PollEvent();

            Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
            ReactorMsgEvent msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);
            RDMDirectoryMsgEvent dirEvent = msgEvent as RDMDirectoryMsgEvent;
            Assert.NotNull(dirEvent);
            DirectoryMsg dirMsg = dirEvent.DirectoryMsg;
            Assert.NotNull(dirMsg);
            Assert.Equal(DirectoryMsgType.REQUEST, dirMsg.DirectoryMsgType);
            DirectoryRequest dirRequest = dirMsg.DirectoryRequest;
            Assert.NotNull(dirRequest);
            Assert.Equal(ALL_FILTERS, dirRequest.Filter);

            var directoryRefresh = CreateDirectoryRefresh(new List<string>(), null, null, null, true, ALL_FILTERS, false, StreamStates.OPEN, DataStates.OK);

            Msg msg = new Msg();          
            Assert.Equal(CodecReturnCode.SUCCESS, consumer.ReactorChannel.Watchlist.ConvertRDMToCodecMsg(directoryRefresh, msg));
            provider.SubmitAndDispatch(msg, submitOptions);

            consumerReactor.Dispatch(0);

            var directoryRequest1 = CreateDirectoryRequest(460, ServiceFilterFlags.INFO | ServiceFilterFlags.LOAD | ServiceFilterFlags.LINK);
            msg.Clear();
            Assert.Equal(CodecReturnCode.SUCCESS, consumer.ReactorChannel.Watchlist.ConvertRDMToCodecMsg(directoryRequest1, msg));
            Assert.Equal(ReactorReturnCode.SUCCESS, consumer.Submit(msg, submitOptions));

            submitOptions.Clear();
            msg.Clear();
            var directoryRequest2 = CreateDirectoryRequest(7191, ServiceFilterFlags.INFO | ServiceFilterFlags.LINK | ServiceFilterFlags.DATA | ServiceFilterFlags.STATE, 4);
            Assert.Equal(CodecReturnCode.SUCCESS, consumer.ReactorChannel.Watchlist.ConvertRDMToCodecMsg(directoryRequest2, msg));
            Assert.Equal(ReactorReturnCode.SUCCESS, consumer.Submit(msg, submitOptions));

            submitOptions.Clear();
            msg.Clear();
            var directoryRequest3 = CreateDirectoryRequest(7001, ServiceFilterFlags.INFO | ServiceFilterFlags.STATE, 5);
            Assert.Equal(CodecReturnCode.SUCCESS, consumer.ReactorChannel.Watchlist.ConvertRDMToCodecMsg(directoryRequest3, msg));
            Assert.Equal(ReactorReturnCode.SUCCESS, consumer.Submit(msg, submitOptions));

            List<String> serviceNames = new List<string> { "IDN_RDF", "ELEKTRON_DD", "QPR0V1", "QPR0V2", "NI_PUB" };
            List<int> serviceIds = new List<int> { 460, 7191, 7001, 7002, 37397 };
            List<ServiceFlags> serviceFlags = new List<ServiceFlags>() { ServiceFlags.HAS_INFO | ServiceFlags.HAS_LOAD,
                ServiceFlags.HAS_INFO | ServiceFlags.HAS_LINK | ServiceFlags.HAS_DATA,
                ServiceFlags.HAS_INFO | ServiceFlags.HAS_STATE | ServiceFlags.HAS_DATA,
                ServiceFlags.HAS_INFO | ServiceFlags.HAS_LOAD | ServiceFlags.HAS_DATA,
                ServiceFlags.HAS_INFO | ServiceFlags.HAS_LINK | ServiceFlags.HAS_LOAD
            };
            List<MapEntryActions> actions = new List<MapEntryActions> { MapEntryActions.ADD, MapEntryActions.ADD, MapEntryActions.ADD, MapEntryActions.ADD, MapEntryActions.ADD };

            directoryRefresh = CreateDirectoryRefresh(serviceNames, serviceIds, serviceFlags, actions, true, ALL_FILTERS, false, StreamStates.OPEN, DataStates.OK);

            msg.Clear();
            submitOptions.Clear();
            Assert.Equal(CodecReturnCode.SUCCESS, consumer.ReactorChannel.Watchlist.ConvertRDMToCodecMsg(directoryRefresh, msg));
            provider.SubmitAndDispatch(msg, submitOptions);

            consumerReactor.Dispatch(3);
            for (int i = 0; i < 3; i++)
            {
                evt = consumerReactor.PollEvent();
                Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
                msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
                Assert.Equal(MsgClasses.UPDATE, msgEvent.Msg.MsgClass);
            }

            TestReactorComponent.CloseSession(consumer, provider);
            consumerReactor.Close();
            providerReactor.Close();
        }

        [Fact]
        public void WatchlistDirectoryRefreshAsUpdateTest()
        {
            ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
            submitOptions.Clear();

            SetUp(out var consumer, out var provider, out var consumerReactor, out var providerReactor);

            int servicesAdded = 0;
            int servicesDeleted = 0;
            int servicesUpdated = 0;

            consumer.ReactorChannel.Watchlist.DirectoryHandler.ServiceCache.ServiceRemovedCallback += (s, val) => servicesDeleted++;
            consumer.ReactorChannel.Watchlist.DirectoryHandler.ServiceCache.ServiceAddedCallback += s => servicesAdded++;
            consumer.ReactorChannel.Watchlist.DirectoryHandler.ServiceCache.ServiceUpdatedCallback += s => servicesUpdated++;

            consumer.ReactorChannel.Watchlist.DirectoryHandler.LoginStreamOpen(out _);

            var directoryRequest = new DirectoryRequest();
            directoryRequest.Filter = ServiceFilterFlags.INFO | ServiceFilterFlags.STATE;
            directoryRequest.StreamId = 2;

            Msg msg = new Msg();
            Assert.Equal(CodecReturnCode.SUCCESS, consumer.ReactorChannel.Watchlist.ConvertRDMToCodecMsg(directoryRequest, msg));
            submitOptions.ServiceName = "IDN_RDF";
            Assert.Equal(ReactorReturnCode.SUCCESS, consumer.Submit(msg, submitOptions));

            submitOptions.Clear();

            // Watchlist requested a directory, provide it
            providerReactor.Dispatch(1);
            TestReactorEvent evt = providerReactor.PollEvent();

            Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
            ReactorMsgEvent msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REQUEST, msgEvent.Msg.MsgClass);

            RDMDirectoryMsgEvent dirEvent = msgEvent as RDMDirectoryMsgEvent;
            Assert.NotNull(dirEvent);

            DirectoryMsg dirMsg = dirEvent.DirectoryMsg;
            Assert.NotNull(dirMsg);
            Assert.Equal(DirectoryMsgType.REQUEST, dirMsg.DirectoryMsgType);

            DirectoryRequest dirRequest = dirMsg.DirectoryRequest;
            Assert.NotNull(dirRequest);

            Assert.Equal(ALL_FILTERS, dirRequest.Filter);

            List<String> serviceNames = new List<string> { "IDN_RDF", "ELEKTRON_DD", "QPR0V1", "QPR0V2", "NI_PUB" };
            List<int> serviceIds = new List<int> { 460, 7191, 7001, 7002, 37397 };
            List<ServiceFlags> serviceFlags = new List<ServiceFlags>() { ServiceFlags.HAS_INFO | ServiceFlags.HAS_LOAD,
                ServiceFlags.HAS_INFO | ServiceFlags.HAS_LINK | ServiceFlags.HAS_DATA,
                ServiceFlags.HAS_INFO | ServiceFlags.HAS_STATE | ServiceFlags.HAS_DATA,
                ServiceFlags.HAS_INFO | ServiceFlags.HAS_LOAD | ServiceFlags.HAS_DATA,
                ServiceFlags.HAS_INFO | ServiceFlags.HAS_LINK | ServiceFlags.HAS_LOAD
            };
            List<MapEntryActions> actions = new List<MapEntryActions> { MapEntryActions.ADD, MapEntryActions.ADD, MapEntryActions.ADD, MapEntryActions.ADD, MapEntryActions.ADD };

            var directoryRefresh = CreateDirectoryRefresh(serviceNames, serviceIds, serviceFlags, actions, true, ALL_FILTERS, false, StreamStates.OPEN, DataStates.OK);

            msg.Clear();
            submitOptions.Clear();
            Assert.Equal(CodecReturnCode.SUCCESS, consumer.ReactorChannel.Watchlist.ConvertRDMToCodecMsg(directoryRefresh, msg));
            provider.SubmitAndDispatch(msg, submitOptions);

            consumerReactor.Dispatch(1);

            evt = consumerReactor.PollEvent();

            Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);

            dirEvent = msgEvent as RDMDirectoryMsgEvent;
            Assert.NotNull(dirEvent);

            dirMsg = dirEvent.DirectoryMsg;
            Assert.NotNull(dirMsg);
            Assert.Equal(DirectoryMsgType.REFRESH, dirMsg.DirectoryMsgType);

            DirectoryRefresh dirRefresh = dirMsg.DirectoryRefresh;
            Assert.NotNull(dirRefresh);
            Assert.Equal(2, dirRefresh.StreamId);
            Assert.Equal(directoryRequest.Filter, dirRefresh.Filter);
            Assert.Single(dirRefresh.ServiceList);
            Assert.Equal(460, dirRefresh.ServiceList[0].ServiceId);
            Assert.False(dirRefresh.ServiceList[0].HasData);
            Assert.False(dirRefresh.ServiceList[0].HasLink);
            Assert.False(dirRefresh.ServiceList[0].HasLoad);
            Assert.True(dirRefresh.ServiceList[0].HasInfo);
            Assert.Equal("IDN_RDF", dirRefresh.ServiceList[0].Info.ServiceName.ToString());

            Assert.Equal(5, servicesAdded);
            Assert.Equal(0, servicesUpdated);
            Assert.Equal(0, servicesDeleted);

            serviceNames = new List<string> { "IDN_RDF", "ELEKTRON_DD", "QPR0V1", "QPR0V2", "NI_PUB" };
            serviceIds = new List<int> { 460, 7191, 7001, 7002, 37397 };
            serviceFlags = new List<ServiceFlags>() { ServiceFlags.HAS_INFO | ServiceFlags.HAS_LOAD | ServiceFlags.HAS_STATE,
                ServiceFlags.HAS_INFO | ServiceFlags.HAS_LINK | ServiceFlags.HAS_DATA | ServiceFlags.HAS_STATE,
                ServiceFlags.HAS_INFO | ServiceFlags.HAS_STATE | ServiceFlags.HAS_DATA | ServiceFlags.HAS_LOAD,
                ServiceFlags.HAS_INFO | ServiceFlags.HAS_LOAD | ServiceFlags.HAS_DATA | ServiceFlags.HAS_STATE,
                ServiceFlags.HAS_INFO | ServiceFlags.HAS_LINK | ServiceFlags.HAS_LOAD | ServiceFlags.HAS_STATE
            };
            actions = new List<MapEntryActions> { MapEntryActions.UPDATE, MapEntryActions.UPDATE, MapEntryActions.UPDATE, MapEntryActions.UPDATE, MapEntryActions.UPDATE };

            directoryRefresh = CreateDirectoryRefresh(serviceNames, serviceIds, serviceFlags, actions, true, ALL_FILTERS, false, StreamStates.OPEN, DataStates.OK);

            msg.Clear();
            submitOptions.Clear();
            Assert.Equal(CodecReturnCode.SUCCESS, consumer.ReactorChannel.Watchlist.ConvertRDMToCodecMsg(directoryRefresh, msg));
            provider.SubmitAndDispatch(msg, submitOptions);

            consumerReactor.Dispatch(1);

            evt = consumerReactor.PollEvent();

            Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
            msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
            Assert.Equal(MsgClasses.UPDATE, msgEvent.Msg.MsgClass);

            Assert.Equal(5, servicesAdded);
            Assert.Equal(5, servicesUpdated);
            Assert.Equal(0, servicesDeleted);

            TestReactorComponent.CloseSession(consumer, provider);
            consumerReactor.Close();
            providerReactor.Close();
        }

        [Fact]
        public void WatchlistServiceDownWithMulipleDirectoryRequestsTest()
        {
            /* Test two directory request with filter 0 and make sure call backs for responses have filter 0.
            * Test a third directory request with INFO filter only and make sure no response received by user
            * when only STATE filter is sent by provider.
            * Test a fourth directory request with STATE filter only and make sure response received by user
            * with only STATE filter.
            * Test a fifth directory request with LOAD filter only and make sure response received by user
            * with only LOAD filter.
            * Test a sixth directory request with STATE and LOAD filter and make sure response received by user
            * with only STATE and LOAD filter. */

            ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
            submitOptions.Clear();

            SetUp(out var consumer, out var provider, out var consumerReactor, out var providerReactor);

            int servicesAdded = 0;
            int servicesDeleted = 0;
            int servicesUpdated = 0;

            consumer.ReactorChannel.Watchlist.DirectoryHandler.ServiceCache.ServiceRemovedCallback += (s, val) => servicesDeleted++;
            consumer.ReactorChannel.Watchlist.DirectoryHandler.ServiceCache.ServiceAddedCallback += s => servicesAdded++;
            consumer.ReactorChannel.Watchlist.DirectoryHandler.ServiceCache.ServiceUpdatedCallback += s => servicesUpdated++;

            consumer.ReactorChannel.Watchlist.DirectoryHandler.LoginStreamOpen(out _);

            providerReactor.Dispatch(1); // dispatch consumer watchlist request
            providerReactor.PollEvent();

            var directoryRequest1 = CreateDirectoryRequest(460, ALL_FILTERS, 5);

            Msg msg = new Msg();
            Assert.Equal(CodecReturnCode.SUCCESS, consumer.ReactorChannel.Watchlist.ConvertRDMToCodecMsg(directoryRequest1, msg));
            Assert.Equal(ReactorReturnCode.SUCCESS, consumer.Submit(msg, submitOptions));

            submitOptions.Clear();

            var directoryRequest2 = CreateDirectoryRequest(460, ServiceFilterFlags.INFO, 10);

            msg.Clear();
            Assert.Equal(CodecReturnCode.SUCCESS, consumer.ReactorChannel.Watchlist.ConvertRDMToCodecMsg(directoryRequest2, msg));
            Assert.Equal(ReactorReturnCode.SUCCESS, consumer.Submit(msg, submitOptions));

            submitOptions.Clear();

            var directoryRequest3 = CreateDirectoryRequest(460, ServiceFilterFlags.STATE, 15);

            msg.Clear();
            Assert.Equal(CodecReturnCode.SUCCESS, consumer.ReactorChannel.Watchlist.ConvertRDMToCodecMsg(directoryRequest3, msg));
            Assert.Equal(ReactorReturnCode.SUCCESS, consumer.Submit(msg, submitOptions));

            submitOptions.Clear();

            var directoryRequest4 = CreateDirectoryRequest(460, ServiceFilterFlags.LOAD, 20);

            msg.Clear();
            Assert.Equal(CodecReturnCode.SUCCESS, consumer.ReactorChannel.Watchlist.ConvertRDMToCodecMsg(directoryRequest4, msg));
            Assert.Equal(ReactorReturnCode.SUCCESS, consumer.Submit(msg, submitOptions));

            submitOptions.Clear();

            var directoryRequest5 = CreateDirectoryRequest(460, ServiceFilterFlags.STATE | ServiceFilterFlags.LOAD, 25);

            msg.Clear();
            Assert.Equal(CodecReturnCode.SUCCESS, consumer.ReactorChannel.Watchlist.ConvertRDMToCodecMsg(directoryRequest5, msg));
            Assert.Equal(ReactorReturnCode.SUCCESS, consumer.Submit(msg, submitOptions));

            List<string> serviceNames = new List<string> { "IDN_RDF" };
            List<int> serviceIds = new List<int> { 460 };
            List<ServiceFlags>  serviceFlags = new List<ServiceFlags>() { ServiceFlags.HAS_INFO | ServiceFlags.HAS_LOAD | ServiceFlags.HAS_STATE | ServiceFlags.HAS_DATA };
            List<MapEntryActions>  actions = new List<MapEntryActions> { MapEntryActions.ADD };

            var directoryRefresh = CreateDirectoryRefresh(serviceNames, serviceIds, serviceFlags, actions, true, ALL_FILTERS, false, StreamStates.OPEN, DataStates.OK);

            msg.Clear();
            submitOptions.Clear();
            Assert.Equal(CodecReturnCode.SUCCESS, consumer.ReactorChannel.Watchlist.ConvertRDMToCodecMsg(directoryRefresh, msg));
            provider.SubmitAndDispatch(msg, submitOptions);

            consumerReactor.Dispatch(5);

            var eventDictionary = new Dictionary<int, DirectoryMsg>();
            for (int i = 0; i < 5; i++)
            {
                var evt = consumerReactor.PollEvent();

                Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
                var msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
                Assert.Equal(MsgClasses.REFRESH, msgEvent.Msg.MsgClass);
                var dirEvent = msgEvent as RDMDirectoryMsgEvent;
                Assert.NotNull(dirEvent);
                var dirMsg = dirEvent.DirectoryMsg;
                Assert.NotNull(dirMsg);
                Assert.Equal(DirectoryMsgType.REFRESH, dirMsg.DirectoryMsgType);
                eventDictionary.Add(dirMsg.StreamId, dirMsg);
            }

            var msg5 = eventDictionary[5];
            DirectoryRefresh dirRefresh = msg5.DirectoryRefresh;
            Assert.NotNull(dirRefresh);
            Assert.Equal(5, dirRefresh.StreamId);
            Assert.Single(dirRefresh.ServiceList);
            Assert.Equal(460, dirRefresh.ServiceList[0].ServiceId);
            Assert.Equal(ALL_FILTERS, dirRefresh.Filter);

            var msg10 = eventDictionary[10];
            dirRefresh = msg10.DirectoryRefresh;
            Assert.NotNull(dirRefresh);
            Assert.Equal(10, dirRefresh.StreamId);
            Assert.Single(dirRefresh.ServiceList);
            Assert.Equal(460, dirRefresh.ServiceList[0].ServiceId);
            Assert.Equal(ServiceFilterFlags.INFO, dirRefresh.Filter);

            var msg15 = eventDictionary[15];
            dirRefresh = msg15.DirectoryRefresh;
            Assert.NotNull(dirRefresh);
            Assert.Equal(15, dirRefresh.StreamId);
            Assert.Single(dirRefresh.ServiceList);
            Assert.Equal(460, dirRefresh.ServiceList[0].ServiceId);
            Assert.Equal(ServiceFilterFlags.STATE, dirRefresh.Filter);

            var msg20 = eventDictionary[20];
            dirRefresh = msg20.DirectoryRefresh;
            Assert.NotNull(dirRefresh);
            Assert.Equal(20, dirRefresh.StreamId);
            Assert.Single(dirRefresh.ServiceList);
            Assert.Equal(460, dirRefresh.ServiceList[0].ServiceId);
            Assert.Equal(ServiceFilterFlags.LOAD, dirRefresh.Filter);

            var msg25 = eventDictionary[25];
            dirRefresh = msg25.DirectoryRefresh;
            Assert.NotNull(dirRefresh);
            Assert.Equal(25, dirRefresh.StreamId);
            Assert.Single(dirRefresh.ServiceList);
            Assert.Equal(460, dirRefresh.ServiceList[0].ServiceId);
            Assert.Equal(ServiceFilterFlags.STATE | ServiceFilterFlags.LOAD, dirRefresh.Filter);

            serviceFlags = new List<ServiceFlags>() { ServiceFlags.HAS_LOAD | ServiceFlags.HAS_STATE | ServiceFlags.HAS_DATA };
            actions = new List<MapEntryActions> { MapEntryActions.UPDATE };

            var directoryUpdate = CreateDirectoryUpdate(serviceNames, serviceIds, serviceFlags, actions, ServiceFilterFlags.LOAD | ServiceFilterFlags.STATE);
            directoryUpdate.ServiceList[0].State.ServiceStateVal = 0;
            directoryUpdate.ServiceList[0].State.HasStatus = true;
            directoryUpdate.ServiceList[0].State.Status.DataState(DataStates.SUSPECT);
            directoryUpdate.ServiceList[0].State.Status.StreamState(StreamStates.CLOSED_RECOVER);

            msg.Clear();
            submitOptions.Clear();
            Assert.Equal(CodecReturnCode.SUCCESS, consumer.ReactorChannel.Watchlist.ConvertRDMToCodecMsg(directoryUpdate, msg));
            provider.SubmitAndDispatch(msg, submitOptions);

            consumerReactor.Dispatch(4);
            eventDictionary.Clear();

            for (int i = 0; i < 4; i++)
            {
                var evt = consumerReactor.PollEvent();

                Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
                var msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
                Assert.Equal(MsgClasses.UPDATE, msgEvent.Msg.MsgClass);
                var dirEvent = msgEvent as RDMDirectoryMsgEvent;
                Assert.NotNull(dirEvent);
                var dirMsg = dirEvent.DirectoryMsg;
                Assert.NotNull(dirMsg);
                Assert.Equal(DirectoryMsgType.UPDATE, dirMsg.DirectoryMsgType);
                eventDictionary.Add(dirMsg.StreamId, dirMsg);
            }

            msg5 = eventDictionary[5];
            var dirUpdate = msg5.DirectoryUpdate;
            Assert.NotNull(dirUpdate);
            Assert.Equal(5, dirUpdate.StreamId);
            Assert.Single(dirUpdate.ServiceList);
            Assert.Equal(460, dirUpdate.ServiceList[0].ServiceId);
            Assert.Equal(ALL_FILTERS, dirUpdate.Filter);

            msg15 = eventDictionary[15];
            dirUpdate = msg15.DirectoryUpdate;
            Assert.NotNull(dirUpdate);
            Assert.Equal(15, dirUpdate.StreamId);
            Assert.Single(dirUpdate.ServiceList);
            Assert.Equal(460, dirUpdate.ServiceList[0].ServiceId);
            Assert.Equal(ServiceFilterFlags.STATE, dirUpdate.Filter);

            msg20 = eventDictionary[20];
            dirUpdate = msg20.DirectoryUpdate;
            Assert.NotNull(dirUpdate);
            Assert.Equal(20, dirUpdate.StreamId);
            Assert.Single(dirUpdate.ServiceList);
            Assert.Equal(460, dirUpdate.ServiceList[0].ServiceId);
            Assert.Equal(ServiceFilterFlags.LOAD, dirUpdate.Filter);

            msg25 = eventDictionary[25];
            dirUpdate = msg25.DirectoryUpdate;
            Assert.NotNull(dirUpdate);
            Assert.Equal(25, dirUpdate.StreamId);
            Assert.Single(dirUpdate.ServiceList);
            Assert.Equal(460, dirUpdate.ServiceList[0].ServiceId);
            Assert.Equal(ServiceFilterFlags.STATE | ServiceFilterFlags.LOAD, dirUpdate.Filter);

            directoryUpdate.ServiceList[0].State.ServiceStateVal = 1;
            directoryUpdate.ServiceList[0].State.HasStatus = true;
            directoryUpdate.ServiceList[0].State.Status.DataState(DataStates.SUSPECT);
            directoryUpdate.ServiceList[0].State.Status.StreamState(StreamStates.CLOSED_RECOVER);

            msg.Clear();
            submitOptions.Clear();
            Assert.Equal(CodecReturnCode.SUCCESS, consumer.ReactorChannel.Watchlist.ConvertRDMToCodecMsg(directoryUpdate, msg));
            provider.SubmitAndDispatch(msg, submitOptions);

            consumerReactor.Dispatch(4);
            eventDictionary.Clear();

            for (int i = 0; i < 4; i++)
            {
                var evt = consumerReactor.PollEvent();

                Assert.Equal(TestReactorEventType.DIRECTORY_MSG, evt.EventType);
                var msgEvent = (ReactorMsgEvent)evt.ReactorEvent;
                Assert.Equal(MsgClasses.UPDATE, msgEvent.Msg.MsgClass);
                var dirEvent = msgEvent as RDMDirectoryMsgEvent;
                Assert.NotNull(dirEvent);
                var dirMsg = dirEvent.DirectoryMsg;
                Assert.NotNull(dirMsg);
                Assert.Equal(DirectoryMsgType.UPDATE, dirMsg.DirectoryMsgType);
                eventDictionary.Add(dirMsg.StreamId, dirMsg);
            }

            msg5 = eventDictionary[5];
            dirUpdate = msg5.DirectoryUpdate;
            Assert.NotNull(dirUpdate);
            Assert.Equal(5, dirUpdate.StreamId);
            Assert.Single(dirUpdate.ServiceList);
            Assert.Equal(460, dirUpdate.ServiceList[0].ServiceId);
            Assert.Equal(ALL_FILTERS, dirUpdate.Filter);

            msg15 = eventDictionary[15];
            dirUpdate = msg15.DirectoryUpdate;
            Assert.NotNull(dirUpdate);
            Assert.Equal(15, dirUpdate.StreamId);
            Assert.Single(dirUpdate.ServiceList);
            Assert.Equal(460, dirUpdate.ServiceList[0].ServiceId);
            Assert.Equal(ServiceFilterFlags.STATE, dirUpdate.Filter);

            msg20 = eventDictionary[20];
            dirUpdate = msg20.DirectoryUpdate;
            Assert.NotNull(dirUpdate);
            Assert.Equal(20, dirUpdate.StreamId);
            Assert.Single(dirUpdate.ServiceList);
            Assert.Equal(460, dirUpdate.ServiceList[0].ServiceId);
            Assert.Equal(ServiceFilterFlags.LOAD, dirUpdate.Filter);

            msg25 = eventDictionary[25];
            dirUpdate = msg25.DirectoryUpdate;
            Assert.NotNull(dirUpdate);
            Assert.Equal(25, dirUpdate.StreamId);
            Assert.Single(dirUpdate.ServiceList);
            Assert.Equal(460, dirUpdate.ServiceList[0].ServiceId);
            Assert.Equal(ServiceFilterFlags.STATE | ServiceFilterFlags.LOAD, dirUpdate.Filter);

            TestReactorComponent.CloseSession(consumer, provider);
            consumerReactor.Close();
            providerReactor.Close();
        }
        private void SetUp(out Consumer consumer, out Provider provider, out TestReactor consumerReactor, out TestReactor providerReactor)
        {
            consumerReactor = new TestReactor();
            providerReactor = new TestReactor();

            consumer = new Consumer(consumerReactor);
            provider = new Provider(providerReactor);

            ConsumerRole consumerRole = (ConsumerRole)consumer.ReactorRole;
            consumerRole.WatchlistOptions.EnableWatchlist = true;
            consumerRole.WatchlistOptions.ChannelOpenEventCallback = consumer;
            consumerRole.ChannelEventCallback = consumer;
            consumerRole.LoginMsgCallback = consumer;
            consumerRole.DirectoryMsgCallback = consumer;
            consumerRole.DictionaryMsgCallback = consumer;
            consumerRole.DefaultMsgCallback = consumer;

            ProviderRole providerRole = (ProviderRole)provider.ReactorRole;
            providerRole.ChannelEventCallback = provider;
            providerRole.LoginMsgCallback = provider;
            providerRole.DirectoryMsgCallback = provider;
            providerRole.DictionaryMsgCallback = provider;
            providerRole.DefaultMsgCallback = provider;

            // Connect the consumer and provider
            ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
            opts.SetupDefaultLoginStream = false;
            opts.SetupDefaultDirectoryStream = false;

            provider.Bind(opts);

            TestReactor.OpenSession(consumer, provider, opts);

            consumer.ReactorChannel.Watchlist.LoginHandler = new MockLoginHandler();
            consumer.ReactorChannel.Watchlist.ItemHandler = new MockItemHandler();
        }

        public static DirectoryRefresh CreateDirectoryRefresh(List<string> serviceNames, List<int> serviceIds, List<ServiceFlags> flags, List<MapEntryActions> actions,
            bool solicited, 
            long filter,
            bool clearCache,
            int streamState,
            int dataState
            )
        {
            DirectoryRefresh directoryRefresh = new DirectoryRefresh();
            directoryRefresh.StreamId = 2;
            directoryRefresh.State.StreamState(streamState);
            directoryRefresh.State.DataState(dataState);
            directoryRefresh.Solicited = solicited;
            directoryRefresh.ClearCache = clearCache;
            directoryRefresh.Filter = filter;

            for (int i = 0; i < serviceNames.Count; i++)
            {
                Service s = new Service();
                ServiceBuilder.BuildRDMService(s, flags[i], actions[i], FilterEntryActions.SET);
                s.ServiceId = serviceIds[i];
                if (s.HasInfo)
                {
                    s.Info.ServiceName.Data(serviceNames[i]);
                }
                directoryRefresh.ServiceList.Add(s);
            }

            return directoryRefresh;
        }

        public static DirectoryUpdate CreateDirectoryUpdate(List<string> serviceNames,
            List<int> serviceIds, 
            List<ServiceFlags> flags, 
            List<MapEntryActions> actions,
            long filter
            )
        {
            DirectoryUpdate directoryUpdate = new DirectoryUpdate();
            directoryUpdate.StreamId = 2;
            directoryUpdate.Filter = filter;

            for (int i = 0; i < serviceNames.Count; i++)
            {
                Service s = new Service();
                ServiceBuilder.BuildRDMService(s, flags[i], actions[i], FilterEntryActions.SET);
                s.ServiceId = serviceIds[i];
                if (s.HasInfo)
                {
                    s.Info.ServiceName.Data(serviceNames[i]);
                }
                directoryUpdate.ServiceList.Add(s);
            }

            return directoryUpdate;
        }

        public static DirectoryRequest CreateDirectoryRequest(int serviceId, long filter, int streamId = 2)
        {
            DirectoryRequest directoryRequest = new DirectoryRequest();

            directoryRequest.StreamId = streamId;
            directoryRequest.HasServiceId = true;
            directoryRequest.ServiceId = serviceId;
            directoryRequest.Filter = filter;

            return directoryRequest;
        }

    }
    internal class MockLoginHandler : IWlLoginHandler
    {
        public bool SupportSingleOpen { get; set; }

        public WlStream Stream { get; set; }

        public bool SupportAllowSuspectData => throw new NotImplementedException();

        public LoginRequest LoginRequestForEDP => throw new NotImplementedException();

        public bool IsRttEnabled { get => throw new NotImplementedException(); set => throw new NotImplementedException(); }

        public bool SupportPost => true;

        public bool SupportOptimizedPauseResume => true;

        public void AddPendingRequest(WlStream wlStream)
        {
            return;
        }
        public ReactorReturnCode CallbackUserWithMsg(string location, IMsg msg, WlRequest wlRequest, out ReactorErrorInfo errorInfo)
        {
            errorInfo = null;
            return ReactorReturnCode.SUCCESS;
        }

        public ReactorReturnCode CallbackUserWithMsgBase(string location, IMsg msg, IRdmMsg msgBase, WlRequest wlRequest, out ReactorErrorInfo errorInfo)
        {
            errorInfo = null;
            return ReactorReturnCode.SUCCESS;
        }

        public void ChannelDown()
        {
            return;
        }

        public void ChannelUp(out ReactorErrorInfo errorInfo)
        {
            throw new NotImplementedException();
        }

        public void Clear()
        {
            return;
        }

        public ReactorReturnCode Dispatch(out ReactorErrorInfo errorInfo)
        {
            errorInfo = null;
            return ReactorReturnCode.SUCCESS;
        }

        public void OnRequestTimeout(WlStream wlStream)
        {
            throw new NotImplementedException();
        }

        public ReactorReturnCode ReadMsg(WlStream wlStream, DecodeIterator decodeIt, IMsg msg, out ReactorErrorInfo errorInfo)
        {
            errorInfo = null;
            return ReactorReturnCode.SUCCESS;
        }

        public ReactorReturnCode SubmitMsg(WlRequest request, IMsg msg, ReactorSubmitOptions submitOptions, out ReactorErrorInfo errorInfo)
        {
            errorInfo = null;
            return ReactorReturnCode.SUCCESS;
        }

        public ReactorReturnCode SubmitRequest(WlRequest request, IRequestMsg requestMsg, bool isReissue, ReactorSubmitOptions submitOptions, out ReactorErrorInfo errorInfo)
        {
            errorInfo = null;
            return ReactorReturnCode.SUCCESS;
        }
    }
    internal class MockItemHandler : IWlItemHandler
    {
        public void AddPendingRequest(WlStream wlStream)
        {
            return;
        }

        public ReactorReturnCode CallbackUserWithMsg(string location, IMsg msg, WlRequest wlRequest, out ReactorErrorInfo errorInfo)
        {
            errorInfo = null;
            return ReactorReturnCode.SUCCESS;
        }

        public ReactorReturnCode CallbackUserWithMsgBase(string location, IMsg msg, IRdmMsg msgBase, WlRequest wlRequest, out ReactorErrorInfo errorInfo)
        {
            errorInfo = null;
            return ReactorReturnCode.SUCCESS;
        }
        public void ChannelDown()
        {
            return;
        }
        public void ChannelUp(out ReactorErrorInfo errorInfo)
        {
            throw new NotImplementedException();
        }

        public void Clear()
        {
            return;
        }

        public ReactorReturnCode Dispatch(out ReactorErrorInfo errorInfo)
        {
            errorInfo = null;
            return ReactorReturnCode.SUCCESS;
        }

        public void Init(ConsumerRole consumerRole)
        {
        }

        public bool IsRequestRecoverable(WlRequest wlRequest, int streamState)
        {
            return true;
        }

        public ReactorReturnCode LoginStreamClosed(State state)
        {
            throw new NotImplementedException();
        }

        public ReactorReturnCode LoginStreamOpen(out ReactorErrorInfo errorInfo)
        {
            throw new NotImplementedException();
        }

        public void OnRequestTimeout(WlStream stream)
        {
            throw new NotImplementedException();
        }

        public ReactorReturnCode PauseAll()
        {
            throw new NotImplementedException();
        }

        public ReactorReturnCode ReadMsg(WlStream wlStream, DecodeIterator decodeIt, IMsg msg, out ReactorErrorInfo errorInfo)
        {
            errorInfo = null;
            return ReactorReturnCode.SUCCESS;
        }

        public ReactorReturnCode ResumeAll()
        {
            throw new NotImplementedException();
        }

        public ReactorReturnCode SubmitMsg(WlRequest request, IMsg msg, ReactorSubmitOptions submitOptions, out ReactorErrorInfo errorInfo)
        {
            errorInfo = null;
            return ReactorReturnCode.SUCCESS;
        }

        public ReactorReturnCode SubmitRequest(WlRequest request, IRequestMsg requestMsg, bool isReissue, ReactorSubmitOptions submitOptions, out ReactorErrorInfo errorInfo)
        {
            errorInfo = null;
            return ReactorReturnCode.SUCCESS;
        }
    }
}
