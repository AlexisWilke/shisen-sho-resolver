<?php

$r = 0;
foreach(glob("boards/*-board.txt") as $n)
{
	$f = fopen($n, "r");
	if($f === false)
	{
		echo "error: could not open \"$n\".\n";
		exit(1);
	}

	$z = 0;
	$c = 0;
	while(($l = fgets($f)) !== false)
	{
		$v = explode(",", $l);
		$c += count($v);
		foreach($v as $t)
		{
			if($t == 0)
			{
				++$z;
			}
		}
	}
	if($c == $z)
	{
		echo "info: found empty board: $n\n";
		++$r;
	}

	fclose($f);
}

if($r == 0)
{
	echo "error: no empty board found (board cannot be resolved).\n";
}

