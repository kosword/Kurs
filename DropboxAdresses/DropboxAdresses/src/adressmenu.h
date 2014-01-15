#ifndef ADRESSMENU_H
#define ADRESSMENU_H

#include <QWidget>
#include <QMovie>
#include <QLabel>

namespace Ui {
class AdressMenu;
}

class AdressMenu : public QWidget
{
    Q_OBJECT

public:
    explicit AdressMenu(QWidget *parent = 0);
    ~AdressMenu();

private slots:
    void signOutButClicked();

public slots:
    void setAddEnable(bool val);
    void setDelEnable(bool val);
    void setEditEnable(bool val);
    void startMove();
    void stopMove();

signals:
    void signOutRequested();
    void addRecordRequested();
    void deleteRecordRequested();
    void editRecordRequested();

private:
    Ui::AdressMenu *ui;
    QMovie *loading;
};

#endif // ADRESSMENU_H
