package SPA.ServerSide;

public final class Mysql {

    /**
     * Set MySQL/Mariadb global connection string
     *
     * @param dbConnection a global or default string for either embedded
     * MySQL/Mariadb database name or remote MySQL/Mariadb connection string
     * @param remote reserved for the future use
     */
    public final static native void SetMysqlDBGlobalConnectionString(String dbConnection, boolean remote);

    /**
     * Set embedded MySQL/Mariadb server initial options. Currently, the
     * function is not implemented but reserved for the future use
     *
     * @param options Embedded MySQL/Mariadb initial options string like
     * 'datadir=.;language=./share;default-storage-engine=MyISAM;skip-innodb;key-buffer-size=64M;console'.
     * If the string is null or empty, the options string will not be changed
     * @return options string for embedded MySQL/Mariadb server initialization
     */
    public final static native String SetMysqlEmbeddedOptions(String options);

    /**
     * Do authentication through MySQL/Mariadb C connector
     *
     * @param hSocket A server peer SocketPro socket handle
     * @param userId A string for user id
     * @param password A string for user password
     * @param nSvsId A service Id
     * @param dbConnection A connection string to MySQL/Mariadb database
     * @return true for success authentication, and false for failure
     * authentication
     */
    public final static native boolean DoMySQLAuthentication(long hSocket, String userId, String password, int nSvsId, String dbConnection);
}
