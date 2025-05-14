/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

/**
 * This class describes available Reactor Debugging Levels
 */
public class ReactorDebuggerLevels {

    public static final int LEVEL_NONE = 0x0000; //No messages will be debugged

    public static final int LEVEL_CONNECTION = 0x0001; //If applied, messages related to Connection will be debugged

    public static final int LEVEL_EVENTQUEUE = 0x0002; //If applied, the ReactorDebugger will log the number of events to be dispatched associated with different ReactorChannels

    public static final int LEVEL_TUNNELSTREAM = 0x0004; //If applied, ReactorDebugger will log messages associated with different TunnelStream events
}
