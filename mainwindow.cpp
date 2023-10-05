#include "mainwindow.h"
#include "ui_mainwindow.h"



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    // Initialisation de l'interface graphique
    ui->setupUi(this);

    // Instanciation de la socket
    tcpSocket = new QTcpSocket(this);

    // Attachement d'un slot qui sera appelé à chaque fois que des données arrivent (mode asynchrone)
    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(gerer_donnees()));

    // Idem pour les erreurs
    connect(tcpSocket, SIGNAL(errorOccurred(QAbstractSocket::SocketError)), this, SLOT(afficher_erreur(QAbstractSocket::SocketError)));

    // Instanciation du timer
    pTimer = new QTimer();

    // Lancement du timer avec un tick toutes les 1000 ms
    pTimer->start(1000);

    // Instanciation carte
    pCarte = new QImage();
    pCarte->load(":/carte_la_rochelle_plan.png");
    ui->label_carte->setPixmap(QPixmap::fromImage(*pCarte)); // Carte Plan

    ui->pushButtonCarte->setIcon(QIcon(":/carte_la_rochelle_satellite.png")); // Icône du bouton

    pCarteTrs = new QImage();
    pCarteTrs->load(":/trs.png");
    ui->label_carte_trs->setPixmap(QPixmap::fromImage(*pCarteTrs));

    calcul_px = 0.0;
    calcul_py = 0.0;
    dernierpx = 0.0;
    dernierpy = 0.0;


    pVide = new QImage();
    pVide->load(":/photo_vide.png");


    // Association du "tick" du timer à l'appel d'une méthode SLOT faire_qqchose()
    connect(pTimer, SIGNAL(timeout()), this, SLOT(mettre_a_jour_ihm()));
}

MainWindow::~MainWindow()
{
    // Destruction de la socket
    tcpSocket->abort();
    delete tcpSocket;

    // Destruction de l'interface graphique
    delete ui;

    // Arrêt du timer
    pTimer->stop();

    // Destruction du timer
    delete pTimer;

    //Destruction Carte
    delete pCarte;

    // Destruction carte trs
    delete pCarteTrs;
}

void MainWindow::on_connexionButton_clicked()
{
    // Récupération des paramètres
    QString adresse_ip = ui->lineEdit_ip->text();
    unsigned short port_tcp = ui->lineEdit_port->text().toInt();

    // Connexion au serveur
    tcpSocket->connectToHost(adresse_ip, port_tcp);
}

void MainWindow::on_deconnexionButton_clicked()
{
    // Déconnexion du serveur
    tcpSocket->close();
}

void MainWindow::on_envoiButton_clicked()
{
    // Préparation de la requête
    QByteArray requete;
    requete = "RETR\r\n";

    // Envoi de la requête
    tcpSocket->write(requete);
}

void MainWindow::gerer_donnees()
{
    // Réception des données
    QByteArray reponse = tcpSocket->readAll();
    QString trame = QString(reponse);
    qDebug() << trame;

    // Affichage
    ui->lineEdit_reponse->setText(QString(reponse));

    // Decodage
    QStringList liste = trame.split(",");
    qDebug() <<liste[1];

    QString nb_satellite = liste[7];
    int satellite = nb_satellite.toInt();

    if (satellite >=3){


        //Date
        int heures = liste[1].mid(0, 2).toInt();
        int minutes = liste[1].mid(2, 2).toInt();
        int secondes = liste[1].mid(4, 2).toInt();
        int premier_releve = 28957;
        int timestamp = (heures * 3600) + (minutes * 60) + secondes;

        unsigned int heure_ecoule = (timestamp - premier_releve) / 3600;
        unsigned int min_ecoule = ((timestamp - premier_releve) % 3600) / 60;
        unsigned int sec_ecoule = ((timestamp - premier_releve) % 3600) % 60;
        QString heure_ecouleQString = QString("%1").arg(heure_ecoule);
        QString min_ecouleQString = QString("%1").arg(min_ecoule);
        QString sec_ecouleQString = QString("%1").arg(sec_ecoule);



        ui->lineEdit_temps->setText(heure_ecouleQString + " h " + min_ecouleQString + " min " + sec_ecouleQString + " s");



        QString postype = liste[6];
        QString nb_satellite = liste[7];
        QString precision_horizontale = liste[8];
        QString hauteur_geo = liste[11];
        QString tps_last_maj = liste[13];


        // Latitude

        double latitude = 0.0;
        double degres_lat = liste[2].mid(0,2).toDouble();
        double minutes_lat = liste[2].mid(2,7).toDouble();
        QString N_S = liste[3];

        if( N_S == "S"){
            latitude = (degres_lat + (minutes_lat / 60))*(-1);
        }else if(N_S == "N"){
            latitude = degres_lat + (minutes_lat / 60);

        }else{
            latitude =(degres_lat + (minutes_lat / 60));
        }
        QString latitudestring = QString("%1").arg(latitude);
        ui->lineEdit_latitude->setText(latitudestring);


        // Longitude
        double longitude = 0.0;
        double degres_long = liste[4].mid(0,3).toDouble();
        double minutes_long = liste[4].mid(3,7).toDouble();
        QString W_E = liste[5];

        if( W_E == "W"){
            longitude = (degres_long + (minutes_long / 60))*(-1);
        }else if(W_E == "E"){
            longitude = degres_long + (minutes_long / 60);

        }else{
            longitude =(degres_long + (minutes_long / 60));
        }
        QString longitude_string = QString("%1").arg(longitude);
        ui->lineEdit_longitude->setText(longitude_string);

        //Altitude
        double altitude = liste[9].toDouble();
        QString altitudeQString = QString("%1").arg(altitude);
        ui->lineEdit_altitude->setText(altitudeQString);

        // Frequence Cardiaque
        int frequence = liste[14].mid(1,3).toInt();
        QString frequenceQString = QString("%1").arg(frequence);
        ui->lineEdit_FC->setText(frequenceQString);
        qDebug() <<frequence;

        //Carte
        ui->label_carte->setPixmap(QPixmap::fromImage(*pCarte));

        //FCmax
        QString ageRentre = ui->lineEdit_Age->text();
        bool ok;
        int age = ageRentre.toInt(&ok);

        int FCmax= 207 - (0.7*age);
        QString FCmaxQString =QString("%1").arg(FCmax);
        ui->lineEdit_FCMax->setText(FCmaxQString);
        qDebug() <<FCmax;


        //progression bar
        int IntFreq = (frequence*1.0/FCmax*1.0)*100.0;
        ui->progressBar->setValue(IntFreq);
        qDebug() << IntFreq;
        qDebug() << Click;


        //Dessin sur carte trs
        // Position projetée
        const double lat_hg = 46.173311;
        const double long_hg = -1.195703;
        const double lat_bd = 46.135451;
        const double long_bd = -1.136125;
        const double largeur_carte = 694.0;
        const double hauteur_carte = 638.0;

        calcul_px = largeur_carte * ((longitude - long_hg) / (long_bd - long_hg));
        calcul_py = hauteur_carte * (1.0 - (latitude - lat_bd) / (lat_hg - lat_bd));

        QPainter p(pCarteTrs);
        if ((dernierpx != 0.0 && dernierpy != 0.0)) {
            p.setPen(QPen(Qt::red, 2));
            p.drawLine(dernierpx, dernierpy, calcul_px, calcul_py);
            p.end();
            ui->label_carte_trs->setPixmap(QPixmap::fromImage(*pCarteTrs));
        } else {
        }

        //distance

        latB=latA;
        lonB=lonA;

        latA=qDegreesToRadians(latitude);
        lonA=qDegreesToRadians(longitude);



        //distance
        if(latB != 0 && lonB != 0){
            AB = 6378 * acos((sin(latB)*sin(latA)) + (cos(latB)* cos(latA))* (cos(lonB - lonA)));
            ABtt = AB + ABr;
            QString distAB_string = QString("%1").arg(ABtt);
            ui->lineEdit_Distance->setText(distAB_string);
        }else{

        }


        //calorie
        QString poidsQString = ui->lineEdit_Poids->text();
        int poids = poidsQString.toInt();
        calories = AB * poids * 1.036;
        QString caloriesQString =QString("%1").arg(calories);
        ui->lineEdit_Calorie->setText(caloriesQString);

        //vitesse
        double temps_ecoule_sec = 1.0;
        double vitesse_kph = ABtt / temps_ecoule_sec;
        QString vitesseQString = QString("%1").arg(vitesse_kph);
        ui->lineEdit_Vitesse->setText(vitesseQString);


        // courbe fréquence
        QPainter painter(pVide);
        ui->label_Courbe->setPixmap(QPixmap::fromImage(*pVide));
        painter.setPen(QPen(Qt::transparent, 1));
        painter.drawLine(compteur, 200, compteur,200);
        painter.setPen(QPen(Qt::red, 1));
        painter.drawLine(compteur, 500, compteur,600 - frequence);
        compteur += 1;
        if (compteur >= ui->label_Courbe->width()) {
            pVide->fill(Qt::transparent);
            compteur = 0;
        }

        //courbe altitude

        double altitudeDouble = altitude;
        painter.setPen(QPen(Qt::black, 1));
        painter.drawLine(compteur, 600, compteur,550 - altitudeDouble);
        ui->label_Car->width();
        painter.end();

    }
    else{
        ui->lineEdit_Satellite->setText("nombre de satellite insufisant");
    }

}


void MainWindow::mettre_a_jour_ihm()
{
    qDebug() << "tic";

    // Préparation de la requête
    QByteArray requete;
    requete = "RETR\r\n";

    dernierpx = calcul_px;
    dernierpy = calcul_py;

    // Envoi de la requête
    tcpSocket->write(requete);

    ABr = AB;
}

void MainWindow::afficher_erreur(QAbstractSocket::SocketError socketError)
{
    switch (socketError)
    {
    case QAbstractSocket::RemoteHostClosedError:
        break;
    case QAbstractSocket::HostNotFoundError:
        QMessageBox::information(this, tr("Client TCP"),
                                 tr("Hôte introuvable"));
            break;
    case QAbstractSocket::ConnectionRefusedError:
        QMessageBox::information(this, tr("Client TCP"),
                                 tr("Connexion refusée"));
            break;
    default:
        QMessageBox::information(this, tr("Client TCP"),
                                 tr("Erreur : %1.")
                                     .arg(tcpSocket->errorString()));
    }
}



void MainWindow::on_pushButtonCarte_clicked()
{
    Click=!Click;
    QString carte;
    if (Click == true) {
        carte = ":/carte_la_rochelle_plan.png";
        ui->pushButtonCarte->setIcon(QIcon(":/carte_la_rochelle_satellite.png"));

    } else {
        carte = ":/carte_la_rochelle_satellite.png";
        ui->pushButtonCarte->setIcon(QIcon(":/carte_la_rochelle_plan.png"));

    }

    pCarte->load(carte);


}


