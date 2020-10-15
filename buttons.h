#ifndef BUTTONS_H
#define BUTTONS_H

#include <QWidget>
#include <QToolButton>
#include <QTableWidget>
#include <QTableWidgetItem>

class DelButton : public QToolButton
{
	Q_OBJECT
private:
	QTableWidgetItem *_item;
	QTableWidget *_table;

public:
	DelButton(QIcon &icon, QTableWidgetItem *item,
			  QTableWidget *table, QWidget *parent=nullptr);
	~DelButton();

private slots:
	void _activate();

signals:
	void activate(int row);

};

#endif // BUTTONS_H
