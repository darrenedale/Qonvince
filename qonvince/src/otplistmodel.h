#ifndef QONVINCE_OTPLISTMODEL_H
#define QONVINCE_OTPLISTMODEL_H

#include <QAbstractListModel>

namespace Qonvince {

	class OtpListModel
	: public QAbstractListModel {
		public:
			static constexpr const int TypeRole = Qt::UserRole + 1;
			static constexpr const int NameRole = Qt::UserRole + 2;
			static constexpr const int IssuerRole = Qt::UserRole + 3;
			static constexpr const int LabelRole = Qt::UserRole + 4;
			static constexpr const int CodeRole = Qt::UserRole + 5;
			static constexpr const int IconRole = Qt::UserRole + 6;
			static constexpr const int TimeToNextCodeRole = Qt::UserRole + 7;
			static constexpr const int TimeSinceLastCodeRole = Qt::UserRole + 8;
			static constexpr const int IntervalRole = Qt::UserRole + 9;
			static constexpr const int CountdownRole = Qt::UserRole + 10;
			static constexpr const int RevealOnDemandRole = Qt::UserRole + 11;

			OtpListModel();

			QVariant headerData(int, Qt::Orientation, int) const override;
			QVariant data(const QModelIndex &, int role) const override;
			int rowCount(const QModelIndex &) const override;
	};

}
#endif // QONVINCE_OTPLISTMODEL_H
