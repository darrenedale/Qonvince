#ifndef QONVINCE_OTPCODEITEMDELEGATE_H
#define QONVINCE_OTPCODEITEMDELEGATE_H


#include <QStyledItemDelegate>


namespace Qonvince {

	class OtpCodeItemDelegate
	: public QStyledItemDelegate {
	public:
		OtpCodeItemDelegate();

		virtual void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const;
		virtual QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const;
	};

}  // namespace Qonvince

#endif  // QONVINCE_OTPCODEITEMDELEGATE_H
