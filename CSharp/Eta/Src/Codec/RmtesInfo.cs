/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

using Refinitiv.Eta.Common;

namespace Refinitiv.Eta.Codec
{
    class RmtesInfo
    {
        internal RmtesBuffer StringBuffer { get; set; }
        internal CodecReturnCode CodecReturnCode { get; set; }
        internal RmtesWorkingSet Set { get; set; }      
        internal int Value { get; set; }       
        internal char CharValue { get; set; }
        internal ByteBuffer Iter { get; set; }
        internal ESCReturnCode ESCRetCode { get; set; }

        internal RmtesInfo ReturnUCS2ToUTF8(ByteBuffer data, int value)
        {
            Iter = data;
            Value = value;
            return this;
        }

        internal RmtesInfo ReturnUTF8ToUCS2(int value, char c)
        {
            Value = value;
            CharValue = c;
            return this;
        }

        internal RmtesInfo ReturnControlParse(ESCReturnCode escSuccess, RmtesWorkingSet currentSet, int i, ByteBuffer iIter)
        {
            ESCRetCode = escSuccess;
            Set = currentSet;
            Value = i;
            Iter = iIter;
            return this;
        }
    }
}
