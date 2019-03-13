<?php

$idMessage0 = SPA\BaseID::idReservedTwo + 100;
$idMessage1 = SPA\BaseID::idReservedTwo + 101;
$idMessage2 = SPA\BaseID::idReservedTwo + 102;
$idMessage3 = SPA\BaseID::idReservedTwo + 103;
$idMessage4 = SPA\BaseID::idReservedTwo + 104;
$TEST_QUEUE_KEY = 'queue_name_0';

function testEnqueue($sq) {
	global $idMessage0, $idMessage1, $idMessage2, $TEST_QUEUE_KEY;
	$idMsg = 0;
	$ok = true;
	$buff = SpBuffer();
	echo 'Going to enqueue 1024 messages ......<br/>';
	for ($n = 0; $n < 1024; ++$n) {
		$str = '' . $n . ' Object test';
		switch ($n % 3) {
		case 0:
			$idMsg = $idMessage0;
			break;
		case 1:
			$idMsg = $idMessage1;
			break;
		default:
			$idMsg = $idMessage2;
			break;
        }
		$ok = $sq->Enqueue($TEST_QUEUE_KEY, $idMsg, $buff->SaveString('SampleName')->SaveString($str)->SaveInt($n));
		$buff->Size = 0;
		if (!$ok) {
			echo 'Session closed<br/>';
			break;
		}
	}
	if ($ok) {
		echo '1024 messages enqueued<br/>';
	}
	return $ok;
}

try {
	$sq = LockSpHandler('my_queue');
	$ok = true;
	do {
		$ok = $sq->StartTrans($TEST_QUEUE_KEY);
		if (!$ok) {
			break;
		}
		$ok = testEnqueue($sq);
		if (!$ok) {
			break;
		}
		$buff = SpBuffer();
		$sq->BatchMessage($idMessage3, $buff->SaveString('Hello')->SaveString('World'));
		$buff->Size = 0; //reset buffer size to 0
		$sq->BatchMessage($idMessage4, $buff->SaveBool(true)->SaveDouble(234.456)->SaveString('MyTestWhatever'));
		$ok = $sq->EnqueueBatch($TEST_QUEUE_KEY);
		if (!$ok) {
			break;
		}
		$ok = $sq->EndTrans(false); //false -- commit, true -- rollback
		if (!$ok) {
			break;
		}
		$ok = $sq->GetKeys(function($keys){
			echo 'Keys opened: ';
			echo var_dump($keys).'<br/>';
		});
		if (!$ok) {
			break;
		}
		$res = null;
		do {
			//Dequeue is always synchronous
			$res = $sq->Dequeue($TEST_QUEUE_KEY, function($q, $reqId){
				global $idMessage0, $idMessage1, $idMessage2, $idMessage3, $idMessage4;
				switch ($reqId) {
				case $idMessage0:
				case $idMessage1:
				case $idMessage2:
					echo $q->LoadString().':'.$q->LoadString().' '.$q->LoadInt().'<br/>';
					break;
				case $idMessage3:
					echo $q->LoadString().' '.$q->LoadString().'<br/>';
					break;
				case $idMessage4:
					echo 'Bool: '.$q->LoadBool().', double: '.$q->LoadDouble().', string: '.$q->LoadString().'<br/>';
					break;
				default:
					throw new Exception("Unsupported message");
					break;
				}
			});
			echo var_dump($res).'<br/>';
		} while($res['messages'] > 0);
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
