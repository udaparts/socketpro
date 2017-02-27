DELIMITER @@
CREATE TRIGGER sakila_category_trigger_insert
AFTER INSERT ON sakila.category FOR EACH ROW
BEGIN
    DECLARE msg VARCHAR(128);
    DECLARE res VARCHAR(16);
    SET msg = CONCAT('category_id=', new.category_id, '@sakila.category');
	SELECT NotifyCache(1, msg) INTO res;
END;
@@
CREATE TRIGGER sakila_category_trigger_update
AFTER UPDATE ON sakila.category FOR EACH ROW
BEGIN
    DECLARE msg VARCHAR(128);
    DECLARE res VARCHAR(16);
    SET msg = CONCAT('category_id=', new.category_id, '@sakila.category');
	SELECT NotifyCache(2, msg) INTO res;
END;
@@
CREATE TRIGGER sakila_category_trigger_delete
AFTER DELETE ON sakila.category FOR EACH ROW
BEGIN
    DECLARE msg VARCHAR(128);
    DECLARE res VARCHAR(16);
    SET msg = CONCAT('category_id=', old.category_id, '@sakila.category');
	SELECT NotifyCache(3, msg) INTO res;
END;
