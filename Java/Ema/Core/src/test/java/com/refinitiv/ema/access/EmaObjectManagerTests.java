/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

import org.junit.Test;

import static org.junit.Assert.assertEquals;

public class EmaObjectManagerTests {

    @Test
    public void testEmaObjectManagerInitializationAndLimitChange()
    {
        EmaObjectManager manager = new EmaObjectManager();
        manager.initialize(10, 6, 7, 8, 9);

        assertEquals(10, manager._ommDatePool.size());
        assertEquals(10, manager._ommIntPool.size());
        assertEquals(10, manager._ommUIntPool.size());
        assertEquals(10, manager._ommFloatPool.size());
        assertEquals(10, manager._ommDoublePool.size());
        assertEquals(10, manager._ommBufferPool.size());
        assertEquals(10, manager._ommAsciiPool.size());
        assertEquals(10, manager._ommUtf8Pool.size());
        assertEquals(10, manager._ommRmtesPool.size());
        assertEquals(10, manager._ommRealPool.size());
        assertEquals(10, manager._ommDatePool.size());
        assertEquals(10, manager._ommTimePool.size());
        assertEquals(10, manager._ommQosPool.size());
        assertEquals(10, manager._ommDateTimePool.size());
        assertEquals(10, manager._ommStatePool.size());
        assertEquals(10, manager._ommEnumPool.size());
        assertEquals(10, manager._ommArrayPool.size());

        assertEquals(10, manager._ommDatePool.getLimit());
        assertEquals(10, manager._ommIntPool.getLimit());
        assertEquals(10, manager._ommUIntPool.getLimit());
        assertEquals(10, manager._ommFloatPool.getLimit());
        assertEquals(10, manager._ommDoublePool.getLimit());
        assertEquals(10, manager._ommBufferPool.getLimit());
        assertEquals(10, manager._ommAsciiPool.getLimit());
        assertEquals(10, manager._ommUtf8Pool.getLimit());
        assertEquals(10, manager._ommRmtesPool.getLimit());
        assertEquals(10, manager._ommRealPool.getLimit());
        assertEquals(10, manager._ommDatePool.getLimit());
        assertEquals(10, manager._ommTimePool.getLimit());
        assertEquals(10, manager._ommQosPool.getLimit());
        assertEquals(10, manager._ommDateTimePool.getLimit());
        assertEquals(10, manager._ommStatePool.getLimit());
        assertEquals(10, manager._ommEnumPool.getLimit());
        assertEquals(10, manager._ommArrayPool.getLimit());

        assertEquals(6, manager._filterListPool.size());
        assertEquals(6, manager._vectorPool.size());
        assertEquals(6, manager._mapPool.size());
        assertEquals(6, manager._noDataPool.size());
        assertEquals(6, manager._ommErrorPool.size());
        assertEquals(6, manager._seriesPool.size());
        assertEquals(6, manager._xmlPool.size());
        assertEquals(6, manager._jsonPool.size());
        assertEquals(6, manager._ansiPagePool.size());
        assertEquals(6, manager._opaquePool.size());
        assertEquals(6, manager._elementListPool.size());
        assertEquals(6, manager._fieldListPool.size());

        assertEquals(6, manager._filterListPool.getLimit());
        assertEquals(6, manager._vectorPool.getLimit());
        assertEquals(6, manager._mapPool.getLimit());
        assertEquals(6, manager._noDataPool.getLimit());
        assertEquals(6, manager._ommErrorPool.getLimit());
        assertEquals(6, manager._seriesPool.getLimit());
        assertEquals(6, manager._xmlPool.getLimit());
        assertEquals(6, manager._jsonPool.getLimit());
        assertEquals(6, manager._ansiPagePool.getLimit());
        assertEquals(6, manager._opaquePool.getLimit());
        assertEquals(6, manager._elementListPool.getLimit());
        assertEquals(6, manager._fieldListPool.getLimit());

        assertEquals(5 * 6, manager._fieldEntryPool.size());
        assertEquals(5 * 6, manager._mapEntryPool.size());
        assertEquals(5 * 6, manager._elementEntryPool.size());
        assertEquals(5 * 6, manager._seriesEntryPool.size());
        assertEquals(5 * 6, manager._filterEntryPool.size());
        assertEquals(5 * 6, manager._vectorEntryPool.size());

        assertEquals(5 * 6, manager._fieldEntryPool.getLimit());
        assertEquals(5 * 6, manager._mapEntryPool.getLimit());
        assertEquals(5 * 6, manager._elementEntryPool.getLimit());
        assertEquals(5 * 6, manager._seriesEntryPool.getLimit());
        assertEquals(5 * 6, manager._filterEntryPool.getLimit());
        assertEquals(5 * 6, manager._vectorEntryPool.getLimit());

        assertEquals(7, manager._reqMsgPool.size());
        assertEquals(7, manager._refreshMsgPool.size());
        assertEquals(7, manager._statusMsgPool.size());
        assertEquals(7, manager._updateMsgPool.size());
        assertEquals(7, manager._ackMsgPool.size());
        assertEquals(7, manager._postMsgPool.size());
        assertEquals(7, manager._genericMsgPool.size());

        assertEquals(7, manager._reqMsgPool.getLimit());
        assertEquals(7, manager._refreshMsgPool.getLimit());
        assertEquals(7, manager._statusMsgPool.getLimit());
        assertEquals(7, manager._updateMsgPool.getLimit());
        assertEquals(7, manager._ackMsgPool.getLimit());
        assertEquals(7, manager._postMsgPool.getLimit());
        assertEquals(7, manager._genericMsgPool.getLimit());

        assertEquals(8, manager._singleItemPool.getLimit());
        assertEquals(8, manager._batchItemPool.getLimit());
        assertEquals(8, manager._longObjectPool.getLimit());
        assertEquals(8, manager._intObjectPool.getLimit());
        assertEquals(8, manager._subItemPool.getLimit());
        assertEquals(8, manager._tunnelItemPool.getLimit());
        assertEquals(8, manager._timeoutEventPool.getLimit());

        assertEquals(9, manager._etaObjectsPoolsLimit);

        manager.setEtaObjectsPoolsLimit(30);
        assertEquals(30, manager._etaObjectsPoolsLimit);

        manager.setEtaObjectsPoolsLimit(0);
        assertEquals(0, manager._etaObjectsPoolsLimit);

        manager.setEtaObjectsPoolsLimit(2);
        assertEquals(2, manager._etaObjectsPoolsLimit);

        manager.setEmaObjectPoolLimit(1, DataType.DataTypes.INT);
        manager.setEmaObjectPoolLimit(2, DataType.DataTypes.UINT);
        manager.setEmaObjectPoolLimit(3, DataType.DataTypes.DATE);
        manager.setEmaObjectPoolLimit(4, DataType.DataTypes.TIME);
        manager.setEmaObjectPoolLimit(5, DataType.DataTypes.REAL);
        manager.setEmaObjectPoolLimit(8, DataType.DataTypes.FLOAT);
        manager.setEmaObjectPoolLimit(9, DataType.DataTypes.DOUBLE);
        manager.setEmaObjectPoolLimit(10, DataType.DataTypes.RMTES);
        manager.setEmaObjectPoolLimit(11, DataType.DataTypes.ASCII);
        manager.setEmaObjectPoolLimit(12, DataType.DataTypes.BUFFER);
        manager.setEmaObjectPoolLimit(13, DataType.DataTypes.ENUM);
        manager.setEmaObjectPoolLimit(14, DataType.DataTypes.ARRAY);
        manager.setEmaObjectPoolLimit(15, DataType.DataTypes.QOS);
        manager.setEmaObjectPoolLimit(16, DataType.DataTypes.STATE);
        manager.setEmaObjectPoolLimit(17, DataType.DataTypes.DATETIME);
        manager.setEmaObjectPoolLimit(18, DataType.DataTypes.UTF8);

        assertEquals(3, manager._ommDatePool.getLimit());
        assertEquals(1, manager._ommIntPool.getLimit());
        assertEquals(2, manager._ommUIntPool.getLimit());
        assertEquals(8, manager._ommFloatPool.getLimit());
        assertEquals(9, manager._ommDoublePool.getLimit());
        assertEquals(12, manager._ommBufferPool.getLimit());
        assertEquals(11, manager._ommAsciiPool.getLimit());
        assertEquals(18, manager._ommUtf8Pool.getLimit());
        assertEquals(10, manager._ommRmtesPool.getLimit());
        assertEquals(5, manager._ommRealPool.getLimit());
        assertEquals(4, manager._ommTimePool.getLimit());
        assertEquals(15, manager._ommQosPool.getLimit());
        assertEquals(17, manager._ommDateTimePool.getLimit());
        assertEquals(16, manager._ommStatePool.getLimit());
        assertEquals(13, manager._ommEnumPool.getLimit());
        assertEquals(14, manager._ommArrayPool.getLimit());

        manager.setDataTypePoolsLimit(17);

        assertEquals(17, manager._ommDatePool.getLimit());
        assertEquals(17, manager._ommIntPool.getLimit());
        assertEquals(17, manager._ommUIntPool.getLimit());
        assertEquals(17, manager._ommFloatPool.getLimit());
        assertEquals(17, manager._ommDoublePool.getLimit());
        assertEquals(17, manager._ommBufferPool.getLimit());
        assertEquals(17, manager._ommAsciiPool.getLimit());
        assertEquals(17, manager._ommUtf8Pool.getLimit());
        assertEquals(17, manager._ommRmtesPool.getLimit());
        assertEquals(17, manager._ommRealPool.getLimit());
        assertEquals(17, manager._ommTimePool.getLimit());
        assertEquals(17, manager._ommQosPool.getLimit());
        assertEquals(17, manager._ommDateTimePool.getLimit());
        assertEquals(17, manager._ommStatePool.getLimit());
        assertEquals(17, manager._ommEnumPool.getLimit());
        assertEquals(17, manager._ommArrayPool.getLimit());

        manager.setEmaObjectPoolLimit(1, DataType.DataTypes.FILTER_LIST);
        manager.setEmaObjectPoolLimit(2, DataType.DataTypes.FIELD_LIST);
        manager.setEmaObjectPoolLimit(3, DataType.DataTypes.ELEMENT_LIST);
        manager.setEmaObjectPoolLimit(4, DataType.DataTypes.ERROR);
        manager.setEmaObjectPoolLimit(5, DataType.DataTypes.MAP);
        manager.setEmaObjectPoolLimit(6, DataType.DataTypes.SERIES);
        manager.setEmaObjectPoolLimit(7, DataType.DataTypes.VECTOR);
        manager.setEmaObjectPoolLimit(8, DataType.DataTypes.JSON);
        manager.setEmaObjectPoolLimit(9, DataType.DataTypes.XML);
        manager.setEmaObjectPoolLimit(10, DataType.DataTypes.ANSI_PAGE);
        manager.setEmaObjectPoolLimit(11, DataType.DataTypes.OPAQUE);
        manager.setEmaObjectPoolLimit(12, DataType.DataTypes.NO_DATA);

        assertEquals(1, manager._filterListPool.getLimit());
        assertEquals(7, manager._vectorPool.getLimit());
        assertEquals(5, manager._mapPool.getLimit());
        assertEquals(12, manager._noDataPool.getLimit());
        assertEquals(4, manager._ommErrorPool.getLimit());
        assertEquals(6, manager._seriesPool.getLimit());
        assertEquals(9, manager._xmlPool.getLimit());
        assertEquals(8, manager._jsonPool.getLimit());
        assertEquals(10, manager._ansiPagePool.getLimit());
        assertEquals(11, manager._opaquePool.getLimit());
        assertEquals(3, manager._elementListPool.getLimit());
        assertEquals(2, manager._fieldListPool.getLimit());

        manager.setComplexTypePoolsLimit(20);

        assertEquals(20, manager._filterListPool.getLimit());
        assertEquals(20, manager._vectorPool.getLimit());
        assertEquals(20, manager._mapPool.getLimit());
        assertEquals(20, manager._noDataPool.getLimit());
        assertEquals(20, manager._ommErrorPool.getLimit());
        assertEquals(20, manager._seriesPool.getLimit());
        assertEquals(20, manager._xmlPool.getLimit());
        assertEquals(20, manager._jsonPool.getLimit());
        assertEquals(20, manager._ansiPagePool.getLimit());
        assertEquals(20, manager._opaquePool.getLimit());
        assertEquals(20, manager._elementListPool.getLimit());
        assertEquals(20, manager._fieldListPool.getLimit());

        manager.setSessionObjectPoolLimit(11);
        assertEquals(11, manager._sessionObjectsPoolLimit);
        assertEquals(11, manager._batchItemPool.getLimit());
        assertEquals(11, manager._singleItemPool.getLimit());

        manager.setEmaObjectPoolLimit(1, DataType.DataTypes.REQ_MSG);
        manager.setEmaObjectPoolLimit(2, DataType.DataTypes.REFRESH_MSG);
        manager.setEmaObjectPoolLimit(3, DataType.DataTypes.POST_MSG);
        manager.setEmaObjectPoolLimit(4, DataType.DataTypes.UPDATE_MSG);
        manager.setEmaObjectPoolLimit(5, DataType.DataTypes.ACK_MSG);
        manager.setEmaObjectPoolLimit(6, DataType.DataTypes.STATUS_MSG);
        manager.setEmaObjectPoolLimit(7, DataType.DataTypes.GENERIC_MSG);

        assertEquals(1, manager._reqMsgPool.getLimit());
        assertEquals(2, manager._refreshMsgPool.getLimit());
        assertEquals(3, manager._postMsgPool.getLimit());
        assertEquals(4, manager._updateMsgPool.getLimit());
        assertEquals(5, manager._ackMsgPool.getLimit());
        assertEquals(6, manager._statusMsgPool.getLimit());
        assertEquals(7, manager._genericMsgPool.getLimit());

        manager.setMsgTypePoolsLimit(-1);

        assertEquals(-1, manager._reqMsgPool.getLimit());
        assertEquals(-1, manager._refreshMsgPool.getLimit());
        assertEquals(-1, manager._postMsgPool.getLimit());
        assertEquals(-1, manager._updateMsgPool.getLimit());
        assertEquals(-1, manager._ackMsgPool.getLimit());
        assertEquals(-1, manager._statusMsgPool.getLimit());
        assertEquals(-1, manager._genericMsgPool.getLimit());
    }
}
