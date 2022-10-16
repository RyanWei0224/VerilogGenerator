#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDataStream>
#include <QFile>
#include <QIcon>
#include <QMainWindow>
#include <QString>
#include <QTableWidget>
#include <QTextStream>

#include <tuple>

#include "bincheck.h"
#include "buttons.h"

namespace Ui {
	class MainWindow;
}

class MainWindow : public QMainWindow{
	Q_OBJECT

public:
	explicit MainWindow(QWidget* parent = nullptr);
	~MainWindow();

private slots:
	void _addRow();
	void buttonHandler(int row, RowButton::id_t id);
	void selectDir();
	bool generate();

private:
	typedef BinCheck::Stat CheckStat;

	CheckStat buttonStat(int row, int col);
	void setButtonStat(int row, int col, CheckStat stat);
	bool boxStat(int row, int col);
	void setBoxStat(int row, int col, bool stat);
	void addRow(bool useLast=true,
				const QString& name="",
				CheckStat io=CheckStat::CHECK_0,
				CheckStat wire=CheckStat::CHECK_0,
				CheckStat sign=CheckStat::CHECK_0,
				bool tbChecked=false,
				const QString& len="");
	void swapRows(int row1, int row2);

	bool openFile(const QString& fileName, QFile& f, QTextStream& s);
	void closeFile(QFile& f, QTextStream& s);

	bool readLog();
	void saveLog();

	int readTemplate();

	void printPins(QTextStream& s, bool& p, const QString& name, const QString& pad);

	Ui::MainWindow* ui;
	QIcon delIcon, addIcon, upIcon, downIcon;
	QTableWidget* table;

	QString curPath;

	struct rowEntry{
		QString name;
		CheckStat io, wire, sign;
		bool tbChecked;
		QString len;
	};

	friend QDataStream& operator>>(QDataStream& in, rowEntry& entry);
	friend QDataStream& operator<<(QDataStream& out, const rowEntry& entry);

	static QString clkName;
	static QString moduleContent;
	static QString header;
	static QString params;
	static QString checkNeq;
	static QString content;
	static QString paramsNoClk;
	static QString contentNoClk;

	static const int nameCol;
	static const int ioCol;
	static const int wireCol;
	static const int signCol;
	static const int tbCol;
	static const int lenCol;
	static const int buttonCol;
};

#endif // MAINWINDOW_H
