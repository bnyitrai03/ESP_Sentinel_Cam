#include "camera.h"
#include "camera_app.h"
#include "config.h"
#include "error_handler.h"
#include "mqtt.h"
#include "mytime.h"
#include "storage.h"
#include "wifi.h"

void camera_app();

class CameraApp {
public:
  CameraApp();

private:
  Camera _cam;
  Wifi _wifi;
  Config _config;
  MQTT _mqtt;
};