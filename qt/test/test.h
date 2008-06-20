#ifndef __test_h_
#define __test_h_

#include <QWidget>

class QUrl;
class QLineEdit;
class QLabel;
class QMozView;

class MyBrowser : public QWidget
{
Q_OBJECT
public:
  MyBrowser(QWidget *parent = 0);
public slots:
  void go();

private:
  QLineEdit* location;
  QMozView* mozView;
  QLabel* status;
};

#endif /* __test_h_ */
