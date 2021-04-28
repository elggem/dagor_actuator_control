<?PHP

header('Content-type: text/plain; charset=utf8', true);

function check_header($name, $value = false) {
    if(!isset($_SERVER[$name])) {
        return false;
    }
    if($value && $_SERVER[$name] != $value) {
        return false;
    }
    return true;
}

function sendFile($path) {
    header($_SERVER["SERVER_PROTOCOL"].' 200 OK', true, 200);
    header('Content-Type: application/octet-stream', true);
    header('Content-Disposition: attachment; filename='.basename($path));
    header('Content-Length: '.filesize($path), true);
    header('x-MD5: '.md5_file($path), true);
    readfile($path);
}

if(!check_header('HTTP_USER_AGENT', 'ESP32-http-Update')) {
    header($_SERVER["SERVER_PROTOCOL"].' 403 Forbidden', true, 403);
    echo "only for ESP32 updater!\n";
    exit();
}

if(
    !check_header('HTTP_X_ESP32_STA_MAC') ||
    !check_header('HTTP_X_ESP32_AP_MAC') ||
    !check_header('HTTP_X_ESP32_FREE_SPACE') ||
    !check_header('HTTP_X_ESP32_SKETCH_SIZE') ||
    !check_header('HTTP_X_ESP32_CHIP_SIZE') ||
    !check_header('HTTP_X_ESP32_SDK_VERSION') ||
    !check_header('HTTP_X_ESP32_VERSION')
) {
    header($_SERVER["SERVER_PROTOCOL"].' 403 Forbidden', true, 403);
    echo "only for ESP32 updater! (header)\n";
    exit();
}

$db = array(
    "8C:CE:4E:8E:DC:30" => "DAGOR-DEV-v6",
    "8C:CE:4E:8E:32:88" => "DAGOR-DEV-v6"
);

if(isset($db[$_SERVER['HTTP_X_ESP32_STA_MAC']])) {
    if($db[$_SERVER['HTTP_X_ESP32_STA_MAC']] != $_SERVER['HTTP_X_ESP32_VERSION']) {
        sendFile("./".$db[$_SERVER['HTTP_X_ESP32_STA_MAC']].".bin");
    } else {
        header($_SERVER["SERVER_PROTOCOL"].' 304 Not Modified', true, 304);
    }
    exit();
}

header($_SERVER["SERVER_PROTOCOL"].' 500 no version for ESP MAC', true, 500);
