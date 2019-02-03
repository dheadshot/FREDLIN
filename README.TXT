FREDLIN README
-----------------
FREDLIN Version 0.61.01 BETA

FREDLIN: An EDLIN-like line editor for Linux.

For details on how to use FREDLIN and information about how FREDLIN came to be, see the file "frhelp.txt" that is included in this repository.

IMPORTANT: Before attempting to compile FREDLIN, ensure you have downloaded the "libqdinp2" library from https://github.com/dheadshot/libqdinp2 and placed the file "libqdinp2.a" in the "libs/" directory and the file "qdinp2.h" in the main FREDLIN directory (the same one this file is in).

How to use the distributed file:
(If you have simply downloaded one of the .tar.gz files from the "predist/" directory, this is the section to read.)
1.	Extract "fredlin" from the tar.gz file.
2.	If it does not have executable rights on your system already, give it them with the "chmod +x fredlin" command.
3.	To open a text file, run it with the command "./fredlin textfile", where "textfile" is the filename of the text file.
4.	For help in using FREDLIN, use the "H" command from within FREDLIN (if it was compiled with Help enabled) or read the "frhelp.txt" textfile.

How to compile for x86 systems:
(If you have downloaded the source files, this is the section to read.)
1.	Download "libqdinp2" and place the library (the .a file) in the "libs/" directory and the header file (the .h file) in the main FREDLIN directory (the same directory you found this file).
2.	Run the "makex86.sh" shell script.  If you do not want the Help system to be enabled, specify the "-H" flag.  To assign "Cancel" to Ctrl+C instead of Ctrl+D, specify the "-I" flag.  The flags can be specified in either order.
3.	The compiled file will be in the "distrib/" directory and will be called "fredlin".  The x86 distributable .tar.gz file will be in the "predist/" directory.  Copy these to where you need them.

How to compile for other systems:
(If you have downloaded the source files and do not use an x86 PC, this is the section to read.)
1.	Download "libqdinp2" and place the library (the .a file) in the "libs/" directory and the header file (the .h file) in the main FREDLIN directory.
2.	In the shell, "export" your preferred compilation flags in "$CFLAGS" and linking flags in "$LDFLAGS".  If you wish to compile without including the Help system, specify the "-DNOHELP" flag amongst the compilation flags.  If you wish to use Ctrl+C to cancel instead of Ctrl+D then include the flag "-DSIGINTTOCANCEL" amongsth the compilation flags.
3.	In the shell, run "make distrib/fredlin".
4.	The compiled file will be in the "distrib/" directory and will be called "fredlin".  Copy this to where you need it.


----------------------------------END OF README--------------------------------
