#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_messageBox_textChanged()
{
    QString text = ui->messageBox->toPlainText();
    printf("%s\n", text.toStdString().c_str());

    if (text.endsWith("\n"))
    {
        text.truncate(text.length()-1);
        if (!text.startsWith("\n") && text.length() > 0)
            ui->chatMessages->append(text);
        ui->messageBox->clear();
    }
}
