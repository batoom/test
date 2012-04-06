#!/bin/sh

echo $#

if [ $# = 0 ]; then
	echo "no param"
else
	echo $@
fi

