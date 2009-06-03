/***************************************************************************
 *   Copyright (C) 2005-2007 by Grup de Gràfics de Girona                  *
 *   http://iiia.udg.es/GGG/index.html?langu=uk                            *
 *                                                                         *
 *   Universitat de Girona                                                 *
 ***************************************************************************/
#include "angletool.h"
#include "q2dviewer.h"
#include "logging.h"
#include "volume.h"
#include "drawer.h"
#include "drawerpolyline.h"
#include "drawertext.h"
#include "mathtools.h"
//vtk
#include <vtkRenderWindowInteractor.h>
#include <vtkCommand.h>
#include <vtkProp.h>
#include <vtkLine.h>
#include <vtkPoints.h>

#include "mathtools.h"
#include <vtkMath.h>
//Qt
#include <QList>

namespace udg {

AngleTool::AngleTool( QViewer *viewer, QObject *parent )
 : Tool(viewer, parent)
{
    m_toolName = "AngleTool";
    m_hasSharedData = false;

    m_2DViewer = qobject_cast<Q2DViewer *>( viewer );
    if( !m_2DViewer )
        DEBUG_LOG(QString("El casting no ha funcionat!!! És possible que viewer no sigui un Q2DViewer!!!-> ")+ viewer->metaObject()->className() );

    m_circumferencePolyline= NULL;
    m_mainPolyline=NULL;
    m_state = NONE;
}

AngleTool::~AngleTool()
{
    if ( m_state != NONE )
    {
        if ( m_mainPolyline )
            delete m_mainPolyline;
        if (m_circumferencePolyline )
            delete m_circumferencePolyline;
    }
}

void AngleTool::handleEvent( long unsigned eventID )
{
    switch( eventID )
    {
        case vtkCommand::LeftButtonPressEvent:

            if( m_2DViewer->getInput() )
            {
                if ( m_2DViewer->getInteractor()->GetRepeatCount() == 0 )
                {
                    if ( m_state == NONE )
                        this->annotateFirstPoint();
                    else if ( m_state == FIRST_POINT_FIXED )
                    {
                        this->fixFirstSegment();
                        this->findInitialDegreeArc();
                    }
                    else
                    {
                        //voldrem enregistrar l'últim punt, pertant posem l'estat a none
                        m_state = NONE;
                        computeAngle();
                        delete m_circumferencePolyline;
                    }
                    m_2DViewer->getDrawer()->refresh();
                }
            }
        break;

        case vtkCommand::MouseMoveEvent:

            if( m_mainPolyline && m_state == FIRST_POINT_FIXED  )
                this->simulateFirstSegmentOfAngle();
            else if ( m_mainPolyline && m_state == CENTER_FIXED )
                this->simulateSecondSegmentOfAngle();

        break;
    }
}

void AngleTool::findInitialDegreeArc()
{
    //Per saber quin l'angle inicial, cal calcular l'angle que forma el primer segment anotat i un segment fictici totalment horitzontal.
    double horizontalP2[3], *vd1, *vd2, *pv;
    double *p1 = m_mainPolyline->getPoint( 0 );
    double *p2 = m_mainPolyline->getPoint( 1 );

    int coord1, depthCoord;

    switch( m_2DViewer->getView() )
    {
        case QViewer::AxialPlane:
            coord1 = 0;
            depthCoord = 2;
            break;

        case QViewer::SagitalPlane:
            coord1 = 1;
            depthCoord = 0;
            break;

        case QViewer::CoronalPlane:
            coord1 = 2;
            depthCoord = 1;
            break;
    }

    for (int i = 0; i < 3; i++)
        horizontalP2[i] = p2[i];

    vd1 = MathTools::directorVector( p1, p2 );

    horizontalP2[coord1] += 10.0;
    vd2 = MathTools::directorVector( horizontalP2, p2 );
    pv = MathTools::crossProduct(vd1, vd2);

    if ( pv[depthCoord] > 0 )
    {
        m_initialDegreeArc =(int)MathTools::angleInDegrees( vd1, vd2 );
    }
    else
    {
        m_initialDegreeArc = -1 * (int)MathTools::angleInDegrees( vd1, vd2 );
    }
}

void AngleTool::annotateFirstPoint()
{
    m_mainPolyline = new DrawerPolyline;
    m_2DViewer->getDrawer()->draw( m_mainPolyline , m_2DViewer->getView(), m_2DViewer->getCurrentSlice() );

    double clickedWorldPoint[3];
    m_2DViewer->getEventWorldCoordinate( clickedWorldPoint );

    //afegim el punt
    m_mainPolyline->addPoint( clickedWorldPoint );
    m_mainPolyline->update( DrawerPrimitive::VTKRepresentation );

    //actualitzem l'estat de la tool
    m_state = FIRST_POINT_FIXED;
}

void AngleTool::fixFirstSegment()
{
    m_mainPolyline->update( DrawerPrimitive::VTKRepresentation );

    //posem l'estat de la tool a CENTER_FIXED, així haurà agafat l'últim valor.
    m_state = CENTER_FIXED;

    //creem la polilínia per a dibuixar l'arc de circumferència i l'afegim al drawer
    m_circumferencePolyline = new DrawerPolyline;
    m_2DViewer->getDrawer()->draw( m_circumferencePolyline , m_2DViewer->getView(), m_2DViewer->getCurrentSlice() );
}

void AngleTool::drawCircumference()
{
    double degreesIncrease, *newPoint, radius;
    int initialAngle, finalAngle, depthCoord;

    double *firstPoint = m_mainPolyline->getPoint(0);
    double *circleCentre = m_mainPolyline->getPoint(1);
    double *lastPoint = m_mainPolyline->getPoint(2);

    // calculem l'angle que formen els dos segments
    double *firstSegment = MathTools::directorVector( firstPoint, circleCentre );
    double *secondSegment = MathTools::directorVector( lastPoint, circleCentre );
    double angle = MathTools::angleInDegrees( firstSegment, secondSegment );
    
    // calculem el radi de l'arc de circumferència que mesurarà
    // un quart del segment més curt dels dos que formen l'angle
    double distance1 = MathTools::getDistance3D( firstPoint, circleCentre );
    double distance2 = MathTools::getDistance3D( circleCentre, lastPoint );
    radius = MathTools::minimum( distance1, distance2 ) / 4.0;

    int view = m_2DViewer->getView();
    switch( view )
    {
        case QViewer::AxialPlane:
            depthCoord = 2;
            break;

        case QViewer::SagitalPlane:
            depthCoord = 0;
            break;

        case QViewer::CoronalPlane:
            depthCoord = 1;
            break;
    }

    // calculem el rang de les iteracions per pintar l'angle correctament
    initialAngle = 360 - m_initialDegreeArc;
    finalAngle = int(360 - ( angle+m_initialDegreeArc ) );
        
    double *pv = MathTools::crossProduct(firstSegment, secondSegment);
    if ( pv[depthCoord] > 0 )
    {
        finalAngle = int(angle-m_initialDegreeArc);
    }
    if ( (initialAngle-finalAngle) > 180 )
    {
        initialAngle = int( angle-m_initialDegreeArc );
        finalAngle = -m_initialDegreeArc;
    }

    for ( int i = initialAngle; i > finalAngle; i-- )
    {
        degreesIncrease = i*1.0*vtkMath::DoubleDegreesToRadians();
        newPoint = new double[3];

        switch( view )
        {
            case QViewer::AxialPlane:
                newPoint[0] = cos( degreesIncrease )*radius + circleCentre[0];
                newPoint[1] = sin( degreesIncrease )*radius + circleCentre[1];
                newPoint[2] = 0.0;
                break;

            case QViewer::SagitalPlane:
                newPoint[0] = 0.0;
                newPoint[1] = cos( degreesIncrease )*radius + circleCentre[1];
                newPoint[2] = sin( degreesIncrease )*radius + circleCentre[2];
                break;

            case QViewer::CoronalPlane:
                newPoint[0] = sin( degreesIncrease )*radius + circleCentre[0];
                newPoint[1] = 0.0;
                newPoint[2] = cos( degreesIncrease )*radius + circleCentre[2];
                break;
        }
        m_circumferencePolyline->addPoint( newPoint );
    }

    m_circumferencePolyline->update( DrawerPrimitive::VTKRepresentation );
}

void AngleTool::simulateFirstSegmentOfAngle()
{
    double clickedWorldPoint[3];
    m_2DViewer->getEventWorldCoordinate( clickedWorldPoint );

    if ( m_mainPolyline->getNumberOfPoints() == 2 ) //és que ja havíem assignat el segon punt que determina el primer segment de l'angle
    {
        //esborrem aquest segon punt
        m_mainPolyline->removePoint( 1 );
    }

    //afegim el nou segon punt que simula el nou primer angle
    m_mainPolyline->addPoint( clickedWorldPoint );
    m_mainPolyline->update( DrawerPrimitive::VTKRepresentation );
    m_2DViewer->getDrawer()->refresh();
}

void AngleTool::simulateSecondSegmentOfAngle()
{
    double clickedWorldPoint[3];
    m_2DViewer->getEventWorldCoordinate( clickedWorldPoint );

    if ( m_mainPolyline->getNumberOfPoints() == 3 ) //és que ja havíem assignat el segon punt que determina el primer segment de l'angle
    {
        //esborrem aquest segon punt
        m_mainPolyline->removePoint( 2 );
    }

    //afegim el nou segon punt que simula el nou punt
    m_mainPolyline->addPoint( clickedWorldPoint );
    m_mainPolyline->update( DrawerPrimitive::VTKRepresentation );

    m_circumferencePolyline->deleteAllPoints();
    drawCircumference();
    m_2DViewer->getDrawer()->refresh();
}

void AngleTool::computeAngle()
{
    double *p1 = m_mainPolyline->getPoint(0);
    double *p2 = m_mainPolyline->getPoint(1);
    double *p3 = m_mainPolyline->getPoint(2);

    double *vd1 = MathTools::directorVector( p1, p2 );
    double *vd2 = MathTools::directorVector( p3, p2 );

    for (int i = 0; i < 3; i++)
    {
        if ( fabs( vd1[i] ) < 0.0001 )
            vd1[i] = 0.0;

        if ( fabs( vd2[i] ) < 0.0001 )
            vd2[i] = 0.0;
    }

    double angle = MathTools::angleInDegrees( vd1, vd2 );

    DrawerText * text = new DrawerText;
    text->setText( tr("%1 degrees").arg( angle,0,'f',1) );
    textPosition( p1, p2, p3, text );

    text->update( DrawerPrimitive::VTKRepresentation );
    m_2DViewer->getDrawer()->draw( text , m_2DViewer->getView(), m_2DViewer->getCurrentSlice() );
    m_2DViewer->getDrawer()->refresh();
}

void AngleTool::textPosition( double *p1, double *p2, double *p3, DrawerText *angleText )
{
    double position[3];
    int i, horizontalCoord, verticalCoord;

    switch( m_2DViewer->getView() )
    {
        case Q2DViewer::Axial:
            horizontalCoord = 0;
            verticalCoord = 1;
            break;

        case Q2DViewer::Sagital:
            horizontalCoord = 1;
            verticalCoord = 2;
            break;

        case Q2DViewer::Coronal:
            horizontalCoord = 0;
            verticalCoord = 2;
            break;
    }

        //mirem on estan horitzontalment els punts p1 i p3 respecte del p2
    if ( p1[0] <= p2[0] )
    {
        angleText->setHorizontalJustification( "Left" );

        if ( p3[horizontalCoord] <= p2[horizontalCoord] )
        {
            angleText->setAttatchmentPoint( p2 );
        }
        else
        {
            for ( i = 0; i < 3; i++ )
                position[i] = p2[i];

            if ( p2[verticalCoord] <= p3[verticalCoord] )
            {
                position[verticalCoord] -= 2.;
            }
            else
            {
                position[verticalCoord] += 2.;
            }
            angleText->setAttatchmentPoint( position );
        }
    }
    else
    {
        angleText->setHorizontalJustification( "Right" );

        if ( p3[horizontalCoord] <= p2[horizontalCoord] )
        {
            angleText->setAttatchmentPoint( p2 );
        }
        else
        {
            for ( i = 0; i < 3; i++ )
                position[i] = p2[i];

            if ( p2[verticalCoord] <= p3[verticalCoord] )
            {
                position[verticalCoord] += 2.;
            }
            else
            {
                position[verticalCoord] -= 2.;
            }
            angleText->setAttatchmentPoint( position );
        }
    }
}
}
