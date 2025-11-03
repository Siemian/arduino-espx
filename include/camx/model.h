#pragma once

typedef struct {
    String name;
    const char alias[12];
    int8_t d0;
    int8_t d1;
    int8_t d2;
    int8_t d3;
    int8_t d4;
    int8_t d5;
    int8_t d6;
    int8_t d7;
    int8_t xclk;
    int8_t pclk;
    int8_t vsync;
    int8_t href;
    int8_t sccb_sda;
    int8_t sccb_scl;
    int8_t pwdn;
    int8_t reset;
} CamxModel;

const uint8_t CAMX_MODELS_COUNT = 11;

const CamxModel CAMX_MODELS[CAMX_MODELS_COUNT] = {
    {
        "AiThinker",
        "aithinker",
        5, 18, 19, 21, 36, 39, 34, 35,
        0, 22, 25, 23, 26, 27, 32, -1
    },
    {
        "XIAO",
        "xiao",
        15, 17, 18, 16, 14, 12, 11, 48,
        10, 13, 38, 47, 40, 39, -1, -1
    },
    {
        "M5",
        "m5",
        32, 35, 34, 5, 39, 18, 36, 19,
        27, 21, 22, 26, 25, 23, -1, 15
    },
    {
        "M5 Fish Eye",
        "m5fisheye",
        32, 35, 34, 5, 39, 18, 36, 19,
        27, 21, 25, 26, 22, 23, -1, 15
    },
    {
        "M5 Timer X",
        "m5timerx",
        32, 35, 34, 5, 39, 18, 36, 19,
        27, 21, 22, 26, 25, 23, -1, 15
    },
    {
        "ESP - EYE",
        "espeye",
        34, 13, 14, 35, 39, 38, 37, 36,
        4, 25, 5, 27, 18, 23, -1, -1
    },
    {
        "ESP - EYE S3",
        "espeyes3",
        11, 9, 8, 10, 12, 18, 17, 16,
        15, 13, 6, 7, 4, 5, -1, -1
    },
    {
        "WROVER",
        "wrover",
        4, 5, 18, 19, 36, 39, 34, 35,
        21, 22, 25, 23, 26, 27, -1, -1
    },
    {
        "WROOM S3",
        "wrooms3",
        11, 9, 8, 10, 12, 18, 17, 16,
        15, 13, 6, 7, 4, 5, -1, -1
    },
    {
        "TTGO Plus",
        "ttgoplus",
        34, 13, 26, 35, 39, 38, 37, 36,
        4, 25, 5, 27, 18, 23, -1, -1
    },
    {
        "TTGO PIR",
        "ttgopir",
        5, 14, 4, 15, 18, 23, 36, 39,
        32, 19, 27, 25, 13, 12, 26, -1
    }
};

namespace espx::camx {
/**
 * Camera models pin definitions
 */
class Model {
public:
    CamxModel *pinout;

    /**
     * Constructor
     */
    Model() : pinout(NULL) {

    }

    /**
     * Set model from const char*
     * @param model
     */
    void set(const char *model) {
        String mod(model);

        set(mod);
    }

    /**
     * Set model from string
     * @param alias
     */
    void set(String& alias) {
        for (uint8_t i = 0; i < CAMX_MODELS_COUNT; i++) {
            const CamxModel *model = &(CAMX_MODELS[i]);

            if (alias == model->alias) {
                pinout = (CamxModel *) model;
                return;
            }
        }

        Serial.printf("Unknown model: %s", alias.c_str());
    }

    /**
     *
     */
    void prompt() {
        String choices[CAMX_MODELS_COUNT];

        for (uint8_t i = 0; i < CAMX_MODELS_COUNT; i++)
            choices[i] = (&(CAMX_MODELS[i]))->name;

        const int choice = promptChoice("Select a resolution", choices, CAMX_MODELS_COUNT);
        const String alias = (&(CAMX_MODELS[choice]))->alias;

        remember(alias.c_str());
    }

    inline void aithinker() { set("aithinker"); }

    inline void xiao() { set("xiao"); }

    inline void wrooms3() { set("wrooms3"); }

    inline void ttgoplus() { set("ttgoplus"); }

    inline void ttgopir() { set("ttgopir"); }

    inline void m5() { set("m5"); }

    inline void m5fisheye() { set("m5fisheye"); }

    inline void m5timerx() { set("m5timerx"); }

    inline void espeye() { set("espeye"); }

    inline void espeyes3() { set("espeyes3"); }

    inline void wrover() { set("wrover"); }

    protected:
        /**
         * Show the user a message to skip prompting in the future
         * @param alias
         */
        void remember(const char *alias) {
            Serial.printf("You can skip prompting next time with `camx.model.%s()`\n", alias);
            set(alias);
        }
    };
}

