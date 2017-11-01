package SPA.UDB;

public enum tagManagementSystem {

    msUnknown(-1),
    msSqlite(0),
    msMysql(1),
    msODBC(2),
    msMsSQL(3),
    msOracle(4),
    msDB2(5),
    msPostgreSQL(6),
    msMongoDB(7);

    private final int intValue;
    private static java.util.HashMap<Integer, tagManagementSystem> mappings;

    private static java.util.HashMap<Integer, tagManagementSystem> getMappings() {
        synchronized (tagManagementSystem.class) {
            if (mappings == null) {
                mappings = new java.util.HashMap<>();
            }
        }
        return mappings;
    }

    @SuppressWarnings("LeakingThisInConstructor")
    private tagManagementSystem(int value) {
        intValue = value;
        getMappings().put(value, this);
    }

    public int getValue() {
        return intValue;
    }

    public static tagManagementSystem forValue(int value) {
        return getMappings().get(value);
    }
}
