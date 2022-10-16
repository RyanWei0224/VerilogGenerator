#ifndef BUTTONS_H
#define BUTTONS_H

#include <QPersistentModelIndex>
#include <QTableWidget>
#include <QToolButton>

class RowButton : public QToolButton{
	Q_OBJECT

public:
	typedef uint8_t id_t;

private:
	QPersistentModelIndex idx;
	id_t id;

public:
	RowButton(const QIcon& icon, int button_id, int row,
			  QTableWidget* table, QWidget* parent = nullptr);
	~RowButton() = default;

private slots:
	void activate();

signals:
	void activated(int row, RowButton::id_t id);
};

#endif // BUTTONS_H
