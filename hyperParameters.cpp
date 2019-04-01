//
//  hyperParameters.cpp
//  HyperNomad
//
//  Created by Christophe Tribes on 19-03-28.
//  Copyright © 2019 GERAD. All rights reserved.
//

#include "hyperParameters.hpp"

std::vector<NOMAD::bb_input_type> HyperParameters::getTypes() const
{
    if ( _expandedHyperParameters.size() == 0 )
    {
        throw NOMAD::Exception ( __FILE__ , __LINE__ ,"HyperParameters: Cannot get dimension because the hyper parameters structure has not been expanded");
    }
    
    std::vector<NOMAD::bb_input_type> bbi;
    
    for ( auto const & block : _expandedHyperParameters )
    {
        std::vector<NOMAD::bb_input_type> blockBBI= block.getTypes( );
        bbi.insert(bbi.end(), std::begin(blockBBI), std::end(blockBBI));
    }
    return bbi;
}

size_t HyperParameters::getDimension() const
{
    size_t dim=0;
    
    if ( _expandedHyperParameters.size() == 0 )
    {
        throw NOMAD::Exception ( __FILE__ , __LINE__ ,"HyperParameters: Cannot get dimension because the hyper parameters structure has not been expanded");
    }

    
    for ( auto const & block : _expandedHyperParameters )
    {
        dim += block.getDimension( );
    }
    
    return dim;
}


NOMAD::Point HyperParameters::getValues( valueType t) const
{
    
    size_t dim = getDimension();
    NOMAD::Point values( static_cast<int>(dim) );
    
    if ( _expandedHyperParameters.size() == 0 )
    {
        throw NOMAD::Exception ( __FILE__ , __LINE__ ,"HyperParameters: Cannot get values because the hyper parameters structure has not been expanded");
    }

    
    std::vector<NOMAD::Double> X0;
    for ( auto const & block : _expandedHyperParameters )
    {
        std::vector<NOMAD::Double> blockValues = block.getValues( t );
        X0.insert(X0.end(), std::begin(blockValues), std::end(blockValues));
    }
    
    if ( X0.size() != dim )
    {
        throw NOMAD::Exception ( __FILE__ , __LINE__ ,"HyperParameters: Dimensions are incompatible");
    }
    
    for ( size_t i = 0 ; i < X0.size() ; i++ )
    {
        values[i] = X0[i];
    }
    return values;
}

NOMAD::Point HyperParameters::getValues( const HyperParameters & hp , const NOMAD::Point & v ) const
{
    NOMAD::Point updateV ( getDimension() );
    
    
//    else
//    {
//        std::string err = "The value type of " + name + "is not known ";
//        throw NOMAD::Exception ( __FILE__ , __LINE__ ,err);
//    }
//
//    std::vector<NOMAD::Double> valuesAssociatedParameters = getAssociatedValues( t );
//    values.insert(values.end(),std::begin(valuesAssociatedParameters), std::end(valuesAssociatedParameters));
//    return values;
    
    return updateV;
}



// TODO
std::vector<size_t> HyperParameters::getIndexFixedParams() const
{
    std::vector<size_t> indices;
    return indices;
}

void HyperParameters::update( const NOMAD::Point & x )
{
    // Start over from baseHyperParameters that have not been expanded to full size
    _expandedHyperParameters = _baseHyperParameters;
    
    if ( _expandedHyperParameters.size() == 0 )
    {
        throw NOMAD::Exception ( __FILE__ , __LINE__ ,"HyperParameters: Cannot update because the hyper parameters structure has not been expanded" );
    }

    NOMAD::Point xBlock (x);
    for ( auto & block : _expandedHyperParameters )
    {
        // Update the head of block parameter
        block.headOfBlockHyperParameter.value = xBlock[0];
        
        // Expand the block structure from updtate the head value, update the associated parameters with xBlock value and trim xBlock for next block
        block.expand();
        block.updateAssociatedParameters ( xBlock );
    }
    if ( xBlock.size() != 0 )
    {
        throw NOMAD::Exception ( __FILE__ , __LINE__ ,"HyperParameters: Cannot update because the structure of hyper parameters is not consistent with the size of the point." );
    }
    
}

void HyperParameters::expand ()
{
    _expandedHyperParameters = _baseHyperParameters;
    for ( auto & block : _expandedHyperParameters )
    {
        block.expand();
    }
}


std::vector<HyperParameters> HyperParameters::getNeighboors( const NOMAD::Point & x )
{
    // HyperParameters poneNeighboor = getPlusOneNeighboor();
    
    std::vector<HyperParameters> neighboors;
    
    return neighboors;
}
