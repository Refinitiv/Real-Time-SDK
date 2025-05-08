/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022, 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.examples.rtviewer.desktop.itemview;

import com.refinitiv.ema.access.*;
import com.refinitiv.ema.rdm.DataDictionary;
import com.refinitiv.ema.rdm.DictionaryEntry;
import com.refinitiv.ema.rdm.EmaRdm;
import com.refinitiv.eta.codec.MapEntryActions;
import com.refinitiv.eta.rdm.DomainTypes;

import java.util.*;
import java.util.stream.Collectors;

public class OmmServiceDataClient implements OmmConsumerClient {

    private DataDictionary dataDictionary = EmaFactory.createDataDictionary();

    private boolean dictionaryComplete;

    @Override
    public void onRefreshMsg(RefreshMsg refreshMsg, OmmConsumerEvent event) {
        ServiceInfoResponseModel serviceInfoResponse = (ServiceInfoResponseModel) event.closure();
        if (refreshMsg.domainType() == DomainTypes.DICTIONARY) {
            decodeFidsFromDictionary(refreshMsg, serviceInfoResponse);
        } else if (refreshMsg.domainType() == DomainTypes.SOURCE) {
            decodeDirectoryRefreshMsg(refreshMsg, serviceInfoResponse);
        }

        if (dictionaryComplete) {
            serviceInfoResponse.dictionaryLoadedProperty().set(true);
        }
    }

    @Override
    public void onUpdateMsg(UpdateMsg updateMsg, OmmConsumerEvent event) {
        if (updateMsg.domainType() == DomainTypes.SOURCE) {
            ServiceInfoResponseModel serviceInfoResponse = (ServiceInfoResponseModel) event.closure();
            decodeDirectoryUpdateMsg(updateMsg, serviceInfoResponse);
        }
    }

    @Override
    public void onStatusMsg(StatusMsg statusMsg, OmmConsumerEvent event) {
    }

    @Override
    public void onGenericMsg(GenericMsg genericMsg, OmmConsumerEvent consumerEvent) {

    }

    @Override
    public void onAckMsg(AckMsg ackMsg, OmmConsumerEvent consumerEvent) {

    }

    @Override
    public void onAllMsg(Msg msg, OmmConsumerEvent consumerEvent) {

    }

    private void decodeFidsFromDictionary(RefreshMsg msg, ServiceInfoResponseModel closure) {
        if (msg.payload().dataType() == DataType.DataTypes.SERIES) {
            dataDictionary.decodeFieldDictionary(msg.payload().series(), EmaRdm.DICTIONARY_NORMAL);
            if (msg.complete()) {
                final java.util.Map<String, Integer> dictionaryFids = dataDictionary.entries().stream()
                        .collect(Collectors.toMap(DictionaryEntry::acronym, DictionaryEntry::fid));
                closure.setFids(dictionaryFids);
                dictionaryComplete = true;
            }
        }
    }

    private void decodeDirectoryRefreshMsg(RefreshMsg msg, ServiceInfoResponseModel serviceInfoResponse) {
        decodeServiceInfoFromDirectory(msg, serviceInfoResponse);
        if (msg.complete()) {
            serviceInfoResponse.sourceRefreshCompletedProperty().set(!serviceInfoResponse.sourceRefreshCompletedPropertyValue());
        }
    }

    private void decodeDirectoryUpdateMsg(UpdateMsg msg, ServiceInfoResponseModel serviceInfoResponse) {
        decodeServiceInfoFromDirectory(msg, serviceInfoResponse);
        serviceInfoResponse.sourceRefreshCompletedProperty().set(!serviceInfoResponse.sourceRefreshCompletedPropertyValue());
    }

    private void decodeServiceInfoFromDirectory(Msg msg, ServiceInfoResponseModel serviceInfoResponse) {
        if (DataType.DataTypes.MAP == msg.payload().dataType()) {

            serviceInfoResponse.acquireServiceInfoLock();

            java.util.Map<Long, ServiceInfoModel> serviceInfoMap = serviceInfoResponse.getServiceInfoMap();
            for (MapEntry mapEntry : msg.payload().map()) {
                if (mapEntry.action() == MapEntryActions.ADD || mapEntry.action() == MapEntryActions.UPDATE) {
                    if (!serviceInfoMap.containsKey(mapEntry.key().uintValue())) {
                        serviceInfoMap.put(mapEntry.key().uintValue(), new ServiceInfoModel());
                    }

                    final ServiceInfoModel serviceInfo = serviceInfoMap.get(mapEntry.key().uintValue());
                    if (mapEntry.loadType() == DataType.DataTypes.FILTER_LIST) {
                        mapEntry.filterList().stream()
                                .filter(fe -> fe.loadType() == DataType.DataTypes.ELEMENT_LIST)
                                .map(FilterEntry::elementList)
                                .flatMap(Collection::stream)
                                .forEach(e -> this.handleDirectoryElement(e, serviceInfo));
                    }
                } else {
                    serviceInfoMap.remove(mapEntry.key().uintValue());
                }
            }

            serviceInfoResponse.releaseServiceInfoLock();
        }
    }

    private void handleDirectoryElement(ElementEntry e, ServiceInfoModel serviceInfo) {
        if (EmaRdm.ENAME_NAME.equals(e.name())) {
            serviceInfo.setServiceName(e.ascii().toString());
        }
        if (EmaRdm.ENAME_CAPABILITIES.equals(e.name())) {
            serviceInfo.getSupportedDomains().clear();
            for (OmmArrayEntry arrayEntry : e.array()) {
                final int capValue = (int) arrayEntry.uintValue();
                if (SupportedItemDomains.isSupported(capValue)) {
                    serviceInfo.getSupportedDomains().add(SupportedItemDomains.getByDomainType(capValue));
                }
            }
        }
        if (EmaRdm.ENAME_SVC_STATE.equals(e.name())) {
            serviceInfo.setServiceUp(e.uintValue() == 1);
        }
        if (EmaRdm.ENAME_ACCEPTING_REQS.equals(e.name())) {
            serviceInfo.setAcceptingRequests(e.uintValue() == 1);
        }
    }
}
