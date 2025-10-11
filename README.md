# ESPx

A collection of utilities for the ESP32 in the Arduino environment.

### Examples

#### Take picture from camera

```c++
#include <espx.h>
#include <espx/camx.h>

void setup() {
    // configure camx through Serial Monitor
    camx.model.prompt();
    camx.pixformat.prompt();
    camx.quality.prompt();
    camx.resolution.prompt();
    camx.begin().raise();
}

void loop() {
    auto frame = camx.grab();

    Serial.printf("Frame size: %d bytes\n", frame.length);
}
```

#### Motion detection from camera frames

```c++
#include <JPEGDEC.h>
#include <espx.h>
#include <espx/camx.h>
#include <espx/camx/motionx.h>

void setup() {
    // configure camx through Serial Monitor
    camx.model.prompt();
    camx.pixformat.jpeg();
    camx.quality.high();
    camx.resolution.vga();
    camx.begin().raise();
    
    // configure motion detection
    motionx.smooth(0.8);
    motionx.trainFor(10);
    motionx.diffBy(10);
    motionx.threshold("30%");
    motionx.throttle("3s");
}

void loop() {
    auto frame = camx.grab();
    motionx.process(frame);

    // motionx is true if ratio >= threshold
    if (motionx)
      Serial.println("Motion detected");
}
```

#### Object detection using Edge Impulse

```c++
#include <JPEGDEC.h>
#include <object-detection_inferencing.h>
#include <espx.h>
#include <espx/camx.h>
#include <espx/camx/fomox.h>

void setup() {
    // configure camx through Serial Monitor
    camx.model.prompt();
    camx.pixformat.jpeg();
    camx.quality.high();
    camx.resolution.qvga();
    camx.begin().raise();

    fomox.moreConfidentThan(0.6);
}

void loop() {
    auto frame = camx.grab();
    fomox.process(frame);
    Serial.printf("Found %d objects in %dms\n", fomox.count, fomox.stopwatch.millis());

    for (auto object : fomox.objects) {
        Serial.printf(
            "Found object of class %s at coordinates (%d, %d) with confidence %.2f\n",
            object.label,
            object.cx,
            object.cy,
            object.score
        );
    }
}
```

#### Connect to WiFi

```c++
#include <espx.h>
#include <espx/wifix.h>

void setup() {
    wifix("SSID", "PASSWORD");

    if (wifix) {
        // we are connected to WiFi, print IP address
        Serial.println(wifix.ip);
    }
}
```

#### Create threads

```c++
#include <espx.h>
#include <espx/threadx.h>

void setup() {
  // create an "anonymous" thread with default config
  threadx([](void *) {
      while (true) {
          Serial.println("Inside anonymous thread");
          delay(1000);
      }
  });

  // you can also create a "detached" function that runs once
  // in background and exits (no while loop needed!)
  threadx([](void *) {
      Serial.println("Run and exit!");
  });
}
```

#### Make HTTP request

```c++
#include <espx.h>
#include <espx/wifix.h>
#include <espx/httpx.h>

void setup() {
    wifix("SSID", "PASSWORD").raise();
}

void loop() {
    auto response = httpx.run(
        "https://icanhazdadjoke.com/",
        httpx.Insecure(),
        httpx.Header("Accept", "text/plain"),
        httpx.Body("GET")
    );

    Serial.printf("Response [%d]\n", response.code);
    Serial.println(response.text());
    httpx.end();
}
```