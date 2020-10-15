#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QFile>
#include <QTextStream>
#include <QString>
#include <QPair>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = nullptr);
	int buttonStat(int row, int col);
	QTableWidget* table();
	~MainWindow();

public slots:
	//void removeRow(int row);

private slots:
	void _addRow();

	void on_toolButton_clicked();

	void on_pushButton_clicked();

private:
	Ui::MainWindow *ui;
	QIcon delIcon,addIcon;

	QFile f;
	QTextStream s;
	QString curPath;
	static const QString header;
	static const QString params;
	static const QString content;
	static const QString paramsNoClk;
	static const QString contentNoClk;

	typedef  QPair<QString,int> pair1;
	typedef  QPair<int,QString> pair2;
	typedef  QPair<pair1,pair2> myPair;

	bool openFile(const QString &fileName);
	void closeFile();
	void addRow(bool useLast=true,
				const QString& name="",
				int io=0,
				int wire=0,
				int sign=0,
				const QString& lenm1="");
	void printIO(bool& p, const QString &name, const QString &pad);

	bool readLog();
	void saveLog();

};

#endif // MAINWINDOW_H