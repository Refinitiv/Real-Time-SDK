/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System.Runtime.CompilerServices;

namespace LSEG.Eta.Codec
{
	internal class EncodingLevel
	{
		internal int _countWritePos; // The position of the count pointer in the buffer
		internal int _initElemStartPos; // The start position of an initialize element
		internal int _containerStartPos; // The start position of a container
		internal int _currentCount; // Current number of elements
		internal EncodeIteratorStates _encodingState; // The current encoding state
		internal int _containerType; // The containerType of the list
		internal EncodeIteratorFlags _flags; // Internal flag values used for various purposes
		internal int _reservedBytes; /* Bytes reserved for data that will need to be written later
	                                     (e.g. count for a container's entries that is written after summary data is encoded). */
		internal object _listType; // Pointer to actual list type
		internal FieldSetDef _fieldListSetDef; // RsslFieldListSetDef, used to encode this level
        internal ElementSetDef _elemListSetDef; // RsslElementListSetDef, used to decode this level

        /* Buffer size mark used internally */
        internal readonly EncodeSizeMark _internalMark = new EncodeSizeMark();

		/* Second Internal mark */
		internal readonly EncodeSizeMark _internalMark2 = new EncodeSizeMark();

		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		internal void Init(int dFormat, EncodeIteratorStates eState, object lType, int cStartPos)
		{
			_countWritePos = 0;
			_initElemStartPos = 0;
			_currentCount = 0;
			_internalMark._sizePos = 0;
			_internalMark._sizeBytes = 0;
			_internalMark2._sizePos = 0;
			_internalMark2._sizeBytes = 0;
			_fieldListSetDef = null;
			_encodingState = eState;
			_containerType = dFormat;
			_reservedBytes = 0;
			_listType = lType;
			_containerStartPos = cStartPos;
		}

		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		internal void RealignBuffer(int offset)
		{
			_countWritePos += offset;
			_initElemStartPos += offset;
			_internalMark._sizePos += offset;
			_internalMark2._sizePos += offset;
			_containerStartPos += offset;
		}
	}



}