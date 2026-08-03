在CentOS下，
1、修改“knewcode.conf”和“directory-alias.conf”文件中的相应目录。
2、将“knewcode.conf”文件拷贝到“/etc/httpd/conf.modules.d/”目录下；将“directory-alias.conf”文件拷贝到“/etc/httpd/conf.d/”目录下。
3、修改“/etc/httpd/conf/httpd.conf”文件，修改主页目录、端口等。
4、用“sudo chmod -R 777 ×××”命令设置主页目录的权限。
5、用“sudo chown -R www-data ×××”命令设置主页目录的所有者为web用户。

在Ubuntu下，
1、修改“knewcode.conf”和“directory-alias.conf”文件中的相应目录。
2、将“knewcode.conf”文件拷贝到“/etc/apache2/mods-enabled/”目录下；将“directory-alias.conf”文件拷贝到“/etc/apache2/conf-enabled/”目录下。
3、修改“/etc/apache2/apache2.conf”等文件，修改主页目录、端口等。
4、用“sudo chmod -R 777 ×××”命令设置主页目录的权限。
5、用“sudo chown -R www-data ×××”命令设置主页目录的所有者为web用户。
