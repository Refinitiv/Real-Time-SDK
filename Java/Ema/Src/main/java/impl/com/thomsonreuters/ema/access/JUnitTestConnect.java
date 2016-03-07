///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

import com.thomsonreuters.ema.access.Data;

//This class is created as a connect bridge between JUNIT test and EMA external/internal interface/classes.

public class JUnitTestConnect
{
	// used only for JUNIT tests
	public static FieldListImpl createFieldList()
	{
		return new FieldListImpl(true);
	}

	// used only for JUNIT tests
	public static ElementListImpl createElementList()
	{
		return new ElementListImpl(true);
	}

	// used only for JUNIT tests
	public static MapImpl createMap()
	{
		return new MapImpl(true);
	}
	
	// used only for JUNIT tests
	public static VectorImpl createVector()
	{
		return new VectorImpl(true);
	}

	// used only for JUNIT tests
	public static SeriesImpl createSeries()
	{
		return new SeriesImpl(true);
	}

	// used only for JUNIT tests
	public static FilterListImpl createFilterList()
	{
		return new FilterListImpl(true);
	}
	
	// used only for JUNIT tests
	public static OmmArrayImpl createArray()
	{
		return new OmmArrayImpl(true);
	}

	// used only for JUNIT tests
	public static void setRsslData(Data data, com.thomsonreuters.upa.codec.Msg rsslMsgEncoded, int majVer, int minVer,
			com.thomsonreuters.upa.codec.DataDictionary rsslDictionary, Object localFlSetDefDb)
	{
		((CollectionDataImpl) data).decode(rsslMsgEncoded, majVer, minVer, rsslDictionary);
	}

	// used only for JUNIT tests
	public static void setRsslData(Data data, com.thomsonreuters.upa.codec.Buffer rsslBufferEncoded, int majVer, int minVer,
			com.thomsonreuters.upa.codec.DataDictionary rsslDictionary, Object localFlSetDefDb)
	{
		((CollectionDataImpl) data).decode(rsslBufferEncoded, majVer, minVer, rsslDictionary, localFlSetDefDb);
	}
	
	// used only for JUNIT tests
	public static void setRsslData(Data data, Data dataEncoded, int majVer, int minVer,
			com.thomsonreuters.upa.codec.DataDictionary rsslDictionary, Object localFlSetDefDb)
	{
		((CollectionDataImpl) data).decode(((DataImpl)dataEncoded).encodedData(), majVer, minVer,  rsslDictionary, localFlSetDefDb);
	}
}