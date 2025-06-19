/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Ema.Access.Tests;

internal class TestUtilities
{
    public static void eta_EncodeDictionaryMsg(Eta.Codec.DataDictionary dictionary)
    {
        Eta.Codec.Buffer buf = new Eta.Codec.Buffer();
        buf.Data(new Eta.Common.ByteBuffer(12300));
        Eta.Codec.IRefreshMsg msg = (Eta.Codec.IRefreshMsg)new Eta.Codec.Msg();
        Eta.Codec.EncodeIterator encodeIter = new Eta.Codec.EncodeIterator();
        Eta.Codec.Buffer nameBuf = new Eta.Codec.Buffer();
        Eta.Codec.Int dictionaryFid = new Eta.Codec.Int();
        dictionaryFid.Value(-32768);

        // load field and Type dictionaries
        dictionary.Clear();

        Assert.Equal(Eta.Codec.CodecReturnCode.SUCCESS,
            dictionary.LoadFieldDictionary(DataDictionaryTests.FIELD_DICTIONARY_FILENAME, out _));
        Assert.Equal(Eta.Codec.CodecReturnCode.SUCCESS,
            dictionary.LoadEnumTypeDictionary(DataDictionaryTests.ENUM_TABLE_FILENAME, out _));

        // set-up message
        msg.MsgClass = Eta.Codec.MsgClasses.REFRESH;
        msg.DomainType = (int)Eta.Rdm.DomainType.DICTIONARY;
        msg.ContainerType = Eta.Codec.DataTypes.SERIES;
        msg.State.StreamState(Eta.Codec.StreamStates.OPEN);
        msg.State.DataState(Eta.Codec.DataStates.OK);
        msg.State.Code(Eta.Codec.StateCodes.NONE);
        msg.State.Text().Data("Field Dictionary Refresh (starting fid " + dictionaryFid.ToLong() + ")");
        msg.ApplySolicited();
        msg.ApplyHasMsgKey();
        msg.MsgKey.Filter = Eta.Rdm.Dictionary.VerbosityValues.NORMAL;
        msg.MsgKey.ApplyHasName();
        msg.MsgKey.ApplyHasFilter();
        msg.MsgKey.ApplyHasServiceId();
        msg.MsgKey.ServiceId = 1;

        // DictionaryName
        nameBuf.Data("RWFFld");
        msg.MsgKey.Name = nameBuf;

        // StreamId
        msg.StreamId = 3;

        // clear encode iterator
        encodeIter.Clear();

        // set iterator buffer
        encodeIter.SetBufferAndRWFVersion(buf, Eta.Codec.Codec.MajorVersion(), Eta.Codec.Codec.MinorVersion());

        Assert.Equal(Eta.Codec.CodecReturnCode.ENCODE_CONTAINER, msg.EncodeInit(encodeIter, 0));

        // encode part of field dictionary (limited by buffer size)
        Assert.Equal(Eta.Codec.CodecReturnCode.DICT_PART_ENCODED, dictionary.EncodeFieldDictionary(encodeIter, dictionaryFid,
            Eta.Rdm.Dictionary.VerbosityValues.NORMAL, out _));

        Assert.Equal(Eta.Codec.CodecReturnCode.SUCCESS, msg.EncodeComplete(encodeIter, true));
    }
}
