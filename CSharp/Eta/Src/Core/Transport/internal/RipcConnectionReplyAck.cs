/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.Transports.Internal
{
   internal struct RipcConnectionReplyAck
   {
      public RipcConnectionReply RipcReply;

      public ushort MessageLength => (ushort)(RipcConnectionReplyAck.FixedSizeOf
                                   + (string.IsNullOrEmpty(ComponentVersionString) ? 0 : ComponentVersionString.Length));
      public static byte HeaderLength => (byte)(RipcConnectionReply.FixedSizeOf
                                              + sizeof(int)
                                       );

      public int RipcVersion;

      public short MaxUserMsgSize;

      public byte SessionFlags;

      public byte PingTimeout;

      public byte MajorVersion;

      public byte MinorVersion;

      public short ComponentType;

      public byte ComponentLevel;

      public byte ComponentVersionLength => (byte)(sizeof(byte) + (_componentVersionString??string.Empty).Length);

      public byte VersionStringLength => (byte)(_componentVersionString??string.Empty).Length;

      string _componentVersionString;
      public string ComponentVersionString
      {
         get => _componentVersionString;
         set
         {
            _componentVersionString
                  = value.Substring(0, (value.Length > RipcVersionInfo.ComponentVersionStringLenMax)
                                          ? RipcVersionInfo.ComponentVersionStringLenMax
                                          : value.Length);

            RipcReply.MessageLength = MessageLength;
            RipcReply.HeaderLength = HeaderLength;
         }
      }

      /// <summary>
      /// Size offixed portion of structure.
      /// </summary>
      internal static int FixedSizeOf =>
           RipcConnectionReply.FixedSizeOf
         + sizeof(int) // RipcVersion
         + sizeof(short) // MaxUserMshSize
         + sizeof(byte) * 9 // all of the rest
         ;

      public override string ToString()
      {
         return $"RipcReply: {RipcReply}, RipcVersion: {RipcVersion}, MaxUserMsgSize: {MaxUserMsgSize}, SessionFlags: {SessionFlags}, PingTimeout: {PingTimeout}, Version: {MajorVersion}.{MinorVersion}";
      }
   }
}
