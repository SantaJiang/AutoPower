#include "customdateedit.h"
#include <QStyleOptionComboBox>
#include <QPainter>
#include <QStyle>

CustomDateEdit::CustomDateEdit(QWidget *parent) : QDateEdit(parent), m_pCalendar(new MiniCalendarWidget(this)) {
    setCalendarPopup(true);
    setButtonSymbols(QAbstractSpinBox::NoButtons);
    m_pCalendar->setFixedSize(214,240);
    m_pCalendar->setWeekNameFormat(MiniCalendarWidget::WeekNameFormat_Short);
    connect(m_pCalendar, &MiniCalendarWidget::clicked, this, &CustomDateEdit::onDateSelected);
}

void CustomDateEdit::setShowLunar(bool showLunar)
{
    m_pCalendar->setShowLunar(showLunar);
}

void CustomDateEdit::mousePressEvent(QMouseEvent *event) {
    QStyleOptionComboBox opt;
    opt.initFrom(this);
    QRect arrowRect = style()->subControlRect(QStyle::CC_ComboBox, &opt, QStyle::SC_ComboBoxArrow, this);
    if(arrowRect.contains(event->pos()))
    {
        m_pCalendar->setDate(this->date());
        m_pCalendar->move(mapToGlobal(QPoint(0, height())));
        m_pCalendar->show();
    }
    else
    {
        QDateEdit::mousePressEvent(event);
    }
}

void CustomDateEdit::paintEvent(QPaintEvent *event) {
    QDateEdit::paintEvent(event);

    QStyleOptionComboBox opt;
    opt.initFrom(this);

    QPainter painter(this);
    style()->drawComplexControl(QStyle::CC_ComboBox, &opt, &painter, this);
}

void CustomDateEdit::onDateSelected(const QDate &date) {
    setDate(date);
}
