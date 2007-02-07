<?php
function read_from_file($filename) {
	if(file_exists($filename)) {
		$handle = fopen($filename, "r");
		$contents = fread($handle, filesize ($filename));
		fclose($handle);
		return $contents;
	} else {
		return "";
	}
}
?>
