#include "otplistmodel.h"

#include <QString>
#include <QStringBuilder>

#include "application.h"
#include "otp.h"
#include "settings.h"


namespace Qonvince {


	OtpListModel::OtpListModel() = default;


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
