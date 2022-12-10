#ifndef QONVINCE_CHANGEPASSPHRASEDIALOGUE_H
#define QONVINCE_CHANGEPASSPHRASEDIALOGUE_H

#include <memory>

#include <QDialog>
#include <QString>

class QWidget;

namespace Qonvince
{
    namespace Ui
    {
        class ChangePassphraseDialogue;
    }

    class ChangePassphraseDialogue
            : public QDialog
    {
    Q_OBJECT

    public:
        explicit ChangePassphraseDialogue(QWidget * parent = nullptr);
        explicit ChangePassphraseDialogue(const QString & msg, QWidget * parent = nullptr);
        ~ChangePassphraseDialogue() override;

        [[nodiscard]] QString message() const;
        void setMessage(const QString & msg);
        [[nodiscard]] QString currentPassphrase() const;
        void setCurrentPassphrase(const QString & phrase);
        [[nodiscard]] QString newPassphrase() const;
        void setNewPassphrase(const QString & phrase);

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
        void currentPassphraseChanged(QString);
        void newPassphraseChanged(QString);

    private:
        std::unique_ptr<Ui::ChangePassphraseDialogue> m_ui;
    };
}  // namespace Qonvince

#endif  // QONVINCE_CHANGEPASSPHRASEDIALOGUE_H
