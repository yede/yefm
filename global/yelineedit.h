#ifndef YE_LINEEDIT_H
#define YE_LINEEDIT_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
//==============================================================================================================================

class LineEditButton : public QLabel
{
	Q_OBJECT

public:
	LineEditButton(QWidget *parent = 0);

signals:
	void clicked();

protected:
	void mousePressEvent(QMouseEvent *event);
};
//==============================================================================================================================

class LineEditSideWidget : public QWidget
{
	Q_OBJECT

signals:
	void sizeHintChanged();

public:
	LineEditSideWidget(QWidget *parent = 0);

protected:
	bool event(QEvent *event);

};
//==============================================================================================================================

class QHBoxLayout;

/*
    LineEdit is a subclass of QLineEdit that provides an easy and simple
    way to add widgets on the left or right hand side of the text.

    The layout of the widgets on either side are handled by a QHBoxLayout.
    You can set the spacing around the widgets with setWidgetSpacing().

    As widgets are added to the class they are inserted from the outside
    into the center of the widget.
*/

class LineEdit : public QLineEdit
{
    Q_OBJECT
    Q_PROPERTY(QString inactiveText READ inactiveText WRITE setInactiveText)

public:
    enum WidgetPosition {
        LeftSide,
        RightSide
    };

    LineEdit(QWidget *parent = 0);
    LineEdit(const QString &contents, QWidget *parent = 0);

	LineEditButton *addButton(WidgetPosition position, const QString &tips, const QObject *receiver, const char *method);

	void addWidget(QWidget *widget, WidgetPosition position);
    void removeWidget(QWidget *widget);
    void setWidgetSpacing(int spacing);
    int widgetSpacing() const;
    int textMargin(WidgetPosition position) const;
    QString inactiveText() const;
    void setInactiveText(const QString &text);

    void paintEvent(QPaintEvent *event);

protected:
    void resizeEvent(QResizeEvent *event);
    bool event(QEvent *event);

protected slots:
    void updateTextMargins();

private:
    void init();
    void updateSideWidgetLocations();

	LineEditSideWidget *m_leftWidget;
	LineEditSideWidget *m_rightWidget;
    QHBoxLayout *m_leftLayout;
    QHBoxLayout *m_rightLayout;
    QString m_inactiveText;
};

#endif // YE_LINEEDIT_H

