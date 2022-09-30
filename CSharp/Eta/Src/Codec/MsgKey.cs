/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

namespace Refinitiv.Eta.Codec
{
    /// <summary>
    /// ETA Message Key houses a variety of attributes used to help identify the
    /// contents flowing within a particular stream.This information, in conjunction
    /// with domainType and quality of service information, can be used to uniquely
    /// identify a data stream.
    /// </summary>
    /// <seealso cref="Msg"/>
    /// <seealso cref="MsgKeyFlags"/>
	public class MsgKey : IMsgKey
	{
		private readonly Buffer _name = new Buffer();
		private readonly Buffer _encodedAttrib = new Buffer();
		internal readonly int SEED = 23;
		internal readonly int PRIME = 31;

        /// <summary>
		/// Creates <see cref="MsgKey"/>.
		/// </summary>
		/// <returns> MsgKey object
		/// </returns>
		/// <seealso cref="MsgKey"/>
        public MsgKey()
        {
            AttribContainerType = DataTypes.CONTAINER_TYPE_MIN;
        }

        /// <summary>
		/// Clears <seealso cref="IMsgKey"/> object. Useful for object reuse during
		/// encoding. While decoding, <seealso cref="IMsgKey"/> object can be reused without
		/// using <see cref="Clear()"/>.
		/// </summary>
		public void Clear()
		{
			Flags = 0;
			ServiceId = 0;
			NameType = 0;
			_name.Clear();
			Filter = 0L;
			Identifier = 0;
			AttribContainerType = DataTypes.CONTAINER_TYPE_MIN;
			_encodedAttrib.Clear();
		}

		internal virtual void CopyReferences(MsgKey msgKey)
		{
			Flags = msgKey.Flags;
			NameType = msgKey.NameType;
			(_name).CopyReferences(msgKey.Name);
			Filter = msgKey.Filter;
			Identifier = msgKey.Identifier;
			AttribContainerType = msgKey.AttribContainerType;
			(_encodedAttrib).CopyReferences(msgKey.EncodedAttrib);
		}

		// copies this key into destKey
		// copies name and attrib only if the flags are set
		internal virtual CodecReturnCode Copy(IMsgKey destKey, int copyFlags)
		{
			CodecReturnCode ret;
			destKey.Flags = Flags;
			destKey.ServiceId = ServiceId;
			destKey.NameType = NameType;
			destKey.Filter = Filter;
			destKey.Identifier = Identifier;
			destKey.AttribContainerType = AttribContainerType;
			if ((copyFlags & CopyMsgFlags.KEY_NAME) > 0)
			{
				if ((ret = (_name).CopyWithOrWithoutByteBuffer(destKey.Name)) != CodecReturnCode.SUCCESS)
				{
					return ret;
				}
			}
			else
			{
				destKey.Flags = destKey.Flags & ~MsgKeyFlags.HAS_NAME;
			}
			if ((copyFlags & CopyMsgFlags.KEY_ATTRIB) > 0)
			{
				if ((ret = (_encodedAttrib).CopyWithOrWithoutByteBuffer(destKey.EncodedAttrib)) != CodecReturnCode.SUCCESS)
				{
					return ret;
				}
			}
			else
			{
				destKey.Flags = destKey.Flags & ~MsgKeyFlags.HAS_ATTRIB;
			}
			return CodecReturnCode.SUCCESS;
		}

        /// <summary>
		/// Checks the presence of the Service Id presence flag.<br />
		/// <br />
		/// Flags may also be bulk-get via <see cref="Flags"/>.
		/// </summary>
		/// <seealso cref="Flags"/>
		/// <returns> true - if exists; false if does not exist. </returns>
		public bool CheckHasServiceId()
		{
			return ((Flags & MsgKeyFlags.HAS_SERVICE_ID) > 0 ? true : false);
		}

        /// <summary>
		/// Checks the presence of the Name presence flag.<br />
		/// <br />
		/// Flags may also be bulk-get via <see cref="Flags"/>.
		/// </summary>
		/// <seealso cref="Flags"/>
		/// <returns> true - if exists; false if does not exist. </returns>
		public bool CheckHasName()
		{
			return ((Flags & MsgKeyFlags.HAS_NAME) > 0 ? true : false);
		}

        /// <summary>
        /// Checks the presence of the Name Type presence flag.<br />
        /// <br />
        /// Flags may also be bulk-get via <see cref="Flags"/>.
        /// </summary>
        /// <seealso cref="Flags"/>
        /// <returns> true - if exists; false if does not exist. </returns>
		public bool CheckHasNameType()
		{
			return ((Flags & MsgKeyFlags.HAS_NAME_TYPE) > 0 ? true : false);
		}

        /// <summary>
        /// Checks the presence of the Filter presence flag.<br />
        /// <br />
        /// Flags may also be bulk-get via <see cref="Flags"/>.
        /// </summary>
        /// <seealso cref="Flags"/>
        /// <returns> true - if exists; false if does not exist. </returns>
		public bool CheckHasFilter()
		{
			return ((Flags & MsgKeyFlags.HAS_FILTER) > 0 ? true : false);
		}

        /// <summary>
        /// Checks the presence of the Identifier presence flag.<br />
        /// <br />
        /// Flags may also be bulk-get via <see cref="Flags"/>.
        /// </summary>
        /// <seealso cref="Flags"/>
        /// <returns> true - if exists; false if does not exist. </returns>
		public bool CheckHasIdentifier()
		{
			return ((Flags & MsgKeyFlags.HAS_IDENTIFIER) > 0 ? true : false);
		}

        /// <summary>
        /// Checks the presence of the Attribute presence flag.<br />
        /// <br />
        /// Flags may also be bulk-get via <see cref="Flags"/>.
        /// </summary>
        /// <seealso cref="Flags"/>
        /// <returns> true - if exists; false if does not exist. </returns>
		public bool CheckHasAttrib()
		{
			return ((Flags & MsgKeyFlags.HAS_ATTRIB) > 0 ? true : false);
		}

        /// <summary>
        /// Applies the Service Id presence flag.<br />
        /// <br />
        /// Flags may also be bulk-set via <see cref="Flags"/>.
        /// </summary>
        /// <seealso cref="Flags"/>
        public void ApplyHasServiceId()
		{
			Flags |= MsgKeyFlags.HAS_SERVICE_ID;
		}

        /// <summary>
        /// Applies the Name presence flag.<br />
        /// <br />
        /// Flags may also be bulk-set via <see cref="Flags"/>.
        /// </summary>
        /// <seealso cref="Flags"/>
		public void ApplyHasName()
		{
			Flags |= MsgKeyFlags.HAS_NAME;
		}

        /// <summary>
        /// Applies the Name Type presence flag.<br />
        /// <br />
        /// Flags may also be bulk-set via <see cref="Flags"/>.
        /// </summary>
        /// <seealso cref="Flags"/>
		public void ApplyHasNameType()
		{
			Flags |= MsgKeyFlags.HAS_NAME_TYPE;
		}

        /// <summary>
        /// Applies the Filter presence flag.<br />
        /// <br />
        /// Flags may also be bulk-set via <see cref="Flags"/>.
        /// </summary>
        /// <seealso cref="Flags"/>
		public void ApplyHasFilter()
		{
			Flags |= MsgKeyFlags.HAS_FILTER;
		}

        /// <summary>
        /// Applies the Identifier presence flag.<br />
        /// <br />
        /// Flags may also be bulk-set via <see cref="Flags"/>.
        /// </summary>
        /// <seealso cref="Flags"/>
		public void ApplyHasIdentifier()
		{
			Flags |= MsgKeyFlags.HAS_IDENTIFIER;
		}

        /// <summary>
        /// Applies the Attribute presence flag.<br />
        /// <br />
        /// Flags may also be bulk-set via <see cref="Flags"/>.
        /// </summary>
        /// <seealso cref="Flags"/>
		public void ApplyHasAttrib()
		{
			Flags |= MsgKeyFlags.HAS_ATTRIB;
		}

        /// <summary>
		/// Gets or sets all the flags applicable to this message key. Must be in the
		/// range of 0 - 32767.
		/// </summary>
		/// <seealso cref="MsgKeyFlags"/>
		public int Flags { get; set; }

        /// <summary>
        /// Applies the Service Id presence flag.<br />
        /// <br />
        /// Flags may also be bulk-set via <see cref="Flags"/>.
        /// </summary>
        /// <seealso cref="Flags"/>
        public int ServiceId { get; set; }

        /// <summary>
        /// Applies the Name Type presence flag.<br />
        /// <br />
        /// Flags may also be bulk-set via <see cref="Flags"/>.
        /// </summary>
        /// <seealso cref="Flags"/>
		public int NameType { get; set; }

        /// <summary>
		/// Gets or sets the name associated with the contents of the stream. Specific name type
		/// and contents should comply with the rules associated with the nameType member.
		/// </summary>
		public Buffer Name
		{
            get
            {
                return _name;
            }

            set
            {
                (this._name).CopyReferences(value);
            }
		}

        /// <summary>
		/// Gets or sets combination of filterId bit values that describe content for domain model
		/// types with a <seealso cref="FilterList"/> payload. Filter identifier values are
		/// defined by the corresponding domain model specification. Must be in the
		/// range of 0 - 4294967296 (2^32).
		/// <ul>
		/// <li>When specified in <seealso cref="IRequestMsg"/>, filter conveys information
		/// about desired entries in responses</li>
		/// <li>When specified on a message housing a FilterList payload, filter
		/// conveys information about which filter entries are present.</li>
		/// </ul>
		/// </summary>
		public long Filter { get; set; }

        /// <summary>
		/// Gets or sets user specified numeric identifier, is defined on a per-domain
		/// model basis. Must be in the range of -2147483648 - 2147483647.
		/// </summary>
		public int Identifier { get; set; }

        /// <summary>
        /// Gets or sets container Type of the msgKey attributes. Must be a container type from
        /// the <see cref="DataTypes"/> enumeration in the range <see cref="DataTypes.CONTAINER_TYPE_MIN"/>
        /// to 255. Can indicate the presence of a ETA container type (value
        /// <see cref="DataTypes.NO_DATA"/> - 224) or some type of customer-defined container
        /// type (value 225 - 255).
        /// </summary>
		public int AttribContainerType { get; set; }

        /// <summary>
        /// Gets or sets encoded MsgKey attribute information, used for additional identification
        /// attributes. Contents are typically specified in the domain model. Type is
        /// specified by attribContainerType.
        /// </summary>
		public Buffer EncodedAttrib
		{
            get
            {
                return _encodedAttrib;
            }

            set
            {
                (_encodedAttrib).CopyReferences(value);
            }
		}

        /// <summary>
		/// Compares two MsgKey structures to determine if they are the same.
		/// </summary>
		/// <param name="thatKey"> - the other key to compare to this one
		/// </param>
		/// <returns> returns true if keys match, false otherwise. </returns>
		public bool Equals(IMsgKey thatKey)
		{
			if (thatKey == null)
			{
				return false;
			}

			if (((CheckHasNameType() && thatKey.CheckHasNameType()) && NameType != thatKey.NameType) || ((CheckHasNameType() && NameType != 1) &&
                !thatKey.CheckHasNameType()) || ((thatKey.CheckHasNameType() && thatKey.NameType != 1) && !CheckHasNameType())) // One has a name type and it is set to "1" and the other does NOT have a name type -  Only One has a name type and it is NOT set to "1" -  they both have a name type and the are NOT equal
			{
				return false;
			}

			// unset the Name Type from the flag, because we have all ready tested them above.
			int tmpFlags1 = Flags & ~MsgKeyFlags.HAS_NAME_TYPE;
			int tmpFlags2 = thatKey.Flags & ~MsgKeyFlags.HAS_NAME_TYPE;

			if (tmpFlags1 != tmpFlags2) // check the rest of the flags (not including Name Type)
			{
				return false;
			}
			if (CheckHasServiceId() && (ServiceId != thatKey.ServiceId))
			{
				return false;
			}
			if (CheckHasNameType() && (NameType != thatKey.NameType))
			{
				return false;
			}
			if (CheckHasName() && !_name.Equals(thatKey.Name))
			{
				return false;
			}
			if (CheckHasFilter() && (Filter != thatKey.Filter))
			{
				return false;
			}
			if (CheckHasIdentifier() && (Identifier != thatKey.Identifier))
			{
				return false;
			}
			if (CheckHasAttrib())
			{
				if (AttribContainerType != thatKey.AttribContainerType)
				{
					return false;
				}
				if (!_encodedAttrib.Equals(thatKey.EncodedAttrib))
				{
					return false;
				}
			}

			return true;
		}

        /// <summary>
        /// Gets the hash code of this object
        /// </summary>
        /// <returns>The hash code</returns>
		public override int GetHashCode()
		{
			int result = SEED;
			result = PRIME * result + Flags;

			if (CheckHasServiceId())
			{
				result = PRIME * result + ServiceId;
			}

			if (CheckHasNameType())
			{
				result = PRIME * result + NameType;
			}

			if (CheckHasFilter())
			{
				int cc = (int)(Filter ^ ((long)((ulong)Filter >> 32)));
				result = PRIME * result + cc;
			}

			if (CheckHasIdentifier())
			{
				result = PRIME * result + Identifier;
			}

			if (CheckHasAttrib())
			{
				result = PRIME * result + AttribContainerType;
			}

			if (CheckHasName())
			{
				result = result ^ (_name).GetHashCode();
			}

			if (CheckHasAttrib())
			{
				result = result ^ (_encodedAttrib).GetHashCode();
			}
			return result;
		}

        /// <summary>
		/// Performs a deep copy of a MsgKey. Expects all memory to be owned and
		/// managed by user. If the memory for the buffers (i.e. name, attrib)
		/// are not provided, they will be created.
		/// </summary>
		/// <param name="destKey"> Destination to copy into. It cannot be null.
		/// </param>
		/// <returns> <see cref="CodecReturnCode.SUCCESS"/>, if the key is copied successfully,
		///         <see cref="CodecReturnCode.BUFFER_TOO_SMALL"/> if the buffer provided is too small </returns>
		public CodecReturnCode Copy(IMsgKey destKey)
		{
			if (null == destKey)
			{
				return CodecReturnCode.INVALID_ARGUMENT;
			}

			return Copy(destKey, CopyMsgFlags.KEY);
		}

        /// <summary>
		/// Adds a FilterId to the key filter.
		/// </summary>
		/// <param name="filterId"> The FilterId you want added to the filter. (e.g. STATE)
		/// </param>
		/// <returns> Failure if this key does not contain filter element, INVALID_DATA if
		/// the filterId is greater then maximum filterID value (31), and SUCCESS otherwise
		/// </returns>
		/// <seealso cref="Rdm.Directory.ServiceFilterIds"/>
		public CodecReturnCode AddFilterId(int filterId)
		{
			if (!CheckHasFilter())
			{
				return CodecReturnCode.FAILURE;
			}
			if (filterId >= 32)
			{
				return CodecReturnCode.INVALID_DATA;
			}
			long index = 1 << (filterId & 7);
			Filter = Filter | index;
			return CodecReturnCode.SUCCESS;
		}

        /// <summary>
		/// Checks if FilterId is present in key filter.
		/// </summary>
		/// <param name="filterId"> The FilterId you want to check for
		/// </param>
		/// <returns> true - if exists; false if does not exist.
		/// </returns>
		/// <seealso cref="Rdm.Directory.ServiceFilterIds"/>
		public bool CheckHasFilterId(int filterId)
		{
			if (!CheckHasFilter())
			{
				return false;
			}
			int index = 1 << (filterId & 7);
			if ((Filter & index) > 0)
			{
				return true;
			}
			return false;
		}
	}

}