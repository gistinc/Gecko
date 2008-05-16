#ifndef __QMozView_h_
#define __QMozView_h_

#include <QWidget>

class QMozViewListener;

class QMozView : public QWidget
{
  Q_OBJECT

public:
  QMozView(QWidget *parent = 0);
  virtual ~QMozView();

  void loadUri(const QString& uri);

signals:
  void locationChanged(const QString& newUri);
  void titleChanged(const QString& newTitle);
  void statusChanged(const QString& newStatus);

protected:
  void resizeEvent(QResizeEvent* event);

private:
  class Private;
  Private* mPrivate;

  friend class QMozViewListener;
};

#endif /* __QMozView_h_ */

