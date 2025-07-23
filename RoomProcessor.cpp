#include "RoomProcessor.h"
#include "globe.h"
#include "LogComm.h"
#include "ServiceDefine.h"
#include "util/tc_hash_fun.h"
#include "uuid.h"
#include "HallServer.h"
#include "util/tc_base64.h"
#include "ServiceUtil.h"


//
using namespace std;
using namespace dataproxy;
using namespace dbagent;

static std::map<string, dbagent::Eum_Col_Type> tbMatchRoomInfo = {
        { "room_index", dbagent::STRING },
        { "uper_room_index", dbagent::STRING },
        { "icon", dbagent::STRING },
        { "room_key", dbagent::STRING },
        { "room_id", dbagent::STRING },
        { "room_name", dbagent::STRING },
        { "room_desc", dbagent::STRING },
        { "begin_time", dbagent::STRING },
        { "cur_player_count", dbagent::INT },
        { "max_player_count", dbagent::INT },
        { "min_player_count", dbagent::INT },
        { "reward_pool", dbagent::STRING },
        { "seat_num", dbagent::INT },
        { "match_type", dbagent::INT },
        { "game_type", dbagent::INT },
        { "table_type", dbagent::INT },
        { "online_game", dbagent::INT },
        { "end_signup", dbagent::STRING },
        { "end_rebuy", dbagent::STRING },
        { "rebuy_count", dbagent::INT },
        { "add_on", dbagent::STRING },
        { "base_score", dbagent::INT },
        { "signup_fee", dbagent::STRING },
        { "match_addr", dbagent::STRING },
        { "game_status", dbagent::INT },
        { "reward_info", dbagent::STRING },
        { "create_time", dbagent::STRING },
        { "think_time", dbagent::INT },
        { "reward_conf", dbagent::STRING },
        { "blind_conf", dbagent::STRING },
        { "begin_signup_time", dbagent::STRING },
        { "end_signup_time", dbagent::STRING },
        { "tag", dbagent::INT },
        { "status", dbagent::INT },
        { "cycle", dbagent::INT }
};

static std::map<string, dbagent::Eum_Col_Type> tbMatchRoomMember = {
    { "room_index", dbagent::STRING },
    { "uid", dbagent::BIGINT },
    { "member_type", dbagent::INT },
    { "score", dbagent::BIGINT },
    { "chair_id", dbagent::INT },
    { "table_id", dbagent::INT },
    { "rank", dbagent::INT },
    { "reward_info", dbagent::STRING },
    { "update_time", dbagent::STRING }
};

/**
 *
*/
RoomProcessor::RoomProcessor()
{

}

/**
 *
*/
RoomProcessor::~RoomProcessor()
{

}

//
int RoomProcessor::selectMatchRoom(const string& roomIndex, room::MatchRoomInfo& roomInfo)
{
    TReadDataReq dataReq;
    dataReq.resetDefautlt();
    dataReq.keyName = I2S(E_REDIS_TYPE_HASH) + ":" + I2S(MATCH_ROOM_LIST) + ":" + roomIndex;
    dataReq.operateType = E_REDIS_READ;
    dataReq.clusterInfo.resetDefautlt();
    dataReq.clusterInfo.busiType = E_REDIS_PROPERTY;
    dataReq.clusterInfo.frageFactorType = E_FRAGE_FACTOR_STRING;
    dataReq.clusterInfo.frageFactor = tars::hash<string>()(roomIndex);

    vector<TField> fields;
    TField tfield;
    tfield.colArithType = E_NONE;
    for(auto item : tbMatchRoomInfo)
    {
        tfield.colName = item.first;
        fields.push_back(tfield);
    }
    dataReq.fields = fields;

    dataproxy::TReadDataRsp dataRsp;
    int iRet = g_app.getOuterFactoryPtr()->getDBAgentServantPrx(roomIndex)->redisRead(dataReq, dataRsp);
    if ((iRet != 0) || (dataRsp.iResult != 0))
    {
        ROLLLOG_ERROR << "load tb_useraccount cache failed, iRet: " << iRet << ", iResult: " << dataRsp.iResult << endl;
        return -2;
    }

    for (auto it = dataRsp.fields.begin(); it != dataRsp.fields.end(); ++it)
    {
        for (auto itfields = it->begin(); itfields != it->end(); ++itfields)
        {
            if (itfields->colName == "room_index")
            {
                roomInfo.roomIndex = itfields->colValue;
            }
            else if (itfields->colName == "icon")
            {
                roomInfo.icon = itfields->colValue;
            }
            else if (itfields->colName == "room_key")
            {
                roomInfo.roomKey = itfields->colValue;
            }
            else if (itfields->colName == "room_name")
            {
                roomInfo.roomName = itfields->colValue;
            }
            else if (itfields->colName == "room_desc")
            {
                roomInfo.roomDesc = itfields->colValue;
            }
            else if (itfields->colName == "game_type")
            {
                roomInfo.gameType = S2I(itfields->colValue);
            }
            else if (itfields->colName == "match_type")
            {
                roomInfo.matchType = S2I(itfields->colValue);
            }
            else if (itfields->colName == "table_type")
            {
                roomInfo.tableType = S2I(itfields->colValue);
            }
            else if (itfields->colName == "online_game")
            {
                roomInfo.onlineGame = S2I(itfields->colValue);
            }
            else if (itfields->colName == "room_id")
            {
                roomInfo.roomId = itfields->colValue;
            }
            else if (itfields->colName == "seat_num")
            {
                roomInfo.seatCount = S2I(itfields->colValue);
            }
            else if (itfields->colName == "end_signup")
            {
                roomInfo.bDelaySignUp = itfields->colValue.empty() ? true : false;
            }
            else if (itfields->colName == "end_rebuy")
            {
                roomInfo.bRebuy = itfields->colValue.empty() ? true : false;
            }
            else if (itfields->colName == "add_on")
            {
                roomInfo.bAddon = itfields->colValue.empty() ? true : false;
            }
            else if (itfields->colName == "cur_player_count")
            {
                roomInfo.curPlayerCount = S2I(itfields->colValue);
            }
            else if (itfields->colName == "min_player_count")
            {
                roomInfo.minPlayerCount = S2I(itfields->colValue);
            }
            else if (itfields->colName == "max_player_count")
            {
                roomInfo.maxPlayerCount = S2I(itfields->colValue);
            }
            else if (itfields->colName == "game_status")
            {
                roomInfo.gameStatus = (room::E_MTT_STATUS)S2I(itfields->colValue);
            }
            else if (itfields->colName == "reward_info")
            {
                roomInfo.rewardInfo = itfields->colValue;
            }
            else if (itfields->colName == "blind_conf")
            {
                roomInfo.blindConf = itfields->colValue;
            }
            else if (itfields->colName == "begin_signup_time")
            {
                roomInfo.beginSignUpTime = ServiceUtil::GetTimeStamp(itfields->colValue);
            }
            else if (itfields->colName == "end_signup_time")
            {
                roomInfo.endSignUpTime = ServiceUtil::GetTimeStamp(itfields->colValue);
            }
            else if (itfields->colName == "begin_time")
            {
                roomInfo.beginTime = ServiceUtil::GetTimeStamp(itfields->colValue);
            }
            else if (itfields->colName == "create_time")
            {
                roomInfo.createTime = ServiceUtil::GetTimeStamp(itfields->colValue);
            }
            else if (itfields->colName == "base_score")
            {
                roomInfo.baseScore = S2L(itfields->colValue);
            }
            else if (itfields->colName == "rebuy_count")
            {
                roomInfo.rebuyCount = S2I(itfields->colValue);
            }
            else if (itfields->colName == "signup_fee")
            {
                roomInfo.signUpFee = itfields->colValue;
            }
            else if (itfields->colName == "tag")
            {
                roomInfo.iTag = S2I(itfields->colValue);
            }
        }
    }
    return 0;
}

int RoomProcessor::selectMatchRoomIndexList(const vector<vector<string>>& whlist, vector<string>& vecRoomIndex)
{
    dbagent::TDBReadReq rDataReq;
    rDataReq.keyIndex = 0;
    rDataReq.queryType = dbagent::E_SELECT;
    rDataReq.tableName = "tb_match_room_list";

    vector<dbagent::TField> fields;
    dbagent::TField tfield;
    tfield.colArithType = E_NONE;
    tfield.colName = "room_index";
    fields.push_back(tfield);
    rDataReq.fields = fields;

    vector<dbagent::ConditionGroup> conditionGroups;
    dbagent::ConditionGroup conditionGroup;
    conditionGroup.relation = dbagent::AND;
    vector<dbagent::Condition> conditions;

    dbagent::Condition condition;
    condition.condtion = dbagent::E_EQ;
    condition.colType = dbagent::STRING;

    condition.colName = "status";
    condition.colValues = "1";
    conditions.push_back(condition);

    for(auto item : whlist)
    {
        if(item.size() != 3)
        {
            continue;
        }
        condition.colName = item[0];
        condition.condtion = dbagent::Eum_Condition(S2I(item[1]));
        condition.colValues = item[2];
        condition.colType = condition.condtion == E_IN ? dbagent::INT : dbagent::STRING;
        conditions.push_back(condition);
    }

    conditionGroup.condition = conditions;
    conditionGroups.push_back(conditionGroup);
    rDataReq.conditions = conditionGroups;

    vector<dbagent::OrderBy> orderBys;
    dbagent::OrderBy orderBy;
    orderBy.sort = dbagent::DESC;
    orderBy.colName = "tag";
    orderBys.push_back(orderBy);
    orderBy.colName = "create_time";
    orderBys.push_back(orderBy);
    rDataReq.orderbyCol = orderBys;

    dbagent::TDBReadRsp dataRsp;
    int iRet = g_app.getOuterFactoryPtr()->getDBAgentServantPrx(0)->read(rDataReq, dataRsp);
    if (iRet != 0 || dataRsp.iResult != 0)
    {
        ROLLLOG_ERROR << "read data from dbagent failed, rDataReq:" << printTars(rDataReq) << ",dataRsp: " << printTars(dataRsp) << endl;
        return -1;
    }

    for (auto it = dataRsp.records.begin(); it != dataRsp.records.end(); ++it)
    {
        for (auto itfield = it->begin(); itfield != it->end(); ++itfield)
        {
            if (itfield->colName == "room_index")
            {
                vecRoomIndex.push_back(itfield->colValue);
            }
        }
    }
    return 0;
}

int RoomProcessor::selectSelfRoomIndexList(const long lPlayerID, vector<string>& vecRoomIndex)
{
    dbagent::TDBReadReq rDataReq;
    rDataReq.keyIndex = 0;
    rDataReq.queryType = dbagent::E_SELECT;
    rDataReq.tableName = "tb_match_room_member";

    vector<dbagent::TField> fields;
    dbagent::TField tfield;
    tfield.colArithType = E_NONE;
    tfield.colName = "room_index";
    fields.push_back(tfield);
    rDataReq.fields = fields;

    vector<dbagent::ConditionGroup> conditionGroups;
    dbagent::ConditionGroup conditionGroup;
    conditionGroup.relation = dbagent::AND;
    vector<dbagent::Condition> conditions;

    dbagent::Condition condition;
    condition.condtion = dbagent::E_EQ;
    condition.colType = dbagent::STRING;

    condition.colName = "uid";
    condition.colValues = L2S(lPlayerID);
    conditions.push_back(condition);

    condition.colName = "member_type";
    condition.colValues = "0";
    conditions.push_back(condition);

    conditionGroup.condition = conditions;
    conditionGroups.push_back(conditionGroup);
    rDataReq.conditions = conditionGroups;

    vector<dbagent::OrderBy> orderBys;
    dbagent::OrderBy orderBy;
    orderBy.sort = dbagent::DESC;
    orderBy.colName = "update_time";
    orderBys.push_back(orderBy);
    rDataReq.orderbyCol = orderBys;

    dbagent::TDBReadRsp dataRsp;
    int iRet = g_app.getOuterFactoryPtr()->getDBAgentServantPrx(0)->read(rDataReq, dataRsp);
    if (iRet != 0 || dataRsp.iResult != 0)
    {
        ROLLLOG_ERROR << "read data from dbagent failed, rDataReq:" << printTars(rDataReq) << ",dataRsp: " << printTars(dataRsp) << endl;
        return -1;
    }

    for (auto it = dataRsp.records.begin(); it != dataRsp.records.end(); ++it)
    {
        for (auto itfield = it->begin(); itfield != it->end(); ++itfield)
        {
            if (itfield->colName == "room_index")
            {
                vecRoomIndex.push_back(itfield->colValue);
            }
        }
    }
    return 0;
}

int RoomProcessor::selectMatchRoomList(const vector<vector<string>>& whlist ,vector<room::MatchRoomInfo>& roomInfoList)
{
    vector<string> vecRoomIndex;
    int iRet = selectMatchRoomIndexList(whlist, vecRoomIndex);
    for(auto roomIndex : vecRoomIndex)
    {
        room::MatchRoomInfo roomInfo;
        iRet = selectMatchRoom(roomIndex, roomInfo);
        if(iRet == 0)
        {
            roomInfoList.push_back(roomInfo);
        }
    }
    return 0;
}

int RoomProcessor::selectSelfRoomList(const long lPlayerID ,vector<room::MatchRoomInfo>& roomInfoList)
{
    vector<string> vecRoomIndex;
    int iRet = selectSelfRoomIndexList(lPlayerID, vecRoomIndex);
    for(auto roomIndex : vecRoomIndex)
    {
        room::MatchRoomInfo roomInfo;
        iRet = selectMatchRoom(roomIndex, roomInfo);
        if(iRet != 0 || roomInfo.gameStatus == 5)
        {
            continue;
        }
        roomInfoList.push_back(roomInfo);
    }
    return 0;
}

//
int RoomProcessor::createMatchRoom(const room::createMatchRoomReq& req, room::createMatchRoomResp& resp)
{
    dataproxy::TWriteDataReq wdataReq;
    wdataReq.resetDefautlt();
    wdataReq.keyName = I2S(E_REDIS_TYPE_HASH) + ":" + I2S(MATCH_ROOM_LIST) + ":" + req.roomIndex;
    wdataReq.operateType = E_REDIS_INSERT;
    wdataReq.paraExt.resetDefautlt();
    wdataReq.paraExt.queryType = dbagent::E_UPDATE;
    wdataReq.clusterInfo.resetDefautlt();
    wdataReq.clusterInfo.busiType = E_REDIS_PROPERTY;
    wdataReq.clusterInfo.frageFactorType = E_FRAGE_FACTOR_STRING;
    wdataReq.clusterInfo.frageFactor = tars::hash<string>()(req.roomIndex);

    vector<TField> fields;
    TField tfield;
    tfield.colArithType = E_NONE;
    for(auto item : tbMatchRoomInfo)
    {
        auto it = req.insertInfo.find(item.first);
        if(it == req.insertInfo.end())
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
    int iRet = g_app.getOuterFactoryPtr()->getDBAgentServantPrx(req.roomIndex)->redisWrite(wdataReq, wdataRsp);
    if (iRet != 0 || wdataRsp.iResult != 0)
    {
        ROLLLOG_ERROR << "redisWrite failed, iRet: " << iRet << ", wdataRsp.iResult: " << wdataRsp.iResult << endl;
        return -2;
    }
    return 0;
}

//
int RoomProcessor::updateMatchRoom(const room::updateMatchRoomReq& req, room::updateMatchRoomResp& resp)
{
    int iRet = 0;
    vector<string> vecRoomIndex;
    if(req.roomIndex.empty())
    {
        vector<vector<string>> whlist = {
            {"online_game", "0" , "1"},//=
        };

        iRet = selectMatchRoomIndexList(whlist, vecRoomIndex);
        if(iRet != 0)
        {
            ROLLLOG_ERROR << "select room index list err, iRet: " << iRet << endl;
            return iRet;
        }
    }
    else
    {
        vecRoomIndex.push_back(req.roomIndex);
    }

    for(auto roomIndex : vecRoomIndex)
    {
        ROLLLOG_DEBUG<< "roomIndex: "<< roomIndex << endl;
        room::MatchRoomInfo roomInfo;
        selectMatchRoom(roomIndex, roomInfo);

        dataproxy::TWriteDataReq wdataReq;
        wdataReq.resetDefautlt();
        wdataReq.keyName = I2S(E_REDIS_TYPE_HASH) + ":" + I2S(MATCH_ROOM_LIST) + ":" + roomIndex;
        wdataReq.operateType = E_REDIS_WRITE;
        wdataReq.paraExt.resetDefautlt();
        wdataReq.paraExt.queryType = dbagent::E_UPDATE;
        wdataReq.clusterInfo.resetDefautlt();
        wdataReq.clusterInfo.busiType = E_REDIS_PROPERTY;
        wdataReq.clusterInfo.frageFactorType = E_FRAGE_FACTOR_STRING;
        wdataReq.clusterInfo.frageFactor = tars::hash<string>()(roomIndex);;

        vector<TField> fields;
        TField tfield;
        tfield.colArithType = E_NONE;
        for(auto item : tbMatchRoomInfo)
        {
            auto it = req.updateInfo.find(item.first);
            if(it == req.updateInfo.end())
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
        iRet = g_app.getOuterFactoryPtr()->getDBAgentServantPrx(roomIndex)->redisWrite(wdataReq, wdataRsp);
        if (iRet != 0 || wdataRsp.iResult != 0)
        {
            ROLLLOG_ERROR << "redisWrite failed, iRet: " << iRet << ", wdataRsp.iResult: " << wdataRsp.iResult << endl;
            return iRet;
        }
    }
    return 0;
}

int RoomProcessor::updateMatchRoomMember(const room::updateRoomMemberReq &req, room::updateRoomMemberResp &resp)
{
    FUNC_ENTRY("");
    int iRet = 0;
    __TRY__

    iRet = selectMatchRoomMemberByID(req.roomIndex, req.uid, resp.memberInfo);
    if(iRet != 0)
    {
        LOG_ERROR << "select room member err. roomIndex:"<< req.roomIndex << ", uid:"<< req.uid << endl;
        return iRet;
    }

    dataproxy::TWriteDataReq wdataReq;
    wdataReq.resetDefautlt();
    wdataReq.keyName = I2S(E_REDIS_TYPE_LIST) + ":" + I2S(MATCH_ROOM_MEMBER) + ":" + req.roomIndex;
    wdataReq.operateType = resp.memberInfo.uid == 0 ? E_REDIS_INSERT : E_REDIS_WRITE;
    wdataReq.clusterInfo.resetDefautlt();
    wdataReq.clusterInfo.busiType = E_REDIS_PROPERTY;
    wdataReq.clusterInfo.frageFactorType = E_FRAGE_FACTOR_STRING;
    wdataReq.clusterInfo.frageFactor = tars::hash<string>()(req.roomIndex);

    vector<TField> fields;
    TField tfield;
    tfield.colArithType = E_NONE;
    for(auto item : tbMatchRoomMember)
    {
        auto it = req.updateInfo.find(item.first);
        if(it == req.updateInfo.end())
        {
            continue;
        }
        tfield.colName = item.first;
        tfield.colType = item.second;
        tfield.colValue = it->second;
        fields.push_back(tfield);
    }
    wdataReq.fields = fields;

    if(wdataReq.operateType == E_REDIS_WRITE)
    {
        wdataReq.paraExt.resetDefautlt();
        wdataReq.paraExt.queryType = dbagent::E_UPDATE;

        tfield.colName = "uid";
        tfield.colValue = L2S(req.uid);
        wdataReq.paraExt.fields.insert(std::make_pair("uid", tfield));
    }

    dataproxy::TWriteDataRsp wdataRsp;
    iRet = g_app.getOuterFactoryPtr()->getDBAgentServantPrx(req.roomIndex)->redisWrite(wdataReq, wdataRsp);
    if (iRet != 0 || wdataRsp.iResult != 0)
    {
        ROLLLOG_ERROR << "updateMatchRoomMember err, iRet: " << iRet << ", iResult: " << wdataRsp.iResult << endl;
        return -2;
    }

    ROLLLOG_DEBUG << "updateMatchRoomMember, iRet: " << iRet << ", wdataRsp: " << printTars(wdataRsp) << endl;

    selectMatchRoomMemberByID(req.roomIndex, req.uid, resp.memberInfo);

    __CATCH__
    FUNC_EXIT("", iRet);
    return iRet;
}

int RoomProcessor::parseMemberInfo(const vector<dbagent::TField> &fields, room::MemberInfo &memberInfo)
{
    for (auto itfield = fields.begin(); itfield != fields.end(); ++itfield)
    {
        if (itfield->colName == "uid")
        {
            memberInfo.uid = S2L(itfield->colValue);
        }
        else if (itfield->colName == "member_type")
        {
            memberInfo.memberType = S2I(itfield->colValue);
        }
        else if (itfield->colName == "score")
        {
            memberInfo.score = S2L(itfield->colValue);
        }
        else if (itfield->colName == "update_time")
        {
            string value = itfield->colValue;
            memberInfo.updateTime = ServiceUtil::GetTimeStamp(value);
        }
        else if (itfield->colName == "table_id")
        {
            memberInfo.iTableID = S2I(itfield->colValue);
        }
        else if (itfield->colName == "chair_id")
        {
            memberInfo.iChairID = S2I(itfield->colValue);
        }
        else if (itfield->colName == "room_index")
        {
            memberInfo.roomIndex = itfield->colValue;
        }
        else if (itfield->colName == "rank")
        {
            memberInfo.iRank = S2I(itfield->colValue);
        }
        else if (itfield->colName == "reward_info")
        {
            memberInfo.rewardInfo = itfield->colValue;
        }
    }
    return 0;
}

int RoomProcessor::selectMatchRoomMember(const string& roomIndex, vector<room::MemberInfo>& vecMemberInfo)
{
    int iRet = 0;

    dataproxy::TReadDataReq rdataReq;
    rdataReq.resetDefautlt();
    rdataReq.keyName = I2S(E_REDIS_TYPE_HASH) + ":" + I2S(MATCH_ROOM_MEMBER) + ":" + roomIndex;
    rdataReq.operateType = E_REDIS_READ;
    rdataReq.clusterInfo.resetDefautlt();
    rdataReq.clusterInfo.busiType = E_REDIS_PROPERTY;
    rdataReq.clusterInfo.frageFactorType = E_FRAGE_FACTOR_STRING;
    rdataReq.clusterInfo.frageFactor = tars::hash<string>()(roomIndex);;

    vector<TField> fields;
    TField tfield;
    tfield.colArithType = E_NONE;
    for(auto item : tbMatchRoomMember)
    {
        tfield.colName = item.first;
        fields.push_back(tfield);
    }
    rdataReq.fields = fields;

    TReadDataRsp rdataRsp;
    iRet = g_app.getOuterFactoryPtr()->getDBAgentServantPrx(roomIndex)->redisRead(rdataReq, rdataRsp);
    if ((iRet != 0) || (rdataRsp.iResult != 0))
    {
        ROLLLOG_ERROR << "load room member cache failed, iRet: " << iRet << ", iResult: " << rdataRsp.iResult << endl;
        return -2;
    }

    LOG_DEBUG << "rdataReq:"<< printTars(rdataReq)<<", rdataRsp:"<< printTars(rdataRsp)<< endl;
    for (auto it = rdataRsp.fields.begin(); it != rdataRsp.fields.end(); ++it)
    {
        MemberInfo memberInfo;
        iRet = parseMemberInfo(*it, memberInfo);
        if(iRet != 0)
        {
            continue;
        }
        vecMemberInfo.push_back(memberInfo);
    }

    return 0;
}

int RoomProcessor::selectMatchRoomMemberByID(const string& roomIndex, const long uid, room::MemberInfo& memberInfo)
{
    int iRet = 0;

    dataproxy::TReadDataReq rdataReq;
    rdataReq.resetDefautlt();
    rdataReq.keyName = I2S(E_REDIS_TYPE_HASH) + ":" + I2S(MATCH_ROOM_MEMBER) + ":" + roomIndex;
    rdataReq.operateType = E_REDIS_READ;
    rdataReq.clusterInfo.resetDefautlt();
    rdataReq.clusterInfo.busiType = E_REDIS_PROPERTY;
    rdataReq.clusterInfo.frageFactorType = E_FRAGE_FACTOR_STRING;
    rdataReq.clusterInfo.frageFactor = tars::hash<string>()(roomIndex);;

    vector<TField> fields;
    TField tfield;
    tfield.colArithType = E_NONE;
    for(auto item : tbMatchRoomMember)
    {
        tfield.colName = item.first;
        fields.push_back(tfield);
    }
    rdataReq.fields = fields;

    tfield.colName = "uid";
    tfield.colValue = L2S(uid);
    rdataReq.paraExt.fields.insert(std::make_pair("uid", tfield));

    TReadDataRsp rdataRsp;
    iRet = g_app.getOuterFactoryPtr()->getDBAgentServantPrx(roomIndex)->redisRead(rdataReq, rdataRsp);
    if ((iRet != 0) || (rdataRsp.iResult != 0))
    {
        ROLLLOG_ERROR << "select member cache failed, iRet: " << iRet << ", iResult: " << rdataRsp.iResult << endl;
        return -2;
    }

    LOG_DEBUG << "rdataRsp:"<< printTars(rdataRsp)<< endl;
    for (auto it = rdataRsp.fields.begin(); it != rdataRsp.fields.end(); ++it)
    {
        iRet = parseMemberInfo(*it, memberInfo);
        if(iRet != 0)
        {
            continue;
        }
    }

    return 0;
}

int RoomProcessor::deleteMatchRoomMember(const string& roomIndex)
{
    dataproxy::TWriteDataReq req;
    req.resetDefautlt();
    req.keyName = I2S(E_REDIS_TYPE_HASH) + ":" + I2S(MATCH_ROOM_MEMBER) + ":" + roomIndex;
    req.operateType = E_REDIS_DELETE;
    req.clusterInfo.resetDefautlt();
    req.clusterInfo.busiType = E_REDIS_PROPERTY;
    req.clusterInfo.frageFactorType = E_FRAGE_FACTOR_STRING;
    req.clusterInfo.frageFactor = tars::hash<string>()(roomIndex);

    dataproxy::TWriteDataRsp rsp;
    int iRet = g_app.getOuterFactoryPtr()->getDBAgentServantPrx(roomIndex)->redisWrite(req, rsp);
    if (iRet != 0 || rsp.iResult != 0)
    {
        ROLLLOG_ERROR << "delete room member err, iRet: " << iRet << ", iResult: " << rsp.iResult << endl;
        return iRet;
    }

    return iRet;
}
