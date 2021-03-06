package SPA.ServerSide;

public final class Plugin {

    /**
     * Returns a plug-in version string like "1.0.0.1 for this plug-in"
     *
     * @param pName a case-sensitive name to find a plug-in
     * @return a plug-in version string like "2.1.0.7" or null string if no
     * plug-in is found
     */
    public final static native String GetSPluginVersion(String pName);

    /**
     * Set zero, one or more pairs of settings (key/value) by a JSON string. All
     * pairs of unknown or unsupported settings (key/value) will be silently
     * ignored
     *
     * @param pName a case-sensitive name to find a plug-in
     * @param jsonOptions A UTF-8 JSON string containing zero, one or more pairs
     * of settings (key/value). Specifically, all supported settings will be set
     * to default states when setting an empty JSON object string
     * @return true if it is successful; Otherwise, false if a non-JSON string
     * is passed in or no plug-in is found.
     */
    public final static native boolean SetSPluginGlobalOptions(String pName, String jsonOptions);

    /**
     * Get a UTF-8 JSON string object having zero, one or more pair of settings
     * (key/value). It is noted that calling the method will still return a JSON
     * string and show all supported settings even though the method
     * SetSPluginGlobalOptions has not been called yet
     *
     * @param pName a case-sensitive name to find a plug-in
     * @param estimatedSize An estimated JSON string length. If JSON string
     * length is over 1024, a proper value must be given. Otherwise, JSON string
     * may be truncated
     * @return a UTF-8 JSON string object having zero, one or more pair of
     * settings (key/value), or a null string if no plug-in is found
     */
    public final static native String GetSPluginGlobalOptions(String pName, int estimatedSize);

    /**
     * Provide authentication at server side by use of this plug-in.
     *
     * @param pName a case-sensitive name to find a plug-in
     * @param hSocket SocketPro server socket handle
     * @param userId a user identification string from client
     * @param password a password string from client
     * @param nSvsId a service identification number
     * @param connString an optional or required connection string without user
     * id or password
     * @return 1: authentication permitted, 0: authentication denied, -1:
     * authentication not implemented or no plug-in found, -2: authentication
     * failed due to an internal error
     */
    public final static native int DoSPluginAuthentication(String pName, long hSocket, String userId, String password, int nSvsId, String connString);

    /**
     * Authentication permitted and DB handle opened and cached
     */
    public final static int AUTHENTICATION_OK = 1;

    /**
     * Authentication can not be made, but DB handle opened and cached
     */
    public final static int AUTHENTICATION_PROCESSED = 0;

    /**
     * Authentication failed, and no DB handle opened or cached
     */
    public final static int AUTHENTICATION_FAILED = -1;

    /**
     * Authentication cannot be made due to an internal error, and no DB handle
     * opened or cached
     */
    public final static int AUTHENTICATION_INTERNAL_ERROR = -2;

    /**
     * Authentication not implemented at all
     */
    public final static int AUTHENTICATION_NOT_IMPLEMENTED = -3;
}
