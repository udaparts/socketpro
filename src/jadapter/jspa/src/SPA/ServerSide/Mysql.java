package SPA.ServerSide;

public class Mysql {

    /**
     * Set mysql global connection string.
     *
     * @param dbConnection a global or default string for either embedded mysql
     * database name or remote mysql connection string
     * @param remote true for setting a connection string to a remote mysql
     * database; and false for setting an embedded mysql database name
     */
    public final static native void SetMysqlDBGlobalConnectionString(String dbConnection, boolean remote);

    /**
     * Set embedded mysql server initial options.
     *
     * @param options Embedded mysql initial options string like
     * 'datadir=.;language=./share;default-storage-engine=MyISAM;skip-innodb;key-buffer-size=64M;console'.
     * If the string is null or empty, the options string will not be changed
     * @return options string for embedded mysql server initialization
     */
    public final static native String SetMysqlEmbeddedOptions(String options);

    public final static int DISABLE_REMOTE_MYSQL = 0x1;
    public final static int DISABLE_EMBEDDED_MYSQL = 0x2;
}
