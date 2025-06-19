/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.examples.rtviewer.desktop.itemview.fx;

import com.refinitiv.ema.access.DataType;
import com.refinitiv.ema.access.FieldEntry;
import com.refinitiv.ema.access.SummaryData;
import javafx.scene.input.MouseEvent;

import java.util.function.Consumer;

import static com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST;

public interface MarketByComponent {

    public static final int CURRENCY = 15; /*Currency*/
    public static final int TRD_UNITS = 53; /*UNITS*/
    public static final int RDN_EXCH = 1709; /*External ID*/
    public static final int OR_RNK_RUL= 3425; /* ORDER RANK RULE" */

    public void configureButton(String name, Consumer<MouseEvent> func);

    public void clear();

    public void setDisableBtn(boolean value);

    public static String decodeValueToStr(FieldEntry fieldEntry) {
        final String value;
        switch (fieldEntry.loadType()) {
            case DataType.DataTypes.REAL:
                value = String.valueOf(fieldEntry.real().asDouble());
                break;
            case DataType.DataTypes.INT:
                value = String.valueOf(fieldEntry.intValue());
                break;
            case DataType.DataTypes.UINT:
                value = String.valueOf(fieldEntry.uintValue());
                break;
            case DataType.DataTypes.ASCII:
                value = fieldEntry.ascii().toString();
                break;
            case DataType.DataTypes.ENUM:
                value = fieldEntry.hasEnumDisplay() ? fieldEntry.enumDisplay() : String.valueOf(fieldEntry.enumValue());
                break;
            case DataType.DataTypes.RMTES:
                value = fieldEntry.rmtes().toString();
                break;
            case DataType.DataTypes.ERROR:
                value = fieldEntry.error().errorCodeAsString();
                break;
            default:
                value = "";
        }
        return value;
    }

    public  static void decodeSummaryData(SummaryData summaryData, SummaryDataModel summaryDataModel) {
        if (FIELD_LIST == summaryData.dataType()) {
            for (FieldEntry fe : summaryData.fieldList()) {
                switch (fe.fieldId()) {
                    case CURRENCY:
                        summaryDataModel.setCurrency(MarketByComponent.decodeValueToStr(fe));
                        break;
                    case TRD_UNITS:
                        summaryDataModel.setUnits(MarketByComponent.decodeValueToStr(fe));
                        break;
                    case RDN_EXCH:
                        summaryDataModel.setExchId(MarketByComponent.decodeValueToStr(fe));
                        break;
                    case OR_RNK_RUL:
                        summaryDataModel.setRankRule(MarketByComponent.decodeValueToStr(fe).trim());
                        break;
                }
            }
        }
    }
}
