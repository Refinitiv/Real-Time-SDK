/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */


using System.Collections;
using System.Collections.Generic;

using LSEG.Eta.Codec;

namespace LSEG.Ema.Access;

internal sealed class OmmArrayEnumerator : IEnumerator<OmmArrayEntry>
{
    #region IEnumerator Public members

    public OmmArrayEntry Current { get; private set; } = new();

    object IEnumerator.Current => Current;

    public void Dispose()
    {
    }

    public bool MoveNext()
    {
        if (m_AtEnd)
            return false;

        if (m_ErrorCode != OmmError.ErrorCodes.NO_ERROR)
        {
            if (m_ErrorCode == OmmError.ErrorCodes.ITERATOR_OVERRUN)
                m_AtEnd = true;
            Current.Error(m_ErrorCode);
            m_ErrorCode = OmmError.ErrorCodes.NO_ERROR;
            return true;
        }

        CodecReturnCode retCode = m_ArrayEntry.Decode(m_DecodeIterator);

        switch (retCode)
        {
            case CodecReturnCode.SUCCESS:
                Current.Decode(m_DecodeIterator, m_Array.PrimitiveType);
                return true;

            case CodecReturnCode.END_OF_CONTAINER:
                m_AtEnd = true;
                return false;

            case CodecReturnCode.INVALID_ARGUMENT:
            case CodecReturnCode.INCOMPLETE_DATA:
                Current.Error(OmmError.ErrorCodes.INCOMPLETE_DATA);
                return true;

            case CodecReturnCode.UNSUPPORTED_DATA_TYPE:
                Current.Error(OmmError.ErrorCodes.UNSUPPORTED_DATA_TYPE);
                return true;

            default:
                Current.Error(OmmError.ErrorCodes.UNKNOWN_ERROR);
                return true;
        }
    }

    public void Reset()
    {
        // create a new enumerator via OmmArray.GetEnumerator() instread
        throw new OmmInvalidUsageException("OmmArrayEnumerator does not support Reset()");
    }

    #endregion

    #region Private members

    internal OmmError.ErrorCodes ErrorCode { get => m_ErrorCode; }

    private int m_MajVer;
    private int m_MinVer;
    private DecodeIterator m_DecodeIterator = new();
    private bool m_AtEnd;
    private OmmError.ErrorCodes m_ErrorCode;
    private Data.DataCode m_DataCode;
    private LSEG.Eta.Codec.Array m_Array = new();
    private LSEG.Eta.Codec.ArrayEntry m_ArrayEntry = new();

    internal CodecReturnCode SetRsslData(Eta.Codec.Buffer buffer, int majVer, int minVer)
    {
        m_MajVer = majVer;
        m_MinVer = minVer;

        m_DecodeIterator.Clear();

        CodecReturnCode retCode = m_DecodeIterator.SetBufferAndRWFVersion(buffer, m_MajVer, m_MinVer);
        if (CodecReturnCode.SUCCESS != retCode)
        {
            m_AtEnd = false;
            m_DataCode = (buffer.Length > 0) ? Data.DataCode.NO_CODE : Data.DataCode.BLANK;
            m_ErrorCode = OmmError.ErrorCodes.ITERATOR_SET_FAILURE;
            return retCode;
        }

        retCode = m_Array.Decode(m_DecodeIterator);

        // shortcut for the likely outcome
        if (retCode == CodecReturnCode.SUCCESS)
        {
            m_AtEnd = false;
            m_DataCode = Data.DataCode.NO_CODE;
            m_ErrorCode = OmmError.ErrorCodes.NO_ERROR;
            return retCode;// false;
        }

        // dispatch unlikely outcome
        switch (retCode)
        {
            case CodecReturnCode.BLANK_DATA:
                m_AtEnd = true;
                m_DataCode = Data.DataCode.BLANK;
                m_ErrorCode = OmmError.ErrorCodes.NO_ERROR;
                return retCode;

            case CodecReturnCode.ITERATOR_OVERRUN:
                m_AtEnd = false;
                m_DataCode = (buffer.Length > 0) ? Data.DataCode.NO_CODE : Data.DataCode.BLANK;
                m_ErrorCode = OmmError.ErrorCodes.ITERATOR_OVERRUN;
                return retCode;

            case CodecReturnCode.INCOMPLETE_DATA:
                m_AtEnd = false;
                m_DataCode = (buffer.Length > 0) ? Data.DataCode.NO_CODE : Data.DataCode.BLANK;
                m_ErrorCode = OmmError.ErrorCodes.INCOMPLETE_DATA;
                return retCode;

            default:
                m_AtEnd = false;
                m_DataCode = (buffer.Length > 0) ? Data.DataCode.NO_CODE : Data.DataCode.BLANK;
                m_ErrorCode = OmmError.ErrorCodes.UNKNOWN_ERROR;
                return retCode;
        }
    }

    #endregion
}