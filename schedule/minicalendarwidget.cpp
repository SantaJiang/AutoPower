#pragma execution_character_set("utf-8")

#include "minicalendarwidget.h"
#include "qfontdatabase.h"
#include "qdatetime.h"
#include "qlayout.h"
#include "qlabel.h"
#include "qtoolbutton.h"
#include <QPainter>

MiniCalendarWidget::MiniCalendarWidget(QWidget *parent) : QWidget(parent)
{
    setWindowFlags(Qt::Popup);

    //判断图形字体是否存在,不存在则加入
    QFontDatabase fontDb;
    if (!fontDb.families().contains("FontAwesome")) {
        int fontId = fontDb.addApplicationFont("://fontawesome-webfont.ttf");
        QStringList fontName = fontDb.applicationFontFamilies(fontId);
    }

    if (fontDb.families().contains("FontAwesome")) {
        iconFont = QFont("FontAwesome");
        iconFont.setHintingPreference(QFont::PreferNoHinting);
    }

    btnClick = false;

    calendarStyle = CalendarStyle_Red;
    weekNameFormat = WeekNameFormat_Short;
    date = QDate::currentDate();

    weekTextColor = QColor(0, 0, 0);
    weekBgColor = QColor(255, 255, 255);

    showLunar = false;
    enableBorder = false;
    selectType = SelectType_Circle;

    borderColor = QColor(180, 180, 180);
    weekColor = QColor(0, 0, 0);
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
    otherBgColor = QColor(255, 255, 255);
    selectBgColor = QColor(55, 144, 250);
    hoverBgColor = QColor(180, 180, 180);

    initWidget();
    initStyle();
    initDate();
}

MiniCalendarWidget::~MiniCalendarWidget()
{
}

void MiniCalendarWidget::initWidget()
{
    setObjectName("miniCalendarWidget");

    //顶部widget
    QWidget *widgetTop = new QWidget();
    widgetTop->setObjectName("widgetTop");
    widgetTop->setMinimumHeight(36);
    widgetTop->setContentsMargins(0,0,0,1);
    widgetTop->setStyleSheet("QWidget#widgetTop{background:rgb(255,255,255);}");

    //上个月按钮
    QToolButton *btnPrevMonth = new QToolButton();
    btnPrevMonth->setObjectName("btnPrevMonth");
    btnPrevMonth->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    btnPrevMonth->setFont(iconFont);
    btnPrevMonth->setText(QChar(0xf060));

    //下个月按钮
    QToolButton *btnNextMonth = new QToolButton();
    btnNextMonth->setObjectName("btnNextMonth");
    btnNextMonth->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    btnNextMonth->setFont(iconFont);
    btnNextMonth->setText(QChar(0xf061));

    labelYearMonth = new QLabel();
    labelYearMonth->setFixedSize(96,36);
    labelYearMonth->setAlignment(Qt::AlignCenter);
    QFont labelFont;
    labelFont.setFamily("Microsoft Yahei");
    labelFont.setPixelSize(14);
    labelYearMonth->setFont(labelFont);

    //顶部横向布局
    QHBoxLayout *layoutTop = new QHBoxLayout(widgetTop);
    layoutTop->setContentsMargins(0, 0, 0, 0);
    layoutTop->addWidget(btnPrevMonth);
    layoutTop->addWidget(labelYearMonth);
    layoutTop->addWidget(btnNextMonth);

    //星期widget
    QWidget *widgetWeek = new QWidget();
    widgetWeek->setObjectName("widgetWeek");
    widgetWeek->setMinimumHeight(30);

    //星期布局
    QHBoxLayout *layoutWeek = new QHBoxLayout(widgetWeek);
    layoutWeek->setMargin(0);
    layoutWeek->setSpacing(0);

    for (int i = 0; i < 7; i++)
    {
        QLabel *lab = new QLabel;
        lab->setAlignment(Qt::AlignCenter);
        layoutWeek->addWidget(lab);
        labWeeks.append(lab);
    }

    setWeekNameFormat(WeekNameFormat_Long);

    //日期标签widget
    QWidget *widgetBody = new QWidget();
    widgetBody->setObjectName("widgetBody");

    //日期标签布局
    QGridLayout *layoutBody = new QGridLayout(widgetBody);
    layoutBody->setMargin(0);
    layoutBody->setHorizontalSpacing(0);
    layoutBody->setVerticalSpacing(0);

    //逐个添加日标签
    for (int i = 0; i < 42; i++)
    {
        MiniCalendarItem *lab = new MiniCalendarItem();
        connect(lab, SIGNAL(clicked(QDate, MiniCalendarItem::DayType)), this, SLOT(slotClicked(QDate, MiniCalendarItem::DayType)));
        layoutBody->addWidget(lab, i / 7, i % 7);
        dayItems.append(lab);
    }

    //主布局
    QVBoxLayout *verLayoutCalendar = new QVBoxLayout(this);
    verLayoutCalendar->setMargin(1);
    verLayoutCalendar->setSpacing(0);
    verLayoutCalendar->addWidget(widgetTop);
    verLayoutCalendar->addWidget(widgetWeek);
    verLayoutCalendar->addWidget(widgetBody, 1);

    //绑定按钮和下拉框信号
    connect(btnPrevMonth, SIGNAL(clicked(bool)), this, SLOT(showPreviousMonth()));
    connect(btnNextMonth, SIGNAL(clicked(bool)), this, SLOT(showNextMonth()));
}

void MiniCalendarWidget::initStyle()
{
    //设置样式
    QStringList qss;
    //星期名称样式
    qss.append(QString("QLabel{background:%1;color:%2;}").arg(weekBgColor.name()).arg(weekTextColor.name()));
    //
    qss.append("QWidget#widgetTop{border-style: solid;border-width: 0px 0px 1px 0px;border-color: rgb(0, 120, 212);}");
    //边框
    qss.append("QWidget#widgetBody{border:0px;}");
    //顶部下拉框
    qss.append(QString("QToolButton{padding:0px;background:none;border:none;border-radius:5px;}"));
    qss.append(QString("QToolButton:hover{background:rgb(230,230,230);color:rgb(0,0,0);}"));

    //自定义日控件颜色
    QString strSelectType;
    if (selectType == SelectType_Rect) {
        strSelectType = "SelectType_Rect";
    } else if (selectType == SelectType_Circle) {
        strSelectType = "SelectType_Circle";
    } else if (selectType == SelectType_Triangle) {
        strSelectType = "SelectType_Triangle";
    }

    qss.append(QString("MiniCalendarItem{qproperty-showLunar:%1;}").arg(showLunar));
    qss.append(QString("MiniCalendarItem{qproperty-selectType:%1;}").arg(strSelectType));
    qss.append(QString("MiniCalendarItem{qproperty-borderColor:%1;}").arg(borderColor.name()));
    qss.append(QString("MiniCalendarItem{qproperty-weekColor:%1;}").arg(weekColor.name()));
    qss.append(QString("MiniCalendarItem{qproperty-lunarColor:%1;}").arg(lunarColor.name()));
    qss.append(QString("MiniCalendarItem{qproperty-currentTextColor:%1;}").arg(currentTextColor.name()));
    qss.append(QString("MiniCalendarItem{qproperty-otherTextColor:%1;}").arg(otherTextColor.name()));
    qss.append(QString("MiniCalendarItem{qproperty-selectTextColor:%1;}").arg(selectTextColor.name()));
    qss.append(QString("MiniCalendarItem{qproperty-hoverTextColor:%1;}").arg(hoverTextColor.name()));
    qss.append(QString("MiniCalendarItem{qproperty-currentLunarColor:%1;}").arg(currentLunarColor.name()));
    qss.append(QString("MiniCalendarItem{qproperty-otherLunarColor:%1;}").arg(otherLunarColor.name()));
    qss.append(QString("MiniCalendarItem{qproperty-selectLunarColor:%1;}").arg(selectLunarColor.name()));
    qss.append(QString("MiniCalendarItem{qproperty-hoverLunarColor:%1;}").arg(hoverLunarColor.name()));
    qss.append(QString("MiniCalendarItem{qproperty-currentBgColor:%1;}").arg(currentBgColor.name()));
    qss.append(QString("MiniCalendarItem{qproperty-otherBgColor:%1;}").arg(otherBgColor.name()));
    qss.append(QString("MiniCalendarItem{qproperty-selectBgColor:%1;}").arg(selectBgColor.name()));
    qss.append(QString("MiniCalendarItem{qproperty-hoverBgColor:%1;}").arg(hoverBgColor.name()));

    this->setStyleSheet(qss.join(""));
}

//初始化日期面板
void MiniCalendarWidget::initDate()
{
    int year = date.year();
    int month = date.month();

    //设置为今天,设置变量防止重复触发
    btnClick = true;
    labelYearMonth->setText(QString("%1年%2月").arg(year).arg(month));
    btnClick = false;

    //首先判断当前月的第一天是星期几
    int week = LunarCalendarInfo::Instance()->getFirstDayOfWeek(year, month, false);
    //当前月天数
    int countDay = LunarCalendarInfo::Instance()->getMonthDays(year, month);
    //上月天数
    int countDayPre = LunarCalendarInfo::Instance()->getMonthDays(1 == month ? year - 1 : year, 1 == month ? 12 : month - 1);

    //如果上月天数上月刚好一周则另外处理
    int startPre, endPre, startNext, endNext, index, tempYear, tempMonth, tempDay;
    if (0 == week) {
        startPre = 0;
        endPre = 7;
        startNext = 0;
        endNext = 42 - (countDay + 7);
    } else {
        startPre = 0;
        endPre = week;
        startNext = week + countDay;
        endNext = 42;
    }

    //纠正1月份前面部分偏差,1月份前面部分是上一年12月份
    tempYear = year;
    tempMonth = month - 1;
    if (tempMonth < 1) {
        tempYear--;
        tempMonth = 12;
    }

    //显示上月天数
    for (int i = startPre; i < endPre; i++) {
        index = i;
        tempDay = countDayPre - endPre + i + 1;

        QDate date(tempYear, tempMonth, tempDay);
        QString lunar = LunarCalendarInfo::Instance()->getLunarDay(tempYear, tempMonth, tempDay);
        dayItems.at(index)->setDate(date, lunar, MiniCalendarItem::DayType_MonthPre);
    }

    //纠正12月份后面部分偏差,12月份后面部分是下一年1月份
    tempYear = year;
    tempMonth = month + 1;
    if (tempMonth > 12) {
        tempYear++;
        tempMonth = 1;
    }

    //显示下月天数
    for (int i = startNext; i < endNext; i++) {
        index = 42 - endNext + i;
        tempDay = i - startNext + 1;

        QDate date(tempYear, tempMonth, tempDay);
        QString lunar = LunarCalendarInfo::Instance()->getLunarDay(tempYear, tempMonth, tempDay);
        dayItems.at(index)->setDate(date, lunar, MiniCalendarItem::DayType_MonthNext);
    }

    //重新置为当前年月
    tempYear = year;
    tempMonth = month;

    //显示当前月
    for (int i = week; i < (countDay + week); i++) {
        index = (0 == week ? (i + 7) : i);
        tempDay = i - week + 1;

        QDate date(tempYear, tempMonth, tempDay);
        QString lunar = LunarCalendarInfo::Instance()->getLunarDay(tempYear, tempMonth, tempDay);
        if (0 == (i % 7) || 6 == (i % 7)) {
            dayItems.at(index)->setDate(date, lunar, MiniCalendarItem::DayType_WeekEnd);
        } else {
            dayItems.at(index)->setDate(date, lunar, MiniCalendarItem::DayType_MonthCurrent);
        }
    }

    dayChanged(this->date);
}

void MiniCalendarWidget::slotClicked(const QDate &date, const MiniCalendarItem::DayType &dayType)
{
    this->date = date;
    if (MiniCalendarItem::DayType_MonthPre == dayType)
    {
        this->date = date.addMonths(1);
        showPreviousMonth();
    }
    else if (MiniCalendarItem::DayType_MonthNext == dayType)
    {
        this->date = date.addMonths(-1);
        showNextMonth();
    }
    else
    {
        dayChanged(this->date);
    }

    this->close();
}

void MiniCalendarWidget::dayChanged(const QDate &date)
{
    //计算星期几,当前天对应标签索引=日期+星期几-1
    int year = date.year();
    int month = date.month();
    int day = date.day();
    int week = LunarCalendarInfo::Instance()->getFirstDayOfWeek(year, month, false);

    //选中当前日期,其他日期恢复,这里还有优化空间,比方说类似单选框机制
    for (int i = 0; i < 42; i++)
    {
        //当月第一天是星期天要另外计算
        int index = day + week - 1;
        if (week == 0) {
            index = day + 6;
        }

        dayItems.at(i)->setSelect(i == index);
    }

    //发送日期单击信号
    emit clicked(date);
    //发送日期更新信号
    emit selectionChanged();
}

void MiniCalendarWidget::dateChanged(int year, int month, int day)
{
    //如果原有天大于28则设置为1,防止出错
    date.setDate(year, month, day > 28 ? 1 : day);
    initDate();
}

MiniCalendarWidget::CalendarStyle MiniCalendarWidget::getCalendarStyle() const
{
    return this->calendarStyle;
}

MiniCalendarWidget::WeekNameFormat MiniCalendarWidget::getWeekNameFormat() const
{
    return this->weekNameFormat;
}

QDate MiniCalendarWidget::getDate() const
{
    return this->date;
}

QColor MiniCalendarWidget::getWeekTextColor() const
{
    return this->weekTextColor;
}

QColor MiniCalendarWidget::getWeekBgColor() const
{
    return this->weekBgColor;
}

bool MiniCalendarWidget::getShowLunar() const
{
    return this->showLunar;
}

bool MiniCalendarWidget::getEnableBorder() const
{
    return this->enableBorder;
}

MiniCalendarWidget::SelectType MiniCalendarWidget::getSelectType() const
{
    return this->selectType;
}

QColor MiniCalendarWidget::getBorderColor() const
{
    return this->borderColor;
}

QColor MiniCalendarWidget::getWeekColor() const
{
    return this->weekColor;
}

QColor MiniCalendarWidget::getLunarColor() const
{
    return this->lunarColor;
}

QColor MiniCalendarWidget::getCurrentTextColor() const
{
    return this->currentTextColor;
}

QColor MiniCalendarWidget::getOtherTextColor() const
{
    return this->otherTextColor;
}

QColor MiniCalendarWidget::getSelectTextColor() const
{
    return this->selectTextColor;
}

QColor MiniCalendarWidget::getHoverTextColor() const
{
    return this->hoverTextColor;
}

QColor MiniCalendarWidget::getCurrentLunarColor() const
{
    return this->currentLunarColor;
}

QColor MiniCalendarWidget::getOtherLunarColor() const
{
    return this->otherLunarColor;
}

QColor MiniCalendarWidget::getSelectLunarColor() const
{
    return this->selectLunarColor;
}

QColor MiniCalendarWidget::getHoverLunarColor() const
{
    return this->hoverLunarColor;
}

QColor MiniCalendarWidget::getCurrentBgColor() const
{
    return this->currentBgColor;
}

QColor MiniCalendarWidget::getOtherBgColor() const
{
    return this->otherBgColor;
}

QColor MiniCalendarWidget::getSelectBgColor() const
{
    return this->selectBgColor;
}

QColor MiniCalendarWidget::getHoverBgColor() const
{
    return this->hoverBgColor;
}

QSize MiniCalendarWidget::sizeHint() const
{
    return QSize(600, 500);
}

QSize MiniCalendarWidget::minimumSizeHint() const
{
    return QSize(200, 150);
}

//显示上月日期
void MiniCalendarWidget::showPreviousMonth()
{
    int year = date.year();
    int month = date.month();
    int day = date.day();
    if (year <= 1901 && month == 1) {
        return;
    }

    month--;
    if (month < 1) {
        month = 12;
        year--;
    }

    dateChanged(year, month, day);
}

//显示下月日期
void MiniCalendarWidget::showNextMonth()
{
    int year = date.year();
    int month = date.month();
    int day = date.day();
    if (year >= 2099 && month == 12) {
        return;
    }

    month++;
    if (month > 12) {
        month = 1;
        year++;
    }

    dateChanged(year, month, day);
}

//转到今天
void MiniCalendarWidget::showToday()
{
    date = QDate::currentDate();
    initDate();
    dayChanged(date);
}

void MiniCalendarWidget::setCalendarStyle(const MiniCalendarWidget::CalendarStyle &calendarStyle)
{
    if (this->calendarStyle != calendarStyle) {
        this->calendarStyle = calendarStyle;
    }
}

void MiniCalendarWidget::setWeekNameFormat(const MiniCalendarWidget::WeekNameFormat &weekNameFormat)
{
    if (this->weekNameFormat != weekNameFormat) {
        this->weekNameFormat = weekNameFormat;

        QStringList listWeek;
        if (weekNameFormat == WeekNameFormat_Short) {
            listWeek << "一" << "二" << "三" << "四" << "五" << "六" << "日" ;
        } else if (weekNameFormat == WeekNameFormat_Normal) {
            listWeek << "周一" << "周二" << "周三" << "周四" << "周五" << "周六" << "周日";
        } else if (weekNameFormat == WeekNameFormat_Long) {
            listWeek << "星期一" << "星期二" << "星期三" << "星期四" << "星期五" << "星期六" << "星期天";
        } else if (weekNameFormat == WeekNameFormat_En) {
            listWeek << "Mon" << "Tue" << "Wed" << "Thu" << "Fri" << "Sat" << "Sun";
        }

        //逐个添加日期文字
        for (int i = 0; i < 7; i++) {
            labWeeks.at(i)->setText(listWeek.at(i));
        }
    }
}

void MiniCalendarWidget::setDate(const QDate &date)
{
    if (this->date != date) {
        this->date = date;
        initDate();
    }
}

void MiniCalendarWidget::setWeekTextColor(const QColor &weekTextColor)
{
    if (this->weekTextColor != weekTextColor) {
        this->weekTextColor = weekTextColor;
        initStyle();
    }
}

void MiniCalendarWidget::setWeekBgColor(const QColor &weekBgColor)
{
    if (this->weekBgColor != weekBgColor) {
        this->weekBgColor = weekBgColor;
        initStyle();
    }
}

void MiniCalendarWidget::setShowLunar(bool showLunar)
{
    if (this->showLunar != showLunar) {
        this->showLunar = showLunar;
        initStyle();
    }
}

void MiniCalendarWidget::setEnableBorder(bool bVisible)
{
    if (this->enableBorder != bVisible) {
        this->enableBorder = bVisible;
        update();
    }
}

void MiniCalendarWidget::setSelectType(const MiniCalendarWidget::SelectType &selectType)
{
    if (this->selectType != selectType) {
        this->selectType = selectType;
        initStyle();
    }
}

void MiniCalendarWidget::setBorderColor(const QColor &borderColor)
{
    if (this->borderColor != borderColor) {
        this->borderColor = borderColor;
        initStyle();
    }
}

void MiniCalendarWidget::setWeekColor(const QColor &weekColor)
{
    if (this->weekColor != weekColor) {
        this->weekColor = weekColor;
        initStyle();
    }
}

void MiniCalendarWidget::setLunarColor(const QColor &lunarColor)
{
    if (this->lunarColor != lunarColor) {
        this->lunarColor = lunarColor;
        initStyle();
    }
}

void MiniCalendarWidget::setCurrentTextColor(const QColor &currentTextColor)
{
    if (this->currentTextColor != currentTextColor) {
        this->currentTextColor = currentTextColor;
        initStyle();
    }
}

void MiniCalendarWidget::setOtherTextColor(const QColor &otherTextColor)
{
    if (this->otherTextColor != otherTextColor) {
        this->otherTextColor = otherTextColor;
        initStyle();
    }
}

void MiniCalendarWidget::setSelectTextColor(const QColor &selectTextColor)
{
    if (this->selectTextColor != selectTextColor) {
        this->selectTextColor = selectTextColor;
        initStyle();
    }
}

void MiniCalendarWidget::setHoverTextColor(const QColor &hoverTextColor)
{
    if (this->hoverTextColor != hoverTextColor) {
        this->hoverTextColor = hoverTextColor;
        initStyle();
    }
}

void MiniCalendarWidget::setCurrentLunarColor(const QColor &currentLunarColor)
{
    if (this->currentLunarColor != currentLunarColor) {
        this->currentLunarColor = currentLunarColor;
        initStyle();
    }
}

void MiniCalendarWidget::setOtherLunarColor(const QColor &otherLunarColor)
{
    if (this->otherLunarColor != otherLunarColor) {
        this->otherLunarColor = otherLunarColor;
        initStyle();
    }
}

void MiniCalendarWidget::setSelectLunarColor(const QColor &selectLunarColor)
{
    if (this->selectLunarColor != selectLunarColor) {
        this->selectLunarColor = selectLunarColor;
        initStyle();
    }
}

void MiniCalendarWidget::setHoverLunarColor(const QColor &hoverLunarColor)
{
    if (this->hoverLunarColor != hoverLunarColor) {
        this->hoverLunarColor = hoverLunarColor;
        initStyle();
    }
}

void MiniCalendarWidget::setCurrentBgColor(const QColor &currentBgColor)
{
    if (this->currentBgColor != currentBgColor) {
        this->currentBgColor = currentBgColor;
        initStyle();
    }
}

void MiniCalendarWidget::setOtherBgColor(const QColor &otherBgColor)
{
    if (this->otherBgColor != otherBgColor) {
        this->otherBgColor = otherBgColor;
        initStyle();
    }
}

void MiniCalendarWidget::setSelectBgColor(const QColor &selectBgColor)
{
    if (this->selectBgColor != selectBgColor) {
        this->selectBgColor = selectBgColor;
        initStyle();
    }
}

void MiniCalendarWidget::setHoverBgColor(const QColor &hoverBgColor)
{
    if (this->hoverBgColor != hoverBgColor) {
        this->hoverBgColor = hoverBgColor;
        initStyle();
    }
}

void MiniCalendarWidget::paintEvent(QPaintEvent *)
{
    //绘制准备工作,启用反锯齿
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    painter.setPen(QColor(0,120,212));
    painter.setBrush(QColor(255,255,255));
    painter.drawRect(rect());
}

void MiniCalendarWidget::wheelEvent(QWheelEvent *event)
{
    if (event->angleDelta().ry() > 100)
    {
        showPreviousMonth();
    }
    else if (event->angleDelta().ry() < -100)
    {
        showNextMonth();
    }
}
