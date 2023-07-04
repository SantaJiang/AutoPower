#ifndef TRAYICON_H
#define TRAYICON_H

#include <QObject>
#include <QSystemTrayIcon>
#include <QMenu>

class TrayIcon : public QObject
{
    Q_OBJECT
public:
    static TrayIcon *Instance();
    explicit TrayIcon(QObject *parent = 0);

private:
    static QScopedPointer<TrayIcon> self;
    QWidget *mainWidget;            //Corresponds to the owning master window
    QSystemTrayIcon *trayIcon;      //Tray objects
    QMenu *menu;                    //Right-click menu
    bool exitDirect;

private slots:
    void iconIsActived(QSystemTrayIcon::ActivationReason reason);

public:
    bool getVisible()       const;

public Q_SLOTS:
    //Set whether to exit directly. If not, send a signal to the main interface
    void setExitDirect(bool exitDirect);

    //Sets the owning master form
    void setMainWidget(QWidget *mainWidget);

    //Show message
    void showMessage(const QString &title, const QString &msg,
                     QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::Information, int msecs = 5000);

    void setIcon(const QString &strIcon);
    void setToolTip(const QString &tip);
    void setVisible(bool visible);
    void closeAll();

Q_SIGNALS:
    void trayIconExit();
};

#endif // TRAYICON_H
