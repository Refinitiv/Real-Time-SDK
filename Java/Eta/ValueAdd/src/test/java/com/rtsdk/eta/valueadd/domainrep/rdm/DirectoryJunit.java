///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.rtsdk.eta.valueadd.domainrep.rdm;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.List;

import org.junit.Test;

import com.rtsdk.eta.codec.Buffer;
import com.rtsdk.eta.codec.Codec;
import com.rtsdk.eta.codec.CodecFactory;
import com.rtsdk.eta.codec.CodecReturnCodes;
import com.rtsdk.eta.codec.DataStates;
import com.rtsdk.eta.codec.DataTypes;
import com.rtsdk.eta.codec.DecodeIterator;
import com.rtsdk.eta.codec.EncodeIterator;
import com.rtsdk.eta.codec.FilterEntryActions;
import com.rtsdk.eta.codec.MapEntryActions;
import com.rtsdk.eta.codec.Msg;
import com.rtsdk.eta.codec.Qos;
import com.rtsdk.eta.codec.QosRates;
import com.rtsdk.eta.codec.QosTimeliness;
import com.rtsdk.eta.codec.State;
import com.rtsdk.eta.codec.StateCodes;
import com.rtsdk.eta.codec.StreamStates;
import com.rtsdk.eta.codec.UInt;
import com.rtsdk.eta.rdm.Directory;
import com.rtsdk.eta.valueadd.domainrep.rdm.directory.ConsumerStatusService;
import com.rtsdk.eta.valueadd.domainrep.rdm.directory.DirectoryClose;
import com.rtsdk.eta.valueadd.domainrep.rdm.directory.DirectoryConsumerStatus;
import com.rtsdk.eta.valueadd.domainrep.rdm.directory.DirectoryMsgFactory;
import com.rtsdk.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType;
import com.rtsdk.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh;
import com.rtsdk.eta.valueadd.domainrep.rdm.directory.DirectoryRefreshFlags;
import com.rtsdk.eta.valueadd.domainrep.rdm.directory.DirectoryRequest;
import com.rtsdk.eta.valueadd.domainrep.rdm.directory.DirectoryRequestFlags;
import com.rtsdk.eta.valueadd.domainrep.rdm.directory.DirectoryStatus;
import com.rtsdk.eta.valueadd.domainrep.rdm.directory.DirectoryStatusFlags;
import com.rtsdk.eta.valueadd.domainrep.rdm.directory.DirectoryUpdate;
import com.rtsdk.eta.valueadd.domainrep.rdm.directory.DirectoryUpdateFlags;
import com.rtsdk.eta.valueadd.domainrep.rdm.directory.Service;

public class DirectoryJunit
{
    private DecodeIterator dIter = CodecFactory.createDecodeIterator();
    private EncodeIterator encIter = CodecFactory.createEncodeIterator();
    private Msg msg = CodecFactory.createMsg();

    private void buildRDMServiceGroup(List<Service.ServiceGroup> groupStateList, int action)
    {
        int flags =
                Service.ServiceGroupFlags.HAS_MERGED_TO_GROUP
                        | Service.ServiceGroupFlags.HAS_STATUS;
        State state = CodecFactory.createState();
        state.text().data("state");
        state.code(StateCodes.FAILOVER_COMPLETED);
        state.dataState(DataStates.SUSPECT);
        state.streamState(StreamStates.OPEN);

        Service.ServiceGroup rdmServiceGroupState = new Service.ServiceGroup();
        rdmServiceGroupState.clear();
        rdmServiceGroupState.flags(flags);
        rdmServiceGroupState.action(action);
        rdmServiceGroupState.group().data("group");
        if (rdmServiceGroupState.checkHasMergedToGroup())
        {
            rdmServiceGroupState.mergedToGroup().data("mergedToGroup");
        }

        if (rdmServiceGroupState.checkHasStatus())
        {
            rdmServiceGroupState.status().text().data("state");
            rdmServiceGroupState.status().code(StateCodes.FAILOVER_COMPLETED);
            rdmServiceGroupState.status().dataState(DataStates.SUSPECT);
            rdmServiceGroupState.status().streamState(StreamStates.OPEN);
        }
        
        groupStateList.add(rdmServiceGroupState);
    }

    private void buildRDMServiceData(Service.ServiceData rdmServiceData, int action)
    {
        int flags = Service.ServiceDataFlags.HAS_DATA;

        rdmServiceData.clear();

        rdmServiceData.flags(flags);
        rdmServiceData.action(action);
        rdmServiceData.type(1);
        if (rdmServiceData.checkHasData())
        {
            rdmServiceData.data().data("data");
            rdmServiceData.dataType(DataTypes.ASCII_STRING);
        }
    }

    private void buildRDMServiceLoad(Service.ServiceLoad rdmServiceLoad, int action)
    {
        int flags = Service.ServiceLoadFlags.HAS_LOAD_FACTOR |
                Service.ServiceLoadFlags.HAS_OPEN_LIMIT |
                Service.ServiceLoadFlags.HAS_OPEN_WINDOW;

        long loadFactor = 1;
        long openLimit = 1;
        long openWindow = 1;

        rdmServiceLoad.clear();

        rdmServiceLoad.flags(flags);
        rdmServiceLoad.action(action);
        if (rdmServiceLoad.checkHasOpenLimit())
        {
            rdmServiceLoad.openLimit(openLimit);
            rdmServiceLoad.applyHasOpenLimit();
        }

        if (rdmServiceLoad.checkHasOpenWindow())
        {
            rdmServiceLoad.openWindow(openWindow);
            rdmServiceLoad.applyHasOpenWindow();
        }

        if (rdmServiceLoad.checkHasLoadFactor())
        {
            rdmServiceLoad.loadFactor(loadFactor);
            rdmServiceLoad.applyHasLoadFactor();
        }
    }

    private void buildRDMServiceState(Service.ServiceState rdmServiceState, int action)
    {
        int flags = Service.ServiceStateFlags.HAS_ACCEPTING_REQS
                | Service.ServiceStateFlags.HAS_STATUS;

        long acceptingRequests = 1;
        long serviceState = 1;

        rdmServiceState.clear();

        rdmServiceState.flags(flags);
        rdmServiceState.action(action);
        rdmServiceState.serviceState(serviceState);

        rdmServiceState.acceptingRequests(acceptingRequests);

        if (rdmServiceState.checkHasStatus())
        {
            rdmServiceState.status().text().data("state");
            rdmServiceState.status().code(StateCodes.FAILOVER_COMPLETED);
            rdmServiceState.status().dataState(DataStates.SUSPECT);
            rdmServiceState.status().streamState(StreamStates.OPEN);
        }
    }

    private void buildRDMServiceLink(Service.ServiceLinkInfo serviceLinkInfo, int action)
    {
        int flags = Service.ServiceLinkFlags.HAS_CODE |
                Service.ServiceLinkFlags.HAS_TEXT |
                Service.ServiceLinkFlags.HAS_TYPE;

        long linkCode = 1;
        long linkState = 1;
        long type = DataTypes.ASCII_STRING;

        Service.ServiceLink serviceLink = new Service.ServiceLink();

        serviceLinkInfo.linkList().add(serviceLink);
        serviceLinkInfo.action(action);
        serviceLink.clear();

        serviceLink.action(MapEntryActions.ADD);
        serviceLink.flags(flags);
        serviceLink.name().data("name");
        serviceLink.linkState(linkState);

        if (serviceLink.checkHasCode())
        {
            serviceLink.linkCode(linkCode);
        }

        if (serviceLink.checkHasText())
        {
            serviceLink.text().data("text");
        }

        if (serviceLink.checkHasType())
        {
            serviceLink.type(type);
        }
    }

    private void buildRDMServiceInfo(Service.ServiceInfo rdmServiceInfo, int action)
    {
        int flags = Service.ServiceInfoFlags.HAS_ACCEPTING_CONS_STATUS |
                Service.ServiceInfoFlags.HAS_DICTS_PROVIDED |
                Service.ServiceInfoFlags.HAS_DICTS_USED |
                Service.ServiceInfoFlags.HAS_IS_SOURCE |
                Service.ServiceInfoFlags.HAS_ITEM_LIST |
                Service.ServiceInfoFlags.HAS_QOS |
                Service.ServiceInfoFlags.HAS_SUPPORT_OOB_SNAPSHOTS |
                Service.ServiceInfoFlags.HAS_SUPPORT_QOS_RANGE |
                Service.ServiceInfoFlags.HAS_VENDOR;

        Qos qos = CodecFactory.createQos();
        qos.dynamic(true);
        qos.rate(QosRates.JIT_CONFLATED);
        qos.timeliness(QosTimeliness.DELAYED);
        qos.timeInfo(1);
        rdmServiceInfo.clear();
        rdmServiceInfo.action(action);
        rdmServiceInfo.flags(flags);
        rdmServiceInfo.acceptingConsumerStatus(1);
        rdmServiceInfo.dictionariesProvidedList().add("dictprov1");
        rdmServiceInfo.dictionariesUsedList().add("dictused1");
        rdmServiceInfo.isSource(1);
        rdmServiceInfo.itemList().data("itemList");
        rdmServiceInfo.qosList().add(qos);
        rdmServiceInfo.supportsOutOfBandSnapshots(1);
        rdmServiceInfo.supportsQosRange(1);
        rdmServiceInfo.vendor().data("vendor");
        rdmServiceInfo.capabilitiesList().add((long)1);
        rdmServiceInfo.serviceName().data("servicename");
    }

    @Test
    public void serviceCopyTests()
    {
        int flagsBase[] =
        {
                Service.ServiceFlags.HAS_DATA,
                Service.ServiceFlags.HAS_INFO,
                Service.ServiceFlags.HAS_LINK,
                Service.ServiceFlags.HAS_LOAD,
                Service.ServiceFlags.HAS_STATE
        };

        int[] flagsList = TypedMessageTestUtil._createFlagCombinations(flagsBase, false);

        Service rdmService1 = DirectoryMsgFactory.createService();
        Service rdmService2 = DirectoryMsgFactory.createService();

        int filterActions[] = { FilterEntryActions.CLEAR, FilterEntryActions.SET, FilterEntryActions.UPDATE };
        System.out.println("Service copy tests...");

        for (int i = 0; i < flagsList.length; i++)
        {
            int flags = flagsList[i];
            for (int filterAction : filterActions)
            {
                buildRDMService(rdmService1, flags, MapEntryActions.ADD, filterAction);
                int ret = rdmService1.copy(rdmService2);
                assertEquals(CodecReturnCodes.SUCCESS, ret);
                verifyRDMService(rdmService1, rdmService2);
            }
        }
        System.out.println("Done.");
    }

    @Test
    public void serviceTests()
    {
        int flagsBase[] =
        {
                Service.ServiceFlags.HAS_DATA,
                Service.ServiceFlags.HAS_INFO,
                Service.ServiceFlags.HAS_LINK,
                Service.ServiceFlags.HAS_LOAD,
                Service.ServiceFlags.HAS_STATE
        };

        int[] flagsList = TypedMessageTestUtil._createFlagCombinations(flagsBase, false);

        Service rdmService = DirectoryMsgFactory.createService();
        Service rdmServiceDec = DirectoryMsgFactory.createService();

        int filterActions[] = { FilterEntryActions.CLEAR, FilterEntryActions.SET, FilterEntryActions.UPDATE };
        System.out.println("Service tests...");

        for (int i = 0; i < flagsList.length; i++)
        {
            int flags = flagsList[i];
            for (int filterAction : filterActions)
            {
                rdmServiceDec.clear();

                buildRDMService(rdmService, flags, MapEntryActions.ADD, filterAction);
                // allocate a ByteBuffer and associate it with an RsslBuffer
                ByteBuffer bb = ByteBuffer.allocate(1024);
                Buffer buffer = CodecFactory.createBuffer();
                buffer.data(bb);

                encIter.clear();
                encIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
                int ret = rdmService.encode(encIter);
                assertEquals(CodecReturnCodes.SUCCESS, ret);
                dIter.clear();
                bb.flip();
                buffer.data(bb);
                dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(),
                                             Codec.minorVersion());
                ret = rdmServiceDec.decode(dIter);

                assertEquals(CodecReturnCodes.SUCCESS, ret);
                verifyRDMService(rdmService, rdmServiceDec);
            }
        }
        System.out.println("Done.");
    }

    private void verifyRDMService(Service rdmService, Service rdmServiceDec)
    {
        assertEquals(rdmService.flags(), rdmServiceDec.flags());
        assertEquals(rdmService.action(), rdmServiceDec.action());

        if (rdmService.checkHasInfo() && rdmService.info().action() != FilterEntryActions.CLEAR)
        {
            verifyServiceInfo(rdmService.info(), rdmServiceDec.info());
        }
        if (rdmService.checkHasLink() && rdmService.link().action() != FilterEntryActions.CLEAR)
        {
            verifyServiceLinkList(rdmService.link(), rdmServiceDec.link());
        }
        if (rdmService.checkHasState() && rdmService.state().action() != FilterEntryActions.CLEAR)
        {
            verifyServiceState(rdmService.state(), rdmServiceDec.state());
        }
        if (rdmService.checkHasLoad() && rdmService.load().action() != FilterEntryActions.CLEAR)
        {
            verifyServiceLoad(rdmService.load(), rdmServiceDec.load());
        }
        if (rdmService.checkHasData() && rdmService.data().action() != FilterEntryActions.CLEAR)
        {
            verifyServiceData(rdmService.data(), rdmServiceDec.data());
        }
        assertEquals(rdmService.groupStateList().size(), rdmServiceDec.groupStateList().size());
        Service.ServiceGroup group = rdmService.groupStateList().get(0);

        if (group.action() != FilterEntryActions.CLEAR)
        {
            Service.ServiceGroup groupDec = rdmServiceDec.groupStateList().get(0);
            verifyServiceGroupState(group, groupDec);
        }
    }

    private void buildRDMService(Service rdmService, int flags, int serviceAddOrDeleteAction, int filterAddOrClearAction)
    {
        rdmService.clear();
        rdmService.action(serviceAddOrDeleteAction);
        rdmService.flags(flags);

        // checking only set action for the filters
        // other filter unit tests cover other filter actions
        if (rdmService.checkHasInfo())
            buildRDMServiceInfo(rdmService.info(), filterAddOrClearAction);
        if (rdmService.checkHasLink())
            buildRDMServiceLink(rdmService.link(), filterAddOrClearAction);
        if (rdmService.checkHasState())
            buildRDMServiceState(rdmService.state(), filterAddOrClearAction);
        if (rdmService.checkHasLoad())
            buildRDMServiceLoad(rdmService.load(), filterAddOrClearAction);
        if (rdmService.checkHasData())
            buildRDMServiceData(rdmService.data(), filterAddOrClearAction);
        buildRDMServiceGroup(rdmService.groupStateList(), filterAddOrClearAction);
    }

    @Test
    public void direcoryConnStatusTests()
    {
        DirectoryConsumerStatus encRDMMsg = (DirectoryConsumerStatus)DirectoryMsgFactory.createMsg();
        encRDMMsg.rdmMsgType(DirectoryMsgType.CONSUMER_STATUS);
        DirectoryConsumerStatus decRDMMsg = (DirectoryConsumerStatus)DirectoryMsgFactory.createMsg();
        decRDMMsg.rdmMsgType(DirectoryMsgType.CONSUMER_STATUS);

        /* Parameters to test with */
        int streamId = -5;

        List<ConsumerStatusService> consumerStatusServiceList = new ArrayList<ConsumerStatusService>(4);
        
        ConsumerStatusService consumerStatusService1 = DirectoryMsgFactory.createConsumerStatusService();
        consumerStatusService1.sourceMirroringMode(Directory.SourceMirroringMode.ACTIVE_WITH_STANDBY);
        consumerStatusService1.serviceId(2);
        consumerStatusService1.action(MapEntryActions.ADD);
        consumerStatusServiceList.add(consumerStatusService1);

        ConsumerStatusService consumerStatusService2 = DirectoryMsgFactory.createConsumerStatusService();
        consumerStatusService2.sourceMirroringMode(Directory.SourceMirroringMode.ACTIVE_NO_STANDBY);
        consumerStatusService2.serviceId(4);
        consumerStatusService2.action(MapEntryActions.ADD);
        consumerStatusServiceList.add(consumerStatusService2);

        ConsumerStatusService consumerStatusService3 = DirectoryMsgFactory.createConsumerStatusService();
        consumerStatusService3.sourceMirroringMode(Directory.SourceMirroringMode.ACTIVE_NO_STANDBY);
        consumerStatusService3.serviceId(5);
        consumerStatusService3.action(MapEntryActions.DELETE);
        consumerStatusServiceList.add(consumerStatusService3);

        ConsumerStatusService consumerStatusService4 = DirectoryMsgFactory.createConsumerStatusService();
        consumerStatusService4.sourceMirroringMode(Directory.SourceMirroringMode.STANDBY);
        consumerStatusService4.serviceId(6);
        consumerStatusService4.action(MapEntryActions.UPDATE);
        consumerStatusServiceList.add(consumerStatusService4);

        System.out.println("DirectoryConsumerStatus tests...");

        for (int k = 0; k <= consumerStatusServiceList.size(); ++k)
        {
            dIter.clear();
            encIter.clear();
            encRDMMsg.clear();
            encRDMMsg.rdmMsgType(DirectoryMsgType.CONSUMER_STATUS);
            encRDMMsg.streamId(streamId);

            // Encode increasing number of source mirroring info elements
            if (k >= 1)
            {
                for (int i = 0; i < k; ++i)
                {
                    encRDMMsg.consumerServiceStatusList().add(consumerStatusServiceList.get(i));
                }
            }

            Buffer membuf = CodecFactory.createBuffer();
            membuf.data(ByteBuffer.allocate(1024));

            encIter.setBufferAndRWFVersion(membuf, Codec.majorVersion(), Codec.minorVersion());
            int ret = encRDMMsg.encode(encIter);
            assertEquals(CodecReturnCodes.SUCCESS, ret);

            dIter.setBufferAndRWFVersion(membuf, Codec.majorVersion(),
                                         Codec.minorVersion());
            ret = msg.decode(dIter);
            assertEquals(CodecReturnCodes.SUCCESS, ret);
            ret = decRDMMsg.decode(dIter, msg);
            assertEquals(CodecReturnCodes.SUCCESS, ret);

            assertEquals(decRDMMsg.rdmMsgType(),
                         DirectoryMsgType.CONSUMER_STATUS);

            assertEquals(encRDMMsg.streamId(), decRDMMsg.streamId());
            assertEquals(encRDMMsg.rdmMsgType(), decRDMMsg.rdmMsgType());
            assertEquals(k, decRDMMsg.consumerServiceStatusList().size());

            for (int i = 0; i < k; ++i)
            {
                ConsumerStatusService decConsStatusService = decRDMMsg.consumerServiceStatusList().get(i);
                assertEquals(consumerStatusServiceList.get(i).serviceId(), decConsStatusService.serviceId());
                assertEquals(consumerStatusServiceList.get(i).action(), decConsStatusService.action());
                if (decConsStatusService.action() != MapEntryActions.DELETE)
                {
                    assertEquals(consumerStatusServiceList.get(i).sourceMirroringMode(), decConsStatusService.sourceMirroringMode());
                }

            }
        }
    }

    @Test
    public void directoryConnStatusCopyTests()
    {
        DirectoryConsumerStatus directoryConnStatusMsg1 = (DirectoryConsumerStatus)DirectoryMsgFactory.createMsg();
        directoryConnStatusMsg1.rdmMsgType(DirectoryMsgType.CONSUMER_STATUS);
        DirectoryConsumerStatus directoryConnStatusMsg2 = (DirectoryConsumerStatus)DirectoryMsgFactory.createMsg();
        directoryConnStatusMsg2.rdmMsgType(DirectoryMsgType.CONSUMER_STATUS);

        /* Parameters to test with */
        int streamId = -5;
        UInt serviceId = CodecFactory.createUInt();
        serviceId.value(1);

        long sourceMirroringMode = 1;

        ConsumerStatusService consumerStatusService1 = DirectoryMsgFactory.createConsumerStatusService();
        consumerStatusService1.sourceMirroringMode(sourceMirroringMode);
        consumerStatusService1.action(MapEntryActions.UPDATE);

        System.out.println("DirectoryConsumerStatus copy tests...");
        directoryConnStatusMsg1.consumerServiceStatusList().add(consumerStatusService1);
        directoryConnStatusMsg1.streamId(streamId);
        int ret = directoryConnStatusMsg1.copy(directoryConnStatusMsg2);

        assertEquals(CodecReturnCodes.SUCCESS, ret);

        // verify deep copy
        assertEquals(directoryConnStatusMsg1.streamId(), directoryConnStatusMsg2.streamId());
        assertTrue(directoryConnStatusMsg1.consumerServiceStatusList() != directoryConnStatusMsg2.consumerServiceStatusList());
        assertEquals(directoryConnStatusMsg1.consumerServiceStatusList().size(), directoryConnStatusMsg2.consumerServiceStatusList().size());
        ConsumerStatusService consStatusService1 = directoryConnStatusMsg1.consumerServiceStatusList().get(0);
        assertEquals(consumerStatusService1.sourceMirroringMode(), consStatusService1.sourceMirroringMode());

        System.out.println("Done.");
    }

    @Test
    public void directoryConnStatusToStringTests()
    {
        DirectoryConsumerStatus directoryConnStatusMsg1 = (DirectoryConsumerStatus)DirectoryMsgFactory.createMsg();
        directoryConnStatusMsg1.rdmMsgType(DirectoryMsgType.CONSUMER_STATUS);

        /* Parameters to test with */
        int streamId = -5;
        int serviceId = 1;
        long sourceMirroringMode = 1;

        ConsumerStatusService consumerStatusService1 = DirectoryMsgFactory.createConsumerStatusService();
        consumerStatusService1.sourceMirroringMode(sourceMirroringMode);
        consumerStatusService1.action(MapEntryActions.UPDATE);
        consumerStatusService1.serviceId(serviceId);

        System.out.println("DirectoryConsumerStatus toString tests...");
        directoryConnStatusMsg1.consumerServiceStatusList().add(consumerStatusService1);
        directoryConnStatusMsg1.streamId(streamId);

        directoryConnStatusMsg1.toString();
        System.out.println("Done.");
    }

    @Test
    public void consumerStatusServiceTests()
    {
        ConsumerStatusService consumerStatusService = DirectoryMsgFactory.createConsumerStatusService();
        ConsumerStatusService consumerStatusServiceDec = DirectoryMsgFactory.createConsumerStatusService();

        long sourceMirroringMode = 1;
        consumerStatusService.sourceMirroringMode(sourceMirroringMode);
        System.out.println("ConsumerStatusService tests...");
        // allocate a ByteBuffer and associate it with an RsslBuffer
        ByteBuffer bb = ByteBuffer.allocate(1024);
        Buffer buffer = CodecFactory.createBuffer();
        buffer.data(bb);

        encIter.clear();
        encIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        int ret = consumerStatusService.encode(encIter);
        assertEquals(CodecReturnCodes.SUCCESS, ret);
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(),
                                     Codec.minorVersion());
        ret = consumerStatusServiceDec.decode(dIter, msg);

        assertEquals(CodecReturnCodes.SUCCESS, ret);
        assertEquals(consumerStatusService.sourceMirroringMode(), consumerStatusServiceDec.sourceMirroringMode());

        System.out.println("Done.");
    }

    @Test
    public void serviceDataTests()
    {
        int flagsBase[] =
        {
                Service.ServiceDataFlags.HAS_DATA
        };

        int[] flagsList = TypedMessageTestUtil._createFlagCombinations(flagsBase, false);

        Service.ServiceData rdmServiceData = new Service.ServiceData();
        Service.ServiceData rdmServiceDataDec = new Service.ServiceData();

        int action = FilterEntryActions.SET;
        System.out.println("ServiceData copy tests...");

        for (int i = 0; i < flagsList.length; i++)
        {
            rdmServiceData.clear();
            rdmServiceDataDec.clear();

            int flags = flagsList[i];
            rdmServiceData.flags(flags);
            rdmServiceData.action(action);
            if (rdmServiceData.checkHasData())
            {
                rdmServiceData.data().data("data");
                rdmServiceData.dataType(DataTypes.ASCII_STRING);
            }

            // allocate a ByteBuffer and associate it with an RsslBuffer
            ByteBuffer bb = ByteBuffer.allocate(1024);
            Buffer buffer = CodecFactory.createBuffer();
            buffer.data(bb);

            encIter.clear();
            encIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
            int ret = rdmServiceData.encode(encIter);
            assertEquals(CodecReturnCodes.SUCCESS, ret);
            dIter.clear();
            bb.flip();
            buffer.data(bb);
            dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(),
                                         Codec.minorVersion());
            ret = rdmServiceDataDec.decode(dIter);

            assertEquals(CodecReturnCodes.SUCCESS, ret);
            assertEquals(rdmServiceData.flags(), rdmServiceDataDec.flags());
            assertEquals(rdmServiceData.dataType(), rdmServiceDataDec.dataType());
            if (rdmServiceData.checkHasData())
                assertEquals(rdmServiceData.data().toString(), rdmServiceDataDec.data().toString());
        }
        System.out.println("Done.");
    }

    @Test
    public void serviceStateCopyTests()
    {
        int flagsBase[] =
        {
                Service.ServiceStateFlags.HAS_ACCEPTING_REQS,
                Service.ServiceStateFlags.HAS_STATUS
        };

        int[] flagsList = TypedMessageTestUtil._createFlagCombinations(flagsBase, false);

        Service.ServiceState rdmServiceState = new Service.ServiceState();
        Service.ServiceState rdmServiceState2 = new Service.ServiceState();

        int action = FilterEntryActions.SET;
        long acceptingRequests = 1;
        long serviceState = 1;

        System.out.println("ServiceState copy tests...");

        for (int i = 0; i < flagsList.length; i++)
        {
            rdmServiceState.clear();
            rdmServiceState2.clear();

            int flags = flagsList[i];
            rdmServiceState.flags(flags);
            rdmServiceState.action(action);
            rdmServiceState.serviceState(serviceState);

            if (rdmServiceState.checkHasAcceptingRequests())
            {
                rdmServiceState.acceptingRequests(acceptingRequests);
                rdmServiceState.applyHasAcceptingRequests();
            }

            if (rdmServiceState.checkHasStatus())
            {
                rdmServiceState.status().text().data("state");
                rdmServiceState.status().code(StateCodes.FAILOVER_COMPLETED);
                rdmServiceState.status().dataState(DataStates.SUSPECT);
                rdmServiceState.status().streamState(StreamStates.OPEN);
                rdmServiceState.applyHasStatus();
            }

            int ret = rdmServiceState.copy(rdmServiceState2);

            assertEquals(CodecReturnCodes.SUCCESS, ret);
            verifyServiceState(rdmServiceState, rdmServiceState2);

        }
        System.out.println("Done.");
    }

    private void verifyServiceState(Service.ServiceState rdmServiceState, Service.ServiceState rdmServiceState2)
    {
        assertEquals(rdmServiceState.flags(), rdmServiceState2.flags());
        assertEquals(rdmServiceState.action(), rdmServiceState2.action());
        if (rdmServiceState2.checkHasAcceptingRequests())
            assertEquals(rdmServiceState.acceptingRequests(), rdmServiceState2.acceptingRequests());
        assertEquals(rdmServiceState.serviceState(), rdmServiceState2.serviceState());

        if (rdmServiceState2.checkHasStatus())
        {
            State state = rdmServiceState.status();
            State decState = rdmServiceState2.status();
            assertNotNull(decState);
            assertEquals(state.code(), decState.code());
            assertEquals(state.dataState(), decState.dataState());
            assertEquals(state.streamState(), decState.streamState());
            assertEquals(state.text().toString(), decState.text().toString());
        }
    }

    private void verifyServiceGroupState(Service.ServiceGroup rdmServiceGroupState, Service.ServiceGroup rdmServiceGroupState2)
    {
        assertEquals(rdmServiceGroupState.flags(), rdmServiceGroupState2.flags());
        assertEquals(rdmServiceGroupState.action(), rdmServiceGroupState2.action());
        if (rdmServiceGroupState2.checkHasMergedToGroup())
            assertEquals(rdmServiceGroupState.mergedToGroup().toString(), rdmServiceGroupState2.mergedToGroup().toString());
        assertEquals(rdmServiceGroupState.group().toString(), rdmServiceGroupState2.group().toString());

        if (rdmServiceGroupState2.checkHasStatus())
        {
            State decState = rdmServiceGroupState2.status();
            State state = rdmServiceGroupState.status();
            assertNotNull(decState);
            assertEquals(state.code(), decState.code());
            assertEquals(state.dataState(), decState.dataState());
            assertEquals(state.streamState(), decState.streamState());
            assertEquals(state.text().toString(), decState.text().toString());
        }
    }

    private void verifyServiceData(Service.ServiceData rdmServiceData, Service.ServiceData rdmServiceData2)
    {
        assertEquals(rdmServiceData.flags(), rdmServiceData2.flags());
        assertEquals(rdmServiceData.type(), rdmServiceData2.type());
        assertEquals(rdmServiceData.action(), rdmServiceData2.action());
        if (rdmServiceData.checkHasData())
        {
            assertEquals(rdmServiceData.dataType(), rdmServiceData2.dataType());
            assertEquals(rdmServiceData.data().toString(), rdmServiceData2.data().toString());
        }
    }

    @Test
    public void serviceGroupStateTests()
    {
        int flagsBase[] =
        {
                Service.ServiceGroupFlags.HAS_MERGED_TO_GROUP,
                Service.ServiceGroupFlags.HAS_STATUS // // uncomment when
                // encode/decode of state type is implemented by upaj
        };

        int[] flagsList = TypedMessageTestUtil._createFlagCombinations(flagsBase, false);

        Service.ServiceGroup rdmServiceGroupState = new Service.ServiceGroup();
        Service.ServiceGroup rdmServiceGroupStateDec = new Service.ServiceGroup();

        int action = FilterEntryActions.SET;
        State state = CodecFactory.createState();
        state.text().data("state");
        state.code(StateCodes.FAILOVER_COMPLETED);
        state.dataState(DataStates.SUSPECT);
        state.streamState(StreamStates.OPEN);

        System.out.println("ServiceGroup copy tests...");

        for (int i = 0; i < flagsList.length; i++)
        {
            rdmServiceGroupState.clear();
            rdmServiceGroupStateDec.clear();

            int flags = flagsList[i];
            rdmServiceGroupState.flags(flags);
            rdmServiceGroupState.action(action);
            rdmServiceGroupState.group().data("group");
            if (rdmServiceGroupState.checkHasMergedToGroup())
            {
                rdmServiceGroupState.mergedToGroup().data("mergedToGroup");
            }

            if (rdmServiceGroupState.checkHasStatus())
            {
                rdmServiceGroupState.status().text().data("state");
                rdmServiceGroupState.status().code(StateCodes.FAILOVER_COMPLETED);
                rdmServiceGroupState.status().dataState(DataStates.SUSPECT);
                rdmServiceGroupState.status().streamState(StreamStates.OPEN);
            }

            // allocate a ByteBuffer and associate it with an RsslBuffer
            ByteBuffer bb = ByteBuffer.allocate(1024);
            Buffer buffer = CodecFactory.createBuffer();
            buffer.data(bb);

            encIter.clear();
            encIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
            int ret = rdmServiceGroupState.encode(encIter);
            assertEquals(CodecReturnCodes.SUCCESS, ret);
            dIter.clear();
            bb.flip();
            buffer.data(bb);
            dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(),
                                         Codec.minorVersion());
            ret = rdmServiceGroupStateDec.decode(dIter);

            assertEquals(CodecReturnCodes.SUCCESS, ret);
            assertEquals(rdmServiceGroupState.flags(), rdmServiceGroupStateDec.flags());
            assertEquals(rdmServiceGroupState.group().toString(), rdmServiceGroupStateDec.group().toString());

            if (rdmServiceGroupStateDec.checkHasMergedToGroup())
                assertEquals(rdmServiceGroupState.mergedToGroup().toString(), rdmServiceGroupStateDec.mergedToGroup().toString());

            if (rdmServiceGroupStateDec.checkHasStatus())
            {
                State decState = rdmServiceGroupStateDec.status();
                assertNotNull(decState);
                assertEquals(state.code(), decState.code());
                assertEquals(state.dataState(), decState.dataState());
                assertEquals(state.streamState(), decState.streamState());
                assertEquals(state.text().toString(), decState.text().toString());
            }
        }
        System.out.println("Done.");
    }

    @Test
    public void serviceDataCopyTests()
    {
        int flagsBase[] =
        {
                Service.ServiceDataFlags.HAS_DATA
        };

        int[] flagsList = TypedMessageTestUtil._createFlagCombinations(flagsBase, false);

        Service.ServiceData rdmServiceData = new Service.ServiceData();
        Service.ServiceData rdmServiceData2 = new Service.ServiceData();

        int action = FilterEntryActions.SET;
        System.out.println("ServiceData copy tests...");

        for (int i = 0; i < flagsList.length; i++)
        {
            rdmServiceData.clear();
            rdmServiceData2.clear();

            int flags = flagsList[i];
            rdmServiceData.flags(flags);
            rdmServiceData.action(action);
            if (rdmServiceData.checkHasData())
            {
                rdmServiceData.data().data("data");
                rdmServiceData.dataType(DataTypes.ASCII_STRING);
            }

            int ret = rdmServiceData.copy(rdmServiceData2);

            assertEquals(CodecReturnCodes.SUCCESS, ret);
            assertEquals(rdmServiceData.flags(), rdmServiceData2.flags());
            assertEquals(rdmServiceData.dataType(), rdmServiceData2.dataType());
            if (rdmServiceData2.checkHasData())
            {
                assertEquals(rdmServiceData.data().toString(), rdmServiceData2.data().toString());
            }
        }
        System.out.println("Done.");
    }

    @Test
    public void serviceStateTests()
    {
        int flagsBase[] =
        {
                Service.ServiceStateFlags.HAS_ACCEPTING_REQS
                // RDMServiceStateFlags.HAS_STATUS // uncomment when
                // encode/decode of state type is implemented by upaj
        };

        int[] flagsList = TypedMessageTestUtil._createFlagCombinations(flagsBase, false);

        Service.ServiceState rdmServiceState = new Service.ServiceState();
        Service.ServiceState rdmServiceStateDec = new Service.ServiceState();

        int action = FilterEntryActions.SET;
        long acceptingRequests = 1;
        long serviceState = 1;
        State state = CodecFactory.createState();
        state.text().data("state");
        state.code(StateCodes.FAILOVER_COMPLETED);
        state.dataState(DataStates.SUSPECT);
        state.streamState(StreamStates.OPEN);

        System.out.println("ServiceState tests...");

        for (int i = 0; i < flagsList.length; i++)
        {
            rdmServiceState.clear();
            rdmServiceStateDec.clear();

            int flags = flagsList[i];
            rdmServiceState.flags(flags);
            rdmServiceState.action(action);
            rdmServiceState.serviceState(serviceState);

            if (rdmServiceState.checkHasAcceptingRequests())
            {
                rdmServiceState.acceptingRequests(acceptingRequests);
                rdmServiceState.applyHasAcceptingRequests();
            }

            if (rdmServiceState.checkHasStatus())
            {
                rdmServiceState.status().text().data("state");
                rdmServiceState.status().code(StateCodes.FAILOVER_COMPLETED);
                rdmServiceState.status().dataState(DataStates.SUSPECT);
                rdmServiceState.status().streamState(StreamStates.OPEN);
                rdmServiceState.applyHasStatus();
            }

            // allocate a ByteBuffer and associate it with an RsslBuffer
            ByteBuffer bb = ByteBuffer.allocate(1024);
            Buffer buffer = CodecFactory.createBuffer();
            buffer.data(bb);

            encIter.clear();
            encIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
            int ret = rdmServiceState.encode(encIter);
            assertEquals(CodecReturnCodes.SUCCESS, ret);
            dIter.clear();
            bb.flip();
            buffer.data(bb);
            dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(),
                                         Codec.minorVersion());
            ret = rdmServiceStateDec.decode(dIter);

            assertEquals(CodecReturnCodes.SUCCESS, ret);
            verifyServiceState(rdmServiceState, rdmServiceStateDec);
        }
        System.out.println("Done.");
    }

    @Test
    public void serviceGroupStateCopyTests()
    {
        int flagsBase[] =
        {
                Service.ServiceGroupFlags.HAS_MERGED_TO_GROUP,
                Service.ServiceGroupFlags.HAS_STATUS
        };

        int[] flagsList = TypedMessageTestUtil._createFlagCombinations(flagsBase, false);

        Service.ServiceGroup rdmServiceGroupState = new Service.ServiceGroup();
        Service.ServiceGroup rdmServiceGroupState2 = new Service.ServiceGroup();

        int action = FilterEntryActions.SET;
        State state = CodecFactory.createState();
        state.text().data("state");
        state.code(StateCodes.FAILOVER_COMPLETED);
        state.dataState(DataStates.SUSPECT);
        state.streamState(StreamStates.OPEN);

        System.out.println("ServiceGroup copy tests...");

        for (int i = 0; i < flagsList.length; i++)
        {
            rdmServiceGroupState.clear();
            rdmServiceGroupState2.clear();

            int flags = flagsList[i];
            rdmServiceGroupState.flags(flags);
            rdmServiceGroupState.action(action);
            rdmServiceGroupState.group().data("group");
            if (rdmServiceGroupState.checkHasMergedToGroup())
            {
                rdmServiceGroupState.mergedToGroup().data("mergedToGroup");
            }

            if (rdmServiceGroupState.checkHasStatus())
            {
                rdmServiceGroupState.status().text().data("state");
                rdmServiceGroupState.status().code(StateCodes.FAILOVER_COMPLETED);
                rdmServiceGroupState.status().dataState(DataStates.SUSPECT);
                rdmServiceGroupState.status().streamState(StreamStates.OPEN);
            }

            int ret = rdmServiceGroupState.copy(rdmServiceGroupState2);

            assertEquals(CodecReturnCodes.SUCCESS, ret);
            assertEquals(rdmServiceGroupState.flags(), rdmServiceGroupState2.flags());
            if (rdmServiceGroupState2.checkHasMergedToGroup())
                assertEquals(rdmServiceGroupState.mergedToGroup().toString(), rdmServiceGroupState2.mergedToGroup().toString());
            assertEquals(rdmServiceGroupState.group().toString(), rdmServiceGroupState2.group().toString());

            if (rdmServiceGroupState2.checkHasStatus())
            {
                State decState = rdmServiceGroupState2.status();
                assertNotNull(decState);
                assertEquals(state.code(), decState.code());
                assertEquals(state.dataState(), decState.dataState());
                assertEquals(state.streamState(), decState.streamState());
                assertEquals(state.text().toString(), decState.text().toString());
            }

        }
        System.out.println("Done.");
    }

    @Test
    public void serviceLinkTests()
    {
        int flagsBase[] =
        {
                Service.ServiceLinkFlags.HAS_CODE,
                Service.ServiceLinkFlags.HAS_TEXT,
                Service.ServiceLinkFlags.HAS_TYPE
        };

        int[] flagsList = TypedMessageTestUtil._createFlagCombinations(flagsBase, false);

        Service.ServiceLink rdmServiceLink = new Service.ServiceLink();
        Service.ServiceLink rdmServiceLinkDec = new Service.ServiceLink();

        long linkCode = 1;
        long linkState = 1;
        long type = 1;
        int actions[] = { MapEntryActions.ADD, MapEntryActions.DELETE };
        System.out.println("ServiceLink tests...");

        for (int i = 0; i < flagsList.length; i++)
        {
            for (int action : actions)
            {
                rdmServiceLink.clear();
                rdmServiceLinkDec.clear();
                rdmServiceLink.action(action);
                int flags = flagsList[i];
                rdmServiceLink.flags(flags);

                rdmServiceLink.linkState(linkState);

                if (rdmServiceLink.checkHasCode())
                {
                    rdmServiceLink.linkCode(linkCode);
                }

                if (rdmServiceLink.checkHasText())
                {
                    rdmServiceLink.text().data("text");
                }

                if (rdmServiceLink.checkHasType())
                {
                    rdmServiceLink.type(type);
                }
                // allocate a ByteBuffer and associate it with an RsslBuffer
                ByteBuffer bb = ByteBuffer.allocate(1024);
                Buffer buffer = CodecFactory.createBuffer();
                buffer.data(bb);

                encIter.clear();
                encIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
                int ret = rdmServiceLink.encode(encIter);
                assertEquals(CodecReturnCodes.SUCCESS, ret);
                dIter.clear();
                bb.flip();
                buffer.data(bb);
                dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(),
                                             Codec.minorVersion());
                ret = rdmServiceLinkDec.decode(dIter);

                assertEquals(CodecReturnCodes.SUCCESS, ret);
                assertEquals(rdmServiceLink.flags(), rdmServiceLinkDec.flags());
                assertEquals(rdmServiceLink.linkState(), rdmServiceLinkDec.linkState());
                if (rdmServiceLinkDec.checkHasCode())
                    assertEquals(rdmServiceLink.linkCode(), rdmServiceLinkDec.linkCode());
                if (rdmServiceLink.checkHasType())
                    assertEquals(rdmServiceLink.type(), rdmServiceLinkDec.type());
                if (rdmServiceLink.checkHasText())
                    assertEquals(rdmServiceLink.text().toString(), rdmServiceLinkDec.text().toString());
            }
        }
        System.out.println("Done.");
    }

    @Test
    public void serviceInfoCopyTests()
    {
        int flagsBase[] =
        {
                Service.ServiceInfoFlags.HAS_ACCEPTING_CONS_STATUS,
                Service.ServiceInfoFlags.HAS_DICTS_PROVIDED,
                Service.ServiceInfoFlags.HAS_DICTS_USED,
                Service.ServiceInfoFlags.HAS_IS_SOURCE,
                Service.ServiceInfoFlags.HAS_ITEM_LIST,
                Service.ServiceInfoFlags.HAS_QOS,
                Service.ServiceInfoFlags.HAS_SUPPORT_OOB_SNAPSHOTS,
                Service.ServiceInfoFlags.HAS_SUPPORT_QOS_RANGE,
                Service.ServiceInfoFlags.HAS_VENDOR
        };
        Qos qos = CodecFactory.createQos();
        qos.dynamic(true);
        qos.rate(QosRates.JIT_CONFLATED);
        qos.timeliness(QosTimeliness.DELAYED);
        qos.timeInfo(1);
        int[] flagsList = TypedMessageTestUtil._createFlagCombinations(flagsBase, false);

        Service.ServiceInfo rdmServiceInfo = new Service.ServiceInfo();
        Service.ServiceInfo rdmServiceInfoDec = new Service.ServiceInfo();

        int actions[] = { MapEntryActions.ADD, MapEntryActions.DELETE };
        System.out.println("ServiceLink tests...");

        for (int i = 0; i < flagsList.length; i++)
        {
            for (int action : actions)
            {
                rdmServiceInfo.clear();
                rdmServiceInfoDec.clear();
                rdmServiceInfo.action(action);
                int flags = flagsList[i];
                rdmServiceInfo.flags(flags);

                if (rdmServiceInfo.checkHasAcceptingConsumerStatus())
                {
                    rdmServiceInfo.acceptingConsumerStatus(1);

                }

                if (rdmServiceInfo.checkHasDictionariesProvided())
                {
                    rdmServiceInfo.dictionariesProvidedList().add("dictprov1");
                }

                if (rdmServiceInfo.checkHasDictionariesUsed())
                {
                    rdmServiceInfo.dictionariesUsedList().add("dictused1");
                }

                if (rdmServiceInfo.checkHasIsSource())
                {
                    rdmServiceInfo.isSource(1);
                }

                if (rdmServiceInfo.checkHasItemList())
                {
                    rdmServiceInfo.itemList().data("itemList");
                }

                if (rdmServiceInfo.checkHasQos())
                {
                    rdmServiceInfo.qosList().add(qos);
                }

                if (rdmServiceInfo.checkHasSupportsOutOfBandSnapshots())
                {
                    rdmServiceInfo.supportsOutOfBandSnapshots(1);
                }
                if (rdmServiceInfo.checkHasSupportsQosRange())
                {
                    rdmServiceInfo.supportsQosRange(1);
                }
                if (rdmServiceInfo.checkHasVendor())
                {
                    rdmServiceInfo.vendor().data("vendor");
                }

                rdmServiceInfo.capabilitiesList().add((long)1);
                rdmServiceInfo.serviceName().data("servicename");

                int ret = rdmServiceInfo.copy(rdmServiceInfoDec);
                assertEquals(CodecReturnCodes.SUCCESS, ret);
                assertEquals(rdmServiceInfo.flags(), rdmServiceInfoDec.flags());

                if (rdmServiceInfo.checkHasAcceptingConsumerStatus())
                {
                    assertEquals(rdmServiceInfo.acceptingConsumerStatus(), rdmServiceInfoDec.acceptingConsumerStatus());
                }

                if (rdmServiceInfo.checkHasDictionariesProvided())
                {
                    assertEquals(rdmServiceInfo.dictionariesProvidedList().size(), rdmServiceInfoDec.dictionariesProvidedList().size());
                    assertEquals(rdmServiceInfo.dictionariesProvidedList().get(0), rdmServiceInfoDec.dictionariesProvidedList().get(0));
                }

                if (rdmServiceInfo.checkHasDictionariesUsed())
                {
                    assertEquals(rdmServiceInfo.dictionariesUsedList().size(), rdmServiceInfoDec.dictionariesUsedList().size());
                    assertEquals(rdmServiceInfo.dictionariesUsedList().get(0), rdmServiceInfoDec.dictionariesUsedList().get(0));
                }

                if (rdmServiceInfo.checkHasIsSource())
                {
                    assertEquals(rdmServiceInfo.isSource(), rdmServiceInfoDec.isSource());
                }
                if (rdmServiceInfo.checkHasItemList())
                {
                    assertEquals(rdmServiceInfo.itemList().toString(), rdmServiceInfoDec.itemList().toString());
                }

                if (rdmServiceInfo.checkHasQos())
                {
                    assertEquals(rdmServiceInfo.qosList().size(), rdmServiceInfoDec.qosList().size());
                    int j = 0;
                    for (Qos qos1 : rdmServiceInfo.qosList())
                    {
                        Qos qos2 = rdmServiceInfoDec.qosList().get(j);
                        assertEquals(qos1.isDynamic(), qos2.isDynamic());
                        assertEquals(qos1.timeInfo(), qos2.timeInfo());
                        assertEquals(qos1.timeliness(), qos2.timeliness());
                        assertEquals(qos1.rate(), qos2.rate());
                        assertEquals(qos1.rateInfo(), qos2.rateInfo());
                        j++;
                    }

                }

                if (rdmServiceInfo.checkHasSupportsOutOfBandSnapshots())
                {
                    assertEquals(rdmServiceInfo.supportsOutOfBandSnapshots(), rdmServiceInfoDec.supportsOutOfBandSnapshots());
                }
                if (rdmServiceInfo.checkHasSupportsQosRange())
                {
                    assertEquals(rdmServiceInfo.supportsQosRange(), rdmServiceInfoDec.supportsQosRange());
                }

                if (rdmServiceInfo.checkHasVendor())
                {
                    assertEquals(rdmServiceInfo.vendor().toString(), rdmServiceInfoDec.vendor().toString());
                }
                assertEquals(rdmServiceInfo.capabilitiesList().size(), rdmServiceInfoDec.capabilitiesList().size());
                assertEquals(rdmServiceInfo.capabilitiesList().get(0), rdmServiceInfoDec.capabilitiesList().get(0));
                assertEquals(rdmServiceInfo.serviceName().toString(), rdmServiceInfoDec.serviceName().toString());
            }
        }
        System.out.println("Done.");

    }

    @Test
    public void serviceInfoTests()
    {
        int flagsBase[] =
        {
                Service.ServiceInfoFlags.HAS_ACCEPTING_CONS_STATUS,
                Service.ServiceInfoFlags.HAS_DICTS_PROVIDED,
                Service.ServiceInfoFlags.HAS_DICTS_USED,
                Service.ServiceInfoFlags.HAS_IS_SOURCE,
                Service.ServiceInfoFlags.HAS_ITEM_LIST,
                Service.ServiceInfoFlags.HAS_QOS,
                Service.ServiceInfoFlags.HAS_SUPPORT_OOB_SNAPSHOTS,
                Service.ServiceInfoFlags.HAS_SUPPORT_QOS_RANGE,
                Service.ServiceInfoFlags.HAS_VENDOR
        };
        Qos qos = CodecFactory.createQos();
        qos.dynamic(true);
        qos.rate(QosRates.JIT_CONFLATED);
        qos.timeliness(QosTimeliness.DELAYED);
        qos.timeInfo(1);
        int[] flagsList = TypedMessageTestUtil._createFlagCombinations(flagsBase, false);

        Service.ServiceInfo rdmServiceInfo = new Service.ServiceInfo();
        Service.ServiceInfo rdmServiceInfoDec = new Service.ServiceInfo();

        int actions[] = { MapEntryActions.ADD, MapEntryActions.DELETE };
        System.out.println("ServiceLink tests...");

        for (int i = 0; i < flagsList.length; i++)
        {
            int flags = flagsList[i];
            for (int action : actions)
            {
                rdmServiceInfoDec.clear();

                rdmServiceInfo.clear();
                rdmServiceInfo.action(action);
                rdmServiceInfo.flags(flags);

                if (rdmServiceInfo.checkHasAcceptingConsumerStatus())
                {
                    rdmServiceInfo.acceptingConsumerStatus(1);
                }

                if (rdmServiceInfo.checkHasDictionariesProvided())
                {
                    rdmServiceInfo.dictionariesProvidedList().add("dictprov1");
                }

                if (rdmServiceInfo.checkHasDictionariesUsed())
                {
                    rdmServiceInfo.dictionariesUsedList().add("dictused1");
                }

                if (rdmServiceInfo.checkHasIsSource())
                {
                    rdmServiceInfo.isSource(1);
                }

                if (rdmServiceInfo.checkHasItemList())
                {
                    rdmServiceInfo.itemList().data("itemList");
                }

                if (rdmServiceInfo.checkHasQos())
                {
                    rdmServiceInfo.qosList().add(qos);
                }

                if (rdmServiceInfo.checkHasSupportsOutOfBandSnapshots())
                {
                    rdmServiceInfo.supportsOutOfBandSnapshots(1);
                }
                if (rdmServiceInfo.checkHasSupportsQosRange())
                {
                    rdmServiceInfo.supportsQosRange(1);
                }
                if (rdmServiceInfo.checkHasVendor())
                {
                    rdmServiceInfo.vendor().data("vendor");
                }

                rdmServiceInfo.capabilitiesList().add((long)1);
                rdmServiceInfo.serviceName().data("servicename");

                // allocate a ByteBuffer and associate it with an RsslBuffer
                ByteBuffer bb = ByteBuffer.allocate(1024);
                Buffer buffer = CodecFactory.createBuffer();
                buffer.data(bb);

                encIter.clear();
                encIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
                int ret = rdmServiceInfo.encode(encIter);
                assertEquals(CodecReturnCodes.SUCCESS, ret);
                dIter.clear();
                bb.flip();
                buffer.data(bb);
                dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(),
                                             Codec.minorVersion());
                ret = rdmServiceInfoDec.decode(dIter);

                assertEquals(CodecReturnCodes.SUCCESS, ret);
                if (rdmServiceInfo.action() != FilterEntryActions.CLEAR)
                    verifyServiceInfo(rdmServiceInfo, rdmServiceInfoDec);
            }
        }
        System.out.println("Done.");
    }

    private void verifyServiceInfo(Service.ServiceInfo rdmServiceInfo, Service.ServiceInfo rdmServiceInfoDec)
    {
        assertEquals(rdmServiceInfo.flags(), rdmServiceInfoDec.flags());
        assertEquals(rdmServiceInfo.action(), rdmServiceInfoDec.action());
        if (rdmServiceInfo.checkHasAcceptingConsumerStatus())
        {
            assertEquals(rdmServiceInfo.acceptingConsumerStatus(), rdmServiceInfoDec.acceptingConsumerStatus());
        }

        if (rdmServiceInfo.checkHasDictionariesProvided())
        {
            assertEquals(rdmServiceInfo.dictionariesProvidedList().size(), rdmServiceInfoDec.dictionariesProvidedList().size());
            assertEquals(rdmServiceInfo.dictionariesProvidedList().get(0), rdmServiceInfoDec.dictionariesProvidedList().get(0));
        }

        if (rdmServiceInfo.checkHasDictionariesUsed())
        {
            assertEquals(rdmServiceInfo.dictionariesUsedList().size(), rdmServiceInfoDec.dictionariesUsedList().size());
            assertEquals(rdmServiceInfo.dictionariesUsedList().get(0), rdmServiceInfoDec.dictionariesUsedList().get(0));
        }

        if (rdmServiceInfo.checkHasIsSource())
        {
            assertEquals(rdmServiceInfo.isSource(), rdmServiceInfoDec.isSource());
        }
        if (rdmServiceInfo.checkHasItemList())
        {
            assertEquals(rdmServiceInfo.itemList().toString(), rdmServiceInfoDec.itemList().toString());
        }

        if (rdmServiceInfo.checkHasQos())
        {
            assertEquals(rdmServiceInfo.qosList().size(), rdmServiceInfoDec.qosList().size());
            Qos qos = rdmServiceInfo.qosList().get(0);
            Qos qosDec = rdmServiceInfoDec.qosList().get(0);
            assertEquals(qos.rate(), qosDec.rate());
            assertEquals(qos.isDynamic(), qosDec.isDynamic());
            assertEquals(qos.timeliness(), qosDec.timeliness());
            assertEquals(qos.timeInfo(), qosDec.timeInfo());
            assertEquals(qos.rateInfo(), qosDec.rateInfo());
        }

        if (rdmServiceInfo.checkHasSupportsOutOfBandSnapshots())
        {
            assertEquals(rdmServiceInfo.supportsOutOfBandSnapshots(), rdmServiceInfoDec.supportsOutOfBandSnapshots());
        }
        if (rdmServiceInfo.checkHasSupportsQosRange())
        {
            assertEquals(rdmServiceInfo.supportsQosRange(), rdmServiceInfoDec.supportsQosRange());
        }

        if (rdmServiceInfo.checkHasVendor())
        {
            assertEquals(rdmServiceInfo.vendor().toString(), rdmServiceInfoDec.vendor().toString());
        }
        assertEquals(rdmServiceInfo.capabilitiesList().size(), rdmServiceInfoDec.capabilitiesList().size());
        assertEquals(rdmServiceInfo.capabilitiesList().get(0), rdmServiceInfoDec.capabilitiesList().get(0));
        assertEquals(rdmServiceInfo.serviceName().toString(), rdmServiceInfoDec.serviceName().toString());
    }

    @Test
    public void serviceLinkCopyTests()
    {
        int flagsBase[] =
        {
                Service.ServiceLinkFlags.HAS_CODE,
                Service.ServiceLinkFlags.HAS_TEXT,
                Service.ServiceLinkFlags.HAS_TYPE
        };

        int[] flagsList = TypedMessageTestUtil._createFlagCombinations(flagsBase, false);

        Service.ServiceLink rdmServiceLink = new Service.ServiceLink();
        Service.ServiceLink rdmServiceLink2 = new Service.ServiceLink();

        long linkCode = 1;
        long linkState = 1;
        long type = 1;

        System.out.println("ServiceLink copy tests...");

        for (int i = 0; i < flagsList.length; i++)
        {
            rdmServiceLink.clear();
            rdmServiceLink2.clear();

            int flags = flagsList[i];
            rdmServiceLink.flags(flags);

            rdmServiceLink.linkState(linkState);

            if (rdmServiceLink.checkHasCode())
            {
                rdmServiceLink.linkCode(linkCode);
            }

            if (rdmServiceLink.checkHasText())
            {
                rdmServiceLink.text().data("text");
            }

            if (rdmServiceLink.checkHasType())
            {
                rdmServiceLink.type(type);
            }

            int ret = rdmServiceLink.copy(rdmServiceLink2);

            assertEquals(CodecReturnCodes.SUCCESS, ret);
            assertEquals(rdmServiceLink.flags(), rdmServiceLink2.flags());
            assertEquals(rdmServiceLink.linkState(), rdmServiceLink2.linkState());
            if (rdmServiceLink2.checkHasCode())
                assertEquals(rdmServiceLink.linkCode(), rdmServiceLink2.linkCode());
            if (rdmServiceLink.checkHasType())
                assertEquals(rdmServiceLink.type(), rdmServiceLink2.type());
            if (rdmServiceLink.checkHasText())
                assertEquals(rdmServiceLink.text().toString(), rdmServiceLink2.text().toString());
        }
        System.out.println("Done.");
    }

    @Test
    public void serviceLinkListCopyTests()
    {
        int flagsBase[] =
        {
                Service.ServiceLinkFlags.HAS_CODE,
                Service.ServiceLinkFlags.HAS_TEXT,
                Service.ServiceLinkFlags.HAS_TYPE
        };

        int[] flagsList = TypedMessageTestUtil._createFlagCombinations(flagsBase, false);

        Service.ServiceLink rdmServiceLink = new Service.ServiceLink();
        Service.ServiceLink rdmServiceLink2 = new Service.ServiceLink();
        Service.ServiceLinkInfo linkList1 = new Service.ServiceLinkInfo();
        Service.ServiceLinkInfo linkList2 = new Service.ServiceLinkInfo();
        long linkCode = 1;
        long linkState = 1;
        long type = 1;

        System.out.println("ServiceLink copy tests...");

        for (int i = 0; i < flagsList.length; i++)
        {
            rdmServiceLink.clear();
            rdmServiceLink2.clear();
            linkList1.clear();
            linkList2.clear();
            int flags = flagsList[i];
            rdmServiceLink.flags(flags);

            rdmServiceLink.linkState(linkState);

            if (rdmServiceLink.checkHasCode())
            {
                rdmServiceLink.linkCode(linkCode);
            }

            if (rdmServiceLink.checkHasText())
            {
                rdmServiceLink.text().data("text");
            }

            if (rdmServiceLink.checkHasType())
            {
                rdmServiceLink.type(type);
            }
            linkList1.action(FilterEntryActions.SET);
            linkList1.linkList().add(rdmServiceLink);

            int ret = linkList1.copy(linkList2);

            assertEquals(CodecReturnCodes.SUCCESS, ret);
            assertEquals(linkList1.action(), linkList2.action());
            assertEquals(linkList1.linkList().size(), linkList2.linkList().size());
            rdmServiceLink2 = linkList2.linkList().get(0);
            assertEquals(rdmServiceLink.flags(), rdmServiceLink2.flags());
            assertEquals(rdmServiceLink.linkState(), rdmServiceLink2.linkState());
            if (rdmServiceLink2.checkHasCode())
                assertEquals(rdmServiceLink.linkCode(), rdmServiceLink2.linkCode());
            if (rdmServiceLink.checkHasType())
                assertEquals(rdmServiceLink.type(), rdmServiceLink2.type());
            if (rdmServiceLink.checkHasText())
                assertEquals(rdmServiceLink.text().toString(), rdmServiceLink2.text().toString());
        }
        System.out.println("Done.");
    }

    @Test
    public void serviceLinkListTests()
    {
        int flagsBase[] =
        {
                Service.ServiceLinkFlags.HAS_CODE,
                Service.ServiceLinkFlags.HAS_TEXT,
                Service.ServiceLinkFlags.HAS_TYPE
        };

        int[] flagsList = TypedMessageTestUtil._createFlagCombinations(flagsBase, false);

        Service.ServiceLink rdmServiceLink = new Service.ServiceLink();
        Service.ServiceLinkInfo linkList = new Service.ServiceLinkInfo();
        Service.ServiceLinkInfo linkListDec = new Service.ServiceLinkInfo();

        long linkCode = 1;
        long linkState = 1;
        long type = 1;
        int actions[] = { MapEntryActions.ADD, MapEntryActions.DELETE };
        System.out.println("ServiceLink tests...");
        for (int i = 0; i < flagsList.length; i++)
        {
            for (int action : actions)
            {
                rdmServiceLink.clear();
                rdmServiceLink.action(action);
                linkList.clear();
                linkListDec.clear();
                int flags = flagsList[i];
                rdmServiceLink.flags(flags);
                rdmServiceLink.name().data("name");
                rdmServiceLink.linkState(linkState);

                if (rdmServiceLink.checkHasCode())
                {
                    rdmServiceLink.linkCode(linkCode);
                }

                if (rdmServiceLink.checkHasText())
                {
                    rdmServiceLink.text().data("text");
                }

                if (rdmServiceLink.checkHasType())
                {
                    rdmServiceLink.type(type);
                }
                linkList.action(FilterEntryActions.SET);
                linkList.linkList().add(rdmServiceLink);
                // allocate a ByteBuffer and associate it with an RsslBuffer
                ByteBuffer bb = ByteBuffer.allocate(1024);
                Buffer buffer = CodecFactory.createBuffer();
                buffer.data(bb);

                encIter.clear();
                encIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());

                int ret = linkList.encode(encIter);
                assertEquals(CodecReturnCodes.SUCCESS, ret);
                dIter.clear();
                bb.flip();
                buffer.data(bb);
                dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(),
                                             Codec.minorVersion());
                ret = linkListDec.decode(dIter);

                assertEquals(CodecReturnCodes.SUCCESS, ret);
                if (action == MapEntryActions.DELETE)
                {
                    assertEquals(0, linkListDec.linkList().size());
                }
                else
                {
                    verifyServiceLinkList(linkList, linkListDec);
                }
            }
        }
        System.out.println("Done.");
    }

    private void verifyServiceLinkList(Service.ServiceLinkInfo linkList, Service.ServiceLinkInfo linkListDec)
    {
        assertEquals(linkList.linkList().size(), linkListDec.linkList().size());
        assertEquals(linkList.action(), linkList.action());
        Service.ServiceLink rdmServiceLink = linkList.linkList().get(0);
        Service.ServiceLink rdmServiceLinkDec = linkListDec.linkList().get(0);
        assertEquals(rdmServiceLink.flags(), rdmServiceLinkDec.flags());
        assertEquals(rdmServiceLink.linkState(), rdmServiceLinkDec.linkState());
        if (rdmServiceLinkDec.checkHasCode())
            assertEquals(rdmServiceLink.linkCode(), rdmServiceLinkDec.linkCode());
        if (rdmServiceLink.checkHasType())
            assertEquals(rdmServiceLink.type(), rdmServiceLinkDec.type());
        if (rdmServiceLink.checkHasText())
            assertEquals(rdmServiceLink.text().toString(), rdmServiceLinkDec.text().toString());
    }

    @Test
    public void serviceLoadTests()
    {
        int flagsBase[] =
        {
                Service.ServiceLoadFlags.HAS_LOAD_FACTOR,
                Service.ServiceLoadFlags.HAS_OPEN_LIMIT,
                Service.ServiceLoadFlags.HAS_OPEN_WINDOW
        };

        int[] flagsList = TypedMessageTestUtil._createFlagCombinations(flagsBase, false);

        Service.ServiceLoad rdmServiceLoad = new Service.ServiceLoad();
        Service.ServiceLoad rdmServiceLoadDec = new Service.ServiceLoad();

        long loadFactor = 1;
        long openLimit = 1;
        long openWindow = 1;

        System.out.println("ServiceLoad tests...");

        for (int i = 0; i < flagsList.length; i++)
        {
            rdmServiceLoad.clear();
            rdmServiceLoadDec.clear();

            int flags = flagsList[i];
            rdmServiceLoad.flags(flags);

            if (rdmServiceLoad.checkHasOpenLimit())
            {
                rdmServiceLoad.openLimit(openLimit);
                rdmServiceLoad.applyHasOpenLimit();
            }

            if (rdmServiceLoad.checkHasOpenWindow())
            {
                rdmServiceLoad.openWindow(openWindow);
                rdmServiceLoad.applyHasOpenWindow();
            }

            if (rdmServiceLoad.checkHasLoadFactor())
            {
                rdmServiceLoad.loadFactor(loadFactor);
                rdmServiceLoad.applyHasLoadFactor();
            }

            // allocate a ByteBuffer and associate it with an RsslBuffer
            ByteBuffer bb = ByteBuffer.allocate(1024);
            Buffer buffer = CodecFactory.createBuffer();
            buffer.data(bb);

            encIter.clear();
            encIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
            int ret = rdmServiceLoad.encode(encIter);
            assertEquals(CodecReturnCodes.SUCCESS, ret);
            dIter.clear();
            bb.flip();
            buffer.data(bb);
            dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(),
                                         Codec.minorVersion());
            ret = rdmServiceLoadDec.decode(dIter);

            assertEquals(CodecReturnCodes.SUCCESS, ret);
            assertEquals(rdmServiceLoad.flags(), rdmServiceLoadDec.flags());

            verifyServiceLoad(rdmServiceLoad, rdmServiceLoadDec);
        }
        System.out.println("Done.");
    }

    private void verifyServiceLoad(Service.ServiceLoad rdmServiceLoad, Service.ServiceLoad rdmServiceLoadDec)
    {
        assertEquals(rdmServiceLoad.action(), rdmServiceLoadDec.action());
        if (rdmServiceLoadDec.checkHasLoadFactor())
            assertEquals(rdmServiceLoad.loadFactor(), rdmServiceLoadDec.loadFactor());

        if (rdmServiceLoadDec.checkHasOpenLimit())
            assertEquals(rdmServiceLoad.openLimit(), rdmServiceLoadDec.openLimit());

        if (rdmServiceLoadDec.checkHasOpenWindow())
            assertEquals(rdmServiceLoad.openWindow(), rdmServiceLoadDec.openWindow());
    }

    @Test
    public void serviceLoadCopyTests()
    {
        int flagsBase[] =
        {
                Service.ServiceLoadFlags.HAS_LOAD_FACTOR,
                Service.ServiceLoadFlags.HAS_OPEN_LIMIT,
                Service.ServiceLoadFlags.HAS_OPEN_WINDOW
        };

        int[] flagsList = TypedMessageTestUtil._createFlagCombinations(flagsBase, false);

        Service.ServiceLoad rdmServiceLoad = new Service.ServiceLoad();
        Service.ServiceLoad rdmServiceLoad2 = new Service.ServiceLoad();

        long loadFactor = 1;
        long openLimit = 1;
        long openWindow = 1;

        System.out.println("ServiceLoad copy tests...");

        for (int i = 0; i < flagsList.length; i++)
        {
            rdmServiceLoad.clear();
            rdmServiceLoad2.clear();

            int flags = flagsList[i];
            rdmServiceLoad.flags(flags);

            if (rdmServiceLoad.checkHasOpenLimit())
            {
                rdmServiceLoad.openLimit(openLimit);
                rdmServiceLoad.applyHasOpenLimit();
            }

            if (rdmServiceLoad.checkHasOpenWindow())
            {
                rdmServiceLoad.openWindow(openWindow);
                rdmServiceLoad.applyHasOpenWindow();
            }

            if (rdmServiceLoad.checkHasLoadFactor())
            {
                rdmServiceLoad.loadFactor(loadFactor);
                rdmServiceLoad.applyHasLoadFactor();
            }

            int ret = rdmServiceLoad.copy(rdmServiceLoad2);

            assertEquals(CodecReturnCodes.SUCCESS, ret);
            assertEquals(rdmServiceLoad.flags(), rdmServiceLoad2.flags());

            verifyServiceLoad(rdmServiceLoad, rdmServiceLoad2);
        }
        System.out.println("Done.");
    }

    @Test
    public void consumerStatusServiceCopyTests()
    {
        ConsumerStatusService consumerStatusService1 = DirectoryMsgFactory.createConsumerStatusService();
        ConsumerStatusService consumerStatusService2 = DirectoryMsgFactory.createConsumerStatusService();
        int serviceId = 1;
        long sourceMirroringMode = 1;
        int action = MapEntryActions.UPDATE;
        consumerStatusService1.serviceId(serviceId);
        consumerStatusService1.sourceMirroringMode(sourceMirroringMode);
        consumerStatusService1.action(action);

        System.out.println("ConsumerStatusService copy tests...");
        int ret = consumerStatusService1.copy(consumerStatusService2);

        assertEquals(CodecReturnCodes.SUCCESS, ret);

        // verify deep copy
        assertEquals(consumerStatusService1.action(), consumerStatusService2.action());
        assertEquals(consumerStatusService1.serviceId(), consumerStatusService2.serviceId());
        assertEquals(consumerStatusService1.sourceMirroringMode(), consumerStatusService2.sourceMirroringMode());

        System.out.println("Done.");
    }

    @Test
    public void consumerStatusServiceToStringTests()
    {
        ConsumerStatusService consumerStatusService1 = DirectoryMsgFactory.createConsumerStatusService();
        long sourceMirroringMode = 1;
        int serviceId = 1;
        consumerStatusService1.sourceMirroringMode(sourceMirroringMode);
        consumerStatusService1.action(MapEntryActions.UPDATE);
        consumerStatusService1.serviceId(serviceId);

        System.out.println("ConsumerStatusService toString tests...");
        consumerStatusService1.toString();

        System.out.println("Done.");
    }

    @Test
    public void directoryCloseCopyTests()
    {
        DirectoryClose closeRDMMsg1 = (DirectoryClose)DirectoryMsgFactory.createMsg();
        DirectoryClose closeRDMMsg2 = (DirectoryClose)DirectoryMsgFactory.createMsg();
        closeRDMMsg1.rdmMsgType(DirectoryMsgType.CLOSE);
        closeRDMMsg2.rdmMsgType(DirectoryMsgType.CLOSE);
        int streamId = -5;
        closeRDMMsg1.streamId(streamId);

        System.out.println("LoginClose copy tests...");

        // deep copy
        int ret = closeRDMMsg1.copy(closeRDMMsg2);
        assertEquals(CodecReturnCodes.SUCCESS, ret);

        // verify deep copy
        assertEquals(closeRDMMsg1.streamId(), closeRDMMsg2.streamId());
        System.out.println("Done.");
    }

    @Test
    public void directoryCloseToStringTests()
    {
        DirectoryClose closeRDMMsg1 = (DirectoryClose)DirectoryMsgFactory.createMsg();
        closeRDMMsg1.rdmMsgType(DirectoryMsgType.CLOSE);
        int streamId = -5;
        closeRDMMsg1.streamId(streamId);

        System.out.println("LoginClose toString tests...");

        closeRDMMsg1.toString();
        System.out.println("Done.");
    }

    @Test
    public void directoryCloseTests()
    {
        DirectoryClose encRDMMsg = (DirectoryClose)DirectoryMsgFactory.createMsg();
        DirectoryClose decRDMMsg = (DirectoryClose)DirectoryMsgFactory.createMsg();
        decRDMMsg.rdmMsgType(DirectoryMsgType.CLOSE);
        encRDMMsg.rdmMsgType(DirectoryMsgType.CLOSE);
        int streamId = -5;

        dIter.clear();
        encIter.clear();
        Buffer membuf = CodecFactory.createBuffer();
        membuf.data(ByteBuffer.allocate(1024));

        System.out.println("LoginClose tests...");
        encRDMMsg.clear();

        encRDMMsg.streamId(streamId);
        encIter.setBufferAndRWFVersion(membuf, Codec.majorVersion(), Codec.minorVersion());

        int ret = encRDMMsg.encode(encIter);
        assertEquals(CodecReturnCodes.SUCCESS, ret);

        dIter.setBufferAndRWFVersion(membuf, Codec.majorVersion(),
                                     Codec.minorVersion());
        ret = msg.decode(dIter);
        assertEquals(CodecReturnCodes.SUCCESS, ret);
        ret = decRDMMsg.decode(dIter, msg);

        assertEquals(CodecReturnCodes.SUCCESS, ret);

        assertEquals(streamId, decRDMMsg.streamId());

        System.out.println("Done.");
    }

    @Test
    public void directoryUpdateTests()
    {
        DirectoryUpdate encRDMMsg = (DirectoryUpdate)DirectoryMsgFactory.createMsg();
        encRDMMsg.rdmMsgType(DirectoryMsgType.UPDATE);
        DirectoryUpdate decRDMMsg = (DirectoryUpdate)DirectoryMsgFactory.createMsg();
        decRDMMsg.rdmMsgType(DirectoryMsgType.UPDATE);

        System.out.println("Directory update tests...");

        int flagsBase[] =
        {
                DirectoryUpdateFlags.HAS_FILTER,
                DirectoryUpdateFlags.HAS_SERVICE_ID,
                DirectoryUpdateFlags.HAS_SEQ_NUM
        };

        int serviceFlags = Service.ServiceFlags.HAS_DATA |
                Service.ServiceFlags.HAS_INFO |
                Service.ServiceFlags.HAS_LINK |
                Service.ServiceFlags.HAS_LOAD |
                Service.ServiceFlags.HAS_STATE;
        /* Parameter setup */
        int streamId = -5;
        int serviceId = 1;
        int filter = 2;
        int seqNum = 1;
        Service rdmService = DirectoryMsgFactory.createService();
        int[] flagsList = TypedMessageTestUtil._createFlagCombinations(flagsBase, false);
        int[] actions = { MapEntryActions.ADD, MapEntryActions.UPDATE, MapEntryActions.DELETE };
        for (int i = 0; i < flagsList.length; i++)
        {
            int flags = flagsList[i];
            for (int serviceAction : actions)
            {
                dIter.clear();
                encIter.clear();
                encRDMMsg.clear();
                decRDMMsg.clear();
                Buffer membuf = CodecFactory.createBuffer();
                membuf.data(ByteBuffer.allocate(1024));

                encRDMMsg.flags(flags);
                encRDMMsg.streamId(streamId);

                if (encRDMMsg.checkHasServiceId())
                    encRDMMsg.serviceId(serviceId);

                if (encRDMMsg.checkHasFilter())
                    encRDMMsg.filter(filter);

                if (encRDMMsg.checkHasSequenceNumber())
                    encRDMMsg.sequenceNumber(seqNum);
				// filter action clear is not tested here. see service tests for complete coverage
                buildRDMService(rdmService, serviceFlags, serviceAction, FilterEntryActions.SET); 
                encRDMMsg.serviceList().add(rdmService);
                encIter.setBufferAndRWFVersion(membuf, Codec.majorVersion(), Codec.minorVersion());
                int ret = encRDMMsg.encode(encIter);
                assertEquals(CodecReturnCodes.SUCCESS, ret);

                dIter.setBufferAndRWFVersion(membuf, Codec.majorVersion(), Codec.minorVersion());
                ret = msg.decode(dIter);
                assertEquals(CodecReturnCodes.SUCCESS, ret);
                ret = decRDMMsg.decode(dIter, msg);
                assertEquals(CodecReturnCodes.SUCCESS, ret);
                assertEquals(streamId, decRDMMsg.streamId());
                assertEquals(flags, decRDMMsg.flags());

                /* Check parameters */
                if (encRDMMsg.checkHasServiceId())
                {
                    assertEquals(serviceId, decRDMMsg.serviceId());
                }

                if (encRDMMsg.checkHasFilter())
                {
                    assertEquals(filter, decRDMMsg.filter());
                }

                if (encRDMMsg.checkHasSequenceNumber())
                {
                    assertEquals(seqNum, decRDMMsg.sequenceNumber());
                }

                if (encRDMMsg.serviceList().get(0).action() != MapEntryActions.DELETE)
                {
                    assertEquals(encRDMMsg.serviceList().size(), decRDMMsg.serviceList().size());
                    verifyRDMService(encRDMMsg.serviceList().get(0), decRDMMsg.serviceList().get(0));
                }
            }
        }
        System.out.println("Done.");
    }

    @Test
    public void directoryRefreshCopyTests()
    {
        DirectoryRefresh rdmDirRefreshMsg1 = (DirectoryRefresh)DirectoryMsgFactory.createMsg();
        rdmDirRefreshMsg1.rdmMsgType(DirectoryMsgType.REFRESH);
        DirectoryRefresh rdmDirRefreshMsg2 = (DirectoryRefresh)DirectoryMsgFactory.createMsg();
        rdmDirRefreshMsg2.rdmMsgType(DirectoryMsgType.REFRESH);

        System.out.println("Directory refresh copy tests...");

        int flagsBase[] =
        {
                DirectoryRefreshFlags.CLEAR_CACHE,
                DirectoryRefreshFlags.SOLICITED,
                DirectoryRefreshFlags.HAS_SERVICE_ID,
                DirectoryRefreshFlags.HAS_SEQ_NUM
        };

        int serviceFlags = Service.ServiceFlags.HAS_DATA |
                Service.ServiceFlags.HAS_INFO |
                Service.ServiceFlags.HAS_LINK |
                Service.ServiceFlags.HAS_LOAD |
                Service.ServiceFlags.HAS_STATE;

        /* Parameter setup */
        int streamId = -5;
        int serviceId = 1;
        int filter = 2;
        int seqNum = 1;
        State state = CodecFactory.createState();
        Buffer buffer = CodecFactory.createBuffer();
        buffer.data("state");
        state.text(buffer);
        state.code(StateCodes.FAILOVER_COMPLETED);
        state.dataState(DataStates.SUSPECT);
        state.streamState(StreamStates.OPEN);

        Service rdmService = DirectoryMsgFactory.createService();
        int[] flagsList = TypedMessageTestUtil._createFlagCombinations(flagsBase, false);
        int[] serviceAddorDeleteActions = { MapEntryActions.ADD, MapEntryActions.DELETE };

        for (int i = 0; i < flagsList.length; i++)
        {
            int flags = flagsList[i];
            for (int serviceAddorDeleteAction : serviceAddorDeleteActions)
            {
                rdmDirRefreshMsg1.clear();
                rdmDirRefreshMsg2.clear();

                rdmDirRefreshMsg1.flags(flags);
                rdmDirRefreshMsg1.streamId(streamId);

                if (rdmDirRefreshMsg1.checkHasServiceId())
                    rdmDirRefreshMsg1.serviceId(serviceId);

                rdmDirRefreshMsg1.filter(filter);
                rdmDirRefreshMsg1.state().code(state.code());
                rdmDirRefreshMsg1.state().dataState(state.dataState());
                rdmDirRefreshMsg1.state().text().data("state");
                rdmDirRefreshMsg1.state().streamState(state.streamState());

                if (rdmDirRefreshMsg1.checkHasSequenceNumber())
                    rdmDirRefreshMsg1.sequenceNumber(seqNum);
                buildRDMService(rdmService, serviceFlags, serviceAddorDeleteAction, FilterEntryActions.SET);
                rdmDirRefreshMsg1.serviceList().add(rdmService);

                int ret = rdmDirRefreshMsg1.copy(rdmDirRefreshMsg2);
                assertEquals(CodecReturnCodes.SUCCESS, ret);
                assertEquals(streamId, rdmDirRefreshMsg2.streamId());
                assertEquals(flags, rdmDirRefreshMsg2.flags());

                /* Check parameters */
                if (rdmDirRefreshMsg1.checkHasServiceId())
                {
                    assertEquals(serviceId, rdmDirRefreshMsg2.serviceId());
                }

                assertEquals(filter, rdmDirRefreshMsg2.filter());

                if (rdmDirRefreshMsg1.checkHasSequenceNumber())
                {
                    assertEquals(seqNum, rdmDirRefreshMsg2.sequenceNumber());
                }

                if (rdmDirRefreshMsg1.checkSolicited())
                {
                    assertEquals(rdmDirRefreshMsg1.checkSolicited(), rdmDirRefreshMsg2.checkSolicited());
                }
                if (rdmDirRefreshMsg1.checkClearCache())
                {
                    assertEquals(rdmDirRefreshMsg1.checkClearCache(), rdmDirRefreshMsg2.checkClearCache());
                }

                State state2 = rdmDirRefreshMsg2.state();
                assertNotNull(state2);
                assertEquals(state.code(), state2.code());
                assertEquals(state.dataState(), state2.dataState());
                assertEquals(state.streamState(), state2.streamState());
                assertEquals(state.text().toString(), state2.text().toString());

                if (rdmDirRefreshMsg1.serviceList().get(0).action() != MapEntryActions.DELETE)
                {
                    assertEquals(rdmDirRefreshMsg1.serviceList().size(), rdmDirRefreshMsg2.serviceList().size());
                    verifyRDMService(rdmDirRefreshMsg1.serviceList().get(0), rdmDirRefreshMsg2.serviceList().get(0));
                }
            }

        }
        System.out.println("Done.");
    }

    @Test
    public void directoryUpdateCopyTests()
    {
        DirectoryUpdate rdmDirUpdateMsg1 = (DirectoryUpdate)DirectoryMsgFactory.createMsg();
        rdmDirUpdateMsg1.rdmMsgType(DirectoryMsgType.UPDATE);
        DirectoryUpdate rdmDirUpdateMsg2 = (DirectoryUpdate)DirectoryMsgFactory.createMsg();
        rdmDirUpdateMsg2.rdmMsgType(DirectoryMsgType.UPDATE);

        System.out.println("Directory update copy tests...");

        int flagsBase[] =
        {
                DirectoryUpdateFlags.HAS_FILTER,
                DirectoryUpdateFlags.HAS_SERVICE_ID,
                DirectoryUpdateFlags.HAS_SEQ_NUM
        };

        int serviceFlags = Service.ServiceFlags.HAS_DATA |
                Service.ServiceFlags.HAS_INFO |
                Service.ServiceFlags.HAS_LINK |
                Service.ServiceFlags.HAS_LOAD |
                Service.ServiceFlags.HAS_STATE;

        /* Parameter setup */
        int streamId = -5;
        int serviceId = 1;
        int filter = 2;
        int seqNum = 1;
        Service rdmService = DirectoryMsgFactory.createService();
        int[] flagsList = TypedMessageTestUtil._createFlagCombinations(flagsBase, false);
        int[] serviceAddorDeleteActions = { MapEntryActions.ADD, MapEntryActions.DELETE };
        for (int i = 0; i < flagsList.length; i++)
        {
            int flags = flagsList[i];
            for (int serviceAddorDeleteAction : serviceAddorDeleteActions)
            {
                rdmDirUpdateMsg1.clear();
                rdmDirUpdateMsg2.clear();

                rdmDirUpdateMsg1.flags(flags);
                rdmDirUpdateMsg1.streamId(streamId);

                if (rdmDirUpdateMsg1.checkHasServiceId())
                    rdmDirUpdateMsg1.serviceId(serviceId);

                if (rdmDirUpdateMsg1.checkHasFilter())
                    rdmDirUpdateMsg1.filter(filter);

                if (rdmDirUpdateMsg1.checkHasSequenceNumber())
                    rdmDirUpdateMsg1.sequenceNumber(seqNum);
                buildRDMService(rdmService, serviceFlags, serviceAddorDeleteAction, FilterEntryActions.SET);
                rdmDirUpdateMsg1.serviceList().add(rdmService);

                int ret = rdmDirUpdateMsg1.copy(rdmDirUpdateMsg2);
                assertEquals(CodecReturnCodes.SUCCESS, ret);
                assertEquals(streamId, rdmDirUpdateMsg2.streamId());
                assertEquals(flags, rdmDirUpdateMsg2.flags());

                /* Check parameters */
                if (rdmDirUpdateMsg1.checkHasServiceId())
                {
                    assertEquals(serviceId, rdmDirUpdateMsg2.serviceId());
                }

                if (rdmDirUpdateMsg1.checkHasFilter())
                {
                    assertEquals(filter, rdmDirUpdateMsg2.filter());
                }

                if (rdmDirUpdateMsg1.checkHasSequenceNumber())
                {
                    assertEquals(seqNum, rdmDirUpdateMsg2.sequenceNumber());
                }

                if (rdmDirUpdateMsg1.serviceList().get(0).action() != MapEntryActions.DELETE)
                {
                    assertEquals(rdmDirUpdateMsg1.serviceList().size(), rdmDirUpdateMsg2.serviceList().size());
                    verifyRDMService(rdmDirUpdateMsg1.serviceList().get(0), rdmDirUpdateMsg2.serviceList().get(0));
                }
            }

        }
        System.out.println("Done.");
    }

    @Test
    public void directoryRefreshTests()
    {
        DirectoryRefresh encRDMMsg = (DirectoryRefresh)DirectoryMsgFactory.createMsg();
        encRDMMsg.rdmMsgType(DirectoryMsgType.REFRESH);
        DirectoryRefresh decRDMMsg = (DirectoryRefresh)DirectoryMsgFactory.createMsg();
        decRDMMsg.rdmMsgType(DirectoryMsgType.REFRESH);

        System.out.println("Directory refresh tests...");

        int flagsBase[] =
        {
                DirectoryRefreshFlags.HAS_SERVICE_ID,
                DirectoryRefreshFlags.HAS_SEQ_NUM,
                DirectoryRefreshFlags.CLEAR_CACHE,
                DirectoryRefreshFlags.SOLICITED
        };

        int serviceFlags = Service.ServiceFlags.HAS_DATA |
                Service.ServiceFlags.HAS_INFO |
                Service.ServiceFlags.HAS_LINK |
                Service.ServiceFlags.HAS_LOAD |
                Service.ServiceFlags.HAS_STATE;

        /* Parameter setup */
        int streamId = -5;
        int serviceId = 1;
        int filter = 2;
        int seqNum = 1;
        State state = CodecFactory.createState();
        Buffer buffer = CodecFactory.createBuffer();
        buffer.data("state");
        state.text(buffer);
        state.code(StateCodes.FAILOVER_COMPLETED);
        state.dataState(DataStates.SUSPECT);
        state.streamState(StreamStates.OPEN);

        Service rdmService = DirectoryMsgFactory.createService();
        int[] flagsList = TypedMessageTestUtil._createFlagCombinations(flagsBase, false);
        int[] actions = { MapEntryActions.ADD, MapEntryActions.UPDATE, MapEntryActions.DELETE };
        for (int i = 0; i < flagsList.length; i++)
        {
            int flags = flagsList[i];
            for (int serviceAction : actions)
            {
                dIter.clear();
                encIter.clear();
                encRDMMsg.clear();
                decRDMMsg.clear();
                Buffer membuf = CodecFactory.createBuffer();
                membuf.data(ByteBuffer.allocate(1024));

                encRDMMsg.flags(flags);
                encRDMMsg.streamId(streamId);

                if (encRDMMsg.checkHasServiceId())
                    encRDMMsg.serviceId(serviceId);

                encRDMMsg.state().code(state.code());
                encRDMMsg.state().dataState(state.dataState());
                encRDMMsg.state().text().data("state");
                encRDMMsg.state().streamState(state.streamState());

                encRDMMsg.filter(filter);

                if (encRDMMsg.checkHasSequenceNumber())
                    encRDMMsg.sequenceNumber(seqNum);
                buildRDMService(rdmService, serviceFlags, serviceAction, FilterEntryActions.SET);
                encRDMMsg.serviceList().add(rdmService);
                encIter.setBufferAndRWFVersion(membuf, Codec.majorVersion(), Codec.minorVersion());
                int ret = encRDMMsg.encode(encIter);
                assertEquals(CodecReturnCodes.SUCCESS, ret);

                dIter.setBufferAndRWFVersion(membuf, Codec.majorVersion(), Codec.minorVersion());
                ret = msg.decode(dIter);
                assertEquals(CodecReturnCodes.SUCCESS, ret);
                ret = decRDMMsg.decode(dIter, msg);
                assertEquals(CodecReturnCodes.SUCCESS, ret);
                assertEquals(streamId, decRDMMsg.streamId());
                assertEquals(flags, decRDMMsg.flags());

                /* Check parameters */
                assertEquals(filter, decRDMMsg.filter());
                if (encRDMMsg.checkHasServiceId())
                {
                    assertEquals(serviceId, decRDMMsg.serviceId());
                }
                if (encRDMMsg.checkSolicited())
                {
                    assertEquals(encRDMMsg.checkSolicited(), decRDMMsg.checkSolicited());
                }
                if (encRDMMsg.checkClearCache())
                {
                    assertEquals(encRDMMsg.checkClearCache(), decRDMMsg.checkClearCache());
                }
                if (encRDMMsg.checkHasSequenceNumber())
                {
                    assertEquals(seqNum, decRDMMsg.sequenceNumber());
                }

                if (encRDMMsg.checkHasSequenceNumber())
                {
                    assertEquals(seqNum, decRDMMsg.sequenceNumber());
                }

                if (encRDMMsg.serviceList().get(0).action() != MapEntryActions.DELETE)
                {
                    assertEquals(encRDMMsg.serviceList().size(), decRDMMsg.serviceList().size());
                    verifyRDMService(encRDMMsg.serviceList().get(0), decRDMMsg.serviceList().get(0));
                }

                State decState = decRDMMsg.state();
                assertNotNull(decState);
                assertEquals(state.code(), decState.code());
                assertEquals(state.dataState(), decState.dataState());
                assertEquals(state.streamState(), decState.streamState());
                assertEquals(state.text().toString(), decState.text().toString());
            }
        }
        System.out.println("Done.");
    }

    @Test
    public void directoryStatusTests()
    {
        DirectoryStatus encRDMMsg = (DirectoryStatus)DirectoryMsgFactory.createMsg();
        encRDMMsg.rdmMsgType(DirectoryMsgType.STATUS);
        DirectoryStatus decRDMMsg = (DirectoryStatus)DirectoryMsgFactory.createMsg();
        decRDMMsg.rdmMsgType(DirectoryMsgType.STATUS);

        System.out.println("DirectoryStatus tests...");

        int flagsBase[] =
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

        State state = CodecFactory.createState();
        Buffer buffer = CodecFactory.createBuffer();
        buffer.data("state");
        state.text(buffer);
        state.code(StateCodes.FAILOVER_COMPLETED);
        state.dataState(DataStates.SUSPECT);
        state.streamState(StreamStates.OPEN);

        int[] flagsList = TypedMessageTestUtil._createFlagCombinations(flagsBase, false);
        for (int i = 0; i < flagsList.length; i++)
        {
            dIter.clear();
            encIter.clear();
            encRDMMsg.clear();

            Buffer membuf = CodecFactory.createBuffer();
            membuf.data(ByteBuffer.allocate(1024));

            int flags = flagsList[i];
            encRDMMsg.flags(flags);
            encRDMMsg.streamId(streamId);

            if (encRDMMsg.checkHasState())
            {
                encRDMMsg.state().code(state.code());
                encRDMMsg.state().dataState(state.dataState());
                encRDMMsg.state().text().data("state");
                encRDMMsg.state().streamState(state.streamState());
            }

            if (encRDMMsg.checkHasServiceId())
                encRDMMsg.serviceId(serviceId);

            if (encRDMMsg.checkHasFilter())
                encRDMMsg.filter(filter);
            
            if(encRDMMsg.checkClearCache())
            {
            	encRDMMsg.applyClearCache();
            }

            encIter.setBufferAndRWFVersion(membuf, Codec.majorVersion(), Codec.minorVersion());
            int ret = encRDMMsg.encode(encIter);
            assertEquals(CodecReturnCodes.SUCCESS, ret);

            dIter.setBufferAndRWFVersion(membuf, Codec.majorVersion(), Codec.minorVersion());
            ret = msg.decode(dIter);
            assertEquals(CodecReturnCodes.SUCCESS, ret);
            ret = decRDMMsg.decode(dIter, msg);
            assertEquals(CodecReturnCodes.SUCCESS, ret);
            assertEquals(streamId, decRDMMsg.streamId());
            assertEquals(flags, decRDMMsg.flags());

            /* Check parameters */
            if (encRDMMsg.checkHasServiceId())
            {
                assertEquals(serviceId, decRDMMsg.serviceId());
            }

            if (encRDMMsg.checkHasFilter())
            {
                assertEquals(filter, decRDMMsg.filter());
            }

            if (decRDMMsg.checkHasState())
            {
                State decState = decRDMMsg.state();
                assertNotNull(decState);
                assertEquals(state.code(), decState.code());
                assertEquals(state.dataState(), decState.dataState());
                assertEquals(state.streamState(), decState.streamState());
                assertEquals(state.text().toString(), decState.text().toString());
            }
        }
        System.out.println("Done.");
    }

    @Test
    public void directoryStatusCopyTests()
    {
        DirectoryStatus statusRDMMsg1 = (DirectoryStatus)DirectoryMsgFactory.createMsg();
        statusRDMMsg1.rdmMsgType(DirectoryMsgType.STATUS);
        DirectoryStatus statusRDMMsg2 = (DirectoryStatus)DirectoryMsgFactory.createMsg();
        statusRDMMsg2.rdmMsgType(DirectoryMsgType.STATUS);

        System.out.println("DirectoryStatus copy tests...");

        /* Parameter setup */
        int streamId = -5;
        int serviceId = 1;
        int filter = 1;
        State state = CodecFactory.createState();
        Buffer buffer = CodecFactory.createBuffer();
        buffer.data("state");
        state.text(buffer);
        state.code(StateCodes.FAILOVER_COMPLETED);
        state.dataState(DataStates.SUSPECT);
        state.streamState(StreamStates.OPEN);

        int flags = DirectoryStatusFlags.HAS_FILTER | DirectoryStatusFlags.HAS_SERVICE_ID | DirectoryStatusFlags.HAS_STATE | DirectoryStatusFlags.CLEAR_CACHE;
        statusRDMMsg1.clear();
        statusRDMMsg1.rdmMsgType(DirectoryMsgType.STATUS);
        statusRDMMsg1.flags(flags);
        statusRDMMsg1.streamId(streamId);
        if (statusRDMMsg1.checkHasState())
        {
            statusRDMMsg1.state().code(state.code());
            statusRDMMsg1.state().dataState(state.dataState());
            statusRDMMsg1.state().text().data("state");
            statusRDMMsg1.state().streamState(state.streamState());
        }

        if (statusRDMMsg1.checkHasServiceId())
            statusRDMMsg1.serviceId(serviceId);

        if (statusRDMMsg1.checkHasFilter())
            statusRDMMsg1.filter(filter);
        
        if(statusRDMMsg1.checkClearCache())
        {
        	statusRDMMsg1.applyClearCache();
        }

        int ret = statusRDMMsg1.copy(statusRDMMsg2);
        assertEquals(CodecReturnCodes.SUCCESS, ret);
        assertEquals(streamId, statusRDMMsg2.streamId());
        assertEquals(serviceId, statusRDMMsg2.serviceId());
        assertEquals(filter, statusRDMMsg2.filter());
        assertEquals(flags, statusRDMMsg2.flags());
        assertEquals(statusRDMMsg1.checkClearCache(), statusRDMMsg2.checkClearCache());

        if (statusRDMMsg1.checkHasState())
        {
            State refState1 = statusRDMMsg1.state();
            State refState2 = statusRDMMsg2.state();
            assertNotNull(refState2);
            assertEquals(refState1.code(), refState2.code());
            assertEquals(refState1.dataState(), refState2.dataState());
            assertEquals(refState1.streamState(), refState2.streamState());
            assertEquals(refState1.text().toString(), refState2.text().toString());
            assertTrue(refState1.text() != refState2.text());
        }
        System.out.println("Done.");
    }

    @Test
    public void directoryStatusToStringTests()
    {
        DirectoryStatus statusRDMMsg1 = (DirectoryStatus)DirectoryMsgFactory.createMsg();
        statusRDMMsg1.rdmMsgType(DirectoryMsgType.STATUS);
        DirectoryStatus statusRDMMsg2 = (DirectoryStatus)DirectoryMsgFactory.createMsg();
        statusRDMMsg2.rdmMsgType(DirectoryMsgType.STATUS);

        System.out.println("DirectoryStatus toString tests...");

        /* Parameter setup */
        int streamId = -5;
        int serviceId = 1;
        int filter = 1;
        State state = CodecFactory.createState();
        Buffer buffer = CodecFactory.createBuffer();
        buffer.data("state");
        state.text(buffer);
        state.code(StateCodes.FAILOVER_COMPLETED);
        state.dataState(DataStates.SUSPECT);
        state.streamState(StreamStates.OPEN);

        int flags = DirectoryStatusFlags.HAS_FILTER | DirectoryStatusFlags.HAS_SERVICE_ID | DirectoryStatusFlags.HAS_STATE;
        statusRDMMsg1.clear();
        statusRDMMsg1.rdmMsgType(DirectoryMsgType.STATUS);
        statusRDMMsg1.flags(flags);
        statusRDMMsg1.streamId(streamId);
        if (statusRDMMsg1.checkHasState())
        {
            statusRDMMsg1.state().code(state.code());
            statusRDMMsg1.state().dataState(state.dataState());
            statusRDMMsg1.state().text().data("state");
            statusRDMMsg1.state().streamState(state.streamState());
        }

        if (statusRDMMsg1.checkHasServiceId())
            statusRDMMsg1.serviceId(serviceId);

        if (statusRDMMsg1.checkHasFilter())
            statusRDMMsg1.filter(filter);

        statusRDMMsg1.toString();

        System.out.println("Done.");
    }

    @Test
    public void directoryRequestTests()
    {
        DirectoryRequest encRDMMsg = (DirectoryRequest)DirectoryMsgFactory.createMsg();
        encRDMMsg.rdmMsgType(DirectoryMsgType.REQUEST);
        DirectoryRequest decRDMMsg = (DirectoryRequest)DirectoryMsgFactory.createMsg();
        decRDMMsg.rdmMsgType(DirectoryMsgType.REQUEST);

        System.out.println("DirectoryRequest tests...");

        /* Parameter setup */
        int streamId = -5;
        int serviceId = 1;
        int filter = 1;
        State state = CodecFactory.createState();
        Buffer buffer = CodecFactory.createBuffer();
        buffer.data("state");
        state.text(buffer);
        state.code(StateCodes.FAILOVER_COMPLETED);
        state.dataState(DataStates.SUSPECT);
        state.streamState(StreamStates.OPEN);

        int flagsBase[] =
        {
                DirectoryRequestFlags.HAS_SERVICE_ID,
                DirectoryRequestFlags.STREAMING
        };

        int[] flagsList = TypedMessageTestUtil._createFlagCombinations(flagsBase, false);

        System.out.println("DirectoryRequest tests...");

        for (int i = 0; i < flagsList.length; i++)
        {
            dIter.clear();
            encIter.clear();
            Buffer membuf = CodecFactory.createBuffer();
            membuf.data(ByteBuffer.allocate(1024));

            int flags = flagsList[i];

            encRDMMsg.clear();
            encRDMMsg.rdmMsgType(DirectoryMsgType.REQUEST);
            encRDMMsg.flags(flags);
            encRDMMsg.streamId(streamId);
            encRDMMsg.filter(filter);
            if (encRDMMsg.checkHasServiceId())
                encRDMMsg.serviceId(serviceId);

            encIter.setBufferAndRWFVersion(membuf, Codec.majorVersion(), Codec.minorVersion());
            int ret = encRDMMsg.encode(encIter);
            assertEquals(CodecReturnCodes.SUCCESS, ret);

            dIter.setBufferAndRWFVersion(membuf, Codec.majorVersion(),
                                         Codec.minorVersion());
            ret = msg.decode(dIter);
            assertEquals(CodecReturnCodes.SUCCESS, ret);
            ret = decRDMMsg.decode(dIter, msg);
            assertEquals(CodecReturnCodes.SUCCESS, ret);

            assertEquals(DirectoryMsgType.REQUEST, decRDMMsg.rdmMsgType());
            assertEquals(flags, decRDMMsg.flags());

            assertEquals(filter, decRDMMsg.filter());
            assertEquals(streamId, decRDMMsg.streamId());
            if (decRDMMsg.checkHasServiceId())
                assertEquals(serviceId, decRDMMsg.serviceId());
        }
        System.out.println("Done.");
    }

    @Test
    public void directoryRequestCopyTests()
    {
        DirectoryRequest reqMsg1 = (DirectoryRequest)DirectoryMsgFactory.createMsg();
        reqMsg1.rdmMsgType(DirectoryMsgType.REQUEST);

        DirectoryRequest reqMsg2 = (DirectoryRequest)DirectoryMsgFactory.createMsg();
        reqMsg2.rdmMsgType(DirectoryMsgType.REQUEST);

        System.out.println("DirectoryRequest copy tests...");

        /* Parameter setup */
        int streamId = -5;
        int serviceId = 1;
        int filter = 1;

        reqMsg1.clear();
        reqMsg1.applyStreaming();
        reqMsg1.rdmMsgType(DirectoryMsgType.REQUEST);
        reqMsg1.streamId(streamId);
        reqMsg1.filter(filter);
        reqMsg1.applyHasServiceId();
        reqMsg1.serviceId(serviceId);
        int ret = reqMsg1.copy(reqMsg2);
        assertEquals(CodecReturnCodes.SUCCESS, ret);

        assertEquals(DirectoryMsgType.REQUEST, reqMsg2.rdmMsgType());
        assertEquals(reqMsg1.flags(), reqMsg2.flags());
        assertEquals(reqMsg1.checkStreaming(), reqMsg2.checkStreaming());
        assertEquals(filter, reqMsg2.filter());
        assertEquals(streamId, reqMsg2.streamId());
        assertEquals(serviceId, reqMsg2.serviceId());
        System.out.println("Done.");
    }

    @Test
    public void directoryRequestToStringTests()
    {
        DirectoryRequest encRDMMsg = (DirectoryRequest)DirectoryMsgFactory.createMsg();
        encRDMMsg.rdmMsgType(DirectoryMsgType.REQUEST);

        System.out.println("DirectoryRequest toString tests...");

        /* Parameter setup */
        int streamId = -5;
        int serviceId = 1;
        int filter = Directory.ServiceFilterFlags.DATA | Directory.ServiceFilterFlags.GROUP | Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.LINK | Directory.ServiceFilterFlags.LOAD | Directory.ServiceFilterFlags.STATE;

        encRDMMsg.clear();
        encRDMMsg.rdmMsgType(DirectoryMsgType.REQUEST);
        encRDMMsg.streamId(streamId);
        encRDMMsg.filter(filter);
        encRDMMsg.applyHasServiceId();
        encRDMMsg.serviceId(serviceId);

        encRDMMsg.toString();

        System.out.println("Done.");
    }
}
