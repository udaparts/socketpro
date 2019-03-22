<?php

const idMessage0 = SPA\BaseID::idReservedTwo + 100;
const idMessage1 = SPA\BaseID::idReservedTwo + 101;
const idMessage2 = SPA\BaseID::idReservedTwo + 102;
const idMessage3 = SPA\BaseID::idReservedTwo + 103;
const idMessage4 = SPA\BaseID::idReservedTwo + 104;
const TEST_QUEUE_KEY = 'queue_name_0';

function testEnqueue($sq) {
    $idMsg = 0;
    $ok = true;
    $buff = SpBuffer();
    echo 'Going to enqueue 1024 messages ......<br/>';
    for ($n = 0; $n < 1024; ++$n) {
        $str = '' . $n . ' Object test';
        switch ($n % 3) {
            case 0:
                $idMsg = idMessage0;
                break;
            case 1:
                $idMsg = idMessage1;
                break;
            default:
                $idMsg = idMessage2;
                break;
        }
        $ok = $sq->Enqueue(TEST_QUEUE_KEY, $idMsg, $buff->SaveString('SampleName')->SaveString($str)->SaveInt($n));
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
    $ok = false;
    do {
        if (!$sq->StartTrans(TEST_QUEUE_KEY)) {
            break;
        }
        if (!testEnqueue($sq)) {
            break;
        }
        $buff = SpBuffer();
        $sq->BatchMessage(idMessage3, $buff->SaveString('Hello')->SaveString('World'));
        $buff->Size = 0; //reset buffer size to 0
        $sq->BatchMessage(idMessage4, $buff->SaveBool(true)->SaveDouble(234.456)->SaveString('MyTestWhatever'));
        if (!$sq->EnqueueBatch(TEST_QUEUE_KEY)) {
            break;
        }
        if (!$sq->EndTrans(false)) { //false -- commit, true -- rollback
            break;
        }
        if (!$sq->GetKeys(function($keys) {
                    echo 'Keys opened: ';
                    echo var_dump($keys) . '<br/>';
                })) {
            break;
        }
        $res = null;
        do {
            //Dequeue is always synchronous
            $res = $sq->Dequeue(TEST_QUEUE_KEY, function($q, $reqId) {
                switch ($reqId) {
                    case idMessage0:
                    case idMessage1:
                    case idMessage2:
                        echo $q->LoadString() . ':' . $q->LoadString() . ' ' . $q->LoadInt() . '<br/>';
                        break;
                    case idMessage3:
                        echo $q->LoadString() . ' ' . $q->LoadString() . '<br/>';
                        break;
                    case idMessage4:
                        echo 'Bool: ' . $q->LoadBool() . ', double: ' . $q->LoadDouble() . ', string: ' . $q->LoadString() . '<br/>';
                        break;
                    default:
                        throw new Exception("Unsupported message");
                }
            });
            echo var_dump($res) . '<br/>';
        } while ($res['messages'] > 0);
        $ok = true;
    } while (false);
    if ($ok) {
        echo 'All requests executed successfully<br/>';
    } else {
        echo 'Session closed<br/>';
    }
    echo 'PHP script completed without exception';
} catch (Exception $e) {
    echo 'Caught exception: ', $e->getMessage();
}
?>
