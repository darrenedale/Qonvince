#ifndef QONVINCE_PASSPHRASEDIALOGUE_H
#define QONVINCE_PASSPHRASEDIALOGUE_H

#include <memory>

#include <QDialog>
#include <QString>

class QWidget;

namespace Qonvince
{
    namespace Ui
    {
        class PassphraseDialogue;
    }

    class PassphraseDialogue
            : public QDialog
    {
    Q_OBJECT

    public:
        explicit PassphraseDialogue(QWidget * parent = nullptr);
        explicit PassphraseDialogue(const QString & msg, QWidget * parent = nullptr);
        ~PassphraseDialogue() override;

        [[nodiscard]] QString message() const;
        void setMessage(const QString & msg);
        [[nodiscard]] QString passphrase() const;
        void setPassphrase(const QString & pwd);

    public Q_SLOTS:
        inline void showMessage()
        {
            setMessageVisible(true);
        }

        inline void hideMessage()
        {
            setMessageVisible(false);
        }

        void setMessageVisible(bool);

    Q_SIGNALS:
        void passphraseChanged(QString);

    private:
        std::unique_ptr<Ui::PassphraseDialogue> m_ui;
    };
}  // namespace Qonvince

#endif  // QONVINCE_PASSPHRASEDIALOGUE_H
