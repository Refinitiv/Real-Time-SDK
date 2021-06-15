package com.refinitiv.ema.examples.rrtmdviewer.desktop.common;

import java.util.HashMap;
import java.util.Map;
import java.util.Objects;

/**
 * Needed for accessing to common settings and services from different parts of application.
 */
public class ApplicationSingletonContainer {

    private final static Map<String, Object> BEAN_CONTAINER = new HashMap<>();

    public static <T> T getBean(Class<T> type) {
        Object tmp = Objects.requireNonNull(BEAN_CONTAINER.get(type.getName()));
        if (type.isInstance(tmp)) {
            return type.cast(tmp);
        }
        throw new IllegalArgumentException();
    }

    /**
     * Add new bean to the singleton container.
     * @param type interface or parent class to which {@code impl} could be cast
     * @param impl instance of {@code type} class or any of its subtypes.
     */
    public static void addBean(Class<?> type, Object impl) {
        if (!BEAN_CONTAINER.containsKey(type.getName())) {
            synchronized (ApplicationSingletonContainer.class) {
                if (!BEAN_CONTAINER.containsKey(type.getName())) {
                    BEAN_CONTAINER.put(type.getName(), impl);
                }
            }
        }
    }

    public static <T> boolean containsBean(Class<T> type) {
        return BEAN_CONTAINER.containsKey(type.getName());
    }

    public static void deleteBean(Class<?> type) {
        BEAN_CONTAINER.remove(type.getName());
    }

    /**
     * Clear container.
     */
    public void clear() {
        BEAN_CONTAINER.clear();
    }
}
