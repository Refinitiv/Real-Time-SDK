/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

using Refinitiv.Eta.Common;
using Refinitiv.Eta.Transports.Internal;

namespace Refinitiv.Eta.Transports
{
    internal class BigTransportBuffer : TransportBuffer 
    {
        private static ushort s_fragmentID = 0;
        internal ushort FragmentID { get; private set; }

        internal TransportBuffer FirstBuffer { get; set; }

        public override int Length => Data.WritePosition;

        public override int Capacity => Data.Capacity;

        public override int DataStartPosition => Data.ReadPosition;

        internal override bool IsRipcMessage
        {
            get
            {
                return false;
            }
        }

        internal bool IsWritePaused { get; set; }

        internal RipcVersions RipcVersion { get; private set; }

        internal int FirstFragmentHeaderLength { get; private set; }

        internal int NextFragmentHeaderLength { get; private set; }

        public BigTransportBuffer(int size)
        {
            Data = new ByteBuffer(size);
            NextID(); // Get next fragmented ID for this buffer
            IsBigBuffer = true;
            IsWritePaused = false;
        }

        internal override void Clear()
        {
            Data = null;
            IsBigBuffer = true;
        }

        void NextID()
        {
            ++s_fragmentID;

            if(s_fragmentID == 0)
            {
                s_fragmentID = 1;
            }

            FragmentID = s_fragmentID;
        }

        internal void SetRipcVersion(RipcVersions ripcVersion)
        {
            RipcVersion = ripcVersion;

            if(ripcVersion >= RipcVersions.VERSION13)
            {
                FirstFragmentHeaderLength = 10;
                NextFragmentHeaderLength = 6;
            }
            else
            {
                FirstFragmentHeaderLength = 9;
                NextFragmentHeaderLength = 5;
            }
        }

        public override void ReturnToPool()
        {

        }

    }
}
