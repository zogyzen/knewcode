::到“https://sourceforge.net/projects/gsoap2/”下载最新版gsoap
wsdl2h -s  -o zzsdzfp.h http://111.202.226.69:8001/ZZSDZFP_TBKP_SPBM/services/WKWebService?wsdl
soapcpp2 -i -C -x -L zzsdzfp.h -ID:\gitblit_codes\example\gsoap-2.8\gsoap\import
pause
