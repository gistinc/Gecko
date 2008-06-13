#ifndef __test_h_
#define __test_h_

#include <QWidget>

class QUrl;
class QLineEdit;
class QMozView;

class MyBrowser : public QWidget
{
Q_OBJECT
public:
  MyBrowser(QWidget *parent = 0);
public slots:
  void updateLocation(const QString& url);
  void updateTitle(const QString& title);
  void go();

private:
  QLineEdit* location;
  QMozView* mozView;
};

#endif /* __test_h_ */
