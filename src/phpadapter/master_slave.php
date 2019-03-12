<?php

$cb_err_ex = function($err) {
	if ($err['ec']) {
		echo 'error code: '.$err['ec'].', error message: '.$err['em'].'<br/>';
	}
	else {
		echo 'affected: '.$err['affected'].', fails: '.$err['fails'].', oks: '.$err['oks'].', last id: '.$err['id'].'<br/>';
	}
};

try {
	$dbPool = GetSpPool('masterdb');
	$cache = $dbPool->Cache;
	echo 'Cached DB Tables: ';
	echo var_dump($cache->DbTable).'<br/><br/>';

	echo 'Keys: ';
	echo var_dump($cache->FindKeys('sakila', 'actor')).'<br/><br/>';

	echo 'Table Meta: ';
	echo var_dump($cache->GetColumMeta('sakila', 'actor')).'<br/><br/>';

	echo 'Rows: ';
	echo $cache->GetRowCount('sakila', 'actor').'<br/><br/>';

	echo 'Columns: ';
	echo $cache->GetColumnCount('sakila', 'actor').'<br/><br/>';

	echo 'sakila.actor last_name Ordinal: ';
	echo $cache->FindOrdinal('sakila', 'actor', 'last_name').'<br/><br/>';

	echo 'SELECT * FROM sakila.actor WHERE actor_id BETWEEN 5 and 35:<br/>';
	$tbl = $cache->Between('sakila', 'actor', 0, 5, 35);
	echo var_dump($tbl->Data).'<br/><br/>';

	echo 'SELECT * FROM sakila.actor WHERE actor_id BETWEEN 5 and 35 ORDER BY actor_id DESC:<br/>';
	$tbl->Sort(0, true);
	echo var_dump($tbl->Data).'<br/><br/>';

	echo 'SELECT * FROM sakila.actor WHERE actor_id IN(9,10) ORDER BY actor_id DESC:<br/>';
	$sub = $tbl->In(0, array(9,10));
	echo var_dump($sub->Data).'<br/><br/>';

	echo 'SELECT * FROM sakila.actor WHERE actor_id NOT IN(24,9,10) ORDER BY actor_id DESC:<br/>';
	$sub = $tbl->NotIn(0, array(24,9,10));
	echo var_dump($sub->Data).'<br/><br/>';

	$slave = GetSpHandler('slavedb0');
	echo 'SELECT * FROM mysqldb.company:<br/>';
	$vData = array();
	$res = $slave->Execute('SELECT * FROM mysqldb.company', true, function($v){
		global $vData;
		$vData = array_merge($vData, $v);
	});
	echo var_dump($vData).'<br/><br/>';
	echo 'Final result: ';
	echo $cb_err_ex($res).'<br/><br/>';
} catch(Exception $e) {
	echo 'Caught exception: ', $e->getMessage();
}

?>
