#include "../include/serv.h"

/*
 * -----------------------
 * RESTfulness in C.......
 * ***********************
 * Oskar Bahner Hansen....
 * cph-oh82@cphbusiness.dk
 * 2024-11-08.............
 * -----------------------
 */

static const char *s_http_addr = "http://localhost:9999"; // HTTP port
//static const char *s_https_addr = "https://localhost:8443"; // HTTPS port
static const char *s_root_dir = "./resources";

void ev_handler(struct mg_connection *c, int ev, void *ev_data)
{

    if (ev == MG_EV_HTTP_MSG)
    { /* http request */
        struct mg_http_message *hm = (struct mg_http_message *) ev_data;

        if (mg_match(hm->uri, mg_str("/api/weather"), NULL))

        { /* weather api */
            bstring_t weather_report_json;
            bstring_init(weather_report_json);
            struct mg_str caps[3];

            if (hm->query.buf != NULL && hm->query.len > 0 && mg_match(hm->query, mg_str("city=*"), caps)) {
                if (caps[0].len > 0) {
                    struct mg_str city_str = mg_strdup(caps[0]);
                    get_weather_str(weather_report_json, city_str.buf);
                    free(city_str.buf);
                }
            }

            c_log_info(LOG_TAG, "%.*s %.*s", hm->uri.len, hm->uri.buf, hm->query.len, hm->query.buf);

            bstring_push_back(weather_report_json, 0);
            if (bstring_size(weather_report_json) > 1)
                mg_http_reply(c, 200, "Content-Type: application/json\r\n",
                        (char*) bstring_view(weather_report_json, 0, bstring_size(weather_report_json)));
            else
                mg_http_reply(c, 200, "Content-Type: application/json\r\n",
                        "{%m: %m}", mg_print_esc, 0, "404", mg_print_esc, 0, "not found");
        }

        else

        { /* file server */
            struct mg_http_serve_opts serv_opts =  { .root_dir = s_root_dir };
            mg_http_serve_dir(c, ev_data, &serv_opts);
        }
    }

}

int serve(int argc, char **argv)
{

    struct mg_mgr mgr;
    mg_mgr_init(&mgr);

    mg_http_listen(&mgr, s_http_addr, ev_handler, NULL);

    c_log_info(LOG_TAG, "listening on (http) %s", s_http_addr);

    for (;;)
        mg_mgr_poll(&mgr, 1000);

    mg_mgr_free(&mgr);
    return EXIT_SUCCESS;

}
