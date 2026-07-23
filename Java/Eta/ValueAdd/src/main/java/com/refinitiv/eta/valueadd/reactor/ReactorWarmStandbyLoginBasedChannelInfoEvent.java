/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

import java.util.*;
import java.util.concurrent.locks.Lock;

import static com.refinitiv.eta.valueadd.reactor.ReactorWarmStandbyMode.LOGIN_BASED;

/**
 * Represents the Reactor warm standby login based channel information.
 *
 * @see ReactorWarmStandbyChannelInfoEvent
 */
public final class ReactorWarmStandbyLoginBasedChannelInfoEvent extends ReactorWarmStandbyChannelInfoEvent
{
    private ReactorWarmStandbyChannelDetails activeChannel;
    private final List<ReactorWarmStandbyChannelDetails> channelsList = new ArrayList<>();

    private ReactorWarmStandbyLoginBasedChannelInfoEvent()
    {
    }

    @Override
    public int warmStandbyMode()
    {
        return LOGIN_BASED;
    }

    /**
     * Returns an active channel
     *
     * @return ReactorWarmStandbyChannelDetails
     *
     * @see ReactorWarmStandbyChannelDetails
     */
    public ReactorWarmStandbyChannelDetails activeChannel()
    {
        return activeChannel;
    }

    void activeChannel(ReactorWarmStandbyChannelDetails activeChannel)
    {
        this.activeChannel = activeChannel;
    }

    /**
     * Returns a list of active and standby channels
     *
     * @return a list of ReactorWarmStandbyChannelDetails
     *
     * @see ReactorWarmStandbyChannelDetails
     */
    public List<ReactorWarmStandbyChannelDetails> channelsList()
    {
        return channelsList;
    }

    @Override
    public void clear()
    {
        super.clear();
        activeChannel = null;
        channelsList.clear();
    }

    @Override
    public String toString()
    {
        StringBuilder stringBuilder = super.buildStringBuilder();
        stringBuilder.insert(0, "ReactorWarmStandbyLoginBasedChannelInfoEvent: " + EOL);

        stringBuilder.append(TAB)
                .append("activeChannel: ")
                .append(activeChannel() == null ? "N/A" : activeChannel())
                .append(EOL);

        stringBuilder.append(TAB)
                .append("channelsList: ")
                .append(EOL);
        for (ReactorWarmStandbyChannelDetails channel : channelsList())
        {
            stringBuilder.append(TAB)
                    .append(TAB)
                    .append(channel == null ? "N/A" : channel)
                    .append(EOL);
        }
        stringBuilder.append(EOL);

        return stringBuilder.toString();
    }

    static ReactorWarmStandbyLoginBasedChannelInfoEvent create(ReactorChannel reactorChannel)
    {
        ReactorWarmStandbyLoginBasedChannelInfoEvent wsbChannelInfoEvent =
                new ReactorWarmStandbyLoginBasedChannelInfoEvent();

        if (reactorChannel == null || reactorChannel.warmStandByHandlerImpl == null)
        {
            return wsbChannelInfoEvent;
        }

        Lock lock = reactorChannel.warmStandByHandlerImpl.warmStandByHandlerLock();
        lock.lock();
        try
        {
            if (reactorChannel.warmStandByHandlerImpl.activeReactorChannel() != null)
            {
                wsbChannelInfoEvent.activeChannel(ReactorWarmStandbyChannelDetails.create(reactorChannel.warmStandByHandlerImpl
                        .activeReactorChannel().channel()));
            }

            for (ReactorChannel rc : reactorChannel.warmStandByHandlerImpl.channelList())
            {
                if (rc.channel() != null)
                {
                    wsbChannelInfoEvent.channelsList.add(ReactorWarmStandbyChannelDetails.create(rc.channel()));
                }
            }
        }
        finally
        {
            lock.unlock();
        }

        return wsbChannelInfoEvent;
    }

}
