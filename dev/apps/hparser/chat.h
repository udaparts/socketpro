
#pragma once


namespace Chatting {
    void AddAChatGroup(unsigned int chatGroupId, const wchar_t *description);
    unsigned int GetCountOfChatGroups();
    unsigned int GetJoinedGroupIds(unsigned int *pChatGroupId, unsigned int count);
    unsigned int GetAChatGroup(unsigned int chatGroupId, wchar_t *description, unsigned int chars);
    void RemoveChatGroup(unsigned int chatGroupId);
    void GetJoinedGroupIds(std::vector<unsigned int> &vChatGroup);
    void ClearAllChatGroups();
    void ChatGroupLogicAnd(SPA::CUQueue &qGroup);
    void ChatGroupLogicOr(SPA::CUQueue &qGroup);
    void ChatGroupLogicXor(SPA::CUQueue &qGroup);
};



