/*
 * Copyright 2015 - 2022 Darren Edale
 *
 * This file is part of Qonvince.
 *
 * Qonvince is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Qonvince is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Qonvince. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef QONVINCE_OTPEDITOR_H
#define QONVINCE_OTPEDITOR_H

#include <memory>

#include <QWidget>

#include "otp.h"

class QCloseEvent;
class QDragEnterEvent;
class QDropEvent;

namespace Qonvince
{
    namespace Ui
    {
        class OtpEditor;
    }

    class OtpEditor : public QWidget
    {
    Q_OBJECT

    public:
        explicit OtpEditor(QWidget * = nullptr);

        explicit OtpEditor(Otp *, QWidget * = nullptr);

        ~OtpEditor() override;

        [[nodiscard]] QString name() const;
        [[nodiscard]] QString issuer() const;
        [[nodiscard]] OtpType type() const;
        [[nodiscard]] bool revealOnDemand() const;

        [[nodiscard]] inline Otp * otp() const
        {
            return m_otp;
        }

    public Q_SLOTS:

        void setOtp(Otp *);
        void setName(const QString &);
        void setIssuer(const QString &);
        void setType(OtpType);
        void setRevealOnDemand(bool);
        void chooseIcon();
        void readBarcode();
        void readBarcode(const QString &);
        bool createBarcode();
        bool createBarcode(const QString &);

    protected:
        void dragEnterEvent(QDragEnterEvent *) override;
        void dropEvent(QDropEvent *) override;

    Q_SIGNALS:

        void typeChanged(OtpType);
        void issuerChanged(QString);
        void nameChanged(QString);
        void seedChanged(QString);
        void iconChanged(QIcon);
        void digitsChanged(int);
        void displayPluginNameChanged(QString);
        void revealOnDemandChanged(bool);
        void counterChanged(quint64);
        void durationChanged(int);
        void intervalChanged(int);
        void baseTimeChanged(QDateTime);
        void baseTimeChangedInSeconds(qint64);
        void closing();

    private Q_SLOTS:

        void updateWindowTitle();
        void updateHeading();
        void onCodeSeedEditingFinished();
        void setCodeBaseTimeFromWidget();
        void setCounter(quint64);
        void resetCounter();
        void seedWidgetTextEdited();
        void onDisplayPluginChanged();
        void onIconSelected(const QIcon &);
        void onIconCleared();

    private:
        std::unique_ptr<Ui::OtpEditor> m_ui;
        Otp * m_otp;
        QByteArray m_originalSeed;
    };
}  // namespace Qonvince

#endif  // QONVINCE_OTPEDITOR_H
