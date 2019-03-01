<?php

$cb_err = function($err) {
	if ($err['ec']) {
		echo 'error code: '.$err['ec'].', error message: '.$err['em'].'<br/>';
	}
};

$cb_err_ex = function($err) {
	if ($err['ec']) {
		echo 'error code: '.$err['ec'].', error message: '.$err['em'].'<br/>';
	}
	else {
		echo 'affected: '.$err['affected'].', fails: '.$err['fails'].', oks: '.$err['oks'].', last id: '.$err['id'].'<br/>';
	}
};

function TestCreateTables($db, $cb_ex) {
	$sql = 'Create database if not exists mysqldb character set utf8 collate utf8_general_ci;USE mysqldb;';
	$sql .=	'CREATE TABLE IF NOT EXISTS company(ID bigint PRIMARY KEY NOT NULL,name CHAR(64)NOT NULL,ADDRESS varCHAR(256)not null,Income Decimal(21,2)not null);';
	$sql .=	'CREATE TABLE IF NOT EXISTS employee(EMPLOYEEID bigint AUTO_INCREMENT PRIMARY KEY NOT NULL unique,CompanyId bigint not null,name CHAR(64)NOT NULL,JoinDate DATETIME(6)default null,IMAGE MEDIUMBLOB,DESCRIPTION MEDIUMTEXT,Salary DECIMAL(25,2),FOREIGN KEY(CompanyId)REFERENCES company(id));';
	$sql .= 'DROP PROCEDURE IF EXISTS sp_TestProc;CREATE PROCEDURE sp_TestProc(in p_company_id int,inout p_sum_salary DECIMAL(25,2),out p_last_dt datetime)BEGIN select * from employee where companyid>=p_company_id;select sum(salary)+p_sum_salary into p_sum_salary from employee where companyid>=p_company_id;select now()into p_last_dt;END';
	return $db->Execute($sql, $cb_ex);
}

function TestPreparedStatements($db, $cb_ex) {
	if (!$db->Prepare('INSERT INTO mysqldb.company(ID,NAME,ADDRESS,Income)VALUES(?,?,?,?)', null)) {
		return false;
	}
	$vParam = array(1, 'Google Inc.', '1600 Amphitheatre Parkway, Mountain View, CA 94043, USA', 66000010000.15,
		2, 'Microsoft Inc.', '700 Bellevue Way NE- 22nd Floor, Bellevue, WA 98804, USA', 93600000000.12,
		3, 'Apple Inc.', '1 Infinite Loop, Cupertino, CA 95014, USA', 234000000000.14
	);
	if (!$db->Execute($vParam, $cb_ex)) {
		return false;
	}
	return true;
}

function InsertBLOBByPreparedStatement($db, $cb_ex) {
	if (!$db->Prepare('insert into employee(CompanyId,name,JoinDate,image,DESCRIPTION,Salary)values(?,?,?,?,?,?)', null)) {
		return false;
	}
	$wstr = '';
	while (mb_strlen($wstr) < 256 * 1024) {
		$wstr .= '广告做得不那么夸张的就不说了，看看这三家，都是正儿八经的公立三甲，附属医院，不是武警，也不是部队，更不是莆田，都在卫生部门直接监管下，照样明目张胆地骗人。';
	}
	$str = '';
	while (strlen($str) < 512 * 1024) {
		$str .= 'The epic takedown of his opponent on an all-important voting day was extraordinary even by the standards of the 2016 campaign -- and quickly drew a scathing response from Trump.';
	}
	$buff = SpBuffer();

	$blob = SpBuffer();
	$blob->SaveString($wstr);
	//1st set
    //blob.PopBytes() -- convert all data inside blob memory into an array of bytes
    $buff->SaveObject(1)->SaveObject('Ted Cruz')->SaveObject(new DateTime())->SaveObject($blob->PopBytes())->SaveObject($wstr)->SaveObject('254000.15');

	$blob->SaveAString($str);
    //2nd set
    $buff->SaveObject(1)->SaveObject('Donald Trump')->SaveObject(new DateTime())->SaveObject($blob->PopBytes())->SaveObject($str)->SaveObject(20254000.35);

	$blob->SaveAString($str)->SaveString($wstr);
    //3rd set
    $buff->SaveObject(2)->SaveObject('Hillary Clinton')->SaveObject(new DateTime())->SaveObject($blob->PopBytes())->SaveObject($wstr)->SaveObject(6254000.42);
	if (!$db->Execute($buff, $cb_ex)) {
		return false;
	}
	return true;
}

$cbOutput = function($v, $proc) {
	if (!$proc) {
		//ignore regular records
	}
	else {
		echo 'Procedure output: ';
		echo var_dump($v).'<br/>';
	}
};

function TestStoredProcedure($db, $cb_ex) {
	global $cbOutput;
	if (!$db->Prepare('call mysqldb.sp_TestProc(?,?,?)', null)) {
		return false;
	}
	$vParam = array(1, 1.25, null, //1st set
     2, 1.14, null, //2nd set
     0, 2.18, null //3rd set
	);
	if (!$db->Execute($vParam, $cb_ex, $cbOutput)) {
		return false;
	}
	return true;
}

$vData = array();
$cbRow = function($v, $proc) {
	global $vData;
	$vData = array_merge($vData, $v);
};

$cbMeta = function($meta) {
	echo '<br/>Meta data: ';
	echo var_dump($meta).'<br/>';
};
$ok = false;
try {
	$db = GetSpHandler('masterdb');
	do {
		if (!TestCreateTables($db, $cb_err_ex)) {
			break;
		}
		if (!$db->Execute('delete from mysqldb.employee;delete from mysqldb.company', $cb_err_ex)) {
			break;
		}
		if (!TestPreparedStatements($db, $cb_err_ex)) {
			break;
		}
		if (!InsertBLOBByPreparedStatement($db, $cb_err_ex)) {
			break;
		}
		if (!TestStoredProcedure($db, $cb_err_ex)) {
			break;
		}
		$sql = "select actor_id, first_name from sakila.actor where actor_id between ? and ?";
		if(!$db->Prepare($sql, null)) {
			break;
		}

		$vParam = array(1, 3);
		$err = $db->Execute($vParam, true, $cbRow, $cbMeta);
		echo 'Data: ';
		echo var_dump($vData).'<br/><br/>';

		echo 'Final result: ';
		echo $cb_err_ex($err).'<br/>';

		$ok = true;
	} while(false);
	if($ok) {
		echo 'All requests executed successfully<br/>';
	}
	else {
		echo 'Session closed<br/>';
	}
	echo 'PHP script completed without exception';
} catch(Exception $e) {
	echo 'Caught exception: '.$e->getMessage();
}

?>
