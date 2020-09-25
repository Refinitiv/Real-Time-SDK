package com.refinitiv.ema.unittest;

import com.refinitiv.ema.access.EmaFactory;
import com.refinitiv.ema.access.FieldList;
import com.refinitiv.ema.access.Msg;
import com.refinitiv.ema.access.OmmReal;

import static com.refinitiv.ema.unittest.TestUtilities.checkResult;

public class TestMsgUtilities {
    public static void compareMsgFields(Msg expected, Msg actual, String checkName) {
        String checkPrefix = checkName + ": Msg field compare: ";
        checkResult(expected.domainType() == actual.domainType(), checkPrefix + "domainType");
        checkResult(expected.streamId() == actual.streamId(), checkPrefix + "streamId");

        checkResult(expected.hasMsgKey() == actual.hasMsgKey(), checkPrefix + "hasMsgKey");

        checkResult(expected.hasName() == actual.hasName(), checkPrefix + "hasName");
        if(expected.hasName())
            checkResult(expected.name().equals(actual.name()), checkPrefix + "name");

        checkResult(expected.hasNameType() == actual.hasNameType(), checkPrefix + "hasNameType");
        if(expected.hasNameType())
            checkResult(expected.nameType() == actual.nameType(), checkPrefix + "nameType");

        checkResult(expected.hasServiceId() == actual.hasServiceId(), checkPrefix + "hasServiceId");
        if(expected.hasServiceId())
            checkResult(expected.serviceId() == actual.serviceId(), checkPrefix + "serviceId");

        checkResult(expected.hasFilter() == actual.hasFilter(), checkPrefix + "hasFilter");
        if(expected.hasFilter())
            checkResult(expected.filter() == actual.filter(), checkPrefix + "filter");

        checkResult(expected.hasId() == actual.hasId(), checkPrefix + "hasId");
        if(expected.hasId())
            checkResult(expected.id() == actual.id(), checkPrefix + "id");

        checkResult(expected.hasExtendedHeader() == actual.hasExtendedHeader(), checkPrefix + "hasExtendedHeader");
        if(expected.hasExtendedHeader())
            checkResult(expected.extendedHeader().equals(actual.extendedHeader()), checkPrefix + "extendedHeader");

    }

    public static FieldList createFiledListBodySample() {
        FieldList fieldList = EmaFactory.createFieldList();
        fieldList.add(EmaFactory.createFieldEntry().real(22, 3991, OmmReal.MagnitudeType.EXPONENT_NEG_2));
        fieldList.add(EmaFactory.createFieldEntry().real(30, 10, OmmReal.MagnitudeType.EXPONENT_0));

        return fieldList;
    }
}
