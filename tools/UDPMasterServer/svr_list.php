<?
// List the servers

require 'db.php';
require 'svr_cleanup.php';

$connection = db_connect();

// Clean up the server list
svr_cleanup($connection);

$query = "SELECT address, port FROM mastersvr";
$result = mysql_query($query, $connection);

// Show the servers
while( $row = mysql_fetch_array($result) ) {
   $addr = $row['address'];
   $port = $row['port'];
   echo "\"$addr\", \"$port\"\n";
} 

mysql_close();

?>