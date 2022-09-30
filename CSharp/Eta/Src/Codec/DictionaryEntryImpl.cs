/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

using System;

namespace Refinitiv.Eta.Codec
{

	internal class DictionaryEntryImpl : IDictionaryEntry
	{
		internal readonly Buffer _acronym = new Buffer();
		internal readonly Buffer _ddeAcronym = new Buffer();
		internal Int16 _fid;
		internal Int16 _rippleToField;
		internal SByte _fieldType;
		internal UInt16 _length;
		internal Byte _enumLength;
		internal Byte _rwfType;
		internal UInt16 _rwfLength;
		internal IEnumTypeTable _enumTypeTable;

		public Buffer Acronym { get => _acronym; }

		public Buffer DdeAcronym{ get=>_ddeAcronym; }

		public Int16 Fid { get=>_fid; }

		public Int16 RippleToField { get => _rippleToField; }

		public SByte FieldType { get=> _fieldType; }

		public UInt16 Length{ get =>_length; }

		public Byte EnumLength { get=>_enumLength; }

		public Byte RwfType { get=> _rwfType; }

		public UInt16 RwfLength { get=>_rwfLength; }

		public IEnumTypeTable EnumTypeTable { get=> _enumTypeTable; }
	}
}