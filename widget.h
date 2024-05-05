#ifndef WIDGET_H
#define WIDGET_H

#include <QListWidgetItem>
#include <QWidget>

class Problem {
public:
    QString statement;
    QList<QString> choices;
    QString correctChoice;
    int score;

    Problem();
    explicit Problem(const QJsonObject &obj);
    void editProblem(const QString &_statement, const QList<QString> &_choices, const QString &_correctChoice, int _score);
    [[nodiscard]] QJsonObject toJsonObject() const;
};

class Exam {
public:
    QList<Problem> problems;
    QString name;

    explicit Exam(const QJsonObject &obj);
    Exam(const QString &_name, const QList<Problem> &_problems);

    Exam * addProblem(const Problem &problem);
    void save();
    Exam *removeProblem(int index);
};

QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void on_addExamPushButton_clicked();

    void on_addProblemPushButton_clicked();

    void on_deleteProblemPushButton_clicked();

    void on_examsListWidget_itemClicked(QListWidgetItem *item);

    void on_problemsListWidget_itemClicked(QListWidgetItem *item);

    void on_saveProblemPushButton_clicked();

    void on_deleteChoicePushButton_clicked();

    void on_addChoicePushButton_clicked();

    void on_choicesListWidget_itemClicked([[maybe_unused]] QListWidgetItem *item);

    void on_choicesListWidget_itemDoubleClicked(QListWidgetItem *item);

    void on_exportExamPushButton_clicked();

private:
    Ui::Widget *ui;
    QList<Exam> exams;

    void loadExams();
    void setStatus(const QString &status);
    void updateExamsList();

    Exam &getCurrentExam();

    void updateProblemsList();

    void updateProblemDetail();

    Problem &getCurrentProblem();
};
#endif // WIDGET_H
