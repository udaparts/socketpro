<?php

try {
	$dbPool = GetSpPool('masterdb');
	$ok = true;
	do {
		$cache = $dbPool->Cache;
		echo $cache->FindOrdinal('sakila', 'actor', 'firsT_name').'<br/>';
		$tbl = $cache->In('sakila', 'actor', 1, array('jon', 'GRACE'));
		$tbl->Sort(0, true);
		echo var_dump($tbl->Data).'<br/>';
	} while(false);
	if($ok) {
		echo 'All requests executed successfully<br/>';
	}
	else {
		echo 'Session closed<br/>';
	}
	echo 'PHP script completed without exception';
} catch(Exception $e) {
	echo 'Caught exception: ', $e->getMessage();
}

?>
