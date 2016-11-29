1、修改“kc_apache_mod.conf”和“kc_apache_mod.load”文件中的相应目录；
2、在linux下，将这2个文件拷贝到“/etc/apache2/mods-available/”目录下；
3、使用“ln -s /etc/apache2/mods-available/kc_apache_mod.conf /etc/apache2/mods-enabled/”命令，建立这2个文件的软链接；
4、修改“/etc/apache2/sites-available/000-default.conf”文件，修改主页目录；
5、用“sudo chmod -R 777 ×××”命令设置主页目录的权限。
6、用“sudo chown -R www-data ×××”命令设置主页目录的所有者为web用户。
