<?php
// Liero Xtreme utilities for PHP
// Code released under the LGPL license
// Created on 27/04/2009 by Karel Petranek

// Requirements:
// PHP 4 or 5
// gd (for the image processing functions) and zib modules

// This file contains functions for working with (Open)LieroX frontends

// Defines (mostly taken from OpenLieroX)
define("DEFAULT_WIDTH", 640);
define("DEFAULT_HEIGHT", 480);
define("MAINTITLES_COUNT", 5);
define("BOX_LEFT", 15);
define("BOX_TOP", 130);
define("BOX_WIDTH", 610);
define("BOX_HEIGHT", 335);
define("CREDITS_LINE_HEIGHT", 13);
define("CREDITS_FONT", "lxfont.ttf");
define("CREDITS_FONT_SIZE", 9);
define("LOGO_Y", 10);
define("BUT_QUIT", 12);
define("QUIT_X", 25);
define("QUIT_Y", 440);


// LXUTILS_PATH
$LXUTILS_PATH = "./";
if (defined("LXUTILS_PATH"))  {
  // Make sure there's a slash at the end
  $path = LXUTILS_PATH;
  if ($path[strlen($path) - 1] != '/' && $path[strlen($path) - 1] != '\\')
    $path .= '/';

  $LXUTILS_PATH = $path;
}

// Includes
require_once $LXUTILS_PATH . "lx_configreader.php";

// Structures

////////////////////////////
// FrontendConfig info structure (private)
// $BoxLightColor - highlight color of the box stripes
// $BoxDarkColor - dark color of the box stripes
// $BoxVisible - true if the box is visible
// $Credits1Color - color of credits heading
// $Credits2Color - color of credits
// $MainTitlesPos - main titles position
// $MainTitlesSpacing - main titles spacing
// $CreditsPos - credits position
// $CreditsSpacing - credits spacing
// $CreatorText - creator credits
class FrontendConfig  {
  var $BoxLightColor;
  var $BoxDarkColor;
  var $BoxVisible;
  var $Credits1Color;
  var $Credits2Color;
  var $MainTitlesPos;
  var $MainTitlesSpacing;
  var $CreditsPos;
  var $CreditsSpacing;
  var $CreatorText;
}

///////////////////
// Creates a preview of the frontend in the zip file
// Parameters:
// $zipFile - the zip file to be analysed
// $width - width of the preview
// $height - height of the preview
// $hq - high quality (will use imagecopyresampled if set)
// 
// Return value:
// GD image representing the preview
function LXCreateFrontendPreview($zipFile, $width = DEFAULT_WIDTH, $height = DEFAULT_HEIGHT, $hq = false)
{
  // Open the ZIP file
  $zip = new ZipArchive;
  if ($zip->open($zipFile) !== true)
    return false;
    
  // Find all necessary files
  $files = Array();
  $fl = ZIPARCHIVE::FL_NOCASE | ZIPARCHIVE::FL_NODIR;
  $files["frontend.cfg"] = $zip->locateName("frontend.cfg", $fl);
  $files["colours.cfg"] = $zip->locateName("colours.cfg", $fl);
  $files["background_wob.png"] = $zip->locateName("background_wob.png",$fl);
  $files["maintitles.png"] = $zip->locateName("maintitles.png", $fl);
  $files["lierox.png"] = $zip->locateName("lierox.png", $fl);
  $files["buttons.png"] = $zip->locateName("buttons.png", $fl);
  
  // Check for mandatory files
  if ($files["background_wob.png"] === false || $files["maintitles.png"] === false || $files["lierox.png"] === false || $files["buttons.png"] === false)
    return false;
  
  // Get additional data
  if (!isset($files["frontend.cfg"]))
    $files["frontend.cfg"] = 0;
  if (!isset($files["colours.cfg"]))
    $files["colours.cfg"] = 0;
  $config = LXAnalyzeFrontendCfg($zip, $files["frontend.cfg"], $files["colours.cfg"]);
  
  // Load images
  $background = LXLoadZippedImage($zip, $files["background_wob.png"]);
  if (!$background)
    return false;
  $maintitles = LXLoadZippedImage($zip, $files["maintitles.png"]);
  if (!$maintitles)
    return false;
  $logo = LXLoadZippedImage($zip, $files["lierox.png"]);
  if (!$logo)
    return false;
  $buttons = LXLoadZippedImage($zip, $files["buttons.png"]);
  if (!$buttons)
    return false;
    
  // Close the ZIP
  $zip->close();
    
  // Check sizes
  if (imagesx($background) < DEFAULT_WIDTH || imagesy($background) < DEFAULT_HEIGHT)
    return false;
  
  // Allocate result
  $result = imagecreatetruecolor(DEFAULT_WIDTH, DEFAULT_HEIGHT);
  if (!$result)  {
    imagedestroy($background);
    imagedestroy($maintitles);
    return false;
  }
  
  // Draw everything
  LXDrawBackground($result, $background);
  imagedestroy($background);
  LXDrawMainTitles($result, $maintitles, $config);
  imagedestroy($maintitles);
  LXDrawLogo($result, $logo);
  imagedestroy($logo);
  LXDrawQuitButton($result, $buttons);
  imagedestroy($buttons);
  LXDrawBox($result, $config);
  LXDrawCredits($result, $config);
  
  // Resize if needed
  if ($width == DEFAULT_WIDTH && $height == DEFAULT_HEIGHT)
    return $result;
  
  // If hq is set, use bicubic resampling
  $resized = imagecreatetruecolor($width, $height);
  if ($hq)
    imagecopyresampled($resized, $result, 0, 0, 0, 0, $width, $height, imagesx($result), imagesy($result));
  else
    imagecopyresized($resized, $result, 0, 0, 0, 0, $width, $height, imagesx($result), imagesy($result));
    
  imagedestroy($result);
    
  return $resized;
}

/////////////////
// Private function, loads zipped image
//
// Parameters:
// $zip_entry - opened ZIP file handle
//
// Return value:
// GD image, false on error
function LXLoadZippedImage($zip, $zip_entry)
{
  $buf = $zip->getFromIndex($zip_entry);
  if (!$buf)
    return false;
    
  return imagecreatefromstring($buf);
}

///////////////////
// Private function, draws a background on the resulting frontend preview
//
// Parameters:
// $result - the resulting preview
// $background - the background image
function LXDrawBackground($result, $background)
{
  imagecopy($result, $background, 0, 0, 0, 0, imagesx($background), imagesy($background));
}

///////////////////
// Private function, draws main titles
//
// Parameters:
// $result - the result image
// $maintitles - the main titles image
// $config - the frontend configuration
function LXDrawMainTitles($result, $maintitles, $config)
{
  $titleheight = (imagesy($maintitles) - 10) / (MAINTITLES_COUNT * 2) - 5; // 5px spacing, 10px on top
  
  // Draw all the titles
  $y = $config->MainTitlesPos[1];
  $x = $config->MainTitlesPos[0];
  $w = imagesx($maintitles);
  for ($i = 0; $i < MAINTITLES_COUNT; $i++)  {
    $dy = $i * ($titleheight + $config->MainTitlesSpacing);
    imagecopy($result, $maintitles, $x, $y + $dy, 0, 10 + $i * 40, $w, 39);
  }

}

//////////////////
// Private function, draws the box stripe
//
// Parameters:
// $result - the resulting image
// $config - the frontend configuration
function LXDrawBox($result, $config)
{
  // Check for visiblity 
  if (!$config->BoxVisible)
    return false;

  // Get the light & dark colors
  $light = imagecolorallocate($result, $config->BoxLightColor[0], $config->BoxLightColor[1], $config->BoxLightColor[2]);
  $dark = imagecolorallocate($result, $config->BoxDarkColor[0], $config->BoxDarkColor[1], $config->BoxDarkColor[2]);
  
  // Draw the box
  imagerectangle($result, BOX_LEFT, BOX_TOP, BOX_LEFT + BOX_WIDTH - 1, BOX_TOP + BOX_HEIGHT - 1, $light);
  imagerectangle($result, BOX_LEFT + 1, BOX_TOP + 1, BOX_LEFT + BOX_WIDTH - 2, BOX_TOP + BOX_HEIGHT - 2, $dark);
  
}

/////////////////////
// Private function, draws credits
//
// Parameters:
// $result - the resulting image
// $config - the frontend configuration
function LXDrawCredits($result, $config)
{
  global $LXUTILS_PATH;

  $text = Array("OpenLieroX Preview",
   "- Original code by Jason Boettcher",
   "- Ported and enhanced by",
	 "  K. Petránek, Albert Zeyer, Daniel Sjoholm,", // In order of joining
	 "  Martin Griffin, Sergiy Pylypenko",
	 "- Supported by the [RIP] clan",
	 $config->CreatorText
   );
   
   // Get the correct color
   $col1 = imagecolorallocate($result, $config->Credits1Color[0], $config->Credits1Color[1], $config->Credits1Color[2]);
   $col2 = imagecolorallocate($result, $config->Credits2Color[0], $config->Credits2Color[1], $config->Credits2Color[2]);
   
   $lh = CREDITS_LINE_HEIGHT;  // Line height
   $x = $config->CreditsPos[0];
   $y = $config->CreditsPos[1] + $lh;
      
   // Draw the credits
   if (file_exists($LXUTILS_PATH . CREDITS_FONT))  {
     imagettftext($result, CREDITS_FONT_SIZE, 0, $x, $y, $col1, $LXUTILS_PATH . CREDITS_FONT, $text[0]);
     
     for ($i = 1; $i < count($text); $i++)  {
      imagettftext($result, CREDITS_FONT_SIZE, 0, $x, $y + ($lh + $config->CreditsSpacing) * $i, $col2, $LXUTILS_PATH . CREDITS_FONT, $text[$i]);
     }
     
   // Fallback if the font has not been found
   } else {
     $y -= $lh;  // imagestring uses top-left coords
     imagestring($result, 1, $x, $y, $text[0], $col1);
     
     for ($i = 1; $i < count($text); $i++)  {
      imagestring($result, 1, $x, $y + ($lh + $config->CreditsSpacing) * $i, $text[$i], $col2);
     } 
   }
}

///////////////////
// Private function, draws the logo
//
// Parameters:
// $result - the result image
// $logo - the logo image
function LXDrawLogo($result, $logo)
{
  $res_w = imagesx($result);
  $x = round(($res_w - imagesx($logo)) / 2);
  
  imagecopy($result, $logo, $x, LOGO_Y, 0, 0, imagesx($logo), imagesy($logo));
}

///////////////////
// Private function, draws the quit button
//
// Parameters:
// $result - the result image
// $buttons - the buttons image map
function LXDrawQuitButton($result, $buttons)
{
  $sy = 5 + BUT_QUIT * 40;  // 5px on top + 40px per button
  imagecopy($result, $buttons, QUIT_X, QUIT_Y, 5, $sy, imagesx($buttons), 20);
}

///////////////////
// Private function, finds and analyzes frontend.cfg and colours.cfg files
// If the file is not found, default values are returned
//
// Parameters:
// $zip - the zip file
// $frontend - zip file entry index to frontend.cfg file
// $colours - zip file entry index to colours.cfg file
//
// Return value:
// FrontendCfgInfo structure
function LXAnalyzeFrontendCfg($zip, $frontend, $colours)
{
  // Initialize the result
  $returnValue = new FrontendConfig();
  $returnValue->BoxLightColor = Array(130, 130, 130);
  $returnValue->BoxDarkColor = Array(60, 60, 60);
  $returnValue->BoxVisible = true;
  $returnValue->Credits1Color = Array(150, 150, 150);
  $returnValue->Credits2Color = Array(96, 96, 96);
  $returnValue->MainTitlesPos = Array(50, 160);
  $returnValue->MainTitlesSpacing = 15;
  $returnValue->CreditsPos = Array(370, 379);
  $returnValue->CreditsSpacing = 0;
  $returnValue->CreatorText = "";

  // Read frontend.cfg
  $buf = ($frontend === false ? 0 : $zip->getFromIndex($frontend));
  if ($buf)  {
  
    // Parse the ini file
    $frontend_config = LXParseIni($buf);
    if (!$frontend_config)
      return $returnValue;
    unset($buf);
    
    // Get data from frontend config
    
    // Main titles
    if (isset($frontend_config["MainTitles"]))  {
      $mt = $frontend_config["MainTitles"];
      if (isset($mt['X']))
        $returnValue->MainTitlesPos[0] = $mt['X'];
      if (isset($mt['Y']))
        $returnValue->MainTitlesPos[1] = $mt['Y'];
      if (isset($mt['Spacing']))
        $returnValue->MainTitlesSpacing = $mt['Spacing'];    
    }
    
    // Credits
    if (isset($frontend_config["Credits"]))  {
      $cr = $frontend_config["Credits"];
      if (isset($cr['X']))
        $returnValue->CreditsPos[0] = $cr['X'];
      if (isset($cr['Y']))
        $returnValue->CreditsPos[1] = $cr['Y'];
      if (isset($cr['Spacing']))
        $returnValue->CreditsSpacing = $cr['Spacing']; 
      if (isset($cr['FrontendCredits']))
        $returnValue->CreatorText = $cr['FrontendCredits'];      
    }
    
    // Page boxes
    if (isset($frontend_config["PageBoxes"]))  {
      $pb = $frontend_config["PageBoxes"];
      if (isset($pb["Visible"]))
        $returnValue->BoxVisible = strcasecmp($pb["Visible"], "true") == 0 ? true : false; 
    }
    unset($frontend_config);
  }
  
  // Get the data from color config
  $buf = $colours === false ? 0 : $zip->getFromIndex($colours);
  if ($buf)  {
      
    // Parse the ini file
    $colours_config = LXParseIni($buf);
    if (!$colours_config)
      return $returnValue;
    unset($buf);
    
    // Check
    if (!isset($colours_config["Colours"]))
      return $returnValue;
      
    $c = $colours_config["Colours"];
      
    // Credits
    if (isset($c["Credits1"]))
      $returnValue->Credits1Color = LXParseColor($c["Credits1"], $returnValue->Credits1Color);
    if (isset($colours_config["Credits2"]))
      $returnValue->Credits2Color = LXParseColor($c["Credits2"], $returnValue->Credits2Color);
      
    // Box
    if (isset($colours_config["BoxDark"]))
      $returnValue->BoxDarkColor = LXParseColor($c["BoxDark"], $returnValue->BoxDarkColor); 
    if (isset($colours_config["BoxLight"]))
      $returnValue->BoxLightColor = LXParseColor($c["BoxLight"], $returnValue->BoxLightColor);  
  }
    
  return $returnValue;
}

///////////////////
// Private function, converts hex color to an RGB array
function LXParseColor($color, $default = Array(0, 0, 0))
{
  if (!$color)
    return $default;

  // Check for # at the beginning
  if ($color[0] == '#')  {
    $color = substr($color, 1, strlen($color) - 1);
  }
  
  // Check length
  if (strlen($color) < 6)
    return $default;
    
  // Get the color
  $result = Array();
  $result[] = hexdec($color[0] . $color[1]);
  $result[] = hexdec($color[2] . $color[3]);
  $result[] = hexdec($color[4] . $color[5]);
  
  return $result;
}
?>