#include <lwip/api.h>
#include <string.h>
#include <espressif/esp_common.h>

// #include "action.h"
// #include "utils.h"
#include "config.h"
#include "relay.h"

char statusResponse[512];
char * status()
{
    sprintf(statusResponse, "HTTP/1.1 200 OK\r\n"
        "Content-type: application/json\r\n\r\n"
        "{\"relay_1\":%d,\"relay_2\":%d}",
        Relay1.status(), Relay2.status());

    return statusResponse;
}

void relay(char *data)
{
    if (strstr((char *)data, (char *)"/relay/1/on")) {
        Relay1.on();
    } else if (strstr((char *)data, (char *)"/relay/1/off")) {
        Relay1.off();
    } else if (strstr((char *)data, (char *)"/relay/1/toggle")) {
        Relay1.toggle();
    } else if (strstr((char *)data, (char *)"/relay/2/on")) {
        Relay2.on();
    } else if (strstr((char *)data, (char *)"/relay/2/off")) {
        Relay2.off();
    } else if (strstr((char *)data, (char *)"/relay/2/toggle")) {
        Relay2.toggle();
    }
}

char * parse_request(void *data)
{
    char * response = NULL;
    // printf("data: %s\n", (char *)data);

    if (strstr((char *)data, (char *)"/status")) {
        printf("Get status\n");
        response = status();
    } else if (strstr((char *)data, (char *)"/relay")) {
        printf("Action on relay\n");
        relay((char *)data);
        response = (char *)"OK";
    } else {
        printf("unknown route\n");
    }

    return response;
}

#define HTTPD_PORT 80

void httpd_task(void *pvParameters)
{
    struct netconn *client = NULL;
    struct netconn *nc = netconn_new(NETCONN_TCP);
    if (nc == NULL) {
        printf("Failed to allocate socket\n");
        vTaskDelete(NULL);
    }
    netconn_bind(nc, IP_ADDR_ANY, HTTPD_PORT);
    netconn_listen(nc);
    char * response = NULL;
    while (1) {
        err_t err = netconn_accept(nc, &client);
        if (err == ERR_OK) {
            struct netbuf *nb;
            if ((err = netconn_recv(client, &nb)) == ERR_OK) {
                void *data = NULL;
                u16_t len;
                if (netbuf_data(nb, &data, &len) == ERR_OK) {
                    // printf("Received data:\n%.*s\n", len, (char*) data);
                    response = parse_request(data);
                }
                if (!response) {
                    response = (char *)"HTTP/1.1 404 OK\r\nContent-type: text/html\r\n\r\nUnknown route\r\n";
                }
                netconn_write(client, response, strlen(response), NETCONN_COPY);
            }
            netbuf_delete(nb);
        }
        netconn_close(client);
        netconn_delete(client);
    }
}