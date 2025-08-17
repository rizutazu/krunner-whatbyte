#ifndef SM_H
#define SM_H

// sm => state machine
#include <QString>
#include <QList>

// parse {0x12, 12, } array
void parseValueStringArray(const QString &input, QList<int> &result);

// parse \x12\x34 array, space allowed
void parseEscapeStringArray(const QString &input, QList<int> &result);
#endif //SM_H
