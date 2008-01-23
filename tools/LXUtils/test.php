<html>
  <head>
    <title>LieroX Utils Test Page</title>
  </head>
  <body>
    <!--
    LEVEL INFO
    -->
    <h1>Level Info Test</h1>
    <?php
    require "lxutils.php";
    
    $levelInfo = LXLevelInfo("JukkeDome.lxl");  // Get the level info
    if ($levelInfo === false)
      echo "Could not get the level info!<br>\n";
    else  {
      imagepng($levelInfo->MinimapImage, "preview.png");  // Save the minimap as png
      
      // Print the level info
      echo "<b>Width:</b> " . $levelInfo->Width . "<br>\n";
      echo "<b>Height:</b> " . $levelInfo->Height . "<br>\n";
      echo "<b>Theme:</b> " . $levelInfo->Theme . "<br>\n";
      echo "<b>Name:</b> " . $levelInfo->Name . "<br>\n";
      echo "<b>Preview:</b></br>\n";
      echo "<img src=\"preview.png\" alt=\"Preview not available\">\n";
     
      // Clean up the rubbish 
      $levelInfo->Destroy();
    }
    ?>
    
    <!--
    ORIGINAL LIERO LEVEL INFO
    -->
    <h1>Original Liero Level Info Test</h1>
    <?php
    
    $origLevelInfo = LXLevelInfo("fallout2.lev");  // Get the level info
    if ($origLevelInfo === false)
      echo "Could not get the original level info!<br>\n";
    else  {
      imagepng($origLevelInfo->MinimapImage, "preview_orig.png");  // Save the minimap as png
      
      // Print the level info
      echo "<b>Width:</b> " . $origLevelInfo->Width . "<br>\n";
      echo "<b>Height:</b> " . $origLevelInfo->Height . "<br>\n";
      echo "<b>Theme:</b> " . $origLevelInfo->Theme . "<br>\n";
      echo "<b>Name:</b> " . $origLevelInfo->Name . "<br>\n";
      echo "<b>Preview:</b></br>\n";
      echo "<img src=\"preview_orig.png\" alt=\"Preview not available\">\n";
     
      // Clean up the rubbish 
      $origLevelInfo->Destroy();
    }
    ?>    
       
    <!--
    MOD INFO
    -->
    <h1>Mod Info Test</h1>
    <?php
    
    $modInfo = LXModInfo(".");  // Get the mod info
    if ($modInfo === false)
      echo "Could not get the mod info!<br>\n";
    else  {     
      // Print the mod info
      echo "<b>Name:</b> " . $modInfo->Name . "<br>\n";
      echo "<b>Weapon count:</b> " . $modInfo->WeaponCount . "<br>\n";
      echo "<b>Projectile count:</b> " . $modInfo->ProjectileCount . "<br>\n";
      echo "<b>Overall damage:</b> " . $modInfo->OverallDamage . "<br>\n";
      
      // "Grade" the mod
      // HINT: this grading is not exact
      $spamGrade = $modInfo->ProjectileCount / $modInfo->WeaponCount;
      $damageGrade = $modInfo->OverallDamage / $modInfo->WeaponCount;
      $spamGradeStr = $spamGrade >= 2 ? "spammy" : "not spammy";
      $damageGradeStr = "balanced";
      if ($damageGrade >= 33)
        $damageGradeStr = "overpowered";
      else if ($damageGrade >= 10)
        $damageGradeStr = "balanced";
      else
        $damageGradeStr = "underpowered";
        
      echo "<b>Grading:</b> " . $spamGradeStr . " and " . $damageGradeStr . "<br>\n";
      echo "<b>List of weapons:</b></br>\n";
      for ($i = 0; $i < $modInfo->WeaponCount; $i++) {
        echo ($i + 1) . ". " . $modInfo->Weapons[$i] . "<br>\n";
      }
    }
    ?>    
       
    <!--
    ANIMATED SKIN
    -->
    <h1>Skin Test</h1>
    <?php  
    // Create an animation from default.png, with default color
    $animSkin = LXSkinToAnimGIF("default.png");
    if ($animSkin === false) {
      echo "Could not create an animated GIF.<br>\n";
    } else {
      // Save the animation to a file
      $fp = fopen("default_animated.gif", "wb");
      if ($fp) {
        fwrite($fp, $animSkin);
        fclose($fp);
        unset($animSkin); // Cleanup
        
        echo "<img src=\"default_animated.gif\" alt=\"\"><br>\n"; // Display it
      } else {
        echo "Could not write out the animation.<br>\n";
      }
    }
    
    // Define some nice animation
    // The numbers are frame numbers of the skin, counting starts from 1 (not 0)
    $def = Array(
    Array("frame" => 5, "flipped" => false, "color" => Array(255, 0, 0)),
    Array("frame" => 12, "flipped" => false, "color" => Array(255, 255, 0)),
    Array("frame" => 19, "flipped" => false, "color" => Array(0, 255, 0)),
    Array("frame" => 5, "flipped" => true, "color" => Array(0, 255, 255)),
    Array("frame" => 12, "flipped" => true, "color" => Array(0, 0, 255)),
    Array("frame" => 19, "flipped" => true, "color" => -1));
    
    $animSkin2 = LXSkinToAnimGifAdv("default.png", $def, 20, 0);
    if ($animSkin2 === false) {
      echo "Could not create an advanced animated GIF.<br>\n";
    } else {
      // Save the animation to a file
      $fp = fopen("default_advanimated.gif", "wb");
      if ($fp) {
        fwrite($fp, $animSkin2);
        fclose($fp);
        unset($animSkin2); // Cleanup
        
        echo "<img src=\"default_advanimated.gif\" alt=\"\"><br>\n"; // Display it
      } else {
        echo "Could not write out the advanced animation.<br>\n";
      }
    }    
    
    // Get the 10th frame from the skin, make it red + flipped and display it
    // HINT: now it returns GD image, not GIF data
    $skinFrame = LXSkinGetFrame("default.png", 10, true, Array(255, 0, 0)); 
    if ($skinFrame === false) {
      echo "Could not get the frame from skin.<br>\n";
    } else {
      imagepng($skinFrame, "default_frame.png");
      imagedestroy($skinFrame); // Cleanup
      
      echo "<img src=\"default_frame.png\" alt=\"\"><br>\n";
    }
            
    ?>
        
    <!--
    SERVER LIST
    -->
    <h1>Server List Test</h1>
    <?php
    $masterservers[0] = "http://lieroxtreme.thegaminguniverse.com/server/svr_list.php";
    $masterservers[1] = "http://thelobby.altervista.org/server/svr_list.php";
    $servers = LXGetServerList($masterservers);
    
    echo "<b>Number of servers:</b> " . count($servers) . "</b><br>\n";
    echo "<b>Server list:</b><br>\n";
    for ($i = 0; $i < count($servers); $i++)  {
      echo $servers[$i] . "<br>\n";
    }
    ?>
    
    <!--
    SERVER INFO
    -->
    <h1>Server Info Test</h1>
    <?php
    $randomServer = $servers[rand(0, count($servers) - 1)];
    $serverInfo = LXServerInfo($randomServer);
    if ($serverInfo === false)
      echo "Could not get the server info!<br>\n";
    else {
      echo "<b>Name:</b> " . $serverInfo->Name . "<br>\n";
      echo "<b>Ping:</b> " . $serverInfo->Ping . "<br>\n";
      echo "<b>Max players:</b> " . $serverInfo->MaxPlayers . "<br>\n";
      echo "<b>Player count:</b> " . $serverInfo->NumPlayers . "<br>\n";
      echo "<b>State:</b> " . $serverInfo->State . "<br>\n";
      echo "<b>Map name:</b> " . $serverInfo->MapName . "<br>\n";
      echo "<b>Mod name:</b> " . $serverInfo->ModName . "<br>\n";
      echo "<b>Game Mode:</b> " . $serverInfo->GameMode . "<br>\n";
      echo "<b>Lives:</b> " . $serverInfo->Lives . "<br>\n";
      echo "<b>Max kills:</b> " . $serverInfo->MaxKills . "<br>\n";
      echo "<b>Loading time:</b> " . $serverInfo->LoadingTime . " %<br>\n";
      echo "<b>Bonuses:</b> " . ($serverInfo->BonusesOn ? "on" : "off") . "<br>\n";
      echo "<b>Worms:</b><br>\n";
      for ($i = 0; $i < $serverInfo->NumPlayers; $i++)  {
        echo "  <i>" . $serverInfo->Worms[$i]->Name . " </i>(";
        echo $serverInfo->Worms[$i]->Kills . " kills, \n";
        if ($serverInfo->Worms[$i]->Lives == "out")
          echo "out of the game)<br>\n";
        else
          echo $serverInfo->Worms[$i]->Lives . " lives)\n";
          
        $ip = $serverInfo->Worms[$i]->IP;
        echo "[" . ($ip ? $ip : "unknown IP")  . "]<br>\n";
      }
    }
    ?> 
    
    <!--
    FAST SERVER INFO
    -->
    <h1>Fast Server Info Test</h1>
    <?php
    $randomServer = $servers[rand(0, count($servers) - 1)];
    $fastServerInfo = LXServerInfo($randomServer);
    if ($fastServerInfo === false)
      echo "Could not get the fast server info!<br>\n";
    else {
      echo "<b>Name:</b> " . $fastServerInfo->Name . "<br>\n";
      echo "<b>Ping:</b> " . $fastServerInfo->Ping . "<br>\n";
      echo "<b>Max players:</b> " . $fastServerInfo->MaxPlayers . "<br>\n";
      echo "<b>Player count:</b> " . $fastServerInfo->NumPlayers . "<br>\n";
      echo "<b>State:</b> " . $fastServerInfo->State . "<br>\n";
    }
    ?>
    
    <!--
    FAST SERVER INFO
    -->
    <h1>Ping Server Test</h1>
    <?php
    $randomServer = $servers[rand(0, count($servers) - 1)];
    $ping = LXPingServer($randomServer);
    if ($ping === false)
      echo "Could not ping the server (connection timeout)!<br>\n";
    else {
      echo "<b>Ping to " . $randomServer . ":</b> " . $ping;
    }
    ?>    
  </body>
</html>