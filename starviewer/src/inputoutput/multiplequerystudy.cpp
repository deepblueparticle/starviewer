/***************************************************************************
 *   Copyright (C) 2005 by Grup de Gràfics de Girona                       *
 *   http://iiia.udg.es/GGG/index.html?langu=uk                            *
 *                                                                         *
 *   Universitat de Girona                                                 *
 ***************************************************************************/

#include "multiplequerystudy.h"

#include <QSemaphore>
#include <QString>
#include <QMessageBox>
#include <QApplication>

#include "status.h"
#include "starviewersettings.h"
#include "pacsparameters.h"
#include "qquerystudythread.h"

#include "errordcmtk.h"

namespace udg {

MultipleQueryStudy::MultipleQueryStudy( QObject *parent )
 : QObject( parent )
{
    StarviewerSettings settings;

    m_semaphoreActiveThreads = new QSemaphore( settings.getMaxConnections().toInt( NULL , 10 ) );
}

void MultipleQueryStudy::setMask( DicomMask mask )
{
    m_searchMask = mask;
}

void MultipleQueryStudy::setPacsList( PacsList list )
{
     m_pacsList = list;
}

void MultipleQueryStudy::threadFinished()
{
    m_semaphoreActiveThreads->release();
}

void MultipleQueryStudy::slotErrorConnectingPacs( int pacsID )
{
    emit ( errorConnectingPacs ( pacsID ) );
}

void MultipleQueryStudy::slotErrorQueringStudiesPacs( int pacsID )
{
    emit ( errorQueringStudiesPacs ( pacsID ) );
}


Status MultipleQueryStudy::StartQueries()
{
    QList<QQueryStudyThread *> llistaThreads;
    bool error = false;
    Status state;
    PacsParameters pacsParameters;
    QString missatgeLog;

    initializeResultsList();

    m_pacsList.firstPacs();

    while ( !m_pacsList.end() ) //Anem creant threads per cercar
    {
        QQueryStudyThread *thread = new QQueryStudyThread;

        //aquest signal ha de ser QDirectConnection, pq sera el propi thread qui executara l'slot d'alliberar un recurs del semafor, si fos queued, hauria de ser el pare qui respongues al signal, pero com estaria fent el sem_wait no respondria mai! i tindríem deadlock
        
        connect( thread , SIGNAL( finished() ) , this , SLOT( threadFinished() ) , Qt::DirectConnection );
        connect( thread , SIGNAL( errorConnectingPacs( int ) ) , this , SLOT ( slotErrorConnectingPacs( int  ) ) );
        connect( thread , SIGNAL( errorQueringStudiesPacs( int ) ) , this , SLOT ( slotErrorQueringStudiesPacs( int  ) ) );
        m_semaphoreActiveThreads->acquire();//Demanem recurs, hi ha un maxim de threads limitat
        pacsParameters = m_pacsList.getPacs();

        thread->queryStudy( m_pacsList.getPacs() , m_searchMask );

        llistaThreads.append( thread );
        m_pacsList.nextPacs();
    }

    foreach ( QQueryStudyThread *thread , llistaThreads )
    {
        thread->wait();
        //fusionem les resultats dels diferents threads
        m_studyList += thread->getStudyList();
        m_seriesList += thread->getSeriesList();
        m_imageList += thread->getImageList();

        delete thread;
    }

    //si no hi ha error retornem l'status ok
    if ( !error )
    {
        state.setStatus( DcmtkNoError );
    }

    return state;
}

void MultipleQueryStudy::initializeResultsList()
{
    m_studyList.clear();
    m_seriesList.clear();
    m_imageList.clear();
}

QList<DICOMStudy> MultipleQueryStudy::getStudyList()
{
    return m_studyList;
}

QList<DICOMSeries> MultipleQueryStudy::getSeriesList()
{
    return m_seriesList;
}

QList<DICOMImage> MultipleQueryStudy::getImageList()
{
    return m_imageList;
}

MultipleQueryStudy::~MultipleQueryStudy()
{
}

}
