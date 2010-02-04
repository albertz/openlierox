#!/usr/bin/python

# Liero Xtreme utilities for Python
# Code released under the LGPL license
# Created on 18/02/2009 by Karel Petranek
# Ported to Python on 04/02/2010 by Albert Zeyer

# Requirements:
# gd (for the image processing functions) and zlib modules

# This file contains functions for working with (Open)LieroX levels (*.lxl)

// Includes
require_once "binaryfunctions.php";

// Defines
define("LXL_IMAGE", 1);     // Level type (image)
define("LXL_PIXMAP", 0);    // Level type (pixmap) - not supported at the moment, very rare
define("LXL_ORIGINAL", 2);  // Level type (original Liero level)

define("MAP_VERSION", 0);             // Version of the map format
define("PX_EMPTY", 1);                // Empty pixel (air)
define("PX_DIRT", 2);                 // Dirt pixel
define("PX_ROCK", 4);                 // Rock pixel (solid)
define("ORIGINAL_SIZE1", 176400);                 // Original Liero level filesize
define("ORIGINAL_SIZE2", 176402);                 // Original Liero level filesize
define("ORIGINAL_POWERLEVEL_SIZE", 177178);       // Original Liero powerlevel filesize
define("PALETTE_SIZE", 768);                      // Size of the color palette (in bytes), for original levels

if (!defined("LXUTILS_PATH"))
  define("LXUTILS_PATH", "./");

// Structures

class MapInfo:
	Width = 0
	Height = 0
	Name = ""
	Theme = "" # always "dirt"
	ObjectCount = 0
	MapImage = None
	MinimapImage = None
	Type = None # LXL_IMAGE, LXL_PIXMAP or LXL_ORIGINAL
  
	def Destroy:

		if ($this->MapImage) 
      imagedestroy($this->MapImage);
    if ($this->MinimapImage)
      imagedestroy($this->MinimapImage);
  }
}

// Public functions

///////////////////////////
// Gets a basic info about a LieroX Level
// Parameters: 
// $level: level filename
//
// Return value:
// false on error
// MapInfo structure on success, images are set to 0
// NOTE: call Destroy() on the result when you don't need it anymore
function LXLevelInfo($level)
{
  // Check for original Liero level
  list(, $extension) = explode(".", $level);
  if (strcasecmp($extension, "lev") == 0)
    return LXOriginalLevelInfo($level);
    
  // Open the file
  $fp = fopen($level, "rb");
  if (!$fp)
    return false;
   
  // If an unknown extension is given, try to autodetect the format
  if (strcasecmp($extension, "lxl") != 0)  {
    $returnValue = ReadBasicLXLevelInfo($fp);
    fclose($fp);
    if ($returnValue === false)  {
      return LXOriginalLevelInfo($level);
    }
    
  // LXL level
  } else {
    
    // Read basic information
    $returnValue = ReadBasicLXLevelInfo($fp);
    fclose($fp);
    if ($returnValue === false)
      return false;
  }
    
  return $returnValue;
}

///////////////////////////
// Gets the info about a LieroX Level
// Parameters: 
// $level: level filename
// $minimap_w: desired minimap width in result
// $minimap_h: desired minimap height in result
// $hq: set to true if you want to have a high-quality minimap (uses resampling instead of resizing)
//
// Return value:
// false on error
// MapInfo structure on success
// NOTE: call Destroy() on the result when you don't need it anymore
function LXLevelInfoAdv($level, $minimap_w = 128, $minimap_h = 96, $hq = false)
{
  // Get the extension
  $tmp = explode(".", $level);
  $extension = "";
  if (count($tmp) > 1)
    $extension = $tmp[1];
    
  // Check for original Liero level
  if (strcasecmp($extension, "lev") == 0)
    return LXOriginalLevelInfoEnhanced($level, $minimap_w, $minimap_h, $hq);
    
  // Open the file
  $fp = fopen($level, "rb");
  if (!$fp)
    return false;
    
  // Unknown extension, try to autodetect the format
  $returnValue = false;
  if (strcasecmp($extension, "lxl") != 0) {
    $returnValue = ReadBasicLXLevelInfo($fp);  // Try LXL
    if ($returnValue === false)  {
      fclose($fp);
      return LXOriginalLevelInfoEnhanced($level, $minimap_w, $minimap_h, $hq); // Try LEV
    }
    
  // .lxl extension
  } else {
    // Read basic information
    $returnValue = ReadBasicLXLevelInfo($fp);
    if ($returnValue === false)  {
      fclose($fp);
      return false;
    }
  }
  
  // Get the level image (depends on format)
  $level_image = false;
  switch ($returnValue->Type)  {
    case LXL_IMAGE:
      $level_image = LoadImageFormat($fp, $returnValue->Width, $returnValue->Height);
    break;
    case LXL_PIXMAP:
      $level_image = LoadPixmapFormat($fp, $returnValue->Width, $returnValue->Height, $returnValue->Theme, $returnValue->ObjectCount);
    break;
  }
  
  // Not needed anymore
  fclose($fp);
  
  // Loading the image preview failed
  if (!$level_image)
    return false;
  
  // Create the minimap
  $minimap_image = imagecreatetruecolor($minimap_w, $minimap_h);
  if (!$minimap_image)
    return false;
    
  // HINT: use imagecopyresampled for better results
  if ($hq)
    imagecopyresampled($minimap_image, $level_image, 0, 0, 0, 0, $minimap_w,
                    $minimap_h, $returnValue->Width, $returnValue->Height);  
  else
    imagecopyresized($minimap_image, $level_image, 0, 0, 0, 0, $minimap_w,
                    $minimap_h, $returnValue->Width, $returnValue->Height);
  
  // Fill in the return info
  $returnValue->MapImage = $level_image;
  $returnValue->MinimapImage = $minimap_image;
  
  return $returnValue;
}

// Private functions

/////////////////////
// Reads basic information about the level
// Parameters:
// $fp: opened level file
// 
// Return value:
// Returns the MapInfo structure (images are set to 0) on success,
// false otherwise
function ReadBasicLXLevelInfo($fp)
{
  // Header
  $id = ReadFixedCStr($fp, 32);
  $version = ReadInt32LE($fp);
  
  // Check the header
  if (($id != "LieroX Level" && $id != "LieroX CTF Level") || $version != MAP_VERSION)
    return false;

    
  // Name
  $name = ReadFixedCStr($fp, 64);
  
  // Dimensions
  $width = ReadInt32LE($fp);
  $height = ReadInt32LE($fp);
  
  // Type
  $type = ReadInt32LE($fp);
  if ($type != LXL_PIXMAP && $type != LXL_IMAGE)
    return false;
    
  // Theme
  $theme = ReadFixedCStr($fp, 32);
  $num_objects = ReadInt32LE($fp);
  
  // Fill in the return info
  $returnValue = new MapInfo;
  $returnValue->Width = $width;
  $returnValue->Height = $height;
  $returnValue->Theme = $theme;
  $returnValue->Name = $name;
  $returnValue->MapImage = 0;
  $returnValue->MinimapImage = 0;
  $returnValue->Type = $type;

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
// Gets basic info about the original DOS Liero level
// Parameters:
// $level - level filename
// Return value:
// false when failed
// MapInfo structure on success, images are set to zero
function LXOriginalLevelInfo($level)
{ 
  // Validate the level
  $filesize = filesize($level);
  if ($filesize != ORIGINAL_SIZE1 && $filesize != ORIGINAL_SIZE2 && $filesize != ORIGINAL_POWERLEVEL_SIZE)
      return false;
    
  // Basic info
  return ReadBasicOriginalLevelInfo($level);
}

//////////////////////////
// Gets info about the original DOS Liero level
// Half private - gets called automatically from LXLevelInfo
// Parameters:
// $level - level filename
// Return value:
// false when failed
// MapInfo structure on success
function LXOriginalLevelInfoEnhanced($level, $minimap_w = 128, $minimap_h = 96, $hq = false)
{
  $powerlevel = false;
  
  // Validate the level
  $filesize = filesize($level);
  if ($filesize != ORIGINAL_SIZE1 && $filesize != ORIGINAL_SIZE2)  {
    if ($filesize == ORIGINAL_POWERLEVEL_SIZE)
      $powerlevel = true;
    else
      return false;
  }

  // Open
  $fp = fopen($level, "rb");
  if (!$fp)
    return false;
    
  // Basic info
  $returnValue = ReadBasicOriginalLevelInfo($level);
  if ($returnValue === false)
    return false;

  // Load the palette (not a powerlevel file)
  $palette = "";
  if (!$powerlevel)  {
    // Open the palette from LXUTILS directory
    $dir = LXUTILS_PATH;
    if ($dir[strlen($dir) - 1] != '/' && $dir[strlen($dir) - 1] != '\\')
      $dir .= '/';
    $fpal = fopen($dir . "lieropal.act", "rb");
    
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
  $bytearr = fread($fp, $returnValue->Width * $returnValue->Height);
  
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
  $level_image = imagecreatetruecolor($returnValue->Width, $returnValue->Height);
  if (!$level_image)
    return false;
  
  // Get the image
  $n = 0;
  for ($y = 0; $y < $returnValue->Height; $y++)  {
    for ($x = 0; $x < $returnValue->Width; $x++)  {
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
    
  // Resize the map image
  if ($hq)
    imagecopyresampled($minimap_image, $level_image, 0, 0, 0, 0, $minimap_w,
                    $minimap_h, $returnValue->Width, $returnValue->Height);    
  else
    imagecopyresized($minimap_image, $level_image, 0, 0, 0, 0, $minimap_w,
                    $minimap_h, $returnValue->Width, $returnValue->Height);  
  
  // Fill in the info
  $returnValue->MapImage = $level_image;
  $returnValue->MinimapImage = $minimap_image;
  
  return $returnValue;
}

//////////////////////////
// Gets basic info about the original DOS Liero level
// Parameters:
// $level - file name (of a validated level)
//
// Return value:
// false when failed
// MapInfo structure on success, images are set to 0
function ReadBasicOriginalLevelInfo($level)
{
  // Liero levels don't contain a name, we just take the filename
  $parts = explode("/", $level);
  if (count($parts) > 1)  {
    $parts = explode(".", $parts[count($parts) - 1]);
    $name = $parts[0];
    $name = ucfirst($name); 
  } else {
    $name = $level;
  }
    
  // Liero levels have fixed dimensions and theme
  $returnValue = new MapInfo;
  $returnValue->Width = 504;
  $returnValue->Height = 350;
  $returnValue->Theme = "dirt";
  $returnValue->ObjectCount = 0;
  $returnValue->Name = $name;
  $returnValue->MapImage = 0;
  $returnValue->MinimapImage = 0;
  $returnValue->Type = LXL_ORIGINAL;
    
  return $returnValue;
}

?>
