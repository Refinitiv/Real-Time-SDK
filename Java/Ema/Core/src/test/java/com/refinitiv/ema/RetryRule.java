package com.refinitiv.ema;

import org.junit.rules.TestRule;
import org.junit.runner.Description;
import org.junit.runners.model.Statement;

public class RetryRule implements TestRule {

    private int retryCount;

    public RetryRule(int retryCount) {
        this.retryCount = retryCount;
    }

    public Statement apply(Statement base, Description description) {
        return statement(base, description);
    }

    private Statement statement(final Statement base, final Description description) {

        return new Statement() {

            @Override
            public void evaluate() throws Throwable {

                Throwable ex = null;
                for (int i = 0; i < retryCount; i++) {
                    try {
                        base.evaluate();
                        return;
                    }
                    catch (Throwable e) {
                        ex = e;
                        System.out.println("Run #" + i + " of " + description.getDisplayName() + " FAILED");
                    }
                }
                throw ex;
            }
        };
    }
}
