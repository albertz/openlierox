<?php
// Liero Xtreme utilities for PHP
// Code released under the LGPL license
// Created on 18/02/2009 by Karel Petranek

// Requirements:
// PHP 4 or 5
// gd (for the image processing functions) and zlib modules

// This file contains functions for working with (Open)LieroX mods

// Includes
require_once "binaryfunctions.php";

// Defines
define("GSE_VERSION", 7);
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

// Structures

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

// Public functions

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

// Private functions

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

?>
