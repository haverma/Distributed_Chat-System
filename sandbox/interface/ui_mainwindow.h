/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 4.8.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHeaderView>
#include <QtGui/QListWidget>
#include <QtGui/QMainWindow>
#include <QtGui/QMenuBar>
#include <QtGui/QPlainTextEdit>
#include <QtGui/QStatusBar>
#include <QtGui/QTextBrowser>
#include <QtGui/QToolBar>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralWidget;
    QPlainTextEdit *messageBox;
    QTextBrowser *chatMessages;
    QListWidget *usersList;
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(766, 581);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        messageBox = new QPlainTextEdit(centralWidget);
        messageBox->setObjectName(QString::fromUtf8("messageBox"));
        messageBox->setGeometry(QRect(200, 470, 541, 51));
        chatMessages = new QTextBrowser(centralWidget);
        chatMessages->setObjectName(QString::fromUtf8("chatMessages"));
        chatMessages->setGeometry(QRect(200, 10, 541, 451));
        usersList = new QListWidget(centralWidget);
        new QListWidgetItem(usersList);
        new QListWidgetItem(usersList);
        new QListWidgetItem(usersList);
        usersList->setObjectName(QString::fromUtf8("usersList"));
        usersList->setGeometry(QRect(10, 10, 181, 511));
        MainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 766, 26));
        MainWindow->setMenuBar(menuBar);
        mainToolBar = new QToolBar(MainWindow);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        MainWindow->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        MainWindow->setStatusBar(statusBar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0, QApplication::UnicodeUTF8));

        const bool __sortingEnabled = usersList->isSortingEnabled();
        usersList->setSortingEnabled(false);
        QListWidgetItem *___qlistwidgetitem = usersList->item(0);
        ___qlistwidgetitem->setText(QApplication::translate("MainWindow", "user1", 0, QApplication::UnicodeUTF8));
        QListWidgetItem *___qlistwidgetitem1 = usersList->item(1);
        ___qlistwidgetitem1->setText(QApplication::translate("MainWindow", "user2", 0, QApplication::UnicodeUTF8));
        QListWidgetItem *___qlistwidgetitem2 = usersList->item(2);
        ___qlistwidgetitem2->setText(QApplication::translate("MainWindow", "user3", 0, QApplication::UnicodeUTF8));
        usersList->setSortingEnabled(__sortingEnabled);

    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
