
package SPA.ServerSide;

public final class Odbc {
    /**
     * Set ODBC global connection string.
     * 
     * @param dbConnection a global or default string for ODBC connection string
     */
    public final static native void SetOdbcDBGlobalConnectionString(String dbConnection);
}
