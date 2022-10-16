#ifndef BINCHECK_H
#define BINCHECK_H

#include <QGroupBox>
#include <QRadioButton>
#include <QString>

class BinCheck : public QGroupBox{
	Q_OBJECT

public:
	enum Stat{
		NO_CHECK,
		CHECK_0,
		CHECK_1,
	};

public:
	explicit BinCheck(const QString& no,
					  const QString& yes,
					  bool canSelectNull = false,
					  QWidget* parent = nullptr);
	void set(bool yes);
	void reset();
	void set(Stat stat);
	Stat get();
	~BinCheck() = default;

signals:
	void isSet(bool checked);

private slots:
	void setBox0(bool checked);
	void setBox1(bool checked);

private:
	QRadioButton* box0;
	QRadioButton* box1;
	bool selNull;
	bool pauseSig;
};

#endif // BINCHECK_H
