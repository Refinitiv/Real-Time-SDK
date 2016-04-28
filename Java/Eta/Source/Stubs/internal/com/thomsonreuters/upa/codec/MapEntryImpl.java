package com.thomsonreuters.upa.codec;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.Date;
import com.thomsonreuters.upa.codec.DateTime;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.Double;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.Enum;
import com.thomsonreuters.upa.codec.Float;
import com.thomsonreuters.upa.codec.Int;
import com.thomsonreuters.upa.codec.MapEntry;
import com.thomsonreuters.upa.codec.Qos;
import com.thomsonreuters.upa.codec.Real;
import com.thomsonreuters.upa.codec.State;
import com.thomsonreuters.upa.codec.Time;
import com.thomsonreuters.upa.codec.UInt;

class MapEntryImpl implements MapEntry
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
	public int encode(EncodeIterator iter, Int keyData)
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public int encode(EncodeIterator iter, UInt keyData)
	{
		return CodecReturnCodes.FAILURE;   
	}

	@Override
	public int encode(EncodeIterator iter, Real keyData)
	{
		return CodecReturnCodes.FAILURE;    
	}

	@Override
	public int encode(EncodeIterator iter, Date keyData)
	{
		return CodecReturnCodes.FAILURE;  
	}

	@Override
	public int encode(EncodeIterator iter, Time keyData)
	{
		return CodecReturnCodes.FAILURE; 
	}

	@Override
	public int encode(EncodeIterator iter, DateTime keyData)
	{
		return CodecReturnCodes.FAILURE;   
	}

	@Override
	public int encode(EncodeIterator iter, Qos keyData)
	{
		return CodecReturnCodes.FAILURE;    
	}

	@Override
	public int encode(EncodeIterator iter, State keyData)
	{
		return CodecReturnCodes.FAILURE; 
	}

	@Override
	public int encode(EncodeIterator iter, Enum keyData)
	{
		return CodecReturnCodes.FAILURE;    
	}

	@Override
	public int encode(EncodeIterator iter, Buffer keyData)
	{
		return CodecReturnCodes.FAILURE;    
	}

	@Override
	public int encodeInit(EncodeIterator iter, int maxEncodingSize)
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public int encodeInit(EncodeIterator iter, Int keyData, int maxEncodingSize)
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public int encodeInit(EncodeIterator iter, UInt keyData, int maxEncodingSize)
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public int encodeInit(EncodeIterator iter, Real keyData, int maxEncodingSize)
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public int encodeInit(EncodeIterator iter, Date keyData, int maxEncodingSize)
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public int encodeInit(EncodeIterator iter, Time keyData, int maxEncodingSize)
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public int encodeInit(EncodeIterator iter, DateTime keyData, int maxEncodingSize)
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public int encodeInit(EncodeIterator iter, Qos keyData, int maxEncodingSize)
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public int encodeInit(EncodeIterator iter, State keyData, int maxEncodingSize)
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public int encodeInit(EncodeIterator iter, Enum keyData, int maxEncodingSize)
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public int encodeInit(EncodeIterator iter, Buffer keyData, int maxEncodingSize)
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public int encodeComplete(EncodeIterator iter, boolean success)
	{
		return CodecReturnCodes.FAILURE;
	}
	
	@Override
	public int decode(DecodeIterator iter, Object keyData)
	{
		return CodecReturnCodes.FAILURE;			
	}
	
	@Override
	public boolean checkHasPermData()
	{
			return false;
	}

	@Override
	public void applyHasPermData()
	{
		
	}

	@Override
	public void action(int action)
	{
	   
	}

	@Override
	public int action()
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public int flags() 
	{
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public void flags(int flags) 
	{
	   
	}

	@Override
	public Buffer permData() 
	{
		return null;
	}

	@Override
	public void permData(Buffer permData) 
	{
	   
	}

	@Override
	public Buffer encodedKey() 
	{
		return null;
	}

	@Override
	public void encodedKey(Buffer encodedKey) 
	{
	    
	}

	@Override
	public Buffer encodedData() 
	{
		return null;
	}

	@Override
	public void encodedData(Buffer encodedData) 
	{
	   
	}

    @Override
    public int encode(EncodeIterator iter, Float keyData)
    {
    	return CodecReturnCodes.FAILURE;
    }

    @Override
    public int encode(EncodeIterator iter, Double keyData)
    {
    	return CodecReturnCodes.FAILURE;
    }

    @Override
    public int encodeInit(EncodeIterator iter, Float keyData, int maxEncodingSize)
    {
    	return CodecReturnCodes.FAILURE;
    }

    @Override
    public int encodeInit(EncodeIterator iter, Double keyData, int maxEncodingSize)
    {
    	return CodecReturnCodes.FAILURE;
    }
}
