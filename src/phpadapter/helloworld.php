<?php

try {
	$idReservedTwo = 0x2001; //Your request id must larger than 0x2001
	
	//Request ids must be the same ones at server side, which could be implemented with one of C/C++, C#/VB.NET, Java and Python languages
	$idSayHello = $idReservedTwo + 1;
	$idSleep = $idReservedTwo + 2;
	$idEcho = $idReservedTwo + 3;
	
	do {
		$hw = GetSpHandler('my_hello_world');
		
		//streaming all requests for the best network efficiency
		//async request
		if(!$hw->SendRequest($idSayHello, SpBuffer()->SaveString('Hillary')->SaveString('Clinton'), function($res) {
			echo $res->LoadString();
			echo '<br/>';
		})) {
			echo 'Session closed<br/>';
			break;
		}
		//async request
		if (!$hw->SendRequest($idSayHello, SpBuffer()->SaveString('Jack')->SaveString('Smith'), function($res) {
				echo $res->LoadString();
				echo '<br/>';
			}, function($canceled) {
				echo canceled ? 'Request canceld<br/>' : 'Session closed<br/>';
			}, function($errMsg, $errCode, $errWhere, $reqId) {
				echo $errMsg; //an exception from server
				echo '<br/>';
		})) {
			echo 'Session closed<br/>';
			break;
		}
		
		$data = array(
			'nullStr' => null,
			'objNull' => null,
			'aDate' => new DateTime(),
			'aDouble' => 1234.567,
			'aBool' => true,
			'unicodeStr' => 'Unicode',
			'asciiStr' => 'ASCII',
			'objBool' => true,
			'objString' => 'test',
			'objArrString' => array('Hello', 'world'),
			'objArrInt' => array(1, 76890)
		);
		echo 'Structure input: ';
		echo var_dump($data).'<br/>';
		$buff = SpBuffer()->Save($data, function($d, $q) {
			//serialize member values into buffer q with a specific order, which must be in agreement with server implementation
			$q->SaveString($d['nullStr']); //4 bytes for length
			$q->SaveObject($d['objNull']); //2 bytes for data type
			$q->SaveDate($d['aDate']); //8 bytes for ulong with accuracy to 1 micro-second
			$q->SaveDouble($d['aDouble']); //8 bytes
			$q->SaveBool($d['aBool']); //1 byte
			$q->SaveString($d['unicodeStr']); //4 bytes for string length + (length * 2) bytes for string data -- UTF16 low-endian
			$q->SaveAString($d['asciiStr']); //4 bytes for ASCII string length + length bytes for string data
			$q->SaveObject($d['objBool']); //2 bytes for data type + 2 bytes for variant bool
			$q->SaveObject($d['objString']); //2 bytes for data type + 4 bytes for string length + (length * 2) bytes for string data -- UTF16-lowendian
			$q->SaveObject($d['objArrString']); //2 bytes for data type + 4 bytes for array size + (4 bytes for string length + (length * 2) bytes for string data) * arraysize -- UTF16-lowendian
			$q->SaveObject($d['objArrInt']); //2 bytes for data type + 4 bytes for array size + arraysize * 8 bytes for int64_t data
		});
		//async request
		if(!$hw->SendRequest($idEcho, $buff, function($q) {
			//de-serialize when result comes from a remote server
			$d = array(
				'nullStr' => $q->LoadString(),
				'objNull' => $q->LoadObject(),
				'aDate' => $q->LoadDate(),
				'aDouble' => $q->LoadDouble(),
				'aBool' => $q->LoadBool(),
				'unicodeStr' => $q->LoadString(),
				'asciiStr' => $q->LoadAString(),
				'objBool' => $q->LoadObject(),
				'objString' => $q->LoadObject(),
				'objArrString' => $q->LoadObject(),
				'objArrInt' => $q->LoadObject()
			);
			echo 'Structure output: ';
			echo var_dump($d).'<br/>';
		})) {
			echo 'Session closed<br/>';
			break;
		}
		//All requests will be processed and returned in order within one session or socket for the best network efficency
		//sync request
		$buff = $hw->SendRequest($idSleep, SpBuffer()->SaveUInt(100), true); //last call should be always sync within PHP environment
		echo 'buff size = '.$buff->Size.'<br/>'; //should be empty because the request returns nothing
	} while(false);
	echo 'PHP script completed without exception';
} catch(Exception $e) {
	echo 'Caught exception: ', $e->getMessage();
}

?>
