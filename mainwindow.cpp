#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "bincheck.h"
#include "buttons.h"

#include <QCheckBox>
#include <QDataStream>
#include <QDate>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QIcon>
#include <QIODevice>
#include <QMainWindow>
#include <QMap>
#include <QMapIterator>
#include <QMessageBox>
#include <QRegularExpression>
#include <QSet>
#include <QSize>
#include <QString>
#include <QStringList>
#include <QTableWidget>
#include <QTextStream>
#include <QToolButton>
#include <QVector>
#include <QWidget>

using Qt::endl;

const int MainWindow::nameCol = 0;
const int MainWindow::ioCol   = 1;
const int MainWindow::wireCol = 2;
const int MainWindow::signCol = 3;
const int MainWindow::tbCol   = 4;
const int MainWindow::lenCol  = 5;
const int MainWindow::buttonCol  = 6;

MainWindow::MainWindow(QWidget* parent)
	: QMainWindow(parent), ui(new Ui::MainWindow){
	// Setup UI.
	ui->setupUi(this);
	table = ui->tableWidget;
	table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
	table->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	delIcon = QIcon(":/img/delete.jpg");
	addIcon = QIcon(":/img/add.jpg");
	upIcon = QIcon(":/img/up.jpg");
	downIcon = QIcon(":/img/down.jpg");
	// Add table button.
	auto* button = new QToolButton();
	button->setIcon(addIcon);
	button->setIconSize(QSize(20, 20));
	table->setCellWidget(0, buttonCol, button);
	connect(button, &QAbstractButton::clicked, this, &MainWindow::_addRow);
	// Connect signal-slots.
	connect(ui->dirButton, &QAbstractButton::clicked, this, &MainWindow::selectDir);
	connect(ui->genButton, &QAbstractButton::clicked, this, &MainWindow::generate);
	// Set init text.
	curPath = QDir::currentPath();
	ui->curDir->setText(curPath);
	if(!readLog()){
		addRow(false, "clk");
		addRow(false, "rst");
	}
	int res = readTemplate();
	if(res >= 0){
		QString str = "Read " + QString::number(res) + " entries from template.txt";
		QMessageBox::information(this, "Success", str);
	}
}

MainWindow::~MainWindow(){
	saveLog();
	delete ui;
}

void MainWindow::_addRow(){
	addRow();
}

void MainWindow::swapRows(int row1, int row2){
	if(row1 == row2) return;

	QString str;
	CheckStat stat;
	bool checked;

	str = table->item(row1, nameCol)->text();
	table->item(row1, nameCol)->setText(table->item(row2, nameCol)->text());
	table->item(row2, nameCol)->setText(str);

	str = table->item(row1, lenCol)->text();
	table->item(row1, lenCol)->setText(table->item(row2, lenCol)->text());
	table->item(row2, lenCol)->setText(str);

	stat = buttonStat(row1, ioCol);
	setButtonStat(row1, ioCol, buttonStat(row2, ioCol));
	setButtonStat(row2, ioCol, stat);

	stat = buttonStat(row1, wireCol);
	setButtonStat(row1, wireCol, buttonStat(row2, wireCol));
	setButtonStat(row2, wireCol, stat);

	stat = buttonStat(row1, signCol);
	setButtonStat(row1, signCol, buttonStat(row2, signCol));
	setButtonStat(row2, signCol, stat);

	checked = boxStat(row1, tbCol);
	setBoxStat(row1, tbCol, boxStat(row2, tbCol));
	setBoxStat(row2, tbCol, checked);
}

void MainWindow::buttonHandler(int row, RowButton::id_t id){
	QMessageBox::StandardButton ans;
	switch(id){
	case 0:
		ans = QMessageBox::question(this, "Deleting...", "Do you want to delete this from the table?");
		if(ans == QMessageBox::Yes)
			table->removeRow(row);
		break;
	// Move up.
	case 1:
		if(row == 0) break;
		swapRows(row-1, row);
		break;
	// Move down.
	case 2:
		if(row >= table->rowCount() - 2) break;
		swapRows(row, row+1);
		break;
	}
}

void MainWindow::selectDir(){
	QString path = QDir::toNativeSeparators(
		QFileDialog::getExistingDirectory(this, tr("Directory to save"),
			ui->curDir->text())
	);
	ui->curDir->setText(path);
}

MainWindow::CheckStat MainWindow::buttonStat(int row, int col){
	return dynamic_cast<BinCheck*>(table->cellWidget(row, col))->get();
}

void MainWindow::setButtonStat(int row, int col, CheckStat stat){
	dynamic_cast<BinCheck*>(table->cellWidget(row, col))->set(stat);
}

bool MainWindow::boxStat(int row, int col){
	return table->cellWidget(row, col)->findChild<QCheckBox*>()->isChecked();
}

void MainWindow::setBoxStat(int row, int col, bool stat){
	table->cellWidget(row, col)->findChild<QCheckBox*>()->setChecked(stat);
}

void MainWindow::addRow(bool useLast, const QString& name, CheckStat io, CheckStat wire, CheckStat sign, bool tbChecked, const QString& len){
	// Add row.
	int row = table->rowCount()-1;
	table->insertRow(row);
	useLast = useLast && row>0;

	BinCheck* check;
	BinCheck::Stat stat;

	// Add checkbox;
	QWidget* widget = new QWidget(table);
	QHBoxLayout* layout = new QHBoxLayout(widget);
	QCheckBox* box = new QCheckBox(widget);
	layout->addWidget(box);
	layout->setAlignment(box, Qt::AlignCenter);
	widget->setLayout(layout);
	if(useLast) tbChecked = boxStat(row-1, tbCol);
	box->setChecked(tbChecked);
	table->setCellWidget(row, tbCol, widget);

	// Add checks.
	check = new BinCheck("in", "out", true);
	connect(check, &BinCheck::isSet, box, &QCheckBox::setEnabled);
	//connect(check, &BinCheck::isSet, box, &QCheckBox::setChecked);
	table->setCellWidget(row, ioCol, check);
	stat = useLast ? buttonStat(row-1, ioCol) : io;
	check->set(stat);
	box->setEnabled(stat != CheckStat::NO_CHECK);

	check = new BinCheck("wire", "reg");
	table->setCellWidget(row, wireCol, check);
	stat = useLast ? buttonStat(row-1, wireCol) : wire;
	check->set(stat);

	check = new BinCheck("uns", "sign");
	table->setCellWidget(row, signCol, check);
	stat = useLast ? buttonStat(row-1, signCol) : sign;
	check->set(stat);

	// Add name and len.
	table->setItem(row, nameCol, new QTableWidgetItem(name));
	table->setItem(row, lenCol, new QTableWidgetItem(len));

	// Add control buttons.
	RowButton* button;
	QGridLayout* glayout;
	widget = new QWidget(table);
	glayout = new QGridLayout(widget);
	glayout->setSpacing(0);
	glayout->setContentsMargins(8,1,8,1);

	button=new RowButton(delIcon, 0, row, table);
	button->setIconSize(QSize(20, 20));
	glayout->addWidget(button, 0, 0, 2, 1);
	glayout->setAlignment(button, Qt::AlignCenter);
	connect(button, &RowButton::activated, this, &MainWindow::buttonHandler);

	button=new RowButton(upIcon, 1, row, table);
	button->setIconSize(QSize(16, 8));
	glayout->addWidget(button, 0, 1);
	glayout->setAlignment(button, Qt::AlignCenter);
	connect(button, &RowButton::activated, this, &MainWindow::buttonHandler);

	button=new RowButton(downIcon, 2, row, table);
	button->setIconSize(QSize(16, 8));
	glayout->addWidget(button, 1, 1);
	glayout->setAlignment(button, Qt::AlignCenter);
	connect(button, &RowButton::activated, this, &MainWindow::buttonHandler);

	widget->setLayout(glayout);
	table->setCellWidget(row, buttonCol, widget);

	//table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

bool MainWindow::openFile(const QString& fileName, QFile& f, QTextStream& s){
	f.setFileName(fileName);
	if(!f.open(QIODevice::WriteOnly | QIODevice::Text)){
		QString str = "During opening of\n";
		str += fileName;
		str += "\nAn error occured:\n";
		str += f.errorString();
		str += "\nPlease try again.";
		QMessageBox::critical(this, "Error", str);
		return false;
	}
	s.setDevice(&f);
	return true;
}

void MainWindow::closeFile(QFile& f, QTextStream& s){
	s.setDevice(nullptr);
	f.close();
}

bool MainWindow::readLog(){
	QDir::setCurrent(curPath);
	QFile f("log.dat");
	if(!f.open(QIODevice::ReadOnly))
		return false;

	QDataStream d(&f);
	int j;
	QStringList l;
	QVector<rowEntry> ll;

	d >> j >> l >> ll;

	if(d.status() != QDataStream::Ok)
		return false;
	if(l.length() != 8)
		return false;
	if(j >= 8 || j < 0)
		return false;

	ui->fileName->setText(l[0]);
	ui->moduleName->setText(l[1]);
	ui->curDir->setText(l[2]);
	ui->function->setText(l[3]);
	ui->func_tb->setText(l[4]);
	ui->version->setText(l[5]);
	ui->author->setText(l[6]);
	ui->localparam->setText(l[7]);
	ui->createFolder->setChecked(j&1);
	j>>=1;
	ui->doFile->setChecked(j&1);
	j>>=1;
	ui->testbench->setCurrentIndex(j);
	for(const auto& it : ll){
		addRow(false, it.name, it.io, it.wire, it.sign, it.tbChecked, it.len);
	}
	d.setDevice(nullptr);
	f.close();
	return true;
}

void MainWindow::saveLog(){
	QDir::setCurrent(curPath);
	QFile f("log.dat");
	if(!f.open(QIODevice::WriteOnly)){
		QString str = "Error: can't write to log\n" + f.errorString();
		QMessageBox::critical(this, "Error", str);
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
	l.append(ui->localparam->text());

	int j = ui->testbench->currentIndex() * 4;
	j += ui->doFile->isChecked() * 2;
	j += ui->createFolder->isChecked();

	QVector<rowEntry> ll;
	for(int i=0;i<table->rowCount()-1;++i){
		ll.append({table->item(i, nameCol)->text(),
				   buttonStat(i, ioCol),
				   buttonStat(i, wireCol),
				   buttonStat(i, signCol),
				   boxStat(i, tbCol),
				   table->item(i, lenCol)->text()
				  });
	}

	d << j << l << ll;
	d.setDevice(nullptr);
	f.close();
}

int MainWindow::readTemplate(){
	static const QMap<QString, QString*> map{
		{"clkName", &clkName},
		{"moduleContent", &moduleContent},
		{"header", &header},
		{"params", &params},
		{"checkNeq", &checkNeq},
		{"content", &content},
		{"paramsNoClk", &paramsNoClk},
		{"contentNoClk", &contentNoClk},
	};

	QMap<QString, QString> changeContent;
	QDir::setCurrent(curPath);
	QFile f("template.txt");
	if(!f.open(QIODevice::ReadOnly | QIODevice::Text))
		return -2;

	QTextStream s(&f);

	QString str = s.readAll(), errMsg = "While reading template.txt:\n";
;
	str.replace("```nnl\n", "");
	typedef qsizetype qsize_t;
	qsize_t pos = 0, pos2;
	bool valid = false;
	while(true){
		pos = str.indexOf("```", pos);
		if(pos == -1){
			valid = true;
			break;
		}
		pos += 3;
		pos2 = str.indexOf('\n', pos);
		if(pos2 == -1){
			errMsg += "Unterminated name found at pos " + QString::number(pos);
			QMessageBox::critical(this, "Error", errMsg);
			break;
		}
		QString name = str.sliced(pos, pos2-pos);
		pos2 += 1;
		pos = str.indexOf("```end\n", pos2);
		if(pos == -1){
			if(name.length() > 25){
				name = name.first(20) + "...";
			}
			errMsg += "Template " + name + " is not ended!";
			QMessageBox::critical(this, "Error", errMsg);
			break;
		}
		QString contentStr = str.sliced(pos2, pos - pos2);
		pos += 7;

		if(name == "clkName"){
			contentStr = contentStr.trimmed();
		}

		if(changeContent.contains(name)){
			errMsg += "Redefinition of template " + name;
			QMessageBox::critical(this, "Error", errMsg);
			break;
		}else if(!map.contains(name)){
			if(name.length() > 25){
				name = name.first(20) + "...";
			}
			errMsg += "Unknown template name " + name;
			QMessageBox::critical(this, "Error", errMsg);
			break;
		}else{
			changeContent[name] = contentStr;
		}
	}
	if(valid){
		QMapIterator<QString, QString> it(changeContent);
		while(it.hasNext()){
			it.next();
			*map[it.key()] = it.value();
		}
		return changeContent.size();
	}
	return -1;
}

void MainWindow::printPins(QTextStream& s, bool& p, const QString& name, const QString& pad){
	if(p){
		s << ',' << endl << pad << name;
	}else{
		p = true;
		s << name;
	}
}

bool MainWindow::generate(){
	// Prefix
	QString prefix = ui->curDir->text();
	// File names
	QString file, tbFile, doFile;
	// Infos
	QString module, module_tb, func, func_tb, date, author, version, build;

	file = ui->fileName->text();
	module = ui->moduleName->text();

	func = ui->function->text();
	if(func.isEmpty()) func = "Implementation of module " + module;
	func_tb = ui->func_tb->text();
	if(func_tb.isEmpty()) func_tb = "Testbench of module " + module;
	date = QDate::currentDate().toString("yyyy-MM-dd");
	author = ui->author->text();
	version = ui->version->text();
	build = "build-" + date;

	// Name and prefix
	QStringList input, ouput, intra;
	QStringList inpre, oupre, intpre;
	QStringList tb_in, tb_out;
	QStringList dat_in, real_out, dat_out, ans_pre;

	QSet<QString> globalParams, localParams;

	// Temp variables
	QString str;
	QStringList list;
	typedef qsizetype qsize_t;
	qsize_t maxl = 0, maxn = 0, pref_len = 0, maxp = 0;
	qsize_t pos1, pos2;
	bool p = false;

	bool hasClk;

	QDir dir(prefix);
	QFile f;
	QTextStream s;

	auto replaceStr = [=](QString& rep_str){
		rep_str.replace("*FILE*", file);
		rep_str.replace("*MODULE*", module);
	};

	if(file.isEmpty()){
		str = "File name can't be empty!";
		QMessageBox::critical(this, "Error", str);
		return false;
	}
	if(module.isEmpty()){
		str = "Module name can't be empty!";
		QMessageBox::critical(this, "Error", str);
		return false;
	}
	if(ui->createFolder->isChecked()){
		dir.setPath(dir.filePath(file));
		if(!dir.exists()){
			dir.setPath(prefix);
			if(!(dir.mkdir(file) && dir.cd(file))){
				str = "Can't create the directory!";
				QMessageBox::critical(this, "Error", str);
				return false;
			}
		}
	}

	if(ui->testbench->currentText() == "tb_*"){
		module_tb = "tb_" + module;
		tbFile = "tb_" + file + ".v";
	}else{
		module_tb = module + "_tb";
		tbFile = file + "_tb.v";
	}

	file += ".v";
	if(dir.exists(file)){
		list.append(file);
	}
	if(dir.exists(tbFile)){
		list.append(tbFile);
	}

	if(ui->doFile->isChecked()){
		doFile = ui->fileName->text() + ".do";
		if(dir.exists(doFile)){
			list.append(doFile);
		}
	}
	if(!list.empty()){
		str = "The following files:\n";
		str += list.join('\n');
		str += "\nwill be overwritten. Continue?";
		switch(QMessageBox::question(this, "Overwrite", str)){
		case QMessageBox::Yes:
			break;
		default:
			return false;
		}
	}

	if(!QDir::setCurrent(dir.path())){
		str = "Can't change to directory\n";
		str += dir.path();
		str += "\nPlease try again or reselect the dir.";
		QMessageBox::critical(this, "Error", str);
		return false;
	}

	if(!openFile(file, f, s)){
		closeFile(f, s);
		return false;
	}

	// Header

	s << "////////////////////////////////" << endl;
	s << "//module name: " << module << endl;
	s << "//function: " << func << endl;
	s << "//time: " << date << endl;
	s << "//author: " << author << endl;
	s << "//version: " << version << endl;
	s << "//mark of revision:" << endl;
	s << "//" << build << endl;
	s << "////////////////////////////////" << endl;
	s << endl;

	// Process table here.......................

	for(int i = 0; i < table->rowCount()-1; ++i){
		QString name, tb_name, ans_name;
		bool in_tb = false;
		switch(buttonStat(i, ioCol)){
		case CheckStat::CHECK_1:
			prefix = "output ";
			name = module + "_ouput_";
			tb_name = "ou_";
			ans_name = "ans_";
			in_tb = boxStat(i, tbCol);
			break;
		case CheckStat::CHECK_0:
			prefix = "input  ";
			name = module + "_input_";
			tb_name = "in_";
			in_tb = boxStat(i, tbCol);
			break;
		case CheckStat::NO_CHECK:
			prefix = "       ";
			name = "";
			break;
		}

		str = table->item(i, nameCol)->text();
		if(str.isEmpty()) str = QString::number(i);
		name += str;
		tb_name += str;
		ans_name += str;

		switch(buttonStat(i, wireCol)){
		case CheckStat::CHECK_1:
			prefix += "reg  ";
			break;
		case CheckStat::CHECK_0:
		case CheckStat::NO_CHECK:
			bool internal = buttonStat(i, ioCol) == CheckStat::NO_CHECK;
			prefix += internal ? "wire " : "     ";
			break;
		}

		pref_len = prefix.length();

		switch(buttonStat(i, signCol)){
		case CheckStat::CHECK_1:
			prefix += "signed ";
			break;
		case CheckStat::CHECK_0:
		case CheckStat::NO_CHECK:
			prefix += "       ";
			break;
		}

		str = table->item(i, lenCol)->text();
		if(!str.isEmpty()){
			unsigned len = str.toUInt(&p);
			if(p){
				str = QString::number(len - 1);
				prefix += '['+str+":0] ";
			}else{
				prefix += '['+str+"-1:0] ";
				globalParams.insert(str);
			}
		}

		if(prefix.length() > maxl)
			maxl = prefix.length();

		switch(buttonStat(i, ioCol)){
		case CheckStat::CHECK_1:
			ouput.append(name);
			tb_out.append(tb_name);
			oupre.append(prefix);
			if(in_tb){
				real_out.append(tb_name);
				dat_out.append(ans_name);
				str = prefix;
				ans_pre.append(str.remove(0, pref_len));
			}
			if(name.length() > maxn)
				maxn = name.length();
			break;
		case CheckStat::CHECK_0:
			input.append(name);
			tb_in.append(tb_name);
			inpre.append(prefix);
			if(in_tb){
				dat_in.append(tb_name);
			}
			if(name.length() > maxn)
				maxn = name.length();
			break;
		case CheckStat::NO_CHECK:
			intra.append(name);
			intpre.append(prefix);
			break;
		}
	}

	hasClk = input.contains(module+"_input_"+clkName);

	for(QString& st: inpre){
		int diff = maxl - st.length();
		if(diff > 0)
			st += QString(diff, ' ');
	}
	for(QString& st: oupre){
		int diff = maxl - st.length();
		if(diff > 0)
			st += QString(diff, ' ');
	}
	for(QString& st: intpre){
		int diff = maxl - st.length();
		if(diff > 0)
			st += QString(diff, ' ');
	}
	for(QString& st: ans_pre){
		int diff = maxl - pref_len - st.length();
		if(diff > 0)
			st += QString(diff, ' ');
	}

	// Module definition

	str = "module " + module + " (";
	s << str;
	str = QString(str.length(), ' ');
	p = false;
	for(QString& st: input){
		printPins(s, p, st, str);
	}
	for(QString& st: ouput){
		printPins(s, p, st, str);
	}
	s << ");" << endl;
	s << '\t' << endl;

	// Param definition

	str = ui->localparam->text();
	static QRegularExpression regexp("[;, ]");
	list = str.split(regexp, Qt::SkipEmptyParts);
	for(const auto& x : list){
		localParams.insert(x);
	}
	globalParams -= localParams;
	maxp = 0;
	for(const QString& it : qAsConst(globalParams)){
		if(it.length() > maxp) maxp = it.length();
	}
	for(const QString& it : qAsConst(localParams)){
		if(it.length() > maxp) maxp = it.length();
	}
	if(maxp > 0){
		for(const QString& it: qAsConst(globalParams)){
			str = it;
			int diff = maxp - str.length();
			if(diff > 0) str += QString(diff, ' ');
			s << '\t' << "parameter  " << str << " = X;" << endl;
		}
		for(const QString& it: qAsConst(localParams)){
			str = it;
			int diff = maxp - str.length();
			if(diff > 0) str += QString(diff, ' ');
			s << '\t' << "localparam " << str << " = X;" << endl;
		}
		s << '\t' << endl;
	}

	// Pins definition

	for(qsize_t i = 0; i < input.length(); ++i){
		s << '\t' << inpre[i] << input[i] << ';' << endl;
	}
	for(qsize_t i = 0; i < ouput.length(); ++i){
		s << '\t' << oupre[i] << ouput[i] << ';' << endl;
	}
	s << '\t' << endl;
	for(qsize_t i = 0; i < intra.length(); ++i){
		s << '\t' << intpre[i] << intra[i] << ';' << endl;
	}
	intra.clear();
	intpre.clear();

	s << '\t' << endl;

	// Module content

	str = moduleContent;
	replaceStr(str);
	s << str;
	closeFile(f, s);

	// -------- Start testbench generation --------

	if(!openFile(tbFile, f, s)){
		closeFile(f, s);
		return false;
	}

	// Header generation

	s << "////////////////////////////////" << endl;
	s << "//module name: " << module_tb << endl;
	s << "//function: " << func_tb << endl;
	s << "//time: " << date << endl;
	s << "//author: " << author << endl;
	s << "//version: " << version << endl;
	s << "//mark of revision:" << endl;
	s << "//" << build << endl;
	s << "////////////////////////////////" << endl;
	s << endl;

	// "Define" generation

	str = header;
	replaceStr(str);
	if(!str.isEmpty()){
		s << str;
		s << endl;
	}

	// Testbench module

	s << "module " << module_tb << "();" << endl;
	s << '\t' << endl;

	// Parameters

	str = hasClk ? params : paramsNoClk;
	replaceStr(str);
	pos1 = str.indexOf("*PBLANK*");
	pos2 = str.lastIndexOf("parameter", pos1);
	if(pos1 != -1 && pos2 != -1){
		// Add "parameter" and ' 'x2
		maxp = pos1 = pos1 - (pos2 + 9 + 2);
		for(const QString& it : qAsConst(globalParams)){
			if(it.length() > maxp) maxp = it.length();
		}
		pos1 = maxp - pos1;
		str.replace("*PBLANK*", QString(pos1, ' '));
	}else if(pos1 != -1){
		str.replace("*PBLANK*", "");
	}
	if(!str.isEmpty()){
		s << str;
		s << '\t' << endl;
	}
	for(const QString& it : qAsConst(globalParams)){
		str = it;
		int diff = maxp - str.length();
		if(diff > 0) str += QString(diff, ' ');
		s << '\t' << "parameter " << str << " = X;" << endl;
	}
	if(!globalParams.empty()) s << '\t' << endl;

	// Inputs and outputs

	for(qsize_t i = 0; i < tb_in.length(); ++i){
		s << "\treg  " << inpre[i].remove(0, pref_len);
		s << tb_in[i] << ';' << endl;
	}
	for(qsize_t i = 0; i < tb_out.length(); ++i){
		s << "\twire " << oupre[i].remove(0, pref_len);
		s << tb_out[i] << ';' << endl;
	}
	s << '\t' << endl;

	inpre.clear();
	oupre.clear();
	input += ouput;
	ouput.clear();
	tb_in += tb_out;
	tb_out.clear();

	// Insert module

	s << '\t' << module << ' ';
	if(!globalParams.isEmpty()){
		s << "#(";
		p = false;
		for(const QString& it : qAsConst(globalParams)){
			if(p) s << ", ";
			s << '.' << it << '(' << it << ')';
			p = true;
		}
		s << ") ";
	}

	s << module << "_0(";
	for(qsize_t i = 0; i < input.length(); ++i){
		if(i != 0){
			s << ',';
		}
		str = QString(maxn - input[i].length() + 1,' ');
		s << endl << "\t\t." << input[i] << str << '(' << tb_in[i] << ')';
	}
	s << ");" << endl;
	s << '\t' << endl;

	// Content of testbench

	str = hasClk ? content : contentNoClk;
	replaceStr(str);
	pos1 = str.indexOf("*ANS_OUT_DEF*");
	pos2 = str.indexOf("*BLANK*");
	qsize_t output_pad = 0;
	if(pos1 != -1 && pos2 != -1 && !dat_out.empty()){
		qsize_t from_pos = str.lastIndexOf("\n", pos2) + 1;
		QString tmp_qstr = str.sliced(from_pos, pos2 - from_pos);
		qsize_t max_plen = pos2 - from_pos + (4-1) * tmp_qstr.count('\t');
		// Remove length of "\treg  " (4+3+2)
		max_plen -= 4 + 3 + 2;
		maxp = 0;
		for(const QString& it : ans_pre){
			if(it.length() > maxp) maxp = it.length();
		}
		if(maxp > max_plen){
			max_plen = maxp - max_plen;
		}else{
			output_pad = max_plen - maxp;
			max_plen = 0;
		}
		str.replace("*BLANK*", QString(max_plen, ' '));
	}else if(pos2 != -1){
		str.replace("*BLANK*", QString(12, ' '));
	}
	if(pos1 != -1){
		QString def_str = "";
		for(qsize_t i = 0; i < dat_out.length(); ++i){
			def_str += "\treg  " + ans_pre[i];
			def_str += QString(output_pad, ' ');
			def_str += dat_out[i];
			def_str += ";\n";
		}
		str.replace("*ANS_OUT_DEF*", def_str);
	}
	str.replace("*IN_NUM*", QString::number(dat_in.length()));
	str.replace("*OUT_NUM*", QString::number(dat_out.length()));
	str.replace("*IN_VARS*", dat_in.join(','));
	str.replace("*OUT_VARS*", dat_out.join(','));

	list = QStringList(dat_in.length(), "%d");
	str.replace("*IN_STR*", list.join(' '));
	list = QStringList(dat_out.length(), "%d");
	str.replace("*OUT_STR*", list.join(' '));

	if(dat_in.empty()){
		str.replace("*IN_ST*", "/*\n");
		str.replace("*IN_END*", "*/\n");
	}else{
		str.replace("*IN_ST*", "");
		str.replace("*IN_END*", "");
	}
	if(dat_out.empty()){
		str.replace("*OUT_ST*", "/*\n");
		str.replace("*OUT_END*", "*/\n");
	}else{
		str.replace("*OUT_ST*", "");
		str.replace("*OUT_END*", "");
	}

	QString neq_str = "";
	for(qsize_t i = 0; i < dat_out.length(); ++i){
		QString cur_neq = checkNeq;
		cur_neq.replace("*ANS_OUT*", dat_out[i]);
		cur_neq.replace("*REAL_OUT*", real_out[i]);
		neq_str += cur_neq;
	}
	str.replace("*CHECK_NEQUAL*", neq_str);
	s << str;
	closeFile(f, s);

	// Generate doFile

	if(!doFile.isEmpty()){
		if(!openFile(doFile, f, s)){
			closeFile(f, s);
			return false;
		}
		s << "vlog -work work -stats=none " << file << endl;
		s << "vlog -work work -stats=none " << tbFile << endl;
		s << "vsim -gui work." << module_tb << endl;
		s << "add wave sim:/" << module_tb << "/*" << endl;
		s << "run -all" << endl;
		closeFile(f, s);
	}

	// Final
	saveLog();
	str = "Successfully generated the following files:";
	str += '\n' + file;
	str += '\n' + tbFile;
	if(!doFile.isEmpty())
		str += '\n' + doFile;
	QMessageBox::information(this, "Success", str);
	return true;
}

QDataStream& operator>>(QDataStream& in, MainWindow::rowEntry& entry){
	return in >> entry.name >> entry.io >> entry.sign >> entry.wire
			  >> entry.tbChecked >> entry.len;
}

QDataStream& operator<<(QDataStream& out, const MainWindow::rowEntry& entry){
	return out << entry.name << entry.io << entry.sign << entry.wire
			   << entry.tbChecked << entry.len;
}
