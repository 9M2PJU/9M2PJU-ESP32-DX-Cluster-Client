#include "WebAdmin.h"
#include <WiFi.h>

// DNS captive portal: redirect all domains to our AP IP (192.168.4.1)
static const IPAddress AP_IP(192, 168, 4, 1);
static const IPAddress AP_MASK(255, 255, 255, 0);

// =============================================================================
// Embedded HTML page (stored in PROGMEM)
// =============================================================================
const char WebAdmin::PAGE_HTML[] PROGMEM = R"HTML(<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>9M2PJU DX Cluster Setup</title>
<style>
*{box-sizing:border-box;margin:0;padding:0}
body{font-family:system-ui,-apple-system,sans-serif;background:#0a0e1a;color:#e0e0e0;min-height:100vh;padding:12px}
.card{max-width:480px;margin:0 auto;background:#111827;border:1px solid #1e293b;border-radius:12px;padding:20px}
h1{font-size:1.3rem;text-align:center;margin-bottom:4px;color:#07b6d3}
.subtitle{text-align:center;color:#64748b;font-size:.8rem;margin-bottom:18px}
fieldset{border:1px solid #1e293b;border-radius:8px;padding:14px;margin-bottom:14px}
legend{font-size:.85rem;color:#07b6d3;padding:0 6px}
label{display:block;font-size:.8rem;color:#94a3b8;margin-bottom:4px;margin-top:10px}
label:first-child{margin-top:0}
input,select{width:100%;padding:10px 12px;border:1px solid #334155;border-radius:6px;background:#1e293b;color:#e0e0e0;font-size:.9rem;outline:none}
input:focus,select:focus{border-color:#07b6d3}
.hint{font-size:.7rem;color:#64748b;margin-top:3px}
.btn{width:100%;padding:12px;border:none;border-radius:8px;background:#07b6d3;color:#000;font-size:1rem;font-weight:600;cursor:pointer;margin-top:16px}
.btn:active{background:#06a0b8}
.scan-btn{width:auto;padding:6px 14px;font-size:.75rem;margin-top:6px;background:#334155;color:#e0e0e0}
.scan-btn:active{background:#475569}
.adv-toggle{font-size:.75rem;color:#64748b;cursor:pointer;text-decoration:underline;margin-top:8px}
.adv{display:none}
#scanResult{margin-top:8px}
#scanResult select{margin-top:4px}
.msg{padding:10px;border-radius:6px;font-size:.85rem;margin-top:12px;text-align:center}
.msg.ok{background:#064e3b;color:#34d399}
.msg.err{background:#7f1d1d;color:#fca5a5}
.status{font-size:.7rem;color:#64748b;text-align:center;margin-top:12px}
</style>
</head>
<body>
<div class="card">
<h1>9M2PJU DX Cluster</h1>
<p class="subtitle">ESP32 DX Cluster Client &mdash; Setup</p>
<form id="cfg" action="/save" method="POST">
<fieldset>
<legend>Wi-Fi</legend>
<label>Network (SSID)</label>
<div id="scanResult"></div>
<input type="text" id="ssid" name="ssid" placeholder="Your Wi-Fi name" required>
<button type="button" class="btn scan-btn" onclick="doScan()">Scan for networks</button>
<label>Password</label>
<input type="password" name="wifipass" placeholder="Your Wi-Fi password">
</fieldset>
<fieldset>
<legend>DX Cluster Login</legend>
<label>Callsign</label>
<input type="text" name="callsign" placeholder="e.g. N0CALL" required>
<div class="hint">Your amateur radio callsign</div>
<label>Callsign Password (optional)</label>
<input type="password" name="callpass" placeholder="Only if the cluster requires one">
<div class="hint">Sent when the cluster asks "Password:" &mdash; leave blank if unsure</div>
</fieldset>
<fieldset>
<legend>DX Cluster Server</legend>
<label>Host / Address</label>
<input type="text" name="host" placeholder="e.g. 9m2pju.hamradio.my" required>
<label>Port</label>
<input type="number" name="port" value="7300" min="1" max="65535" required>
</fieldset>
<p class="adv-toggle" onclick="var e=document.getElementById('adv');e.style.display=e.style.display=='block'?'none':'block'">Advanced settings &darr;</p>
<fieldset id="adv" class="adv">
<legend>Advanced</legend>
<label>Post-login command (optional)</label>
<input type="text" name="postlogin" placeholder="e.g. set/dx">
<div class="hint">Sent once after login. Leave blank if unsure.</div>
</fieldset>
<button type="submit" class="btn">Save &amp; Reboot</button>
</form>
<div id="msg"></div>
<p class="status">After saving, the device reboots and connects to your Wi-Fi.</p>
</div>
<script>
function doScan(){
var r=document.getElementById('scanResult');
r.innerHTML='<div class="hint">Scanning...</div>';
fetch('/scan').then(function(r){return r.json()}).then(function(d){
if(!d||!d.length){r.innerHTML='<div class="hint">No networks found</div>';return}
var h='<label>Select network</label><select onchange="document.getElementById(\'ssid\').value=this.value"><option value="">-- choose --</option>';
d.forEach(function(n){h+='<option value="'+n.s+'">'+n.s+(n.r!=''?' ('+n.r+' dBm)':'')+'</option>'});
h+='</select>';
r.innerHTML=h;
}).catch(function(){r.innerHTML='<div class="hint">Scan failed</div>'});
}
document.getElementById('cfg').addEventListener('submit',function(e){
e.preventDefault();
var fd=new FormData(this);
var p=new URLSearchParams();
for(var k of fd.keys())p.append(k,fd.get(k));
var msg=document.getElementById('msg');
msg.className='msg';
msg.textContent='Saving...';
fetch('/save',{method:'POST',body:p}).then(function(r){return r.text()}).then(function(t){
msg.className='msg ok';
msg.textContent='Saved! Rebooting... Please wait and reconnect to your Wi-Fi.';
}).catch(function(){
msg.className='msg err';
msg.textContent='Save failed. Please try again.';
});
});
</script>
</body>
</html>)HTML";

// =============================================================================
// Implementation
// =============================================================================

void WebAdmin::begin(AppConfig &config, const String &apName) {
  _config = &config;
  _apName = apName;
  _reboot = false;
  _running = true;

  // Start Wi-Fi AP
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(AP_IP, AP_IP, AP_MASK);
  WiFi.softAP(apName.c_str());
  Serial.print(F("AP started: "));
  Serial.println(apName);
  Serial.print(F("AP IP: "));
  Serial.println(WiFi.softAPIP());

  // DNS captive portal: resolve all queries to our IP
  _dns.start(53, "*", AP_IP);

  // HTTP server routes
  _server.on("/", HTTP_GET, [this]() { handleRoot(); });
  _server.on("/save", HTTP_POST, [this]() { handleSave(); });
  _server.on("/scan", HTTP_GET, [this]() { handleScan(); });
  // Captive portal catch-all redirects
  _server.on("/generate_204", HTTP_GET, [this]() { handleRedirect(); });
  _server.on("/gen_204", HTTP_GET, [this]() { handleRedirect(); });
  _server.on("/hotspot-detect.html", HTTP_GET, [this]() { handleRedirect(); });
  _server.on("/connecttest.txt", HTTP_GET, [this]() { handleRedirect(); });
  _server.on("/fwlink", HTTP_GET, [this]() { handleRedirect(); });
  _server.onNotFound([this]() { handleNotFound(); });

  _server.begin();
  Serial.println(F("Web admin server started"));
}

void WebAdmin::loop() {
  if (!_running) return;
  _dns.processNextRequest();
  _server.handleClient();
}

bool WebAdmin::shouldReboot() const {
  return _reboot;
}

void WebAdmin::stop() {
  if (!_running) return;
  _server.stop();
  _dns.stop();
  WiFi.softAPdisconnect(true);
  _running = false;
}

void WebAdmin::handleRoot() {
  _server.sendHeader("Cache-Control", "no-store, no-cache, must-revalidate");
  _server.send_P(200, "text/html", PAGE_HTML);
}

void WebAdmin::handleRedirect() {
  _server.sendHeader("Location", "http://192.168.4.1/", true);
  _server.send(302, "text/plain", "");
}

void WebAdmin::handleNotFound() {
  // Redirect everything to the config page (captive portal behaviour)
  _server.sendHeader("Location", "http://192.168.4.1/", true);
  _server.send(302, "text/plain", "");
}

void WebAdmin::handleScan() {
  Serial.println(F("WiFi scan requested"));
  int n = WiFi.scanNetworks();
  String json = "[";
  for (int i = 0; i < n; ++i) {
    if (i > 0) json += ",";
    String ssid = WiFi.SSID(i);
    int rssi = WiFi.RSSI(i);
    // Escape quotes in SSID
    ssid.replace("\"", "\\\"");
    json += "{\"s\":\"" + ssid + "\",\"r\":\"" + String(rssi) + "\"}";
  }
  json += "]";
  WiFi.scanDelete();
  _server.sendHeader("Cache-Control", "no-store");
  _server.send(200, "application/json", json);
}

void WebAdmin::handleSave() {
  if (!_config) {
    _server.send(500, "text/plain", "Internal error");
    return;
  }

  _config->wifiSsid = _server.arg("ssid");
  _config->wifiPassword = _server.arg("wifipass");
  _config->callsign = _server.arg("callsign");
  _config->callsignPassword = _server.arg("callpass");
  _config->clusterHost = _server.arg("host");
  _config->clusterPort = (uint16_t)_server.arg("port").toInt();
  _config->postLoginCommand = _server.arg("postlogin");

  // Apply defaults for empty fields
  if (_config->clusterPort == 0) _config->clusterPort = 7300;
  if (_config->clusterHost.isEmpty()) _config->clusterHost = "9m2pju.hamradio.my";

  _config->save();
  Serial.println(F("Config saved to NVS"));

  _server.send(200, "text/plain", "OK");
  _reboot = true;
}
