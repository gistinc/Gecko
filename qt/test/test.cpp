#include "test.h"
#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QUrl>
#include <QDebug>

#include "QMozView.h"
#include "QMozApp.h"

MyBrowser::MyBrowser(QWidget *parent)
: QWidget(parent)
{
  QVBoxLayout* layout = new QVBoxLayout(this);
  
  location = new QLineEdit(this);
  layout->addWidget(location);

  mozView = new QMozView(this);
  layout->addWidget(mozView, 1);
  mozView->loadUri("http://mozilla.org");
  
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

  return app.exec();
}
