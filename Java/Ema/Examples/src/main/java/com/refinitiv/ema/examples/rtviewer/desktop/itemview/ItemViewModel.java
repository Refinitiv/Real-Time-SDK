/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.examples.rtviewer.desktop.itemview;

import com.refinitiv.eta.codec.CodecReturnCodes;

import java.util.List;

public class ItemViewModel {

    private String serviceName;

    private SupportedItemDomains domain;

    private boolean isBatch;
    private boolean hasView;

    private String RIC;
    private List<String> batchRICs;
    private List<Integer> marketPriceView;

    private boolean isSnapshot;

    private String error;

    public String getError() {
        return error;
    }

    public String getServiceName() {
        return serviceName;
    }

    public SupportedItemDomains getDomain() {
        return domain;
    }

    public boolean isBatch() {
        return isBatch;
    }

    public boolean hasView() {
        return hasView;
    }

    public String getRIC() {
        return RIC;
    }

    public List<String> getBatchRICs() {
        return batchRICs;
    }

    public List<Integer> getMarketPriceView() {
        return marketPriceView;
    }

    public boolean isSnapshot() {
        return isSnapshot;
    }

    private ItemViewModel(String serviceName, SupportedItemDomains domain, boolean isBatch,
                          boolean hasView, String RIC, List<String> batchRICs, List<Integer> view,
                          boolean isSnapshot) {
        this.serviceName = serviceName;
        this.domain = domain;
        this.isBatch = isBatch;
        this.hasView = hasView;
        this.RIC = RIC;
        this.batchRICs = batchRICs;
        this.marketPriceView = view;
        this.isSnapshot = isSnapshot;
    }

    public int validate() {

        return CodecReturnCodes.SUCCESS;
    }

    public static ItemViewModelBuilder builder() {
        return new ItemViewModelBuilder();
    }

    public static class ItemViewModelBuilder {

        private String serviceName;

        private SupportedItemDomains domain;

        private boolean isBatch;
        private boolean hasView;

        private String RIC;
        private List<String> batchRICs;
        private List<Integer> marketPriceView;

        private boolean isSnapshot;

        public ItemViewModelBuilder serviceName(String serviceName) {
            this.serviceName = serviceName;
            return this;
        }

        public ItemViewModelBuilder domain(SupportedItemDomains domain) {
            this.domain = domain;
            return this;
        }

        public ItemViewModelBuilder isBatch(boolean isBatch) {
            this.isBatch = isBatch;
            return this;
        }

        public ItemViewModelBuilder hasView(boolean hasView) {
            this.hasView = hasView;
            return this;
        }

        public ItemViewModelBuilder RIC(String RIC) {
            this.RIC = RIC;
            return this;
        }

        public ItemViewModelBuilder batchRICs(List<String> batchRICs) {
            this.batchRICs = batchRICs;
            return this;
        }

        public ItemViewModelBuilder marketPriceView(List<Integer> marketPriceView) {
            this.marketPriceView = marketPriceView;
            return this;
        }

        public ItemViewModelBuilder isSnapshot(boolean isSnapshot) {
            this.isSnapshot = isSnapshot;
            return this;
        }

        public ItemViewModel build() {
            return new ItemViewModel(serviceName, domain, isBatch, hasView, RIC, batchRICs, marketPriceView, isSnapshot);
        }
    }

    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();
        if (isBatch) {
            sb.append("Batch items: ");
            for (String item : getBatchRICs()) {
                sb.append(item + " ");
            }
            sb.append('\n');
        } else {
            sb.append("Requested item: ");
            sb.append(getRIC());
            sb.append('\n');
        }

        if (isSnapshot) {
            sb.append("Snapshot Request\n");
        }

        sb.append("Service name " + serviceName + "\n");
        sb.append("Domain type: " + domain.getDomainType() + "\n");

        return sb.toString();
    }
}
