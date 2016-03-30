
#include "test.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QWidget *widget = new QWidget;
    Ui_Form ui;
    ui.setupUi(widget);

    widget->show();
    return app.exec();
}


