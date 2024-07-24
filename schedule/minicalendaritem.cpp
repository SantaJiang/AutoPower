#pragma execution_character_set("utf-8")

#include "minicalendaritem.h"
#include "qpainter.h"
#include "qevent.h"
#include "qdatetime.h"

MiniCalendarItem::MiniCalendarItem(QWidget *parent) : QWidget(parent)
{
    hover = false;
    pressed = false;

    select = false;
    showLunar = true;
    enableBorder = false;
    selectType = SelectType_Rect;

    date = QDate::currentDate();
    lunar = "初一";
    dayType = DayType_MonthCurrent;

    borderColor = QColor(180, 180, 180);
    weekColor = QColor(255, 0, 0);
    superColor = QColor(255, 129, 6);
    lunarColor = QColor(55, 156, 238);

    currentTextColor = QColor(0, 0, 0);
    otherTextColor = QColor(200, 200, 200);
    selectTextColor = QColor(255, 255, 255);
    hoverTextColor = QColor(250, 250, 250);

    currentLunarColor = QColor(150, 150, 150);
    otherLunarColor = QColor(200, 200, 200);
    selectLunarColor = QColor(255, 255, 255);
    hoverLunarColor = QColor(250, 250, 250);

    currentBgColor = QColor(255, 255, 255);
    otherBgColor = QColor(240, 240, 240);
    selectBgColor = QColor(208, 47, 18);
    hoverBgColor = QColor(204, 183, 180);
}

void MiniCalendarItem::enterEvent(QEvent *)
{
    hover = true;
    update();
}

void MiniCalendarItem::leaveEvent(QEvent *)
{
    hover = false;
    update();
}

void MiniCalendarItem::mousePressEvent(QMouseEvent *)
{
    pressed = true;
    update();

    emit clicked(date, dayType);
}

void MiniCalendarItem::mouseReleaseEvent(QMouseEvent *)
{
    pressed = false;
    update();
}

void MiniCalendarItem::paintEvent(QPaintEvent *)
{
    //绘制准备工作,启用反锯齿
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    //绘制背景和边框
    drawBg(&painter);

    //优先绘制选中状态,其次绘制悬停状态
    if (select) {
        drawBgCurrent(&painter, selectBgColor);
    } else if (hover) {
        drawBgCurrent(&painter, hoverBgColor);
    }

    //绘制日期
    drawDay(&painter);

    //绘制农历信息
    drawLunar(&painter);
}

void MiniCalendarItem::drawBg(QPainter *painter)
{
    painter->save();

    //根据当前类型选择对应的颜色
    QColor bgColor = currentBgColor;
    if (dayType == DayType_MonthPre || dayType == DayType_MonthNext) {
        bgColor = otherBgColor;
    }

    if(getEnableBorder())
        painter->setPen(borderColor);
    else
        painter->setPen(bgColor);

    painter->setBrush(bgColor);
    painter->drawRect(rect());

    painter->restore();
}

void MiniCalendarItem::drawBgCurrent(QPainter *painter, const QColor &color)
{
    int width = this->width();
    int height = this->height();
    int side = qMin(width, height);

    painter->save();

    painter->setPen(Qt::NoPen);
    painter->setBrush(color);

    //根据设定绘制背景样式
    if (selectType == SelectType_Rect)
    {
        painter->drawRect(rect());
    }
    else if (selectType == SelectType_Circle)
    {
        int radius = side / 2;
        painter->drawEllipse(QPointF(width / 2, height / 2), radius, radius);
    }
    else if (selectType == SelectType_Triangle) {
        int radius = side / 3;
        QPolygon pts;
        pts.setPoints(3, 1, 1, radius, 1, 1, radius);
        painter->drawRect(rect());
        painter->setBrush(superColor);
        painter->drawConvexPolygon(pts);
    }

    painter->restore();
}

void MiniCalendarItem::drawDay(QPainter *painter)
{
    int width = this->width();
    int height = this->height();
    int side = qMin(width, height);

    painter->save();

    //根据当前类型选择对应的颜色
    QColor color = currentTextColor;
    if (dayType == DayType_MonthPre || dayType == DayType_MonthNext) {
        color = otherTextColor;
    } else if (dayType == DayType_WeekEnd) {
        color = weekColor;
    }

    if (select) {
        color = selectTextColor;
    } else if (hover) {
        color = hoverTextColor;
    }

    painter->setPen(color);

    if (showLunar) {
        QFont font;
        font.setPixelSize(side / 2.7);
        painter->setFont(font);

        QRect dayRect = QRect(0, 0, width, height / 1.5);
        painter->drawText(dayRect, Qt::AlignHCenter | Qt::AlignBottom, QString::number(date.day()));
    } else {
        QFont font;
        font.setPixelSize(side / 2);
        painter->setFont(font);

        QRect dayRect = QRect(0, 0, width, height);
        painter->drawText(dayRect, Qt::AlignCenter, QString::number(date.day()));
    }

    painter->restore();
}

void MiniCalendarItem::drawLunar(QPainter *painter)
{
    if (!showLunar) {
        return;
    }

    int width = this->width();
    int height = this->height();
    int side = qMin(width, height);

    painter->save();

    QStringList listDayName;
    listDayName << "*" << "初一" << "初二" << "初三" << "初四" << "初五" << "初六" << "初七" << "初八" << "初九" << "初十"
                << "十一" << "十二" << "十三" << "十四" << "十五" << "十六" << "十七" << "十八" << "十九" << "二十"
                << "廿一" << "廿二" << "廿三" << "廿四" << "廿五" << "廿六" << "廿七" << "廿八" << "廿九" << "三十";

    //根据当前类型选择对应的颜色
    QColor color = currentLunarColor;
    if (dayType == DayType_MonthPre || dayType == DayType_MonthNext) {
        color = otherLunarColor;
    }

    if (select) {
        color = selectTextColor;
    } else if (hover) {
        color = hoverTextColor;
    }

    painter->setPen(color);

    QFont font;
    font.setPixelSize(side / 4);
    font.setBold(true);
    painter->setFont(font);    

    QRect lunarRect(0, height / 2, width, height / 2.2);
    painter->drawText(lunarRect, Qt::AlignCenter, lunar);

    painter->restore();
}

bool MiniCalendarItem::getSelect() const
{
    return this->select;
}

bool MiniCalendarItem::getShowLunar() const
{
    return this->showLunar;
}

bool MiniCalendarItem::getEnableBorder() const
{
    return this->enableBorder;
}

MiniCalendarItem::SelectType MiniCalendarItem::getSelectType() const
{
    return this->selectType;
}

QDate MiniCalendarItem::getDate() const
{
    return this->date;
}

QString MiniCalendarItem::getLunar() const
{
    return this->lunar;
}

MiniCalendarItem::DayType MiniCalendarItem::getDayType() const
{
    return this->dayType;
}

QColor MiniCalendarItem::getBorderColor() const
{
    return this->borderColor;
}

QColor MiniCalendarItem::getWeekColor() const
{
    return this->weekColor;
}

QColor MiniCalendarItem::getSuperColor() const
{
    return this->superColor;
}

QColor MiniCalendarItem::getLunarColor() const
{
    return this->lunarColor;
}

QColor MiniCalendarItem::getCurrentTextColor() const
{
    return this->currentTextColor;
}

QColor MiniCalendarItem::getOtherTextColor() const
{
    return this->otherTextColor;
}

QColor MiniCalendarItem::getSelectTextColor() const
{
    return this->selectTextColor;
}

QColor MiniCalendarItem::getHoverTextColor() const
{
    return this->hoverTextColor;
}

QColor MiniCalendarItem::getCurrentLunarColor() const
{
    return this->currentLunarColor;
}

QColor MiniCalendarItem::getOtherLunarColor() const
{
    return this->otherLunarColor;
}

QColor MiniCalendarItem::getSelectLunarColor() const
{
    return this->selectLunarColor;
}

QColor MiniCalendarItem::getHoverLunarColor() const
{
    return this->hoverLunarColor;
}

QColor MiniCalendarItem::getCurrentBgColor() const
{
    return this->currentBgColor;
}

QColor MiniCalendarItem::getOtherBgColor() const
{
    return this->otherBgColor;
}

QColor MiniCalendarItem::getSelectBgColor() const
{
    return this->selectBgColor;
}

QColor MiniCalendarItem::getHoverBgColor() const
{
    return this->hoverBgColor;
}

QSize MiniCalendarItem::sizeHint() const
{
    return QSize(100, 100);
}

QSize MiniCalendarItem::minimumSizeHint() const
{
    return QSize(20, 20);
}

void MiniCalendarItem::setSelect(bool select)
{
    if (this->select != select) {
        this->select = select;
        update();
    }
}

void MiniCalendarItem::setShowLunar(bool showLunar)
{
    if (this->showLunar != showLunar) {
        this->showLunar = showLunar;
        update();
    }
}

void MiniCalendarItem::setSelectType(const MiniCalendarItem::SelectType &selectType)
{
    if (this->selectType != selectType) {
        this->selectType = selectType;
        update();
    }
}

void MiniCalendarItem::setDate(const QDate &date)
{
    if (this->date != date) {
        this->date = date;
        update();
    }
}

void MiniCalendarItem::setLunar(const QString &lunar)
{
    if (this->lunar != lunar) {
        this->lunar = lunar;
        update();
    }
}

void MiniCalendarItem::setDayType(const MiniCalendarItem::DayType &dayType)
{
    if (this->dayType != dayType) {
        this->dayType = dayType;
        update();
    }
}

void MiniCalendarItem::setDate(const QDate &date, const QString &lunar, const DayType &dayType)
{
    this->date = date;
    this->lunar = lunar;
    this->dayType = dayType;
    update();
}

void MiniCalendarItem::setEnableBorder(bool bVisible)
{
    if (this->enableBorder != bVisible) {
        this->enableBorder = bVisible;
        update();
    }
}

void MiniCalendarItem::setBorderColor(const QColor &borderColor)
{
    if (this->borderColor != borderColor) {
        this->borderColor = borderColor;
        update();
    }
}

void MiniCalendarItem::setWeekColor(const QColor &weekColor)
{
    if (this->weekColor != weekColor) {
        this->weekColor = weekColor;
        update();
    }
}

void MiniCalendarItem::setSuperColor(const QColor &superColor)
{
    if (this->superColor != superColor) {
        this->superColor = superColor;
        update();
    }
}

void MiniCalendarItem::setLunarColor(const QColor &lunarColor)
{
    if (this->lunarColor != lunarColor) {
        this->lunarColor = lunarColor;
        update();
    }
}

void MiniCalendarItem::setCurrentTextColor(const QColor &currentTextColor)
{
    if (this->currentTextColor != currentTextColor) {
        this->currentTextColor = currentTextColor;
        update();
    }
}

void MiniCalendarItem::setOtherTextColor(const QColor &otherTextColor)
{
    if (this->otherTextColor != otherTextColor) {
        this->otherTextColor = otherTextColor;
        update();
    }
}

void MiniCalendarItem::setSelectTextColor(const QColor &selectTextColor)
{
    if (this->selectTextColor != selectTextColor) {
        this->selectTextColor = selectTextColor;
        update();
    }
}

void MiniCalendarItem::setHoverTextColor(const QColor &hoverTextColor)
{
    if (this->hoverTextColor != hoverTextColor) {
        this->hoverTextColor = hoverTextColor;
        update();
    }
}

void MiniCalendarItem::setCurrentLunarColor(const QColor &currentLunarColor)
{
    if (this->currentLunarColor != currentLunarColor) {
        this->currentLunarColor = currentLunarColor;
        update();
    }
}

void MiniCalendarItem::setOtherLunarColor(const QColor &otherLunarColor)
{
    if (this->otherLunarColor != otherLunarColor) {
        this->otherLunarColor = otherLunarColor;
        update();
    }
}

void MiniCalendarItem::setSelectLunarColor(const QColor &selectLunarColor)
{
    if (this->selectLunarColor != selectLunarColor) {
        this->selectLunarColor = selectLunarColor;
        update();
    }
}

void MiniCalendarItem::setHoverLunarColor(const QColor &hoverLunarColor)
{
    if (this->hoverLunarColor != hoverLunarColor) {
        this->hoverLunarColor = hoverLunarColor;
        update();
    }
}

void MiniCalendarItem::setCurrentBgColor(const QColor &currentBgColor)
{
    if (this->currentBgColor != currentBgColor) {
        this->currentBgColor = currentBgColor;
        update();
    }
}

void MiniCalendarItem::setOtherBgColor(const QColor &otherBgColor)
{
    if (this->otherBgColor != otherBgColor) {
        this->otherBgColor = otherBgColor;
        update();
    }
}

void MiniCalendarItem::setSelectBgColor(const QColor &selectBgColor)
{
    if (this->selectBgColor != selectBgColor) {
        this->selectBgColor = selectBgColor;
        update();
    }
}

void MiniCalendarItem::setHoverBgColor(const QColor &hoverBgColor)
{
    if (this->hoverBgColor != hoverBgColor) {
        this->hoverBgColor = hoverBgColor;
        update();
    }
}
