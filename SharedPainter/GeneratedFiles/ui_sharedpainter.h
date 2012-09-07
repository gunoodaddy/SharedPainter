/********************************************************************************
** Form generated from reading UI file 'sharedpainter.ui'
**
** Created: Fri Sep 7 14:42:27 2012
**      by: Qt User Interface Compiler version 4.8.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SHAREDPAINTER_H
#define UI_SHAREDPAINTER_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QGraphicsView>
#include <QtGui/QHeaderView>
#include <QtGui/QMainWindow>
#include <QtGui/QMenuBar>
#include <QtGui/QStatusBar>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_SharedPainterClass
{
public:
    QAction *actionConnect;
    QAction *actionExit;
    QAction *actionExitApp;
    QAction *actionConnectDirect;
    QAction *actionServer;
    QAction *actionUndo;
    QAction *actionScreen_Shot;
    QAction *actionClient;
    QAction *actionBroadCastServerType;
    QAction *actionBroadCastClientType;
    QWidget *centralWidget;
    QVBoxLayout *verticalLayout;
    QGraphicsView *painterView;
    QMenuBar *menuBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *SharedPainterClass)
    {
        if (SharedPainterClass->objectName().isEmpty())
            SharedPainterClass->setObjectName(QString::fromUtf8("SharedPainterClass"));
        SharedPainterClass->resize(679, 500);
        actionConnect = new QAction(SharedPainterClass);
        actionConnect->setObjectName(QString::fromUtf8("actionConnect"));
        actionExit = new QAction(SharedPainterClass);
        actionExit->setObjectName(QString::fromUtf8("actionExit"));
        actionExitApp = new QAction(SharedPainterClass);
        actionExitApp->setObjectName(QString::fromUtf8("actionExitApp"));
        actionConnectDirect = new QAction(SharedPainterClass);
        actionConnectDirect->setObjectName(QString::fromUtf8("actionConnectDirect"));
        actionServer = new QAction(SharedPainterClass);
        actionServer->setObjectName(QString::fromUtf8("actionServer"));
        actionUndo = new QAction(SharedPainterClass);
        actionUndo->setObjectName(QString::fromUtf8("actionUndo"));
        actionScreen_Shot = new QAction(SharedPainterClass);
        actionScreen_Shot->setObjectName(QString::fromUtf8("actionScreen_Shot"));
        actionClient = new QAction(SharedPainterClass);
        actionClient->setObjectName(QString::fromUtf8("actionClient"));
        actionBroadCastServerType = new QAction(SharedPainterClass);
        actionBroadCastServerType->setObjectName(QString::fromUtf8("actionBroadCastServerType"));
        actionBroadCastClientType = new QAction(SharedPainterClass);
        actionBroadCastClientType->setObjectName(QString::fromUtf8("actionBroadCastClientType"));
        centralWidget = new QWidget(SharedPainterClass);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        verticalLayout = new QVBoxLayout(centralWidget);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        painterView = new QGraphicsView(centralWidget);
        painterView->setObjectName(QString::fromUtf8("painterView"));

        verticalLayout->addWidget(painterView);

        SharedPainterClass->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(SharedPainterClass);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 679, 21));
        SharedPainterClass->setMenuBar(menuBar);
        statusBar = new QStatusBar(SharedPainterClass);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        SharedPainterClass->setStatusBar(statusBar);

        retranslateUi(SharedPainterClass);
        QObject::connect(actionExit, SIGNAL(triggered()), SharedPainterClass, SLOT(actionExit()));

        QMetaObject::connectSlotsByName(SharedPainterClass);
    } // setupUi

    void retranslateUi(QMainWindow *SharedPainterClass)
    {
        SharedPainterClass->setWindowTitle(QApplication::translate("SharedPainterClass", "SharedPainter", 0, QApplication::UnicodeUTF8));
        actionConnect->setText(QApplication::translate("SharedPainterClass", "Connect", 0, QApplication::UnicodeUTF8));
        actionExit->setText(QApplication::translate("SharedPainterClass", "Exit", 0, QApplication::UnicodeUTF8));
        actionExitApp->setText(QApplication::translate("SharedPainterClass", "Exit", 0, QApplication::UnicodeUTF8));
        actionConnectDirect->setText(QApplication::translate("SharedPainterClass", "Connect", 0, QApplication::UnicodeUTF8));
        actionServer->setText(QApplication::translate("SharedPainterClass", "Server", 0, QApplication::UnicodeUTF8));
        actionUndo->setText(QApplication::translate("SharedPainterClass", "Undo", 0, QApplication::UnicodeUTF8));
        actionScreen_Shot->setText(QApplication::translate("SharedPainterClass", "Screen Shot", 0, QApplication::UnicodeUTF8));
        actionClient->setText(QApplication::translate("SharedPainterClass", "Client", 0, QApplication::UnicodeUTF8));
        actionBroadCastServerType->setText(QApplication::translate("SharedPainterClass", "Server", 0, QApplication::UnicodeUTF8));
        actionBroadCastClientType->setText(QApplication::translate("SharedPainterClass", "Client", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class SharedPainterClass: public Ui_SharedPainterClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SHAREDPAINTER_H
