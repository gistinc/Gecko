#include "test.h"
#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QUrl>

#include "QMozView.h"

MyBrowser::MyBrowser(QWidget *parent)
: QWidget(parent)
{
  QVBoxLayout* layout = new QVBoxLayout(this);
  
  location = new QLineEdit(this);
  layout->addWidget(location);

  mozView = new QMozView(this);
  layout->addWidget(mozView);
  mozView->loadUri("http://mozilla.org");
  
  connect(mozView, SIGNAL(locationChanged(const QString&)),
    this, SLOT(updateLocation(const QString&)));

  connect(mozView, SIGNAL(titleChanged(const QString&)),
    this, SLOT(updateTitle(const QString&)));

  connect(location, SIGNAL(returnPressed()),
    this, SLOT(go()));
  
  //webView->page()->mainFrame()->evaluateJavaScript("alert('embed');");
}

void MyBrowser::updateLocation(const QString& url)
{
  location->setText(url);
}

void MyBrowser::updateTitle(const QString& title)
{
  setWindowTitle(title);
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
