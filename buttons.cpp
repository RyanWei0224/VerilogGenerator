#include "buttons.h"

#include <QMessageBox>

RowButton::RowButton(const QIcon& icon, int button_id, int row, QTableWidget* table, QWidget* parent)
	: QToolButton(parent), idx(table->model()->index(row, 0)), id(button_id){
	setIcon(icon);
	connect(this, &QAbstractButton::clicked, this, &RowButton::activate);
}

void RowButton::activate(){
	int row = idx.row();
	if(row >= 0){
		emit activated(row, id);
	}else{
		QMessageBox::critical(this, "Error", "Error, can't find row of button!");
	}
}
