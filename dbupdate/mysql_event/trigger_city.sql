DELIMITER @@
CREATE TRIGGER sakila_city_trigger_insert
AFTER INSERT ON sakila.city FOR EACH ROW
BEGIN
    DECLARE msg VARCHAR(128);
    DECLARE res VARCHAR(16);
    SET msg = CONCAT('city_id=', new.city_id, '@sakila.city');
	SELECT NotifyCache(0, msg) INTO res;
END;
@@
CREATE TRIGGER sakila_city_trigger_update
AFTER UPDATE ON sakila.city FOR EACH ROW
BEGIN
    DECLARE msg VARCHAR(128);
    DECLARE res VARCHAR(16);
    SET msg = CONCAT('city_id=', new.city_id, '@sakila.city');
	SELECT NotifyCache(1, msg) INTO res;
END;
@@
CREATE TRIGGER sakila_city_trigger_delete
AFTER DELETE ON sakila.city FOR EACH ROW
BEGIN
    DECLARE msg VARCHAR(128);
    DECLARE res VARCHAR(16);
    SET msg = CONCAT('city_id=', old.city_id, '@sakila.city');
	SELECT NotifyCache(2, msg) INTO res;
END;
