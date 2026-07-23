/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;


/**
 * Represents warm standby session information associated with an EMA consumer channel.
 * <p>
 * EMA populates this type from the underlying ETA warm standby session state and returns
 * one of its concrete subclasses from {@link OmmConsumer#getWarmStandbyChannelInformation()} or
 * {@link OmmConsumer#getWarmStandbyChannelInformation(java.util.List)}.
 * The concrete subtype depends on the configured warm standby mode:
 * <ul>
 *     <li>{@link WarmStandbyLoginBasedChannelInformation} for {@link WarmStandbyMode#LOGIN_BASED}</li>
 *     <li>{@link WarmStandbyServiceBasedChannelInformation} for {@link WarmStandbyMode#SERVICE_BASED}</li>
 * </ul>
 * <p>
 * This base class exposes information common to all warm standby modes, such as the warm standby
 * group name and the active warm standby mode.
 *
 * @see OmmConsumer#getWarmStandbyChannelInformation()
 * @see OmmConsumer#getWarmStandbyChannelInformation(java.util.List)
 * @see WarmStandbyLoginBasedChannelInformation
 * @see WarmStandbyServiceBasedChannelInformation
 */
public abstract class WarmStandbyChannelInformation
{
    private String warmStandbyGroupName;
    private String sessionChannelName;
    private final StringBuilder stringBuilder = new StringBuilder();

    static final String EOL = System.lineSeparator();

    static final String TAB = "\t";

    /**
     * Returns the configured warm standby group name for the channel.
     * <p>
     * The value identifies the warm standby group configured for the session channel and is
     * populated when the underlying transport provides the information.
     *
     * @return configured warm standby group name, or {@code null} if unavailable
     */
    public String warmStandbyGroupName()
    {
        return warmStandbyGroupName;
    }

    void warmStandbyGroupName(String warmStandbyGroupName)
    {
        this.warmStandbyGroupName = warmStandbyGroupName;
    }

    /**
     * Returns the configured session channel name for the channel.
     *
     * @return configured session channel name, or {@code null} if unavailable
     */
    public String sessionChannelName()
    {
        return sessionChannelName;
    }

    void sessionChannelName(String sessionChannelName)
    {
        this.sessionChannelName = sessionChannelName;
    }

    /**
     * Returns the warm standby mode associated with the channel.
     *
     * @return one of the {@link WarmStandbyMode} int constants
     */
    public abstract int warmStandbyMode();

    /**
     * Builds the common string representation shared by all warm standby information types.
     * <p>
     * Subclasses typically append their own mode-specific fields to the returned builder when
     * implementing {@link #toString()}.
     *
     * @return string builder containing the common warm standby fields
     */
    protected StringBuilder buildStringBuilder()
    {
        stringBuilder.setLength(0);

        stringBuilder.append(TAB)
                .append("warmStandbyMode: ")
                .append(WarmStandbyMode.toString(warmStandbyMode()))
                .append(EOL);

        stringBuilder.append(TAB)
                .append("warmStandbyGroupName: ")
                .append(warmStandbyGroupName() == null ? "N/A" : warmStandbyGroupName())
                .append(EOL);

        stringBuilder.append(TAB)
                .append("sessionChannelName: ")
                .append(sessionChannelName() == null ? "N/A" : sessionChannelName())
                .append(EOL);

        return stringBuilder;
    }

    /**
     * Returns a human-readable representation of the common warm standby information.
     * <p>
     * Concrete subclasses extend this output with additional mode-specific details.
     *
     * @return formatted warm standby information string
     */
    @Override
    public String toString()
    {
        return buildStringBuilder().toString();
    }

    /**
     * Defines the warm standby mode used by EMA.
     */
    public static class WarmStandbyMode
    {
        private WarmStandbyMode()
        {
            throw new AssertionError();
        }

        /**
         * No warm standby mode is specified.
         */
        public static final int NONE = 0;

        /**
         * Login-based warm standby mode.
         */
        public static final int LOGIN_BASED = 1;

        /**
         * Service-based warm standby mode.
         */
        public static final int SERVICE_BASED = 2;

        /**
         * Returns the name of the given warm standby mode constant.
         *
         * @param mode one of the {@link WarmStandbyMode} int constants
         * @return the name string, e.g. {@code "LOGIN_BASED"}, or {@code "UNKNOWN(n)"} if unrecognized
         */
        public static String toString(int mode)
        {
            switch (mode)
            {
                case NONE:         return "NONE";
                case LOGIN_BASED:  return "LOGIN_BASED";
                case SERVICE_BASED: return "SERVICE_BASED";
                default:           return "UNKNOWN(" + mode + ")";
            }
        }
    }
}
