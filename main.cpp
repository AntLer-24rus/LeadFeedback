#include "leadfeedback.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    LeadFeedback w;
    w.show();

    return a.exec();
}
