#!/bin/sh
if test  "$1" = "--help"  -o "$1" = "-?"
then
    echo 'Make for x86 platforms. Specify option "-I" to use Ctrl+C to cancel instead '
    echo 'of Ctrl+D and option "-H" to compile without help built in.'
    exit
fi

if test  "$1" = "-I"  -o "$2" = "-I" 
then
    export DEFFLAGS="-DSIGINTTOCANCEL"
else
    export DEFFLAGS=""
fi
if test  "$1" = "-H"  -o  "$2" = "-H" 
then
    export DEFFLAGS="$DEFFLAGS -DNOHELP"
fi

export CFLAGS="-march=i486 -mtune=i686 -Os -pipe $DEFFLAGS"
echo "CFlags: $CFLAGS"
if test -e libs/libqdinp2.a
then
    export LDFLAGS="-L./libs -Wl,-O1"
else
    export LDFLAGS="-Wl,-O1"
fi
make distrib/fredlin
if test -e distrib/fredlin
then
    echo 'Making distributable'
    tar -cvzf predist/fredlinx86.tar.gz distrib/fredlin README.TXT frhelp.txt
fi
