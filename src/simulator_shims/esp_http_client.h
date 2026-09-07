#pragma once

#include_next <esp_http_client.h>

#ifndef HTTP_EVENT_REDIRECT
#define HTTP_EVENT_REDIRECT static_cast<http_event>(2)
#endif

inline esp_err_t esp_http_client_set_url(esp_http_client_handle_t handle, const char* url) {
  if (!handle || !url) {
    return ESP_ERR_INVALID_ARG;
  }
  handle->config.url = url;
  return ESP_OK;
}

inline esp_err_t esp_http_client_set_method(esp_http_client_handle_t handle, esp_http_client_method_t method) {
  if (!handle) {
    return ESP_ERR_INVALID_ARG;
  }
  handle->config.method = method;
  return ESP_OK;
}
