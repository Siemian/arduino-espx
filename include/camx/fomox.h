#pragma once

#ifndef __JPEGDEC__
#error "You *must* include the JPEGDEC library"
#endif

#ifndef EI_CLASSIFIER_OBJECT_DETECTION
#error "You *must* include an Edge Impulse library for object detection"
#endif

#if EI_CLASSIFIER_INPUT_WIDTH != EI_CLASSIFIER_INPUT_HEIGHT
#error "Only models trained on squared images are supported"
#endif

// detect grayscale vs RGB mode
#define EI_BPP ((EI_CLASSIFIER_NN_INPUT_FRAME_SIZE == EI_CLASSIFIER_RAW_SAMPLE_COUNT) ? 1 : 2)

#include <vector>
#include "./jdecoder.h"
#include "../BBox.h"

using ei::signal_t;


namespace espx::camx {
    /**
     * Edge Impulse' FOMO runner
     */
    class Fomox : public Jdecoder {
    public:
        uint8_t count;
        std::vector<BBox> objects;
        signal_t signal;
        ei_impulse_result_t result;
        uint8_t pixels[EI_CLASSIFIER_INPUT_WIDTH * EI_CLASSIFIER_INPUT_WIDTH * EI_BPP];

        /**
         *
         */
        Fomox() : count(0), minConfidence(0.5) {
            signal.total_length = EI_CLASSIFIER_RAW_SAMPLE_COUNT;
            signal.get_data = [this](size_t offset, size_t length, float *out) {
                return feedDataToModel(offset, length, out);
            };
            debug(false);
        }

        /**
         *
         * @param enabled
         */
        void debug(bool enabled = true) {
            debugEnabled = enabled;
        }

        /**
         * Set min confidence threshold
         * @param confidence
         */
        void moreConfidentThan(float confidence) {
            minConfidence = confidence;
        }

        /**
         *
         * @tparam Frame
         * @param frame
         * @return
         */
        template<class Frame>
        OpStatus& process(Frame& frame) {
            return process(frame.buf, frame.length);
        }

        /**
         *
         * @param buf
         * @param len
         * @return
         */
        OpStatus& process(uint8_t *buf, size_t len) {
            decode(buf, len);

            if (status.failed())
                return status;

            EI_IMPULSE_ERROR error = run_classifier(&signal, &result, debugEnabled);
            stopwatch.stop();

            if (error != EI_IMPULSE_OK)
                status.failWithCode("run_classifier returned error", error);
            else fillObjects();

            return status;
        }

    protected:
        bool debugEnabled;
        float minConfidence;
        struct {
            uint16_t dx;
        } projection;

        /**
         * Implement custom JPEGDEC.openFLASH call
         * @param buf
         * @param length
         * @return
         */
        virtual bool openFlash(uint8_t *buf, size_t length) {
            return jpegdec.openFLASH(buf, length, [](JPEGDRAW *mcu) -> int {
                Fomox *self = (Fomox*) mcu->pUser;

                return self->handleMCU(mcu);
            });
        }

        /**
         *
         */
        void onOpenSuccess() {
            // compute scale + shift
            // assume w > h
            const uint16_t w = dimensions.width;
            const uint16_t h = dimensions.height;
            const uint8_t scale = detectScale(h, EI_CLASSIFIER_INPUT_WIDTH);
            const float stride = cast<float>(h) / scale / EI_CLASSIFIER_INPUT_WIDTH;

            projection.dx = (w - h) / 2;
            jconfig.scale = scale;
            jconfig.stride = stride;
            jpegdec.setCropArea(projection.dx, 0, h, h);
            jpegdec.setMaxOutputSize(w / 8);
        }

        void onDecodeSuccess() {
            // don't stop stopwatch
        }

#if EI_BPP == 1
        /**
         *
         * @param y
         * @param x
         * @param pixel
         * @return
         */
        bool handleRowGray(uint16_t y, uint16_t width, const uint8_t *rowPixels) {
            return true;
        }
#else
        /**
         * Model wants color images, but we're capturing in grayscale mode
         * @param y
         * @param x
         * @param pixel
         * @return
         */
        bool handleRowGray(uint16_t y, uint16_t width, const uint8_t *rowPixels) {
            status.fail("Fomo model expects color images, but you're capturing in grayscale");

            // return true to skip further processing
            return true;
        }

        /**
         *
         * @param y
         * @param width
         * @param rowPixels
         * @return
         */
        bool handleRowColor(uint16_t y, uint16_t width, const uint16_t *rowPixels) {
            const size_t offset = y * EI_CLASSIFIER_INPUT_WIDTH;
            const uint16_t *pixelsWithOffset = ((uint16_t*) pixels) + offset;

            for (float x = 0, j = 0; j < EI_CLASSIFIER_INPUT_WIDTH; x += jconfig.stride, j++) {
                const uint16_t *dst = pixelsWithOffset + cast<uint16_t>(j);
                const uint16_t *src = rowPixels + cast<uint16_t>(x);

                memcpy((uint8_t *) dst, (uint8_t*) src, 2);
            }

            // return true to skip further processing
            return true;
        }

        /**
         * Convert pixels to RGB24
         * @param offset
         * @param length
         * @param out
         * @return
         */
        int feedDataToModel(size_t offset, size_t length, float *out) {
            uint16_t *rgbPixels = (uint16_t*) pixels;

            for (uint16_t i = offset; i < offset + length; i++) {
                const uint16_t pixel = rgbPixels[i];
                uint32_t r = (pixel >> 11) & 0x1F;
                uint32_t g = (pixel >> 5)  & 0x3F;
                uint32_t b = pixel & 0x1F;

                // scale to 8 bits (0–255)
                r = (r << 3) | (r >> 2);
                g = (g << 2) | (g >> 4);
                b = (b << 3) | (b >> 2);

                (*out++) = (r << 16) | (g << 8) | b;
            }

            return 0;
        }
#endif

        /**
         * Detect greatest scale factor from orig to dest
         * @param orig
         * @param dest
         * @return
         */
        uint8_t detectScale(uint16_t orig, uint16_t dest) {
            uint8_t scale = 8;

            while (scale > 1) {
                const uint16_t proj = orig / scale;

                if (proj >= dest)
                    return scale;

                scale >>= 1;
            }

            return 1;
        }

        /**
         * Copy bboxes to objects iterable
         */
        void fillObjects() {
            count = 0;
            objects.clear();

            for (size_t ix = 0, i = 0; ix < result.bounding_boxes_count; ix++) {
                auto bb = result.bounding_boxes[ix];

                if (bb.value > minConfidence) {
                    const float scale = jconfig.scale * jconfig.stride;

                    BBox bbox(bb.label, bb.value, (projection.dx + bb.x) * scale, bb.y * scale, bb.width * scale, bb.height * scale);
                    objects.push_back(bbox);
                    count++;
                }
            }

            status.succeed();
        }
    };
}

// singleton
static espx::camx::Fomox fomox;