<pre>
Command line program to quickly bulk rename files by finding and replacing patterns.
Can work with multiple directories simultaneously and omit filenames from being renamed.
Works best by adding the program to your Windows system PATH.

Rename keywords:
between              Replace text between (not including) two patterns.
between+             Replace text between (including) two patterns.
!dots                Replace periods with spaces, ignoring pref and ext.
!series              Default parameters for filenames with s01e01 pattern.
!rnsubs              Match a folder's filenames (subs) to menu stems. 
!lower               Lowercase every letter.
!cap                 Capitalize every word.

Menu keywords:
!index               Show index numbers for filenames.
rm #(,#-#) [name]    Remove/restore filenames with index #s or name.
rm- (#(,#-#)) [name] Remove all filenames, except index/name.
!find !rfind [pat]   Remove all filenames containing/without pattern.
rmfolders, rmfiles   Remove all folders or files.
chdir, adir, rmdir   Change, add, or remove a working directory.
adir+                Add all menu folders to working directories.
!pwd                 Print work directories.

Pattern matches:
#begin               The start of filename.
#end                 The end of filename.
#ext                 Selects the file extension.
?                    Any digit 0-9. Can use match in replacement.
*                    Zero or one of any character or digit.
#^                   A number sequence for replacement. (01, 02...)
#index #             Points to the filename's index.

Other keywords:
!reload              Reload the menu from source files.
!wordcount           Get count of words and lines in menu text files.
!print               Creates a text file with menu list.
!history             Show a list of rename history. Undo past renames.
!undo                Undo the last rename.
q, exit, ''          Quit.
</pre>