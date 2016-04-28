package com.thomsonreuters.upa.codec;

/**
 * Codec utility methods.
 */
public class CodecUtils 
{
    /**
     * This class is not instantiated
     */
    private CodecUtils()
    {
        throw new AssertionError();
    }

    /**
     * Used to add a two byte identifier to the group ID.
     * 
     * @param groupId {@link Buffer} with any existing groupID information and
     *            space to append groupIdToAdd
     * @param groupIdToAdd two byte ID to append to the groupID contained in
     *            groupId buffer
     * 
     * @return {@link CodecReturnCodes#INCOMPLETE_DATA} if length of
     *         groupIdToAdd is less than 2 bytes or
     *         {@link CodecReturnCodes#BUFFER_TOO_SMALL} if groupId buffer does
     *         not have space to add 2 bytes or {@link CodecReturnCodes#SUCCESS}
     *         if adding of group id is successful.
     */
    public static int addGroupId(Buffer groupId, byte[] groupIdToAdd)
    {
        if (groupIdToAdd.length < 2)
            return CodecReturnCodes.INCOMPLETE_DATA;

        if (groupId.data().remaining() < 2)
            return CodecReturnCodes.BUFFER_TOO_SMALL;

        int savePos = groupId.data().position();
        groupId.data().put(groupIdToAdd, 0, 2);

        // set length
        groupId.data(groupId.data(), 0, savePos + 2);

        return CodecReturnCodes.SUCCESS;
    }

}
