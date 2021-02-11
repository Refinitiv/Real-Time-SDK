package com.refinitiv.eta.json.util;

import java.util.ArrayDeque;
import java.util.Deque;
import java.util.function.Supplier;

public class UtilQueue<T> {
    Deque<T> queue;
    private final Supplier<T> supplier;

    UtilQueue(int initialSize, Supplier<T> supplier) {
        queue = new ArrayDeque<>(initialSize);
        this.supplier = supplier;
    }

    public T get() {
        T element = queue.poll();
        if (element == null)
            return supplier.get();
        else
            return element;
    }

    public void add(T element) {
        queue.add(element);
    }

    public int size() {
        return queue.size();
    }

    public void growPool(int numOfObjects) {
        for (int i = 0; i < numOfObjects; i++) {
            queue.add(supplier.get());
        }
    }
}
