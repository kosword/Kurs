#ifndef EDITRECORDFORM_H
#define EDITRECORDFORM_H

#include <QDialog>

namespace Ui {
class EditRecordForm;
}

class EditRecordForm : public QDialog
{
    Q_OBJECT

public:
    explicit EditRecordForm(QString* _name, QString* _adress, QString* _phone, QWidget *parent = 0);
    ~EditRecordForm();

private slots:
    void nameChanged();
    void adressChanged();
    void phoneChanged();

private:
    Ui::EditRecordForm *ui;
    QString* name;
    QString* adress;
    QString* phone;
};

#endif // EDITRECORDFORM_H
