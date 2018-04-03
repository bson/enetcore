// -*- C -*-
#ifndef _RUNES_H_
#define _RUNES_H_
enum Rune {
    RUNE_5x8_BLANK       = 0,
    RUNE_5x8_A           = 1,
    RUNE_5x8_B           = 2,
    RUNE_5x8_C           = 3,
    RUNE_5x8_D           = 4,
    RUNE_5x8_E           = 5,
    RUNE_5x8_F           = 6,
    RUNE_5x8_G           = 7,
    RUNE_5x8_H           = 8,
    RUNE_5x8_I           = 9,
    RUNE_5x8_J           = 10,
    RUNE_5x8_K           = 11,
    RUNE_5x8_L           = 12,
    RUNE_5x8_M           = 13,
    RUNE_5x8_N           = 14,
    RUNE_5x8_O           = 15,
    RUNE_5x8_P           = 16,
    RUNE_5x8_Q           = 17,
    RUNE_5x8_R           = 18,
    RUNE_5x8_S           = 19,
    RUNE_5x8_T           = 20,
    RUNE_5x8_U           = 21,
    RUNE_5x8_V           = 22,
    RUNE_5x8_W           = 23,
    RUNE_5x8_X           = 24,
    RUNE_5x8_Y           = 25,
    RUNE_5x8_Z           = 26,
    RUNE_5x8_EX          = 27,
    RUNE_5x8_DQUOTE      = 28,
    RUNE_5x8_AT          = 29,
    RUNE_5x8_HASH        = 30,
    RUNE_5x8_DOLLAR      = 31,
    RUNE_5x8_PERCENT     = 32,
    RUNE_5x8_ET          = 33,
    RUNE_5x8_QUOTE       = 34,
    RUNE_5x8_LPAREN      = 35,
    RUNE_5x8_RPAREN      = 36,
    RUNE_5x8_STAR        = 37,
    RUNE_5x8_PLUS        = 38,
    RUNE_5x8_COMMA       = 39,
    RUNE_5x8_MINUS       = 40,
    RUNE_5x8_PERIOD      = 41,
    RUNE_5x8_SLASH       = 42,
    RUNE_5x8_0           = 43,
    RUNE_5x8_1           = 44,
    RUNE_5x8_2           = 45,
    RUNE_5x8_3           = 46,
    RUNE_5x8_4           = 47,
    RUNE_5x8_5           = 48,
    RUNE_5x8_6           = 49,
    RUNE_5x8_7           = 50,
    RUNE_5x8_8           = 51,
    RUNE_5x8_9           = 52,
    RUNE_5x8_COLON       = 53,
    RUNE_5x8_SEMICOLON   = 54,
    RUNE_5x8_LTHAN       = 55,
    RUNE_5x8_EQUAL       = 56,
    RUNE_5x8_GTHAN       = 57,
    RUNE_5x8_QMARK       = 58,
    RUNE_5x8_LBRACKET    = 59,
    RUNE_5x8_BSLASH      = 60,
    RUNE_5x8_RBRACKET    = 61,
    RUNE_5x8_CARET       = 62,
    RUNE_5x8_USCORE      = 63,
    RUNE_5x8_BACKTICK    = 64,
    RUNE_5x8_a           = 65,
    RUNE_5x8_b           = 66,
    RUNE_5x8_c           = 67,
    RUNE_5x8_d           = 68,
    RUNE_5x8_e           = 69,
    RUNE_5x8_f           = 70,
    RUNE_5x8_g           = 71,
    RUNE_5x8_h           = 72,
    RUNE_5x8_i           = 73,
    RUNE_5x8_j           = 74,
    RUNE_5x8_k           = 75,
    RUNE_5x8_l           = 76,
    RUNE_5x8_m           = 77,
    RUNE_5x8_n           = 78,
    RUNE_5x8_o           = 79,
    RUNE_5x8_p           = 80,
    RUNE_5x8_q           = 81,
    RUNE_5x8_r           = 82,
    RUNE_5x8_s           = 83,
    RUNE_5x8_t           = 84,
    RUNE_5x8_u           = 85,
    RUNE_5x8_v           = 86,
    RUNE_5x8_w           = 87,
    RUNE_5x8_x           = 88,
    RUNE_5x8_y           = 89,
    RUNE_5x8_z           = 90,
    RUNE_5x8_LCURLY      = 91,
    RUNE_5x8_VBAR        = 92,
    RUNE_5x8_RCURLY      = 93,
    RUNE_5x8_TILDE       = 94,
    RUNE_INS_9x16_BLANK  = 0,
    RUNE_INS_9x16_A      = 1,
    RUNE_INS_9x16_B      = 2,
    RUNE_INS_9x16_C      = 3,
    RUNE_INS_9x16_D      = 4,
    RUNE_INS_9x16_E      = 5,
    RUNE_INS_9x16_F      = 6,
    RUNE_INS_9x16_G      = 7,
    RUNE_INS_9x16_H      = 8,
    RUNE_INS_9x16_I      = 9,
    RUNE_INS_9x16_J      = 10,
    RUNE_INS_9x16_K      = 11,
    RUNE_INS_9x16_L      = 12,
    RUNE_INS_9x16_M      = 13,
    RUNE_INS_9x16_N      = 14,
    RUNE_INS_9x16_O      = 15,
    RUNE_INS_9x16_P      = 16,
    RUNE_INS_9x16_Q      = 17,
    RUNE_INS_9x16_R      = 18,
    RUNE_INS_9x16_S      = 19,
    RUNE_INS_9x16_T      = 20,
    RUNE_INS_9x16_U      = 21,
    RUNE_INS_9x16_V      = 22,
    RUNE_INS_9x16_W      = 23,
    RUNE_INS_9x16_X      = 24,
    RUNE_INS_9x16_Y      = 25,
    RUNE_INS_9x16_Z      = 26,
    RUNE_INS_9x16_DEG    = 27,
    RUNE_INS_9x16_PLUS   = 28,
    RUNE_INS_9x16_MINUS  = 29,
    RUNE_INS_9x16_PERIOD = 30,
    RUNE_INS_9x16_COMMA  = 31,
    RUNE_INS_9x16_LBRACKET = 32,
    RUNE_INS_9x16_RBRACKET = 33,
    RUNE_INS_9x16_QMARK  = 34,
    RUNE_INS_9x16_EX     = 35,
    RUNE_INS_9x16_RARROW = 36,
    RUNE_INS_9x16_LARROW = 37,
    RUNE_INS_9x16_UARROW = 38,
    RUNE_INS_9x16_DARROW = 39,
    RUNE_INS_9x16_PERCENT = 40,
    RUNE_INS_9x16_0      = 41,
    RUNE_INS_9x16_1      = 42,
    RUNE_INS_9x16_2      = 43,
    RUNE_INS_9x16_3      = 44,
    RUNE_INS_9x16_4      = 45,
    RUNE_INS_9x16_5      = 46,
    RUNE_INS_9x16_6      = 47,
    RUNE_INS_9x16_7      = 48,
    RUNE_INS_9x16_8      = 49,
    RUNE_INS_9x16_9      = 50,
    RUNE_INS_9x16_a      = 51,
    RUNE_INS_9x16_b      = 52,
    RUNE_INS_9x16_c      = 53,
    RUNE_INS_9x16_d      = 54,
    RUNE_INS_9x16_e      = 55,
    RUNE_INS_9x16_f      = 56,
    RUNE_INS_9x16_g      = 57,
    RUNE_INS_9x16_h      = 58,
    RUNE_INS_9x16_i      = 59,
    RUNE_INS_9x16_j      = 60,
    RUNE_INS_9x16_k      = 61,
    RUNE_INS_9x16_l      = 62,
    RUNE_INS_9x16_m      = 63,
    RUNE_INS_9x16_n      = 64,
    RUNE_INS_9x16_o      = 65,
    RUNE_INS_9x16_p      = 66,
    RUNE_INS_9x16_q      = 67,
    RUNE_INS_9x16_r      = 68,
    RUNE_INS_9x16_s      = 69,
    RUNE_INS_9x16_t      = 70,
    RUNE_INS_9x16_u      = 71,
    RUNE_INS_9x16_v      = 72,
    RUNE_INS_9x16_w      = 73,
    RUNE_INS_9x16_x      = 74,
    RUNE_INS_9x16_y      = 75,
    RUNE_INS_9x16_z      = 76,
    RUNE_DIGIT0          = 0,
    RUNE_DIGIT1          = 1,
    RUNE_DIGIT2          = 2,
    RUNE_DIGIT3          = 3,
    RUNE_DIGIT4          = 4,
    RUNE_DIGIT5          = 5,
    RUNE_DIGIT6          = 6,
    RUNE_DIGIT7          = 7,
    RUNE_DIGIT8          = 8,
    RUNE_DIGIT9          = 9,
    RUNE_BLANK           = 10,
};
#endif // _RUNES_H_