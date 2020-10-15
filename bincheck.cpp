#include "bincheck.h"

BinCheck::BinCheck(const QString &no,
				   const QString &yes, bool canSelectNull,
				   QWidget *parent) : QGroupBox(parent){
	box0=new QRadioButton(no,this);
	box1=new QRadioButton(yes,this);
	layout=new QHBoxLayout(this);
	layout->addWidget(box0);
	layout->addWidget(box1);
	layout->setSpacing(6);
	layout->setContentsMargins(6,6,6,6);
	setLayout(layout);
	if(canSelectNull){
		box0->setAutoExclusive(false);
		connect(box0,&QAbstractButton::toggled,this,&BinCheck::setBox0);
		connect(box1,&QAbstractButton::toggled,this,&BinCheck::setBox1);
	}else{
		box0->setChecked(true);
	}
	_selNull=canSelectNull;
}

void BinCheck::set(bool yes){
	if(yes)
		box1->setChecked(true);
	else
		box0->setChecked(true);
}

void BinCheck::reset(){
	if(_selNull){
		box0->setChecked(false);
		box1->setChecked(false);
	}
}

int BinCheck::get(){
	if(box1->isChecked())
		return 1;
	else if(box0->isChecked())
		return 0;
	else
		return -1;
}

BinCheck::~BinCheck(){
	/*printf("Clear!\n");
	fflush(stdout);*/
}

void BinCheck::setBox0(bool checked){
	if(checked)
		box1->setChecked(false);
}

void BinCheck::setBox1(bool checked){
	if(checked)
		box0->setChecked(false);
}
