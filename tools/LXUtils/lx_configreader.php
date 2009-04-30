<?php
// Liero Xtreme utilities for PHP
// Code released under the LGPL license
// Created on 18/02/2009 by Karel Petranek

// Requirements:
// PHP 4 or 5
// gd (for the image processing functions) and zlib modules

// This file contains functions for a simple .ini parsing

function LXParseIni($data)
{
  $data = str_replace("\r\n", "\n", $data);
  
  // Split to lines
  $rows = explode("\n", $data);
  
  // Parse the lines
  $result = Array();
  $section = "*default*";
  for ($i = 0; $i < count($rows); $i++)  {
    // Sanitize
    $line = trim($rows[$i]);
    if (!$line)
      continue;
      
    // Comment
    if ($line[0] == "#" || $line[0] == ";")
      continue;
      
    // Section
    if ($line[0] == '[' && $line[strlen($line) - 1] == ']')  {
      $section = substr($line, 1, strlen($line) - 2);
      continue;
    }
    
    // Tokenize
    $token = explode("=", $line);
    if (count($token) != 2)
      continue;
    $token[0] = trim($token[0]);
    $token[1] = trim($token[1]);
      
    // Add
    $result[$section][$token[0]] = $token[1];
  }
  
  return $result;
}


?>