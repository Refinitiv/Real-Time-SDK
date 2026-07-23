/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

import static com.refinitiv.ema.access.WarmStandbyChannelInformation.*;

/**
 * Information about a warm standby change event.
 * <p>
 * This interface provides details about the warm standby mode, the channels involved,
 * the service name, service ID, warm standby group name, and session channel name.
 * </p>
 */
public interface WarmStandbyChangeEventInfo
{
    /**
     * Warm standby mode for this change event.
     *
     * @return one of the {@link WarmStandbyMode} int constants indicating the warm standby mode.
     */
    int warmStandbyMode();

    /**
     * The previous channel name.
     *
     * @return the previous channel name, or {@code null} if not available.
     */
    String previousChannelName();

    /**
     * The current channel name.
     *
     * @return the current channel name, or {@code null} if not available.
     */
    String currentChannelName();

    /**
     * The service name.
     * <p>
     * This value is applicable only when {@link #warmStandbyMode()} is {@link WarmStandbyMode#SERVICE_BASED}.
     * </p>
     *
     * @return the service name, or {@code null} when not applicable or not available.
     */
    String serviceName();

    /**
     * The service ID.
     * <p>
     * This value is applicable only when {@link #warmStandbyMode()} is {@link WarmStandbyMode#SERVICE_BASED}.
     * </p>
     *
     * @return the service ID, or {@code -1} when not applicable or not available.
     */
    int serviceId();

    /**
     * The warm standby group name.
     *
     * @return the warm standby group name, or {@code null} if not available.
     */
    String wsbGroupName();

    /**
     * The session channel name for request routing.
     *
     * @return the session channel name, or {@code null} if not available.
     */
    String sessionChannelName();
}
