<?php
// Binary functions for LX Utils
// Code released under the LPGL license
// Created 20/11/2007 by Karel Petranek

// This file contains helper functions for binary data conversion,
// binary data reading, NULL terminated string reading and file skipping 

////////////////////////////
// Converts binary chunk to 32 bit Little Endian integer
function BinToInt32LE($bin)
{
  // Thank you, php.net :)
  $binary_data = substr($bin, 0, 4);
  $unpacked_data = @unpack("V", $binary_data);
  return $unpacked_data[1];
}

////////////////////////////
// Converts binary chunk to 32 bit Big Endian integer
function BinToInt32BE($bin)
{
  // Thank you, php.net :)
  $binary_data = substr($bin, 0, 4);
  $unpacked_data = @unpack("N", $binary_data);
  return $unpacked_data[1];
}

////////////////////////////
// Converts binary chunk to 16 bit Little Endian integer
function BinToInt16LE($bin)
{
  // Thank you, php.net :)
  $binary_data = substr($bin, 0, 2);
  $unpacked_data = @unpack("v", $binary_data);
  return $unpacked_data[1];
}

////////////////////////////
// Converts binary chunk to 16 bit Big Endian integer
function BinToInt16BE($bin)
{
  // Thank you, php.net :)
  $binary_data = substr($bin, 0, 2);
  $unpacked_data = @unpack("n", $binary_data);
  return $unpacked_data[1];
}

////////////////////////////
// Converts binary chunk to 16 bit Little Endian integer
function BinToFloatLE($bin)
{
  $binary_data = substr($bin, 0, 4);
  $unpacked_data = $unpacked_data = @unpack("f", $binary_data); // Lil endian
  
  // Dirty endian check
  if (pack("L", 123456) == pack("N", 123456))  {
    // Big endian
    strrev($binary_data);
    $unpacked_data = unpack("f", $binary_data);
  }
 
  return $unpacked_data[1];
}

///////////////////////////
// Reads a NULL terminated string from a binary chunk
function BinToStr($binary_data)
{
    $unpacked_data = "";
    for ($i = 0; $i < strlen($binary_data); $i++)  {
      if (ord($binary_data[$i]) == 0) // Null termination
        break;
      
      $unpacked_data .= $binary_data[$i];
    }
    
    return $unpacked_data;
}

////////////////////////////
// Reads a 32bit Little Endian integer from file $fp
function ReadInt32LE($fp)
{  
  $binary_data = fread($fp, 4);
  return BinToInt32LE($binary_data);   
}

////////////////////////////
// Reads a 32bit Little Endian float from file $fp
function ReadFloatLE($fp)
{  
  $binary_data = fread($fp, 4);
  return BinToFloatLE($binary_data);   
}



////////////////////////////
// Reads a fixed length C-string (0 terminated) from file $fp
function ReadFixedCStr($fp, $len)
{
  $binary_data = fread($fp, $len);
  return BinToStr($binary_data);
}

////////////////////////////
// Reads a variable length C-string from file
function ReadVariableCStr($fp)
{
  $result = "";
  $ch = fgetc($fp);
  while (ord($ch))  {
    $result .= $ch;
    $ch = fgetc($fp);
  }
  
  return $result;
}

////////////////////////////
// Reads a variable length Pascal string from file
function ReadVariablePascalStr($fp)
{
  $len = ord(fgetc($fp));
  if ($len)
    return fread($fp, $len);
  else
    return ""; 
}

///////////////////////////
// Skips $len bytes in the file
function Skip($fp, $len)
{
  fseek($fp, ftell($fp) + $len, SEEK_SET);
}

?>