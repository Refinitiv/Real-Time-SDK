/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022,2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

import java.io.OutputStream;

class ReactorDebuggerOptionsImpl implements ReactorDebuggerOptions {

    volatile private int _debuggingLevels = ReactorDebuggerLevels.LEVEL_NONE;

    OutputStream _outputStream;
    int _capacity;

    @Override
    public void copy(ReactorDebuggerOptions dst) {
        if (dst != null) {
            dst.setDebuggingLevels(_debuggingLevels);
            dst.outputStream(_outputStream);
            dst.capacity(_capacity);
        }
    }

    @Override
    public void setDebuggingLevels(int value) {
        _debuggingLevels = value;
    }

    @Override
    public void enableLevel(int level) {
        if (level == ReactorDebuggerLevels.LEVEL_CONNECTION || level == ReactorDebuggerLevels.LEVEL_EVENTQUEUE || level == ReactorDebuggerLevels.LEVEL_TUNNELSTREAM) {
            _debuggingLevels |= level;
        }
    }

    @Override
    public void disableLevel(int level) {
        if (level == ReactorDebuggerLevels.LEVEL_CONNECTION || level == ReactorDebuggerLevels.LEVEL_EVENTQUEUE || level == ReactorDebuggerLevels.LEVEL_TUNNELSTREAM) {
            _debuggingLevels &= ~level;
        }
    }

    @Override
    public boolean debugConnectionLevel() {
        return (_debuggingLevels & ReactorDebuggerLevels.LEVEL_CONNECTION) != 0;
    }

    @Override
    public boolean debugEventQueueLevel() {
        return (_debuggingLevels & ReactorDebuggerLevels.LEVEL_EVENTQUEUE) != 0;
    }

    @Override
    public boolean debugTunnelStreamLevel() {
        return (_debuggingLevels & ReactorDebuggerLevels.LEVEL_TUNNELSTREAM) != 0;
    }

    @Override
    public boolean debugEnabled() {
        return _debuggingLevels != ReactorDebuggerLevels.LEVEL_NONE;
    }

    @Override
    public int debuggingLevels() {
        return _debuggingLevels;
    }

    @Override
    public OutputStream outputStream() {
        return _outputStream;
    }

    @Override
    public void outputStream(OutputStream outputStream) {
        this._outputStream = outputStream;
    }

    @Override
    public int capacity() {
        return _capacity;
    }

    @Override
    public void capacity(int capacity) {
        this._capacity = capacity;
    }

    @Override
    public void clear() {
        _debuggingLevels = ReactorDebuggerLevels.LEVEL_NONE;
        _outputStream = null;
        _capacity = 0;
    }

}


