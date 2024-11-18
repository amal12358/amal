#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QTextStream>
#include <QRegularExpressionValidator>
#include <QRegularExpression>
#include <QHeaderView>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {

    ui->setupUi(this);
    setupDatabase();


    model = new QSqlTableModel(this, db);
    model->setTable("MACHINES");
    model->select();


    ui->this_2->setModel(model);


    model->setHeaderData(0, Qt::Horizontal, "ID");
    model->setHeaderData(1, Qt::Horizontal, "Type");
    model->setHeaderData(2, Qt::Horizontal, "Localisation");
    model->setHeaderData(5, Qt::Horizontal, "Etat");
    model->setHeaderData(3, Qt::Horizontal, "Date Achat");
    model->setHeaderData(4, Qt::Horizontal, "Dernier Entretien");


    ui->this_2->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);


    setupInputValidation();


    updateStatistics();
}

MainWindow::~MainWindow() {
    db.close();
    delete ui;
}


void MainWindow::on_label_linkActivated(const QString &link) {

    qDebug() << "Link activated: " << link;
}



void MainWindow::setupDatabase() {
    db = QSqlDatabase::addDatabase("QODBC");
    db.setHostName("localhost");
    db.setDatabaseName("xe");
    db.setUserName("AMAL");
    db.setPassword("0000");

    if (!db.open()) {
        showError("Database connection failed: " + db.lastError().text());
        return;
    }
    qDebug() << "Connected to the database successfully!";
}


void MainWindow::showError(const QString &msg) {
    QMessageBox::critical(this, "Error", msg);
}


void MainWindow::refreshTable() {
    model->select();
}


bool MainWindow::validateInput() {
    if (ui->idInput->text().isEmpty()) {
        showError("ID cannot be empty.");
        return false;
    }
    if (ui->typeInput->text().isEmpty()) {
        showError("Type cannot be empty.");
        return false;
    }
    if (ui->localisationInput->text().isEmpty()) {
        showError("Localisation cannot be empty.");
        return false;
    }
    if (ui->dateAchatInput->text().isEmpty()) {
        showError("Date of purchase cannot be empty.");
        return false;
    }
    if (ui->dernierEntretienInput->text().isEmpty()) {
        showError("Last maintenance date cannot be empty.");
        return false;
    }
    if (ui->etatInput->currentText().isEmpty()) {
        showError("Machine state cannot be empty.");
        return false;
    }
    return true;
}


void MainWindow::logAction(const QString &action) {
    qDebug() << "Action performed:" << action;
}

// Execute the SQL query with bindings
void MainWindow::executeQuery(const QString &queryStr, const QMap<QString, QVariant> &bindings) {
    QSqlQuery query;
    query.prepare(queryStr);

    for (auto it = bindings.cbegin(); it != bindings.cend(); ++it) {
        query.bindValue(it.key(), it.value());
    }

    if (!query.exec()) {
        showError("Database operation failed: " + query.lastError().text());
    }
}


void MainWindow::setupInputValidation() {
    ui->idInput->setValidator(new QIntValidator(0, 999999, this));
    QRegularExpressionValidator *nameValidator = new QRegularExpressionValidator(QRegularExpression("[A-Za-z ]+"), this);
    ui->typeInput->setValidator(nameValidator);
}


void MainWindow::on_createButton_clicked() {
    if (!validateInput()) {
        return;
    }

    QMap<QString, QVariant> bindings;
    bindings[":id"] = ui->idInput->text();
    bindings[":type"] = ui->typeInput->text();
    bindings[":localisation"] = ui->localisationInput->text();
    bindings[":DATE_ACHAT"] = ui->dateAchatInput->text();
    bindings[":dernierEntretien"] = ui->dernierEntretienInput->text();
    bindings[":ETAT"] = ui->etatInput->currentText();

    executeQuery("INSERT INTO MACHINES (id, type, localisation, DATE_ACHAT, DERNIER_ENTRETIEN, ETAT) VALUES (:id, :type, :localisation, :DATE_ACHAT, :dernierEntretien, :ETAT)", bindings);
    logAction("Created new record with ID: " + ui->idInput->text());
    refreshTable();
    updateStatistics();
}


void MainWindow::on_readButton_clicked() {
    refreshTable();
}


void MainWindow::on_updateButton_clicked() {
    QModelIndexList selectedRows = ui->this_2->selectionModel()->selectedRows();
    if (selectedRows.isEmpty()) {
        showError("Please select a row to update.");
        return;
    }

    if (!validateInput()) {
        return;
    }

    int row = selectedRows.first().row();

    QMap<QString, QVariant> bindings;
    bindings[":type"] = ui->typeInput->text();
    bindings[":localisation"] = ui->localisationInput->text();
    bindings[":ETAT"] = ui->etatInput->currentText();
    bindings[":id"] = model->data(model->index(row, 0)).toInt();

    executeQuery("UPDATE MACHINES SET type = :type, localisation = :localisation, ETAT = :ETAT WHERE id = :id", bindings);
    logAction("Updated record with ID: " + QString::number(bindings[":id"].toInt()));
    refreshTable();
    updateStatistics();
}



void MainWindow::on_deleteButton_clicked() {

    QModelIndexList selectedRows = ui->this_2->selectionModel()->selectedRows();


    if (selectedRows.isEmpty()) {
        showError("Please select a row to delete.");
        return;
    }


    int row = selectedRows.first().row();
    int id = model->data(model->index(row, 0)).toInt();


    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Confirm Deletion",
                                  "Are you sure you want to delete this record?",
                                  QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
                                  QMessageBox::No);


    if (reply == QMessageBox::Yes) {

        QString type = model->data(model->index(row, 1)).toString();
        QString localisation = model->data(model->index(row, 2)).toString();
        QString ETAT = model->data(model->index(row, 5)).toString();
        logAction("Deleting record - ID: " + QString::number(id) + ", Type: " + type + ", Localisation: " + localisation + ", ETAT: " + ETAT);


        QMap<QString, QVariant> bindings;
        bindings[":id"] = id;
        executeQuery("DELETE FROM MACHINES WHERE id = :id", bindings);


        refreshTable();
    } else if (reply == QMessageBox::Cancel) {

        return;
    }
}


void MainWindow::on_exportButton_clicked() {
    QString fileName = QFileDialog::getSaveFileName(this, "Export Data", "", "CSV Files (*.csv);;All Files (*)");
    if (fileName.isEmpty()) {
        return;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        showError("Could not open file for writing: " + file.errorString());
        return;
    }

    QTextStream out(&file);
    for (int i = 0; i < model->columnCount(); ++i) {
        out << model->headerData(i, Qt::Horizontal).toString() << ",";
    }
    out << "\n";

    for (int row = 0; row < model->rowCount(); ++row) {
        for (int col = 0; col < model->columnCount(); ++col) {
            out << model->data(model->index(row, col)).toString() << ",";
        }
        out << "\n";
    }

    file.close();
    QMessageBox::information(this, "Success", "Data exported successfully!");
}


void MainWindow::on_filterButton_clicked() {
    QString filter = "";
    QString id = ui->idInput->text();
    QString type = ui->typeInput->text();
    QString localisation = ui->localisationInput->text();
    QString ETAT = ui->etatInput->currentText();

    if (!id.isEmpty()) {
        filter += QString("id = '%1'").arg(id);
    }
    if (!type.isEmpty()) {
        if (!filter.isEmpty()) filter += " AND ";
        filter += QString("type LIKE '%%1%'").arg(type);
    }
    if (!localisation.isEmpty()) {
        if (!filter.isEmpty()) filter += " AND ";
        filter += QString("localisation LIKE '%%1%'").arg(localisation);
    }
    if (!ETAT.isEmpty()) {
        if (!filter.isEmpty()) filter += " AND ";
        filter += QString("ETAT LIKE '%%1%'").arg(ETAT);
    }

    model->setFilter(filter);
    model->select();
}


void MainWindow::on_sortButton_clicked() {
    bool ascending = ui->ascendingRadioButton->isChecked();
    int column = ui->columnComboBox->currentIndex();
    model->setSort(column, ascending ? Qt::AscendingOrder : Qt::DescendingOrder);
    model->select();
}


void MainWindow::updateStatistics() {
    QSqlQuery query;
    query.exec("SELECT ETAT, COUNT(*) FROM MACHINES GROUP BY ETAT");

    int functionalCount = 0;
    int brokenCount = 0;
    int repairingCount = 0;

    while (query.next()) {
        QString state = query.value(0).toString();
        int count = query.value(1).toInt();

        if (state == "Fonctionelle") {
            functionalCount = count;
        } else if (state == "En Panne") {
            brokenCount = count;
        } else if (state == "En Reparation") {
            repairingCount = count;
        }
    }


    ui->fonctionnelCountLabel->setText(QString::number(functionalCount));
    ui->enPanneCountLabel->setText(QString::number(brokenCount));
    ui->enReparationCountLabel->setText(QString::number(repairingCount));
}
