#ifndef __QMozView_h_
#define __QMozView_h_

#ifndef Q_MOZVIEW_EXPORT
#define Q_MOZVIEW_EXPORT
#endif

#include <QWidget>

class QMozViewListener;

class Q_MOZVIEW_EXPORT QMozView : public QWidget
{
  Q_OBJECT

public:
  QMozView(QWidget *parent = 0, unsigned int flags = 0);
  virtual ~QMozView();

  void loadUri(const QString& uri);

signals:
  void locationChanged(const QString& newUri);
  void titleChanged(const QString& newTitle);
  void statusChanged(const QString& newStatus);

protected:
  void resizeEvent(QResizeEvent* event);
  virtual QMozView* openWindow(unsigned int flags);

private:
  class Private;
  Private* mPrivate;

  friend class QMozViewListener;
};

#endif /* __QMozView_h_ */

