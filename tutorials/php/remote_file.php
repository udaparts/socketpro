<?php

$cb = function($em) {
    echo 'Async file exchange completed: ';
    echo var_dump($em) . '<br/>';
};
$ok = false;
try {
    $file = GetSpHandler('my_file');
    do {
        $dir = 'C:\\cyedev\\commlib\\boost_1_55_0\\stage\\lib64\\';
        $webroot = '/home/yye/sp_test/';
        //force processing requests left in client queue untile all of them is completed
        if (!$file->WaitAll()) {
            break;
        }
        //streaming files downloading and uploading
        if (!$file->Download($webroot . 'spfile1.test', $dir . 'jvm.lib', $cb)) {
            break;
        }
        echo 'Downloading jvm.lib started<br/>';
        if (!$file->Download($webroot . 'spfile2.test', $dir . 'libboost_wave-vc110-mt-1_55.lib', $cb)) {
            break;
        }
        echo 'Downloading libboost_wave-vc110-mt-1_55.lib started<br/>';
        if (!$file->Download($webroot . 'spfile3.test', $dir . 'libboost_log_setup-vc110-mt-gd-1_55.lib', $cb)) {
            break;
        }
        echo 'Downloading libboost_log_setup-vc110-mt-gd-1_55.lib started<br/>';
        if (!$file->Download($webroot . 'spfile4.test', $dir . 'libboost_graph-vc110-mt-gd-1_55.lib', $cb)) {
            break;
        }
        echo 'Downloading libboost_graph-vc110-mt-gd-1_55.lib started<br/>';
        $em = $file->Download($webroot . 'spfile5.test', $dir . 'libboost_filesystem-vc110-mt-1_55.lib', true); //sync
        echo 'Downloaded libboost_filesystem-vc110-mt-1_55.lib completed: ';
        echo var_dump($em) . '<br/><br/>';

        if (!$file->Upload($webroot . 'spfile1.test', $dir . 'jvm_copy.lib', $cb)) {
            break;
        }
        echo 'Uploading jvm.lib started<br/>';
        if (!$file->Upload($webroot . 'spfile2.test', $dir . 'libboost_wave-vc110-mt-1_55_copy.lib', $cb)) {
            break;
        }
        echo 'Uploading libboost_wave-vc110-mt-1_55.lib started<br/>';
        if (!$file->Upload($webroot . 'spfile3.test', $dir . 'libboost_log_setup-vc110-mt-gd-1_55_copy.lib', $cb)) {
            break;
        }
        echo 'Uploading libboost_log_setup-vc110-mt-gd-1_55.lib started<br/>';
        if (!$file->Upload($webroot . 'spfile4.test', $dir . 'libboost_graph-vc110-mt-gd-1_55_copy.lib', $cb)) {
            break;
        }
        echo 'Uploading libboost_graph-vc110-mt-gd-1_55.lib started<br/>';
        $em = $file->Upload($webroot . 'spfile5.test', $dir . 'libboost_filesystem-vc110-mt-1_55_copy.lib', true); //sync
        echo 'Uploaded libboost_filesystem-vc110-mt-1_55.lib completed: ';
        echo var_dump($em) . '<br/><br/>';
        $ok = true;
    } while (false);
    if ($ok) {
        echo 'PHP script completed without exception';
    } else {
        echo 'Session closed with error: ';
        echo var_dump($file->Socket->Error) . '<br/>';
    }
} catch (Exception $e) {
    echo 'Caught exception: ', $e->getMessage();
}
?>
