#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>

#include "esp_log.h"
#include "esp_transport_tcp.h"
#include "esp_transport_internal.h"

static const char *TAG = "tcp_udp_mock";

typedef struct udp_ctx_s {
    int sock;
    struct sockaddr_in dst;
    // simple datagram-to-stream buffer
    unsigned char rbuf[4096];
    size_t rbuf_len;
    size_t rbuf_pos;
} udp_ctx_t;

static int set_nonblock(int fd, bool nb)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) return -1;
    if (nb) flags |= O_NONBLOCK; else flags &= ~O_NONBLOCK;
    return fcntl(fd, F_SETFL, flags);
}

static int udp_connect(esp_transport_handle_t t, const char *host, int port, int timeout_ms)
{
    (void)host; (void)port; (void)timeout_ms;
    udp_ctx_t *ctx = calloc(1, sizeof(udp_ctx_t));
    if (!ctx) return -1;

    const char *env_dst = getenv("MQTT_UDP_DST");
    const char *env_in  = getenv("MQTT_UDP_IN");
    const char *env_out = getenv("MQTT_UDP_OUT");
    const char *dst_ip = env_dst ? env_dst : "127.0.0.1";
    int port_in = env_in ? atoi(env_in) : 7771;   // where we listen (server->client)
    int port_out = env_out ? atoi(env_out) : 7777; // where we send (client->server)

    ctx->sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (ctx->sock < 0) {
        free(ctx);
        return -1;
    }

    struct sockaddr_in bind_addr = {0};
    bind_addr.sin_family = AF_INET;
    bind_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    bind_addr.sin_port = htons((uint16_t)port_in);
    if (bind(ctx->sock, (struct sockaddr*)&bind_addr, sizeof(bind_addr)) < 0) {
        int e = errno;
        ESP_LOGE(TAG, "bind failed: %d", e);
        close(ctx->sock);
        free(ctx);
        return -1;
    }

    memset(&ctx->dst, 0, sizeof(ctx->dst));
    ctx->dst.sin_family = AF_INET;
    ctx->dst.sin_port = htons((uint16_t)port_out);
    inet_pton(AF_INET, dst_ip, &ctx->dst.sin_addr);

    set_nonblock(ctx->sock, true);
    esp_transport_set_context_data(t, ctx);
    return 0; // success
}

static int udp_read(esp_transport_handle_t t, char *buffer, int len, int timeout_ms)
{
    udp_ctx_t *ctx = (udp_ctx_t*)esp_transport_get_context_data(t);
    if (!ctx) return -1;

    // Serve from buffer first
    if (ctx->rbuf_pos < ctx->rbuf_len) {
        size_t available = ctx->rbuf_len - ctx->rbuf_pos;
        size_t n = (size_t)len < available ? (size_t)len : available;
        memcpy(buffer, ctx->rbuf + ctx->rbuf_pos, n);
        ctx->rbuf_pos += n;
        if (ctx->rbuf_pos == ctx->rbuf_len) {
            ctx->rbuf_pos = ctx->rbuf_len = 0;
        }
        return (int)n;
    }

    // No buffered data: wait for next datagram
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(ctx->sock, &rfds);
    struct timeval tv, *ptv = esp_transport_utils_ms_to_timeval(timeout_ms, &tv);
    int rv = select(ctx->sock + 1, &rfds, NULL, NULL, ptv);
    if (rv == 0) return 0;      // timeout
    if (rv < 0) return -1;      // error

    ssize_t r = recv(ctx->sock, ctx->rbuf, sizeof(ctx->rbuf), 0);
    if (r <= 0) {
        if (r < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) return 0;
        return -1;
    }
    ctx->rbuf_len = (size_t)r;
    ctx->rbuf_pos = 0;

    // Now serve request from buffer
    size_t n = (size_t)len < ctx->rbuf_len ? (size_t)len : ctx->rbuf_len;
    memcpy(buffer, ctx->rbuf, n);
    ctx->rbuf_pos = n;
    if (ctx->rbuf_pos == ctx->rbuf_len) {
        ctx->rbuf_pos = ctx->rbuf_len = 0;
    }
    return (int)n;
}

static int udp_write(esp_transport_handle_t t, const char *buffer, int len, int timeout_ms)
{
    (void)timeout_ms;
    udp_ctx_t *ctx = (udp_ctx_t*)esp_transport_get_context_data(t);
    if (!ctx) return -1;
    ssize_t s = sendto(ctx->sock, buffer, len, 0, (struct sockaddr*)&ctx->dst, sizeof(ctx->dst));
    return (s < 0) ? -1 : (int)s;
}

static int udp_close(esp_transport_handle_t t)
{
    udp_ctx_t *ctx = (udp_ctx_t*)esp_transport_get_context_data(t);
    if (ctx) {
        if (ctx->sock >= 0) close(ctx->sock);
        free(ctx);
        esp_transport_set_context_data(t, NULL);
    }
    return 0;
}

static int udp_poll(esp_transport_handle_t t, int timeout_ms)
{
    udp_ctx_t *ctx = (udp_ctx_t*)esp_transport_get_context_data(t);
    if (!ctx) return -1;
    // If we already have buffered data, signal readable immediately
    if (ctx->rbuf_pos < ctx->rbuf_len) return 1;
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(ctx->sock, &rfds);
    struct timeval tv, *ptv = esp_transport_utils_ms_to_timeval(timeout_ms, &tv);
    int rv = select(ctx->sock + 1, &rfds, NULL, NULL, ptv);
    if (rv < 0) return -1;
    return rv; // 0 timeout, >0 readable
}

static int udp_get_socket(esp_transport_handle_t t)
{
    udp_ctx_t *ctx = (udp_ctx_t*)esp_transport_get_context_data(t);
    return ctx ? ctx->sock : -1;
}

// API required by esp-mqtt
// void esp_transport_tcp_set_keep_alive(esp_transport_handle_t t, esp_transport_keep_alive_t *cfg)
// {
//     (void)t; (void)cfg; // no-op for UDP
// }

// void esp_transport_tcp_set_interface_name(esp_transport_handle_t t, struct ifreq *if_name)
// {
//     (void)t; (void)if_name; // no-op
// }

esp_transport_handle_t esp_transport_tcp_init2(void)
{
    esp_transport_handle_t t = esp_transport_init();
    if (!t) return NULL;
    t->foundation = esp_transport_init_foundation_transport();
    if (!t->foundation) {
        esp_transport_destroy(t);
        return NULL;
    }
    esp_transport_set_func(t, udp_connect, udp_read, udp_write, udp_close, udp_poll, udp_poll, NULL);
    t->_get_socket = udp_get_socket;
    esp_transport_set_default_port(t, 1883);
    ESP_LOGI(TAG, "Initialized UDP-based tcp_transport mock (IN=%s OUT=%s)",
             getenv("MQTT_UDP_IN") ? getenv("MQTT_UDP_IN") : "7771",
             getenv("MQTT_UDP_OUT") ? getenv("MQTT_UDP_OUT") : "7777");
    return t;
}
