#include "buttons.h"

#include <QMessageBox>

DelButton::DelButton(const QIcon& icon, int row, QTableWidget* _table, QWidget* parent)
	: QToolButton(parent), idx(_table->model()->index(row, 0)), table(_table){
	setIcon(icon);
	connect(this, &QAbstractButton::clicked, this, &DelButton::activate);
	connect(this, &DelButton::activated, _table, &QTableWidget::removeRow);
}

void DelButton::activate(){
	int row = idx.row();
	if(row >= 0){
		auto ans = QMessageBox::question(this, "Deleting...", "Do you want to delete this from the table?");
		switch(ans){
			case QMessageBox::Yes:
				emit activated(row);
			break;
			default:;
		}
	}else{
		QMessageBox::critical(this, "Error", "Error, can't find row of button!");
	}
}
