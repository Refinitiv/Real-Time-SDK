/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Runtime.CompilerServices;

namespace LSEG.Eta.Codec
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

		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public Buffer GetAcronym() { return _acronym; }

		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public Buffer GetDdeAcronym() { return _ddeAcronym; }

		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public Int16 GetFid() { return _fid; }

		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public Int16 GetRippleToField() { return _rippleToField; }

		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public SByte GetFieldType() { return _fieldType; }

		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public UInt16 GetLength() { return _length; }

		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public Byte GetEnumLength() { return _enumLength; }

		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public Byte GetRwfType() { return _rwfType; }

		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public UInt16 GetRwfLength() { return _rwfLength; }

		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public IEnumTypeTable GetEnumTypeTable() { return _enumTypeTable; }
	}
}