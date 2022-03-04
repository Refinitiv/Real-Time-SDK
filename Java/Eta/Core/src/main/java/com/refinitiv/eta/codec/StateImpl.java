/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.codec;

class StateImpl implements State
{
    private int _streamState;
    private int _dataState;
    private int _code;
    private final Buffer _text = CodecFactory.createBuffer();
    boolean _isBlank;

    @Override
    public void clear()
    {
        _streamState = StreamStates.UNSPECIFIED;
        _dataState = DataStates.SUSPECT;
        _code = StateCodes.NONE;
        _text.clear();
    }

    void blank()
    {
        clear();
        _isBlank = true;
    }

    @Override
    public boolean isBlank()
    {
        return _isBlank;
    }
	
    @Override
    public int copy(State destState)
    {
        if (null == destState)
            return CodecReturnCodes.INVALID_ARGUMENT;

        ((StateImpl)destState)._streamState = _streamState;
        ((StateImpl)destState)._dataState = _dataState;
        ((StateImpl)destState)._code = _code;
        ((StateImpl)destState)._isBlank = _isBlank;

        int ret = 0;
        if ((ret = ((BufferImpl)_text).copyWithOrWithoutByteBuffer(destState.text())) != CodecReturnCodes.SUCCESS)
            return ret;

        return CodecReturnCodes.SUCCESS;
    }

    @Override
    public int encode(EncodeIterator iter)
    {
        if (!isBlank())
        {
            return Encoders.PrimitiveEncoder.encodeState((EncodeIteratorImpl)iter, this);
        }
        else
        {
            return CodecReturnCodes.INVALID_ARGUMENT;
        }
    }

    @Override
    public int decode(DecodeIterator iter)
    {
        return Decoders.decodeState(iter, this);
    }

    @Override
    public String toString()
    {
        StringBuilder strBuf = new StringBuilder("State: " + StreamStates.info(_streamState) + "/" +
                                                 DataStates.info(_dataState) + "/" +
                                                 StateCodes.info(_code) + " - text: ");

        strBuf.append("\"" + _text.toString() + "\"");

        return strBuf.toString();
    }

    @Override
    public boolean isFinal()
    {
        return _streamState == StreamStates.CLOSED_RECOVER ||
                _streamState == StreamStates.CLOSED ||
                _streamState == StreamStates.REDIRECTED;
    }

    @Override
    public int streamState(int streamState)
    {
        if (!(streamState >= 0 && streamState <= 31))
        {
            return CodecReturnCodes.INVALID_ARGUMENT;
        }

        _streamState = streamState;
        _isBlank = false;

        return CodecReturnCodes.SUCCESS;
    }

    @Override
    public int streamState()
    {
        return _streamState;
    }

    @Override
    public int dataState(int dataState)
    {
        if (!(dataState >= 0 && dataState <= 7))
        {
            return CodecReturnCodes.INVALID_ARGUMENT;
        }

        _dataState = dataState;
        _isBlank = false;

        return CodecReturnCodes.SUCCESS;
    }

    @Override
    public int dataState()
    {
        return _dataState;
    }

    @Override
    public int code(int code)
    {
        if (!(code >= StateCodes.NONE && code <= StateCodes.MAX_RESERVED))
        {
            return CodecReturnCodes.INVALID_ARGUMENT;
        }

        _code = code;
        _isBlank = false;

        return CodecReturnCodes.SUCCESS;
    }

    @Override
    public int code()
    {
        return _code;
    }

    @Override
    public int text(Buffer text)
    {
        if (text == null)
        {
            return CodecReturnCodes.INVALID_ARGUMENT;
        }

        ((BufferImpl)_text).copyReferences(text);
        _isBlank = false;

        return CodecReturnCodes.SUCCESS;
    }

    @Override
    public Buffer text()
    {
        return _text;
    }

    @Override
    public boolean equals(State thatState)
    {
        return ((thatState != null) &&
                (_streamState == ((StateImpl)thatState)._streamState) &&
                (_dataState == ((StateImpl)thatState)._dataState) &&
                (_code == ((StateImpl)thatState)._code) &&
                isTextEquals(thatState.text()));
    }

    private boolean isTextEquals(Buffer thatText)
    {
        if (_text == null && thatText == null)
        {
            return true;
        }
        else if (_text != null)
        {
            return _text.equals(thatText);
        }

        return false;
    }
}
