﻿#ifndef MINICALENDARWIDGET_H
#define MINICALENDARWIDGET_H

#include <QWidget>
#include <QDate>
#include <QWheelEvent>
#include "lunarcalendarinfo.h"
#include "minicalendaritem.h"

class QLabel;
class QComboBox;
class MiniCalendarItem;
class MiniCalendarWidget : public QWidget
{
    Q_OBJECT
    Q_ENUMS(CalendarStyle)
    Q_ENUMS(WeekNameFormat)
    Q_ENUMS(SelectType)

    Q_PROPERTY(CalendarStyle calendarStyle READ getCalendarStyle WRITE setCalendarStyle)
    Q_PROPERTY(WeekNameFormat weekNameFormat READ getWeekNameFormat WRITE setWeekNameFormat)
    Q_PROPERTY(QDate date READ getDate WRITE setDate)

    Q_PROPERTY(QColor weekTextColor READ getWeekTextColor WRITE setWeekTextColor)
    Q_PROPERTY(QColor weekBgColor READ getWeekBgColor WRITE setWeekBgColor)

    Q_PROPERTY(bool showLunar READ getShowLunar WRITE setShowLunar)
    Q_PROPERTY(bool enableBorder READ getEnableBorder WRITE setEnableBorder)
    Q_PROPERTY(SelectType selectType READ getSelectType WRITE setSelectType)

    Q_PROPERTY(QColor borderColor READ getBorderColor WRITE setBorderColor)
    Q_PROPERTY(QColor weekColor READ getWeekColor WRITE setWeekColor)
    Q_PROPERTY(QColor lunarColor READ getLunarColor WRITE setLunarColor)

    Q_PROPERTY(QColor currentTextColor READ getCurrentTextColor WRITE setCurrentTextColor)
    Q_PROPERTY(QColor otherTextColor READ getOtherTextColor WRITE setOtherTextColor)
    Q_PROPERTY(QColor selectTextColor READ getSelectTextColor WRITE setSelectTextColor)
    Q_PROPERTY(QColor hoverTextColor READ getHoverTextColor WRITE setHoverTextColor)

    Q_PROPERTY(QColor currentLunarColor READ getCurrentLunarColor WRITE setCurrentLunarColor)
    Q_PROPERTY(QColor otherLunarColor READ getOtherLunarColor WRITE setOtherLunarColor)
    Q_PROPERTY(QColor selectLunarColor READ getSelectLunarColor WRITE setSelectLunarColor)
    Q_PROPERTY(QColor hoverLunarColor READ getHoverLunarColor WRITE setHoverLunarColor)

    Q_PROPERTY(QColor currentBgColor READ getCurrentBgColor WRITE setCurrentBgColor)
    Q_PROPERTY(QColor otherBgColor READ getOtherBgColor WRITE setOtherBgColor)
    Q_PROPERTY(QColor selectBgColor READ getSelectBgColor WRITE setSelectBgColor)
    Q_PROPERTY(QColor hoverBgColor READ getHoverBgColor WRITE setHoverBgColor)

public:
    enum CalendarStyle {
        CalendarStyle_Red = 0
    };

    enum WeekNameFormat {
        WeekNameFormat_Short = 0,   //短名称模式
        WeekNameFormat_Normal = 1,  //普通名称模式
        WeekNameFormat_Long = 2,    //长名称模式
        WeekNameFormat_En = 3       //英文模式
    };

    enum SelectType {
        SelectType_Rect = 0,        //矩形背景
        SelectType_Circle = 1,      //圆形背景
        SelectType_Triangle = 2,    //带三角标
        SelectType_Image = 3        //图片背景
    };

    explicit MiniCalendarWidget(QWidget *parent = 0);
    ~MiniCalendarWidget();

public:
    CalendarStyle getCalendarStyle()    const;
    WeekNameFormat getWeekNameFormat()  const;
    QDate getDate()                     const;

    QColor getWeekTextColor()           const;
    QColor getWeekBgColor()             const;

    bool getShowLunar()                 const;
    bool getEnableBorder()              const;
    SelectType getSelectType()          const;

    QColor getBorderColor()             const;
    QColor getWeekColor()               const;
    QColor getLunarColor()              const;

    QColor getCurrentTextColor()        const;
    QColor getOtherTextColor()          const;
    QColor getSelectTextColor()         const;
    QColor getHoverTextColor()          const;

    QColor getCurrentLunarColor()       const;
    QColor getOtherLunarColor()         const;
    QColor getSelectLunarColor()        const;
    QColor getHoverLunarColor()         const;

    QColor getCurrentBgColor()          const;
    QColor getOtherBgColor()            const;
    QColor getSelectBgColor()           const;
    QColor getHoverBgColor()            const;

    QSize sizeHint()                    const;
    QSize minimumSizeHint()             const;

public Q_SLOTS:
    //上一月,下一月
    void showPreviousMonth();
    void showNextMonth();

    //转到今天
    void showToday();

    //设置整体样式
    void setCalendarStyle(const CalendarStyle &calendarStyle);
    //设置星期名称格式
    void setWeekNameFormat(const WeekNameFormat &weekNameFormat);

    //设置日期
    void setDate(const QDate &date);

    //设置顶部星期名称文字颜色+背景色
    void setWeekTextColor(const QColor &weekTextColor);
    void setWeekBgColor(const QColor &weekBgColor);

    //设置是否显示农历信息
    void setShowLunar(bool showLunar);
    //是否显示边框
    void setEnableBorder(bool bVisible);
    //设置选中背景样式
    void setSelectType(const SelectType &selectType);

    //设置边框颜色
    void setBorderColor(const QColor &borderColor);
    //设置周末颜色
    void setWeekColor(const QColor &weekColor);
    //设置农历节日颜色
    void setLunarColor(const QColor &lunarColor);

    //设置当前月文字颜色
    void setCurrentTextColor(const QColor &currentTextColor);
    //设置其他月文字颜色
    void setOtherTextColor(const QColor &otherTextColor);
    //设置选中日期文字颜色
    void setSelectTextColor(const QColor &selectTextColor);
    //设置悬停日期文字颜色
    void setHoverTextColor(const QColor &hoverTextColor);

    //设置当前月农历文字颜色
    void setCurrentLunarColor(const QColor &currentLunarColor);
    //设置其他月农历文字颜色
    void setOtherLunarColor(const QColor &otherLunarColor);
    //设置选中日期农历文字颜色
    void setSelectLunarColor(const QColor &selectLunarColor);
    //设置悬停日期农历文字颜色
    void setHoverLunarColor(const QColor &hoverLunarColor);

    //设置当前月背景颜色
    void setCurrentBgColor(const QColor &currentBgColor);
    //设置其他月背景颜色
    void setOtherBgColor(const QColor &otherBgColor);
    //设置选中日期背景颜色
    void setSelectBgColor(const QColor &selectBgColor);
    //设置悬停日期背景颜色
    void setHoverBgColor(const QColor &hoverBgColor);

protected:
    void paintEvent(QPaintEvent *) override;
    void wheelEvent(QWheelEvent *event) override;

signals:
    void clicked(const QDate &date);
    void selectionChanged();

private slots:
    void initWidget();
    void initStyle();
    void initDate();
    void slotClicked(const QDate &date, const MiniCalendarItem::DayType &dayType);
    void dayChanged(const QDate &date);
    void dateChanged(int year, int month, int day);

private:
    QFont iconFont;                     //图形字体
    bool btnClick;                      //按钮单击,避开下拉选择重复触发
    QLabel *labelYearMonth;             //年月
    QList<QLabel *> labWeeks;           //顶部星期名称
    QList<MiniCalendarItem *> dayItems;//日期元素

    CalendarStyle calendarStyle;        //整体样式
    WeekNameFormat weekNameFormat;      //星期名称格式
    QDate date;                         //当前日期

    QColor weekTextColor;               //星期名称文字颜色
    QColor weekBgColor;                 //星期名称背景色

    bool showLunar;                     //显示农历
    bool enableBorder;                  //显示边框
    SelectType selectType;              //选中模式

    QColor borderColor;                 //边框颜色
    QColor weekColor;                   //周末颜色
    QColor lunarColor;                  //农历节日颜色

    QColor currentTextColor;            //当前月文字颜色
    QColor otherTextColor;              //其他月文字颜色
    QColor selectTextColor;             //选中日期文字颜色
    QColor hoverTextColor;              //悬停日期文字颜色

    QColor currentLunarColor;           //当前月农历文字颜色
    QColor otherLunarColor;             //其他月农历文字颜色
    QColor selectLunarColor;            //选中日期农历文字颜色
    QColor hoverLunarColor;             //悬停日期农历文字颜色

    QColor currentBgColor;              //当前月背景颜色
    QColor otherBgColor;                //其他月背景颜色
    QColor selectBgColor;               //选中日期背景颜色
    QColor hoverBgColor;                //悬停日期背景颜色
};

#endif // MINICALENDARWIDGET_H
