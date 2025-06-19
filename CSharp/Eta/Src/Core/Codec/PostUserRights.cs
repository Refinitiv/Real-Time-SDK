/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.Codec
{
	/// <summary>
	/// The PostUserRights indicate whether the posting user is allowed to create or
	/// destroy items in the cache of record. This is also used to indicate whether
	/// the user has the ability to change the permData associated with an item in
	/// the cache of record.
	/// </summary>
	/// <seealso cref="IPostMsg"/>
	sealed public class PostUserRights
	{
		/// <summary>
		/// This class is not instantiated
		/// </summary>
		private PostUserRights()
		{
            throw new System.NotImplementedException();
		}

		/// <summary>
		/// (0x00) No user rights </summary>
		public const int NONE = 0x00;

		/// <summary>
		/// (0x01) User is allowed to create records in cache with this post </summary>
		public const int CREATE = 0x01;

		/// <summary>
		/// (0x02) User is allowed to delete/remove records from cache with this post
		/// </summary>
		public const int DELETE = 0x02;

		/// <summary>
		/// (0x04) User is allowed to modify the permData for records already in
		/// cache with this post.
		/// </summary>
		public const int MODIFY_PERM = 0x04;
	}

}