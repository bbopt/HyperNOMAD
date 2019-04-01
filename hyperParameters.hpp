//
//  HyperParameters.hpp
//  HyperNomad
//
//  Created by Christophe Tribes on 19-03-28.
//  Copyright © 2019 GERAD. All rights reserved.
//
/* ------------------------------------------------------------------------------*/
/*  HYPERNOMAD - Hyper-parameter optimization of deep neural networks with NOMAD */
/*                                                                               */
/*                                                                               */
/*  This program is free software: you can redistribute it and/or modify it      */
/*  under the terms of the GNU Lesser General Public License as published by     */
/*  the Free Software Foundation, either version 3 of the License, or (at your   */
/*  option) any later version.                                                   */
/*                                                                               */
/*  This program is distributed in the hope that it will be useful, but WITHOUT  */
/*  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or        */
/*  FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License  */
/*  for more details.                                                            */
/*                                                                               */
/*  You should have received a copy of the GNU Lesser General Public License     */
/*  along with this program. If not, see <http://www.gnu.org/licenses/>.         */
/*                                                                               */
/*  You can find information on the NOMAD software at www.gerad.ca/nomad         */
/* ------------------------------------------------------------------------------*/

#ifndef __HYPERPARAMETERS__
#define __HYPERPARAMETERS__

#include "nomad.hpp"


enum valueType {  LOWER_BOUND= -1 ,CURRENT_VALUE =0, UPPER_BOUND =1 };

enum ReportValueType {NO_REPORT, COPY_VALUE, INITIAL_VALUE, AVERAGE_VALUE} ;

struct GenericHyperParameter
{
    std::string name;
    NOMAD::bb_input_type type;
    
    NOMAD::Double value;
    NOMAD::Double upperBoundValue;
    NOMAD::Double lowerBoundValue;
    
    ReportValueType reportValueType;
    
    bool isFixed;
};



enum NeighborType {NONE,PLUS_ONE_MINUS_ONE, PLUS_TWO_MINUS_TWO, LOOP_PLUS_ONE, LOOP_MINUS_ONE } ;
enum AssociatedHyperParametersType {ZERO_TIME,ONE_TIME,MULTIPLE_TIMES} ;
struct HyperParametersBlock
{
    
    // The attributes
    std::string name;
    
    // The parameter at the head of a block.
    // If it is a categorical parameters, associated variables are possible (can be none).
    // If not a categorical parameter: there is no associated parameters (ZERO_TIME) and no neigboor type (NONE)
    GenericHyperParameter headOfBlockHyperParameter;
    
    // How to obtain neigboors
    NeighborType neighboorType;
    
    // Make the link between the categorical parameter and the associated parameters
    AssociatedHyperParametersType associatedParametersType;
    
    std::vector<GenericHyperParameter> inBlockAssociatedHyperParameters;
    
    
    // Utility functions follow
    void expand()
    {
        // The expansion is performed for MULTIPLE_TIMES associated parameters
        if ( headOfBlockHyperParameter.type == NOMAD::CATEGORICAL && associatedParametersType == MULTIPLE_TIMES ) // The head parameter of block has associated parameters
        {
            NOMAD::Double categoricalValue = headOfBlockHyperParameter.value;
            
            if ( ! categoricalValue.is_defined() || ! categoricalValue.is_integer() || categoricalValue < 0 )
            {
                std::string err = "The dimension of an hyper parameter block (head parameter " + headOfBlockHyperParameter.name + ") is invalid ";
                throw NOMAD::Exception ( __FILE__ , __LINE__ ,err);
            }
            // Expand the associated parameters by copying multiple times
            std::vector<GenericHyperParameter> tmpAssociatedHyperParameters = inBlockAssociatedHyperParameters;
            for ( size_t i= 1 ; i < categoricalValue.round() ; i++ )
                inBlockAssociatedHyperParameters.insert(inBlockAssociatedHyperParameters.end(),std::begin(tmpAssociatedHyperParameters), std::end(tmpAssociatedHyperParameters));
        }
    }
    
    void updateAssociatedParameters( NOMAD::Point & x )
    {
        if ( x.size() < inBlockAssociatedHyperParameters.size()+1 )
        {
            std::string err = "Cannot update the associated parameter with a point of insufficient dimension (head parameter " + headOfBlockHyperParameter.name + ").";
            throw NOMAD::Exception ( __FILE__ , __LINE__ ,err);
        }
        
        // update associated parameters from x
        size_t shift=1;
        for ( auto & aP : inBlockAssociatedHyperParameters )
        {
            aP.value = x[shift++];
        }
        
        std::cout << x << std::endl;
        
        NOMAD::Point xTrimmed ( static_cast<int>(x.size() - shift) );
        for ( size_t i=0 ; i < xTrimmed.size(); i++ )
        {
            xTrimmed[i]=x[i+shift];
        }
        x = xTrimmed;
        std::cout << x << std::endl;
        
    }
    
    std::vector<NOMAD::bb_input_type> getAssociatedTypes ( ) const
    {
        std::vector<NOMAD::bb_input_type> bbi;
        for ( auto aP : inBlockAssociatedHyperParameters )
        {
            bbi.push_back( aP.type );
        }
        return bbi;
    }
    
    std::vector<NOMAD::Double> getAssociatedValues ( valueType t ) const
    {
        std::vector<NOMAD::Double> values;
        for ( auto aP : inBlockAssociatedHyperParameters )
        {
            if ( t == CURRENT_VALUE )
                values.push_back( aP.value );
            else if ( t == LOWER_BOUND )
                values.push_back( aP.lowerBoundValue );
            else if ( t == UPPER_BOUND )
                values.push_back ( aP.upperBoundValue );
            else
            {
                std::string err = "The value type of " + name + "is not known ";
                throw NOMAD::Exception ( __FILE__ , __LINE__ ,err);
            }
        }
        return values;
    }
    
    size_t getDimension ( ) const
    {
        return inBlockAssociatedHyperParameters.size()+1;
    }
    
    
    std::vector<NOMAD::bb_input_type> getTypes(  ) const
    {
        std::vector<NOMAD::bb_input_type> bbi{headOfBlockHyperParameter.type };
        std::vector<NOMAD::bb_input_type> bbiAssociatedParameters = getAssociatedTypes( );
        bbi.insert(bbi.end(),std::begin(bbiAssociatedParameters), std::end(bbiAssociatedParameters));
        
        return bbi;
    }
    std::vector<NOMAD::Double> getValues( valueType t ) const
    {
        
        std::vector<NOMAD::Double> values;
        if ( t == CURRENT_VALUE )
            values.push_back( headOfBlockHyperParameter.value  );
        else if ( t == LOWER_BOUND )
            values.push_back( headOfBlockHyperParameter.lowerBoundValue  );
        else if ( t == UPPER_BOUND )
            values.push_back ( headOfBlockHyperParameter.upperBoundValue  );
        else
        {
            std::string err = "The value type of " + name + "is not known ";
            throw NOMAD::Exception ( __FILE__ , __LINE__ ,err);
        }
        
        std::vector<NOMAD::Double> valuesAssociatedParameters = getAssociatedValues( t );
        values.insert(values.end(),std::begin(valuesAssociatedParameters), std::end(valuesAssociatedParameters));
        return values;
    }
    
    
};

class HyperParameters {

protected:
    std::vector<HyperParametersBlock> _baseHyperParameters;
    
    std::string _databaseName;
    std::string _bbEXE;

    void expand();
public:
    
    NOMAD::Point getValues( valueType t ) const;
    NOMAD::Point getValues( const HyperParameters & hp , const NOMAD::Point & v ) const;
    
    std::string getBB () const { return _bbEXE;  }
    
    size_t getDimension() const;
    
    std::vector<HyperParameters> getNeighboors( const NOMAD::Point & x ) ;
    std::vector<NOMAD::bb_input_type> getTypes() const;
    void update( const NOMAD::Point & x );
    
    std::vector<size_t> getIndexFixedParams() const;
    
    virtual void read ( const std::string & hyperParamFileName ) = 0 ;
    

private:
    virtual void init ( void ) = 0;
    
    std::vector<HyperParametersBlock> _expandedHyperParameters;
    

};

#endif
