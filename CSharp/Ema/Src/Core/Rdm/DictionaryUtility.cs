/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Access;

namespace LSEG.Ema.Rdm;

/// <summary>
/// Utilities that can be used by EMA application.
/// </summary>
/// <seealso cref="DataDictionary"/>
public sealed class DictionaryUtility
{
    /// <summary>
    /// Extract an DataDictionary object used by FieldList when decoding it.
    /// The DataDictionary is valid only in the context of a callback method.
    /// </summary>
    /// <param name="fieldList">the FieldList to extract <see cref="DataDictionary"/></param>
    /// <returns>the DataDictionary if it exists</returns>
    /// <exception cref="OmmInvalidUsageException">Thrown if this method fails to retrieve the <see cref="DataDictionary"/> from the indicated Field List object.</exception>
    public static Rdm.DataDictionary ExtractDataDictionary(Ema.Access.FieldList fieldList)
    {
        if (fieldList.m_objectManager == null)
        {
            throw new OmmInvalidUsageException("Failed to extract DataDictionary from the passed in FieldList",
                    OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
        }

        var dictionary = new Ema.Rdm.DataDictionary();
        dictionary.rsslDataDictionary(fieldList.m_dataDictionary!);
        return dictionary;
    }
}
