<?php

$dir = 'C:\\cyedev\\commlib\\boost_1_55_0\\stage\\lib64\\';
$webroot = '/home/yye/sp_test/';

try {
    $file = GetSpHandler('my_file');
    do {
        //force processing requests left in client queue untile all of them is completed
        if (!$file->WaitAll()) {
            echo 'Session closed <br/>';
            break;
        }
        //streaming files downloading and uploading
        if (!$file->Download($webroot . 'spfile1.test', $dir . 'jvm.lib', function($em) {
                    echo 'File download result: ';
                    echo var_dump($em) . '<br/>';
                }, function($canceled) {
                    echo $canceled ? 'Download canceled<br/>' : 'Session closed <br/>';
                })
        ) {
            echo 'Session closed <br/>';
            break;
        }
        echo 'Downloading jvm.lib<br/>';

        $em = $file->Upload($webroot . 'spfile1.test', $dir . 'jvm_copy.lib', true);
        echo 'Uploaded jvm.lib completed: ';
        echo var_dump($em) . '<br/><br/>';
    } while (false);
    echo 'PHP script completed without exception';
} catch (Exception $e) {
    echo 'Caught exception: ', $e->getMessage();
}
?>
