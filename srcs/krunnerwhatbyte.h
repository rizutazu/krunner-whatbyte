#ifndef KRUNNERWHATBYTE_H
#define KRUNNERWHATBYTE_H
#include <KRunner/AbstractRunner>
#include <KRunner/Action>

class KRunnerWhatByte : public KRunner::AbstractRunner
{
    Q_OBJECT

public:
    KRunnerWhatByte(QObject *parent, const KPluginMetaData &metaData);
    ~KRunnerWhatByte() override;
    void match(KRunner::RunnerContext &context) override;
    void run(const KRunner::RunnerContext &context, const KRunner::QueryMatch &match) override;
private:
    static KRunner::Action copyAction;
    void handleWhat(KRunner::RunnerContext &context, const QString &arg);
    void handlePrint(KRunner::RunnerContext &context, const QString &arg);
};


#endif //KRUNNERWHATBYTE_H
