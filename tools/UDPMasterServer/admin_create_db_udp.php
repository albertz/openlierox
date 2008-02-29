<?

$user = $_GET['user'];
$pass = $_GET['pass'];

require 'db.php';

//if ( $user != $admin_user or $pass != $admin_password )
//	die("Error = \"Wrong username/password\"\n");

$connection = db_connect();

/*
$query = "DROP TABLE mastersvr";

$result = mysql_query($query, $connection);

$query = "CREATE TABLE mastersvr (address VARCHAR(50) NOT NULL, port VARCHAR(10) NOT NULL, lastping INT NOT NULL)";

echo "\n\n" . $query . "\n\n";

$result = mysql_query($query, $connection) or die("SQL error: " . mysql_error() . "\n");
*/

$query = "DROP TABLE udpmastersvr";

$result = mysql_query($query, $connection);


$query = "CREATE TABLE udpmastersvr (addr VARCHAR(50) NOT NULL PRIMARY KEY, lastping INT NOT NULL, " .
			"name VARCHAR(255) NOT NULL, maxworms INT NOT NULL, numplayers INT NOT NULL, state INT NOT NULL)";

/*
// This table for future advanced UDP masterserver
$query = "CREATE TABLE udpmastersvr (addr VARCHAR(50) NOT NULL PRIMARY KEY, lastping INT NOT NULL, " .
			"name VARCHAR(255) NOT NULL, maxworms INT NOT NULL, numplayers INT NOT NULL, state INT NOT NULL, ".
			"gamemap VARCHAR(255) NOT NULL DEFAULT \"\", gamemod VARCHAR(255) NOT NULL DEFAULT \"\", " .
			"gametype INT NOT NULL DEFAULT 0, lives INT NOT NULL DEFAULT 0, maxkills INT NOT NULL DEFAULT 0, " .
			"loadingtime INT NOT NULL DEFAULT 0, bonuses INT NOT NULL DEFAULT 0";
for( $f=0; $f<32; $f++ )
{
	$query .= ", wormname" . strval($f) . " VARCHAR(255) NOT NULL DEFAULT \"\"" .
				", wormlives" . strval($f) . " INT NOT NULL DEFAULT -1" .
				", wormkills" . strval($f) . " INT NOT NULL DEFAULT 0" ;
};
$query .= " )";
*/

echo "\n\n" . $query . "\n\n";

$result = mysql_query($query, $connection) or die("SQL error: " . mysql_error() . "\n");

echo "Tables created okay\n";

?>
