#include "sm.h"

// parse {0x12, 12, } array
// state      | input | next state | action
// init       | space | init
// init       | {     | next_value
    // next_value | space | next_value
    // next_value | 1-9   | dec_value     | v = v * 10 + input
        // dec_value | 0-9   | dec_value | v = v * 10 + input
        // dec_value | space | value_fin | push v
        // dec_value | ,     | next_value
        // dec_value | }     | end       | push v
    // next_value | 0   | hex0
    // hex0       | x   | hex_x
    // hex_x      | 0-f | hex_value
        // hex_value | 0~f   | hex_value | v = v * 16 + v
        // hex_value | space | value_fin | push v
        // hex_value | ,     | next_value
        // hex_value | }     | end       | push v
    // value_fin | space | value_fin
    // value_fin | ,     | next_value
// value_fin  | }     | end | return
// next_value | }     | end | return
void parseValueStringArray(const QString &input, QList<int> &result) {

    enum State {
        INIT,       // init state
        NEXT_VALUE, // ready to read next dec/hex
        DEC_VALUE,  // now parsing dec value (leading char is not 0)
        HEX_0,       // is this hex value? (leading char == '0')
        HEX_X,      // is this hex value? (leading char == 'x')
        HEX_VALUE,  // now parsing hex value
        VALUE_FIN,  // complete value parsing, before reaching separator ','
        ERROR,      // err
        FIN         // complete
    };

    result.clear();

    State current = INIT;
    int value = 0;

    for (const char c : input.toStdString()) {
        switch (current) {
            case INIT: {
                if (c == '{') {
                    current = NEXT_VALUE;
                    break;
                }
                if (c != ' ') {
                    current = ERROR;
                    break;
                }
                break;
            }
            case NEXT_VALUE: {
                if (c == ' ') {
                    continue;
                }
                if (c == '0') {
                    current = HEX_0;
                    break;
                }
                if (c >= '1' && c <= '9') {
                    current = DEC_VALUE;
                    value = c - '0';
                    break;
                }
                if (c == '}') {
                    current = FIN;
                    break;
                }
                current = ERROR;
                break;
            }
            case HEX_0: {
                if (c == 'x') {
                    current = HEX_X;
                    break;
                }
                current = ERROR;
                break;
            }
            case HEX_X: {
                if (c >= '0' && c <= '9') {
                    current = HEX_VALUE;
                    value = c - '0';
                    break;
                }
                if (c >= 'a' && c <= 'f') {
                    current = HEX_VALUE;
                    value = c - 'a' + 10;
                    break;
                }
                if (c >= 'A' && c <= 'F') {
                    current = HEX_VALUE;
                    value = c - 'A' + 10;
                    break;
                }
                current = ERROR;
                break;
            }
            case HEX_VALUE: {
                if (c >= '0' && c <= '9') {
                    value = value * 16 + (c - '0');
                    if (value > 255) {
                        current = ERROR;
                    }
                    break;
                }
                if (c >= 'a' && c <= 'f') {
                    value = value * 16 + (c - 'a' + 10);
                    if (value > 255) {
                        current = ERROR;
                    }
                    break;
                }
                if (c >= 'A' && c <= 'F') {
                    value = value * 16 + (c - 'A' + 10);
                    if (value > 255) {
                        current = ERROR;
                    }
                    break;
                }
                if (c == ' ') {
                    current = VALUE_FIN;
                    result.push_back(value);
                    value = 0;
                    break;
                }
                if (c == ',') {
                    current = NEXT_VALUE;
                    result.push_back(value);
                    value = 0;
                    break;
                }
                if (c == '}') {
                    current = FIN;
                    result.push_back(value);
                    value = 0;
                    break;
                }
                current = ERROR;
                break;
            }
            case DEC_VALUE: {
                if (c >= '0' && c <= '9') {
                    value = value * 10 + (c - '0');
                    if (value > 255) {
                        current = ERROR;
                    }
                    break;
                }
                if (c == ' ') {
                    current = VALUE_FIN;
                    result.push_back(value);
                    value = 0;
                    break;
                }
                if (c == ',') {
                    current = NEXT_VALUE;
                    result.push_back(value);
                    value = 0;
                    break;
                }
                if (c == '}') {
                    current = FIN;
                    result.push_back(value);
                    value = 0;
                    break;
                }
                current = ERROR;
                break;
            }
            case VALUE_FIN: {
                if (c == ' ') {
                    continue;
                }
                if (c == '}') {
                    current = FIN;
                    break;
                }
                if (c == ',') {
                    current = NEXT_VALUE;
                    break;
                }
                current = ERROR;
                break;
            }
            case ERROR: {
                result.clear();
                return;
            }
            case FIN: {
                return;
            }
        }
    }
    if (current != FIN) {
        result.clear();
    }
}

// parse \x12\x34 array, space allowed
// init      | space | init
// init      | \     | esc_slash
// esc_slash | x     | esc_x
// esc_x     | 0-f   | esc_v0 | v = v
// esc_v0    | 0-f   | init   | v = v * 16 + v, push
// esc_v0    | \     | esc_slash | push
// esc_v0    | space | init   | push
// prepend ' ' first, final state must be init
void parseEscapeStringArray(const QString &input, QList<int> &result) {
    enum State {
        INIT,   // init state
        ESC_SLASH, // already read '\', ready to read 'x'
        ESC_X,  // already read 'x', ready to read first digit
        ESC_V0, // first digit ok, ready to read second digit
        ERROR
    };

    auto s = input.toStdString();
    s += ' ';
    result.clear();

    State current = INIT;
    int value = 0;
    for (const char c : s) {
        switch (current) {
            case INIT: {
                if (c == ' ') {
                    continue;
                }
                if (c == '\\') {
                    current = ESC_SLASH;
                    break;
                }
                current = ERROR;
                break;
            }
            case ESC_SLASH: {
                if (c == 'x') {
                    current = ESC_X;
                    break;
                }
                current = ERROR;
                break;
            }
            case ESC_X: {
                if (c >= '0' && c <= '9') {
                    current = ESC_V0;
                    value = c - '0';
                    break;
                }
                if (c >= 'a' && c <= 'f') {
                    current = ESC_V0;
                    value = c - 'a' + 10;
                    break;
                }
                if (c >= 'A' && c <= 'F') {
                    current = ESC_V0;
                    value = c - 'A' + 10;
                    break;
                }
                current = ERROR;
                break;
            }
            case ESC_V0: {
                if (c >= '0' && c <= '9') {
                    current = INIT;
                    value = value * 16 + (c - '0');
                    result.push_back(value);
                    value = 0;
                    break;
                }
                if (c >= 'a' && c <= 'f') {
                    current = INIT;
                    value = value * 16 + (c - 'a' + 10);
                    result.push_back(value);
                    value = 0;
                    break;
                }
                if (c >= 'A' && c <= 'F') {
                    current = INIT;
                    value = value * 16 + (c - 'A' + 10);
                    result.push_back(value);
                    value = 0;
                    break;
                }
                if (c == ' ') {
                    current = INIT;
                    result.push_back(value);
                    value = 0;
                    break;
                }
                if (c == '\\') {
                    current = ESC_SLASH;
                    result.push_back(value);
                    value = 0;
                    break;
                }
                current = ERROR;
                break;
            }
            case ERROR: {
                result.clear();
                return;
            }
        }
    }
    if (current != INIT) {
        result.clear();
    }
}
