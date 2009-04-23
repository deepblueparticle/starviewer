/***************************************************************************
 *   Copyright (C) 2005 by Grup de Gràfics de Girona                       *
 *   http://iiia.udg.es/GGG/index.html?langu=uk                            *
 *                                                                         *
 *   Universitat de Girona                                                 *
 ***************************************************************************/
#ifndef UDGQ2DVIEWEREXTENSION_H
#define UDGQ2DVIEWEREXTENSION_H

#include "ui_q2dviewerextensionbase.h"
#include <QWidget>

// FWD declarations
class QAction;

namespace udg {

// FWD declarations
class Volume;
class ToolManager;
class ToolConfiguration;
class HangingProtocol;
//Estructura pacient
class Patient;
//Visualitzador
class Q2DViewerWidget;
class Q2DViewer;
// Menus
class MenuGridWidget;
class TableMenu;
class QDicomDump;

/**
Extensió que s'executarà  per defecte a l'obrir un model

	@author Grup de Gràfics de Girona  ( GGG ) <vismed@ima.udg.es>
*/
class Q2DViewerExtension : public QWidget , private Ui::Q2DViewerExtensionBase {
Q_OBJECT
public:
    Q2DViewerExtension( QWidget *parent = 0 );

    ~Q2DViewerExtension();

    /// Li assigna el volum principal
    void setInput( Volume *input );

    /// Mètode per assignar un pacient
    void setPatient( Patient *patient );

    /// Mètode per obtenir el pacient
    Patient* getPatient() const;

public slots:
    /// Mostrar menu per seleccionar grid predefinit
    ///TODO S'ha de canviar el mètode per tal que no es crei l'objecte cada cop
    void showPredefinedGrid();

    /// Mostrar el menu de la taula per seleccionar grids
    ///TODO S'ha de canviar el mètode per tal que no es crei l'objecte cada cop
    void showInteractiveTable();

    /// Mostrar menu per seleccionar grid predefinit dins una serie
    ///TODO S'ha de canviar el mètode per tal que no es crei l'objecte cada cop
    void showPredefinedImageGrid();

    /// Mostrar el menu de la taula per seleccionar grids dins una serie
    ///TODO S'ha de canviar el mètode per tal que no es crei l'objecte cada cop
    void showInteractiveImageTable();

    /// Mostrar la informació del volum a cada visualitzador o no.
    void showViewerInformation( bool show );

    ///Mostrar la pantalla de Dicom dump, amb informació de la imatge que es visualitza en aquell moment
    void showDicomDumpCurrentDisplayedImage();

    //TODO mètode per solucionar problemes perquè la sincronització encara no està adaptada
    // a la resta de les tools
    /// Desactiva l'eina de sincronització.
    void disableSynchronization();

	// Apilicar un hanging protocol
	void setHangingProtocol( int hangingProtocolNumber );

private:
    /// Crea les connexions entre signals i slots
    void createConnections();

    /// Llegir/Escriure la configuració de l'aplicació
    void readSettings();
    void writeSettings();

    /**
     * Inicialitza les tools que tindrà l'extensió
     */
    void initializeTools();

private slots:
    /// Comprova si el nou volum té fases i per tant hem d'activar/descativar la vista coronal+sagital
    void validePhases();

    /**
     * Habilita o deshabilita el botó de Dump d'info DICOM
     * segons en la vista en la que ens trobem
     */
    void updateDICOMInformationButton( int view );

    /// Activa la sincronització al visualitzador
    void synchronization( Q2DViewerWidget *, bool );

    /// Mètode que tracta el nou visualitzador afegit
    void activateNewViewer( Q2DViewerWidget *newViewerWidget );

    /// Mètode que tracta el canvi de visualitzador seleccionat
    void changeSelectedViewer( Q2DViewerWidget *viewerWidget );

    /// Mètode que busca els hanging protocols aplicables
    void searchHangingProtocols();

private:
    /// El volum principal
    Volume *m_mainVolume;

    /// Accions
    QAction *m_singleShotAction;
    QAction *m_multipleShotAction;
    QAction *m_screenShotTriggerAction;
    QAction *m_sagitalViewAction;
    QAction *m_coronalViewAction;

    // llistes de tools
    QStringList m_availableToolsList;
    QStringList m_availableActionToolsList;

    /// El diàleg per escollir un window level ajustat per l'usuari
    QCustomWindowLevelDialog *m_customWindowLevelDialog;

    ///Obre la finestra de dicomdump per la imatge que tenim seleccionada en aquests moments
    QDicomDump *m_dicomDumpCurrentDisplayedImage;

    /// Pacient que estem tractant
    Patient *m_patient;

    /// Menús per seleccionar el grid de series( predefinits + taula )
    MenuGridWidget *m_predefinedSeriesGrid;
    TableMenu *m_seriesTableGrid;

    /// Menús per seleccionar el grid de les llesques d'una serie ( predefinits + taula )
    MenuGridWidget *m_predefinedSlicesGrid;
    TableMenu *m_sliceTableGrid;

    /// ToolManager per configurar l'entorn deToolConfiguration * configuration = getConfiguration();
    ToolManager *m_toolManager;

    ///Perfil a mostrar la interficie
    QString m_profile;

    /// ültim visualitzador seleccionat
    Q2DViewerWidget *m_lastSelectedViewer;

    /// Llista de hanging protocols aplicables
    QList<HangingProtocol * > m_hangingCandidates;

};

} // end namespace udg

#endif
