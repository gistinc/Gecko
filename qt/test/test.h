#ifndef __test_h_
#define __test_h_

#include <QDialog>
#include "QMozView.h"

class QUrl;
class QLineEdit;
class QLabel;

class MyQMozView : public QMozView
{
public:
  MyQMozView(QWidget *parent = 0, unsigned int flags = 0);
protected:
  QMozView* openWindow(unsigned int flags);
};

class MyBrowser : public QDialog
{
Q_OBJECT
public:
  MyBrowser(QWidget *parent = 0, unsigned int flags = 0);
  QMozView* getQMozView() {return mozView;}
  void loadUri(const QString& uri);
public slots:
  void go();
  void startModal();
  void exitModal();
private:
  QLineEdit* location;
  MyQMozView* mozView;
  QLabel* status;
};

#endif /* __test_h_ */
