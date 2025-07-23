#include "UserInfoProcessor.h"
#include "globe.h"
#include "LogComm.h"
#include "ServiceDefine.h"
#include "util/tc_hash_fun.h"
#include "uuid.h"
#include "HallServer.h"
#include "util/tc_base64.h"
#include "CommonCode.pb.h"
#include "ServiceUtil.h"
#include "TimeUtil.h"
#include "tinyxml2.h"
#include <json/json.h>
#include <curl/curl.h>
#include "util/tc_md5.h"
#include "smtp-ssl.h"

#define RECOMMEND_LIMIT 1

//
using namespace std;
using namespace dataproxy;
using namespace dbagent;

static const int AUTH_CODE_VALIDITY_PERIOD = 300; //验证码有效期
static const int MIN_USERNAME_LEN = 1; //手机号最小长度

static std::map<string, dbagent::Eum_Col_Type> tbUserAccount = {
        {"uid", dbagent::BIGINT},
        {"username", dbagent::STRING},
        {"password", dbagent::STRING},
        {"reg_type", dbagent::INT},
        {"reg_time", dbagent::STRING},
        {"reg_ip", dbagent::STRING},
        {"reg_device_no", dbagent::STRING},
        {"device_id", dbagent::STRING},
        {"device_type", dbagent::STRING},
        {"platform", dbagent::INT},
        {"channel_id", dbagent::INT},
        {"area_id", dbagent::INT},
        {"agcid", dbagent::BIGINT},
        {"province_code", dbagent::INT},
        {"city_code", dbagent::STRING},
        {"realname", dbagent::STRING},
        {"idc_no", dbagent::STRING},
        {"idc_verify", dbagent::INT},
        {"is_forbidden", dbagent::INT},
        {"forbidden_time", dbagent::STRING},
        {"last_login_time", dbagent::STRING},
        {"last_logout_time", dbagent::STRING},
        {"last_login_ip", dbagent::STRING},
        {"last_login_address", dbagent::STRING},
        {"is_ban_chat", dbagent::INT}
};


static std::map<string, dbagent::Eum_Col_Type> tbUserInfo = {
        { "uid", dbagent::BIGINT },
        { "gender", dbagent::INT },
        { "nickname", dbagent::STRING },
        { "head_str", dbagent::STRING },
        { "signature", dbagent::STRING },
        { "reward_point", dbagent::BIGINT },
        { "entry_point", dbagent::BIGINT },
        { "video_point", dbagent::BIGINT },
        { "safe_pwd", dbagent::STRING },
        { "telephone", dbagent::STRING },
        { "level", dbagent::INT },
        { "experience", dbagent::INT }
};

static std::map<string, dbagent::Eum_Col_Type> tbUserAddress = {
        { "gid", dbagent::BIGINT },
        { "uid", dbagent::BIGINT },
        { "status", dbagent::INT },
        { "nickname", dbagent::STRING },
        { "telephone", dbagent::STRING },
        { "address", dbagent::STRING }
};

static std::map<string, dbagent::Eum_Col_Type> tbUserProps = {
        { "uid", dbagent::BIGINT },
        { "props_id", dbagent::BIGINT },
        { "props_type", dbagent::INT },
        { "get_channel", dbagent::STRING },
        { "get_time", dbagent::STRING },
        { "cost_time", dbagent::STRING },
        { "uuid", dbagent::STRING },
        { "state", dbagent::STRING }
};

/**
 *
*/
UserInfoProcessor::UserInfoProcessor()
{

}

/**
 *
*/
UserInfoProcessor::~UserInfoProcessor()
{

}

//初始化用户信息
tars::Int32 UserInfoProcessor::initUser(const userinfo::InitUserReq &req, userinfo::InitUserResp &resp)
{
    FUNC_ENTRY("");
    int iRet = 0;
    __TRY__

    LOG_DEBUG << "req: "<< printTars(req)<< endl;

    if (req.uid <= 0 || req.userName.length() <= 0)
    {
        ROLLLOG_ERROR << "param invalid, userinfo:: InitUserReqreq: " << printTars(req) << endl;
        resp.resultCode = -1;
        return -1;
    }

    auto pDBAgentServant = g_app.getOuterFactoryPtr()->getDBAgentServantPrx(req.uid);
    if (!pDBAgentServant)
    {
        ROLLLOG_ERROR << "pDBAgentServant is null" << endl;
        resp.resultCode = -2;
        return -2;
    }
    map<string, string> updateInfo = {
        {"uid", L2S(req.uid)},
        {"username", req.userName},
        {"password", req.passwd},
        {"reg_type", I2S(req.reg_type)},
        {"reg_time", ServiceUtil::CurTimeFormat()},
        {"device_id", req.deviceID},
        {"device_type", req.deviceType},
        {"platform", L2S(req.platform)},
        {"channel_id", I2S(req.channnelID)},
        {"area_id", I2S(req.areaID)},
        {"is_forbidden", "0"}
    };

    UserAccount useraccout;
    iRet = updateUserAccount(req.uid, updateInfo, useraccout, true);
    if(iRet != 0)
    {
        ROLLLOG_ERROR << "insert useraccount err. uid: "<< req.uid << endl;
        resp.resultCode = -3;
        return -3;
    }

    // userinfo
    string sNickname = "guest" + L2S(req.uid);
    string sHeadUrl = "touxiang_" + I2S(rand() % 30 + 1);
    string sGender = I2S(rand() % 2 + 1);

    updateInfo.clear();
    updateInfo = {
        { "uid", L2S(req.uid) },
        { "gender", sGender },
        { "nickname", sNickname },
        { "head_str", sHeadUrl},
        { "telephone", req.telephone}
    };
    UserInfo userinfo;
    iRet = updateUserInfo(req.uid, updateInfo, userinfo, true);
    if(iRet != 0)
    {
        ROLLLOG_ERROR << "insert userinfo err. uid: "<< req.uid << endl;
        resp.resultCode = -3;
        return -3;
    }
    __CATCH__
    FUNC_EXIT("", iRet);
    return iRet;
}

//USER_INFO = 21, //#tbl_user_info
//查询
int UserInfoProcessor::selectUserInfo(long uid, UserInfo &userinfo)
{
    int iRet = 0;
    auto pDBAgentServant = g_app.getOuterFactoryPtr()->getDBAgentServantPrx(uid);
    if (!pDBAgentServant)
    {
        ROLLLOG_ERROR << "pDBAgentServant is null" << endl;
        return -1;
    }

    TReadDataReq dataReq;
    dataReq.resetDefautlt();
    dataReq.keyName = I2S(E_REDIS_TYPE_HASH) + ":" + I2S(USER_INFO) + ":" + L2S(uid);
    dataReq.operateType = E_REDIS_READ;
    dataReq.clusterInfo.resetDefautlt();
    dataReq.clusterInfo.busiType = E_REDIS_PROPERTY;
    dataReq.clusterInfo.frageFactorType = 1;
    dataReq.clusterInfo.frageFactor = uid;

    vector<TField> fields;
    TField tfield;
    tfield.colArithType = E_NONE;
    for(auto item : tbUserInfo)
    {
        tfield.colName = item.first;
        fields.push_back(tfield);
    }
    dataReq.fields = fields;

    dataproxy::TReadDataRsp dataRsp;
    iRet = pDBAgentServant->redisRead(dataReq, dataRsp);
    if ((iRet != 0) || (dataRsp.iResult != 0))
    {
        ROLLLOG_ERROR << "load tb_userinfo cache failed, iRet: " << iRet << ", iResult: " << dataRsp.iResult << endl;
        return -2;
    }

    if(dataRsp.fields.size() == 0)
    {
        ROLLLOG_DEBUG << "uid not exist. uid:  "<< uid << endl;
        return XGameRetCode::GAME_USER_NOT_EXIST;
    }

    userinfo.uid = uid;
    for (auto it = dataRsp.fields.begin(); it != dataRsp.fields.end(); ++it)
    {
        for (auto itfield = it->begin(); itfield != it->end(); ++itfield)
        {
            if (itfield->colName == "nickname")
            {
                userinfo.nickname = itfield->colValue;
            }
            else if (itfield->colName == "head_str")
            {
                userinfo.head_str = itfield->colValue;
            }
            else if (itfield->colName == "signature")
            {
                userinfo.signature = itfield->colValue;
            }
            else if (itfield->colName == "gender")
            {
                userinfo.gender = S2I(itfield->colValue);
            }
            else if (itfield->colName == "reward_point")
            {
                userinfo.reward_point = S2L(itfield->colValue);
            }        
            else if (itfield->colName == "entry_point")
            {
                userinfo.entry_point = S2L(itfield->colValue);
            }
            else if (itfield->colName == "video_point")
            {
                userinfo.video_point = S2L(itfield->colValue);
            }
            else if (itfield->colName == "safe_pwd")
            {
                userinfo.safe_pwd = itfield->colValue;
            }
            else if (itfield->colName == "telephone")
            {
                userinfo.telephone = itfield->colValue;
            }
            else if (itfield->colName == "level")
            {
                userinfo.level = S2I(itfield->colValue);
            }
            else if (itfield->colName == "experience")
            {
                userinfo.experience = S2I(itfield->colValue);
            }
        }
    }

    return 0;
}

int UserInfoProcessor::updateUserInfo(long uid, const map<string, string> &updateInfo, UserInfo &mUserInfo, bool bInsert)
{
    int iRet = 0;
    if(!bInsert)
    {
        UserInfo userinfo;
        int iRet = selectUserInfo(uid, userinfo);
        if(iRet != 0)
        {
            ROLLLOG_ERROR << "seletc userinfo err."<< endl;
            return -1;
        }
    }

    dataproxy::TWriteDataReq wdataReq;
    wdataReq.resetDefautlt();
    wdataReq.keyName = I2S(E_REDIS_TYPE_HASH) + ":" + I2S(USER_INFO) + ":" + L2S(uid);
    wdataReq.operateType =  bInsert ? E_REDIS_INSERT : E_REDIS_WRITE;
    wdataReq.paraExt.resetDefautlt();
    wdataReq.paraExt.queryType = dbagent::E_UPDATE;
    wdataReq.clusterInfo.resetDefautlt();
    wdataReq.clusterInfo.busiType = E_REDIS_PROPERTY;
    wdataReq.clusterInfo.frageFactorType = E_FRAGE_FACTOR_USER_ID;
    wdataReq.clusterInfo.frageFactor = uid;

    vector<TField> fields;
    TField tfield;
    tfield.colArithType = E_NONE;
    for(auto item : tbUserInfo)
    {
        auto it = updateInfo.find(item.first);
        if(it == updateInfo.end())
        {
            continue;
        }
        tfield.colName = item.first;
        tfield.colType = item.second;
        tfield.colValue = it->second;
        fields.push_back(tfield);

    }
    wdataReq.fields = fields;

    TWriteDataRsp wdataRsp;
    iRet = g_app.getOuterFactoryPtr()->getDBAgentServantPrx(uid)->redisWrite(wdataReq, wdataRsp);
    if (iRet != 0 || wdataRsp.iResult != 0)
    {
        ROLLLOG_ERROR << "redisWrite failed, iRet: " << iRet << ", wdataRsp.iResult: " << wdataRsp.iResult << endl;
        return -2;
    }

    iRet = selectUserInfo(uid, mUserInfo);
    if(iRet != 0)
    {
        ROLLLOG_ERROR << "seletc userinfo err."<< endl;
        return -3;
    }

    return 0;
}

int UserInfoProcessor::updateUserWealth(long uid, const map<string, int> &updateInfo, long * curCount)
{
    int iRet = 0;
    UserInfo userinfo;
    iRet = selectUserInfo(uid, userinfo);
    if(iRet != 0)
    {
        ROLLLOG_ERROR << "seletc userinfo err."<< endl;
        return -1;
    }

    dataproxy::TWriteDataReq wdataReq;
    wdataReq.resetDefautlt();
    wdataReq.keyName = I2S(E_REDIS_TYPE_HASH) + ":" + I2S(USER_INFO) + ":" + L2S(uid);
    wdataReq.operateType = E_REDIS_WRITE;
    wdataReq.paraExt.resetDefautlt();
    wdataReq.paraExt.queryType = dbagent::E_UPDATE;
    wdataReq.clusterInfo.resetDefautlt();
    wdataReq.clusterInfo.busiType = E_REDIS_PROPERTY;
    wdataReq.clusterInfo.frageFactorType = E_FRAGE_FACTOR_USER_ID;
    wdataReq.clusterInfo.frageFactor = uid;

    vector<TField> fields;
    TField tfield;
    tfield.colArithType = E_NONE;
    for(auto item : tbUserInfo)
    {
        auto it = updateInfo.find(item.first);
        if(it == updateInfo.end())
        {
            continue;
        }
        if(item.first == "video_point" || item.first == "entry_point" || item.first == "reward_point")
        {
            *curCount = (item.first == "video_point" ? userinfo.video_point : (item.first == "entry_point" ? userinfo.entry_point : userinfo.reward_point)) + it->second;
        }

        if(*curCount < 0)
        {
            ROLLLOG_ERROR << "userinfo: " << printTars(userinfo) << ", colName: " << item.first << ", changeCount:"<< it->second << endl;
            return -2;
        }

        tfield.colName = item.first;
        tfield.colType = item.second;
        tfield.colValue = I2S(abs(it->second));
        tfield.colArithType = it->second > 0 ? E_ADD : E_SUB;
        fields.push_back(tfield);
    }
    wdataReq.fields = fields;

    TWriteDataRsp wdataRsp;
    iRet = g_app.getOuterFactoryPtr()->getDBAgentServantPrx(uid)->redisWrite(wdataReq, wdataRsp);
    if (iRet != 0 || wdataRsp.iResult != 0)
    {
        ROLLLOG_ERROR << "redisWrite failed, iRet: " << iRet << ", wdataRsp.iResult: " << wdataRsp.iResult << endl;
        return -3;
    }
    return 0;
}

int UserInfoProcessor::selectUserWealth(long uid, const string &colName)
{
    int iRet = 0;
    auto pDBAgentServant = g_app.getOuterFactoryPtr()->getDBAgentServantPrx(uid);
    if (!pDBAgentServant)
    {
        ROLLLOG_ERROR << "pDBAgentServant is null" << endl;
        return -1;
    }

    TReadDataReq dataReq;
    dataReq.resetDefautlt();
    dataReq.keyName = I2S(E_REDIS_TYPE_HASH) + ":" + I2S(USER_INFO) + ":" + L2S(uid);
    dataReq.operateType = E_REDIS_READ;
    dataReq.clusterInfo.resetDefautlt();
    dataReq.clusterInfo.busiType = E_REDIS_PROPERTY;
    dataReq.clusterInfo.frageFactorType = 1;
    dataReq.clusterInfo.frageFactor = uid;

    vector<TField> fields;
    TField tfield;
    tfield.colArithType = E_NONE;
    for(auto item : tbUserInfo)
    {
        tfield.colName = item.first;
        fields.push_back(tfield);
    }
    dataReq.fields = fields;

    dataproxy::TReadDataRsp dataRsp;
    iRet = pDBAgentServant->redisRead(dataReq, dataRsp);
    if ((iRet != 0) || (dataRsp.iResult != 0))
    {
        ROLLLOG_ERROR << "load tb_userinfo cache failed, iRet: " << iRet << ", iResult: " << dataRsp.iResult << endl;
        return -2;
    }

    if(dataRsp.fields.size() == 0)
    {
        ROLLLOG_DEBUG << "uid not exist. uid:  "<< uid << endl;
        return XGameRetCode::GAME_USER_NOT_EXIST;
    }

    for (auto it = dataRsp.fields.begin(); it != dataRsp.fields.end(); ++it)
    {
        for (auto itfield = it->begin(); itfield != it->end(); ++itfield)
        {
            if (itfield->colName == colName)
            {
                return S2I(itfield->colValue);
            }
        }
    }
    return 0;
}


//USER_ACCOUNT = 20,     //#tbl_user_account
//查询
int UserInfoProcessor::selectUserAccount(long uid, UserAccount &useraccount)
{
    int iRet = 0;
    auto pDBAgentServant = g_app.getOuterFactoryPtr()->getDBAgentServantPrx(uid);
    if (!pDBAgentServant)
    {
        ROLLLOG_ERROR << "pDBAgentServant is null" << endl;
        return -1;
    }

    TReadDataReq dataReq;
    dataReq.resetDefautlt();
    dataReq.keyName = I2S(E_REDIS_TYPE_HASH) + ":" + I2S(USER_ACCOUNT) + ":" + L2S(uid);
    dataReq.operateType = E_REDIS_READ;
    dataReq.clusterInfo.resetDefautlt();
    dataReq.clusterInfo.busiType = E_REDIS_PROPERTY;
    dataReq.clusterInfo.frageFactorType = E_FRAGE_FACTOR_STRING;
    dataReq.clusterInfo.frageFactor = tars::hash<string>()(L2S(uid));

    vector<TField> fields;
    TField tfield;
    tfield.colArithType = E_NONE;
    for(auto item : tbUserAccount)
    {
        tfield.colName = item.first;
        fields.push_back(tfield);
    }
    dataReq.fields = fields;

    dataproxy::TReadDataRsp dataRsp;
    iRet = pDBAgentServant->redisRead(dataReq, dataRsp);
    if ((iRet != 0) || (dataRsp.iResult != 0))
    {
        ROLLLOG_ERROR << "load tb_useraccount cache failed, iRet: " << iRet << ", iResult: " << dataRsp.iResult << endl;
        return -2;
    }
    
    useraccount.uid = uid;
    for (auto it = dataRsp.fields.begin(); it != dataRsp.fields.end(); ++it)
    {
        for (auto itfields = it->begin(); itfields != it->end(); ++itfields)
        {
            if (itfields->colName == "username")
            {
                useraccount.username = itfields->colValue;
            }
            else if (itfields->colName == "password")
            {
                useraccount.password = itfields->colValue;
            }
            else if (itfields->colName == "reg_type")
            {
                useraccount.reg_type = S2I(itfields->colValue);
            }
            else if (itfields->colName == "reg_time")
            {
                useraccount.reg_time = itfields->colValue;
            }
            else if (itfields->colName == "reg_ip")
            {
                useraccount.reg_ip = S2I(itfields->colValue);
            }
            else if (itfields->colName == "reg_device_no")
            {
                useraccount.reg_device_no = itfields->colValue;
            }
            else if (itfields->colName == "device_id")
            {
                useraccount.device_id = itfields->colValue;
            }
            else if (itfields->colName == "device_type")
            {
                useraccount.device_type = itfields->colValue;
            }
            else if (itfields->colName == "agcid")
            {
                useraccount.agcid = S2I(itfields->colValue);
            }
            else if (itfields->colName == "platform")
            {
                useraccount.platform = (userinfo::E_Platform_Type)S2I(itfields->colValue);
            }
            else if (itfields->colName == "channel_id")
            {
                useraccount.channel_id = (userinfo::E_Channel_ID)S2I(itfields->colValue);
            }
            else if (itfields->colName == "area_id")
            {
                useraccount.area_id = S2I(itfields->colValue);
            }
            else if (itfields->colName == "is_forbidden")
            {
                useraccount.is_forbidden = S2I(itfields->colValue);
            }
            else if (itfields->colName == "forbidden_time")
            {
                useraccount.forbidden_time = itfields->colValue;
            }
            else if (itfields->colName == "province_code")
            {
                useraccount.province_code = S2I(itfields->colValue);
            }
            else if (itfields->colName == "city_code")
            {
                useraccount.city_code = S2I(itfields->colValue);
            }
            else if (itfields->colName == "realname")
            {
                useraccount.realname = itfields->colValue;
            }
            else if (itfields->colName == "idc_no")
            {
                useraccount.idc_no = itfields->colValue;
            }
            else if (itfields->colName == "idc_verify")
            {
                useraccount.idc_verify = S2I(itfields->colValue);
            }
            else if (itfields->colName == "last_login_time")
            {
                useraccount.last_login_time = itfields->colValue;
            }
            else if (itfields->colName == "last_logout_time")
            {
                useraccount.last_logout_time = itfields->colValue;
            }
            else if (itfields->colName == "is_ban_chat")
            {
                useraccount.is_ban_chat = S2I(itfields->colValue);
            }
        }
    }
    return 0;
}

int UserInfoProcessor::updateUserAccount(long uid, const map<string, string> &updateInfo, UserAccount &mUserAccount, bool bInsert)
{
    int iRet = 0;
    if(!bInsert)
    {
        UserAccount useraccount;
        int iRet = selectUserAccount(uid, useraccount);
        if(iRet != 0)
        {
            ROLLLOG_ERROR << "select useraccount err."<< endl;
            return -1;
        }
    }

    dataproxy::TWriteDataReq wdataReq;
    wdataReq.resetDefautlt();
    wdataReq.keyName = I2S(E_REDIS_TYPE_HASH) + ":" + I2S(USER_ACCOUNT) + ":" + L2S(uid);
    wdataReq.operateType = bInsert ? E_REDIS_INSERT : E_REDIS_WRITE;
    wdataReq.paraExt.resetDefautlt();
    wdataReq.paraExt.queryType = dbagent::E_UPDATE;
    wdataReq.clusterInfo.resetDefautlt();
    wdataReq.clusterInfo.busiType = E_REDIS_PROPERTY;
    wdataReq.clusterInfo.frageFactorType = E_FRAGE_FACTOR_USER_ID;
    wdataReq.clusterInfo.frageFactor = uid;

    vector<TField> fields;
    TField tfield;
    tfield.colArithType = E_NONE;

    for(auto item : tbUserAccount)
    {
        auto it = updateInfo.find(item.first);
        if(it == updateInfo.end())
        {
            continue;
        }
        tfield.colName = item.first;
        tfield.colType = item.second;
        tfield.colValue = it->second;
        fields.push_back(tfield);

    }
    wdataReq.fields = fields;

    TWriteDataRsp wdataRsp;
    iRet = g_app.getOuterFactoryPtr()->getDBAgentServantPrx(uid)->redisWrite(wdataReq, wdataRsp);
    if (iRet != 0 || wdataRsp.iResult != 0)
    {
        ROLLLOG_ERROR << "redisWrite failed, iRet: " << iRet << ", wdataRsp.iResult: " << wdataRsp.iResult << endl;
        return -2;
    }

    iRet = selectUserAccount(uid, mUserAccount);
    if(iRet != 0)
    {
        ROLLLOG_ERROR << "select useraccount err."<< endl;
        return -3;
    }
    return 0;
}

int UserInfoProcessor::selectUserAddress(long uid, vector<UserAddress> &vAddress)
{
    int iRet = 0;
    auto pDBAgentServant = g_app.getOuterFactoryPtr()->getDBAgentServantPrx(uid);
    if (!pDBAgentServant)
    {
        ROLLLOG_ERROR << "pDBAgentServant is null" << endl;
        return -1;
    }

    TReadDataReq dataReq;
    dataReq.resetDefautlt();
    dataReq.keyName = I2S(E_REDIS_TYPE_HASH) + ":" + I2S(USER_ADDRESS) + ":" + L2S(uid);
    dataReq.operateType = E_REDIS_READ;
    dataReq.clusterInfo.resetDefautlt();
    dataReq.clusterInfo.busiType = E_REDIS_PROPERTY;
    dataReq.clusterInfo.frageFactorType = 1;
    dataReq.clusterInfo.frageFactor = uid;

    vector<TField> fields;
    TField tfield;
    tfield.colArithType = E_NONE;
    for(auto item : tbUserAddress)
    {
        tfield.colName = item.first;
        fields.push_back(tfield);
    }
    dataReq.fields = fields;

    dataproxy::TReadDataRsp dataRsp;
    iRet = pDBAgentServant->redisRead(dataReq, dataRsp);
    if ((iRet != 0) || (dataRsp.iResult != 0))
    {
        ROLLLOG_ERROR << "load tb_usera_ddress cache failed, iRet: " << iRet << ", iResult: " << dataRsp.iResult << endl;
        return -2;
    }

    UserAddress userAddress;
    userAddress.uid = uid;
    for (auto it = dataRsp.fields.begin(); it != dataRsp.fields.end(); ++it)
    {
        for (auto itfield = it->begin(); itfield != it->end(); ++itfield)
        {
            if (itfield->colName == "gid")
            {
                userAddress.gid = S2L(itfield->colValue);
            }
            else if (itfield->colName == "nickname")
            {
                userAddress.nickname = itfield->colValue;
            }
            else if (itfield->colName == "telephone")
            {
                userAddress.telephone = itfield->colValue;
            }
            else if (itfield->colName == "address")
            {
                userAddress.address = itfield->colValue;
            }
            else if (itfield->colName == "status")
            {
                userAddress.status = S2I(itfield->colValue);
            }
        }
        vAddress.push_back(userAddress);
    }

    return 0;
}

int UserInfoProcessor::resetUserAddress(long uid)
{
    int iRet = 0;
    
    dataproxy::TWriteDataReq wdataReq;
    wdataReq.resetDefautlt();
    wdataReq.keyName = I2S(E_REDIS_TYPE_HASH) + ":" + I2S(USER_ADDRESS) + ":" + L2S(uid);
    wdataReq.operateType = E_REDIS_WRITE;
    wdataReq.paraExt.resetDefautlt();
    wdataReq.paraExt.queryType = dbagent::E_UPDATE;
    wdataReq.clusterInfo.resetDefautlt();
    wdataReq.clusterInfo.busiType = E_REDIS_PROPERTY;
    wdataReq.clusterInfo.frageFactorType = E_FRAGE_FACTOR_USER_ID;
    wdataReq.clusterInfo.frageFactor = uid;

    vector<TField> fields;
    TField tfield;
    tfield.colArithType = E_NONE;
    tfield.colName = "status";
    tfield.colType = INT;
    tfield.colValue = I2S(0);
    fields.push_back(tfield);

    wdataReq.fields = fields;

    TWriteDataRsp wdataRsp;
    iRet = g_app.getOuterFactoryPtr()->getDBAgentServantPrx(uid)->redisWrite(wdataReq, wdataRsp);
    if (iRet != 0 || wdataRsp.iResult != 0)
    {
        ROLLLOG_ERROR << "redisWrite failed, iRet: " << iRet << ", wdataRsp.iResult: " << wdataRsp.iResult << endl;
        return -2;
    }
    
    return iRet;
}

int UserInfoProcessor::updateUserAddress(long gid, const map<string, string> &updateAddress, bool bInsert)
{
    int iRet = 0;
    
    dataproxy::TWriteDataReq wdataReq;
    wdataReq.resetDefautlt();
    wdataReq.keyName = I2S(E_REDIS_TYPE_HASH) + ":" + I2S(USER_ADDRESS_GID) + ":" + L2S(gid);
    wdataReq.operateType = bInsert ? E_REDIS_INSERT : E_REDIS_WRITE;
    wdataReq.paraExt.resetDefautlt();
    wdataReq.paraExt.queryType = dbagent::E_UPDATE;
    wdataReq.clusterInfo.resetDefautlt();
    wdataReq.clusterInfo.busiType = E_REDIS_PROPERTY;
    wdataReq.clusterInfo.frageFactorType = E_FRAGE_FACTOR_USER_ID;
    wdataReq.clusterInfo.frageFactor = gid;

    vector<TField> fields;
    TField tfield;
    tfield.colArithType = E_NONE;

    for(auto item : tbUserAddress)
    {
        auto it = updateAddress.find(item.first);
        if(it == updateAddress.end())
        {
            continue;
        }
        tfield.colName = item.first;
        tfield.colType = item.second;
        tfield.colValue = it->second;
        fields.push_back(tfield);

    }
    wdataReq.fields = fields;

    TWriteDataRsp wdataRsp;
    iRet = g_app.getOuterFactoryPtr()->getDBAgentServantPrx(gid)->redisWrite(wdataReq, wdataRsp);
    if (iRet != 0 || wdataRsp.iResult != 0)
    {
        ROLLLOG_ERROR << "redisWrite failed, iRet: " << iRet << ", wdataRsp.iResult: " << wdataRsp.iResult << endl;
        return -2;
    }
    
    return iRet;
}

int UserInfoProcessor::deleteUserAddress(long gid)
{
    int iRet = 0;
    
    dataproxy::TWriteDataReq wdataReq;
    wdataReq.resetDefautlt();
    wdataReq.keyName = I2S(E_REDIS_TYPE_HASH) + ":" + I2S(USER_ADDRESS_GID) + ":" + L2S(gid);
    wdataReq.operateType = E_REDIS_WRITE;
    wdataReq.paraExt.resetDefautlt();
    wdataReq.paraExt.queryType = dbagent::E_DELETE;
    wdataReq.clusterInfo.resetDefautlt();
    wdataReq.clusterInfo.busiType = E_REDIS_PROPERTY;
    wdataReq.clusterInfo.frageFactorType = E_FRAGE_FACTOR_USER_ID;
    wdataReq.clusterInfo.frageFactor = gid;

    TWriteDataRsp wdataRsp;
    iRet = g_app.getOuterFactoryPtr()->getDBAgentServantPrx(gid)->redisWrite(wdataReq, wdataRsp);
    if (iRet != 0 || wdataRsp.iResult != 0)
    {
        ROLLLOG_ERROR << "redisWrite failed, iRet: " << iRet << ", wdataRsp.iResult: " << wdataRsp.iResult << endl;
        return -2;
    }
    
    return iRet;
}

int UserInfoProcessor::selectUserProps(long uid, vector<userinfo::PropsInfo> &vecPropsInfo)
{
    FUNC_ENTRY("");
    int iRet = 0;

    dataproxy::TReadDataReq dataReq;
    dataReq.resetDefautlt();
    dataReq.keyName = I2S(E_REDIS_TYPE_LIST) + ":" + I2S(USER_PROPS_INFO) + ":" + L2S(uid);
    dataReq.operateType = E_REDIS_READ;
    dataReq.clusterInfo.resetDefautlt();
    dataReq.clusterInfo.busiType = E_REDIS_PROPERTY;
    dataReq.clusterInfo.frageFactorType = E_FRAGE_FACTOR_USER_ID;
    dataReq.clusterInfo.frageFactor = uid;
    dataReq.paraExt.resetDefautlt();
    dataReq.paraExt.subOperateType = E_REDIS_LIST_RANGE;//根据范围取数据
    dataReq.paraExt.start = 0;//起始下标从0开始
    dataReq.paraExt.end = -1;//终止最大结束下标为-1

    vector<TField> fields;
    TField tfield;
    tfield.colArithType = E_NONE;
    for(auto colItem : tbUserProps)
    {
        tfield.colName = colItem.first;
        tfield.colType = colItem.second;
        fields.push_back(tfield);
    }

    //过滤条件
    //dataReq.paraExt.fields.insert(std::make_pair("state", tfield));

    TReadDataRsp dataRsp;
    iRet = g_app.getOuterFactoryPtr()->getDBAgentServantPrx(uid)->redisRead(dataReq, dataRsp);
    if (iRet != 0 || dataRsp.iResult != 0)
    {
        ROLLLOG_ERROR << "get propsinfo err, iRet: " << iRet << ", iResult: " << dataRsp.iResult << endl;
        return -1;
    }

    LOG_DEBUG << "dataRsp.fields.size:"<< dataRsp.fields.size()<<endl;
    for (auto it = dataRsp.fields.begin(); it != dataRsp.fields.end(); ++it)
    {
        userinfo::PropsInfo propsInfo;
        for (auto itTField = it->begin(); itTField != it->end(); ++itTField)
        {
            if(itTField->colName == "uuid")
            {
                propsInfo.uuid = itTField->colValue;
            }
            else if(itTField->colName == "props_id")
            {
                propsInfo.propsID = S2I(itTField->colValue);
            }
            else if(itTField->colName == "props_type")
            {
                propsInfo.propsType = S2I(itTField->colValue);
            }
            else if(itTField->colName == "get_channel")
            {
                propsInfo.getChannel = itTField->colValue;
            }
            else if(itTField->colName == "get_time")
            {
                propsInfo.getTime = ServiceUtil::GetTimeStamp(itTField->colValue);
            }
            else if(itTField->colName == "cost_time")
            {
                propsInfo.costTime = ServiceUtil::GetTimeStamp(itTField->colValue);
            }
            else if(itTField->colName == "state")
            {
                propsInfo.iState = S2I(itTField->colValue);
            }
        }

        config::PropsInfoCfg propsCfg;
        iRet = g_app.getOuterFactoryPtr()->getPropsInfoConfig(propsInfo.propsID, propsCfg);
        if(iRet != 0 || (!propsCfg.propsTime.empty() && TNOW > ServiceUtil::GetTimeStamp(propsCfg.propsTime)))
        {
            LOG_DEBUG << "props time expired. porpsID:"<< propsInfo.propsID << endl;
            continue;
        }

        vecPropsInfo.push_back(propsInfo);
    }

    FUNC_EXIT("", iRet);
    return 0;
}

int UserInfoProcessor::selectUserPropsByUUID(long uid, const string& uuid, userinfo::PropsInfo &propsInfo)
{
    FUNC_ENTRY("");
    int iRet = 0;
    __TRY__

    vector<userinfo::PropsInfo> vecPropsInfo;
    iRet = selectUserProps(uid, vecPropsInfo);
    if(iRet != 0)
    {
        LOG_DEBUG << "select user props err. iRet:"<< iRet << ", uid:"<< uid << endl;
        return iRet;
    }

    auto it = std::find_if(vecPropsInfo.begin(), vecPropsInfo.end(), [uuid](const userinfo::PropsInfo &propsItem)->bool{
        return propsItem.uuid == uuid;
    });
    if(it == vecPropsInfo.end())
    {
        LOG_DEBUG << "not find props by uuid, uuid:"<< uuid << endl;
        return -1;
    }
    propsInfo.uuid = it->uuid;
    propsInfo.propsID = it->propsID;
    propsInfo.propsType = it->propsType;
    propsInfo.getChannel = it->getChannel;
    propsInfo.getTime = it->getTime;
    propsInfo.costTime = it->costTime;
    propsInfo.iState = it->iState;

    __CATCH__
    FUNC_EXIT("", iRet);
    return iRet;
}

int UserInfoProcessor::selectUserPropsByID(long uid, const int propsId, vector<userinfo::PropsInfo> &vecPropsInfo)
{
    FUNC_ENTRY("");
    int iRet = 0;
    __TRY__

    vector<userinfo::PropsInfo> vecAllPropsInfo;
    iRet = selectUserProps(uid, vecAllPropsInfo);
    if(iRet != 0)
    {
        LOG_DEBUG << "select user props err. iRet:"<< iRet << ", uid:"<< uid << endl;
        return iRet;
    }

    LOG_DEBUG << "size:"<< vecAllPropsInfo.size() << endl;
    for(auto item : vecAllPropsInfo)
    {
        if(item.iState != 0)
        {
            continue;
        }
        vecPropsInfo.push_back(item);
    }

    __CATCH__
    FUNC_EXIT("", iRet);
    return iRet;
}


int UserInfoProcessor::insertUserProps(long uid, const userinfo::PropsInfo &propsInfo)
{
    FUNC_ENTRY("");
    int iRet = 0;
    __TRY__

    userinfo::PropsInfo propsItem;
    iRet = selectUserPropsByUUID(uid, propsInfo.uuid, propsItem);
    if(iRet == 0)
    {
        LOG_ERROR << "props exist. iRet:"<< iRet << ", uuid:"<< propsInfo.uuid << ", uid:"<< uid << endl;
        return iRet;
    }

    dataproxy::TWriteDataReq dataReq;
    dataReq.resetDefautlt();
    dataReq.keyName = I2S(E_REDIS_TYPE_LIST) + ":" + I2S(USER_PROPS_INFO) + ":" + L2S(uid);
    dataReq.operateType = E_REDIS_INSERT;
    dataReq.clusterInfo.resetDefautlt();
    dataReq.clusterInfo.busiType = E_REDIS_PROPERTY;
    dataReq.clusterInfo.frageFactorType = E_FRAGE_FACTOR_USER_ID;
    dataReq.clusterInfo.frageFactor = uid;

    vector<TField> fields;
    TField tfield;
    tfield.colArithType = E_NONE;

    for(auto colItem : tbUserProps)
    {
        tfield.colName = colItem.first;
        tfield.colType = colItem.second;
        if(tfield.colName == "uid")
        {
            tfield.colValue = L2S(uid);
        }
        if(tfield.colName == "uuid")
        {
            tfield.colValue = propsInfo.uuid;
        }
        else if(tfield.colName == "props_id")
        {
            tfield.colValue = I2S(propsInfo.propsID);
        }
        else if(tfield.colName == "props_type")
        {
            tfield.colValue = I2S(propsInfo.propsType);
        }
        else if(tfield.colName == "get_channel")
        {
            tfield.colValue = propsInfo.getChannel;
        }
        else if(tfield.colName == "get_time")
        {
            tfield.colValue = ServiceUtil::GetTimeFormat(propsInfo.getTime);
        }
        else if(tfield.colName == "cost_time")
        {
            tfield.colValue = ServiceUtil::GetTimeFormat(propsInfo.costTime);
        }
        else if(tfield.colName == "state")
        {
            tfield.colValue = I2S(propsInfo.iState);
        }
        fields.push_back(tfield);
    }
    dataReq.fields = fields;

    dataproxy::TWriteDataRsp dataRsp;
    iRet = g_app.getOuterFactoryPtr()->getDBAgentServantPrx(uid)->redisWrite(dataReq, dataRsp);
    if (iRet != 0 || dataRsp.iResult != 0)
    {
        ROLLLOG_ERROR << "updateUserProps err, iRet: " << iRet << ", iResult: " << dataRsp.iResult << endl;
        return -2;
    }

    __CATCH__
    FUNC_EXIT("", iRet);
    return iRet;
}

int UserInfoProcessor::updateUserPropsByUUID(long uid, const string& uuid, const map<string, string> &updateInfo)
{
    FUNC_ENTRY("");
    int iRet = 0;
    __TRY__

    userinfo::PropsInfo propsItem;
    iRet = selectUserPropsByUUID(uid, uuid, propsItem);
    if(iRet != 0 || propsItem.iState != 0)
    {
        LOG_ERROR << "props not exist. iRet:"<< iRet << ", uuid:"<< uuid << ", uid:"<< uid << endl;
        return iRet;
    }

    dataproxy::TWriteDataReq wdataReq;
    wdataReq.resetDefautlt();
    wdataReq.keyName = I2S(E_REDIS_TYPE_LIST) + ":" + I2S(USER_PROPS_INFO) + ":" + L2S(uid);
    wdataReq.operateType =  E_REDIS_WRITE;
    wdataReq.paraExt.resetDefautlt();
    wdataReq.paraExt.queryType = dbagent::E_UPDATE;
    wdataReq.clusterInfo.resetDefautlt();
    wdataReq.clusterInfo.busiType = E_REDIS_PROPERTY;
    wdataReq.clusterInfo.frageFactorType = E_FRAGE_FACTOR_USER_ID;
    wdataReq.clusterInfo.frageFactor = uid;

    vector<TField> fields;
    TField tfield;
    tfield.colArithType = E_NONE;
    for(auto item : tbUserProps)
    {
        auto it = updateInfo.find(item.first);
        if(it == updateInfo.end())
        {
            continue;
        }
        tfield.colName = item.first;
        tfield.colType = item.second;
        tfield.colValue = it->second;
        fields.push_back(tfield);
    }
    wdataReq.fields = fields;

    tfield.colName = "uuid";
    tfield.colType = STRING;
    tfield.colValue = uuid;
    wdataReq.paraExt.fields.insert(std::make_pair("uuid", tfield));

    TWriteDataRsp wdataRsp;
    iRet = g_app.getOuterFactoryPtr()->getDBAgentServantPrx(uid)->redisWrite(wdataReq, wdataRsp);
    if (iRet != 0 || wdataRsp.iResult != 0)
    {
        ROLLLOG_ERROR << "redisWrite failed, iRet: " << iRet << ", wdataRsp.iResult: " << wdataRsp.iResult << endl;
        return -2;
    }

    __CATCH__
    FUNC_EXIT("", iRet);
    return iRet;
}

int UserInfoProcessor::updateUserPropsByID(long uid, const int propsId, const int count, const map<string, string> &updateInfo)
{
    FUNC_ENTRY("");
    int iRet = 0;
    __TRY__

    vector<userinfo::PropsInfo> vecPropsItem;
    iRet = selectUserPropsByID(uid, propsId, vecPropsItem);
    if(iRet != 0)
    {
        LOG_ERROR << "props not exist. iRet:"<< iRet << ", propsId:"<< propsId << ", uid:"<< uid << endl;
        return iRet;
    }

    if(int(vecPropsItem.size()) < count)
    {
        LOG_ERROR << "props lack. uid:"<< uid<< ", propsId:"<< propsId << ", count:"<< count << ", size:"<< vecPropsItem.size()<< endl;
        return -1;
    }

    int loop_count = count;
    for(auto propsItem : vecPropsItem)
    {
        if(loop_count <= 0)
        {
            break;
        }
        LOG_DEBUG << "uuid:"<< propsItem.uuid << endl;

        if(updateUserPropsByUUID(uid, propsItem.uuid, updateInfo) != 0)
        {
            continue;
        }
        loop_count--;
    }

    __CATCH__
    FUNC_EXIT("", iRet);
    return iRet;
}

int UserInfoProcessor::getPropsCountById(long uid, int propsId)
{
    int count = 0;
    vector<userinfo::PropsInfo> vecPropsInfo;
    int iRet = selectUserProps(uid, vecPropsInfo);
    if(iRet != 0)
    {
        LOG_DEBUG << "select user props err. iRet:"<< iRet << ", uid:"<< uid << endl;
        return count;
    }

    for(auto propsItem : vecPropsInfo)
    {
        if(propsItem.propsID != propsId || propsItem.iState != 0)
        {
            continue;
        }
        count++;
    }
    return count;
}

//玩家备注
int UserInfoProcessor::updateUserRemark(long uid, long remark_uid, const string& content)
{
    FUNC_ENTRY("");
    int iRet = 0;
    __TRY__

    if(ServiceUtil::getWordCount(content) > REMARK_WORD_COUNT)
    {
        return XGameRetCode::USER_INFO_REMARK_LENGTH_LIMIT;
    }

    dataproxy::TWriteDataReq dataReq;
    dataReq.resetDefautlt();
    dataReq.keyName = I2S(E_REDIS_TYPE_LIST) + ":" + I2S(USER_REMARK) + ":" + L2S(uid);
    dataReq.operateType = E_REDIS_WRITE;
    dataReq.clusterInfo.resetDefautlt();
    dataReq.clusterInfo.busiType = E_REDIS_PROPERTY;
    dataReq.clusterInfo.frageFactorType = E_FRAGE_FACTOR_USER_ID;
    dataReq.clusterInfo.frageFactor = uid;
    dataReq.paraExt.resetDefautlt();
    dataReq.paraExt.queryType = E_REPLACE;

    vector<TField> fields;
    TField tfield;
    tfield.colArithType = E_NONE;

    tfield.colName = "uid";
    tfield.colType = BIGINT;
    tfield.colValue = L2S(uid);
    fields.push_back(tfield);

    tfield.colName = "remark_uid";
    tfield.colType = BIGINT;
    tfield.colValue = I2S(remark_uid);
    fields.push_back(tfield);

    tfield.colName = "content";
    tfield.colType = STRING;
    tfield.colValue = content;
    fields.push_back(tfield);

    tfield.colName = "log_time";
    tfield.colType = STRING;
    tfield.colValue = ServiceUtil::GetTimeFormat(TNOW);
    fields.push_back(tfield);

    dataReq.fields = fields;

    dataproxy::TWriteDataRsp dataRsp;
    iRet = g_app.getOuterFactoryPtr()->getDBAgentServantPrx(uid)->redisWrite(dataReq, dataRsp);
    if (iRet != 0 || dataRsp.iResult != 0)
    {
        ROLLLOG_ERROR << "updateUserRewark err, iRet: " << iRet << ", iResult: " << dataRsp.iResult << endl;
        return -2;
    }

    ROLLLOG_DEBUG << "updateUserRewark, iRet: " << iRet << ", dataRsp: " << printTars(dataRsp) << endl;

    __CATCH__
    FUNC_EXIT("", iRet);
    return iRet;
}

//
string UserInfoProcessor::selectUserRemark(long uid, long remark_uid)
{
    FUNC_ENTRY("");
    int iRet = 0;

    dataproxy::TReadDataReq dataReq;
    dataReq.resetDefautlt();
    dataReq.keyName = I2S(E_REDIS_TYPE_LIST) + ":" + I2S(USER_REMARK) + ":" + L2S(uid);
    dataReq.operateType = E_REDIS_READ;
    dataReq.clusterInfo.resetDefautlt();
    dataReq.clusterInfo.busiType = E_REDIS_PROPERTY;
    dataReq.clusterInfo.frageFactorType = E_FRAGE_FACTOR_USER_ID;
    dataReq.clusterInfo.frageFactor = uid;
    dataReq.paraExt.resetDefautlt();
    dataReq.paraExt.subOperateType = E_REDIS_LIST_RANGE;//根据范围取数据
    dataReq.paraExt.start = 0;//起始下标从0开始
    dataReq.paraExt.end = -1;//终止最大结束下标为-1

    vector<TField> fields;
    TField tfield;
    tfield.colArithType = E_NONE;
    tfield.colName = "uid";
    tfield.colType = BIGINT;
    fields.push_back(tfield);
    tfield.colName = "remark_uid";
    tfield.colType = BIGINT;
    fields.push_back(tfield);
    tfield.colName = "content";
    tfield.colType = STRING;
    fields.push_back(tfield);

    TReadDataRsp dataRsp;
    iRet = g_app.getOuterFactoryPtr()->getDBAgentServantPrx(uid)->redisRead(dataReq, dataRsp);
    if (iRet != 0 || dataRsp.iResult != 0)
    {
        ROLLLOG_ERROR << "get propsinfo err, iRet: " << iRet << ", iResult: " << dataRsp.iResult << endl;
        return "";
    }

    LOG_DEBUG << "dataRsp.fields.size:"<< dataRsp.fields.size()<<endl;
    for (auto it = dataRsp.fields.begin(); it != dataRsp.fields.end(); ++it)
    {
        long remarkUid = 0;
        string content = "";
        for (auto itTField = it->begin(); itTField != it->end(); ++itTField)
        {
            if(itTField->colName == "content")
            {
                content = itTField->colValue;
            }
            if(itTField->colName == "remark_uid")
            {
                remarkUid = S2L(itTField->colValue);
            }
        }
        if(remarkUid == remark_uid)
        {
            return content;
        }
    }
    FUNC_EXIT("", iRet);
    return "";
}

int UserInfoProcessor::listUserRemark(long uid, vector<userinfo::UserRemarkInfo> &vecRemarkInfo)
{
    FUNC_ENTRY("");
    int iRet = 0;

    dataproxy::TReadDataReq dataReq;
    dataReq.resetDefautlt();
    dataReq.keyName = I2S(E_REDIS_TYPE_LIST) + ":" + I2S(USER_REMARK) + ":" + L2S(uid);
    dataReq.operateType = E_REDIS_READ;
    dataReq.clusterInfo.resetDefautlt();
    dataReq.clusterInfo.busiType = E_REDIS_PROPERTY;
    dataReq.clusterInfo.frageFactorType = E_FRAGE_FACTOR_USER_ID;
    dataReq.clusterInfo.frageFactor = uid;
    dataReq.paraExt.resetDefautlt();
    dataReq.paraExt.subOperateType = E_REDIS_LIST_RANGE;//根据范围取数据
    dataReq.paraExt.start = 0;//起始下标从0开始
    dataReq.paraExt.end = -1;//终止最大结束下标为-1

    vector<TField> fields;
    TField tfield;
    tfield.colArithType = E_NONE;
    tfield.colName = "uid";
    tfield.colType = BIGINT;
    fields.push_back(tfield);
    tfield.colName = "remark_uid";
    tfield.colType = BIGINT;
    fields.push_back(tfield);
    tfield.colName = "content";
    tfield.colType = STRING;
    fields.push_back(tfield);

    TReadDataRsp dataRsp;
    iRet = g_app.getOuterFactoryPtr()->getDBAgentServantPrx(uid)->redisRead(dataReq, dataRsp);
    if (iRet != 0 || dataRsp.iResult != 0)
    {
        ROLLLOG_ERROR << "get propsinfo err, iRet: " << iRet << ", iResult: " << dataRsp.iResult << endl;
        return -1;
    }

    LOG_DEBUG << "dataRsp.fields.size:"<< dataRsp.fields.size()<<endl;
    for (auto it = dataRsp.fields.begin(); it != dataRsp.fields.end(); ++it)
    {
        userinfo::UserRemarkInfo info;
        for (auto itTField = it->begin(); itTField != it->end(); ++itTField)
        {
            if(itTField->colName == "remark_uid")
            {
                info.uid = S2L(itTField->colValue);
            }
            else if(itTField->colName == "content")
            {
                info.content = itTField->colValue;
            }
        }

        vecRemarkInfo.push_back(info);
    }

    FUNC_EXIT("", iRet);
    return 0;
}
