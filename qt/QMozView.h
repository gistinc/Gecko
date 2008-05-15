#ifndef __QMozView_h_
#define __QMozView_h_

#include <QWidget>

class QMozView : public QWidget
{
  Q_OBJECT

public:
  QMozView(QWidget *parent = 0);
  virtual ~QMozView();

  void loadUri(const QString& uri);

signals:
  void locationChanged(const QString& newUri);

protected:
  void resizeEvent(QResizeEvent* event);

private:
  class Private;
  Private* mPrivate;
};

#endif /* __QMozView_h_ */

