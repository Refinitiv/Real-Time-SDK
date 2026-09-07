/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Access;
using LSEG.Eta.Codec;
using System;
using System.Collections.Generic;

namespace LSEG.Ema.Domain.Internal
{
    internal static class EncodingExtensions
    {
        public static OmmArray ToOmmArray<T>(this IEnumerable<T> seq, Action<OmmArray, T> addItemAction) =>
            seq.ToOmmArray(addItemAction, new OmmArray());

        public static OmmArray ToOmmArray<T>(this IEnumerable<T> seq, Action<OmmArray, T> addItemAction, OmmArray ommArray)
        {
            foreach (var item in seq)
            {
                addItemAction(ommArray, item);
            }
            return ommArray.Complete();
        }

        public static OmmQos Populate(this OmmQos ommQos, uint timeliness, uint rate)
        {
            var qos = new Qos();
            Utilities.ToRsslQos(rate, timeliness, qos);

            ommQos.Decode(qos);

            return ommQos;
        }

        /// <summary>
        /// Retrieves the ServiceId from the message and validates that it is within the range of a ushort. If it is not, an OmmInvalidUsageException is thrown.
        /// </summary>
        /// <param name="message">The message from which to retrieve the ServiceId.</param>
        /// <returns>The ServiceId as a ushort.</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if the ServiceId is not within the range of a ushort.</exception>
        public static ushort GetServiceId(this Access.Msg message)
        {
            var rawServiceId = message.ServiceId();
            if (!(rawServiceId is >= ushort.MinValue and <= ushort.MaxValue))
                throw new OmmInvalidUsageException($"ServiceId value {rawServiceId} must be between {ushort.MinValue} and {ushort.MaxValue}", OmmInvalidUsageException.ErrorCodes.VALUE_OUT_OF_RANGE);
            return (ushort)rawServiceId;
        }

        public static ushort GetServiceId(this Access.MapEntry mapEntry)
        {
            var rawServiceId = mapEntry.Key.UInt();
            if (!(rawServiceId is >= ushort.MinValue and <= ushort.MaxValue))
                throw new OmmInvalidUsageException($"ServiceId value {rawServiceId} must be between {ushort.MinValue} and {ushort.MaxValue}", OmmInvalidUsageException.ErrorCodes.VALUE_OUT_OF_RANGE);
            return (ushort)rawServiceId;
        }
    }
}
