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
#include "fileutils.hpp"

const std::string UndefinedStr="Undefined";

enum class ValueType { LOWER_BOUND ,CURRENT_VALUE , UPPER_BOUND , INITIAL_VALUE , FIXED_VARIABLE };

class HyperParameters {
private:
    
    enum class ReportValueType { NO_REPORT, COPY_VALUE, COPY_INITIAL_VALUE } ;
    
    struct GenericHyperParameter
    {
        std::string searchName=UndefinedStr;
        std::string fullName;
        
        NOMAD::bb_input_type type=NOMAD::CONTINUOUS ;
        
        NOMAD::Double value {};
        
        // The bounds can have undefined NOMAD::Double
        NOMAD::Double lowerBoundValue;
        NOMAD::Double upperBoundValue;
        
        // Remaining attributes do not need to be set during initialization because they have default.
        ReportValueType reportValueType = ReportValueType::NO_REPORT ;
        
        bool isFixed = false;

        NOMAD::Double initialValue {};

        
        // This for managing when a value is set with X0, LOWER_BOUND, UPPER_BOUND keywords in HyperParam file or by setting hyper param individually (for ex.: DROPOUT_RATE 0.2 - - VAR).
        bool settingByName = false;
        
        bool isDefined () const { return searchName.compare(UndefinedStr) != 0 && searchName.size() != 0 ;}
        
        void display ( bool detailedDisplay ) const;
        
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
        void expandAssociatedParameters(); // Expanding baseHyperParameter -> expandHyperParameter
        
        // Update the values of all the associated parameters using values in x
        void updateAssociatedParameters( NOMAD::Point & x , NOMAD::Point & lb , NOMAD::Point & ub );
        
        // Get an updated group of associated hyper parameters
        std::vector<GenericHyperParameter> updateAssociatedParameters ( std::vector<GenericHyperParameter> & fromGroup  ) const;
        
        void expandAndUpdateAssociatedParametersWithConstraints( ); // increase to match head parameter value

        
        void reduceAssociatedParametersWithConstraints( ); // decrease to match head parameter value
        
        const GenericHyperParameter & getHyperParameter ( size_t index ) const ;
        
        std::vector<NOMAD::bb_input_type> getAssociatedTypes ( ) const;
        std::vector<NOMAD::Double> getAssociatedValues ( ValueType t ) const;
        std::vector<size_t> getIndexFixedParams( size_t & current_index ) const ;

        std::set<int> getVariableGroup( size_t & current_index ) const ;
        
        size_t getDimension ( ) const ;
        size_t getNumberOfGroupsAssociatedParameters ( ) const { return groupsOfAssociatedHyperParameters.size(); }
        
        std::vector<NOMAD::bb_input_type> getTypes(  ) const;
        std::vector<NOMAD::Double> getValues( ValueType t ) const;
        std::vector<HyperParametersBlock> getNeighboorsOfBlock( ) const;
        
        std::vector<std::string> getSearchNames() const;
        
        
        // Get an hyper parameter to perform modification
        GenericHyperParameter* getHyperParameter( const std::string & searchName ) ;
        
        void check();
        
        void display( bool detailedDisplay ) const;

    };
    
    std::vector<HyperParametersBlock> _baseHyperParameters;
    std::vector<HyperParametersBlock> _expandedHyperParameters;
    
    std::vector<std::string> _allSearchNames;
    
    std::string _dataset;
    std::string _bbEXE;
    std::vector<NOMAD::bb_output_type> _bbot;
    size_t _maxBbEval;
    std::list<std::string> _registeredDataset;

    
    NOMAD::Point _X0, _lowerBound, _upperBound , _fixedVariables;
    
    size_t _hyperDisplay;
    
    size_t _lhIterationSearch;
    
    bool _explicitSetLowerBounds;
    bool _explicitSetUpperBounds;
    bool _explicitSetX0;

    void expand();
    
    void check();
    
    void registerSearchNames();
    
    
    // for _baseHyperParameters only
    GenericHyperParameter * getHyperParameter( const std::string & searchName ) ;
    
    void initBlockStructureToDefault ( void );
    
    HyperParameters ( const std::vector<HyperParametersBlock> & hpbs);

    void read ( const std::string & hyperParamFileName );
    
    void updateAndCheckAfterReading();
    
    void interpretX0( NOMAD::Parameter_Entries * entries ) ;
    
    void interpretBoundsAndFixed( const std::string & paramName , const NOMAD::Parameter_Entries & entries , NOMAD::Point & param ) ;
    
public:
    
    void operator=(const HyperParameters&) = delete; // No usual assignement is allowed --> see private constructor for assignement from blocks of hyper parameters
    
    HyperParameters ( const std::string & hyperParamFileName , const std::string & hyperNomadPath , const std::string & defaultPytorchBB );
    
    NOMAD::Point getValues( ValueType t ) const;
    
    const std::string & getBB ( void ) const { return _bbEXE;  }
    const vector<NOMAD::bb_output_type> & getBbOutputType ( void ) const { return _bbot; }
    size_t getMaxBbEval( void ) const { return _maxBbEval; }
    
    size_t getDimension( void ) const;
    
    std::vector<NOMAD::bb_input_type> getTypes() const;
    
    void updateFromBaseAndPerformExpansion( const NOMAD::Point & x , bool explicitSetX0 = false, bool explicitSetLowerBounds = false , bool explicitSetUpperBounds = false );
    
    std::vector<size_t> getIndexFixedParams() const;
    std::vector<std::set<int>> getVariableGroupsIndices() const;
    
    std::vector<HyperParameters> getNeighboors( const NOMAD::Point & x ) ;
    
    void setHyperDisplay ( size_t d ) { _hyperDisplay = d; }
    size_t getHyperDisplay() const { return _hyperDisplay ;}
    
    size_t getLhIterationSearch () const { return _lhIterationSearch ;}
    
    void display() const;

};

#endif

