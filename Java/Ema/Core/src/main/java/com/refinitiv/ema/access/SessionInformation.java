/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

import java.util.ArrayList;
import java.util.List;

/**
 * Contains channel information for an EMA session.
 * <p>
 * A session may include standard channel connections and warm standby channel information.
 * This type is a mutable container populated by EMA and created through
 * {@link EmaFactory#createSessionInformation()}.
 * <p>
 * EMA may clear and repopulate the same instance across repeated calls, so applications should
 * treat the populated contents as a point-in-time view rather than a stable snapshot.
 * <p>
 * The lists returned by {@link #channelList()} and {@link #warmStandbyChannelList()} are owned by
 * this object. Applications should treat them as read-only views of the populated session state
 * and should not add, remove, or replace entries.
 * <p>
 * This class is not thread-safe.
 */
public final class SessionInformation
{
    private final List<ChannelInformation> channelList = new ArrayList<>();
    private final List<WarmStandbyChannelInformation> warmStandbyChannelList = new ArrayList<>();

    private final StringBuilder stringBuilder = new StringBuilder();
    private static final String EOL = System.lineSeparator();
    private static final String TAB = "\t";

    /**
     * Creates a {@code SessionInformation} instance.
     *
     */
    SessionInformation()
    {
    }

    /**
     * Returns the list of standard channel information entries for this session.
     * <p>
     * EMA clears and repopulates this list when the {@code SessionInformation} instance is reused.
     * Applications should treat the returned list as read-only and should not modify its contents.
     *
     * @return list of standard channel information entries for the current session state
     */
    public List<ChannelInformation> channelList()
    {
        return channelList;
    }

    /**
     * Returns the list of warm standby channel information entries for this session.
     * <p>
     * EMA clears and repopulates this list when the {@code SessionInformation} instance is reused.
     * Applications should treat the returned list as read-only and should not modify its contents.
     *
     * @return list of warm standby channel information entries for the current session state
     */
    public List<WarmStandbyChannelInformation> warmStandbyChannelList()
    {
        return warmStandbyChannelList;
    }

    /**
     * Clears all stored channel information from this session.
     */
    void clear()
    {
        channelList.clear();
        warmStandbyChannelList.clear();
    }

    /**
     * Returns a string representation of this session information.
     * <p>
     * Empty channel lists are represented as {@code N/A}.
     *
     * @return string representation of this session information
     */
    @Override
    public String toString()
    {
        stringBuilder.setLength(0);

        stringBuilder.append(EOL)
                .append("SessionInformation: ")
                .append(EOL);

        stringBuilder.append(TAB)
                .append("channelList: ")
                .append(EOL);
        if (channelList.isEmpty())
        {
            stringBuilder.append(TAB)
                    .append(TAB)
                    .append("N/A")
                    .append(EOL);
        }
        else
        {
            for (ChannelInformation ci : channelList)
            {
                stringBuilder.append(TAB)
                        .append(ci == null ? TAB + "N/A" : EOL + ci + EOL);
            }
        }
        stringBuilder.append(EOL);

        stringBuilder.append(TAB)
                .append("warmStandbyChannelList: ")
                .append(EOL);
        if (warmStandbyChannelList.isEmpty())
        {
            stringBuilder.append(TAB)
                    .append(TAB)
                    .append("N/A")
                    .append(EOL);
        }
        else
        {
            for (WarmStandbyChannelInformation wci : warmStandbyChannelList)
            {
                stringBuilder.append(TAB)
                        .append(wci == null ? TAB + "N/A" : wci);
            }
        }
        stringBuilder.append(EOL);

        return stringBuilder.toString();
    }
}
