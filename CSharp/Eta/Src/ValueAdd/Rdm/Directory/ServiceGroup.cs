/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using LSEG.Eta.Rdm;
using System.Diagnostics;
using System.Text;
using static LSEG.Eta.Rdm.Directory;
using Buffer = LSEG.Eta.Codec.Buffer;

namespace LSEG.Eta.ValueAdd.Rdm
{
    /// <summary>
    /// The RDM Service Group State. 
    /// Contains information provided by the Source Directory Group filter.
    /// </summary>
    sealed public class ServiceGroup
    {
        private Buffer _group;
        private Buffer _mergedToGroup;
        private State _status;
        private FilterEntryActions _action;

        private const string eol = "\n";
        private const string tab = "\t";

        private StringBuilder stringBuf = new StringBuilder();
        private ElementList elementList = new ElementList();
        private ElementEntry element = new ElementEntry();
        private State tmpStatus = new State();

        /// <summary>
        /// Checks the presence of the status field.
        /// </summary>
        public bool HasStatus { get => (Flags & ServiceGroupFlags.HAS_STATUS) != 0; set { if (value) Flags |= ServiceGroupFlags.HAS_STATUS; else Flags &= ~(ServiceGroupFlags.HAS_STATUS); } }

        /// <summary>
        /// Checks the presence of the MergedToGroup field.
        /// </summary>
        public bool HasMergedToGroup { get => (Flags & ServiceGroupFlags.HAS_MERGED_TO_GROUP) != 0; set { if (value) Flags |= ServiceGroupFlags.HAS_MERGED_TO_GROUP; else Flags &= ~(ServiceGroupFlags.HAS_MERGED_TO_GROUP); } }

        /// <summary>
        /// The status field for service group.
        /// </summary>
        public State Status { 
            get => _status; 
            set 
            { 
                Debug.Assert(HasStatus);
                value.Copy(_status);
            }  
        }

        /// <summary>
        /// The mergedToGroup for this service to the user specified buffer.
        /// Buffer used by this object's mergedToGroup field will be set to passed 
        /// in buffer's data and position. Note that this creates garbage if buffer is backed by String object.
        /// </summary>
        public Buffer MergedToGroup { get => _mergedToGroup; set { Debug.Assert(HasMergedToGroup); BufferHelper.CopyBuffer(value, _mergedToGroup); } } 

        /// <summary>
        /// The service group flags. Populate with <see cref="ServiceGroupFlags"/>
        /// </summary>
        public ServiceGroupFlags Flags { get; set; }

        /// <summary>
        /// The Item Group name associated with this status.
        /// </summary>
        public Buffer Group { get => _group; set { BufferHelper.CopyBuffer(value, _group); } }

        /// <summary>
        /// The action associated with group state filter.
        /// </summary>
        public FilterEntryActions Action { get => _action; set { _action = value; } }

        /// <summary>
        /// The filterId - Populated by <see cref="ServiceFilterIds"/>
        /// </summary>
        public int FilterId { get => ServiceFilterIds.GROUP;  }

        /// <summary>
        /// Directory Status Message constructor.
        /// </summary>
        public ServiceGroup()
        {
            _status = new State();
            _group = new Buffer();
            _mergedToGroup = new Buffer();
            Clear();
        }

        /// <summary>
        /// Clears the current contents of the Directory Service Group object and prepares it for re-use.
        /// </summary>
        public void Clear()
        {
            Flags = 0;
            _group.Clear();
            _mergedToGroup.Clear();
            _status.Clear();
            _status.StreamState(StreamStates.OPEN);
            _status.Code(StateCodes.NONE);
            _status.DataState(DataStates.OK);
            _action = FilterEntryActions.SET;
        }

        /// <summary>
        /// Performs a deep copy of this object into <c>destServiceGroup</c>.
        /// </summary>
        /// <param name="destServiceGroup">ServiceGroup object that will have this object's information copied into.</param>
        /// <returns><see cref="CodecReturnCode"/> indicating success or failure.</returns>
        public CodecReturnCode Copy(ServiceGroup destServiceGroup)
        {
            Debug.Assert(destServiceGroup != null);

            destServiceGroup.Clear();
            destServiceGroup.Flags = Flags;
            destServiceGroup.Action = Action;
            if (HasMergedToGroup)
            {
                destServiceGroup.HasMergedToGroup = true;
                BufferHelper.CopyBuffer(MergedToGroup, destServiceGroup.MergedToGroup);
            }

            if (HasStatus)
            {
                destServiceGroup.HasStatus = true;
                destServiceGroup.Status = Status;
            }
            BufferHelper.CopyBuffer(Group, destServiceGroup.Group);

            return CodecReturnCode.SUCCESS;
        }

        /// <summary>
        /// Encodes this Directory Service Group using the provided <c>encodeIter</c>.
        /// </summary>
        /// <param name="encodeIter">Encode iterator that has a buffer set to encode into.</param>
        /// <returns><see cref="CodecReturnCode"/> indicating success or failure.</returns>
        public CodecReturnCode Encode(EncodeIterator encodeIter)
        {
            elementList.Clear();
            elementList.ApplyHasStandardData();
            CodecReturnCode ret = elementList.EncodeInit(encodeIter, null, 0);
            if (ret != CodecReturnCode.SUCCESS)
                return ret;
            element.Clear();

            if (HasMergedToGroup)
            {
                element.Clear();
                element.Name = ElementNames.MERG_TO_GRP;
                element.DataType = Codec.DataTypes.BUFFER;
                ret = element.Encode(encodeIter, MergedToGroup);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;
            }

            if (HasStatus)
            {
                element.Clear();
                element.Name = ElementNames.STATUS;
                element.DataType = Codec.DataTypes.STATE;
                ret = element.Encode(encodeIter, Status);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;
            }

            element.Clear();
            element.Name = ElementNames.GROUP;
            element.DataType = Codec.DataTypes.BUFFER;
            ret = element.Encode(encodeIter, Group);
            if (ret != CodecReturnCode.SUCCESS)
                return ret;

            ret = elementList.EncodeComplete(encodeIter, true);
            if (ret < CodecReturnCode.SUCCESS)
                return ret;

            return CodecReturnCode.SUCCESS;
        }

        /// <summary>
        /// Decodes this Directory Service Group using the provided <c>decodeIter</c>.
        /// </summary>
        /// <param name="decodeIter">Decode iterator that has already decoded the initial message.</param>
        /// <returns><see cref="CodecReturnCode"/> indicating success or failure.</returns>
        public CodecReturnCode Decode(DecodeIterator decodeIter)
        {
            Clear();
            elementList.Clear();
            element.Clear();

            CodecReturnCode ret = elementList.Decode(decodeIter, null);
            if (ret != CodecReturnCode.SUCCESS)
                return ret;

            //decode element list elements
            while ((ret = element.Decode(decodeIter)) != CodecReturnCode.END_OF_CONTAINER)
            {
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;

                //Group
                if (element.Name.Equals(ElementNames.GROUP))
                {
                    Group = element.EncodedData;
                }
                //MergedToGroup
                else if (element.Name.Equals(ElementNames.MERG_TO_GRP))
                {
                    HasMergedToGroup = true;
                    MergedToGroup = element.EncodedData;                    
                }
                //Status
                else if (element.Name.Equals(ElementNames.STATUS))
                {
                    ret = tmpStatus.Decode(decodeIter);
                    if (ret != CodecReturnCode.SUCCESS && ret != CodecReturnCode.BLANK_DATA)
                    {
                        return ret;
                    }
                    HasStatus = true;
                    Status = tmpStatus;
                }

            }

            return CodecReturnCode.SUCCESS;
        }

        /// <summary>
        /// Returns a human readable string representation of the Service Group Info.
        /// </summary>
        /// <returns>String containing the string representation.</returns>
        public override string ToString()
        {
            stringBuf.Clear();
            stringBuf.Append(tab);
            stringBuf.Append(tab);
            stringBuf.Append("GroupFilter:");
            stringBuf.Append(eol);

            stringBuf.Append(tab);
            stringBuf.Append(tab);
            stringBuf.Append(tab);
            stringBuf.Append("group: ");
            stringBuf.Append(Group.ToHexString());
            stringBuf.Append(eol);

            if (HasMergedToGroup)
            {
                stringBuf.Append(tab);
                stringBuf.Append(tab);
                stringBuf.Append(tab);
                stringBuf.Append("mergedToGroup: ");
                stringBuf.Append(MergedToGroup.ToHexString());
                stringBuf.Append(eol);
            }
            if (HasStatus)
            {
                stringBuf.Append(tab);
                stringBuf.Append(tab);
                stringBuf.Append(tab);
                stringBuf.Append("status: ");
                stringBuf.Append(Status);
                stringBuf.Append(eol);
            }

            return stringBuf.ToString();
        }

    }
}
