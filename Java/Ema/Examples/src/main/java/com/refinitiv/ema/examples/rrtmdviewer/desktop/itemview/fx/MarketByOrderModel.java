/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.examples.rrtmdviewer.desktop.itemview.fx;

import java.util.Objects;

/**
 * Model for which represents data in {@link MarketByOrderComponent}
 */
public class MarketByOrderModel {

    private String id;

    private String quoteTime;

    private double size;

    private double price;

    private String modelType;

    public MarketByOrderModel(String id) {
        this.id = id;
    }

    public MarketByOrderModel(MarketByOrderModel copy) {
        this.id = copy.id;
        this.quoteTime = copy.quoteTime;
        this.size = copy.size;
        this.price = copy.price;
        this.modelType = copy.modelType;
    }

    public String getId() {
        return id;
    }

    public void setId(String id) {
        this.id = id;
    }

    public String getQuoteTime() {
        return quoteTime;
    }

    public void setQuoteTime(String quoteTime) {
        this.quoteTime = quoteTime;
    }

    public double getSize() {
        return size;
    }

    public void setSize(double size) {
        this.size = size;
    }

    public double getPrice() {
        return price;
    }

    public void setPrice(double price) {
        this.price = price;
    }

    public String getModelType() {
        return modelType;
    }

    public void setModelType(String modelType) {
        this.modelType = modelType;
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (!(o instanceof MarketByOrderModel)) return false;
        MarketByOrderModel that = (MarketByOrderModel) o;
        return Objects.equals(id, that.id) &&
                Objects.equals(quoteTime, that.quoteTime) &&
                Objects.equals(size, that.size) &&
                Objects.equals(price, that.price) &&
                Objects.equals(modelType, that.modelType);
    }

    @Override
    public int hashCode() {
        return Objects.hash(id, quoteTime, size, price, modelType);
    }
}
