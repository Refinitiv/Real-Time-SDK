/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

import com.refinitiv.ema.domain.directory.*;
import com.refinitiv.ema.rdm.DataDictionary;
import com.refinitiv.ema.rdm.EmaRdm;
import com.refinitiv.ema.unittest.TestUtilities;
import com.refinitiv.eta.codec.Codec;
import org.junit.Test;

import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;
import java.util.Collections;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertSame;
import static org.junit.Assert.assertTrue;

public class DirectoryServiceFluentApiTest
{
    @Test
    public void givenInfoStateLoadAndData_whenMutated_thenAllMutatorsReturnSameInstance()
    {
        DirectoryServiceInfo info = EmaFactory.Domain.createDirectoryServiceInfo();
        assertSame(info, info.clear());
        assertSame(info, info.action(FilterEntry.FilterAction.UPDATE));
        assertSame(info, info.serviceName("SERVICE")
                .vendor("VENDOR")
                .isSource(true)
                .supportsQosRange(true)
                .supportsOutOfBandSnapshots(false)
                .acceptingConsumerStatus(true)
                .itemList("ITEM_LIST")
                .capabilitiesList(Collections.singletonList(6L))
                .dictionariesProvidedList(Collections.singletonList("RWFFld"))
                .dictionariesUsedList(Collections.singletonList("RWFEnum"))
                .qosList(Collections.emptyList()));
        DirectoryServiceInfo sourceInfo = EmaFactory.Domain.createDirectoryServiceInfo()
                .serviceName("SOURCE")
                .capabilitiesList(Collections.singletonList(1L));
        assertSame(info, info.copy(sourceInfo));
        assertSame(info, info.decode(decodeElementList(sourceInfo.encode())));

        DirectoryServiceState state = EmaFactory.Domain.createDirectoryServiceState();
        assertSame(state, state.clear());
        assertSame(state, state.action(FilterEntry.FilterAction.UPDATE));
        assertSame(state, state.serviceState(EmaRdm.SERVICE_DOWN)
                .acceptingRequests(EmaRdm.SERVICE_YES)
                .status(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "OK"));
        DirectoryServiceState sourceState = EmaFactory.Domain.createDirectoryServiceState()
                .serviceState(EmaRdm.SERVICE_UP)
                .status(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "SRC");
        assertSame(state, state.copy(sourceState));
        assertSame(state, state.decode(decodeElementList(sourceState.encode())));

        DirectoryServiceLoad load = EmaFactory.Domain.createDirectoryServiceLoad();
        assertSame(load, load.clear());
        assertSame(load, load.action(FilterEntry.FilterAction.UPDATE));
        assertSame(load, load.openLimit(5).openWindow(6).loadFactor(7));
        DirectoryServiceLoad sourceLoad = EmaFactory.Domain.createDirectoryServiceLoad()
                .openLimit(10)
                .openWindow(11)
                .loadFactor(12);
        assertSame(load, load.copy(sourceLoad));
        assertSame(load, load.decode(decodeElementList(sourceLoad.encode())));

        DirectoryServiceData data = EmaFactory.Domain.createDirectoryServiceData();
        assertSame(data, data.clear());
        assertSame(data, data.action(FilterEntry.FilterAction.UPDATE));
        assertSame(data, data.type(EmaRdm.DataTypes.HEADLINE).dataAsAscii("ASCII"));
        DirectoryServiceData sourceData = EmaFactory.Domain.createDirectoryServiceData()
                .type(EmaRdm.DataTypes.STATUS)
                .dataAsEnum(5);
        assertSame(data, data.copy(sourceData));
        assertSame(data, data.decode(decodeElementList(sourceData.encode())));
    }

    @Test
    public void givenGroupLinkAndLinkInfo_whenMutated_thenAllMutatorsReturnSameInstance()
    {
        ByteBuffer groupBuffer = ByteBuffer.wrap("GROUP".getBytes(StandardCharsets.US_ASCII));
        ByteBuffer mergedBuffer = ByteBuffer.wrap("MERGED".getBytes(StandardCharsets.US_ASCII));

        DirectoryServiceGroup group = EmaFactory.Domain.createDirectoryServiceGroup();
        assertSame(group, group.clear());
        assertSame(group, group.action(FilterEntry.FilterAction.UPDATE));
        assertSame(group, group.group(groupBuffer.duplicate())
                .mergedToGroup(mergedBuffer.duplicate())
                .status(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "GROUP"));
        DirectoryServiceGroup sourceGroup = EmaFactory.Domain.createDirectoryServiceGroup()
                .group(groupBuffer.duplicate())
                .status(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "SRC_GROUP");
        assertSame(group, group.copy(sourceGroup));
        assertSame(group, group.decode(decodeElementList(sourceGroup.encode())));

        DirectoryServiceLink link = EmaFactory.Domain.createDirectoryServiceLink();
        assertSame(link, link.clear());
        assertSame(link, link.action(MapEntry.MapAction.UPDATE));
        assertSame(link, link.name("LINK")
                .type(EmaRdm.SERVICE_LINK_INTERACTIVE)
                .linkState(EmaRdm.LinkStates.UP)
                .linkCode(EmaRdm.SERVICE_LINK_CODE_OK)
                .text("TEXT"));
        DirectoryServiceLink sourceLink = EmaFactory.Domain.createDirectoryServiceLink()
                .name("SRC_LINK")
                .linkState(EmaRdm.LinkStates.DOWN);
        assertSame(link, link.copy(sourceLink));
        assertSame(link, link.decode(decodeElementList(sourceLink.encode())));

        DirectoryServiceLinkInfo linkInfo = EmaFactory.Domain.createDirectoryServiceLinkInfo();
        assertSame(linkInfo, linkInfo.clear());
        assertSame(linkInfo, linkInfo.action(MapEntry.MapAction.UPDATE));
        assertSame(linkInfo, linkInfo.linkList(Collections.singletonList(sourceLink)));
        DirectoryServiceLinkInfo sourceLinkInfo = EmaFactory.Domain.createDirectoryServiceLinkInfo()
                .linkList(Collections.singletonList(EmaFactory.Domain.createDirectoryServiceLink()
                        .name("MAP_LINK")
                        .linkState(EmaRdm.LinkStates.UP)));
        assertSame(linkInfo, linkInfo.copy(sourceLinkInfo));
        assertSame(linkInfo, linkInfo.decode(decodeMap(sourceLinkInfo.encode())));
    }

    @Test
    public void givenDirectoryServiceAndConsumerStatusService_whenMutated_thenMutatorsReturnSameInstance()
    {
        DirectoryServiceInfo info = EmaFactory.Domain.createDirectoryServiceInfo()
                .serviceName("SERVICE")
                .capabilitiesList(Collections.singletonList(6L));
        DirectoryServiceState state = EmaFactory.Domain.createDirectoryServiceState()
                .serviceState(EmaRdm.SERVICE_UP);
        DirectoryServiceLoad load = EmaFactory.Domain.createDirectoryServiceLoad()
                .openLimit(1);
        DirectoryServiceData data = EmaFactory.Domain.createDirectoryServiceData()
                .type(EmaRdm.DataTypes.HEADLINE)
                .dataAsAscii("DATA");
        DirectoryServiceLink link = EmaFactory.Domain.createDirectoryServiceLink()
                .name("LINK")
                .linkState(EmaRdm.LinkStates.UP);
        DirectoryServiceLinkInfo linkInfo = EmaFactory.Domain.createDirectoryServiceLinkInfo()
                .linkList(Collections.singletonList(link));
        DirectoryServiceGroup group = EmaFactory.Domain.createDirectoryServiceGroup()
                .group(ByteBuffer.wrap("GRP".getBytes(StandardCharsets.US_ASCII)));

        DirectoryService service = EmaFactory.Domain.createDirectoryService();
        assertSame(service, service.clear());
        assertSame(service, service.serviceId(100)
                .action(MapEntry.MapAction.UPDATE)
                .info(info)
                .state(state)
                .load(load)
                .data(data)
                .link(linkInfo)
                .groupStateList(Collections.singletonList(group)));

        DirectoryService sourceService = EmaFactory.Domain.createDirectoryService()
                .serviceId(200)
                .action(MapEntry.MapAction.ADD)
                .info(info)
                .state(state)
                .load(load)
                .data(data)
                .link(linkInfo)
                .groupStateList(Collections.singletonList(group));
        assertSame(service, service.copy(sourceService));
        assertSame(service, service.decode(decodeFilterList(sourceService.encode())));
        assertEquals(200, service.serviceId());
        assertEquals(MapEntry.MapAction.ADD, service.action());
        assertTrue(service.checkHasInfo());

        DirectoryConsumerStatusService consumerStatusService = EmaFactory.Domain.createDirectoryConsumerStatusService();
        assertSame(consumerStatusService, consumerStatusService.clear());
        assertSame(consumerStatusService, consumerStatusService.action(MapEntry.MapAction.UPDATE)
                .serviceId(321)
                .sourceMirroringMode(EmaRdm.SourceMirroringMode.ACTIVE_WITH_STANDBY)
                .warmStandbyMode(EmaRdm.WarmStandbyDirectoryServiceTypes.ACTIVE));

        DirectoryConsumerStatusService sourceConsumerStatus = EmaFactory.Domain.createDirectoryConsumerStatusService()
                .serviceId(654)
                .sourceMirroringMode(EmaRdm.SourceMirroringMode.STANDBY);
        assertSame(consumerStatusService, consumerStatusService.copy(sourceConsumerStatus));
        assertSame(consumerStatusService,
                consumerStatusService.decode(decodeElementList(sourceConsumerStatus.encode())));
    }

    private ElementList decodeElementList(ElementList encoded)
    {
        DataDictionary dictionary = createDictionary();
        ElementList decoded = JUnitTestConnect.createElementList();
        JUnitTestConnect.setRsslData(decoded, encoded, Codec.majorVersion(), Codec.minorVersion(),
                ((DataDictionaryImpl) dictionary).rsslDataDictionary(), null);
        return decoded;
    }

    private Map decodeMap(Map encoded)
    {
        DataDictionary dictionary = createDictionary();
        Map decoded = JUnitTestConnect.createMap();
        JUnitTestConnect.setRsslData(decoded, encoded, Codec.majorVersion(), Codec.minorVersion(),
                ((DataDictionaryImpl) dictionary).rsslDataDictionary(), null);
        return decoded;
    }

    private FilterList decodeFilterList(FilterList encoded)
    {
        DataDictionary dictionary = createDictionary();
        FilterList decoded = JUnitTestConnect.createFilterList();
        JUnitTestConnect.setRsslData(decoded, encoded, Codec.majorVersion(), Codec.minorVersion(),
                ((DataDictionaryImpl) dictionary).rsslDataDictionary(), null);
        return decoded;
    }

    private DataDictionary createDictionary()
    {
        DataDictionary dictionary = EmaFactory.createDataDictionary();
        dictionary.loadFieldDictionary(TestUtilities.getFieldDictionaryFileName());
        dictionary.loadEnumTypeDictionary(TestUtilities.getEnumTableFileName());
        return dictionary;
    }
}

