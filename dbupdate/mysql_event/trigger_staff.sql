DELIMITER @@
CREATE TRIGGER sakila_staff_trigger_insert
AFTER INSERT ON sakila.staff FOR EACH ROW
BEGIN
    DECLARE msg VARCHAR(128);
    DECLARE res VARCHAR(16);
    SET msg = CONCAT('staff_id=', new.staff_id, '@sakila.staff');
	SELECT NotifyCache(0, msg) INTO res;
END;
@@
CREATE TRIGGER sakila_staff_trigger_update
AFTER UPDATE ON sakila.staff FOR EACH ROW
BEGIN
    DECLARE msg VARCHAR(128);
    DECLARE res VARCHAR(16);
    SET msg = CONCAT('staff_id=', new.staff_id, '@sakila.staff');
	SELECT NotifyCache(1, msg) INTO res;
END;
@@
CREATE TRIGGER sakila_staff_trigger_delete
AFTER DELETE ON sakila.staff FOR EACH ROW
BEGIN
    DECLARE msg VARCHAR(128);
    DECLARE res VARCHAR(16);
    SET msg = CONCAT('staff_id=', old.staff_id, '@sakila.staff');
	SELECT NotifyCache(2, msg) INTO res;
END;
