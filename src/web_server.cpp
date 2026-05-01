#include "web_server.h"
#include "runtime_config.h"
#include <WebServer.h>
#include <WiFi.h>

static WebServer      s_srv(80);
static RuntimeConfig *s_cfg   = nullptr;
static AlarmManager  *s_alarm = nullptr;

// Stored in flash (.rodata) — no SRAM until request
static const char PAGE[] =
"<!DOCTYPE html><html><head>"
"<meta charset=UTF-8><meta name=viewport content='width=device-width'>"
"<title>Budik</title><style>"
"body{font-family:sans-serif;max-width:480px;margin:auto;padding:1em;"
"background:#0d1b2a;color:#e0e8f0}"
"h2{color:#f0a500}"
"label{display:block;margin:.6em 0 .2em;font-size:.9em;color:#8ab4d8}"
"input[type=text],input[type=number]{width:100%;box-sizing:border-box;"
"padding:.4em;background:#1e3a5f;color:#fff;border:1px solid #405070;"
"border-radius:4px}"
".row{display:flex;gap:.5em}.row input{flex:1}"
"input[type=checkbox]{width:auto;margin-right:.4em}"
"button{background:#f0a500;color:#0d1b2a;border:none;padding:.6em 1.4em;"
"border-radius:6px;font-weight:bold;cursor:pointer;margin-top:.8em}"
"hr{border-color:#1e3a5f}.ok{color:#27ae60;font-weight:bold}"
"</style></head><body>"
"<h2>&#128276; Nastaveni budiku</h2>"
"<form method=POST action=/save>"
"<label>NTP server</label>"
"<input type=text name=ntp value='%NTP%' maxlength=63>"
"<label>Kalendar ICS URL</label>"
"<input type=text name=cal value='%CAL%' maxlength=255>"
"<hr><label>Budik</label>"
"<div class=row>"
"<input type=number name=alh min=0 max=23 value=%ALH% placeholder=Hod>"
"<input type=number name=alm min=0 max=59 value=%ALM% placeholder=Min>"
"</div>"
"<label><input type=checkbox name=ale %ALE%>Budik zapnut</label>"
"<button type=submit>Ulozit</button>"
"</form>%MSG%</body></html>";

static String build_page(const char *msg) {
    AlarmConfig ac = s_alarm ? s_alarm->get() : AlarmConfig{};
    String p(PAGE);
    p.replace("%NTP%", s_cfg ? s_cfg->ntp_server   : "");
    p.replace("%CAL%", s_cfg ? s_cfg->calendar_url : "");
    p.replace("%ALH%", String(ac.hour));
    p.replace("%ALM%", String(ac.minute));
    p.replace("%ALE%", ac.enabled ? "checked" : "");
    p.replace("%MSG%", msg);
    return p;
}

static void handle_root() {
    s_srv.send(200, "text/html", build_page(""));
}

static void handle_save() {
    if (s_cfg) {
        if (s_srv.hasArg("ntp"))
            strlcpy(s_cfg->ntp_server,   s_srv.arg("ntp").c_str(), RT_NTP_LEN);
        if (s_srv.hasArg("cal"))
            strlcpy(s_cfg->calendar_url, s_srv.arg("cal").c_str(), RT_CAL_LEN);
        runtime_config_save(*s_cfg);
    }
    if (s_alarm && s_srv.hasArg("alh") && s_srv.hasArg("alm")) {
        s_alarm->set((uint8_t)s_srv.arg("alh").toInt(),
                     (uint8_t)s_srv.arg("alm").toInt(),
                     s_srv.hasArg("ale"));
        s_alarm->save();
    }
    s_srv.send(200, "text/html",
        build_page("<p class=ok>&#10003; Ulozeno!</p>"));
}

void web_server_begin(RuntimeConfig *cfg, AlarmManager *alarm) {
    s_cfg   = cfg;
    s_alarm = alarm;
    s_srv.on("/",     HTTP_GET,  handle_root);
    s_srv.on("/save", HTTP_POST, handle_save);
    s_srv.onNotFound([](){ s_srv.send(404,"text/plain","Not found"); });
    s_srv.begin();
    Serial.printf("[Web] http://%s\n", WiFi.localIP().toString().c_str());
}

void web_server_handle() { s_srv.handleClient(); }
