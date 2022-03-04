/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.         	  --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.examples.rrtmdviewer.desktop.itemview;

import com.refinitiv.ema.access.*;
import com.refinitiv.ema.examples.rrtmdviewer.desktop.common.ApplicationSingletonContainer;
import com.refinitiv.ema.rdm.EmaRdm;

import java.util.Objects;

public class ItemViewServiceImpl implements ItemViewService {

    private static final int DEBUG_BUFFERED_LENGTH = 1024;

    private final ReqMsg reqMsg;

    private final OmmItemViewClient ommItemViewClient;

    private final OmmServiceDataClient serviceDataClient;

    public ItemViewServiceImpl() {
        this.reqMsg = EmaFactory.createReqMsg();
        this.ommItemViewClient = new OmmItemViewClient();
        this.serviceDataClient = new OmmServiceDataClient();
    }

    @Override
    public void loadServiceData(ServiceInfoResponseModel serviceInfoResponse) {
        final OmmConsumer ommConsumer = Objects.requireNonNull(ApplicationSingletonContainer.getBean(OmmConsumer.class));
        reqMsg.domainType(EmaRdm.MMT_DICTIONARY).name("RWFFld").filter(EmaRdm.DICTIONARY_NORMAL);
        ommConsumer.registerClient(reqMsg, serviceDataClient, serviceInfoResponse);
        reqMsg.clear().domainType(EmaRdm.MMT_DIRECTORY);
        ommConsumer.registerClient(reqMsg, serviceDataClient, serviceInfoResponse);
    }

    @Override
    public void sendItemRequest(ItemRequestModel request) {
        final OmmConsumer ommConsumer = Objects.requireNonNull(ApplicationSingletonContainer.getBean(OmmConsumer.class));
        ReqMsg reqMsg = EmaFactory.createReqMsg();
        reqMsg.serviceName(request.getView().getServiceName()).domainType(request.getView().getDomain().getDomainType()).interestAfterRefresh(!request.getView().isSnapshot());

        if (request.getView().isBatch() || request.getView().hasView()) {
            ElementList list = EmaFactory.createElementList();
            if (request.getView().hasView()) {

                OmmArray array = EmaFactory.createOmmArray();
                for (int fid : request.getView().getMarketPriceView()) {
                    array.add(EmaFactory.createOmmArrayEntry().intValue(fid));
                }
                list.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_VIEW_TYPE, 1));
                list.add(EmaFactory.createElementEntry().array(EmaRdm.ENAME_VIEW_DATA, array));
            }

            if (request.getView().isBatch()) {
                OmmArray array = EmaFactory.createOmmArray();

                for (String item : request.getView().getBatchRICs()) {
                    array.add(EmaFactory.createOmmArrayEntry().ascii(item));
                }
                list.add(EmaFactory.createElementEntry().array(EmaRdm.ENAME_BATCH_ITEM_LIST, array));
            } else {
                reqMsg.name(request.getView().getRIC());
            }

            reqMsg.payload(list);
        } else {
            reqMsg.name(request.getView().getRIC());
        }
        ommConsumer.registerClient(reqMsg, ommItemViewClient, request);
    }

    @Override
    public void unregister(long handle) {
        final OmmConsumer ommConsumer = Objects.requireNonNull(ApplicationSingletonContainer.getBean(OmmConsumer.class));
        ommConsumer.unregister(handle);
    }
}
