#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QMap>
#include <QVariant>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_createButton_clicked();
    void on_readButton_clicked();
    void on_updateButton_clicked();
    void on_deleteButton_clicked();
    void on_exportButton_clicked();
    void on_filterButton_clicked();
    void on_sortButton_clicked();
    void on_label_linkActivated(const QString &link);

private:
    Ui::MainWindow *ui;
    QSqlDatabase db;
    QSqlTableModel *model;

    void setupDatabase();
    void showError(const QString &msg);
    void refreshTable();
    bool validateInput();
    void logAction(const QString &action);
    void executeQuery(const QString &queryStr, const QMap<QString, QVariant> &bindings);
    void setupInputValidation();
    void updateStatistics();
};

#endif
