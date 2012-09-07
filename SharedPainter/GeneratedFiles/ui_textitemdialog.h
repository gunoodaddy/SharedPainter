/********************************************************************************
** Form generated from reading UI file 'textitemdialog.ui'
**
** Created: Fri Sep 7 14:42:26 2012
**      by: Qt User Interface Compiler version 4.8.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TEXTITEMDIALOG_H
#define UI_TEXTITEMDIALOG_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QFontComboBox>
#include <QtGui/QGroupBox>
#include <QtGui/QHeaderView>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QSpinBox>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_TextItemDialog
{
public:
    QLineEdit *lineEdit;
    QPushButton *okButton;
    QPushButton *cancelButton;
    QGroupBox *groupBox;
    QPushButton *clrButton;
    QSpinBox *sizeSpinBox;
    QFontComboBox *fontComboBox;

    void setupUi(QWidget *TextItemDialog)
    {
        if (TextItemDialog->objectName().isEmpty())
            TextItemDialog->setObjectName(QString::fromUtf8("TextItemDialog"));
        TextItemDialog->resize(242, 143);
        lineEdit = new QLineEdit(TextItemDialog);
        lineEdit->setObjectName(QString::fromUtf8("lineEdit"));
        lineEdit->setGeometry(QRect(10, 10, 221, 20));
        okButton = new QPushButton(TextItemDialog);
        okButton->setObjectName(QString::fromUtf8("okButton"));
        okButton->setGeometry(QRect(45, 110, 75, 23));
        cancelButton = new QPushButton(TextItemDialog);
        cancelButton->setObjectName(QString::fromUtf8("cancelButton"));
        cancelButton->setGeometry(QRect(125, 110, 75, 23));
        groupBox = new QGroupBox(TextItemDialog);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        groupBox->setGeometry(QRect(10, 40, 221, 51));
        clrButton = new QPushButton(groupBox);
        clrButton->setObjectName(QString::fromUtf8("clrButton"));
        clrButton->setGeometry(QRect(120, 20, 31, 20));
        clrButton->setFlat(false);
        sizeSpinBox = new QSpinBox(groupBox);
        sizeSpinBox->setObjectName(QString::fromUtf8("sizeSpinBox"));
        sizeSpinBox->setGeometry(QRect(160, 20, 51, 22));
        sizeSpinBox->setMinimum(8);
        sizeSpinBox->setMaximum(100);
        fontComboBox = new QFontComboBox(groupBox);
        fontComboBox->setObjectName(QString::fromUtf8("fontComboBox"));
        fontComboBox->setGeometry(QRect(10, 20, 101, 22));

        retranslateUi(TextItemDialog);
        QObject::connect(cancelButton, SIGNAL(clicked()), TextItemDialog, SLOT(close()));
        QObject::connect(okButton, SIGNAL(clicked()), TextItemDialog, SLOT(clickedOkButton()));
        QObject::connect(clrButton, SIGNAL(clicked()), TextItemDialog, SLOT(clickedColorButton()));

        QMetaObject::connectSlotsByName(TextItemDialog);
    } // setupUi

    void retranslateUi(QWidget *TextItemDialog)
    {
        TextItemDialog->setWindowTitle(QApplication::translate("TextItemDialog", "Text Item", 0, QApplication::UnicodeUTF8));
        okButton->setText(QApplication::translate("TextItemDialog", "OK", 0, QApplication::UnicodeUTF8));
        cancelButton->setText(QApplication::translate("TextItemDialog", "Cancel", 0, QApplication::UnicodeUTF8));
        groupBox->setTitle(QApplication::translate("TextItemDialog", "Font", 0, QApplication::UnicodeUTF8));
        clrButton->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class TextItemDialog: public Ui_TextItemDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TEXTITEMDIALOG_H
