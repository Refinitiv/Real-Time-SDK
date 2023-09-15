/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.Transports.Internal
{
    internal struct RipcConnectionReply
    {
        private ushort _messageLength;

        public ushort MessageLength
        {
            get => _messageLength == 0 ? FixedSizeOf : _messageLength;
            set => _messageLength = value;
        }

        public RipcFlags Flags;
        public RipcOpCode OpCode;
        public byte HeaderLength { internal get; set; }
        public Unused Unused;

        internal static ushort FixedSizeOf =>
            sizeof(ushort)
            + sizeof(RipcFlags)
            + sizeof(RipcOpCode)
            + sizeof(byte)
            + sizeof(Unused);

        public override string ToString()
        {
            return $"MessageLength: {MessageLength}, Flags: {Flags}, OpCode: {OpCode}, HeaderLength: {HeaderLength}, Unused: {Unused}, FixedSizeOf: {RipcConnectionReply.FixedSizeOf}";
        }
    }
}

