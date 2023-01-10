/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using LSEG.Eta.Rdm;
using System.Diagnostics;
using System.Text;
using static LSEG.Eta.Rdm.Directory;

namespace LSEG.Eta.ValueAdd.Rdm
{
    /// <summary>
    /// The RDM Service State. Contains information provided by the Source Directory State filter.
    /// </summary>
    public class ServiceState
    {
        private long m_AcceptingRequests;
        private State m_Status;

        private ElementList m_ElementList = new ElementList();
        private ElementEntry m_Element = new ElementEntry();

        private UInt m_TmpUInt = new UInt();
        private State m_TmpStatus = new State();

        private StringBuilder m_StringBuf = new StringBuilder();
        private const string eol = "\n";
        private const string tab = "\t";

        /// <summary>
        /// Action associated with this Service Info.
        /// </summary>
        public FilterEntryActions Action { get; set; }

        /// <summary>
        /// The service state flags. It is populated by <see cref="ServiceStateFlags"/>
        /// </summary>
        public ServiceStateFlags Flags { get; set; }

        /// <summary>
        /// Indicates the presence of the Status field.
        /// </summary>
        public bool HasStatus { get => (Flags & ServiceStateFlags.HAS_STATUS) != 0; set { if (value) Flags |= ServiceStateFlags.HAS_STATUS; else Flags &= ~ServiceStateFlags.HAS_STATUS; } }

        /// <summary>
        /// Indicates the presence of the AcceptingRequests field.
        /// </summary>
        public bool HasAcceptingRequests { get => (Flags & ServiceStateFlags.HAS_ACCEPTING_REQS) != 0; set { if (value) Flags |= ServiceStateFlags.HAS_ACCEPTING_REQS; else Flags &= ~ServiceStateFlags.HAS_ACCEPTING_REQS; } }

        /// <summary>
        /// Flag indicating whether the service is accepting item requests.
        /// </summary>
        public long AcceptingRequests { get => m_AcceptingRequests; set { Debug.Assert(HasAcceptingRequests); m_AcceptingRequests = value; } }

        /// <summary>
        /// Status of this service.
        /// </summary>
        public State Status { 
            get => m_Status; 
            set
            {
                Debug.Assert(HasStatus && value != null);
                value.Copy(m_Status);
            }
        }

        /// <summary>
        /// The current state of the service.
        /// </summary>
        public long ServiceStateVal { get; set; }

        /// <summary>
        /// The filterId.
        /// </summary>
        public int FilterId { get => ServiceFilterIds.STATE; }

        /// <summary>
        /// Performs an update of ServiceState object.
        /// </summary>
        /// <param name="destServiceState">ServiceLoad object to update with information from this object. It cannot be null.</param>
        /// <returns><see cref="CodecReturnCode"/> value indicating the status of the operation.</returns>
        public CodecReturnCode Update(ServiceState destServiceState)
        {
            Debug.Assert(destServiceState != null);
            destServiceState.Flags = Flags;
            destServiceState.Action = Action;
            destServiceState.ServiceStateVal = ServiceStateVal;

            if (HasAcceptingRequests)
            {
                destServiceState.HasAcceptingRequests = true;
                destServiceState.AcceptingRequests = AcceptingRequests;
            }

            if (HasStatus)
            {
                destServiceState.HasStatus = true;
                destServiceState.Status = Status;
            }

            return CodecReturnCode.SUCCESS;
        }

        /// <summary>
        /// Service State constructor.
        /// </summary>
        public ServiceState()
        {
            m_Status = new State();
            Clear();
        }

        /// <summary>
        /// Clears the current contents of the service state object and prepares it for re-use.
        /// </summary>
        public void Clear()
        {
            Flags = 0;
            m_Status.Clear();
            m_Status.StreamState(StreamStates.OPEN);
            m_Status.Code(StateCodes.NONE);
            m_Status.DataState(DataStates.OK);
            Action = FilterEntryActions.SET;
            ServiceStateVal = (long)ServiceStates.UP;
            m_AcceptingRequests = 1;
        }

        /// <summary>
        /// Performs a deep copy of this object into <c>destServiceState</c>.
        /// </summary>
        /// <param name="destServiceState">ServiceState object that will have this object's information copied into.</param>
        /// <returns><see cref="CodecReturnCode"/> indicating success or failure.</returns>
        public CodecReturnCode Copy(ServiceState destServiceState)
        {
            Debug.Assert(destServiceState != null);
            destServiceState.Clear();
            destServiceState.Flags = Flags;
            destServiceState.Action = Action;
            destServiceState.ServiceStateVal  = ServiceStateVal;

            if (HasAcceptingRequests)
            {
                destServiceState.HasAcceptingRequests = true;
                destServiceState.AcceptingRequests = AcceptingRequests;
            }

            if (HasStatus)
            {
                destServiceState.HasStatus = true;
                destServiceState.Status = Status;
            }

            return CodecReturnCode.SUCCESS;
        }

        /// <summary>
        /// Encodes this Service State message using the provided <c>encodeIter</c>.
        /// </summary>
        /// <param name="encodeIter">Encode iterator that has a buffer set to encode into.</param>
        /// <returns><see cref="CodecReturnCode"/> indicating success or failure.</returns>
        public CodecReturnCode Encode(EncodeIterator encodeIter)
        {
            m_ElementList.Clear();
            m_ElementList.ApplyHasStandardData();
            CodecReturnCode ret = m_ElementList.EncodeInit(encodeIter, null, 0);
            if (ret != CodecReturnCode.SUCCESS)
                return ret;
            m_Element.Clear();
            m_Element.Name = ElementNames.SVC_STATE;
            m_Element.DataType = Codec.DataTypes.UINT;
            m_TmpUInt.Value(ServiceStateVal);
            ret = m_Element.Encode(encodeIter, m_TmpUInt);
            if (ret != CodecReturnCode.SUCCESS)
                return ret;

            if (HasAcceptingRequests)
            {
                m_Element.Clear();
                m_Element.Name = ElementNames.ACCEPTING_REQS;
                m_Element.DataType = Codec.DataTypes.UINT;
                m_TmpUInt.Value(AcceptingRequests);

                ret = m_Element.Encode(encodeIter, m_TmpUInt);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;
            }

            if (HasStatus)
            {
                m_Element.Clear();
                m_Element.Name = ElementNames.STATUS;
                m_Element.DataType = Codec.DataTypes.STATE;
                ret = m_Element.Encode(encodeIter, Status);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;
            }

            ret = m_ElementList.EncodeComplete(encodeIter, true);
            if (ret < CodecReturnCode.SUCCESS)
                return ret;
            return CodecReturnCode.SUCCESS;
        }

        /// <summary>
        /// Decodes this Service State using the provided <c>decodeIter</c>.
        /// </summary>
        /// <param name="decodeIter">Decode iterator that has already decoded the initial message.</param>
        /// <returns><see cref="CodecReturnCode"/> indicating success or failure.</returns>
        public CodecReturnCode Decode(DecodeIterator decodeIter)
        {
            m_ElementList.Clear();
            m_Element.Clear();
            CodecReturnCode ret = m_ElementList.Decode(decodeIter, null);
            if (ret < CodecReturnCode.SUCCESS)
            {
                return ret;
            }
            bool foundServiceState = false;
            //Decode element list elements
            while ((ret = m_Element.Decode(decodeIter)) != CodecReturnCode.END_OF_CONTAINER)
            {
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;

                //ServiceState
                if (m_Element.Name.Equals(ElementNames.SVC_STATE))
                {
                    ret = m_TmpUInt.Decode(decodeIter);
                    if (ret != CodecReturnCode.SUCCESS
                            && ret != CodecReturnCode.BLANK_DATA)
                    {
                        return ret;
                    }
                    ServiceStateVal = m_TmpUInt.ToLong();
                    foundServiceState = true;
                }
                //AcceptingRequests 
                else if (m_Element.Name.Equals(ElementNames.ACCEPTING_REQS))
                {
                    ret = m_TmpUInt.Decode(decodeIter);
                    if (ret != CodecReturnCode.SUCCESS
                            && ret != CodecReturnCode.BLANK_DATA)
                    {
                        return ret;
                    }
                    HasAcceptingRequests = true;
                    AcceptingRequests = m_TmpUInt.ToLong();
                }
                //Status
                else if (m_Element.Name.Equals(ElementNames.STATUS))
                {
                    ret = m_TmpStatus.Decode(decodeIter);
                    if (ret != CodecReturnCode.SUCCESS
                            && ret != CodecReturnCode.BLANK_DATA)
                    {
                        return ret;
                    }
                    HasStatus = true;
                    Status = m_TmpStatus;
                }
            }

            if (!foundServiceState)
                return CodecReturnCode.FAILURE;

            return CodecReturnCode.SUCCESS;
        }

        /// <summary>
        /// Returns a human readable string representation of the Service State.
        /// </summary>
        /// <returns>String containing the string representation.</returns>
        public override string ToString()
        {
            m_StringBuf.Clear();
            m_StringBuf.Append(tab);
            m_StringBuf.Append(tab);
            m_StringBuf.Append("StateFilter:");
            m_StringBuf.Append(eol);

            m_StringBuf.Append(tab);
            m_StringBuf.Append(tab);
            m_StringBuf.Append(tab);
            m_StringBuf.Append("ServiceState: ");
            m_StringBuf.Append(ServiceStateVal);
            m_StringBuf.Append(eol);

            if (HasAcceptingRequests)
            {
                m_StringBuf.Append(tab);
                m_StringBuf.Append(tab);
                m_StringBuf.Append(tab);
                m_StringBuf.Append("AcceptingRequests: ");
                m_StringBuf.Append(AcceptingRequests);
                m_StringBuf.Append(eol);
            }

            if (HasStatus)
            {
                m_StringBuf.Append(tab);
                m_StringBuf.Append(tab);
                m_StringBuf.Append(tab);
                m_StringBuf.Append(Status);
                m_StringBuf.Append(eol);
            }

            return m_StringBuf.ToString();
        }
    }
}
