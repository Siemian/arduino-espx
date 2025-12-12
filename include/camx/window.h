#pragma once
#include <esp_camera.h>

namespace espx::camx {
    /**
     * Configure camera window/ROI (Region of Interest)
     * Allows cropping and scaling at the sensor level
     */
    class Window {
    public:
        bool enabled;
        int offsetX;
        int offsetY;
        int totalX;
        int totalY;
        int outputX;
        int outputY;

        /**
         * Constructor
         */
        Window() : enabled(false), offsetX(0), offsetY(0), totalX(0), totalY(0), outputX(0), outputY(0) {
        }

        /**
         * Disable window/ROI (use full frame)
         */
        void disable() {
            enabled = false;
        }

        /**
         * Reset window on sensor to full frame
         * This explicitly clears window settings on the sensor by setting full frame dimensions
         * @param sensor Pointer to sensor_t from esp_camera_sensor_get()
         * @param fullWidth Full frame width (e.g., resolution width)
         * @param fullHeight Full frame height (e.g., resolution height)
         */
        void reset(sensor_t *sensor, int fullWidth, int fullHeight) {
            if (sensor == NULL || sensor->set_res_raw == NULL) {
                return;
            }

            // Set window to full frame dimensions to effectively disable it
            sensor->set_res_raw(
                sensor,
                0,      // startX (not used)
                0,      // startY (not used)
                0,      // endX (not used)
                0,      // endY (not used)
                0,      // offsetX = 0 (start from top-left)
                0,      // offsetY = 0
                fullWidth,  // totalX = full width
                fullHeight, // totalY = full height
                fullWidth,  // outputX = full width (no scaling)
                fullHeight, // outputY = full height (no scaling)
                false,  // scale (not used)
                false   // binning (not used)
            );
            enabled = false;
        }

        /**
         * Set window/ROI parameters
         * @param offsetX X offset from source frame (starting position)
         * @param offsetY Y offset from source frame (starting position)
         * @param totalX Window width (crop area width)
         * @param totalY Window height (crop area height)
         * @param outputX Output width (optional, 0 = use totalX, must maintain aspect ratio)
         * @param outputY Output height (optional, 0 = use totalY, must maintain aspect ratio)
         */
        void set(int offsetX, int offsetY, int totalX, int totalY, int outputX = 0, int outputY = 0) {
            this->enabled = true;
            this->offsetX = offsetX;
            this->offsetY = offsetY;
            this->totalX = totalX;
            this->totalY = totalY;
            
            // If output not specified, use total size
            if (outputX == 0 || outputY == 0) {
                this->outputX = totalX;
                this->outputY = totalY;
            } else {
                this->outputX = outputX;
                this->outputY = outputY;
            }
        }

        /**
         * Set window/ROI with just crop area (no scaling)
         * @param offsetX X offset from source frame
         * @param offsetY Y offset from source frame
         * @param width Crop area width
         * @param height Crop area height
         */
        void crop(int offsetX, int offsetY, int width, int height) {
            set(offsetX, offsetY, width, height);
        }

        /**
         * Set window/ROI with crop and scale
         * @param offsetX X offset from source frame
         * @param offsetY Y offset from source frame
         * @param cropWidth Crop area width
         * @param cropHeight Crop area height
         * @param outputWidth Output width (must maintain aspect ratio)
         * @param outputHeight Output height (must maintain aspect ratio)
         */
        void cropAndScale(int offsetX, int offsetY, int cropWidth, int cropHeight, int outputWidth, int outputHeight) {
            set(offsetX, offsetY, cropWidth, cropHeight, outputWidth, outputHeight);
        }

        /**
         * Apply window settings to sensor
         * This should be called after camera.begin() and sensor.begin()
         * @param sensor Pointer to sensor_t from esp_camera_sensor_get()
         */
        void apply(sensor_t *sensor) {
            if (!enabled || sensor == NULL || sensor->set_res_raw == NULL) {
                return;
            }

            // According to documentation:
            // - startX, startY, endX, endY are not used (set to 0)
            // - scale and binning are not used (set to false)
            // - Only offset, total, and output are meaningful
            sensor->set_res_raw(
                sensor,
                0,      // startX (not used)
                0,      // startY (not used)
                0,      // endX (not used)
                0,      // endY (not used)
                offsetX,
                offsetY,
                totalX,
                totalY,
                outputX,
                outputY,
                false,  // scale (not used)
                false   // binning (not used)
            );
        }

        /**
         * Round values to multiples of 4 for better compatibility
         * Some sensors require sizes to be multiples of 4
         */
        void roundToMultipleOf4() {
            offsetX = (offsetX / 4) * 4;
            offsetY = (offsetY / 4) * 4;
            totalX = (totalX / 4) * 4;
            totalY = (totalY / 4) * 4;
            outputX = (outputX / 4) * 4;
            outputY = (outputY / 4) * 4;
        }

        /**
         * Validate that output maintains aspect ratio of window
         * @return true if aspect ratios match (within tolerance)
         */
        bool validateAspectRatio(float tolerance = 0.01f) {
            if (totalX == 0 || totalY == 0 || outputX == 0 || outputY == 0) {
                return false;
            }

            float windowRatio = (float)totalX / (float)totalY;
            float outputRatio = (float)outputX / (float)outputY;
            float diff = (windowRatio > outputRatio) ? (windowRatio - outputRatio) : (outputRatio - windowRatio);

            return diff <= tolerance;
        }

        /**
         * Get current output width (or totalX if output not set)
         */
        int getOutputWidth() const {
            return outputX > 0 ? outputX : totalX;
        }

        /**
         * Get current output height (or totalY if output not set)
         */
        int getOutputHeight() const {
            return outputY > 0 ? outputY : totalY;
        }

        /**
         * Check if window is configured and valid
         */
        bool isValid() const {
            if (!enabled) return false;
            if (totalX <= 0 || totalY <= 0) return false;
            if (offsetX < 0 || offsetY < 0) return false;
            return true;
        }
    };
}

