/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.Transports.Internal
{
   internal struct RipcConnectionRequest
   {
      public short MessageLength => (short)(FixedSizeOf
                                      + RipcCompressionMapSize
                                      + HostNameLength
                                      + IPAddressLength
                                      + VersionStringLength
                                    );

      public RipcFlags Flags;

      public int ConnectionVersion;

      public Unused FlagsEx => default(Unused);

      public byte HeaderLength => (byte)(HeaderFixedSizeOf
                                        + RipcCompressionMapSize
                                        + HostNameLength
                                        + IPAddressLength);

      internal byte RipcCompressionMapSize => 0;

      public byte[] RipcCompressionMap;

      public byte PingTimeout;

      public RipcSessionFlags SessionFlags;

      public ProtocolType ProtocolType;

      public byte MajorVersion;

      public byte MinorVersion;

      internal byte HostNameLength => (byte)((HostName ?? string.Empty).Length);

      public string HostName;

      internal byte IPAddressLength => (byte)((IPAddress ?? string.Empty).Length);

      public string IPAddress;

      public byte ComponentVersionLength => (byte)(sizeof(byte) 
                                                + (ComponentVersionString??string.Empty).Length);

      public byte VersionStringLength => (byte)(ComponentVersionString??string.Empty).Length;

      string _componentVersionString;
      public string ComponentVersionString
      {
         get => _componentVersionString;
          set => _componentVersionString 
              = value.Substring(0, (value.Length > RipcVersionInfo.ComponentVersionStringLenMax)
                  ? RipcVersionInfo.ComponentVersionStringLenMax
                  : value.Length);
      }

      internal static ushort FixedSizeOf =>
           sizeof(ushort)        // MessageLength
         + sizeof(RipcFlags)     // RipcFlags
         + sizeof(int)         // ConnectionVersion
         + sizeof(byte) * 10     // Unused, HeaderLength, RipcCompBitmapLength,
                                 // PingTimeout, SessopnFlags, ProtocolType, 
                                 // MajorVersion, MinorVersion, HostNameLength,
                                 // IPAddressLength
         + sizeof(byte) * 2      // ComponentVersionLength, VersionStringLength
         ;

      internal static ushort HeaderFixedSizeOf =>
           sizeof(ushort)        // MessageLength
         + sizeof(RipcFlags)     // RipcFlags
         + sizeof(int)         // ConnectionVersion
         + sizeof(byte) * 10     // Unused, HeaderLength, RipcCompBitmapLength,
                                 // PingTimeout, SessopnFlags, ProtocolType, 
                                 // MajorVersion, MinorVersion, HostNameLength,
                                 // IPAddressLength
           ;  

      public override string ToString()
      {
         return $"MessageLength: {MessageLength}, Flags: {Flags}, ConnectionVersion: {ConnectionVersion}, HeaderLength: {HeaderLength}, RipcCompressionMapSize: {RipcCompressionMapSize}, RipcCompressionMap: {RipcCompressionMap}, PingTimeout: {PingTimeout}, MajorVersion: {MajorVersion}, MinorVersion: {MinorVersion}, HostName: {HostName}, IPAddress: {IPAddress} ComponentVersionString: {ComponentVersionString}";
      }
   }


}
