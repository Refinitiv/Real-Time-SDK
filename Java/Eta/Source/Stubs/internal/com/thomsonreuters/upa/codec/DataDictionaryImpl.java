package com.thomsonreuters.upa.codec;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataDictionary;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.DictionaryEntry;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.Enum;
import com.thomsonreuters.upa.codec.EnumType;
import com.thomsonreuters.upa.codec.EnumTypeTable;
import com.thomsonreuters.upa.codec.FieldSetDefDb;
import com.thomsonreuters.upa.codec.Int;
import com.thomsonreuters.upa.transport.Error;

class DataDictionaryImpl implements DataDictionary
{
   
    
    @Override
    public void clear()
    {
        
    }
    
    @Override
    public DictionaryEntry entry(int fieldId)
    {
    	return null;
    }

    @Override
    public EnumType entryEnumType(DictionaryEntry entryInt, Enum value)
    {
       return null;
    }
    
    @Override
    public int loadFieldDictionary(String filename, Error error)
    {
    	return CodecReturnCodes.FAILURE;
    }
    
    @Override
    public int loadEnumTypeDictionary(String filename, Error error)
    {
    	return CodecReturnCodes.FAILURE;
    }

  
    
    @Override
    public int extractDictionaryType(DecodeIterator iterInt, Int dictionaryType, Error error)
    {
    	return CodecReturnCodes.FAILURE;
    }
    
    @Override
    public int encodeFieldDictionary(EncodeIterator iter, Int currentFid, int verbosity, Error error)
    {
    	return CodecReturnCodes.FAILURE;
    }

	@Override
    public int decodeFieldDictionary(DecodeIterator iter, int verbosity, Error error)
    {
		return CodecReturnCodes.FAILURE;
    }
    
    @Override
    public int encodeEnumTypeDictionary(EncodeIterator iter, int verbosity, Error error)
    {
    	return CodecReturnCodes.FAILURE;
    }
    
    @Override
    public int decodeEnumTypeDictionary(DecodeIterator iter, int verbosity, Error error)
    {
    	return CodecReturnCodes.FAILURE;
    }
    
    @Override
    public FieldSetDefDb fieldSetDef()
    {
        return null;
    }
    
 
	@Override
    public int minFid() 
    {
        return 0;
    }

    @Override
    public int maxFid() 
    {
        return 0;
    }

    @Override
    public int numberOfEntries() 
    {
        return 0;
    }

    @Override
    public EnumTypeTable[] enumTables() 
    {
        return null;
    }

    @Override
    public int enumTableCount() 
    {
        return 0;
    }

    @Override
    public int infoDictionaryId() 
    {
        return 0;
    }

    @Override
    public Buffer infoFieldVersion() 
    {
        return null;
    }

    @Override
    public Buffer infoEnumRTVersion() 
    {
        return null;
    }

    @Override
    public Buffer infoEnumDTVersion() 
    {
        return null;
    }

    @Override
    public Buffer infoFieldFilename() 
    {
        return null;
    }

    @Override
    public Buffer infoFieldDesc() 
    {
        return null;
    }

    @Override
    public Buffer infoFieldBuild() 
    {
        return null;
    }

    @Override
    public Buffer infoFieldDate() 
    {
        return null;
    }

    @Override
    public Buffer infoEnumFilename() 
    {
        return null;
    }

    @Override
    public Buffer infoEnumDesc() 
    {
        return null;
    }

    @Override
    public Buffer infoEnumDate() 
    {
        return null;
    }
    
    @Override
    public String toString()
    {
       return null;
    }
}
