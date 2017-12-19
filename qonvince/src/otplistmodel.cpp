#include "otplistmodel.h"

#include <QString>
#include <QStringBuilder>

#include "application.h"
#include "otp.h"
#include "settings.h"


namespace Qonvince {


	OtpListModel::OtpListModel() {
		Q_ASSERT_X(qonvinceApp, __PRETTY_FUNCTION__, "it is not possible to create an OtpListModel without a Qonvince::Application instance");

		// adding/removing Otp is done directly on Application object
		// these connections ensure that the model emits the appropriate
		// signals when this occurs
		// the model itself is read-only
		// TODO Application to propagate Otp changes, model to emit signals
		m_appConnections.push_back(connect(qonvinceApp, qOverload<int, Otp *>(&Application::otpAdded), [this](int index) {
			beginInsertRows({}, index, index);
			endInsertRows();
		}));

		m_appConnections.push_back(connect(qonvinceApp, qOverload<int>(&Application::otpRemoved), [this](int index) {
			beginRemoveRows({}, index, index);
			endRemoveRows();
		}));
	}


	OtpListModel::~OtpListModel() {
		for(const auto & connection : m_appConnections) {
			disconnect(connection);
		};
	}


	QVariant OtpListModel::headerData(int section, Qt::Orientation, int role) const {
		if(0 == section && role == Qt::DisplayRole) {
			return tr("Otp");
		}

		return {};
	}


	QVariant OtpListModel::data(const QModelIndex & index, int role) const {
		auto * otp = qonvinceApp->otp(index.row());

		if(!otp) {
			return {};
		}

		switch(role) {
			case Qt::EditRole:
				return data(index, LabelRole);

			case Qt::DisplayRole:
				return static_cast<QString>(data(index, LabelRole).toString() % QStringLiteral(" ") % data(index, CodeRole).toString());

			case OtpRole: {
				QVariant ret;
				ret.setValue<Otp *>(otp);
				return ret;
			}

			case TypeRole:
				return static_cast<int>(otp->type());

			case NameRole:
				return otp->name();

			case IssuerRole:
				return otp->issuer();

			case LabelRole:
				switch(qonvinceApp->settings().codeLabelDisplayStyle()) {
					case Settings::CodeLabelDisplayStyle::IssuerOnly:
						return otp->issuer();
					case Settings::CodeLabelDisplayStyle::NameOnly:
						return otp->name();
					case Settings::CodeLabelDisplayStyle::IssuerAndName: {
						auto issuer = otp->issuer();
						auto name = otp->name();

						if(issuer.isEmpty()) {
							return name;
						}
						else if(name.isEmpty()) {
							return issuer;
						}
						else {
							return {issuer % QStringLiteral(": ") % name};
						}
					}
				}

			case CodeRole:
				return otp->code();

			case IconRole:
				return otp->icon();

			case TimeToNextCodeRole:
				return otp->timeToNextCode();

			case TimeSinceLastCodeRole:
				return otp->timeSinceLastCode();

			case IntervalRole:
				return otp->interval();

			case CountdownRole:
				return otp->counter();

			case RevealOnDemandRole:
				return otp->revealOnDemand();
		}

		return {};
	}


	int OtpListModel::rowCount(const QModelIndex &) const {
		return qonvinceApp->otpCount();
	}
}  // namespace Qonvince
