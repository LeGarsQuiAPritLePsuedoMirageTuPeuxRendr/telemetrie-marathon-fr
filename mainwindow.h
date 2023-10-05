#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtNetwork>
#include <QMessageBox>
#include <QTimer>
#include <QDebug>
#include <QImage>
#include <QLabel>
#include <QPixmap>
#include <QPainter>
#include <bitset>
#include <QLayout>
#include <QSqlDatabase>



namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_connexionButton_clicked();

    void on_deconnexionButton_clicked();

    void on_envoiButton_clicked();

    void gerer_donnees();

    void afficher_erreur(QAbstractSocket::SocketError);

    void mettre_a_jour_ihm();

    void on_lineEdit_reponse_cursorPositionChanged(int arg1, int arg2);

    void on_pushButtonCarte_clicked();

private:
    Ui::MainWindow *ui;
    QTcpSocket *tcpSocket;
    QTimer *pTimer;
    QImage *pCarte;
    QImage *pCarteTrs;
    QImage *pVide;


    bool Click = false;
    double calcul_px;
    double calcul_py;
    double dernierpx;
    double dernierpy;
    QSqlDatabase bdd;
    float AB=0,ABr,ABtt;
    float latA,latB,lonA,lonB;
    float calories;
    float vitesse;
    double compteur;
};

#endif // MAINWINDOW_H
