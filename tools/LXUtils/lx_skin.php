<?php
// Liero Xtreme utilities for PHP
// Code released under the LGPL license
// Created on 18/02/2009 by Karel Petranek

// Requirements:
// PHP 4 or 5
// gd (for the image processing functions) and zlib modules

// This file contains functions for working with (Open)LieroX skins

// Includes
require_once "GIFEncoder.class.php";
require_once "HttpClient.class.php";

// Defines
define("SKIN_HEIGHT", 36);  // Height of the skin image in pixels
define("FRAME_COUNT", 21);  // Number of frames in the skin
define("FRAME_WIDTH", 32);  // Width of one frame, in pixels

// Public functions

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
  for ($i = 0; $i < FRAME_COUNT; $i++) { // 21 is the common count of frames
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
  if (imagesy($skin_image) < SKIN_HEIGHT) {
    imagedestroy($skin_image);
    return false;    
  }
   
  // Render the frames
  $frames = Array();
  $frame_delays = Array();
  $num_frames = floor(imagesx($skin_image) / FRAME_WIDTH);
  
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
  if (imagesy($skin_image) < SKIN_HEIGHT) {
    imagedestroy($skin_image);
    return false;
  }
    
  // Verify the frame
  $num_frames = floor(imagesx($skin_image) / FRAME_WIDTH);
  $frame = max(min($frame, $num_frames), 1) - 1; // Convert to 0-based
  
  // Get the frame
  return GetSkinFrame($skin_image, $frame, $flipped, $color);
}



// Private functions

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
  $result = imagecreatetruecolor(FRAME_WIDTH, SKIN_HEIGHT / 2);
  if (!$result)
    return false;
    
  // Allocate the pink (transparent) color first
  $transparent = imagecolorallocatealpha($result, 255, 255, 255, 0);
    
  // Set the transparent color
  imagecolortransparent($result, $transparent);
    
  // Get the X-coordinate of the frame
  $frame_start = $frame * FRAME_WIDTH;
  
  // Render the frame  
  for ($y = 0; $y < SKIN_HEIGHT / 2; $y++)  {
    for ($x = 0; $x < FRAME_WIDTH; $x++) {
      $pixel = imagecolorat($img, $x + $frame_start, $y);
      
      // Get the RGB from pixel
      $r = ($pixel >> 16) & 0xFF;
      $g = ($pixel >> 8) & 0xFF;
      $b = $pixel & 0xFF;
     
       
      // Colorize before copying
      {
        // Get the mask
        $mask = imagecolorat($img, $x + $frame_start, $y + SKIN_HEIGHT / 2);
        
        // Get the RGB from mask
        $mr = ($mask >> 16) & 0xFF;
        $mg = ($mask >> 8) & 0xFF;
        $mb = $mask & 0xFF;        
        
        // Define colorized pixels
        $cr = $cg = $cb = 0;
        
        // If the mask is pink or black, don't colorize
        if (($mr == 255 && $mg == 0 && $mb == 255) ||
            ($mr == 0 && $mg == 0 && $mb == 0)) {
          
          imagesetpixel($result, $flipped ? FRAME_WIDTH - 1 - $x : $x, $y, $transparent);
          
        // Colorize the pixel    
        } else {
          if ($color == -1)  {
            imagesetpixel($result, $flipped ? FRAME_WIDTH - 1 - $x : $x, $y, $pixel);
            continue;
          } else {
            $cr = min(($r / 96) * $color[0], 255);
            $cg = min(($g / 156) * $color[1], 255);
            $cb = min(($b / 252) * $color[2], 255);
          } 
          
          // Make sure the pixel is not the magic pink
          if ($cr == 255 && $cg == 0 && $cb == 255) {
            $cr = 240;
            $cb = 240;
          }
          
          // Put the colorized pixel
          $colorized = imagecolorallocate($result, $cr, $cg, $cb);
          imagesetpixel($result, $flipped ? FRAME_WIDTH - 1 - $x : $x, $y, $colorized);          
        }
      }     
    }
  }
  
  return $result; 
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
    
    default:
      $loaded_image = imagecreatefrompng($filename);
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

?>