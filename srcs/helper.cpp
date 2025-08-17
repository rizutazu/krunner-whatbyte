#include "helper.h"
#include <QStringList>

QString stringToHexRepresentation(const QString &str) {
    static const char hex[] = {
        '0', '1', '2', '3',
        '4', '5', '6', '7',
        '8', '9', 'a', 'b',
        'c', 'd', 'e', 'f'
    };
    QString result;
    QByteArray b = str.toUtf8();
    for (qsizetype i = 0; i < b.length(); i++) {
        const char cc = b[i];
        const char high = hex[(cc & 0xf0) >> 4];
        const char low = hex[cc & 0xf];

        result += QStringLiteral("0x");
        if (high != '0') {
            result += QChar::fromLatin1(high);
        }
        result += QChar::fromLatin1(low);

        if (i != b.length() - 1) {
            result += QStringLiteral(", ");
        }
    }
    if (b.length() > 0) {
        result = QStringLiteral("{") + result + QStringLiteral("}");
    }
    return result;
}

bool isZeroWidthUnicode(const QString &str) {
     static const QStringList zeroWidthUnicodes = {
        QStringLiteral("​"), // zwsp
        QStringLiteral("﻿"), // bom
        QStringLiteral("⁠"), // wj
        QStringLiteral("᠎") // mvs
    };

    // joiners need special care, since QTextBoundaryFinder::Grapheme do not split them.
    static const QStringList zeroWidthJoiners = {
        QStringLiteral("‌"), // zwnj
        QStringLiteral("‍"), // zwj
    };

    if (zeroWidthUnicodes.contains(str)) {
        return true;
    }

    for (const auto &s : zeroWidthJoiners) {
        if (str.contains(s)) {
            return true;
        }
    }
    return false;
}
