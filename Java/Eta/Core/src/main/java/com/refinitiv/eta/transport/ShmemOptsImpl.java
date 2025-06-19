/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport;

class ShmemOptsImpl implements ShmemOpts
{
    private long _maxReaderLag;

    /* Make a deep copy of this object object to the specified object.
     * 
     * destOpts is the destination object.
     */
    void copy(ShmemOptsImpl destOpts)
    {
        destOpts._maxReaderLag = _maxReaderLag;
    }

    @Override
    public String toString()
    {
        return "ShmemOpts" + "\n" + "\t\tmaxReaderLag: " + _maxReaderLag;
    }

    @Override
    public void maxReaderLag(long maxReaderLag)
    {
        _maxReaderLag = maxReaderLag;
    }

    @Override
    public long maxReaderLag()
    {
        return _maxReaderLag;
    }
}
