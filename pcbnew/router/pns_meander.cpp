/*
 * KiRouter - a push-and-(sometimes-)shove PCB router
 *
 * Copyright (C) 2013-2014 CERN
 * Copyright (C) 2016-2022 KiCad Developers, see AUTHORS.txt for contributors.
 * Author: Tomasz Wlostowski <tomasz.wlostowski@cern.ch>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <base_units.h> // God forgive me doing this...

#include "pns_node.h"
#include "pns_itemset.h"
#include "pns_meander.h"
#include "pns_meander_placer_base.h"
#include "pns_router.h"
#include "pns_debug_decorator.h"

namespace PNS {

const MEANDER_SETTINGS& MEANDER_SHAPE::Settings() const
{
    return m_placer->MeanderSettings();
}


const MEANDER_SETTINGS& MEANDERED_LINE::Settings() const
{
    return m_placer->MeanderSettings();
}


void MEANDERED_LINE::MeanderSegment( const SEG& aBase, bool aSide, int aBaseIndex )
{
    double base_len = aBase.Length();

    SHAPE_LINE_CHAIN lc;

    bool     singleSided = Settings().m_singleSided;
    bool     side = aSide;
    VECTOR2D dir( aBase.B - aBase.A );

    if( !m_dual )
        AddCorner( aBase.A );

    bool turning = false;
    bool started = false;

    m_last = aBase.A;

    do
    {
        MEANDER_SHAPE m( m_placer, m_width, m_dual );

        m.SetBaselineOffset( m_baselineOffset );
        m.SetBaseIndex( aBaseIndex );

        double thr = (double) m.spacing();

        bool fail = false;
        double remaining = base_len - ( m_last - aBase.A ).EuclideanNorm();

        auto addSingleIfFits = [&]()
        {
            fail = true;

            if( m.Fit( MT_SINGLE, aBase, m_last, side ) )
            {
                AddMeander( new MEANDER_SHAPE( m ) );
                fail = false;
                started = false;
            }

            if( fail && !singleSided )
            {
                if( m.Fit( MT_SINGLE, aBase, m_last, !side ) )
                {
                    AddMeander( new MEANDER_SHAPE( m ) );
                    fail = false;
                    started = false;
                    side = !side;
                }
            }
        };

        if( remaining < Settings( ).m_step )
            break;

        if( !singleSided && remaining > 3.0 * thr )
        {
            if( !turning )
            {
                for( int i = 0; i < 2; i++ )
                {
                    bool checkSide = ( i == 0 ) ? side : !side;

                    if( m.Fit( MT_CHECK_START, aBase, m_last, checkSide ) )
                    {
                        turning = true;
                        AddMeander( new MEANDER_SHAPE( m ) );
                        side = !checkSide;
                        started = true;
                        break;
                    }
                }

                if( !turning )
                    addSingleIfFits();
            }
            else
            {
                bool rv = m.Fit( MT_CHECK_FINISH, aBase, m_last, side );

                if( rv )
                {
                    m.Fit( MT_TURN, aBase, m_last, side );
                    AddMeander( new MEANDER_SHAPE( m ) );
                    side = !side;
                    started = true;
                }
                else
                {
                    m.Fit( MT_FINISH, aBase, m_last, side );
                    started = false;
                    AddMeander( new MEANDER_SHAPE( m ) );
                    turning = false;
                }
            }
        }
        else if( !singleSided && started )
        {
            bool rv = m.Fit( MT_FINISH, aBase, m_last, side );

            if( rv )
                AddMeander( new MEANDER_SHAPE( m ) );

            break;

        }
        else if( !turning && remaining > thr * 2.0 )
        {
            addSingleIfFits();
        }
        else
        {
           fail = true;
        }

        remaining = base_len - ( m_last - aBase.A ).EuclideanNorm( );

        if( remaining < Settings( ).m_step )
            break;

        if( fail )
        {
            MEANDER_SHAPE tmp( m_placer, m_width, m_dual );
            tmp.SetBaselineOffset( m_baselineOffset );
            tmp.SetBaseIndex( aBaseIndex );

            int nextP = tmp.spacing() - 2 * tmp.cornerRadius() + Settings().m_step;
            VECTOR2I pn = m_last + dir.Resize( nextP );

            if( aBase.Contains( pn ) && !m_dual )
            {
                AddCorner( pn );
            } else
                break;
        }


    } while( true );

    if( !m_dual )
        AddCorner( aBase.B );
}


int MEANDER_SHAPE::MinAmplitude() const
{
    int minAmplitude = Settings().m_minAmplitude;

    // DP meanders don't really support smaller amplitudes
    minAmplitude = std::max( minAmplitude, std::abs( m_baselineOffset ) * 2 );

    // The path length won't be correct with very small arcs
    if( m_placer->MeanderSettings().m_cornerStyle == MEANDER_STYLE_ROUND )
        minAmplitude = std::max( minAmplitude, m_width + std::abs( m_baselineOffset ) * 2 );

    return minAmplitude;
}


int MEANDER_SHAPE::cornerRadius() const
{
    // TODO: fix diff-pair meandering so we can use non-100% radii
    int rPercent = m_dual ? 100 : Settings().m_cornerRadiusPercentage;

    int optCr = (int64_t) spacing() * rPercent / 200;
    int minCr = std::abs( m_baselineOffset );
    int maxCr = std::min( m_amplitude / 2, spacing() / 2 );

    int cr = std::clamp( optCr, minCr, maxCr );

    return cr;
}


int MEANDER_SHAPE::spacing( ) const
{
    if( !m_dual )
    {
        return std::max( m_width + m_placer->Clearance(), Settings().m_spacing );
    }
    else
    {
        int sp = m_width + m_placer->Clearance() + ( 2 * std::abs( m_baselineOffset ) );
        return std::max( sp, Settings().m_spacing );
    }
}


SHAPE_LINE_CHAIN MEANDER_SHAPE::makeMiterShape( const VECTOR2D& aP, const VECTOR2D& aDir,
                                                bool aSide )
{
    SHAPE_LINE_CHAIN lc;

    if( aDir.EuclideanNorm( ) == 0.0f )
    {
        lc.Append( aP );
        return lc;
    }

    VECTOR2D dir_u( aDir );
    VECTOR2D dir_v( aDir.Perpendicular( ) );
    VECTOR2D p = aP;
    lc.Append( ( int ) p.x, ( int ) p.y );

    // fixme: refactor
    switch( m_placer->MeanderSettings().m_cornerStyle )
    {
    case MEANDER_STYLE_ROUND:
    {
        VECTOR2D center = aP + dir_v * ( aSide ? -1.0 : 1.0 );

        lc.Append( SHAPE_ARC( center, aP, ( aSide ? -ANGLE_90 : ANGLE_90 ) ) );
    }
        break;

    case MEANDER_STYLE_CHAMFER:
    {
        double radius = (double) aDir.EuclideanNorm();
        double correction = 0;

        if( m_dual && radius > m_meanCornerRadius )
            correction = (double)( -2 * abs(m_baselineOffset) ) * tan( 22.5 * M_PI / 180.0 );

        VECTOR2D dir_cu = dir_u.Resize( correction );
        VECTOR2D dir_cv = dir_v.Resize( correction );

        p = aP - dir_cu;
        lc.Append( ( int ) p.x, ( int ) p.y );
        p = aP + dir_u + (dir_v + dir_cv) * ( aSide ? -1.0 : 1.0 );
        lc.Append( ( int ) p.x, ( int ) p.y );
    }
        break;
    }

    p = aP + dir_u + dir_v * ( aSide ? -1.0 : 1.0 );
    lc.Append( ( int ) p.x, ( int ) p.y );

    return lc;
}


void MEANDER_SHAPE::start( SHAPE_LINE_CHAIN* aTarget, const VECTOR2D& aWhere, const VECTOR2D& aDir )
{
    m_currentTarget = aTarget;
    m_currentTarget->Clear();
    m_currentTarget->Append( aWhere );
    m_currentDir = aDir;
    m_currentPos = aWhere;
}


void MEANDER_SHAPE::forward( int aLength )
{
    m_currentPos += m_currentDir.Resize( aLength );
    m_currentTarget->Append( m_currentPos );
}


void MEANDER_SHAPE::turn( const EDA_ANGLE& aAngle )
{
    RotatePoint( m_currentDir, aAngle );
}


void MEANDER_SHAPE::miter( int aRadius, bool aSide )
{
    if( aRadius <= 0 )
    {
        turn( aSide ? ANGLE_90 : -ANGLE_90 );
        return;
    }

    VECTOR2D dir = m_currentDir.Resize( (double) aRadius );
    SHAPE_LINE_CHAIN lc = makeMiterShape( m_currentPos, dir, aSide );

    m_currentPos = lc.CPoint( -1 );
    turn( aSide ? ANGLE_90 : -ANGLE_90 );

    m_currentTarget->Append( lc );
}


void MEANDER_SHAPE::uShape( int aSides, int aCorner, int aTop )
{
    forward( aSides );
    miter( aCorner, true );
    forward( aTop );
    miter( aCorner, true );
    forward( aSides );
}


SHAPE_LINE_CHAIN MEANDER_SHAPE::genMeanderShape( const VECTOR2D& aP, const VECTOR2D& aDir,
                                                 bool aSide, MEANDER_TYPE aType,
                                                 int aBaselineOffset )
{
    int cr = cornerRadius();
    int offset = aBaselineOffset;
    int spc = spacing();
    int amplitude = m_amplitude;
    int targetBaseLen = m_targetBaseLen;

    if( aSide )
        offset *= -1;

    VECTOR2D dir_u_b( aDir.Resize( offset ) );
    VECTOR2D dir_v_b( dir_u_b.Perpendicular() );

    if( 2 * cr > amplitude )
    {
        cr = amplitude / 2;
    }

    if( 2 * cr > spc )
    {
        cr = spc / 2;
    }

    if( cr - offset < 0 )
    {
        cr = offset;
    }

    m_meanCornerRadius = cr;

    int sCorner = cr - offset;
    int uCorner = cr + offset;
    int startSide = amplitude - 2 * cr + std::abs( offset );
    int turnSide = amplitude - cr;
    int top = spc - 2 * cr;

    SHAPE_LINE_CHAIN lc;

    start( &lc, aP + dir_v_b, aDir );

    switch( aType )
    {
    case MT_EMPTY:
    {
        lc.Append( aP + dir_v_b + aDir );
        break;
    }
    case MT_START:
    {
        if( targetBaseLen )
            top = std::max( top, targetBaseLen - sCorner - uCorner * 2 + offset );

        miter( sCorner, false );
        uShape( startSide, uCorner, top );
        forward( std::min( sCorner, uCorner ) );
        forward( std::abs( offset ) );
        break;
    }

    case MT_FINISH:
    {
        if( targetBaseLen )
            top = std::max( top, targetBaseLen - cr - spc );

        start( &lc, aP - dir_u_b, aDir );
        turn( -ANGLE_90 );
        forward( std::min( sCorner, uCorner ) );
        forward( std::abs( offset ) );
        uShape( startSide, uCorner, top );
        miter( sCorner, false );

        if( targetBaseLen >= spc + cr )
            lc.Append( aP + dir_v_b + aDir.Resize( targetBaseLen ) );
        else
            lc.Append( aP + dir_v_b + aDir.Resize( 2 * spc - cr ) );

        break;
    }

    case MT_TURN:
    {
        if( targetBaseLen )
            top = std::max( top, targetBaseLen - uCorner * 2 + offset * 2 );

        start( &lc, aP - dir_u_b, aDir );
        turn( -ANGLE_90 );
        forward( std::abs( offset ) );
        uShape( turnSide, uCorner, top );
        forward( std::abs( offset ) );
        break;
    }

    case MT_SINGLE:
    {
        if( targetBaseLen )
            top = std::max( top, ( targetBaseLen - sCorner * 2 - uCorner * 2 ) / 2 );

        miter( sCorner, false );
        uShape( startSide, uCorner, top );
        miter( sCorner, false );
        lc.Append( aP + dir_v_b + aDir.Resize( 2 * spc ) );
        break;
    }

    default: break;
    }

    if( aSide )
    {
        SEG axis( aP, aP + aDir );

        lc.Mirror( axis );
    }

    return lc;
}


bool MEANDERED_LINE::CheckSelfIntersections( MEANDER_SHAPE* aShape, int aClearance )
{
    for( int i = m_meanders.size() - 1; i >= 0; i-- )
    {
        MEANDER_SHAPE* m = m_meanders[i];

        if( m->Type() == MT_EMPTY || m->Type() == MT_CORNER )
            continue;

        const SEG& b1 = aShape->BaseSegment();
        const SEG& b2 = m->BaseSegment();

        if( b1.ApproxParallel( b2 ) )
            continue;

        int n = m->CLine( 0 ).SegmentCount();

        for( int j = n - 1; j >= 0; j-- )
        {
            if( aShape->CLine( 0 ).Collide( m->CLine( 0 ) .CSegment( j ), aClearance ) )
                return false;
        }
    }

    return true;
}


bool MEANDER_SHAPE::Fit( MEANDER_TYPE aType, const SEG& aSeg, const VECTOR2I& aP, bool aSide )
{
    const MEANDER_SETTINGS& st = Settings();

    bool checkMode = false;
    MEANDER_TYPE prim1, prim2;

    if( aType == MT_CHECK_START )
    {
        prim1 = MT_START;
        prim2 = MT_TURN;
        checkMode = true;
    }
    else if( aType == MT_CHECK_FINISH )
    {
        prim1 = MT_TURN;
        prim2 = MT_FINISH;
        checkMode = true;
    }

    if( checkMode )
    {
        MEANDER_SHAPE m1( m_placer, m_width, m_dual );
        MEANDER_SHAPE m2( m_placer, m_width, m_dual );

        m1.SetBaselineOffset( m_baselineOffset );
        m2.SetBaselineOffset( m_baselineOffset );

        bool c1 = m1.Fit( prim1, aSeg, aP, aSide );
        bool c2 = false;

        if( c1 )
            c2 = m2.Fit( prim2, aSeg, m1.End(), !aSide );

        if( c1 && c2 )
        {
            m_type = prim1;
            m_shapes[0] = m1.m_shapes[0];
            m_shapes[1] = m1.m_shapes[1];
            m_baseSeg =aSeg;
            m_p0 = aP;
            m_side = aSide;
            m_amplitude = m1.Amplitude();
            m_dual = m1.m_dual;
            m_baseSeg = m1.m_baseSeg;
            m_baseIndex = m1.m_baseIndex;
            updateBaseSegment();
            m_baselineOffset = m1.m_baselineOffset;
            return true;
        }
        else
        {
            return false;
        }
    }

    int minAmpl = MinAmplitude();
    int maxAmpl = std::max( st.m_maxAmplitude, minAmpl );

    for( int ampl = maxAmpl; ampl >= minAmpl; ampl -= st.m_step )
    {
        m_amplitude = ampl;

        if( m_dual )
        {
            m_shapes[0] = genMeanderShape( aP, aSeg.B - aSeg.A, aSide, aType, m_baselineOffset );
            m_shapes[1] = genMeanderShape( aP, aSeg.B - aSeg.A, aSide, aType, -m_baselineOffset );
        }
        else
        {
            m_shapes[0] = genMeanderShape( aP, aSeg.B - aSeg.A, aSide, aType, 0 );
        }

        m_type = aType;
        m_baseSeg = aSeg;
        m_p0 = aP;
        m_side = aSide;

        updateBaseSegment();

        if( m_placer->CheckFit( this ) )
            return true;
    }

    return false;
}


void MEANDER_SHAPE::Recalculate()
{
    m_shapes[0] = genMeanderShape( m_p0, m_baseSeg.B - m_baseSeg.A, m_side, m_type,
                                   m_dual ? m_baselineOffset : 0 );

    if( m_dual )
        m_shapes[1] = genMeanderShape( m_p0, m_baseSeg.B - m_baseSeg.A, m_side, m_type,
                                       -m_baselineOffset );

    updateBaseSegment();
}


void MEANDER_SHAPE::Resize( int aAmpl )
{
    if( aAmpl < 0 )
        return;

    m_amplitude = aAmpl;

    Recalculate();
}


void MEANDER_SHAPE::MakeEmpty()
{
    updateBaseSegment();

    VECTOR2I dir = m_clippedBaseSeg.B - m_clippedBaseSeg.A;

    m_type = MT_EMPTY;
    m_amplitude = 0;

    m_shapes[0] = genMeanderShape( m_p0, dir, m_side, m_type, m_dual ? m_baselineOffset : 0 );

    if( m_dual )
        m_shapes[1] = genMeanderShape( m_p0, dir, m_side, m_type, -m_baselineOffset );
}


void MEANDERED_LINE::AddCorner( const VECTOR2I& aA, const VECTOR2I& aB )
{
    MEANDER_SHAPE* m = new MEANDER_SHAPE( m_placer, m_width, m_dual );

    m->MakeCorner( aA, aB );
    m_last = aA;

    m_meanders.push_back( m );
}


void MEANDERED_LINE::AddArc( const SHAPE_ARC& aArc1, const SHAPE_ARC& aArc2 )
{
    MEANDER_SHAPE* m = new MEANDER_SHAPE( m_placer, m_width, m_dual );

    m->MakeArc( aArc1, aArc2 );
    m_last = aArc1.GetP1();

    m_meanders.push_back( m );
}


void MEANDERED_LINE::AddArcAndPt( const SHAPE_ARC& aArc1, const VECTOR2I& aPt2 )
{
    SHAPE_ARC arc2( aPt2, aPt2, aPt2, 0 );

    AddArc( aArc1, arc2 );
}


void MEANDERED_LINE::AddPtAndArc( const VECTOR2I& aPt1, const SHAPE_ARC& aArc2 )
{
    SHAPE_ARC arc1( aPt1, aPt1, aPt1, 0 );

    AddArc( arc1, aArc2 );
}


void MEANDER_SHAPE::MakeCorner( const VECTOR2I& aP1, const VECTOR2I& aP2 )
{
    SetType( MT_CORNER );
    m_shapes[0].Clear();
    m_shapes[1].Clear();
    m_shapes[0].Append( aP1 );
    m_shapes[1].Append( aP2 );
    m_clippedBaseSeg.A = aP1;
    m_clippedBaseSeg.B = aP1;
}


void MEANDER_SHAPE::MakeArc( const SHAPE_ARC& aArc1, const SHAPE_ARC& aArc2 )
{
    SetType( MT_CORNER );
    m_shapes[0].Clear();
    m_shapes[1].Clear();
    m_shapes[0].Append( aArc1 );
    m_shapes[1].Append( aArc2 );
    m_clippedBaseSeg.A = aArc1.GetP1();
    m_clippedBaseSeg.B = aArc1.GetP1();
}


void MEANDERED_LINE::AddMeander( MEANDER_SHAPE* aShape )
{
    m_last = aShape->BaseSegment().B;
    m_meanders.push_back( aShape );
}


void MEANDERED_LINE::Clear()
{
    for( MEANDER_SHAPE* m : m_meanders )
    {
        delete m;
    }

    m_meanders.clear( );
}


int MEANDER_SHAPE::BaselineLength() const
{
    return m_clippedBaseSeg.Length();
}


long long int MEANDER_SHAPE::CurrentLength() const
{
    return CLine( 0 ).Length();
}


long long int MEANDER_SHAPE::MinTunableLength() const
{
    MEANDER_SHAPE copy = *this;

    copy.SetTargetBaselineLength( BaselineLength() );
    copy.Resize( copy.MinAmplitude() );

    return copy.CurrentLength();
}


void MEANDER_SHAPE::updateBaseSegment( )
{
    if( m_dual )
    {
        VECTOR2I midpA = ( CLine( 0 ).CPoint( 0 )  + CLine( 1 ).CPoint( 0  ) ) / 2;
        VECTOR2I midpB = ( CLine( 0 ).CPoint( -1 ) + CLine( 1 ).CPoint( -1 ) ) / 2;

        m_clippedBaseSeg.A = m_baseSeg.LineProject( midpA );
        m_clippedBaseSeg.B = m_baseSeg.LineProject( midpB );
    }
    else
    {
        m_clippedBaseSeg.A = m_baseSeg.LineProject( CLine( 0 ).CPoint( 0 ) );
        m_clippedBaseSeg.B = m_baseSeg.LineProject( CLine( 0 ).CPoint( -1 ) );
    }
}

}
