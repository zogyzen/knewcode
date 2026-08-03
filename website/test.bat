set pths=%~dp0
set pth=%pths:~0,-1%
set apachepth=%pth%/../library/windows/Apache24x64
set websitepth=%pth%

%apachepth%/bin/httpd -X -d %apachepth% -f %websitepth%/my-prj.conf
:start http://127.0.0.1:801

pause
