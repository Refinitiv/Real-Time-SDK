/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.Qos;
import org.junit.Test;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotSame;
import static org.junit.Assert.assertSame;

public class DirectoryQosTest
{
    @Test
    public void givenNewDirectoryQos_whenConstructed_thenUseDefaultRealtimeTickByTick()
    {
        DirectoryQosImpl directoryQos = createDirectoryQos();

        assertEquals(OmmQos.Timeliness.REALTIME, directoryQos.timeliness());
        assertEquals(OmmQos.Rate.TICK_BY_TICK, directoryQos.rate());

        OmmQos ommQos = directoryQos.ommQos();
        assertEquals(OmmQos.Timeliness.REALTIME, ommQos.timeliness());
        assertEquals(OmmQos.Rate.TICK_BY_TICK, ommQos.rate());
    }

    @Test
    public void givenMutatedDirectoryQos_whenClear_thenRestoreDefaults()
    {
        DirectoryQosImpl directoryQos = createDirectoryQos()
                .rate(15)
                .timeliness(10);

        assertSame(directoryQos, directoryQos.clear());
        assertEquals(OmmQos.Timeliness.REALTIME, directoryQos.timeliness());
        assertEquals(OmmQos.Rate.TICK_BY_TICK, directoryQos.rate());

        OmmQos ommQos = directoryQos.ommQos();
        assertEquals(OmmQos.Timeliness.REALTIME, ommQos.timeliness());
        assertEquals(OmmQos.Rate.TICK_BY_TICK, ommQos.rate());
    }

    @Test
    public void givenOutOfRangeValues_whenCreateOmmQos_thenConversionMatchesUtilities()
    {
        DirectoryQosImpl directoryQos = createDirectoryQos()
                .rate(70000)
                .timeliness(70000);

        OmmQos actual = directoryQos.ommQos();
        OmmQos expected = createExpectedQos(70000, 70000);

        assertEquals(expected.rate(), actual.rate());
        assertEquals(expected.timeliness(), actual.timeliness());
        assertEquals(expected.rateAsString(), actual.rateAsString());
        assertEquals(expected.timelinessAsString(), actual.timelinessAsString());
    }

    @Test
    public void givenSpecialQosConstants_whenCreateOmmQos_thenMapToExpectedSpecialValues()
    {
        DirectoryQosImpl directoryQos = createDirectoryQos()
                .rate(OmmQos.Rate.JUST_IN_TIME_CONFLATED)
                .timeliness(OmmQos.Timeliness.INEXACT_DELAYED);

        OmmQos actual = directoryQos.ommQos();

        assertEquals(OmmQos.Rate.JUST_IN_TIME_CONFLATED, actual.rate());
        assertEquals(OmmQos.Timeliness.INEXACT_DELAYED, actual.timeliness());
        assertEquals("JustInTimeConflated", actual.rateAsString());
        assertEquals("InexactDelayed", actual.timelinessAsString());
    }

    @Test
    public void givenMutatedDirectoryQos_whenCreateOmmQosTwice_thenEachCallReturnsIndependentSnapshot()
    {
        DirectoryQosImpl directoryQos = createDirectoryQos()
                .rate(15)
                .timeliness(10);

        OmmQos firstSnapshot = directoryQos.ommQos();

        directoryQos.rate(OmmQos.Rate.TICK_BY_TICK)
                .timeliness(OmmQos.Timeliness.REALTIME);

        OmmQos secondSnapshot = directoryQos.ommQos();

        assertNotSame(firstSnapshot, secondSnapshot);
        assertEquals(15, firstSnapshot.rate());
        assertEquals(10, firstSnapshot.timeliness());
        assertEquals(OmmQos.Rate.TICK_BY_TICK, secondSnapshot.rate());
        assertEquals(OmmQos.Timeliness.REALTIME, secondSnapshot.timeliness());
    }

    @Test
    public void givenSourceDirectoryQos_whenCopy_thenCopyValuesAndReturnSameInstance()
    {
        DirectoryQosImpl source = createDirectoryQos()
                .rate(15)
                .timeliness(10);
        DirectoryQosImpl destination = createDirectoryQos();

        assertSame(destination, destination.copy(source));
        assertEquals(15, destination.rate());
        assertEquals(10, destination.timeliness());

        OmmQos copiedQos = destination.ommQos();
        assertEquals(15, copiedQos.rate());
        assertEquals(10, copiedQos.timeliness());
    }

    @Test
    public void givenDirectoryQos_whenCopyIntoSelf_thenPreserveValuesAndReturnSameInstance()
    {
        DirectoryQosImpl directoryQos = createDirectoryQos()
                .rate(15)
                .timeliness(10);

        assertSame(directoryQos, directoryQos.copy(directoryQos));
        assertEquals(15, directoryQos.rate());
        assertEquals(10, directoryQos.timeliness());
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenNullSource_whenCopy_thenThrowInvalidUsage()
    {
        createDirectoryQos().copy(null);
    }

    private OmmQos createExpectedQos(int rate, int timeliness)
    {
        Qos etaQos = CodecFactory.createQos();
        Utilities.toRsslQos(rate, timeliness, etaQos);

        OmmQosImpl qos = new OmmQosImpl();
        qos.decode(etaQos);

        return qos;
    }

    private DirectoryQosImpl createDirectoryQos()
    {
        return new DirectoryQosImpl();
    }
}

