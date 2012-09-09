#pragma once

#include <QDialog>
#include "ui_TextItemDialog.h"

class TextItemDialog : public QDialog
{
	Q_OBJECT

public:
	TextItemDialog(QWidget *parent = 0);
	~TextItemDialog();

public:
	const QColor &textColor( void) { return clr_; }
	const QString &text( void ) { return text_; }
	const QFont &font( void ) { return font_; }

private:
	void changeColor( const QColor &color );

protected slots:
	void clickedColorButton( void );
	void clickedOkButton( void );

private:
	Ui::TextItemDialog ui;
	QColor clr_;
	QString text_;
	QFont font_;
};
