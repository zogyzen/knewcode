rem  ///////////delete file///////////
del /Q/S *.depend
del /Q/S *.layout
rmdir /Q/S debug
rem  ///////////delete obj dir///////////
dir /b/s obj>>del_fales.txt
for /f "delims=" %%i in (del_fales.txt) do rd /q/s "%%i" 
del /q/s del_fales.txt
rem  ///////////delete bin dir///////////
dir /b/s bin>>del_fales.txt
for /f "delims=" %%i in (del_fales.txt) do rd /q/s "%%i" 
del /q/s del_fales.txt 
pause