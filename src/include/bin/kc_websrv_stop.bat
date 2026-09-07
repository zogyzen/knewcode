set pthb=%~dp0
for /f "delims=" %%i in (%pthb%pid/my-prj.xml.pid) do (
    echo %%i
	taskkill /F /PID %%i
)
::exit
