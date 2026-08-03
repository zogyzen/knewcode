TEMPLATE = subdirs

SUBDIRS += \
    kc_framework \
    so_log_boost \
    so_log_my \
    kc_web_main \
    kc_webapi_work \
    kc_sql_postgresql \
    kc_sql_dm \
    # kc_sql_mysql \
    kc_sql_sqlite \
    kc_session_cookie \
    kc_os_file \
    kc_request_relay \
    kc_h5_sse_ws \
    kc_os_exec \
    # kc_parse_script \
    kc_chai_script \
    so_mq_callback \
    so_web_request \
    so_prjs \
    # so_print \
    # task_exec \
    kc_apache_mod \
    # kc_apache_mod_topr \
    # kc_apache_mod_full \
    kc_nginx_mod \
    kc_java_mod \
    kc_websrv_c_api \
    kc_websrv_proxy \
    # kc_websrv_mod \
    # kc_websrv_coost \
    kc_websrv \
    # kc_websrv_guard \
    # kc_websrv_pipe \
    so_chai_sys \
    so_chai_sql \
    so_sample/so_helloworld

!mingw {
    SUBDIRS += \
        kc_sql_oracle
}
