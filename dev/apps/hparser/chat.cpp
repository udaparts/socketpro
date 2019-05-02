
#include "stdafx.h"
#include "chat.h"
#include <boost/unordered_map.hpp>

namespace Chatting {
    boost::unordered_map<unsigned int, std::wstring> g_mapChatGroup;
    SPA::CUCriticalSection g_csChat;

    void AddAChatGroup(unsigned int nChatGroupId, const wchar_t *strDescription) {
        if (strDescription == NULL)
            strDescription = L"";
        g_csChat.lock();
        g_mapChatGroup[nChatGroupId] = strDescription;
        g_csChat.unlock();
    }

    unsigned int GetCountOfChatGroups() {
        g_csChat.lock();
        unsigned int count = (unsigned int) g_mapChatGroup.size();
        g_csChat.unlock();
        return count;
    }

    unsigned int GetJoinedGroupIds(unsigned int *pChatGroupId, unsigned int count) {
        unsigned int n = 0;
        if (pChatGroupId == NULL)
            count = 0;
        g_csChat.lock();
        boost::unordered_map<unsigned int, std::wstring>::const_iterator it, end = g_mapChatGroup.cend();
        for (it = g_mapChatGroup.cbegin(); count && it != end; ++it, --count, ++n) {
            pChatGroupId[n] = it->first;
        }
        g_csChat.unlock();
        return n;
    }

    unsigned int GetAChatGroup(unsigned int chatGroupId, wchar_t *description, unsigned int chars) {
        if (description == NULL)
            chars = 0;
        if (chars) {
            --chars;
            if (chars) {
                g_csChat.lock();
                boost::unordered_map<unsigned int, std::wstring>::const_iterator it = g_mapChatGroup.find(chatGroupId);
                if (it != g_mapChatGroup.cend()) {
                    const std::wstring &des = it->second;
                    if (des.size() > chars)
                        chars = (unsigned int) des.size();
                    ::memcpy(description, des.c_str(), chars * sizeof (wchar_t));
                }
                g_csChat.unlock();
            }
            description[chars] = 0;
        }
        return chars;
    }

    void RemoveChatGroup(unsigned int chatGroupId) {
        g_csChat.lock();
        g_mapChatGroup.erase(chatGroupId);
        g_csChat.unlock();
    }

    void ClearAllChatGroups() {
        g_csChat.lock();
        g_mapChatGroup.clear();
        g_csChat.unlock();
    }

    bool Seek(unsigned int GroupId) {
        boost::unordered_map<unsigned int, std::wstring>::const_iterator it = g_mapChatGroup.find(GroupId);
        return (it != g_mapChatGroup.cend());
    }

    void ChatGroupLogicAnd(SPA::CUQueue &qGroup) {
        int n, count = qGroup.GetSize() / sizeof (unsigned int);
        unsigned int *p = (unsigned int *) qGroup.GetBuffer();
        for (n = count - 1; n >= 0; --n) {
            unsigned int grpId = p[n];
            g_csChat.lock();
            if (!Seek(grpId)) {
                g_csChat.unlock();
                qGroup.Pop((unsigned int) (sizeof (unsigned int)), n * sizeof (unsigned int));
            } else
                g_csChat.unlock();
        }
    }

    void ChatGroupLogicOr(SPA::CUQueue &qGroup) {
        ChatGroupLogicAnd(qGroup);
        g_csChat.lock();
        boost::unordered_map<unsigned int, std::wstring>::const_iterator it, end = g_mapChatGroup.cend();
        for (it = g_mapChatGroup.cbegin(); it != end; ++it) {
            qGroup << it->first;
        }
        g_csChat.unlock();
    }

    void ChatGroupLogicXor(SPA::CUQueue &qGroup) {

    }

    void GetJoinedGroupIds(std::vector<unsigned int> &vChatGroup) {
        vChatGroup.clear();
        SPA::CAutoLock al(g_csChat);
        boost::unordered_map<unsigned int, std::wstring>::const_iterator it, end = g_mapChatGroup.cend();
        for (it = g_mapChatGroup.cbegin(); it != end; ++it) {
            vChatGroup.push_back(it->first);
        }
    }
};