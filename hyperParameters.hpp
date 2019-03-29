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


enum valueType { CURRENT_VALUE =0, LOWER_BOUND= -1 , UPPER_BOUND =1 };

enum ReportValueType {NO_REPORT, COPY_VALUE, INITIAL_VALUE, AVERAGE_VALUE} ;

struct GenericHyperParameter
{
    std::string name;
    NOMAD::bb_input_type type;
    
    NOMAD::Double value;
    NOMAD::Double upperBoundValue;
    NOMAD::Double lowerBoundValue;
    
    ReportValueType reportValueType;
};



enum NeighborType {NONE,PLUS_ONE_MINUS_ONE, PLUS_TWO_MINUS_TWO, LOOP_PLUS_ONE, LOOP_MINUS_ONE } ;
enum AssociatedHyperParametersType {ZERO_TIME,ONE_TIME,MULTIPLE_TIMES} ;
struct HyperParametersBlock
{
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
    
    std::vector<NOMAD::bb_input_type> getAssociatedTypes ( )
    {
        std::vector<NOMAD::bb_input_type> bbi;
        for ( auto aP : inBlockAssociatedHyperParameters )
        {
            bbi.push_back( aP.type );
        }
        return bbi;
    }
    
    std::vector<NOMAD::Double> getAssociatedValues ( valueType t )
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
               std::cerr << "The value type of " << name << "is not known " << endl;
        }
        return values;
    }
    
    size_t getDimension ( )
    {
        size_t dim = 1;
        
        if ( headOfBlockHyperParameter.type == NOMAD::CATEGORICAL && associatedParametersType > ZERO_TIME ) // The head parameter of block has associated parameters
        {
            dim = inBlockAssociatedHyperParameters.size();
            if ( associatedParametersType == MULTIPLE_TIMES )
            {
                NOMAD::Double categoricalValue = headOfBlockHyperParameter.value;
                
                if ( ! categoricalValue.is_defined() || ! categoricalValue.is_integer() || categoricalValue < 0 )
                {
                    std::cerr << "The dimension of an hyper parameter block (head parameter " << headOfBlockHyperParameter.name << ") is invalid " << endl;
                    return 0;
                }
                
                dim *= categoricalValue.round();
            }
            dim ++; // This is to account for the categorical variable
        }
        // The head parameter of block has no associated parameters --> dim = 1
        
        return dim;
    }
    
    
    std::vector<NOMAD::bb_input_type> getTypes(  )
    {
        std::vector<NOMAD::bb_input_type> bbi{headOfBlockHyperParameter.type };
        
        if ( headOfBlockHyperParameter.type == NOMAD::CATEGORICAL && associatedParametersType > ZERO_TIME ) // The head parameter of block has associated parameters
        {
            NOMAD::Double categoricalValue = headOfBlockHyperParameter.value;
            if ( ! categoricalValue.is_defined() || ! categoricalValue.is_integer() || categoricalValue < 0 )
            {
                std::cerr << "The dimension of an hyper parameter block (head parameter " << headOfBlockHyperParameter.name << ") is invalid " << endl;
                return bbi;
            }
            
            std::vector<NOMAD::bb_input_type> bbiAssociatedParameters = getAssociatedTypes( );
            size_t times = ( (associatedParametersType == MULTIPLE_TIMES ) ? categoricalValue.round() : 1);
            for ( size_t i= 0 ; i < times ; i++ )
            {
                bbi.insert(bbi.end(),std::begin(bbiAssociatedParameters), std::end(bbiAssociatedParameters));
            }
        }
        
        return bbi;
    }
    std::vector<NOMAD::Double> getValues( valueType t )
    {
        std::vector<NOMAD::Double> values;
        if ( t == CURRENT_VALUE )
            values.push_back( headOfBlockHyperParameter.value  );
        else if ( t == LOWER_BOUND )
            values.push_back( headOfBlockHyperParameter.lowerBoundValue  );
        else if ( t == UPPER_BOUND )
            values.push_back ( headOfBlockHyperParameter.upperBoundValue  );
        else
            std::cerr << "The value type of " << name << "is not known " << endl;
        
        if ( headOfBlockHyperParameter.type == NOMAD::CATEGORICAL && associatedParametersType > ZERO_TIME ) // The head parameter of block has associated parameters
        {
            NOMAD::Double categoricalValue = headOfBlockHyperParameter.value;
            if ( ! categoricalValue.is_defined() || ! categoricalValue.is_integer() || categoricalValue < 0 )
            {
                std::cerr << "The dimension of an hyper parameter block (head parameter " << headOfBlockHyperParameter.name << ") is invalid " << endl;
                return values;
            }
            
            std::vector<NOMAD::Double> valuesAssociatedParameters = getAssociatedValues( t );
            size_t times = ( (associatedParametersType == MULTIPLE_TIMES ) ? categoricalValue.round() : 1);
            for ( size_t i= 0 ; i < times ; i++ )
            {
                values.insert(values.end(),std::begin(valuesAssociatedParameters), std::end(valuesAssociatedParameters));
            }
        }
        
        return values;
    }
};

class HyperParameters {

protected:
    std::vector<HyperParametersBlock> _allParameters;
    std::string _databaseName;
    std::string _bbEXE;

public:
    
    NOMAD::Point getValues( valueType t ) const;
    
    std::string getBB () const { return _bbEXE;  }
    
    size_t getDimension() const;
    
    // std::vector<HyperParameters> getNeighboors();
    std::vector<NOMAD::bb_input_type> getTypes() const;
    void update( const NOMAD::Eval_Point & x );
    
    virtual void read ( const std::string & hyperParamFileName ) = 0 ;

private:
    virtual void init ( void ) = 0;
    

};

#endif
