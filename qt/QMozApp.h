#ifndef __QMozApp_h_
#define __QMozApp_h_

#ifndef Q_MOZVIEW_EXPORT
#define Q_MOZVIEW_EXPORT
#endif

#include <QObject>

class QMozApp : public QObject
{
    Q_OBJECT

public:
    QMozApp();
    QMozApp(const QString& profilePath);
    virtual ~QMozApp();

    QString stringPref(const QString& name) const;
    void setStringPref(const QString& name, const QString& value);
    int intPref(const QString& name) const;
    void setIntPref(const QString& name, int value);
    bool boolPref(const QString& name) const;
    void setBoolPref(const QString& name, bool value);

private:
    class Private;
    Private* mPrivate;
};

#endif /* __QMozApp_h_ */
