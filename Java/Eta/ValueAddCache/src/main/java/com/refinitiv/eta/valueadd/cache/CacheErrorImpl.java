/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.cache;

import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.valueadd.cache.CacheError;

class CacheErrorImpl implements CacheError
{
    int _errorId = CodecReturnCodes.SUCCESS;
    String _text = null;

    @Override
    public int errorId()
    {
        return _errorId;
    }

    @Override
    public void errorId(int errorId)
    {
        _errorId = errorId;
    }

    @Override
    public String text()
    {
        return _text;
    }

    @Override
    public void text(String text)
    {
        _text = text;
    }

    @Override
    public void clear()
    {
        _errorId = CodecReturnCodes.SUCCESS;
        _text = null;
    }

    public String toString()
    {
        return "Error" + "\n" + "\tErrorId: " + _errorId + "\n" + "\ttext: " + _text;
    }

}