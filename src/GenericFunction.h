/********************************************************
 **
 **  GenericFunction.h
 **
 **  Part of the Makernet framework by Jeremy Gilbert
 **
 **  License: GPL 3
 **  See footer for copyright and license details.
 **
 ********************************************************/

#ifndef GENERICFUNCTION_H
#define GENERICFUNCTION_H

#define CLOSURE_STATE_SIZE 8

#include <strings.h>
#include <Debug.h>

template <typename T>
class generic_function;

template <typename ReturnValue, typename... Args>
class generic_function<ReturnValue(Args...)> {
public:
    template <typename T>
    generic_function& operator=(T t) {
        auto func = new CallableT<T>(t);
        size_ = sizeof( func ); // for instrumentation, OK to remove
        if( sizeof(func) > CLOSURE_STATE_SIZE ){
        	DPR( dSTLEMBED, "Closure too big");
        	while(1) ;
        }
        memcpy( closureState_, (void *)&func, sizeof(func) );
        // callable_ = (ICallable *)closureState_;
        return *this;
    }
    
    ReturnValue operator()(Args... args) const {
        // assert(callable_);
        return ((ICallable *)closureState_)->Invoke(args...);
    }
    
    // Temporary hack
     uint8_t size_ = 0; 

private:
    class ICallable {
    public:
        virtual ~ICallable() = default;
        virtual ReturnValue Invoke(Args...) = 0;
    };
    
    template <typename T>
    class CallableT : public ICallable {
    public:
        CallableT(const T& t)
        : t_(t) {
        }
        
        ~CallableT() override = default;
        
        ReturnValue Invoke(Args... args) override {
            return t_(args...);
        }
        
    private:
        T t_;
    };

//     ICallable *callable_;
     uint8_t closureState_[CLOSURE_STATE_SIZE];

};

// int toCapture = 1;


// void func() {
//     toCapture++;
// }


// int jimmy() {
//     generic_function<void()> f;
//     f = func;
//     f();
    
//     f = [=]() {  toCapture++;  };
//     f();
    
//     return 0;
// }









#endif