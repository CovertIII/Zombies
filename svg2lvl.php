#!/usr/bin/php
<?php
if (PHP_SAPI != "cli") {
		echo "Use this script in command line only\n";
		echo "Usage: php ./scaff.php template_file.txt\n";
    exit;
}

//check the argc to see if there is a template file
if($argc != 2){
	echo "Usage: ./svg2lvl.php lvl.svg\n Will print to STOUT.";
    exit;
}

$svglvl = simplexml_load_file($argv[1]);

if($svglvl == FALSE){
	echo "Can't find file " . $argv[1] . "\n";
	exit;
}

$h = $svglvl['height'];
echo "hw {$svglvl['height']} {$svglvl['width']}\n";
echo "sc 2\n";

foreach($svglvl->g->circle as $circles){
    $mass = round($circles['r']*$circles['r']*3.14159/25.2, 1);
    //Safezone
		$circles['cy'] = $h - $circles['cy'];
		
    if($circles['fill'] == "#e5e5e5"){
        echo "sz {$circles['cx']} {$circles['cy']} {$circles['r']}\n";
    }
    //Hero
    else if($circles['fill'] == "#ff0000" || $circles['fill'] == "#FF0000"){
        echo "h {$circles['cx']} {$circles['cy']} 0 0 {$circles['r']} $mass\n";
    }
    //Persons
    else if($circles['fill'] == "#ffff00"){
        echo "p {$circles['cx']} {$circles['cy']} 0 0 {$circles['r']} $mass\n";
    }
    else if($circles['fill'] == "#00bf5f"){
        echo "z {$circles['cx']} {$circles['cy']} 0 0 {$circles['r']} $mass\n";
    }
}

foreach($svglvl->g->g as $vppl){
		$circles = $vppl->circle;
		$circles['cy'] = $h - $circles['cy'];
		
		$line = $vppl->line;
		$line['y1'] = $h - $line['y1'];
		$line['y2'] = $h - $line['y2'];
		
		$vx = $line['x2'] - $line['x1'];
		$vy = $line['y2'] - $line['y1'];
		
		if( (($line['x1']-$circles['cx'])^2 + ($line['y1']-$circles['cy'])^2) > (($line['x2']-$circles['cx'])^2 + ($line['y2']-$circles['cy'])^2)){
			$vx *= -1;
			$vy *= -1;
		}
		
    $mass = round($circles['r']*$circles['r']*3.14159/25.2, 1);

    //Safezone
    if($circles['fill'] == "#e5e5e5"){
        echo "sz {$circles['cx']} {$circles['cy']} {$circles['r']}\n";
    }
    //Hero
    else if($circles['fill'] == "#ff0000" || $circles['fill'] == "#FF0000"){
        echo "h {$circles['cx']} {$circles['cy']} $vx $vy {$circles['r']} $mass\n";
    }
    //Persons
    else if($circles['fill'] == "#ffff00"){
        echo "p {$circles['cx']} {$circles['cy']} $vx $vy {$circles['r']} $mass\n";
    }
    else if($circles['fill'] == "#00bf5f"){
        echo "z {$circles['cx']} {$circles['cy']} $vx $vy {$circles['r']} $mass\n";
    }

}


foreach($svglvl->g->line as $walls){
	$walls['y1'] = $h - $walls['y1'];
	$walls['y2'] = $h - $walls['y2'];
    echo "w {$walls['x1']} {$walls['y1']} {$walls['x2']} {$walls['y2']}\n";
}
