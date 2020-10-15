<<<<<<< HEAD
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "bincheck.h"
#include "buttons.h"
#include <QToolButton>
#include <QMessageBox>
#include <QFileDialog>
#include <QString>
#include <QStringList>
#include <QDir>
#include <QDate>
#include <QDataStream>
#include <QVector>

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	ui->tableWidget->horizontalHeader()->setSectionResizeMode(0,QHeaderView::Stretch);
	ui->tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	delIcon=QIcon(":/pics/delete.jpg");
	addIcon=QIcon(":/pics/add.jpg");
	auto button=new QToolButton();
	button->setIcon(addIcon);
	ui->tableWidget->setCellWidget(0,5,button);
	connect(button,&QAbstractButton::clicked,this,&MainWindow::_addRow);
	curPath=QDir::currentPath();
	ui->curDir->setText(curPath);
	if(!readLog()){
		addRow(false,"clk");
	}
}

MainWindow::~MainWindow(){
	saveLog();
	delete ui;
}

void MainWindow::_addRow(){
	addRow();
}

int MainWindow::buttonStat(int row, int col){
	return static_cast<BinCheck*>(ui->tableWidget->cellWidget(row,col))->get();
}

QTableWidget *MainWindow::table(){
	return ui->tableWidget;
}

void MainWindow::addRow(bool useLast, const QString &name, int io, int wire, int sign, const QString &lenm1){
	int row=ui->tableWidget->rowCount()-1;
	ui->tableWidget->insertRow(row);

	BinCheck* check;
	int stat;
	check=new BinCheck("in","out",true);
	ui->tableWidget->setCellWidget(row,1,check);
	if(row>0&&useLast)
		stat=buttonStat(row-1,1);
	else
		stat=io;

	if(stat>=0)
		check->set(stat>0);
	else
		check->reset();

	check=new BinCheck("wire","reg");
	ui->tableWidget->setCellWidget(row,2,check);
	stat=(row>0&&useLast)?buttonStat(row-1,2):wire;
	check->set(stat>0);

	check=new BinCheck("uns","sign");
	ui->tableWidget->setCellWidget(row,3,check);
	stat=(row>0&&useLast)?buttonStat(row-1,3):sign;
	check->set(stat>0);

	table()->setItem(row,0,new QTableWidgetItem(name));
	table()->setItem(row,4,new QTableWidgetItem(lenm1));

	DelButton* button=new DelButton(delIcon,
									table()->item(row,4),table());
	ui->tableWidget->setCellWidget(row,5,button);

	//ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

void MainWindow::on_toolButton_clicked(){
	QString path=QDir::toNativeSeparators(
		QFileDialog::getExistingDirectory(this,tr("Directory to save"),
		ui->curDir->text()));
	ui->curDir->setText(path);
}

bool MainWindow::openFile(const QString& fileName){
	f.setFileName(fileName);
	if(!f.open(QIODevice::WriteOnly|QIODevice::Text)){
		QString str="During opening of\n";
		str+=fileName;
		str+="\nAn error occured:\n";
		str+=f.errorString();
		str+="\nPlease try again.";
		QMessageBox::critical(this,"Error",str);
		return false;
	}
	s.setDevice(&f);
	return true;
}

void MainWindow::closeFile(){
	s.setDevice(nullptr);
	f.close();
}

void MainWindow::printIO(bool &p, const QString &name, const QString &pad){
	if(p){
		s<<','<<endl<<pad<<name;
	}else{
		p=true;
		s<<name;
	}

}

bool MainWindow::readLog(){
	QDir::setCurrent(curPath);
	f.setFileName("log.dat");
	if(!f.open(QIODevice::ReadOnly))
		return false;
	QDataStream d(&f);
	int j;
	QStringList l;
	QVector<myPair> ll;
	d>>j>>l>>ll;
	if(d.status()!=QDataStream::Ok)
		return false;
	if(l.length()!=8)
		return false;
	if(j>=8||j<0)
		return false;
	for(auto it:ll){
		if(it.first.second<-1||it.first.second>1)
			return false;
		if(it.second.first<0||it.second.first>3)
			return  false;
	}
	ui->fileName->setText(l[0]);
	ui->moduleName->setText(l[1]);
	ui->curDir->setText(l[2]);
	ui->function->setText(l[3]);
	ui->func_tb->setText(l[4]);
	ui->version->setText(l[5]);
	ui->author->setText(l[6]);
	ui->param->setText(l[7]);
	ui->createFolder->setChecked(j&1);
	j>>=1;
	ui->doFile->setChecked(j&1);
	j>>=1;
	ui->testbench->setCurrentIndex(j);
	for(auto it:ll){
		addRow(false,
			   it.first.first,
			   it.first.second,
			   it.second.first>>1,
			   it.second.first&1,
			   it.second.second);
	}
	d.setDevice(nullptr);
	f.close();
	return true;
}

void MainWindow::saveLog(){
	QDir::setCurrent(curPath);
	f.setFileName("log.dat");
	if(!f.open(QIODevice::WriteOnly)){
		fprintf(stderr,"Error: can't write to log\n");
		fprintf(stderr,f.errorString().toStdString().c_str());
		fflush(stderr);
		return;
	}
	QDataStream d(&f);
	QStringList l;
	l.append(ui->fileName->text());
	l.append(ui->moduleName->text());
	l.append(ui->curDir->text());
	l.append(ui->function->text());
	l.append(ui->func_tb->text());
	l.append(ui->version->text());
	l.append(ui->author->text());
	l.append(ui->param->text());
	int j=ui->testbench->currentIndex()*4;
	j+=ui->doFile->isChecked()*2;
	j+=ui->createFolder->isChecked();
	QVector<myPair> ll;
	for(int i=0;i<table()->rowCount()-1;++i){
		ll.append(myPair(
			pair1(
				table()->item(i,0)->text(),
				buttonStat(i,1)),
			pair2(
				buttonStat(i,2)*2+buttonStat(i,3),
				table()->item(i,4)->text())));
	}
	d<<j<<l<<ll;
	d.setDevice(nullptr);
	f.close();
}

void MainWindow::on_pushButton_clicked(){
	QString prefix=ui->curDir->text(),file,tbFile,doFile;
	QString module,module_tb,date,func,paramstr,str;
	QDir dir(prefix);
	QStringList list,input,ouput,intra;
	QStringList inpre,oupre,intpre;

	int maxl=29,maxn=0;
	bool p=false;

	file=ui->fileName->text();
	module=ui->moduleName->text();
	if(file.isEmpty()){
		str="File name can't be empty!";
		QMessageBox::critical(this,"Error",str);
		return;
	}
	if(module.isEmpty()){
		str="Module name can't be empty!";
		QMessageBox::critical(this,"Error",str);
		return;
	}
	if(ui->createFolder->isChecked()){
		dir.setPath(dir.filePath(file));
		if(!dir.exists()){
			dir.setPath(prefix);
			if(!(dir.mkdir(file)&&dir.cd(file))){
				str="Can't create the directory!";
				QMessageBox::critical(this,"Error",str);
				return;
			}
		}
	}
	file+=".v";

	if(ui->testbench->currentText()=="tb_*"){
		tbFile="tb_"+ui->fileName->text()+".v";
		module_tb="tb_"+module;
	}else{
		tbFile=ui->fileName->text()+"_tb.v";
		module_tb=module+"_tb";
	}

	if(dir.exists(file)){
		list.append(file);
	}
	if(dir.exists(tbFile)){
		list.append(tbFile);
	}

	if(ui->doFile->isChecked()){
		doFile=ui->fileName->text()+".do";
		if(dir.exists(doFile)){
			list.append(doFile);
		}
	}
	if(!list.empty()){
		str="The following files:\n";
		str+=list.join('\n');
		str+="\nwill be overwritten. Continue?";
		switch(QMessageBox::question(this,"Overwrite",str)){
		case QMessageBox::Yes:
			break;
		default:
			return;
		}
	}

	if(!QDir::setCurrent(dir.path())){
		str="Can't change to directory\n";
		str+=dir.path();
		str+="\nPlease try again or reselect the dir.";
		QMessageBox::critical(this,"Error",str);
		return;
	}

	if(!openFile(file)){
		closeFile();
		return;
	}

	date=QDate::currentDate().toString("yyyy-MM-dd");
	s<<"////////////////////////////////"<<endl;
	s<<"//module name: "<<module<<endl;
	func=ui->function->text();
	str=(func.isEmpty())?"Implementation of module "+module:func;
	s<<"//function: "<<str<<endl;
	s<<"//time: "<<date<<endl;
	s<<"//author: "<<ui->author->text()<<endl;
	s<<"//version: "<<ui->version->text()<<endl;
	s<<"//mark of revision:"<<endl;
	s<<"//build-"<<date<<endl;
	s<<"////////////////////////////////"<<endl;
	s<<endl;

// Process table here.......................

	for(int i=0;i<table()->rowCount()-1;++i){
		// prefix not used
		prefix="";
		QString name;
		switch(buttonStat(i,1)){
		case 1:
			prefix+="output ";
			name=module+"_ouput_";
			break;
		case 0:
			prefix+="input  ";
			name=module+"_input_";
			break;
		//case -1:
		default:
			prefix+="       ";
			name="";
			break;
		}
		str=table()->item(i,0)->text();
		if(str.isEmpty())
			name+=QString::number(i);
		else
			name+=str;

		switch(buttonStat(i,2)){
		case 1:
			prefix+="reg  ";
			break;
		//case 0:
		default:
			prefix+=(buttonStat(i,1)>=0)?"     "
										:"wire ";
			break;
		}

		switch(buttonStat(i,3)){
		case 1:
			prefix+="signed ";
			break;
		//case 0:
		default:
			prefix+="       ";
			break;
		}

		str=table()->item(i,4)->text();
		if(str.isEmpty()){
			//prefix+="";
		}else{
			prefix+='['+str+":0] ";
		}

		if(prefix.length()>maxl)
			maxl=prefix.length();

		switch(buttonStat(i,1)){
		case 1:
			ouput.append(name);
			oupre.append(prefix);
			if(name.length()>maxn)
				maxn=name.length();
			break;
		case 0:
			input.append(name);
			inpre.append(prefix);
			if(name.length()>maxn)
				maxn=name.length();
			break;
		//case -1:
		default:
			intra.append(name);
			intpre.append(prefix);
			break;
		}
	}
	++maxn;
	for(QString& st: inpre){
		int diff=maxl-st.length();
		if(diff>0)
			st+=QString(diff,' ');
	}
	for(QString& st: oupre){
		int diff=maxl-st.length();
		if(diff>0)
			st+=QString(diff,' ');
	}

	for(QString& st: intpre){
		int diff=maxl-st.length();
		if(diff>0)
			st+=QString(diff,' ');
	}

// Continue.......................

	str="module "+module+" (";
	s<<str;
	str=QString(str.length(),' ');
	for(QString& st: input){
		printIO(p,st,str);
	}
	for(QString& st: ouput){
		printIO(p,st,str);
	}
	s<<");"<<endl;
	s<<'\t'<<endl;
	paramstr=ui->param->text();
	if(!paramstr.isEmpty())
		s<<'\t'<<"parameter "<<paramstr<<"=4;"<<endl;
	s<<'\t'<<endl;

	for(int i=0;i<input.length();++i){
		s<<'\t'<<inpre[i]<<input[i]<<';'<<endl;
	}
	for(int i=0;i<ouput.length();++i){
		s<<'\t'<<oupre[i]<<ouput[i]<<';'<<endl;
	}
	s<<'\t'<<endl;
	for(int i=0;i<intra.length();++i){
		s<<'\t'<<intpre[i]<<intra[i]<<';'<<endl;
	}
	intra.clear();
	intpre.clear();
	s<<'\t'<<endl;
	s<<'\t'<<"//Module starts here... For example:"<<endl;
	s<<'\t'<<"always@(posedge "<<module<<"_input_clk or negedge "<<module<<"_input_rst) begin"<<endl;
	s<<"\t\t"<<"if(~"<<module<<"_input_rst) begin"<<endl;
	s<<"\t\t\t"<<"// Reset here..."<<endl;
	s<<"\t\t"<<"end else begin"<<endl;
	s<<"\t\t\t"<<"// Works done here..."<<endl;
	s<<"\t\t"<<"end"<<endl;
	s<<'\t'<<"end"<<endl;
	s<<'\t'<<endl;
	s<<"endmodule"<<endl;
	closeFile();

///////////////////////////////////////////

	if(!openFile(tbFile)){
		closeFile();
		return;
	}
	s<<"////////////////////////////////"<<endl;
	s<<"//module name: "<<module_tb<<endl;
	func=ui->func_tb->text();
	str=(func.isEmpty())?"Testbench of module "+module:func;
	s<<"//function: "<<str<<endl;
	s<<"//time: "<<date<<endl;
	s<<"//author: "<<ui->author->text()<<endl;
	s<<"//version: "<<ui->version->text()<<endl;
	s<<"//mark of revision:"<<endl;
	s<<"//build-"<<date<<endl;
	s<<"////////////////////////////////"<<endl;
	s<<endl;
	//s<<"`timescale 1ps/1ps"<<endl;
	s<<header<<endl;
	s<<endl;
	s<<"module "<<module_tb<<"();"<<endl;
	s<<'\t'<<endl;
	bool hasClk=input.contains(module+"_input_clk");
	str=((hasClk)?params:paramsNoClk);
	if(!paramstr.isEmpty())
		str.replace("PARAM",paramstr);
	s<<str<<endl;
	s<<'\t'<<endl;
	for(int i=0;i<input.length();++i){
		s<<"\treg  "<<inpre[i].remove(0,12);
		inpre[i]=input[i];
		input[i].remove(0,module.length()+1);
		s<<input[i].remove(2,3)<<';'<<endl;
	}
	for(int i=0;i<ouput.length();++i){
		s<<"\twire "<<oupre[i].remove(0,12);
		oupre[i]=ouput[i];
		ouput[i].remove(0,module.length()+1);
		s<<ouput[i].remove(2,3)<<';'<<endl;
	}
	s<<'\t'<<endl;

	inpre+=oupre;
	oupre.clear();
	input+=ouput;
	ouput.clear();
	p=false;

	s<<'\t'<<module<<' ';
	if(!paramstr.isEmpty())
		s<<"#(."<<paramstr<<'('<<paramstr<<")) ";
	s<<module<<"0(";
	for(int i=0;i<input.length();++i){
		if(p)
			s<<',';
		else
			p=true;
		s<<endl<<"\t\t."<<inpre[i]
		 <<QString(maxn-inpre[i].length(),' ')
		 <<'('<<input[i]<<')';
	}
	s<<");"<<endl;
	s<<'\t'<<endl;
	str=((hasClk)?content:contentNoClk);
	if(paramstr.isEmpty())
		paramstr="PARAM";
	str.replace("*PARAM*",paramstr);
	maxl=paramstr.length()-6;
	str.replace("*BLANK*",QString((maxl>0)?maxl:0,' '));
	str.replace("*BLANK2*",QString((maxl<0)?-maxl:0,' '));
	s<<str<<endl;
	s<<'\t'<<endl;
	s<<"endmodule"<<endl;
	closeFile();

///////////////////////////////////////////

	if(!doFile.isEmpty()){
		if(!openFile(doFile)){
			closeFile();
			return;
		}
		s<<"vlog -work work -stats=none "<<file<<endl;
		s<<"vlog -work work -stats=none "<<tbFile<<endl;
		s<<"vsim -gui work."<<module_tb<<endl;
		s<<"add wave sim:/"<<module_tb<<"/*"<<endl;
		s<<"run -all"<<endl;
		closeFile();
	}
	saveLog();
	str="Successfully generated the following files:";
	str+='\n'+file;
	str+='\n'+tbFile;
	if(!doFile.isEmpty())
		str+='\n'+doFile;
	QMessageBox::information(this,"Success",str);
}
=======
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "bincheck.h"
#include "buttons.h"
#include <QToolButton>
#include <QMessageBox>
#include <QFileDialog>
#include <QString>
#include <QStringList>
#include <QDir>
#include <QDate>
#include <QDataStream>
#include <QVector>

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	ui->tableWidget->horizontalHeader()->setSectionResizeMode(0,QHeaderView::Stretch);
	ui->tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	delIcon=QIcon(":/pics/delete.jpg");
	addIcon=QIcon(":/pics/add.jpg");
	auto button=new QToolButton();
	button->setIcon(addIcon);
	ui->tableWidget->setCellWidget(0,5,button);
	connect(button,&QAbstractButton::clicked,this,&MainWindow::_addRow);
	curPath=QDir::currentPath();
	ui->curDir->setText(curPath);
	if(!readLog()){
		addRow(false,"clk");
	}
}

MainWindow::~MainWindow(){
	saveLog();
	delete ui;
}

void MainWindow::_addRow(){
	addRow();
}

int MainWindow::buttonStat(int row, int col){
	return static_cast<BinCheck*>(ui->tableWidget->cellWidget(row,col))->get();
}

QTableWidget *MainWindow::table(){
	return ui->tableWidget;
}

void MainWindow::addRow(bool useLast, const QString &name, int io, int wire, int sign, const QString &lenm1){
	int row=ui->tableWidget->rowCount()-1;
	ui->tableWidget->insertRow(row);

	BinCheck* check;
	int stat;
	check=new BinCheck("in","out",true);
	ui->tableWidget->setCellWidget(row,1,check);
	if(row>0&&useLast)
		stat=buttonStat(row-1,1);
	else
		stat=io;

	if(stat>=0)
		check->set(stat>0);
	else
		check->reset();

	check=new BinCheck("wire","reg");
	ui->tableWidget->setCellWidget(row,2,check);
	stat=(row>0&&useLast)?buttonStat(row-1,2):wire;
	check->set(stat>0);

	check=new BinCheck("uns","sign");
	ui->tableWidget->setCellWidget(row,3,check);
	stat=(row>0&&useLast)?buttonStat(row-1,3):sign;
	check->set(stat>0);

	table()->setItem(row,0,new QTableWidgetItem(name));
	table()->setItem(row,4,new QTableWidgetItem(lenm1));

	DelButton* button=new DelButton(delIcon,
									table()->item(row,4),table());
	ui->tableWidget->setCellWidget(row,5,button);

	//ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

void MainWindow::on_toolButton_clicked(){
	QString path=QDir::toNativeSeparators(
		QFileDialog::getExistingDirectory(this,tr("Directory to save"),
		ui->curDir->text()));
	ui->curDir->setText(path);
}

bool MainWindow::openFile(const QString& fileName){
	f.setFileName(fileName);
	if(!f.open(QIODevice::WriteOnly|QIODevice::Text)){
		QString str="During opening of\n";
		str+=fileName;
		str+="\nAn error occured:\n";
		str+=f.errorString();
		str+="\nPlease try again.";
		QMessageBox::critical(this,"Error",str);
		return false;
	}
	s.setDevice(&f);
	return true;
}

void MainWindow::closeFile(){
	s.setDevice(nullptr);
	f.close();
}

void MainWindow::printIO(bool &p, const QString &name, const QString &pad){
	if(p){
		s<<','<<endl<<pad<<name;
	}else{
		p=true;
		s<<name;
	}

}

bool MainWindow::readLog(){
	QDir::setCurrent(curPath);
	f.setFileName("log.dat");
	if(!f.open(QIODevice::ReadOnly))
		return false;
	QDataStream d(&f);
	int j;
	QStringList l;
	QVector<myPair> ll;
	d>>j>>l>>ll;
	if(d.status()!=QDataStream::Ok)
		return false;
	if(l.length()!=8)
		return false;
	if(j>=8||j<0)
		return false;
	for(auto it:ll){
		if(it.first.second<-1||it.first.second>1)
			return false;
		if(it.second.first<0||it.second.first>3)
			return  false;
	}
	ui->fileName->setText(l[0]);
	ui->moduleName->setText(l[1]);
	ui->curDir->setText(l[2]);
	ui->function->setText(l[3]);
	ui->func_tb->setText(l[4]);
	ui->version->setText(l[5]);
	ui->author->setText(l[6]);
	ui->param->setText(l[7]);
	ui->createFolder->setChecked(j&1);
	j>>=1;
	ui->doFile->setChecked(j&1);
	j>>=1;
	ui->testbench->setCurrentIndex(j);
	for(auto it:ll){
		addRow(false,
			   it.first.first,
			   it.first.second,
			   it.second.first>>1,
			   it.second.first&1,
			   it.second.second);
	}
	d.setDevice(nullptr);
	f.close();
	return true;
}

void MainWindow::saveLog(){
	QDir::setCurrent(curPath);
	f.setFileName("log.dat");
	if(!f.open(QIODevice::WriteOnly)){
		fprintf(stderr,"Error: can't write to log\n");
		fprintf(stderr,f.errorString().toStdString().c_str());
		fflush(stderr);
		return;
	}
	QDataStream d(&f);
	QStringList l;
	l.append(ui->fileName->text());
	l.append(ui->moduleName->text());
	l.append(ui->curDir->text());
	l.append(ui->function->text());
	l.append(ui->func_tb->text());
	l.append(ui->version->text());
	l.append(ui->author->text());
	l.append(ui->param->text());
	int j=ui->testbench->currentIndex()*4;
	j+=ui->doFile->isChecked()*2;
	j+=ui->createFolder->isChecked();
	QVector<myPair> ll;
	for(int i=0;i<table()->rowCount()-1;++i){
		ll.append(myPair(
			pair1(
				table()->item(i,0)->text(),
				buttonStat(i,1)),
			pair2(
				buttonStat(i,2)*2+buttonStat(i,3),
				table()->item(i,4)->text())));
	}
	d<<j<<l<<ll;
	d.setDevice(nullptr);
	f.close();
}

void MainWindow::on_pushButton_clicked(){
	QString prefix=ui->curDir->text(),file,tbFile,doFile;
	QString module,module_tb,date,func,paramstr,str;
	QDir dir(prefix);
	QStringList list,input,ouput,intra;
	QStringList inpre,oupre,intpre;

	int maxl=29,maxn=0;
	bool p=false;

	file=ui->fileName->text();
	module=ui->moduleName->text();
	if(file.isEmpty()){
		str="File name can't be empty!";
		QMessageBox::critical(this,"Error",str);
		return;
	}
	if(module.isEmpty()){
		str="Module name can't be empty!";
		QMessageBox::critical(this,"Error",str);
		return;
	}
	if(ui->createFolder->isChecked()){
		dir.setPath(dir.filePath(file));
		if(!dir.exists()){
			dir.setPath(prefix);
			if(!(dir.mkdir(file)&&dir.cd(file))){
				str="Can't create the directory!";
				QMessageBox::critical(this,"Error",str);
				return;
			}
		}
	}
	file+=".v";

	if(ui->testbench->currentText()=="tb_*"){
		tbFile="tb_"+ui->fileName->text()+".v";
		module_tb="tb_"+module;
	}else{
		tbFile=ui->fileName->text()+"_tb.v";
		module_tb=module+"_tb";
	}

	if(dir.exists(file)){
		list.append(file);
	}
	if(dir.exists(tbFile)){
		list.append(tbFile);
	}

	if(ui->doFile->isChecked()){
		doFile=ui->fileName->text()+".do";
		if(dir.exists(doFile)){
			list.append(doFile);
		}
	}
	if(!list.empty()){
		str="The following files:\n";
		str+=list.join('\n');
		str+="\nwill be overwritten. Continue?";
		switch(QMessageBox::question(this,"Overwrite",str)){
		case QMessageBox::Yes:
			break;
		default:
			return;
		}
	}

	if(!QDir::setCurrent(dir.path())){
		str="Can't change to directory\n";
		str+=dir.path();
		str+="\nPlease try again or reselect the dir.";
		QMessageBox::critical(this,"Error",str);
		return;
	}

	if(!openFile(file)){
		closeFile();
		return;
	}

	date=QDate::currentDate().toString("yyyy-MM-dd");
	s<<"////////////////////////////////"<<endl;
	s<<"//module name: "<<module<<endl;
	func=ui->function->text();
	str=(func.isEmpty())?"Implementation of module "+module:func;
	s<<"//function: "<<str<<endl;
	s<<"//time: "<<date<<endl;
	s<<"//author: "<<ui->author->text()<<endl;
	s<<"//version: "<<ui->version->text()<<endl;
	s<<"//mark of revision:"<<endl;
	s<<"//build-"<<date<<endl;
	s<<"////////////////////////////////"<<endl;
	s<<endl;

// Process table here.......................

	for(int i=0;i<table()->rowCount()-1;++i){
		// prefix not used
		prefix="";
		QString name;
		switch(buttonStat(i,1)){
		case 1:
			prefix+="output ";
			name=module+"_ouput_";
			break;
		case 0:
			prefix+="input  ";
			name=module+"_input_";
			break;
		//case -1:
		default:
			prefix+="       ";
			name="";
			break;
		}
		str=table()->item(i,0)->text();
		if(str.isEmpty())
			name+=QString::number(i);
		else
			name+=str;

		switch(buttonStat(i,2)){
		case 1:
			prefix+="reg  ";
			break;
		//case 0:
		default:
			prefix+=(buttonStat(i,1)>=0)?"     "
										:"wire ";
			break;
		}

		switch(buttonStat(i,3)){
		case 1:
			prefix+="signed ";
			break;
		//case 0:
		default:
			prefix+="       ";
			break;
		}

		str=table()->item(i,4)->text();
		if(str.isEmpty()){
			//prefix+="";
		}else{
			prefix+='['+str+":0] ";
		}

		if(prefix.length()>maxl)
			maxl=prefix.length();

		switch(buttonStat(i,1)){
		case 1:
			ouput.append(name);
			oupre.append(prefix);
			if(name.length()>maxn)
				maxn=name.length();
			break;
		case 0:
			input.append(name);
			inpre.append(prefix);
			if(name.length()>maxn)
				maxn=name.length();
			break;
		//case -1:
		default:
			intra.append(name);
			intpre.append(prefix);
			break;
		}
	}
	++maxn;
	for(QString& st: inpre){
		int diff=maxl-st.length();
		if(diff>0)
			st+=QString(diff,' ');
	}
	for(QString& st: oupre){
		int diff=maxl-st.length();
		if(diff>0)
			st+=QString(diff,' ');
	}

	for(QString& st: intpre){
		int diff=maxl-st.length();
		if(diff>0)
			st+=QString(diff,' ');
	}

// Continue.......................

	str="module "+module+" (";
	s<<str;
	str=QString(str.length(),' ');
	for(QString& st: input){
		printIO(p,st,str);
	}
	for(QString& st: ouput){
		printIO(p,st,str);
	}
	s<<");"<<endl;
	s<<'\t'<<endl;
	paramstr=ui->param->text();
	if(!paramstr.isEmpty())
		s<<'\t'<<"parameter "<<paramstr<<"=4;"<<endl;
	s<<'\t'<<endl;

	for(int i=0;i<input.length();++i){
		s<<'\t'<<inpre[i]<<input[i]<<';'<<endl;
	}
	for(int i=0;i<ouput.length();++i){
		s<<'\t'<<oupre[i]<<ouput[i]<<';'<<endl;
	}
	s<<'\t'<<endl;
	for(int i=0;i<intra.length();++i){
		s<<'\t'<<intpre[i]<<intra[i]<<';'<<endl;
	}
	intra.clear();
	intpre.clear();
	s<<'\t'<<endl;
	s<<'\t'<<"//Module starts here... For example:"<<endl;
	s<<'\t'<<"always@(posedge "<<module<<"_input_clk or negedge "<<module<<"_input_rst) begin"<<endl;
	s<<"\t\t"<<"if(~"<<module<<"_input_rst) begin"<<endl;
	s<<"\t\t\t"<<"// Reset here..."<<endl;
	s<<"\t\t"<<"end else begin"<<endl;
	s<<"\t\t\t"<<"// Works done here..."<<endl;
	s<<"\t\t"<<"end"<<endl;
	s<<'\t'<<"end"<<endl;
	s<<'\t'<<endl;
	s<<"endmodule"<<endl;
	closeFile();

///////////////////////////////////////////

	if(!openFile(tbFile)){
		closeFile();
		return;
	}
	s<<"////////////////////////////////"<<endl;
	s<<"//module name: "<<module_tb<<endl;
	func=ui->func_tb->text();
	str=(func.isEmpty())?"Testbench of module "+module:func;
	s<<"//function: "<<str<<endl;
	s<<"//time: "<<date<<endl;
	s<<"//author: "<<ui->author->text()<<endl;
	s<<"//version: "<<ui->version->text()<<endl;
	s<<"//mark of revision:"<<endl;
	s<<"//build-"<<date<<endl;
	s<<"////////////////////////////////"<<endl;
	s<<endl;
	//s<<"`timescale 1ps/1ps"<<endl;
	s<<header<<endl;
	s<<endl;
	s<<"module "<<module_tb<<"();"<<endl;
	s<<'\t'<<endl;
	bool hasClk=input.contains(module+"_input_clk");
	str=((hasClk)?params:paramsNoClk);
	if(!paramstr.isEmpty())
		str.replace("PARAM",paramstr);
	s<<str<<endl;
	s<<'\t'<<endl;
	for(int i=0;i<input.length();++i){
		s<<"\treg  "<<inpre[i].remove(0,12);
		inpre[i]=input[i];
		input[i].remove(0,module.length()+1);
		s<<input[i].remove(2,3)<<';'<<endl;
	}
	for(int i=0;i<ouput.length();++i){
		s<<"\twire "<<oupre[i].remove(0,12);
		oupre[i]=ouput[i];
		ouput[i].remove(0,module.length()+1);
		s<<ouput[i].remove(2,3)<<';'<<endl;
	}
	s<<'\t'<<endl;

	inpre+=oupre;
	oupre.clear();
	input+=ouput;
	ouput.clear();
	p=false;

	s<<'\t'<<module<<' ';
	if(!paramstr.isEmpty())
		s<<"#(."<<paramstr<<'('<<paramstr<<")) ";
	s<<module<<"0(";
	for(int i=0;i<input.length();++i){
		if(p)
			s<<',';
		else
			p=true;
		s<<endl<<"\t\t."<<inpre[i]
		 <<QString(maxn-inpre[i].length(),' ')
		 <<'('<<input[i]<<')';
	}
	s<<");"<<endl;
	s<<'\t'<<endl;
	str=((hasClk)?content:contentNoClk);
	if(!paramstr.isEmpty())
		str.replace("PARAM",paramstr);
	s<<str<<endl;
	s<<'\t'<<endl;
	s<<"endmodule"<<endl;
	closeFile();

///////////////////////////////////////////

	if(!doFile.isEmpty()){
		if(!openFile(doFile)){
			closeFile();
			return;
		}
		s<<"vlog -work work -stats=none "<<file<<endl;
		s<<"vlog -work work -stats=none "<<tbFile<<endl;
		s<<"vsim -gui work."<<module_tb<<endl;
		s<<"add wave sim:/"<<module_tb<<"/*"<<endl;
		s<<"run -all"<<endl;
		closeFile();
	}
	saveLog();
	str="Successfully generated the following files:";
	str+='\n'+file;
	str+='\n'+tbFile;
	if(!doFile.isEmpty())
		str+='\n'+doFile;
	QMessageBox::information(this,"Success",str);
}


const QString MainWindow::header=
"`define EOF -1\n"
"`define NULL 0\n"
"`timescale 1ps/1ps";

const QString MainWindow::params=
"	parameter PARAM=4;\n"
"	parameter CYCLE=20;\n"
"	parameter HALF_CYCLE=CYCLE/2;\n"
"	parameter INFILE=\"x.dat\";\n"
"	parameter OUTFILE=\"out.dat\";\n"
"	parameter STRLEN=640;";

const QString MainWindow::content=
"	//Module starts here... example below:\n"
"	//Needs to change PARAM, in_x, ou_freg and length/sign of correct_out\n"
"	integer                DELAY_CYC=2;\n"
"	integer                DELAY;\n"
"	integer                fd_in=0;\n"
"	integer                fd_out=0;\n"
"	reg         [STRLEN:1] fmt_str_in;\n"
"	reg         [STRLEN:1] fmt_str_out;\n"
"	reg  signed [PARAM-1:0]    correct_out;\n"
"	reg                    in_finish;\n"
"	reg                    out_finish;\n"
"	\n"
"	task exit;\n"
"	begin\n"
"		if(fd_in)\n"
"			$fclose(fd_in);\n"
"		if(fd_out)\n"
"			$fclose(fd_out);\n"
"		$stop;\n"
"		$finish;\n"
"	end\n"
"	endtask\n"
"	\n"
"	task open_err;\n"
"		input integer fd;\n"
"		input [STRLEN:1] FILE;\n"
"		integer err_code;\n"
"		reg [STRLEN:1] err_str;\n"
"	begin\n"
"		err_code=$ferror(fd,err_str);\n"
"		$display(\"Error(%0d) opening file %0s:\\n%0s\",err_code,FILE,err_str);\n"
"		exit();\n"
"	end\n"
"	endtask\n"
"	\n"
"	task read_err;\n"
"		input [STRLEN:1] FILE;\n"
"	begin\n"
"		$display(\"Error reading file %0s\",FILE);\n"
"		exit();\n"
"	end\n"
"	endtask\n"
"	\n"
"	\n"
"	\n"
"	initial begin\n"
"		in_clk<=1'b0;\n"
"		in_rst<=1'b0;\n"
"		in_finish<=1'b0;\n"
"		out_finish<=1'b0;\n"
"		#CYCLE;\n"
"		in_rst<=1'b1;\n"
"		while(!(in_finish&&out_finish)) begin\n"
"			#HALF_CYCLE; in_clk<=~in_clk;\n"
"		end\n"
"		$display(\"Testbench ended successfully, find no mistakes.\");\n"
"		exit();\n"
"	end\n"
"	\n"
"	initial begin\n"
"		fd_in=$fopen(INFILE,\"r\");\n"
"		if(fd_in==`NULL) begin\n"
"			open_err(fd_in,INFILE);\n"
"		end\n"
"		$sformat(fmt_str_in,\"%%%0dd\",PARAM);\n"
"		#CYCLE;\n"
"		begin : in_break\n"
"			forever begin\n"
"				case($fscanf(fd_in,fmt_str_in,in_x))\n"
"					1: begin\n"
"						#CYCLE;\n"
"					end\n"
"					`EOF: begin\n"
"						disable in_break;\n"
"					end\n"
"					default: begin\n"
"						read_err(INFILE);\n"
"					end\n"
"				endcase\n"
"			end\n"
"		end\n"
"		$display(\"Input has finished...\");\n"
"		in_finish<=1'b1;\n"
"	end\n"
"	\n"
"	initial begin\n"
"		fd_out=$fopen(OUTFILE,\"r\");\n"
"		if(fd_out==`NULL) begin\n"
"			open_err(fd_out,OUTFILE);\n"
"		end\n"
"		$fscanf(fd_out,\"DELAY = %d\",DELAY_CYC);\n"
"		DELAY=DELAY_CYC*CYCLE;\n"
"		#CYCLE;\n"
"		#DELAY;\n"
"		$sformat(fmt_str_out,\"%%%0dd\",PARAM);\n"
"		begin : out_break\n"
"			forever begin\n"
"				case($fscanf(fd_out,fmt_str_out,correct_out))\n"
"					1: begin\n"
"						if(correct_out!=ou_freg) begin\n"
"							$display(\"Output is incorrect:\");\n"
"							$display(\"Output: %0d\",ou_freg);\n"
"							$display(\"Should be: %0d\",correct_out);\n"
"							exit();\n"
"						end\n"
"						#CYCLE;\n"
"					end\n"
"					`EOF: begin\n"
"						disable out_break;\n"
"					end\n"
"					default: begin\n"
"						read_err(OUTFILE);\n"
"					end\n"
"				endcase\n"
"			end\n"
"		end\n"
"		$display(\"Output has finished...\");\n"
"		out_finish<=1'b1;\n"
"	end";

const QString MainWindow::paramsNoClk=
"	parameter PARAM=4;\n"
"	parameter INFILE=\"x.dat\";\n"
"	parameter OUTFILE=\"out.dat\";\n"
"	parameter STRLEN=640;";

const QString MainWindow::contentNoClk=
"	//Module starts here... example below:\n"
"	//Needs to change PARAM, in_x, ou_f and length/sign of correct_out\n"
"	integer                DELAY=10;\n"
"	integer                fd_in=0;\n"
"	integer                fd_out=0;\n"
"	reg         [STRLEN:1] fmt_str_in;\n"
"	reg         [STRLEN:1] fmt_str_out;\n"
"	reg  signed [PARAM-1:0]    correct_out;\n"
"	\n"
"	task exit;\n"
"	begin\n"
"		if(fd_in)\n"
"			$fclose(fd_in);\n"
"		if(fd_out)\n"
"			$fclose(fd_out);\n"
"		$stop;\n"
"		$finish;\n"
"	end\n"
"	endtask\n"
"	\n"
"	task open_err;\n"
"		input integer fd;\n"
"		input [STRLEN:1] FILE;\n"
"		integer err_code;\n"
"		reg [STRLEN:1] err_str;\n"
"	begin\n"
"		err_code=$ferror(fd,err_str);\n"
"		$display(\"Error(%0d) opening file %0s:\\n%0s\",err_code,FILE,err_str);\n"
"		exit();\n"
"	end\n"
"	endtask\n"
"	\n"
"	task read_err;\n"
"		input [STRLEN:1] FILE;\n"
"	begin\n"
"		$display(\"Error reading file %0s\",FILE);\n"
"		exit();\n"
"	end\n"
"	endtask\n"
"	\n"
"	\n"
"	\n"
"	initial begin\n"
"		fd_in=$fopen(INFILE,\"r\");\n"
"		if(fd_in==`NULL) begin\n"
"			open_err(fd_in,INFILE);\n"
"		end\n"
"		fd_out=$fopen(OUTFILE,\"r\");\n"
"		if(fd_out==`NULL) begin\n"
"			open_err(fd_out,OUTFILE);\n"
"		end\n"
"		$fscanf(fd_out,\"DELAY = %d\",DELAY);\n"
"		$sformat(fmt_str_in,\"%%%0dd\",PARAM);\n"
"		$sformat(fmt_str_out,\"%%%0dd\",PARAM);\n"
"		begin : in_break\n"
"			forever begin\n"
"				case($fscanf(fd_in,fmt_str_in,in_x))\n"
"					1: begin\n"
"						// Nothing here...\n"
"					end\n"
"					`EOF: begin\n"
"						disable in_break;\n"
"					end\n"
"					default: begin\n"
"						read_err(INFILE);\n"
"					end\n"
"				endcase\n"
"				case($fscanf(fd_out,fmt_str_out,correct_out))\n"
"					1: begin\n"
"						#DELAY;\n"
"						if(correct_out!=ou_f) begin\n"
"							$display(\"Output is incorrect:\");\n"
"							$display(\"Output: %0d\",ou_f);\n"
"							$display(\"Should be: %0d\",correct_out);\n"
"							exit();\n"
"						end\n"
"					end\n"
"					`EOF: begin\n"
"						$display(\"Warning: Output is shorter than input!\");\n"
"						exit();\n"
"					end\n"
"					default: begin\n"
"						read_err(OUTFILE);\n"
"					end\n"
"				endcase\n"
"			end\n"
"		end\n"
"		$display(\"Testbench ended successfully, find no mistakes.\");\n"
"		exit();\n"
"	end";
>>>>>>> 30c3946c2f4bf8923a0929e8f4c118d6102e7429
