#include "bincheck.h"

#include <QHBoxLayout>

BinCheck::BinCheck(const QString &no, const QString &yes,
				   bool canSelectNull, QWidget *parent)
	: QGroupBox(parent), selNull(canSelectNull), pauseSig(false){
	box0 = new QRadioButton(no, this);
	box1 = new QRadioButton(yes, this);

	QHBoxLayout* layout = new QHBoxLayout(this);
	layout->addWidget(box0);
	layout->addWidget(box1);
	layout->setSpacing(6);
	layout->setContentsMargins(6, 6, 6, 6);
	setLayout(layout);

	if(selNull){
		box0->setAutoExclusive(false);
		connect(box0, &QAbstractButton::toggled, this, &BinCheck::setBox0);
		connect(box1, &QAbstractButton::toggled, this, &BinCheck::setBox1);
	}else{
		box0->setChecked(true);
	}
}

void BinCheck::set(bool yes){
	if(yes){
		box1->setChecked(true);
	}else{
		box0->setChecked(true);
	}
}

void BinCheck::reset(){
	if(selNull){
		box0->setChecked(false);
		box1->setChecked(false);
	}
}

void BinCheck::set(Stat stat){
	switch(stat){
	case NO_CHECK:
		reset();
		break;
	case CHECK_0:
		set(false);
		break;
	case CHECK_1:
		set(true);
		break;
	}
}

BinCheck::Stat BinCheck::get(){
	if(box1->isChecked()){
		return CHECK_1;
	}else if(box0->isChecked()){
		return CHECK_0;
	}else{
		return NO_CHECK;
	}
}

void BinCheck::setBox0(bool checked){
	if(pauseSig) return;
	if(checked){
		pauseSig = true;
		box1->setChecked(false);
		pauseSig = false;
	}
	emit isSet(checked);
}

void BinCheck::setBox1(bool checked){
	if(pauseSig) return;
	if(checked){
		pauseSig = true;
		box0->setChecked(false);
		pauseSig = false;
	}
	emit isSet(checked);
}
