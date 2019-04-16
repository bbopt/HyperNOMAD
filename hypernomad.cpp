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



/*-------------------------------------------------------------------*/
/*            Example of a problem with categorical variables        */
/*-------------------------------------------------------------------*/
#include "nomad.hpp"
#include "hyperParameters.hpp"
#include <vector>
#include <memory>
using namespace std;
using namespace NOMAD;



#define USE_SURROGATE false
/*--------------------------------------------------*/
/*  user class to define categorical neighborhoods  */
/*--------------------------------------------------*/
class My_Extended_Poll : public Extended_Poll
{
    
private:
    
    // vector of signatures
    int _extended_poll_call;
    std::shared_ptr<HyperParameters> _hyperParameters;
    
    
public:
    
    // constructor:
    My_Extended_Poll ( Parameters & p , std::shared_ptr<HyperParameters> & hyperParameters ):
    Extended_Poll ( p ), _hyperParameters(std::move(hyperParameters))
    {
    }
    
    // destructor:
    virtual ~My_Extended_Poll ( void ) {}
    
    // construct the extended poll points:
    virtual void construct_extended_points ( const Eval_Point &);
    
}; 


/*------------------------------------------*/
/*            NOMAD main function           */
/*------------------------------------------*/
int main ( int argc , char ** argv )
{

    // NOMAD initializations:
    begin ( argc , argv );
    
    // display:
    Display out ( cout );
    out.precision ( DISPLAY_PRECISION_STD );
    
    std::string hyperParamFile="";
    if ( argc > 1 )
        hyperParamFile = argv[1];
    
    try
    {
        
        // parameters creation:
        Parameters p ( out );
        
        std::shared_ptr<HyperParameters> hyperParameters = std::make_shared<HyperParameters>(hyperParamFile);
        //        if ( ! readHyperParametersFromFile(hyperParameters,argv[0]) )
        //        {
        //            std::cerr << "Problem reading hyper parameter file" << std::endl;
        //            return 0;
        //        }
        
//        NOMAD::Point X0 = hyperParameters->getValues( CURRENT_VALUE) ;
//        std::vector<HyperParameters> neighboors = hyperParameters->getNeighboors(X0);
//        return 0 ;
        
        p.set_DISPLAY_DEGREE( 3 );
        
        p.set_DIMENSION( static_cast<int>(hyperParameters->getDimension()) );
        p.set_X0( hyperParameters->getValues( CURRENT_VALUE) );
        p.set_BB_INPUT_TYPE( hyperParameters->getTypes() );
        p.set_LOWER_BOUND( hyperParameters->getValues( LOWER_BOUND ) );
        p.set_UPPER_BOUND( hyperParameters->getValues( UPPER_BOUND ) );

        std::vector<size_t> indexFixedParams = hyperParameters->getIndexFixedParams();
        for ( auto i : indexFixedParams )
            p.set_FIXED_VARIABLE( static_cast<int>(i) );
        
        p.set_BB_OUTPUT_TYPE ( hyperParameters->getBbOutputType() );
        p.set_BB_EXE( hyperParameters->getBB() );

        p.set_MAX_BB_EVAL( static_cast<int>(hyperParameters->getMaxBbEval()) );

        p.set_EXTENDED_POLL_TRIGGER ( 10 , false );
        p.set_DISPLAY_STATS("bbe ( sol ) obj");
        p.set_STATS_FILE("stats.txt","bbe ( sol ) obj");
	p.set_HISTORY_FILE("history.txt");
        
//        if ( USE_SURROGATE )
//            p.set_HAS_SGTE ( true );
        
        
        // parameters validation:
        p.check();
        
        // extended poll:
        My_Extended_Poll ep ( p , hyperParameters );
        
        // algorithm creation and execution:
        Mads mads ( p , NULL , &ep , NULL , NULL );
        mads.run();
    }
    catch ( exception & e ) {
        string error = string ( "HYPER NOMAD has been interrupted: " ) + e.what();
        if ( Slave::is_master() )
            cerr << endl << error << endl << endl;
    }
    
    
    Slave::stop_slaves ( out );
    end();
    
    return EXIT_SUCCESS;
}

/*--------------------------------------*/
/*  construct the extended poll points  */
/*      (categorical neighborhoods)     */
/*--------------------------------------*/
void My_Extended_Poll::construct_extended_points ( const Eval_Point & x)
{
    
    // Get the neighboors of the point (an update of the hyper parameters structure is performed)
    std::vector<HyperParameters> neighboors = _hyperParameters->getNeighboors(x);

    for ( auto & nHyperParameters : neighboors )
    {
        size_t nDim = nHyperParameters.getDimension();
        vector<bb_input_type> nBbit = nHyperParameters.getTypes();

        NOMAD::Point nLowerBound = nHyperParameters.getValues( LOWER_BOUND );
        NOMAD::Point nUpperBound = nHyperParameters.getValues( UPPER_BOUND );
        NOMAD::Point nX = nHyperParameters.getValues( CURRENT_VALUE );

        // Create a parameter to obtain a signature for this neighboor
        NOMAD::Parameters nP ( _p.out() );
        nP.set_DIMENSION( static_cast<int>(nDim) );
        nP.set_X0 ( nX );
        nP.set_LOWER_BOUND( nLowerBound );
        nP.set_UPPER_BOUND( nUpperBound );

        nP.set_BB_INPUT_TYPE( nBbit );
        nP.set_MESH_TYPE( NOMAD::XMESH );  // Need to force set XMesh

        std::vector<size_t> indexFixed = _hyperParameters->getIndexFixedParams();
        for ( auto i : indexFixed )
            nP.set_FIXED_VARIABLE( static_cast<int>(i) );

        // Some parameters come from the original problem definition
        nP.set_BB_OUTPUT_TYPE( _p.get_bb_output_type() );
        nP.set_BB_EXE( _p.get_bb_exe() );
        // Check is need to create a valid signature
        nP.check();

        // The signature to be registered with the neighboor point
        add_extended_poll_point ( nX , *(nP.get_signature()) );
    }
    
//    // number of convolutional layers
//    int n = static_cast<int> (x[0].value());
//
//    // number of fully connected layer
//    int index_m = _shift1*n + 1;
//    int m = static_cast<int> (x[index_m].value());
//
//    // get the size and type of the current point
//    const vector<bb_input_type> & bbit_curr = x.get_signature()->get_input_types();
//    int current_dim = bbit_curr.size();
//
//    // get the bounds and and directions of the current point
//    const Point & d0_curr = x.get_signature()->get_mesh()->get_initial_poll_size();
//    const Point & lb_curr = x.get_signature()->get_lb();
//    const Point & ub_curr = x.get_signature()->get_ub();
//
//    // dimensions of the neighbors
//    int dim1 = current_dim + _shift1;
//    int dim2 = current_dim - _shift1;
//    int dim3 = current_dim + _shift2;
//    int dim4 = current_dim - _shift2;
//
//    if (n < ub_curr[0]){
//        // We can add a convolutional layer
//        Point d0_1 (dim1);
//        Point lb_1 (dim1);
//        Point ub_1 (dim1);
//
//        vector<bb_input_type> bbit_1 (dim1);
//        bbit_1[0] = bbit_1[index_m + _shift1] = NOMAD::CATEGORICAL;
//
//        for ( int i = 1 ; i < index_m + _shift1 ; i++ )
//            bbit_1[i] = NOMAD::INTEGER;
//
//        for ( int i = index_m + _shift1 + 1 ; i < dim1 ; i++ )
//            bbit_1[i] = bbit_curr[i-_shift1];
//
//        // categorical variables don't need bounds
//        d0_1[0] = d0_1[index_m + _shift1] = 1;
//
//        for (int i = 1; i< index_m; i++){
//            lb_1  [i] = lb_curr[i];
//            ub_1  [i] = ub_curr[i];
//            d0_1  [i] = d0_curr[i];
//        }
//
//        for ( int i = index_m ; i < index_m + _shift1 ; i++ )
//        {
//            lb_1  [i] = lb_curr[i-_shift1];
//            ub_1  [i] = ub_curr[i-_shift1];
//            d0_1  [i] = d0_curr[i-_shift1];
//        }
//
//        for ( int i = index_m + _shift1 + 1 ; i < dim1 ; i++ ){
//            lb_1  [i] = lb_curr[i-_shift1];
//            ub_1  [i] = ub_curr[i-_shift1];
//            d0_1  [i] = d0_curr[i-_shift1];
//        }
//
//        NOMAD::Signature s1 (dim1,
//                             bbit_1                     ,
//                             d0_1                       ,
//                             lb_1                       ,
//                             ub_1                       ,
//                            _p.get_direction_types   () ,
//                            _p.get_sec_poll_dir_types() ,
//                            _p.get_int_poll_dir_types() ,
//                            _p.out()                    );
//
//
//
//        Point y (dim1);
//        y[0] = n + 1;
//        y[1] = 10;
//        y[2] = 5;
//        y[3] = 1;
//        y[4] = 0;
//        y[5] = 0;
//        for (int i = 5 ; i < dim1 ; i++)
//        {
//            y[i] = x[i-_shift1];
//        }
//        add_extended_poll_point (y , s1);
//        // cerr << endl << " Done with 1st neighbor." << endl << endl;
//    }
//
//    if (n>lb_curr[0]){
//        // We can substract a convolutional layer
//
//        Point d0_2 (dim2);
//        Point lb_2 (dim2);
//        Point ub_2 (dim2);
//
//        vector<bb_input_type> bbit_2 (dim2);
//        bbit_2[0] = bbit_2[index_m - _shift1] = NOMAD::CATEGORICAL;
//
//        for ( int i = 1 ; i < index_m - _shift1 ; i++ )
//            bbit_2[i] = NOMAD::INTEGER;
//
//        for ( int i = index_m - _shift1 ; i < dim2 ; i++ )
//            bbit_2[i] = bbit_curr[i+_shift1];
//
//        // Categorical variables don't need bounds
//        d0_2[0] = d0_2[index_m - _shift1] = 1;
//
//        for ( int i = 1 ; i < index_m - _shift1 ; i++ )
//        {
//            lb_2  [i] = lb_curr[i+_shift1];
//            ub_2  [i] = ub_curr[i+_shift1];
//            d0_2  [i] = d0_curr[i+_shift1];
//        }
//        lb_2  [index_m - _shift1] = lb_curr[index_m];
//        ub_2  [index_m - _shift1] = ub_curr[index_m];
//
//        for ( int i = index_m - _shift1 +1 ; i < dim2 ; i++ )
//        {
//            lb_2  [i] = lb_curr[i+_shift1];
//            ub_2  [i] = ub_curr[i+_shift1];
//            d0_2  [i] = d0_curr[i+_shift1];
//        }
//
//        NOMAD::Signature s2 (dim2,
//                             bbit_2                     ,
//                             d0_2                       ,
//                             lb_2                       ,
//                             ub_2                       ,
//                             _p.get_direction_types   () ,
//                             _p.get_sec_poll_dir_types() ,
//                             _p.get_int_poll_dir_types() ,
//                             _p.out()                    );
//
//
//        Point y2 (dim2);
//        y2[0] = n - 1;
//        for (int i = 1 ; i < dim2 ; i++)
//        {
//            y2[i] = x[i+_shift1];
//        }
//        add_extended_poll_point (y2 , s2);
//        // cerr << endl << " Done with 2nd neighbor." << endl << endl;
//
//    }
//
//    if(m<ub_curr[index_m]){
//        // We can add a full layer
//        Point d0_3 (dim3);
//        Point lb_3 (dim3);
//        Point ub_3 (dim3);
//
//        vector<bb_input_type> bbit_3 (dim3);
//        bbit_3[0] = bbit_3[index_m] = NOMAD::CATEGORICAL;
//
//        for ( int i = 1 ; i < index_m ; i++ )
//            bbit_3[i] = NOMAD::INTEGER;
//
//        bbit_3[index_m + 1] = NOMAD::INTEGER;
//
//        for ( int i = index_m + _shift2 +1 ; i < dim3 ; i++ )
//            bbit_3[i] = bbit_curr[i-_shift2];
//
//        // Categorical variables don't need bounds
//        d0_3[0] = 1;
//
//        for ( int i = 1 ; i < index_m ; i++ )
//        {
//            lb_3  [i] = lb_curr[i];
//            ub_3  [i] = ub_curr[i];
//            d0_3  [i] = d0_curr[i];
//        }
//        // Categorical variables don't need bounds
//        d0_3[index_m] = 1;
//
//        lb_3  [index_m + 1] = lb_curr[index_m + 1];
//        ub_3  [index_m + 1] = ub_curr[index_m + 1];
//        d0_3  [index_m + 1] = d0_curr[index_m + 1];
//
//        for ( int i = index_m + _shift2 + 1 ; i < dim3 ; i++ ){
//            lb_3  [i] = lb_curr[i-_shift2];
//            ub_3  [i] = ub_curr[i-_shift2];
//            d0_3  [i] = d0_curr[i-_shift2];
//        }
//
//        NOMAD::Signature s3 (dim3,
//                             bbit_3                     ,
//                             d0_3                       ,
//                             lb_3                       ,
//                             ub_3                       ,
//                             _p.get_direction_types   () ,
//                             _p.get_sec_poll_dir_types() ,
//                             _p.get_int_poll_dir_types() ,
//                             _p.out()                    );
//        Point y3 (dim3);
//
//        for (int i = 0 ; i < index_m ; i++)
//        {
//            y3[i] = x[i];
//        }
//        y3[index_m] = m+1;
//        y3[index_m + _shift2] = 100;
//        for (int i = index_m + _shift2 + 1 ; i < dim3 ; i++)
//        {
//            y3[i] = x[i-_shift2];
//        }
//        add_extended_poll_point (y3 , s3);
//        // cerr << endl << " Done with 3th neighbor." << endl << endl;
//    }
//
//    if(m>lb_curr[index_m]){
//        // we can substract a layer
//        Point d0_4 (dim4);
//        Point lb_4 (dim4);
//        Point ub_4 (dim4);
//
//        vector<bb_input_type> bbit_4 (dim4);
//        bbit_4[0] = bbit_4[index_m] = NOMAD::CATEGORICAL;
//
//        for ( int i = 1 ; i < index_m ; i++ )
//            bbit_4[i] = NOMAD::INTEGER;
//
//        for ( int i = index_m + 1 ; i < dim4 ; i++ )
//            bbit_4[i] = bbit_curr[i+_shift2];
//
//        // Categorical variables don't need bounds
//        d0_4[0] = 1;
//
//        for ( int i = 1 ; i < index_m ; i++ )
//        {
//            lb_4  [i] = lb_curr[i];
//            ub_4  [i] = ub_curr[i];
//            d0_4  [i] = d0_curr[i];
//        }
//        // Categorical variables don't need bounds
//        d0_4[index_m] = 1;
//
//        for ( int i = index_m + 1 ; i < dim4 ; i++ ){
//            lb_4  [i] = lb_curr[i+_shift2];
//            ub_4  [i] = ub_curr[i+_shift2];
//            d0_4  [i] = d0_curr[i+_shift2];
//        }
//
//        NOMAD::Signature s4 (dim4,
//                             bbit_4                     ,
//                             d0_4                       ,
//                             lb_4                       ,
//                             ub_4                       ,
//                             _p.get_direction_types   () ,
//                             _p.get_sec_poll_dir_types() ,
//                             _p.get_int_poll_dir_types() ,
//                             _p.out()                    );
//        Point y4 (dim4);
//
//        for (int i = 0 ; i < index_m ; i++)
//        {
//            y4[i] = x[i];
//        }
//        y4[index_m] = m-1;
//        for (int i = index_m + 1 ; i < dim4 ; i++)
//        {
//            y4[i] = x[i+_shift2];
//        }
//        add_extended_poll_point (y4 , s4);
//        // cerr << endl << " Done with 4th neighbor." << endl << endl;
//     }
}


