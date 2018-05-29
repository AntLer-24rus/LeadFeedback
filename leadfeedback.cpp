#include "leadfeedback.h"
#include "ui_leadfeedback.h"
#include <QDateTime>
#include <QDesktopServices>
#include <QTextCodec>

LeadFeedback::LeadFeedback(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::LeadFeedback)
{
    ui->setupUi(this);
}

LeadFeedback::~LeadFeedback()
{
    delete ui;
}

string LeadFeedback::prepareString(int charPerLine, QString str)
{
    for (int i = 1; i <= str.length() / charPerLine; i++) {
        int ind = str.lastIndexOf(" ",-(str.length() - charPerLine * i));
        str.replace(ind,1, "\t\n\t");
    }
    return str.toStdString();
}

void LeadFeedback::on_closeWindow_clicked()
{
    QApplication::quit();
}

void LeadFeedback::on_openFile_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Открыть шаблон отзыва/рецензии"),
                                                    QDir::currentPath(),
                                                    tr("Document files (*.docx);;All files (*.*)"),
                                                    nullptr,
                                                    QFileDialog::DontUseNativeDialog);
    ui->pathToFile->setText(fileName);
}

void LeadFeedback::on_loadStudentsFromFile_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    "Открыть файл со студентами",
                                                    QDir::currentPath(),
                                                    "Student list (*.slst)"/*,
                                                    NULL,
                                                    QFileDialog::DontUseNativeDialog*/);
    QFile fileStudents(fileName);
    if(fileStudents.open(QIODevice::ReadWrite | QIODevice::Text)) {
        int i =0;
        ui->tableStudents->clear();
        ui->tableStudents->setRowCount(0);
        //        ui->tableStudents->setColumnCount(4);
        ui->tableStudents->setHorizontalHeaderLabels(QStringList() << "Ф.И.О" << "Тема" << "Дата" << "Оценка");
        while (!fileStudents.atEnd()) {
            QString line = fileStudents.readLine();
            line = line.trimmed();
            if (line.left(2) == "//") {
                continue;
            }
            QStringList lst = line.split(";");
            // Строка в файле содержит:
            //              0 - ФИО
            //              1 - Тема
            //              2 - Дата сдачи
            //              3 - Оценка
            ui->tableStudents->insertRow(i);

            for (int j = 0; j < lst.count(); j++) {
                if(ui->tableStudents->columnCount() <= j){
                    ui->tableStudents->insertColumn(j);

                }
                ui->tableStudents->setItem(i,j,new QTableWidgetItem(lst.at(j)));
                switch (j) {
                case 0:
                case 1:
                    ui->tableStudents->item(i,j)->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
                    break;
                default:
                    ui->tableStudents->item(i,j)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
                    break;
                }
            }
            i++;
        }
        // Ресайзим колонки по содержимому
        ui->tableStudents->resizeColumnsToContents();
    }

    fileStudents.close();
}

void LeadFeedback::on_saveStedentsToFile_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(
                this,
                "Выберте файл для сохранения",
                QDir::currentPath(),
                "Student list (*.slst)");
    QFile fileStudents(fileName);
    if(fileStudents.open(QIODevice::WriteOnly | QIODevice::Text)) {
        for (int i = 0; i < ui->tableStudents->rowCount(); i++) {
            QString line;
            for (int j = 0; j < ui->tableStudents->columnCount(); j++) {
                line = line.append(ui->tableStudents->item(i,j)->text().trimmed());
                if (j != ui->tableStudents->columnCount() - 1) {
                    line = line.append(";");
                }
            }
            fileStudents.write(line.append("\n").toUtf8());
            qDebug() << line;
        }
    }
    fileStudents.close();
}

void LeadFeedback::on_cleanTableStudents_clicked()
{
    ui->tableStudents->clear();
    ui->tableStudents->setRowCount(0);
    ui->tableStudents->setHorizontalHeaderLabels(QStringList() << "Ф.И.О" << "Тема" << "Дата" << "Оценка");
}

void LeadFeedback::on_autoSizeColStudents_clicked()
{
    ui->tableStudents->resizeColumnsToContents();
}

void LeadFeedback::on_addStudent_clicked()
{
    ui->tableStudents->insertRow(ui->tableStudents->rowCount());
}

void LeadFeedback::on_removeStudent_clicked()
{
    int numRow = ui->tableStudents->currentRow();
    if (numRow < 0)
    {
            numRow = ui->tableStudents->rowCount() - 1;
    }
    ui->tableStudents->removeRow(numRow);
}

void LeadFeedback::on_generateFeedbacks_clicked()
{
    QString fileName = ui->pathToFile->text();
    QString warningString = "";
    if (fileName.isEmpty())
    {
        if (!warningString.isEmpty())
            warningString.append("\n");
        warningString.append("\tНе выбран файл шаблона");

    }
    if (!QFileInfo(fileName).exists()){
        if (!warningString.isEmpty())
            warningString.append("\n");
        warningString.append("\tФайл шаблона не существует");
    }
    if (ui->tableStudents->rowCount() < 1) {
        if (!warningString.isEmpty())
            warningString.append("\n");
        warningString.append("\tНет ниодного студента");
    }
    if (ui->limitations->count() < 1) {
        if (!warningString.isEmpty())
            warningString.append("\n");
        warningString.append("\tНет ниодного замечания");
    }

    if (!warningString.isEmpty()) {
        QMessageBox::warning(this, tr("LeadFeedback"),
                             "Ошибки заполения:\n"
                             + warningString
                             + "\nУстраните ошибки и попробуйте заново...");
        return;
    }

    try
    {
        WordProcessingCompiler& l_compiler = WordProcessingCompiler::getInstance();
        time_t l_start = clock();
        l_compiler.compile(fileName.toLocal8Bit().constData(), "current_template.dfw");

        qDebug() << "Compiled (in"
            << (double) (clock() - l_start) / CLOCKS_PER_SEC
            << "seconds).";

        WordProcessingMerger& l_merger = WordProcessingMerger::getInstance();
        l_start = clock();

        l_merger.load("current_template.dfw");

        for (int i = 0; i < ui->tableStudents->rowCount(); i++) {
            l_merger.setClipboardValue("Отзыв","Институт",ui->institute->text().toStdString());
            l_merger.setClipboardValue("Отзыв","Кафедра",prepareString(70, ui->chair->text()));
            l_merger.setClipboardValue("Отзыв","Дисциплина",prepareString(70-14, ui->subject->text()));
            l_merger.setClipboardValue("Отзыв","группа",ui->group->text().toStdString());
            l_merger.setClipboardValue("Отзыв","курс", ui->course->currentText().toDouble());
            l_merger.setClipboardValue("Отзыв","семестр", ui->term->currentText().toDouble());
            l_merger.setClipboardValue("Отзыв","направление",prepareString(70-23, ui->direction->document()->toPlainText()));
            l_merger.setClipboardValue("Отзыв","профиль",prepareString(70-8, ui->profile->document()->toPlainText()));

            QString leaderFulNameAndTitle = ui->leadSurname->text() + " " +
                                            ui->leadName->text() + " " +
                                            ui->leadMiddleName->text() + ", " +
                                            ui->leadTitle->text();

            l_merger.setClipboardValue("Отзыв","ФИО_Руководителя",prepareString(70-29, leaderFulNameAndTitle));
            l_merger.setClipboardValue("Отзыв","характеристика",prepareString(70, ui->characteristic->toPlainText()));



            QString fullName = ui->tableStudents->item(i,0)->text();
            QString topic = ui->tableStudents->item(i,1)->text();
            QDateTime date = QDateTime::fromString(ui->tableStudents->item(i,2)->text(),"dd.MM.yyyy").addDays(1);
            QString score;
            switch (ui->tableStudents->item(i,3)->text().toInt()) {
            case 3:
                score = "3, удовлетворительно";
                break;
            case 4:
                score = "4, хорошо";
                break;
            case 5:
                score = "5, отлично";
                break;
            default:
                score = "";
                break;
            }


            l_merger.setClipboardValue("Отзыв","Тема",prepareString(70-5, topic));
            l_merger.setClipboardValue("Отзыв","ФИО",prepareString(70-9, fullName));
            l_merger.setClipboardValue("Отзыв","оценка", score.toStdString());
            l_merger.setClipboardValue("Отзыв","дата", (double) date.toTime_t());

            int x = QRandomGenerator::global()->bounded((double) ui->limitations->count());
            QString limitation = ui->limitations->item(x)->text();
            QString quality = ui->quality->toPlainText();

            l_merger.setClipboardValue("Отзыв","замечания",prepareString(70, limitation));
            l_merger.setClipboardValue("Отзыв","качество",prepareString(70, quality));

            QString leader_name_formated = ui->leadName->text().left(1) + ". " +
                                           ui->leadMiddleName->text().left(1) + ". " +
                                           ui->leadSurname->text();

            l_merger.setClipboardValue("Отзыв","ИО_Фамилия_Р",leader_name_formated.toStdString());

            l_merger.paste("Отзыв");
        }






        qDebug() << "Created (in"
            << (double) (clock() - l_start) / CLOCKS_PER_SEC
            << "seconds).";

        QString fileName = QFileDialog::getSaveFileName(this,
                                                        tr("Сохранить отзыв/рецензию"),
                                                        QDir::currentPath(),
                                                        tr("Document files (*.docx);;All files (*.*)"),
                                                        nullptr,
                                                        QFileDialog::DontUseNativeDialog);
        if (fileName.isEmpty()) {
            return;
        }
        l_merger.save(fileName.toLocal8Bit().constData());


        QMessageBox::StandardButton reply;
          reply = QMessageBox::question(this, tr("LeadFeedback"), tr("Открыть полученный документ?"),
                                        QMessageBox::Yes|QMessageBox::No);
          if (reply == QMessageBox::Yes) {
            QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));
          }

    }
    catch (const exception& p_exception)
    {
        QMessageBox::warning(this, tr("LeadFeedback"),
                             "Критическая ошибка\n"
                              + tr(p_exception.what()));
    }
}

void LeadFeedback::on_addLimitatio_clicked()
{
    QListWidgetItem *item = new QListWidgetItem("Введите текст замечания или недостатков", ui->limitations);
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    ui->limitations->editItem(item);
}


void LeadFeedback::on_limitations_itemDoubleClicked(QListWidgetItem *item)
{
    ui->limitations->editItem(item);
}

void LeadFeedback::on_removeLimitation_clicked()
{
    QListWidgetItem *item = ui->limitations->currentItem();
    if (item != nullptr) {
        delete item;
    }
}

void LeadFeedback::on_loadLimitationsFromFile_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    "Открыть файл со студентами",
                                                    QDir::currentPath(),
                                                    "Limitation list (*.llst)"/*,
                                                    NULL,
                                                    QFileDialog::DontUseNativeDialog*/);
    QFile fileLimitation(fileName);
    if(fileLimitation.open(QIODevice::ReadWrite | QIODevice::Text)) {
        while (!fileLimitation.atEnd()) {
            QString line = fileLimitation.readLine();
            line = line.trimmed();
            QListWidgetItem *item = new QListWidgetItem(line, ui->limitations);
            item->setFlags(item->flags() | Qt::ItemIsEditable);
        }
        // Ресайзим колонки по содержимому
        ui->tableStudents->resizeColumnsToContents();
    }

    fileLimitation.close();
}

void LeadFeedback::on_saveLimitationsToFile_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(
                this,
                "Выберте файл для сохранения",
                QDir::currentPath(),
                "Limitation list (*.llst)");
    QFile fileLimitation(fileName);
    if(fileLimitation.open(QIODevice::WriteOnly | QIODevice::Text)) {
        foreach (QListWidgetItem *item, ui->limitations->findItems("*", Qt::MatchWildcard)) {
            fileLimitation.write(item->text().append("\n").toUtf8());
        }
    }
    fileLimitation.close();
}

