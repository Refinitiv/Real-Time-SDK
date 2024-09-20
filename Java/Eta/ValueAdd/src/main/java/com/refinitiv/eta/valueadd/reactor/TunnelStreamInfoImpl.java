/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

import java.util.Objects;

class TunnelStreamInfoImpl implements TunnelStreamInfo {

    int _ordinaryBuffersUsed;
    int _bigBuffersUsed;

    TunnelStreamInfoImpl () {


    }

    TunnelStreamInfoImpl (int ordinaryBuffersUsed, int bigBuffersUsed) {
        _ordinaryBuffersUsed = ordinaryBuffersUsed;
        _bigBuffersUsed = bigBuffersUsed;
    }

    @Override
    public String toString() {
        return "TunnelStreamInfo" + "\n"
                + "\tordinaryBuffersUsed: " + _ordinaryBuffersUsed + "\n"
                + "\tbigBuffersUsed: " + _bigBuffersUsed + "\n"
                + "\ttotalBuffersUsed: " + (_ordinaryBuffersUsed + _bigBuffersUsed) + "\n";
    }

    @Override
    public int buffersUsed() {
        return _ordinaryBuffersUsed + _bigBuffersUsed;
    }

    public void ordinaryBuffersUsed(int ordinaryBuffersUsed) {
        _ordinaryBuffersUsed = ordinaryBuffersUsed;
    }

    @Override
    public int ordinaryBuffersUsed() {
        return _ordinaryBuffersUsed;
    }

    public void bigBuffersUsed(int bigBuffersUsed) {
        _bigBuffersUsed = bigBuffersUsed;
    }

    @Override
    public int bigBuffersUsed() {
        return _bigBuffersUsed;
    }

    @Override
    public void clear() {
        _ordinaryBuffersUsed = 0;
        _bigBuffersUsed = 0;
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        TunnelStreamInfoImpl that = (TunnelStreamInfoImpl) o;
        return _ordinaryBuffersUsed == that._ordinaryBuffersUsed &&
                _bigBuffersUsed == that._bigBuffersUsed;
    }

    @Override
    public int hashCode() {
        return Objects.hash(_ordinaryBuffersUsed, _bigBuffersUsed);
    }
}
