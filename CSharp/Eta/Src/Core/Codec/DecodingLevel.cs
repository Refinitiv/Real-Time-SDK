/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System.Runtime.CompilerServices;

namespace LSEG.Eta.Codec
{
	internal class DecodingLevel
	{
		internal int _endBufPos; // Parsing internals, current position
		internal object _listType; // Pointer to actual list type
		internal int _nextEntryPos; // End of the current payload (end of a Message payload, or the end of a container Entry)
		internal FieldSetDef _fieldListSetDef; // RsslFieldListSetDef, used to decode this level
        internal ElementSetDef _elemListSetDef; // RsslElementListSetDef, used to decode this level
        internal int _itemCount; // number of items in the list
		internal int _nextItemPosition; // index of next item. Iterator is off when _nextItemPosition >= itemCount
		internal int _setCount; // number of items in the set
		internal int _nextSetPosition; // index of next item in a set
		internal int _containerType; // Type of container to decode for this level

		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		internal void Setup(int type, object container)
		{
			_itemCount = 0;
			_nextItemPosition = 0;
			_nextSetPosition = 0;
			_containerType = type;
			_listType = container;
		}

		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		internal int EndBufPos()
		{
			return _endBufPos;
		}

		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		internal int EndBufPos(int _endBufPos)
		{
			this._endBufPos = _endBufPos;
			return _endBufPos;
		}

	}

}