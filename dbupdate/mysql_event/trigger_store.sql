DELIMITER @@
CREATE TRIGGER sakila_store_trigger_insert
AFTER INSERT ON sakila.store FOR EACH ROW
BEGIN
    DECLARE msg VARCHAR(128);
    DECLARE res VARCHAR(16);
    SET msg = CONCAT('store_id=', new.store_id, '@sakila.store');
	SELECT NotifyCache(1, msg) INTO res;
END;
@@
CREATE TRIGGER sakila_store_trigger_update
AFTER UPDATE ON sakila.store FOR EACH ROW
BEGIN
    DECLARE msg VARCHAR(128);
    DECLARE res VARCHAR(16);
    SET msg = CONCAT('store_id=', new.store_id, '@sakila.store');
	SELECT NotifyCache(2, msg) INTO res;
END;
@@
CREATE TRIGGER sakila_store_trigger_delete
AFTER DELETE ON sakila.store FOR EACH ROW
BEGIN
    DECLARE msg VARCHAR(128);
    DECLARE res VARCHAR(16);
    SET msg = CONCAT('store_id=', old.store_id, '@sakila.store');
	SELECT NotifyCache(3, msg) INTO res;
END;
