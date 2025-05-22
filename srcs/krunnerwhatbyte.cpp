#include "krunnerwhatbyte.h"
#include "sm.h"
#include <QIcon>
#include <QApplication>
#include <QClipboard>
#include <QMimeData>

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
    QString s;
    for (long long int i = 0; i < arg.length(); i++) {

        const QChar cc = arg.data()[i];
        s += QStringLiteral("'") + cc + QStringLiteral("' => {");
        const QByteArray b = QString(cc).toUtf8();
        for (long long int j = 0; j < b.length(); j++) {
            const char c = b[j];
            const char high = hex[(c & 0xf0) >> 4];
            const char low = hex[c & 0xf];
            s += QStringLiteral("0x");
            if (high != '0') {
                s += QChar::fromLatin1(high);
            }
            s += QChar::fromLatin1(low);
            if (j != b.length() - 1) {
                s += QStringLiteral(", ");
            }
        }

        if (i != arg.length() - 1) {
            s += QStringLiteral("}, \n");
        } else {
            s += QStringLiteral("}");
        }
    }

    // QString s = QString::number(arg.toStdString().length()) + QStringLiteral(" bytes");
    KRunner::QueryMatch match(this);
    match.setRelevance(1);
    match.setText(s);
    match.setIcon(QIcon::fromTheme(QStringLiteral("accessories-character-map")));
    match.setMultiLine(true);
    match.setMatchCategory(QStringLiteral("what byte"));
    context.addMatch(match);

    s.clear();
    s += QString::number(arg.length()) + QStringLiteral(" character(s), ");
    s += QString::number(arg.toStdString().length()) + QStringLiteral(" byte(s)");
    match.setText(s);
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
        std::string s;  // for display only
        for (const int i : result) {
            const char c = (char)i;
            b.append(c);
            s += c;
        }
        KRunner::QueryMatch match(this);
        match.setRelevance(1);
        match.setText(QString::fromStdString(s));
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