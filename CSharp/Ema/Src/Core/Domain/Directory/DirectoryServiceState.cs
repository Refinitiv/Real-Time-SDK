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
    /// The RDM Service State. Contains information provided by the Source Directory State filter.
    /// </summary>
    public sealed class DirectoryServiceState : DirectoryServiceFilter<ElementList, DirectoryServiceState>, ICloneable
    {
        private OptionalField<bool> m_AcceptingRequests = new();
        private StateField m_Status = new();
        private bool m_IsServiceUp;

        /// <inheritdoc />
        public override int FilterId => EmaRdm.SERVICE_STATE_ID;

        /// <summary>
        /// The state of the service.
        /// </summary>
        /// <returns>Returns true if the service is up, false otherwise.</returns>
        public bool IsServiceUp() => m_IsServiceUp;
        /// <summary>
        /// The state of the service.
        /// </summary>
        /// <param name="value">the value to set</param>
        /// <returns>The current instance to support method chaining.</returns>
        public DirectoryServiceState IsServiceUp(bool value)
        {
            m_IsServiceUp = value;
            return this;
        }

        /// <summary>
        /// Flag indicating whether the service is accepting item requests.
        /// </summary>
        /// <returns>Returns true if the service is accepting item requests, false otherwise.</returns>
        public bool AcceptingRequests() => m_AcceptingRequests.Value;
        /// <summary>
        /// Flag indicating whether the service is accepting item requests.
        /// </summary>
        /// <param name="value">the value to set</param>
        /// <returns>The current instance to support method chaining.</returns>
        public DirectoryServiceState AcceptingRequests(bool value)
        {
            m_AcceptingRequests.Value = value;
            return this;
        }

        /// <summary>
        /// Gets presence of the acceptingRequests field.
        /// </summary>
        public bool HasAcceptingRequests => m_AcceptingRequests.HasValue;

        /// <summary>
        /// Status to be applied to all items being provided by this service.
        /// </summary>
        /// <returns>Returns status to be applied to all items being provided by this service.</returns>
        public OmmState Status() => m_Status.Value();
        /// <summary>
        /// Sets status to be applied to all items being provided by this service.
        /// </summary>
        /// <param name="value">the value to set</param>
        /// <returns>The current instance to support method chaining.</returns>
        public DirectoryServiceState Status(OmmState value)
        {
            m_Status.Value(value);
            return this;
        }
        /// <summary>
        /// Sets status to be applied to all items being provided by this service.
        /// </summary>
        /// <param name="streamState">the stream state to set</param>
        /// <param name="dataState">the data state to set</param>
        /// <param name="statusCode">the status code to set</param>
        /// <param name="statusText">the status text to set</param>
        /// <returns>The current instance to support method chaining.</returns>
        public DirectoryServiceState Status(int streamState, int dataState, int statusCode, string statusText)
        {
            m_Status.Value(streamState, dataState, statusCode, statusText);
            return this;
        }

        /// <summary>
        /// Gets flag that indicates presence of the status field.
        /// </summary>
        public bool HasStatus => m_Status.HasValue;

        /// <inheritdoc />
        public override DirectoryServiceState Clear()
        {
            base.Clear();
            m_IsServiceUp = false;
            m_AcceptingRequests.Clear();
            m_Status.Clear();
            return this;
        }

        /// <summary>
        /// Creates a copy of this instance.
        /// </summary>
        /// <returns>A new <see cref="DirectoryServiceState"/> instance that is a copy of this instance.</returns>
        public DirectoryServiceState Clone() => new DirectoryServiceState().CopyFrom(this);

        /// <inheritdoc />
        object ICloneable.Clone() => Clone();

        /// <inheritdoc />
        public override DirectoryServiceState CopyFrom(DirectoryServiceState source)
        {
            base.CopyFrom(source);
            m_IsServiceUp = source.m_IsServiceUp;
            m_AcceptingRequests.CopyFrom(source.m_AcceptingRequests);
            m_Status.CopyFrom(source.m_Status);
            return this;
        }

        internal override void AppendToString(StringBuilder sb, int indent)
        {
            base.AppendToString(sb, indent);
            sb.AddIndent(indent).AppendLine($"IsServiceUp: {m_IsServiceUp}");
            if (HasAcceptingRequests)
            {
                sb.AddIndent(indent).AppendLine($"AcceptingRequests: {m_AcceptingRequests.Value}");
            }
            if (HasStatus)
            {
                sb.AddIndent(indent).AppendLine($"Status: {m_Status}");
            }
        }

        internal override void DecodeFrom(ElementList elementList)
        {
            Clear();

            var foundServiceState = false;
            foreach (var elementEntry in elementList)
            {
                var elementName = elementEntry.Name;

                switch (elementName)
                {
                    case EmaRdm.ENAME_SVC_STATE:
                        var serviceState = elementEntry.UIntValue();
                        if (serviceState == EmaRdm.SERVICE_DOWN || serviceState == EmaRdm.SERVICE_UP)
                        {
                            IsServiceUp(serviceState == EmaRdm.SERVICE_UP);
                        }
                        else
                        {
                            Clear();
                            throw new OmmInvalidUsageException($"Invalid element value {serviceState} of {elementName}", OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
                        }
                        foundServiceState = true;
                        break;
                    case EmaRdm.ENAME_ACCEPTING_REQS:
                        var acceptingRequests = elementEntry.UIntValue();
                        if (acceptingRequests == 0 || acceptingRequests == 1)
                        {
                            AcceptingRequests(acceptingRequests == 1);
                        }
                        else
                        {
                            Clear();
                            throw new OmmInvalidUsageException($"Invalid element value {acceptingRequests} of {elementName}", OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
                        }
                        break;
                    case EmaRdm.ENAME_STATUS:
                        if (elementEntry.Code != Data.DataCode.BLANK)
                        {
                            var status = elementEntry.OmmStateValue();
                            if (status.StreamState is OmmState.StreamStates.OPEN or OmmState.StreamStates.CLOSED_RECOVER)
                            {
                                Status(status);
                            }
                            else
                            {
                                Clear();
                                throw new OmmInvalidUsageException($"Invalid element value {status} of {elementName}", OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
                            }
                        }
                        break;
                    default:
                        break;
                }
            }

            if (!foundServiceState)
            {
                Clear();
                throw new OmmInvalidUsageException($"{EmaRdm.ENAME_SVC_STATE} element is absent", OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
            }
        }

        internal override void EncodeTo(ElementList elementList)
        {
            elementList.AddUInt(EmaRdm.ENAME_SVC_STATE, (ulong)(m_IsServiceUp ? EmaRdm.SERVICE_UP : EmaRdm.SERVICE_DOWN));

            if (HasAcceptingRequests)
            {
                elementList.AddUInt(EmaRdm.ENAME_ACCEPTING_REQS, m_AcceptingRequests.Value ? 1ul : 0ul);
            }

            if (HasStatus)
            {
                var state = Status();
                elementList.AddState(EmaRdm.ENAME_STATUS, state.StreamState, state.DataState, state.StatusCode, state.StatusText);
            }
        }
    }
}