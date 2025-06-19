/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using LSEG.Eta.Common;
using LSEG.Eta.ValueAdd.Rdm;
using System;
using System.Collections.Generic;
using Xunit;
using static LSEG.Eta.Rdm.Directory;
using Buffer = LSEG.Eta.Codec.Buffer;
using DataTypes = LSEG.Eta.Codec.DataTypes;

namespace LSEG.Eta.ValuedAdd.Tests
{
    public class DirectoryTest
    {
        private DecodeIterator dIter = new DecodeIterator();
        private EncodeIterator encIter = new EncodeIterator();
        private Msg msg = new Msg();

        private void BuildRDMServiceGroup(List<ServiceGroup> groupStateList, FilterEntryActions action)
        {
            State state = new State();
            state.Text().Data("state");
            state.Code(StateCodes.FAILOVER_COMPLETED);
            state.DataState(DataStates.SUSPECT);
            state.StreamState(StreamStates.OPEN);

            ServiceGroup rdmServiceGroupState = new ServiceGroup();
            rdmServiceGroupState.Clear();
            rdmServiceGroupState.Flags = ServiceGroupFlags.HAS_MERGED_TO_GROUP | ServiceGroupFlags.HAS_STATUS;
            rdmServiceGroupState.Action = action;
            rdmServiceGroupState.Group.Data("group");
            if (rdmServiceGroupState.HasMergedToGroup)
            {
                rdmServiceGroupState.MergedToGroup.Data("mergedToGroup");
            }

            if (rdmServiceGroupState.HasStatus)
            {
                rdmServiceGroupState.Status.Text().Data("state");
                rdmServiceGroupState.Status.Code(StateCodes.FAILOVER_COMPLETED);
                rdmServiceGroupState.Status.DataState(DataStates.SUSPECT);
                rdmServiceGroupState.Status.StreamState(StreamStates.OPEN);
            }

            groupStateList.Add(rdmServiceGroupState);
        }

        private void BuildRDMServiceData(ServiceData rdmServiceData, FilterEntryActions action)
        {
            rdmServiceData.Clear();

            rdmServiceData.Flags = ServiceDataFlags.HAS_DATA;
            rdmServiceData.Action = action;
            rdmServiceData.Type = 1;
            if (rdmServiceData.HasData)
            {
                rdmServiceData.Data.Data("data");
                rdmServiceData.DataType = Codec.DataTypes.ASCII_STRING;
            }
        }

        private void BuildRDMServiceLoad(ServiceLoad rdmServiceLoad, FilterEntryActions action)
        {
            long loadFactor = 1;
            long openLimit = 1;
            long openWindow = 1;

            rdmServiceLoad.Clear();

            rdmServiceLoad.Flags = ServiceLoadFlags.HAS_LOAD_FACTOR | ServiceLoadFlags.HAS_OPEN_LIMIT | ServiceLoadFlags.HAS_OPEN_WINDOW;
            rdmServiceLoad.Action = action;
            if (rdmServiceLoad.HasOpenLimit)
            {
                rdmServiceLoad.OpenLimit = openLimit;
            }

            if (rdmServiceLoad.HasOpenWindow)
            {
                rdmServiceLoad.OpenWindow = openWindow;
            }

            if (rdmServiceLoad.HasLoadFactor)
            {
                rdmServiceLoad.LoadFactor = loadFactor;
            }
        }

        private void BuildRDMServiceState(ServiceState rdmServiceState, FilterEntryActions action)
        {
            long acceptingRequests = 1;
            long serviceState = 1;

            rdmServiceState.Clear();

            rdmServiceState.Flags = ServiceStateFlags.HAS_ACCEPTING_REQS | ServiceStateFlags.HAS_STATUS;
            rdmServiceState.Action = action;
            rdmServiceState.ServiceStateVal = serviceState;

            rdmServiceState.AcceptingRequests = acceptingRequests;

            if (rdmServiceState.HasStatus)
            {
                rdmServiceState.Status.Text().Data("state");
                rdmServiceState.Status.Code(StateCodes.FAILOVER_COMPLETED);
                rdmServiceState.Status.DataState(DataStates.SUSPECT);
                rdmServiceState.Status.StreamState(StreamStates.OPEN);
            }
        }

        private void BuildRDMServiceLink(ServiceLinkInfo serviceLinkInfo, FilterEntryActions action)
        {
            long linkCode = 1;
            long linkState = 1;
            long type = DataTypes.ASCII_STRING;

            ServiceLink serviceLink = new ServiceLink();

            serviceLinkInfo.LinkList.Add(serviceLink);
            serviceLinkInfo.Action = action;
            serviceLink.Clear();

            serviceLink.Action = MapEntryActions.ADD;
            serviceLink.Flags = ServiceLinkFlags.HAS_CODE | ServiceLinkFlags.HAS_TEXT | ServiceLinkFlags.HAS_TYPE;
            serviceLink.Name.Data("name");
            serviceLink.LinkState = linkState;

            if (serviceLink.HasCode)
            {
                serviceLink.LinkCode = linkCode;
            }

            if (serviceLink.HasText)
            {
                serviceLink.Text.Data("text");
            }

            if (serviceLink.HasType)
            {
                serviceLink.Type = type;
            }
        }

        private void BuildRDMServiceInfo(ServiceInfo rdmServiceInfo, FilterEntryActions action)
        {
            ServiceInfoFlags flags = ServiceInfoFlags.HAS_ACCEPTING_CONS_STATUS 
                | ServiceInfoFlags.HAS_DICTS_PROVIDED 
                | ServiceInfoFlags.HAS_DICTS_USED 
                | ServiceInfoFlags.HAS_IS_SOURCE 
                | ServiceInfoFlags.HAS_ITEM_LIST 
                | ServiceInfoFlags.HAS_QOS 
                | ServiceInfoFlags.HAS_SUPPORT_OOB_SNAPSHOTS 
                | ServiceInfoFlags.HAS_SUPPORT_QOS_RANGE 
                | ServiceInfoFlags.HAS_VENDOR;

            Qos qos = new Qos();
            qos.IsDynamic = true;
            qos.Rate(QosRates.JIT_CONFLATED);
            qos.Timeliness(QosTimeliness.DELAYED);
            qos.TimeInfo(1);
            rdmServiceInfo.Clear();
            rdmServiceInfo.Action = action;
            rdmServiceInfo.Flags = flags;
            rdmServiceInfo.AcceptConsumerStatus = 1;
            rdmServiceInfo.DictionariesProvidedList.Add("dictprov1");
            rdmServiceInfo.DictionariesUsedList.Add("dictused1");
            rdmServiceInfo.IsSource = 1;
            rdmServiceInfo.ItemList.Data("itemList");
            rdmServiceInfo.QosList.Add(qos);
            rdmServiceInfo.SupportsOOBSnapshots = 1;
            rdmServiceInfo.SupportsQosRange = 1;
            rdmServiceInfo.Vendor.Data("vendor");
            rdmServiceInfo.CapabilitiesList.Add(1);
            rdmServiceInfo.ServiceName.Data("servicename");
        }

        private void BuildRDMService(Service rdmService, ServiceFlags flags, MapEntryActions serviceAddOrDeleteAction, FilterEntryActions filterAddOrClearAction)
        {
            rdmService.Clear();
            rdmService.Action = serviceAddOrDeleteAction;
            rdmService.Flags = flags;

            // checking only set action for the filters
            // other filter unit tests cover other filter actions
            if (rdmService.HasInfo)
                BuildRDMServiceInfo(rdmService.Info, filterAddOrClearAction);
            if (rdmService.HasLink)
                BuildRDMServiceLink(rdmService.Link, filterAddOrClearAction);
            if (rdmService.HasState)
                BuildRDMServiceState(rdmService.State, filterAddOrClearAction);
            if (rdmService.HasLoad)
                BuildRDMServiceLoad(rdmService.Load, filterAddOrClearAction);
            if (rdmService.HasData)
                BuildRDMServiceData(rdmService.Data, filterAddOrClearAction);
            BuildRDMServiceGroup(rdmService.GroupStateList, filterAddOrClearAction);
        }

        private void VerifyServiceInfo(ServiceInfo rdmServiceInfo, ServiceInfo rdmServiceInfoDec)
        {
            Assert.True(rdmServiceInfo.Flags == rdmServiceInfoDec.Flags);
            Assert.True(rdmServiceInfo.Action == rdmServiceInfoDec.Action);
            if (rdmServiceInfo.HasAcceptingConsStatus)
            {
                Assert.True(rdmServiceInfo.AcceptConsumerStatus == rdmServiceInfoDec.AcceptConsumerStatus);
            }

            if (rdmServiceInfo.HasDictionariesProvided)
            {
                Assert.True(rdmServiceInfo.DictionariesProvidedList.Count == rdmServiceInfoDec.DictionariesProvidedList.Count);
                Assert.Equal(rdmServiceInfo.DictionariesProvidedList[0], rdmServiceInfoDec.DictionariesProvidedList[0]);
            }

            if (rdmServiceInfo.HasDictionariesUsed)
            {
                Assert.True(rdmServiceInfo.DictionariesUsedList.Count == rdmServiceInfoDec.DictionariesUsedList.Count);
                Assert.True(rdmServiceInfo.DictionariesUsedList[0] == rdmServiceInfoDec.DictionariesUsedList[0]);
            }

            if (rdmServiceInfo.HasIsSource)
            {
                Assert.Equal(rdmServiceInfo.IsSource, rdmServiceInfoDec.IsSource);
            }
            if (rdmServiceInfo.HasItemList)
            {
                Assert.Equal(rdmServiceInfo.ItemList.ToString(), rdmServiceInfoDec.ItemList.ToString());
            }

            if (rdmServiceInfo.HasQos)
            {
                Assert.Equal(rdmServiceInfo.QosList.Count, rdmServiceInfoDec.QosList.Count);
                Qos qos = rdmServiceInfo.QosList[0];
                Qos qosDec = rdmServiceInfoDec.QosList[0];
                Assert.Equal(qos.Rate(), qosDec.Rate());
                Assert.Equal(qos.IsDynamic, qosDec.IsDynamic);
                Assert.Equal(qos.Timeliness(), qosDec.Timeliness());
                Assert.Equal(qos.TimeInfo(), qosDec.TimeInfo());
                Assert.Equal(qos.RateInfo(), qosDec.RateInfo());
            }

            if (rdmServiceInfo.HasSupportOOBSnapshots)
            {
                Assert.Equal(rdmServiceInfo.SupportsOOBSnapshots, rdmServiceInfoDec.SupportsOOBSnapshots);
            }
            if (rdmServiceInfo.HasSupportQosRange)
            {
                Assert.Equal(rdmServiceInfo.SupportsQosRange, rdmServiceInfoDec.SupportsQosRange);
            }

            if (rdmServiceInfo.HasVendor)
            {
                Assert.Equal(rdmServiceInfo.Vendor.ToString(), rdmServiceInfoDec.Vendor.ToString());
            }
            Assert.Equal(rdmServiceInfo.CapabilitiesList.Count, rdmServiceInfoDec.CapabilitiesList.Count);
            Assert.Equal(rdmServiceInfo.CapabilitiesList[0], rdmServiceInfoDec.CapabilitiesList[0]);
            Assert.Equal(rdmServiceInfo.ServiceName.ToString(), rdmServiceInfoDec.ServiceName.ToString());
        }

        private void VerifyServiceLoad(ServiceLoad rdmServiceLoad, ServiceLoad rdmServiceLoadDec)
        {
            Assert.True(rdmServiceLoad.Action == rdmServiceLoadDec.Action);
            if (rdmServiceLoadDec.HasLoadFactor)
                Assert.True(rdmServiceLoad.LoadFactor == rdmServiceLoadDec.LoadFactor);

            if (rdmServiceLoadDec.HasOpenLimit)
                Assert.True(rdmServiceLoad.OpenLimit == rdmServiceLoadDec.OpenLimit);

            if (rdmServiceLoadDec.HasOpenWindow)
                Assert.True(rdmServiceLoad.OpenWindow == rdmServiceLoadDec.OpenWindow);
        }

        private void VerifyServiceLinkList(ServiceLinkInfo linkList, ServiceLinkInfo linkListDec)
        {
            Assert.Equal(linkList.LinkList.Count, linkListDec.LinkList.Count);
            Assert.Equal(linkList.Action, linkList.Action);
            ServiceLink rdmServiceLink = linkList.LinkList[0];
            ServiceLink rdmServiceLinkDec = linkListDec.LinkList[0];
            Assert.Equal(rdmServiceLink.Flags, rdmServiceLinkDec.Flags);
            Assert.Equal(rdmServiceLink.LinkState, rdmServiceLinkDec.LinkState);
            if (rdmServiceLinkDec.HasCode)
                Assert.Equal(rdmServiceLink.LinkCode, rdmServiceLinkDec.LinkCode);
            if (rdmServiceLink.HasType)
                Assert.Equal(rdmServiceLink.Type, rdmServiceLinkDec.Type);
            if (rdmServiceLink.HasText)
                Assert.Equal(rdmServiceLink.Text.ToString(), rdmServiceLinkDec.Text.ToString());
        }

        private void VerifyServiceState(ServiceState rdmServiceState, ServiceState rdmServiceState2)
        {
            Assert.Equal(rdmServiceState.Flags, rdmServiceState2.Flags);
            Assert.Equal(rdmServiceState.Action, rdmServiceState2.Action);
            if (rdmServiceState2.HasAcceptingRequests)
                Assert.Equal(rdmServiceState.AcceptingRequests, rdmServiceState2.AcceptingRequests);
            Assert.Equal(rdmServiceState.ServiceStateVal, rdmServiceState2.ServiceStateVal);

            if (rdmServiceState2.HasStatus)
            {
                State state = rdmServiceState.Status;
                State decState = rdmServiceState2.Status;
                Assert.NotNull(decState);
                Assert.Equal(state.Code(), decState.Code());
                Assert.Equal(state.DataState(), decState.DataState());
                Assert.Equal(state.StreamState(), decState.StreamState());
                Assert.Equal(state.Text().ToString(), decState.Text().ToString());
            }
        }

        private void VerifyServiceData(ServiceData rdmServiceData, ServiceData rdmServiceData2)
        {
            Assert.Equal(rdmServiceData.Flags, rdmServiceData2.Flags);
            Assert.Equal(rdmServiceData.Type, rdmServiceData2.Type);
            Assert.Equal(rdmServiceData.Action, rdmServiceData2.Action);
            if (rdmServiceData.HasData)
            {
                Assert.Equal(rdmServiceData.DataType, rdmServiceData2.DataType);
                Assert.Equal(rdmServiceData.Data.ToString(), rdmServiceData2.Data.ToString());
            }
        }

        private void VerifyServiceGroupState(ServiceGroup rdmServiceGroupState, ServiceGroup rdmServiceGroupState2)
        {
            Assert.Equal(rdmServiceGroupState.Flags, rdmServiceGroupState2.Flags);
            Assert.Equal(rdmServiceGroupState.Action, rdmServiceGroupState2.Action);
            if (rdmServiceGroupState2.HasMergedToGroup)
                Assert.Equal(rdmServiceGroupState.MergedToGroup.ToString(), rdmServiceGroupState2.MergedToGroup.ToString());
            Assert.Equal(rdmServiceGroupState.Group.ToString(), rdmServiceGroupState2.Group.ToString());

            if (rdmServiceGroupState2.HasStatus)
            {
                State decState = rdmServiceGroupState2.Status;
                State state = rdmServiceGroupState.Status;
                Assert.NotNull(decState);
                Assert.Equal(state.Code(), decState.Code());
                Assert.Equal(state.DataState(), decState.DataState());
                Assert.Equal(state.StreamState(), decState.StreamState());
                Assert.Equal(state.Text().ToString(), decState.Text().ToString());
            }
        }

        private void VerifyRDMService(Service rdmService, Service rdmServiceDec)
        {
            Assert.True(rdmService.Flags == rdmServiceDec.Flags);
            Assert.True(rdmService.Action == rdmServiceDec.Action);

            if (rdmService.HasInfo && rdmService.Info.Action != FilterEntryActions.CLEAR)
            {
                VerifyServiceInfo(rdmService.Info, rdmServiceDec.Info);
            }
            if (rdmService.HasLink && rdmService.Link.Action != FilterEntryActions.CLEAR)
            {
                VerifyServiceLinkList(rdmService.Link, rdmServiceDec.Link);
            }
            if (rdmService.HasState && rdmService.State.Action != FilterEntryActions.CLEAR)
            {
                VerifyServiceState(rdmService.State, rdmServiceDec.State);
            }
            if (rdmService.HasLoad && rdmService.Load.Action != FilterEntryActions.CLEAR)
            {
                VerifyServiceLoad(rdmService.Load, rdmServiceDec.Load);
            }
            if (rdmService.HasData && rdmService.Data.Action != FilterEntryActions.CLEAR)
            {
                VerifyServiceData(rdmService.Data, rdmServiceDec.Data);
            }
            Assert.True(rdmService.GroupStateList.Count == rdmServiceDec.GroupStateList.Count);
            ServiceGroup group = rdmService.GroupStateList[0];

            if (group.Action != FilterEntryActions.CLEAR)
            {
                ServiceGroup groupDec = rdmServiceDec.GroupStateList[0];
                VerifyServiceGroupState(group, groupDec);
            }
        }

        [Fact]
        public void ServiceCopyTests()
        {
            ServiceFlags[] flagsBase =
            {
                ServiceFlags.HAS_DATA,
                ServiceFlags.HAS_INFO,
                ServiceFlags.HAS_LINK,
                ServiceFlags.HAS_LOAD,
                ServiceFlags.HAS_STATE
            };

            ServiceFlags[] flagsList = TypedMessageUtil.CreateFlagCombinations(flagsBase, false);

            Service rdmService1 = new Service();
            Service rdmService2 = new Service();

            FilterEntryActions[] filterActions = { FilterEntryActions.CLEAR, FilterEntryActions.SET, FilterEntryActions.UPDATE };
            Console.WriteLine("Service copy tests...");

            for (int i = 0; i < flagsList.Length; i++)
            {
                foreach (FilterEntryActions filterAction in filterActions)
                {
                    BuildRDMService(rdmService1, flagsList[i], MapEntryActions.ADD, filterAction);
                    CodecReturnCode ret = rdmService1.Copy(rdmService2);
                    Assert.True(CodecReturnCode.SUCCESS == ret);
                    VerifyRDMService(rdmService1, rdmService2);
                }
            }
            Console.WriteLine("Done.");
        }

        [Fact]
        public void ServiceTests()
        {
            ServiceFlags[] flagsBase =
            {
                ServiceFlags.HAS_DATA,
                ServiceFlags.HAS_INFO,
                ServiceFlags.HAS_LINK,
                ServiceFlags.HAS_LOAD,
                ServiceFlags.HAS_STATE
            };

            ServiceFlags[] flagsList = TypedMessageUtil.CreateFlagCombinations(flagsBase, false);

            Service rdmService = new Service();
            Service rdmServiceDec = new Service();

            FilterEntryActions[] filterActions = { FilterEntryActions.CLEAR, FilterEntryActions.SET, FilterEntryActions.UPDATE };
            Console.WriteLine("Service tests...");

            for (int i = 0; i < flagsList.Length; i++)
            {
                foreach (FilterEntryActions filterAction in filterActions)
                {
                    rdmServiceDec.Clear();

                    BuildRDMService(rdmService, flagsList[i], MapEntryActions.ADD, filterAction);
                    // allocate a ByteBuffer and associate it with an RsslBuffer
                    ByteBuffer bb = new ByteBuffer(1024);
                    Buffer buffer = new Buffer();
                    buffer.Data(bb);

                    encIter.Clear();
                    encIter.SetBufferAndRWFVersion(buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
                    CodecReturnCode ret = rdmService.Encode(encIter);
                    Assert.Equal(CodecReturnCode.SUCCESS, ret);
                    dIter.Clear();
                    bb.Flip();
                    buffer.Data(bb);
                    dIter.SetBufferAndRWFVersion(buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
                    ret = rdmServiceDec.Decode(dIter);

                    Assert.Equal(CodecReturnCode.SUCCESS, ret);
                    VerifyRDMService(rdmService, rdmServiceDec);
                }
            }
            Console.WriteLine("Done.");
        }
        
        [Fact]
        public void DirecoryConnStatusTests()
        {
            DirectoryConsumerStatus encRDMMsg = new DirectoryConsumerStatus();
            DirectoryConsumerStatus decRDMMsg = new DirectoryConsumerStatus();

            // Parameters to test with
            int streamId = -5;

            List<ConsumerStatusService> consumerStatusServiceList = new List<ConsumerStatusService>(4);

            ConsumerStatusService consumerStatusService1 = new ConsumerStatusService();
            consumerStatusService1.SourceMirroringModeVal = SourceMirroringMode.ACTIVE_WITH_STANDBY;
            consumerStatusService1.ServiceId = 2;
            consumerStatusService1.Action = MapEntryActions.ADD;
            consumerStatusServiceList.Add(consumerStatusService1);

            ConsumerStatusService consumerStatusService2 = new ConsumerStatusService();
            consumerStatusService2.SourceMirroringModeVal = SourceMirroringMode.ACTIVE_NO_STANDBY;
            consumerStatusService2.ServiceId = 4;
            consumerStatusService2.Action = MapEntryActions.ADD;
            consumerStatusServiceList.Add(consumerStatusService2);

            ConsumerStatusService consumerStatusService3 = new ConsumerStatusService();
            consumerStatusService3.SourceMirroringModeVal = SourceMirroringMode.ACTIVE_NO_STANDBY;
            consumerStatusService3.ServiceId = 5;
            consumerStatusService3.Action = MapEntryActions.DELETE;
            consumerStatusServiceList.Add(consumerStatusService3);

            ConsumerStatusService consumerStatusService4 = new ConsumerStatusService();
            consumerStatusService4.SourceMirroringModeVal = SourceMirroringMode.STANDBY;
            consumerStatusService4.ServiceId = 6;
            consumerStatusService4.Action = MapEntryActions.UPDATE;
            consumerStatusServiceList.Add(consumerStatusService4);

            Console.WriteLine("DirectoryConsumerStatus tests...");

            for (int k = 0; k <= consumerStatusServiceList.Count; ++k)
            {
                dIter.Clear();
                encIter.Clear();
                encRDMMsg.Clear();
                encRDMMsg.StreamId = streamId;

                // Encode increasing number of source mirroring info elements
                if (k >= 1)
                {
                    for (int i = 0; i < k; ++i)
                    {
                        encRDMMsg.ConsumerServiceStatusList.Add(consumerStatusServiceList[i]);
                    }
                }

                Buffer membuf = new Buffer();
                membuf.Data(new ByteBuffer(1024));

                encIter.SetBufferAndRWFVersion(membuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
                CodecReturnCode ret = encRDMMsg.Encode(encIter);
                Assert.Equal(CodecReturnCode.SUCCESS, ret);

                dIter.SetBufferAndRWFVersion(membuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
                ret = msg.Decode(dIter);
                Assert.Equal(CodecReturnCode.SUCCESS, ret);
                ret = decRDMMsg.Decode(dIter, msg);
                Assert.Equal(CodecReturnCode.SUCCESS, ret);

                Assert.Equal(encRDMMsg.StreamId, decRDMMsg.StreamId);
                Assert.Equal(k, decRDMMsg.ConsumerServiceStatusList.Count);

                for (int i = 0; i < k; ++i)
                {
                    ConsumerStatusService decConsStatusService = decRDMMsg.ConsumerServiceStatusList[i];
                    Assert.Equal(consumerStatusServiceList[i].ServiceId, decConsStatusService.ServiceId);
                    Assert.Equal(consumerStatusServiceList[i].Action, decConsStatusService.Action);
                    if (decConsStatusService.Action != MapEntryActions.DELETE)
                    {
                        Assert.Equal(consumerStatusServiceList[i].SourceMirroringModeVal, decConsStatusService.SourceMirroringModeVal);
                    }

                }
            }
        }

        [Fact]
        public void DirectoryConnStatusCopyTests()
        {
            DirectoryConsumerStatus directoryConnStatusMsg1 = new DirectoryConsumerStatus();
            DirectoryConsumerStatus directoryConnStatusMsg2 = new DirectoryConsumerStatus();

            /* Parameters to test with */
            int streamId = -5;
            UInt serviceId = new UInt();
            serviceId.Value(1);

            long sourceMirroringMode = 1;

            ConsumerStatusService consumerStatusService1 = new ConsumerStatusService();
            consumerStatusService1.SourceMirroringModeVal = sourceMirroringMode;
            consumerStatusService1.Action = MapEntryActions.UPDATE;

            Console.WriteLine("DirectoryConsumerStatus copy tests...");
            directoryConnStatusMsg1.ConsumerServiceStatusList.Add(consumerStatusService1);
            directoryConnStatusMsg1.StreamId = streamId;
            CodecReturnCode ret = directoryConnStatusMsg1.Copy(directoryConnStatusMsg2);

            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            // Verify deep copy
            Assert.Equal(directoryConnStatusMsg1.StreamId, directoryConnStatusMsg2.StreamId);
            Assert.True(directoryConnStatusMsg1.ConsumerServiceStatusList != directoryConnStatusMsg2.ConsumerServiceStatusList);
            Assert.Equal(directoryConnStatusMsg1.ConsumerServiceStatusList.Count, directoryConnStatusMsg2.ConsumerServiceStatusList.Count);
            ConsumerStatusService consStatusService1 = directoryConnStatusMsg1.ConsumerServiceStatusList[0];
            Assert.Equal(consumerStatusService1.SourceMirroringModeVal, consStatusService1.SourceMirroringModeVal);

            Console.WriteLine("Done.");
        }

        [Fact]
        public void ConsumerStatusServiceTests()
        {
            ConsumerStatusService consumerStatusService = new ConsumerStatusService();
            ConsumerStatusService consumerStatusServiceDec = new ConsumerStatusService();

            long sourceMirroringMode = 1;
            consumerStatusService.SourceMirroringModeVal = sourceMirroringMode;
            Console.WriteLine("ConsumerStatusService tests...");
            // allocate a ByteBuffer and associate it with an RsslBuffer
            ByteBuffer bb = new ByteBuffer(1024);
            Buffer buffer = new Buffer();
            buffer.Data(bb);

            encIter.Clear();
            encIter.SetBufferAndRWFVersion(buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
            CodecReturnCode ret = consumerStatusService.Encode(encIter);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);
            dIter.Clear();
            dIter.SetBufferAndRWFVersion(buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
            ret = consumerStatusServiceDec.Decode(dIter, msg);

            Assert.Equal(CodecReturnCode.SUCCESS, ret);
            Assert.Equal(consumerStatusService.SourceMirroringModeVal, consumerStatusServiceDec.SourceMirroringModeVal);

            Console.WriteLine("Done.");
        }

        [Fact]
        public void ServiceDataTests()
        {
            ServiceDataFlags[] flagsBase =
            {
                ServiceDataFlags.HAS_DATA
            };

            ServiceDataFlags[] flagsList = TypedMessageUtil.CreateFlagCombinations(flagsBase, false);

            ServiceData rdmServiceData = new ServiceData();
            ServiceData rdmServiceDataDec = new ServiceData();

            FilterEntryActions action = FilterEntryActions.SET;
            Console.WriteLine("ServiceData copy tests...");

            for (int i = 0; i < flagsList.Length; i++)
            {
                rdmServiceData.Clear();
                rdmServiceDataDec.Clear();

                rdmServiceData.Flags = flagsList[i];
                rdmServiceData.Action = action;
                if (rdmServiceData.HasData)
                {
                    rdmServiceData.Data.Data("data");
                    rdmServiceData.DataType = DataTypes.ASCII_STRING;
                }

                // allocate a ByteBuffer and associate it with an RsslBuffer
                ByteBuffer bb = new ByteBuffer(1024);
                Buffer buffer = new Buffer();
                buffer.Data(bb);

                encIter.Clear();
                encIter.SetBufferAndRWFVersion(buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
                CodecReturnCode ret = rdmServiceData.Encode(encIter);
                Assert.Equal(CodecReturnCode.SUCCESS, ret);
                dIter.Clear();
                bb.Flip();
                buffer.Data(bb);
                dIter.SetBufferAndRWFVersion(buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
                ret = rdmServiceDataDec.Decode(dIter);

                Assert.Equal(CodecReturnCode.SUCCESS, ret);
                Assert.Equal(rdmServiceData.Flags, rdmServiceDataDec.Flags);
                Assert.Equal(rdmServiceData.DataType, rdmServiceDataDec.DataType);
                if (rdmServiceData.HasData)
                    Assert.Equal(rdmServiceData.Data.ToString(), rdmServiceDataDec.Data.ToString());
            }
            Console.WriteLine("Done.");
        }

        [Fact]
        public void ServiceStateCopyTests()
        {
            ServiceStateFlags[] flagsBase =
            {
                ServiceStateFlags.HAS_ACCEPTING_REQS,
                ServiceStateFlags.HAS_STATUS
            };

            ServiceStateFlags[] flagsList = TypedMessageUtil.CreateFlagCombinations(flagsBase, false);

            ServiceState rdmServiceState = new ServiceState();
            ServiceState rdmServiceState2 = new ServiceState();

            FilterEntryActions action = FilterEntryActions.SET;
            long acceptingRequests = 1;
            long serviceState = 1;

            Console.WriteLine("ServiceState copy tests...");

            for (int i = 0; i < flagsList.Length; i++)
            {
                rdmServiceState.Clear();
                rdmServiceState2.Clear();

                rdmServiceState.Flags = flagsList[i];
                rdmServiceState.Action = action;
                rdmServiceState.ServiceStateVal = serviceState;

                if (rdmServiceState.HasAcceptingRequests)
                {
                    rdmServiceState.AcceptingRequests = acceptingRequests;
                    rdmServiceState.HasAcceptingRequests = true;
                }

                if (rdmServiceState.HasStatus)
                {
                    rdmServiceState.Status.Text().Data("state");
                    rdmServiceState.Status.Code(StateCodes.FAILOVER_COMPLETED);
                    rdmServiceState.Status.DataState(DataStates.SUSPECT);
                    rdmServiceState.Status.StreamState(StreamStates.OPEN);
                    rdmServiceState.HasStatus = true;
                }

                CodecReturnCode ret = rdmServiceState.Copy(rdmServiceState2);

                Assert.Equal(CodecReturnCode.SUCCESS, ret);
                VerifyServiceState(rdmServiceState, rdmServiceState2);

            }
            Console.WriteLine("Done.");
        }

        [Fact]
        public void ServiceGroupStateTests()
        {
            ServiceGroupFlags[] flagsBase =
            {
                ServiceGroupFlags.HAS_MERGED_TO_GROUP,
                ServiceGroupFlags.HAS_STATUS
            };

            ServiceGroupFlags[] flagsList = TypedMessageUtil.CreateFlagCombinations(flagsBase, false);

            ServiceGroup rdmServiceGroupState = new ServiceGroup();
            ServiceGroup rdmServiceGroupStateDec = new ServiceGroup();

            FilterEntryActions action = FilterEntryActions.SET;
            State state = new State();
            state.Text().Data("state");
            state.Code(StateCodes.FAILOVER_COMPLETED);
            state.DataState(DataStates.SUSPECT);
            state.StreamState(StreamStates.OPEN);

            Console.WriteLine("ServiceGroup copy tests...");

            for (int i = 0; i < flagsList.Length; i++)
            {
                rdmServiceGroupState.Clear();
                rdmServiceGroupStateDec.Clear();

                rdmServiceGroupState.Flags = flagsList[i];
                rdmServiceGroupState.Action = action;
                rdmServiceGroupState.Group.Data("group");
                if (rdmServiceGroupState.HasMergedToGroup)
                {
                    rdmServiceGroupState.MergedToGroup.Data("mergedToGroup");
                }

                if (rdmServiceGroupState.HasStatus)
                {
                    rdmServiceGroupState.Status.Text().Data("state");
                    rdmServiceGroupState.Status.Code(StateCodes.FAILOVER_COMPLETED);
                    rdmServiceGroupState.Status.DataState(DataStates.SUSPECT);
                    rdmServiceGroupState.Status.StreamState(StreamStates.OPEN);
                }

                // allocate a ByteBuffer and associate it with an RsslBuffer
                ByteBuffer bb = new ByteBuffer(1024);
                Buffer buffer = new Buffer();
                buffer.Data(bb);

                encIter.Clear();
                encIter.SetBufferAndRWFVersion(buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
                CodecReturnCode ret = rdmServiceGroupState.Encode(encIter);
                Assert.Equal(CodecReturnCode.SUCCESS, ret);
                dIter.Clear();
                bb.Flip();
                buffer.Data(bb);
                dIter.SetBufferAndRWFVersion(buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
                ret = rdmServiceGroupStateDec.Decode(dIter);

                Assert.Equal(CodecReturnCode.SUCCESS, ret);
                Assert.Equal(rdmServiceGroupState.Flags, rdmServiceGroupStateDec.Flags);
                Assert.Equal(rdmServiceGroupState.Group.ToString(), rdmServiceGroupStateDec.Group.ToString());

                if (rdmServiceGroupStateDec.HasMergedToGroup)
                {
                    Assert.Equal(rdmServiceGroupState.MergedToGroup.ToString(), rdmServiceGroupStateDec.MergedToGroup.ToString());
                }                   

                if (rdmServiceGroupStateDec.HasStatus)
                {
                    State decState = rdmServiceGroupStateDec.Status;
                    Assert.NotNull(decState);
                    Assert.Equal(state.Code(), decState.Code());
                    Assert.Equal(state.DataState(), decState.DataState());
                    Assert.Equal(state.StreamState(), decState.StreamState());
                    Assert.Equal(state.Text().ToString(), decState.Text().ToString());
                }
            }
            Console.WriteLine("Done.");
        }

        [Fact]
        public void ServiceDataCopyTests()
        {
            ServiceDataFlags[] flagsBase =
            {
                ServiceDataFlags.HAS_DATA
            };

            ServiceDataFlags[] flagsList = TypedMessageUtil.CreateFlagCombinations(flagsBase, false);

            ServiceData rdmServiceData = new ServiceData();
            ServiceData rdmServiceData2 = new ServiceData();

            FilterEntryActions action = FilterEntryActions.SET;
            Console.WriteLine("ServiceData copy tests...");

            for (int i = 0; i < flagsList.Length; i++)
            {
                rdmServiceData.Clear();
                rdmServiceData2.Clear();

                rdmServiceData.Flags = flagsList[i];
                rdmServiceData.Action = action;
                if (rdmServiceData.HasData)
                {
                    rdmServiceData.Data.Data("data");
                    rdmServiceData.DataType = DataTypes.ASCII_STRING;
                }

                CodecReturnCode ret = rdmServiceData.Copy(rdmServiceData2);

                Assert.Equal(CodecReturnCode.SUCCESS, ret);
                Assert.Equal(rdmServiceData.Flags, rdmServiceData2.Flags);
                Assert.Equal(rdmServiceData.DataType, rdmServiceData2.DataType);
                if (rdmServiceData2.HasData)
                {
                    Assert.Equal(rdmServiceData.Data.ToString(), rdmServiceData2.Data.ToString());
                }
            }
            Console.WriteLine("Done.");
        }

        [Fact]
        public void ServiceStateTests()
        {
            ServiceStateFlags[] flagsBase =
            {
                ServiceStateFlags.HAS_ACCEPTING_REQS,
                ServiceStateFlags.HAS_STATUS
            };

            ServiceStateFlags[] flagsList = TypedMessageUtil.CreateFlagCombinations(flagsBase, false);

            ServiceState rdmServiceState = new ServiceState();
            ServiceState rdmServiceStateDec = new ServiceState();

            FilterEntryActions action = FilterEntryActions.SET;
            long acceptingRequests = 1;
            long serviceState = 1;
            State state = new State();
            state.Text().Data("state");
            state.Code(StateCodes.FAILOVER_COMPLETED);
            state.DataState(DataStates.SUSPECT);
            state.StreamState(StreamStates.OPEN);

            Console.WriteLine("ServiceState tests...");

            for (int i = 0; i < flagsList.Length; i++)
            {
                rdmServiceState.Clear();
                rdmServiceStateDec.Clear();

                rdmServiceState.Flags = flagsList[i];
                rdmServiceState.Action = action;
                rdmServiceState.ServiceStateVal = serviceState;

                if (rdmServiceState.HasAcceptingRequests)
                {
                    rdmServiceState.AcceptingRequests = acceptingRequests;
                    rdmServiceState.HasAcceptingRequests = true;
                }

                if (rdmServiceState.HasStatus)
                {
                    rdmServiceState.Status.Text().Data("state");
                    rdmServiceState.Status.Code(StateCodes.FAILOVER_COMPLETED);
                    rdmServiceState.Status.DataState(DataStates.SUSPECT);
                    rdmServiceState.Status.StreamState(StreamStates.OPEN);
                    rdmServiceState.HasStatus = true;
                }

                // allocate a ByteBuffer and associate it with an RsslBuffer
                ByteBuffer bb = new ByteBuffer(1024);
                Buffer buffer = new Buffer();
                buffer.Data(bb);

                encIter.Clear();
                encIter.SetBufferAndRWFVersion(buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
                CodecReturnCode ret = rdmServiceState.Encode(encIter);
                Assert.Equal(CodecReturnCode.SUCCESS, ret);
                dIter.Clear();
                bb.Flip();
                buffer.Data(bb);
                dIter.SetBufferAndRWFVersion(buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
                ret = rdmServiceStateDec.Decode(dIter);

                Assert.Equal(CodecReturnCode.SUCCESS, ret);
                VerifyServiceState(rdmServiceState, rdmServiceStateDec);
            }
            Console.WriteLine("Done.");
        }

        [Fact]
        public void ServiceGroupStateCopyTests()
        {
            ServiceGroupFlags[] flagsBase =
            {
                ServiceGroupFlags.HAS_MERGED_TO_GROUP,
                ServiceGroupFlags.HAS_STATUS
            };

            ServiceGroupFlags[] flagsList = TypedMessageUtil.CreateFlagCombinations(flagsBase, false);

            ServiceGroup rdmServiceGroupState = new ServiceGroup();
            ServiceGroup rdmServiceGroupState2 = new ServiceGroup();

            FilterEntryActions action = FilterEntryActions.SET;
            State state = new State();
            state.Text().Data("state");
            state.Code(StateCodes.FAILOVER_COMPLETED);
            state.DataState(DataStates.SUSPECT);
            state.StreamState(StreamStates.OPEN);

            Console.WriteLine("ServiceGroup copy tests...");

            for (int i = 0; i < flagsList.Length; i++)
            {
                rdmServiceGroupState.Clear();
                rdmServiceGroupState2.Clear();

                rdmServiceGroupState.Flags = flagsList[i];
                rdmServiceGroupState.Action = action;
                rdmServiceGroupState.Group.Data("group");
                if (rdmServiceGroupState.HasMergedToGroup)
                {
                    rdmServiceGroupState.MergedToGroup.Data("mergedToGroup");
                }

                if (rdmServiceGroupState.HasStatus)
                {
                    rdmServiceGroupState.Status.Text().Data("state");
                    rdmServiceGroupState.Status.Code(StateCodes.FAILOVER_COMPLETED);
                    rdmServiceGroupState.Status.DataState(DataStates.SUSPECT);
                    rdmServiceGroupState.Status.StreamState(StreamStates.OPEN);
                }

                CodecReturnCode ret = rdmServiceGroupState.Copy(rdmServiceGroupState2);

                Assert.Equal(CodecReturnCode.SUCCESS, ret);
                Assert.Equal(rdmServiceGroupState.Flags, rdmServiceGroupState2.Flags);
                if (rdmServiceGroupState2.HasMergedToGroup)
                {
                    Assert.Equal(rdmServiceGroupState.MergedToGroup.ToString(), rdmServiceGroupState2.MergedToGroup.ToString());
                }
                Assert.Equal(rdmServiceGroupState.Group.ToString(), rdmServiceGroupState2.Group.ToString());

                if (rdmServiceGroupState2.HasStatus)
                {
                    State decState = rdmServiceGroupState2.Status;
                    Assert.NotNull(decState);
                    Assert.Equal(state.Code(), decState.Code());
                    Assert.Equal(state.DataState(), decState.DataState());
                    Assert.Equal(state.StreamState(), decState.StreamState());
                    Assert.Equal(state.Text().ToString(), decState.Text().ToString());
                }

            }
            Console.WriteLine("Done.");
        }

        [Fact]
        public void ServiceLinkTests()
        {
            ServiceLinkFlags[] flagsBase =
            {
                ServiceLinkFlags.HAS_CODE,
                ServiceLinkFlags.HAS_TEXT,
                ServiceLinkFlags.HAS_TYPE
            };

            ServiceLinkFlags[] flagsList = TypedMessageUtil.CreateFlagCombinations(flagsBase, false);

            ServiceLink rdmServiceLink = new ServiceLink();
            ServiceLink rdmServiceLinkDec = new ServiceLink();

            long linkCode = 1;
            long linkState = 1;
            long type = 1;
            MapEntryActions[] actions = { MapEntryActions.ADD, MapEntryActions.DELETE };
            Console.WriteLine("ServiceLink tests...");

            for (int i = 0; i < flagsList.Length; i++)
            {
                foreach (MapEntryActions action in actions)
                {
                    rdmServiceLink.Clear();
                    rdmServiceLinkDec.Clear();
                    rdmServiceLink.Action = action;
                    rdmServiceLink.Flags = flagsList[i];

                    rdmServiceLink.LinkState = linkState;

                    if (rdmServiceLink.HasCode)
                    {
                        rdmServiceLink.LinkCode = linkCode;
                    }

                    if (rdmServiceLink.HasText)
                    {
                        rdmServiceLink.Text.Data("text");
                    }

                    if (rdmServiceLink.HasType)
                    {
                        rdmServiceLink.Type = type;
                    }
                    
                    ByteBuffer bb = new ByteBuffer(1024);
                    Buffer buffer = new Buffer();
                    buffer.Data(bb);

                    encIter.Clear();
                    encIter.SetBufferAndRWFVersion(buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
                    CodecReturnCode ret = rdmServiceLink.Encode(encIter);
                    Assert.Equal(CodecReturnCode.SUCCESS, ret);
                    dIter.Clear();
                    bb.Flip();
                    buffer.Data(bb);
                    dIter.SetBufferAndRWFVersion(buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
                    ret = rdmServiceLinkDec.Decode(dIter);

                    Assert.Equal(CodecReturnCode.SUCCESS, ret);
                    Assert.Equal(rdmServiceLink.Flags, rdmServiceLinkDec.Flags);
                    Assert.Equal(rdmServiceLink.LinkState, rdmServiceLinkDec.LinkState);
                    if (rdmServiceLinkDec.HasCode)
                        Assert.Equal(rdmServiceLink.LinkCode, rdmServiceLinkDec.LinkCode);
                    if (rdmServiceLink.HasType)
                        Assert.Equal(rdmServiceLink.Type, rdmServiceLinkDec.Type);
                    if (rdmServiceLink.HasText)
                        Assert.Equal(rdmServiceLink.Text.ToString(), rdmServiceLinkDec.Text.ToString());
                }
            }
            Console.WriteLine("Done.");
        }

        [Fact]
        public void ServiceInfoCopyTests()
        {
            ServiceInfoFlags[] flagsBase =
            {
                ServiceInfoFlags.HAS_ACCEPTING_CONS_STATUS,
                ServiceInfoFlags.HAS_DICTS_PROVIDED,
                ServiceInfoFlags.HAS_DICTS_USED,
                ServiceInfoFlags.HAS_IS_SOURCE,
                ServiceInfoFlags.HAS_ITEM_LIST,
                ServiceInfoFlags.HAS_QOS,
                ServiceInfoFlags.HAS_SUPPORT_OOB_SNAPSHOTS,
                ServiceInfoFlags.HAS_SUPPORT_QOS_RANGE,
                ServiceInfoFlags.HAS_VENDOR
            };
            Qos qos = new Qos();
            qos.IsDynamic = true;
            qos.Rate(QosRates.JIT_CONFLATED);
            qos.Timeliness(QosTimeliness.DELAYED);
            qos.TimeInfo(1);
            ServiceInfoFlags[] flagsList = TypedMessageUtil.CreateFlagCombinations(flagsBase, false);

            ServiceInfo rdmServiceInfo = new ServiceInfo();
            ServiceInfo rdmServiceInfoDec = new ServiceInfo();

            FilterEntryActions[] actions = { FilterEntryActions.UPDATE, FilterEntryActions.CLEAR };
            Console.WriteLine("ServiceLink tests...");

            for (int i = 0; i < flagsList.Length; i++)
            {
                foreach (FilterEntryActions action in actions)
                {
                    rdmServiceInfo.Clear();
                    rdmServiceInfoDec.Clear();
                    rdmServiceInfo.Action = action;
                    rdmServiceInfo.Flags = flagsList[i];

                    if (rdmServiceInfo.HasAcceptingConsStatus)
                    {
                        rdmServiceInfo.AcceptConsumerStatus = 1;

                    }

                    if (rdmServiceInfo.HasDictionariesProvided)
                    {
                        rdmServiceInfo.DictionariesProvidedList.Add("dictprov1");
                    }

                    if (rdmServiceInfo.HasDictionariesUsed)
                    {
                        rdmServiceInfo.DictionariesUsedList.Add("dictused1");
                    }

                    if (rdmServiceInfo.HasIsSource)
                    {
                        rdmServiceInfo.IsSource = 1;
                    }

                    if (rdmServiceInfo.HasItemList)
                    {
                        rdmServiceInfo.ItemList.Data("itemList");
                    }

                    if (rdmServiceInfo.HasQos)
                    {
                        rdmServiceInfo.QosList.Add(qos);
                    }

                    if (rdmServiceInfo.HasSupportOOBSnapshots)
                    {
                        rdmServiceInfo.SupportsOOBSnapshots = 1;
                    }
                    if (rdmServiceInfo.HasSupportQosRange)
                    {
                        rdmServiceInfo.SupportsQosRange = 1;
                    }
                    if (rdmServiceInfo.HasVendor)
                    {
                        rdmServiceInfo.Vendor.Data("vendor");
                    }

                    rdmServiceInfo.CapabilitiesList.Add((long)1);
                    rdmServiceInfo.ServiceName.Data("servicename");

                    CodecReturnCode ret = rdmServiceInfo.Copy(rdmServiceInfoDec);
                    Assert.Equal(CodecReturnCode.SUCCESS, ret);
                    Assert.Equal(rdmServiceInfo.Flags, rdmServiceInfoDec.Flags);

                    if (rdmServiceInfo.HasAcceptingConsStatus)
                    {
                        Assert.Equal(rdmServiceInfo.AcceptConsumerStatus, rdmServiceInfoDec.AcceptConsumerStatus);
                    }

                    if (rdmServiceInfo.HasDictionariesProvided)
                    {
                        Assert.Equal(rdmServiceInfo.DictionariesProvidedList.Count, rdmServiceInfoDec.DictionariesProvidedList.Count);
                        Assert.Equal(rdmServiceInfo.DictionariesProvidedList[0], rdmServiceInfoDec.DictionariesProvidedList[0]);
                    }

                    if (rdmServiceInfo.HasDictionariesUsed)
                    {
                        Assert.Equal(rdmServiceInfo.DictionariesUsedList.Count, rdmServiceInfoDec.DictionariesUsedList.Count);
                        Assert.Equal(rdmServiceInfo.DictionariesUsedList[0], rdmServiceInfoDec.DictionariesUsedList[0]);
                    }

                    if (rdmServiceInfo.HasIsSource)
                    {
                        Assert.Equal(rdmServiceInfo.IsSource, rdmServiceInfoDec.IsSource);
                    }
                    if (rdmServiceInfo.HasItemList)
                    {
                        Assert.Equal(rdmServiceInfo.ItemList.ToString(), rdmServiceInfoDec.ItemList.ToString());
                    }

                    if (rdmServiceInfo.HasQos)
                    {
                        Assert.Equal(rdmServiceInfo.QosList.Count, rdmServiceInfoDec.QosList.Count);
                        int j = 0;
                        foreach (Qos qos1 in rdmServiceInfo.QosList)
                        {
                            Qos qos2 = rdmServiceInfoDec.QosList[j];
                            Assert.Equal(qos1.IsDynamic, qos2.IsDynamic);
                            Assert.Equal(qos1.TimeInfo(), qos2.TimeInfo());
                            Assert.Equal(qos1.Timeliness(), qos2.Timeliness());
                            Assert.Equal(qos1.Rate(), qos2.Rate());
                            Assert.Equal(qos1.RateInfo(), qos2.RateInfo());
                            j++;
                        }

                    }

                    if (rdmServiceInfo.HasSupportOOBSnapshots)
                    {
                        Assert.Equal(rdmServiceInfo.SupportsOOBSnapshots, rdmServiceInfoDec.SupportsOOBSnapshots);
                    }
                    if (rdmServiceInfo.HasSupportQosRange)
                    {
                        Assert.Equal(rdmServiceInfo.SupportsQosRange, rdmServiceInfoDec.SupportsQosRange);
                    }

                    if (rdmServiceInfo.HasVendor)
                    {
                        Assert.Equal(rdmServiceInfo.Vendor.ToString(), rdmServiceInfoDec.Vendor.ToString());
                    }
                    Assert.Equal(rdmServiceInfo.CapabilitiesList.Count, rdmServiceInfoDec.CapabilitiesList.Count);
                    Assert.Equal(rdmServiceInfo.CapabilitiesList[0], rdmServiceInfoDec.CapabilitiesList[0]);
                    Assert.Equal(rdmServiceInfo.ServiceName.ToString(), rdmServiceInfoDec.ServiceName.ToString());
                }
            }
            Console.WriteLine("Done.");

        }

        [Fact]
        public void ServiceInfoTests()
        {
            ServiceInfoFlags[] flagsBase =
            {
                ServiceInfoFlags.HAS_ACCEPTING_CONS_STATUS,
                ServiceInfoFlags.HAS_DICTS_PROVIDED,
                ServiceInfoFlags.HAS_DICTS_USED,
                ServiceInfoFlags.HAS_IS_SOURCE,
                ServiceInfoFlags.HAS_ITEM_LIST,
                ServiceInfoFlags.HAS_QOS,
                ServiceInfoFlags.HAS_SUPPORT_OOB_SNAPSHOTS,
                ServiceInfoFlags.HAS_SUPPORT_QOS_RANGE,
                ServiceInfoFlags.HAS_VENDOR
            };
            Qos qos = new Qos();
            qos.IsDynamic = true;
            qos.Rate(QosRates.JIT_CONFLATED);
            qos.Timeliness(QosTimeliness.DELAYED);
            qos.TimeInfo(1);
            ServiceInfoFlags[] flagsList = TypedMessageUtil.CreateFlagCombinations(flagsBase, false);

            ServiceInfo rdmServiceInfo = new ServiceInfo();
            ServiceInfo rdmServiceInfoDec = new ServiceInfo();

            FilterEntryActions[] actions = { FilterEntryActions.SET, FilterEntryActions.CLEAR };
            Console.WriteLine("ServiceInfo tests...");

            for (int i = 0; i < flagsList.Length; i++)
            {
                foreach (FilterEntryActions action in actions)
                {
                    rdmServiceInfoDec.Clear();

                    rdmServiceInfo.Clear();
                    rdmServiceInfo.Action = action;
                    rdmServiceInfo.Flags = flagsList[i];

                    if (rdmServiceInfo.HasAcceptingConsStatus)
                    {
                        rdmServiceInfo.AcceptConsumerStatus = 1;
                    }

                    if (rdmServiceInfo.HasDictionariesProvided)
                    {
                        rdmServiceInfo.DictionariesProvidedList.Add("dictprov1");
                    }

                    if (rdmServiceInfo.HasDictionariesUsed)
                    {
                        rdmServiceInfo.DictionariesUsedList.Add("dictused1");
                    }

                    if (rdmServiceInfo.HasIsSource)
                    {
                        rdmServiceInfo.IsSource = 1;
                    }

                    if (rdmServiceInfo.HasItemList)
                    {
                        rdmServiceInfo.ItemList.Data("itemList");
                    }

                    if (rdmServiceInfo.HasQos)
                    {
                        rdmServiceInfo.QosList.Add(qos);
                    }

                    if (rdmServiceInfo.HasSupportOOBSnapshots)
                    {
                        rdmServiceInfo.SupportsOOBSnapshots = 1;
                    }
                    if (rdmServiceInfo.HasSupportQosRange)
                    {
                        rdmServiceInfo.SupportsQosRange = 1;
                    }
                    if (rdmServiceInfo.HasVendor)
                    {
                        rdmServiceInfo.Vendor.Data("vendor");
                    }

                    rdmServiceInfo.CapabilitiesList.Add(1);
                    rdmServiceInfo.ServiceName.Data("servicename");

                    // allocate a ByteBuffer and associate it with an RsslBuffer
                    ByteBuffer bb = new ByteBuffer(1024);
                    Buffer buffer = new Buffer();
                    buffer.Data(bb);

                    encIter.Clear();
                    encIter.SetBufferAndRWFVersion(buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
                    CodecReturnCode ret = rdmServiceInfo.Encode(encIter);
                    Assert.Equal(CodecReturnCode.SUCCESS, ret);
                    dIter.Clear();
                    bb.Flip();
                    buffer.Data(bb);
                    dIter.SetBufferAndRWFVersion(buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
                    ret = rdmServiceInfoDec.Decode(dIter);

                    Assert.Equal(CodecReturnCode.SUCCESS, ret);
                    if (rdmServiceInfo.Action != FilterEntryActions.CLEAR)
                        VerifyServiceInfo(rdmServiceInfo, rdmServiceInfoDec);
                }
            }
            Console.WriteLine("Done.");
        }

        [Fact]
        public void ServiceLinkCopyTests()
        {
            ServiceLinkFlags[] flagsBase =
            {
                ServiceLinkFlags.HAS_CODE,
                ServiceLinkFlags.HAS_TEXT,
                ServiceLinkFlags.HAS_TYPE
            };

            ServiceLinkFlags[] flagsList = TypedMessageUtil.CreateFlagCombinations(flagsBase, false);

            ServiceLink rdmServiceLink = new ServiceLink();
            ServiceLink rdmServiceLink2 = new ServiceLink();

            long linkCode = 1;
            long linkState = 1;
            long type = 1;

            Console.WriteLine("ServiceLink copy tests...");

            for (int i = 0; i < flagsList.Length; i++)
            {
                rdmServiceLink.Clear();
                rdmServiceLink2.Clear();

                rdmServiceLink.Flags = flagsList[i];

                rdmServiceLink.LinkState = linkState;

                if (rdmServiceLink.HasCode)
                {
                    rdmServiceLink.LinkCode = linkCode;
                }

                if (rdmServiceLink.HasText)
                {
                    rdmServiceLink.Text.Data("text");
                }

                if (rdmServiceLink.HasType)
                {
                    rdmServiceLink.Type = type;
                }

                CodecReturnCode ret = rdmServiceLink.Copy(rdmServiceLink2);

                Assert.Equal(CodecReturnCode.SUCCESS, ret);
                Assert.Equal(rdmServiceLink.Flags, rdmServiceLink2.Flags);
                Assert.Equal(rdmServiceLink.LinkState, rdmServiceLink2.LinkState);
                if (rdmServiceLink2.HasCode)
                    Assert.Equal(rdmServiceLink.LinkCode, rdmServiceLink2.LinkCode);
                if (rdmServiceLink.HasType)
                    Assert.Equal(rdmServiceLink.Type, rdmServiceLink2.Type);
                if (rdmServiceLink.HasText)
                    Assert.Equal(rdmServiceLink.Text.ToString(), rdmServiceLink2.Text.ToString());
            }
            Console.WriteLine("Done.");
        }

        [Fact]
        public void ServiceLinkListCopyTests()
        {
            ServiceLinkFlags[] flagsBase =
            {
                ServiceLinkFlags.HAS_CODE,
                ServiceLinkFlags.HAS_TEXT,
                ServiceLinkFlags.HAS_TYPE
            };

            ServiceLinkFlags[] flagsList = TypedMessageUtil.CreateFlagCombinations(flagsBase, false);

            ServiceLink rdmServiceLink = new ServiceLink();
            ServiceLink rdmServiceLink2 = new ServiceLink();
            ServiceLinkInfo linkList1 = new ServiceLinkInfo();
            ServiceLinkInfo linkList2 = new ServiceLinkInfo();
            long linkCode = 1;
            long linkState = 1;
            long type = 1;

            Console.WriteLine("ServiceLink copy tests...");

            for (int i = 0; i < flagsList.Length; i++)
            {
                rdmServiceLink.Clear();
                rdmServiceLink2.Clear();
                linkList1.Clear();
                linkList2.Clear();
                rdmServiceLink.Flags = flagsList[i];

                rdmServiceLink.LinkState = linkState;

                if (rdmServiceLink.HasCode)
                {
                    rdmServiceLink.LinkCode = linkCode;
                }

                if (rdmServiceLink.HasText)
                {
                    rdmServiceLink.Text.Data("text");
                }

                if (rdmServiceLink.HasType)
                {
                    rdmServiceLink.Type = type;
                }
                linkList1.Action = FilterEntryActions.SET;
                linkList1.LinkList.Add(rdmServiceLink);

                CodecReturnCode ret = linkList1.Copy(linkList2);

                Assert.Equal(CodecReturnCode.SUCCESS, ret);
                Assert.Equal(linkList1.Action, linkList2.Action);
                Assert.Equal(linkList1.LinkList.Count, linkList2.LinkList.Count);
                rdmServiceLink2 = linkList2.LinkList[0];
                Assert.Equal(rdmServiceLink.Flags, rdmServiceLink2.Flags);
                Assert.Equal(rdmServiceLink.LinkState, rdmServiceLink2.LinkState);
                if (rdmServiceLink2.HasCode)
                    Assert.Equal(rdmServiceLink.LinkCode, rdmServiceLink2.LinkCode);
                if (rdmServiceLink.HasType)
                    Assert.Equal(rdmServiceLink.Type, rdmServiceLink2.Type);
                if (rdmServiceLink.HasText)
                    Assert.Equal(rdmServiceLink.Text.ToString(), rdmServiceLink2.Text.ToString());
            }
            Console.WriteLine("Done.");
        }

        [Fact]
        public void ServiceLinkListTests()
        {
            ServiceLinkFlags[] flagsBase =
            {
                ServiceLinkFlags.HAS_CODE,
                ServiceLinkFlags.HAS_TEXT,
                ServiceLinkFlags.HAS_TYPE
            };

            ServiceLinkFlags[] flagsList = TypedMessageUtil.CreateFlagCombinations(flagsBase, false);

            ServiceLink rdmServiceLink = new ServiceLink();
            ServiceLinkInfo linkList = new ServiceLinkInfo();
            ServiceLinkInfo linkListDec = new ServiceLinkInfo();

            long linkCode = 1;
            long linkState = 1;
            long type = 1;
            MapEntryActions[] actions = { MapEntryActions.ADD, MapEntryActions.DELETE };
            Console.WriteLine("ServiceLink tests...");
            for (int i = 0; i < flagsList.Length; i++)
            {
                foreach (MapEntryActions action in actions)
                {
                    rdmServiceLink.Clear();
                    rdmServiceLink.Action = action;
                    linkList.Clear();
                    linkListDec.Clear();
                    rdmServiceLink.Flags = flagsList[i];
                    rdmServiceLink.Name.Data("name");
                    rdmServiceLink.LinkState = linkState;

                    if (rdmServiceLink.HasCode)
                    {
                        rdmServiceLink.LinkCode = linkCode;
                    }

                    if (rdmServiceLink.HasText)
                    {
                        rdmServiceLink.Text.Data("text");
                    }

                    if (rdmServiceLink.HasType)
                    {
                        rdmServiceLink.Type = type;
                    }
                    linkList.Action = FilterEntryActions.SET;
                    linkList.LinkList.Add(rdmServiceLink);
                    
                    ByteBuffer bb = new ByteBuffer(1024);
                    Buffer buffer = new Buffer();
                    buffer.Data(bb);

                    encIter.Clear();
                    encIter.SetBufferAndRWFVersion(buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

                    CodecReturnCode ret = linkList.Encode(encIter);
                    Assert.Equal(CodecReturnCode.SUCCESS, ret);
                    dIter.Clear();
                    bb.Flip();
                    buffer.Data(bb);
                    dIter.SetBufferAndRWFVersion(buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
                    ret = linkListDec.Decode(dIter);

                    Assert.Equal(CodecReturnCode.SUCCESS, ret);
                    if (action == MapEntryActions.DELETE)
                    {
                        Assert.Empty(linkListDec.LinkList);
                    }
                    else
                    {
                        VerifyServiceLinkList(linkList, linkListDec);
                    }
                }
            }
            Console.WriteLine("Done.");
        }

        [Fact]
        public void ServiceLoadTests()
        {
            ServiceLoadFlags[] flagsBase =
            {
                ServiceLoadFlags.HAS_LOAD_FACTOR,
                ServiceLoadFlags.HAS_OPEN_LIMIT,
                ServiceLoadFlags.HAS_OPEN_WINDOW
            };

            ServiceLoadFlags[] flagsList = TypedMessageUtil.CreateFlagCombinations(flagsBase, false);

            ServiceLoad rdmServiceLoad = new ServiceLoad();
            ServiceLoad rdmServiceLoadDec = new ServiceLoad();

            long loadFactor = 1;
            long openLimit = 1;
            long openWindow = 1;

            Console.WriteLine("ServiceLoad tests...");

            for (int i = 0; i < flagsList.Length; i++)
            {
                rdmServiceLoad.Clear();
                rdmServiceLoadDec.Clear();

                rdmServiceLoad.Flags = flagsList[i];

                if (rdmServiceLoad.HasOpenLimit)
                {
                    rdmServiceLoad.OpenLimit = openLimit;
                    rdmServiceLoad.HasOpenLimit = true;
                }

                if (rdmServiceLoad.HasOpenWindow)
                {
                    rdmServiceLoad.OpenWindow = openWindow;
                    rdmServiceLoad.HasOpenWindow = true;
                }

                if (rdmServiceLoad.HasLoadFactor)
                {
                    rdmServiceLoad.LoadFactor = loadFactor;
                    rdmServiceLoad.HasLoadFactor = true;
                }

                // allocate a ByteBuffer and associate it with an RsslBuffer
                ByteBuffer bb = new ByteBuffer(1024);
                Buffer buffer = new Buffer();
                buffer.Data(bb);

                encIter.Clear();
                encIter.SetBufferAndRWFVersion(buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
                CodecReturnCode ret = rdmServiceLoad.Encode(encIter);
                Assert.Equal(CodecReturnCode.SUCCESS, ret);
                dIter.Clear();
                bb.Flip();
                buffer.Data(bb);
                dIter.SetBufferAndRWFVersion(buffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
                ret = rdmServiceLoadDec.Decode(dIter);

                Assert.Equal(CodecReturnCode.SUCCESS, ret);
                Assert.Equal(rdmServiceLoad.Flags, rdmServiceLoadDec.Flags);

                VerifyServiceLoad(rdmServiceLoad, rdmServiceLoadDec);
            }
            Console.WriteLine("Done.");
        }

        [Fact]
        public void ServiceLoadCopyTests()
        {
            ServiceLoadFlags[] flagsBase =
            {
                ServiceLoadFlags.HAS_LOAD_FACTOR,
                ServiceLoadFlags.HAS_OPEN_LIMIT,
                ServiceLoadFlags.HAS_OPEN_WINDOW
            };

            ServiceLoadFlags[] flagsList = TypedMessageUtil.CreateFlagCombinations(flagsBase, false);

            ServiceLoad rdmServiceLoad = new ServiceLoad();
            ServiceLoad rdmServiceLoad2 = new ServiceLoad();

            long loadFactor = 1;
            long openLimit = 1;
            long openWindow = 1;

            Console.WriteLine("ServiceLoad copy tests...");

            for (int i = 0; i < flagsList.Length; i++)
            {
                rdmServiceLoad.Clear();
                rdmServiceLoad2.Clear();

                rdmServiceLoad.Flags = flagsList[i];

                if (rdmServiceLoad.HasOpenLimit)
                {
                    rdmServiceLoad.OpenLimit = openLimit;
                    rdmServiceLoad.HasOpenLimit = true;
                }

                if (rdmServiceLoad.HasOpenWindow)
                {
                    rdmServiceLoad.OpenWindow = openWindow;
                    rdmServiceLoad.HasOpenWindow = true;
                }

                if (rdmServiceLoad.HasLoadFactor)
                {
                    rdmServiceLoad.LoadFactor = loadFactor;
                    rdmServiceLoad.HasLoadFactor = true;
                }

                CodecReturnCode ret = rdmServiceLoad.Copy(rdmServiceLoad2);

                Assert.Equal(CodecReturnCode.SUCCESS, ret);
                Assert.Equal(rdmServiceLoad.Flags, rdmServiceLoad2.Flags);

                VerifyServiceLoad(rdmServiceLoad, rdmServiceLoad2);
            }
            Console.WriteLine("Done.");
        }

        [Fact]
        public void ConsumerStatusServiceCopyTests()
        {
            ConsumerStatusService consumerStatusService1 = new ConsumerStatusService();
            ConsumerStatusService consumerStatusService2 = new ConsumerStatusService();
            int serviceId = 1;
            long sourceMirroringMode = 1;
            MapEntryActions action = MapEntryActions.UPDATE;
            consumerStatusService1.ServiceId = serviceId;
            consumerStatusService1.SourceMirroringModeVal = sourceMirroringMode;
            consumerStatusService1.Action = action;

            Console.WriteLine("ConsumerStatusService copy tests...");
            CodecReturnCode ret = consumerStatusService1.Copy(consumerStatusService2);

            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            // Verify deep copy
            Assert.Equal(consumerStatusService1.Action, consumerStatusService2.Action);
            Assert.Equal(consumerStatusService1.ServiceId, consumerStatusService2.ServiceId);
            Assert.Equal(consumerStatusService1.SourceMirroringModeVal, consumerStatusService2.SourceMirroringModeVal);

            Console.WriteLine("Done.");
        }

        [Fact]
        public void ConsumerStatusServiceToStringTests()
        {
            ConsumerStatusService consumerStatusService1 = new ConsumerStatusService();
            
            consumerStatusService1.SourceMirroringModeVal = 1;
            consumerStatusService1.Action = MapEntryActions.UPDATE;
            consumerStatusService1.ServiceId = 1;

            Console.WriteLine("ConsumerStatusService ToString tests...");
            string consStatusServiceStr = consumerStatusService1.ToString();
            Assert.Contains("ConsumerStatusService: ", consStatusServiceStr);
            Assert.Contains("sourceMirroringMode: 1", consStatusServiceStr);

            Console.WriteLine("Done.");
        }

        [Fact]
        public void DirectoryCloseCopyTests()
        {
            DirectoryClose closeRDMMsg1 = new DirectoryClose();
            DirectoryClose closeRDMMsg2 = new DirectoryClose();
            int streamId = -5;
            closeRDMMsg1.StreamId = streamId;

            Console.WriteLine("LoginClose copy tests...");

            // deep copy
            CodecReturnCode ret = closeRDMMsg1.Copy(closeRDMMsg2);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            // Verify deep copy
            Assert.Equal(closeRDMMsg1.StreamId, closeRDMMsg2.StreamId);
            Console.WriteLine("Done.");
        }

        [Fact]
        public void DirectoryCloseToStringTests()
        {
            DirectoryClose closeRDMMsg1 = new DirectoryClose();
            int streamId = -5;
            closeRDMMsg1.StreamId = streamId;

            Console.WriteLine("DirectoryClose ToString tests...");

            string directoryCloseStr = closeRDMMsg1.ToString();
            Assert.Contains("streamId: -5", directoryCloseStr);
            Console.WriteLine("Done.");
        }

        [Fact]
        public void DirectoryCloseTests()
        {
            DirectoryClose encRDMMsg = new DirectoryClose();
            DirectoryClose decRDMMsg = new DirectoryClose();
          
            int streamId = -5;

            dIter.Clear();
            encIter.Clear();
            Buffer membuf = new Buffer();
            membuf.Data(new ByteBuffer(1024));

            Console.WriteLine("LoginClose tests...");
            encRDMMsg.Clear();

            encRDMMsg.StreamId = streamId;
            encIter.SetBufferAndRWFVersion(membuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

            CodecReturnCode ret = encRDMMsg.Encode(encIter);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            dIter.SetBufferAndRWFVersion(membuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
            ret = msg.Decode(dIter);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);
            ret = decRDMMsg.Decode(dIter, msg);

            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            Assert.Equal(streamId, decRDMMsg.StreamId);

            Console.WriteLine("Done.");
        }

        [Fact]
        public void DirectoryUpdateTests()
        {
            DirectoryUpdate encRDMMsg = new DirectoryUpdate();
            DirectoryUpdate decRDMMsg = new DirectoryUpdate();

            Console.WriteLine("Directory update tests...");

            DirectoryUpdateFlags[] flagsBase =
            {
                DirectoryUpdateFlags.HAS_FILTER,
                DirectoryUpdateFlags.HAS_SERVICE_ID,
                DirectoryUpdateFlags.HAS_SEQ_NUM
            };

            ServiceFlags serviceFlags = ServiceFlags.HAS_DATA |
                    ServiceFlags.HAS_INFO |
                    ServiceFlags.HAS_LINK |
                    ServiceFlags.HAS_LOAD |
                    ServiceFlags.HAS_STATE;
            // Parameter setup
            int streamId = -5;
            int serviceId = 1;
            int filter = 2;
            int seqNum = 1;
            Service rdmService = new Service();
            DirectoryUpdateFlags[] flagsList = TypedMessageUtil.CreateFlagCombinations(flagsBase, false);
            MapEntryActions[] actions = { MapEntryActions.ADD, MapEntryActions.UPDATE, MapEntryActions.DELETE };
            for (int i = 0; i < flagsList.Length; i++)
            {
                DirectoryUpdateFlags flags = flagsList[i];
                foreach (MapEntryActions serviceAction in actions)
                {
                    dIter.Clear();
                    encIter.Clear();
                    encRDMMsg.Clear();
                    decRDMMsg.Clear();
                    Buffer membuf = new Buffer();
                    membuf.Data(new ByteBuffer(1024));

                    encRDMMsg.Flags = flags;
                    encRDMMsg.StreamId = streamId;

                    if (encRDMMsg.HasServiceId)
                        encRDMMsg.ServiceId = serviceId;

                    if (encRDMMsg.HasFilter)
                        encRDMMsg.Filter = filter;

                    if (encRDMMsg.HasSequenceNumber)
                        encRDMMsg.SequenceNumber = seqNum;
                    // filter action clear is not tested here. see service tests for complete coverage
                    BuildRDMService(rdmService, serviceFlags, serviceAction, FilterEntryActions.SET);
                    encRDMMsg.ServiceList.Add(rdmService);
                    encIter.SetBufferAndRWFVersion(membuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
                    CodecReturnCode ret = encRDMMsg.Encode(encIter);
                    Assert.Equal(CodecReturnCode.SUCCESS, ret);

                    dIter.SetBufferAndRWFVersion(membuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
                    ret = msg.Decode(dIter);
                    Assert.Equal(CodecReturnCode.SUCCESS, ret);
                    ret = decRDMMsg.Decode(dIter, msg);
                    Assert.Equal(CodecReturnCode.SUCCESS, ret);
                    Assert.Equal(streamId, decRDMMsg.StreamId);
                    Assert.Equal(flags, decRDMMsg.Flags);

                    /* Check parameters */
                    if (encRDMMsg.HasServiceId)
                    {
                        Assert.Equal(serviceId, decRDMMsg.ServiceId);
                    }

                    if (encRDMMsg.HasFilter)
                    {
                        Assert.Equal(filter, decRDMMsg.Filter);
                    }

                    if (encRDMMsg.HasSequenceNumber)
                    {
                        Assert.Equal(seqNum, decRDMMsg.SequenceNumber);
                    }

                    if (encRDMMsg.ServiceList[0].Action != MapEntryActions.DELETE)
                    {
                        Assert.Equal(encRDMMsg.ServiceList.Count, decRDMMsg.ServiceList.Count);
                        VerifyRDMService(encRDMMsg.ServiceList[0], decRDMMsg.ServiceList[0]);
                    }
                }
            }
            Console.WriteLine("Done.");
        }

        [Fact]
        public void DirectoryRefreshCopyTests()
        {
            DirectoryRefresh rdmDirRefreshMsg1 = new DirectoryRefresh();
            DirectoryRefresh rdmDirRefreshMsg2 = new DirectoryRefresh();

            Console.WriteLine("Directory refresh copy tests...");

            DirectoryRefreshFlags[] flagsBase =
            {
                DirectoryRefreshFlags.CLEAR_CACHE,
                DirectoryRefreshFlags.SOLICITED,
                DirectoryRefreshFlags.HAS_SERVICE_ID,
                DirectoryRefreshFlags.HAS_SEQ_NUM
            };

            ServiceFlags serviceFlags = ServiceFlags.HAS_DATA |
                    ServiceFlags.HAS_INFO |
                    ServiceFlags.HAS_LINK |
                    ServiceFlags.HAS_LOAD |
                    ServiceFlags.HAS_STATE;

            /* Parameter setup */
            int streamId = -5;
            int serviceId = 1;
            int filter = 2;
            int seqNum = 1;
            State state = new State();
            Buffer buffer = new Buffer();
            buffer.Data("state");
            state.Text(buffer);
            state.Code(StateCodes.FAILOVER_COMPLETED);
            state.DataState(DataStates.SUSPECT);
            state.StreamState(StreamStates.OPEN);

            Service rdmService = new Service();
            DirectoryRefreshFlags[] flagsList = TypedMessageUtil.CreateFlagCombinations<DirectoryRefreshFlags>(flagsBase, false);
            MapEntryActions[] serviceAddorDeleteActions = { MapEntryActions.ADD, MapEntryActions.DELETE };

            for (int i = 0; i < flagsList.Length; i++)
            {
                DirectoryRefreshFlags flags = flagsList[i];
                foreach (MapEntryActions serviceAddorDeleteAction in serviceAddorDeleteActions)
                {
                    rdmDirRefreshMsg1.Clear();
                    rdmDirRefreshMsg2.Clear();

                    rdmDirRefreshMsg1.Flags = flags;
                    rdmDirRefreshMsg1.StreamId = streamId;

                    if (rdmDirRefreshMsg1.HasServiceId)
                    {
                        rdmDirRefreshMsg1.ServiceId = serviceId;
                    }

                    rdmDirRefreshMsg1.Filter = filter;
                    rdmDirRefreshMsg1.State.Code(state.Code());
                    rdmDirRefreshMsg1.State.DataState(state.DataState());
                    rdmDirRefreshMsg1.State.Text().Data("state");
                    rdmDirRefreshMsg1.State.StreamState(state.StreamState());

                    if (rdmDirRefreshMsg1.HasSequenceNumber)
                        rdmDirRefreshMsg1.SequenceNumber = seqNum;
                    BuildRDMService(rdmService, serviceFlags, serviceAddorDeleteAction, FilterEntryActions.SET);
                    rdmDirRefreshMsg1.ServiceList.Add(rdmService);

                    CodecReturnCode ret = rdmDirRefreshMsg1.Copy(rdmDirRefreshMsg2);
                    Assert.Equal(CodecReturnCode.SUCCESS, ret);
                    Assert.Equal(streamId, rdmDirRefreshMsg2.StreamId);
                    Assert.Equal(flags, rdmDirRefreshMsg2.Flags);

                    /* Check parameters */
                    if (rdmDirRefreshMsg1.HasServiceId)
                    {
                        Assert.Equal(serviceId, rdmDirRefreshMsg2.ServiceId);
                    }

                    Assert.Equal(filter, rdmDirRefreshMsg2.Filter);

                    if (rdmDirRefreshMsg1.HasSequenceNumber)
                    {
                        Assert.Equal(seqNum, rdmDirRefreshMsg2.SequenceNumber);
                    }

                    if (rdmDirRefreshMsg1.Solicited)
                    {
                        Assert.Equal(rdmDirRefreshMsg1.Solicited, rdmDirRefreshMsg2.Solicited);
                    }
                    if (rdmDirRefreshMsg1.ClearCache)
                    {
                        Assert.Equal(rdmDirRefreshMsg1.ClearCache, rdmDirRefreshMsg2.ClearCache);
                    }

                    State state2 = rdmDirRefreshMsg2.State;
                    Assert.NotNull(state2);
                    Assert.Equal(state.Code(), state2.Code());
                    Assert.Equal(state.DataState(), state2.DataState());
                    Assert.Equal(state.StreamState(), state2.StreamState());
                    Assert.Equal(state.Text().ToString(), state2.Text().ToString());

                    if (rdmDirRefreshMsg1.ServiceList[0].Action != MapEntryActions.DELETE)
                    {
                        Assert.Equal(rdmDirRefreshMsg1.ServiceList.Count, rdmDirRefreshMsg2.ServiceList.Count);
                        VerifyRDMService(rdmDirRefreshMsg1.ServiceList[0], rdmDirRefreshMsg2.ServiceList[0]);
                    }
                }

            }
            Console.WriteLine("Done.");
        }

        [Fact]
        public void DirectoryUpdateCopyTests()
        {
            DirectoryUpdate rdmDirUpdateMsg1 = new DirectoryUpdate();
            DirectoryUpdate rdmDirUpdateMsg2 = new DirectoryUpdate();

            Console.WriteLine("Directory update copy tests...");

            DirectoryUpdateFlags[] flagsBase =
            {
                DirectoryUpdateFlags.HAS_FILTER,
                DirectoryUpdateFlags.HAS_SERVICE_ID,
                DirectoryUpdateFlags.HAS_SEQ_NUM
            };

            ServiceFlags serviceFlags = ServiceFlags.HAS_DATA |
                    ServiceFlags.HAS_INFO |
                    ServiceFlags.HAS_LINK |
                    ServiceFlags.HAS_LOAD |
                    ServiceFlags.HAS_STATE;

            /* Parameter setup */
            int streamId = -5;
            int serviceId = 1;
            int filter = 2;
            int seqNum = 1;
            Service rdmService = new Service();
            DirectoryUpdateFlags[] flagsList = TypedMessageUtil.CreateFlagCombinations(flagsBase, false);
            MapEntryActions[] serviceAddorDeleteActions = { MapEntryActions.ADD, MapEntryActions.DELETE };
            for (int i = 0; i < flagsList.Length; i++)
            {
                DirectoryUpdateFlags flags = flagsList[i];
                foreach (MapEntryActions serviceAddorDeleteAction in serviceAddorDeleteActions)
                {
                    rdmDirUpdateMsg1.Clear();
                    rdmDirUpdateMsg2.Clear();

                    rdmDirUpdateMsg1.Flags = flags;
                    rdmDirUpdateMsg1.StreamId = streamId;

                    if (rdmDirUpdateMsg1.HasServiceId)
                        rdmDirUpdateMsg1.ServiceId = serviceId;

                    if (rdmDirUpdateMsg1.HasFilter)
                        rdmDirUpdateMsg1.Filter = filter;

                    if (rdmDirUpdateMsg1.HasSequenceNumber)
                        rdmDirUpdateMsg1.SequenceNumber = seqNum;
                    BuildRDMService(rdmService, serviceFlags, serviceAddorDeleteAction, FilterEntryActions.SET);
                    rdmDirUpdateMsg1.ServiceList.Add(rdmService);

                    CodecReturnCode ret = rdmDirUpdateMsg1.Copy(rdmDirUpdateMsg2);
                    Assert.Equal(CodecReturnCode.SUCCESS, ret);
                    Assert.Equal(streamId, rdmDirUpdateMsg2.StreamId);
                    Assert.Equal(flags, rdmDirUpdateMsg2.Flags);

                    /* Check parameters */
                    if (rdmDirUpdateMsg1.HasServiceId)
                    {
                        Assert.Equal(serviceId, rdmDirUpdateMsg2.ServiceId);
                    }

                    if (rdmDirUpdateMsg1.HasFilter)
                    {
                        Assert.Equal(filter, rdmDirUpdateMsg2.Filter);
                    }

                    if (rdmDirUpdateMsg1.HasSequenceNumber)
                    {
                        Assert.Equal(seqNum, rdmDirUpdateMsg2.SequenceNumber);
                    }

                    if (rdmDirUpdateMsg1.ServiceList[0].Action != MapEntryActions.DELETE)
                    {
                        Assert.Equal(rdmDirUpdateMsg1.ServiceList.Count, rdmDirUpdateMsg2.ServiceList.Count);
                        VerifyRDMService(rdmDirUpdateMsg1.ServiceList[0], rdmDirUpdateMsg2.ServiceList[0]);
                    }
                }

            }
            Console.WriteLine("Done.");
        }

        [Fact]
        public void DirectoryRefreshTests()
        {
            DirectoryRefresh encRDMMsg = new DirectoryRefresh();
            DirectoryRefresh decRDMMsg = new DirectoryRefresh();

            Console.WriteLine("Directory refresh tests...");

            DirectoryRefreshFlags[] flagsBase =
            {
                DirectoryRefreshFlags.HAS_SERVICE_ID,
                DirectoryRefreshFlags.HAS_SEQ_NUM,
                DirectoryRefreshFlags.CLEAR_CACHE,
                DirectoryRefreshFlags.SOLICITED
            };

            ServiceFlags serviceFlags = ServiceFlags.HAS_DATA |
                    ServiceFlags.HAS_INFO |
                    ServiceFlags.HAS_LINK |
                    ServiceFlags.HAS_LOAD |
                    ServiceFlags.HAS_STATE;

            /* Parameter setup */
            int streamId = -5;
            int serviceId = 1;
            int filter = 2;
            int seqNum = 1;
            State state = new State();
            Buffer buffer = new Buffer();
            buffer.Data("state");
            state.Text(buffer);
            state.Code(StateCodes.FAILOVER_COMPLETED);
            state.DataState(DataStates.SUSPECT);
            state.StreamState(StreamStates.OPEN);

            Service rdmService = new Service();
            DirectoryRefreshFlags[] flagsList = TypedMessageUtil.CreateFlagCombinations(flagsBase, false);
            MapEntryActions[] actions = { MapEntryActions.ADD, MapEntryActions.UPDATE, MapEntryActions.DELETE };
            for (int i = 0; i < flagsList.Length; i++)
            {
                DirectoryRefreshFlags flags = flagsList[i];
                foreach (MapEntryActions serviceAction in actions)
                {
                    dIter.Clear();
                    encIter.Clear();
                    encRDMMsg.Clear();
                    decRDMMsg.Clear();
                    Buffer membuf = new Buffer();
                    membuf.Data(new ByteBuffer(1024));

                    encRDMMsg.Flags = flags;
                    encRDMMsg.StreamId = streamId;

                    if (encRDMMsg.HasServiceId)
                        encRDMMsg.ServiceId = serviceId;

                    encRDMMsg.State.Code(state.Code());
                    encRDMMsg.State.DataState(state.DataState());
                    encRDMMsg.State.Text().Data("state");
                    encRDMMsg.State.StreamState(state.StreamState());

                    encRDMMsg.Filter = filter;

                    if (encRDMMsg.HasSequenceNumber)
                        encRDMMsg.SequenceNumber = seqNum;
                    BuildRDMService(rdmService, serviceFlags, serviceAction, FilterEntryActions.SET);
                    encRDMMsg.ServiceList.Add(rdmService);
                    encIter.SetBufferAndRWFVersion(membuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
                    CodecReturnCode ret = encRDMMsg.Encode(encIter);
                    Assert.Equal(CodecReturnCode.SUCCESS, ret);

                    dIter.SetBufferAndRWFVersion(membuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
                    ret = msg.Decode(dIter);
                    Assert.Equal(CodecReturnCode.SUCCESS, ret);
                    ret = decRDMMsg.Decode(dIter, msg);
                    Assert.Equal(CodecReturnCode.SUCCESS, ret);
                    Assert.Equal(streamId, decRDMMsg.StreamId);
                    Assert.Equal(flags, decRDMMsg.Flags);

                    /* Check parameters */
                    Assert.Equal(filter, decRDMMsg.Filter);
                    if (encRDMMsg.HasServiceId)
                    {
                        Assert.Equal(serviceId, decRDMMsg.ServiceId);
                    }
                    if (encRDMMsg.Solicited)
                    {
                        Assert.Equal(encRDMMsg.Solicited, decRDMMsg.Solicited);
                    }
                    if (encRDMMsg.ClearCache)
                    {
                        Assert.Equal(encRDMMsg.ClearCache, decRDMMsg.ClearCache);
                    }
                    if (encRDMMsg.HasSequenceNumber)
                    {
                        Assert.Equal(seqNum, decRDMMsg.SequenceNumber);
                    }

                    if (encRDMMsg.HasSequenceNumber)
                    {
                        Assert.Equal(seqNum, decRDMMsg.SequenceNumber);
                    }

                    if (encRDMMsg.ServiceList[0].Action != MapEntryActions.DELETE)
                    {
                        Assert.Equal(encRDMMsg.ServiceList.Count, decRDMMsg.ServiceList.Count);
                        VerifyRDMService(encRDMMsg.ServiceList[0], decRDMMsg.ServiceList[0]);
                    }

                    State decState = decRDMMsg.State;
                    Assert.NotNull(decState);
                    Assert.Equal(state.Code(), decState.Code());
                    Assert.Equal(state.DataState(), decState.DataState());
                    Assert.Equal(state.StreamState(), decState.StreamState());
                    Assert.Equal(state.Text().ToString(), decState.Text().ToString());
                }
            }
            Console.WriteLine("Done.");
        }

        [Fact]
        public void DirectoryStatusTests()
        {
            DirectoryStatus encRDMMsg = new DirectoryStatus();
            DirectoryStatus decRDMMsg = new DirectoryStatus();

            Console.WriteLine("DirectoryStatus tests...");

            DirectoryStatusFlags[] flagsBase =
            {
                DirectoryStatusFlags.HAS_STATE,
                DirectoryStatusFlags.HAS_FILTER,
                DirectoryStatusFlags.HAS_SERVICE_ID,
                DirectoryStatusFlags.CLEAR_CACHE,
            };
            /* Parameter setup */
            int streamId = -5;
            int serviceId = 1;
            int filter = 2;

            State state = new State();
            Buffer buffer = new Buffer();
            buffer.Data("state");
            state.Text(buffer);
            state.Code(StateCodes.FAILOVER_COMPLETED);
            state.DataState(DataStates.SUSPECT);
            state.StreamState(StreamStates.OPEN);

            DirectoryStatusFlags[] flagsList = TypedMessageUtil.CreateFlagCombinations(flagsBase, false);
            for (int i = 0; i < flagsList.Length; i++)
            {
                dIter.Clear();
                encIter.Clear();
                encRDMMsg.Clear();

                Buffer membuf = new Buffer();
                membuf.Data(new ByteBuffer(1024));

                DirectoryStatusFlags flags = flagsList[i];
                encRDMMsg.Flags = flags;
                encRDMMsg.StreamId = streamId;

                if (encRDMMsg.HasState)
                {
                    encRDMMsg.State.Code(state.Code());
                    encRDMMsg.State.DataState(state.DataState());
                    encRDMMsg.State.Text().Data("state");
                    encRDMMsg.State.StreamState(state.StreamState());
                }

                if (encRDMMsg.HasServiceId)
                    encRDMMsg.ServiceId = serviceId;

                if (encRDMMsg.HasFilter)
                    encRDMMsg.Filter = filter;

                if (encRDMMsg.ClearCache)
                {
                    encRDMMsg.ClearCache = true;
                }

                encIter.SetBufferAndRWFVersion(membuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
                CodecReturnCode ret = encRDMMsg.Encode(encIter);
                Assert.Equal(CodecReturnCode.SUCCESS, ret);

                dIter.SetBufferAndRWFVersion(membuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
                ret = msg.Decode(dIter);
                Assert.Equal(CodecReturnCode.SUCCESS, ret);
                ret = decRDMMsg.Decode(dIter, msg);
                Assert.Equal(CodecReturnCode.SUCCESS, ret);
                Assert.Equal(streamId, decRDMMsg.StreamId);
                Assert.Equal(flags, decRDMMsg.Flags);

                /* Check parameters */
                if (encRDMMsg.HasServiceId)
                {
                    Assert.Equal(serviceId, decRDMMsg.ServiceId);
                }

                if (encRDMMsg.HasFilter)
                {
                    Assert.Equal(filter, decRDMMsg.Filter);
                }

                if (decRDMMsg.HasState)
                {
                    State decState = decRDMMsg.State;
                    Assert.NotNull(decState);
                    Assert.Equal(state.Code(), decState.Code());
                    Assert.Equal(state.DataState(), decState.DataState());
                    Assert.Equal(state.StreamState(), decState.StreamState());
                    Assert.Equal(state.Text().ToString(), decState.Text().ToString());
                }
            }
            Console.WriteLine("Done.");
        }

        [Fact]
        public void DirectoryStatusCopyTests()
        {
            DirectoryStatus statusRDMMsg1 = new DirectoryStatus();
            DirectoryStatus statusRDMMsg2 = new DirectoryStatus();

            Console.WriteLine("DirectoryStatus copy tests...");

            /* Parameter setup */
            int streamId = -5;
            int serviceId = 1;
            int filter = 1;
            State state = new State();
            Buffer buffer = new Buffer();
            buffer.Data("state");
            state.Text(buffer);
            state.Code(StateCodes.FAILOVER_COMPLETED);
            state.DataState(DataStates.SUSPECT);
            state.StreamState(StreamStates.OPEN);

            DirectoryStatusFlags flags = DirectoryStatusFlags.HAS_FILTER | DirectoryStatusFlags.HAS_SERVICE_ID | DirectoryStatusFlags.HAS_STATE | DirectoryStatusFlags.CLEAR_CACHE;
            statusRDMMsg1.Clear();
            statusRDMMsg1.Flags = flags;
            statusRDMMsg1.StreamId = streamId;
            if (statusRDMMsg1.HasState)
            {
                statusRDMMsg1.State.Code(state.Code());
                statusRDMMsg1.State.DataState(state.DataState());
                statusRDMMsg1.State.Text().Data("state");
                statusRDMMsg1.State.StreamState(state.StreamState());
            }

            if (statusRDMMsg1.HasServiceId)
                statusRDMMsg1.ServiceId = serviceId;

            if (statusRDMMsg1.HasFilter)
                statusRDMMsg1.Filter = filter;

            if (statusRDMMsg1.ClearCache)
            {
                statusRDMMsg1.ClearCache = true;
            }

            CodecReturnCode ret = statusRDMMsg1.Copy(statusRDMMsg2);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);
            Assert.Equal(streamId, statusRDMMsg2.StreamId);
            Assert.Equal(serviceId, statusRDMMsg2.ServiceId);
            Assert.Equal(filter, statusRDMMsg2.Filter);
            Assert.Equal(flags, statusRDMMsg2.Flags);
            Assert.Equal(statusRDMMsg1.ClearCache, statusRDMMsg2.ClearCache);

            if (statusRDMMsg1.HasState)
            {
                State refState1 = statusRDMMsg1.State;
                State refState2 = statusRDMMsg2.State;
                Assert.NotNull(refState2);
                Assert.Equal(refState1.Code(), refState2.Code());
                Assert.Equal(refState1.DataState(), refState2.DataState());
                Assert.Equal(refState1.StreamState(), refState2.StreamState());
                Assert.Equal(refState1.Text().ToString(), refState2.Text().ToString());
                Assert.True(refState1.Text() != refState2.Text());
            }
            Console.WriteLine("Done.");
        }

        [Fact]
        public void DirectoryStatusToStringTests()
        {
            DirectoryStatus statusRDMMsg1 = new DirectoryStatus();

            Console.WriteLine("DirectoryStatus ToString tests...");

            /* Parameter setup */
            int streamId = -5;
            int serviceId = 1;
            int filter = 1;
            State state = new State();
            Buffer buffer = new Buffer();
            buffer.Data("state");
            state.Text(buffer);
            state.Code(StateCodes.FAILOVER_COMPLETED);
            state.DataState(DataStates.SUSPECT);
            state.StreamState(StreamStates.OPEN);

            DirectoryStatusFlags flags = DirectoryStatusFlags.HAS_FILTER | DirectoryStatusFlags.HAS_SERVICE_ID | DirectoryStatusFlags.HAS_STATE;
            statusRDMMsg1.Clear();
            statusRDMMsg1.Flags = flags;
            statusRDMMsg1.StreamId = streamId;
            if (statusRDMMsg1.HasState)
            {
                statusRDMMsg1.State.Code(state.Code());
                statusRDMMsg1.State.DataState(state.DataState());
                statusRDMMsg1.State.Text().Data("state");
                statusRDMMsg1.State.StreamState(state.StreamState());
            }

            if (statusRDMMsg1.HasServiceId)
                statusRDMMsg1.ServiceId = serviceId;

            if (statusRDMMsg1.HasFilter)
                statusRDMMsg1.Filter = filter;

            string directoryStatusStr = statusRDMMsg1.ToString();
            Assert.Contains("DirectoryStatus:", directoryStatusStr);
            Assert.Contains("streamId: -5", directoryStatusStr);
            Assert.Contains("filter: 1", directoryStatusStr);
            Assert.Contains("serviceId: 1", directoryStatusStr);
            Assert.Contains("state: ", directoryStatusStr);

            Console.WriteLine("Done.");
        }

        [Fact]
        public void DirectoryRequestTests()
        {
            DirectoryRequest encRDMMsg = new DirectoryRequest();
            DirectoryRequest decRDMMsg = new DirectoryRequest();

            Console.WriteLine("DirectoryRequest tests...");

            /* Parameter setup */
            int streamId = -5;
            int serviceId = 1;
            int filter = 1;
            State state = new State();
            Buffer buffer = new Buffer();
            buffer.Data("state");
            state.Text(buffer);
            state.Code(StateCodes.FAILOVER_COMPLETED);
            state.DataState(DataStates.SUSPECT);
            state.StreamState(StreamStates.OPEN);

            DirectoryRequestFlags[] flagsBase =
            {
                DirectoryRequestFlags.HAS_SERVICE_ID,
                DirectoryRequestFlags.STREAMING
            };

            DirectoryRequestFlags[] flagsList = TypedMessageUtil.CreateFlagCombinations(flagsBase, false);

            Console.WriteLine("DirectoryRequest tests...");

            for (int i = 0; i < flagsList.Length; i++)
            {
                dIter.Clear();
                encIter.Clear();
                Buffer membuf = new Buffer();
                membuf.Data(new ByteBuffer(1024));

                DirectoryRequestFlags flags = flagsList[i];

                encRDMMsg.Clear();
                encRDMMsg.Flags = flags;
                encRDMMsg.StreamId = streamId;
                encRDMMsg.Filter = filter;
                if (encRDMMsg.HasServiceId)
                    encRDMMsg.ServiceId = serviceId;

                encIter.SetBufferAndRWFVersion(membuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
                CodecReturnCode ret = encRDMMsg.Encode(encIter);
                Assert.Equal(CodecReturnCode.SUCCESS, ret);

                dIter.SetBufferAndRWFVersion(membuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
                ret = msg.Decode(dIter);
                Assert.Equal(CodecReturnCode.SUCCESS, ret);
                ret = decRDMMsg.Decode(dIter, msg);
                Assert.Equal(CodecReturnCode.SUCCESS, ret);
                Assert.Equal(flags, decRDMMsg.Flags);

                Assert.Equal(filter, decRDMMsg.Filter);
                Assert.Equal(streamId, decRDMMsg.StreamId);
                if (decRDMMsg.HasServiceId)
                    Assert.Equal(serviceId, decRDMMsg.ServiceId);
            }
            Console.WriteLine("Done.");
        }

        [Fact]
        public void DirectoryRequestCopyTests()
        {
            DirectoryRequest reqMsg1 = new DirectoryRequest();
            DirectoryRequest reqMsg2 = new DirectoryRequest();

            Console.WriteLine("DirectoryRequest copy tests...");

            /* Parameter setup */
            int streamId = -5;
            int serviceId = 1;
            int filter = 1;

            reqMsg1.Clear();
            reqMsg1.Streaming = true;
            reqMsg1.StreamId = streamId;
            reqMsg1.Filter = filter;
            reqMsg1.HasServiceId = true;
            reqMsg1.ServiceId = serviceId;
            CodecReturnCode ret = reqMsg1.Copy(reqMsg2);
            Assert.Equal(CodecReturnCode.SUCCESS, ret);

            Assert.Equal(reqMsg1.Flags, reqMsg2.Flags);
            Assert.Equal(reqMsg1.Streaming, reqMsg2.Streaming);
            Assert.Equal(filter, reqMsg2.Filter);
            Assert.Equal(streamId, reqMsg2.StreamId);
            Assert.Equal(serviceId, reqMsg2.ServiceId);
            Console.WriteLine("Done.");
        }

        [Fact]
        public void DirectoryRequestToStringTests()
        {
            DirectoryRequest encRDMMsg = new DirectoryRequest();

            Console.WriteLine("DirectoryRequest ToString tests...");

            /* Parameter setup */
            int streamId = -5;
            int serviceId = 1;
            int filter = ServiceFilterFlags.DATA | ServiceFilterFlags.GROUP | ServiceFilterFlags.INFO | ServiceFilterFlags.LINK | ServiceFilterFlags.LOAD | ServiceFilterFlags.STATE;

            encRDMMsg.Clear();
            encRDMMsg.StreamId = streamId;
            encRDMMsg.Filter = filter;
            encRDMMsg.HasServiceId = true;
            encRDMMsg.ServiceId = serviceId;

            string directoryRequestStr = encRDMMsg.ToString();
            Assert.Contains("DirectoryRequest: ", directoryRequestStr);
            Assert.Contains("streaming: False", directoryRequestStr);
            Assert.Contains("filter: INFO | DATA | GROUP | LINK | LOAD | STATE", directoryRequestStr);
            Assert.Contains("streamId: -5", directoryRequestStr);
            Assert.Contains("serviceId: 1", directoryRequestStr);

            Console.WriteLine("Done.");
        }


    }
}
