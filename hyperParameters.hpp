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

const std::string UndefinedStr="Undefined";

enum class ValueType { LOWER_BOUND ,CURRENT_VALUE , UPPER_BOUND , INITIAL_VALUE };

class HyperParameters {
private:
    
    enum class ReportValueType { NO_REPORT, COPY_VALUE, COPY_INITIAL_VALUE } ;
    enum class FixedParameterType { NEVER, ALWAYS, IF_IN_LAST_GROUP, IF_IN_FIRST_GROUP };
    
    struct GenericHyperParameter
    {
        std::string name=UndefinedStr;
        NOMAD::bb_input_type type=NOMAD::CONTINUOUS ;
        
        NOMAD::Double value {};
        
        NOMAD::Double lowerBoundValue;
        NOMAD::Double upperBoundValue;
        
        // Remaining attributes do not need to be set during initialization.
        ReportValueType reportValueType = ReportValueType::NO_REPORT ;
        
        FixedParameterType fixedParamType = FixedParameterType::NEVER;
        
        NOMAD::Double fixedValue {};
        NOMAD::Double initialValue {};

        bool isFixed = false;
        
        bool isDefined () const { return name.compare(UndefinedStr) != 0 ;}
        
    };
    
    enum class NeighborType {NONE,PLUS_ONE_MINUS_ONE_RIGHT,PLUS_ONE_MINUS_ONE_LEFT, LOOP_PLUS_ONE_RIGHT, LOOP_PLUS_ONE_LEFT, LOOP_MINUS_ONE_RIGHT , LOOP_MINUS_ONE_LEFT } ;
    enum class AssociatedHyperParametersType {ZERO_TIME,ONE_TIME,MULTIPLE_TIMES} ;
    
    typedef std::vector<std::vector<GenericHyperParameter>> GroupsOfAssociatedHyperParameters;
    
    struct HyperParametersBlock
    {
        
        // The attributes
        std::string name;
        
        // The parameter at the head of a block.
        // If it is a categorical parameters, associated variables are possible (can be none).
        // If not a categorical parameter: there is no associated parameters (ZERO_TIME) and no neigboor type (NONE)
        GenericHyperParameter headOfBlockHyperParameter;
        
        // How to obtain neigboors
        NeighborType neighborType;
        
        // Make the link between the categorical parameter and the associated parameters
        AssociatedHyperParametersType associatedParametersType = AssociatedHyperParametersType::ZERO_TIME;
        
        GroupsOfAssociatedHyperParameters groupsOfAssociatedHyperParameters;
        
        //---------------------------------------------//
        // Utility functions follow
        //---------------------------------------------//
        
        // Expansion
        void expandAssociatedParameters(); // Used for expanding baseHyperParameter -> expandHyperParameter
        
        // Set the flags for dynamic fixed variables
        void setAssociatedParametersType();
        
        
        // Update the values of all the associated parameters using values in x
        void updateAssociatedParameters( NOMAD::Point & x );
        
        // Get an updated group of associated hyper parameters
        std::vector<GenericHyperParameter> updateAssociatedParameters ( std::vector<GenericHyperParameter> & fromGroup , bool isLastGroup =false, bool isFirstGroup = false  ) const;
        
        void expandAndUpdateAssociatedParametersWithConstraints( ); // increase to match head parameter value

        
        void reduceAssociatedParametersWithConstraints( ); // decrease to match head parameter value
        
        
        std::vector<NOMAD::bb_input_type> getAssociatedTypes ( ) const;
        std::vector<NOMAD::Double> getAssociatedValues ( ValueType t ) const;
        std::vector<size_t> getIndexFixedParams( size_t & current_index ) const ;
        
        size_t getDimension ( ) const ;
        size_t getNumberOfGroupsAssociatedParameters ( ) const { return groupsOfAssociatedHyperParameters.size(); }
        
        std::vector<NOMAD::bb_input_type> getTypes(  ) const;
        std::vector<NOMAD::Double> getValues( ValueType t ) const;
        std::vector<HyperParametersBlock> getNeighboorsOfBlock( ) const;
        
        void check();

    };
    
    std::vector<HyperParametersBlock> _baseHyperParameters;
    std::vector<HyperParametersBlock> _expandedHyperParameters;
    
    std::string _databaseName;
    std::string _bbEXE;
    vector<NOMAD::bb_output_type> _bbot;
    size_t _maxBbEval;
    
    NOMAD::Point _X0;

    void expand();
    
    void check();
    
    void initBlockStructureToDefault ( void );
    
    HyperParameters ( const std::vector<HyperParametersBlock> & hpbs);

    
    
    
public:
    
    void operator=(const HyperParameters&) = delete; // No usual assignement is allowed --> see private constructor for assignement from blocks of hyper parameters
    
    HyperParameters ( const std::string & hyperParamFileName );
    
    NOMAD::Point getValues( ValueType t ) const;
    
    const std::string & getBB ( void ) const { return _bbEXE;  }
    const vector<NOMAD::bb_output_type> & getBbOutputType ( void ) const { return _bbot; }
    size_t getMaxBbEval( void ) const { return _maxBbEval; }
    
    size_t getDimension( void ) const;
    
    std::vector<NOMAD::bb_input_type> getTypes() const;
    void update( const NOMAD::Point & x );
    
    // MAYBE TODO
    // std::vector<std::pair<size_t,NOMAD::Double>> getFixedParams() const;
    std::vector<size_t> getIndexFixedParams() const;
    
    std::vector<HyperParameters> getNeighboors( const NOMAD::Point & x ) ;

};

#endif
