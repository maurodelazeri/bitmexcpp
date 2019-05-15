# BitmexCPP

libwebsockets implementation

This is a simple example of how to connect https://www.bitmex.com using C++

This project has only one dependency https://libwebsockets.org/ 

if you want to use https://conan.io/ the conanfile.txt is avaiable

ex: with clion

```
conan install . -s build_type=Debug --install-folder=cmake-build-debug --build missing
```

You can set different parameters as you need, a full list of parameters can be found here https://libwebsockets.org/lws-api-doc-master/html/structlws__context__creation__info.html


```
    creation_info.port                     = CONTEXT_PORT_NO_LISTEN;
    creation_info.iface                    = NULL;
    creation_info.protocols                = protocols;
    creation_info.ssl_cert_filepath        = NULL;
    creation_info.ssl_private_key_filepath = NULL;
    creation_info.extensions               = NULL;
    creation_info.gid                      = -1;
    creation_info.uid                      = -1;
    creation_info.options                 |= LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
    creation_info.fd_limit_per_thread      = 1024;
    creation_info.max_http_header_pool     = 1024;
    creation_info.ws_ping_pong_interval    = 10;
    creation_info.ka_time                  = 10;
    creation_info.ka_probes                = 10;
    creation_info.ka_interval              = 10;
```
