package SPA.UDB;

public enum tagRollbackPlan {

    /**
     * Manual transaction will rollback whenever there is an error by default
     */
    rpDefault(0),
    /**
     * Manual transaction will rollback whenever there is an error by default
     */
    rpRollbackErrorAny(0),
    /**
     * Manual transaction will rollback as long as the number of errors is less
     * than the number of ok processing statements
     */
    rpRollbackErrorLess(1),
    /**
     * Manual transaction will rollback as long as the number of errors is less
     * or equal than the number of ok processing statements
     */
    rpRollbackErrorEqual(2),
    /**
     * Manual transaction will rollback as long as the number of errors is more
     * than the number of ok processing statements
     */
    rpRollbackErrorMore(3),
    /**
     * Manual transaction will rollback only if all the processing statements
     * are failed
     */
    rpRollbackErrorAll(4),
    /**
     * Manual transaction will rollback always no matter what happens.
     */
    rpRollbackAlways(5);

    private final int intValue;
    private static java.util.HashMap<Integer, tagRollbackPlan> mappings;

    private static java.util.HashMap<Integer, tagRollbackPlan> getMappings() {
        synchronized (tagRollbackPlan.class) {
            if (mappings == null) {
                mappings = new java.util.HashMap<>();
            }
        }
        return mappings;
    }

    @SuppressWarnings("LeakingThisInConstructor")
    private tagRollbackPlan(int value) {
        intValue = value;
        getMappings().put(value, this);
    }

    public int getValue() {
        return intValue;
    }

    public static tagRollbackPlan forValue(int value) {
        return getMappings().get(value);
    }
}
