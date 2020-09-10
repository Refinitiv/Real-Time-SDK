///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.rtsdk.ema.access;


class OmmErrorImpl extends DataImpl implements OmmError
{
	private final static String NO_ERROR_STRING = "NoError";
	private final static String NO_DICTIONARY_STRING = "NoDictionary";
	private final static String ITERATOR_SET_FAILURE_STRING = "IteratorSetFailure";
	private final static String ITERATOR_OVERRUN_STRING = "IteratorOverrun";
	private final static String FIELD_ID_NOT_FOUND_STRING = "FieldIdNotFound";
	private final static String INCOMPLETE_DATA_STRING = "IncompleteData";
	private final static String UNSUPPORTED_DATA_TYPE_STRING = "UnsupportedDataType";
	private final static String NO_SET_DEFINITION_STING = "NoSetDefinition";
	private final static String UNKNOWN_ERROR_STRING = "UnknownError";
	private final static String DEFAULT_STRING 	= "Unrecognized ErrorCode value ";
	
	private int _errorCode;

	@Override
	public int dataType()
	{
		return DataType.DataTypes.ERROR;
	}

	@Override
	public String errorCodeAsString()
	{
		switch (errorCode())
		{
			case ErrorCode.NO_ERROR :
				return NO_ERROR_STRING;
			case ErrorCode.NO_DICTIONARY :
				return NO_DICTIONARY_STRING;
			case ErrorCode.FIELD_ID_NOT_FOUND :
				return FIELD_ID_NOT_FOUND_STRING;
			case ErrorCode.ITERATOR_OVERRUN :
				return ITERATOR_OVERRUN_STRING;
			case ErrorCode.ITERATOR_SET_FAILURE :
				return ITERATOR_SET_FAILURE_STRING;
			case ErrorCode.INCOMPLETE_DATA :
				return INCOMPLETE_DATA_STRING;
			case ErrorCode.NO_SET_DEFINITION :
				return NO_SET_DEFINITION_STING;
			case ErrorCode.UNSUPPORTED_DATA_TYPE :
				return UNSUPPORTED_DATA_TYPE_STRING;
			case ErrorCode.UNKNOWN_ERROR :
				return UNKNOWN_ERROR_STRING;
			default :
				return DEFAULT_STRING + errorCode();
		}
	}

	@Override
	public int errorCode()
	{
		return _errorCode;
	}

	@Override
	public String toString()
	{
		return toString(0);
	}
	
	String toString(int indent)
	{
		_toString.setLength(0);
		
		Utilities.addIndent(_toString, indent).append("OmmError");
		++indent;
		Utilities.addIndent(_toString, indent, true).append("  ErrorCode=\"").append(errorCodeAsString()).append("\"");
		--indent;
		
		Utilities.addIndent(_toString, indent, true).append("OmmErrorEnd\n");
		
		return _toString.toString();
	}
	
	@Override
	void decode(com.rtsdk.eta.codec.Buffer rsslBuffer, int errorCode)
	{
		_rsslBuffer = rsslBuffer;
		_errorCode = errorCode;
	}
}