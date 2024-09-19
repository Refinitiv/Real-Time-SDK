/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.codec;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.Decoders;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.SeriesEntry;

class SeriesEntryImpl implements SeriesEntry
{
    final Buffer _encodedData = CodecFactory.createBuffer();

    @Override
    public void clear()
    {
        _encodedData.clear();
    }

    @Override
    public int encode(EncodeIterator iter)
    {
        return Encoders.encodeSeriesEntry(iter, this);
    }

    @Override
    public int encodeInit(EncodeIterator iter, int maxEncodingSize)
    {
        return Encoders.encodeSeriesEntryInit(iter, this, maxEncodingSize);
    }

    @Override
    public int encodeComplete(EncodeIterator iter, boolean success)
    {
        return Encoders.encodeSeriesEntryComplete(iter, success);
    }

    @Override
    public int decode(DecodeIterator iter)
    {
        return Decoders.decodeSeriesEntry(iter, this);
    }

    @Override
    public Buffer encodedData()
    {
        return _encodedData;
    }

    @Override
    public void encodedData(Buffer encodedData)
    {
        assert (encodedData != null) : "encodedData must be non-null";

        ((BufferImpl)_encodedData).copyReferences(encodedData);
    }
}
