DELIMITER @@
CREATE FUNCTION NotifyCache(eventType int, msg varchar(2048))
	returns VARCHAR(16)
    NO SQL
BEGIN
    DECLARE res VARCHAR(16);
	SELECT GetConnections() INTO res;
	IF LENGTH(res) = 0 THEN
		# if the function SetConnectionString is not called yet, we call it below for building connection to remote SocketPro server at host ws-yye-1
		Select SetConnectionString('host=ws-yye-1;port=20901;userid=uid_event;pwd=pass_1;svsid=268436480') INTO res;
	END IF;
	SELECT NotifyDatabaseEvent(eventType, msg, '1') INTO res;
	return res;
END;
