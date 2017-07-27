DELIMITER @@
CREATE TRIGGER sakila_language_trigger_insert
AFTER INSERT ON sakila.language FOR EACH ROW
BEGIN
    DECLARE msg VARCHAR(128);
    DECLARE res VARCHAR(16);
    SET msg = CONCAT('language_id=', new.language_id, '@sakila.language');
	SELECT NotifyCache(0, msg) INTO res;
END;
@@
CREATE TRIGGER sakila_language_trigger_update
AFTER UPDATE ON sakila.language FOR EACH ROW
BEGIN
    DECLARE msg VARCHAR(128);
    DECLARE res VARCHAR(16);
    SET msg = CONCAT('language_id=', new.language_id, '@sakila.language');
	SELECT NotifyCache(1, msg) INTO res;
END;
@@
CREATE TRIGGER sakila_language_trigger_delete
AFTER DELETE ON sakila.language FOR EACH ROW
BEGIN
    DECLARE msg VARCHAR(128);
    DECLARE res VARCHAR(16);
    SET msg = CONCAT('language_id=', old.language_id, '@sakila.language');
	SELECT NotifyCache(2, msg) INTO res;
END;
