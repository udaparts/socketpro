package SPA.ClientSide;

import javax.json.*;

public class CConnectionContext {

    public CConnectionContext() {

    }

    public CConnectionContext(String host, int port, String userId, String password) {
        Host = host;
        Port = port;
        UserId = userId;
        Password = password;
    }

    public CConnectionContext(String host, int port, String userId, String password, SPA.tagEncryptionMethod em) {
        Host = host;
        Port = port;
        UserId = userId;
        Password = password;
        EncrytionMethod = em;
    }

    public CConnectionContext(String host, int port, String userId, String password, SPA.tagEncryptionMethod em, boolean zip) {
        Host = host;
        Port = port;
        UserId = userId;
        Password = password;
        EncrytionMethod = em;
        Zip = zip;
    }

    public CConnectionContext(String host, int port, String userId, String password, SPA.tagEncryptionMethod em, boolean zip, boolean v6) {
        Host = host;
        Port = port;
        UserId = userId;
        Password = password;
        EncrytionMethod = em;
        Zip = zip;
        V6 = v6;
    }

    void Normalize() throws Exception {
        if (Host == null) {
            throw new Exception("Host string cannot be null");
        }
        Host = Host.trim().toLowerCase();
        if (Host.length() == 0) {
            throw new Exception("Host string cannot be empty");
        }
        if (Port <= 0) {
            throw new Exception("Port number must be a positive number");
        }
    }

    boolean IsSame(CConnectionContext cc) {
        if (cc == null) {
            return false;
        }
        return (Host.equalsIgnoreCase(cc.Host) && Port == cc.Port);
    }

    JsonObject ToJsonObject() {
        JsonObjectBuilder job = Json.createObjectBuilder();
        job.add("Host", Host);
        job.add("Port", Port);
        job.add("UserId", UserId);
        job.add("Password", Password);
        job.add("EncrytionMethod", EncrytionMethod.getValue());
        job.add("V6", V6);
        job.add("Zip", Zip);
        if (AnyData == null) {
            job.addNull("AnyData");
        } else {
            String type = AnyData.getClass().getName();
            switch (type) {
                case "java.lang.Long":
                    job.add("AnyData", (long) AnyData);
                    break;
                case "java.lang.Double":
                    job.add("AnyData", (double) AnyData);
                    break;
                case "java.lang.Boolean":
                    job.add("AnyData", (boolean) AnyData);
                    break;
                case "java.lang.String":
                    job.add("AnyData", (String) AnyData);
                    break;
                case "javax.json.JsonArray":
                    job.add("AnyData", (javax.json.JsonArray) AnyData);
                    break;
                case "javax.json.JsonObject":
                    job.add("AnyData", (javax.json.JsonObject) AnyData);
                    break;
            }
        }
        return job.build();
    }

    public String Host = "";
    public int Port = 0;
    public String UserId = "";
    public String Password = "";
    public SPA.tagEncryptionMethod EncrytionMethod = SPA.tagEncryptionMethod.NoEncryption;
    public boolean V6 = false;
    public boolean Zip = false;
    public Object AnyData = null;
}
