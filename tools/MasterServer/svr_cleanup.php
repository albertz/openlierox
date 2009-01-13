<?

// Cleans up any servers that are older then 5 minutes
function svr_cleanup($connection)
{
   $ctime = time() - 5*60;
   $query = "DELETE FROM mastersvr WHERE lastping < $ctime";
  
   mysql_query($query, $connection);
}

?>