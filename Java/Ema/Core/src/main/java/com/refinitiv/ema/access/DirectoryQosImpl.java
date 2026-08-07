/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

import com.refinitiv.ema.domain.directory.DirectoryQos;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.Qos;

/**
 * Default implementation of {@link DirectoryQos}.
 * <p>
 * This mutable object stores the timeliness and rate values for a single RDM
 * Source Directory QoS entry and can translate them into an {@link OmmQos}
 * snapshot when directory service information is encoded.
 */
final class DirectoryQosImpl implements DirectoryQos
{
    private int timeliness;
    private int rate;

    /**
     * Creates a directory qos object initialized to the default real-time,
     * tick-by-tick QoS.
     */
    DirectoryQosImpl()
    {
        clear();
    }

    /**
     * Returns the configured timeliness value.
     * <p>
     * The value typically corresponds to
     * {@link OmmQos.Timeliness} constants, but may
     * also be an explicit timeliness value understood by EMA.
     *
     * @return timeliness value
     */
    @Override
    public int timeliness()
    {
        return timeliness;
    }

    /**
     * Sets the timeliness value.
     *
     * @param timeliness the timeliness value
     * @return this directory qos implementation instance
     */
    @Override
    public DirectoryQosImpl timeliness(int timeliness)
    {
        this.timeliness = timeliness;
        return this;
    }

    /**
     * Returns the configured rate value.
     * <p>
     * The value typically corresponds to
     * {@link OmmQos.Rate} constants, but may also be
     * an explicit rate value understood by EMA.
     *
     * @return rate value
     */
    @Override
    public int rate()
    {
        return rate;
    }

    /**
     * Sets the rate value.
     *
     * @param rate the rate value
     * @return this directory qos implementation instance
     */
    @Override
    public DirectoryQosImpl rate(int rate)
    {
        this.rate = rate;
        return this;
    }

    /**
     * Resets this object to the default real-time, tick-by-tick QoS.
     *
     * @return this directory qos implementation instance
     */
    @Override
    public DirectoryQosImpl clear()
    {
        timeliness = OmmQos.Timeliness.REALTIME;
        rate = OmmQos.Rate.TICK_BY_TICK;
        return this;
    }

    /**
     * Replaces the contents of this object with values from another directory
     * QoS instance.
     *
     * @param sourceQos source directory qos to copy from
     * @return this directory qos implementation instance
     * @throws OmmInvalidUsageException if {@code sourceQos} is {@code null}
     */
    @Override
    public DirectoryQosImpl copy(DirectoryQos sourceQos)
    {
        if (sourceQos == null)
        {
            throw new OmmInvalidUsageExceptionImpl().message("sourceQos can not be null",
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        if (sourceQos == this)
        {
            return this;
        }

        timeliness = sourceQos.timeliness();
        rate = sourceQos.rate();
        return this;
    }

    OmmQos ommQos()
    {
        Qos qos = CodecFactory.createQos();

        Utilities.toRsslQos(rate, timeliness, qos);

        OmmQosImpl ommQos = new OmmQosImpl();
        ommQos.decode(qos);

        return ommQos;
    }

    @Override
    public String toString()
    {
        return ommQos().toString();
    }
}


