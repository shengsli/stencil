/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Pipeline.hpp
 * Author: tonio
 *
 * Created on June 2, 2018, 6:53 PM
 */

#ifndef PIPELINE_HPP
#define PIPELINE_HPP


#include <cstdlib>
#include <iostream>
#include <vector>
#include <type_traits>
#include <functional>
#include <stdarg.h>
#include <typeinfo>

#include <utility>
#include <thread>


using namespace std;

//double stage1(int in, int x, double z) {
//    
//    in *= x;
//    
//    return in/z;
//}
//
//double stage2(double in, int x, double z) {
//    
//    in -= x + 0.5;
//    
//    return in;
//}
//
//string stage3(double in, int x, double z) {
//    
//    return to_string(in)+"HAI";
//}
//
//template<typename ...PSRTs>
//struct PipeRT;
//
//template<typename T> 
//struct function_traits;  
//
//template<typename R, typename ...Args> 
//struct function_traits<std::function<R(Args...)>>
//{
//    static const size_t nargs = sizeof...(Args);
//
//    typedef R result_type;
//
//    template <size_t i>
//    struct arg
//    {
//        typedef typename std::tuple_element<i, std::tuple<Args...>>::type type;
//    };
//};
//
//typedef std::function<string(double)> fun;
//
//template<template <typename ...PSR> class T,
//                typename ...PSTs>
//class PipelineImpl {
//    
//    private:
//        
//        tuple<PSTs...> pipeStages;
//        tuple<PSR...> pipeReturnTypes;
//        
//        template<std::size_t I = 0, typename RT, typename ...LSITs, typename ARG, typename ...ARGs>
//        inline typename std::enable_if<I == sizeof...(PSTs), RT>::type
//        runStages( RT(*lastStage)(LSITs...), ARG arg, ARGs ...args){
//            return arg; // arg is the result of the pipeline
//        }
//
//        template<std::size_t I = 0, typename RT, typename ...LSITs, typename ARG, typename ...ARGs>
//        inline typename std::enable_if<I < sizeof...(PSTs), RT>::type
//        runStages( RT(*lastStage)(LSITs...), ARG arg, ARGs ...args ) {
//            
//            
//          return runStages<I + 1>( lastStage, std::get<I>(pipeStages)(arg, args...), args... ); 
//          
//        }
//    
//    public:
//        
//        PipelineImpl(tuple<PSR...> pipeReturnTypes, tuple<PSTs...> pipeStages) : pipeStages(pipeStages), pipeReturnTypes(pipeReturnTypes) {}
//        
//        template<typename OUT, typename IN, typename ...ARGs> 
//        vector<OUT> operator()(vector<OUT> &v, vector<IN> input, ARGs... args) {
//            
//            for(auto el : input) {
//               
//                auto r = runStages( get<std::tuple_size<tuple<PSTs...>>::value-1>(pipeStages), el, args... );
//                
//                
//                v.push_back(r);
//            }
//            
//            return v;
//        }
//};
//
//class PipelineInitialiser {
//        
//    public: 
//        
//        template<template <typename ...PSR> class T,
//                typename ...PSTs>
//        PipelineImpl<PSTs...> operator()(PSTs ...pss) {
//            
//            tuple<PSR...> stagesReturnTypes;
//            tuple<PSTs...> pipeStages = make_tuple(pss...);
//             
//            PipelineImpl<PipeRT<PSR...>, PSTs...> pipeline(stagesReturnTypes, pipeStages);
//            
//            return pipeline;
//        }
//        
//};

//template< template<typename ...PSR> class T,
//            typename ...PSTs>
//PipelineImpl<PSTs...> Pipeline(PSTs ...pss) {
//    
//    PipelineInitialiser pipeInit;
//    
//    return pipeInit.operator ()<PipeRT<PSR...>>(pss...);
//}


class PipeStageBase {
    
    virtual void execute() = 0;
    
};

template<typename IN, typename OUT>
class PipeStage : PipeStageBase {
    
    
public:
    
    vector<OUT>* outBuffer;
    
    vector<IN> inBuffer;
    
    void execute(){
        
        for(IN in : inBuffer ){
            
            OUT out = this->run( in );
            
            outBuffer->push_back( out );
        }
    }
    
    virtual OUT run(IN in) = 0;
    
};

class Pipeline {
    
};

class PipelineInit {
    
    template<typename ...PSs>
    Pipeline operator()(PSs ...pss){
        
        executePipeline<0>(pss...);
        
        
    }
    
    template<size_t N, typename LIN, typename LOUT>
    void executePipeline(PipeStage<LIN,LOUT> pipeStage) {
        
        vector<LOUT> collector;
        
        pipeStage.outBuffer = &collector;
        
    }
    
    template<size_t N, typename IN1, typename OUT1, typename OUT2, typename ...PSs>
    void executePipeline(PipeStage<IN1,OUT1> pipeStage1, PipeStage<OUT1,OUT2> pipeStage2, PSs ...pss) {
        
        pipeStage1.outBuffer = pipeStage2.inBuffer;
        
        executePipeline<N+1>(pipeStage2, pss...);
    }
    
};


class Stage1 : PipeStage<int,double> {
    
    double run(int in) {
        
        return in/2.0;
    }
}stage1;

class Stage2 : PipeStage<double,string> {
    
    string run(double in) {
        
        return std::to_string( in/2.0 );
    }
}stage2;



class Stage3 : PipeStage<string,double> {
    
    double run(string in) {
        
        return std::stod( in );
    }
}stage3;




#endif /* PIPELINE_HPP */

