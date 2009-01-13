<?
// De-Register a server

require 'db.php';
require 'svr_cleanup.php';

$port = $_GET['port'];
$addr = $GLOBALS['REMOTE_ADDR'];
$lastping = time();

$connection = db_connect();

// Clean up the server list
svr_cleanup($connection);

// Check if this server already exists
$query = "DELETE FROM mastersvr WHERE port = \"$port\" AND address = \"$addr\"";

$result = mysql_query($query, $connection);

// Internal error
if($result == 0) {
   echo "internal error\n";
   exit;
}

?>