/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.domain.directory;

import com.refinitiv.ema.access.OmmInvalidUsageException;

import java.util.List;

/**
 * Represents a quality-of-service entry used by the RDM Source Directory
 * service-info filter.
 * <p>
 * This mutable object stores QoS timeliness and rate values in the same form
 * used by {@link com.refinitiv.ema.access.OmmQos}. Instances are typically
 * created through
 * {@link com.refinitiv.ema.access.EmaFactory.Domain#createDirectoryQos()} and supplied
 * to {@link DirectoryServiceInfo#qosListFromDirectoryQos(List)}.
 */
public interface DirectoryQos
{
    /**
     * Returns the configured timeliness value.
     * <p>
     * The value typically comes from {@link com.refinitiv.ema.access.OmmQos.Timeliness}
     * but may also be an explicit timeliness value understood by EMA.
     *
     * @return timeliness value
     */
    int timeliness();

    /**
     * Sets the timeliness value.
     *
     * @param timeliness the timeliness value
     * @return this directory qos instance
     */
    DirectoryQos timeliness(int timeliness);

    /**
     * Returns the configured rate value.
     * <p>
     * The value typically comes from {@link com.refinitiv.ema.access.OmmQos.Rate}
     * but may also be an explicit rate value understood by EMA.
     *
     * @return rate value
     */
    int rate();

    /**
     * Sets the rate value.
     *
     * @param rate the rate value
     * @return this directory qos instance
     */
    DirectoryQos rate(int rate);

    /**
     * Resets this object to the default real-time, tick-by-tick QoS.
     *
     * @return this directory qos instance
     */
    DirectoryQos clear();

    /**
     * Replaces the contents of this object with values from another directory
     * QoS instance.
     *
     * @param sourceQos source directory qos to copy from
     * @return this directory qos instance
     * @throws OmmInvalidUsageException if {@code sourceQos} is {@code null}
     */
    DirectoryQos copy(DirectoryQos sourceQos);
}


