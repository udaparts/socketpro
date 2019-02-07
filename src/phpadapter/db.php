<?php

try {
	$db = GetSpHandler('masterdb');
	$ok = true;
	do {
		$sql = "select actor_id, first_name from sakila.actor where actor_id between ? and ?";
		$ok = $db->Prepare($sql, false);
		if (!$ok) {
			break;
		}
		$vData = array();
		$vParam = array(1, 3);
		$res = $db->Execute($vParam, true, function($v, $proc) {
			global $vData;
			$vData = array_merge($vData, $v);
		}, function($mydb){
			echo 'Meta data: ';
			echo var_dump($mydb->ColMeta).'<br/><br/>';
		});
		echo 'Meta data: ';
		echo var_dump($vData).'<br/><br/>';

		echo 'Final result: ';
		echo var_dump($res).'<br/><br/>';
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
