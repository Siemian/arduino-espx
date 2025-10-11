/**
 * Perform object detection on camera frames
 * using Edge Impulse's FOMO model
 */
#include <JPEGDEC.h>
#include <tinyml4all-object-detection_inferencing.h>
#include <espx.h>
#include <espx/camx.h>
#include <espx/camx/fomox.h>


void setup() {
    delay(1000);
    Serial.begin(115200);
    Serial.println("Fomox example: object detection");

    // configure camx through Serial Monitor
    camx.model.prompt();
    camx.pixformat.jpeg();
    camx.quality.high();
    camx.resolution.qvga();

    // initialize camx,
    // enter endless loop on error
    camx.begin().raise();

    // fomox doesn't need initialization
    // but you can set the minimum
    // confidence threshold for objects to be included
    // in the results (from 0 to 1)
    fomox.moreConfidentThan(0.6);
}


void loop() {
    auto frame = camx.grab();

    if (!fomox.process(frame)) {
        Serial.print("Failure: ");
        Serial.println(fomox.failure());
        return;
    }

    Serial.printf("Found %d objects in %dms\n", fomox.count, fomox.stopwatch.millis());

    // loop over objects
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