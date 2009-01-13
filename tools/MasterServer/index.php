<!-- Xtreme notepad html'ing -->
<html>
<head>
<title>Liero Xtreme site</title>
<? include('../style.php'); ?>
</head>
<body bgcolor="#441A11" text="#000000" link="#D0CC70" vlink="#D0CC70" alink="#D0CC70" background="../img/back.jpg">
<center>
 <img src="../img/title.gif"> 
</center>
<br><br>
<table width=100% border=0 cellspacing=0 cellpadding=2 bgcolor=#ffffff background="../img/back.jpg">
   <tr>
      <td width=15% valign=top>
         <!-- Side column table -->
		 <? include('../side.php'); 
		 ?>
      </td>
      <td valign=top>
         <!-- News tables -->

		 <!-- 11th June 2003 -->
         <table width=75% border=0 cellspacing=0 cellpadding=0 bgcolor=#0080FF>            
            <tr>
               <td class="newshead"><b>Server List</b></td>
            </tr>
         </table>
         <table width=75% border=0 cellspacing=0 cellpadding=0 bgcolor=#ffffff>
            <tr><td class="nav">


            <!-- PHP Code -->
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
			$count = 0;
			echo "<table border=1 width=50% cellspacing=3 cellpadding=3 bgcolor=#C0C0C0>\n";
			echo "<tr><td>Address</td><td>Port</td></tr>\n";
            while( $row = mysql_fetch_array($result) ) {
               $addr = $row['address'];
               $port = $row['port'];
			   //echo "\"$addr\", \"$port\"\n";
			   echo "<tr><td>$addr</td><td>$port</td></tr>\n";
            }
			echo "</table>"
            ?>
			<!-- End PHP Code -->


            </td></tr>
         </table>
         <br>
         <br>		
      </td>
   </tr>
</table>

</body>
</html>

