#ifndef BUTTONS_H
#define BUTTONS_H

#include <QPersistentModelIndex>
#include <QTableWidget>
#include <QToolButton>

class DelButton : public QToolButton{
	Q_OBJECT

private:
	QPersistentModelIndex idx;
	QTableWidget* table;

public:
	DelButton(const QIcon& icon, int row,
			  QTableWidget* _table, QWidget* parent = nullptr);
	~DelButton() = default;

private slots:
	void activate();

signals:
	void activated(int row);
};

#endif // BUTTONS_H
