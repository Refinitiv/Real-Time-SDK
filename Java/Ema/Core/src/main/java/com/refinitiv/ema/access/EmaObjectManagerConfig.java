package com.refinitiv.ema.access;

public class EmaObjectManagerConfig {

    // Methods for changing local EmaObjectManager limits (EMA objects)

    /**
     * Gets the current limit of the pools in the local OmmProvider pool manager that hold EMA Data type objects
     * @param provider current OmmProvider instance
     * @return the current limit
     */
    public static int getEmaObjectManagerDataTypePoolLimit(OmmProvider provider)
    {
        if (provider instanceof OmmIProviderImpl)
        {
            OmmIProviderImpl instance = (OmmIProviderImpl)provider;
            if (instance._objManager != null)
                return instance._objManager.getEmaObjectDataTypePoolLimit();
            else return -1;
        }
        else if (provider instanceof OmmNiProviderImpl)
        {
            OmmNiProviderImpl instance = (OmmNiProviderImpl)provider;
            if (instance._objManager != null)
                return instance._objManager.getEmaObjectDataTypePoolLimit();
            else return -1;
        }
        else
        {
            return -1;
        }
    }

    /**
     * Gets the current limit of the pools in the local OmmProvider pool manager that hold EMA Complex type objects
     * @param provider current OmmProvider instance
     * @return the current limit
     */
    public static int getEmaObjectManagerComplexTypePoolLimit(OmmProvider provider)
    {
        if (provider instanceof OmmIProviderImpl)
        {
            OmmIProviderImpl instance = (OmmIProviderImpl)provider;
            if (instance._objManager != null)
                return instance._objManager.getEmaObjectComplexTypePoolLimit();
            else return -1;
        }
        else if (provider instanceof OmmNiProviderImpl)
        {
            OmmNiProviderImpl instance = (OmmNiProviderImpl)provider;
            if (instance._objManager != null)
                return instance._objManager.getEmaObjectComplexTypePoolLimit();
            else return -1;
        }
        else
        {
            return -1;
        }
    }

    /**
     * Gets the current limit of the pools in the local OmmProvider pool manager that hold EMA Message type objects
     * @param provider current OmmProvider instance
     * @return the current limit
     */
    public static int getEmaObjectManagerMsgTypePoolLimit(OmmProvider provider)
    {
        if (provider instanceof OmmIProviderImpl)
        {
            OmmIProviderImpl instance = (OmmIProviderImpl)provider;
            if (instance._objManager != null)
                return instance._objManager.getEmaObjectMsgTypePoolLimit();
            else return -1;
        }
        else if (provider instanceof OmmNiProviderImpl)
        {
            OmmNiProviderImpl instance = (OmmNiProviderImpl)provider;
            if (instance._objManager != null)
                return instance._objManager.getEmaObjectMsgTypePoolLimit();
            else return -1;
        }
        else
        {
            return -1;
        }
    }

    /**
     * Sets the current limit of the pool in the local OmmProvider pool manager that hold Data type EMA objects
     * @param provider current OmmProvider instance
     * @param limit the new limit to be set
     */
    public static void setEmaObjectManagerDataTypePoolLimit(OmmProvider provider, int limit)
    {
        if (provider instanceof OmmIProviderImpl)
        {
            OmmIProviderImpl instance = (OmmIProviderImpl)provider;
            if (instance._objManager != null)
                instance._objManager.setDataTypePoolsLimit(limit);
        }
        else if (provider instanceof OmmNiProviderImpl)
        {
            OmmNiProviderImpl instance = (OmmNiProviderImpl)provider;
            instance._objManager.setDataTypePoolsLimit(limit);
        }
    }

    /**
     * Sets the current limit of the pool in the local OmmProvider pool manager that hold Complex type EMA objects
     * @param provider current OmmProvider instance
     * @param limit the new limit to be set
     */
    public static void setEmaObjectManagerComplexTypePoolLimit(OmmProvider provider, int limit)
    {
        if (provider instanceof OmmIProviderImpl)
        {
            OmmIProviderImpl instance = (OmmIProviderImpl)provider;
            if (instance._objManager != null)
                instance._objManager.setComplexTypePoolsLimit(limit);
        }
        else if (provider instanceof OmmNiProviderImpl)
        {
            OmmNiProviderImpl instance = (OmmNiProviderImpl)provider;
            instance._objManager.setComplexTypePoolsLimit(limit);
        }
    }

    /**
     * Sets the current limit of the pool in the local OmmProvider pool manager that hold Message type EMA objects
     * @param provider current OmmProvider instance
     * @param limit the new limit to be set
     */
    public static void setEmaObjectManagerMsgTypePoolLimit(OmmProvider provider, int limit)
    {
        if (provider instanceof OmmIProviderImpl)
        {
            OmmIProviderImpl instance = (OmmIProviderImpl)provider;
            if (instance._objManager != null)
                instance._objManager.setMsgTypePoolsLimit(limit);
        }
        else if (provider instanceof OmmNiProviderImpl)
        {
            OmmNiProviderImpl instance = (OmmNiProviderImpl)provider;
            instance._objManager.setMsgTypePoolsLimit(limit);
        }
    }

    /**
     * Gets the current limit of the pool in the local OmmConsumer pool manager that holds Data type EMA objects
     * @param consumer current OmmConsumer instance
     * @return the current limit
     */
    public static int getEmaObjectManagerDataTypePoolLimit(OmmConsumer consumer)
    {
        if (consumer instanceof OmmConsumerImpl)
        {
            OmmConsumerImpl instance = (OmmConsumerImpl)consumer;
            if (instance._objManager != null)
                return instance._objManager.getEmaObjectDataTypePoolLimit();
            else return -1;
        }
        else
        {
            return -1;
        }
    }

    /**
     * Gets the current limit of the pool in the local OmmConsumer pool manager that holds Complex type EMA objects
     * @param consumer current OmmConsumer instance
     * @return the current limit
     */
    public static int getEmaObjectManagerComplexTypePoolLimit(OmmConsumer consumer)
    {
        if (consumer instanceof OmmConsumerImpl)
        {
            OmmConsumerImpl instance = (OmmConsumerImpl)consumer;
            if (instance._objManager != null)
                return instance._objManager.getEmaObjectComplexTypePoolLimit();
            else return -1;
        }
        else
        {
            return -1;
        }
    }

    /**
     * Gets the current limit of the pool in the local OmmConsumer pool manager that holds Message type EMA objects
     * @param consumer current OmmConsumer instance
     * @return the current limit
     */
    public static int getEmaObjectManagerMsgTypePoolLimit(OmmConsumer consumer)
    {
        if (consumer instanceof OmmConsumerImpl)
        {
            OmmConsumerImpl instance = (OmmConsumerImpl)consumer;
            if (instance._objManager != null)
                return instance._objManager.getEmaObjectMsgTypePoolLimit();
            else return -1;
        }
        else
        {
            return -1;
        }
    }

    /**
     * Sets the current limit of the pool in the local OmmConsumer pool manager that holds Data type EMA objects
     * @param consumer current OmmConsumer instance
     * @param limit the new limit to be set
     */
    public static void setEmaObjectManagerDataTypePoolLimit(OmmConsumer consumer, int limit)
    {
        if (consumer instanceof OmmConsumerImpl)
        {
            OmmConsumerImpl instance = (OmmConsumerImpl)consumer;
            if (instance._objManager != null)
                instance._objManager.setDataTypePoolsLimit(limit);
        }
    }

    /**
     * Sets the current limit of the pool in the local OmmConsumer pool manager that holds Complex type EMA objects
     * @param consumer current OmmConsumer instance
     * @param limit the new limit to be set
     */
    public static void setEmaObjectManagerComplexTypePoolLimit(OmmConsumer consumer, int limit)
    {
        if (consumer instanceof OmmConsumerImpl)
        {
            OmmConsumerImpl instance = (OmmConsumerImpl)consumer;
            if (instance._objManager != null)
                instance._objManager.setComplexTypePoolsLimit(limit);
        }
    }

    /**
     * Sets the current limit of the pool in the local OmmConsumer pool manager that holds Message type EMA objects
     * @param consumer current OmmConsumer instance
     * @param limit the new limit to be set
     */
    public static void setEmaObjectManagerMsgTypePoolLimit(OmmConsumer consumer, int limit)
    {
        if (consumer instanceof OmmConsumerImpl)
        {
            OmmConsumerImpl instance = (OmmConsumerImpl)consumer;
            if (instance._objManager != null)
                instance._objManager.setMsgTypePoolsLimit(limit);
        }
    }

    // Methods for changing local EmaObjectManager limits (objects related to Sessions)

    /**
     * Gets the current limit of the pools in the local OmmProvider pool manager that hold objects related to Sessions
     * @param provider current OmmProvider instance
     * @return the current limit
     */
    public static int getEmaObjectManagerSessionObjectsPoolLimit(OmmProvider provider)
    {
        if (provider instanceof OmmIProviderImpl)
        {
            OmmIProviderImpl instance = (OmmIProviderImpl)provider;
            if (instance._objManager != null)
                return instance._objManager.getSessionObjectPoolLimit();
            else return -1;
        }
        else if (provider instanceof OmmNiProviderImpl)
        {
            OmmNiProviderImpl instance = (OmmNiProviderImpl)provider;
            if (instance._objManager != null)
                return instance._objManager.getSessionObjectPoolLimit();
            else return -1;
        }
        else
        {
            return -1;
        }
    }

    /**
     * Sets the current limit of the pools in the local OmmProvider pool manager that hold objects related to Sessions
     * @param provider current OmmProvider instance
     * @param limit the new limit to be set
     */
    public static void setEmaObjectManagerSessionObjectsPoolLimit(OmmProvider provider, int limit)
    {
        if (provider instanceof OmmIProviderImpl)
        {
            OmmIProviderImpl instance = (OmmIProviderImpl)provider;
            if (instance._objManager != null)
                instance._objManager.setSessionObjectPoolLimit(limit);
        }
        else if (provider instanceof OmmNiProviderImpl)
        {
            OmmNiProviderImpl instance = (OmmNiProviderImpl)provider;
            instance._objManager.setSessionObjectPoolLimit(limit);
        }
    }

    /**
     * Gets the current limit of the pools in the local OmmConsumer pool manager that hold objects related to Sessions
     * @param consumer current OmmConsumer instance
     * @return the current limit
     */
    public static int getEmaObjectManagerSessionObjectsPoolLimit(OmmConsumer consumer)
    {
        if (consumer instanceof OmmConsumerImpl)
        {
            OmmConsumerImpl instance = (OmmConsumerImpl)consumer;
            if (instance._objManager != null)
                return instance._objManager.getSessionObjectPoolLimit();
            else return -1;
        }
        else
        {
            return -1;
        }
    }

    /**
     * Sets the current limit of the pools in the local OmmConsumer pool manager that hold objects related to Sessions
     * @param consumer current OmmConsumer instance
     * @param limit the new limit to be set
     */
    public static void setEmaObjectManagerSessionObjectsPoolLimit(OmmConsumer consumer, int limit)
    {
        if (consumer instanceof OmmConsumerImpl)
        {
            OmmConsumerImpl instance = (OmmConsumerImpl)consumer;
            if (instance._objManager != null)
                instance._objManager.setSessionObjectPoolLimit(limit);
        }
    }


    // Methods for changing local EmaObjectManager limits (ETA objects)

    /**
     * Gets the current limit of the pools in the local OmmProvider pool manager that hold ETA objects
     * @param provider current OmmProvider instance
     * @return the current limit
     */
    public static int getEmaObjectManagerEtaObjectsPoolLimit(OmmProvider provider)
    {
        if (provider instanceof OmmIProviderImpl)
        {
            OmmIProviderImpl instance = (OmmIProviderImpl)provider;
            if (instance._objManager != null)
                return instance._objManager.getEtaObjectsPoolsLimit();
            else return -1;
        }
        else if (provider instanceof OmmNiProviderImpl)
        {
            OmmNiProviderImpl instance = (OmmNiProviderImpl)provider;
            if (instance._objManager != null)
                return instance._objManager.getEtaObjectsPoolsLimit();
            else return -1;
        }
        else
        {
            return -1;
        }
    }

    /**
     * Sets the current limit of the pools in the local OmmProvider pool manager that hold ETA objects
     * @param provider current OmmProvider instance
     * @param limit the new limit to be set
     */
    public static void setEmaObjectManagerEtaObjectsPoolLimit(OmmProvider provider, int limit)
    {
        if (provider instanceof OmmIProviderImpl)
        {
            OmmIProviderImpl instance = (OmmIProviderImpl)provider;
            if (instance._objManager != null)
                instance._objManager.setEtaObjectsPoolsLimit(limit);
        }
        else if (provider instanceof OmmNiProviderImpl)
        {
            OmmNiProviderImpl instance = (OmmNiProviderImpl)provider;
            instance._objManager.setEtaObjectsPoolsLimit(limit);
        }
    }

    /**
     * Gets the current limit of the pools in the local OmmConsumer pool manager that hold ETA objects
     * @param consumer current OmmConsumer instance
     * @return the current limit
     */
    public static int getEmaObjectManagerEtaObjectsPoolLimit(OmmConsumer consumer)
    {
        if (consumer instanceof OmmConsumerImpl)
        {
            OmmConsumerImpl instance = (OmmConsumerImpl)consumer;
            if (instance._objManager != null)
                return instance._objManager.getEtaObjectsPoolsLimit();
            else return -1;
        }
        else
        {
            return -1;
        }
    }

    /**
     * Sets the current limit of the pools in the local OmmConsumer pool manager that hold ETA objects
     * @param consumer current OmmConsumer instance
     * @param limit the new limit to be set
     */
    public static void setEmaObjectManagerEtaObjectsPoolLimit(OmmConsumer consumer, int limit)
    {
        if (consumer instanceof OmmConsumerImpl)
        {
            OmmConsumerImpl instance = (OmmConsumerImpl)consumer;
            if (instance._objManager != null)
                instance._objManager.setEtaObjectsPoolsLimit(limit);
        }
    }

    /**
     * Gets the current count of the objects in the pool of the given type in the local OmmProvider pool manager
     * @param provider current OmmProvider instance
     * @param dataType the type of the objects in the pool
     * @return current object count in the pool
     */
    public static int getEmaObjectManagerEmaObjectsPoolCount(OmmProvider provider, int dataType)
    {
        if (provider instanceof OmmIProviderImpl)
        {
            OmmIProviderImpl instance = (OmmIProviderImpl)provider;
            if (instance._objManager != null)
                return instance._objManager.getEmaObjectPoolCount(dataType);
            else return -1;
        }
        else if (provider instanceof OmmNiProviderImpl)
        {
            OmmNiProviderImpl instance = (OmmNiProviderImpl)provider;
            if (instance._objManager != null)
                return instance._objManager.getEmaObjectPoolCount(dataType);
            else return -1;
        }
        else
        {
            return -1;
        }
    }

    /**
     * Gets the current count of the objects in the pool of the given type in the local OmmConsumer pool manager
     * @param consumer current OmmConsumer instance
     * @param dataType the type of the objects in the pool
     * @return current object count in the pool
     */
    public static int getEmaObjectManagerEmaObjectsPoolCount(OmmConsumer consumer, int dataType)
    {
        if (consumer instanceof OmmConsumerImpl)
        {
            OmmConsumerImpl instance = (OmmConsumerImpl)consumer;
            if (instance._objManager != null)
                return instance._objManager.getEmaObjectPoolCount(dataType);
            else return -1;
        }
        else
        {
            return -1;
        }
    }
}
