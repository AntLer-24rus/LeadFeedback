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
                                                    tr("Document files (*.docx);;All files (*.*)"));
    ui->pathToFile->setText(fileName);
}

void LeadFeedback::on_loadStudentsFromFile_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    "Открыть файл со студентами",
                                                    QDir::currentPath(),
                                                    "Student list (*.slst)");
    qDebug() << "Select file" << fileName;
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
    if (fileName.isEmpty())
    {
        qDebug() << "Empty path to file";
        return;
    } else if (!QFileInfo(fileName).exists()){
        qDebug() << "file not exists";
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
        l_merger.setClipboardValue("Отзыв","Институт","Нефти и газа");

        l_merger.setClipboardValue("Отзыв","Кафедра",prepareString(70, "Топливообеспечения и горюче-смазочных материалов"));


        l_merger.setClipboardValue("Отзыв","Дисциплина",prepareString(70-14, "Информатика Федеральное государственное автономное образовательное учреждение высшего образования"));
        l_merger.setClipboardValue("Отзыв","Тема",prepareString(70-5, "Расчет личных финансовых трат"));
        l_merger.setClipboardValue("Отзыв","ФИО",prepareString(70-9, "Сатышев Антон Сергеевич"));
        l_merger.setClipboardValue("Отзыв","группа","НГ10-25Б");
        l_merger.setClipboardValue("Отзыв","курс",(double) 2);
        l_merger.setClipboardValue("Отзыв","семестр",(double) 4);
        l_merger.setClipboardValue("Отзыв","направление",prepareString(70-23, "23.03.03 Эксплуатация транспортно-технологических машин и комплексов"));
        l_merger.setClipboardValue("Отзыв","профиль",prepareString(70-8, "23.03.03.06 Сервис транспортных и транспортно-технологических машин и оборудования (нефтепродуктообеспечение и газоснабжение)"));
        l_merger.setClipboardValue("Отзыв","ФИО_Руководителя",prepareString(70-29, "Ганжа Владимир Александрович, кандидат технических наук"));
        l_merger.setClipboardValue("Отзыв","характеристика",prepareString(70, "Ганжа Владимир Александрович, кандидат технических наук"));
        l_merger.setClipboardValue("Отзыв","замечания",prepareString(70, "Ганжа Владимир Александрович, кандидат технических наук"));
        l_merger.setClipboardValue("Отзыв","качество",prepareString(70, "Ганжа Владимир Александрович, кандидат технических наук"));
        l_merger.setClipboardValue("Отзыв","оценка","4 хорошо");
        l_merger.setClipboardValue("Отзыв","ИО_Фамилия_Р","В.А. Ганжа");


        l_merger.setClipboardValue("Отзыв","дата", (double) QDateTime::currentDateTime().toTime_t());

        l_merger.paste("Отзыв");
        qDebug() << "Created (in"
            << (double) (clock() - l_start) / CLOCKS_PER_SEC
            << "seconds).";

        QString fileName = QFileDialog::getSaveFileName(this,
                                                        tr("Сохранить отзыв/рецензию"),
                                                        QDir::currentPath(),
                                                        tr("Document files (*.docx);;All files (*.*)"));
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
