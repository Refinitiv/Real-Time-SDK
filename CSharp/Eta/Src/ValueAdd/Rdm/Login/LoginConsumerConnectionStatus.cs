/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System.Diagnostics;
using System.Text;

using Refinitiv.Eta.Codec;
using Refinitiv.Eta.Rdm;

using Buffer = Refinitiv.Eta.Codec.Buffer;

namespace Refinitiv.Eta.ValueAdd.Rdm
{
    /// <summary>
    /// The RDM Login Consumer Connection Status. Used by an OMM Consumer to send Warm Standby information.
    /// </summary>
    public class LoginConsumerConnectionStatus : MsgBase
    {
        #region Private Fields

        private IGenericMsg m_GenericMsg = new Msg();
        private Map map = new();
        private MapEntry mapEntry = new();
        private Buffer tempBuffer = new();

        #endregion
        #region Public Message Properties

        public override int StreamId { get; set; }

        public override int DomainType { get => m_GenericMsg.DomainType; }

        public override int MsgClass { get => m_GenericMsg.MsgClass; }

        public LoginConsumerConnectionStatusFlags Flags { get; set; }

        /// <summary>
        /// Warm Standby Information. Presence indicated by <see cref="LoginConsumerConnectionStatusFlags.HAS_WARM_STANDBY_INFO"/>
        /// </summary>
        public LoginWarmStandByInfo WarmStandbyInfo { get; set; } = new LoginWarmStandByInfo();

        /// <summary>
        /// Checks the presence of Warm Standby Information.
        /// </summary>
        public bool HasWarmStandbyInfo
        {
            get => (Flags & LoginConsumerConnectionStatusFlags.HAS_WARM_STANDBY_INFO) != 0;
            set
            {
                if (value)
                    Flags |= LoginConsumerConnectionStatusFlags.HAS_WARM_STANDBY_INFO;
                else
                    Flags &= ~LoginConsumerConnectionStatusFlags.HAS_WARM_STANDBY_INFO;
            }
        }

        #endregion

        public LoginConsumerConnectionStatus()
        {
            Clear();
        }

        /// Performs a deep copy of this object into <c>destConnStatusMsg</c>.
        public CodecReturnCode Copy(LoginConsumerConnectionStatus destConnStatusMsg)
        {
            Debug.Assert(destConnStatusMsg != null);
            destConnStatusMsg.StreamId = StreamId;
            if (HasWarmStandbyInfo)
            {
                destConnStatusMsg.HasWarmStandbyInfo = true;
                destConnStatusMsg.WarmStandbyInfo.WarmStandbyMode = WarmStandbyInfo.WarmStandbyMode;
                destConnStatusMsg.WarmStandbyInfo.Action = WarmStandbyInfo.Action;
            }
            return CodecReturnCode.SUCCESS;
        }

        public override CodecReturnCode Encode(EncodeIterator EncodeIter)
        {
            // Encode ConsumerConnectionStatus Generic message.
            // Used to send the WarmStandbyMode.
            m_GenericMsg.Clear();
            m_GenericMsg.MsgClass = MsgClasses.GENERIC;
            m_GenericMsg.DomainType = (int)Eta.Rdm.DomainType.LOGIN;
            m_GenericMsg.StreamId = StreamId;
            m_GenericMsg.ContainerType = DataTypes.MAP;
            m_GenericMsg.Flags = (int)GenericMsgFlags.HAS_MSG_KEY;
            m_GenericMsg.MsgKey.Flags = (int)MsgKeyFlags.HAS_NAME;
            m_GenericMsg.MsgKey.Name = ElementNames.CONS_CONN_STATUS;

            CodecReturnCode ret = m_GenericMsg.EncodeInit(EncodeIter, 0);
            if (ret != CodecReturnCode.ENCODE_CONTAINER)
                return CodecReturnCode.FAILURE;

            map.Clear();
            map.Flags = MapFlags.NONE;
            map.KeyPrimitiveType = DataTypes.ASCII_STRING;
            map.ContainerType = DataTypes.ELEMENT_LIST;
            ret = map.EncodeInit(EncodeIter, 0, 0);
            if (ret != CodecReturnCode.SUCCESS)
                return CodecReturnCode.FAILURE;

            if (HasWarmStandbyInfo)
            {
                mapEntry.Clear();
                mapEntry.Flags = MapEntryFlags.NONE;
                mapEntry.Action = WarmStandbyInfo.Action;
                if (mapEntry.Action != MapEntryActions.DELETE)
                {
                    ret = mapEntry.EncodeInit(EncodeIter, ElementNames.WARMSTANDBY_INFO, 0);

                    ret = WarmStandbyInfo.Encode(EncodeIter);
                    if (ret != CodecReturnCode.SUCCESS)
                        return CodecReturnCode.FAILURE;

                    ret = mapEntry.EncodeComplete(EncodeIter, true);
                    if (ret != CodecReturnCode.SUCCESS)
                        return CodecReturnCode.FAILURE;
                }
                else
                {
                    ret = mapEntry.Encode(EncodeIter, ElementNames.WARMSTANDBY_INFO);
                    if (ret != CodecReturnCode.SUCCESS)
                        return CodecReturnCode.FAILURE;
                }

            }

            ret = map.EncodeComplete(EncodeIter, true);
            if (ret != CodecReturnCode.SUCCESS)
                return CodecReturnCode.FAILURE;

            ret = m_GenericMsg.EncodeComplete(EncodeIter, true);
            if (ret != CodecReturnCode.SUCCESS)
                return CodecReturnCode.FAILURE;

            return CodecReturnCode.SUCCESS;
        }

        /// <summary>
        /// Decode a ETA message into an RDM message.
        /// </summary>
        /// <param name="dIter"></param>
        /// <param name="msg"></param>
        /// <returns></returns>
        public override CodecReturnCode Decode(DecodeIterator dIter, Msg msg)
        {
            if (msg.MsgClass != MsgClasses.GENERIC || msg.DomainType != (int)Refinitiv.Eta.Rdm.DomainType.LOGIN)
                return CodecReturnCode.FAILURE;

            IMsgKey key = msg.MsgKey;

            if (key == null || !key.CheckHasName())
                return CodecReturnCode.FAILURE;

            if (!key.Name.Equals(ElementNames.CONS_CONN_STATUS))
            {
                // Unknown generic msg name
                return CodecReturnCode.FAILURE;
            }

            if (msg.ContainerType != DataTypes.MAP)
                return CodecReturnCode.FAILURE;

            Clear();
            StreamId = msg.StreamId;

            map.Clear();

            CodecReturnCode ret = map.Decode(dIter);
            if (ret != CodecReturnCode.SUCCESS)
                return CodecReturnCode.FAILURE;

            if (!(map.KeyPrimitiveType == DataTypes.ASCII_STRING || map.KeyPrimitiveType == DataTypes.BUFFER))
                return CodecReturnCode.FAILURE;

            if (map.ContainerType != DataTypes.ELEMENT_LIST)
                return CodecReturnCode.FAILURE;

            mapEntry.Clear();
            tempBuffer.Clear();
            while ((ret = mapEntry.Decode(dIter, tempBuffer)) != CodecReturnCode.END_OF_CONTAINER)
            {
                if (ret < CodecReturnCode.SUCCESS)
                    return CodecReturnCode.FAILURE;

                if (tempBuffer.Equals(ElementNames.WARMSTANDBY_INFO))
                {
                    HasWarmStandbyInfo = true;
                    if (mapEntry.Action != MapEntryActions.DELETE)
                    {
                        ret = WarmStandbyInfo.Decode(dIter, msg);
                        if (ret != CodecReturnCode.SUCCESS)
                            return CodecReturnCode.FAILURE;
                    }
                    WarmStandbyInfo.Action = mapEntry.Action;
                }
            }

            return CodecReturnCode.SUCCESS;
        }

        public override string ToString()
        {
            StringBuilder toStringBuilder = PrepareStringBuilder();
            toStringBuilder.Insert(0, "LoginConsumerConnectionStatus: \n");

            if (HasWarmStandbyInfo)
            {
                toStringBuilder.Append(tab);
                toStringBuilder.Append("warmStandbyInfo: ");
                toStringBuilder.Append(WarmStandbyInfo);
            }

            return toStringBuilder.ToString();
        }

        public override void Clear()
        {
            WarmStandbyInfo.Clear();
            m_GenericMsg.Clear();
            m_GenericMsg.MsgClass = MsgClasses.STATUS;
            m_GenericMsg.DomainType = (int)Refinitiv.Eta.Rdm.DomainType.LOGIN;
            m_GenericMsg.Flags = StatusMsgFlags.HAS_MSG_KEY;
            m_GenericMsg.MsgKey.Flags = MsgKeyFlags.HAS_NAME;
            m_GenericMsg.MsgKey.Name = ElementNames.CONS_CONN_STATUS;
            m_GenericMsg.ContainerType = DataTypes.MAP;
        }
    }
}
