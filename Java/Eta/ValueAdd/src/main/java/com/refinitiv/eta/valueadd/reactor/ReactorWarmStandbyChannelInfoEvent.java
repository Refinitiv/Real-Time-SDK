/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

import static com.refinitiv.eta.valueadd.reactor.ReactorWarmStandbyMode.LOGIN_BASED;
import static com.refinitiv.eta.valueadd.reactor.ReactorWarmStandbyMode.SERVICE_BASED;

/**
 * An event that contains the reactor warm standby channel information.
 *
 * @see ReactorEvent
 */
public abstract class ReactorWarmStandbyChannelInfoEvent extends ReactorEvent
{
    private final StringBuilder stringBuilder = new StringBuilder();
    public static final String EOL = System.lineSeparator();
    public static final String TAB = "\t";

    /**
     * The warm standby mode
     *
     * @return Value indicating warm standby modes
     *
     * @see ReactorWarmStandbyMode
     */
    public abstract int warmStandbyMode();

    protected StringBuilder buildStringBuilder()
    {
        stringBuilder.setLength(0);

        stringBuilder.append(TAB)
                .append("warmStandbyMode: ")
                .append(warmStandbyMode() == LOGIN_BASED ? "LOGIN_BASED" :
                        warmStandbyMode() == SERVICE_BASED ? "SERVICE_BASED" : "NONE")
                .append(EOL);

        return stringBuilder;
    }

    @Override
    public String toString()
    {
        return buildStringBuilder().toString();
    }
}
