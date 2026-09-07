/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Access;
using System;
using System.Collections.Generic;
using System.Text;

namespace LSEG.Ema.Domain.Internal
{
    internal static class ToStringExtensions
    {
        public static void AppendToString<T>(
            this IEnumerable<T> seq,
            StringBuilder builder,
            int indent,
            Action<StringBuilder, T, int> appendItem,
            bool wrapInBraces = false)
        {
            builder.AddIndent(indent).AppendLine("[");
            var additionalIndent = wrapInBraces ? 1 : 0;
            foreach (var item in seq)
            {
                if (wrapInBraces)
                    builder.AddIndent(indent + additionalIndent).AppendLine("{");

                appendItem(builder, item, indent + additionalIndent + 1);

                if (wrapInBraces)
                    builder.AddIndent(indent + additionalIndent).AppendLine("}");
            }
            builder.AddIndent(indent).AppendLine("]");
        }

        public static StringBuilder AppendToString(this EmaBuffer value, StringBuilder builder, int indent)
        {
            return builder.AddIndent(indent).Append(value?.AsRawHexString() ?? "<null>").AppendLine();
        }
    }
}
