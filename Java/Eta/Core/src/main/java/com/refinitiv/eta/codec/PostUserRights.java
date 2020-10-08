package com.refinitiv.eta.codec;

/**
 * The PostUserRights indicate whether the posting user is allowed to create or
 * destroy items in the cache of record. This is also used to indicate whether
 * the user has the ability to change the permData associated with an item in
 * the cache of record.
 * 
 * @see PostMsg
 */
public class PostUserRights
{
    /**
     * This class is not instantiated
     */
    private PostUserRights()
    {
        throw new AssertionError();
    }

    /** (0x00) No user rights */
    public static final int NONE = 0x00;

    /** (0x01) User is allowed to create records in cache with this post */
    public static final int CREATE = 0x01;

    /**
     * (0x02) User is allowed to delete/remove records from cache with this post
     */
    public static final int DELETE = 0x02;

    /**
     * (0x04) User is allowed to modify the permData for records already in
     * cache with this post.
     */
    public static final int MODIFY_PERM = 0x04;
}
