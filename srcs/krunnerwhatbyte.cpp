#include "krunnerwhatbyte.h"
#include "sm.h"
#include <QIcon>
#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QTextBoundaryFinder>

KRunner::Action KRunnerWhatByte::copyAction = KRunner::Action(QStringLiteral("copy"), QStringLiteral("application-octet-stream"), QStringLiteral("copy as file"));

KRunnerWhatByte::KRunnerWhatByte(QObject *parent, const KPluginMetaData &metaData)
        : AbstractRunner(parent, metaData) {
}

KRunnerWhatByte::~KRunnerWhatByte() = default;
static const char hex[] = {
    '0', '1', '2', '3',
    '4', '5', '6', '7',
    '8', '9', 'a', 'b',
    'c', 'd', 'e', 'f'
};
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
    if (match.selectedAction() == copyAction) {
        const QByteArray ba = match.data().toByteArray();
        QMimeData *mime = new QMimeData;
        mime->setData(QStringLiteral("application/octet-stream"), ba);
        QApplication::clipboard()->setMimeData(mime);
    }
    QApplication::clipboard()->setText(match.text());
}

void KRunnerWhatByte::handleWhat(KRunner::RunnerContext &context, const QString &arg) {
    QString matchText;
    QTextBoundaryFinder finder(QTextBoundaryFinder::Grapheme, arg);
    qsizetype next, prev = 0, charCount = 0;

    // find each unicode character
    while ((next = finder.toNextBoundary()) != -1) {
        // 'char' => {
        matchText += QStringLiteral("'") + arg.mid(prev, next - prev) + QStringLiteral("' => {");
        // each byte
        QByteArray b = arg.mid(prev, next - prev).toUtf8();
        for (qsizetype i = 0; i < b.length(); i++) {
            const char cc = b[i];
            const char high = hex[(cc & 0xf0) >> 4];
            const char low = hex[cc & 0xf];

            matchText += QStringLiteral("0x");
            if (high != '0') {
                matchText += QChar::fromLatin1(high);
            }
            matchText += QChar::fromLatin1(low);

            if (i != b.length() - 1) {
                matchText += QStringLiteral(", ");
            }
        }
        // }
        matchText += QStringLiteral("}, ");

        prev = next;
        charCount++;
    }

    if (charCount != 0) {
        // remove last comma `, `
        matchText = matchText.first(matchText.length() - 2);
    }

    KRunner::QueryMatch match(this);
    match.setRelevance(1);
    match.setText(matchText);
    match.setIcon(QIcon::fromTheme(QStringLiteral("accessories-character-map")));
    match.setMultiLine(true);
    match.setMatchCategory(QStringLiteral("what byte"));
    context.addMatch(match);

    matchText.clear();
    matchText += QString::number(charCount) + QStringLiteral(" character(s), ");
    matchText += QString::number(arg.toStdString().length()) + QStringLiteral(" byte(s)");
    match.setText(matchText);
    match.setRelevance(0);
    context.addMatch(match);
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
        match.addAction(copyAction);
        match.setMatchCategory(QStringLiteral("constructed text"));
        context.addMatch(match);
    }
}

K_PLUGIN_CLASS_WITH_JSON(KRunnerWhatByte, "krunnerwhatbyte.json")

#include "krunnerwhatbyte.moc"