#ifndef _HallServer_H_
#define _HallServer_H_

#include <iostream>
#include "servant/Application.h"
#include "OuterFactoryImp.h"

///
using namespace tars;

/**
 *
 **/
class HallServer : public Application
{
public:
    /**
     *
     **/
    virtual ~HallServer() {};

    /**
     *
     **/
    virtual void initialize();

    /**
     *
     **/
    virtual void destroyApp();

public:
    /*
    * 配置变更，重新加载配置
    */
    bool reloadSvrConfig(const string &command, const string &params, string &result);

public:
    /**
    * 初始化外部接口对象
    **/
    int initOuterFactory();

    /**
    * 取外部接口对象
    **/
    OuterFactoryImpPtr getOuterFactoryPtr()
    {
        return _pOuter;
    }

private:
    OuterFactoryImpPtr _pOuter;  //外部接口对象
};

///
extern HallServer g_app;

////////////////////////////////////////////
#endif
