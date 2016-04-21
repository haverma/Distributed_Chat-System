#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void updateText(QString str);
    void addUser(QString str);
    void clearUsers();
    void deleteUserEntry(const char* str);
private slots:
    void on_messageBox_textChanged();

private:
    Ui::MainWindow *ui;
};


#endif // MAINWINDOW_H
