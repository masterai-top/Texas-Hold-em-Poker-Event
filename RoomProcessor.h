#ifndef _RoomInfo_Processor_H_
#define _RoomInfo_Processor_H_

//
#include <util/tc_singleton.h>
#include "RoomProto.h"
#include "DBAgentProto.h"
#include "DataProxyProto.h"

#include "Hall.pb.h"

//
using namespace tars;
using namespace std;
using namespace room;


/**
 *RoomProcessor
 *
 */
class RoomProcessor
{
public:
    /**
     *
    */
    RoomProcessor();

    /**
     *
    */
    ~RoomProcessor();

public:
    //
    int selectMatchRoom(const string& roomIndex, room::MatchRoomInfo& roomInfo);
    //
    int selectMatchRoomList(const vector<vector<string>>& whlist, vector<room::MatchRoomInfo>& roomInfoList);
    //
    int selectSelfRoomList(const long lPlayerID ,vector<room::MatchRoomInfo>& roomInfoList);
    //
    int createMatchRoom(const room::createMatchRoomReq& req, room::createMatchRoomResp& resp);
    //
    int updateMatchRoom(const room::updateMatchRoomReq& req, room::updateMatchRoomResp& resp);
    //
    int updateMatchRoomMember(const room::updateRoomMemberReq &req, room::updateRoomMemberResp &resp);
    //
    int selectMatchRoomMember(const string& roomIndex, vector<room::MemberInfo>& vecMemberInfo);
    //
    int selectMatchRoomMemberByID(const string& roomIndex, const long uid, room::MemberInfo& memberInfo);
    //
    int deleteMatchRoomMember(const string& roomIndex);

public:
    //
    int selectMatchRoomIndexList(const vector<vector<string>>& whlist, vector<string>& vecRoomIndex);
    //
    int selectSelfRoomIndexList(const long lPlayerID, vector<string>& vecRoomIndex);
    //
    int parseMemberInfo(const vector<dbagent::TField> &fields, room::MemberInfo &memberInfo);
};

//singleton
typedef TC_Singleton<RoomProcessor, CreateStatic, DefaultLifetime> RoomProcessorSingleton;

#endif

