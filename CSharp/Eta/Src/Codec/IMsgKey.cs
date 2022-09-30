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
	/// contents flowing within a particular stream. This information, in conjunction
	/// with domainType and quality of service information, can be used to uniquely
	/// identify a data stream.
	/// </summary>
	/// <seealso cref="IMsg"/>
	/// <seealso cref="MsgKeyFlags"/>
	public interface IMsgKey
	{
		/// <summary>
		/// Checks the presence of the Service Id presence flag.<br />
		/// <br />
		/// Flags may also be bulk-get via <see cref="Flags"/>.
		/// </summary>
		/// <seealso cref="Flags"/>
		/// <returns> true - if exists; false if does not exist. </returns>
		bool CheckHasServiceId();

		/// <summary>
		/// Checks the presence of the Name presence flag.<br />
		/// <br />
		/// Flags may also be bulk-get via <see cref="Flags"/>.
		/// </summary>
		/// <seealso cref="Flags"/>
		/// <returns> true - if exists; false if does not exist. </returns>
		bool CheckHasName();

        /// <summary>
        /// Checks the presence of the Name Type presence flag.<br />
        /// <br />
        /// Flags may also be bulk-get via <see cref="Flags"/>.
        /// </summary>
        /// <seealso cref="Flags"/>
        /// <returns> true - if exists; false if does not exist. </returns>
        bool CheckHasNameType();

        /// <summary>
        /// Checks the presence of the Filter presence flag.<br />
        /// <br />
        /// Flags may also be bulk-get via <see cref="Flags"/>.
        /// </summary>
        /// <seealso cref="Flags"/>
        /// <returns> true - if exists; false if does not exist. </returns>
        bool CheckHasFilter();

        /// <summary>
        /// Checks the presence of the Identifier presence flag.<br />
        /// <br />
        /// Flags may also be bulk-get via <see cref="Flags"/>.
        /// </summary>
        /// <seealso cref="Flags"/>
        /// <returns> true - if exists; false if does not exist. </returns>
        bool CheckHasIdentifier();

        /// <summary>
        /// Checks the presence of the Attribute presence flag.<br />
        /// <br />
        /// Flags may also be bulk-get via <see cref="Flags"/>.
        /// </summary>
        /// <seealso cref="Flags"/>
        /// <returns> true - if exists; false if does not exist. </returns>
        bool CheckHasAttrib();

        /// <summary>
        /// Applies the Service Id presence flag.<br />
        /// <br />
        /// Flags may also be bulk-set via <see cref="Flags"/>.
        /// </summary>
        /// <seealso cref="Flags"/>
        void ApplyHasServiceId();

        /// <summary>
        /// Applies the Name presence flag.<br />
        /// <br />
        /// Flags may also be bulk-set via <see cref="Flags"/>.
        /// </summary>
        /// <seealso cref="Flags"/>
        void ApplyHasName();

        /// <summary>
        /// Applies the Name Type presence flag.<br />
        /// <br />
        /// Flags may also be bulk-set via <see cref="Flags"/>.
        /// </summary>
        /// <seealso cref="Flags"/>
        void ApplyHasNameType();

        /// <summary>
        /// Applies the Filter presence flag.<br />
        /// <br />
        /// Flags may also be bulk-set via <see cref="Flags"/>.
        /// </summary>
        /// <seealso cref="Flags"/>
        void ApplyHasFilter();

        /// <summary>
        /// Applies the Identifier presence flag.<br />
        /// <br />
        /// Flags may also be bulk-set via <see cref="Flags"/>.
        /// </summary>
        /// <seealso cref="Flags"/>
        void ApplyHasIdentifier();

        /// <summary>
        /// Applies the Attribute presence flag.<br />
        /// <br />
        /// Flags may also be bulk-set via <see cref="Flags"/>.
        /// </summary>
        /// <seealso cref="Flags"/>
        void ApplyHasAttrib();

		/// <summary>
		/// Clears <seealso cref="IMsgKey"/> object. Useful for object reuse during
		/// encoding. While decoding, <seealso cref="IMsgKey"/> object can be reused without
		/// using <see cref="Clear()"/>.
		/// </summary>
		void Clear();

		/// <summary>
		/// Gets or sets identifier associated with a service (a logical mechanism that provides
		/// or enables access to a set of capabilities). This value should correspond
		/// to the service content being requested or provided. In ETA, a service
		/// corresponds to a subset of  content provided by a component, where the
		/// Source Directory domain defines specific attributes associated with each
		/// service. These attributes include information such as QoS, the specific
		/// domain types available, and any dictionaries required to consume information
		/// from the service. The Source Directory domain model can obtain this and
		/// other types of information. Must be in the range of 0 - 65535.
		/// </summary>
		int ServiceId { get; set; }

		/// <summary>
		/// Gets or sets numeric value, typically enumerated, that indicates the type of the name
		/// member. Examples are User Name or RIC (i.e., the Reuters Instrument
		/// Code). name types are defined on a per-domain model basis.
		/// Must be in the range of 0 - 255.
		/// </summary>
		int NameType { get; set; }

		/// <summary>
		/// Gets or sets the name associated with the contents of the stream. Specific name type
		/// and contents should comply with the rules associated with the nameType member.
		/// </summary>
		Buffer Name { get; set; }

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
		long Filter { get; set; }

		/// <summary>
		/// Gets or sets user specified numeric identifier, is defined on a per-domain
		/// model basis. Must be in the range of -2147483648 - 2147483647.
		/// </summary>
		int Identifier { get; set; }

        /// <summary>
        /// Gets or sets container Type of the msgKey attributes. Must be a container type from
        /// the <see cref="DataTypes"/> enumeration in the range <see cref="DataTypes.CONTAINER_TYPE_MIN"/>
        /// to 255. Can indicate the presence of a ETA container type (value
        /// <see cref="DataTypes.NO_DATA"/> - 224) or some type of customer-defined container
        /// type (value 225 - 255).
        /// </summary>
        int AttribContainerType { get; set; }

        /// <summary>
        /// Gets or sets encoded MsgKey attribute information, used for additional identification
        /// attributes. Contents are typically specified in the domain model. Type is
        /// specified by attribContainerType.
        /// </summary>
        Buffer EncodedAttrib { get; set; }

		/// <summary>
		/// Compares two MsgKey structures to determine if they are the same.
		/// </summary>
		/// <param name="thatKey"> - the other key to compare to this one
		/// </param>
		/// <returns> returns true if keys match, false otherwise. </returns>
		bool Equals(IMsgKey thatKey);

        /// <summary>
        /// Performs a deep copy of a MsgKey. Expects all memory to be owned and
        /// managed by user. If the memory for the buffers (i.e. name, attrib)
        /// are not provided, they will be created.
        /// </summary>
        /// <param name="destKey"> Destination to copy into. It cannot be null.
        /// </param>
        /// <returns> <see cref="CodecReturnCode.SUCCESS"/>, if the key is copied successfully,
        ///         <see cref="CodecReturnCode.BUFFER_TOO_SMALL"/> if the buffer provided is too small </returns>
        CodecReturnCode Copy(IMsgKey destKey);

        /// <summary>
        /// Adds a FilterId to the key filter.
        /// </summary>
        /// <param name="filterId"> The FilterId you want added to the filter. (e.g. STATE)
        /// </param>
        /// <returns> Failure if this key does not contain filter element, INVALID_DATA if
        /// the filterId is greater then maximum filterID value (31), and SUCCESS otherwise
        /// </returns>
        /// <seealso cref="Rdm.Directory.ServiceFilterIds"/>
        CodecReturnCode AddFilterId(int filterId);

		/// <summary>
		/// Checks if FilterId is present in key filter.
		/// </summary>
		/// <param name="filterId"> The FilterId you want to check for
		/// </param>
		/// <returns> true - if exists; false if does not exist.
		/// </returns>
		/// <seealso cref="Rdm.Directory.ServiceFilterIds"/>
		bool CheckHasFilterId(int filterId);

		/// <summary>
		/// Gets or sets all the flags applicable to this message key. Must be in the
		/// range of 0 - 32767.
		/// </summary>
		/// <seealso cref="MsgKeyFlags"/>
		int Flags { get; set; }
	}
}