/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Access;
using LSEG.Ema.Domain.Internal;
using LSEG.Ema.Rdm;
using System;
using System.Text;

namespace LSEG.Ema.Domain.Directory
{
    /// <summary>
    /// The RDM Service Link. Contains information about an upstream source
    /// associated with the service.
    /// </summary>
    public sealed class DirectoryServiceLink : DirectoryServiceFilter<ElementList, DirectoryServiceLink>, ICloneable
    {
        private OptionalField<string> m_Text = new();
        private OptionalField<LinkCode> m_LinkCode = new();
        private OptionalField<UpstreamSourceType> m_Type = new();
        private string m_Name = "";
        private bool m_IsLinkUp;

        /// <inheritdoc />
        public override int FilterId => EmaRdm.SERVICE_LINK_ID;

        /// <summary>
        /// Upstream source name. It's not a part of encoded data.
        /// </summary>
        public string Name() => m_Name;
        /// <summary>
        /// Upstream source name. It's not a part of encoded data.
        /// </summary>
        public DirectoryServiceLink Name(string value)
        {
            if (value == null)
                throw new OmmInvalidUsageException("name cannot be null", OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
            m_Name = value;
            return this;
        }

        /// <summary>
        /// Type of this service link.
        /// </summary>
        public UpstreamSourceType Type() => m_Type.Value;
        /// <summary>
        /// Type of this service link.
        /// </summary>
        public DirectoryServiceLink Type(UpstreamSourceType value)
        {
            if (!System.Enum.IsDefined(value))
                throw new OmmInvalidUsageException($"Invalid element value {value} of {EmaRdm.ENAME_TYPE}", OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
            m_Type.Value = value;
            return this;
        }

        /// <summary>
        /// Indicates presence of the link type field.
        /// </summary>
        public bool HasType => m_Type.HasValue;

        /// <summary>
        /// Flag indicating whether the source is up or down.
        /// </summary>
        public bool IsLinkUp()
        {
            return m_IsLinkUp;
        }
        /// <summary>
        /// Flag indicating whether the source is up or down.
        /// </summary>
        public DirectoryServiceLink IsLinkUp(bool value)
        {
            m_IsLinkUp = value;
            return this;
        }

        /// <summary>
        /// Code indicating additional information about the status of the source.
        /// </summary>
        public LinkCode LinkCode() => m_LinkCode.Value;
        /// <summary>
        /// Code indicating additional information about the status of the source.
        /// </summary>
        public DirectoryServiceLink LinkCode(LinkCode value)
        {
            if (!System.Enum.IsDefined(value))
                throw new OmmInvalidUsageException($"Invalid element value {value} of {EmaRdm.ENAME_LINK_CODE}", OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
            m_LinkCode.Value = value;
            return this;
        }

        /// <summary>
        /// Indicates presence of the link code field.
        /// </summary>
        public bool HasLinkCode => m_LinkCode.HasValue;

        /// <summary>
        /// Text further describing the state provided by the linkState and
        /// linkCode members.
        /// </summary>
        public string Text() => m_Text.Value;
        /// <summary>
        /// Text further describing the state provided by the linkState and
        /// linkCode members.
        /// </summary>
        public DirectoryServiceLink Text(string value)
        {
            if (value == null)
                throw new OmmInvalidUsageException("text can not be null", OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
            m_Text.Value = value;
            return this;
        }

        /// <summary>
        /// Indicates presence of the link text field.
        /// </summary>
        public bool HasText => m_Text.HasValue;

        /// <inheritdoc />
        public override DirectoryServiceLink Clear()
        {
            base.Clear();
            m_Name = "";
            m_Type.Clear();
            m_IsLinkUp = false;
            m_LinkCode.Clear();
            m_Text.Clear();
            return this;
        }

        /// <summary>
        /// Creates a copy of this instance.
        /// </summary>
        /// <returns>A new <see cref="DirectoryServiceLink"/> instance that is a copy of this instance.</returns>
        public DirectoryServiceLink Clone() => new DirectoryServiceLink().CopyFrom(this);

        /// <inheritdoc />
        object ICloneable.Clone() => Clone();

        /// <inheritdoc />
        public override DirectoryServiceLink CopyFrom(DirectoryServiceLink source)
        {
            base.CopyFrom(source);
            m_Name = source.m_Name;
            m_Type.CopyFrom(source.m_Type);
            m_IsLinkUp = source.m_IsLinkUp;
            m_LinkCode.CopyFrom(source.m_LinkCode);
            m_Text.CopyFrom(source.m_Text);
            return this;
        }

        internal override void AppendToString(StringBuilder sb, int indent)
        {
            base.AppendToString(sb, indent);
            sb.AddIndent(indent).AppendLine($"Name: {m_Name}");
            if (HasType)
            {
                sb.AddIndent(indent).AppendLine($"Type: {m_Type}");
            }
            sb.AddIndent(indent).AppendLine($"IsLinkUp: {m_IsLinkUp}");
            if (HasLinkCode)
            {
                sb.AddIndent(indent).AppendLine($"LinkCode: {m_LinkCode}");
            }
            if (HasText)
            {
                sb.AddIndent(indent).AppendLine($"Text: {m_Text}");
            }
        }

        internal override void DecodeFrom(ElementList elementList)
        {
            Clear();

            var foundLinkState = false;
            foreach (var elementEntry in elementList)
            {
                var elementName = elementEntry.Name;

                switch (elementName)
                {
                    case EmaRdm.ENAME_TYPE:
                        var type = elementEntry.UIntValue();
                        Type((UpstreamSourceType)type);
                        break;
                    case EmaRdm.ENAME_LINK_STATE:
                        var linkState = elementEntry.UIntValue();
                        if (linkState is EmaRdm.SERVICE_DOWN or EmaRdm.SERVICE_UP)
                        {
                            IsLinkUp(linkState == EmaRdm.SERVICE_UP);
                        }
                        else
                        {
                            Clear();
                            throw new OmmInvalidUsageException($"Invalid element value of {elementName}", OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
                        }
                        foundLinkState = true;
                        break;
                    case EmaRdm.ENAME_LINK_CODE:
                        var linkCode = elementEntry.UIntValue();
                        LinkCode((LinkCode)linkCode);
                        break;
                    case EmaRdm.ENAME_TEXT:
                        if (elementEntry.Code != Data.DataCode.BLANK)
                        {
                            Text(elementEntry.OmmAsciiValue().Value);
                        }
                        break;
                    default:
                        break;
                }
            }

            if (!foundLinkState)
            {
                Clear();
                throw new OmmInvalidUsageException(
                        $"{EmaRdm.ENAME_LINK_STATE} element is absent",
                        OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
            }
        }

        internal override void EncodeTo(ElementList elementList)
        {
            if (HasType)
            {
                elementList.AddUInt(EmaRdm.ENAME_TYPE, (ulong)m_Type.Value);
            }

            elementList.AddUInt(EmaRdm.ENAME_LINK_STATE, IsLinkUp() ? 1UL : 0UL);

            if (HasLinkCode)
            {
                elementList.AddUInt(EmaRdm.ENAME_LINK_CODE, (ulong)m_LinkCode.Value);
            }

            if (HasText)
            {
                elementList.AddAscii(EmaRdm.ENAME_TEXT, m_Text.Value);
            }
        }
    }

    /// <summary>
    /// Indicates whether the upstream source is interactive or broadcast. This does
    /// not describe whether the service itself is interactive or broadcast.
    /// </summary>
    public enum UpstreamSourceType
    {
        /// <summary>
        /// Upstream source is interactive
        /// </summary>
        INTERACTIVE = 1,
        /// <summary>
        /// Upstream source is broadcast
        /// </summary>
        BROADCAST = 2,
    }

    /// <summary>
    /// Provides additional status information about the upstream source
    /// </summary>
    public enum LinkCode
    {
        /// <summary>
        /// None
        /// </summary>
        NONE = 0,
        /// <summary>
        /// Ok
        /// </summary>
        OK = 1,
        /// <summary>
        /// Recovery started
        /// </summary>
        RECOVERY_STARTED = 2,
        /// <summary>
        /// Recovery completed
        /// </summary>
        RECOVERY_COMPLETED = 3,
    }
}