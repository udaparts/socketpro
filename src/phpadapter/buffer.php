<?php
try {
	$strTest = '春节震撼发布：第三次国民革命军檄文';
	echo 'String input: '.$strTest.'<br/>';
	echo 'String output: '.SpBuffer()->SaveString($strTest)->LoadString().'<br/>';
	echo 'String output: '.SpBuffer()->SaveAString($strTest)->LoadAString().'<br/>';
	echo 'String output: '.SpBuffer()->SaveObject($strTest)->LoadObject().'<br/>';

	$arr = array(true, 'MyTest', 12345, 27.567, null, '春节震撼发布');
	echo 'array input: ';
	echo var_dump($arr).'<br/>';
	echo 'array output: ';
	echo var_dump(SpBuffer()->SaveObject($arr)->LoadObject()).'<br>';

	$large_number = 0x7fffffffffffffff;
	echo 'Large number input: '.$large_number.'<br/>';
	echo 'Large number output: '.SpBuffer()->SaveLong($large_number)->LoadLong().'<br/>';
	echo 'Large number output: '.SpBuffer()->SaveULong($large_number)->LoadULong().'<br/>';
	
	$uint = 0xffffffff;
	echo 'Unsigned int input: '.$uint.'<br/>';
	echo 'Unsigned int output: '.SpBuffer()->SaveUInt($uint)->LoadUInt().'<br/>';
	
	$flt = 1234.567;
	echo 'Double input: '.$flt.'<br/>';
	echo 'Double output: '.SpBuffer()->SaveDouble($flt)->LoadDouble().'<br/>';
	echo 'Double output: '.SpBuffer()->SaveObject($flt)->LoadObject().'<br/>';
	echo 'Double output: '.SpBuffer()->SaveAString($flt)->LoadAString().'<br/>';
	echo 'Double output: '.SpBuffer()->SaveString($flt)->LoadString().'<br/>';
	
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
	
	//demo complex structure serialization and de-serialization, which will be used with client/server HelloWorld example
	echo 'Structure input: ';
	echo var_dump($data).'<br/>';
	$res = SpBuffer()->Save($data, function($d, $q) {
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
	})->Load(function($q){
		//de-serialize when result comes from server
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
		return $d;
	});
	echo 'Structure output: ';
	echo var_dump($res).'<br/>';
}
catch(Exception $e) {
	echo 'Caught exception: ', $e->getMessage();
}
?>