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
    std::vector<NOMAD::bb_input_type> bbi;
    
    for ( auto block : _allParameters )
    {
        std::vector<NOMAD::bb_input_type> blockBBI= block.getTypes( );
        bbi.insert(bbi.end(), std::begin(blockBBI), std::end(blockBBI));
    }
    return bbi;
}

size_t HyperParameters::getDimension() const
{
    size_t dim=0;
    
    for ( auto block : _allParameters )
    {
        dim += block.getDimension( );
    }
    
    return dim;
}


NOMAD::Point HyperParameters::getValues( valueType t) const
{
    size_t dim = getDimension();
    NOMAD::Point values( static_cast<int>(dim) );
    
    std::vector<NOMAD::Double> X0;
    for ( auto block : _allParameters )
    {
        std::vector<NOMAD::Double> blockValues = block.getValues( t );
        X0.insert(X0.end(), std::begin(blockValues), std::end(blockValues));
    }
    
    if ( X0.size() != dim )
    {
        std::cerr << "Dimensions are incompatible" << std::endl;
        return values;
    }
    
    for ( size_t i = 0 ; i < X0.size() ; i++ )
    {
        values[i] = X0[i];
    }
    return values;
}


void HyperParameters::update( const NOMAD::Eval_Point & x )
{
    
}

//std::vector<HyperParameters> HyperParameters::getNeighboors()
//{
//    HyperParameters poneNeighboor = getPlusOneNeighboor();
//    
//    std::vector<HyperParameters> neighboors;
//    
//    
//    
//    return neighboors;
//}
