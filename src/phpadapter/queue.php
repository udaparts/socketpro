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
		$ok = $sq->Enqueue($TEST_QUEUE_KEY, $idMsg, $buff->SaveString('SampleName')->SaveString($str)->SaveInt($n), null);
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

function testDequeue($sq, $cb) {
	global $TEST_QUEUE_KEY;
    echo 'Going to dequeue messages ......<br/>';
    //optionally, add one extra to improve processing concurrency at both client and server sides for better performance and through-output
    return $sq->Dequeue($TEST_QUEUE_KEY, $cb) && $sq->Dequeue($TEST_QUEUE_KEY, $cb);
}

$cb = function($mc, $fsize, $msgs, $bytes) {
	global $cb, $sq, $TEST_QUEUE_KEY;
	echo 'Total message count=' . $mc . ', queue file size=' . $fsize . ', messages dequeued=' . $msgs . ', message bytes dequeued=' . $bytes . '<br/>';
	if ($mc) {
        $sq->Dequeue($TEST_QUEUE_KEY, $cb);
    }
};

try {
	$sq = LockSpHandler('my_queue');
	$ok = true;
	do {
		$ok = $sq->StartTrans($TEST_QUEUE_KEY, null);
		if (!$ok) {
			break;
		}
		$ok = testEnqueue($sq);
		if (!$ok) {
			break;
		}
		$buff = SpBuffer();
		$sq->BatchMessage($idMessage3, $buff->SaveString('Hello')->SaveString('World'));
		$buff->Size = 0;
		$sq->BatchMessage($idMessage4, $buff->SaveBool(true)->SaveDouble(234.456)->SaveString('MyTestWhatever'));
		$buff->Size = 0;
		$ok = $sq->EnqueueBatch($TEST_QUEUE_KEY, null);
		if (!$ok) {
			break;
		}
		$ok = $sq->EndTrans(false, null);
		if (!$ok) {
			break;
		}
		$ok = testDequeue($sq, $cb);
		if (!$ok) {
			break;
		}
		$ok = $sq->CloseQueue($TEST_QUEUE_KEY, null);
		if (!$ok) {
			break;
		}
		$keys = $sq->GetKeys(true);
		echo 'Keys opened: ';
		echo var_dump($keys).'<br/>';
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