/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using Refinitiv.Eta.Codec;
using Refinitiv.Eta.Rdm;
using System.Diagnostics;
using System.Text;
using static Refinitiv.Eta.Rdm.Directory;

namespace Refinitiv.Eta.ValueAdd.Rdm
{
    /// <summary>
    /// The RDM Service Load. 
    /// Contains information provided by the Source Directory Load filter.
    /// </summary>
    public class ServiceLoad
    {
        private long m_OpenLimit;
        private long m_OpenWindow;
        private long m_LoadFactor;

        private ElementList m_ElementList = new ElementList();
        private ElementEntry m_Element = new ElementEntry();
        private UInt m_TmpUInt = new UInt();
         
        private StringBuilder m_StringBuf = new StringBuilder();
        private const string eol = "\n";
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
        /// The filterId - Populated by <see cref="Directory.ServiceFilterIds"/>
        /// </summary>
        public int FilterId { get => ServiceFilterIds.LOAD; }

        public ServiceLoad()
        {
            Clear();
        }

        public void Clear()
        {
            Flags = 0;
            Action = FilterEntryActions.SET;
            m_OpenLimit = 4294967295L;
            m_OpenWindow = 4294967295L;
            m_LoadFactor = 65535;
        }

        public override string ToString()
        {
            m_StringBuf.Clear();
            m_StringBuf.Append(tab);
            m_StringBuf.Append(tab);
            m_StringBuf.Append("LoadFilter:");
            m_StringBuf.Append(eol);

            m_StringBuf.Append(tab);
            m_StringBuf.Append(tab);
            m_StringBuf.Append(tab);
            m_StringBuf.Append("OpenLimit: ");
            m_StringBuf.Append(OpenLimit);
            m_StringBuf.Append(eol);

            m_StringBuf.Append(tab);
            m_StringBuf.Append(tab);
            m_StringBuf.Append(tab);
            m_StringBuf.Append("OpenWindow: ");
            m_StringBuf.Append(OpenWindow);
            m_StringBuf.Append(eol);

            m_StringBuf.Append(tab);
            m_StringBuf.Append(tab);
            m_StringBuf.Append(tab);
            m_StringBuf.Append("LoadFactor: ");
            m_StringBuf.Append(LoadFactor);
            m_StringBuf.Append(eol);

            return m_StringBuf.ToString();
        }

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

        public CodecReturnCode Encode(EncodeIterator encIter)
        {
            m_ElementList.Clear();
            m_ElementList.ApplyHasStandardData();
            CodecReturnCode ret = m_ElementList.EncodeInit(encIter, null, 0);
            if (ret != CodecReturnCode.SUCCESS)
                return ret;

            if (HasOpenLimit)
            {
                m_Element.Clear();
                m_Element.Name = ElementNames.OPEN_LIMIT;
                m_Element.DataType = Codec.DataTypes.UINT;
                m_TmpUInt.Value(OpenLimit);
                ret = m_Element.Encode(encIter, m_TmpUInt);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;
            }

            if (HasOpenWindow)
            {
                m_Element.Clear();
                m_Element.Name = ElementNames.OPEN_WINDOW;
                m_Element.DataType = Codec.DataTypes.UINT;
                m_TmpUInt.Value(OpenWindow);
                ret = m_Element.Encode(encIter, m_TmpUInt);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;
            }

            if (HasLoadFactor)
            {
                m_Element.Clear();
                m_Element.Name = ElementNames.LOAD_FACT;
                m_Element.DataType = Codec.DataTypes.UINT;
                m_TmpUInt.Value(LoadFactor);
                ret = m_Element.Encode(encIter, m_TmpUInt);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;
            }

            ret = m_ElementList.EncodeComplete(encIter, true);
            if (ret < CodecReturnCode.SUCCESS)
                return ret;
            return CodecReturnCode.SUCCESS;
        }

        public CodecReturnCode Decode(DecodeIterator dIter)
        {
            m_ElementList.Clear();
            m_Element.Clear();

            // Decode element list
            CodecReturnCode ret = m_ElementList.Decode(dIter, null);
            if (ret < CodecReturnCode.SUCCESS)
                return ret;

            // Decode element list elements
            while ((ret = m_Element.Decode(dIter)) != CodecReturnCode.END_OF_CONTAINER)
            {
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;

                //OpenLimit
                if (m_Element.Name.Equals(ElementNames.OPEN_LIMIT))
                {
                    ret = m_TmpUInt.Decode(dIter);
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
                    ret = m_TmpUInt.Decode(dIter);
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
                    ret = m_TmpUInt.Decode(dIter);
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
    }
}
