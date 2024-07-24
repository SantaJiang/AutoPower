#ifndef CUSTOMDATEEDIT_H
#define CUSTOMDATEEDIT_H

#include <QDateEdit>
#include <QMouseEvent>
#include "minicalendarwidget.h"

class CustomDateEdit : public QDateEdit {
    Q_OBJECT

public:
    CustomDateEdit(QWidget *parent = nullptr);
    void setShowLunar(bool showLunar);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private slots:
    void onDateSelected(const QDate &date);

private:
    MiniCalendarWidget *m_pCalendar;
};

#endif // CUSTOMDATEEDIT_H
