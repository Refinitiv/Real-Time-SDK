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

namespace LSEG.Eta.ValueAdd.Rdm
{
    /// <summary>
    /// The RDM Service Load. 
    /// Contains information provided by the Source Directory Load filter.
    /// </summary>
    sealed public class ServiceLoad
    {
        private long m_OpenLimit;
        private long m_OpenWindow;
        private long m_LoadFactor;

        private ElementList m_ElementList = new ElementList();
        private ElementEntry m_Element = new ElementEntry();
        private UInt m_TmpUInt = new UInt();
         
        private StringBuilder m_StringBuf = new StringBuilder();
        private const string tab = "\t";

        /// <summary>
        /// The action associated with this Load filter.
        /// </summary>
        public FilterEntryActions Action { get; set; }

        /// <summary>
        /// The service load flags. Populate with <see cref="ServiceLoadFlags"/>
        /// </summary>
        public ServiceLoadFlags Flags { get; set; }

        /// <summary>
        /// Indicates presence of the openLimit field.
        /// </summary>
        public bool HasOpenLimit { get => (Flags & ServiceLoadFlags.HAS_OPEN_LIMIT) != 0; set { if (value) Flags |= ServiceLoadFlags.HAS_OPEN_LIMIT; else Flags &= ~ServiceLoadFlags.HAS_OPEN_LIMIT; } }

        /// <summary>
        /// Indicates presence of the openWindow field.
        /// </summary>
        public bool HasOpenWindow { get => (Flags & ServiceLoadFlags.HAS_OPEN_WINDOW) != 0; set { if (value) Flags |= ServiceLoadFlags.HAS_OPEN_WINDOW; else Flags &= ~ServiceLoadFlags.HAS_OPEN_WINDOW; } }

        /// <summary>
        /// Indicates presence of the loadFactor field.
        /// </summary>
        public bool HasLoadFactor { get => (Flags & ServiceLoadFlags.HAS_LOAD_FACTOR) != 0; set { if (value) Flags |= ServiceLoadFlags.HAS_LOAD_FACTOR; else Flags &= ~ServiceLoadFlags.HAS_LOAD_FACTOR; } }

        /// <summary>
        /// The maximum number of items the Consumer is allowed to open from this service.
        /// </summary>
        public long OpenLimit { get => m_OpenLimit; set { Debug.Assert(HasOpenLimit); m_OpenLimit = value; } }

        /// <summary>
        /// The maximum number of items the Consumer may have outstanding (i.e. waiting for a RefreshMsg) from this service.
        /// </summary>
        public long OpenWindow { get => m_OpenWindow; set { Debug.Assert(HasOpenWindow); m_OpenWindow = value; } }

        /// <summary>
        /// The number indicating the current workload of the source providing the data.
        /// </summary>
        public long LoadFactor { get => m_LoadFactor; set { Debug.Assert(HasLoadFactor); m_LoadFactor = value; } }

        /// <summary>
        /// The filterId - Populated by <see cref="ServiceFilterIds"/>
        /// </summary>
        public int FilterId { get => ServiceFilterIds.LOAD; }

        /// <summary>
        /// Performs an update of ServiceLoad object.
        /// </summary>
        /// <param name="destServiceLoad">ServiceLoad object to update with information from this object. It cannot be null.</param>
        /// <returns><see cref="CodecReturnCode"/> value indicating the status of the operation.</returns>
        public CodecReturnCode Update(ServiceLoad destServiceLoad)
        {
            Debug.Assert(destServiceLoad != null);
            if (HasLoadFactor)
            {
                destServiceLoad.HasLoadFactor = true;
                destServiceLoad.LoadFactor = LoadFactor;
            }

            if (HasOpenLimit)
            {
                destServiceLoad.HasOpenLimit = true;
                destServiceLoad.OpenLimit = OpenLimit;
            }

            if (HasOpenWindow)
            {
                destServiceLoad.HasOpenWindow = true;
                destServiceLoad.OpenWindow = OpenWindow;
            }

            destServiceLoad.Action = Action;

            return CodecReturnCode.SUCCESS;
        }

        /// <summary>
        /// Service Load constructor.
        /// </summary>
        public ServiceLoad()
        {
            Clear();
        }

        /// <summary>
        /// Clears the current contents of the Service Load object and prepares it for re-use.
        /// </summary>
        public void Clear()
        {
            Flags = 0;
            Action = FilterEntryActions.SET;
            m_OpenLimit = 4294967295L;
            m_OpenWindow = 4294967295L;
            m_LoadFactor = 65535;
        }

        /// <summary>
        /// Performs a deep copy of this object into <c>destServiceLoad</c>.
        /// </summary>
        /// <param name="destServiceLoad">ServiceLoad object that will have this object's information copied into.</param>
        /// <returns><see cref="CodecReturnCode"/> indicating success or failure.</returns>
        public CodecReturnCode Copy(ServiceLoad destServiceLoad)
        {
            Debug.Assert(destServiceLoad != null);
            destServiceLoad.Clear();
            if (HasLoadFactor)
            {
                destServiceLoad.HasLoadFactor = true;
                destServiceLoad.LoadFactor = LoadFactor;
            }

            if (HasOpenLimit)
            {
                destServiceLoad.HasOpenLimit = true;
                destServiceLoad.OpenLimit = OpenLimit;
            }

            if (HasOpenWindow)
            {
                destServiceLoad.HasOpenWindow = true;
                destServiceLoad.OpenWindow = OpenWindow;
            }

            destServiceLoad.Action = Action;

            return CodecReturnCode.SUCCESS;
        }

        /// <summary>
        /// Encodes this Service Load using the provided <c>encodeIter</c>.
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

            if (HasOpenLimit)
            {
                m_Element.Clear();
                m_Element.Name = ElementNames.OPEN_LIMIT;
                m_Element.DataType = Codec.DataTypes.UINT;
                m_TmpUInt.Value(OpenLimit);
                ret = m_Element.Encode(encodeIter, m_TmpUInt);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;
            }

            if (HasOpenWindow)
            {
                m_Element.Clear();
                m_Element.Name = ElementNames.OPEN_WINDOW;
                m_Element.DataType = Codec.DataTypes.UINT;
                m_TmpUInt.Value(OpenWindow);
                ret = m_Element.Encode(encodeIter, m_TmpUInt);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;
            }

            if (HasLoadFactor)
            {
                m_Element.Clear();
                m_Element.Name = ElementNames.LOAD_FACT;
                m_Element.DataType = Codec.DataTypes.UINT;
                m_TmpUInt.Value(LoadFactor);
                ret = m_Element.Encode(encodeIter, m_TmpUInt);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;
            }

            ret = m_ElementList.EncodeComplete(encodeIter, true);
            if (ret < CodecReturnCode.SUCCESS)
                return ret;
            return CodecReturnCode.SUCCESS;
        }

        /// <summary>
        /// Decodes this Service Load using the provided <c>decodeIter</c>.
        /// </summary>
        /// <param name="decodeIter">Decode iterator that has already decoded the initial message.</param>
        /// <returns><see cref="CodecReturnCode"/> indicating success or failure.</returns>
        public CodecReturnCode Decode(DecodeIterator decodeIter)
        {
            m_ElementList.Clear();
            m_Element.Clear();

            // Decode element list
            CodecReturnCode ret = m_ElementList.Decode(decodeIter, null);
            if (ret < CodecReturnCode.SUCCESS)
                return ret;

            // Decode element list elements
            while ((ret = m_Element.Decode(decodeIter)) != CodecReturnCode.END_OF_CONTAINER)
            {
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;

                //OpenLimit
                if (m_Element.Name.Equals(ElementNames.OPEN_LIMIT))
                {
                    ret = m_TmpUInt.Decode(decodeIter);
                    if (ret != CodecReturnCode.SUCCESS
                            && ret != CodecReturnCode.BLANK_DATA)
                    {
                        return ret;
                    }
                    HasOpenLimit = true;
                    OpenLimit = m_TmpUInt.ToLong();
                }
                //OpenWindow
                else if (m_Element.Name.Equals(ElementNames.OPEN_WINDOW))
                {
                    ret = m_TmpUInt.Decode(decodeIter);
                    if (ret != CodecReturnCode.SUCCESS
                            && ret != CodecReturnCode.BLANK_DATA)
                    {
                        return ret;
                    }
                    HasOpenWindow = true;
                    OpenWindow = m_TmpUInt.ToLong();
                }
                //LoadFactor
                else if (m_Element.Name.Equals(ElementNames.LOAD_FACT))
                {
                    ret = m_TmpUInt.Decode(decodeIter);
                    if (ret != CodecReturnCode.SUCCESS
                            && ret != CodecReturnCode.BLANK_DATA)
                    {
                        return ret;
                    }
                    HasLoadFactor = true;
                    LoadFactor = m_TmpUInt.ToLong();
                }
            }

            return CodecReturnCode.SUCCESS;
        }

        /// <summary>
        /// Returns a human readable string representation of the Service Load.
        /// </summary>
        /// <returns>String containing the string representation.</returns>
        public override string ToString()
        {
            m_StringBuf.Clear();
            m_StringBuf.Append(tab);
            m_StringBuf.Append(tab);
            m_StringBuf.Append("LoadFilter:");
            m_StringBuf.AppendLine();

            m_StringBuf.Append(tab);
            m_StringBuf.Append(tab);
            m_StringBuf.Append(tab);
            m_StringBuf.Append("OpenLimit: ");
            m_StringBuf.Append(OpenLimit);
            m_StringBuf.AppendLine();

            m_StringBuf.Append(tab);
            m_StringBuf.Append(tab);
            m_StringBuf.Append(tab);
            m_StringBuf.Append("OpenWindow: ");
            m_StringBuf.Append(OpenWindow);
            m_StringBuf.AppendLine();

            m_StringBuf.Append(tab);
            m_StringBuf.Append(tab);
            m_StringBuf.Append(tab);
            m_StringBuf.Append("LoadFactor: ");
            m_StringBuf.Append(LoadFactor);
            m_StringBuf.AppendLine();

            return m_StringBuf.ToString();
        }
    }
}
