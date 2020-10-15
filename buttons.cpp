#include "buttons.h"
#include <QMessageBox>

DelButton::DelButton(QIcon& icon, QTableWidgetItem *item, QTableWidget *table, QWidget* parent)
	: QToolButton (parent){
	setIcon(icon);
	_item=item;
	_table=table;
	connect(this,&QAbstractButton::clicked,this,&DelButton::_activate);
	connect(this,&DelButton::activate,_table,&QTableWidget::removeRow);
	//connect(this,SIGNAL(activate(QTableWidgetItem *)),window,SIGNAL(removeRow(QTableWidgetItem *)));
}

DelButton::~DelButton(){

}

void DelButton::_activate(){
	int row=_table->row(_item);
	if(row>=0){
		switch(QMessageBox::question(this,"Deleting...","Do you want to delete this from the table?")){
			case QMessageBox::Yes:
				emit activate(row);
			break;
			default:;
		}
	}else{
		fprintf(stderr,"Error, can't find row of button!\n");
		fflush(stderr);
	}
}
