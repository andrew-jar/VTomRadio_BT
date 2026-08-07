#include "pluginsManager.h"

pluginsManager pm;

Plugin::Plugin() {
}

void pluginsManager::init() {
  const size_t pendingCount = pending.size();
  plugins.reserve(plugins.size() + pendingCount + 8);
  // pending plugin-ek átemelése
  for (auto* p : pending) {
    plugins.push_back(p);
  }
  pending.clear();
  _ready = true;
  Serial.printf("PM.init(): moved=%u, total=%u\n", (unsigned)pendingCount, (unsigned)plugins.size());
}

bool pluginsManager::ready() const {
  return _ready;
}

void Plugin::registerPlugin() {
  pm.add(this);
}

void pluginsManager::add(Plugin* plugin) {
  if (!plugin) return;
  if (!_ready) {
    pending.push_back(plugin);   // <<< setup előtti regisztráció
    return;
  }
  // Runtime add can race with call_event() iteration over plugins.
  // Keep manager immutable after init unless a synchronized path is introduced.
  Serial.println("PM.add(): ignored after init (runtime add disabled)");
}

size_t pluginsManager::count() const {
  return plugins.size();
}

Plugin* pluginsManager::get(size_t index) {
  if (index < plugins.size()) {
    return plugins[index];
  }
  return nullptr;
}

