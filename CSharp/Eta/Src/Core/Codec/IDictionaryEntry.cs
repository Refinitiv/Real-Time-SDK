/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System;

namespace LSEG.Eta.Codec
{

	/// <summary>
	/// A data dictionary entry, containing field information and an enumeration table reference if present.
	/// </summary>
	/// <seealso cref="MfFieldTypes"/>
	/// <seealso cref="DataTypes"/>
	public interface IDictionaryEntry
	{
		/// <summary>
		/// Acronym.
		/// </summary>
		/// <returns> the acronym
		/// </returns>
		Buffer GetAcronym();

		/// <summary>
		/// DDE Acronym.
		/// </summary>
		/// <returns> the ddeAcronym
		/// </returns>
		Buffer GetDdeAcronym();

		/// <summary>
		/// The fieldId the entry corresponds to.
		/// </summary>
		/// <returns> the fid
		/// </returns>
		Int16 GetFid();

		/// <summary>
		/// The field to ripple data to.
		/// </summary>
		/// <returns> the rippleToField
		/// </returns>
		Int16 GetRippleToField();

		/// <summary>
		/// Marketfeed Field Type.
		/// </summary>
		/// <returns> the fieldType
		/// </returns>
		SByte GetFieldType();

		/// <summary>
		/// Marketfeed length.
		/// </summary>
		/// <returns> the length
		/// </returns>
		UInt16 GetLength();

		/// <summary>
		/// Marketfeed enum length.
		/// </summary>
		/// <returns> the enumLength
		/// </returns>
		Byte GetEnumLength();

		/// <summary>
		/// RWF type.
		/// </summary>
		/// <returns> the rwfType
		/// </returns>
		Byte GetRwfType();

		/// <summary>
		/// RWF Length.
		/// </summary>
		/// <returns> the .GetRwfLength()
		/// </returns>
		UInt16 GetRwfLength();

		/// <summary>
		/// A reference to the corresponding enumerated types table, if this field uses one.
		/// </summary>
		/// <returns> the enumTypeTable
		/// </returns>
		IEnumTypeTable GetEnumTypeTable();
	}
}