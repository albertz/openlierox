<?php
// Liero Xtreme utilities for PHP
// Code released under the LGPL license
// Created on 18/02/2009 by Karel Petranek

// Requirements:
// PHP 4 or 5
// gd (for the image processing functions) and zlib modules

// This file contains functions for working with (Open)LieroX network
// Allows you to obtain server list, information about servers, pings and more

// Includes
require_once "HttpClient.class.php";

// Nat types
define("NAT_NONE", 0); 
define("NAT_SYMMETRIC", 1);
define("NAT_RESTRICTED", 2);

// Variable types for advanced features
define("SVT_BOOL", 0);
define("SVT_INT", 0);
define("SVT_FLOAT", 0);
define("SVT_STRING", 0);
define("SVT_COLOR", 0);

// Structures

////////////////////////////
// Worm info structure
// $Name - name of the worm
// $Lives - number of lives (only for OpenLieroX beta3+ servers)
// $Kills - number of kills
// $IP - IP address of the player
class WormInfo  {
  var $Name;
  var $Lives;
  var $Kills;
  var $IP;
}


///////////////////////////
// Advanced feature structure
// Serves as a helper struct for server info structure
// $Name - name of the feature
// $HumanName - human-readable fancy name
// $Value - value of the feature
// $ValueType - data type of the value, can be SVT_BOOL, SVT_INT, SVT_FLOAT, SVT_STRING or SVT_COLOR
// $OlderClientsSupported - true if older clients can join when the feature is enabled
class AdvancedFeature  {
  var $Name;
  var $HumanName;
  var $Value;
  var $ValueType;
  var $OlderClientsSupported;
}

///////////////////////////
// Server info structure
// $Name - name of the server
// $Ping - ping (in milliseconds) to the server
// $MaxPlayers - server capacity
// $NumPlayers - current number of players on the server
// $State - state of the server (Open/Playing/Loading)
// $MapName - name of the map
// $ModName - name of the mod
// $GameMode - game mode (Deathmatch, Team Deathmatch, Tag, ...)
// $Lives - number of lives each worm has at the beginning of the game
// $MaxKills - number of kills that needs to be reached to win
// $LoadingTime - weapon reloading time
// $BonusesOn - bonuses on/off
// $Worms - array of WormInfo structure
// $NatType - type of the NAT the server is behind, possible values are NAT_NONE, NAT_SYMMETRIC and NAT_RESTRICTED
// $Version - OpenLieroX version the server is running
// $GameSpeed - game speed multiplicator
// $AdvancedFeatures - array of all the advanced features (see AdvancedFeature structure) the server has (e.g. allow connecting during game), available since OpenLieroX beta 9
class ServerInfo  {
  var $Addr;
  var $Name;
  var $Ping;
  var $MaxPlayers;
  var $NumPlayers;
  var $State;
  var $MapName;
  var $ModName;
  var $GameMode;
  var $Lives;
  var $MaxKills;
  var $LoadingTime;
  var $BonusesOn;
  var $Worms;
  var $NatType;
  var $Version;
  var $GameSpeed;
  var $AdvancedFeatures;
}

//////////////////////////
// Fast server info (this one is shown in serverlist in LX)
// The items have the same meaning as in the above structure
class FastServerInfo  {
  var $Addr;
  var $Name;
  var $Ping;
  var $MaxPlayers;
  var $NumPlayers;
  var $State;
}

// Public functions

/////////////////////////
// Gets the server info for a server specified in $ip
// Parameters:
// $ip - IP address and (optionally) port separated by :
// $timeout - how long (in milliseconds) to wait for a reply
// $socket - socket to send/receive from, by default allocate socket automatically.
// Return value:
// false on error
// ServerInfo structure on success
function LXServerInfo($ip, $timeout = 2000, $socket = false)
{
  // Build the packet
  $packet = chr(0xFF) . chr(0xFF) . chr(0xFF) . chr(0xFF);
  $packet .= "lx::getinfo" . chr(0x00);
  
  // Send the packet
  $res = SendPacketAndWaitResponses($ip, $packet, $timeout, 1, $socket);
  if (!$res)
    return false;
     
  list($response, $ping, $remoteaddr) = $res[0];
  
  // Check the response  
  if (!$response)
    return false;
    
  // Parse the response
  
  // Check for connectionless header
  if (ord($response[0]) != 0xFF && 
      ord($response[1]) != 0xFF &&
      ord($response[2]) != 0xFF &&
      ord($response[3]) != 0xFF)
      return false;
  $response = substr($response, 4);
  
  // Set default version
  $version = "LieroX/0.56";
  
  // Check the command
  $command = BinToStr($response);
  if ($command != "lx::serverinfo")
    return false;
  $response = substr($response, strlen($command) + 1);  // +1 - null termination
  
  // Server name
  $name = BinToStr($response);
  $response = substr($response, strlen($name) + 1);
  
  // Max players
  $max_players = ord($response[0]);
  $response = substr($response, 1);
  
  // State
  $states = Array("Open", "Loading", "Playing");
  $state = ord($response[0]);
  if ($state >= 0 && $state < count($states))
    $state = $states[$state]; // Convert to string representation
  else
    $state = "Unknown";
  $response = substr($response, 1);
  
  // Map name
  $map_name = BinToStr($response);
  $response = substr($response, strlen($map_name) + 1);
  if (strpos($map_name, "levels/") === 0)
    $map_name = substr($map_name, 7);  
  
  // Mod name
  $mod_name = BinToStr($response);
  $response = substr($response, strlen($mod_name) + 1);
  
  // Game mode
  $game_modes = Array("Deathmatch", "Team Deathmatch", "Tag", "Demolitions");
  $game_mode = ord($response[0]);
  if ($game_mode >= 0 && $game_mode < count($game_modes))
    $game_mode = $game_modes[$game_mode];  // Convert to string representation
  else
    $game_mode = "Unknown";
  $response = substr($response, 1);
  
  // Lives
  $lives = BinToInt16BE($response);
  if ($lives == 65535)
    $lives = "unlimited";
  $response = substr($response, 2);
  
  // Max kills
  $max_kills = BinToInt16BE($response);
  if ($max_kills == 65535)
    $max_kills = "unlimited";
  $response = substr($response, 2);
  
  // Loading time
  $loading = BinToInt16BE($response);
  $response = substr($response, 2);
  
  // Bonuses
  $bonuses_on = ord($response[0]) != 0;
  $response = substr($response, 1);
  
  // Number of players
  $num_players = ord($response[0]);
  $num_players = $num_players > $max_players ? $max_players : $num_players;
  $response = substr($response, 1);
  
  // Worm info
  $worms = Array();
  for ($i = 0; $i < $num_players && strlen($response) > 0; $i++)  {
    $worms[$i] = new WormInfo;
    
    // Worm name
    $worms[$i]->Name = BinToStr($response);
    $response = substr($response, strlen($worms[$i]->Name) + 1);
    
    // Kills
    $worms[$i]->Kills = BinToInt16LE($response);
    $response = substr($response, 2);
    
    $worms[$i]->Lives = "&ndash;"; // We don't know the lives yet
    $worms[$i]->IP = ""; // We don't know the IP yet
  }
  
  // Lives (only OLX beta 3+)
  $version = "OpenLieroX/0.57_beta3";
  if (strlen($response) >= $num_players * 2)  {
    for ($i = 0; $i < $num_players; $i++)  {
      $worms[$i]->Lives = BinToInt16LE($response);
      if ($worms[$i]->Lives == 65535)
        $worms[$i]->Lives = "out";
      if ($worms[$i]->Lives == 65534)
        $worms[$i]->Lives = "&ndash;";
      $response = substr($response, 2);    
    }
  }
  
  // IP addresses (only OLX beta 4+)
  $version = "OpenLieroX/0.57_beta4";
  if (strlen($response))  {
    for ($i = 0; $i < $num_players; $i++)  {
      $worms[$i]->IP = BinToStr($response);
      $response = substr($response, strlen($worms[$i]->IP) + 1);
    }
  }
  
  // Version (beta 5+)
  if (strlen($response))  {
    $version = BinToStr($response);
    $response = substr($response, strlen($version) + 1);
  }
  
  // Game speed
  $game_speed = 1.0;
  if (strlen($response))  {
    $game_speed = BinToFloatLE($response);
    $response = substr($response, 4);  
  }
  
  // Features
  $features = Array();
  if (strlen($response))  {
    $ft_count = BinToInt16LE($response);  // Feature count
    $response = substr($response, 2);
    
    // Read the features
    for ($i = 0; $i < $ft_count; $i++)  {
      $ft = new AdvancedFeature();
      
      // Name
      $ft->Name = BinToStr($response);
      $response = substr($response, strlen($ft->Name) + 1);
      
      // Human name
      $ft->HumanName = BinToStr($response);
      $response = substr($response, strlen($ft->HumanName) + 1);
      
      // Value
      $ft->ValueType = ord($response[0]);
      $response = substr($response, 1);
      switch ($ft->ValueType)  {
      case SVT_BOOL:
        $ft->Value = (ord($response[0]) != 0);
        $response = substr($response, 1);
      break;
      case SVT_INT:
        $ft->Value = BinToInt32LE($response);
        $response = substr($response, 4);
      break;
      case SVT_FLOAT:
        $ft->Value = BinToFloatLE($response);
        $response = substr($response, 4);
      break;
      case SVT_STRING:
        $ft->Value = BinToStr($response);
        $response = substr($response, strlen($ft->Value) + 1);
      break;
      case SVT_COLOR:
        $ft->Value = Array(ord($response[0]), ord($response[1]), ord($response[2]), ord($response[3]));
        $response = substr($response, 4);
      break;
      default:
        $ft->Value = 0;                     
      }   
      
      // Old clients supported
      $ft->OlderClientsSupported = (ord($response[0]) != 0);
      $response = substr($response, 1);   
      
      $features[] = $ft; // Add        
    }
  }
  
  // Game mode (as string, since beta 9)
  if (strlen($response))  {
    $game_mode = BinToStr($response);  // Overwrite the original one
    $response = substr($response, strlen($game_mode) + 1);
  }

  
  // Fill in the info
  $returnValue = new ServerInfo;
  $returnValue->Name = $name;
  $returnValue->Ping = $ping;
  $returnValue->MaxPlayers = $max_players;
  $returnValue->NumPlayers = $num_players;
  $returnValue->State = $state;
  $returnValue->MapName = $map_name;
  $returnValue->ModName = $mod_name;
  $returnValue->GameMode = $game_mode;
  $returnValue->Lives = $lives;
  $returnValue->MaxKills = $max_kills;
  $returnValue->LoadingTime = $loading;
  $returnValue->BonusesOn = $bonuses_on;
  $returnValue->Worms = $worms;
  $returnValue->NatType = NAT_NONE;
  $returnValue->Addr = $ip;
  $returnValue->Version = $version;
  $returnValue->GameSpeed = $game_speed;
  $returnValue->AdvancedFeatures = $features;
  
  return $returnValue;
}


///////////////////////////
// Obtains a fast info about a server
// Parameters:
// $ip - ip of the server, optionally a port separated by :
// $timeout - timeout in milliseconds
// Return value:
// false when failed
// FastServerInfo structure when success
function LXFastInfo($ip, $timeout = 2000)
{
   
  // Build the packet
  $packet = chr(0xFF) . chr(0xFF) . chr(0xFF) . chr(0xFF);
  $packet .= "lx::query" . chr(0x00) . chr(0x00);
  
  // Send the packet
  $res = SendPacketAndWaitResponses($ip, $packet, $timeout, 1);
  if (!$res)
    return false;
    
  list($response, $ping, $remoteaddr) = $res[0];
  
  // Check the response  
  if (!$response)
    return false;
    
  // Parse the response
  
  // Check for connectionless header
  if (ord($response[0]) != 0xFF && 
      ord($response[1]) != 0xFF &&
      ord($response[2]) != 0xFF &&
      ord($response[3]) != 0xFF)
      return false;
  $response = substr($response, 4);
  
  // Check the command
  $command = BinToStr($response);
  if ($command != "lx::queryreturn")
    return false;
  $response = substr($response, strlen($command) + 1);  // +1 - null termination
  
  // Server name
  $name = BinToStr($response);
  $response = substr($response, strlen($name) + 1);

  // Player count
  $num_players = ord($response[0]);
  $response = substr($response, 1);
  
  // Max players
  $max_players = ord($response[0]);
  $response = substr($response, 1);
  
  // State
  $states = Array("Open", "Loading", "Playing");
  $state = ord($response[0]);
  if ($state >= 0 && $state < count($states))
    $state = $states[$state]; // Convert to string representation
  else
    $state = "Unknown";
  $response = substr($response, 1);

  // Fill in the info
  $returnValue = new FastServerInfo;
  $returnValue->Name = $name;
  $returnValue->Ping = $ping;
  $returnValue->NumPlayers = $num_players;
  $returnValue->MaxPlayers = $max_players;
  $returnValue->State = $state;
  $returnValue->Addr = $ip;

  return $returnValue;
}

///////////////////////////
// Pings a server
// Parameters:
// $ip - ip of the server, optionally a port separated by :
// $timeout - timeout in milliseconds
// Return value:
// false when failed
// ping in milliseconds when success
function LXPingServer($ip, $timeout = 2000)
{  
  // Build the packet
  $packet = chr(0xFF) . chr(0xFF) . chr(0xFF) . chr(0xFF); // Header
  $packet .= "lx::ping" . chr(0x00);

  // Send the packet
  $res = SendPacketAndWaitResponses($ip, $packet, $timeout, 1);
  if (!$res)
    return false;
    
  list($response, $ping, $remoteaddr) = $res[0];
  
  // Check the response  
  if (!$response)
    return false;
    
  // Check the response
  
  // Check for connectionless header
  if (ord($response[0]) != 0xFF && 
      ord($response[1]) != 0xFF &&
      ord($response[2]) != 0xFF &&
      ord($response[3]) != 0xFF)
      return false;
  $response = substr($response, 4);
  
  // Check the command
  $command = BinToStr($response);
  if ($command != "lx::pong")
    return false;

  return $ping;
}

///////////////////////////////
// Gets the server list from specified servers
// Parameters:
// $masterservers - array of masterserver addresses
// Return value:
// array of server IPs
// Special thanks: Bram Ueffing for providing his code!
function LXGetServerList($masterservers)
{
  // Defines
  $pattern = '/"(.*?)",.*?"(.*?)"/i'; // Regexp patern for parsing

  // Get info from all servers in the list
  $returnValue = Array();
  for ($i = 0; $i < count($masterservers); $i++)  {
    // Get the server list
    $buffer = HttpClient::quickGet($masterservers[$i]);
    
    // Parse the server list
    preg_match_all($pattern, $buffer, $result);
    
    // Join the IP and port to one
    $ips_with_port = Array();
    for ($i = 0; $i < count($result[1]); $i++)
      $ips_with_port[$i] = $result[1][$i] . ":" . $result[2][$i];
    
    // Add the servers from this master server to the result
    // HINT: array_merge also excludes doubled values
    if ($i > 0)
      $returnValue = array_merge($returnValue, $ips_with_port);
    else
      $returnValue = $ips_with_port;
  }
  
  return $returnValue;
}


///////////////////////////////
// Gets the server list from specified UDP masterservers
// Parameters:
// $masterservers - array of masterserver addresses
// Return value:
// array of server IPs:ports and server info
function LXGetUdpServerList($masterservers, $timeout = 3000)
{
	foreach ($masterservers as $ip)  {
		$packet = chr(0xFF) . chr(0xFF) . chr(0xFF) . chr(0xFF); // Header
		$packet .= "lx::getserverlist" . chr(0x00);
		
		// Send the packet
		$res = SendPacketAndWaitResponses($ip, $packet, $timeout);
		if (!$res)
		  return false;
		$ret = Array();
		  
		foreach ($res as $response2)  {
			$response = $response2[0];
			
			// Check for connectionless header
			if (ord($response[0]) != 0xFF && 
			    ord($response[1]) != 0xFF &&
			    ord($response[2]) != 0xFF &&
			    ord($response[3]) != 0xFF)
			    continue;
			$response = substr($response, 4);
			
			// Check for a correct response
			if (strpos($response, "lx::serverlist".chr(0x00)) !== 0)
				continue;
			$response = substr( $response, strlen("lx::serverlist".chr(0x00))+1 );
			
			// Read the response
			while (strlen($response ) > 0)  {
				$info = new FastServerInfo;
				$info->Ping = 999;  // Ping cannot be determined for NAT servers
				$pos1 = strpos($response, chr(0x00));
				$info->Addr = substr($response, 0, $pos1);
				$pos1 += 1;
				$pos2 = strpos($response, chr(0x00), $pos1);
				$info->Name = substr($response, $pos1, $pos2 - $pos1);
				$pos2 += 1;
				$info->NumPlayers = ord($response[$pos2]);
				$pos2 += 1;
				$info->MaxPlayers = ord($response[$pos2]);
				$pos2 += 1;
				$info->State = ord($response[$pos2]);
				$pos2 += 1;
				$response = substr($response, $pos2);
				$ret[] = $info;
			}
		}
	}
	if ($ret == Array())
	  return false;
	return $ret;
}

/////////////////////////
// Gets the server info for a server specified in $ip, using UDP masterserver in $master
// Parameters:
// $ip - IP address and (optionally) port separated by :
// $master - IP address and (optionally) port separated by : of UDP masterserver
// $timeout - how long (in milliseconds) to wait for a reply
// Return value:
// false on error
// ServerInfo structure on success
function LXUdpServerInfo($ip, $master, $timeout = 6000)
{
  $time_start = CurrentTime();
	if (LXPingServer($ip, $timeout / 2) !== false)
		return LXServerInfo($ip, $timeout);	// Server is NOT behind NAT
		
	// Update the timeout
	$timeout -= CurrentTime() - $time_start;
	if ($timeout <= 0)
    return false;
		
	// Build the packet
	$packet = chr(0xFF) . chr(0xFF) . chr(0xFF) . chr(0xFF); // Header
	$packet .= "lx::traverse" . chr(0x00);
	$packet .= $ip;
	
  // Open the socket
	$socket = OpenUdpSocket();
	if (!$socket)
		return false;
		
	// Send
	$responses = SendPacketAndWaitResponses($master, $packet, $timeout, -1, $socket);
	if ($responses == false)
		return false;
		
	// Parse the response
	$realAddr = $ip;
	$traverseAddr = "";
	$connechHereAddr = "";
	foreach( $responses as $resp )  {
		if (strpos($resp[0], chr(0xFF).chr(0xFF).chr(0xFF).chr(0xFF)."lx::connect_here".chr(0x00) ) === 0)
			$connechHereAddr = $resp[2];

		if (strpos($resp[0], chr(0xFF).chr(0xFF).chr(0xFF).chr(0xFF)."lx::traverse".chr(0x00) ) === 0)  {
			$traverseAddr = substr($resp[0], strlen(chr(0xFF).chr(0xFF).chr(0xFF).chr(0xFF)."lx::traverse".chr(0x00)));
			$traverseAddr = substr($traverseAddr, 0, strpos($traverseAddr, chr(0x00)) );
		}
	}
	
	// Get address to query
	if ($connechHereAddr != "")
		$realAddr = $connechHereAddr;
	else if ($traverseAddr != "")
		$realAddr = $traverseAddr;

  // Try to get the server info
	$ret = LXServerInfo($realAddr, $timeout, $socket);
  fclose($socket);
  
	if ($ret == false)
		return false;
	
  // NAT type	
	$ret->NatType = NAT_SYMMETRIC;
	if ($traverseAddr == $connechHereAddr && $traverseAddr != "")
		$ret->NatType = NAT_RESTRICTED;
		
	// Real IP
	$ret->Addr = $realAddr;
	
	return $ret;
}



// Private functions

////////////////////////////
// Private function, returns current time
function CurrentTime()
{
  $t = gettimeofday();
  return ($t["sec"] * 1000) + ($t["usec"] / 1000);
}



///////////////////////////
// Private function, opens connectionless UDP socket on given port, or random port by default
// Parameters:
// $port - port number, 0 = random
// Return value:
// opened stream_socket handle, or NULL
function OpenUdpSocket($port = 0)
{
  $errno = 0;
  $errorstr = "";
  if($port)
    return stream_socket_server("udp://0.0.0.0:" . strval($port), $errno, $errorstr, STREAM_SERVER_BIND);
  
  // Open the socket
  $fp = false;
  $count = 50;
  while( ! $fp && $count > 0 )  {
    $fp = stream_socket_server("udp://0.0.0.0:" . strval(rand(1024, 65535)), $errno, $errorstr, STREAM_SERVER_BIND);
	  $count -= 1;
  }
  
  return $fp;
}

//////////////////////////
// Private function, sends a packet to server and waits for one or more responce packets
// Parameters:
// $ip - ip[:port]
// $packet - packet data
// $timeout - timeout in milliseconds
// $maxreplies - maximum amount of reply packets to return, 
// -1 - no limit, 0 - send packet and return immediately, 1 - return only first responce.
// $socket - send with specified socket, if false allocate/free socket automatically.
// Return value:
// array with Array(responce, ping, remote address), false on failure.
function SendPacketAndWaitResponses($ip, $packet, $timeout, $maxreplies = -1, $socket = false)
{
  // Adjust the address
  if (!strpos($ip, ":"))
    $ip .= ":23400"; // Append default LX port
       
  // Open the socket
  if($socket)
  	$fp = $socket;
  else
    $fp = OpenUdpSocket();
  if (!$fp)
    return false;
    
  // Setup the sent time for ping calculation
  $sent_time = CurrentTime(); 
  
  // Send the packet 
//echo "Sent packet to " . strval($ip) .": " . strval($packet) . "<br>\n"; flush();
  stream_socket_sendto($fp, $packet, 0, $ip);

  // Set the timeout
  
  // Read the response
  $response = Array();
  while( CurrentTime() - $sent_time < $timeout && ( $maxreplies < 0 || count($response) < $maxreplies ) )  {
  	// stream_set_timeout() does not work with stream_socket_recvfrom() - using select()
    //stream_set_timeout($fp, 0, ( $timeout + $sent_time - CurrentTime() ) * 1000);
    
    // Wait for data
  	$r = Array($fp);
  	$w = NULL;
  	$e = NULL;
  	if (stream_select($r, $w, $e, 0, 500000) == 0)
  		continue;
  	
  	// Receive
  	$remoteaddr = "";
    $resp = stream_socket_recvfrom($fp, 4096, 0, $remoteaddr);
  //echo "Got response from " . $remoteaddr . ": " . $resp . "<br>\n"; flush();
    $ping = Round(CurrentTime() - $sent_time);
    
    // Got response
  	if ($resp != false)
  		$response[] = Array($resp, $ping, $remoteaddr);
  }
  
  // Close the connection
  if (!$socket)
    fclose($fp);

  if ($response == Array())
  	return false;
  	
  return $response;
}