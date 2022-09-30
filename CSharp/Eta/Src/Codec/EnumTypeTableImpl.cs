/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Collections.Generic;

namespace Refinitiv.Eta.Codec
{

	internal class EnumTypeTableImpl : IEnumTypeTable
	{
		internal UInt16 _maxValue;
		internal List<IEnumType> _enumTypes;
		internal UInt16 _fidReferenceCount;
		internal List<Int16> _fidReferences;

		public UInt16 MaxValue
		{
            get
            {
                return _maxValue;
            }

            internal set
            {
                _maxValue = value;
            }
		}


		public List<IEnumType> EnumTypes
		{
            get
            {
                return _enumTypes;
            }

            internal set
            {
                _enumTypes = value;
            }
		}

		public List<Int16> FidReferences
		{
            get
            {
                return _fidReferences;
            }

            internal set
            {
                _fidReferences = value;
            }
		}
	}
}