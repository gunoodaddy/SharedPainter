#include "stdafx.h"
#include "TextItemDialog.h"

static QColor LAST_COLOR = QColor(85, 170, 255);
static int LAST_SIZE_VALUE = 30;
static QFont LAST_FONT( "±¼¸²" );

TextItemDialog::TextItemDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	ui.fontComboBox->setCurrentFont( LAST_FONT );
	ui.sizeSpinBox->setValue( LAST_SIZE_VALUE );
	
	changeColor( LAST_COLOR );
}

TextItemDialog::~TextItemDialog()
{

}

void TextItemDialog::clickedColorButton( void )
{
	clr_ = QColorDialog::getColor( clr_, this, tr("Pen Color"));

	changeColor( clr_ );

}

void TextItemDialog::clickedOkButton( void )
{
	text_ = ui.lineEdit->text();
	if( text_.isEmpty() )
	{
		QMessageBox::warning(this, "", tr("Please Input text."));
		return;
	}

	int size = ui.sizeSpinBox->value();

	font_ = ui.fontComboBox->currentFont();
	font_.setPixelSize( size );
	font_.setBold( true );

	LAST_SIZE_VALUE = size;
	LAST_COLOR = clr_;
	LAST_FONT = ui.fontComboBox->currentFont();

	accept();
}

void TextItemDialog::changeColor( const QColor &color )
{
	clr_ = color;

	QString s = "background-color: ";
	ui.clrButton->setStyleSheet(s + color.name());
}