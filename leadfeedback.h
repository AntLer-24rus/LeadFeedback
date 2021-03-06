#ifndef LEADFEEDBACK_H
#define LEADFEEDBACK_H

#include <QMainWindow>
#include <QFileDialog>
#include <QDebug>
#include "WordProcessingCompiler.h"
#include "WordProcessingMerger.h"
#include <exception>
#include <QMessageBox>
#include <QListWidget>
#include <QRandomGenerator>

namespace Ui {
class LeadFeedback;
}
using namespace DocxFactory;

class LeadFeedback : public QMainWindow
{
    Q_OBJECT

public:
    explicit LeadFeedback(QWidget *parent = 0);
    ~LeadFeedback();
    string prepareString(int charPerLine, QString str);

private slots:
    void on_closeWindow_clicked();

    void on_openFile_clicked();

    void on_loadStudentsFromFile_clicked();

    void on_saveStedentsToFile_clicked();

    void on_cleanTableStudents_clicked();

    void on_autoSizeColStudents_clicked();

    void on_addStudent_clicked();

    void on_removeStudent_clicked();

    void on_generateFeedbacks_clicked();

    void on_addLimitatio_clicked();

    void on_limitations_itemDoubleClicked(QListWidgetItem *item);

    void on_removeLimitation_clicked();

    void on_loadLimitationsFromFile_clicked();

    void on_saveLimitationsToFile_clicked();

private:
    Ui::LeadFeedback *ui;
};

#endif // LEADFEEDBACK_H
