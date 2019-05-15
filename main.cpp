#include <iostream>
#include <cstdlib>
#include <string>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <signal.h>
#include <rapidjson/stringbuffer.h>

#include <libwebsockets.h>

using namespace rapidjson;
using namespace std;

static int destroy_flag = 0;
static int writeable_flag = 0;

struct session_data
{
    int fd;
};

static void sigint_handler(int sig)
{
    destroy_flag = 1;
}

string getJson(void* data)
{
    Document json;
    json.Parse(reinterpret_cast<const char*>(data));

    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    json.Accept(writer);

    return buffer.GetString();
}

string createJsonString()
{
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    writer.StartObject();

    writer.Key("op");
    writer.String("subscribe");

    writer.Key("args");
    writer.String("orderBookL2_25::XBTUSD");

    writer.EndObject();
    return buffer.GetString();
}

// pick the callback function to process data received from server
static int lws_event_callback(struct lws* conn, enum lws_callback_reasons reason, void* user, void* data, size_t len)
{
    cout << "registering lws event callback" << endl;

    switch(reason)
    {
        case LWS_CALLBACK_CLIENT_ESTABLISHED:
        {
            cout << "received LWS_CALLBACK_CLIENT_ESTABLISHED" << endl;
            lws_callback_on_writable(conn);
            break;
        }
        case LWS_CALLBACK_PROTOCOL_INIT:
        {
            cout << "received LWS_CALLBACK_PROTOCOL_INIT" << endl;
            break;
        }
        case LWS_CALLBACK_CLIENT_RECEIVE:
        {
            cout << "received LWS_CALLBACK_CLIENT_RECEIVE" << endl;

            lwsl_debug((char *)(getJson(data).c_str()));
            cout << getJson(data) << endl;

            break;
        }
        case LWS_CALLBACK_CLIENT_WRITEABLE:
        {
            cout << "received LWS_CALLBACK_CLIENT_WRITEABLE" << endl;

            if(writeable_flag == 1)
            {
                break;
            }

            writeable_flag = 1;

            unsigned char message[512];
            memset(&message[LWS_PRE], 0, 512 - LWS_PRE);

            string json = "{\"op\": \"subscribe\", \"args\": [\"orderBookL2_25:XBTUSD\"]}";//createJsonString();
            int length = json.length();
            cout << json << endl;
            lwsl_debug((char*)json.c_str());

            strncpy((char*)message + LWS_PRE, json.c_str(), length);
            lwsl_debug(reinterpret_cast<char *>(message + LWS_PRE));
            lws_write(conn, &message[LWS_PRE], length, LWS_WRITE_TEXT);

            break;
        }
        case LWS_CALLBACK_CLOSED:
        {
            cout << "received LWS_CALLBACK_CLOSED" << endl;
            break;
        }
        case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
        {
            cout << "received LWS_CALLBACK_CLIENT_CONNECTION_ERROR" << endl;
            break;
        }
        default:
        {
            cout << "received callback reason " << reason << endl;
        }
    }

    return 0;
}

static struct lws_protocols protocols[] =
        {
                {
                        "example_protocol",
                        lws_event_callback,
                             0,
                                65536,
                },
                {NULL, NULL, 0, 0} /* terminator */
        };

int main(void)
{
    cout << "initiating BitMEX websocket" << endl;

    cout << "setting log level" << endl;
    int logs = LLL_ERR  | LLL_WARN;
    lws_set_log_level(logs, NULL);
    cout << "log level set" << endl;

    signal(SIGINT, sigint_handler);

    cout << "creating context" << endl;
    struct lws_context_creation_info creation_info;
    memset(&creation_info, 0, sizeof(creation_info));

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

    struct lws_context* context = lws_create_context(&creation_info);
    cout << "context created" << endl;

    cout << "creating lws connection" << endl;
    struct lws_client_connect_info connect_info = {0};

    string host = "www.bitmex.com";
    string path = "/realtime";
    int port = 443;

    connect_info.context        = context;
    connect_info.address        = host.c_str();
    connect_info.port           = port;
    connect_info.path           = path.c_str();
    connect_info.host           = host.c_str();
    connect_info.origin         = host.c_str();
    connect_info.protocol       = protocols[0].name;
    connect_info.ssl_connection = LCCSCF_USE_SSL | LCCSCF_ALLOW_SELFSIGNED | LCCSCF_SKIP_SERVER_CERT_HOSTNAME_CHECK;
    struct lws* conn = lws_client_connect_via_info(&connect_info);
    cout << "connecting to " << connect_info.host << ":" << connect_info.port << ":" << connect_info.path << endl;

    if(!conn)
    {
        cout << "error creating lws connection" << endl;
    }

    cout << "done initiating and creating lws connection" << endl;

    lws_callback_on_writable(conn);

    while(true)
    {
        lws_service(context, 500);
    }

    lws_context_destroy(context);

    return 0;
}