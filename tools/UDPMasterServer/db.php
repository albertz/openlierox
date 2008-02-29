<?

////////////////
// Open a connection to the database

function db_connect()
{
   $user = "thegamin_lierox";
   $password = "secret";
   $host = "localhost";
   $database = "thegamin_lieroxtreme";

   // Connect to the server
   $connect = @mysql_connect($host, $user, $password) or die("Error = \"could not connect to server\""); 

   // Select the database
   $db_select = @mysql_select_db($database) or die("Error = \"could not select the database\"");

   return $connect;
}

?>