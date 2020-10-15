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

using Qt::endl;

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
	maxl=paramstr.length()-4;
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
