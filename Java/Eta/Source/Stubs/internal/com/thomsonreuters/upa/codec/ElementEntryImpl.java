package com.thomsonreuters.upa.codec;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataTypes;
import com.thomsonreuters.upa.codec.Date;
import com.thomsonreuters.upa.codec.DateTime;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.Double;
import com.thomsonreuters.upa.codec.ElementEntry;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.Enum;
import com.thomsonreuters.upa.codec.Float;
import com.thomsonreuters.upa.codec.Int;
import com.thomsonreuters.upa.codec.Qos;
import com.thomsonreuters.upa.codec.Real;
import com.thomsonreuters.upa.codec.State;
import com.thomsonreuters.upa.codec.Time;
import com.thomsonreuters.upa.codec.UInt;

class ElementEntryImpl implements ElementEntry
{
	
	@Override
	public void clear()
	{
		
	}

	@Override
	public int encode(EncodeIterator iter)
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public int encodeBlank(EncodeIterator iter)
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public int encode(EncodeIterator iter, Int data)
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public int encode(EncodeIterator iter, UInt data)
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public int encode(EncodeIterator iter, Real data)
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public int encode(EncodeIterator iter, Date data)
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public int encode(EncodeIterator iter, Time data)
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public int encode(EncodeIterator iter, DateTime data)
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public int encode(EncodeIterator iter, Qos data)
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public int encode(EncodeIterator iter, State data)
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public int encode(EncodeIterator iter, Enum data)
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public int encode(EncodeIterator iter, Buffer data)
	{
		return CodecReturnCodes.FAILURE;
	}
	
	@Override
	public int encodeInit(EncodeIterator iter, int encodingMaxSize)
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public int encodeComplete(EncodeIterator iter, boolean success)
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public int decode(DecodeIterator iter)
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public void name(Buffer name)
	{
	    
	}

	@Override
	public Buffer name()
	{
		return null;
	}
	
	@Override
	public void dataType(int dataType)
	{
        
	}

	@Override
	public int dataType()
	{
		return DataTypes.UNKNOWN;
	}

	@Override
	public void encodedData(Buffer encodedData)
	{
	    
	}

	@Override
	public Buffer encodedData()
	{
		return null;
	}

    @Override
    public int encode(EncodeIterator iter, Float data)
    {
    	return CodecReturnCodes.FAILURE;
    }

    @Override
    public int encode(EncodeIterator iter, Double data)
    {
    	return CodecReturnCodes.FAILURE;
    }
}
