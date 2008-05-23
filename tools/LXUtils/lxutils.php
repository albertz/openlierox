<?php
// Liero Xtreme utilities for PHP
// Code released under the LGPL license
// Created on 19/01/2008 by Karel Petranek

// Requirements:
// PHP 4 or 5
// gd and zlib modules

// Includes and defines
require "HttpClient.class.php";
require "GIFEncoder.class.php";
require "binaryfunctions.php";

// Some defines
define("GSE_VERSION", 7);
define("MAP_VERSION", 0);
define("MPT_PIXMAP", 1);
define("MPT_IMAGE", 1);
define("PX_EMPTY", 1);
define("PX_DIRT", 2);
define("PX_ROCK", 4);
define("PALETTE_SIZE", 768);
define("WPN_PROJECTILE", 0);
define("WPN_SPECIAL", 1);
define("WPN_BEAM", 2);
define("PRJ_PIXEL", 0);
define("PRJ_IMAGE", 1);
define("PJ_EXPLODE", 1);
define("PJ_BOUNCE", 0);
define("PJ_CARVE", 4);
define("PJ_INJURE", 2);
define("TRL_PROJECTILE", 3);


////////////////////////////
// Map info structure
// $Width - map width
// $Height - map height
// $Name - map name
// $Theme - theme name, should always be "dirt"
// $MapImage - image of the map (not resized) 
// $MinimapImage - gd image containing resized map image
// Destroy - frees the images stored in the structure, call this when you
// don't use the structure anymore
class MapInfo  {
  var $Width;
  var $Height;
  var $Name;
  var $Theme;
  var $MapImage;
  var $MinimapImage;
  
  function Destroy()
  {
    imagedestroy($this->MapImage);
    imagedestroy($this->MinimapImage);
  }
}

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
// $LoadinTime - weapon reloading time
// $BonusesOn - bonuses on/off
// $Worms - array of WormInfo structure
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

/////////////////////////
// Mod info structure
// $Name - name of the mod
// $WeaponCount - number of weapons in the mod
// $ProjectileCount - number of projectiles in the mod
// $OverallDamage - sumed damage of all projectiles in the mod
// $Weapons - array with weapon names
class ModInfo  {
  var $Name;
  var $WeaponCount;
  var $ProjectileCount;
  var $OverallDamage;
  var $Weapons;
}

////////////////////////////
// Private function, returns current time
function CurrentTime()
{
  $t = gettimeofday();
  return ($t["sec"] * 1000) + ($t["usec"] / 1000);
}

///////////////////////////
// Gets the info about a LieroX Level
// Parameters: 
// $level: level filename
// $minimap_w: desired minimap width in result
// $minimap_h: desired minimap height in result
//
// Return value:
// false on error
// MapInfo structure when success
// NOTE: call Destroy() on the result when you don't need it anymore
function LXLevelInfo($level, $minimap_w = 128, $minimap_h = 96)
{
  // Check for original Liero level
  list(, $extension) = explode(".", $level);
  if (strcasecmp($extension, "lev") == 0)
    return LXOriginalLevelInfo($level, $minimap_w, $minimap_h);

  // Open the file
  $fp = fopen($level, "rb");
  if (!$fp)
    return false;
    
  // Header
  $id = ReadFixedCStr($fp, 32);
  $version = ReadInt32LE($fp);
  
  // Check the header
  if (($id != "LieroX Level" && id != "LieroX CTF Level") || $version != MAP_VERSION)
    return false;

    
  // Name
  $name = ReadFixedCStr($fp, 64);
  
  // Dimensions
  $width = ReadInt32LE($fp);
  $height = ReadInt32LE($fp);
  
  // Type
  $type = ReadInt32LE($fp);
    
  // Theme
  $theme = ReadFixedCStr($fp, 32);
  $num_objects = ReadInt32LE($fp);
  
  // Get the level image (depends on format)
  $level_image = false;
  switch ($type)  {
    case MPT_IMAGE:
      $level_image = LoadImageFormat($fp, $width, $height);
    break;
    case MPT_PIXMAP:
      $level_image = LoadPixmapFormat($fp, $width, $height, $theme, $num_objects);
    break;
  }
  
  // Loading the image preview failed
  if (!$level_image)
    return false;
  
  // Create the minimap
  $minimap_image = imagecreatetruecolor($minimap_w, $minimap_h);
  if (!$minimap_image)
    return false;
    
  // HINT: use imagecopyresampled for better results
  imagecopyresized($minimap_image, $level_image, 0, 0, 0, 0, $minimap_w,
                    $minimap_h, $width, $height);
  
  // Fill in the return info
  $returnValue = new MapInfo;
  $returnValue->Width = $width;
  $returnValue->Height = $height;
  $returnValue->Theme = $theme;
  $returnValue->Name = $name;
  $returnValue->MapImage = $level_image;
  $returnValue->MinimapImage = $minimap_image;
  
  return $returnValue;
}

///////////////////////////
// Private function, loads the image format LX level
// Parameters:
// $fp - file handle
// $width - level width
// $height - level height
// Return value:
// handle to map image if successful, false otherwise
function LoadImageFormat($fp, $width, $height)
{
  // Compressed data info
  $packed_size = ReadInt32LE($fp);
  $unpacked_size = ReadInt32LE($fp);
  
  // Read the compressed data
  $compressed = fread($fp, $packed_size);
  $uncompressed = gzuncompress($compressed);
  if (!$uncompressed) // Cannot uncompress
    return false;
  unset($compressed);  // Save some memory
  
  // Create the level image
  $level_image = imagecreatetruecolor($width, $height);
  if (!$level_image)
    return false;
  
  $bi = 0;  // Position in $uncompressed buffer, beginning of background image
  $fi = $width * $height * 3;  // Beginning of foreground image ($uncompressed)
  $mi = $fi + $width * $height * 3;  // Beginning of material ($uncompressed)
  for ($y = 0; $y < $height; $y++)  {
    for ($x = 0; $x < $width; $x++)  {
      // Empty pixels go from background, solid pixels from foreground image
      if (ord($uncompressed[$mi]) & PX_EMPTY)  {
        $color = imagecolorallocate($level_image, ord($uncompressed[$bi]),
                 ord($uncompressed[$bi+1]), ord($uncompressed[$bi+2]));
        imagesetpixel($level_image, $x, $y, $color);  // Put the pixel
      } else {
        $color = imagecolorallocate($level_image, ord($uncompressed[$fi]),
                 ord($uncompressed[$fi+1]), ord($uncompressed[$fi+2]));
        imagesetpixel($level_image, $x, $y, $color);  // Put the pixel    
      }
      $bi += 3;
      $fi += 3;
      $mi++;
    }
  }
  
  unset($uncompressed); // Save memory
  
  return $level_image;
}

///////////////////////////
// Private function, loads the pixmap format LX level
// Parameters:
// $fp - file handle
// $width - level width
// $height - level height
// $theme - theme name
// $objcount - object count
// Return value:
// handle to map image if successful, false otherwise
function LoadPixmapFormat($fp, $width, $height, $theme, $objcount)
{
  // HINT: this would be very hard because we need some files from
  // lierox directory, we would have to code placing stones and other
  // stuff... The result is not worth the energy because this format
  // is not used at all!
  return false;
}

//////////////////////////
// Gets info about the original DOS Liero level
// Half private - gets called automatically from LXLevelInfo
// Parameters:
// $level - level filename
// Return value:
// false when failed
// MapInfo structure on success
function LXOriginalLevelInfo($level, $minimap_w = 128, $minimap_h = 96)
{
  $powerlevel = false;
  
  // Validate the level
  $filesize = filesize($level);
  if ($filesize != 176400 && $filesize != 176402)  {
    if ($filesize == 177178)
      $powerlevel = true;
    else
      return false;
  }

  // Open
  $fp = fopen($level, "rb");
  if (!$fp)
    return false;
    
  // Liero levels don't contain a name, we just take the filename
  list($name) = explode(".", $level);
  $name = ucfirst($name); 
    
  // Liero levels have fixed dimensions and theme
  $width = 504;
  $height = 350;
  $theme = "dirt";
  
  // Load the palette (not a powerlevel file)
  $palette = "";
  if (!$powerlevel)  {
    $fpal = fopen("lieropal.act", "rb");
    if (!$fpal)  {
      fclose($fp);
      return false;
    }
    
    $palette = fread($fpal, PALETTE_SIZE);
    fclose($fpal);
    
    if (strlen($palette) != PALETTE_SIZE)
      return false;
  }
  
  // Load the image map
  $bytearr = fread($fp, $width * $height);
  
  // Load the palette from the file if powerlevel
  if ($powerlevel)  {
    $id = fread($fp, 10);
    if ($id != "POWERLEVEL")  {
      fclose($fp);
      return false;
    }
    
    // Load the palette
    $palette = fread($fp, PALETTE_SIZE);
    if (strlen($palette) != PALETTE_SIZE)  {
      fclose($fp);
      return false;
    }
      
    // Convert 6bit colors to 8bit colors
    for ($n = 0; $n < PALETTE_SIZE; $n++)  {
      $f = ord($palette[$n]) / 63.0 * 255.0;
      $palette[$n] = chr((int)$f);
    }
  }
  
  // Close the file (not needed anymore)
  fclose($fp);
  
  // Allocate the image
  $level_image = imagecreatetruecolor($width, $height);
  if (!$level_image)
    return false;
  
  // Get the image
  $n = 0;
  for ($y = 0; $y < $height; $y++)  {
    for ($x = 0; $x < $width; $x++)  {
      $p = ord($bytearr[$n]) * 3;
      $color = imagecolorallocate($level_image, ord($palette[$p]),
               ord($palette[$p + 1]), ord($palette[$p + 2]));
      imagesetpixel($level_image, $x, $y, $color);
      $n++;    
    }
  }
  
  // Generate the minimap
  $minimap_image = imagecreatetruecolor($minimap_w, $minimap_h);
  if (!$minimap_image)
    return false;
    
  // HINT: use imagecopyresampled for better results
  imagecopyresized($minimap_image, $level_image, 0, 0, 0, 0, $minimap_w,
                    $minimap_h, $width, $height);  
  
  // Fill in the info
  $returnValue = new MapInfo;
  $returnValue->Width = $width;
  $returnValue->Height = $height;
  $returnValue->Theme = $theme;
  $returnValue->Name = $name;
  $returnValue->MapImage = $level_image;
  $returnValue->MinimapImage = $minimap_image;
  
  return $returnValue;
}

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
  {
    return stream_socket_server("udp://0.0.0.0:" . strval($port), $errno, $errorstr, STREAM_SERVER_BIND);
  };
  $fp = false;
  $count = 50;
  while( ! $fp && $count > 0 )
  {
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
  while( CurrentTime() - $sent_time < $timeout && ( $maxreplies < 0 || count($response) < $maxreplies ) )
  {
  	// stream_set_timeout() does not work with stream_socket_recvfrom() - using select()
    //stream_set_timeout($fp, 0, ( $timeout + $sent_time - CurrentTime() ) * 1000);
	$r = Array($fp);
	$w = NULL;
	$e = NULL;
	if( stream_select($r, $w, $e, 0, 500000) == 0 )
		continue;
	$remoteaddr = "";
  	$resp = stream_socket_recvfrom($fp, 4096, 0, $remoteaddr);
//echo "Got responce from " . $remoteaddr . ": " . $resp . "<br>\n"; flush();
    $ping = Round(CurrentTime() - $sent_time);
	if( $resp != false )
		$response[] = Array( $resp, $ping, $remoteaddr );
  }
  
  // Close the connection
  if(!$socket)
    fclose($fp);

  if( $response == Array() )
  	return false;
  return $response;
}


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
  if (strlen($response))  {
    for ($i = 0; $i < $num_players; $i++)  {
      $worms[$i]->IP = BinToStr($response);
      $response = substr($response, strlen($worms[$i]->IP) + 1);
    }
  }
  
  $version = "Pre-Beta5";
  if (strlen($response))  {
    $version = BinToStr($response);
	$response = substr($response, strlen($version) + 1);
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
  $returnValue->NatType = "No NAT";
  $returnValue->Addr = $ip;
  $returnValue->Version = $version;
  
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
function LXGetUdpServerList( $masterservers, $timeout = 3000 )
{
	foreach ($masterservers as $ip)  
	{
		$packet = chr(0xFF) . chr(0xFF) . chr(0xFF) . chr(0xFF); // Header
		$packet .= "lx::getserverlist" . chr(0x00);
		
		// Send the packet
		$res = SendPacketAndWaitResponses($ip, $packet, $timeout);
		if (!$res)
		  return false;
		$ret = Array();
		  
		foreach ( $res as $response2 )
		{
			$response = $response2[0];
			// Check for connectionless header
			if (ord($response[0]) != 0xFF && 
			    ord($response[1]) != 0xFF &&
			    ord($response[2]) != 0xFF &&
			    ord($response[3]) != 0xFF)
			    continue;
			$response = substr($response, 4);
			if( strpos($response, "lx::serverlist".chr(0x00)) !== 0 )
				continue;
			$response = substr( $response, strlen("lx::serverlist".chr(0x00))+1 );
			while( strlen( $response ) > 0 )
			{
				$info = new FastServerInfo;
				$info->Ping = 999;
				$pos1 = strpos($response, chr(0x00));
				$info->Addr = substr( $response, 0, $pos1 );
				$pos1 += 1;
				$pos2 = strpos($response, chr(0x00), $pos1);
				$info->Name = substr( $response, $pos1, $pos2 - $pos1 );
				$pos2 += 1;
				$info->NumPlayers = ord($response[$pos2]);
				$pos2 += 1;
				$info->MaxPlayers = ord($response[$pos2]);
				$pos2 += 1;
				$info->State = ord($response[$pos2]);
				$pos2 += 1;
				$response = substr( $response, $pos2 );
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
	if( LXPingServer($ip) !== false )
		return LXServerInfo($ip);	// Server is NOT behind NAT
	$packet = chr(0xFF) . chr(0xFF) . chr(0xFF) . chr(0xFF); // Header
	$packet .= "lx::traverse" . chr(0x00);
	$packet .= $ip;
	$socket = OpenUdpSocket();
	if(!$socket)
		return false;
	$responses = SendPacketAndWaitResponses( $master, $packet, $timeout, -1, $socket );
	if( $responses == false )
		return false;
	$realAddr = $ip;
	$traverseAddr = "";
	$connechHereAddr = "";
	foreach( $responses as $resp )
	{
		if( strpos($resp[0], chr(0xFF).chr(0xFF).chr(0xFF).chr(0xFF)."lx::connect_here".chr(0x00) ) === 0 )
		{
			$connechHereAddr = $resp[2];
		}
		if( strpos($resp[0], chr(0xFF).chr(0xFF).chr(0xFF).chr(0xFF)."lx::traverse".chr(0x00) ) === 0 )
		{
			$traverseAddr = substr($resp[0], strlen(chr(0xFF).chr(0xFF).chr(0xFF).chr(0xFF)."lx::traverse".chr(0x00)));
			$traverseAddr = substr($traverseAddr, 0, strpos($traverseAddr, chr(0x00)) );
		}
	}
	if( $connechHereAddr != "" )
	{
		$realAddr = $connechHereAddr;
	}
	else if ( $traverseAddr != "" )
	{
		$realAddr = $traverseAddr;
	};
	$ret = LXServerInfo($realAddr, 2000, $socket);
    fclose($socket);
	if( $ret == false )
		return false;
	$ret->NatType = "Symmetric NAT";
	if( $traverseAddr == $connechHereAddr && $traverseAddr != "" )
		$ret->NatType = "Restricted NAT";
	$ret->Addr = $realAddr;
	return $ret;
}
    
//////////////////////
// Gets useful info about the mod
// Parameters:
// $path - path to mod, for example "lierox/Classic"
// Return value:
// false when failed
// ModInfo structure on success
function LXModInfo($path)
{ 
  // Check
  if (!$path)
    return false;
    
  // Add the trailing slash if not present
  if ($path[strlen($path) - 1] != '\\' &&
      $path[strlen($path) - 1] != '/')
    $path .= "/";
    
  // Add the script.lgs
  $path .= "script.lgs";
  
  // Open the mod file
  $fp = fopen($path, "rb");
  if (!$fp)
    return false;
    
  // File ID
  $id = ReadFixedCStr($fp, 20);
  if ($id != "Liero Game Script")
    return false;
    
  // Version
  $version = ReadInt32LE($fp);
  if ($version != GSE_VERSION)
    return false;
    
  // Mod name
  $name = ReadFixedCStr($fp, 64);
  
  // Number of weapons
  $num_weapons = ReadInt32LE($fp);
  
  // Weapon names & projectile count
  $num_projectiles = 0;
  $overall_damage = 0;
  $weapons = Array();
  for ($i = 0; $i < $num_weapons; $i++)  {

    // Read the name
    $weapons[$i] = ReadVariablePascalStr($fp);
    
    // Read the type
    $type = ReadInt32LE($fp);
    
    // Skip other info
    switch ($type)  {
      case WPN_SPECIAL:
        Skip($fp, 24);
      break;
      
      case WPN_BEAM:
        Skip($fp, 32);
        $overall_damage += ReadInt32LE($fp);
        Skip($fp, 8);
      break;
      
      case WPN_PROJECTILE:
        Skip($fp, 40);
        if (ReadInt32LE($fp)) // Uses sound?
          ReadVariablePascalStr($fp); // Skip the sound name
        SkipProjectile($fp, $num_projectiles, $overall_damage);
      break;       
    }
  }
  
  // Fill in the info
  $returnValue = new ModInfo;
  $returnValue->Name = $name;
  $returnValue->WeaponCount = $num_weapons;
  $returnValue->ProjectileCount = $num_projectiles;
  $returnValue->OverallDamage = $overall_damage;
  $returnValue->Weapons = $weapons;
  
  return $returnValue;
}

//////////////
// Private function, skips the projectile in the game mod file
// Parameters:
// $fp - handle to the file
// $projectile_count - projectile counting variable, will be changed
// Return value:
// none
function SkipProjectile($fp, &$projectile_count, &$overall_damage)
{
  $projectile_count++;
  $my_damage = 0;  // Damage that can be caused by this projectile

  // Type and timeout
  $type = ReadInt32LE($fp);
  $timer_time = ReadFloatLE($fp);
  
  Skip($fp, 4); // Timer time var
  $prj_trail = ReadInt32LE($fp);  // Projectile trail type
  if (ReadInt32LE($fp)) { // Uses gravity?
    ReadInt32LE($fp); // Skip the gravity
  }
  ReadInt32LE($fp); // Skip the dampening
  
  if ($type == PRJ_IMAGE)  {
    ReadVariablePascalStr($fp); // Skip the image name
    Skip($fp, 12);
    $use_angle = ReadInt32LE($fp);
    $use_spec_angle = ReadInt32LE($fp);
    if ($use_angle || $use_spec_angle) {  // Use (special) angle?
      ReadInt32LE($fp);  // Skip "angle images"
    }
    if (ReadInt32LE($fp))  {  // Animating?
      Skip($fp, 8);
    }
  } else {  // PRJ_PIXEL
    $num_colors = ReadInt32LE($fp);
    Skip($fp, 12);
    if ($num_colors == 2)
      Skip($fp, 12);
  }
  
  // Ground hit
  $hit_type = ReadInt32LE($fp);
  $hit_projectiles = 0;
  switch ($hit_type)  {
    case PJ_EXPLODE:
      $my_damage = max($my_damage, ReadInt32LE($fp)); // Damage
      $hit_projectiles = ReadInt32LE($fp);  // Spawn any child projectiles?
      $use_sound = ReadInt32LE($fp); // Uses hit sound?
      Skip($fp, 4); // Shake
      if ($use_sound)  {
        ReadVariablePascalStr($fp); // Skip the sound filename
      }
    break;
    
    case PJ_BOUNCE:
      Skip($fp, 8);
    break;
    
    case PJ_CARVE:
      Skip($fp, 4);
    break;
  }

  // Timer
  $tmr_projectiles = 0;  // Spawn projectiles when timer is up?
  if ($timer_time != 0)  {
    if (ReadInt32LE($fp) == PJ_EXPLODE) {  // Timer type
      $my_damage = max($my_damage, ReadInt32LE($fp)); // Damage  
      $tmr_projectiles = ReadInt32LE($fp);
      Skip($fp, 4);  // Shake
    }
  }
  
  // Player hit
  $ply_type = ReadInt32LE($fp);
  $ply_projectiles = 0; // Spawn projectiles when player gets hit?
  if ($ply_type == PJ_INJURE || $ply_type == PJ_EXPLODE)  {
    $my_damage = max($my_damage, ReadInt32LE($fp)); // Damage
      
    $ply_projectiles = ReadInt32LE($fp);
  } else if ($ply_type == PJ_BOUNCE)
    Skip($fp, 4); // Bounce coeff
   
  // Explosion event
  Skip($fp, 4); // Explode type
  $my_damage = max($my_damage, ReadInt32LE($fp)); // Damage
  $exp_projectiles = ReadInt32LE($fp); // Spawn projectiles on explosion?
  if (ReadInt32LE($fp))  { // Use explosion sound?
    ReadVariablePascalStr($fp); // Skip the sound filename
  }
    
  // Touch event
  Skip($fp, 4);  // Touch type
  $my_damage = max($my_damage, ReadInt32LE($fp)); // Damage
  $tch_projectiles = ReadInt32LE($fp);
  if (ReadInt32LE($fp))  {  // Use touch sound?
    ReadVariablePascalStr($fp);  // Skip the sound filename
  }
  
  // Add my damage to overall damage
  $overall_damage += $my_damage;
    
  // Skip any child projectiles
  if ($tmr_projectiles || $ply_projectiles || $exp_projectiles ||
     $tch_projectiles || $hit_projectiles)  {
    Skip($fp, 24);
    SkipProjectile($fp, $projectile_count, $overall_damage);   
  }
  
  // Skip trail projectiles
  if ($prj_trail == TRL_PROJECTILE)  {
    Skip($fp, 24);
    SkipProjectile($fp, $projectile_count, $overall_damage);
  }
}

//////////////////////////////
// Private function, loads an image (works for both local and remote)
// Parameters:
// $filename - path or URL
// Return value:
// false on failure
// GD image handle (always truecolor image) on success
function LoadImage($filename)
{
  // Get the extension
  $tmp = explode(".", $filename);
  $extension = strtolower($tmp[count($tmp)-1]);

  // If the file is remote, first download it
  $file_contents = "";
  if (strtolower(substr($filename, 0, 5)) == "http:") {
    // Get the file
    $file_contents = HttpClient::quickGet($lxskin);
    if (!$file_contents)
      return false;
      
    $extension = "downloaded";
  }
  
  // Load the image
  $loaded_image = 0;
  switch ($extension)  {
    case "png":
      $loaded_image = imagecreatefrompng($filename);
    break;
    
    case "jpg":
    case "jpeg":
      $loaded_image = imagecreatefromjpeg($filename);
    break;
    
    case "gif":
      $loaded_image = imagecreatefromgif($filename);
    break;
    
    case "downloaded":
      $loaded_image = imagecreatefromstring($file_contents);
    break; 
  }
  
  // Check if successfully loaded
  if (!$loaded_image)
    return false;
    
  // Save memory
  unset($file_contents);
    
  // Convert to true color image
  $result_image = false;
  if (!imageistruecolor($loaded_image))  {
    $result_image = imagecreatetruecolor(imagesx($loaded_image),
                    imagesy($loaded_image));
    if (!$result_image)  {
      imagedestroy($loaded_image);
      return false;
    }
    imagecopy($result_image, $loaded_image, 0, 0, 0, 0, imagesx($loaded_image),
             imagesy($loaded_image));
             
    // Cleanup
    imagedestroy($loaded_image);
             
  // Already truecolor
  } else {
    $result_image = $loaded_image;
  }
  
  return $result_image;
}

/////////////////////////////
// Converts LieroX skin to an animated gif
// Parameters:
// $lxskin - path to the LieroX skin
// $color - color of the skin, Array(R, G, B) or -1 for default skin color
// Return value:
// gif file contents (NOT image handle) when successful, false otherwise
function LXSkinToAnimGIF($lxskin, $color = -1)
{
  // Simple animation
  $anim_def = Array();
  for ($i = 0; $i < 21; $i++) { // 21 is the common count of frames
    $anim_def[$i]["frame"] = $i;
    $anim_def[$i]["flipped"] = false;
    $anim_def[$i]["color"] = $color;
  }
    
  return LXSkinToAnimGIFAdv($lxskin, $anim_def, 20, 0);
}

/////////////////////////////
// Converts LieroX skin to an animated gif (advanced)
// Parameters:
// $lxskin - path to the LieroX skin
// $anim_def - array which specifies info for each frame
// $frame_delay - delay between two frames (use 1 if unsure)
// $repeats - number of repeats (0 = infinite)
// Return value:
// gif file contents (NOT image handle) when successful, false otherwise
function LXSkinToAnimGIFAdv($lxskin, $anim_def, $frame_delay, $repeats)
{
  // Load the image
  $skin_image = LoadImage($lxskin);
  if (!$skin_image)
    return false;
    
  // Verify the skin
  if (imagesy($skin_image) != 36) {
    imagedestroy($skin_image);
    return false;    
  }
   
  // Render the frames
  $frames = Array();
  $frame_delays = Array();
  $num_frames = floor(imagesx($skin_image) / 32);
  
  for ($i = 0; $i < count($anim_def);) {
    // Get the frame info
    $frame_nr = max(min($anim_def[$i]["frame"], $num_frames), 1) - 1;
    $frame_fl = $anim_def[$i]["flipped"];
    $frame_cl = $anim_def[$i]["color"];
    
    // Render the frame
    $frame_image = GetSkinFrame($skin_image, $frame_nr, $frame_fl, $frame_cl);
    
    // Get the gif data
    if ($frame_image) {
      // Get the gif data
      ob_start();
      imagegif($frame_image);
      $frames[$i] = ob_get_contents();
      ob_end_clean();
      $frame_delays[$i] = $frame_delay;
      
      // Cleanup
      imagedestroy($frame_image);
      
      $i++;
    } 
  }
  
  // Cleanup
  imagedestroy($skin_image);
  
  // Create the gif
  $gif_output = new GIFEncoder($frames, $frame_delays, $repeats,
                2, 255, 0, 255, "bin");
                
  $returnValue = $gif_output->GetAnimation();
  
  return $returnValue ? $returnValue : false;
}

//////////////////////
// Private function, extracts one frame from the skin image
// Parameters:
// $img - true color GD image handle
// $frame - frame number
// $flipped - true if the image should be horizontally flipped
// $color - skin color, -1 for default)
// Return value:
// false on failure
// image handle on success
function GetSkinFrame($img, $frame, $flipped, $color)
{
  // Allocate the image
  $result = imagecreatetruecolor(32, 18);
  if (!$result)
    return false;
    
  // Allocate the pink (transparent) color first
  $transparent = imagecolorallocate($result, 255, 0, 255);
    
  // Set the transparent color
  imagecolortransparent($result, $transparent);
    
  // Get the X-coordinate of the frame
  $frame_start = $frame * 32;
  
  // Render the frame  
  for ($y = 0; $y < 18; $y++)  {
    for ($x = 0; $x < 32; $x++) {
      $pixel = imagecolorat($img, $x + $frame_start, $y);
      
      // Get the RGB from pixel
      $r = ($pixel >> 16) & 0xFF;
      $g = ($pixel >> 8) & 0xFF;
      $b = $pixel & 0xFF;
     
      // If no colorizing, just copy the pixel
      if ($color == -1)  {
        imagesetpixel($result, $flipped ? 31 - $x : $x, $y, $pixel);  
        
      // Colorize before copying
      } else {
        // Get the mask
        $mask = imagecolorat($img, $x + $frame_start, $y + 18);
        
        // Get the RGB from mask
        $mr = ($mask >> 16) & 0xFF;
        $mg = ($mask >> 8) & 0xFF;
        $mb = $mask & 0xFF;        
        
        // Define colorized pixels
        $cr = $cg = $cb = 0;
        
        // If the mask is pink or black, don't colorize
        if (($mr == 255 && $mg == 0 && $mb == 255) ||
            ($mr == 0 && $mg == 0 && $mb == 0)) {
          $cr = $r;
          $cg = $g;
          $cb = $b;
          
        // Colorize the pixel    
        } else {
          $cr = min(($r / 96) * $color[0], 255);
          $cg = min(($g / 156) * $color[1], 255);
          $cb = min(($b / 252) * $color[2], 255);
          
          // Make sure the pixel is not the magic pink
          if ($cr == 255 && $cg == 0 && $cb == 255) {
            $cr = 240;
            $cb = 240;
          }
        }
        
        // Put the colorized pixel
        $colorized = imagecolorallocate($result, $cr, $cg, $cb);
        imagesetpixel($result, $flipped ? 31 - $x : $x, $y, $colorized);
      }     
    }
  }
  
  return $result; 
}

/////////////////////////////
// Gets one frame from the skin
// Parameters:
// $lxskin - path or URL to the skin
// $frame - frame number, counted from 1
// $flipped - true to flip the image horizontally
// $color - color (Array(r, g, b)) or -1 for default color
// Return value:
// false on failure
// GD image handle on success
function LXSkinGetFrame($lxskin, $frame, $flipped = false, $color = -1)
{
  // Load the skin
  $skin_image = LoadImage($lxskin);
  if (!$skin_image)
    return false;
    
  // Verify the skin
  if (imagesy($skin_image) != 36) {
    imagedestroy($skin_image);
    return false;
  }
    
  // Verify the frame
  $num_frames = floor(imagesx($skin_image) / 32);
  $frame = max(min($frame, $num_frames), 1) - 1; // Convert to 0-based
  
  // Get the frame
  return GetSkinFrame($skin_image, $frame, $flipped, $color);
}
?>