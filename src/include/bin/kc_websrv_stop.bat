for /f "delims=" %%i in (kc_websrv.pid) do (
    echo %%i
	taskkill /F /PID %%i
)
exit
