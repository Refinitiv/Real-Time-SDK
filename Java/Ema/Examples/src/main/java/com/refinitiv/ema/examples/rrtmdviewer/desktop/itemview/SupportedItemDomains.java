package com.refinitiv.ema.examples.rrtmdviewer.desktop.itemview;

import com.refinitiv.eta.rdm.DomainTypes;

import java.util.HashMap;
import java.util.Map;

public enum SupportedItemDomains {

    MARKET_PRICE(DomainTypes.MARKET_PRICE, "MARKET PRICE"),
    MARKET_BY_PRICE(DomainTypes.MARKET_BY_PRICE, "MARKET BY PRICE"),
    MARKET_BY_ORDER(DomainTypes.MARKET_BY_ORDER, "MARKET BY ORDER");

    private static final Map<Integer, SupportedItemDomains> ITEM_DOMAIN_TYPE_RELATION = new HashMap<>();

    static {
        for (SupportedItemDomains domain : values()) {
            ITEM_DOMAIN_TYPE_RELATION.put(domain.getDomainType(), domain);
        }
    }

    private int domainType;
    private String textLabel;

    SupportedItemDomains(int domainType, String textLabel) {
        this.domainType = domainType;
        this.textLabel = textLabel;
    }

    public int getDomainType() {
        return domainType;
    }

    public String getTextLabel() {
        return textLabel;
    }

    @Override
    public String toString() {
        return textLabel;
    }

    public static boolean isSupported(int domainType) {
        return ITEM_DOMAIN_TYPE_RELATION.containsKey(domainType);
    }

    public static SupportedItemDomains getByDomainType(int domainType) {
        return ITEM_DOMAIN_TYPE_RELATION.get(domainType);
    }
}
