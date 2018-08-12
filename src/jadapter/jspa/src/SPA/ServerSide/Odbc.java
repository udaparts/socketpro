package SPA.ServerSide;

public final class Odbc {

    /**
     * Set ODBC global connection string.
     *
     * @param dbConnection a global or default string for ODBC connection string
     */
    public final static native void SetOdbcDBGlobalConnectionString(String dbConnection);

    /**
     * Do authentication through an ODBC driver
     *
     * @param hSocket a SocketPro peer socket handle
     * @param userId a user id string
     * @param password a password string
     * @param nSvsId a service identification number
     * @param odbcDriver a string to identify an ODBC driver
     * @param dsn an optional data source name
     * @return true if authentication is succeeded, and false if authentication
     * is failed
     */
    public final static native boolean DoODBCAuthentication(long hSocket, String userId, String password, int nSvsId, String odbcDriver, String dsn);
}
