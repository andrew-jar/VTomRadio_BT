#include "fs_api_http.h"

#include <FS.h>
#if __has_include(<LittleFS.h>)
  #include <LittleFS.h>
#endif
#if __has_include(<SPIFFS.h>)
  #include <SPIFFS.h>
#endif
#include <ESP.h>

#include <time.h>
#include <vector>

namespace fs_api_http {
namespace {

struct UploadSession {
  AsyncWebServerRequest* request = nullptr;
  bool active = false;
  bool hadError = false;
  bool disconnectHooked = false;
  String error;
  String targetPath;
};

std::vector<UploadSession> g_uploadSessions;

enum class FsBackend {
  None,
  Little,
  Spiffs,
};

bool fs_ready(fs::FS& fs) {
  File root = fs.open("/");
  if (!root) {
    return false;
  }
  root.close();
  return true;
}

FsBackend detect_backend() {
#if __has_include(<LittleFS.h>)
  if (fs_ready(LittleFS)) {
    return FsBackend::Little;
  }
#endif
#if __has_include(<SPIFFS.h>)
  if (fs_ready(SPIFFS)) {
    return FsBackend::Spiffs;
  }
#endif
#if __has_include(<LittleFS.h>)
  return FsBackend::Little;
#elif __has_include(<SPIFFS.h>)
  return FsBackend::Spiffs;
#else
  return FsBackend::None;
#endif
}

const char* backend_name(FsBackend backend) {
  switch (backend) {
    case FsBackend::Little:
      return "littlefs";
    case FsBackend::Spiffs:
      return "spiffs";
    default:
      return "none";
  }
}

bool with_active_fs(fs::FS*& outFs) {
  switch (detect_backend()) {
#if __has_include(<LittleFS.h>)
    case FsBackend::Little:
      outFs = &LittleFS;
      return true;
#endif
#if __has_include(<SPIFFS.h>)
    case FsBackend::Spiffs:
      outFs = &SPIFFS;
      return true;
#endif
    default:
      outFs = nullptr;
      return false;
  }
}

String normalize_path(String path) {
  path.trim();
  path.replace("\\", "/");
  while (path.indexOf("//") >= 0) {
    path.replace("//", "/");
  }
  while (path.indexOf("..") >= 0) {
    path.replace("..", "");
  }
  if (!path.length()) {
    return String("/");
  }
  if (!path.startsWith("/")) {
    path = "/" + path;
  }
  return path;
}

String json_escape(const String& in) {
  String out;
  out.reserve(in.length() + 8);
  for (size_t i = 0; i < in.length(); i++) {
    const char c = in[i];
    switch (c) {
      case '\\': out += "\\\\"; break;
      case '"': out += "\\\""; break;
      case '\n': out += "\\n"; break;
      case '\r': out += "\\r"; break;
      case '\t': out += "\\t"; break;
      default:
        out += c;
        break;
    }
  }
  return out;
}

void ensure_parent_dirs(fs::FS& fs, const String& fullPath) {
  const int slash = fullPath.lastIndexOf('/');
  if (slash <= 0) {
    return;
  }

  const String parent = fullPath.substring(0, slash);
  String current;
  int from = 0;
  while (from < (int)parent.length()) {
    const int to = parent.indexOf('/', from) >= 0 ? parent.indexOf('/', from) : parent.length();
    const String part = parent.substring(from, to);
    if (part.length()) {
      current += "/" + part;
      if (!fs.exists(current)) {
        fs.mkdir(current);
      }
    }
    from = to + 1;
  }
}

UploadSession& get_session(AsyncWebServerRequest* request) {
  for (auto& session : g_uploadSessions) {
    if (session.request == request) {
      return session;
    }
  }
  g_uploadSessions.push_back(UploadSession{});
  g_uploadSessions.back().request = request;
  return g_uploadSessions.back();
}

void clear_session(AsyncWebServerRequest* request) {
  for (size_t i = 0; i < g_uploadSessions.size(); i++) {
    if (g_uploadSessions[i].request == request) {
      g_uploadSessions.erase(g_uploadSessions.begin() + i);
      return;
    }
  }
}

bool is_manager_upload_request(AsyncWebServerRequest* request) {
  if (!request || request->url() != "/upload") {
    return false;
  }
  return request->hasParam("path") || request->hasParam("path", true) || request->hasParam("path", false, true);
}

String request_path_param(AsyncWebServerRequest* request) {
  if (request->hasParam("path")) {
    return request->getParam("path")->value();
  }
  if (request->hasParam("path", true)) {
    return request->getParam("path", true)->value();
  }
  if (request->hasParam("path", false, true)) {
    return request->getParam("path", false, true)->value();
  }
  return String();
}

bool collect_remove_targets(fs::FS& fs, const String& path, std::vector<String>& files, std::vector<String>& dirs) {
  File entry = fs.open(path);
  if (!entry) {
    return false;
  }

  if (!entry.isDirectory()) {
    entry.close();
    files.push_back(path);
    return true;
  }

  File child = entry.openNextFile();
  while (child) {
    String childPath = String(child.path());
    if (!childPath.length()) {
      childPath = String(child.name());
    }
    if (!childPath.startsWith("/")) {
      childPath = path + "/" + childPath;
    }

    const bool childIsDir = child.isDirectory();
    child.close();

    if (childIsDir) {
      collect_remove_targets(fs, normalize_path(childPath), files, dirs);
    } else {
      files.push_back(normalize_path(childPath));
    }

    child = entry.openNextFile();
  }

  entry.close();
  dirs.push_back(path);
  return true;
}

bool remove_recursive(fs::FS& fs, const String& rawPath) {
  const String path = normalize_path(rawPath);
  if (path == "/") {
    return false;
  }

  std::vector<String> files;
  std::vector<String> dirs;
  if (!collect_remove_targets(fs, path, files, dirs)) {
    return false;
  }

  bool ok = true;
  for (const String& filePath : files) {
    if (fs.exists(filePath) && !fs.remove(filePath)) {
      ok = false;
    }
    delay(0);
  }

  for (auto it = dirs.rbegin(); it != dirs.rend(); ++it) {
    const String& dirPath = *it;
    if (dirPath != "/" && fs.exists(dirPath) && !fs.rmdir(dirPath)) {
      ok = false;
    }
    delay(0);
  }

  return ok;
}

void list_recursive_json(File dir, String& out, bool& first) {
  File f = dir.openNextFile();
  while (f) {
    String path = String(f.path());
    if (!path.length()) {
      path = String(f.name());
    }
    if (!path.startsWith("/")) {
      path = "/" + path;
    }

    if (!first) {
      out += ",";
    }
    first = false;

    out += "{\"path\":\"";
    out += json_escape(path);
    out += "\",\"size\":";
    out += String((uint32_t)(f.isDirectory() ? 0 : f.size()));
    out += ",\"dir\":";
    out += f.isDirectory() ? "true" : "false";
    out += ",\"mtime\":";
    const time_t lastWrite = f.getLastWrite();
    out += String((uint32_t)(lastWrite > 0 ? lastWrite : 0));
    out += "}";

    if (f.isDirectory()) {
      list_recursive_json(f, out, first);
    }

    f = dir.openNextFile();
  }
}

void handle_fs_ping(AsyncWebServerRequest* request) {
  fs::FS* fs = nullptr;
  if (with_active_fs(fs) && fs) {
    request->send(200, "application/json", "{\"ok\":true}");
    return;
  }
  request->send(500, "application/json", "{\"ok\":false,\"error\":\"fs_not_mounted\"}");
}

void handle_fs_info(AsyncWebServerRequest* request) {
  fs::FS* fs = nullptr;
  FsBackend backend = detect_backend();
  if (!with_active_fs(fs) || !fs) {
    request->send(500, "application/json", "{\"ok\":false,\"error\":\"fs_not_mounted\"}");
    return;
  }

  uint32_t total = 0;
  uint32_t used = 0;
  switch (backend) {
#if __has_include(<LittleFS.h>)
    case FsBackend::Little:
      total = static_cast<uint32_t>(LittleFS.totalBytes());
      used = static_cast<uint32_t>(LittleFS.usedBytes());
      break;
#endif
#if __has_include(<SPIFFS.h>)
    case FsBackend::Spiffs:
      total = static_cast<uint32_t>(SPIFFS.totalBytes());
      used = static_cast<uint32_t>(SPIFFS.usedBytes());
      break;
#endif
    default:
      request->send(500, "application/json", "{\"ok\":false,\"error\":\"fs_not_mounted\"}");
      return;
  }

  const uint32_t free = total > used ? (total - used) : 0;

  String out = "{";
  out += "\"ok\":true,";
  out += "\"backend\":\"";
  out += backend_name(backend);
  out += "\",";
  out += "\"total\":";
  out += String(total);
  out += ",\"used\":";
  out += String(used);
  out += ",\"free\":";
  out += String(free);
  out += "}";

  request->send(200, "application/json", out);
}

void handle_fs_list(AsyncWebServerRequest* request) {
  fs::FS* fs = nullptr;
  if (!with_active_fs(fs) || !fs) {
    request->send(500, "application/json", "[]");
    return;
  }

  File root = fs->open("/");
  if (!root) {
    request->send(500, "application/json", "[]");
    return;
  }

  String out = "[";
  bool first = true;
  list_recursive_json(root, out, first);
  out += "]";

  request->send(200, "application/json", out);
}

void handle_fs_read(AsyncWebServerRequest* request) {
  fs::FS* fs = nullptr;
  if (!with_active_fs(fs) || !fs) {
    request->send(500, "text/plain", "fs_not_mounted");
    return;
  }

  const String path = normalize_path(request_path_param(request));
  if (path == "/") {
    request->send(400, "text/plain", "missing_or_invalid_path");
    return;
  }

  if (!fs->exists(path)) {
    request->send(404, "text/plain", "not_found");
    return;
  }

  File f = fs->open(path, "r");
  if (!f) {
    request->send(500, "text/plain", "open_failed");
    return;
  }
  if (f.isDirectory()) {
    f.close();
    request->send(400, "text/plain", "path_is_directory");
    return;
  }
  f.close();

  request->send(*fs, path, "application/octet-stream");
}

void handle_fs_mkdir(AsyncWebServerRequest* request) {
  fs::FS* fs = nullptr;
  if (!with_active_fs(fs) || !fs) {
    request->send(500, "text/plain", "fs_not_mounted");
    return;
  }

  const String path = normalize_path(request_path_param(request));
  if (path == "/") {
    request->send(400, "text/plain", "missing_or_invalid_path");
    return;
  }
  if (fs->exists(path) || fs->mkdir(path)) {
    request->send(200, "text/plain", "OK");
    return;
  }
  request->send(500, "text/plain", "mkdir_failed");
}

void handle_fs_rmdir(AsyncWebServerRequest* request) {
  fs::FS* fs = nullptr;
  if (!with_active_fs(fs) || !fs) {
    request->send(500, "text/plain", "fs_not_mounted");
    return;
  }

  const String path = normalize_path(request_path_param(request));
  if (path == "/") {
    request->send(400, "text/plain", "missing_or_invalid_path");
    return;
  }
  if (!fs->exists(path)) {
    request->send(404, "text/plain", "not_found");
    return;
  }
  if (remove_recursive(*fs, path)) {
    request->send(200, "text/plain", "OK");
    return;
  }
  request->send(500, "text/plain", "rmdir_failed");
}

void handle_fs_delete(AsyncWebServerRequest* request) {
  fs::FS* fs = nullptr;
  if (!with_active_fs(fs) || !fs) {
    request->send(500, "text/plain", "fs_not_mounted");
    return;
  }

  const String path = normalize_path(request_path_param(request));
  if (path == "/") {
    request->send(400, "text/plain", "missing_or_invalid_path");
    return;
  }
  if (!fs->exists(path)) {
    request->send(404, "text/plain", "not_found");
    return;
  }
  if (remove_recursive(*fs, path)) {
    request->send(200, "text/plain", "OK");
    return;
  }
  request->send(500, "text/plain", "delete_failed");
}

void handle_reboot(AsyncWebServerRequest* request) {
  request->send(200, "application/json", "{\"ok\":true,\"reboot\":true}");
  delay(100);
  ESP.restart();
}

} // namespace

void register_routes(AsyncWebServer& server) {
  // Isolated WiFi filesystem API layer for external manager compatibility.
  server.on("/api/fs/ping", HTTP_GET, [](AsyncWebServerRequest* request) {
    handle_fs_ping(request);
  });

  server.on("/api/fs/list", HTTP_GET, [](AsyncWebServerRequest* request) {
    handle_fs_list(request);
  });

  server.on("/api/fs/info", HTTP_GET, [](AsyncWebServerRequest* request) {
    handle_fs_info(request);
  });

  server.on("/api/fs/read", HTTP_GET, [](AsyncWebServerRequest* request) {
    handle_fs_read(request);
  });

  server.on("/api/fs/mkdir", HTTP_POST, [](AsyncWebServerRequest* request) {
    handle_fs_mkdir(request);
  });

  server.on("/api/fs/rmdir", HTTP_POST, [](AsyncWebServerRequest* request) {
    handle_fs_rmdir(request);
  });

  server.on("/api/fs/delete", HTTP_POST, [](AsyncWebServerRequest* request) {
    handle_fs_delete(request);
  });

  server.on("/api/reboot", HTTP_POST, [](AsyncWebServerRequest* request) {
    handle_reboot(request);
  });
}

bool handle_manager_upload_chunk(
  AsyncWebServerRequest* request,
  const String& filename,
  size_t index,
  uint8_t* data,
  size_t len,
  bool final
) {
  (void)filename;
  if (!is_manager_upload_request(request)) {
    return false;
  }

  UploadSession& session = get_session(request);
  session.active = true;

  fs::FS* fs = nullptr;
  if (!with_active_fs(fs) || !fs) {
    session.hadError = true;
    session.error = "fs_not_mounted";
    return true;
  }

  if (index == 0) {
    if (!session.disconnectHooked) {
      session.disconnectHooked = true;
      request->onDisconnect([request]() {
        clear_session(request);
      });
    }

    session.hadError = false;
    session.error = "";
    session.targetPath = normalize_path(request_path_param(request));

    if (session.targetPath == "/") {
      session.hadError = true;
      session.error = "invalid_target_path";
      return true;
    }

    ensure_parent_dirs(*fs, session.targetPath);
    delay(0);

    request->_tempFile = fs->open(session.targetPath, "w");
    if (!request->_tempFile) {
      session.hadError = true;
      session.error = "open_failed";
      return true;
    }
  }

  if (len && request->_tempFile && !session.hadError) {
    if (request->_tempFile.write(data, len) != len) {
      session.hadError = true;
      session.error = "write_failed";
    }
    delay(0);
  }

  if (final) {
    if (request->_tempFile) {
      request->_tempFile.close();
      delay(0);
    }
    if (session.hadError && session.targetPath.length()) {
      fs->remove(session.targetPath);
      delay(0);
    }
  }

  return true;
}

bool handle_manager_upload_post(AsyncWebServerRequest* request) {
  if (!is_manager_upload_request(request)) {
    return false;
  }

  UploadSession& session = get_session(request);
  if (session.hadError) {
    request->send(500, "text/plain", session.error.length() ? session.error : "upload_failed");
  } else {
    request->send(200, "text/plain", "OK");
  }

  clear_session(request);
  return true;
}

} // namespace fs_api_http
