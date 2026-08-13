#pragma once

#include "../AsyncWebServer/ESPAsyncWebServer.h"

namespace fs_api_http {

void register_routes(AsyncWebServer& server);

bool handle_manager_upload_chunk(
  AsyncWebServerRequest* request,
  const String& filename,
  size_t index,
  uint8_t* data,
  size_t len,
  bool final
);

bool handle_manager_upload_post(AsyncWebServerRequest* request);

} // namespace fs_api_http
