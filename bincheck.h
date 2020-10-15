#ifndef BINCHECK_H
#define BINCHECK_H

#include <QWidget>
#include <QRadioButton>
#include <QGroupBox>
#include <QHBoxLayout>

class BinCheck : public QGroupBox
{
	Q_OBJECT
private:
	QRadioButton* box0;
	QRadioButton* box1;
	QHBoxLayout* layout;
	bool _selNull;

public:
	explicit BinCheck(const QString &no,
					  const QString &yes,
					  bool canSelectNull = false,
					  QWidget *parent = nullptr);
	void set(bool yes);
	void reset();
	int get();
	~BinCheck();

signals:

public slots:

private slots:
	void setBox0(bool checked);
	void setBox1(bool checked);
};

#endif // BINCHECK_H
