#include "widget.h"
#include "./ui_widget.h"

#include <QDesktopServices>
#include <QJsonDocument>
#include <QInputDialog>
#include <QJsonObject>
#include <QJsonArray>
#include <QListWidget>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QMessageBox>

#include "yaml-cpp/yaml.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget) {
    ui->setupUi(this);
    this->loadExams();
}

Widget::~Widget() {
    delete ui;
}

void Widget::setStatus(const QString &status) {
    this->ui->statusLabel->setText("当前状态：" + status);
}

void Widget::updateExamsList() {
    this->setStatus("更新试卷列表");
    this->ui->examsListWidget->clear();
    for (const auto& exam:this->exams) {
        this->ui->examsListWidget->addItem(exam.name);
    }
    this->ui->deleteProblemPushButton->setEnabled(false);
    this->ui->addProblemPushButton->setEnabled(false);
    this->ui->saveProblemPushButton->setEnabled(false);
    this->ui->addChoicePushButton->setEnabled(false);
    this->ui->deleteChoicePushButton->setEnabled(false);
    this->ui->exportExamPushButton->setEnabled(false);
    this->setStatus("就绪");
}

void Widget::updateProblemsList() {
    this->setStatus("更新题目列表");
    this->ui->problemsListWidget->clear();
    const auto& exam = this->getCurrentExam();
    for (const auto& problem:exam.problems) {
        this->ui->problemsListWidget->addItem(problem.statement.left(12) + "...");
    }
    this->ui->addProblemPushButton->setEnabled(true);
    this->ui->saveProblemPushButton->setEnabled(false);
    this->ui->addChoicePushButton->setEnabled(false);
    this->ui->deleteProblemPushButton->setEnabled(false);
    this->ui->deleteChoicePushButton->setEnabled(false);
    this->ui->exportExamPushButton->setEnabled(true);
    this->setStatus("就绪");
}

void Widget::updateProblemDetail() {
    this->setStatus("更新题目详情");
    const auto& problem = this->getCurrentProblem();
    this->ui->statementTextEdit->setPlainText(problem.statement);
    this->ui->choicesListWidget->clear();
    for (const auto& choice:problem.choices) {
        this->ui->choicesListWidget->addItem(choice);
    }
    this->ui->correctAnswerLineEdit->setText(problem.correctChoice);
    this->ui->scoreDoubleSpinBox->setValue(problem.score);
    this->ui->deleteProblemPushButton->setEnabled(true);
    this->ui->saveProblemPushButton->setEnabled(true);
    this->ui->addChoicePushButton->setEnabled(true);
    this->ui->deleteChoicePushButton->setEnabled(false);
    this->setStatus("就绪");
}

void Widget::on_addExamPushButton_clicked() {
    this->setStatus("添加试卷");
    // 弹出一个输入框，要求用户输入试卷名
    const QString name = QInputDialog::getText(this, "添加试卷", "请输入试卷名：");
    if (name.isEmpty()) return;
    this->exams.append(Exam(name, QList<Problem>()));
    this->setStatus("就绪");
    this->updateExamsList();
}

void Widget::loadExams() {
    this->setStatus("加载试卷列表");
    QDir examDir("./exams/");
    if (!examDir.exists()) examDir.mkdir(".");
    for (const auto& info:examDir.entryInfoList()) {
        if (!info.fileName().endsWith("json")) continue;
        QFile file(info.absoluteFilePath());
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) continue;
        auto jsonDoc = QJsonDocument::fromJson(file.readAll());
        this->exams.append(Exam(jsonDoc.object()));
        file.close();
    }
    this->setStatus("就绪");
    this->updateExamsList();
}

Problem::Problem() {
    this->statement = "请您在这里填写题目描述，不需要包含选项。";
    this->choices = QList<QString>();
    this->choices << "选项 A 描述" << "选项 B 描述" << "选项 C 描述" << "选项 D 描述";
    this->correctChoice = "A";
    this->score = 2.0;
}

Problem::Problem(const QJsonObject &obj) {
    this->statement = obj["statement"].toString();
    this->choices = QStringList();
    for (const auto& choice:obj["choices"].toArray()) {
        this->choices.append(choice.toString());
    }
    this->correctChoice = obj["correctChoice"].toString();
    this->score = obj["score"].toDouble();
}

void Problem::editProblem(const QString &_statement, const QList<QString> &_choices, const QString &_correctChoice, double _score) {
    this->statement = _statement;
    this->choices = _choices;
    this->correctChoice = _correctChoice;
    this->score = _score;
}

QJsonObject Problem::toJsonObject() const {
    QJsonObject obj;
    obj["statement"] = this->statement;
    QJsonArray _choices;
    for (const auto& choice:this->choices) {
        _choices.append(choice);
    }
    obj["choices"] = _choices;
    obj["correctChoice"] = this->correctChoice;
    obj["score"] = this->score;
    return obj;
}

Exam::Exam(const QJsonObject &obj) {
    this->name = obj["name"].toString();
    for (const auto& problem:obj["problems"].toArray()) {
        this->problems.append(Problem(problem.toObject()));
    }
}

Exam *Exam::addProblem(const Problem &problem) {
    this->problems.append(problem);
    this->save();
    return this;
}

Exam::Exam(const QString &_name, const QList<Problem> &_problems) {
    this->name = _name;
    this->problems = _problems;
    this->save();
}

void Exam::save() {
    QFile file("./exams/" + this->name + ".json");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return;
    QJsonObject obj;
    obj["name"] = this->name;
    QJsonArray array;
    for (const auto& problem:this->problems) {
        array.append(problem.toJsonObject());
    }
    obj["problems"] = array;
    QJsonDocument doc;
    doc.setObject(obj);
    file.write(doc.toJson());
    file.close();
}

Exam *Exam::removeProblem(int index) {
    this->problems.removeAt(index);
    return this;
}

Exam& Widget::getCurrentExam() {
    if (this->ui->examsListWidget->currentRow() == -1) throw std::runtime_error("no exam selected");
    return this->exams[this->ui->examsListWidget->currentRow()];
}

Problem& Widget::getCurrentProblem() {
    if (this->ui->problemsListWidget->currentRow() == -1) throw std::runtime_error("no problem selected");
    return this->getCurrentExam().problems[this->ui->problemsListWidget->currentRow()];
}

void Widget::on_addProblemPushButton_clicked() {
    this->setStatus("添加题目");
    this->getCurrentExam().addProblem(Problem())->save();
    this->setStatus("就绪");
    this->updateProblemsList();
}

void Widget::on_deleteProblemPushButton_clicked() {
    if (this->ui->problemsListWidget->currentRow() == -1) return;
    this->setStatus("删除题目");
    this->getCurrentExam().removeProblem(this->ui->problemsListWidget->currentRow())->save();
    this->setStatus("就绪");
    this->updateProblemsList();
}

void Widget::on_examsListWidget_itemClicked([[maybe_unused]] QListWidgetItem *item) {
    this->updateProblemsList();
}

void Widget::on_problemsListWidget_itemClicked([[maybe_unused]] QListWidgetItem *item) {
    this->updateProblemDetail();
}

void Widget::on_saveProblemPushButton_clicked() {
    this->setStatus("保存题目");
    QList<QString> choices;
    for (int i = 0; i < this->ui->choicesListWidget->count(); i++) {
        choices.append(this->ui->choicesListWidget->item(i)->text());
    }
    this->getCurrentProblem().editProblem(this->ui->statementTextEdit->toPlainText(), choices, this->ui->correctAnswerLineEdit->text(), this->ui->scoreDoubleSpinBox->value());
    this->getCurrentExam().save();
    this->setStatus("就绪");
    this->updateProblemDetail();
    this->updateProblemsList();
}

void Widget::on_deleteChoicePushButton_clicked() {
    if (this->ui->choicesListWidget->currentRow() == -1) return;
    this->setStatus("删除选项");
    this->ui->choicesListWidget->takeItem(this->ui->choicesListWidget->currentRow());
    this->setStatus("就绪");
}

void Widget::on_addChoicePushButton_clicked() {
    this->setStatus("添加选项");
    // 弹出输入框，要求用户输入选项
    QString choice = QInputDialog::getText(this, "添加选项", "请输入选项内容：");
    if (choice.isEmpty()) return;
    this->ui->choicesListWidget->addItem(choice);
    this->setStatus("就绪");
}

void Widget::on_choicesListWidget_itemClicked([[maybe_unused]] QListWidgetItem *item) {
    this->ui->deleteChoicePushButton->setEnabled(true);
}

void Widget::on_choicesListWidget_itemDoubleClicked(QListWidgetItem *item) {
    // 弹出输入框，要求用户输入新选项
    this->setStatus("修改选项");
    QString choice = QInputDialog::getText(this, "修改选项", "请输入选项内容：", QLineEdit::Normal, item->text());
    if (choice.isEmpty()) return;
    item->setText(choice);
    this->setStatus("就绪");
}

void Widget::on_exportExamPushButton_clicked() {
    const auto checkProblem = [](const Problem &problem) {
        if (problem.statement.isEmpty()) return false;
        if (problem.choices.empty()) return false;
        if (problem.correctChoice.size() > 1) return false;
        if (problem.correctChoice[0].toLatin1() - 'A' > problem.choices.size()) return false;
        return true;
    };
    const auto generateProblemYaml = [this]() {
        YAML::Node node;
        // 打开输入框，要求用户输入 PID
        QString pid = QInputDialog::getText(this, "导出试卷", "请输入在 Hydro 中的 PID：");
        if (pid.isEmpty()) return node;
        node["pid"] = pid.toStdString();
        node["owner"] = 1;
        node["title"] = this->getCurrentExam().name.toStdString();
        return node;
    };
    const auto generateMarkdown = [this]() {
        const auto generateOne = [](const Problem &problem, int idx) {
            QString choices = "";
            for (const auto &choice:problem.choices) {
                choices += QString("- %1\n").arg(choice);
            }
            return QString("%1\n\n{{ select(%2) }}\n\n%3\n")
                .arg(problem.statement)
                .arg(idx)
                .arg(choices);
        };
        QString statement;
        int idx = 0;
        for (const auto &problem:this->getCurrentExam().problems) {
            statement += generateOne(problem, ++idx);
        }
        return statement;
    };
    const auto generateConfigYaml = [this]() {
        YAML::Node node;
        node["type"] = "objective";
        YAML::Node answer;
        int idx = 0;
        for (const auto &problem:this->getCurrentExam().problems) {
            idx++;
            answer[QString("%1").arg(idx).toStdString()].push_back(problem.correctChoice.toStdString());
            answer[QString("%1").arg(idx).toStdString()].push_back(problem.score);
        }
        node["answers"] = answer;
        return node;
    };
    int idx = 0;
    for (const auto &problem:this->getCurrentExam().problems) {
        if (!checkProblem(problem)) {
            QMessageBox::warning(this, "导出失败", QString("题目 %1 不合法。").arg(idx));
            return;
        }
        idx++;
    }
    // 创建一个临时目录
    QString path = QString("./export/%1_%2/testdata")
            .arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"),
                 this->getCurrentExam().name);
    QDir tempDir(path);
    if (!tempDir.exists()) {
        if (!tempDir.mkpath(".")) {
            QMessageBox::warning(this, "导出失败", QString("无法创建临时目录 %1。").arg(tempDir.absolutePath()));
            return;
        }
    }
    tempDir.cdUp();
    QFile problemYaml(tempDir.path() + "/problem.yaml");
    if (!problemYaml.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "导出失败", "无法创建临时文件。");
        return;
    }
    problemYaml.write(Dump(generateProblemYaml()).c_str());
    problemYaml.close();
    QFile statementMd(tempDir.path() + "/problem_zh.md");
    if (!statementMd.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "导出失败", "无法创建临时文件。");
        return;
    }
    statementMd.write(generateMarkdown().toUtf8());
    statementMd.close();
    QFile configYaml(tempDir.path() + "/testdata/config.yaml");
    if (!configYaml.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "导出失败", "无法创建临时文件。");
        return;
    }
    configYaml.write(Dump(generateConfigYaml()).c_str());
    configYaml.close();
    tempDir.cdUp();
    QDesktopServices::openUrl(QUrl::fromLocalFile(tempDir.path()));
}

