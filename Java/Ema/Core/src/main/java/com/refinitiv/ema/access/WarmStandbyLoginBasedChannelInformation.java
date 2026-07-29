/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;


import com.refinitiv.eta.valueadd.reactor.ReactorWarmStandbyChannelDetails;
import com.refinitiv.eta.valueadd.reactor.ReactorWarmStandbyLoginBasedChannelInfoEvent;

import java.util.ArrayList;
import java.util.List;

/**
 * Represents warm standby session information for a login-based warm standby consumer channel.
 * <p>
 * EMA populates this type from the underlying ETA warm standby session state when
 * {@link OmmConsumer#getWarmStandbyChannelInformation()} or
 * {@link OmmConsumer#getWarmStandbyChannelInformation(java.util.List)} is called for a consumer
 * configured with {@link WarmStandbyMode#LOGIN_BASED}.
 * <p>
 * This class exposes the list of channels participating in the login-based warm standby group and,
 * when available, the zero-based index of the currently active channel within that list.
 *
 * @see WarmStandbyChannelInformation
 * @see WarmStandbyChannelDetails
 * @see OmmConsumer#getWarmStandbyChannelInformation()
 * @see OmmConsumer#getWarmStandbyChannelInformation(java.util.List)
 */
public final class WarmStandbyLoginBasedChannelInformation extends WarmStandbyChannelInformation
{
    private static final String ACTIVE_CHANNEL_MARKER = formatChannelMarker("active:");
    private static final String STANDBY_CHANNEL_MARKER = formatChannelMarker("standby:");

    private int activeChannelIndex = -1;
    private final List<WarmStandbyChannelDetails> channelsList = new ArrayList<>();

    private WarmStandbyLoginBasedChannelInformation()
    {
    }

    /**
     * Returns the zero-based index of the currently active warm standby channel.
     * <p>
     * The returned index corresponds to an entry in {@link #channelsList()} when EMA can match the
     * active channel reported by the underlying transport to one of the returned channel details.
     *
     * @return zero-based index of the active channel, or {@code -1} if no active channel is
     *         available or no matching entry exists in {@link #channelsList()}
     */
    public int activeChannelIndex()
    {
        return activeChannelIndex;
    }

    /**
     * Returns the warm standby channels associated with this login-based session.
     * <p>
     * The returned list may be empty when the underlying transport does not
     * report any channel details.
     *
     * @return list of warm standby channel details
     *
     * @see WarmStandbyChannelDetails
     */
    public List<WarmStandbyChannelDetails> channelsList()
    {
        return channelsList;
    }

    /**
     * Returns the warm standby mode associated with this information instance.
     *
     * @return {@link WarmStandbyMode#LOGIN_BASED}
     */
    @Override
    public int warmStandbyMode()
    {
        return WarmStandbyMode.LOGIN_BASED;
    }

    /**
     * Returns a human-readable representation of the login-based warm standby session information.
     * <p>
     * The returned string includes the warm standby mode, warm standby group name, and the reported
     * channel list. Each channel entry is prefixed with a state marker. Active channels use
     * {@code "active:"}; standby channels use {@code "standby:"}.
     *
     * @return formatted login-based warm standby information string
     */
    @Override
    public String toString()
    {
        StringBuilder stringBuilder = super.buildStringBuilder();
        stringBuilder.insert(0, "WarmStandbyLoginBasedChannelInformation: " + EOL);

        stringBuilder.append(TAB)
                .append("channelsList: ")
                .append(EOL);
        if (channelsList.isEmpty())
        {
            stringBuilder.append(TAB)
                    .append(TAB)
                    .append("N/A")
                    .append(EOL);
        }
        else
        {
            for (int index = 0; index < channelsList.size(); index++)
            {
                WarmStandbyChannelDetails channel = channelsList.get(index);
                stringBuilder.append(TAB)
                        .append(TAB)
                        .append(index == activeChannelIndex() ? ACTIVE_CHANNEL_MARKER : STANDBY_CHANNEL_MARKER)
                        .append(channel == null ? "N/A" : channel)
                        .append(EOL);
            }
        }
        stringBuilder.append(EOL);

        return stringBuilder.toString();
    }

    private static String formatChannelMarker(String channelMarker)
    {
        return String.format("%8s ", channelMarker);
    }

    static WarmStandbyLoginBasedChannelInformation create(ReactorWarmStandbyLoginBasedChannelInfoEvent event)
    {
        if (event == null)
        {
            throw new OmmInvalidUsageExceptionImpl().message("The event cannot be null.",
                    OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
        }

        WarmStandbyLoginBasedChannelInformation loginBasedInfo = new WarmStandbyLoginBasedChannelInformation();
        if (event.channelsList() != null && !event.channelsList().isEmpty())
        {
            for (ReactorWarmStandbyChannelDetails channelDetails : event.channelsList())
            {
                if (channelDetails == null)
                {
                    continue;
                }
                WarmStandbyChannelDetails channel = WarmStandbyChannelDetails.create(channelDetails);
                if (channelDetails.equals(event.activeChannel()))
                {
                    loginBasedInfo.activeChannelIndex = loginBasedInfo.channelsList.size();
                }
                loginBasedInfo.channelsList.add(channel);
            }
        }

        return loginBasedInfo;
    }
}
