/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

class ReactorWarmStandbyEventTypes 
{   
    public static final int INIT = 0;
    public static final int CONNECT_SECONDARY_SERVER = 1;
    public static final int CHANGE_ACTIVE_TO_STANDBY_SERVER = 2;
    public static final int CHANGE_ACTIVE_TO_STANDBY_SERVICE_CHANNEL_DOWN = 5;
    public static final int REMOVE_SERVER_FROM_WSB_GROUP = 7;
    public static final int CONNECT_TO_NEXT_STARTING_SERVER = 8;
    public static final int ACTIVE_SERVER_SERVICE_STATE_FROM_DOWN_TO_UP = 9;
    public static final int MOVE_WSB_HANDLER_BACK_TO_POOL = 10;
    public static final int PREFERRED_HOST_FALLBACK_IN_GROUP = 11;
    
    /* Returns a String representation of the specified ReactorWarmStandbyEventTypes type. */
    static String toString(int type)
    {
        switch (type)
        {
            case INIT:
                return "ReactorWarmStandbyEventType.INIT";
            case CONNECT_SECONDARY_SERVER:
                return "ReactorWarmStandbyEventType.CONNECT_SECONDARY_SERVER";
            case CHANGE_ACTIVE_TO_STANDBY_SERVER:
                return "ReactorWarmStandbyEventType.CHANGE_ACTIVE_TO_STANDBY_SERVER";
            case CHANGE_ACTIVE_TO_STANDBY_SERVICE_CHANNEL_DOWN:
                return "ReactorWarmStandbyEventType.CHANGE_ACTIVE_TO_STANDBY_SERVICE_CHANNEL_DOWN";
            case REMOVE_SERVER_FROM_WSB_GROUP:
                return "ReactorWarmStandbyEventType.REMOVE_SERVER_FROM_WSB_GROUP";
            case CONNECT_TO_NEXT_STARTING_SERVER:
                return "ReactorWarmStandbyEventType.CONNECT_TO_NEXT_STARTING_SERVER";
            case ACTIVE_SERVER_SERVICE_STATE_FROM_DOWN_TO_UP:
                return "ReactorWarmStandbyEventType.ACTIVE_SERVER_SERVICE_STATE_FROM_DOWN_TO_UP";
            case MOVE_WSB_HANDLER_BACK_TO_POOL:
                return "ReactorWarmStandbyEventType.MOVE_WSB_HANDLER_BACK_TO_POOL";               
            default:
                return "ReactorWarmStandbyEventType " + type + " - undefined.";
        }
    }
}
