<?
// UDP masterserver script

$register_port = 23450;

require 'db.php';

set_time_limit(0);


if (($sock = socket_create(AF_INET, SOCK_DGRAM, SOL_UDP)) === false) {
    echo "socket_create() failed: reason: " . socket_strerror(socket_last_error()) . "\n";
	exit;
}
	
if (socket_bind($sock, "0.0.0.0", $register_port) === false) {
    echo "socket_bind() failed: reason: " . socket_strerror(socket_last_error($sock)) . "\n";
	exit;
}

$connection = db_connect();

function svr_cleanup($connection)
{
   $ctime = time() - 2*60;	// 2 minutes
   $query = "DELETE FROM udpmastersvr WHERE lastping < $ctime";
   mysql_query($query, $connection);
}

echo "UDP server started at port $register_port\n";

while(true)
{
	svr_cleanup($connection);
	$data = "";
	$source = "";
	$sourcePort = 0;
	$ret = socket_recvfrom($sock, $data, 1500, 0, $source, $sourcePort);
	if( $ret == -1 )
		break;

	$srcaddr = $source . ":" . strval($sourcePort);
	echo "Got from $srcaddr $data\n";

	// Format: "\xff\xff\xff\xfflx::traverse\0123.45.67.89:12345\0"
	if( strpos( $data, "\xff\xff\xff\xfflx::traverse" ) === 0 )
	{
		$data = substr( $data, strpos( $data, "\0" ) + 1 );
		$dest = substr( $data, 0, strpos( $data, "\0" ) );
		if( ! strpos( $dest, ":" ) )
			continue;
		$destPort = intval(substr( $dest, strpos( $dest, ":" )+1 ));
		$dest = substr( $dest, 0, strpos( $dest, ":" ) );
		$send = "\xff\xff\xff\xfflx::traverse\0" . $srcaddr . "\0";
		socket_sendto($sock, $send, strlen($send), 0, $dest, $destPort);
		echo "Send $send\n";
		continue;
	};

	if( strpos( $data, "\xff\xff\xff\xfflx::ping" ) === 0 )
	{
		$send = "\xff\xff\xff\xfflx::query\0\0";
		socket_sendto($sock, $send, strlen($send), 0, $source, $sourcePort);
		echo "Send $send\n";
		continue;
	};
	
	if( strpos( $data, "\xff\xff\xff\xfflx::queryreturn" ) === 0 )
	{
		$lastping = time();
		$data = substr( $data, strpos( $data, "\0" ) + 1 );
		$name = addslashes(substr( $data, 0, strpos( $data, "\0" ) ));
		$data = substr( $data, strpos( $data, "\0" ) + 1 );
		$numplayers = intval(ord( $data[0] ));
		$maxworms = intval(ord( $data[1] ));
		$state = intval(ord( $data[2] ));
		$query = "SELECT addr FROM udpmastersvr WHERE addr = \"$addr\"";
		$result = mysql_query($query, $connection);
		if( mysql_num_rows($result) > 0 )
			$query = "UPDATE udpmastersvr SET lastping=$lastping, name=\"$name\", " .
					"maxworms=$maxworms, numplayers=$numplayers, state=$state WHERE addr = \"$srcaddr\"";
		else
			$query = "INSERT INTO udpmastersvr (addr, lastping, name, maxworms, numplayers, state) " .
		            "VALUES (\"$srcaddr\", $lastping, \"$name\", $maxworms, $numplayers, $state)";
		$result = mysql_query($query, $connection);
		echo "Updated database $query\n";
		continue;
	};
	if( strpos( $data, "\xff\xff\xff\xfflx::getserverlist" ) === 0 )
	{
		$query = "SELECT addr, name, maxworms, numplayers, state FROM udpmastersvr";
		$result = mysql_query($query, $connection);
		$amount = mysql_num_rows($result);
		echo "Amount of items in database $amount\n";
		$send = "";
		$amount1 = 0;
		for( $f = 0; $f<$amount; $f++, $amount1++ )
		{
			if( strlen($send) >= 255 or $amount1 >= 255 )
			{
				$send = "\xff\xff\xff\xfflx::serverlist\0" . chr(strval($amount1)) . $send;
				socket_sendto($sock, $send, strlen($send), 0, $source, $sourcePort);
				echo "Send $send\n";
				$send = "";
				$amount1 = 0;
			}
			$row = mysql_fetch_array($result);
			$send .= $row["addr"] . "\0" . $row["name"] . "\0" . 
						chr(strval($row["numplayers"])) . 
						chr(strval($row["maxworms"])) . 
						chr(strval($row["state"]));
		};
		$send = "\xff\xff\xff\xfflx::serverlist\0" . chr(strval($amount1)) . $send;
		socket_sendto($sock, $send, strlen($send), 0, $source, $sourcePort);
		echo "Send $send\n";
		continue;
	};
};

socket_close($sock);

?>
