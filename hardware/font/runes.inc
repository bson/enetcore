// -*- C -*-
#include "runes.h"
#include "font.h"

const unsigned char rune_map_5x8[128] = {
    0x80|RUNE_5x8_BLANK, 0x80|RUNE_5x8_BLANK, 0x80|RUNE_5x8_BLANK, 
    0x80|RUNE_5x8_BLANK, 0x80|RUNE_5x8_BLANK, 0x80|RUNE_5x8_BLANK, 
    0x80|RUNE_5x8_BLANK, 0x80|RUNE_5x8_BLANK, 0x80|RUNE_5x8_BLANK, 
    0x80|RUNE_5x8_BLANK, 0x80|RUNE_5x8_BLANK, 0x80|RUNE_5x8_BLANK, 
    0x80|RUNE_5x8_BLANK, 0x80|RUNE_5x8_BLANK, 0x80|RUNE_5x8_BLANK, 
    0x80|RUNE_5x8_BLANK, 0x80|RUNE_5x8_BLANK, 0x80|RUNE_5x8_BLANK, 
    0x80|RUNE_5x8_BLANK, 0x80|RUNE_5x8_BLANK, 0x80|RUNE_5x8_BLANK, 
    0x80|RUNE_5x8_BLANK, 0x80|RUNE_5x8_BLANK, 0x80|RUNE_5x8_BLANK, 
    0x80|RUNE_5x8_BLANK, 0x80|RUNE_5x8_BLANK, 0x80|RUNE_5x8_BLANK, 
    0x80|RUNE_5x8_BLANK, 0x80|RUNE_5x8_BLANK, 0x80|RUNE_5x8_BLANK, 
    0x80|RUNE_5x8_BLANK, 0x80|RUNE_5x8_BLANK, 0x80|RUNE_5x8_BLANK, 
    0x80|RUNE_5x8_EX, RUNE_5x8_DQUOTE, RUNE_5x8_HASH, RUNE_5x8_DOLLAR, 
    RUNE_5x8_PERCENT, RUNE_5x8_ET, 0x80|RUNE_5x8_QUOTE, RUNE_5x8_LPAREN, 
    RUNE_5x8_RPAREN, RUNE_5x8_STAR, RUNE_5x8_PLUS, RUNE_5x8_COMMA, 
    RUNE_5x8_MINUS, 0x80|RUNE_5x8_PERIOD, RUNE_5x8_SLASH, RUNE_5x8_0, 
    RUNE_5x8_1, RUNE_5x8_2, RUNE_5x8_3, RUNE_5x8_4, RUNE_5x8_5, RUNE_5x8_6, 
    RUNE_5x8_7, RUNE_5x8_8, RUNE_5x8_9, 0x80|RUNE_5x8_COLON, RUNE_5x8_SEMICOLON, 
    RUNE_5x8_LTHAN, RUNE_5x8_EQUAL, RUNE_5x8_GTHAN, RUNE_5x8_QMARK, 
    RUNE_5x8_AT, RUNE_5x8_A, RUNE_5x8_B, RUNE_5x8_C, RUNE_5x8_D, 
    RUNE_5x8_E, RUNE_5x8_F, RUNE_5x8_G, RUNE_5x8_H, 0x80|RUNE_5x8_I, 
    RUNE_5x8_J, RUNE_5x8_K, RUNE_5x8_L, RUNE_5x8_M, RUNE_5x8_N, RUNE_5x8_O, 
    RUNE_5x8_P, RUNE_5x8_Q, RUNE_5x8_R, RUNE_5x8_S, RUNE_5x8_T, RUNE_5x8_U, 
    RUNE_5x8_V, RUNE_5x8_W, RUNE_5x8_X, RUNE_5x8_Y, RUNE_5x8_Z, RUNE_5x8_LBRACKET, 
    RUNE_5x8_BSLASH, RUNE_5x8_RBRACKET, RUNE_5x8_CARET, RUNE_5x8_USCORE, 
    RUNE_5x8_BACKTICK, RUNE_5x8_a, RUNE_5x8_b, RUNE_5x8_c, RUNE_5x8_d, 
    RUNE_5x8_e, RUNE_5x8_f, RUNE_5x8_g, RUNE_5x8_h, 0x80|RUNE_5x8_i, 
    RUNE_5x8_j, RUNE_5x8_k, 0x80|RUNE_5x8_l, RUNE_5x8_m, RUNE_5x8_n, 
    RUNE_5x8_o, RUNE_5x8_p, RUNE_5x8_q, RUNE_5x8_r, RUNE_5x8_s, RUNE_5x8_t, 
    RUNE_5x8_u, RUNE_5x8_v, RUNE_5x8_w, RUNE_5x8_x, RUNE_5x8_y, RUNE_5x8_z, 
    RUNE_5x8_LCURLY, 0x80|RUNE_5x8_VBAR, RUNE_5x8_RCURLY, RUNE_5x8_TILDE, 
    0x80|RUNE_5x8_BLANK

    // End of map
};

// 95 runes.  Total data size 475 bytes.
const unsigned char rune_data_5x8[] = {
    // Rune: 5x8_BLANK, code 32, offset 0, size 5
    0x00,0x00,0x00,0x00,0x00,

    // Rune: 5x8_A, code 65, offset 5, size 5
    0x2e,0xc6,0x1f,0x63,0x04,

    // Rune: 5x8_B, code 66, offset 10, size 5
    0x2f,0xc6,0x17,0xe3,0x03,

    // Rune: 5x8_C, code 67, offset 15, size 5
    0x3e,0x84,0x10,0x82,0x07,

    // Rune: 5x8_D, code 68, offset 20, size 5
    0x2f,0xc6,0x18,0xe3,0x03,

    // Rune: 5x8_E, code 69, offset 25, size 5
    0x3f,0x84,0x17,0xc2,0x07,

    // Rune: 5x8_F, code 70, offset 30, size 5
    0x3f,0x84,0x17,0x42,0x00,

    // Rune: 5x8_G, code 71, offset 35, size 5
    0x2e,0x86,0x1c,0xa3,0x07,

    // Rune: 5x8_H, code 72, offset 40, size 5
    0x31,0xc6,0x1f,0x63,0x04,

    // Rune: 5x8_I, code 73, offset 45, size 5
    0x8e,0x10,0x42,0x88,0x03,

    // Rune: 5x8_J, code 74, offset 50, size 5
    0x1f,0x42,0x08,0xa3,0x03,

    // Rune: 5x8_K, code 75, offset 55, size 5
    0x31,0x95,0x51,0x52,0x04,

    // Rune: 5x8_L, code 76, offset 60, size 5
    0x21,0x84,0x10,0xc2,0x07,

    // Rune: 5x8_M, code 77, offset 65, size 5
    0x71,0xd7,0x18,0x63,0x04,

    // Rune: 5x8_N, code 78, offset 70, size 5
    0x71,0xd6,0x1c,0x63,0x04,

    // Rune: 5x8_O, code 79, offset 75, size 5
    0x2e,0xc6,0x18,0xa3,0x03,

    // Rune: 5x8_P, code 80, offset 80, size 5
    0x2f,0xc6,0x17,0x42,0x00,

    // Rune: 5x8_Q, code 81, offset 85, size 5
    0x2e,0xc6,0x18,0xab,0x83,

    // Rune: 5x8_R, code 82, offset 90, size 5
    0x2f,0xc6,0x57,0x52,0x04,

    // Rune: 5x8_S, code 83, offset 95, size 5
    0x2e,0x06,0x07,0xa3,0x03,

    // Rune: 5x8_T, code 84, offset 100, size 5
    0x9f,0x10,0x42,0x08,0x01,

    // Rune: 5x8_U, code 85, offset 105, size 5
    0x31,0xc6,0x18,0xa3,0x03,

    // Rune: 5x8_V, code 86, offset 110, size 5
    0x31,0x46,0xa5,0x08,0x01,

    // Rune: 5x8_W, code 87, offset 115, size 5
    0x31,0xc6,0x5a,0x77,0x04,

    // Rune: 5x8_X, code 88, offset 120, size 5
    0x31,0x2a,0xa2,0x62,0x04,

    // Rune: 5x8_Y, code 89, offset 125, size 5
    0x31,0x2a,0x42,0x08,0x01,

    // Rune: 5x8_Z, code 90, offset 130, size 5
    0x1f,0x22,0x22,0xc2,0x07,

    // Rune: 5x8_EX, code 33, offset 135, size 5
    0x84,0x10,0x42,0x00,0x01,

    // Rune: 5x8_DQUOTE, code 34, offset 140, size 5
    0x4a,0x29,0x00,0x00,0x00,

    // Rune: 5x8_AT, code 64, offset 145, size 5
    0x2e,0xf6,0x5a,0x93,0x07,

    // Rune: 5x8_HASH, code 35, offset 150, size 5
    0x4a,0x7d,0xf5,0x95,0x02,

    // Rune: 5x8_DOLLAR, code 36, offset 155, size 5
    0xc4,0x97,0x4f,0x1f,0x01,

    // Rune: 5x8_PERCENT, code 37, offset 160, size 5
    0x60,0x26,0x22,0x33,0x00,

    // Rune: 5x8_ET, code 38, offset 165, size 5
    0x6e,0x3a,0x53,0xb3,0x07,

    // Rune: 5x8_QUOTE, code 39, offset 170, size 5
    0x84,0x10,0x00,0x00,0x00,

    // Rune: 5x8_LPAREN, code 40, offset 175, size 5
    0x88,0x08,0x21,0x08,0x02,

    // Rune: 5x8_RPAREN, code 41, offset 180, size 5
    0x82,0x20,0x84,0x88,0x00,

    // Rune: 5x8_STAR, code 42, offset 185, size 5
    0x40,0x91,0x4f,0x14,0x00,

    // Rune: 5x8_PLUS, code 43, offset 190, size 5
    0x80,0x90,0x4f,0x08,0x00,

    // Rune: 5x8_COMMA, code 44, offset 195, size 5
    0x00,0x00,0x00,0x08,0x11,

    // Rune: 5x8_MINUS, code 45, offset 200, size 5
    0x00,0x80,0x0f,0x00,0x00,

    // Rune: 5x8_PERIOD, code 46, offset 205, size 5
    0x00,0x00,0x00,0x8c,0x01,

    // Rune: 5x8_SLASH, code 47, offset 210, size 5
    0x00,0x22,0x22,0x02,0x00,

    // Rune: 5x8_0, code 48, offset 215, size 5
    0x44,0xc5,0x18,0x15,0x01,

    // Rune: 5x8_1, code 49, offset 220, size 5
    0xc4,0x10,0x42,0x08,0x01,

    // Rune: 5x8_2, code 50, offset 225, size 5
    0x2e,0x42,0x26,0xc2,0x07,

    // Rune: 5x8_3, code 51, offset 230, size 5
    0x2e,0x42,0x07,0xa3,0x03,

    // Rune: 5x8_4, code 52, offset 235, size 5
    0x88,0xa9,0xf4,0x11,0x02,

    // Rune: 5x8_5, code 53, offset 240, size 5
    0x3f,0x04,0x07,0xa3,0x03,

    // Rune: 5x8_6, code 54, offset 245, size 5
    0x2e,0x86,0x17,0xa3,0x03,

    // Rune: 5x8_7, code 55, offset 250, size 5
    0x1f,0x11,0x21,0x84,0x00,

    // Rune: 5x8_8, code 56, offset 255, size 5
    0x2e,0x46,0x17,0xa3,0x03,

    // Rune: 5x8_9, code 57, offset 260, size 5
    0x2e,0x46,0x0f,0xa3,0x03,

    // Rune: 5x8_COLON, code 58, offset 265, size 5
    0x00,0x10,0x00,0x08,0x00,

    // Rune: 5x8_SEMICOLON, code 59, offset 270, size 5
    0x00,0x10,0x00,0x08,0x11,

    // Rune: 5x8_LTHAN, code 60, offset 275, size 5
    0x10,0x11,0x41,0x10,0x04,

    // Rune: 5x8_EQUAL, code 61, offset 280, size 5
    0x00,0x7c,0xf0,0x01,0x00,

    // Rune: 5x8_GTHAN, code 62, offset 285, size 5
    0x82,0x20,0x88,0x88,0x00,

    // Rune: 5x8_QMARK, code 63, offset 290, size 5
    0x2e,0x46,0x44,0x00,0x01,

    // Rune: 5x8_LBRACKET, code 91, offset 295, size 5
    0x5e,0x08,0x21,0x84,0x07,

    // Rune: 5x8_BSLASH, code 92, offset 300, size 5
    0x20,0x08,0x82,0x20,0x00,

    // Rune: 5x8_RBRACKET, code 93, offset 305, size 5
    0x1e,0x42,0x08,0xa1,0x07,

    // Rune: 5x8_CARET, code 94, offset 310, size 5
    0x44,0x45,0x00,0x00,0x00,

    // Rune: 5x8_USCORE, code 95, offset 315, size 5
    0x00,0x00,0x00,0xc0,0x07,

    // Rune: 5x8_BACKTICK, code 96, offset 320, size 5
    0x84,0x20,0x00,0x00,0x00,

    // Rune: 5x8_a, code 97, offset 325, size 5
    0x00,0x38,0xe8,0xa3,0x07,

    // Rune: 5x8_b, code 98, offset 330, size 5
    0x21,0xbc,0x18,0xe3,0x03,

    // Rune: 5x8_c, code 99, offset 335, size 5
    0x00,0xf8,0x10,0x82,0x07,

    // Rune: 5x8_d, code 100, offset 340, size 5
    0x10,0xfa,0x18,0xa3,0x07,

    // Rune: 5x8_e, code 101, offset 345, size 5
    0x00,0xb8,0xf8,0x82,0x07,

    // Rune: 5x8_f, code 102, offset 350, size 5
    0x88,0x12,0x47,0x08,0x01,

    // Rune: 5x8_g, code 103, offset 355, size 5
    0x00,0xf8,0xe8,0x61,0x74,

    // Rune: 5x8_h, code 104, offset 360, size 5
    0x21,0xbc,0x18,0x63,0x04,

    // Rune: 5x8_i, code 105, offset 365, size 5
    0x80,0x00,0x42,0x08,0x01,

    // Rune: 5x8_j, code 106, offset 370, size 5
    0x00,0x01,0x84,0x90,0x22,

    // Rune: 5x8_k, code 107, offset 375, size 5
    0x42,0x48,0x65,0x94,0x04,

    // Rune: 5x8_l, code 108, offset 380, size 5
    0x84,0x10,0x42,0x08,0x03,

    // Rune: 5x8_m, code 109, offset 385, size 5
    0x00,0xec,0x1a,0x63,0x04,

    // Rune: 5x8_n, code 110, offset 390, size 5
    0x00,0xb4,0x19,0x63,0x04,

    // Rune: 5x8_o, code 111, offset 395, size 5
    0x00,0xb8,0x18,0xa3,0x03,

    // Rune: 5x8_p, code 112, offset 400, size 5
    0x00,0xbc,0x18,0x5f,0x08,

    // Rune: 5x8_q, code 113, offset 405, size 5
    0x00,0xf8,0x18,0x3d,0x84,

    // Rune: 5x8_r, code 114, offset 410, size 5
    0x00,0xf4,0x11,0x42,0x00,

    // Rune: 5x8_s, code 115, offset 415, size 5
    0x00,0xf8,0xe0,0xe0,0x03,

    // Rune: 5x8_t, code 116, offset 420, size 5
    0x84,0x10,0x47,0x08,0x03,

    // Rune: 5x8_u, code 117, offset 425, size 5
    0x00,0xc4,0x18,0xb3,0x05,

    // Rune: 5x8_v, code 118, offset 430, size 5
    0x00,0xc4,0xa8,0x14,0x01,

    // Rune: 5x8_w, code 119, offset 435, size 5
    0x00,0xc4,0x58,0xab,0x02,

    // Rune: 5x8_x, code 120, offset 440, size 5
    0x00,0x44,0x45,0x54,0x04,

    // Rune: 5x8_y, code 121, offset 445, size 5
    0x00,0xc4,0xe8,0x61,0x74,

    // Rune: 5x8_z, code 122, offset 450, size 5
    0x00,0x7c,0x44,0xc4,0x07,

    // Rune: 5x8_LCURLY, code 123, offset 455, size 5
    0x88,0x10,0x41,0x08,0x02,

    // Rune: 5x8_VBAR, code 124, offset 460, size 5
    0x84,0x10,0x42,0x08,0x01,

    // Rune: 5x8_RCURLY, code 125, offset 465, size 5
    0x82,0x10,0x44,0x88,0x00,

    // Rune: 5x8_TILDE, code 126, offset 470, size 5
    0xb6,0x01,0x00,0x00,0x00

    // End of list
};

const Font font_5x8 = {rune_data_5x8, rune_map_5x8, 5, 8, 5, false};


const unsigned char rune_map_ins_9x16[128] = {
    0x80|RUNE_INS_9x16_BLANK, 0x80|RUNE_INS_9x16_BLANK, 0x80|RUNE_INS_9x16_BLANK, 
    0x80|RUNE_INS_9x16_BLANK, 0x80|RUNE_INS_9x16_BLANK, 0x80|RUNE_INS_9x16_BLANK, 
    0x80|RUNE_INS_9x16_BLANK, 0x80|RUNE_INS_9x16_BLANK, 0x80|RUNE_INS_9x16_BLANK, 
    0x80|RUNE_INS_9x16_BLANK, 0x80|RUNE_INS_9x16_BLANK, 0x80|RUNE_INS_9x16_BLANK, 
    0x80|RUNE_INS_9x16_BLANK, 0x80|RUNE_INS_9x16_BLANK, 0x80|RUNE_INS_9x16_BLANK, 
    0x80|RUNE_INS_9x16_BLANK, RUNE_INS_9x16_LARROW, RUNE_INS_9x16_RARROW, 
    RUNE_INS_9x16_UARROW, RUNE_INS_9x16_DARROW, RUNE_INS_9x16_DEG, 
    0x80|RUNE_INS_9x16_BLANK, 0x80|RUNE_INS_9x16_BLANK, 0x80|RUNE_INS_9x16_BLANK, 
    0x80|RUNE_INS_9x16_BLANK, 0x80|RUNE_INS_9x16_BLANK, 0x80|RUNE_INS_9x16_BLANK, 
    0x80|RUNE_INS_9x16_BLANK, 0x80|RUNE_INS_9x16_BLANK, 0x80|RUNE_INS_9x16_BLANK, 
    0x80|RUNE_INS_9x16_BLANK, 0x80|RUNE_INS_9x16_BLANK, 0x80|RUNE_INS_9x16_BLANK, 
    0x80|RUNE_INS_9x16_EX, 0x80|RUNE_INS_9x16_BLANK, 0x80|RUNE_INS_9x16_BLANK, 
    0x80|RUNE_INS_9x16_BLANK, RUNE_INS_9x16_PERCENT, 0x80|RUNE_INS_9x16_BLANK, 
    0x80|RUNE_INS_9x16_BLANK, 0x80|RUNE_INS_9x16_BLANK, 0x80|RUNE_INS_9x16_BLANK, 
    0x80|RUNE_INS_9x16_BLANK, RUNE_INS_9x16_PLUS, 0x80|RUNE_INS_9x16_COMMA, 
    RUNE_INS_9x16_MINUS, 0x80|RUNE_INS_9x16_PERIOD, 0x80|RUNE_INS_9x16_BLANK, 
    RUNE_INS_9x16_0, RUNE_INS_9x16_1, RUNE_INS_9x16_2, RUNE_INS_9x16_3, 
    RUNE_INS_9x16_4, RUNE_INS_9x16_5, RUNE_INS_9x16_6, RUNE_INS_9x16_7, 
    RUNE_INS_9x16_8, RUNE_INS_9x16_9, 0x80|RUNE_INS_9x16_BLANK, 0x80|RUNE_INS_9x16_BLANK, 
    0x80|RUNE_INS_9x16_BLANK, 0x80|RUNE_INS_9x16_BLANK, 0x80|RUNE_INS_9x16_BLANK, 
    RUNE_INS_9x16_QMARK, 0x80|RUNE_INS_9x16_BLANK, RUNE_INS_9x16_A, 
    RUNE_INS_9x16_B, RUNE_INS_9x16_C, RUNE_INS_9x16_D, RUNE_INS_9x16_E, 
    RUNE_INS_9x16_F, RUNE_INS_9x16_G, RUNE_INS_9x16_H, RUNE_INS_9x16_I, 
    RUNE_INS_9x16_J, RUNE_INS_9x16_K, RUNE_INS_9x16_L, RUNE_INS_9x16_M, 
    RUNE_INS_9x16_N, RUNE_INS_9x16_O, RUNE_INS_9x16_P, RUNE_INS_9x16_Q, 
    RUNE_INS_9x16_R, RUNE_INS_9x16_S, RUNE_INS_9x16_T, RUNE_INS_9x16_U, 
    RUNE_INS_9x16_V, RUNE_INS_9x16_W, RUNE_INS_9x16_X, RUNE_INS_9x16_Y, 
    RUNE_INS_9x16_Z, RUNE_INS_9x16_LBRACKET, 0x80|RUNE_INS_9x16_BLANK, 
    RUNE_INS_9x16_RBRACKET, 0x80|RUNE_INS_9x16_BLANK, 0x80|RUNE_INS_9x16_BLANK, 
    0x80|RUNE_INS_9x16_BLANK, RUNE_INS_9x16_a, RUNE_INS_9x16_b, RUNE_INS_9x16_c, 
    RUNE_INS_9x16_d, RUNE_INS_9x16_e, RUNE_INS_9x16_f, RUNE_INS_9x16_g, 
    RUNE_INS_9x16_h, 0x80|RUNE_INS_9x16_i, RUNE_INS_9x16_j, RUNE_INS_9x16_k, 
    0x80|RUNE_INS_9x16_l, RUNE_INS_9x16_m, RUNE_INS_9x16_n, RUNE_INS_9x16_o, 
    RUNE_INS_9x16_p, RUNE_INS_9x16_q, RUNE_INS_9x16_r, RUNE_INS_9x16_s, 
    RUNE_INS_9x16_t, RUNE_INS_9x16_u, RUNE_INS_9x16_v, RUNE_INS_9x16_w, 
    RUNE_INS_9x16_x, RUNE_INS_9x16_y, RUNE_INS_9x16_z, 0x80|RUNE_INS_9x16_BLANK, 
    0x80|RUNE_INS_9x16_BLANK, 0x80|RUNE_INS_9x16_BLANK, 0x80|RUNE_INS_9x16_BLANK, 
    0x80|RUNE_INS_9x16_BLANK

    // End of map
};

// 77 runes.  Total data size 1386 bytes.
const unsigned char rune_data_ins_9x16[] = {
    // Rune: INS_9x16_BLANK, code 32, offset 0, size 18
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,

    // Rune: INS_9x16_A, code 65, offset 18, size 18
    0x38,0x70,0xf0,0x61,0x63,0xcc,0xd8,0xe0,0xff,0xff,0x07,0x0f,0x1e,0x3c,0x78,0x30,
    0x00,0x00,

    // Rune: INS_9x16_B, code 66, offset 36, size 18
    0x7f,0xfe,0x0d,0x1f,0x3c,0x78,0xf8,0xbf,0x7f,0xc3,0x07,0x0f,0x1e,0xfe,0xef,0x0f,
    0x00,0x00,

    // Rune: INS_9x16_C, code 67, offset 54, size 18
    0x7c,0xfc,0x1d,0x1f,0x3c,0x60,0xc0,0x80,0x01,0x03,0x06,0x0f,0x3e,0xee,0x8f,0x0f,
    0x00,0x00,

    // Rune: INS_9x16_D, code 68, offset 72, size 18
    0x7f,0xfe,0x0d,0x1f,0x3c,0x78,0xf0,0xe0,0xc1,0x83,0x07,0x0f,0x1e,0xfe,0xef,0x0f,
    0x00,0x00,

    // Rune: INS_9x16_E, code 69, offset 90, size 18
    0xff,0xff,0x0f,0x18,0x30,0x60,0xc0,0x9f,0x3f,0x03,0x06,0x0c,0x18,0xf0,0xff,0x3f,
    0x00,0x00,

    // Rune: INS_9x16_F, code 70, offset 108, size 18
    0xff,0xff,0x0f,0x18,0x30,0x60,0xc0,0x9f,0x3f,0x03,0x06,0x0c,0x18,0x30,0x60,0x00,
    0x00,0x00,

    // Rune: INS_9x16_G, code 71, offset 126, size 18
    0x7c,0xfc,0x1d,0x1f,0x3c,0x60,0xc0,0xfc,0xf9,0x83,0x07,0x0f,0x3e,0xee,0x8f,0x0f,
    0x00,0x00,

    // Rune: INS_9x16_H, code 72, offset 144, size 18
    0x83,0x07,0x0f,0x1e,0x3c,0x78,0xf0,0xff,0xff,0x83,0x07,0x0f,0x1e,0x3c,0x78,0x30,
    0x00,0x00,

    // Rune: INS_9x16_I, code 73, offset 162, size 18
    0x7e,0xfc,0x60,0xc0,0x80,0x01,0x03,0x06,0x0c,0x18,0x30,0x60,0xc0,0xe0,0xc7,0x0f,
    0x00,0x00,

    // Rune: INS_9x16_J, code 74, offset 180, size 18
    0xfe,0xfd,0x03,0x06,0x0c,0x18,0x30,0x60,0xc0,0x80,0x07,0x0f,0x3e,0xee,0x8f,0x0f,
    0x00,0x00,

    // Rune: INS_9x16_K, code 75, offset 198, size 18
    0x83,0x87,0x8f,0x9b,0xb3,0xe3,0xc3,0x83,0x07,0x1f,0x76,0xcc,0x19,0x37,0x7c,0x30,
    0x00,0x00,

    // Rune: INS_9x16_L, code 76, offset 216, size 18
    0x03,0x06,0x0c,0x18,0x30,0x60,0xc0,0x80,0x01,0x03,0x06,0x0c,0x18,0xf0,0xff,0x3f,
    0x00,0x00,

    // Rune: INS_9x16_M, code 77, offset 234, size 18
    0x01,0x07,0x1f,0x7f,0xff,0x7f,0xf7,0xe4,0xc1,0x83,0x07,0x0f,0x1e,0x3c,0x78,0x30,
    0x00,0x00,

    // Rune: INS_9x16_N, code 78, offset 252, size 18
    0x83,0x07,0x0f,0x3e,0xfc,0xf8,0xf3,0xee,0xf9,0xe3,0x87,0x0f,0x1e,0x3c,0x78,0x30,
    0x00,0x00,

    // Rune: INS_9x16_O, code 79, offset 270, size 18
    0x7c,0xfc,0x1d,0x1f,0x3c,0x78,0xf0,0xe0,0xc1,0x83,0x07,0x0f,0x3e,0xee,0x8f,0x0f,
    0x00,0x00,

    // Rune: INS_9x16_P, code 80, offset 288, size 18
    0x7f,0xfe,0x0d,0x1f,0x3c,0x78,0xf0,0xf0,0x7f,0x7f,0x06,0x0c,0x18,0x30,0x60,0x00,
    0x00,0x00,

    // Rune: INS_9x16_Q, code 81, offset 306, size 18
    0x7c,0xfc,0x1d,0x1f,0x3c,0x78,0xf0,0xe0,0xc1,0x9b,0x77,0xcf,0x1b,0xe7,0x9f,0x33,
    0x00,0x00,

    // Rune: INS_9x16_R, code 82, offset 324, size 18
    0x7f,0xfe,0x0d,0x1f,0x3c,0x78,0xf8,0xbf,0x1f,0x3f,0xe6,0x8c,0x1b,0x3e,0x78,0x30,
    0x00,0x00,

    // Rune: INS_9x16_S, code 83, offset 342, size 18
    0x7c,0xfc,0x1d,0x1f,0x7c,0xc0,0x03,0x1f,0x78,0xc0,0x07,0x0f,0x3e,0xee,0x8f,0x0f,
    0x00,0x00,

    // Rune: INS_9x16_T, code 84, offset 360, size 18
    0xff,0xfe,0x61,0xc0,0x80,0x01,0x03,0x06,0x0c,0x18,0x30,0x60,0xc0,0x80,0x01,0x03,
    0x00,0x00,

    // Rune: INS_9x16_U, code 85, offset 378, size 18
    0x83,0x07,0x0f,0x1e,0x3c,0x78,0xf0,0xe0,0xc1,0x83,0x07,0x0f,0x3e,0xee,0x8f,0x0f,
    0x00,0x00,

    // Rune: INS_9x16_V, code 86, offset 396, size 18
    0x83,0x07,0x0f,0x1e,0x3c,0x78,0xd0,0x31,0x63,0xc6,0xd8,0xb0,0xc1,0x81,0x03,0x02,
    0x00,0x00,

    // Rune: INS_9x16_W, code 87, offset 414, size 18
    0x83,0x07,0x0f,0x1e,0x3c,0x78,0xf0,0xe0,0xc9,0xbb,0xff,0xbf,0x3f,0x3e,0x78,0x30,
    0x00,0x00,

    // Rune: INS_9x16_X, code 88, offset 432, size 18
    0x83,0x07,0x1b,0x33,0xc6,0x86,0x0d,0x0e,0x1c,0x6c,0xd8,0x18,0x33,0x36,0x78,0x30,
    0x00,0x00,

    // Rune: INS_9x16_Y, code 89, offset 450, size 18
    0xc3,0x86,0x0d,0x1b,0x76,0xce,0x0f,0x0f,0x0c,0x18,0x30,0x60,0xc0,0x80,0x01,0x03,
    0x00,0x00,

    // Rune: INS_9x16_Z, code 90, offset 468, size 18
    0xff,0xff,0x03,0x06,0x0e,0x0e,0x0e,0x0e,0x0e,0x0e,0x0e,0x0c,0x18,0xf0,0xff,0x3f,
    0x00,0x00,

    // Rune: INS_9x16_DEG, code 20, offset 486, size 18
    0x00,0xe0,0xe0,0xe3,0xce,0x98,0x3b,0x3e,0x38,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,

    // Rune: INS_9x16_PLUS, code 43, offset 504, size 18
    0x00,0x00,0x00,0x00,0x80,0x01,0x83,0x1f,0x3f,0x18,0x30,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,

    // Rune: INS_9x16_MINUS, code 45, offset 522, size 18
    0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x1f,0x3f,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,

    // Rune: INS_9x16_PERIOD, code 46, offset 540, size 18
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xc0,0x81,0x03,0x07,
    0x00,0x00,

    // Rune: INS_9x16_COMMA, code 44, offset 558, size 18
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x01,0x03,0x06,
    0x0e,0x0c,

    // Rune: INS_9x16_LBRACKET, code 91, offset 576, size 18
    0xff,0xff,0x0f,0x18,0x30,0x60,0xc0,0x80,0x01,0x03,0x06,0x0c,0x18,0xf0,0xff,0x3f,
    0x00,0x00,

    // Rune: INS_9x16_RBRACKET, code 93, offset 594, size 18
    0xff,0xff,0x03,0x06,0x0c,0x18,0x30,0x60,0xc0,0x80,0x01,0x03,0x06,0xfc,0xff,0x3f,
    0x00,0x00,

    // Rune: INS_9x16_QMARK, code 63, offset 612, size 18
    0x7c,0xfc,0x1d,0x1f,0x0c,0x1c,0x1c,0x1c,0x1c,0x18,0x30,0x00,0x00,0x80,0x01,0x03,
    0x00,0x00,

    // Rune: INS_9x16_EX, code 33, offset 630, size 18
    0x18,0x30,0x60,0xc0,0x80,0x01,0x03,0x06,0x0c,0x18,0x30,0x00,0x00,0x80,0x01,0x03,
    0x00,0x00,

    // Rune: INS_9x16_RARROW, code 17, offset 648, size 18
    0x00,0x00,0x00,0xc0,0x01,0x07,0xdc,0xff,0xff,0xe0,0xe0,0xe0,0x00,0x00,0x00,0x00,
    0x00,0x00,

    // Rune: INS_9x16_LARROW, code 16, offset 666, size 18
    0x00,0x00,0x00,0xc0,0xc1,0xc1,0xc1,0xff,0xff,0x0e,0x38,0xe0,0x00,0x00,0x00,0x00,
    0x00,0x00,

    // Rune: INS_9x16_UARROW, code 18, offset 684, size 18
    0x00,0x00,0x60,0xe0,0xe1,0xe7,0xdf,0xb6,0x4c,0x18,0x30,0x60,0xc0,0x00,0x00,0x00,
    0x00,0x00,

    // Rune: INS_9x16_DARROW, code 19, offset 702, size 18
    0x00,0x00,0x60,0xc0,0x80,0x01,0x43,0xa6,0x6d,0xff,0xfc,0xf0,0xc0,0x00,0x00,0x00,
    0x00,0x00,

    // Rune: INS_9x16_PERCENT, code 37, offset 720, size 18
    0x00,0x0e,0x1c,0x3e,0x0e,0x0e,0x0e,0x0e,0x0e,0x0e,0x8e,0x0f,0x0f,0x0e,0x00,0x00,
    0x00,0x00,

    // Rune: INS_9x16_0, code 48, offset 738, size 18
    0x38,0xf8,0xb8,0x33,0x76,0x7c,0xf0,0xe0,0xc1,0x83,0x8f,0x1b,0x73,0xc7,0x07,0x07,
    0x00,0x00,

    // Rune: INS_9x16_1, code 49, offset 756, size 18
    0x38,0x78,0xf8,0x80,0x01,0x03,0x06,0x0c,0x18,0x30,0x60,0xc0,0x80,0x01,0x03,0x06,
    0x00,0x00,

    // Rune: INS_9x16_2, code 50, offset 774, size 18
    0x7c,0xfc,0x1d,0x1f,0x3c,0x18,0x38,0x38,0x38,0x38,0x38,0x38,0x38,0xf0,0xff,0x3f,
    0x00,0x00,

    // Rune: INS_9x16_3, code 51, offset 792, size 18
    0x7c,0xfc,0x1d,0x1f,0x0c,0x18,0x38,0x3e,0x7c,0xc0,0x01,0x0f,0x3e,0xee,0x8f,0x0f,
    0x00,0x00,

    // Rune: INS_9x16_4, code 52, offset 810, size 18
    0x60,0xe0,0xe0,0xe1,0xe3,0xe6,0xcc,0x98,0xff,0xff,0xc1,0x80,0x01,0x03,0x06,0x0c,
    0x00,0x00,

    // Rune: INS_9x16_5, code 53, offset 828, size 18
    0xff,0xfe,0x0d,0x18,0x30,0xe0,0xcf,0x3f,0xe0,0x80,0x01,0x03,0x06,0xfe,0xef,0x0f,
    0x00,0x00,

    // Rune: INS_9x16_6, code 54, offset 846, size 18
    0x7c,0xfc,0x1d,0x1f,0x30,0x60,0xcf,0xbf,0xe3,0x83,0x07,0x0f,0x3e,0xee,0x8f,0x0f,
    0x00,0x00,

    // Rune: INS_9x16_7, code 55, offset 864, size 18
    0xff,0xff,0x03,0x06,0x0e,0x0e,0x0e,0x0e,0x0c,0x18,0x30,0x60,0xc0,0x80,0x01,0x03,
    0x00,0x00,

    // Rune: INS_9x16_8, code 56, offset 882, size 18
    0x7c,0xfc,0x1d,0x1f,0x7c,0xdc,0x1f,0x1f,0x77,0xc7,0x07,0x0f,0x3e,0xee,0x8f,0x0f,
    0x00,0x00,

    // Rune: INS_9x16_9, code 57, offset 900, size 18
    0x7c,0xfc,0x1d,0x1f,0x3c,0x78,0xf0,0x71,0xff,0xfc,0x01,0x0f,0x3e,0xee,0x8f,0x0f,
    0x00,0x00,

    // Rune: INS_9x16_a, code 97, offset 918, size 18
    0x00,0x00,0x00,0x00,0x00,0x00,0x1f,0x7f,0xc0,0xfc,0xfd,0x1b,0x36,0xec,0x9f,0x37,
    0x00,0x00,

    // Rune: INS_9x16_b, code 98, offset 936, size 18
    0x06,0x0c,0x18,0x30,0x60,0xc0,0x9e,0x7f,0xc3,0x86,0x0d,0x1b,0x36,0xee,0xdf,0x1f,
    0x00,0x00,

    // Rune: INS_9x16_c, code 99, offset 954, size 18
    0x00,0x00,0x00,0x00,0x00,0x00,0x1f,0x7f,0xc7,0x06,0x0c,0x18,0x70,0xcc,0x1f,0x1f,
    0x00,0x00,

    // Rune: INS_9x16_d, code 100, offset 972, size 18
    0x80,0x01,0x03,0x06,0x0c,0x98,0xb7,0x7f,0xc3,0x86,0x0d,0x1b,0x36,0xec,0x9f,0x2f,
    0x00,0x00,

    // Rune: INS_9x16_e, code 101, offset 990, size 18
    0x00,0x00,0x00,0x00,0x00,0x00,0x1f,0x7f,0xc6,0x8c,0xf9,0xf3,0x63,0xc0,0x1f,0x1f,
    0x00,0x00,

    // Rune: INS_9x16_f, code 102, offset 1008, size 18
    0xe0,0xe0,0xc3,0x86,0x09,0x03,0x86,0x7f,0xff,0x30,0x60,0xc0,0x80,0x01,0x03,0x06,
    0x00,0x00,

    // Rune: INS_9x16_g, code 103, offset 1026, size 18
    0x00,0x00,0x00,0x00,0x00,0x00,0x3f,0x7f,0xc3,0x86,0x0d,0xf3,0xc7,0x0f,0xd8,0x30,
    0x3f,0x3c,

    // Rune: INS_9x16_h, code 104, offset 1044, size 18
    0x06,0x0c,0x18,0x30,0x60,0xc0,0x9e,0x7f,0xc3,0x86,0x0d,0x1b,0x36,0x6c,0xd8,0x30,
    0x00,0x00,

    // Rune: INS_9x16_i, code 105, offset 1062, size 18
    0x00,0x30,0x60,0x00,0x00,0x00,0x03,0x06,0x0c,0x18,0x30,0x60,0xc0,0x82,0x07,0x07,
    0x00,0x00,

    // Rune: INS_9x16_j, code 106, offset 1080, size 18
    0x00,0xc0,0x80,0x01,0x00,0x00,0x0c,0x18,0x30,0x60,0xc0,0x80,0x01,0x23,0xc6,0x8c,
    0x1f,0x1e,

    // Rune: INS_9x16_k, code 107, offset 1098, size 18
    0x06,0x0c,0x18,0x30,0x60,0xc0,0x90,0x31,0x33,0x36,0x3c,0x78,0xb0,0x61,0xce,0x18,
    0x00,0x00,

    // Rune: INS_9x16_l, code 108, offset 1116, size 18
    0x18,0x30,0x60,0xc0,0x80,0x01,0x03,0x06,0x0c,0x18,0x30,0x60,0xc0,0x82,0x07,0x07,
    0x00,0x00,

    // Rune: INS_9x16_m, code 109, offset 1134, size 18
    0x00,0x00,0x00,0x00,0x00,0xc0,0xd8,0xfb,0xff,0xbb,0x77,0x4f,0x9e,0x3c,0x78,0x30,
    0x00,0x00,

    // Rune: INS_9x16_n, code 110, offset 1152, size 18
    0x00,0x00,0x00,0x00,0x00,0xc0,0x9e,0x7f,0xc3,0x86,0x0d,0x1b,0x36,0x6c,0xd8,0x30,
    0x00,0x00,

    // Rune: INS_9x16_o, code 111, offset 1170, size 18
    0x00,0x00,0x00,0x00,0x00,0x00,0x0f,0x3f,0xe7,0x86,0x0d,0x1b,0x76,0xce,0x0f,0x0f,
    0x00,0x00,

    // Rune: INS_9x16_p, code 112, offset 1188, size 18
    0x00,0x00,0x00,0x00,0x00,0xc0,0x9f,0x7f,0xc3,0x86,0x0d,0x1b,0xf6,0xef,0xcf,0x80,
    0x01,0x03,

    // Rune: INS_9x16_q, code 113, offset 1206, size 18
    0x00,0x00,0x00,0x00,0x00,0x80,0xbf,0x7f,0xc3,0x86,0x0d,0x1b,0xf6,0xcf,0x1b,0x30,
    0x60,0xc0,

    // Rune: INS_9x16_r, code 114, offset 1224, size 18
    0x00,0x00,0x00,0x00,0x00,0xc0,0x9e,0x7f,0xc3,0x06,0x0c,0x18,0x30,0x60,0xc0,0x00,
    0x00,0x00,

    // Rune: INS_9x16_s, code 115, offset 1242, size 18
    0x00,0x00,0x00,0x00,0x00,0x80,0x8f,0x3f,0x03,0x7e,0xf8,0x00,0x33,0xe6,0x8f,0x0f,
    0x00,0x00,

    // Rune: INS_9x16_t, code 116, offset 1260, size 18
    0x18,0x30,0x60,0xc0,0x80,0x01,0x83,0x3f,0x7f,0x18,0x30,0x60,0xc0,0x86,0x0f,0x0e,
    0x00,0x00,

    // Rune: INS_9x16_u, code 117, offset 1278, size 18
    0x00,0x00,0x00,0x00,0x00,0xc0,0xb0,0x61,0xc3,0x86,0x0d,0x1b,0x36,0xee,0x9f,0x37,
    0x00,0x00,

    // Rune: INS_9x16_v, code 118, offset 1296, size 18
    0x00,0x00,0x00,0x00,0x00,0xc0,0xb0,0x61,0xc3,0xcc,0x98,0xe1,0xc1,0x03,0x03,0x06,
    0x00,0x00,

    // Rune: INS_9x16_w, code 119, offset 1314, size 18
    0x00,0x00,0x00,0x00,0x00,0x60,0xf0,0xe0,0xc1,0x93,0x77,0xef,0xfe,0xef,0x8e,0x08,
    0x00,0x00,

    // Rune: INS_9x16_x, code 120, offset 1332, size 18
    0x00,0x00,0x00,0x00,0x00,0xc0,0xb0,0x61,0x66,0x78,0xf0,0xe0,0x61,0x66,0xd8,0x30,
    0x00,0x00,

    // Rune: INS_9x16_y, code 121, offset 1350, size 18
    0x00,0x00,0x00,0x00,0x00,0xc0,0xb0,0x61,0xc3,0x86,0xfd,0xf3,0x06,0x0c,0x18,0xb0,
    0x3f,0x3e,

    // Rune: INS_9x16_z, code 122, offset 1368, size 18
    0x00,0x00,0x00,0x00,0x00,0x80,0x1f,0x3f,0x60,0x60,0x60,0x60,0x60,0xc0,0x8f,0x1f,
    0x00,0x00

    // End of list
};

const Font font_ins_9x16 = {rune_data_ins_9x16, rune_map_ins_9x16, 9, 16, 18, false};


const unsigned char rune_map_ins_25x40[128] = {
    0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 
    0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 
    0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 
    0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 
    0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 
    0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 
    0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 
    0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 
    0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 
    0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 
    0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 
    0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 
    RUNE_DIGIT0, RUNE_DIGIT1, RUNE_DIGIT2, RUNE_DIGIT3, RUNE_DIGIT4, 
    RUNE_DIGIT5, RUNE_DIGIT6, RUNE_DIGIT7, RUNE_DIGIT8, RUNE_DIGIT9, 
    0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 
    0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 
    0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 
    0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 
    0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 
    0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 
    0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 
    0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 
    0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 
    0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 
    0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 
    0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 
    0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 
    0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 
    0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 
    0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 
    0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 0x80|RUNE_BLANK, 
    0x80|RUNE_BLANK, 0x80|RUNE_BLANK

    // End of map
};

// 11 runes.  Total data size 1375 bytes.
const unsigned char rune_data_ins_25x40[] = {
    // Rune: DIGIT0, code 48, offset 0, size 125
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xfc,0x07,0x00,0xff,0x7f,0x80,0xff,0xff,0x83,
    0xff,0xff,0x0f,0xff,0xff,0x1f,0xff,0x80,0x7f,0x7e,0x00,0xfc,0x7c,0x00,0xf0,0xfd,
    0x00,0xe0,0xff,0x00,0x80,0xff,0x01,0x00,0xff,0x03,0x00,0xfe,0x07,0x00,0xfc,0x0f,
    0x00,0xf8,0x1f,0x00,0xf0,0x3f,0x00,0xe0,0x7f,0x00,0xc0,0xff,0x00,0x80,0xff,0x01,
    0x00,0xff,0x03,0x00,0xfe,0x07,0x00,0xfc,0x0f,0x00,0xf8,0x1f,0x00,0xf0,0x3f,0x00,
    0xe0,0x7f,0x00,0xc0,0xff,0x00,0x80,0xff,0x01,0x00,0xff,0x03,0x00,0xfe,0x0f,0x00,
    0x7e,0x1f,0x00,0x7c,0x7e,0x00,0xfc,0xfc,0x03,0xfe,0xf1,0xff,0xff,0xe1,0xff,0xff,
    0x83,0xff,0xff,0x03,0xfc,0xff,0x01,0xc0,0x7f,0x00,0x00,0x00,0x00,

    // Rune: DIGIT1, code 49, offset 125, size 125
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xe0,0x01,0x00,0xe0,0x07,0x00,0xe0,0x0f,0x00,
    0xe0,0x1f,0x00,0xe0,0x3f,0x00,0xe0,0x7f,0x00,0xe0,0xff,0x00,0xe0,0xff,0x01,0x00,
    0xf0,0x03,0x00,0xc0,0x07,0x00,0x80,0x0f,0x00,0x00,0x1f,0x00,0x00,0x3e,0x00,0x00,
    0x7c,0x00,0x00,0xf8,0x00,0x00,0xf0,0x01,0x00,0xe0,0x03,0x00,0xc0,0x07,0x00,0x80,
    0x0f,0x00,0x00,0x1f,0x00,0x00,0x3e,0x00,0x00,0x7c,0x00,0x00,0xf8,0x00,0x00,0xf0,
    0x01,0x00,0xe0,0x03,0x00,0xc0,0x07,0x00,0x80,0x0f,0x00,0x00,0x1f,0x00,0x00,0x3e,
    0x00,0x00,0x7c,0x00,0x00,0xf8,0x00,0x00,0xf0,0x01,0x00,0xe0,0x03,0x00,0xe0,0x0f,
    0x00,0xf0,0x7f,0x00,0xf8,0xff,0x03,0xf8,0xff,0x07,0x00,0x00,0x00,

    // Rune: DIGIT2, code 50, offset 250, size 125
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xfc,0x07,0x00,0xff,0x7f,0x80,0xff,0xff,0x83,
    0xff,0xff,0x0f,0xff,0xff,0x1f,0x7f,0x80,0x7f,0x7f,0x00,0xfc,0x7e,0x00,0xf8,0x7d,
    0x00,0xe0,0xff,0x00,0xc0,0xff,0x01,0x80,0xdf,0x01,0x00,0x3f,0x00,0x00,0x7e,0x00,
    0x00,0x7e,0x00,0x00,0xfc,0x00,0x00,0xfc,0x01,0x00,0xfe,0x01,0x00,0xfe,0x01,0x00,
    0xff,0x01,0x80,0xff,0x01,0xc0,0xff,0x01,0xe0,0xff,0x00,0xf0,0x7f,0x00,0xf8,0x3f,
    0x00,0xf8,0x1f,0x00,0xf8,0x0f,0x00,0xf0,0x07,0x00,0xe0,0x07,0x00,0xc0,0x07,0x00,
    0x80,0x0f,0x00,0x00,0x1f,0x00,0x00,0x7e,0x00,0x00,0xfc,0xff,0xff,0xf9,0xff,0xff,
    0xf7,0xff,0xff,0xef,0xff,0xff,0x9f,0xff,0xff,0x1f,0x00,0x00,0x00,

    // Rune: DIGIT3, code 51, offset 375, size 125
    0x00,0x00,0x00,0x00,0x00,0x00,0xf8,0xff,0xff,0xfb,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xbf,0xff,0xff,0x3f,0x00,0x00,0x7e,0x00,0x00,0x7e,0x00,0x00,0x7e,0x00,
    0x00,0x7e,0x00,0x00,0x7e,0x00,0x00,0x7e,0x00,0x00,0x7e,0x00,0x00,0x7e,0x00,0x00,
    0x7e,0x00,0x00,0x7e,0x00,0x00,0x7e,0x00,0x00,0xfc,0x00,0x00,0xf8,0x07,0x00,0xe0,
    0x3f,0x00,0x00,0xff,0x01,0x00,0xf8,0x0f,0x00,0xc0,0x3f,0x00,0x00,0xfe,0x00,0x00,
    0xf0,0x03,0x00,0xc0,0x07,0x00,0x00,0x0f,0x00,0x00,0xfe,0x01,0x00,0xfc,0x03,0x00,
    0xf8,0x0f,0x00,0xf8,0x1f,0x00,0xf8,0x7c,0x00,0xf8,0xf1,0xff,0xff,0xe1,0xff,0xff,
    0x83,0xff,0xff,0x03,0xfc,0xff,0x01,0xc0,0x7f,0x00,0x00,0x00,0x00,

    // Rune: DIGIT4, code 52, offset 500, size 125
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7f,0x00,0x80,0xff,0x01,0x80,0xff,0x03,
    0x00,0xff,0x07,0x00,0xff,0x0f,0x00,0x7e,0x1f,0x00,0x7e,0x3e,0x00,0xfc,0x7c,0x00,
    0xfc,0xf8,0x00,0xf8,0xf1,0x01,0xf8,0xe1,0x03,0xf0,0xc3,0x07,0xf0,0x83,0x0f,0xe0,
    0x07,0x1f,0xe0,0x07,0x3e,0xc0,0x0f,0x7c,0xc0,0x0f,0xf8,0x80,0x1f,0xf0,0x81,0x1f,
    0xe0,0x03,0x3f,0xc0,0x07,0x3f,0x80,0x0f,0x7e,0x00,0x1f,0xfe,0xff,0xff,0xfc,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xef,0xff,0xff,0x0f,0x00,0xc0,0x07,0x00,0x80,
    0x0f,0x00,0x00,0x1f,0x00,0x00,0x3e,0x00,0x00,0x7c,0x00,0x00,0xf8,0x00,0x00,0xf0,
    0x01,0x00,0xe0,0x03,0x00,0xf0,0x1f,0x00,0xe0,0x3f,0x00,0x00,0x00,

    // Rune: DIGIT5, code 53, offset 625, size 125
    0x00,0x00,0x00,0x00,0x00,0x00,0xf8,0xff,0xff,0xfb,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xbf,0x1f,0x00,0x00,0x1f,0x00,0x00,0x3e,0x00,0x00,0x7c,
    0x00,0x00,0xf8,0x00,0x00,0xf0,0x01,0x00,0xe0,0x03,0x00,0xc0,0x07,0x00,0x80,0x1f,
    0x00,0x00,0xff,0xff,0x07,0xfe,0xff,0x3f,0xfc,0xff,0xff,0xf8,0xff,0xff,0xe3,0xff,
    0xff,0x07,0x00,0xc0,0x1f,0x00,0x00,0x3f,0x00,0x00,0xfc,0x00,0x00,0xf8,0x01,0x00,
    0xe0,0x03,0x00,0xc0,0x77,0x00,0x80,0xff,0x01,0x00,0xff,0x03,0x00,0xfe,0x07,0x00,
    0xfe,0x1f,0x00,0xfc,0x3e,0x00,0xfc,0xfc,0x01,0xfc,0xf1,0xff,0xff,0xe1,0xff,0xff,
    0x83,0xff,0xff,0x03,0xfc,0xff,0x01,0xc0,0x7f,0x00,0x00,0x00,0x00,

    // Rune: DIGIT6, code 54, offset 750, size 125
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xfe,0x3f,0x00,0xff,0xff,0x01,0xff,0xff,0x07,
    0xff,0xff,0x0f,0xfe,0xff,0x3f,0xfe,0x00,0x7e,0xfc,0x00,0xf8,0xfc,0x01,0xf0,0xf9,
    0x01,0xc0,0xf1,0x03,0x00,0xf0,0x07,0x00,0xe0,0x07,0x00,0xc0,0x0f,0x00,0x80,0x1f,
    0x00,0x00,0x7f,0x00,0x00,0xfe,0xff,0x1f,0xfc,0xff,0xff,0xf8,0xff,0xff,0xf3,0xff,
    0xff,0xe7,0xff,0xff,0xdf,0x1f,0x00,0xbf,0x1f,0x00,0xfc,0x3f,0x00,0xf0,0x7f,0x00,
    0xe0,0xff,0x00,0xc0,0xff,0x01,0x80,0xff,0x03,0x00,0xff,0x07,0x00,0xfe,0x0f,0x00,
    0xfe,0x1f,0x00,0xfc,0x7e,0x00,0xfc,0xfc,0x01,0xfc,0xf1,0xff,0xff,0xe1,0xff,0xff,
    0x83,0xff,0xff,0x03,0xfc,0xff,0x01,0xc0,0x7f,0x00,0x00,0x00,0x00,

    // Rune: DIGIT7, code 55, offset 875, size 125
    0x00,0x00,0x00,0x00,0x00,0x00,0xf8,0xff,0xff,0xfb,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0x7f,0xff,0xff,0xff,0x00,0x00,0xfe,0x01,0x00,0xfe,0x01,
    0x00,0xfe,0x01,0x00,0xfe,0x01,0x00,0xfc,0x01,0x00,0xfc,0x01,0x00,0xf8,0x01,0x00,
    0xf8,0x03,0x00,0xf0,0x03,0x00,0xf0,0x07,0x00,0xe0,0x07,0x00,0xc0,0x0f,0x00,0xc0,
    0x1f,0x00,0x80,0x1f,0x00,0x00,0x3f,0x00,0x00,0x7e,0x00,0x00,0x7e,0x00,0x00,0xfc,
    0x00,0x00,0xf8,0x01,0x00,0xf0,0x03,0x00,0xe0,0x07,0x00,0xc0,0x0f,0x00,0x80,0x1f,
    0x00,0x00,0x3f,0x00,0x00,0x7e,0x00,0x00,0xfc,0x00,0x00,0xf8,0x01,0x00,0xf0,0x03,
    0x00,0xe0,0x07,0x00,0xc0,0x0f,0x00,0x00,0x0f,0x00,0x00,0x00,0x00,

    // Rune: DIGIT8, code 56, offset 1000, size 125
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xfc,0x07,0x00,0xff,0x7f,0x80,0xff,0xff,0x83,
    0xff,0xff,0x0f,0xff,0xff,0x1f,0xff,0x80,0x7f,0x7e,0x00,0xfc,0x7c,0x00,0xf0,0xfd,
    0x00,0xe0,0xff,0x00,0x80,0xff,0x01,0x00,0xff,0x03,0x00,0xfe,0x07,0x00,0xfc,0x1f,
    0x00,0xfc,0x7e,0x00,0xfc,0xfc,0xff,0xff,0xf1,0xff,0xff,0xe1,0xff,0xff,0xc3,0xff,
    0xff,0xc7,0xff,0xff,0x9f,0x1f,0x00,0xbf,0x1f,0x00,0xfc,0x1f,0x00,0xf0,0x3f,0x00,
    0xe0,0x7f,0x00,0xc0,0xff,0x00,0x80,0xff,0x01,0x00,0xff,0x03,0x00,0xfe,0x0f,0x00,
    0x7e,0x1f,0x00,0x7c,0x7e,0x00,0xfc,0xfc,0x03,0xfe,0xf1,0xff,0xff,0xe1,0xff,0xff,
    0x83,0xff,0xff,0x03,0xfc,0xff,0x01,0xc0,0x7f,0x00,0x00,0x00,0x00,

    // Rune: DIGIT9, code 57, offset 1125, size 125
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0x3f,0x80,0xff,0xff,0x81,0xff,0xff,0x8f,
    0xff,0xff,0x1f,0xff,0xff,0x7f,0x7f,0x00,0xfe,0x7e,0x00,0xf8,0x7f,0x00,0xe0,0xff,
    0x00,0xc0,0xff,0x00,0x80,0xff,0x01,0x00,0xff,0x03,0x00,0xfe,0x07,0x00,0xfc,0x0f,
    0x00,0xf8,0x1f,0x00,0xf0,0x7f,0x00,0xe0,0xfb,0x01,0xe0,0xf7,0xff,0xff,0xcf,0xff,
    0xff,0x9f,0xff,0xff,0x3f,0xfe,0xff,0x7f,0xf0,0xff,0xff,0x00,0x00,0xf8,0x01,0x00,
    0xe0,0x03,0x00,0xc0,0x07,0x00,0x80,0x0f,0x00,0x00,0x1f,0x00,0x00,0x3e,0x00,0x00,
    0x7c,0x0e,0x00,0xf8,0x3e,0x00,0xf8,0x7d,0x00,0xf8,0xfb,0xff,0xff,0xf3,0xff,0xff,
    0xe7,0xff,0xff,0x87,0xff,0xff,0x07,0xfc,0xff,0x07,0x00,0x00,0x00,

    // Rune: BLANK, code 32, offset 1250, size 125
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00

    // End of list
};

const Font font_ins_25x40 = {rune_data_ins_25x40, rune_map_ins_25x40, 25, 40, 125, false};