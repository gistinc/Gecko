#include "test.h"
#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QUrl>
#include <QDebug>

#include "QMozApp.h"

MyQMozView::MyQMozView(QWidget *parent, unsigned int flags)
  : QMozView(parent, flags)
{
}

QMozView* MyQMozView::openWindow(unsigned int flags)
{
  MyBrowser* newBrowser = new MyBrowser(0, flags);
  newBrowser->resize(400, 400);
  newBrowser->show();
  newBrowser->setAttribute(Qt::WA_DeleteOnClose);
  return newBrowser->getQMozView();
}

MyBrowser::MyBrowser(QWidget *parent, unsigned int flags)
: QWidget(parent)
{
  QVBoxLayout* layout = new QVBoxLayout(this);
  
  location = new QLineEdit(this);
  layout->addWidget(location);

  mozView = new MyQMozView(this, flags);
  layout->addWidget(mozView, 1);
  
  status = new QLabel(this);
  layout->addWidget(status);

  connect(mozView, SIGNAL(locationChanged(const QString&)),
    location, SLOT(setText(const QString&)));

  connect(mozView, SIGNAL(titleChanged(const QString&)),
    this, SLOT(setWindowTitle(const QString&)));

  connect(mozView, SIGNAL(statusChanged(const QString&)),
    status, SLOT(setText(const QString&)));

  connect(location, SIGNAL(returnPressed()),
    this, SLOT(go()));
  
  //webView->page()->mainFrame()->evaluateJavaScript("alert('embed');");
}

void MyBrowser::loadUri(const QString& uri)
{
  location->setText(uri);
  mozView->loadUri(uri);
}

void MyBrowser::go()
{
  mozView->loadUri(location->text());
}

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);

  MyBrowser window;
  
  window.resize(400, 400);
  window.show();

  if(argc > 1)
    window.loadUri(argv[argc - 1]);
  else
    window.loadUri("http://mozilla.org");

  return app.exec();
}
