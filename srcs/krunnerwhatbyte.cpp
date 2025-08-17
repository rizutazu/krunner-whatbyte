#include "krunnerwhatbyte.h"
#include "sm.h"
#include "helper.h"
#include <QIcon>
#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QTextBoundaryFinder>

KRunner::Action KRunnerWhatByte::copyAsFileAction = KRunner::Action(QStringLiteral("copyFile"), QStringLiteral("application-octet-stream"), QStringLiteral("Copy as a file"));
KRunner::Action KRunnerWhatByte::copyAsCArrayAction = KRunner::Action(QStringLiteral("copyArray"), QStringLiteral("edit-copy"), QStringLiteral("Copy as a C array"));

KRunnerWhatByte::KRunnerWhatByte(QObject *parent, const KPluginMetaData &metaData)
        : AbstractRunner(parent, metaData) {
}

KRunnerWhatByte::~KRunnerWhatByte() = default;

void KRunnerWhatByte::match(KRunner::RunnerContext &context) {
    const QString &term = context.query();
    const qint64 idx = term.indexOf(QStringLiteral(" "));
    if (idx != -1) {
        const QString &command = term.first(idx);
        const QString &arg = term.mid(idx + 1);
        if (command == QStringLiteral("what")) { // what byte is in my clipboard?
            handleWhat(context, arg);
        }
        if (command == QStringLiteral("print")) { // construct text by byte
            handlePrint(context, arg);
        }
    }
}

void KRunnerWhatByte::run(const KRunner::RunnerContext &context, const KRunner::QueryMatch &match) {
    Q_UNUSED(context);
    if (match.selectedAction() == copyAsFileAction) {
        const QByteArray ba = match.data().toByteArray();
        QMimeData *mime = new QMimeData;
        mime->setData(QStringLiteral("application/octet-stream"), ba);
        QApplication::clipboard()->setMimeData(mime);   // take ownership
    } else if (match.selectedAction() == copyAsCArrayAction) {
        const QString arg = match.data().toString();
        QApplication::clipboard()->setText(stringToHexRepresentation(arg));
    } else {
        QApplication::clipboard()->setText(match.text());
    }

}

void KRunnerWhatByte::handleWhat(KRunner::RunnerContext &context, const QString &arg) {
    QString matchText;
    QTextBoundaryFinder finder(QTextBoundaryFinder::Grapheme, arg);
    qsizetype next, prev = 0, charCount = 0, zeroWidthCount = 0;

    // find each unicode character
    while ((next = finder.toNextBoundary()) != -1) {

        // `'char' =>`
        const QString word = arg.mid(prev, next - prev);
        matchText += QStringLiteral("'") + word + QStringLiteral("' => ");

        // `{each byte}`
        matchText += stringToHexRepresentation(word);

        // `, `
        matchText += QStringLiteral(", ");

        prev = next;
        charCount++;
        if (isZeroWidthUnicode(word)) {
            zeroWidthCount++;
        }
    }

    if (charCount != 0) {
        // remove last comma `, `
        matchText = matchText.first(matchText.length() - 2);
    }

    KRunner::QueryMatch matchRep(this);
    matchRep.setRelevance(1);
    matchRep.setText(matchText);
    matchRep.setIcon(QIcon::fromTheme(QStringLiteral("accessories-character-map")));
    matchRep.setMultiLine(true);
    matchRep.setMatchCategory(QStringLiteral("what byte"));
    matchRep.addAction(copyAsCArrayAction);
    matchRep.setData(arg);
    context.addMatch(matchRep);

    KRunner::QueryMatch matchStat(this);

    matchText.clear();
    matchText += QString::number(charCount) + QStringLiteral(" character(s), ");
    matchText += QString::number(arg.toStdString().length()) + QStringLiteral(" byte(s)");
    if (zeroWidthCount != 0) {
        matchText += QStringLiteral(", ")
            + QString::number(zeroWidthCount) + QStringLiteral(" zero-width character(s) detected");
    }

    matchStat.setRelevance(0);
    matchStat.setText(matchText);
    matchStat.setIcon(QIcon::fromTheme(QStringLiteral("accessories-character-map")));
    matchStat.setMultiLine(true);
    matchStat.setMatchCategory(QStringLiteral("what byte"));
    context.addMatch(matchStat);
}

void KRunnerWhatByte::handlePrint(KRunner::RunnerContext &context, const QString &arg) {
    const QString &trimmed = arg.trimmed();
    QList<int> result;
    if (trimmed.startsWith(QStringLiteral("{"))) {
        parseValueStringArray(trimmed, result);
    } else {
        parseEscapeStringArray(trimmed, result);
    }
    if (!result.isEmpty()) {
        QByteArray b;   // actual constructed data
        std::string matchText;  // for display only
        for (const int i : result) {
            const char c = (char)i;
            b.append(c);
            matchText += c;
        }
        KRunner::QueryMatch match(this);
        match.setRelevance(1);
        match.setText(QString::fromStdString(matchText));
        match.setData(b);
        match.setIcon(QIcon::fromTheme(QStringLiteral("accessories-character-map")));
        match.setMultiLine(true);
        match.addAction(copyAsFileAction);
        match.setMatchCategory(QStringLiteral("constructed text"));
        context.addMatch(match);
    }
}

K_PLUGIN_CLASS_WITH_JSON(KRunnerWhatByte, "krunnerwhatbyte.json")

#include "krunnerwhatbyte.moc"