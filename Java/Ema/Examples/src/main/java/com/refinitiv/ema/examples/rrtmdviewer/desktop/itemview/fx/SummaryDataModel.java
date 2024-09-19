/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.examples.rrtmdviewer.desktop.itemview.fx;

import javafx.beans.property.SimpleStringProperty;
import javafx.beans.property.StringProperty;

public class SummaryDataModel {
    private final StringProperty currency = new SimpleStringProperty();

    private final StringProperty units = new SimpleStringProperty();

    private final StringProperty exchId = new SimpleStringProperty();

    private final StringProperty rankRule = new SimpleStringProperty();

    public String getCurrency() {
        return currency.get();
    }

    public StringProperty currencyProperty() {
        return currency;
    }

    public void setCurrency(String currency) {
        this.currency.set(currency);
    }

    public String getUnits() {
        return units.get();
    }

    public StringProperty unitsProperty() {
        return units;
    }

    public void setUnits(String units) {
        this.units.set(units);
    }

    public String getExchId() {
        return exchId.get();
    }

    public StringProperty exchIdProperty() {
        return exchId;
    }

    public void setExchId(String exchId) {
        this.exchId.set(exchId);
    }

    public String getRankRule() {
        return rankRule.get();
    }

    public StringProperty rankRuleProperty() {
        return rankRule;
    }

    public void setRankRule(String rankRule) {
        this.rankRule.set(rankRule);
    }

    public void clear() {
        currency.set("");
        units.set("");
        exchId.set("");
        rankRule.set("");
    }
}
