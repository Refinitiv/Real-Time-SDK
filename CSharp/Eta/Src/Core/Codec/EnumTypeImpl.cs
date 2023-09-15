/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.Codec
{

	internal class EnumTypeImpl : IEnumType
	{
		internal ushort _value;
		internal readonly Buffer _display = new (DataDictionary.EncodingChars);
		internal readonly Buffer _meaning = new ();

		public ushort Value
		{
            get
            {
                return _value;
            }

            internal set
            {
                _value = value;
            }
		}

		public Buffer Display
		{
            get
            {
                return _display;
            }

            internal set
            {
                _display.CopyReferences(value);
            }
		}


		public Buffer Meaning
		{
            get
            {
                return _meaning;
            }

            internal set
            {
                _meaning.CopyReferences(value);
            }
		}
	}
}