<?
// Register a server

require 'db.php';
require 'svr_cleanup.php';

$port = $_GET['port'];
$ip = $_GET['ip'];

if($addr == "")
	$ip = $GLOBALS['REMOTE_ADDR'];

$lastping = time();

$connection = db_connect();

// Clean up the server list
svr_cleanup($connection);

// Check if this server already exists
$query = "SELECT address, port FROM mastersvr WHERE address = \"$ip\" AND port=\"$port\"";

$result = mysql_query($query, $connection);

// Internal error
if($result == 0) {
   echo "internal error\n";
   exit;
}


if( mysql_num_rows($result) > 0 ) {
   // Server already exists in list, so update the last_ping field

   $query = "UPDATE mastersvr SET lastping=\"$lastping\" ".
   "WHERE address = \"$ip\" AND port=\"$port\"";
} else {
   // Add server into the list
   
   $query = "INSERT INTO mastersvr (address, port, lastping) ".
            "VALUES (\"$ip\", \"$port\", \"$lastping\")";
}

$result = mysql_query($query, $connection);


?>