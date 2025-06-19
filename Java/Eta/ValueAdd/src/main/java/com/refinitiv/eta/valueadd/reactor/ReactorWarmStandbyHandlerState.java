/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

/**
 * Enumerated types indicating warm standby handler channel state. See {@link ReactorWarmStandbyHandler}.
 */
class ReactorWarmStandbyHandlerState {
    // ReactorWarmStandByHandlerState class cannot be instantiated
    private ReactorWarmStandbyHandlerState()
    {
        throw new AssertionError();
    }
    
    public static final int INITIALIZING =  0x000;
    public static final int RECEIVED_PRIMARY_LOGIN_RESPONSE =  0x001;
    public static final int RECEIVED_PRIMARY_DIRECTORY_RESPONSE = 0x002;
    public static final int RECEIVED_SECONDARY_DIRECTORY_RESPONSE = 0x004;
    public static final int CONNECTING_TO_A_STARTING_SERVER = 0x008;
    public static final int MOVE_TO_CHANNEL_LIST = 0x010;
    public static final int CLOSING_STANDBY_CHANNELS = 0x020;
    public static final int MOVE_TO_NEXT_WSB_GROUP = 0x040;
    public static final int RECEIVED_PRIMARY_FIELD_DICTIONARY_RESPONSE = 0x080;
    public static final int RECEIVED_PRIMARY_ENUM_DICTIONARY_RESPONSE = 0x100;
    public static final int CLOSING = 0x200;
    public static final int INACTIVE = 0x400;
    public static final int MOVED_TO_CHANNEL_LIST = 0x800;
}
