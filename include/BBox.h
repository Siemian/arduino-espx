#pragma once

class BBox {
public:
    String label;
    float score;
    uint16_t x;
    uint16_t y;
    uint16_t w;
    uint16_t h;
    uint16_t cx;
    uint16_t cy;

    BBox(String label, float score, uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
        this->label = label;
        this->score = score;
        this->x = x;
        this->y = y;
        this->w = w;
        this->h = h;
        this->cx = x + w / 2;
        this->cy = y + h / 2;
    }
};