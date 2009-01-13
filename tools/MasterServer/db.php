<?

////////////////
// Open a connection to the database
function db_connect()
{
   $host = "localhost";
   $user = ""; // TODO
   $pass = ""; // TODO
   $database = ""; // TODO

   // Connect to the server
   $connect = @mysql_connect($host, $user, $pass) or die("Error = \"could not connect to server\""); 

   // Select the database
   $db_select = @mysql_select_db($database) or die("Error = \"could not select the database\"");

   return $connect;
}

?>
