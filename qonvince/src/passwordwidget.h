#ifndef QONVINCE_PASWORDWIDGET_H
#define QONVINCE_PASWORDWIDGET_H

#include <memory>

#include <QWidget>

namespace Qonvince {

	namespace Ui {
		class PasswordWidget;
	}

	class PasswordWidget
	: public QWidget {
		Q_OBJECT

	public:
		explicit PasswordWidget(QWidget * parent = nullptr);
		~PasswordWidget() override;

		QString password() const;

	Q_SIGNALS:
		void passwordChanged(const QString & pwd);

	public Q_SLOTS:
		void setPassword(const QString & pwd);

	private:
		std::unique_ptr<Ui::PasswordWidget> m_ui;
	};

}  // namespace Qonvince

#endif  // QONVINCE_PASWORDWIDGET_H
