/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.Transports.Internal
{
   internal struct RipcConnectionReplyNak
   {
      public RipcConnectionReply RipcReply;

      public ushort MessageLength => (byte)(RipcConnectionReply.FixedSizeOf
                                       + sizeof(ushort)// Text Length   
                                                       // Number of ASCII characters in the Text
                                       + (string.IsNullOrEmpty(Text) ? 0 : Text.Length)
                                       + sizeof(byte)  // NULL byte for C
                                         );
      public byte HeaderLength => (byte)(RipcConnectionReply.FixedSizeOf
                                      + sizeof(ushort)// Text Length   
                                                      // Number of ASCII characters in the Text
                                      + (string.IsNullOrEmpty(Text) ? 0 : Text.Length)
                                      + sizeof(byte)  // NULL byte for C
                                        );
      string _text;
      public string Text
      {
         get => _text;
         set
         {  // Fixup the Lengths when the Text is updated.
            _text = value;
            RipcReply.MessageLength = MessageLength;
            RipcReply.HeaderLength = HeaderLength;
         }
      }

      public override string ToString()
      {
         return $"{RipcReply}, Text: {Text}";
      }

      internal static int FixedSizeOf => RipcConnectionReply.FixedSizeOf;
   }
}
