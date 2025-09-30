set(IDF_TARGET linux)

# Re-implement tcp_transport using UDP sockets for testing.
# Reuse IDF's common transport sources for list/dispatch & errors.

set(IDF_TCP_TRANSPORT_DIR ${CMAKE_CURRENT_LIST_DIR}/../../../../deps/idf/components/tcp_transport)

idf_component_register(
    SRCS
        ${IDF_TCP_TRANSPORT_DIR}/transport.c
        ${IDF_TCP_TRANSPORT_DIR}/transport_internal.c
        ${IDF_TCP_TRANSPORT_DIR}/transport_ssl.c
        esp_transport_tcp_udp.c
    INCLUDE_DIRS
        ${IDF_TCP_TRANSPORT_DIR}/include
    PRIV_INCLUDE_DIRS
        ${IDF_TCP_TRANSPORT_DIR}/private_include
    REQUIRES
        esp-tls
)
