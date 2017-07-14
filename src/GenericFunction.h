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


template <typename T>
class generic_function;

template <typename ReturnValue, typename... Args>
class generic_function<ReturnValue(Args...)> {
public:
    template <typename T>
    generic_function& operator=(T t) {
        callable_ = new CallableT<T>(t);
        return *this;
    }
    
    ReturnValue operator()(Args... args) const {
        //assert(callable_);
        return callable_->Invoke(args...);
    }
    
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
    
     ICallable *callable_;
};

int toCapture = 1;


void func() {
    toCapture++;
}


int jimmy() {
    generic_function<void()> f;
    f = func;
    f();
    
    f = [=]() {  toCapture++;  };
    f();
    
    return 0;
}









#endif