cl /I "C:\gtk\include\gtk-4.0" /I "C:\gtk\include" /I "C:\gtk\include\cairo" /I "C:\gtk\include\glib-2.0" /I "C:\gtk\include\gobject-introspection-1.0" /I "C:\gtk\lib\glib-2.0\include" /I "C:\gtk\include\pango-1.0" /I "C:\gtk\include\harfbuzz" /I "C:\gtk\include\gdk-pixbuf-2.0" /I "C:\gtk\include\graphene-1.0" /I "C:\gtk\lib\graphene-1.0\include" /I "C:\pgsql\include" C:\gtk\lib\*.lib C:\pgsql\lib\*.lib ./*.c /O2 /Fe"hotel2000.exe"

if not exist "..\build" mkdir "..\build"
copy hotel2000.exe ..\build
