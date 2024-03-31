#!/bin/sh
#

GENERATE=""
HELP=""
RESET=""
RUN=""
SAMPLE=sample1.png
while test -n "$1"
do
	case "$1" in
	"--generate"|"-g")
		GENERATE="1"
		shift
		;;

	"--help"|"-h")
		HELP="1"
		shift
		;;

	"--less"|"-l")
		LESS="1"
		shift
		;;

	"--reset")
		RESET="1"
		shift
		;;

	"--run"|"-t")
		RUN="1"
		shift
		;;

	"--sample"|"-s")
		shift
		if test -z "$1"
		then
			echo "error: --sample require an argument."
			exit 1
		fi
		if test ! -f "$1"
		then
			echo "error: sample \"$1\" not found."
			exit 1
		fi
		SAMPLE="$1"
		RUN="1"
		shift
		;;

	*)
		echo "error: unknown command line option \"$1\"."
		exit 1
		;;

	esac
done

if test -n "$HELP"
then
	echo "Usage: `basename $0` [-opts]"
	echo "where -opts is one or more of:"
	echo " -g | --generate       run the tool to generate the tiles.cpp file."
	echo " -h | --help           print out this help screen."
	echo " -l | --less           use less against the make output."
	echo "      --reset          reset the BUILD folder before running make."
	echo " -t | --run            run resulting test."
	echo " -s | --sample <name>  select a different sample (default is \"sample1.png\")."
	exit 1
fi

if test -n "$RESET"
then
	rm -rf BUILD
fi

if test ! -d BUILD
then
	mkdir -p BUILD
	cd BUILD
	cmake ..
fi

if test -z "$LESS"
then
	make -C BUILD
else
	make -C BUILD 2>&1 | less -SR
fi

if test -n "$RUN"
then
	if test -n "$GENERATE"
	then
		BUILD/shisen-sho-resolver --generate-tiles tiles.cpp sample1.png
	else
		mkdir -p boards
		BUILD/shisen-sho-resolver "$SAMPLE" --save-board boards/board.txt
	fi
fi

